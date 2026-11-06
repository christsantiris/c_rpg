#include "test_utils.h"
#include "game/actions.h"
#include "game/approach.h"
#include "game/elderwood.h"
#include "game/elderwood_quest.h"
#include "game/heartseed.h"
#include "game/town_road.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define ROUTE_CELLS (MAP_W * MAP_H)
#define ELDER_QUEST_SLOT 99279

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

static int visit_tamsin(GameState *g) {
    game_enter_town2(g);
    return route_path(g, RANGER_TAMSIN_X - 1, RANGER_TAMSIN_Y) && elderwood_quest_talk(g);
}

static int legacy_save(void) {
    FILE *file = fopen("saves/savegame_99279.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = malloc(size + 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    size_t count = fread(data, 1, size, file);
    fclose(file);
    data[count] = 0;
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 245);
    cJSON_DeleteItemFromObject(root, "elderwood_quest_state");
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99279.json", "wb");
    int ok = file && data && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static int has_retired_props(const Map *map) {
    for (int y = 0; y < ELDERWOOD_H; y++) {
        for (int x = 0; x < ELDERWOOD_W; x++) {
            TileType tile = map->tiles[y][x];
            if (tile == TILE_HEARTSEED_ORDERS || tile == TILE_HEARTSEED_STAKE || tile == TILE_HEARTSEED_CUT_STAKE || tile == TILE_HEARTSEED_ALTAR) {
                return 1;
            }
        }
    }
    return 0;
}

static void offer_and_limit(GameState *g, GameState *loaded) {
    game_init(g);
    ASSERT("Ranger Tamsin is reachable beside Stillbury's east road and offers the boss hunt", visit_tamsin(g) && game_quest_offer_active(g) && strstr(g->dialogue_speaker, "Ranger Tamsin") && strstr(g->dialogue_text, "Elderthorn") && !g->elderwood_quest_state);
    ASSERT("a pending Tamsin offer survives save/load", save_game(g, ELDER_QUEST_SLOT) && load_game(loaded, ELDER_QUEST_SLOT) && game_quest_offer_active(loaded) && !loaded->elderwood_quest_state);
    game_handle_quest_offer_key(loaded, SDL_SCANCODE_Y, 1);
    ASSERT("holding Y does not accept the boss hunt", !loaded->elderwood_quest_state);
    game_handle_quest_offer_key(loaded, SDL_SCANCODE_N, 0);
    ASSERT("declining leaves Tamsin's quest available", !loaded->elderwood_quest_state && !loaded->dialogue_active);
    elderwood_quest_talk(loaded);
    loaded->elowen_quest_state = 1;
    loaded->alder_quest_state = 1;
    loaded->dain_quest_state = 1;
    game_handle_quest_offer_key(loaded, SDL_SCANCODE_Y, 0);
    ASSERT("the boss hunt respects the three-active-quest limit", game_quest_offer_at_limit(loaded) && !loaded->elderwood_quest_state && game_active_quest_count(loaded) == 3);
    loaded->elowen_quest_state = 0;
    loaded->alder_quest_state = 0;
    loaded->dain_quest_state = 0;
    game_handle_quest_offer_key(loaded, SDL_SCANCODE_Y, 0);
    QuestJournalEntry entry;
    ASSERT("acceptance adds exactly one boss objective to the active journal", loaded->elderwood_quest_state == 1 && game_active_quest_count(loaded) == 1 && quest_journal_get_entry(loaded, QUEST_TAB_ACTIVE, 0, &entry) && strcmp(entry.title, "Clear Elderwood") == 0 && entry.objective_count == 1 && !entry.objective_complete[0] && entry.stages[0] == ELDERWOOD_DEPTH && entry.reward_gold == ELDERWOOD_QUEST_GOLD);
    int gold = loaded->gold;
    int score = loaded->score;
    elderwood_quest_talk(loaded);
    ASSERT("Tamsin never pays before Elderthorn is defeated", loaded->elderwood_quest_state == 1 && loaded->gold == gold && loaded->score == score);
}

static void expedition(GameState *g, GameState *loaded, unsigned seed, int accept) {
    game_init(g);
    srand(seed);
    if (accept) {
        ASSERT("a fresh expedition accepts Tamsin's hunt", visit_tamsin(g));
        game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    }
    town_road_enter(g, LOCATION_TOWN2, TOWN_SIDE_EAST, 1);
    int x;
    int y;
    town_road_exit(TOWN_SIDE_EAST, &x, &y);
    ASSERT("Stillbury's eastern road still reaches Elderwood", route_path(g, x, y) && g->approach_route == APPROACH_ELDERWOOD_ROAD);
    ASSERT("the coastal road remains blocked until the boss falls", !route_path(g, APPROACH_W - 1, APPROACH_H / 2));
    ASSERT("the visible forest doorway remains usable without accepting a quest", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_ELDERWOOD && g->level == 1);
    for (int stage = 1; stage <= ELDERWOOD_DEPTH; stage++) {
        ASSERT("fresh Elderwood stages contain no Heartseed props", !has_retired_props(&g->map));
        clear_enemies(g);
        ASSERT("every stage retains its real retreat transition after removing the side mission", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && (stage == 1 ? g->approach_route == APPROACH_ELDERWOOD_ROAD : g->location == LOCATION_ELDERWOOD && g->level == stage - 1));
        clear_enemies(g);
        ASSERT("cached stages can be entered again through their actual doorway or boundary", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_ELDERWOOD && g->level == stage);
        ASSERT("cached Elderwood stages cannot restore the retired quest", !has_retired_props(&g->map));
        clear_enemies(g);
        if (stage < ELDERWOOD_DEPTH) {
            ASSERT("every northbound exit reaches the next stage with a northward inward path", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_ELDERWOOD && g->level == stage + 1 && g->player.last_dy == -1 && map_is_walkable(&g->map, g->player.x, g->player.y - 1));
        }
    }
    Enemy *boss = NULL;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == ENEMY_ELDERTHORN) {
            boss = &g->enemies[i];
            boss->active = 1;
            boss->hp = 1;
            break;
        }
    }
    ASSERT("the final stage retains Elderthorn", boss != NULL);
    if (!boss) {
        return;
    }
    g->player.attack = 999;
    g->player.x = boss->x - 1;
    g->player.y = boss->y;
    action_resolve_player(g, (Action){ACTION_MOVE, boss->x, boss->y});
    QuestJournalEntry entry;
    ASSERT("a real boss kill completes the single objective with or without prior acceptance", g->elderwood_defeated && g->elderwood_quest_state == 2 && !game_active_quest_count(g) && quest_journal_get_entry(g, QUEST_TAB_COMPLETED, 0, &entry) && strcmp(entry.title, "Clear Elderwood") == 0 && entry.objective_count == 1 && entry.objective_complete[0]);
    ASSERT("completion before claiming Tamsin's reward survives saving", save_game(g, ELDER_QUEST_SLOT) && load_game(loaded, ELDER_QUEST_SLOT) && loaded->elderwood_quest_state == 2 && loaded->elderwood_defeated);
    ASSERT("the boss victory retains the final northward transition", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->approach_route == APPROACH_ELDERWOOD_ROAD && g->player.last_dy == -1);
    ASSERT("clearing Elderwood opens the real road to Sunken Coast", route_path(g, APPROACH_W - 1, APPROACH_H / 2) && g->approach_route == APPROACH_COAST_ROAD);
    ASSERT("the coastal branch permits a real retreat to Elderwood", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && g->approach_route == APPROACH_ELDERWOOD_ROAD);
    ASSERT("the cleared forest road leads back to Stillbury", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN2);
    town_road_entrance(TOWN_SIDE_EAST, &x, &y);
    ASSERT("the town's eastern entrance and Tamsin remain reachable after victory", route_path(g, x, y) && g->location == LOCATION_TOWN2 && route_path(g, RANGER_TAMSIN_X - 1, RANGER_TAMSIN_Y));
    int gold = g->gold;
    int score = g->score;
    elderwood_quest_talk(g);
    elderwood_quest_talk(g);
    ASSERT("Tamsin pays her separate reward exactly once", g->elderwood_quest_state == 3 && g->gold == gold + ELDERWOOD_QUEST_GOLD && g->score == score + ELDERWOOD_QUEST_SCORE);
    ASSERT("collected rewards remain collected after loading", save_game(g, ELDER_QUEST_SLOT) && load_game(loaded, ELDER_QUEST_SLOT) && loaded->elderwood_quest_state == 3 && loaded->gold == g->gold && loaded->score == g->score);
    elderwood_quest_talk(loaded);
    ASSERT("talking after loading cannot duplicate Tamsin's payment", loaded->gold == g->gold && loaded->score == g->score);
}

static void migration(GameState *g, GameState *loaded) {
    game_init(g);
    game_enter_town2(g);
    g->player.x = RANGER_TAMSIN_X;
    g->player.y = RANGER_TAMSIN_Y;
    g->map.tiles[g->player.y][g->player.x] = TILE_ITEM;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = g->player.x, .y = g->player.y, .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_ring_of_vigor()};
    g->player.hp = 37;
    g->gold = 413;
    map_mark_explored(&g->map, 34, 14);
    ASSERT("old town saves gain Tamsin without trapping the player or losing dropped equipment", save_game(g, ELDER_QUEST_SLOT) && legacy_save() && load_game(loaded, ELDER_QUEST_SLOT) && loaded->map.tiles[RANGER_TAMSIN_Y][RANGER_TAMSIN_X] == TILE_NPC_TAMSIN && map_is_walkable(&loaded->map, loaded->player.x, loaded->player.y) && loaded->floor_items[0].active && loaded->floor_items[0].x != RANGER_TAMSIN_X && loaded->gold == 413 && loaded->player.hp == 37 && map_is_explored(&loaded->map, 34, 14));
    game_init(g);
    elderwood_enter(g);
    for (int level = 1; level < 4; level++) {
        game_descend(g);
    }
    g->heartseed_quest_state = 1;
    g->heartseed_progress = HEARTSEED_ORDERS;
    g->heartseed_sites = 3;
    g->heartseed_guardian = HEARTSEED_BOUND;
    g->elderwood_cache[1].tiles[9][9] = TILE_HEARTSEED_ORDERS;
    g->map.tiles[30][49] = TILE_HEARTSEED_STAKE;
    g->map.tiles[33][53] = TILE_ITEM;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 53, .y = 33, .underlying_tile = TILE_HEARTSEED_ALTAR, .item = item_make_ring_of_vigor()};
    g->enemies[0].type = ENEMY_TREEFOLK;
    g->enemies[0].hp = 7;
    snprintf(g->enemies[0].name, sizeof(g->enemies[0].name), "Bound Treefolk");
    g->player.hp = 31;
    g->gold = 327;
    map_mark_explored(&g->map, 49, 30);
    ASSERT("old active and cached forest stages retire Heartseed props while preserving wounds, loot and exploration", save_game(g, ELDER_QUEST_SLOT) && legacy_save() && load_game(loaded, ELDER_QUEST_SLOT) && loaded->location == LOCATION_ELDERWOOD && !has_retired_props(&loaded->map) && loaded->elderwood_cache[1].tiles[9][9] == TILE_ELDERWOOD_FLOOR && loaded->enemies[0].hp == 7 && strcmp(loaded->enemies[0].name, "Treefolk") == 0 && loaded->floor_items[0].active && loaded->floor_items[0].underlying_tile == TILE_ELDERWOOD_FLOOR && loaded->player.hp == 31 && loaded->gold == 327 && map_is_explored(&loaded->map, 49, 30));
    ASSERT("retired quest progress remains saved without creating an active Heartseed quest", loaded->heartseed_quest_state == 1 && loaded->heartseed_progress == HEARTSEED_ORDERS && !game_active_quest_count(loaded) && !quest_journal_count(loaded, QUEST_TAB_ACTIVE));
    loaded->elderwood_defeated = 1;
    int gold = loaded->gold;
    ASSERT("existing Elderthorn victories gain an unclaimed Tamsin reward without repeating boss loot", save_game(loaded, ELDER_QUEST_SLOT) && legacy_save() && load_game(g, ELDER_QUEST_SLOT) && g->elderwood_quest_state == 2 && g->gold == gold);
}

void test_elderwood_quest(void) {
    printf("Elderwood boss quest and retired Heartseed:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("Elderwood quest fixtures allocate and the isolated save slot is unused", g && loaded && !save_exists(ELDER_QUEST_SLOT));
    if (!g || !loaded || save_exists(ELDER_QUEST_SLOT)) {
        free(g);
        free(loaded);
        return;
    }
    offer_and_limit(g, loaded);
    const unsigned seeds[] = {19, 107, 1301};
    for (int i = 0; i < 3; i++) {
        expedition(g, loaded, seeds[i], i != 1);
    }
    migration(g, loaded);
    remove("saves/savegame_99279.json");
    free(g);
    free(loaded);
}
