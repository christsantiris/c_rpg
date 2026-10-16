#include "test_utils.h"
#include "../src/game/desert_pharaoh.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;
static unsigned char visited[MAP_H][MAP_W];
static int queue[MAP_W * MAP_H];

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = LOCATION_DESERT;
    game.level = DESERT_DEPTH;
    game.max_desert_level_reached = DESERT_DEPTH;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            game.map.tiles[y][x] = TILE_DESERT_WALL;
        }
    }
    game.map.room_count = 1;
    game.map.rooms[0] = (Room){20, 15, 16, 14};
    for (int y = 15; y < 29; y++) {
        for (int x = 20; x < 36; x++) {
            game.map.tiles[y][x] = TILE_DESERT_FLOOR;
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
    game.enemies[0] = (Enemy){.type = ENEMY_DESERT_PHARAOH, .name = "Desert Pharaoh", .x = 28, .y = 22,
        .active = 1, .is_boss = 1, .hp = 220, .max_hp = 220, .attack = 22, .defense = 7,
        .experience = 600, .attack_target_x = -1, .attack_target_y = -1, .boss_aux_x = -1, .boss_aux_y = -1};
    desert_pharaoh_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    return &game.enemies[0];
}

static int find_tile(TileType tile, int *x, int *y) {
    for (int row = 0; row < MAP_H; row++) {
        for (int col = 0; col < MAP_W; col++) {
            if (game.map.tiles[row][col] == tile) {
                *x = col;
                *y = row;
                return 1;
            }
        }
    }
    return 0;
}

static void test_signatures(void) {
    Enemy *e = setup();
    EnemyProjectiles shots = {0};
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("adjacent Pharaoh warns a fixed one-action beam without extra melee", e->boss_attack == PHARAOH_BEAM && e->move_timer == 1 && game.player.hp == 1000 && !shots.count);
    int tx = e->attack_target_x;
    int ty = e->attack_target_y;
    e->hp = 110;
    e->frozen_turns = 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze and half-health damage preserve the exact phase-one beam", e->move_timer == 1 && !e->attack_phase && e->attack_target_x == tx && e->attack_target_y == ty);
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x, game.player.y + 1});
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("an ordinary sidestep escapes while the beam projectile uses its original target", game.player.hp == 1000 && e->move_timer == -1 && shots.count == 1 && shots.shots[0].target_x == tx && shots.shots[0].target_y == ty);
    action_resolve_enemies(&game);
    ASSERT("the entire following counterattack action has no Pharaoh retaliation", e->move_timer == 0 && game.player.hp == 1000);
    action_resolve_enemies(&game);
    ASSERT("Sandbound begins before the next two-action burial warning", e->attack_phase == 1 && e->boss_attack == PHARAOH_BURIAL && e->move_timer == 2);
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x - 1, game.player.y});
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x - 1, game.player.y});
    action_resolve_enemies(&game);
    ASSERT("two ordinary steps escape the locked burial cross", game.player.hp == 1000 && e->move_timer == -1);
    int x;
    int y;
    ASSERT("burial leaves finite two-action sand patches", find_tile(TILE_PHARAOH_SAND_2, &x, &y));
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("sand expires before the next storm and the new band has a two-action warning", !find_tile(TILE_PHARAOH_SAND_2, &x, &y) && !find_tile(TILE_PHARAOH_SAND_1, &x, &y) && e->boss_attack == PHARAOH_STORM && e->move_timer == 2);

    e = setup();
    game.player.defense = 4;
    action_resolve_enemies_with_projectiles(&game, &shots);
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("standing in the beam takes exactly one hit with the existing half-defense formula", game.player.hp == 980 && shots.count == 1 && e->move_timer == -1);
    e = setup();
    game.inventory[0] = item_make_shadow_armor();
    game.inventory[0].evasion_chance = 100;
    game.inventory_count = 1;
    game.equipped_armor = 0;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("signature damage still honors equipped evasion", game.player.hp == 1000);
    e = setup();
    e->y = 17;
    game.player.x = 21;
    game.player.y = 17;
    action_resolve_enemies(&game);
    ASSERT("sandstone obelisks block curse preparation behind cover", e->attack_target_x == -1 && !desert_pharaoh_attack_marks(&game, e, 21, 17));
    e = setup();
    game.player.x = 24;
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_MUMMY, .x = 26, .y = 22, .hp = 100, .max_hp = 100, .frozen_turns = 4};
    game.enemy_count = 2;
    action_resolve_enemies(&game);
    game.enemies[1].active = 0;
    action_resolve_enemies(&game);
    ASSERT("removing living cover cannot create previously unmarked beam tiles", game.player.hp == 1000 && e->move_timer == -1);
    e = setup();
    game.player.x = e->x - 1;
    game.map.tiles[game.player.y - 1][game.player.x] = TILE_DESERT_WALL;
    game.map.tiles[game.player.y + 1][game.player.x] = TILE_DESERT_WALL;
    game.map.tiles[game.player.y][game.player.x - 1] = TILE_DESERT_WALL;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("a player boxed in by actors and terrain receives an empty swing", e->attack_target_x == -1 && game.player.hp == 1000);
}

static void test_storms(void) {
    Enemy *e = setup();
    int x;
    int y;
    find_tile(TILE_PHARAOH_OBELISK, &x, &y);
    ASSERT("obelisk collision leaves neighboring sand available", !map_is_walkable(&game.map, x, y) && map_is_walkable(&game.map, x + 1, y));
    int shelter = 1;
    int moving = 1;
    int damaging = 1;
    int old_x = -1;
    int old_y = -1;
    for (int pass = 0; pass < 12; pass++) {
        e->hp = 110;
        e->attack_phase = 1;
        e->boss_attack = PHARAOH_BURIAL;
        e->move_timer = 0;
        game.player.x = 27;
        game.player.y = 22;
        action_resolve_enemies(&game);
        int tx = e->attack_target_x;
        int ty = e->attack_target_y;
        int dx = e->facing_dx;
        int dy = e->facing_dy;
        moving &= pass == 0 || tx != old_x || ty != old_y || pass % 3 == 0;
        old_x = tx;
        old_y = ty;
        shelter &= desert_pharaoh_shadow(&game, e, x + dx, y + dy) &&
            !desert_pharaoh_shadow(&game, e, x - dx, y - dy) &&
            !desert_pharaoh_attack_marks(&game, e, x + dx, y + dy);
        game.player.x = x + dx;
        game.player.y = y + dy;
        int hp = game.player.hp;
        action_resolve_enemies(&game);
        shelter &= e->attack_target_x == tx && e->attack_target_y == ty && e->facing_dx == dx && e->facing_dy == dy;
        action_resolve_enemies(&game);
        shelter &= game.player.hp == hp && e->move_timer == -1;
        e->move_timer = 1;
        e->attack_target_x = tx;
        e->attack_target_y = ty;
        game.player.x = dx ? tx : 27;
        game.player.y = dx ? 21 : ty;
        hp = game.player.hp;
        damaging &= desert_pharaoh_attack_marks(&game, e, game.player.x, game.player.y);
        action_resolve_enemies(&game);
        damaging &= game.player.hp == hp - 22;
    }
    ASSERT("all four wind directions move sheltered shadows and leave warned shelter unharmed", shelter);
    ASSERT("successive storm bands move between attacks while each two-action footprint remains fixed", moving);
    ASSERT("every storm damages marked cells exactly once using the same predicate", damaging);
    e = setup();
    e->hp = 110;
    e->boss_attack = PHARAOH_BURIAL;
    game.player.x = 23;
    game.player.y = 17;
    action_resolve_enemies(&game);
    ASSERT("a native eastward storm crosses the sheltered player tile without marking it dangerous", e->attack_target_x == 23 && e->facing_dx == 1 && desert_pharaoh_shadow(&game, e, 23, 17) && !desert_pharaoh_attack_marks(&game, e, 23, 17));
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("shelter protects inside the actual generated band through resolution", game.player.hp == 1000 && e->move_timer == -1);
    e = setup();
    game.map.tiles[22][27] = TILE_PHARAOH_SAND_2;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 27, .y = 22, .underlying_tile = TILE_PHARAOH_SAND_2, .item = item_make_health_potion()};
    }
    game.floor_item_count = 2;
    game.map.tiles[22][27] = TILE_ITEM;
    e->frozen_turns = 3;
    action_resolve_enemies(&game);
    ASSERT("burial timers tick beneath every stacked item even while the Pharaoh is frozen", game.player.hp == 992 && game.floor_items[0].underlying_tile == TILE_PHARAOH_SAND_1 && game.floor_items[1].underlying_tile == TILE_PHARAOH_SAND_1 && game.map.tiles[22][27] == TILE_ITEM);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("burial damage stops after two actions without losing stacked loot", game.player.hp == 984 && game.floor_items[0].underlying_tile == TILE_DESERT_FLOOR && game.floor_items[1].underlying_tile == TILE_DESERT_FLOOR && game.floor_item_count == 2);
}

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
        if (!desert_pharaoh_attack_marks(&game, e, x, y)) {
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
        map_generate_desert(&game.map, DESERT_DEPTH);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        enemies_spawn(&game);
        Enemy *e = &game.enemies[0];
        game.enemy_count = 1;
        const Room *room = &game.map.rooms[game.map.room_count - 1];
        int obelisks = 0;
        for (int y = room->y; y < room->y + room->h; y++) {
            for (int x = room->x; x < room->x + room->w; x++) {
                obelisks += game.map.tiles[y][x] == TILE_PHARAOH_OBELISK;
                if (!map_is_walkable(&game.map, x, y) || (x == e->x && y == e->y)) {
                    continue;
                }
                for (int attack = PHARAOH_BEAM; attack <= PHARAOH_STORM; attack++) {
                    e->boss_attack = attack == PHARAOH_BEAM ? PHARAOH_STORM : attack - 1;
                    e->hp = e->max_hp / 2;
                    e->attack_phase = 1;
                    e->move_timer = 0;
                    game.player.x = x;
                    game.player.y = y;
                    desert_pharaoh_tick(&game, e, NULL);
                    escapes &= escape_steps(e) <= e->move_timer;
                }
            }
        }
        props &= obelisks == 4;
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
    ASSERT("100 native Sunscar layouts place exactly four obelisks without touching the boss or exits", props);
    ASSERT("every arena floor in 100 layouts has a walking escape within its declared warning", escapes);
    ASSERT("all 100 obelisk layouts preserve the entrance-to-boss-to-exit route", paths);
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

static void test_rewards(void) {
    int victories = 1;
    for (int method = 0; method < 5; method++) {
        kill_with(method);
        int staves = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            staves += game.floor_items[i].active && strcmp(game.floor_items[i].item.name, "Sandstorm Staff") == 0;
        }
        victories &= !game.enemies[0].active && staves == 1 && game.desert_pharaoh_reward_state == 1 && (game.defeated_bosses & (1 << LOCATION_DESERT));
    }
    ASSERT("melee, bow, Magic Arrow, Fireball and Frost Bolt each guarantee one Sandstorm Staff", victories);
    Enemy *e = setup();
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 100;
    e->hp = 1;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = game.map.rooms[0].x, .y = game.map.rooms[0].y, .underlying_tile = TILE_DESERT_FLOOR, .item = item_make_health_potion()};
    }
    game.floor_item_count = MAX_FLOOR_ITEMS;
    game.map.tiles[game.map.rooms[0].y][game.map.rooms[0].x] = TILE_ITEM;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        game.inventory[i] = item_make_health_potion();
    }
    game.inventory_count = MAX_INVENTORY;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    int x = 0;
    int y = 0;
    ASSERT("full floor capacity preserves every item and reserves reward in a reliquary", game.floor_item_count == MAX_FLOOR_ITEMS && game.floor_items[MAX_FLOOR_ITEMS - 1].item.type == ITEM_POTION_HEALTH && game.desert_pharaoh_reward_state == 1 && find_tile(TILE_PHARAOH_RELIQUARY, &x, &y));
    int saved = save_game(&game, 99189) && load_game(&loaded, 99189);
    ASSERT("a full floor-loot save retains the reserved staff, both claim flags and every existing item", saved && loaded.desert_pharaoh_reward_state == 1 && loaded.sandstorm_staff_unclaimed && loaded.map.tiles[y][x] == TILE_PHARAOH_RELIQUARY && loaded.floor_item_count == MAX_FLOOR_ITEMS);
    game.player.x = x;
    game.player.y = y;
    game.map.tiles[y][x + 1] = TILE_DESERT_LAMP;
    game.map.tiles[y - 1][x] = TILE_DESERT_LAMP;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("full inventory cannot consume the reserved reward", game.desert_pharaoh_reward_state == 1 && game.map.tiles[y][x] == TILE_PHARAOH_RELIQUARY);
    game.floor_items[0].x = x;
    game.floor_items[0].y = y;
    game.floor_items[0].underlying_tile = TILE_PHARAOH_RELIQUARY;
    game.floor_items[1].x = x;
    game.floor_items[1].y = y;
    game.floor_items[1].underlying_tile = TILE_PHARAOH_RELIQUARY;
    game.map.tiles[y][x] = TILE_ITEM;
    ASSERT("reward interaction remains discoverable beneath stacked floor items", game_has_regional_interaction(&game));
    game.inventory_count--;
    int reward_score = game.score;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("reliquary pickup grants original Sandstorm Staff and marks it claimed", game.desert_pharaoh_reward_state == 2 && strcmp(game.inventory[MAX_INVENTORY - 1].name, "Sandstorm Staff") == 0 && game.inventory[MAX_INVENTORY - 1].spell_power_bonus == item_make_sandstorm_staff().spell_power_bonus && game.score == reward_score + game.inventory[MAX_INVENTORY - 1].value);
    ASSERT("claiming a cache beneath loot preserves both objects and repairs both underlays", game.floor_item_count == MAX_FLOOR_ITEMS && game.map.tiles[y][x] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_DESERT_FLOOR && game.floor_items[1].underlying_tile == TILE_DESERT_FLOOR);
    desert_pharaoh_restore_reward(&game);
    ASSERT("claiming cannot recreate a second reward or operate adjacent quest objects", !find_tile(TILE_PHARAOH_RELIQUARY, &x, &y) && game.map.tiles[game.player.y - 1][game.player.x] == TILE_DESERT_LAMP);
    kill_with(0);
    game.player.x = game.enemies[0].x;
    game.player.y = game.enemies[0].y;
    for (int i = 0; i < 3; i++) {
        action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    }
    ASSERT("normal ground pickup also records a claimed reward", game.desert_pharaoh_reward_state == 2);
    game.inventory_count = 0;
    game_open_town_portal(&game);
    game_use_town_portal(&game);
    int saved_claim = save_game(&game, 99189) && load_game(&loaded, 99189);
    ASSERT("a sold or discarded staff cannot be awarded again after travel and save/load", saved_claim && loaded.desert_pharaoh_reward_state == 2 && !loaded.sandstorm_staff_unclaimed && !loaded.floor_item_count);
}


static int make_legacy_save(void) {
    if (!save_game(&game, 99189)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99189.json", "r");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 130);
    cJSON_DeleteItemFromObject(root, "desert_pharaoh_reward_state");
    cJSON *arrays[2] = {cJSON_GetObjectItem(root, "enemies"),
        cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "desert_cache"), DESERT_DEPTH - 1), "enemies")};
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
    file = fopen("saves/savegame_99189.json", "w");
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
    LevelCache *cache = &game.desert_cache[DESERT_DEPTH - 1];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = game.enemy_count;
    memcpy(cache->enemies, game.enemies, sizeof(Enemy) * (size_t)game.enemy_count);
}

static void test_saves(void) {
    Enemy *e = setup();
    e->hp = 110;
    e->boss_attack = PHARAOH_BURIAL;
    e->boss_wave_used = 5;
    action_resolve_enemies(&game);
    e->frozen_turns = 2;
    game.map.tiles[20][26] = TILE_PHARAOH_SAND_2;
    game.map.tiles[20][27] = TILE_PHARAOH_SAND_1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 26, .y = 20, .underlying_tile = TILE_PHARAOH_SAND_2, .item = item_make_health_potion()};
    game.floor_item_count = 1;
    game.map.tiles[20][26] = TILE_ITEM;
    cache_arena();
    int ok = save_game(&game, 99189) && load_game(&loaded, 99189);
    ASSERT("active and cached storms save exact direction, band, phase, freeze and sequence", ok && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].frozen_turns == 2 && loaded.enemies[0].attack_phase == 1 && loaded.enemies[0].boss_attack == PHARAOH_STORM && loaded.enemies[0].boss_wave_used == 6 && loaded.enemies[0].facing_dy == 1 && loaded.enemies[0].attack_target_y == e->attack_target_y && loaded.desert_cache[4].enemies[0].attack_target_x == e->attack_target_x && loaded.desert_cache[4].enemies[0].hp == 110);
    ASSERT("saved sand clocks, floor underlays and obelisks retain native state", ok && loaded.floor_items[0].underlying_tile == TILE_PHARAOH_SAND_2 && loaded.map.tiles[20][27] == TILE_PHARAOH_SAND_1 && loaded.map.tiles[17][22] == TILE_PHARAOH_OBELISK && loaded.desert_cache[4].map.tiles[20][27] == TILE_PHARAOH_SAND_1);
    game.player.x = 26;
    game.player.y = 20;
    e->frozen_turns = 0;
    game_open_town_portal(&game);
    ok = save_game(&game, 99189) && load_game(&loaded, 99189);
    if (ok) {
        game = loaded;
        game_use_town_portal(&game);
    }
    ASSERT("portal return wards lingering sand on arrival without changing Pharaoh health", ok && game.location == LOCATION_DESERT && game.level == DESERT_DEPTH && game.player.hp == 1000 && game.map.tiles[20][26] == TILE_DESERT_FLOOR && game.enemies[0].hp == 110);
    e = setup();
    action_resolve_enemies(&game);
    game_open_town_portal(&game);
    game_use_town_portal(&game);
    ASSERT("a queued beam threatening the portal landing is canceled for a recovery action", game.enemies[0].move_timer == -1 && game.enemies[0].attack_target_x == -1 && game.player.hp == 1000);
    e = setup();
    e->move_timer = -1;
    cache_arena();
    ok = save_game(&game, 99189) && load_game(&loaded, 99189);
    ASSERT("saving during recovery does not invent another immediate attack", ok && loaded.enemies[0].move_timer == -1 && loaded.desert_cache[4].enemies[0].move_timer == -1);
    e = setup();
    e->boss_attack = PHARAOH_BEAM;
    action_resolve_enemies(&game);
    cache_arena();
    ok = save_game(&game, 99189) && load_game(&loaded, 99189);
    ASSERT("two-action burial warnings preserve their original targets in active and cached saves", ok && loaded.enemies[0].boss_attack == PHARAOH_BURIAL && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].attack_target_x == 27 && loaded.enemies[0].attack_target_y == 22 && loaded.desert_cache[4].enemies[0].attack_target_x == 27);
    e = setup();
    e->hp = 73;
    e->move_timer = 17;
    e->frozen_turns = 3;
    game.enemies[1] = (Enemy){.type = ENEMY_MUMMY, .name = "Mummy", .active = 0, .hp = 0, .max_hp = 35, .x = 24, .y = 24};
    game.enemy_count = 2;
    game.gold = 321;
    game.sunscar_lamp_quest_state = 2;
    game.map.tiles[18][23] = TILE_DESERT_LAMP;
    game.portal_active = 1;
    game.portal_location = LOCATION_DESERT;
    game.portal_level = DESERT_DEPTH;
    game.portal_x = 22;
    game.portal_y = 17;
    game.portal_origin_tile = TILE_DESERT_FLOOR;
    for (int y = 15; y < 29; y++) {
        for (int x = 20; x < 36; x++) {
            if (game.map.tiles[y][x] == TILE_PHARAOH_OBELISK) {
                game.map.tiles[y][x] = TILE_DESERT_FLOOR;
            }
        }
    }
    map_mark_explored(&game.map, 24, 24);
    cache_arena();
    ok = make_legacy_save() && load_game(&loaded, 99189);
    ASSERT("version-130 migration resets obsolete Pharaoh AI but preserves damage, freeze and dead enemies", ok && loaded.enemies[0].hp == 73 && loaded.enemies[0].move_timer == 0 && loaded.enemies[0].frozen_turns == 3 && loaded.enemies[0].attack_target_x == -1 && !loaded.enemies[1].active && loaded.desert_cache[4].enemies[0].hp == 73 && !loaded.desert_cache[4].enemies[1].active);
    ASSERT("migration preserves quest props, exploration, gold, player and hidden portal destination", ok && loaded.sunscar_lamp_quest_state == 2 && loaded.map.tiles[18][23] == TILE_DESERT_LAMP && loaded.gold == 321 && loaded.player.x == 27 && loaded.player.y == 22 && map_is_explored(&loaded.map, 24, 24) && loaded.map.tiles[17][22] == TILE_DESERT_FLOOR && loaded.desert_cache[4].map.tiles[17][22] == TILE_DESERT_FLOOR);
    if (ok) {
        game = loaded;
    }
    int props = 0;
    for (int y = 15; y < 29; y++) {
        for (int x = 20; x < 36; x++) {
            props += game.map.tiles[y][x] == TILE_PHARAOH_OBELISK;
        }
    }
    ASSERT("old active and cached arenas receive four sandstone obelisks in place", ok && props == 4);
    setup();
    game.enemies[0].active = 0;
    game.defeated_bosses |= 1 << LOCATION_DESERT;
    game.sandstorm_staff_unclaimed = 1;
    cache_arena();
    ok = make_legacy_save() && load_game(&loaded, 99189);
    ASSERT("the known old unclaimed staff flag migrates to one recoverable reward", ok && loaded.desert_pharaoh_reward_state == 1 && loaded.sandstorm_staff_unclaimed && !loaded.enemies[0].active && loaded.floor_item_count == 1 && strcmp(loaded.floor_items[0].item.name, "Sandstorm Staff") == 0);
    game.sandstorm_staff_unclaimed = 0;
    ok = make_legacy_save() && load_game(&loaded, 99189);
    ASSERT("previously claimed rewards and victories never regenerate during migration", ok && loaded.desert_pharaoh_reward_state == 2 && !loaded.sandstorm_staff_unclaimed && !loaded.floor_item_count && !loaded.enemies[0].active);
}

void test_desert_pharaoh(void) {
    printf("Desert Pharaoh encounter tests:\n");
    test_signatures();
    test_storms();
    test_layouts();
    test_rewards();
    test_saves();
    remove("saves/savegame_99189.json");
}
