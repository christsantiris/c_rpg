#include "test_utils.h"
#include "game/actions.h"
#include "game/approach.h"
#include "game/boss_gates.h"
#include "game/coast_exits.h"
#include "game/crownroad_north.h"
#include "game/eastern_arc.h"
#include "game/elderwood.h"
#include "game/giants_crown.h"
#include "game/jail.h"
#include "game/stillbury_routes.h"
#include "game/town_road.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define ROUTE_CELLS (MAP_W * MAP_H)
#define ROUTE_SLOT 99274

static int route_path(GameState *g, int tx, int ty) {
    int *previous = malloc(sizeof(int) * ROUTE_CELLS);
    int *queue = malloc(sizeof(int) * ROUTE_CELLS);
    if (!previous || !queue) {
        free(previous);
        free(queue);
        return 0;
    }
    for (int i = 0; i < ROUTE_CELLS; i++) {
        previous[i] = -1;
    }
    int start = g->player.y * MAP_W + g->player.x;
    int target = ty * MAP_W + tx;
    int head = 0;
    int tail = 1;
    previous[start] = start;
    queue[0] = start;
    const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail && previous[target] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + steps[side][0];
            int y = cell / MAP_W + steps[side][1];
            if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
                continue;
            }
            int next = y * MAP_W + x;
            TileType tile = g->map.tiles[y][x];
            int transition = (map_is_travel_tile(tile) && tile != TILE_ROAD_GATE_OPEN) || tile == TILE_SWAMP_ENTRANCE || tile == TILE_SWAMP_EXIT || tile == TILE_COAST_ENTRANCE || tile == TILE_COAST_EXIT || tile == TILE_ASHEN_ENTRANCE || tile == TILE_ASHEN_EXIT;
            int occupied = 0;
            for (int i = 0; i < g->enemy_count; i++) {
                occupied |= g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y;
            }
            int entrance = tile == TILE_NORTH_SWAMP || tile == TILE_NORTH_CATACOMBS;
            if (previous[next] >= 0 || occupied || (!map_is_walkable(&g->map, x, y) && !(entrance && next == target)) || (transition && next != target)) {
                continue;
            }
            previous[next] = cell;
            queue[tail++] = next;
        }
    }
    int ok = previous[target] >= 0;
    int count = 0;
    if (ok) {
        for (int cell = target; cell != start; cell = previous[cell]) {
            queue[count++] = cell;
        }
        Location location = g->location;
        int level = g->level;
        int route = g->approach_route;
        for (int i = count - 1; i >= 0; i--) {
            int x = queue[i] % MAP_W;
            int y = queue[i] / MAP_W;
            action_resolve_player(g, (Action){ACTION_MOVE, x, y});
            if (location == LOCATION_ESCAPE_TUNNEL && g->location == location) {
                action_resolve_enemies(g);
            }
            if (g->location != location || g->level != level || g->approach_route != route) {
                ok = i == 0;
                break;
            }
            if (g->player.x != x || g->player.y != y) {
                ok = 0;
                break;
            }
        }
    }
    free(previous);
    free(queue);
    return ok;
}

static void clear_enemies(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
}

static void northern_route(GameState *g) {
    game_enter_town2(g);
    ASSERT("Stillbury's north gate remains usable before any Act 4 quest or boss", route_path(g, 20, 0) && g->location == LOCATION_TOWN_ROAD);
    int x;
    int y;
    town_road_exit(TOWN_SIDE_NORTH, &x, &y);
    ASSERT("north from Stillbury reaches Blackwater with a northward inward path", route_path(g, x, y) && g->approach_route == APPROACH_BLACKWATER_ROAD && g->player.y == APPROACH_H - 2 && g->player.last_dy == -1);
    ASSERT("the demon blocks the road toward the Catacombs", !route_path(g, NORTH_CAT_X, NORTH_CAT_Y));
    ASSERT("the visible swamp door enters the south boundary facing north", route_path(g, NORTH_SWAMP_X, NORTH_SWAMP_Y) && g->location == LOCATION_SWAMP && g->level == 1 && g->player.y == SWAMP_MAP_H - 2 && g->player.last_dy == -1);
    for (int stage = 1; stage <= SWAMP_DEPTH; stage++) {
        clear_enemies(g);
        ASSERT("every generated swamp stage retains a working retreat boundary", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && (stage == 1 ? g->approach_route == APPROACH_BLACKWATER_ROAD : g->location == LOCATION_SWAMP && g->level == stage - 1));
        if (stage == 1) {
            ASSERT("retreating from Blackwater returns to its doorway south of the demon gate", route_path(g, NORTH_SWAMP_X, NORTH_SWAMP_Y) && g->location == LOCATION_SWAMP);
        } else {
            clear_enemies(g);
            ASSERT("cached swamp stages can be crossed again", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->level == stage);
        }
        clear_enemies(g);
        if (stage < SWAMP_DEPTH) {
            ASSERT("every swamp onward boundary reaches the next stage", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_SWAMP && g->level == stage + 1 && g->player.last_dx == 1);
        }
    }
    g->defeated_bosses |= 1u << LOCATION_SWAMP;
    ASSERT("the demon victory returns onto the newly opened northern road", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && northern_approach(g) && g->player.y < NORTH_SWAMP_BARRIER_Y && g->player.last_dx == 1 && map_is_walkable(&g->map, g->player.x + 1, g->player.y));
    ASSERT("Blackwater leads to the Catacombs without clearing Elderwood", !g->elderwood_defeated && route_path(g, NORTH_CAT_X, NORTH_CAT_Y) && g->location == LOCATION_CATACOMBS && g->catacombs_entry_town == LOCATION_TOWN2);
    for (int stage = 1; stage <= CATACOMBS_DEPTH; stage++) {
        clear_enemies(g);
        ASSERT("each Catacombs floor has a reachable retreat stair", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y));
        action_resolve_player(g, (Action){ACTION_ASCEND, 0, 0});
        ASSERT("the actual Catacombs upstairs permits retreat", stage == 1 ? northern_approach(g) : g->location == LOCATION_CATACOMBS && g->level == stage - 1);
        if (stage == 1) {
            ASSERT("the Catacombs doorway permits a real revisit", route_path(g, NORTH_CAT_X, NORTH_CAT_Y) && g->location == LOCATION_CATACOMBS);
        } else {
            clear_enemies(g);
            ASSERT("cached Catacombs floors retain working downstairs", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y));
            action_resolve_player(g, (Action){ACTION_DESCEND, 0, 0});
            ASSERT("cached Catacombs descent returns to the next floor", g->location == LOCATION_CATACOMBS && g->level == stage);
        }
        clear_enemies(g);
        ASSERT("each Catacombs floor has a reachable onward stair or objective", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y));
        action_resolve_player(g, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("only the Grave Marshal gates the final Catacombs passage", g->location == LOCATION_CATACOMBS && g->level == (stage < CATACOMBS_DEPTH ? stage + 1 : stage));
    }
    g->defeated_bosses |= 1u << LOCATION_CATACOMBS;
    action_resolve_player(g, (Action){ACTION_DESCEND, 0, 0});
    ASSERT("the final Catacombs passage opens after the Marshal's defeat", northern_approach(g));
    ASSERT("the cleared northern road reaches Crown Road North", route_path(g, NORTH_ROAD_X, 0) && g->location == LOCATION_CROWNROAD_NORTH && g->enemy_count == NORTH_ENEMIES);
    ASSERT("Crown Road North retains retreat around its living patrols", route_path(g, CROWN_NORTH_X, CROWN_NORTH_H - 1) && northern_approach(g));
    ASSERT("the party can reenter the cached Crown Road North", route_path(g, NORTH_ROAD_X, 0) && g->location == LOCATION_CROWNROAD_NORTH);
    ASSERT("the actual northern Crown Road exit reaches Castle Grounds", route_path(g, CROWN_NORTH_X, 0) && g->location == LOCATION_CASTLE && g->player.last_dy == -1);
    ASSERT("the old Ashen gate is closed in the castle's northern boundary", g->map.tiles[0][CASTLE_ASHEN_GATE_X] == TILE_WALL);
    ASSERT("Castle Grounds retain the southward retreat", route_path(g, CROWNROAD_X, TOWN_H - 1) && g->location == LOCATION_CROWNROAD_NORTH && g->player.last_dy == 1);
    ASSERT("the cached Crown Road returns to the Catacombs road", route_path(g, CROWN_NORTH_X, CROWN_NORTH_H - 1) && northern_approach(g));
    ASSERT("the northern castle journey returns to Stillbury", route_path(g, NORTH_ROAD_X, APPROACH_H - 1) && g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN2);
    approach_enter_region(g, APPROACH_BLACKWATER_ROAD);
    ASSERT("the Catacombs retreat can be entered again", route_path(g, NORTH_CAT_X, NORTH_CAT_Y) && g->location == LOCATION_CATACOMBS);
    game_leave_catacombs(g);
    ASSERT("Catacombs retreat stays on Stillbury's northern road", northern_approach(g) && route_path(g, NORTH_ROAD_X, APPROACH_H - 1) && g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN2);
}

static void eastern_route(GameState *g) {
    game_init(g);
    game_enter_town2(g);
    ASSERT("Stillbury's east gate opens onto Elderwood", route_path(g, TOWN_W - 1, 12) && g->location == LOCATION_TOWN_ROAD);
    int x;
    int y;
    town_road_exit(TOWN_SIDE_EAST, &x, &y);
    ASSERT("the eastern exterior boundary reaches Elderwood facing east", route_path(g, x, y) && g->approach_route == APPROACH_ELDERWOOD_ROAD && g->player.x == 1 && g->player.last_dx == 1);
    ASSERT("Elderthorn guards the route to the coast independently of the demon", !route_path(g, APPROACH_W - 1, APPROACH_H / 2));
    ASSERT("Elderwood's visible doorway starts its five-stage journey", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_ELDERWOOD && g->player.last_dy == -1);
    for (int stage = 1; stage <= ELDERWOOD_DEPTH; stage++) {
        clear_enemies(g);
        ASSERT("Elderwood transitions lie on the actual outdoor boundaries", g->map.stairs_up_y == ELDERWOOD_H - 1 && g->map.stairs_down_y == 0);
        ASSERT("every Elderwood stage permits real retreat", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && (stage == 1 ? g->approach_route == APPROACH_ELDERWOOD_ROAD : g->location == LOCATION_ELDERWOOD && g->level == stage - 1));
        if (stage == 1) {
            ASSERT("the Elderwood doorway remains usable on a revisit", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_ELDERWOOD);
        } else {
            clear_enemies(g);
            ASSERT("cached Elderwood stages preserve northward travel", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->level == stage && g->player.last_dy == -1);
        }
        clear_enemies(g);
        if (stage < ELDERWOOD_DEPTH) {
            ASSERT("every Elderwood onward boundary reaches the next stage", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->level == stage + 1 && g->player.y == ELDERWOOD_H - 2 && g->player.last_dy == -1);
        }
    }
    elderwood_defeat(g);
    ASSERT("Elderthorn's victory returns to the eastern road", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->approach_route == APPROACH_ELDERWOOD_ROAD && g->player.y < g->map.stairs_down_y - 5 && g->player.last_dy == -1 && map_is_walkable(&g->map, g->player.x, g->player.y - 1));
    ASSERT("the eastern road reaches the coast without a swamp victory", !(g->defeated_bosses & (1u << LOCATION_SWAMP)) && route_path(g, APPROACH_W - 1, APPROACH_H / 2) && g->approach_route == APPROACH_COAST_ROAD && g->player.last_dx == 1);
    ASSERT("the Sunken Coast landmark can be entered from Elderwood", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_COAST);
    game_return_to_approach(g);
    ASSERT("physical retreat from the coast returns toward Elderwood", g->approach_route == APPROACH_COAST_ROAD && route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && g->approach_route == APPROACH_ELDERWOOD_ROAD && g->player.last_dx == -1);
    ASSERT("west through cleared Elderwood returns to Stillbury", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN2);
}

static int legacy_version(int version) {
    FILE *file = fopen("saves/savegame_99274.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = malloc(size + 1);
    size_t n = fread(data, 1, size, file);
    fclose(file);
    data[n] = 0;
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99274.json", "wb");
    int ok = file && data && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static void local_adventures(GameState *g, GameState *loaded) {
    const int roads[] = {APPROACH_QUARRY_ROAD, APPROACH_ELDERWOOD_ROAD, APPROACH_COAST_ROAD};
    for (int i = 0; i < 3; i++) {
        game_init(g);
        g->elderwood_defeated = 1;
        g->defeated_bosses |= (1u << LOCATION_SWAMP) | (1u << LOCATION_CATACOMBS) | (1u << LOCATION_COAST);
        approach_enter_region(g, roads[i]);
        int closed = 1;
        for (int x = 0; x < APPROACH_W; x++) {
            closed &= g->map.tiles[0][x] == TILE_MOUNTAIN_WALL && !map_is_walkable(&g->map, x, 0);
        }
        ASSERT("side-adventure cliffs never open a northern road, even after all regional victories", closed);
        g->player.hp = 31;
        g->gold = 321;
        int x = g->player.x;
        int y = g->player.y;
        int potion = g->inventory_count;
        g->inventory[g->inventory_count++] = item_make_health_potion();
        action_resolve_player(g, (Action){ACTION_DROP_ITEM, potion, 0});
        ASSERT("cliff migration preserves the player's exact position, wounds and dropped loot", save_game(g, ROUTE_SLOT) && legacy_version(247) && load_game(loaded, ROUTE_SLOT) && loaded->player.x == x && loaded->player.y == y && loaded->player.hp == 31 && loaded->gold == 321 && loaded->floor_items[0].active && loaded->floor_items[0].x == x && loaded->floor_items[0].y == y);
        ASSERT("local side adventures keep their real retreat to Stillbury's roads", route_path(loaded, loaded->map.stairs_up_x, loaded->map.stairs_up_y) && (loaded->location == LOCATION_TOWN_ROAD ? loaded->road_town == LOCATION_TOWN2 : roads[i] == APPROACH_QUARRY_ROAD ? approach_bridge_road(loaded) : loaded->approach_route == APPROACH_ELDERWOOD_ROAD));
    }
    TownApproach connection;
    ASSERT("neither northern town has a southern bypass into Stillbury's side adventures", !town_road_connection(LOCATION_TOWN3, TOWN_SIDE_SOUTH, &connection) && !town_road_connection(LOCATION_TOWN4, TOWN_SIDE_SOUTH, &connection));
    ASSERT("Rosemoor retains Frostfell and now owns Ashen Hollow's approach", town_road_connection(LOCATION_TOWN3, TOWN_SIDE_NORTH, &connection) && connection.area == LOCATION_FROSTFELL && town_road_connection(LOCATION_TOWN3, TOWN_SIDE_WEST, &connection) && connection.area == LOCATION_ASHEN);
}

static void ashen_route(GameState *g) {
    game_init(g);
    g->location = LOCATION_TOWN3;
    map_generate_town3(&g->map, &g->player.x, &g->player.y);
    g->player.x = 1;
    g->player.y = ROSEMOOR_ASHEN_GATE_Y;
    ASSERT("Rosemoor's real western gate reaches its exterior", route_path(g, 0, ROSEMOOR_ASHEN_GATE_Y) && g->location == LOCATION_TOWN_ROAD);
    int x;
    int y;
    town_road_exit(TOWN_SIDE_WEST, &x, &y);
    ASSERT("Rosemoor's western boundary reaches Ashen's relocated approach facing west", route_path(g, x, y) && g->location == LOCATION_APPROACH && g->approach_route == 13 && g->player.x == APPROACH_W - 2 && g->player.last_dx == -1);
    ASSERT("Ashen's actual doorway remains usable", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_ASHEN);
    for (int stage = 1; stage <= ASHEN_DEPTH; stage++) {
        clear_enemies(g);
        ASSERT("every Ashen stage permits real retreat after relocation", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && (stage == 1 ? g->location == LOCATION_APPROACH && g->approach_route == 13 : g->location == LOCATION_ASHEN && g->level == stage - 1));
        if (stage > 1) {
            clear_enemies(g);
        }
        ASSERT("Ashen's relocated doorway and cached stages permit reentry", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_ASHEN && g->level == stage);
        clear_enemies(g);
        ASSERT("each Ashen onward transition remains usable without defeating the optional boss", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && (stage == ASHEN_DEPTH ? g->location == LOCATION_APPROACH && g->approach_route == 13 : g->location == LOCATION_ASHEN && g->level == stage + 1));
    }
    ASSERT("Ashen's final departure returns to Rosemoor, never Castle Grounds", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN3 && g->player.last_dx == 1);
}

static void escape_route(GameState *g, GameState *loaded) {
    game_init(g);
    g->location = LOCATION_CASTLE;
    map_generate_castle(&g->map, &g->player.x, &g->player.y);
    g->player.x = INFORMANT_X;
    g->player.y = INFORMANT_Y + 1;
    ASSERT("the existing arrest still starts the escape story", jail_talk_nearby(g) && g->location == LOCATION_JAIL);
    g->player.x = JAIL_PRISONER_X;
    g->player.y = JAIL_PRISONER_Y + 1;
    ASSERT("Tomas now offers an escape to Stillbury", jail_talk_nearby(g) && strstr(g->dialogue_text, "Stillbury") && game_quest_offer_active(g));
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "The tunnel leads to Ridgeshire.");
    ASSERT("a legacy unanswered escape offer updates its destination without accepting it", save_game(g, ROUTE_SLOT) && legacy_version(247) && load_game(loaded, ROUTE_SLOT) && loaded->jail_quest_state == 1 && game_quest_offer_active(loaded) && strstr(loaded->dialogue_text, "Stillbury"));
    game_handle_quest_offer_key(loaded, SDL_SCANCODE_Y, 0);
    ASSERT("the real prison hatch enters the existing tunnel", route_path(loaded, JAIL_HATCH_X, JAIL_HATCH_Y) && loaded->location == LOCATION_ESCAPE_TUNNEL && loaded->jail_quest_state == 2);
    loaded->enemies[0].hp = 7;
    int count = loaded->enemy_count;
    int gear = loaded->jail_gear_count;
    loaded->player.hp = 31;
    ASSERT("a saved tunnel escort keeps enemies, wounds and confiscated equipment", save_game(loaded, ROUTE_SLOT) && legacy_version(247) && load_game(g, ROUTE_SLOT) && g->location == LOCATION_ESCAPE_TUNNEL && g->jail_quest_state == 2 && g->enemies[0].hp == 7 && g->enemy_count == count && g->jail_gear_count == gear && g->player.hp == 31 && map_is_explored(&g->map, ESCAPE_TUNNEL_W - 1, 14));
    clear_enemies(g);
    int gold = g->gold;
    int score = g->score;
    ASSERT("the escorted player leaves through the tunnel's actual boundary into Stillbury", route_path(g, ESCAPE_TUNNEL_W - 1, 14) && g->location == LOCATION_TOWN2 && g->jail_quest_state == 3 && g->last_town_visited == LOCATION_TOWN2 && g->player.last_dx == 1);
    ASSERT("Stillbury arrival pays the existing escape reward once and retains equipment recovery", g->gold == gold + ESCAPE_REWARD_GOLD && g->score == score + ESCAPE_REWARD_SCORE && g->jail_gear_count == gear && jail_prisoner_at(g, g->prisoner_x, g->prisoner_y) && !jail_move_exit(g, ESCAPE_TUNNEL_W - 1, 14));
    ASSERT("completed escape saves keep Tomas in Stillbury without paying again", save_game(g, ROUTE_SLOT) && load_game(loaded, ROUTE_SLOT) && loaded->jail_quest_state == 3 && loaded->gold == g->gold && jail_prisoner_at(loaded, loaded->prisoner_x, loaded->prisoner_y));
    game_init(g);
    g->location = LOCATION_CASTLE;
    map_generate_castle(&g->map, &g->player.x, &g->player.y);
    g->map.tiles[0][CASTLE_ASHEN_GATE_X] = TILE_TOWN_EXIT;
    g->player.x = CASTLE_ASHEN_GATE_X;
    g->player.y = 0;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = CASTLE_ASHEN_GATE_X, .y = 0, .underlying_tile = TILE_TOWN_EXIT, .item = item_make_greatsword()};
    g->floor_items[0].item.sharpened = 1;
    ASSERT("an old castle Ashen gate closes without stranding the player or deleting boundary loot", save_game(g, ROUTE_SLOT) && legacy_version(247) && load_game(loaded, ROUTE_SLOT) && loaded->map.tiles[0][CASTLE_ASHEN_GATE_X] == TILE_WALL && loaded->player.y == 1 && loaded->floor_items[0].active && loaded->floor_items[0].y == 1 && loaded->floor_items[0].underlying_tile == TILE_TOWN_FLOOR && loaded->floor_items[0].item.sharpened);
    game_init(g);
    g->emberforge_quest_state = 1;
    game_enter_ashen(g);
    g->enemies[0].hp = 11;
    game_descend(g);
    g->enemies[0].hp = 7;
    g->player.hp = 37;
    ASSERT("an existing Ashen expedition retains active and cached maps, wounds and quests", save_game(g, ROUTE_SLOT) && legacy_version(247) && load_game(loaded, ROUTE_SLOT) && loaded->player.hp == 37 && loaded->enemies[0].hp == 7 && loaded->ashen_cache[0].enemies[0].hp == 11 && loaded->emberforge_quest_state == 1 && memcmp(loaded->map.tiles, g->map.tiles, sizeof(g->map.tiles)) == 0 && memcmp(loaded->ashen_cache[0].map.tiles, g->ashen_cache[0].map.tiles, sizeof(g->map.tiles)) == 0);
    game_return_to_approach(loaded);
    ASSERT("an old active Ashen save retreats toward Rosemoor's real western gate", loaded->location == LOCATION_APPROACH && loaded->approach_route == 13 && route_path(loaded, loaded->map.stairs_up_x, loaded->map.stairs_up_y) && loaded->location == LOCATION_TOWN_ROAD && loaded->road_town == LOCATION_TOWN3);
}

void test_stillbury_routes(void) {
    printf("Stillbury Blackwater / Elderwood geography:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("geography fixtures allocate and the isolated save slot is unused", g && loaded && !save_exists(ROUTE_SLOT));
    if (!g || !loaded || save_exists(ROUTE_SLOT)) {
        free(g);
        free(loaded);
        return;
    }
    const int seeds[] = {19, 107, 1301};
    for (int seed = 0; seed < 3; seed++) {
        srand(seeds[seed]);
        game_init(g);
        northern_route(g);
        eastern_route(g);
        ashen_route(g);
    }
    local_adventures(g, loaded);
    escape_route(g, loaded);
    game_init(g);
    approach_enter_region(g, APPROACH_ELDERWOOD_ROAD);
    elderwood_enter(g);
    g->player.hp = 31;
    g->enemies[0].hp = 7;
    g->map.tiles[46][32] = TILE_ELDERWOOD_ENTRANCE;
    g->map.tiles[1][32] = TILE_ELDERWOOD_EXIT;
    g->map.tiles[47][32] = TILE_ELDERWOOD_WALL;
    g->map.tiles[0][32] = TILE_ELDERWOOD_WALL;
    elderwood_store(g);
    g->elderwood_entry_town = LOCATION_CASTLE;
    ASSERT("legacy Elderwood stages load without resetting wounds, exploration or progress", save_game(g, ROUTE_SLOT) && legacy_version(242) && load_game(loaded, ROUTE_SLOT) && loaded->player.hp == 31 && loaded->enemies[0].hp == 7 && loaded->elderwood_cache[0].enemies[0].hp == 7 && loaded->elderwood_entry_town == LOCATION_TOWN2);
    ASSERT("both live and cached Elderwood transitions extend onto the boundary", loaded->map.tiles[47][32] == TILE_ELDERWOOD_ENTRANCE && loaded->map.tiles[0][32] == TILE_ELDERWOOD_EXIT && loaded->elderwood_cache[0].tiles[47][32] == TILE_ELDERWOOD_ENTRANCE && loaded->elderwood_cache[0].tiles[0][32] == TILE_ELDERWOOD_EXIT);
    loaded->player.x = 32;
    loaded->player.y = ELDERWOOD_H - 1;
    int potion = loaded->inventory_count;
    loaded->inventory[loaded->inventory_count++] = item_make_health_potion();
    action_resolve_player(loaded, (Action){ACTION_DROP_ITEM, potion, 0});
    ASSERT("dropped loot preserves Elderwood's boundary marker", loaded->floor_item_count == 1 && loaded->map.tiles[47][32] == TILE_ELDERWOOD_ENTRANCE);
    ASSERT("current saves retain a boundary item's correct transition underlay", save_game(loaded, ROUTE_SLOT) && load_game(g, ROUTE_SLOT) && g->floor_items[0].underlying_tile == TILE_ELDERWOOD_ENTRANCE);
    action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("picking up saved boundary loot leaves a working retreat", g->map.tiles[47][32] == TILE_ELDERWOOD_ENTRANCE);
    game_init(g);
    approach_enter_region(g, 3);
    g->gold = 321;
    ASSERT("legacy coast approach saves move to the Elderwood route without changing gold", save_game(g, ROUTE_SLOT) && legacy_version(242) && load_game(loaded, ROUTE_SLOT) && loaded->approach_route == APPROACH_COAST_ROAD && loaded->gold == 321);
    int coast_retreat = route_path(loaded, loaded->map.stairs_up_x, loaded->map.stairs_up_y);

    ASSERT("legacy coastal expeditions reach Elderwood's town side even if Elderthorn is alive", coast_retreat && loaded->approach_route == APPROACH_ELDERWOOD_ROAD && !loaded->elderwood_defeated);
    ASSERT("legacy coastal expeditions retain a usable retreat to Stillbury", route_path(loaded, loaded->map.stairs_up_x, loaded->map.stairs_up_y) && loaded->location == LOCATION_TOWN_ROAD);
    game_init(g);
    approach_enter_region(g, 5);
    g->player.x = 32;
    g->player.y = 10;
    ASSERT("legacy swamp-road saves remain on the reachable Stillbury side of the living demon", save_game(g, ROUTE_SLOT) && legacy_version(242) && load_game(loaded, ROUTE_SLOT) && loaded->approach_route == APPROACH_BLACKWATER_ROAD && loaded->player.y > NORTH_SWAMP_BARRIER_Y && route_path(loaded, NORTH_SWAMP_X, NORTH_SWAMP_Y) && loaded->location == LOCATION_SWAMP);
    game_init(g);
    game_enter_swamp(g);
    int room_x;
    int room_y;
    map_room_center(&g->map.rooms[0], &room_x, &room_y);
    g->map.tiles[g->map.stairs_up_y][g->map.stairs_up_x] = TILE_SWAMP_WALL;
    g->map.stairs_up_x = 0;
    g->map.stairs_up_y = room_y;
    g->player.x = 0;
    g->player.y = room_y;
    g->floor_item_count = 2;
    for (int i = 0; i < 2; i++) {
        g->floor_items[i] = (FloorItem){.active = 1, .x = 0, .y = room_y,
            .underlying_tile = TILE_SWAMP_ENTRANCE, .item = item_make_health_potion()};
    }
    g->floor_items[1].item = item_make_greatsword();
    g->floor_items[1].item.sharpened = 1;
    g->floor_items[1].item.attack_bonus += 3;
    g->map.tiles[room_y][0] = TILE_ITEM;
    ASSERT("legacy stacked swamp-boundary loot moves onto a reachable bank with a real ground underlay", save_game(g, ROUTE_SLOT) && legacy_version(242) && load_game(loaded, ROUTE_SLOT) && loaded->player.x == 1 && loaded->floor_items[0].x == 1 && loaded->floor_items[1].x == 1 && loaded->floor_items[0].underlying_tile == TILE_SWAMP_FLOOR && loaded->floor_items[1].underlying_tile == TILE_SWAMP_FLOOR);
    int carried = loaded->inventory_count;
    action_resolve_player(loaded, (Action){ACTION_PICK_UP, 0, 0});
    action_resolve_player(loaded, (Action){ACTION_PICK_UP, 0, 0});

    ASSERT("both migrated drops can be recovered without leaving an item or transition ghost", loaded->inventory_count == carried + 2 && loaded->inventory[carried + 1].sharpened && loaded->inventory[carried + 1].attack_bonus == g->floor_items[1].item.attack_bonus && loaded->map.tiles[room_y][1] == TILE_SWAMP_FLOOR);
    game_init(g);
    g->elderwood_defeated = 1;
    approach_enter_region(g, APPROACH_ELDERWOOD_ROAD);
    g->player.x = 48;
    g->player.y = 14;
    int trail_potion = g->inventory_count;
    g->inventory[g->inventory_count++] = item_make_health_potion();
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, trail_potion, 0});
    ASSERT("the coastal exit trail saves its exact player and loot positions", save_game(g, ROUTE_SLOT) && load_game(loaded, ROUTE_SLOT) && loaded->player.x == 48 && loaded->player.y == 14 && loaded->floor_items[0].x == 48 && loaded->floor_items[0].y == 14 && loaded->floor_items[0].underlying_tile == TILE_TOWN_PATH);
    game_init(g);
    g->defeated_bosses |= 1u << LOCATION_SWAMP;
    approach_enter_region(g, APPROACH_BLACKWATER_ROAD);
    g->player.x = NORTH_SWAMP_X + 3;
    g->player.y = NORTH_SWAMP_Y - 6;
    ASSERT("the Catacombs-side exit trail survives an exact-position save/load", save_game(g, ROUTE_SLOT) && load_game(loaded, ROUTE_SLOT) && loaded->player.x == NORTH_SWAMP_X + 3 && loaded->player.y == NORTH_SWAMP_Y - 6 && map_is_walkable(&loaded->map, loaded->player.x + 1, loaded->player.y));
    game_init(g);
    g->royal_reported_actions = 1;
    game_enter_town_gate(g, LOCATION_TOWN2, TOWN_W - 1, 12);
    ASSERT("the new east gate preserves Stillbury's wanted checkpoint", route_path(g, 35, 12) && eastern_move(g, 34, 12) && g->stillbury_prompt && g->dialogue_x == 33);
    ASSERT("the eastern checkpoint survives save/load", save_game(g, ROUTE_SLOT) && load_game(loaded, ROUTE_SLOT) && loaded->stillbury_prompt && loaded->dialogue_x == 33);
    eastern_handle_key(loaded, SDL_SCANCODE_ESCAPE, 0);
    ASSERT("withdrawing from the east checkpoint leaves its gate reachable", !loaded->stillbury_prompt && loaded->player.x == TOWN_W - 3 && route_path(loaded, TOWN_W - 1, 12) && loaded->location == LOCATION_TOWN_ROAD);
    ASSERT("geography fixtures remove only their own save", remove("saves/savegame_99274.json") == 0);
    free(g);
    free(loaded);
}
