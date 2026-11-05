#include "test_utils.h"
#include "game/actions.h"
#include "game/coast_exits.h"
#include "game/coastal_travel.h"
#include "game/eastern_arc.h"
#include "game/mangrove.h"
#include "game/town_life.h"
#include "game/town_road.h"
#include "screens/harbor.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define ROUTE_CELLS (MAP_W * MAP_H)
#define COAST_SLOT 99276

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

static int find_prop(const GameState *g, TileType tile, int *x, int *y) {
    for (*y = 0; *y < MAP_H; (*y)++) {
        for (*x = 0; *x < MAP_W; (*x)++) {
            if (g->map.tiles[*y][*x] == tile) {
                return 1;
            }
        }
    }
    return 0;
}

static void clear_enemies(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
}

static int cross_coast(GameState *g, int forward) {
    int x;
    int y;
    return coast_exit_point(&g->map, forward, &x, &y) && route_path(g, x, y);
}

static void visit_rowan(GameState *g) {
    eastern_river_enter(g, 1);
    ASSERT("Rowan is reachable from Stillbury's shore without crossing the river", route_path(g, STILLBURY_ROWAN_X - 1, STILLBURY_ROWAN_Y));
    game_talk_to_rowan(g);
}

static int legacy_save(void) {
    FILE *file = fopen("saves/savegame_99276.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = malloc(size + 1);
    size_t count = fread(data, 1, size, file);
    fclose(file);
    data[count] = 0;
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 243);
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99276.json", "wb");
    int ok = file && data && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static void coast_journey(GameState *g, unsigned seed) {
    srand(seed);
    game_init(g);
    game_enter_coast(g);
    for (int stage = 1; stage <= COAST_DEPTH; stage++) {
        clear_enemies(g);
        int x;
        int y;
        int beacon = find_prop(g, TILE_COAST_BEACON_UNLIT, &x, &y);
        ASSERT("only Coast stages 2, 3 and 4 contain beacons, even before accepting", beacon == (stage >= 2 && stage <= 4));
        for (int tide = 0; tide < 2; tide++) {
            int control_x;
            int control_y;
            ASSERT("the tide control has a continuous route from the stage arrival", find_prop(g, TILE_COAST_TIDE_CONTROL, &control_x, &control_y) && route_path(g, control_x, control_y));
            ASSERT("both tide states preserve a usable retreat transition", cross_coast(g, 0) && (stage == 1 ? g->location == LOCATION_APPROACH : g->level == stage - 1));
            if (stage == 1) {
                ASSERT("the coastal doorway still permits reentry", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_COAST);
            } else {
                clear_enemies(g);
                ASSERT("cached coastal exits permit real onward travel", cross_coast(g, 1) && g->level == stage);
            }
            clear_enemies(g);
            ASSERT("the cached tide control remains reachable", route_path(g, control_x, control_y));
            action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
            if (beacon && g->map.tiles[y][x] == TILE_COAST_BEACON_UNLIT) {
                int high = 0;
                for (int row = 0; row < MAP_H; row++) {
                    for (int col = 0; col < MAP_W; col++) {
                        high |= g->map.tiles[row][col] == TILE_COAST_DEEP_WATER;
                    }
                }
                if (!high) {
                    ASSERT("a drained beacon is reachable without an assigned quest", route_path(g, x, y));
                    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
                    ASSERT("lighting a discovered beacon records its completion", g->mara_beacons_lit & (1 << (stage - 2)));
                }
            }
        }
        clear_enemies(g);
        if (stage == COAST_DEPTH) {
            g->defeated_bosses |= 1u << LOCATION_COAST;
        }
        ASSERT("postprocessing and beacon placement preserve each onward transition", cross_coast(g, 1) && (stage < COAST_DEPTH ? g->location == LOCATION_COAST && g->level == stage + 1 : g->location == LOCATION_APPROACH));
    }
    ASSERT("all discovered beacons complete the quest and free the active slot", g->mara_beacons_lit == 7 && g->mara_quest_state == 2 && game_active_quest_count(g) == 0);
}

void test_coastal_quest(void) {
    printf("Stillbury coastal quest and island passage:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("coastal fixtures allocate and the isolated save slot is unused", g && loaded && !save_exists(COAST_SLOT));
    if (!g || !loaded || save_exists(COAST_SLOT)) {
        free(g);
        free(loaded);
        return;
    }
    game_init(g);
    game_enter_tavern(g);
    g->player.x = MARA_TAVERN_X;
    g->player.y = MARA_TAVERN_Y + 1;
    game_talk_to_mara(g);
    ASSERT("Mara introduces Rowan and the island without assigning a quest", g->map.tiles[MARA_TAVERN_Y][MARA_TAVERN_X] == TILE_NPC_MARA && strstr(g->dialogue_text, "Stillbury") && strstr(g->dialogue_text, "Ruined Isle") && !game_quest_offer_active(g) && !game_active_quest_count(g));
    g->mara_quest_state = 2;
    int gold = g->gold;
    game_talk_to_mara(g);
    ASSERT("Mara cannot claim Rowan's reward", g->mara_quest_state == 2 && g->gold == gold);
    g->mara_quest_state = 0;
    visit_rowan(g);
    ASSERT("Rowan offers the coastal beacons from Stillbury's dock", game_quest_offer_active(g) && strstr(g->dialogue_text, "Elderwood") && !game_can_sail_to_island(g));
    g->inventory[g->inventory_count++] = item_make_treasure_map();
    ASSERT("a legacy chart cannot bypass the living Queen", !game_can_sail_to_island(g));
    ASSERT("an undecided Rowan offer survives save/load", save_game(g, COAST_SLOT) && load_game(loaded, COAST_SLOT) && game_quest_offer_active(loaded));
    game_handle_quest_offer_key(loaded, SDL_SCANCODE_N, 0);
    ASSERT("declining leaves Rowan's quest available", !loaded->mara_quest_state && !loaded->dialogue_active);
    g->elowen_quest_state = 1;
    g->alder_quest_state = 1;
    g->dain_quest_state = 1;
    ASSERT("Rowan's offer respects the three-quest limit", game_quest_offer_at_limit(g) && game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0) && !g->mara_quest_state);
    g->alder_quest_state = 2;
    g->mara_beacons_lit = 1;
    g->coast_cache[1].valid = 1;
    g->coast_cache[1].enemies[0].hp = 7;
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("acceptance preserves preexisting beacon progress and cached wounds", g->mara_quest_state == 1 && g->mara_beacons_lit == 1 && g->coast_cache[1].valid && g->coast_cache[1].enemies[0].hp == 7);
    const unsigned seeds[] = {19, 107, 1301};
    for (int i = 0; i < 3; i++) {
        coast_journey(g, seeds[i]);
    }
    gold = g->gold;
    int score = g->score;
    ASSERT("completed beacons and the Queen's defeat survive saving before the reward", save_game(g, COAST_SLOT) && load_game(loaded, COAST_SLOT) && loaded->mara_quest_state == 2 && loaded->mara_beacons_lit == 7 && (loaded->defeated_bosses & (1u << LOCATION_COAST)));
    visit_rowan(g);
    game_talk_to_rowan(g);
    game_talk_to_rowan(g);
    ASSERT("Rowan pays the existing beacon reward exactly once", g->mara_quest_state == 3 && g->gold == gold + 80 && g->score == score + 600);
    ASSERT("the collected reward remains collected after loading", save_game(g, COAST_SLOT) && load_game(loaded, COAST_SLOT) && loaded->mara_quest_state == 3 && loaded->gold == g->gold);
    game_talk_to_rowan(loaded);
    ASSERT("speaking to Rowan after loading cannot duplicate payment", loaded->gold == g->gold && loaded->score == g->score);
    int inventory_count = g->inventory_count;
    for (int i = inventory_count; i < MAX_INVENTORY; i++) {
        g->inventory[i] = item_make_health_potion();
    }
    g->inventory_count = MAX_INVENTORY;
    ASSERT("the Queen's defeat opens sailing without beacons, a chart or inventory space", game_can_sail_to_island(g));
    g->inventory_count = inventory_count;
    g->mara_beacons_lit = 0;
    ASSERT("beacons are not an extra island gate", game_can_sail_to_island(g) && !coastal_trade_restored(g));
    g->mara_beacons_lit = 7;
    g->royal_reported_actions = 3;
    game_enter_town2(g);
    town_life_enter(g, 9, 20);
    town_life_talk(g, 1);
    ASSERT("coastal trade warms civilian reactions while preserving the Crown's record", coastal_trade_restored(g) && strstr(g->dialogue_text, "welcome") && strstr(g->dialogue_text, "checkpoint") && g->royal_reported_actions == 3);
    visit_rowan(g);
    ASSERT("the ship entrance can be reached by walking from Rowan", route_path(g, STILLBURY_DOCK_ENTRY_X, STILLBURY_DOCK_ENTRY_Y) && coastal_at_docks(g));
    g->inventory[g->inventory_count++] = item_make_health_potion();
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, g->inventory_count - 1, 0});
    int loot = g->floor_item_count;
    HarborScreen harbor;
    harbor_init(&harbor);
    ASSERT("boarding is offered from the Stillbury docks", harbor_activate(&harbor, game_can_sail_to_island(g), 0) == HARBOR_BOARD);
    game_enter_island(g);
    ASSERT("sailing preserves the river landing's loot", g->location == LOCATION_ISLAND && g->river_boat.loot_count == loot);
    game_leave_island(g);
    ASSERT("return voyages land beside Stillbury, preserve loot, and face away from the ship", g->location == LOCATION_RIVER_CROSSING && g->player.last_dx == -1 && g->floor_item_count == loot && route_path(g, RIVER_NORTH_X, 0) && g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN2);
    game_init(g);
    game_enter_tavern(g);
    g->map.tiles[MARA_TAVERN_Y][MARA_TAVERN_X] = TILE_TAVERN_FLOOR;
    g->mara_quest_state = 1;
    g->mara_beacons_lit = 5;
    g->player.hp = 31;
    ASSERT("old saves restore Mara without losing player health or beacon progress", save_game(g, COAST_SLOT) && legacy_save() && load_game(loaded, COAST_SLOT) && loaded->map.tiles[MARA_TAVERN_Y][MARA_TAVERN_X] == TILE_NPC_MARA && loaded->mara_quest_state == 1 && loaded->mara_beacons_lit == 5 && loaded->player.hp == 31);
    visit_rowan(loaded);
    loaded->player.x = STILLBURY_DOCK_X;
    loaded->player.y = STILLBURY_DOCK_Y;
    loaded->map.tiles[loaded->player.y][loaded->player.x] = TILE_TOWN_FLOOR;
    ASSERT("old shore saves relocate occupants safely around the new ship", save_game(loaded, COAST_SLOT) && legacy_save() && load_game(g, COAST_SLOT) && map_is_walkable(&g->map, g->player.x, g->player.y) && g->mara_beacons_lit == 5);
    ASSERT("current saves retain pending completion and never duplicate collected rewards", save_game(g, COAST_SLOT) && load_game(loaded, COAST_SLOT) && loaded->mara_quest_state == g->mara_quest_state && loaded->mara_beacons_lit == g->mara_beacons_lit && loaded->gold == g->gold);
    game_init(g);
    game_enter_coast(g);
    g->enemies[0].hp = 7;
    game_return_to_approach(g);
    g->map.tiles[COAST_ROWAN_Y][COAST_ROWAN_X] = TILE_NPC_ROWAN;
    g->map.tiles[COAST_DOCK_Y][COAST_DOCK_X] = TILE_WATCHTOWER;
    g->player.x = COAST_DOCK_ENTRY_X;
    g->player.y = COAST_DOCK_ENTRY_Y;
    g->map.tiles[g->player.y][g->player.x] = TILE_TOWN_PATH;
    g->inventory[g->inventory_count++] = item_make_health_potion();
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, g->inventory_count - 1, 0});
    g->mara_quest_state = 1;
    g->mara_beacons_lit = 5;
    ASSERT("old coastal docks disappear without losing quest progress, cached wounds or dropped loot", save_game(g, COAST_SLOT) && legacy_save() && load_game(loaded, COAST_SLOT) && loaded->map.tiles[COAST_ROWAN_Y][COAST_ROWAN_X] != TILE_NPC_ROWAN && loaded->coast_cache[0].enemies[0].hp == 7 && loaded->mara_beacons_lit == 5 && loaded->floor_item_count == 1 && loaded->floor_items[0].active && map_is_walkable(&loaded->map, loaded->floor_items[0].x, loaded->floor_items[0].y));
    remove("saves/savegame_99276.json");
    free(g);
    free(loaded);
}

