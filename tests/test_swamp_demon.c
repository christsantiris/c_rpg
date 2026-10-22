#include "test_utils.h"
#include "../src/game/swamp_demon.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = LOCATION_SWAMP;
    game.level = SWAMP_BOSS_LEVEL;
    game.max_swamp_level_reached = SWAMP_BOSS_LEVEL;
    game.map.room_count = 2;
    game.map.rooms[0] = (Room){2, 2, 8, 8};
    game.map.rooms[1] = (Room){20, 10, 14, 12};
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            game.map.tiles[y][x] = TILE_SWAMP_WALL;
        }
    }
    for (int room = 0; room < 2; room++) {
        const Room *r = &game.map.rooms[room];
        for (int y = r->y + 1; y < r->y + r->h - 1; y++) {
            for (int x = r->x + 1; x < r->x + r->w - 1; x++) {
                game.map.tiles[y][x] = TILE_SWAMP_FLOOR;
            }
        }
    }
    for (int x = 5; x <= 20; x++) {
        game.map.tiles[16][x] = TILE_SWAMP_FLOOR;
    }
    for (int y = 5; y <= 16; y++) {
        game.map.tiles[y][5] = TILE_SWAMP_FLOOR;
    }
    game.map.stairs_up_x = 5;
    game.map.stairs_up_y = 5;
    game.map.tiles[5][5] = TILE_STAIRS_UP;
    game.map.stairs_down_x = 27;
    game.map.stairs_down_y = 16;
    game.map.tiles[16][27] = TILE_SWAMP_FLOOR;
    game.player.x = 23;
    game.player.y = 16;
    game.player.hp = 1000;
    game.player.max_hp = 1000;
    game.player.mp = 100;
    game.player.max_mp = 100;
    game.player.defense = 0;
    game.inventory_count = 0;
    game.equipped_main_hand = -1;
    game.equipped_off_hand = -1;
    game.equipped_armor = -1;
    game.enemy_count = 1;
    game.floor_item_count = 0;
    game.enemies[0] = (Enemy){.type = ENEMY_SWAMP_DEMON, .name = "Swamp Demon", .x = 28, .y = 16,
        .active = 1, .is_boss = 1, .hp = 180, .max_hp = 180, .attack = 20, .defense = 7,
        .experience = 550, .attack_target_x = -1, .attack_target_y = -1, .boss_aux_x = -1, .boss_aux_y = -1};
    swamp_demon_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    return &game.enemies[0];
}

static int find_tile(TileType tile, int *x, int *y) {
    for (int row = 11; row < 21; row++) {
        for (int col = 21; col < 33; col++) {
            if (game.map.tiles[row][col] == tile) {
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

static void test_rewards(void) {
    int victories = 1;
    for (int method = 0; method < 5; method++) {
        kill_with(method);
        int swords = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            swords += game.floor_items[i].active && strcmp(game.floor_items[i].item.name, "Demonic Sword") == 0;
        }
        victories &= !game.enemies[0].active && swords == 1 && game.swamp_demon_reward_state == 1 && (game.defeated_bosses & (1 << LOCATION_SWAMP));
    }
    ASSERT("melee, bow, Magic Arrow, Fireball and Frost Bolt each guarantee one Demonic Sword", victories);
    Enemy *e = setup();
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 100;
    e->hp = 1;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 22, .y = 20, .underlying_tile = TILE_SWAMP_FLOOR, .item = item_make_health_potion()};
    }
    game.floor_item_count = MAX_FLOOR_ITEMS;
    game.map.tiles[20][22] = TILE_ITEM;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        game.inventory[i] = item_make_health_potion();
    }
    game.inventory_count = MAX_INVENTORY;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    int x = 0;
    int y = 0;
    ASSERT("full floor capacity preserves every item and reserves reward in a reliquary", game.floor_item_count == MAX_FLOOR_ITEMS && game.floor_items[MAX_FLOOR_ITEMS - 1].item.type == ITEM_POTION_HEALTH && game.swamp_demon_reward_state == 1 && find_tile(TILE_DEMON_RELIQUARY, &x, &y));
    game.player.x = x;
    game.player.y = y;
    game.map.tiles[y][x + 1] = TILE_DEMON_VENT;
    game.map.tiles[y - 1][x] = TILE_DEMON_VENT;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("full inventory cannot consume the reserved reward", game.swamp_demon_reward_state == 1 && game.map.tiles[y][x] == TILE_DEMON_RELIQUARY);
    game.inventory_count--;
    int reward_score = game.score;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("reliquary pickup grants original Demonic Sword and marks it claimed", game.swamp_demon_reward_state == 2 && strcmp(game.inventory[MAX_INVENTORY - 1].name, "Demonic Sword") == 0 && game.inventory[MAX_INVENTORY - 1].attack_bonus == item_make_demonic_sword().attack_bonus && game.score == reward_score + game.inventory[MAX_INVENTORY - 1].value);
    swamp_demon_restore_reward(&game);
    ASSERT("claiming cannot recreate a second reward or operate adjacent tide controls", !find_tile(TILE_DEMON_RELIQUARY, &x, &y) && game.map.tiles[game.player.y - 1][game.player.x] == TILE_DEMON_VENT);
    kill_with(0);
    game.player.x = game.enemies[0].x;
    game.player.y = game.enemies[0].y;
    for (int i = 0; i < 3; i++) {
        action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    }
    ASSERT("normal ground pickup also records a claimed reward", game.swamp_demon_reward_state == 2);
}

static int make_legacy_save(void) {
    if (!save_game(&game, 99183)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99183.json", "r");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 127);
    cJSON_DeleteItemFromObject(root, "swamp_demon_reward_state");
    cJSON *arrays[2] = {cJSON_GetObjectItem(root, "enemies"),
        cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "swamp_cache"), 3), "enemies")};
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
    file = fopen("saves/savegame_99183.json", "w");
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

static int rot_count(void) {
    int count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            count += swamp_demon_rot_turns(game.map.tiles[y][x]) > 0;
        }
    }
    return count;
}

static void prepare_pool(Enemy *e, int phase) {
    e->hp = phase ? 90 : 180;
    e->attack_phase = phase;
    e->boss_attack = DEMON_CLEAVE;
    e->move_timer = 0;
    action_resolve_enemies(&game);
}

static void test_patterns(void) {
    Enemy *e = setup();
    game.player.x = 27;
    action_resolve_enemies(&game);
    ASSERT("frontal cleave warns three fixed tiles for one complete action", e->boss_attack == DEMON_CLEAVE && e->move_timer == 1 && game.player.hp == 1000 && swamp_demon_attack_marks(&game, e, 27, 16) && swamp_demon_attack_marks(&game, e, 27, 15) && !swamp_demon_attack_marks(&game, e, 26, 16));
    e->hp = 90;
    e->frozen_turns = 2;
    action_resolve_player(&game, (Action){ACTION_MOVE, 26, 16});
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze and half-health crossing preserve queued cleave and phase", e->move_timer == 1 && e->attack_phase == 0 && e->attack_target_x == 27);
    action_resolve_enemies(&game);
    ASSERT("one ordinary step back escapes cleave without melee retaliation", game.player.hp == 1000 && e->move_timer == -1);
    game.player.x = 27;
    game.player.attack = 10;
    int hp = e->hp;
    action_resolve_player(&game, (Action){ACTION_MOVE, 28, 16});
    action_resolve_enemies(&game);
    ASSERT("recovery permits an adjacent counterattack without extra damage", e->hp < hp && game.player.hp == 1000 && e->move_timer == 0);
    action_resolve_enemies(&game);
    ASSERT("next preparation announces phase two and twin two-turn rot pockets", e->attack_phase == 1 && e->boss_attack == DEMON_POOL && e->move_timer == 2 && e->boss_aux_x >= 0);
    int ax = e->attack_target_x;
    int ay = e->attack_target_y;
    int bx = e->boss_aux_x;
    int by = e->boss_aux_y;
    action_resolve_player(&game, (Action){ACTION_MOVE, 26, 16});
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, 25, 16});
    action_resolve_enemies(&game);
    ASSERT("two ordinary moves escape both locked pools and create at most ten rot tiles", game.player.hp == 1000 && e->move_timer == -1 && rot_count() > 0 && rot_count() <= 10 && abs(ax - bx) + abs(ay - by) >= 4);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("rot fully expires before phase-two miasma fan prepares", rot_count() == 0 && e->boss_attack == DEMON_FAN && e->move_timer == 2);
    game.player.x = 27;
    game.player.y = 16;
    ASSERT("miasma fan has a fixed three-tile wedge", swamp_demon_attack_marks(&game, e, 27, 16) && swamp_demon_attack_marks(&game, e, 25, 18) && !swamp_demon_attack_marks(&game, e, 24, 16));
    action_resolve_player(&game, (Action){ACTION_MOVE, 27, 17});
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, 27, 18});
    action_resolve_enemies(&game);
    ASSERT("two ordinary sideways moves escape miasma fan", game.player.hp == 1000 && e->move_timer == -1);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("phase two cycles cleave, pools and fan without moving the Demon", e->boss_attack == DEMON_CLEAVE && e->x == 28 && e->y == 16);

    e = setup();
    game.player.x = 27;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("standing in a cleave takes one base-strength signature hit", game.player.hp == 980 && e->move_timer == -1);
    e = setup();
    prepare_pool(e, 0);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("pool eruption deals one signature hit with two visible lingering actions", game.player.hp == 980 && game.map.tiles[16][23] == TILE_DEMON_ROT_2 && rot_count() == 5);
    e->frozen_turns = 4;
    action_resolve_enemies(&game);
    ASSERT("active rot damages and counts down while the boss is frozen", game.player.hp == 972 && game.map.tiles[16][23] == TILE_DEMON_ROT_1 && e->move_timer == -1);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("two lingering hits then expiry restore native mud, with no hidden poison", game.player.hp == 964 && game.map.tiles[16][23] == TILE_SWAMP_FLOOR && rot_count() == 0);

    e = setup();
    prepare_pool(e, 0);
    game.floor_item_count = 2;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 23, .y = 16, .underlying_tile = TILE_SWAMP_FLOOR, .item = item_make_health_potion()};
    }
    game.map.tiles[16][23] = TILE_ITEM;
    game.map.tiles[16][22] = TILE_SWAMP_DAUGHTER;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("pool creation updates every stacked loot underlay without replacing a quest tile", game.map.tiles[16][23] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_DEMON_ROT_2 && game.floor_items[1].underlying_tile == TILE_DEMON_ROT_2 && game.map.tiles[16][22] == TILE_SWAMP_DAUGHTER);
    e->active = 0;
    game.player.y = 18;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("rot expires beneath all loot copies even after the Demon dies", game.floor_items[0].underlying_tile == TILE_SWAMP_FLOOR && game.floor_items[1].underlying_tile == TILE_SWAMP_FLOOR && game.floor_items[0].active && game.floor_items[1].active);

    e = setup();
    prepare_pool(e, 0);
    int original_x = e->attack_target_x;
    int original_y = e->attack_target_y;
    e->hp = 90;
    e->frozen_turns = 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze pauses pool preparation without creating active rot", e->move_timer == 2 && rot_count() == 0 && e->boss_aux_x == -1);
    game.player.y++;
    action_resolve_enemies(&game);
    game.player.x--;
    action_resolve_enemies(&game);
    ASSERT("crossing half health cannot widen a queued single pool", e->attack_phase == 0 && rot_count() == 5 && game.map.tiles[original_y][original_x] == TILE_DEMON_ROT_2 && game.player.hp == 1000);

    e = setup();
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_BANDIT, .x = 26, .y = 16, .hp = 10, .max_hp = 10, .frozen_turns = 5};
    game.enemy_count = 2;
    prepare_pool(e, 0);
    game.enemies[1].active = 0;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("an ally blocks targeting and its removal cannot retarget a queued pool", e->attack_target_x == -1 && game.player.hp == 1000 && rot_count() == 0);
    e = setup();
    game.player.x = 23;
    game.player.y = 11;
    game.map.tiles[16][27] = TILE_SWAMP_WALL;
    prepare_pool(e, 0);
    ASSERT("closed diagonal corners block targeting", e->attack_target_x == -1);

    e = setup();
    game.map.tiles[16][26] = TILE_SWAMP_WALL;
    prepare_pool(e, 0);
    ASSERT("solid cover prevents targeting", e->attack_target_x == -1);
    game.map.tiles[16][26] = TILE_SWAMP_FLOOR;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("removing cover cannot create an unannounced pool", game.player.hp == 1000 && rot_count() == 0);
    e = setup();
    e->hp--;
    prepare_pool(e, 0);
    e->hp--;
    game.player.x = 15;
    action_resolve_enemies(&game);
    ASSERT("distant retreat pauses warning without healing or relocating Demon", e->move_timer == 2 && e->hp == 179 && e->x == 28 && e->attack_target_x == 23);
    game.player.x = 23;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("return resumes the original warning", game.player.hp == 980 && e->move_timer == -1);
    e = setup();
    game.player.x = 26;
    for (int y = 15; y <= 17; y++) {
        for (int x = 25; x <= 27; x++) {
            game.map.tiles[y][x] = TILE_SWAMP_WALL;
        }
    }
    game.map.tiles[16][26] = TILE_SWAMP_FLOOR;
    game.map.tiles[16][27] = TILE_SWAMP_FLOOR;
    prepare_pool(e, 0);
    ASSERT("a trapped player receives an empty warning instead of an unavoidable pool", e->attack_target_x == -1);
}

static void test_vents(void) {
    Enemy *e = setup();
    int x = 0;
    int y = 0;
    ASSERT("arena starts with accessible root vents", find_tile(TILE_DEMON_VENT, &x, &y));
    game.player.x = x;
    game.player.y = y;
    game.floor_item_count = 2;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_DEMON_VENT, .item = item_make_health_potion()};
    }
    game.map.tiles[y][x] = TILE_ITEM;
    prepare_pool(e, 1);
    game.map.tiles[y][x + 1] = TILE_DEMON_ROT_2;
    game.map.tiles[y][x + 3] = TILE_DEMON_ROT_2;
    ASSERT("Interact recognizes root vents beneath stacked loot", game_has_regional_interaction(&game));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("vent cancels the queued attack, drains radius two, and exposes two full actions", e->move_timer == -2 && e->attack_target_x == -1 && game.map.tiles[y][x + 1] == TILE_SWAMP_FLOOR && game.map.tiles[y][x + 3] == TILE_DEMON_ROT_1 && game.floor_items[0].underlying_tile == TILE_DEMON_VENT_4 && game.floor_items[1].underlying_tile == TILE_DEMON_VENT_4);
    ASSERT("vent gives neither healing nor mana", game.player.hp == 1000 && game.player.mp == 100);
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("cooling vent cannot be spammed to extend recovery", e->move_timer == -1 && game.floor_items[0].underlying_tile == TILE_DEMON_VENT_3);
    action_resolve_enemies(&game);
    ASSERT("second full counterattack action finishes safely", e->move_timer == 0 && game.player.hp == 1000);
    e->frozen_turns = 5;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("vent cooldown completes independently of boss freeze", game.floor_items[0].underlying_tile == TILE_DEMON_VENT);
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("recovered vent can be opened again", game.floor_items[0].underlying_tile == TILE_DEMON_VENT_5 && e->move_timer == -3);
    swamp_demon_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    ASSERT("idempotent arena preparation preserves vent cooldown", game.floor_items[0].underlying_tile == TILE_DEMON_VENT_5);
    e->frozen_turns = 0;
    e->boss_attack = DEMON_POOL;
    e->move_timer = 2;
    e->attack_target_x = x;
    e->attack_target_y = y;
    ASSERT("standing on a vent does not grant signature immunity", swamp_demon_attack_marks(&game, e, x, y));
    int classes = 1;
    for (int c = 0; c < 3; c++) {
        setup();
        find_tile(TILE_DEMON_VENT, &x, &y);
        game.player.player_class = c == 0 ? CLASS_WARRIOR : c == 1 ? CLASS_ROGUE : CLASS_MAGE;
        game.player.x = x;
        game.player.y = y;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        classes &= game.map.tiles[y][x] == TILE_DEMON_VENT_5 && game.inventory_count == 0;
    }
    ASSERT("all classes can open vents without gear or consumables", classes);
}

static void test_persistence(void) {
    Enemy *e = setup();
    int x = 0;
    int y = 0;
    find_tile(TILE_DEMON_VENT, &x, &y);
    game.player.x = x;
    game.player.y = y;
    swamp_demon_interact(&game);
    game.map.tiles[16][23] = TILE_DEMON_ROT_2;
    e->hp = 79;
    e->attack_phase = 1;
    e->boss_attack = DEMON_POOL;
    e->move_timer = 2;
    e->frozen_turns = 2;
    e->attack_target_x = 23;
    e->attack_target_y = 16;
    e->boss_aux_x = 30;
    e->boss_aux_y = 19;
    LevelCache *cache = &game.swamp_cache[SWAMP_BOSS_LEVEL - 1];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = 1;
    cache->enemies[0] = *e;
    game.swamp_demon_reward_state = 1;
    int ok = save_game(&game, 99183) && load_game(&loaded, 99183);
    ASSERT("save preserves active phase, HP, freeze and both locked pool targets", ok && loaded.enemies[0].hp == 79 && loaded.enemies[0].attack_phase == 1 && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].frozen_turns == 2 && loaded.enemies[0].attack_target_x == 23 && loaded.enemies[0].boss_aux_x == 30 && loaded.enemies[0].boss_aux_y == 19);
    ASSERT("save preserves active and cached rot duration, vent cooldown and reward", ok && loaded.map.tiles[y][x] == TILE_DEMON_VENT_5 && loaded.swamp_cache[SWAMP_BOSS_LEVEL - 1].map.tiles[16][23] == TILE_DEMON_ROT_2 && loaded.swamp_cache[SWAMP_BOSS_LEVEL - 1].enemies[0].boss_aux_y == 19 && loaded.swamp_demon_reward_state == 1);
    e->move_timer = -2;
    game.swamp_demon_reward_state = 2;
    ok = save_game(&game, 99183) && load_game(&loaded, 99183);
    ASSERT("save preserves two-action recovery and claimed sword", ok && loaded.enemies[0].move_timer == -2 && loaded.swamp_demon_reward_state == 2);
    game.portal_active = 1;
    game.portal_location = LOCATION_SWAMP;
    game.portal_level = SWAMP_BOSS_LEVEL;
    game.portal_x = 23;
    game.portal_y = 16;
    game.portal_origin_tile = TILE_DEMON_ROT_2;
    game.location = LOCATION_TOWN2;
    game_use_town_portal(&game);
    ASSERT("portal revisit restores cached queued attack, rot and recovering vents", game.location == LOCATION_SWAMP && game.enemies[0].move_timer == 2 && game.map.tiles[16][23] == TILE_DEMON_ROT_2 && game.map.tiles[y][x] == TILE_DEMON_VENT_5);

    e = setup();
    for (int yy = 11; yy < 21; yy++) {
        for (int xx = 21; xx < 33; xx++) {
            if (game.map.tiles[yy][xx] == TILE_DEMON_VENT) {
                game.map.tiles[yy][xx] = TILE_SWAMP_FLOOR;
            }
        }
    }
    e->hp = 79;
    e->move_timer = 37;
    game.gold = 321;
    game.innkeeper_quest_state = 2;
    map_mark_explored(&game.map, 22, 12);
    game.level = 4;
    game.max_swamp_level_reached = 4;
    cache = &game.swamp_cache[3];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = 1;
    cache->enemies[0] = *e;
    cache->enemies[0].hp = 51;
    game.portal_active = 1;
    game.portal_location = LOCATION_SWAMP;
    game.portal_level = 4;
    game.portal_x = 25;
    game.portal_y = 18;
    game.portal_origin_tile = TILE_SWAMP_FLOOR;
    ok = make_legacy_save() && load_game(&loaded, 99183);
    ASSERT("version 127 migration adds vents without healing active or cached Demon", ok && loaded.enemies[0].hp == 79 && loaded.swamp_cache[SWAMP_BOSS_LEVEL - 1].enemies[0].hp == 51 && loaded.enemies[0].move_timer == 0 && loaded.enemies[0].attack_target_x == -1);
    ASSERT("migration preserves rescue, exploration, gold, position and hidden portal landing", ok && loaded.innkeeper_quest_state == 2 && loaded.gold == 321 && loaded.player.x == 23 && loaded.player.y == 16 && map_is_explored(&loaded.map, 22, 12) && loaded.map.tiles[18][25] == TILE_SWAMP_FLOOR && loaded.swamp_cache[SWAMP_BOSS_LEVEL - 1].map.tiles[18][25] == TILE_SWAMP_FLOOR);
    game.enemies[0].active = 0;
    cache->enemies[0].active = 0;
    game.defeated_bosses |= 1 << LOCATION_SWAMP;
    ok = make_legacy_save() && load_game(&loaded, 99183);
    ASSERT("legacy victories stay defeated without fabricating a sword", ok && !loaded.enemies[0].active && loaded.swamp_demon_reward_state == 2);
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 24, .y = 12, .underlying_tile = TILE_SWAMP_FLOOR, .item = item_make_demonic_sword()};
    game.map.tiles[12][24] = TILE_ITEM;
    ok = make_legacy_save() && load_game(&loaded, 99183);
    ASSERT("known legacy dropped sword becomes persistently recoverable", ok && loaded.swamp_demon_reward_state == 1 && loaded.floor_items[0].active);
    game.floor_items[0].item.sharpened = 1;
    ok = make_legacy_save() && load_game(&loaded, 99183);
    ASSERT("a deliberately dropped sharpened sword does not establish a new reward", ok && loaded.swamp_demon_reward_state == 2 && loaded.floor_items[0].active);
    remove("saves/savegame_99183.json");
}

static int escapes(Enemy *e, int x, int y, int turns) {
    if (!swamp_demon_attack_marks(&game, e, x, y)) {
        return 1;
    }
    if (!turns) {
        return 0;
    }
    static const int steps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int n = 0; n < 4; n++) {
        int nx = x + steps[n][0];
        int ny = y + steps[n][1];
        if (map_is_walkable(&game.map, nx, ny) && !(nx == e->x && ny == e->y) && escapes(e, nx, ny, turns - 1)) {
            return 1;
        }
    }
    return 0;
}

static void test_layouts(void) {
    static unsigned char seen[MAP_H][MAP_W];
    static int queue[MAP_W * MAP_H];
    int valid = 1;
    int safe = 1;
    for (int seed = 0; seed < 100; seed++) {
        setup();
        srand((unsigned)seed);
        map_generate_swamp(&game.map, SWAMP_BOSS_LEVEL);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        enemies_spawn(&game);
        Enemy *e = &game.enemies[0];
        memset(seen, 0, sizeof(seen));
        int head = 0;
        int tail = 0;
        queue[tail++] = game.player.y * MAP_W + game.player.x;
        seen[game.player.y][game.player.x] = 1;
        static const int steps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        while (head < tail) {
            int cell = queue[head++];
            int x = cell % MAP_W;
            int y = cell / MAP_W;
            for (int n = 0; n < 4; n++) {
                int nx = x + steps[n][0];
                int ny = y + steps[n][1];
                if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H || seen[ny][nx] || !map_is_walkable(&game.map, nx, ny)) {
                    continue;
                }
                seen[ny][nx] = 1;
                queue[tail++] = ny * MAP_W + nx;
            }
        }
        int vents = 0;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (game.map.tiles[y][x] == TILE_DEMON_VENT) {
                    vents++;
                    valid &= seen[y][x] && map_is_walkable(&game.map, x, y);
                }
            }
        }
        valid &= vents == 3 && seen[game.map.stairs_down_y][game.map.stairs_down_x];
        const Room *room = &game.map.rooms[game.map.room_count - 1];
        for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
            for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                if (!map_is_walkable(&game.map, x, y) || (e->x == x && e->y == y)) {
                    continue;
                }
                for (int attack = DEMON_CLEAVE; attack <= DEMON_FAN; attack++) {
                    game.player.x = x;
                    game.player.y = y;
                    e->attack_phase = 1;
                    e->hp = e->max_hp / 2;
                    e->boss_attack = attack == DEMON_CLEAVE ? DEMON_FAN : attack == DEMON_POOL ? DEMON_CLEAVE : DEMON_POOL;
                    e->move_timer = 0;
                    swamp_demon_tick(&game, e);
                    safe &= escapes(e, x, y, e->move_timer);
                }
            }
        }
    }
    ASSERT("100 generated clearings retain three reachable vents and both exits", valid);
    ASSERT("every preparation from every interior tile in 100 arenas allows a walking escape within its countdown", safe);
    kill_with(0);
    game.floor_item_count = 0;
    map_generate_swamp(&game.map, SWAMP_BOSS_LEVEL);
    enemies_spawn(&game);
    game_refresh_quest_encounters(&game);
    int reward = 0;
    int living = 0;
    for (int i = 0; i < game.enemy_count; i++) {
        living |= game.enemies[i].active && game.enemies[i].type == ENEMY_SWAMP_DEMON;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            reward |= game.map.tiles[y][x] == TILE_DEMON_RELIQUARY;
        }
    }
    ASSERT("regenerating the arena restores unclaimed sword without reviving the Demon", !living && reward && game.swamp_demon_reward_state == 1);
}

void test_swamp_demon(void) {
    printf("Swamp Demon phased encounter tests:\n");
    test_patterns();
    test_vents();
    test_rewards();
    test_persistence();
    test_layouts();
}
