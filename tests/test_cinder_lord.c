#include "expedition_fixture.h"
#include "test_utils.h"
#include "../src/game/cinder_lord.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = LOCATION_ASHEN;
    game.level = ASHEN_DEPTH;
    game.max_ashen_level_reached = ASHEN_DEPTH;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            game.map.tiles[y][x] = TILE_ASHEN_WALL;
        }
    }
    game.map.room_count = 1;
    game.map.rooms[0] = (Room){20, 15, 16, 14};
    for (int y = 15; y < 29; y++) {
        for (int x = 20; x < 36; x++) {
            game.map.tiles[y][x] = TILE_ASHEN_FLOOR;
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
    game.enemies[0] = (Enemy){.type = ENEMY_CINDER_LORD, .name = "Cinder Lord", .x = 28, .y = 22,
        .active = 1, .is_boss = 1, .hp = 260, .max_hp = 260, .attack = 24, .defense = 9,
        .experience = 750, .attack_target_x = -1, .attack_target_y = -1, .boss_aux_x = -1, .boss_aux_y = -1};
    cinder_lord_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
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
    if (!save_game(&game, 99193)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99193.json", "r");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 132);
    test_mark_legacy_portal(root);
    cJSON_DeleteItemFromObject(root, "cinder_lord_reward_pending");
    cJSON *arrays[2] = {cJSON_GetObjectItem(root, "enemies"),
        cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "ashen_cache"), ASHEN_DEPTH - 1), "enemies")};
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
    file = fopen("saves/savegame_99193.json", "w");
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
    LevelCache *cache = &game.ashen_cache[ASHEN_DEPTH - 1];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = game.enemy_count;
    memcpy(cache->enemies, game.enemies, sizeof(Enemy) * (size_t)game.enemy_count);
}

static void test_signatures(void) {
    Enemy *e = setup();
    EnemyProjectiles shots = {0};
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("an adjacent Cinder Lord prepares a fixed one-action lance without extra melee", e->boss_attack == CINDER_LANCE && e->move_timer == 1 && game.player.hp == 1000 && !shots.count);
    int tx = e->attack_target_x;
    int ty = e->attack_target_y;
    e->hp = 130;
    e->frozen_turns = 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze and crossing half health preserve the original phase-one warning", e->move_timer == 1 && !e->attack_phase && e->attack_target_x == tx && e->attack_target_y == ty);
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x, game.player.y + 1});
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("an ordinary sidestep escapes a lance whose projectile retains the locked endpoint", game.player.hp == 1000 && e->move_timer == -1 && shots.count == 1 && shots.shots[0].target_x == tx && shots.shots[0].target_y == ty);
    action_resolve_enemies(&game);
    ASSERT("the Cinder Lord leaves a whole counterattack action without retaliation", e->move_timer == 0 && game.player.hp == 1000);
    action_resolve_enemies(&game);
    ASSERT("Broken Crucible is announced before the next fixed two-action eruption cross", e->attack_phase == 1 && e->boss_attack == CINDER_ERUPTION && e->move_timer == 2);
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x - 1, game.player.y});
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x - 1, game.player.y});
    action_resolve_enemies(&game);
    int x;
    int y;
    ASSERT("two normal moves escape heat and leave finite H2 patches", game.player.hp == 1000 && e->move_timer == -1 && find_tile(TILE_CINDER_HOT_2, &x, &y));
    action_resolve_enemies(&game);
    ASSERT("lingering heat count down to H1 during recovery", find_tile(TILE_CINDER_HOT_1, &x, &y));
    action_resolve_enemies(&game);
    ASSERT("heat expire before separately warned expanding ember rings", !find_tile(TILE_CINDER_HOT_1, &x, &y) && e->boss_attack == CINDER_RING && e->move_timer == 2);
    e = setup();
    game.player.defense = 4;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("standing in a lance takes one base attack using half defense", game.player.hp == 978 && e->move_timer == -1);
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
    game.map.tiles[22][26] = TILE_ASHEN_LAVA;
    action_resolve_enemies(&game);
    game.map.tiles[22][26] = TILE_ASHEN_FLOOR;
    action_resolve_enemies(&game);
    ASSERT("removing cover cannot expose an unmarked target after declaration", game.player.hp == 1000 && e->attack_target_x == -1);
    e = setup();
    game.map.tiles[21][27] = TILE_ASHEN_WALL;
    game.map.tiles[23][27] = TILE_ASHEN_WALL;
    game.map.tiles[22][26] = TILE_ASHEN_WALL;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("boxed-in players receive an empty swing rather than unavoidable damage", game.player.hp == 1000 && e->attack_target_x == -1);
    e = setup();
    game.map.tiles[22][27] = TILE_ITEM;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 27, .y = 22, .underlying_tile = TILE_CINDER_HOT_2, .item = item_make_health_potion()};
    }
    game.floor_item_count = 2;
    e->frozen_turns = 3;
    action_resolve_enemies(&game);
    ASSERT("hot clocks tick beneath both stacked objects while the boss is frozen", game.player.hp == 992 && game.map.tiles[22][27] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_CINDER_HOT_1 && game.floor_items[1].underlying_tile == TILE_CINDER_HOT_1);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("hot patches expire after two actions without losing loot or adding damage", game.player.hp == 984 && game.floor_item_count == 2 && game.floor_items[0].underlying_tile == TILE_ASHEN_FLOOR && game.floor_items[1].underlying_tile == TILE_ASHEN_FLOOR);
}

static void test_rewards(void) {
    int victories = 1;
    for (int method = 0; method < 5; method++) {
        kill_with(method);
        int x;
        int y;
        victories &= !game.enemies[0].active && game.cinder_lord_reward_pending == 3 && find_tile(TILE_CINDER_RELIQUARY, &x, &y) && (game.defeated_bosses & (1 << LOCATION_ASHEN));
    }
    ASSERT("melee, bow and all three attack spells each reserve one weapon and one Strength Potion", victories);
    Enemy *e = setup();
    e->hp = 1;
    game.player.attack = 100;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 20, .y = 15, .underlying_tile = TILE_ASHEN_FLOOR, .item = item_make_health_potion()};
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
    int cache = find_tile(TILE_CINDER_RELIQUARY, &x, &y);
    ASSERT("full floor and inventory preserve every object and both guaranteed rewards", cache && game.floor_item_count == MAX_FLOOR_ITEMS && game.floor_items[MAX_FLOOR_ITEMS - 1].item.type == ITEM_POTION_HEALTH && game.cinder_lord_reward_pending == 3 && game.gold > 0);
    game.player.x = x;
    game.player.y = y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("a full inventory cannot consume either reserved reward", game.cinder_lord_reward_pending == 3 && game.map.tiles[y][x] == TILE_CINDER_RELIQUARY);
    game.inventory_count--;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("one free slot claims the weapon while the Strength Potion stays reserved", game.cinder_lord_reward_pending == CINDER_POTION_PENDING && strcmp(game.inventory[MAX_INVENTORY - 1].name, "Embercleaver") == 0 && game.map.tiles[y][x] == TILE_CINDER_RELIQUARY);
    int ok = save_game(&game, 99193) && load_game(&loaded, 99193);
    ASSERT("saving a partially claimed cache preserves the remaining potion without another weapon", ok && loaded.cinder_lord_reward_pending == CINDER_POTION_PENDING && loaded.map.tiles[y][x] == TILE_CINDER_RELIQUARY && strcmp(loaded.inventory[MAX_INVENTORY - 1].name, "Embercleaver") == 0);
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_CINDER_RELIQUARY, .item = item_make_health_potion()};
    }
    game.map.tiles[y][x] = TILE_ITEM;
    game.inventory_count--;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("pickup reaches a covered cache, claims the potion and preserves both loot underlays", !game.cinder_lord_reward_pending && game.inventory[MAX_INVENTORY - 1].type == ITEM_POTION_STRENGTH && game.floor_items[0].underlying_tile == TILE_ASHEN_FLOOR && game.floor_items[1].underlying_tile == TILE_ASHEN_FLOOR && game.floor_item_count == MAX_FLOOR_ITEMS && game.map.tiles[y][x] == TILE_ITEM);
    Item weapon = item_make_embercleaver();
    ASSERT("Embercleaver is a Warrior-only two-handed axe with +11 attack, 25 percent penetration and cleave", item_class_allowed(&weapon, CLASS_WARRIOR) && !item_class_allowed(&weapon, CLASS_MAGE) && !item_class_allowed(&weapon, CLASS_ROGUE) && weapon.type == ITEM_WEAPON && weapon.weapon_family == WEAPON_FAMILY_AXE && weapon.weapon_hands == WEAPON_HANDS_TWO && weapon.attack_bonus == 11 && weapon.armor_penetration_percent == 25 && weapon.cleave_percent == 25 && weapon.visual_id == ITEM_VISUAL_EMBERCLEAVER);
    kill_with(0);
    find_tile(TILE_CINDER_RELIQUARY, &x, &y);
    game.player.x = x;
    game.player.y = y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("opening spills both rewards without claiming them", game.inventory_count == 0);
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("a spacious pack collects both released rewards through Pick Up", !game.cinder_lord_reward_pending && game.inventory_count == 2 && strcmp(game.inventory[0].name, "Embercleaver") == 0 && game.inventory[1].type == ITEM_POTION_STRENGTH);
    int attack = game.player.attack;
    action_resolve_player(&game, (Action){ACTION_USE_ITEM, 1, 0});
    ASSERT("the original special Strength Potion still grants one permanent base attack", game.player.attack == attack + 1);
    game.inventory_count = 0;
    game_open_town_portal(&game);
    test_restore_cached_floor(&game);
    ok = save_game(&game, 99193) && load_game(&loaded, 99193);
    ASSERT("consumed, sold or discarded rewards cannot regenerate after travel or saving", ok && !loaded.cinder_lord_reward_pending && loaded.floor_item_count == 0 && !loaded.enemies[0].active);
}

static Enemy *prepare_eruption(int x, int y) {
    Enemy *e = setup();
    game.player.x = x;
    game.player.y = y;
    e->boss_attack = CINDER_LANCE;
    action_resolve_enemies(&game);
    return e;
}

static void test_sluices(void) {
    int x;
    int y;
    setup();
    ASSERT("the final chamber has separate walkable forge sluices and cooling channels", find_tile(TILE_CINDER_SLUICE, &x, &y) && map_is_walkable(&game.map, x, y) && game.map.tiles[y + 1][x] == TILE_CINDER_CHANNEL);
    int all_classes = 1;
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        Enemy *e = prepare_eruption(28, 17);
        game.player.player_class = cls;
        all_classes &= e->boss_attack == CINDER_ERUPTION && e->move_timer == 2 && cinder_lord_attack_turns(&game, e, 28, 18) == 2;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        action_resolve_enemies(&game);
        all_classes &= e->move_timer == -2 && e->attack_target_x == -1 && game.map.tiles[17][28] == TILE_CINDER_SLUICE_4 && game.map.tiles[18][28] == TILE_CINDER_COOL_4;
        action_resolve_enemies(&game);
        all_classes &= e->move_timer == -1 && game.player.hp == 1000;
        action_resolve_enemies(&game);
        all_classes &= e->move_timer == 0 && game.player.hp == 1000;
    }
    ASSERT("every class can redirect a channel-feeding eruption for two whole counterattack actions", all_classes);
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("opening a cooling sluice cannot reset its visible cooldown", game.map.tiles[17][28] == TILE_CINDER_SLUICE_2 && game.map.tiles[18][28] == TILE_CINDER_COOL_2);
    cinder_lord_tick_floor(&game);
    cinder_lord_tick_floor(&game);
    ASSERT("channels and sluices finish cooling together and can be used again", game.map.tiles[17][28] == TILE_CINDER_SLUICE && game.map.tiles[18][28] == TILE_CINDER_CHANNEL);
    Enemy *e = prepare_eruption(28, 17);
    int tx = e->attack_target_x;
    game.player.x = 28;
    game.player.y = 26;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("the wrong sluice cannot cancel an eruption feeding the other channel", e->move_timer == 2 && e->attack_target_x == tx && game.map.tiles[26][28] == TILE_CINDER_SLUICE_5);
    e = setup();
    action_resolve_enemies(&game);
    tx = e->attack_target_x;
    game.player.x = 28;
    game.player.y = 17;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("an off-timing sluice cools its channel but cannot cancel a lance", e->boss_attack == CINDER_LANCE && e->move_timer == 1 && e->attack_target_x == tx);
    e = prepare_eruption(28, 18);
    game.map.tiles[18][28] = TILE_CINDER_COOL_4;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("a still-cooling channel is safe from eruption damage and retains its own timer", game.player.hp == 1000 && game.map.tiles[18][28] == TILE_CINDER_COOL_2);
    e = prepare_eruption(28, 18);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("an uncooled channel remains part of the warned eruption without becoming permanent lava", game.player.hp == 976 && game.map.tiles[18][28] == TILE_CINDER_CHANNEL);
    e = prepare_eruption(28, 17);
    game.floor_items[0] = (FloorItem){.active = 1, .x = 28, .y = 17, .underlying_tile = TILE_CINDER_SLUICE, .item = item_make_health_potion()};
    game.floor_items[1] = (FloorItem){.active = 1, .x = 28, .y = 18, .underlying_tile = TILE_CINDER_CHANNEL, .item = item_make_health_potion()};
    game.floor_item_count = 2;
    game.map.tiles[17][28] = TILE_ITEM;
    game.map.tiles[18][28] = TILE_ITEM;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("loot-covered controls and channels retain redirection and all underlay clocks", e->move_timer == -2 && game.floor_item_count == 2 && game.floor_items[0].underlying_tile == TILE_CINDER_SLUICE_4 && game.floor_items[1].underlying_tile == TILE_CINDER_COOL_4);
}

static Enemy *prepare_ring(void) {
    Enemy *e = setup();
    e->hp = 130;
    e->boss_attack = CINDER_ERUPTION;
    action_resolve_enemies(&game);
    return e;
}

static void test_ring(void) {
    Enemy *e = prepare_ring();
    ASSERT("Broken Crucible locks and warns inner-one and outer-two ring fronts at declaration", e->attack_phase == 1 && e->boss_attack == CINDER_RING && e->move_timer == 2 && e->attack_target_x == e->x && e->attack_target_y == e->y && cinder_lord_attack_turns(&game, e, 27, 22) == 1 && cinder_lord_attack_turns(&game, e, 26, 22) == 2 && !cinder_lord_attack_turns(&game, e, 25, 22));
    action_resolve_player(&game, (Action){ACTION_MOVE, 26, 22});
    action_resolve_enemies(&game);
    ASSERT("walking outward escapes the inner hit while the already warned outer front counts down", e->move_timer == 1 && game.player.hp == 1000 && !cinder_lord_attack_turns(&game, e, 27, 22) && cinder_lord_attack_turns(&game, e, 26, 22) == 1 && game.map.tiles[22][27] == TILE_CINDER_HOT_2);
    action_resolve_player(&game, (Action){ACTION_MOVE, 25, 22});
    action_resolve_enemies(&game);
    ASSERT("a second ordinary step escapes the outer front and leaves a counterattack opening", game.player.hp == 1000 && e->move_timer == -1 && game.map.tiles[22][26] == TILE_CINDER_HOT_2);
    action_resolve_enemies(&game);
    ASSERT("the next entire action has no Cinder Lord retaliation", game.player.hp == 1000 && e->move_timer == 0);
    action_resolve_enemies(&game);
    ASSERT("phase two alternates expanding rings with separately warned eruptions", e->boss_attack == CINDER_ERUPTION && e->move_timer == 2);
    e = prepare_ring();
    action_resolve_enemies(&game);
    ASSERT("standing in the inner ring receives exactly one base-strength signature hit", game.player.hp == 976 && e->move_timer == 1);
    game.player.x = 26;
    action_resolve_enemies(&game);
    ASSERT("the outer ring deals damage on its own declared second action", game.player.hp == 952 && e->move_timer == -1);
    e = prepare_ring();
    action_resolve_player(&game, (Action){ACTION_MOVE, 26, 22});
    action_resolve_enemies(&game);
    e->frozen_turns = 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freezing after the inner front pauses only the saved outer countdown while old heat expires", e->move_timer == 1 && e->attack_target_x == 28 && game.map.tiles[22][27] == TILE_ASHEN_FLOOR && game.player.hp == 1000);
    game.player.x = 25;
    action_resolve_enemies(&game);
    ASSERT("thaw resumes the original outer footprint without repeating the inner hit", e->move_timer == -1 && game.map.tiles[22][27] == TILE_ASHEN_FLOOR && game.map.tiles[22][26] == TILE_CINDER_HOT_2 && game.player.hp == 1000);
    e = prepare_ring();
    game.player.x = 28;
    game.player.y = 17;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("a sluice never cancels a separately warned ember ring", e->move_timer == 2 && e->boss_attack == CINDER_RING && e->attack_target_x == 28);
}

static void test_weapon(void) {
    setup();
    game.inventory[0] = item_make_embercleaver();
    game.inventory[1] = item_make_buckler();
    game.inventory_count = 2;
    game.equipped_off_hand = 1;
    action_resolve_player(&game, (Action){ACTION_EQUIP_ITEM, 0, 0});
    ASSERT("equipping Embercleaver clears the off hand like every two-handed weapon", game.equipped_main_hand == 0 && game.equipped_off_hand == -1);
    game.player.attack = 30;
    game.enemies[0].defense = 20;
    game.enemies[0].hp = 260;
    game.enemies[1] = (Enemy){.type = ENEMY_OBSIDIAN_GUARDIAN, .active = 1, .x = 28, .y = 23, .hp = 100, .max_hp = 100};
    game.enemy_count = 2;
    action_resolve_player(&game, (Action){ACTION_MOVE, 28, 22});
    ASSERT("Embercleaver applies its actual armor penetration and cleaves an adjacent target", game.enemies[0].hp == 245 && game.enemies[1].hp == 93);
    int ok = save_game(&game, 99193) && load_game(&loaded, 99193);
    ASSERT("equipped Embercleaver persists its attack, penetration, cleave, hands and visual", ok && loaded.equipped_main_hand == 0 && loaded.inventory[0].attack_bonus == 11 && loaded.inventory[0].armor_penetration_percent == 25 && loaded.inventory[0].cleave_percent == 25 && loaded.inventory[0].weapon_hands == WEAPON_HANDS_TWO && loaded.inventory[0].visual_id == ITEM_VISUAL_EMBERCLEAVER);
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
        if (!cinder_lord_attack_turns(&game, e, x, y)) {
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
        map_generate_ashen(&game.map, ASHEN_DEPTH);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        enemies_spawn(&game);
        Enemy *e = &game.enemies[0];
        game.enemy_count = 1;
        const Room *room = &game.map.rooms[game.map.room_count - 1];
        int sluices = 0;
        for (int y = room->y; y < room->y + room->h; y++) {
            for (int x = room->x; x < room->x + room->w; x++) {
                sluices += game.map.tiles[y][x] == TILE_CINDER_SLUICE;
                if (!map_is_walkable(&game.map, x, y) || (x == e->x && y == e->y)) {
                    continue;
                }
                for (int attack = CINDER_LANCE; attack <= CINDER_RING; attack++) {
                    e->boss_attack = attack == CINDER_LANCE ? CINDER_RING : attack - 1;
                    e->hp = e->max_hp / 2;
                    e->attack_phase = 1;
                    e->move_timer = 0;
                    game.player.x = x;
                    game.player.y = y;
                    cinder_lord_tick(&game, e, NULL);
                    escapes &= escape_steps(e) <= e->move_timer;
                    if (attack == CINDER_RING && cinder_lord_attack_turns(&game, e, x, y)) {
                        int route = 0;
                        static const int steps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
                        for (int first = 0; first < 4; first++) {
                            int nx = x + steps[first][0];
                            int ny = y + steps[first][1];
                            if (!map_is_walkable(&game.map, nx, ny) || (nx == e->x && ny == e->y) || cinder_lord_attack_turns(&game, e, nx, ny) == 1) {
                                continue;
                            }
                            for (int second = 0; second < 4; second++) {
                                int ex = nx + steps[second][0];
                                int ey = ny + steps[second][1];
                                route |= map_is_walkable(&game.map, ex, ey) && !(ex == e->x && ey == e->y) && !cinder_lord_attack_turns(&game, e, ex, ey);
                            }
                        }
                        escapes &= route;
                    }
                }
            }
        }
        props &= sluices == 2;
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
    ASSERT("100 native Ashen layouts place exactly two sluices without touching the boss or exits", props);
    ASSERT("every arena floor in 100 layouts has a walking escape within its declared warning", escapes);
    ASSERT("all 100 sluice layouts preserve the entrance-to-boss-to-exit route", paths);
}

static void test_saves(void) {
    Enemy *e = prepare_ring();
    action_resolve_player(&game, (Action){ACTION_MOVE, 26, 22});
    action_resolve_enemies(&game);
    e->frozen_turns = 2;
    cache_arena();
    int ok = save_game(&game, 99193) && load_game(&loaded, 99193);
    ASSERT("active and cached saves preserve the half-resolved ring, phase, target and freeze", ok && loaded.enemies[0].boss_attack == CINDER_RING && loaded.enemies[0].move_timer == 1 && loaded.enemies[0].attack_phase == 1 && loaded.enemies[0].frozen_turns == 2 && loaded.enemies[0].attack_target_x == 28 && loaded.ashen_cache[4].enemies[0].move_timer == 1 && loaded.ashen_cache[4].map.tiles[22][27] == TILE_CINDER_HOT_2);
    e = prepare_eruption(28, 17);
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    game.map.tiles[20][27] = TILE_CINDER_HOT_2;
    game.map.tiles[20][26] = TILE_CINDER_HOT_1;
    cache_arena();
    ok = save_game(&game, 99193) && load_game(&loaded, 99193);
    ASSERT("active and cached maps save cooling channels, sluice cooldowns, hot clocks and both counterattack actions", ok && loaded.map.tiles[18][28] == TILE_CINDER_COOL_4 && loaded.map.tiles[17][28] == TILE_CINDER_SLUICE_4 && loaded.map.tiles[20][27] == TILE_CINDER_HOT_2 && loaded.map.tiles[20][26] == TILE_CINDER_HOT_1 && loaded.enemies[0].move_timer == -2 && loaded.ashen_cache[4].map.tiles[18][28] == TILE_CINDER_COOL_4);
    game.player.x = 27;
    game.player.y = 20;
    game_open_town_portal(&game);
    ok = save_game(&game, 99193) && load_game(&loaded, 99193);
    if (ok) {
        game = loaded;
        test_restore_cached_floor(&game);
    }
    ASSERT("escaping preserves archived heat and other arena clocks", ok && game.location == LOCATION_ASHEN && game.map.tiles[20][27] == TILE_CINDER_HOT_2 && game.map.tiles[18][28] == TILE_CINDER_COOL_4 && game.map.tiles[17][28] == TILE_CINDER_SLUICE_4 && game.enemies[0].move_timer == -2 && game.player.hp == 1000);
    e = setup();
    action_resolve_enemies(&game);
    Enemy escaped_lance = game.enemies[0];
    game_open_town_portal(&game);
    test_restore_cached_floor(&game);
    ASSERT("escaping retains the archived lance warning without advancing it", game.enemies[0].move_timer == escaped_lance.move_timer && game.enemies[0].attack_target_x == escaped_lance.attack_target_x && game.player.hp == 1000);
    e = setup();
    e->hp = 73;
    e->move_timer = 17;
    e->frozen_turns = 3;
    game.enemies[1] = (Enemy){.type = ENEMY_OBSIDIAN_GUARDIAN, .name = "Obsidian Guardian", .active = 0, .hp = 0, .max_hp = 100, .x = 24, .y = 24};
    game.enemy_count = 2;
    game.gold = 321;
    game.emberforge_quest_state = 2;
    game.emberforge_progress = 3;
    game.emberforge_encounters = 3;
    game.map.tiles[18][23] = TILE_EMBERFORGE_LIT;
    game.map.tiles[20][23] = TILE_ASHEN_RUIN;
    game.portal_active = 1;
    game.portal_location = LOCATION_ASHEN;
    game.portal_level = ASHEN_DEPTH;
    game.portal_x = 28;
    game.portal_y = 17;
    game.portal_origin_tile = TILE_ASHEN_FLOOR;
    for (int y = 15; y < 29; y++) {
        for (int x = 20; x < 36; x++) {
            if (game.map.tiles[y][x] == TILE_CINDER_SLUICE || game.map.tiles[y][x] == TILE_CINDER_CHANNEL) {
                game.map.tiles[y][x] = TILE_ASHEN_FLOOR;
            }
        }
    }
    map_mark_explored(&game.map, 24, 24);
    cache_arena();
    ok = make_legacy_save() && load_game(&loaded, 99193);
    ASSERT("version-132 migration resets old Cinder AI while preserving boss damage, freeze and defeated enemies", ok && loaded.enemies[0].hp == 73 && loaded.enemies[0].move_timer == 0 && loaded.enemies[0].attack_target_x == -1 && loaded.enemies[0].frozen_turns == 3 && !loaded.enemies[1].active && loaded.ashen_cache[4].enemies[0].hp == 73 && !loaded.ashen_cache[4].enemies[1].active);
    ASSERT("migration preserves Emberforge progress, repaired forge, ruined paving, exploration, gold and portal destination", ok && loaded.emberforge_quest_state == 2 && loaded.emberforge_progress == 3 && loaded.emberforge_encounters == 3 && loaded.map.tiles[18][23] == TILE_EMBERFORGE_LIT && loaded.map.tiles[20][23] == TILE_ASHEN_RUIN && map_is_explored(&loaded.map, 24, 24) && loaded.gold == 321 && loaded.player.x == 27 && loaded.map.tiles[17][28] == TILE_ASHEN_FLOOR && loaded.ashen_cache[4].map.tiles[17][28] == TILE_ASHEN_FLOOR);
    setup();
    game.enemies[0].active = 0;
    game.defeated_bosses |= 1 << LOCATION_ASHEN;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 28, .y = 22, .underlying_tile = TILE_ASHEN_FLOOR, .item = item_make_strength_potion()};
    game.floor_item_count = 1;
    game.map.tiles[22][28] = TILE_ITEM;
    cache_arena();
    ok = make_legacy_save() && load_game(&loaded, 99193);
    ASSERT("old victories retain the original potion without a repeat weapon, potion or boss spawn", ok && !loaded.cinder_lord_reward_pending && !loaded.enemies[0].active && !loaded.ashen_cache[4].enemies[0].active && loaded.floor_item_count == 1 && loaded.floor_items[0].item.type == ITEM_POTION_STRENGTH);
}

void test_cinder_lord(void) {
    printf("Cinder Lord encounter tests:\n");
    test_signatures();
    test_sluices();
    test_ring();
    test_rewards();
    test_weapon();
    test_layouts();
    test_saves();
    remove("saves/savegame_99193.json");
}
