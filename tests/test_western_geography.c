#include "approach_checks.h"
#include "game/boss_gates.h"
#include "game/eastern_arc.h"
#include "game/giants_crown.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define WESTERN_SLOT 99263
#define WESTERN_CELLS (MAP_W * MAP_H)

static int western_path(GameState *g, int tx, int ty, int travel) {
    int previous[WESTERN_CELLS];
    int queue[WESTERN_CELLS];
    for (int i = 0; i < WESTERN_CELLS; i++) {
        previous[i] = -1;
    }
    int start = g->player.y * MAP_W + g->player.x;
    int target = ty * MAP_W + tx;
    int head = 0;
    int tail = 1;
    queue[0] = start;
    previous[start] = start;
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
            int transition = map_is_travel_tile(tile) && tile != TILE_ROAD_GATE_OPEN && tile != TILE_ROAD_GATE_CLOSED;
            if (previous[next] >= 0 || !map_is_walkable(&g->map, x, y) || (transition && next != target)) {
                continue;
            }
            int occupied = 0;
            for (int i = 0; i < g->enemy_count; i++) {
                occupied |= g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y;
            }
            if (occupied && next != target) {
                continue;
            }
            previous[next] = cell;
            queue[tail++] = next;
        }
    }
    if (previous[target] < 0 || !travel) {
        return previous[target] >= 0;
    }
    int count = 0;
    for (int cell = target; cell != start; cell = previous[cell]) {
        queue[count++] = cell;
    }
    Location area = g->location;
    int level = g->level;
    int route = g->approach_route;
    for (int i = count - 1; i >= 0; i--) {
        int x = queue[i] % MAP_W;
        int y = queue[i] / MAP_W;
        action_resolve_player(g, (Action){ACTION_MOVE, x, y});
        if (g->location != area || g->level != level || g->approach_route != route) {
            return i == 0;
        }
        if (g->player.x != x || g->player.y != y) {
            return 0;
        }
    }
    return 1;
}

static void western_cross_stage(GameState *g, int forward) {
    int x = forward ? g->map.stairs_down_x : g->map.stairs_up_x;
    int y = forward ? g->map.stairs_down_y : g->map.stairs_up_y;
    if (g->location == LOCATION_DESERT) {
        x = forward ? 0 : DESERT_MAP_W - 1;
    }
    ASSERT("every western stage has a continuous route to its working boundary transition", western_path(g, x, y, 1));
}

static int western_legacy_version(void) {
    FILE *file = fopen("saves/savegame_99263.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = malloc((size_t)size + 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    size_t count = fread(data, 1, (size_t)size, file);
    data[count] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 238);
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99263.json", "wb");
    int ok = data && file && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static void clear_patrols(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type != ENEMY_LAST_JARL) {
            g->enemies[i].active = 0;
        }
    }
}

static void defeat_jarl(GameState *g) {
    Enemy *jarl = NULL;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == ENEMY_LAST_JARL) {
            jarl = &g->enemies[i];
        }
    }
    ASSERT("the northern court contains the living Jarl", jarl && jarl->active);
    if (!jarl) {
        return;
    }
    clear_patrols(g);
    ASSERT("the Jarl can be reached through the generated court", western_path(g, jarl->x, jarl->y + 1, 1));
    for (int tries = 0; jarl->active && tries < 30; tries++) {
        action_resolve_player(g, (Action){ACTION_MOVE, jarl->x, jarl->y});
    }
    ASSERT("actual Jarl combat opens onward passage without requiring Edda's quest", !jarl->active && g->giants_crown_defeated && !g->crown_quest_state);
}

static void western_journey(GameState *g, GameState *loaded, int seed) {
    game_init(g);
    srand(7600 + seed);
    g->player.hp = g->player.max_hp = 100000;
    g->player.attack = 100000;
    g->player.defense = 1000;
    approach_enter_region(g, APPROACH_SUNSCAR_ROAD);
    ASSERT("the north turn lies beyond the Sunscar entrance when arriving from the east", APPROACH_PEAKS_BRANCH_X < g->map.stairs_down_x);
    ASSERT("the desert landmark has a working doorway", western_path(g, g->map.stairs_down_x, g->map.stairs_down_y, 1) && g->location == LOCATION_DESERT);
    for (int stage = 1; stage <= DESERT_DEPTH; stage++) {
        ASSERT("the desert journey visits each generated stage", g->location == LOCATION_DESERT && g->level == stage);
        g->enemy_count = 0;
        western_cross_stage(g, 1);
    }
    ASSERT("Sunscar's last western exit returns to its road rather than a dead end", approach_sunscar_road(g));
    ASSERT("the desert road continues west and then north into the Peaks road", western_path(g, APPROACH_PEAKS_BRANCH_X, 0, 1) && approach_peaks_road(g) && g->player.y == APPROACH_H - 2 && g->player.last_dy == -1);
    int x;
    int y;
    ASSERT("the southern approach cannot bypass the five-stage pass to reach the bridge", !approach_bridge_exit(g, &x, &y));
    ASSERT("the giant hall enters the southern foothills", western_path(g, g->map.stairs_down_x, g->map.stairs_down_y, 1) && g->location == LOCATION_GIANTS_CROWN && g->level == 1 && !g->giants_entry_from_north);
    for (int stage = 1; stage < GIANTS_DEPTH; stage++) {
        clear_patrols(g);
        western_cross_stage(g, 1);
        ASSERT("the giant stages connect northward without reversing direction", g->level == stage + 1 && g->player.y == GIANTS_H - 2 && g->player.last_dy == -1);
        clear_patrols(g);
        western_cross_stage(g, 0);
        ASSERT("each giant stage retains a real southward retreat", g->level == stage && g->player.y == 1 && g->player.last_dy == 1);
        western_cross_stage(g, 1);
    }
    clear_patrols(g);
    ASSERT("the court's northern exit remains approachable before victory", western_path(g, 32, 1, 1));
    action_resolve_player(g, (Action){ACTION_MOVE, 32, 0});
    ASSERT("the Jarl blocks onward passage from Sunscar while leaving retreat available", g->location == LOCATION_GIANTS_CROWN && g->level == GIANTS_DEPTH && g->player.y == 1);
    defeat_jarl(g);
    western_cross_stage(g, 1);
    ASSERT("the final northern exit reaches the bridge road's southern boundary", approach_bridge_road(g) && g->player.y == APPROACH_H - 2 && g->player.last_dy == -1);
    ASSERT("the bridge can be crossed to its northern bank", western_path(g, APPROACH_BRIDGE_X, 7, 1));
    int item = g->inventory_count;
    g->inventory[g->inventory_count++] = item_make_health_potion();
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, item, 0});
    int px = g->player.x;
    int py = g->player.y;
    map_mark_explored(&g->map, px, py);
    ASSERT("bridge discovery, loot, victory and position survive saving", save_game(g, WESTERN_SLOT) && load_game(loaded, WESTERN_SLOT) && loaded->giants_crown_defeated && loaded->player.x == px && loaded->player.y == py && loaded->floor_items[0].active && map_is_explored(&loaded->map, px, py));
    *g = *loaded;
    ASSERT("the bridge's eastward bank road reaches Stillbury from its western boundary", western_path(g, APPROACH_W - 1, APPROACH_BRIDGE_EXIT_Y, 1) && g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN2 && g->player.x == 1 && g->player.last_dx == 1 && !g->act4_arrived);
    town_road_entrance(TOWN_SIDE_WEST, &x, &y);
    ASSERT("walking into Stillbury begins Act 4 facing east", western_path(g, x, y, 1) && g->location == LOCATION_TOWN2 && g->act4_arrived && g->player.x == 1 && g->player.last_dx == 1);
    ASSERT("Stillbury opens Blackwater north and Elderwood east", g->map.tiles[0][20] == TILE_TOWN_EXIT && g->map.tiles[12][TOWN_W - 1] == TILE_TOWN_EXIT);
    ASSERT("Stillbury's western gate allows retreat", western_path(g, 0, 12, 1) && g->location == LOCATION_TOWN_ROAD);
    town_road_exit(TOWN_SIDE_WEST, &x, &y);
    ASSERT("the western road returns onto the north bank facing west", western_path(g, x, y, 1) && approach_bridge_road(g) && g->player.x == APPROACH_W - 2 && g->player.last_dx == -1);
    ASSERT("the bridge road enters the northern court facing south", western_path(g, APPROACH_BRIDGE_X, APPROACH_H - 1, 1) && g->location == LOCATION_GIANTS_CROWN && g->level == GIANTS_DEPTH && g->player.y == 1 && g->player.last_dy == 1 && g->giants_entry_from_north);
    for (int stage = GIANTS_DEPTH; stage > 1; stage--) {
        clear_patrols(g);
        western_cross_stage(g, 0);
        ASSERT("returning from Stillbury visits all five stages southward", g->level == stage - 1 && g->player.y == 1 && g->player.last_dy == 1);
    }
    clear_patrols(g);
    western_cross_stage(g, 0);
    ASSERT("the southern end returns to the Sunscar approach", approach_peaks_road(g));
    ASSERT("the southward road reaches the desert's north boundary", western_path(g, APPROACH_BRIDGE_X, APPROACH_H - 1, 1) && approach_sunscar_road(g) && g->player.y == 1 && g->player.last_dy == 1);
}

static void northern_journey(GameState *g, GameState *loaded, int seed) {
    game_init(g);
    srand(8100 + seed);
    g->player.hp = g->player.max_hp = 100000;
    g->player.attack = 100000;
    g->player.defense = 1000;
    approach_enter_region(g, APPROACH_PEAKS_NORTH_ROAD);
    ASSERT("Stillbury travelers can cross the bridge before the Jarl dies", western_path(g, APPROACH_BRIDGE_X, APPROACH_H - 1, 1) && g->location == LOCATION_GIANTS_CROWN && g->level == GIANTS_DEPTH && g->giants_entry_from_north && g->player.y == 1 && !g->giants_crown_defeated);
    action_resolve_player(g, (Action){ACTION_MOVE, 32, 0});
    ASSERT("the northern entrance allows retreat while the Jarl lives", approach_bridge_road(g) && g->player.y == APPROACH_H - 2 && g->player.last_dy == -1);
    ASSERT("the northern bridge retreat actually reaches Stillbury", western_path(g, APPROACH_W - 1, APPROACH_BRIDGE_EXIT_Y, 1) && g->location == LOCATION_TOWN_ROAD);
    int x;
    int y;
    town_road_exit(TOWN_SIDE_WEST, &x, &y);
    ASSERT("the Stillbury road re-enters the same crossing", western_path(g, x, y, 1) && approach_bridge_road(g));
    ASSERT("the northern pass entrance is usable again", western_path(g, APPROACH_BRIDGE_X, APPROACH_H - 1, 1) && g->level == GIANTS_DEPTH && g->location == LOCATION_GIANTS_CROWN);
    for (int stage = GIANTS_DEPTH; stage > 1; stage--) {
        clear_patrols(g);
        western_cross_stage(g, 0);
        ASSERT("north-first exploration traverses the five fixed stages in descending order", g->level == stage - 1 && g->player.last_dy == 1);
    }
    clear_patrols(g);
    ASSERT("the southern end of the pass can be approached", western_path(g, 32, GIANTS_H - 2, 1));
    action_resolve_player(g, (Action){ACTION_MOVE, 32, GIANTS_H - 1});
    ASSERT("evading the Jarl cannot open the southern road to Sunscar", g->location == LOCATION_GIANTS_CROWN && g->level == 1 && g->player.y == GIANTS_H - 2 && !g->giants_crown_defeated);
    map_mark_explored(&g->map, g->player.x, g->player.y);
    ASSERT("a north-first expedition saves its entrance, cached stages and living Jarl", save_game(g, WESTERN_SLOT) && load_game(loaded, WESTERN_SLOT) && loaded->giants_entry_from_north && !loaded->giants_crown_defeated && loaded->giants_crown[GIANTS_DEPTH - 1].valid);
    *g = *loaded;
    for (int stage = 1; stage < GIANTS_DEPTH; stage++) {
        clear_patrols(g);
        western_cross_stage(g, 1);
    }
    defeat_jarl(g);
    for (int stage = GIANTS_DEPTH; stage >= 1; stage--) {
        clear_patrols(g);
        western_cross_stage(g, 0);
    }
    ASSERT("after the Jarl's defeat the full southern journey opens Sunscar", approach_peaks_road(g) && g->giants_crown_defeated);
}

static void western_saved_roads(GameState *g, GameState *loaded) {
    game_init(g);
    giants_enter(g, 0);
    g->enemies[0].hp = 37;
    giants_store(g);
    approach_enter_region(g, APPROACH_SUNSCAR_ROAD);
    g->player.x = 38;
    g->player.y = 1;
    for (int y = 0; y <= 24; y++) {
        g->map.tiles[y][38] = TILE_TOWN_PATH;
    }
    g->map.tiles[0][38] = TILE_APPROACH_DUNGEON_TOWN;
    g->gold = 393;
    g->player.hp = 17;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 38, .y = 1, .underlying_tile = TILE_TOWN_PATH, .item = item_make_ring_of_vigor()};
    g->map.tiles[1][38] = TILE_ITEM;
    map_mark_explored(&g->map, 30, 30);
    ASSERT("the previous desert-road save repairs its route without losing progress", save_game(g, WESTERN_SLOT) && western_legacy_version() && load_game(loaded, WESTERN_SLOT) && loaded->gold == 393 && loaded->player.hp == 17 && loaded->floor_items[0].active && loaded->floor_items[0].item.type == ITEM_ACCESSORY && map_is_explored(&loaded->map, 30, 30) && loaded->giants_crown[0].enemies[0].hp == 37 && map_is_walkable(&loaded->map, loaded->floor_items[0].x, loaded->floor_items[0].y));
    ASSERT("the migrated road closes the old turn and uses its new northern transition", loaded->map.tiles[0][38] != TILE_APPROACH_DUNGEON_TOWN && western_path(loaded, APPROACH_PEAKS_BRANCH_X, 0, 1) && approach_peaks_road(loaded));
    game_init(g);
    g->royal_reported_actions = 1;
    game_enter_town_gate(g, LOCATION_TOWN2, 0, 12);
    ASSERT("the western gate challenges a known enemy of the Crown", western_path(g, 4, 12, 1) && eastern_move(g, 5, 12) && g->stillbury_prompt && g->dialogue_x == 7);
    ASSERT("a pending western checkpoint saves its chosen approach", save_game(g, WESTERN_SLOT) && load_game(loaded, WESTERN_SLOT) && loaded->stillbury_prompt && loaded->dialogue_x == 7);
    eastern_handle_key(loaded, SDL_SCANCODE_ESCAPE, 0);
    ASSERT("withdrawing at the western checkpoint returns to the western gate", loaded->player.x == 2 && loaded->player.y == 12 && !loaded->enemy_count);
    eastern_move(g, 5, 12);
    eastern_handle_key(g, SDL_SCANCODE_N, 0);
    ASSERT("resisting at the western gate places its royal guards on that approach", g->stillbury_challenge == 2 && g->enemy_count == 3 && g->enemies[0].x == 8 && map_is_walkable(&g->map, g->enemies[0].x, g->enemies[0].y));
}

void test_western_geography(void) {
    printf("Western road, Jarl pass and Stillbury geography:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("western journey fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    ASSERT("western journey save slot is unused", !save_exists(WESTERN_SLOT));
    if (!save_exists(WESTERN_SLOT)) {
        for (int seed = 0; seed < 5; seed++) {
            western_journey(g, loaded, seed);
        }
        for (int seed = 0; seed < 3; seed++) {
            northern_journey(g, loaded, seed);
        }
        western_saved_roads(g, loaded);
        remove("saves/savegame_99263.json");
    }
    free(g);
    free(loaded);
}
