#include "test_utils.h"
#include "../src/game/red_dragon.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = LOCATION_DRAGONSPINE;
    game.level = DRAGONSPINE_DEPTH;
    game.max_dragonspine_level_reached = DRAGONSPINE_DEPTH;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            game.map.tiles[y][x] = TILE_DRAGON_WALL;
        }
    }
    game.map.room_count = 1;
    game.map.rooms[0] = (Room){20, 15, 32, 27};
    for (int y = 15; y < 42; y++) {
        for (int x = 20; x < 52; x++) {
            game.map.tiles[y][x] = TILE_DRAGON_ASH;
        }
    }
    game.map.stairs_up_x = 20;
    game.map.stairs_up_y = 28;
    game.map.stairs_down_x = 51;
    game.map.stairs_down_y = 28;
    game.map.tiles[28][20] = TILE_DRAGON_ENTRANCE;
    game.map.tiles[28][51] = TILE_DRAGON_EXIT;
    game.player.x = 34;
    game.player.y = 28;
    game.player.hp = 1000;
    game.player.max_hp = 1000;
    game.player.mp = 100;
    game.player.max_mp = 100;
    game.player.attack = 200;
    game.player.defense = 0;
    game.inventory_count = 0;
    game.equipped_main_hand = -1;
    game.equipped_off_hand = -1;
    game.equipped_armor = -1;
    game.floor_item_count = 0;
    game.enemy_count = 1;
    game.enemies[0] = (Enemy){.type = ENEMY_RED_DRAGON, .name = "Red Dragon", .active = 1,
        .is_boss = 1, .x = 36, .y = 28, .hp = 360, .max_hp = 360, .attack = 29, .defense = 11,
        .experience = 1200, .attack_target_x = -1, .attack_target_y = -1};
    red_dragon_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    return &game.enemies[0];
}

static void tick(void) {
    action_resolve_enemies(&game);
}

static void clear_columns(void) {
    for (int y = 15; y < 42; y++) {
        for (int x = 20; x < 52; x++) {
            TileType tile = game.map.tiles[y][x];
            if (tile >= TILE_DRAGON_BASALT && tile <= TILE_DRAGON_CRACKED_COLUMN) {
                game.map.tiles[y][x] = TILE_DRAGON_ASH;
            }
        }
    }
}

static void attack(int type) {
    Enemy *e = &game.enemies[0];
    e->hp = type == DRAGON_WINGBEAT ? e->max_hp * 2 / 3 : type == DRAGON_INFERNO_FIRST ? e->max_hp / 3 : e->max_hp;
    e->boss_wave_used = type == DRAGON_SWEEP || type == DRAGON_WINGBEAT;
    e->move_timer = 0;
    tick();
}

static int find_tile(TileType type, int *x, int *y) {
    for (int row = 0; row < MAP_H; row++) {
        for (int col = 0; col < MAP_W; col++) {
            TileType tile = game.map.tiles[row][col];
            for (int i = 0; tile == TILE_ITEM && i < game.floor_item_count; i++) {
                const FloorItem *f = &game.floor_items[i];
                if (f->active && f->x == col && f->y == row) {
                    tile = (TileType)f->underlying_tile;
                }
            }
            if (tile == type) {
                *x = col;
                *y = row;
                return 1;
            }
        }
    }
    return 0;
}

static int floor_mantles(void) {
    int count = 0;
    for (int i = 0; i < game.floor_item_count; i++) {
        count += game.floor_items[i].active && strcmp(game.floor_items[i].item.name, "Dragon Scale Mantle") == 0;
    }
    return count;
}

static int escaping(const Enemy *e, int x, int y, int turns) {
    if (!red_dragon_attack_marks(&game, e, x, y)) {
        return 1;
    }
    if (!turns) {
        return 0;
    }
    static const int moves[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int n = 0; n < 4; n++) {
        int nx = x + moves[n][0];
        int ny = y + moves[n][1];
        if (map_is_walkable(&game.map, nx, ny) && !(nx == e->x && ny == e->y) && escaping(e, nx, ny, turns - 1)) {
            return 1;
        }
    }
    return 0;
}

static int reachable(const Map *m) {
    static unsigned char visited[MAP_H][MAP_W];
    static int queue[MAP_H * MAP_W];
    memset(visited, 0, sizeof(visited));
    int head = 0;
    int tail = 0;
    int sx = m->stairs_up_x;
    int sy = m->stairs_up_y;
    visited[sy][sx] = 1;
    queue[tail++] = sy * MAP_W + sx;
    static const int moves[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    while (head < tail) {
        int cell = queue[head++];
        for (int n = 0; n < 4; n++) {
            int x = cell % MAP_W + moves[n][0];
            int y = cell / MAP_W + moves[n][1];
            if (map_is_walkable(m, x, y) && !visited[y][x]) {
                visited[y][x] = 1;
                queue[tail++] = y * MAP_W + x;
            }
        }
    }
    return tail;
}

static void cache_arena(void) {
    LevelCache *cache = &game.dragonspine_cache[DRAGONSPINE_DEPTH - 1];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = game.enemy_count;
    memcpy(cache->enemies, game.enemies, sizeof(Enemy) * (size_t)game.enemy_count);
}

static int legacy_save(void) {
    if (!save_game(&game, 99199)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99199.json", "r");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = malloc((size_t)size + 1);
    if (!text) {
        fclose(file);
        return 0;
    }
    size_t length = fread(text, 1, (size_t)size, file);
    text[length] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 135);
    cJSON_DeleteItemFromObject(root, "red_dragon_reward_state");
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99199.json", "w");
    if (!file || !json) {
        if (file) {
            fclose(file);
        }
        free(json);
        return 0;
    }
    int result = fputs(json, file) >= 0;
    fclose(file);
    free(json);
    return result;
}

static void kill_with(int method, int full) {
    Enemy *e = setup();
    if (full) {
        game.inventory_count = MAX_INVENTORY;
        for (int i = 0; i < MAX_INVENTORY; i++) {
            game.inventory[i] = item_make_health_potion();
        }
        game.floor_item_count = MAX_FLOOR_ITEMS;
        for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
            game.floor_items[i] = (FloorItem){.active = 1, .x = 21 + i, .y = 17,
                .underlying_tile = TILE_DRAGON_ASH, .item = item_make_health_potion()};
            game.map.tiles[17][21 + i] = TILE_ITEM;
        }
    }
    e->hp = 1;
    game.player.last_dx = 1;
    game.player.last_dy = 0;
    if (!method) {
        game.player.x = e->x - 1;
        action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    } else if (method == 1) {
        game.player.player_class = CLASS_ROGUE;
        game.player.arrows = 20;
        game.inventory[0] = item_make_bow();
        game.equipped_main_hand = 0;
        if (!full) {
            game.inventory_count = 1;
        }
        action_resolve_player(&game, (Action){ACTION_RANGED_ATTACK, 0, 0});
    } else {
        game.player.player_class = CLASS_MAGE;
        game.player.known_spell_count = 1;
        game.player.equipped_spell = 0;
        game.player.known_spells[0] = method == 2 ? spell_make_magic_arrow() : method == 3 ? spell_make_fireball() : spell_make_frost_bolt();
        action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
    }
}

static void test_signatures(void) {
    Enemy *e = setup();
    attack(DRAGON_BREATH);
    ASSERT("Hoard Defender declares a two-action fixed cone before doing any damage", e->boss_attack == DRAGON_BREATH &&
        e->move_timer == 2 && game.player.hp == 1000 && red_dragon_attack_marks(&game, e, 34, 28) &&
        red_dragon_attack_marks(&game, e, 33, 27) && !red_dragon_attack_marks(&game, e, 33, 26));
    unsigned int warning[DRAGON_WARNING_SIZE];
    memcpy(warning, e->dragon_warning, sizeof(warning));
    game.player.y++;
    tick();
    ASSERT("moving does not retarget a queued cone", e->move_timer == 1 && memcmp(warning, e->dragon_warning, sizeof(warning)) == 0);
    EnemyProjectiles shots = {0};
    Enemy pending = *e;
    action_resolve_enemies_with_projectiles(&game, &shots);
    int shots_ok = shots.count > 0;
    for (int n = 0; n < shots.count; n++) {
        shots_ok &= red_dragon_attack_marks(&game, &pending, shots.shots[n].target_x, shots.shots[n].target_y);
    }
    ASSERT("sidestepping avoids the fixed breath and fire projectiles end only on warned cells", game.player.hp == 1000 && e->move_timer == -1 && shots_ok);
    tick();
    ASSERT("normal recovery grants a full action without melee retaliation", e->move_timer == 0 && game.player.hp == 1000);
    game.player.x = 35;
    game.player.y = 28;
    tick();
    ASSERT("nearby follow-up is a separately warned one-action claw and tail sweep", e->boss_attack == DRAGON_SWEEP &&
        e->move_timer == 1 && red_dragon_attack_marks(&game, e, 35, 28) && !red_dragon_attack_marks(&game, e, 34, 28));
    action_resolve_player(&game, (Action){ACTION_MOVE, 34, 28});
    tick();
    ASSERT("one ordinary retreat escapes the close sweep", game.player.hp == 1000 && e->move_timer == -1);

    e = setup();
    game.player.x = 35;
    game.player.defense = 10;
    attack(DRAGON_BREATH);
    tick();
    tick();
    ASSERT("a player remaining adjacent takes exactly the warned attack after armor, without an extra melee hit",
        game.player.hp == 1000 - (e->attack - 5));

    e = setup();
    attack(DRAGON_BREATH);
    memcpy(warning, e->dragon_warning, sizeof(warning));
    e->hp = e->max_hp * 2 / 3;
    e->frozen_turns = 2;
    game.map.tiles[20][25] = TILE_DRAGON_HOT_HOARD_3;
    tick();
    tick();
    ASSERT("freeze and the two-thirds threshold preserve the pending cone while existing fire expires independently",
        e->move_timer == 2 && e->attack_phase == 0 && memcmp(warning, e->dragon_warning, sizeof(warning)) == 0 &&
        game.map.tiles[20][25] == TILE_DRAGON_HOT_HOARD_1);
    game.player.y++;
    tick();
    tick();
    tick();
    tick();
    ASSERT("Scorched Wings starts on the next declaration with a two-action landing", e->attack_phase == 1 &&
        e->boss_attack == DRAGON_WINGBEAT && e->move_timer == 2);
    int tx = e->attack_target_x;
    int ty = e->attack_target_y;
    e->hp = e->max_hp / 3;
    tick();
    ASSERT("crossing one-third health cannot change a warned landing", e->attack_phase == 1 && e->boss_attack == DRAGON_WINGBEAT &&
        e->attack_target_x == tx && e->attack_target_y == ty && e->move_timer == 1);

    e = setup();
    attack(DRAGON_INFERNO_FIRST);
    memcpy(warning, e->dragon_warning, sizeof(warning));
    int fx = e->facing_dx;
    int fy = e->facing_dy;
    game.player.y += 2;
    tick();
    tick();
    ASSERT("Last Inferno resolves its first cone then declares a fresh two-action quarter-turn cone", e->attack_phase == 2 &&
        e->boss_attack == DRAGON_INFERNO_SECOND && e->move_timer == 2 && e->facing_dx == -fy && e->facing_dy == fx &&
        !red_dragon_attack_marks(&game, e, 34, 28) && red_dragon_attack_marks(&game, e, 36, 26));
    e->frozen_turns = 2;
    memcpy(warning, e->dragon_warning, sizeof(warning));
    tick();
    tick();
    ASSERT("freezing between Inferno breaths preserves the second warning instead of repeating the first",
        e->boss_attack == DRAGON_INFERNO_SECOND && e->move_timer == 2 && memcmp(warning, e->dragon_warning, sizeof(warning)) == 0);
    tick();
    tick();
    ASSERT("the linked sequence ends with two full recovery actions", e->move_timer == -2);
    int hp = game.player.hp;
    tick();
    ASSERT("the first long-recovery action is safe", e->move_timer == -1 && game.player.hp == hp);
    tick();
    ASSERT("the second long-recovery action is also safe", e->move_timer == 0 && game.player.hp == hp);
    e->hp = e->max_hp;
    tick();
    ASSERT("healing cannot reverse Last Inferno", e->attack_phase == 2 && e->boss_attack == DRAGON_INFERNO_FIRST);

    int diagonals = 1;
    for (int dx = -1; dx <= 1; dx += 2) {
        for (int dy = -1; dy <= 1; dy += 2) {
            e = setup();
            clear_columns();
            game.player.x = e->x + dx * 4;
            game.player.y = e->y + dy * 4;
            attack(DRAGON_BREATH);
            diagonals &= e->facing_dx == dx && e->facing_dy == dy && red_dragon_attack_marks(&game, e, game.player.x, game.player.y) &&
                escaping(e, game.player.x, game.player.y, 2);
            game.player.x += dx;
            tick();
            game.player.x += dx;
            tick();
            diagonals &= game.player.hp == 1000;
        }
    }
    ASSERT("fire breath covers all four diagonal firing lines and still permits two walking steps to escape", diagonals);

    e = setup();
    game.player.x = 24;
    game.player.y = 16;
    clear_columns();
    e->hp--;
    // A diagonal longbow can engage from twelve tiles, even though Manhattan distance is twenty-four.
    game.map.rooms[0] = (Room){30, 23, 22, 19};
    tick();
    ASSERT("damaged dragons respond to long diagonal ranged engagement outside the lair", e->move_timer == 2 &&
        e->facing_dx == -1 && e->facing_dy == -1 && red_dragon_attack_marks(&game, e, 24, 16));
}

static void test_cover_and_fire(void) {
    Enemy *e = setup();
    game.map.tiles[28][33] = TILE_DRAGON_WEAK_COLUMN;
    game.player.x = 32;
    attack(DRAGON_BREATH);
    ASSERT("basalt blocks breath and the column itself receives a visible strike warning", red_dragon_attack_marks(&game, e, 33, 28) &&
        !red_dragon_attack_marks(&game, e, 32, 28) && !map_is_walkable(&game.map, 33, 28));
    tick();
    tick();
    ASSERT("first breath cracks weak cover without destroying it or hitting the sheltered player",
        game.map.tiles[28][33] == TILE_DRAGON_CRACKED_COLUMN && game.player.hp == 1000);
    attack(DRAGON_BREATH);
    ASSERT("cracked cover still blocks and warns before breaking", red_dragon_attack_marks(&game, e, 33, 28) &&
        !red_dragon_attack_marks(&game, e, 32, 28));
    tick();
    tick();
    ASSERT("a second warned breath breaks the column without extending that cone", game.map.tiles[28][33] == TILE_DRAGON_COLUMN_RUBBLE &&
        game.player.hp == 1000 && map_is_walkable(&game.map, 33, 28));
    attack(DRAGON_BREATH);
    ASSERT("only a later declaration can warn the newly exposed route", red_dragon_attack_marks(&game, e, 32, 28));
    game.player.y += 2;
    tick();
    tick();
    game.map.tiles[28][33] = TILE_DRAGON_BASALT;
    game.player.x = 32;
    game.player.y = 28;
    attack(DRAGON_BREATH);
    tick();
    tick();
    ASSERT("stable basalt remains usable permanent cover", game.map.tiles[28][33] == TILE_DRAGON_BASALT);

    e = setup();
    game.map.tiles[28][33] = TILE_DRAGON_WEAK_COLUMN;
    game.player.x = 32;
    attack(DRAGON_BREATH);
    game.map.tiles[28][33] = TILE_DRAGON_COLUMN_RUBBLE;
    ASSERT("removing cover during a warning cannot add unannounced cells", !red_dragon_attack_marks(&game, e, 32, 28));
    tick();
    tick();
    ASSERT("the fixed snapshot is also used for damage after cover changes", game.player.hp == 1000);

    e = setup();
    game.map.tiles[28][34] = TILE_DRAGON_HOARD;
    game.map.tiles[27][34] = TILE_DRAGON_FLOOR;
    game.map.tiles[29][34] = TILE_DRAGON_TREASURE;
    game.floor_item_count = 2;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 34, .y = 28, .underlying_tile = TILE_DRAGON_HOARD, .item = item_make_health_potion()};
    game.floor_items[1] = game.floor_items[0];
    game.map.tiles[28][34] = TILE_ITEM;
    attack(DRAGON_WINGBEAT);
    int tx = e->attack_target_x;
    int ty = e->attack_target_y;
    action_resolve_player(&game, (Action){ACTION_MOVE, 34, 27});
    tick();
    action_resolve_player(&game, (Action){ACTION_MOVE, 34, 26});
    tick();
    ASSERT("two walking moves escape a fixed wingbeat and the dragon lands on the vacated target",
        game.player.hp == 1000 && e->x == tx && e->y == ty && e->move_timer == -1);
    ASSERT("wingbeat creates three-action burning patches while preserving the goblet and stacked loot",
        game.map.tiles[27][34] == TILE_DRAGON_HOT_FLOOR_3 && game.map.tiles[29][34] == TILE_DRAGON_TREASURE &&
        game.map.tiles[28][34] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_DRAGON_HOT_HOARD_3 &&
        game.floor_items[1].underlying_tile == TILE_DRAGON_HOT_HOARD_3);
    e->frozen_turns = 10;
    game.player.x = 34;
    game.player.y = 27;
    int hp = game.player.hp;
    tick();
    ASSERT("lingering fire first hits on the following action and shows two remaining actions",
        game.player.hp == hp - 8 && game.map.tiles[27][34] == TILE_DRAGON_HOT_FLOOR_2);
    tick();
    tick();
    ASSERT("fire damages for exactly three actions then restores snow, ash and hoard underlays",
        game.player.hp == hp - 24 && game.map.tiles[27][34] == TILE_DRAGON_FLOOR &&
        game.map.tiles[28][33] == TILE_DRAGON_ASH && game.floor_items[0].underlying_tile == TILE_DRAGON_HOARD &&
        game.floor_items[1].underlying_tile == TILE_DRAGON_HOARD);
    tick();
    ASSERT("expired fire cannot continue damaging a stationary player", game.player.hp == hp - 24);

    e = setup();
    attack(DRAGON_WINGBEAT);
    tick();
    tick();
    ASSERT("a player remaining in the landing cross is hit but never overlapped by the boss", game.player.hp == 1000 - e->attack &&
        !(e->x == game.player.x && e->y == game.player.y));
    e = setup();
    attack(DRAGON_WINGBEAT);
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_DRAKE, .x = 34, .y = 28};
    game.enemy_count = 2;
    game.player.y -= 2;
    // The new actor is deliberately after the dragon in enemy order.
    red_dragon_tick(&game, e, NULL);
    red_dragon_tick(&game, e, NULL);
    ASSERT("new actors cannot be overlapped by a queued landing", e->x == 36 && e->y == 28);

    e = setup();
    game.player.x = 35;
    game.map.tiles[28][34] = TILE_DRAGON_WALL;
    game.map.tiles[27][35] = TILE_DRAGON_WALL;
    game.map.tiles[29][35] = TILE_DRAGON_WALL;
    attack(DRAGON_SWEEP);
    ASSERT("a boxed player receives an empty warning rather than an unavoidable sweep", e->move_timer == 1 &&
        !red_dragon_attack_marks(&game, e, 35, 28));
}

static void test_generated_arenas(void) {
    int columns_ok = 1;
    int routes_ok = 1;
    int escapes_ok = 1;
    int roster_ok = 1;
    int declared = 0;
    int withheld = 0;
    for (int seed = 0; seed < 100; seed++) {
        setup();
        srand(3100 + seed);
        map_generate_dragonspine(&game.map, DRAGONSPINE_DEPTH);
        int before = reachable(&game.map);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        enemies_spawn(&game);
        roster_ok &= game.enemy_count == AREA_ENEMY_LIMIT;
        Enemy *e = &game.enemies[0];
        int stable = 0;
        int weak = 0;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                stable += game.map.tiles[y][x] == TILE_DRAGON_BASALT;
                weak += game.map.tiles[y][x] == TILE_DRAGON_WEAK_COLUMN;
            }
        }
        columns_ok &= stable == 2 && weak == 2;
        routes_ok &= reachable(&game.map) + stable + weak == before;
        game.enemy_count = 1;
        game.dragon_treasure_quest_state = 1;
        game_refresh_quest_encounters(&game);
        int gx = -1;
        int gy = -1;
        routes_ok &= find_tile(TILE_DRAGON_TREASURE, &gx, &gy) && map_is_walkable(&game.map, gx, gy);
        const Room *r = &game.map.rooms[game.map.room_count - 1];
        for (int type = DRAGON_BREATH; type <= DRAGON_INFERNO_FIRST; type++) {
            for (int y = r->y; y < r->y + r->h; y += 2) {
                for (int x = r->x; x < r->x + r->w; x += 2) {
                    if (!map_is_walkable(&game.map, x, y) || (x == e->x && y == e->y)) {
                        continue;
                    }
                    game.player.x = x;
                    game.player.y = y;
                    e->attack_phase = 0;
                    attack(type);
                    declared += e->attack_target_x >= 0;
                    withheld += e->attack_target_x < 0;
                    escapes_ok &= escaping(e, x, y, e->move_timer);
                }
            }
        }
    }
    ASSERT("100 generated lairs retain two stable and two breakable columns", columns_ok);
    ASSERT("column placement preserves original walking routes, the summit exit and the goblet objective", routes_ok);
    ASSERT("all signature declarations sampled across 100 lairs leave ordinary walking escapes", escapes_ok);
    ASSERT("generated lairs allow normal attacks at over 90 percent of sampled positions; only boxed strikes are withheld", declared > withheld * 10);
    ASSERT("new cover does not remove ordinary enemies from the generated roster", roster_ok);
}

static void test_rewards_and_saves(void) {
    int victories = 1;
    for (int method = 0; method < 5; method++) {
        kill_with(method, 0);
        victories &= !game.enemies[0].active && game.red_dragon_reward_state == 1 && floor_mantles() == 1 &&
            (game.defeated_bosses & (1 << LOCATION_DRAGONSPINE)) && !game.dragon_treasure_quest_state;
    }
    ASSERT("all five kill paths retain the original mantle drop and boss victory without completing Ilya's quest", victories);
    Item mantle = boss_equipment_reward(ENEMY_RED_DRAGON);
    ASSERT("Dragon Scale Mantle keeps its existing all-class stats", mantle.defense_bonus == 4 && mantle.max_hp_bonus == 20 &&
        mantle.value == 1400 && mantle.class_mask == ITEM_CLASS_ALL);
    kill_with(0, 0);
    game.player.x = game.enemies[0].x;
    game.player.y = game.enemies[0].y;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("ordinary floor pickup marks the mantle claimed and preserves the original 60 gold", game.red_dragon_reward_state == 2 &&
        game.gold == 60 && game.inventory_count == 1 && !floor_mantles());
    game.inventory_count = 0;
    game_open_town_portal(&game);
    game_use_town_portal(&game);
    ASSERT("sold or dropped claimed equipment never regenerates on return", game.red_dragon_reward_state == 2 && !floor_mantles());

    kill_with(0, 1);
    int x = -1;
    int y = -1;
    ASSERT("full loot floor and inventory reserve the mantle without discarding existing drops",
        game.red_dragon_reward_state == 1 && game.floor_item_count == MAX_FLOOR_ITEMS &&
        find_tile(TILE_DRAGON_RELIQUARY_ASH, &x, &y) && game.gold == 60);
    game.player.x = x;
    game.player.y = y;
    game.floor_items[MAX_FLOOR_ITEMS - 1].x = x;
    game.floor_items[MAX_FLOOR_ITEMS - 1].y = y;
    game.floor_items[MAX_FLOOR_ITEMS - 1].underlying_tile = TILE_DRAGON_RELIQUARY_ASH;
    game.map.tiles[y][x] = TILE_ITEM;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("a full pack cannot consume the mantle cache underneath loot", game.red_dragon_reward_state == 1 &&
        game.floor_items[MAX_FLOOR_ITEMS - 1].underlying_tile == TILE_DRAGON_RELIQUARY_ASH);
    ASSERT("reserved mantle and covered cache survive saving", save_game(&game, 99199) && load_game(&loaded, 99199) &&
        loaded.red_dragon_reward_state == 1 && loaded.floor_items[MAX_FLOOR_ITEMS - 1].underlying_tile == TILE_DRAGON_RELIQUARY_ASH);
    game = loaded;
    game.inventory_count--;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("P recovers the reserved mantle and restores the cache's native underlay",
        game.red_dragon_reward_state == 2 && game.inventory[game.inventory_count - 1].max_hp_bonus == 20 &&
        game.floor_items[MAX_FLOOR_ITEMS - 1].underlying_tile == TILE_DRAGON_ASH);
    kill_with(0, 0);
    game_open_town_portal(&game);
    int trip = save_game(&game, 99199) && load_game(&loaded, 99199);
    if (trip) {
        game = loaded;
        game_use_town_portal(&game);
    }
    ASSERT("unclaimed dropped equipment returns as a reserved cache after portal travel and saving", trip &&
        game.red_dragon_reward_state == 1 && find_tile(TILE_DRAGON_RELIQUARY_ASH, &x, &y));
    game.player.x = x - 1;
    game.player.y = y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("A beside a recovered cache claims the original mantle once", game.red_dragon_reward_state == 2 && game.inventory_count == 1);

    Enemy *e = setup();
    attack(DRAGON_INFERNO_FIRST);
    e->frozen_turns = 2;
    cache_arena();
    ASSERT("active and cached cone masks, phase, facing and freeze survive save/load", save_game(&game, 99199) && load_game(&loaded, 99199) &&
        loaded.enemies[0].boss_attack == DRAGON_INFERNO_FIRST && loaded.enemies[0].attack_phase == 2 && loaded.enemies[0].move_timer == 2 &&
        loaded.enemies[0].frozen_turns == 2 && memcmp(loaded.enemies[0].dragon_warning, e->dragon_warning, sizeof(e->dragon_warning)) == 0 &&
        memcmp(loaded.dragonspine_cache[4].enemies[0].dragon_warning, e->dragon_warning, sizeof(e->dragon_warning)) == 0);
    e->frozen_turns = 0;
    game.player.y += 2;
    tick();
    tick();
    cache_arena();
    ASSERT("half-resolved linked breath resumes with the second cone rather than repeating the first", save_game(&game, 99199) &&
        load_game(&loaded, 99199) && loaded.enemies[0].boss_attack == DRAGON_INFERNO_SECOND && loaded.enemies[0].move_timer == 2 &&
        loaded.dragonspine_cache[4].enemies[0].boss_attack == DRAGON_INFERNO_SECOND);
    tick();
    tick();
    cache_arena();
    ASSERT("the complete long recovery survives active and cached saves", save_game(&game, 99199) && load_game(&loaded, 99199) &&
        loaded.enemies[0].move_timer == -2 && loaded.dragonspine_cache[4].enemies[0].move_timer == -2);

    e = setup();
    attack(DRAGON_WINGBEAT);
    game.player.y -= 2;
    tick();
    tick();
    game.map.tiles[28][39] = TILE_DRAGON_CRACKED_COLUMN;
    cache_arena();
    ASSERT("wingbeat position, three-action fire and cracked cover persist in active and cached maps", save_game(&game, 99199) &&
        load_game(&loaded, 99199) && loaded.enemies[0].x == 34 && loaded.enemies[0].y == 28 &&
        loaded.map.tiles[28][34] == TILE_DRAGON_HOT_ASH_3 && loaded.dragonspine_cache[4].map.tiles[28][34] == TILE_DRAGON_HOT_ASH_3 &&
        loaded.map.tiles[28][39] == TILE_DRAGON_CRACKED_COLUMN);
    game.player.x = 34;
    game.player.y = 28;
    red_dragon_cancel_landing(&game, 34, 28);
    ASSERT("portal wards clear landing fire while preserving surrounding hot cells", game.map.tiles[28][34] == TILE_DRAGON_ASH &&
        game.map.tiles[27][34] == TILE_DRAGON_HOT_ASH_3);
    e = setup();
    attack(DRAGON_INFERNO_FIRST);
    int hp = e->hp;
    game_open_town_portal(&game);
    game_use_town_portal(&game);
    ASSERT("return portal cancels a threatening linked signature without healing or advancing the boss", game.location == LOCATION_DRAGONSPINE &&
        game.enemies[0].move_timer == -1 && game.enemies[0].hp == hp && game.enemies[0].attack_target_x == -1);

    e = setup();
    e->hp -= 31;
    e->frozen_turns = 3;
    e->move_timer = 7;
    game.dragon_treasure_quest_state = 1;
    game.gold = 765;
    game.map.tiles[29][38] = TILE_DRAGON_TREASURE;
    map_mark_explored(&game.map, 38, 29);
    cache_arena();
    game.dragonspine_cache[4].enemies[1] = (Enemy){.type = ENEMY_DRAKE, .active = 0};
    game.dragonspine_cache[4].enemy_count = 2;
    game.portal_active = 1;
    game.portal_location = LOCATION_DRAGONSPINE;
    game.portal_level = 5;
    game.portal_x = 33;
    game.portal_y = 26;
    game.portal_origin_tile = TILE_DRAGON_ASH;
    game.map.tiles[26][33] = TILE_DRAGON_ASH;
    game.dragonspine_cache[4].map.tiles[26][33] = TILE_DRAGON_ASH;
    ASSERT("version-135 migration resets obsolete timing without healing, thawing or reviving enemies", legacy_save() && load_game(&loaded, 99199) &&
        loaded.enemies[0].hp == e->hp && loaded.enemies[0].frozen_turns == 3 && loaded.enemies[0].move_timer == 0 &&
        loaded.dragonspine_cache[4].enemies[0].move_timer == 0 && !loaded.dragonspine_cache[4].enemies[1].active);
    ASSERT("migration preserves exploration, gold, goblet quest and an ash portal landing", loaded.gold == 765 &&
        loaded.dragon_treasure_quest_state == 1 && loaded.map.tiles[29][38] == TILE_DRAGON_TREASURE &&
        map_is_explored(&loaded.map, 38, 29) && loaded.map.tiles[26][33] == TILE_DRAGON_ASH);
    e = setup();
    e->x = 18;
    e->y = 28;
    e->hp = 123;
    ASSERT("legacy pursuing dragons return to their lair with their damage preserved", legacy_save() && load_game(&loaded, 99199) &&
        loaded.enemies[0].x >= 20 && loaded.enemies[0].x < 52 && loaded.enemies[0].hp == 123);
    kill_with(0, 0);
    ASSERT("legacy unclaimed mantle drops remain recoverable without duplication", legacy_save() && load_game(&loaded, 99199) &&
        loaded.red_dragon_reward_state == 1 && !loaded.enemies[0].active);
    e = setup();
    e->active = 0;
    game.defeated_bosses |= 1 << LOCATION_DRAGONSPINE;
    game.dragon_treasure_quest_state = 3;
    ASSERT("old completed victories and goblet turn-ins do not manufacture a replacement mantle", legacy_save() && load_game(&loaded, 99199) &&
        loaded.red_dragon_reward_state == 2 && loaded.dragon_treasure_quest_state == 3 && !loaded.enemies[0].active);
    remove("saves/savegame_99199.json");
}

void test_red_dragon(void) {
    printf("Red Dragon encounter tests:\n");
    test_signatures();
    test_cover_and_fire();
    test_generated_arenas();
    test_rewards_and_saves();
}
