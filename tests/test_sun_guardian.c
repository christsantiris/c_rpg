#include "test_utils.h"
#include "../src/game/sun_guardian.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game_enter_temple(&game);
    for (int level = 1; level < TEMPLE_DEPTH; level++) {
        game_descend(&game);
    }
    for (int i = 0; i < game.enemy_count; i++) {
        if (game.enemies[i].type == ENEMY_FALLEN_SUN_GUARDIAN) {
            game.enemies[0] = game.enemies[i];
            break;
        }
    }
    game.enemy_count = 1;
    game.floor_item_count = 0;
    game.inventory_count = 0;
    game.equipped_main_hand = -1;
    game.equipped_off_hand = -1;
    game.equipped_armor = -1;
    game.player.x = 31;
    game.player.y = 9;
    game.player.hp = 1000;
    game.player.max_hp = 1000;
    game.player.mp = 100;
    game.player.max_mp = 100;
    game.player.defense = 0;
    game.player.attack = 100;
    return &game.enemies[0];
}

static void tick(void) {
    action_resolve_enemies_with_projectiles(&game, NULL);
}

static int find_cache(int *x, int *y) {
    for (int row = 7; row <= 11; row++) {
        for (int col = 22; col <= 42; col++) {
            TileType tile = game.map.tiles[row][col];
            for (int i = 0; tile == TILE_ITEM && i < game.floor_item_count; i++) {
                if (game.floor_items[i].active && game.floor_items[i].x == col && game.floor_items[i].y == row) {
                    tile = (TileType)game.floor_items[i].underlying_tile;
                }
            }
            if (tile == TILE_GUARDIAN_RELIQUARY) {
                *x = col;
                *y = row;
                return 1;
            }
        }
    }
    return 0;
}

static void declare(int attack) {
    Enemy *e = &game.enemies[0];
    e->hp = attack == GUARDIAN_LANCE ? e->max_hp : attack == GUARDIAN_RING ? e->max_hp / 2 : e->max_hp / 4;
    e->boss_wave_used = attack == GUARDIAN_RING || attack == GUARDIAN_LUNAR;
    e->move_timer = 0;
    tick();
}

static int escaping(const Enemy *e, int x, int y, int turns) {
    if (!sun_guardian_attack_marks(&game, e, x, y)) {
        return 1;
    }
    if (!turns) {
        return 0;
    }
    static const int offsets[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int n = 0; n < 4; n++) {
        int nx = x + offsets[n][0];
        int ny = y + offsets[n][1];
        if (map_is_walkable(&game.map, nx, ny) && !(nx == e->x && ny == e->y) && escaping(e, nx, ny, turns - 1)) {
            return 1;
        }
    }
    return 0;
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
            game.floor_items[i] = (FloorItem){.active = 1, .x = 23 + i, .y = 11,
                .underlying_tile = TILE_TEMPLE_FLOOR, .item = item_make_health_potion()};
            game.map.tiles[11][23 + i] = TILE_ITEM;
        }
    }
    e->hp = 1;
    game.player.last_dx = 1;
    game.player.last_dy = 0;
    if (method == 0) {
        action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    } else {
        game.player.x = e->x - 2;
        if (method == 1) {
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
}

static int legacy_save(int version) {
    if (!save_game(&game, 99197)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99197.json", "r");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    if (version < 135) {
        cJSON_DeleteItemFromObject(root, "sun_guardian_reward_state");
    }
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99197.json", "w");
    if (!file || !json) {
        if (file) {
            fclose(file);
        }
        free(json);
        return 0;
    }
    int success = fputs(json, file) >= 0;
    fclose(file);
    free(json);
    return success;
}

static void test_mantle_rebalance(void) {
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        for (int mode = 0; mode < 3; mode++) {
            memset(&game, 0, sizeof(game));
            game.player.player_class = cls;
            game_init(&game);
            int base_hp = game.player.max_hp;
            int base_mp = game.player.max_mp;
            Item old = item_make_sunwarden_mantle();
            old.max_hp_bonus = 15;
            old.max_mp_bonus = 15;
            int mantle = game.inventory_count++;
            game.inventory[mantle] = old;
            int other = game.inventory_count++;
            game.inventory[other] = item_make_dragon_scale_mantle();
            if (mode) {
                game.equipped_armor = mantle;
                game_apply_armor_bonuses(&game, &old);
            }
            if (mode == 2) {
                game.player.hp = 3;
                game.player.mp = 2;
            }
            game.floor_item_count = 1;
            game.floor_items[0] = (FloorItem){.active = 1, .x = game.player.x, .y = game.player.y,
                .underlying_tile = game.map.tiles[game.player.y][game.player.x], .item = old};
            game.map.tiles[game.player.y][game.player.x] = TILE_ITEM;
            for (int level = 0; level < CASTLE_DEPTH; level++) {
                game.castle_loot_count[level] = 1;
                game.castle_loot[level][0] = game.floor_items[0];
            }
            game.gold = 432;
            game.sun_guardian_reward_state = 2;
            game.defeated_bosses |= 1 << LOCATION_TEMPLE;
            game.temple_treasure_state = 3;
            game.island_travel_unlocked = 1;
            int ok = legacy_save(150) && load_game(&loaded, 99197);
            int updated = ok && loaded.inventory[mantle].max_hp_bonus == 8 && loaded.inventory[mantle].max_mp_bonus == 7 &&
                loaded.floor_items[0].item.max_hp_bonus == 8 && loaded.floor_items[0].item.max_mp_bonus == 7;
            for (int level = 0; level < CASTLE_DEPTH; level++) {
                updated &= loaded.castle_loot[level][0].item.max_hp_bonus == 8 && loaded.castle_loot[level][0].item.max_mp_bonus == 7;
            }
            ASSERT("old mantles update in inventory, on the floor and in every castle loot cache", updated);
            ASSERT("mantle migration changes only equipped resource caps and clamps excess HP and MP", ok &&
                loaded.player.max_hp == base_hp + (mode ? 8 : 0) && loaded.player.max_mp == base_mp + (mode ? 7 : 0) &&
                loaded.player.hp == (mode == 2 ? 3 : loaded.player.max_hp) && loaded.player.mp == (mode == 2 ? 2 : loaded.player.max_mp) &&
                loaded.player.defense == game.player.defense);
            ASSERT("mantle rebalance preserves other armor, gold, boss victories and reward progress", ok &&
                loaded.inventory[other].max_hp_bonus == game.inventory[other].max_hp_bonus && loaded.gold == 432 &&
                loaded.defeated_bosses == game.defeated_bosses && loaded.sun_guardian_reward_state == 2 && loaded.temple_treasure_state == 3);
            int hp = loaded.player.hp;
            int mp = loaded.player.mp;
            int max_hp = loaded.player.max_hp;
            int max_mp = loaded.player.max_mp;
            ASSERT("resaving migrated mantles never applies the stat reduction twice", save_game(&loaded, 99197) && load_game(&loaded, 99197) &&
                loaded.player.hp == hp && loaded.player.mp == mp && loaded.player.max_hp == max_hp && loaded.player.max_mp == max_mp);
            if (mode) {
                action_resolve_player(&loaded, (Action){ACTION_DROP_ITEM, mantle, 0});
                ASSERT("dropping an equipped migrated mantle restores the original base resource caps", loaded.equipped_armor == -1 && loaded.player.max_hp == base_hp && loaded.player.max_mp == base_mp);
            }
        }
    }
    remove("saves/savegame_99197.json");
}

static void test_guardian_pressure(void) {
    Enemy *e = setup();
    game.player.x = 30;
    game.player.y = 8;
    tick();
    ASSERT("a diagonal attacker receives a fixed two-action lunar warning instead of an empty lance", e->boss_attack == GUARDIAN_LUNAR && e->move_timer == 2 && e->attack_target_x == 30 && e->attack_target_y == 8 && sun_guardian_attack_marks(&game, e, 30, 8) && game.player.hp == 1000);
    action_resolve_player(&game, (Action){ACTION_MOVE, 30, 7});
    tick();
    action_resolve_player(&game, (Action){ACTION_MOVE, 29, 7});
    tick();
    ASSERT("two ordinary steps escape the targeted lunar burst without losing the counterattack window", game.player.hp == 1000 && e->move_timer == -1);

    e = setup();
    tick();
    tick();
    tick();
    tick();
    ASSERT("the opening phase alternates lances with lunar bursts before losing any health", e->hp == e->max_hp && e->attack_phase == 0 && e->boss_attack == GUARDIAN_LUNAR && e->move_timer == 2);

    for (int plate = 0; plate < 2; plate++) {
        e = setup();
        game.player.x = plate ? 35 : 29;
        game.player.y = 8;
        e->boss_wave_used = plate;
        tick();
        ASSERT("camping either sanctuary draws the opposite attack with a full warning", e->boss_attack == (plate ? GUARDIAN_LANES : GUARDIAN_LUNAR) && e->move_timer == 2 && sun_guardian_attack_marks(&game, e, game.player.x, game.player.y) && game.player.hp == 1000);
        tick();
        tick();
        ASSERT("standing still on one sanctuary cannot avoid every Guardian strike", game.player.hp < 1000 && e->move_timer == -1);
    }

    e = setup();
    e->hp--;
    game.player.x = 31;
    game.player.y = 13;
    tick();
    ASSERT("a provoked Guardian marks diagonal ranged attackers in the approach corridor", map_is_walkable(&game.map, 31, 13) && e->boss_attack == GUARDIAN_LUNAR && e->move_timer == 2 && sun_guardian_attack_marks(&game, e, 31, 13));
    e->boss_attack = GUARDIAN_LANCE;
    e->attack_phase = 1;
    e->boss_wave_used = 3;
    e->attack_target_x = 32;
    e->attack_target_y = 9;
    e->boss_aux_x = 32;
    e->boss_aux_y = 11;
    e->facing_dx = 0;
    e->facing_dy = 1;
    game.temple_cache[TEMPLE_DEPTH - 1].valid = 1;
    game.temple_cache[TEMPLE_DEPTH - 1].map = game.map;
    game.temple_cache[TEMPLE_DEPTH - 1].enemy_count = 1;
    game.temple_cache[TEMPLE_DEPTH - 1].enemies[0] = *e;
    ASSERT("existing active and cached warnings load without changing targets, timing or boss wounds", legacy_save(151) && load_game(&loaded, 99197) && loaded.enemies[0].boss_attack == GUARDIAN_LANCE && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].attack_target_x == 32 && loaded.enemies[0].boss_aux_y == 11 && loaded.enemies[0].hp == e->hp && loaded.temple_cache[TEMPLE_DEPTH - 1].enemies[0].boss_wave_used == 3);
    remove("saves/savegame_99197.json");

    e = setup();
    game.player.player_class = CLASS_MAGE;
    game.player.x = 30;
    game.player.y = 7;
    game.player.last_dx = 1;
    game.player.last_dy = 1;
    game.player.mp = 1000;
    game.player.known_spells[0] = spell_make_frost_bolt();
    game.player.known_spell_count = 1;
    game.player.equipped_spell = 0;
    int unfrozen = 1;
    int retaliation = 0;
    int turns = 0;
    while (e->active && turns < 40) {
        action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
        unfrozen &= e->frozen_turns == 0;
        int hp = game.player.hp;
        tick();
        retaliation += game.player.hp < hp;
        turns++;
    }
    ASSERT("repeated diagonal Frost Bolts cannot freeze-lock the Guardian or prevent its retaliation", !e->active && unfrozen && retaliation > 0 && game.sun_guardian_reward_state == 2);
}

void test_sun_guardian(void) {
    printf("Fallen Sun Guardian encounter tests:\n");
    test_mantle_rebalance();
    test_guardian_pressure();
    Enemy *e = setup();
    ASSERT("summit adds two reachable altars and four walkable sanctuary tiles without moving its doors or treasure",
        game.map.tiles[9][29] == TILE_TEMPLE_ALTAR && game.map.tiles[9][35] == TILE_TEMPLE_ALTAR &&
        game.map.tiles[8][29] == TILE_GUARDIAN_MOON_SANCTUARY && game.map.tiles[10][29] == TILE_GUARDIAN_MOON_SANCTUARY &&
        game.map.tiles[8][35] == TILE_GUARDIAN_SUN_SANCTUARY && game.map.tiles[10][35] == TILE_GUARDIAN_SUN_SANCTUARY &&
        map_is_walkable(&game.map, 29, 8) && map_is_walkable(&game.map, 35, 10) &&
        game.map.tiles[12][32] == TILE_TEMPLE_MOON_DOOR_CLOSED && game.map.tiles[6][32] == TILE_TEMPLE_VAULT_DOOR &&
        game.map.tiles[3][32] == TILE_TEMPLE_TREASURE);
    game.player.y = 18;
    tick();
    ASSERT("undamaged Guardian waits for sanctuary entry", e->boss_attack == 0 && e->move_timer == 0);
    game.player.y = 9;
    int hp = game.player.hp;
    EnemyProjectiles shots = {0};
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("adjacent Guardian declares a fixed one-action lance without immediate melee or retaliation",
        e->boss_attack == GUARDIAN_LANCE && e->move_timer == 1 && game.player.hp == hp && shots.count == 0 &&
        sun_guardian_attack_marks(&game, e, 31, 9) && !sun_guardian_attack_marks(&game, e, 31, 10));
    game.player.y = 10;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("sidestep escapes the original ray and its projectile uses the saved endpoint",
        game.player.hp == hp && shots.count == 1 && shots.shots[0].target_x == 22 && shots.shots[0].target_y == 9 && e->move_timer == -1);
    tick();
    ASSERT("one complete action follows resolution before a new declaration", e->move_timer == 0 && game.player.hp == hp);
    tick();
    ASSERT("the next lunar declaration starts only after recovery", e->move_timer == 2 && e->boss_attack == GUARDIAN_LUNAR);

    e = setup();
    declare(GUARDIAN_LANCE);
    tick();
    ASSERT("standing in the warned Sun lance takes normal armored damage", game.player.hp == 1000 - e->attack);
    e = setup();
    game.temple_alignment = 1;
    declare(GUARDIAN_LANCE);
    tick();
    ASSERT("Moon alignment halves a solar hit", game.player.hp == 1000 - e->attack / 2);
    ASSERT("Sun armor reduces every incoming damage path by one third while Moon exposes the boss",
        sun_guardian_enemy_damage(&game, e, 30) == 30);
    game.temple_alignment = 0;
    ASSERT("Sun armor is bounded and never makes the boss invulnerable",
        sun_guardian_enemy_damage(&game, e, 30) == 20 && sun_guardian_enemy_damage(&game, e, 1) == 1);

    e = setup();
    game.player.x = 32;
    game.player.y = 11;
    e->hp--;
    declare(GUARDIAN_LANCE);
    int ex = e->boss_aux_x;
    e->hp = e->max_hp / 2;
    e->frozen_turns = 2;
    tick();
    tick();
    ASSERT("freeze and crossing half health preserve the original lance, endpoint and remaining warning",
        e->move_timer == 1 && e->attack_phase == 0 && e->boss_attack == GUARDIAN_LANCE && e->boss_aux_x == ex);
    game.player.x = 31;
    tick();
    tick();
    tick();
    ASSERT("Cracked Core begins on the next declaration with a two-action ring",
        e->attack_phase == 1 && e->boss_attack == GUARDIAN_RING && e->move_timer == 2 &&
        sun_guardian_attack_marks(&game, e, 29, 9) && !sun_guardian_attack_marks(&game, e, 31, 9));
    int target_x = e->attack_target_x;
    int target_y = e->attack_target_y;
    e->hp = e->max_hp / 4;
    tick();
    ASSERT("quarter-health threshold cannot replace an already warned ring",
        e->attack_phase == 1 && e->move_timer == 1 && e->attack_target_x == target_x && e->attack_target_y == target_y);
    tick();
    tick();
    tick();
    ASSERT("Eclipse starts on the next declaration with a two-action solar cross",
        e->attack_phase == 2 && e->boss_attack == GUARDIAN_LANES && e->move_timer == 2 &&
        sun_guardian_attack_marks(&game, e, game.player.x, game.player.y));
    int warned_x = game.player.x;
    int warned_y = game.player.y;
    game.player.x--;
    tick();
    game.player.y--;
    tick();
    ASSERT("two ordinary moves escape Eclipse lanes without moving their center",
        !sun_guardian_attack_marks(&game, e, game.player.x, game.player.y) && e->move_timer == -1);
    tick();
    tick();
    ASSERT("Eclipse alternates to a separately warned lunar burst at the new fixed target",
        e->boss_attack == GUARDIAN_LUNAR && e->move_timer == 2 &&
        e->attack_target_x == warned_x - 1 && e->attack_target_y == warned_y - 1);
    e->frozen_turns = 1;
    tick();
    ASSERT("Eclipse freeze preserves its alignment and countdown", e->boss_attack == GUARDIAN_LUNAR && e->move_timer == 2);

    int sanctuary_ok = 1;
    for (int attack = GUARDIAN_LANCE; attack <= GUARDIAN_LUNAR; attack++) {
        e = setup();
        declare(attack);
        int x = attack == GUARDIAN_LUNAR ? 35 : 29;
        e->attack_target_x = attack == GUARDIAN_LANCE ? x - 1 : x;
        e->attack_target_y = 8;
        e->facing_dx = 1;
        e->facing_dy = 0;
        e->boss_aux_x = x + 2;
        e->boss_aux_y = 8;
        sanctuary_ok &= sun_guardian_sanctuary_protects(&game, e, x, 8) && !sun_guardian_attack_marks(&game, e, x, 8);
        game.player.x = x;
        game.player.y = 8;
        int before = game.player.hp;
        while (e->move_timer > 0) {
            tick();
        }
        sanctuary_ok &= game.player.hp == before;
    }
    ASSERT("matching sanctuary tiles prevent every solar signature and lunar burst independent of current stance", sanctuary_ok);

    e = setup();
    declare(GUARDIAN_RING);
    int protected = !sun_guardian_attack_marks(&game, e, 29, 8);
    game.map.tiles[8][29] = TILE_TEMPLE_FLOOR;
    ASSERT("the lunar refuge lies inside the actual sunburst ring and its sanctuary removes a threatened cell",
        protected && sun_guardian_attack_marks(&game, e, 29, 8));

    e = setup();
    game.enemy_count = 2;
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_RELIC_SCARABS, .x = 29, .y = 9, .hp = 10, .max_hp = 10};
    declare(GUARDIAN_LANCE);
    game.enemies[1].active = 0;
    ASSERT("removing declared lance cover never extends the warning beyond its original endpoint",
        e->boss_aux_x == 30 && !sun_guardian_attack_marks(&game, e, 29, 9));
    game.player.x = 28;
    shots.count = 0;
    hp = game.player.hp;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("unannounced cells beyond removed cover cannot be hit or reached by the projectile",
        game.player.hp == hp && shots.count == 1 && shots.shots[0].target_x == 30);

    e = setup();
    declare(GUARDIAN_LANCE);
    game.map.tiles[9][30] = TILE_TEMPLE_WALL;
    game.player.x = 29;
    hp = game.player.hp;
    tick();
    ASSERT("new cover can shorten a declared lance", game.player.hp == hp);

    e = setup();
    declare(GUARDIAN_LANCE);
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_RELIC_SCARABS, .x = 30, .y = 9};
    game.enemy_count = 2;
    sun_guardian_cancel_landing(&game, 29, 9);
    ASSERT("portal wards the original lance footprint even if temporary cover could move before the boss",
        e->move_timer == -1 && e->attack_target_x == -1);

    int escapes_ok = 1;
    for (int alignment = 0; alignment < 2; alignment++) {
        for (int attack = GUARDIAN_LANCE; attack <= GUARDIAN_LUNAR; attack++) {
            for (int y = 7; y <= 11; y++) {
                for (int x = 22; x <= 42; x++) {
                    e = setup();
                    if (x == e->x && y == e->y) {
                        continue;
                    }
                    game.player.x = x;
                    game.player.y = y;
                    game.temple_alignment = alignment;
                    declare(attack);
                    escapes_ok &= e->attack_target_x >= 0 && escaping(e, x, y, e->move_timer);
                }
            }
        }
    }
    ASSERT("all four signatures leave walking escapes from every sanctuary position in both alignments", escapes_ok);

    int toggles_ok = 1;
    for (int player_class = CLASS_WARRIOR; player_class <= CLASS_ROGUE; player_class++) {
        e = setup();
        game.player.player_class = player_class;
        game.player.x = 29;
        game.player.y = 9;
        declare(GUARDIAN_LANES);
        int cx = e->attack_target_x;
        int cy = e->attack_target_y;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        tick();
        toggles_ok &= game.temple_alignment == 1 && e->move_timer == 1 && e->attack_target_x == cx && e->attack_target_y == cy &&
            game.temple_sentinels_awakened && game.map.tiles[12][32] == TILE_TEMPLE_MOON_DOOR_OPEN;
    }
    ASSERT("all classes can spend a warning action at an altar without retargeting its fixed strike", toggles_ok);
    e = setup();
    game.player.x = 29;
    game.player.y = 9;
    game_interact_temple(&game);
    game.enemies[game.enemy_count++] = (Enemy){.active = 1, .type = ENEMY_RELIC_SCARABS, .x = 32, .y = 12};
    game_interact_temple(&game);
    ASSERT("Sun switch refuses an occupied lunar door atomically", game.temple_alignment == 1 && game.map.tiles[12][32] == TILE_TEMPLE_MOON_DOOR_OPEN);
    game.enemies[game.enemy_count - 1].active = 0;
    game.map.tiles[9][30] = TILE_TEMPLE_MOON_DOOR_OPEN;
    game.player.x = 30;
    game_interact_temple(&game);
    ASSERT("Sun switch cannot close a doorway on the player", game.temple_alignment == 1 && map_is_walkable(&game.map, 30, 9));
    game.player.x = 29;
    game_interact_temple(&game);
    ASSERT("clearing doorways permits Sun without repetrifying awakened sentinels",
        game.temple_alignment == 0 && game.temple_sentinels_awakened && game_temple_dormant_sentinels(&game) == 0 && game.enemy_count > 1);

    e = setup();
    game.player.x = 29;
    game.player.y = 9;
    game.floor_item_count = 3;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 29, .y = 9, .underlying_tile = TILE_TEMPLE_ALTAR, .item = item_make_health_potion()};
    game.floor_items[1] = (FloorItem){.active = 1, .x = 32, .y = 12, .underlying_tile = TILE_TEMPLE_MOON_DOOR_CLOSED, .item = item_make_health_potion()};
    game.floor_items[2] = game.floor_items[1];
    game.map.tiles[9][29] = TILE_ITEM;
    game.map.tiles[12][32] = TILE_ITEM;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("loot-covered altars still work and doors update every stacked underlay",
        game.temple_alignment == 1 && game.map.tiles[9][29] == TILE_ITEM && game.map.tiles[12][32] == TILE_ITEM &&
        game.floor_items[1].underlying_tile == TILE_TEMPLE_MOON_DOOR_OPEN && game.floor_items[2].underlying_tile == TILE_TEMPLE_MOON_DOOR_OPEN);
    game.enemies[game.enemy_count++] = (Enemy){.active = 1, .type = ENEMY_RELIC_SCARABS, .x = 32, .y = 12};
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("loot cannot hide a door occupant from the Sun switch", game.temple_alignment == 1 && game.floor_items[1].underlying_tile == TILE_TEMPLE_MOON_DOOR_OPEN);
    game.enemies[game.enemy_count - 1].active = 0;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Sun waits for doorway loot to be cleared rather than hiding a closed door beneath walkable items",
        game.temple_alignment == 1 && game.floor_items[1].underlying_tile == TILE_TEMPLE_MOON_DOOR_OPEN);

    e = setup();
    game.player.defense = 10;
    declare(GUARDIAN_LUNAR);
    int expected = (e->attack + 5 - 5) / 2;
    tick();
    tick();
    ASSERT("Sun alignment halves an Eclipse lunar hit after defense", game.player.hp == 1000 - expected);

    int damage_paths = 1;
    for (int method = 0; method < 5; method++) {
        e = setup();
        int base_hp = e->hp;
        if (method == 0) {
            action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
        } else {
            game.player.x = 30;
            game.player.last_dx = 1;
            game.player.last_dy = 0;
            if (method == 1) {
                game.player.player_class = CLASS_ROGUE;
                game.player.arrows = 10;
                game.inventory[0] = item_make_bow();
                game.inventory_count = 1;
                game.equipped_main_hand = 0;
                srand(7);
                action_resolve_player(&game, (Action){ACTION_RANGED_ATTACK, 0, 0});
            } else {
                game.player.player_class = CLASS_MAGE;
                game.player.known_spells[0] = method == 2 ? spell_make_magic_arrow() : method == 3 ? spell_make_fireball() : spell_make_frost_bolt();
                game.player.known_spell_count = 1;
                game.player.equipped_spell = 0;
                action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
            }
        }
        int sun_damage = base_hp - e->hp;
        e->hp = base_hp;
        e->frozen_turns = 0;
        game.temple_alignment = 1;
        srand(7);
        action_resolve_player(&game, method == 0 ? (Action){ACTION_MOVE, e->x, e->y} :
            method == 1 ? (Action){ACTION_RANGED_ATTACK, 0, 0} : (Action){ACTION_CAST_SPELL, 0, 0});
        int moon_damage = base_hp - e->hp;
        damage_paths &= sun_damage == moon_damage * 2 / 3 && game.player.hp == 1000;
    }
    ASSERT("actual melee, arrows and all three damage spells respect Sun armor without immediate retaliation", damage_paths);

    int victories_ok = 1;
    for (int method = 0; method < 5; method++) {
        kill_with(method, 0);
        int mantle = game.inventory_count - 1;
        victories_ok &= !game.enemies[0].active && game.sun_guardian_reward_state == 2 &&
            mantle >= 0 && game.inventory[mantle].visual_id == ITEM_VISUAL_SUNWARDEN_MANTLE &&
            (game.defeated_bosses & (1 << LOCATION_TEMPLE)) && game.map.tiles[6][32] == TILE_TEMPLE_FLOOR && game.temple_treasure_state == 0;
    }
    ASSERT("all five victory paths grant the mantle directly and open the vault without completing Nahla's quest", victories_ok);
    kill_with(0, 1);
    int cache_x = -1;
    int cache_y = -1;
    ASSERT("full floor and inventory reserve the mantle without deleting ordinary loot",
        game.sun_guardian_reward_state == 1 && game.floor_item_count == MAX_FLOOR_ITEMS && find_cache(&cache_x, &cache_y));
    game.player.x = cache_x;
    game.player.y = cache_y;
    game.floor_items[MAX_FLOOR_ITEMS - 1].x = cache_x;
    game.floor_items[MAX_FLOOR_ITEMS - 1].y = cache_y;
    game.floor_items[MAX_FLOOR_ITEMS - 1].underlying_tile = TILE_GUARDIAN_RELIQUARY;
    game.map.tiles[cache_y][cache_x] = TILE_ITEM;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("full inventory leaves a covered mantle cache intact", game.sun_guardian_reward_state == 1);
    ASSERT("pending mantle and covered cache survive saving", save_game(&game, 99197) && load_game(&loaded, 99197) &&
        loaded.sun_guardian_reward_state == 1 && loaded.map.tiles[cache_y][cache_x] == TILE_ITEM);
    game = loaded;
    game.inventory_count--;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("P claims the mantle beneath loot and restores every item's underlay",
        game.sun_guardian_reward_state == 2 && game.inventory[game.inventory_count - 1].visual_id == ITEM_VISUAL_SUNWARDEN_MANTLE &&
        game.floor_items[MAX_FLOOR_ITEMS - 1].underlying_tile == TILE_TEMPLE_FLOOR);
    game.inventory_count--;
    sun_guardian_restore_reward(&game);
    ASSERT("claimed or sold mantles never regenerate", !find_cache(&cache_x, &cache_y) && game.sun_guardian_reward_state == 2);
    kill_with(0, 1);
    find_cache(&cache_x, &cache_y);
    game.inventory_count--;
    game.player.x = cache_x - 1;
    game.player.y = cache_y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("A beside the cache also recovers the reserved mantle", game.sun_guardian_reward_state == 2 && !find_cache(&cache_x, &cache_y));

    kill_with(0, 1);
    game_open_town_portal(&game);
    int pending_trip = save_game(&game, 99197) && load_game(&loaded, 99197);
    if (pending_trip) {
        game = loaded;
        game_use_town_portal(&game);
    }
    game_ascend(&game);
    game_descend(&game);
    ASSERT("a full-pack mantle remains available after a saved town portal and backtracking",
        pending_trip && game.sun_guardian_reward_state == 1 && find_cache(&cache_x, &cache_y));
    game.player.x = cache_x;
    game.player.y = cache_y;
    game.inventory_count--;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    game.inventory_count--;
    game_open_town_portal(&game);
    game_use_town_portal(&game);
    ASSERT("selling a claimed mantle and revisiting the summit cannot manufacture a replacement",
        game.sun_guardian_reward_state == 2 && !find_cache(&cache_x, &cache_y));

    int equipment_ok = 1;
    for (int player_class = CLASS_WARRIOR; player_class <= CLASS_ROGUE; player_class++) {
        setup();
        game.player.player_class = player_class;
        game.inventory_count = 1;
        game.inventory[0] = item_make_sunwarden_mantle();
        int defense = game.player.defense;
        int max_hp = game.player.max_hp;
        int max_mp = game.player.max_mp;
        action_resolve_player(&game, (Action){ACTION_EQUIP_ITEM, 0, 0});
        equipment_ok &= game.equipped_armor == 0 && game.player.defense == defense + 5 &&
            game.player.max_hp == max_hp + 8 && game.player.max_mp == max_mp + 7 &&
            game.inventory[0].class_mask == ITEM_CLASS_ALL && game.inventory[0].armor_family == ARMOR_FAMILY_LIGHT;
    }
    ASSERT("Sunwarden Mantle equips on all classes with exactly +5 defense, +8 HP and +7 MP", equipment_ok);
    ASSERT("equipped mantle metadata and derived stats survive saving",
        save_game(&game, 99197) && load_game(&loaded, 99197) && loaded.equipped_armor == 0 &&
        loaded.inventory[0].visual_id == ITEM_VISUAL_SUNWARDEN_MANTLE && loaded.player.max_hp == game.player.max_hp &&
        loaded.player.max_mp == game.player.max_mp && loaded.player.defense == game.player.defense);

    e = setup();
    declare(GUARDIAN_LUNAR);
    e->frozen_turns = 2;
    game.temple_cache[4].valid = 1;
    game.temple_cache[4].map = game.map;
    game.temple_cache[4].enemy_count = 1;
    game.temple_cache[4].enemies[0] = *e;
    ASSERT("active and cached Eclipse warnings retain targets, phase, freeze and sequence across save/load",
        save_game(&game, 99197) && load_game(&loaded, 99197) && loaded.enemies[0].attack_phase == 2 &&
        loaded.enemies[0].boss_attack == GUARDIAN_LUNAR && loaded.enemies[0].move_timer == 2 &&
        loaded.enemies[0].frozen_turns == 2 && loaded.enemies[0].attack_target_x == e->attack_target_x &&
        loaded.temple_cache[4].enemies[0].boss_attack == GUARDIAN_LUNAR);
    e->frozen_turns = 0;
    int old_boss_hp = e->hp;
    int old_alignment = game.temple_alignment;
    game_open_town_portal(&game);
    ASSERT("temple portal caches the exact warning", game.portal_active && game.portal_location == LOCATION_TEMPLE &&
        game.temple_cache[4].enemies[0].move_timer == 2);
    game_use_town_portal(&game);
    ASSERT("portal landing wards a threatening signature without healing the Guardian or changing alignment",
        game.location == LOCATION_TEMPLE && game.enemies[0].move_timer == -1 &&
        game.enemies[0].hp == old_boss_hp && game.temple_alignment == old_alignment);
    ASSERT("recovery and sanctuary map survive save/load", save_game(&game, 99197) && load_game(&loaded, 99197) &&
        loaded.enemies[0].move_timer == -1 && loaded.map.tiles[8][29] == TILE_GUARDIAN_MOON_SANCTUARY);

    e = setup();
    e->hp -= 30;
    e->move_timer = 17;
    e->frozen_turns = 3;
    game.gold = 654;
    game.temple_treasure_state = 1;
    game.map.tiles[9][29] = TILE_TEMPLE_FLOOR;
    game.map.tiles[8][29] = TILE_TEMPLE_FLOOR;
    map_mark_explored(&game.map, 29, 8);
    game.temple_cache[4].valid = 1;
    game.temple_cache[4].map = game.map;
    game.temple_cache[4].enemy_count = 2;
    game.temple_cache[4].enemies[0] = *e;
    game.temple_cache[4].enemies[1] = (Enemy){.type = ENEMY_RELIC_SCARABS, .active = 0, .hp = 0};
    game.portal_active = 1;
    game.portal_location = LOCATION_TEMPLE;
    game.portal_level = 5;
    game.portal_x = 29;
    game.portal_y = 9;
    game.portal_origin_tile = TILE_TEMPLE_FLOOR;
    ASSERT("version-134 migration preserves health, freeze and deaths while resetting obsolete boss timing",
        legacy_save(134) && load_game(&loaded, 99197) && loaded.enemies[0].hp == e->hp && loaded.enemies[0].frozen_turns == 3 &&
        loaded.enemies[0].move_timer == 0 && loaded.enemies[0].boss_attack == 0 &&
        loaded.temple_cache[4].enemies[0].move_timer == 0 && !loaded.temple_cache[4].enemies[1].active);
    ASSERT("migration adds sanctuaries without overwriting portal landings, exploration or treasure progress",
        loaded.gold == 654 && loaded.temple_treasure_state == 1 && map_is_explored(&loaded.map, 29, 8) &&
        loaded.map.tiles[8][29] == TILE_GUARDIAN_MOON_SANCTUARY && loaded.map.tiles[9][29] == TILE_TEMPLE_FLOOR &&
        loaded.temple_cache[4].map.tiles[9][29] == TILE_TEMPLE_FLOOR && loaded.map.tiles[3][32] == TILE_TEMPLE_TREASURE);
    e = setup();
    e->active = 0;
    game.defeated_bosses |= 1 << LOCATION_TEMPLE;
    game.temple_treasure_state = 3;
    ASSERT("old victories remain defeated with Nahla's completed quest and no fabricated mantle",
        legacy_save(134) && load_game(&loaded, 99197) && !loaded.enemies[0].active &&
        loaded.temple_treasure_state == 3 && loaded.sun_guardian_reward_state == 0);
    remove("saves/savegame_99197.json");
}
