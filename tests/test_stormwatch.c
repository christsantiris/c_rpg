#include "test_utils.h"
#include "game/stormwatch.h"
#include "game/approach.h"
#include "game/town_road.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define STORM_SLOT 99298

static int reachable(const GameState *g) {
    unsigned char seen[STORM_H][STORM_W] = {{0}};
    int queue[STORM_W * STORM_H];
    int head = 0;
    int tail = 1;
    queue[0] = g->map.stairs_up_y * STORM_W + g->map.stairs_up_x;
    seen[g->map.stairs_up_y][g->map.stairs_up_x] = 1;
    const int dx[] = {1,0,-1,0};
    const int dy[] = {0,1,0,-1};
    while (head < tail) {
        int cell = queue[head++];
        for (int n = 0; n < 4; n++) {
            int x = cell % STORM_W + dx[n];
            int y = cell / STORM_W + dy[n];
            if (x >= 0 && x < STORM_W && y >= 0 && y < STORM_H && !seen[y][x] && map_is_walkable(&g->map, x, y)) {
                seen[y][x] = 1;
                queue[tail++] = y * STORM_W + x;
            }
        }
    }
    for (int y = 0; y < STORM_H; y++) {
        for (int x = 0; x < STORM_W; x++) {
            if (map_is_walkable(&g->map, x, y) && !seen[y][x]) {
                return 0;
            }
            if (g->map.tiles[y][x] == TILE_STORM_SUPPLIES || g->map.tiles[y][x] == TILE_STORM_CRANK) {
                int adjacent = 0;
                for (int n = 0; n < 4; n++) {
                    int nx = x + dx[n];
                    int ny = y + dy[n];
                    if (nx >= 0 && nx < STORM_W && ny >= 0 && ny < STORM_H && seen[ny][nx]) {
                        adjacent = 1;
                    }
                }
                if (!adjacent) {
                    return 0;
                }
            }
        }
    }
    return seen[g->map.stairs_down_y][g->map.stairs_down_x];
}

static void wind_fixture(GameState *g) {
    stormwatch_enter(g, 0);
    g->enemy_count = 0;
    g->player.x = 24;
    g->player.y = 24;
    for (int y = 20; y <= 28; y++) {
        for (int x = 2; x < 62; x++) {
            g->map.tiles[y][x] = TILE_STORM_GRASS;
        }
    }
    g->stormwatch[0].phase[0] = 1;
    stormwatch_wind(g); // Capture the exact two-action warning.
}

static void legacy_version(void) {
    char path[80];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", STORM_SLOT);
    FILE *file = fopen(path, "rb");
    if (!file) {
        return;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = calloc(size + 1, 1);
    fread(buffer, 1, size, file);
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 197);
    cJSON_DeleteItemFromObject(root, "stormwatch");
    cJSON_DeleteItemFromObject(root, "stormwatch_defeated");
    char *json = cJSON_PrintUnformatted(root);
    file = fopen(path, "wb");
    fwrite(json, 1, strlen(json), file);
    fclose(file);
    free(json);
    cJSON_Delete(root);
}

void test_stormwatch(void) {
    printf("Stormwatch Headland\n");
    ASSERT("Stormwatch slot is unused", !save_exists(STORM_SLOT));
    if (save_exists(STORM_SLOT)) {
        return;
    }
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("Stormwatch fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    game_init(g);
    TownApproach connection;
    ASSERT("east Dawnmere leads to Stormwatch", town_road_connection(LOCATION_DAWNMERE, TOWN_SIDE_EAST, &connection) && connection.area == LOCATION_STORMWATCH);
    ASSERT("retired route indices stay reserved", town_approaches[17].area == (Location)-1 && town_approaches[18].area == LOCATION_STORMWATCH);
    int valid_spawns = 1;
    for (int seed = 0; seed < 30; seed++) {
        srand(seed);
        stormwatch_enter(g, 0);
        for (int level = 1; level <= STORM_DEPTH; level++) {
            if (!reachable(g)) {
                valid_spawns = 0;
            }
            if (level >= 2) {
                g->map.tiles[24][32] = TILE_STORM_SHUTTER_DOWN;
                if (!reachable(g)) {
                    valid_spawns = 0;
                }
                g->map.tiles[24][32] = TILE_STORM_SHUTTER_UP;
            }
            for (int i = 0; i < g->enemy_count; i++) {
                Enemy *enemy = &g->enemies[i];
                if (!map_is_walkable(&g->map, enemy->x, enemy->y)) {
                    valid_spawns = 0;
                }
                for (int n = 0; n < i; n++) {
                    if (g->enemies[n].x == enemy->x && g->enemies[n].y == enemy->y) {
                        valid_spawns = 0;
                    }
                }
            }
            if (level < STORM_DEPTH) {
                stormwatch_change_level(g, 1);
            }
        }
    }
    ASSERT("all five stages, exits, enemies and side props stay reachable across 30 seeds and both shutter settings", valid_spawns);
    ASSERT("final departure is locked before victory", stormwatch_travel(g, g->map.stairs_down_x, 0) && g->location == LOCATION_STORMWATCH);
    int gold = g->gold;
    stormwatch_defeat(g);
    stormwatch_defeat(g);
    ASSERT("boss gold and victory are awarded once", g->gold == gold + 220 && g->stormwatch_defeated);
    BossJournalEntry boss;
    ASSERT("Matriarch appears in the boss journal", quest_journal_get_boss(g, 18, &boss) && boss.defeated && !strcmp(boss.name, "Tempest Matriarch"));
    ASSERT("victory returns to the physical Dawnmere approach", stormwatch_travel(g, g->map.stairs_down_x, 0) && g->location == LOCATION_APPROACH && g->approach_route == 18);
    stormwatch_enter(g, 0);
    int original_count = g->enemy_count;
    g->enemies[0].active = 0;
    g->gold += 7;
    stormwatch_change_level(g, 1);
    g->player.x = 31;
    g->player.y = 24;
    ASSERT("raised shutter blocks the crossing", !map_is_walkable(&g->map, 32, 24));
    stormwatch_interact(g);
    ASSERT("crank lowers a walkable timber threshold", g->map.tiles[24][32] == TILE_STORM_SHUTTER_DOWN && map_is_walkable(&g->map, 32, 24));
    g->enemies[0].x = 32;
    g->enemies[0].y = 24;
    g->enemies[0].active = 1;
    stormwatch_interact(g);
    ASSERT("occupied shutter cannot be raised through an enemy", g->map.tiles[24][32] == TILE_STORM_SHUTTER_DOWN);
    g->enemies[0].active = 0;
    stormwatch_interact(g);
    ASSERT("clear shutter raises into a wall", g->map.tiles[24][32] == TILE_STORM_SHUTTER_UP && !map_is_walkable(&g->map, 32, 24));
    stormwatch_change_level(g, 0);
    ASSERT("backtracking retains defeated enemies", g->enemy_count == original_count && !g->enemies[0].active);
    stormwatch_leave(g);
    stormwatch_enter(g, 0);
    ASSERT("fresh physical entry respawns enemies at stage one", g->level == 1 && g->enemy_count > 0 && g->enemies[0].active);
    wind_fixture(g);
    ASSERT("gust warning covers the exact exposed ground", stormwatch_wind_marks(g, 0, 24, 24) && g->stormwatch[0].phase[0] == 2);
    stormwatch_wind(g);
    ASSERT("first warned action does not push", g->player.x == 24 && g->stormwatch[0].phase[0] == 3);
    stormwatch_wind(g);
    ASSERT("gust moves player one safe cardinal tile", g->player.x == 25 && g->player.y == 24);
    wind_fixture(g);
    stormwatch_wind(g);
    stormwatch_interact(g);
    stormwatch_wind(g);
    ASSERT("brace cancels one phase of displacement and expires", g->player.x == 24 && !g->stormwatch[0].braced);
    wind_fixture(g);
    g->map.tiles[24][25] = TILE_STORM_WATER;
    stormwatch_wind(g);
    stormwatch_wind(g);
    ASSERT("gust never throws the player into water", g->player.x == 24);
    wind_fixture(g);
    g->stormwatch[0].next_direction[0] = 2;
    ASSERT("next wind direction cannot change the declared gust", g->stormwatch[0].direction[0] == 0);
    g->enemies[0] = (Enemy){.active = 1, .type = ENEMY_HARPY_RAIDER, .x = 26, .y = 24};
    g->enemies[1] = (Enemy){.active = 1, .type = ENEMY_CLIFF_GARGOYLE, .x = 28, .y = 24};
    g->enemy_count = 2;
    stormwatch_wind(g);
    stormwatch_wind(g);
    ASSERT("raiders move with wind but planted gargoyles resist", g->enemies[0].x == 27 && g->enemies[1].x == 28);
    wind_fixture(g);
    Enemy *enemy = &g->enemies[0];
    *enemy = (Enemy){.active = 1, .type = ENEMY_TEMPEST_MATRIARCH, .is_boss = 1, .x = 20, .y = 24, .hp = 300, .max_hp = 320, .attack = 30, .defense = 7};
    g->enemy_count = 1;
    int damage = 0;
    stormwatch_tick(g, enemy, &damage);
    ASSERT("Matriarch locks a two-action dive lane", enemy->attack_phase && enemy->move_timer == 2 && stormwatch_attack_marks(g, enemy, 24, 24) && !damage);
    int target_x = enemy->attack_target_x;
    enemy->hp = 100;
    stormwatch_tick(g, enemy, &damage);
    ASSERT("crossing half health preserves the pending attack", enemy->attack_target_x == target_x && enemy->boss_attack == STORM_DIVE && enemy->move_timer == 1 && !enemy->boss_wave_used);
    g->player.y = 25;
    stormwatch_tick(g, enemy, &damage);
    ASSERT("dodged dive gives two complete recovery actions", damage == 0 && enemy->move_timer == -2 && !enemy->attack_phase);
    stormwatch_tick(g, enemy, &damage);
    stormwatch_tick(g, enemy, &damage);
    ASSERT("recovery cannot add a free adjacent hit", !damage && enemy->move_timer == 0 && !enemy->attack_phase);
    stormwatch_tick(g, enemy, &damage);
    ASSERT("Matriarch summons only a finite pair", enemy->boss_wave_used && g->enemy_count == 3 && !g->enemies[1].experience && g->enemies[1].revived);
    wind_fixture(g);
    g->last_town_visited = LOCATION_TOWN2;
    g->gold = 777;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 23, .y = 24, .underlying_tile = TILE_STORM_GRASS};
    g->map.tiles[24][23] = TILE_ITEM;
    g->floor_items[0].item = (Item){.type = ITEM_GOLD, .value = 31};
    g->stormwatch[0].braced = 1;
    ASSERT("active stage saves and loads", save_game(g, STORM_SLOT) && load_game(loaded, STORM_SLOT));
    ASSERT("save retains warning mask, exact clock, brace and dropped loot", loaded->location == LOCATION_STORMWATCH && loaded->stormwatch[0].phase[0] == 2 && loaded->stormwatch[0].braced &&
        !memcmp(loaded->stormwatch[0].warned, g->stormwatch[0].warned, sizeof(g->stormwatch[0].warned)) && loaded->floor_item_count == 1 && loaded->floor_items[0].underlying_tile == TILE_STORM_GRASS);
    game_open_town_portal(loaded);
    ASSERT("Return to Town uses last town visited", loaded->location == LOCATION_TOWN2 && loaded->portal_active && loaded->return_portal_town == LOCATION_TOWN2);
    ASSERT("away expedition saves", save_game(loaded, STORM_SLOT) && load_game(g, STORM_SLOT));
    game_use_town_portal(g);
    ASSERT("portal restores the expedition with wind and loot intact", g->location == LOCATION_STORMWATCH && g->player.x == 24 && g->player.y == 24 && g->stormwatch[0].phase[0] == 2 && g->floor_item_count == 1 && g->gold == 777);
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    g->map.tiles[12][TOWN_W - 1] = TILE_WALL;
    g->gold = 321;
    int saved = save_game(g, STORM_SLOT);
    legacy_version();
    ASSERT("version197 Dawnmere saves gain the east exit and preserve progress", saved && load_game(loaded, STORM_SLOT) && loaded->gold == 321 && loaded->map.tiles[12][TOWN_W - 1] == TILE_TOWN_EXIT);
    stormwatch_enter(loaded, 0);
    loaded->stormwatch[0].phase[0] = 9;
    ASSERT("invalid wind state cannot be saved", !save_game(loaded, STORM_SLOT));
    game_init(g);
    stormwatch_enter(g, 0);
    stormwatch_change_level(g, 1);
    g->enemy_count = 0;
    g->player.x = 31;
    g->player.y = 24;
    stormwatch_interact(g);
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 32, .y = 24, .underlying_tile = TILE_STORM_SHUTTER_DOWN};
    g->map.tiles[24][32] = TILE_ITEM;
    stormwatch_interact(g);
    ASSERT("shutter cannot erase dropped loot", g->map.tiles[24][32] == TILE_ITEM && g->floor_items[0].active);
    g->floor_item_count = 0;
    g->map.tiles[24][32] = TILE_STORM_SHUTTER_DOWN;
    g->stormwatch[1].phase[0] = 1;
    stormwatch_wind(g);
    int warned = stormwatch_wind_marks(g, 0, 35, 24);
    stormwatch_interact(g);
    ASSERT("closing cover shrinks the declared wind mask", warned && !stormwatch_wind_marks(g, 0, 35, 24));
    stormwatch_interact(g);
    ASSERT("opening cover never expands an existing warning", !stormwatch_wind_marks(g, 0, 35, 24));
    g->stormwatch[1].phase[0] = 1;
    stormwatch_wind(g);
    ASSERT("next warning includes the newly exposed crossing", stormwatch_wind_marks(g, 0, 35, 24));
    g->player.x = 8;
    g->player.y = 18;
    int before = g->gold;
    stormwatch_interact(g);
    int after = g->gold;
    stormwatch_interact(g);
    ASSERT("optional supply chest grants gold once per expedition", after > before && g->gold == after && g->map.tiles[17][8] == TILE_STORM_EMPTY);
    ASSERT("searched supplies and lowered shutter survive save/load", save_game(g, STORM_SLOT) && load_game(loaded, STORM_SLOT) && loaded->map.tiles[17][8] == TILE_STORM_EMPTY && loaded->map.tiles[24][32] == TILE_STORM_SHUTTER_DOWN);
    wind_fixture(g);
    g->stormwatch[0].phase[0] = 2;
    g->enemies[0] = (Enemy){.active = 1, .type = ENEMY_HARPY_RAIDER, .x = 20, .y = 24, .hp = 40, .max_hp = 40, .frozen_turns = 2, .move_timer = 2,
        .attack_phase = 1, .boss_attack = STORM_DIVE, .attack_target_x = 20, .attack_target_y = 24, .boss_aux_x = 22, .boss_aux_y = 24, .facing_dx = 1};
    g->enemy_count = 1;
    action_resolve_enemies(g);
    ASSERT("freezing preserves attack preparation while environmental time advances", g->enemies[0].move_timer == 2 && g->enemies[0].frozen_turns == 1 && g->stormwatch[0].phase[0] == 3);
    ASSERT("pending attack and freeze round trip exactly", save_game(g, STORM_SLOT) && load_game(loaded, STORM_SLOT) && loaded->enemies[0].attack_phase && loaded->enemies[0].boss_aux_x == 22 && loaded->enemies[0].frozen_turns == 1);
    Enemy gargoyle = {.type = ENEMY_CLIFF_GARGOYLE, .x = 20, .y = 20, .facing_dx = 1, .defense = 7};
    ASSERT("gargoyle armor rewards flanking", stormwatch_defense(&gargoyle, 21, 20) == 7 && stormwatch_defense(&gargoyle, 19, 20) == 2);
    gargoyle.move_timer = -1;
    gargoyle.defense = 1;
    ASSERT("gargoyle is vulnerable throughout recovery", stormwatch_defense(&gargoyle, 21, 20) == 1);
    g->portal_active = 1;
    g->portal_location = LOCATION_DUNGEON;
    stormwatch_enter(g, 0);
    ASSERT("fresh Stormwatch entry preserves another region's portal", g->portal_active && g->portal_location == LOCATION_DUNGEON);
    g->debug_passages_unlocked = 1;
    while (g->level < STORM_DEPTH) {
        stormwatch_change_level(g, 1);
    }
    int count = g->enemy_count;
    stormwatch_change_level(g, 1);
    ASSERT("debug opens departure without granting victory or removing boss", g->location == LOCATION_APPROACH && !g->stormwatch_defeated && g->stormwatch[4].enemy_count == count);
    game_init(g);
    town_road_enter(g, LOCATION_DAWNMERE, TOWN_SIDE_WEST, 0);
    g->map.tiles[TOWN_ROAD_H / 2][TOWN_ROAD_W - 1] = TILE_WALL;
    g->gold = 888;
    saved = save_game(g, STORM_SLOT);
    legacy_version();
    ASSERT("old Dawnmere exterior gains its east continuation", saved && load_game(loaded, STORM_SLOT) && loaded->location == LOCATION_TOWN_ROAD && loaded->gold == 888 && loaded->map.tiles[TOWN_ROAD_H / 2][TOWN_ROAD_W - 1] == TILE_TOWN_ROAD_EXIT_EAST);
    char path[80];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", STORM_SLOT);
    remove(path);
    free(g);
    free(loaded);
}
