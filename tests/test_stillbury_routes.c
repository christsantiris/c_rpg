#include "test_utils.h"
#include "game/actions.h"
#include "game/approach.h"
#include "game/boss_gates.h"
#include "game/coast_exits.h"
#include "game/eastern_arc.h"
#include "game/elderwood.h"
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
            int transition = (map_is_travel_tile(tile) && tile != TILE_ROAD_GATE_OPEN) || tile == TILE_SWAMP_ENTRANCE || tile == TILE_SWAMP_EXIT || tile == TILE_COAST_ENTRANCE || tile == TILE_COAST_EXIT;
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

static int legacy_version(void) {
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 242);
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
    }
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
    ASSERT("legacy Elderwood stages load without resetting wounds, exploration or progress", save_game(g, ROUTE_SLOT) && legacy_version() && load_game(loaded, ROUTE_SLOT) && loaded->player.hp == 31 && loaded->enemies[0].hp == 7 && loaded->elderwood_cache[0].enemies[0].hp == 7 && loaded->elderwood_entry_town == LOCATION_TOWN2);
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
    ASSERT("legacy coast approach saves move to the Elderwood route without changing gold", save_game(g, ROUTE_SLOT) && legacy_version() && load_game(loaded, ROUTE_SLOT) && loaded->approach_route == APPROACH_COAST_ROAD && loaded->gold == 321);
    int coast_retreat = route_path(loaded, loaded->map.stairs_up_x, loaded->map.stairs_up_y);

    ASSERT("legacy coastal expeditions reach Elderwood's town side even if Elderthorn is alive", coast_retreat && loaded->approach_route == APPROACH_ELDERWOOD_ROAD && !loaded->elderwood_defeated);
    ASSERT("legacy coastal expeditions retain a usable retreat to Stillbury", route_path(loaded, loaded->map.stairs_up_x, loaded->map.stairs_up_y) && loaded->location == LOCATION_TOWN_ROAD);
    game_init(g);
    approach_enter_region(g, 5);
    g->player.x = 32;
    g->player.y = 10;
    ASSERT("legacy swamp-road saves remain on the reachable Stillbury side of the living demon", save_game(g, ROUTE_SLOT) && legacy_version() && load_game(loaded, ROUTE_SLOT) && loaded->approach_route == APPROACH_BLACKWATER_ROAD && loaded->player.y > NORTH_SWAMP_BARRIER_Y && route_path(loaded, NORTH_SWAMP_X, NORTH_SWAMP_Y) && loaded->location == LOCATION_SWAMP);
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
    ASSERT("legacy stacked swamp-boundary loot moves onto a reachable bank with a real ground underlay", save_game(g, ROUTE_SLOT) && legacy_version() && load_game(loaded, ROUTE_SLOT) && loaded->player.x == 1 && loaded->floor_items[0].x == 1 && loaded->floor_items[1].x == 1 && loaded->floor_items[0].underlying_tile == TILE_SWAMP_FLOOR && loaded->floor_items[1].underlying_tile == TILE_SWAMP_FLOOR);
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
