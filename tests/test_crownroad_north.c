#include "expedition_fixture.h"
#include "test_utils.h"
#include "game/crownroad_north.h"
#include "game/elderwood.h"
#include "game/approach.h"
#include "game/town_life.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define NORTH_SLOT 99320

static void start(GameState *g) {
    game_init(g);
    g->elderwood_defeated = 1;
    g->defeated_bosses |= 1 << LOCATION_CATACOMBS;
    northroad_enter(g, 0, 0);
}

static void test_layout(GameState *g) {
    start(g);
    unsigned char reached[CROWN_NORTH_H][CROWN_NORTH_W] = {{0}};
    int queue[CROWN_NORTH_W * CROWN_NORTH_H];
    int head = 0;
    int tail = 1;
    queue[0] = (CROWN_NORTH_H - 1) * CROWN_NORTH_W + CROWN_NORTH_X;
    reached[CROWN_NORTH_H - 1][CROWN_NORTH_X] = 1;
    const int steps[4][2] = {{0,1},{1,0},{0,-1},{-1,0}};
    while (head < tail) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % CROWN_NORTH_W + steps[i][0];
            int y = cell / CROWN_NORTH_W + steps[i][1];
            if (x >= 0 && y >= 0 && x < CROWN_NORTH_W && y < CROWN_NORTH_H &&
                !reached[y][x] && map_is_walkable(&g->map, x, y)) {
                reached[y][x] = 1;
                queue[tail++] = y * CROWN_NORTH_W + x;
            }
        }
    }
    int connected = reached[0][CROWN_NORTH_X];
    int wolves = 0;
    int archers = 0;
    int shamans = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *enemy = &g->enemies[i];
        connected &= reached[enemy->y][enemy->x];
        wolves += enemy->type == ENEMY_BLIGHTED_WOLF;
        archers += enemy->type == ENEMY_GOBLIN_ARCHER;
        shamans += enemy->type == ENEMY_GOBLIN_SHAMAN;
    }
    int caches = 0;
    int narrow = 1;
    for (int y = 1; y < CROWN_NORTH_H - 1; y++) {
        for (int x = 1; x < CROWN_NORTH_W - 1; x++) {
            if (g->map.tiles[y][x] == TILE_CROWN_SUPPLIES) {
                caches++;
                connected &= reached[y - 1][x] || reached[y + 1][x] || reached[y][x - 1] || reached[y][x + 1];
            }
            if (g->map.tiles[y][x] == TILE_TOWN_PATH && g->map.tiles[y + 1][x] == TILE_TOWN_PATH &&
                g->map.tiles[y][x + 1] == TILE_TOWN_PATH && g->map.tiles[y + 1][x + 1] == TILE_TOWN_PATH) {
                narrow = 0;
            }
        }
    }
    ASSERT("Crown Road North connects both boundary exits, every patrol and all six side caches", connected && caches == 6);
    ASSERT("six goblin formations include 24 wolves, 12 archers and six shamans at normal sprite types", g->enemy_count == NORTH_ENEMIES && wolves == 24 && archers == 12 && shamans == 6);
    ASSERT("the main paved road stays one tile wide and actors never overlap or stand on blocked terrain", narrow && northroad_valid(g));
    g->player.x = CROWN_NORTH_X;
    g->player.y = 7;
    action_resolve_player(g, (Action){ACTION_DESCEND, 0, 0});
    game_descend(g);
    game_ascend(g);
    enemies_spawn(g);
    ASSERT("stairs controls and generic dungeon helpers preserve the outdoor road and its full patrol roster", g->location == LOCATION_CROWNROAD_NORTH && g->level == 1 && g->enemy_count == NORTH_ENEMIES);
    g->player.y = 2;
    g->player.last_dx = 0;
    g->player.last_dy = -1;
    g->player.known_spells[0] = spell_make_teleport();
    g->player.known_spell_count = 1;
    g->player.equipped_spell = 0;
    g->player.mp = 100;
    action_resolve_player(g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("Teleport stops inside the road instead of stranding the player on its boundary exit", g->location == LOCATION_CROWNROAD_NORTH && g->player.y == 1 && northroad_valid(g));
}

static Enemy actor(EnemyType type, int x, int y, int slot) {
    Enemy enemy = {.active = 1, .type = type, .x = x, .y = y, .hp = 100, .max_hp = 100,
        .attack = 20, .boss_aux_x = 0, .boss_aux_y = slot, .boss_wave_used = 1, .facing_dy = 1};
    snprintf(enemy.name, sizeof(enemy.name), "Patrol fixture");
    return enemy;
}

static void arena(GameState *g) {
    start(g);
    for (int y = 20; y < 49; y++) {
        for (int x = 15; x < 49; x++) {
            g->map.tiles[y][x] = TILE_FOREST_FLOOR;
        }
    }
    g->enemy_count = 0;
    g->player.x = 28;
    g->player.y = 35;
    g->player.hp = 1000;
    g->player.max_hp = 1000;
    g->player.attack = 30;
    g->player.defense = 0;
    g->equipped_main_hand = -1;
    g->equipped_off_hand = -1;
}

static void test_tactics(GameState *g) {
    arena(g);
    g->enemy_count = 3;
    g->enemies[0] = actor(ENEMY_HOBGOBLIN_GUARD, 28, 34, 0);
    g->enemies[1] = actor(ENEMY_GOBLIN_ARCHER, 28, 31, 7);
    g->enemies[2] = actor(ENEMY_BLIGHTED_WOLF, 44, 30, 11);
    g->enemies[0].boss_wave_used = 0;
    g->enemies[1].boss_wave_used = 0;
    g->enemies[2].boss_wave_used = 0;
    g->enemies[2].boss_aux_x = 1;
    int damage = 0;
    northroad_tick(g, &g->enemies[0], &damage);
    ASSERT("spotting the player alerts the local formation without waking distant squads", g->enemies[0].boss_wave_used && g->enemies[1].boss_wave_used && !g->enemies[2].boss_wave_used);
    action_resolve_player(g, (Action){ACTION_MOVE, 28, 34});
    ASSERT("supported shield bearers halve actual frontal melee hits", g->enemies[0].hp == 85);
    g->player.x = 27;
    g->player.y = 34;
    action_resolve_enemies(g);
    ASSERT("supported shield lines keep their facing through the enemy turn so flanking is possible", g->enemies[0].facing_dx == 0 && g->enemies[0].facing_dy == 1);
    action_resolve_player(g, (Action){ACTION_MOVE, 28, 34});
    ASSERT("a side approach bypasses the shield instead of inflating armor everywhere", g->enemies[0].hp == 55);
    g->enemies[0].frozen_turns = 1;
    ASSERT("freezing breaks the shield stance", !northroad_shielded(g, &g->enemies[0]));
    g->enemies[0].frozen_turns = 0;
    g->enemies[1].active = 0;
    ASSERT("removing ranged support breaks the guard formation", !northroad_shielded(g, &g->enemies[0]));

    arena(g);
    g->enemy_count = 1;
    g->enemies[0] = actor(ENEMY_GOBLIN_ARCHER, 28, 30, 7);
    northroad_tick(g, &g->enemies[0], &damage);
    ASSERT("archers warn a fixed target before their shot", !damage && northroad_attack_marks(g, &g->enemies[0], 28, 35));
    g->enemies[0].frozen_turns = 2;
    action_resolve_enemies(g);
    ASSERT("freeze pauses the northern archer's existing warning", g->enemies[0].attack_phase == 1 && g->enemies[0].move_timer == 1 && g->enemies[0].frozen_turns == 1);
    g->enemies[0].frozen_turns = 0;
    g->player.x++;
    northroad_tick(g, &g->enemies[0], &damage);
    ASSERT("sidestepping the committed arrow avoids damage and grants a recovery turn", !damage && !g->enemies[0].attack_phase && g->enemies[0].move_timer == -1);

    arena(g);
    g->enemy_count = 1;
    g->enemies[0] = actor(ENEMY_GOBLIN_BOMBER, 28, 30, 9);
    northroad_tick(g, &g->enemies[0], &damage);
    ASSERT("bombers announce a fixed cross-shaped blast for two player actions", !damage && g->enemies[0].move_timer == 2 &&
        northroad_attack_marks(g, &g->enemies[0], 28, 35) && northroad_attack_marks(g, &g->enemies[0], 29, 35) &&
        !northroad_attack_marks(g, &g->enemies[0], 29, 36));
    g->player.x += 2;
    northroad_tick(g, &g->enemies[0], &damage);
    northroad_tick(g, &g->enemies[0], &damage);
    ASSERT("moving twice escapes the bomb without its target tracking the player", !damage && !g->enemies[0].attack_phase);

    arena(g);
    g->enemy_count = 4;
    for (int i = 0; i < 4; i++) {
        g->enemies[i] = actor(ENEMY_BLIGHTED_WOLF, 22 + i * 4, i % 2 ? 42 : 26, 11 + i);
    }
    for (int turn = 0; turn < 12; turn++) {
        for (int i = 0; i < 4; i++) {
            northroad_tick(g, &g->enemies[i], &damage);
        }
    }
    int surrounds = 1;
    for (int i = 0; i < 4; i++) {
        surrounds &= abs(g->enemies[i].x - g->player.x) + abs(g->enemies[i].y - g->player.y) == 1;
        for (int j = 0; j < i; j++) {
            surrounds &= g->enemies[i].x != g->enemies[j].x || g->enemies[i].y != g->enemies[j].y;
        }
    }
    ASSERT("wolf packs reach four distinct flanks through ordinary walkable movement", surrounds);

    arena(g);
    g->enemy_count = 3;
    g->enemies[0] = actor(ENEMY_GOBLIN_SHAMAN, 28, 30, 10);
    g->enemies[0].move_timer = 3;
    g->enemies[1] = actor(ENEMY_HOBGOBLIN_GUARD, 29, 30, 0);
    g->enemies[1].hp = 85;
    g->enemies[2] = actor(ENEMY_GOBLIN_SCOUT, 27, 30, 4);
    g->enemies[2].active = 0;
    g->enemies[2].hp = 0;
    northroad_tick(g, &g->enemies[0], &damage);
    ASSERT("shamans heal wounded squadmates without reviving defeated enemies", g->enemies[1].hp == 95 && !g->enemies[2].active && g->enemies[2].hp == 0);
}

static int legacy_save(void) {
    FILE *file = fopen("saves/savegame_99320.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = calloc(size + 1, 1);
    fread(text, 1, size, file);
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 201);
    test_mark_legacy_portal(root);
    cJSON_DeleteItemFromObject(root, "northroad");
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99320.json", "wb");
    int ok = file && text && fputs(text, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(text);
    return ok;
}

static void test_travel_and_saves(GameState *g, GameState *loaded) {
    game_init(g);
    northroad_enter(g, 0, 0);
    ASSERT("Crown Road North has no independent boss gate; regional checkpoints control access", g->location == LOCATION_CROWNROAD_NORTH);
    g->debug_passages_unlocked = 1;
    northroad_enter(g, 0, 0);
    ASSERT("debug passage access opens Crown Road North without granting boss credit", g->location == LOCATION_CROWNROAD_NORTH && !g->defeated_bosses && !g->elderwood_defeated);
    start(g);
    g->enemies[0].hp = 7;
    g->enemies[0].frozen_turns = 2;
    g->enemies[7].attack_phase = 1;
    g->enemies[7].boss_attack = NORTH_ARROW;
    g->enemies[7].move_timer = 1;
    g->enemies[7].attack_target_x = g->player.x;
    g->enemies[7].attack_target_y = g->player.y;
    g->enemies[11].active = 0;
    g->enemies[11].hp = 0;
    g->map.tiles[82][18] = TILE_CROWN_EMPTY;
    map_mark_explored(&g->map, 18, 82);
    int px = g->player.x;
    int py = g->player.y;
    ASSERT("active road saves preserve wounds, freeze, dead wolves and warned targets", save_game(g, NORTH_SLOT) && load_game(loaded, NORTH_SLOT) &&
        loaded->location == LOCATION_CROWNROAD_NORTH && loaded->enemies[0].hp == 7 && loaded->enemies[0].frozen_turns == 2 &&
        !loaded->enemies[11].active && loaded->enemies[7].attack_phase == 1 && loaded->enemies[7].move_timer == 1);
    game_open_town_portal(g);
    ASSERT("Return to Town stores the northern battle and visits the last town", g->location == LOCATION_TOWN && g->portal_location == LOCATION_CROWNROAD_NORTH && !g->level_cache[0].valid);
    ASSERT("a northern portal and its cached formation survive town save/load", save_game(g, NORTH_SLOT) && load_game(loaded, NORTH_SLOT) && loaded->northroad.enemies[0].hp == 7);
    test_restore_cached_floor(loaded);
    ASSERT("archived floor reload resumes the exact road position, deaths, warning and explored cache", loaded->location == LOCATION_CROWNROAD_NORTH &&
        loaded->player.x == px && loaded->player.y == py && loaded->enemies[0].hp == 7 && !loaded->enemies[11].active &&
        !loaded->portal_active && loaded->enemies[7].attack_phase == 1 && loaded->map.tiles[82][18] == TILE_CROWN_EMPTY && map_is_explored(&loaded->map, 18, 82));
    loaded->player.x = CROWN_NORTH_X;
    loaded->player.y = 1;
    action_resolve_player(loaded, (Action){ACTION_MOVE, CROWN_NORTH_X, 0});
    ASSERT("walking onto the north boundary reaches Castle Grounds' south gate", loaded->location == LOCATION_CASTLE && loaded->player.y == TOWN_H - 2);
    action_resolve_player(loaded, (Action){ACTION_MOVE, CROWNROAD_X, TOWN_H - 1});
    ASSERT("the castle's south gate enters Crown Road North from its north boundary", loaded->location == LOCATION_CROWNROAD_NORTH && loaded->player.y == 1);
    ASSERT("physical re-entry respawns patrols while keeping explored terrain and claimed supplies", loaded->enemies[0].hp == loaded->enemies[0].max_hp &&
        loaded->enemies[11].active && !loaded->enemies[7].attack_phase && loaded->map.tiles[82][18] == TILE_CROWN_EMPTY && map_is_explored(&loaded->map, 18, 82));
    loaded->player.x = CROWN_NORTH_X;
    loaded->player.y = CROWN_NORTH_H - 2;
    action_resolve_player(loaded, (Action){ACTION_MOVE, CROWN_NORTH_X, CROWN_NORTH_H - 1});
    ASSERT("the south boundary returns to the road beside the Catacombs", northern_approach(loaded) && loaded->player.x == NORTH_ROAD_X && loaded->player.y == 1);
    action_resolve_player(loaded, (Action){ACTION_MOVE, NORTH_ROAD_X, 0});
    ASSERT("the cleared Catacombs approach enters the goblin road before the castle", loaded->location == LOCATION_CROWNROAD_NORTH && loaded->player.y == CROWN_NORTH_H - 2);
    int anchor_x = loaded->enemies[0].x;
    int anchor_y = loaded->enemies[0].y;
    int item_x = loaded->enemies[1].x;
    int item_y = loaded->enemies[1].y;
    loaded->portal_active = 1;
    loaded->portal_location = LOCATION_CROWNROAD_NORTH;
    loaded->portal_level = 1;
    loaded->portal_x = anchor_x;
    loaded->portal_y = anchor_y;
    loaded->floor_item_count = 1;
    loaded->floor_items[0] = (FloorItem){.active = 1, .x = item_x, .y = item_y,
        .underlying_tile = loaded->map.tiles[item_y][item_x], .item = item_make_staff()};
    loaded->map.tiles[item_y][item_x] = TILE_ITEM;
    northroad_leave(loaded, 1);
    northroad_enter(loaded, 1, 0);
    int protected = 1;
    for (int i = 0; i < loaded->enemy_count; i++) {
        const Enemy *enemy = &loaded->enemies[i];
        protected &= !(enemy->x == anchor_x && enemy->y == anchor_y) && !(enemy->x == item_x && enemy->y == item_y);
    }
    ASSERT("respawn relocates patrols around a live portal anchor and preserved floor gear", protected && loaded->enemy_count == NORTH_ENEMIES &&
        loaded->floor_items[0].active && loaded->map.tiles[item_y][item_x] == TILE_ITEM && northroad_valid(loaded));
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    const TownBuilding *merchant = town_life_building(g->location);
    town_life_enter(g, merchant->x + merchant->w / 2, merchant->y + merchant->h - 1);
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Merchant Corvin");
    g->gold = 321;
    ASSERT("legacy merchant dialogue migrates to Spice Merchant without changing progress", save_game(g, NORTH_SLOT) && legacy_save() && load_game(loaded, NORTH_SLOT) &&
        strcmp(loaded->dialogue_speaker, "Spice Merchant Corvin") == 0 && loaded->gold == 321 && !loaded->northroad.valid);
    const TownBuilding *building = town_life_building(LOCATION_DAWNMERE);
    ASSERT("Dawnmere's exterior label and resident use the spice merchant title", strcmp(building->label, "SPICE MERCHANT") == 0 && strcmp(building->speaker, "Spice Merchant Corvin") == 0);
}

static void test_supplies(GameState *g) {
    start(g);
    g->player.x = 18;
    g->player.y = 81;
    g->player.hp = g->player.max_hp - 40;
    g->enemy_count = 1;
    g->enemies[0] = actor(ENEMY_GOBLIN_SCOUT, 19, 82, 4);
    int gold = g->gold;
    ASSERT("side-trail supplies expose the configured interaction control", game_has_regional_interaction(g));
    northroad_interact(g);
    ASSERT("nearby living guards protect their cache", g->gold == gold && g->map.tiles[82][18] == TILE_CROWN_SUPPLIES);
    g->enemies[0].active = 0;
    g->enemies[0].hp = 0;
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("clearing the cache grants its gold and healing through the normal action", g->gold == gold + 25 &&
        g->player.hp == g->player.max_hp - 20 && g->map.tiles[82][18] == TILE_CROWN_EMPTY && northroad_has_interaction(g));
    northroad_leave(g, 1);
    northroad_enter(g, 1, 0);
    g->player.x = 18;
    g->player.y = 81;
    ASSERT("re-entering refreshes patrols without refilling a claimed supply cache", g->enemy_count == NORTH_ENEMIES &&
        northroad_interact(g) && g->gold == gold + 25 && g->map.tiles[82][18] == TILE_CROWN_EMPTY);
}

void test_crownroad_north(void) {
    printf("Crown Road North tests:\n");
    ASSERT("Crown Road North's temporary save slot is unused", !save_exists(NORTH_SLOT));
    if (save_exists(NORTH_SLOT)) {
        return;
    }
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    if (g && loaded) {
        test_layout(g);
        test_tactics(g);
        test_supplies(g);
        test_travel_and_saves(g, loaded);
    } else {
        ASSERT("Crown Road North fixtures allocate", 0);
    }
    remove("saves/savegame_99320.json");
    free(g);
    free(loaded);
}
