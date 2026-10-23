#include "test_utils.h"
#include "approach_checks.h"
#include "game/pirate.h"
#include "game/approach.h"
#include "game/town_road.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define PIRATE_SLOT 99284

static int reachable(const Map *map, int tx, int ty) {
    unsigned char seen[PIRATE_H][PIRATE_W] = {0};
    int queue[PIRATE_H * PIRATE_W];
    int head = 0;
    int tail = 1;
    queue[0] = map->stairs_up_y * PIRATE_W + map->stairs_up_x;
    seen[map->stairs_up_y][map->stairs_up_x] = 1;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail) {
        int cell = queue[head++];
        int x = cell % PIRATE_W;
        int y = cell / PIRATE_W;
        if (x == tx && y == ty) {
            return 1;
        }
        for (int n = 0; n < 4; n++) {
            int nx = x + dx[n];
            int ny = y + dy[n];
            if (nx >= 0 && nx < PIRATE_W && ny >= 0 && ny < PIRATE_H && !seen[ny][nx] && map_is_walkable(map, nx, ny)) {
                seen[ny][nx] = 1;
                queue[tail++] = ny * PIRATE_W + nx;
            }
        }
    }
    return 0;
}

static int saved_version(int version) {
    FILE *file = fopen("saves/savegame_99284.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = malloc(size + 1);
    if (!text) {
        fclose(file);
        return 0;
    }
    size_t count = fread(text, 1, size, file);
    text[count] = 0;
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    if (version < 184) {
        cJSON_DeleteItemFromObject(root, "pirate_cache");
        cJSON_DeleteItemFromObject(root, "pirate_defeated");
    }
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99284.json", "wb");
    int ok = file && text && fputs(text, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(text);
    return ok;
}

void test_pirate(void) {
    printf("Pirate Coast generator, combat and persistence:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    Map *map = malloc(sizeof(*map));
    ASSERT("Pirate Coast fixtures allocate", g && loaded && map);
    if (!g || !loaded || !map) {
        free(g); free(loaded); free(map);
        return;
    }
    ASSERT("Pirate Coast save fixture will not overwrite a character", !save_exists(PIRATE_SLOT));
    if (save_exists(PIRATE_SLOT)) {
        free(g); free(loaded); free(map);
        return;
    }
    int valid = 1;
    int connected = 1;
    int shaped = 1;
    for (unsigned int seed = 1; seed <= 100; seed++) {
        for (int level = 1; level <= PIRATE_DEPTH; level++) {
            pirate_generate(map, level, seed, 0);
            valid &= reachable(map, map->stairs_down_x, map->stairs_down_y);
            for (int i = 0; i < 3; i++) {
                int x;
                int y;
                pirate_chest_position(level, i, &x, &y);
                valid &= map->tiles[y][x] == TILE_PIRATE_CHEST &&
                    (reachable(map, x - 1, y) || reachable(map, x + 1, y) ||
                    reachable(map, x, y - 1) || reachable(map, x, y + 1));
            }
            for (int forward = 0; forward < 2; forward++) {
                int x;
                int y;
                pirate_edge(level, forward, &x, &y);
                valid &= (x == 0 || y == 0 || x == PIRATE_W - 1 || y == PIRATE_H - 1) &&
                    map->tiles[y][x] == (forward ? TILE_PIRATE_EXIT : TILE_PIRATE_ENTRANCE) && reachable(map, x, y);
                int dx = x == 0 || x == PIRATE_W - 1 ? 0 : 1;
                int dy = dx ? 0 : 1;
                valid &= !map_is_walkable(map, x + dx, y + dy) && !map_is_walkable(map, x - dx, y - dy);
            }
            unsigned char cells[PIRATE_H][PIRATE_W];
            pirate_reachable(map, cells);
            int floors = 0;
            for (int y = 0; y < PIRATE_H; y++) {
                for (int x = 0; x < PIRATE_W; x++) {
                    if (map_is_walkable(map, x, y)) {
                        connected &= cells[y][x];
                        floors++;
                    }
                }
            }
            shaped &= floors > 600 && floors < PIRATE_W * PIRATE_H / 2;
        }
    }
    ASSERT("500 generated stages connect boundary gates and every plunder chest", valid);
    ASSERT("every walkable tile belongs to the player's entrance component", connected);
    ASSERT("every stage has defined terrain rather than a giant open rectangle", shaped);
    pirate_generate(map, 3, 13, 0);
    ASSERT("piers have impassable sea between connected wooden platforms", map->tiles[18][10] == TILE_PIRATE_SEA && !map_is_walkable(map, 10, 18) && reachable(map, 28, 25));
    pirate_generate(map, 5, 13, 0);
    ASSERT("the flagship uses connected compartments with bulkheads and sea outside the hull", map->tiles[15][44] == TILE_PIRATE_PALISADE && map->tiles[1][40] == TILE_PIRATE_SEA && reachable(map, 69, 18));
    game_init(g);
    int spawns = 1;
    for (int seed = 1; seed <= 100; seed++) {
        memset(g->pirate_cache, 0, sizeof(g->pirate_cache));
        srand(seed);
        pirate_enter(g);
        for (int level = 1; level <= PIRATE_DEPTH; level++) {
            unsigned char cells[PIRATE_H][PIRATE_W];
            pirate_reachable(&g->map, cells);
            spawns &= g->enemy_count == 8 + level / 2 + (level == PIRATE_DEPTH);
            for (int i = 0; i < g->enemy_count; i++) {
                const Enemy *enemy = &g->enemies[i];
                spawns &= reachable(&g->map, enemy->x, enemy->y) && cells[enemy->y][enemy->x] &&
                    abs(enemy->x - g->player.x) + abs(enemy->y - g->player.y) >= 10;
                for (int j = 0; j < i; j++) {
                    spawns &= enemy->x != g->enemies[j].x || enemy->y != g->enemies[j].y;
                }
            }
            if (level < PIRATE_DEPTH) {
                pirate_change_level(g, 1);
            }
        }
    }
    ASSERT("all enemy spawns across 500 stages are reachable, unique, and clear of arrivals", spawns);
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 20, TOWN_H - 1);
    action_resolve_player(g, (Action){ACTION_MOVE, 20, TOWN_H - 1});
    ASSERT("Dawnmere's southern gate enters its optional town bypass", g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_DAWNMERE);
    g->player.x = TOWN_ROAD_W / 2;
    g->player.y = TOWN_ROAD_H - 2;
    action_resolve_player(g, (Action){ACTION_MOVE, g->player.x, TOWN_ROAD_H - 1});
    ASSERT("southern bypass continuation enters the safe Pirate Coast approach", g->location == LOCATION_APPROACH && approach_route(g) && approach_route(g)->area == LOCATION_PIRATE_COAST && g->enemy_count == 0);
    ASSERT("the approach puts open sea across its entire southern edge", g->map.tiles[APPROACH_H - 1][1] == TILE_COAST_SEA && g->map.tiles[APPROACH_H - 1][APPROACH_W - 2] == TILE_COAST_SEA);
    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y + 1;
    action_resolve_player(g, (Action){ACTION_MOVE, g->player.x, g->player.y - 1});
    ASSERT("walking into the fort enters a rendered pirate stage with its own roster", g->location == LOCATION_PIRATE_COAST && g->level == 1 && g->enemy_count > 0 && g->enemies[0].type >= ENEMY_PIRATE_RAIDER && g->map.tiles[0][8] == TILE_PIRATE_ENTRANCE);
    g->enemies[0].hp = 17;
    g->player.x = 10;
    g->player.y = 8;
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, 0, 0});
    map_mark_explored(&g->map, 10, 8);
    g->player.x = 20;
    g->player.y = 4;
    int gold = g->gold;
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("plunder claims once and changes the chest into ordinary ground", g->gold == gold + 30 && g->pirate_cache[0].treasure_claimed == 1 && g->map.tiles[3][20] == TILE_PIRATE_SAND);
    pirate_change_level(g, 1);
    pirate_change_level(g, 0);
    ASSERT("backtracking preserves wounded pirates, loot, exploration and claimed chests", g->enemies[0].hp == 17 && g->floor_item_count == 1 && g->floor_items[0].active && map_is_explored(&g->map, 10, 8) && g->map.tiles[3][20] != TILE_PIRATE_CHEST);
    const EnemyType renamed[] = {ENEMY_PIRATE_RAIDER, ENEMY_PIRATE_BUCCANEER, ENEMY_PIRATE_SWASHBUCKLER};
    const char *legacy_names[] = {"Cutlass Raider", "Powder Bomber", "Cannon Gunner"};
    const char *current_names[] = {"Raider", "Buccaneer", "Swashbuckler"};
    int active_names[3] = {-1, -1, -1};
    int cached_names[3] = {-1, -1, -1};
    for (int i = 0; i < 3; i++) {
        for (int n = 0; n < g->enemy_count; n++) {
            if (g->enemies[n].type == renamed[i]) {
                active_names[i] = n;
                snprintf(g->enemies[n].name, sizeof(g->enemies[n].name), "%s", legacy_names[i]);
            }
        }
        for (int n = 0; n < g->pirate_cache[1].enemy_count; n++) {
            if (g->pirate_cache[1].enemies[n].type == renamed[i]) {
                cached_names[i] = n;
                snprintf(g->pirate_cache[1].enemies[n].name, sizeof(g->enemies[n].name), "%s", legacy_names[i]);
            }
        }
    }
    Enemy *old_buccaneer = &g->enemies[active_names[1]];
    old_buccaneer->attack_phase = 1;
    old_buccaneer->move_timer = 2;
    old_buccaneer->boss_attack = 2;
    old_buccaneer->frozen_turns = 3;
    Enemy *old_swashbuckler = &g->pirate_cache[1].enemies[cached_names[2]];
    old_swashbuckler->attack_phase = 1;
    old_swashbuckler->move_timer = 2;
    old_swashbuckler->boss_attack = PIRATE_SHOT;
    ASSERT("active coastal progress saves and reloads", save_game(g, PIRATE_SLOT) && load_game(loaded, PIRATE_SLOT) && loaded->enemies[0].hp == 17 && loaded->pirate_cache[0].treasure_claimed == 1 && loaded->floor_item_count == 1);
    int names_match = 1;
    for (int i = 0; i < 3; i++) {
        names_match &= active_names[i] >= 0 && cached_names[i] >= 0 &&
            strcmp(loaded->enemies[active_names[i]].name, current_names[i]) == 0 &&
            strcmp(loaded->pirate_cache[1].enemies[cached_names[i]].name, current_names[i]) == 0;
    }
    ASSERT("saved active and cached pirates adopt their new names", names_match);
    ASSERT("obsolete ranged attacks are cancelled while enemy wounds and freeze survive", !loaded->enemies[active_names[1]].attack_phase &&
        loaded->enemies[active_names[1]].move_timer == 0 && !loaded->pirate_cache[1].enemies[cached_names[2]].attack_phase &&
        loaded->enemies[0].hp == 17 && loaded->enemies[active_names[1]].frozen_turns == 3);
    g->player.x = 1;
    g->player.y = 33;
    g->map.tiles[33][1] = TILE_PIRATE_SAND;
    g->enemies[0].x = 2;
    g->enemies[0].y = 33;
    g->enemies[0].attack_phase = 1;
    g->enemies[0].move_timer = 2;
    g->enemies[0].frozen_turns = 3;
    g->enemies[1].active = 0;
    g->floor_items[0].x = 3;
    g->floor_items[0].y = 33;
    g->floor_items[0].item.sharpened = 1;
    g->pirate_cache[1].enemies[0].x = 2;
    g->pirate_cache[1].enemies[0].y = 33;
    g->pirate_cache[1].enemies[0].hp = 11;
    g->portal_active = 1;
    g->portal_location = LOCATION_PIRATE_COAST;
    g->portal_level = 2;
    g->portal_x = 2;
    g->portal_y = 33;
    g->portal_origin_tile = TILE_PIRATE_SAND;
    gold = g->gold;
    ASSERT("version 184 expeditions migrate onto the new connected layout", save_game(g, PIRATE_SLOT) && saved_version(184) && load_game(loaded, PIRATE_SLOT) &&
        reachable(&loaded->map, loaded->player.x, loaded->player.y) && reachable(&loaded->map, loaded->enemies[0].x, loaded->enemies[0].y) && loaded->map.tiles[0][8] == TILE_PIRATE_ENTRANCE);
    ASSERT("migration preserves wounds, deaths, freeze, plunder, upgraded loot and gold", loaded->enemies[0].hp == 17 && loaded->enemies[0].frozen_turns == 3 &&
        !loaded->enemies[1].active && loaded->pirate_cache[0].treasure_claimed == 1 && loaded->gold == gold && loaded->floor_items[0].active &&
        loaded->floor_items[0].item.sharpened && reachable(&loaded->map, loaded->floor_items[0].x, loaded->floor_items[0].y));
    ASSERT("relocated warning attacks are cancelled before they can hurt the player", !loaded->enemies[0].attack_phase && loaded->enemies[0].move_timer < 0);
    pirate_generate(map, 2, loaded->pirate_cache[1].seed, loaded->pirate_cache[1].treasure_claimed);
    ASSERT("migration also repairs cached stages and return portal anchors", loaded->pirate_cache[1].enemies[0].hp == 11 &&
        reachable(map, loaded->pirate_cache[1].enemies[0].x, loaded->pirate_cache[1].enemies[0].y) && loaded->portal_active && reachable(map, loaded->portal_x, loaded->portal_y));
    *g = *loaded;
    int migrated_x = g->player.x;
    int migrated_y = g->player.y;
    g->enemies[0].attack_phase = 1;
    g->enemies[0].move_timer = 2;
    ASSERT("saving the migrated expedition does not rerun migration or cancel new warnings", save_game(g, PIRATE_SLOT) && load_game(loaded, PIRATE_SLOT) &&
        loaded->player.x == migrated_x && loaded->player.y == migrated_y && loaded->enemies[0].attack_phase == 1 && loaded->enemies[0].move_timer == 2);
    loaded->portal_active = 0;
    loaded->enemies[0].attack_phase = 0;
    loaded->enemies[0].frozen_turns = 0;
    *g = *loaded;
    g->player.x = 10;
    g->player.y = 15;
    game_open_town_portal(g);
    ASSERT("return portal visits the last town rather than Oakhaven", g->location == LOCATION_DAWNMERE && g->portal_active);
    game_use_town_portal(g);
    ASSERT("portal restores Pirate Coast and its exact persistent expedition", g->location == LOCATION_PIRATE_COAST && g->level == 1 && g->enemies[0].hp == 17 && g->floor_item_count == 1 && map_is_walkable(&g->map, g->player.x, g->player.y));
    *loaded = *g;
    pirate_generate(&g->map, 5, 1, 0);
    g->level = 5;
    Enemy crossbow = {.active = 1, .type = ENEMY_PIRATE_CROSSBOW, .x = 30, .y = 15, .attack = 22};
    g->player.x = 35;
    g->player.y = 15;
    for (int y = 14; y <= 17; y++) {
        for (int x = 29; x <= 36; x++) {
            g->map.tiles[y][x] = TILE_PIRATE_DECK;
        }
    }
    int damage = 0;
    pirate_tick(g, &crossbow, &damage);
    ASSERT("crossbows declare a fixed lane with one turn of warning", crossbow.attack_phase && crossbow.move_timer == 1 && damage == 0 && pirate_marks(g, &crossbow, 35, 15));
    g->map.tiles[15][33] = TILE_PIRATE_CRATE;
    ASSERT("cargo blocks both the marked crossbow lane and its damage", !pirate_marks(g, &crossbow, 35, 15));
    pirate_tick(g, &crossbow, &damage);
    ASSERT("covered players take no crossbow damage and the archer reloads", damage == 0 && crossbow.move_timer < 0 && !crossbow.attack_phase);
    g->map.tiles[15][33] = TILE_PIRATE_DECK;
    crossbow.move_timer = 0;
    pirate_tick(g, &crossbow, &damage);
    pirate_tick(g, &crossbow, &damage);
    ASSERT("remaining in a warned uncovered lane causes damage", damage == 22);
    const EnemyType swordsmen[] = {ENEMY_PIRATE_BUCCANEER, ENEMY_PIRATE_SWASHBUCKLER};
    for (int i = 0; i < 2; i++) {
        g->enemy_count = 1;
        g->enemies[0] = (Enemy){.active = 1, .type = swordsmen[i], .x = 30, .y = 15, .hp = 50, .max_hp = 50, .attack = 20};
        g->player.x = 35;
        g->player.y = 15;
        g->player.hp = 500;
        g->player.max_hp = 500;
        g->player.defense = 0;
        action_resolve_enemies(g);
        ASSERT("pirate swordsmen approach without dealing ranged damage", g->player.hp == 500 && g->enemies[0].x > 30 && !g->enemies[0].attack_phase);
        g->enemies[0].x = 34;
        g->enemies[0].y = 15;
        action_resolve_enemies(g);
        ASSERT("pirate swordsmen deal damage when adjacent", g->player.hp == 480);
    }
    *g = *loaded;
    while (g->level > 1) {
        pirate_change_level(g, 0);
    }
    while (g->level < 5) {
        pirate_change_level(g, 1);
    }
    int boss = -1;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == ENEMY_CAPTAIN_VOSS) {
            boss = i;
        }
    }
    ASSERT("Captain Voss appears on the fifth stage", boss >= 0);
    if (boss >= 0) {
        g->player.x = 65;
        g->player.y = 18;
        pirate_tick(g, &g->enemies[boss], &damage);
        ASSERT("Voss announces a three-row blade wave with two turns to escape", g->enemies[boss].move_timer == 2 && pirate_marks(g, &g->enemies[boss], 65, 17) && pirate_marks(g, &g->enemies[boss], 65, 19) && !pirate_marks(g, &g->enemies[boss], 65, 20));
        ASSERT("queued blade waves persist through an active-stage save", save_game(g, PIRATE_SLOT) && load_game(loaded, PIRATE_SLOT) && loaded->enemies[boss].attack_phase == 1 && loaded->enemies[boss].move_timer == 2 && loaded->enemies[boss].attack_target_y == 18);
        g->enemies[boss].frozen_turns = 2;
        action_resolve_enemies(g);
        ASSERT("freezing Voss pauses his warning without retargeting", g->enemies[boss].move_timer == 2 && g->enemies[boss].frozen_turns == 1 && g->enemies[boss].attack_target_y == 18);
        g->enemies[boss].frozen_turns = 0;
    }
    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y;
    action_resolve_player(g, (Action){ACTION_MOVE, PIRATE_W - 1, 18});
    ASSERT("the living captain blocks the final departure", g->location == LOCATION_PIRATE_COAST);
    if (boss >= 0) {
        g->player.attack = 10000;
        g->player.defense = 10000;
        g->player.x = g->enemies[boss].x - 1;
        g->player.y = g->enemies[boss].y;
        action_resolve_player(g, (Action){ACTION_MOVE, g->enemies[boss].x, g->enemies[boss].y});
    }
    ASSERT("actual melee defeat unlocks the departure and grants the captain reward once", g->pirate_defeated && boss >= 0 && !g->enemies[boss].active);
    gold = g->gold;
    pirate_defeat(g);
    ASSERT("captain victory cannot repeat its reward", g->gold == gold);
    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y;
    action_resolve_player(g, (Action){ACTION_MOVE, PIRATE_W - 1, 18});
    ASSERT("victory returns to the landmark approach instead of directly to town", g->location == LOCATION_APPROACH && approach_route(g)->area == LOCATION_PIRATE_COAST);
    walk_return_approach(g);
    ASSERT("walking back enters Dawnmere beside its southern gate", g->location == LOCATION_DAWNMERE && g->player.x == 20 && g->player.y == TOWN_H - 2);
    pirate_enter(g);
    ASSERT("reentering always starts at stage one with persistent enemy wounds", g->level == 1 && g->enemies[0].hp == 17);
    pirate_leave(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 20, 12);
    g->gold = 731;
    ASSERT("version 183 characters gain a southern exit without losing existing progress", save_game(g, PIRATE_SLOT) && saved_version(183) && load_game(loaded, PIRATE_SLOT) && loaded->map.tiles[TOWN_H - 1][20] == TILE_TOWN_EXIT && loaded->gold == 731 && loaded->jail_quest_state == g->jail_quest_state && !loaded->pirate_defeated);
    town_road_enter(g, LOCATION_DAWNMERE, TOWN_SIDE_NORTH, 0);
    g->map.tiles[TOWN_SPRITE_Y + TOWN_SPRITE_H - 1][TOWN_ROAD_W / 2] = TILE_WALL;
    g->map.tiles[TOWN_ROAD_H - 1][TOWN_ROAD_W / 2] = TILE_WALL;
    ASSERT("old Dawnmere bypass saves gain both ends of the southern connection", save_game(g, PIRATE_SLOT) && saved_version(183) && load_game(loaded, PIRATE_SLOT) && loaded->map.tiles[TOWN_SPRITE_Y + TOWN_SPRITE_H - 1][TOWN_ROAD_W / 2] == TILE_TOWN_ROAD_ENTRY_SOUTH && loaded->map.tiles[TOWN_ROAD_H - 1][TOWN_ROAD_W / 2] == TILE_TOWN_ROAD_EXIT_SOUTH && loaded->player.x == g->player.x && loaded->player.y == g->player.y);
    remove("saves/savegame_99284.json");
    free(g); free(loaded); free(map);
}
