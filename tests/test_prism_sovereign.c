#include "test_utils.h"
#include "../src/game/prism_sovereign.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = LOCATION_GLASSDEEP;
    game.level = GLASSDEEP_DEPTH;
    game.max_glassdeep_level_reached = GLASSDEEP_DEPTH;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            game.map.tiles[y][x] = TILE_GLASSDEEP_WALL;
        }
    }
    game.map.room_count = 1;
    game.map.rooms[0] = (Room){20, 15, 16, 14};
    for (int y = 15; y < 29; y++) {
        for (int x = 20; x < 36; x++) {
            game.map.tiles[y][x] = TILE_GLASSDEEP_FLOOR;
        }
    }
    game.player.x = 27;
    game.player.y = 22;
    game.player.hp = 1000;
    game.player.max_hp = 1000;
    game.player.mp = 100;
    game.player.max_mp = 100;
    game.player.defense = 0;
    game.inventory_count = 0;
    game.equipped_main_hand = -1;
    game.equipped_off_hand = -1;
    game.equipped_armor = -1;
    game.floor_item_count = 0;
    game.enemy_count = 1;
    game.enemies[0] = (Enemy){.type = ENEMY_PRISM_SOVEREIGN, .name = "Prism Sovereign", .x = 28, .y = 22,
        .active = 1, .is_boss = 1, .hp = 275, .max_hp = 275, .attack = 24, .defense = 8,
        .experience = 800, .attack_target_x = -1, .attack_target_y = -1, .boss_aux_x = -1, .boss_aux_y = -1};
    prism_sovereign_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    return &game.enemies[0];
}

static int find_tile(TileType tile, int *x, int *y) {
    for (int row = 0; row < MAP_H; row++) {
        for (int col = 0; col < MAP_W; col++) {
            TileType actual = game.map.tiles[row][col];
            for (int i = 0; actual == TILE_ITEM && i < game.floor_item_count; i++) {
                const FloorItem *item = &game.floor_items[i];
                if (item->active && item->x == col && item->y == row) {
                    actual = (TileType)item->underlying_tile;
                }
            }
            if (actual == tile) {
                *x = col;
                *y = row;
                return 1;
            }
        }
    }
    return 0;
}

static void kill_with(int method) {
    Enemy *e = setup();
    e->hp = 1;
    game.player.attack = 100;
    if (method == 0) {
        game.player.x = e->x - 1;
        game.player.y = e->y;
        action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    } else {
        game.player.x = e->x - 2;
        game.player.y = e->y;
        game.player.last_dx = 1;
        game.player.last_dy = 0;
        if (method == 1) {
            game.player.player_class = CLASS_ROGUE;
            game.player.arrows = 10;
            game.inventory_count = 1;
            game.inventory[0] = item_make_bow();
            game.equipped_main_hand = 0;
            action_resolve_player(&game, (Action){ACTION_RANGED_ATTACK, 0, 0});
        } else {
            game.player.player_class = CLASS_MAGE;
            game.player.known_spells[0] = method == 2 ? spell_make_magic_arrow() : method == 3 ? spell_make_fireball() : spell_make_frost_bolt();
            game.player.known_spell_count = 1;
            game.player.equipped_spell = 0;
            action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
        }
    }
}

static int make_legacy_save(void) {
    if (!save_game(&game, 99195)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99195.json", "r");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *buffer = malloc((size_t)length + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    size_t read = fread(buffer, 1, (size_t)length, file);
    buffer[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 133);
    cJSON_DeleteItemFromObject(root, "prism_sovereign_reward_pending");
    cJSON *arrays[2] = {cJSON_GetObjectItem(root, "enemies"),
        cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "glassdeep_cache"), GLASSDEEP_DEPTH - 1), "enemies")};
    for (int group = 0; group < 2; group++) {
        for (int i = 0; i < cJSON_GetArraySize(arrays[group]); i++) {
            cJSON *enemy = cJSON_GetArrayItem(arrays[group], i);
            cJSON_DeleteItemFromObject(enemy, "boss_attack");
            cJSON_DeleteItemFromObject(enemy, "boss_wave_used");
            cJSON_DeleteItemFromObject(enemy, "boss_aux_x");
            cJSON_DeleteItemFromObject(enemy, "boss_aux_y");
        }
    }
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99195.json", "w");
    int ok = file && json;
    if (ok) {
        fputs(json, file);
    }
    if (file) {
        fclose(file);
    }
    free(json);
    return ok;
}

static void cache_arena(void) {
    LevelCache *cache = &game.glassdeep_cache[GLASSDEEP_DEPTH - 1];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = game.enemy_count;
    memcpy(cache->enemies, game.enemies, sizeof(Enemy) * (size_t)game.enemy_count);
}

static void test_rewards(void) {
    int victories = 1;
    for (int method = 0; method < 5; method++) {
        kill_with(method);
        int x;
        int y;
        victories &= !game.enemies[0].active && game.prism_sovereign_reward_pending == 3 && find_tile(TILE_PRISM_RELIQUARY, &x, &y) && (game.defeated_bosses & (1 << LOCATION_GLASSDEEP));
    }
    ASSERT("melee, bow and all three attack spells each reserve one robes and one Strength Potion", victories);
    Enemy *e = setup();
    e->hp = 1;
    game.player.attack = 100;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 20, .y = 15, .underlying_tile = TILE_GLASSDEEP_FLOOR, .item = item_make_health_potion()};
    }
    game.floor_item_count = MAX_FLOOR_ITEMS;
    game.map.tiles[15][20] = TILE_ITEM;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        game.inventory[i] = item_make_health_potion();
    }
    game.inventory_count = MAX_INVENTORY;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    int x;
    int y;
    int cache = find_tile(TILE_PRISM_RELIQUARY, &x, &y);
    ASSERT("full floor and inventory preserve every object and both guaranteed rewards", cache && game.floor_item_count == MAX_FLOOR_ITEMS && game.floor_items[MAX_FLOOR_ITEMS - 1].item.type == ITEM_POTION_HEALTH && game.prism_sovereign_reward_pending == 3 && game.gold > 0);
    game.player.x = x;
    game.player.y = y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("a full inventory cannot consume either reserved reward", game.prism_sovereign_reward_pending == 3 && game.map.tiles[y][x] == TILE_PRISM_RELIQUARY);
    game.inventory_count--;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("one free slot claims the robes while the Strength Potion stays reserved", game.prism_sovereign_reward_pending == PRISM_POTION_PENDING && strcmp(game.inventory[MAX_INVENTORY - 1].name, "Prismweave Robes") == 0 && game.map.tiles[y][x] == TILE_PRISM_RELIQUARY);
    int ok = save_game(&game, 99195) && load_game(&loaded, 99195);
    ASSERT("saving a partially claimed cache preserves the remaining potion without another robes", ok && loaded.prism_sovereign_reward_pending == PRISM_POTION_PENDING && loaded.map.tiles[y][x] == TILE_PRISM_RELIQUARY && strcmp(loaded.inventory[MAX_INVENTORY - 1].name, "Prismweave Robes") == 0);
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_PRISM_RELIQUARY, .item = item_make_health_potion()};
    }
    game.map.tiles[y][x] = TILE_ITEM;
    game.inventory_count--;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("pickup reaches a covered cache, claims the potion and preserves both loot underlays", !game.prism_sovereign_reward_pending && game.inventory[MAX_INVENTORY - 1].type == ITEM_POTION_STRENGTH && game.floor_items[0].underlying_tile == TILE_GLASSDEEP_FLOOR && game.floor_items[1].underlying_tile == TILE_GLASSDEEP_FLOOR && game.floor_item_count == MAX_FLOOR_ITEMS && game.map.tiles[y][x] == TILE_ITEM);
    Item robes = item_make_prismweave_robes();
    ASSERT("Prismweave Robes are Mage-only armor with +3 defense, +35 MP and 12 percent spell-cost reduction", item_class_allowed(&robes, CLASS_MAGE) && !item_class_allowed(&robes, CLASS_WARRIOR) && !item_class_allowed(&robes, CLASS_ROGUE) && robes.type == ITEM_ARMOR && robes.armor_family == ARMOR_FAMILY_ROBE && robes.defense_bonus == 3 && robes.max_mp_bonus == 35 && robes.spell_cost_reduction_percent == 12 && robes.visual_id == ITEM_VISUAL_PRISMWEAVE_ROBES);
    kill_with(0);
    find_tile(TILE_PRISM_RELIQUARY, &x, &y);
    game.player.x = x;
    game.player.y = y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("opening spills both rewards without claiming them", game.inventory_count == 0);
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("a spacious pack collects both released rewards through Pick Up", !game.prism_sovereign_reward_pending && game.inventory_count == 2 && strcmp(game.inventory[0].name, "Prismweave Robes") == 0 && game.inventory[1].type == ITEM_POTION_STRENGTH);
    int attack = game.player.attack;
    action_resolve_player(&game, (Action){ACTION_USE_ITEM, 1, 0});
    ASSERT("the original special Strength Potion still grants one permanent base attack", game.player.attack == attack + 1);
    game.inventory_count = 0;
    game_open_town_portal(&game);
    game_use_town_portal(&game);
    ok = save_game(&game, 99195) && load_game(&loaded, 99195);
    ASSERT("consumed, sold or discarded rewards cannot regenerate after travel or saving", ok && !loaded.prism_sovereign_reward_pending && loaded.floor_item_count == 0 && !loaded.enemies[0].active);
}

static void test_beams(void) {
    Enemy *e = setup();
    EnemyProjectiles shots = {0};
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("Focused Light keeps the eight-cell cardinal beam with one full warning action", e->boss_attack == PRISM_BEAM && e->move_timer == 1 && e->facing_dx == -1 && e->attack_target_x == 20 && e->attack_target_y == 22 && !shots.count && game.player.hp == 1000);
    int tx = e->attack_target_x;
    e->hp = 137;
    e->frozen_turns = 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze and crossing half health preserve a pending Focused Light beam", e->move_timer == 1 && !e->attack_phase && e->attack_target_x == tx);
    action_resolve_player(&game, (Action){ACTION_MOVE, 27, 23});
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("ordinary sidestepping dodges the unchanged fixed beam and enters recovery", game.player.hp == 1000 && e->move_timer == -1 && shots.count == 1 && shots.shots[0].target_x == tx);
    action_resolve_enemies(&game);
    ASSERT("the whole following counterattack action has no Sovereign retaliation", e->move_timer == 0 && game.player.hp == 1000);
    action_resolve_enemies(&game);
    PrismSegment segments[4];
    ASSERT("Fractured Crown marks both incoming and reflected split legs for two actions", e->attack_phase == 1 && e->boss_attack == PRISM_SPLIT && e->move_timer == 2 && prism_sovereign_segments(&game, e, segments) == 4);
    int ax = e->attack_target_x;
    int ay = e->attack_target_y;
    int bx = e->boss_aux_x;
    int by = e->boss_aux_y;
    e->frozen_turns = 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("frozen split beams retain both bounded endpoints and their full countdown", e->move_timer == 2 && e->attack_target_x == ax && e->attack_target_y == ay && e->boss_aux_x == bx && e->boss_aux_y == by);
    game.player.x = 28;
    game.player.y = 20;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("crystal rotation locks throughout a warning including while frozen", game.map.tiles[20][28] == TILE_PRISM_LEFT && e->move_timer == 2 && e->attack_target_x == ax);
    game.player.x = 27;
    game.player.y = 22;
    action_resolve_enemies(&game);
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("split resolution emits every marked segment once and leaves recovery", e->move_timer == -1 && shots.count == 4 && game.player.hp == 1000);
    e = setup();
    game.player.defense = 4;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("standing in the beam takes one base-strength hit using half defense", game.player.hp == 978 && e->move_timer == -1);
    e = setup();
    game.inventory[0] = item_make_shadow_armor();
    game.inventory[0].evasion_chance = 100;
    game.inventory_count = 1;
    game.equipped_armor = 0;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("prismatic signature damage retains equipped evasion", game.player.hp == 1000);
    e = setup();
    game.player.x = 24;
    game.map.tiles[22][26] = TILE_GLASSDEEP_POOL;
    action_resolve_enemies(&game);
    ASSERT("a blocker clips the terminal cell when the warning is declared", e->attack_target_x == 27);
    game.map.tiles[22][26] = TILE_GLASSDEEP_FLOOR;
    action_resolve_enemies(&game);
    ASSERT("removing cover cannot extend a beam into previously unmarked cells", game.player.hp == 1000 && e->move_timer == -1);
    e = setup();
    game.map.tiles[21][27] = TILE_GLASSDEEP_WALL;
    game.map.tiles[23][27] = TILE_GLASSDEEP_WALL;
    game.map.tiles[22][26] = TILE_GLASSDEEP_WALL;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("a player boxed in by terrain and actors receives an empty beam", game.player.hp == 1000 && e->attack_target_x == -1);
}

static void test_reflectors(void) {
    int geometry = 1;
    for (int mode = TILE_PRISM_LEFT; mode <= TILE_PRISM_RETURN; mode++) {
        Enemy *boss = setup();
        game.map.tiles[20][28] = (TileType)mode;
        game.player.x = 28;
        game.player.y = 20;
        action_resolve_enemies(&game);
        PrismSegment route[4];
        int count = prism_sovereign_segments(&game, boss, route);
        int ex = mode == TILE_PRISM_LEFT ? 22 : mode == TILE_PRISM_RIGHT ? 34 : 28;
        int ey = mode == TILE_PRISM_RETURN ? 22 : 20;
        geometry &= count == 2 && route[0].x1 == 28 && route[0].y1 == 22 && route[0].x2 == 28 && route[0].y2 == 20 && route[1].x2 == ex && route[1].y2 == ey;
        action_resolve_player(&game, (Action){ACTION_MOVE, 28, 19});
        EnemyProjectiles shots = {0};
        action_resolve_enemies_with_projectiles(&game, &shots);
        geometry &= shots.count == 2 && shots.shots[1].target_x == ex && shots.shots[1].target_y == ey && game.player.hp == 1000 && boss->move_timer == (mode == TILE_PRISM_RETURN ? -2 : -1);
    }
    ASSERT("LEFT, RIGHT and RETURN each warn and fire the same deterministic incoming leg and endpoint", geometry);
    int all_classes = 1;
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        Enemy *e = setup();
        game.player.player_class = cls;
        e->move_timer = -1;
        game.player.x = 28;
        game.player.y = 20;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        action_resolve_enemies(&game);
        all_classes &= game.map.tiles[20][28] == TILE_PRISM_RIGHT && e->move_timer == 0;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        all_classes &= game.map.tiles[20][28] == TILE_PRISM_RETURN;
        action_resolve_enemies(&game);
        all_classes &= e->move_timer == 1 && e->attack_target_x == e->x && e->attack_target_y == e->y;
        action_resolve_player(&game, (Action){ACTION_MOVE, 27, 20});
        action_resolve_enemies(&game);
        all_classes &= e->move_timer == -2 && game.player.hp == 1000;
        action_resolve_enemies(&game);
        all_classes &= e->move_timer == -1 && game.player.hp == 1000;
        action_resolve_enemies(&game);
        all_classes &= e->move_timer == 0 && game.player.hp == 1000;
    }
    ASSERT("all three classes can rotate a face to RETURN and receive two full stagger counterattacks", all_classes);
    Enemy *e = setup();
    e->move_timer = -1;
    game.player.x = 28;
    game.player.y = 20;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("rotation during recovery consumes its normal action and prepares the next face", game.map.tiles[20][28] == TILE_PRISM_RIGHT && e->move_timer == 0);
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("a second rotation may prepare RETURN before the following warning locks it", game.map.tiles[20][28] == TILE_PRISM_RETURN && e->move_timer == 1);
    e = setup();
    game.player.x = 28;
    game.player.y = 19;
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_SHARD_GOLEM, .x = 28, .y = 21, .hp = 100, .max_hp = 100, .frozen_turns = 5};
    game.enemy_count = 2;
    action_resolve_enemies(&game);
    game.enemies[1].active = 0;
    action_resolve_enemies(&game);
    ASSERT("removing a living blocker cannot invent a previously unannounced reflection", game.player.hp == 1000 && e->move_timer == -1);
    e = setup();
    game.map.tiles[20][28] = TILE_PRISM_RETURN;
    game.player.x = 28;
    game.player.y = 20;
    action_resolve_enemies(&game);
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_SHARD_GOLEM, .x = 28, .y = 21, .hp = 100, .max_hp = 100, .frozen_turns = 5};
    game.enemy_count = 2;
    game.player.x = 27;
    action_resolve_enemies(&game);
    ASSERT("a newly blocking actor stops the beam and prevents an unearned return stagger", e->move_timer == -1 && game.player.hp == 1000);
    e = setup();
    game.player.x = 28;
    game.player.y = 20;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 28, .y = 20, .underlying_tile = TILE_PRISM_LEFT, .item = item_make_health_potion()};
    game.floor_items[1] = game.floor_items[0];
    game.floor_item_count = 2;
    game.map.tiles[20][28] = TILE_ITEM;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("loot-covered crystal faces rotate while preserving all item underlays", game.floor_item_count == 2 && game.map.tiles[20][28] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_PRISM_RIGHT && game.floor_items[1].underlying_tile == TILE_PRISM_RIGHT);
    e = setup();
    e->hp = 137;
    game.map.tiles[20][28] = TILE_PRISM_RETURN;
    action_resolve_enemies(&game);
    PrismSegment segments[4];
    int count = prism_sovereign_segments(&game, e, segments);
    int back = 0;
    for (int n = 0; n < count; n++) {
        back |= segments[n].x2 == e->x && segments[n].y2 == e->y;
    }
    ASSERT("the full split-beam warning includes its visible return endpoint at the crown", count == 4 && back);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("a split-beam return grants the same two-action stagger without changing boss health", e->move_timer == -2 && e->hp == 137 && game.player.hp == 1000);
}

static unsigned char visited[MAP_H][MAP_W];
static int queue[MAP_W * MAP_H];

static int escape_steps(const Enemy *e) {
    memset(visited, 0, sizeof(visited));
    int head = 0;
    int tail = 0;
    queue[tail++] = game.player.y * MAP_W + game.player.x;
    visited[game.player.y][game.player.x] = 1;
    static const int steps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    while (head < tail) {
        int x = queue[head] % MAP_W;
        int y = queue[head++] / MAP_W;
        if (!prism_sovereign_attack_marks(&game, e, x, y)) {
            return visited[y][x] - 1;
        }
        for (int n = 0; n < 4; n++) {
            int nx = x + steps[n][0];
            int ny = y + steps[n][1];
            if (!map_is_walkable(&game.map, nx, ny) || visited[ny][nx] || (nx == e->x && ny == e->y)) {
                continue;
            }
            visited[ny][nx] = visited[y][x] + 1;
            queue[tail++] = ny * MAP_W + nx;
        }
    }
    return MAP_W * MAP_H;
}

static void test_layouts(void) {
    int props = 1;
    int escapes = 1;
    int paths = 1;
    for (int seed = 1; seed <= 100; seed++) {
        setup();
        srand((unsigned int)seed);
        map_generate_glassdeep(&game.map, GLASSDEEP_DEPTH);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        enemies_spawn(&game);
        Enemy *e = &game.enemies[0];
        game.enemy_count = 1;
        const Room *room = &game.map.rooms[game.map.room_count - 1];
        int reflectors = 0;
        for (int y = room->y; y < room->y + room->h; y++) {
            for (int x = room->x; x < room->x + room->w; x++) {
                reflectors += game.map.tiles[y][x] == TILE_PRISM_LEFT;
                if (!map_is_walkable(&game.map, x, y) || (x == e->x && y == e->y)) {
                    continue;
                }
                for (int attack = PRISM_BEAM; attack <= PRISM_SPLIT; attack++) {
                    e->boss_attack = 0;
                    e->hp = attack == PRISM_SPLIT ? e->max_hp / 2 : e->max_hp;
                    e->attack_phase = attack == PRISM_SPLIT;
                    e->move_timer = 0;
                    game.player.x = x;
                    game.player.y = y;
                    prism_sovereign_tick(&game, e, NULL);
                    escapes &= escape_steps(e) <= e->move_timer;
                }
            }
        }
        props &= reflectors == 0 && game.map.tiles[51][39] == TILE_STONEWAKE_CRANE_NEW;
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        e->move_timer = 0;
        memset(visited, 0, sizeof(visited));
        int head = 0;
        int tail = 0;
        queue[tail++] = game.player.y * MAP_W + game.player.x;
        visited[game.player.y][game.player.x] = 1;
        while (head < tail) {
            int x = queue[head] % MAP_W;
            int y = queue[head++] / MAP_W;
            static const int steps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            for (int n = 0; n < 4; n++) {
                int nx = x + steps[n][0];
                int ny = y + steps[n][1];
                if (map_is_walkable(&game.map, nx, ny) && !visited[ny][nx]) {
                    visited[ny][nx] = 1;
                    queue[tail++] = ny * MAP_W + nx;
                }
            }
        }
        paths &= visited[game.map.stairs_down_y][game.map.stairs_down_x] && visited[e->y][e->x];
    }
    ASSERT("100 native quarry layouts have a crane beside the arena and no retired prism reflectors", props);
    ASSERT("every arena floor in 100 layouts has a walking escape within its declared warning", escapes);
    ASSERT("all 100 reflector layouts preserve the entrance-to-boss-to-exit route", paths);
}

static void test_robes(void) {
    setup();
    game.player.player_class = CLASS_MAGE;
    game_init(&game);
    game.inventory_count = 1;
    game.inventory[0] = item_make_prismweave_robes();
    game.equipped_main_hand = -1;
    game.equipped_off_hand = -1;
    game.equipped_armor = -1;
    int base_mp = game.player.max_mp;
    int base_defense = game.player.defense;
    action_resolve_player(&game, (Action){ACTION_EQUIP_ITEM, 0, 0});
    ASSERT("equipping Prismweave Robes actually adds +3 defense and +35 maximum MP", game.equipped_armor == 0 && game.player.max_mp == base_mp + 35 && game.player.defense == base_defense + 3);
    game.player.known_spell_count = 1;
    game.player.known_spells[0] = spell_make_heal();
    game.player.equipped_spell = 0;
    game.player.hp = 1;
    game.player.mp = game.player.max_mp;
    game.player.last_dx = 1;
    game.player.last_dy = 0;
    int before = game.player.mp;
    action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
    int cost = game.player.known_spells[0].mp_cost * 88 / 100;
    ASSERT("Prismweave Robes apply their real spell-cost reduction to casting", game.player.mp == before - cost);
    int ok = save_game(&game, 99195) && load_game(&loaded, 99195);
    ASSERT("equipped robe metadata, maximum MP and mana cost modifier survive saving", ok && loaded.equipped_armor == 0 && loaded.inventory[0].max_mp_bonus == 35 && loaded.inventory[0].spell_cost_reduction_percent == 12 && loaded.inventory[0].visual_id == ITEM_VISUAL_PRISMWEAVE_ROBES && loaded.player.max_mp == base_mp + 35);
    Item top = item_make_archmage_robes();
    Item robes = item_make_prismweave_robes();
    ASSERT("Archmage Robes retain higher defense, mana and cost reduction", top.defense_bonus > robes.defense_bonus && top.max_mp_bonus > robes.max_mp_bonus && top.spell_cost_reduction_percent > robes.spell_cost_reduction_percent);
}

static void test_saves(void) {
    Enemy *e = setup();
    e->hp = 137;
    game.map.tiles[20][28] = TILE_PRISM_RETURN;
    action_resolve_enemies(&game);
    e->frozen_turns = 2;
    cache_arena();
    int ok = save_game(&game, 99195) && load_game(&loaded, 99195);
    ASSERT("active and cached saves preserve split directions, both endpoints, phase and frozen warning", ok && loaded.enemies[0].boss_attack == PRISM_SPLIT && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].attack_phase == 1 && loaded.enemies[0].frozen_turns == 2 && loaded.enemies[0].attack_target_x == e->attack_target_x && loaded.enemies[0].boss_aux_x == e->boss_aux_x && loaded.enemies[0].boss_aux_y == e->boss_aux_y && loaded.glassdeep_cache[4].enemies[0].facing_dy == e->facing_dy && loaded.map.tiles[20][28] == TILE_PRISM_RETURN);
    e->frozen_turns = 0;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    cache_arena();
    ok = save_game(&game, 99195) && load_game(&loaded, 99195);
    ASSERT("the two-action reflected stagger survives active and cached saves", ok && loaded.enemies[0].move_timer == -2 && loaded.glassdeep_cache[4].enemies[0].move_timer == -2 && loaded.enemies[0].hp == 137);
    e = setup();
    action_resolve_enemies(&game);
    game_open_town_portal(&game);
    ok = save_game(&game, 99195) && load_game(&loaded, 99195);
    if (ok) {
        game = loaded;
        game_use_town_portal(&game);
    }
    ASSERT("return portals ward a threatening saved beam without altering boss health", ok && game.location == LOCATION_GLASSDEEP && game.enemies[0].move_timer == -1 && game.enemies[0].attack_target_x == -1 && game.enemies[0].hp == 275 && game.player.hp == 1000);
    e = setup();
    game.map.tiles[22][28] = TILE_GLASSDEEP_RUIN;
    e->hp = 1;
    game.player.attack = 100;
    action_resolve_player(&game, (Action){ACTION_MOVE, 28, 22});
    ASSERT("rewards on native sanctuary paving use the paved cache variant", game.cinder_lord_reward_pending == 0 && game.prism_sovereign_reward_pending == 3 && game.floor_items[0].underlying_tile == TILE_PRISM_PAVED_RELIQUARY);
    game.player.x = 28;
    game.player.y = 22;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("claiming rewards after collecting loose gold preserves the original sanctuary paving", !game.prism_sovereign_reward_pending && game.map.tiles[22][28] == TILE_GLASSDEEP_RUIN);
    e = setup();
    e->hp = 73;
    e->move_timer = 17;
    e->frozen_turns = 3;
    game.enemies[1] = (Enemy){.type = ENEMY_SHARD_GOLEM, .name = "Shard Golem", .active = 0, .hp = 0, .max_hp = 110, .x = 24, .y = 24};
    game.enemy_count = 2;
    game.gold = 321;
    game.glassdeep_quest_state = 2;
    game.glassdeep_quest_progress = 7;
    game.glassdeep_quest_encounters = 7;
    game.map.tiles[18][23] = TILE_GLASSDEEP_RESONATOR_LIT;
    game.map.tiles[20][23] = TILE_GLASSDEEP_RUIN;
    game.portal_active = 1;
    game.portal_location = LOCATION_GLASSDEEP;
    game.portal_level = GLASSDEEP_DEPTH;
    game.portal_x = 28;
    game.portal_y = 20;
    game.portal_origin_tile = TILE_GLASSDEEP_RUIN;
    for (int y = 15; y < 29; y++) {
        for (int x = 20; x < 36; x++) {
            if (game.map.tiles[y][x] >= TILE_PRISM_LEFT && game.map.tiles[y][x] <= TILE_PRISM_RETURN) {
                game.map.tiles[y][x] = TILE_GLASSDEEP_RUIN;
            }
        }
    }
    map_mark_explored(&game.map, 24, 24);
    cache_arena();
    ok = make_legacy_save() && load_game(&loaded, 99195);
    ASSERT("version-133 migration resets obsolete beam timing but preserves damage, freeze and deaths", ok && loaded.enemies[0].hp == 73 && loaded.enemies[0].move_timer == 0 && loaded.enemies[0].attack_target_x == -1 && loaded.enemies[0].frozen_turns == 3 && !loaded.enemies[1].active && loaded.glassdeep_cache[4].enemies[0].hp == 73 && !loaded.glassdeep_cache[4].enemies[1].active);
    ASSERT("migration preserves resonator progress, lit crystals, paving, exploration, gold and the paved portal destination", ok && loaded.glassdeep_quest_state == 2 && loaded.glassdeep_quest_progress == 7 && loaded.glassdeep_quest_encounters == 7 && loaded.map.tiles[18][23] == TILE_GLASSDEEP_RESONATOR_LIT && loaded.map.tiles[20][23] == TILE_GLASSDEEP_RUIN && map_is_explored(&loaded.map, 24, 24) && loaded.gold == 321 && loaded.map.tiles[20][28] == TILE_GLASSDEEP_RUIN && loaded.glassdeep_cache[4].map.tiles[20][28] == TILE_GLASSDEEP_RUIN);
    setup();
    game.enemies[0].active = 0;
    game.defeated_bosses |= 1 << LOCATION_GLASSDEEP;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 28, .y = 22, .underlying_tile = TILE_GLASSDEEP_FLOOR, .item = item_make_strength_potion()};
    game.floor_item_count = 1;
    game.map.tiles[22][28] = TILE_ITEM;
    cache_arena();
    ok = make_legacy_save() && load_game(&loaded, 99195);
    ASSERT("old victories retain original potion drops without manufacturing robes or a second potion", ok && !loaded.prism_sovereign_reward_pending && !loaded.enemies[0].active && !loaded.glassdeep_cache[4].enemies[0].active && loaded.floor_item_count == 1 && loaded.floor_items[0].item.type == ITEM_POTION_STRENGTH);
}

void test_prism_sovereign(void) {
    printf("Prism Sovereign encounter tests:\n");
    test_beams();
    test_reflectors();
    test_rewards();
    test_robes();
    test_layouts();
    test_saves();
    remove("saves/savegame_99195.json");
}
