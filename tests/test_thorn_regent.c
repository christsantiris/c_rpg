#include "test_utils.h"
#include "../src/game/thorn_regent.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = LOCATION_MOONVEIL;
    game.level = MOONVEIL_DEPTH;
    game.max_moonveil_level_reached = MOONVEIL_DEPTH;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            game.map.tiles[y][x] = TILE_MOONVEIL_WALL;
        }
    }
    game.map.room_count = 1;
    game.map.rooms[0] = (Room){20, 15, 16, 14};
    for (int y = 15; y < 29; y++) {
        for (int x = 20; x < 36; x++) {
            game.map.tiles[y][x] = TILE_MOONVEIL_FLOOR;
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
    game.enemies[0] = (Enemy){.type = ENEMY_THORN_REGENT, .name = "Thorn Regent", .x = 28, .y = 22,
        .active = 1, .is_boss = 1, .hp = 240, .max_hp = 240, .attack = 23, .defense = 8,
        .experience = 700, .attack_target_x = -1, .attack_target_y = -1, .boss_aux_x = -1, .boss_aux_y = -1};
    thorn_regent_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
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
    if (!save_game(&game, 99191)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99191.json", "r");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 131);
    cJSON_DeleteItemFromObject(root, "thorn_regent_reward_pending");
    cJSON *arrays[2] = {cJSON_GetObjectItem(root, "enemies"),
        cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "moonveil_cache"), MOONVEIL_DEPTH - 1), "enemies")};
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
    file = fopen("saves/savegame_99191.json", "w");
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
    LevelCache *cache = &game.moonveil_cache[MOONVEIL_DEPTH - 1];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = game.enemy_count;
    memcpy(cache->enemies, game.enemies, sizeof(Enemy) * (size_t)game.enemy_count);
}

static void test_signatures(void) {
    Enemy *e = setup();
    EnemyProjectiles shots = {0};
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("an adjacent Regent prepares a fixed one-action lance without extra melee", e->boss_attack == REGENT_LANCE && e->move_timer == 1 && game.player.hp == 1000 && !shots.count);
    int tx = e->attack_target_x;
    int ty = e->attack_target_y;
    e->hp = 120;
    e->frozen_turns = 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze and crossing half health preserve the original phase-one warning", e->move_timer == 1 && !e->attack_phase && e->attack_target_x == tx && e->attack_target_y == ty);
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x, game.player.y + 1});
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("an ordinary sidestep escapes a lance whose projectile retains the locked endpoint", game.player.hp == 1000 && e->move_timer == -1 && shots.count == 1 && shots.shots[0].target_x == tx && shots.shots[0].target_y == ty);
    action_resolve_enemies(&game);
    ASSERT("the Regent leaves a whole counterattack action without retaliation", e->move_timer == 0 && game.player.hp == 1000);
    action_resolve_enemies(&game);
    ASSERT("Briar Crown is announced before the next fixed two-action root cross", e->attack_phase == 1 && e->boss_attack == REGENT_ROOTS && e->move_timer == 2);
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x - 1, game.player.y});
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x - 1, game.player.y});
    action_resolve_enemies(&game);
    int x;
    int y;
    ASSERT("two normal moves escape roots and leave finite R2 patches", game.player.hp == 1000 && e->move_timer == -1 && find_tile(TILE_REGENT_ROOT_2, &x, &y));
    action_resolve_enemies(&game);
    ASSERT("creeping roots count down to R1 during recovery", find_tile(TILE_REGENT_ROOT_1, &x, &y));
    action_resolve_enemies(&game);
    ASSERT("roots expire before separately warned non-damaging briar regrowth", !find_tile(TILE_REGENT_ROOT_1, &x, &y) && e->boss_attack == REGENT_REGROW && e->move_timer == 2);
    e = setup();
    game.player.defense = 4;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("standing in a lance takes one base attack using half defense", game.player.hp == 979 && e->move_timer == -1);
    e = setup();
    game.inventory[0] = item_make_shadow_armor();
    game.inventory[0].evasion_chance = 100;
    game.inventory_count = 1;
    game.equipped_armor = 0;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("signature attacks retain armor evasion", game.player.hp == 1000);
    e = setup();
    game.player.x = 24;
    game.map.tiles[22][26] = TILE_MOONVEIL_POOL;
    action_resolve_enemies(&game);
    game.map.tiles[22][26] = TILE_MOONVEIL_FLOOR;
    action_resolve_enemies(&game);
    ASSERT("removing cover cannot expose an unmarked target after declaration", game.player.hp == 1000 && e->attack_target_x == -1);
    e = setup();
    game.map.tiles[21][27] = TILE_MOONVEIL_WALL;
    game.map.tiles[23][27] = TILE_MOONVEIL_WALL;
    game.map.tiles[22][26] = TILE_MOONVEIL_WALL;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("boxed-in players receive an empty swing rather than unavoidable damage", game.player.hp == 1000 && e->attack_target_x == -1);
    e = setup();
    game.map.tiles[22][27] = TILE_ITEM;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 27, .y = 22, .underlying_tile = TILE_REGENT_ROOT_2, .item = item_make_health_potion()};
    }
    game.floor_item_count = 2;
    e->frozen_turns = 3;
    action_resolve_enemies(&game);
    ASSERT("root clocks tick beneath both stacked objects while the boss is frozen", game.player.hp == 992 && game.map.tiles[22][27] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_REGENT_ROOT_1 && game.floor_items[1].underlying_tile == TILE_REGENT_ROOT_1);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("root patches expire after two actions without losing loot or adding damage", game.player.hp == 984 && game.floor_item_count == 2 && game.floor_items[0].underlying_tile == TILE_MOONVEIL_FLOOR && game.floor_items[1].underlying_tile == TILE_MOONVEIL_FLOOR);
}

static Enemy *prepare_growth(void) {
    Enemy *e = setup();
    e->hp = 120;
    e->boss_attack = REGENT_ROOTS;
    action_resolve_enemies(&game);
    return e;
}

static void test_nodes(void) {
    Enemy *e = prepare_growth();
    int tx = e->attack_target_x;
    int ty = e->attack_target_y;
    int mask = e->boss_aux_x;
    ASSERT("Briar Crown warns a fixed three-cell growth strip for two actions", e->boss_attack == REGENT_REGROW && e->move_timer == 2 && mask == 7 && ty == 18 && thorn_regent_attack_marks(&game, e, tx - 1, ty));
    e->frozen_turns = 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("frozen regrowth keeps its exact mask and countdown", e->move_timer == 2 && e->boss_aux_x == mask && e->attack_target_x == tx);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("warned briars grow on their exact cells, block walking, and leave recovery without damage", game.map.tiles[ty][tx - 1] == TILE_REGENT_WALL_4 && game.map.tiles[ty][tx] == TILE_REGENT_WALL_4 && game.map.tiles[ty][tx + 1] == TILE_REGENT_WALL_4 && !map_is_walkable(&game.map, tx, ty) && game.player.hp == 1000 && e->move_timer == -1);
    for (int n = 0; n < 4; n++) {
        thorn_regent_tick_floor(&game);
    }
    ASSERT("temporary briars expire after four actions and restore the route", game.map.tiles[ty][tx] == TILE_MOONVEIL_FLOOR && map_is_walkable(&game.map, tx, ty));
    e = prepare_growth();
    tx = e->attack_target_x;
    ty = e->attack_target_y;
    game.player.x = tx;
    game.player.y = ty;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("regrowth skips a player who steps onto a warned growth cell", game.map.tiles[ty][tx] == TILE_MOONVEIL_FLOOR && game.player.hp == 1000);
    e = prepare_growth();
    tx = e->attack_target_x;
    ty = e->attack_target_y;
    game.enemies[1] = (Enemy){.type = ENEMY_THORN_GUARDIAN, .active = 1, .hp = 100, .max_hp = 100, .x = tx, .y = ty, .frozen_turns = 4};
    game.enemy_count = 2;
    game.floor_items[0] = (FloorItem){.active = 1, .x = tx + 1, .y = ty, .underlying_tile = TILE_MOONVEIL_FLOOR, .item = item_make_health_potion()};
    game.floor_item_count = 1;
    game.map.tiles[ty][tx + 1] = TILE_ITEM;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("regrowth skips newly occupied and loot-covered cells without discarding actors or items", game.map.tiles[ty][tx] == TILE_MOONVEIL_FLOOR && game.map.tiles[ty][tx + 1] == TILE_ITEM && game.enemies[1].active && game.floor_item_count == 1);
    int all_classes = 1;
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        e = prepare_growth();
        game.player.player_class = cls;
        game.player.x = e->attack_target_x;
        game.player.y = e->attack_target_y - 2;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        action_resolve_enemies(&game);
        all_classes &= e->move_timer == -2 && e->attack_target_x == -1 && game.map.tiles[17][28] == TILE_REGENT_NODE_4;
        action_resolve_enemies(&game);
        all_classes &= e->move_timer == -1 && game.player.hp == 1000;
        action_resolve_enemies(&game);
        all_classes &= e->move_timer == 0 && game.player.hp == 1000;
    }
    ASSERT("all three classes can tend the warned node and receive two full counterattack actions", all_classes);
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("a resting node cannot be repeatedly tended to reset its cooldown", game.map.tiles[17][28] == TILE_REGENT_NODE_2);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("node cooldown visibly expires back to an available flowering node", game.map.tiles[17][28] == TILE_REGENT_NODE);
    e = prepare_growth();
    tx = e->attack_target_x;
    ty = e->attack_target_y;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    game.player.x = tx;
    game.player.y = ty - 2;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("tending clears the adjacent grown strip and opens the flank immediately", game.map.tiles[ty][tx - 1] == TILE_MOONVEIL_FLOOR && game.map.tiles[ty][tx] == TILE_MOONVEIL_FLOOR && game.map.tiles[ty][tx + 1] == TILE_MOONVEIL_FLOOR && e->move_timer == -3);
    e = setup();
    action_resolve_enemies(&game);
    int old_target = e->attack_target_x;
    game.player.x = 28;
    game.player.y = 16;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("tending a node does not cancel a separately warned lance", e->boss_attack == REGENT_LANCE && e->move_timer == 1 && e->attack_target_x == old_target);
    e = prepare_growth();
    tx = e->attack_target_x;
    ty = e->attack_target_y;
    for (int x = 20; x < 36; x++) {
        if (x < tx - 1 || x > tx + 1) {
            game.map.tiles[ty][x] = TILE_MOONVEIL_WALL;
        }
    }
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("growth rechecks connectivity and refuses to seal a newly narrowed passage", game.map.tiles[ty][tx] == TILE_MOONVEIL_FLOOR && game.map.tiles[ty][tx - 1] == TILE_MOONVEIL_FLOOR && game.map.tiles[ty][tx + 1] == TILE_MOONVEIL_FLOOR);
}

static void test_rewards(void) {
    int victories = 1;
    for (int method = 0; method < 5; method++) {
        kill_with(method);
        int x;
        int y;
        victories &= !game.enemies[0].active && game.thorn_regent_reward_pending == 3 && find_tile(TILE_REGENT_RELIQUARY, &x, &y) && (game.defeated_bosses & (1 << LOCATION_MOONVEIL));
    }
    ASSERT("melee, bow and all three attack spells each reserve one cloak and one Strength Potion", victories);
    Enemy *e = setup();
    e->hp = 1;
    game.player.attack = 100;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 20, .y = 15, .underlying_tile = TILE_MOONVEIL_FLOOR, .item = item_make_health_potion()};
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
    int cache = find_tile(TILE_REGENT_RELIQUARY, &x, &y);
    ASSERT("full floor and inventory preserve every object and both guaranteed rewards", cache && game.floor_item_count == MAX_FLOOR_ITEMS && game.floor_items[MAX_FLOOR_ITEMS - 1].item.type == ITEM_POTION_HEALTH && game.thorn_regent_reward_pending == 3 && game.gold > 0);
    game.player.x = x;
    game.player.y = y;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("a full inventory cannot consume either reserved reward", game.thorn_regent_reward_pending == 3 && game.map.tiles[y][x] == TILE_REGENT_RELIQUARY);
    game.inventory_count--;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("one free slot claims the cloak while the Strength Potion stays reserved", game.thorn_regent_reward_pending == REGENT_POTION_PENDING && strcmp(game.inventory[MAX_INVENTORY - 1].name, "Briarweave Cloak") == 0 && game.map.tiles[y][x] == TILE_REGENT_RELIQUARY);
    int ok = save_game(&game, 99191) && load_game(&loaded, 99191);
    ASSERT("saving a partially claimed cache preserves the remaining potion without another cloak", ok && loaded.thorn_regent_reward_pending == REGENT_POTION_PENDING && loaded.map.tiles[y][x] == TILE_REGENT_RELIQUARY && strcmp(loaded.inventory[MAX_INVENTORY - 1].name, "Briarweave Cloak") == 0);
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_REGENT_RELIQUARY, .item = item_make_health_potion()};
    }
    game.map.tiles[y][x] = TILE_ITEM;
    game.inventory_count--;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("pickup reaches a covered cache, claims the potion and preserves both loot underlays", !game.thorn_regent_reward_pending && game.inventory[MAX_INVENTORY - 1].type == ITEM_POTION_STRENGTH && game.floor_items[0].underlying_tile == TILE_MOONVEIL_FLOOR && game.floor_items[1].underlying_tile == TILE_MOONVEIL_FLOOR && game.floor_item_count == MAX_FLOOR_ITEMS && game.map.tiles[y][x] == TILE_ITEM);
    Item cloak = item_make_briarweave_cloak();
    int classes = 1;
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        classes &= item_class_allowed(&cloak, cls);
    }
    ASSERT("Briarweave Cloak is all-class light armor with exactly +4 defense and 10 percent evasion", classes && cloak.type == ITEM_ARMOR && cloak.armor_family == ARMOR_FAMILY_LIGHT && cloak.defense_bonus == 4 && cloak.evasion_chance == 10 && cloak.visual_id == ITEM_VISUAL_BRIARWEAVE_CLOAK);
    kill_with(0);
    find_tile(TILE_REGENT_RELIQUARY, &x, &y);
    game.player.x = x;
    game.player.y = y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("a spacious pack receives both rewards once through Interact", !game.thorn_regent_reward_pending && game.inventory_count == 2 && strcmp(game.inventory[0].name, "Briarweave Cloak") == 0 && game.inventory[1].type == ITEM_POTION_STRENGTH);
    int attack = game.player.attack;
    action_resolve_player(&game, (Action){ACTION_USE_ITEM, 1, 0});
    ASSERT("the original special Strength Potion still grants one permanent base attack", game.player.attack == attack + 1);
    game.inventory_count = 0;
    game_open_town_portal(&game);
    game_use_town_portal(&game);
    ok = save_game(&game, 99191) && load_game(&loaded, 99191);
    ASSERT("consumed, sold or discarded rewards cannot regenerate after travel or saving", ok && !loaded.thorn_regent_reward_pending && loaded.floor_item_count == 0 && !loaded.enemies[0].active);
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
        if (!thorn_regent_attack_marks(&game, e, x, y)) {
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
        map_generate_moonveil(&game.map, MOONVEIL_DEPTH);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        enemies_spawn(&game);
        Enemy *e = &game.enemies[0];
        game.enemy_count = 1;
        const Room *room = &game.map.rooms[game.map.room_count - 1];
        int nodes = 0;
        for (int y = room->y; y < room->y + room->h; y++) {
            for (int x = room->x; x < room->x + room->w; x++) {
                nodes += game.map.tiles[y][x] == TILE_REGENT_NODE;
                if (!map_is_walkable(&game.map, x, y) || (x == e->x && y == e->y)) {
                    continue;
                }
                for (int attack = REGENT_LANCE; attack <= REGENT_REGROW; attack++) {
                    e->boss_attack = attack == REGENT_LANCE ? REGENT_REGROW : attack - 1;
                    e->hp = e->max_hp / 2;
                    e->attack_phase = 1;
                    e->move_timer = 0;
                    game.player.x = x;
                    game.player.y = y;
                    thorn_regent_tick(&game, e, NULL);
                    escapes &= escape_steps(e) <= e->move_timer;
                }
            }
        }
        props &= nodes == 2;
        e->move_timer = 0;
        e->boss_attack = REGENT_ROOTS;
        game.player.x = e->x - 1;
        game.player.y = e->y;
        thorn_regent_tick(&game, e, NULL);
        thorn_regent_tick(&game, e, NULL);
        thorn_regent_tick(&game, e, NULL);
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
    ASSERT("100 native Moonveil layouts place exactly two nodes without touching the boss or exits", props);
    ASSERT("every arena floor in 100 layouts has a walking escape within its declared warning", escapes);
    ASSERT("all 100 node layouts preserve the entrance-to-boss-to-exit route", paths);
}

static void test_saves(void) {
    Enemy *e = prepare_growth();
    e->frozen_turns = 2;
    cache_arena();
    int ok = save_game(&game, 99191) && load_game(&loaded, 99191);
    ASSERT("active and cached growth warnings preserve phase, mask, target, node choice and freeze", ok && loaded.enemies[0].attack_phase == 1 && loaded.enemies[0].boss_attack == REGENT_REGROW && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].boss_aux_x == 7 && loaded.enemies[0].frozen_turns == 2 && loaded.enemies[0].boss_wave_used == 1 && loaded.desert_cache[4].valid == 0 && loaded.moonveil_cache[4].enemies[0].attack_target_y == e->attack_target_y);
    e->frozen_turns = 0;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    game.map.tiles[22][27] = TILE_REGENT_ROOT_2;
    game.map.tiles[17][28] = TILE_REGENT_NODE_4;
    cache_arena();
    ok = save_game(&game, 99191) && load_game(&loaded, 99191);
    ASSERT("saved temporary walls, root durations, flower cooldowns and recovery retain exact states", ok && loaded.map.tiles[18][28] == TILE_REGENT_WALL_4 && loaded.map.tiles[22][27] == TILE_REGENT_ROOT_2 && loaded.map.tiles[17][28] == TILE_REGENT_NODE_4 && loaded.enemies[0].move_timer == -1 && loaded.moonveil_cache[4].map.tiles[18][28] == TILE_REGENT_WALL_4);
    game_open_town_portal(&game);
    ok = save_game(&game, 99191) && load_game(&loaded, 99191);
    if (ok) {
        game = loaded;
        game_use_town_portal(&game);
    }
    ASSERT("portal return preserves arena clocks and clears roots on the actual landing cell", ok && game.location == LOCATION_MOONVEIL && game.level == MOONVEIL_DEPTH && game.map.tiles[22][27] == TILE_MOONVEIL_FLOOR && game.map.tiles[18][28] == TILE_REGENT_WALL_4 && game.enemies[0].hp == 120 && game.player.hp == 1000);
    e = setup();
    action_resolve_enemies(&game);
    game_open_town_portal(&game);
    game_use_town_portal(&game);
    ASSERT("portal wards cancel a lance threatening the saved landing rather than causing arrival damage", game.enemies[0].move_timer == -1 && game.enemies[0].attack_target_x == -1 && game.player.hp == 1000);
    e = setup();
    e->hp = 73;
    e->move_timer = 17;
    e->frozen_turns = 3;
    game.enemies[1] = (Enemy){.type = ENEMY_THORN_GUARDIAN, .name = "Thorn Guardian", .active = 0, .hp = 0, .max_hp = 95, .x = 24, .y = 24};
    game.enemy_count = 2;
    game.gold = 321;
    game.moonveil_quest_state = 2;
    game.moonveil_quest_progress = 7;
    game.moonveil_quest_encounters = 7;
    game.map.tiles[18][23] = TILE_MOONVEIL_MOONFLOWER;
    game.map.tiles[20][23] = TILE_MOONVEIL_CIRCLE;
    game.portal_active = 1;
    game.portal_location = LOCATION_MOONVEIL;
    game.portal_level = MOONVEIL_DEPTH;
    game.portal_x = 28;
    game.portal_y = 17;
    game.portal_origin_tile = TILE_MOONVEIL_FLOOR;
    for (int y = 15; y < 29; y++) {
        for (int x = 20; x < 36; x++) {
            if (game.map.tiles[y][x] == TILE_REGENT_NODE) {
                game.map.tiles[y][x] = TILE_MOONVEIL_FLOOR;
            }
        }
    }
    map_mark_explored(&game.map, 24, 24);
    cache_arena();
    ok = make_legacy_save() && load_game(&loaded, 99191);
    ASSERT("version-131 migration resets old attack timing while preserving boss damage, freeze and deaths", ok && loaded.enemies[0].hp == 73 && loaded.enemies[0].move_timer == 0 && loaded.enemies[0].attack_target_x == -1 && loaded.enemies[0].frozen_turns == 3 && !loaded.enemies[1].active && loaded.moonveil_cache[4].enemies[0].hp == 73 && !loaded.moonveil_cache[4].enemies[1].active);
    ASSERT("migration preserves Moonseed objectives, restored flower, stone circles, exploration and hidden portal", ok && loaded.moonveil_quest_state == 2 && loaded.moonveil_quest_progress == 7 && loaded.moonveil_quest_encounters == 7 && loaded.map.tiles[18][23] == TILE_MOONVEIL_MOONFLOWER && loaded.map.tiles[20][23] == TILE_MOONVEIL_CIRCLE && map_is_explored(&loaded.map, 24, 24) && loaded.gold == 321 && loaded.player.x == 27 && loaded.map.tiles[17][28] == TILE_MOONVEIL_FLOOR && loaded.moonveil_cache[4].map.tiles[17][28] == TILE_MOONVEIL_FLOOR);
    if (ok) {
        game = loaded;
    }
    int nodes = 0;
    for (int y = 15; y < 29; y++) {
        for (int x = 20; x < 36; x++) {
            nodes += game.map.tiles[y][x] == TILE_REGENT_NODE;
        }
    }
    ASSERT("old living arenas receive two separate flowering nodes without disturbing saved progress", ok && nodes == 2);
    setup();
    game.enemies[0].active = 0;
    game.defeated_bosses |= 1 << LOCATION_MOONVEIL;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 28, .y = 22, .underlying_tile = TILE_MOONVEIL_FLOOR, .item = item_make_strength_potion()};
    game.floor_item_count = 1;
    game.map.tiles[22][28] = TILE_ITEM;
    cache_arena();
    ok = make_legacy_save() && load_game(&loaded, 99191);
    ASSERT("previous victories retain their original Strength Potion without granting repeat rewards or reviving the Regent", ok && !loaded.thorn_regent_reward_pending && !loaded.enemies[0].active && !loaded.moonveil_cache[4].enemies[0].active && loaded.floor_item_count == 1 && loaded.floor_items[0].item.type == ITEM_POTION_STRENGTH);
}

void test_thorn_regent(void) {
    printf("Thorn Regent encounter tests:\n");
    test_signatures();
    test_nodes();
    test_rewards();
    test_layouts();
    test_saves();
    remove("saves/savegame_99191.json");
}
