#include "test_utils.h"
#include "game/giants_crown.h"
#include "game/approach.h"
#include "game/town_road.h"
#include "systems/save_load.h"
#include "screens/quest_journal.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define GIANTS_SLOT 99311
#define GIANTS_SAVE "saves/savegame_99311.json"

static int reachable(const GameState *g) {
    unsigned char seen[GIANTS_H][GIANTS_W] = {{0}};
    int queue[GIANTS_W * GIANTS_H];
    int head = 0;
    int tail = 1;
    queue[0] = (GIANTS_H - 1) * GIANTS_W + 32;
    seen[GIANTS_H - 1][32] = 1;
    const int dx[] = {1,0,-1,0};
    const int dy[] = {0,1,0,-1};
    while (head < tail) {
        int cell = queue[head++];
        for (int n = 0; n < 4; n++) {
            int x = cell % GIANTS_W + dx[n];
            int y = cell / GIANTS_W + dy[n];
            if (x >= 0 && x < GIANTS_W && y >= 0 && y < GIANTS_H && !seen[y][x] && map_is_walkable(&g->map, x, y)) {
                seen[y][x] = 1;
                queue[tail++] = y * GIANTS_W + x;
            }
        }
    }
    if (!seen[0][32]) {
        return 0;
    }
    for (int n = 0; n < g->enemy_count; n++) {
        const Enemy *enemy = &g->enemies[n];
        if (!seen[enemy->y][enemy->x]) {
            return 0;
        }
        for (int k = n + 1; k < g->enemy_count; k++) {
            if (g->enemies[k].x == enemy->x && g->enemies[k].y == enemy->y) {
                return 0;
            }
        }
    }
    return !seen[23][4];
}

static void layouts(GameState *g) {
    int connected = 1;
    int levels = 1;
    int roster = 0;
    int ogres_weakest = 1;
    for (int seed = 0; seed < 32; seed++) {
        game_init(g);
        srand(7200 + seed);
        giants_enter(g, 0);
        for (int level = 1; level <= GIANTS_DEPTH; level++) {
            connected &= reachable(g);
            int bosses = 0;
            for (int n = 0; n < g->enemy_count; n++) {
                Enemy *enemy = &g->enemies[n];
                bosses += enemy->is_boss;
                roster |= enemy->type == ENEMY_OGRE ? 1 : enemy->type == ENEMY_CAVE_TROLL ? 2 :
                    enemy->type == ENEMY_HILL_GIANT ? 4 : enemy->type == ENEMY_STONE_GIANT ? 8 :
                    enemy->type == ENEMY_SHARD_GOLEM ? 16 : enemy->type == ENEMY_FROST_GIANT ? 32 : 0;
                if (level == 1 && enemy->type != ENEMY_OGRE) {
                    Enemy ogre;
                    game_spawn_enemy(g, &ogre, ENEMY_OGRE, 0, 0);
                    ogres_weakest &= enemy->max_hp > ogre.max_hp && enemy->attack >= ogre.attack;
                }
            }
            levels &= bosses == (level == GIANTS_DEPTH) && g->level == level;
            if (level < GIANTS_DEPTH) {
                giants_travel(g, 32, 0);
            }
        }
    }
    ASSERT("all Giant's Peaks departures and enemies remain reachable across 160 seeded stages", connected);
    ASSERT("five stages have one final boss and permit travel past living regular enemies", levels);
    ASSERT("the region uses all six requested recurring and new regular enemies", roster == 63);
    ASSERT("Ogres are the weakest first-stage enemies", ogres_weakest);
}

static void combat(GameState *g) {
    game_init(g);
    giants_enter(g, 0);
    g->player.x = 9;
    g->player.y = 23;
    Enemy enemy;
    game_spawn_enemy(g, &enemy, ENEMY_STONE_GIANT, 11, 23);
    int damage = 0;
    g->map.tiles[23][10] = TILE_GIANTS_WALL;
    ASSERT("solid walls prevent heavy attacks through sealed rooms", !giants_tick(g, &enemy, &damage) && !enemy.attack_phase);
    g->map.tiles[23][10] = TILE_GIANTS_PAVING;
    giants_tick(g, &enemy, &damage);
    ASSERT("heavy strikes lock the target and give two actions of visible warning", enemy.attack_phase && enemy.move_timer == 2 && giants_attack_marks(&enemy, 8, 23));
    g->player.x = 10;
    g->player.y = 24;
    giants_tick(g, &enemy, &damage);
    ASSERT("the first response action does not resolve the warned attack", enemy.move_timer == 1 && damage == 0 && g->map.tiles[23][8] == TILE_GIANTS_CRACK);
    giants_tick(g, &enemy, &damage);
    ASSERT("baiting a strike breaks sealed masonry without damaging a player who escaped", !enemy.attack_phase && damage == 0 && g->map.tiles[23][8] == TILE_GIANTS_PAVING && enemy.move_timer == -2);
    g->map.tiles[23][8] = TILE_GIANTS_CRACK;
    g->player.x = 9;
    g->player.y = 23;
    ASSERT("every class can open a vault after its giants have been defeated", giants_interact(g) && g->map.tiles[23][8] == TILE_GIANTS_PAVING);
    g->player.x = 5;
    int gold = g->gold;
    giants_interact(g);
    giants_interact(g);
    action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("stone coffers pay only once", g->gold == gold + 30 && g->map.tiles[23][4] == TILE_GIANTS_EMPTY);
}

static void persistence(GameState *g, GameState *loaded) {
    ASSERT("Giant's Peaks test save slot is unused", !save_exists(GIANTS_SLOT));
    if (save_exists(GIANTS_SLOT)) {
        return;
    }
    game_init(g);
    game_enter_town3(g);
    giants_enter(g, 0);
    int x = g->enemies[0].x;
    int y = g->enemies[0].y;
    g->enemies[0].hp = 7;
    g->map.tiles[23][8] = TILE_GIANTS_PAVING;
    g->map.tiles[23][4] = TILE_GIANTS_EMPTY;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 32, .y = 45, .underlying_tile = TILE_GIANTS_PAVING, .item = item_make_long_sword()};
    g->map.tiles[45][32] = TILE_ITEM;
    giants_change_level(g, 1);
    giants_change_level(g, 0);
    ASSERT("backtracking preserves wounds, dropped gear, opened vaults and searched coffers", g->enemies[0].hp == 7 && g->enemies[0].x == x && g->enemies[0].y == y && g->floor_item_count == 1 && g->map.tiles[23][8] == TILE_GIANTS_PAVING && g->map.tiles[23][4] == TILE_GIANTS_EMPTY);
    int damage;
    g->player.x = x + 1;
    g->player.y = y;
    giants_tick(g, &g->enemies[0], &damage);
    ASSERT("saving and loading preserves the active region, loot and pending strike", save_game(g, GIANTS_SLOT) && load_game(loaded, GIANTS_SLOT) && loaded->location == LOCATION_GIANTS_CROWN && loaded->enemies[0].hp == 7 && loaded->enemies[0].attack_phase && loaded->enemies[0].move_timer == 2 && loaded->floor_item_count == 1);
    game_open_town_portal(g);
    ASSERT("Giant's Peaks return portal uses the last visited town", g->location == LOCATION_TOWN3 && g->portal_active);
    game_use_town_portal(g);
    ASSERT("return portal resumes the same encounter without respawning enemies", g->location == LOCATION_GIANTS_CROWN && g->enemies[0].hp == 7 && g->floor_item_count == 1);
    giants_leave(g);
    giants_enter(g, 0);
    ASSERT("physical reentry respawns ordinary enemies while retaining discoveries and dropped loot", g->enemies[0].hp == g->enemies[0].max_hp && g->map.tiles[23][4] == TILE_GIANTS_EMPTY && g->map.tiles[23][8] == TILE_GIANTS_PAVING && g->floor_item_count == 1);
    for (int level = 1; level < GIANTS_DEPTH; level++) {
        giants_change_level(g, 1);
    }
    giants_travel(g, 32, 0);
    ASSERT("living Jarl guards the final departure", g->location == LOCATION_GIANTS_CROWN);
    Enemy *boss = &g->enemies[0];
    g->player.x = boss->x - 1;
    g->player.y = boss->y;
    g->player.attack = 10000;
    boss->hp = 1;
    action_resolve_player(g, (Action){ACTION_MOVE, boss->x, boss->y});
    BossJournalEntry journal;
    ASSERT("defeating the Jarl in ordinary combat records his journal victory", !boss->active && g->giants_crown_defeated &&
        quest_journal_get_boss(g, 20, &journal) && journal.defeated && strcmp(journal.name, "The Last Jarl") == 0);
    int gold = g->gold;
    giants_defeat(g);
    ASSERT("Jarl reward cannot repeat and boss victory survives saves", g->gold == gold && save_game(g, GIANTS_SLOT) && load_game(loaded, GIANTS_SLOT) && loaded->giants_crown_defeated);
    giants_travel(g, 32, 0);
    ASSERT("boss victory opens a departure back to Rosemoor's western approach", g->location == LOCATION_APPROACH && approach_route(g)->area == LOCATION_GIANTS_CROWN && approach_route(g)->town == LOCATION_TOWN3);
    game_init(g);
    g->location = LOCATION_CASTLE;
    map_generate_castle(&g->map, &g->player.x, &g->player.y);
    g->enemies[0] = (Enemy){.type = ENEMY_STONE_GIANT, .hp = 17, .max_hp = 60};
    snprintf(g->enemies[0].name, sizeof(g->enemies[0].name), "Giant");
    g->enemy_count = 1;
    g->frostfell_cache[0].valid = 1;
    g->frostfell_cache[0].enemy_count = 1;
    g->frostfell_cache[0].enemies[0] = (Enemy){.type = ENEMY_FROST_GIANT, .hp = 19, .max_hp = 100};
    snprintf(g->frostfell_cache[0].enemies[0].name, sizeof(g->enemies[0].name), "Ice Giant");
    ASSERT("renames normalize saved active and cached giants without healing them", save_game(g, GIANTS_SLOT) && load_game(loaded, GIANTS_SLOT) && strcmp(loaded->enemies[0].name, "Stone Giant") == 0 && loaded->enemies[0].hp == 17 && strcmp(loaded->frostfell_cache[0].enemies[0].name, "Frost Giant") == 0 && loaded->frostfell_cache[0].enemies[0].hp == 19);
    remove(GIANTS_SAVE);
}

static int old_save(GameState *g) {
    if (!save_game(g, GIANTS_SLOT)) {
        return 0;
    }
    FILE *file = fopen(GIANTS_SAVE, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = calloc(size + 1, 1);
    fread(data, 1, size, file);
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 202);
    cJSON_DeleteItemFromObject(root, "giants_crown");
    cJSON_DeleteItemFromObject(root, "giants_crown_defeated");
    data = cJSON_PrintUnformatted(root);
    file = fopen(GIANTS_SAVE, "wb");
    int ok = file && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    cJSON_Delete(root);
    return ok;
}

static void migration(GameState *g, GameState *loaded) {
    if (save_exists(GIANTS_SLOT)) {
        return;
    }
    game_init(g);
    game_enter_ashen(g);
    g->enemies[0].hp = 7;
    game_descend(g);
    g->gold = 317;
    g->emberforge_quest_state = 1;
    g->emberforge_progress = 1;
    ASSERT("version 202 Ashen saves retain active stage, wounds, gold and forge progress after relocation", old_save(g) && load_game(loaded, GIANTS_SLOT) &&
        loaded->location == LOCATION_ASHEN && loaded->level == 2 && loaded->ashen_cache[0].enemies[0].hp == 7 && loaded->gold == 317 && loaded->emberforge_progress == 1 && !loaded->giants_crown[0].valid);
    game_init(g);
    approach_enter_region(g, ASHEN_APPROACH_ROUTE);
    g->map.stairs_up_x = APPROACH_W - 1;
    g->map.stairs_up_y = APPROACH_H / 2;
    g->map.stairs_down_x = APPROACH_W / 2;
    g->map.stairs_down_y = APPROACH_H / 2 - 2;
    g->player.x = APPROACH_W - 8;
    g->player.y = APPROACH_H / 2;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = APPROACH_W - 9, .y = APPROACH_H / 2, .underlying_tile = TILE_TOWN_PATH, .item = item_make_staff()};
    ASSERT("version 202 western Ashen approaches rotate northward with their player, exploration and dropped gear", old_save(g) && load_game(loaded, GIANTS_SLOT) &&
        approach_route(loaded)->town == LOCATION_CASTLE && loaded->player.x == 32 && loaded->player.y == APPROACH_H - 8 &&
        loaded->floor_items[0].x == 32 && loaded->floor_items[0].y == APPROACH_H - 9 &&
        loaded->map.tiles[loaded->floor_items[0].y][32] == TILE_ITEM && map_is_explored(&loaded->map, loaded->player.x, loaded->player.y));
    ASSERT("saving a relocated approach does not rotate it a second time", save_game(loaded, GIANTS_SLOT) && load_game(g, GIANTS_SLOT) &&
        g->player.x == loaded->player.x && g->player.y == loaded->player.y && g->floor_items[0].y == loaded->floor_items[0].y);
    game_init(g);
    g->location = LOCATION_CASTLE;
    map_generate_castle(&g->map, &g->player.x, &g->player.y);
    g->map.tiles[0][CASTLE_ASHEN_GATE_X] = TILE_WALL;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = CASTLE_ASHEN_GATE_X, .y = 5, .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_staff()};
    g->map.tiles[5][CASTLE_ASHEN_GATE_X] = TILE_ITEM;
    ASSERT("version 202 Castle Grounds gain the northern path while preserving dropped objects", old_save(g) && load_game(loaded, GIANTS_SLOT) &&
        loaded->map.tiles[0][CASTLE_ASHEN_GATE_X] == TILE_TOWN_EXIT && loaded->map.tiles[5][CASTLE_ASHEN_GATE_X] == TILE_ITEM &&
        loaded->floor_items[0].underlying_tile == TILE_TOWN_PATH && loaded->map.tiles[10][CROWNROAD_X] == TILE_TOWN_EXIT);
    remove(GIANTS_SAVE);
}

void test_giants_crown(void) {
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("Giant's Peaks fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    TownApproach route;
    ASSERT("Rosemoor's west road reaches Giant's Peaks", town_road_connection(LOCATION_TOWN3, TOWN_SIDE_WEST, &route) && route.area == LOCATION_GIANTS_CROWN);
    ASSERT("Ashen's stable approach now leaves Castle Grounds northward", town_approaches[ASHEN_APPROACH_ROUTE].town == LOCATION_CASTLE && town_approaches[ASHEN_APPROACH_ROUTE].gate_y == 0);
    layouts(g);
    combat(g);
    persistence(g, loaded);
    migration(g, loaded);
    game_init(g);
    ASSERT("new games reset Giant's Peaks state", !g->giants_crown[0].valid && !g->giants_crown_defeated);
    free(g);
    free(loaded);
}
