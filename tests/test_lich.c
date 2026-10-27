#include "test_utils.h"
#include "../src/game/lich.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = LOCATION_DUNGEON;
    game.level = DUNGEON_DEPTH;
    game.max_level_reached = DUNGEON_DEPTH;
    game.map.room_count = 2;
    game.map.rooms[0] = (Room){2, 2, 8, 8};
    game.map.rooms[1] = (Room){20, 10, 14, 12};
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            game.map.tiles[y][x] = TILE_WALL;
        }
    }
    for (int room = 0; room < 2; room++) {
        const Room *r = &game.map.rooms[room];
        for (int y = r->y + 1; y < r->y + r->h - 1; y++) {
            for (int x = r->x + 1; x < r->x + r->w - 1; x++) {
                game.map.tiles[y][x] = TILE_FLOOR;
            }
        }
    }
    for (int x = 5; x <= 20; x++) {
        game.map.tiles[16][x] = TILE_FLOOR;
    }
    for (int y = 5; y <= 16; y++) {
        game.map.tiles[y][5] = TILE_FLOOR;
    }
    game.map.stairs_up_x = 5;
    game.map.stairs_up_y = 5;
    game.map.tiles[5][5] = TILE_STAIRS_UP;
    game.map.stairs_down_x = 27;
    game.map.stairs_down_y = 16;
    game.map.tiles[16][27] = TILE_DUNGEON_STAIRS_SEALED;
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
    game.enemies[0] = (Enemy){.type = ENEMY_LICH_KING, .name = "Lich King", .x = 28, .y = 16,
        .active = 1, .is_boss = 1, .hp = 140, .max_hp = 140, .attack = 18, .defense = 6,
        .experience = 400, .attack_target_x = -1, .attack_target_y = -1, .boss_aux_x = -1, .boss_aux_y = -1};
    lich_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
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

static void test_patterns(void) {
    Enemy *e = setup();
    int hp = game.player.hp;
    action_resolve_enemies(&game);
    ASSERT("Lich declares a fixed lance without immediate damage", e->boss_attack == LICH_LANCE && e->move_timer == 1 && game.player.hp == hp && lich_attack_marks(&game, e, 23, 16));
    game.player.y++;
    action_resolve_enemies(&game);
    ASSERT("ordinary sidestep avoids lance and opens recovery", game.player.hp == hp && e->move_timer == -1);
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 10;
    int boss_hp = e->hp;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    action_resolve_enemies(&game);
    ASSERT("melee recovery hit has no extra Lich retaliation", e->hp < boss_hp && game.player.hp == hp && e->move_timer == 0);

    e = setup();
    game.map.tiles[16][26] = TILE_LICH_PILLAR;
    EnemyProjectiles shots;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("pillar hides every lance tile behind it", !lich_attack_marks(&game, e, game.player.x, game.player.y) && lich_attack_marks(&game, e, 27, 16));
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("cover blocks damage and clips projectile to last open tile", game.player.hp == 1000 && shots.count == 1 && shots.shots[0].target_x == 27 && shots.shots[0].target_y == 16);

    e = setup();
    action_resolve_enemies(&game);
    e->hp = 70;
    e->frozen_turns = 2;
    int target_x = e->attack_target_x;
    game.player.y++;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze preserves pending lance, countdown and old health phase", e->move_timer == 1 && e->attack_phase == 0 && e->attack_target_x == target_x && e->attack_target_y == 16);
    action_resolve_enemies(&game);
    ASSERT("crossing half health does not enlarge an existing warning", e->attack_phase == 0 && e->boss_attack == LICH_LANCE && game.player.hp == 1000);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("next preparation announces phase two and one warned wave", e->attack_phase == 1 && e->boss_attack == LICH_SUMMON && e->boss_wave_used && e->move_timer == 1 && game.enemy_count == 1);
    action_resolve_enemies(&game);
    ASSERT("wave spawns at most two skeletons and they do not attack immediately", game.enemy_count == 3 && game.player.hp == 1000 && game.enemies[1].experience == 0 && game.enemies[1].revived);
    for (int i = 1; i < game.enemy_count; i++) {
        game.enemies[i].active = 0;
    }
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("phase two warns its exact ring for two turns", e->boss_attack == LICH_RING && e->move_timer == 2 && lich_attack_marks(&game, e, 28, 13) && !lich_attack_marks(&game, e, 28, 15));
    game.player.x = 28;
    game.player.y = 13;
    action_resolve_enemies(&game);
    ASSERT("ring still permits an action after its first countdown", game.player.hp == 1000 && e->move_timer == 1);
    game.player.y = 14;
    action_resolve_enemies(&game);
    ASSERT("one ordinary step reaches ring's safe center", game.player.hp == 1000 && e->move_timer == -1);
    for (int turn = 0; turn < 30; turn++) {
        action_resolve_enemies(&game);
    }
    ASSERT("phase-two wave cannot repeat", game.enemy_count == 3);

    e = setup();
    e->attack_phase = 1;
    e->boss_wave_used = 1;
    e->boss_attack = LICH_RING;
    e->move_timer = 1;
    game.player.x = 28;
    game.player.y = 13;
    action_resolve_enemies(&game);
    ASSERT("remaining on warned ring receives base damage plus both seal bonuses", game.player.hp == 978);
    e = setup();
    for (int y = 11; y < 21; y++) {
        for (int x = 21; x < 33; x++) {
            if (game.map.tiles[y][x] == TILE_LICH_SEAL) {
                game.map.tiles[y][x] = TILE_LICH_SEAL_SPENT;
            }
        }
    }
    e->attack_phase = 1;
    e->boss_wave_used = 1;
    e->boss_attack = LICH_RING;
    e->move_timer = 1;
    game.player.x = 28;
    game.player.y = 13;
    action_resolve_enemies(&game);
    ASSERT("silenced seals remove ring damage bonuses", game.player.hp == 982);

    e = setup();
    game.player.x = 18;
    action_resolve_enemies(&game);
    ASSERT("untouched Lich stays dormant outside its chamber", e->move_timer == 0);
    e->hp--;
    action_resolve_enemies(&game);
    ASSERT("a ranged hit from the doorway activates a warned retaliation", e->move_timer == 1 && e->attack_target_x == 18);
    game.player.x = 2;
    action_resolve_enemies(&game);
    ASSERT("distant retreat clears targets without healing or moving Lich", e->move_timer == 0 && e->attack_target_x == -1 && e->hp == 139 && e->x == 28 && e->y == 16);
}

static void test_seals(void) {
    Enemy *e = setup();
    ASSERT("two seals give partial protection while every hit still damages", lich_active_seals(&game) == 2 && lich_enemy_damage(&game, e, 100) == 70 && lich_enemy_damage(&game, e, 1) == 1);
    int x;
    int y;
    find_tile(TILE_LICH_SEAL, &x, &y);
    game.floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_LICH_SEAL, .item = item_make_health_potion()};
    game.floor_item_count = 1;
    game.map.tiles[y][x] = TILE_ITEM;
    game.player.x = x;
    game.player.y = y + 1;
    ASSERT("contextual move-left recognizes a seal beneath loot", game_has_regional_interaction(&game));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("seal remains interactable beneath loot without losing that loot", lich_active_seals(&game) == 1 && game.floor_items[0].active && game.floor_items[0].underlying_tile == TILE_LICH_SEAL_SPENT && lich_enemy_damage(&game, e, 100) == 85);
    find_tile(TILE_LICH_SEAL, &x, &y);
    game.player.x = x;
    game.player.y = y + 1;
    e->move_timer = 2;
    e->boss_attack = LICH_RING;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("last seal cancels danger and grants two complete recovery turns", lich_active_seals(&game) == 0 && lich_enemy_damage(&game, e, 100) == 100 && e->move_timer == -2 && e->attack_target_x == -1);
    lich_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    ASSERT("saved/spent seals are never relit by arena setup", lich_active_seals(&game) == 0);
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
        int blades = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            blades += game.floor_items[i].active && strcmp(game.floor_items[i].item.name, "Cryptblade") == 0;
        }
        victories &= !game.enemies[0].active && blades == 1 && game.lich_reward_state == 1 && (game.defeated_bosses & (1 << LOCATION_DUNGEON));
    }
    ASSERT("melee, bow, Magic Arrow, Fireball and Frost Bolt each guarantee one Cryptblade", victories);
    Enemy *e = setup();
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 100;
    e->hp = 1;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 22, .y = 20, .underlying_tile = TILE_FLOOR, .item = item_make_health_potion()};
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
    ASSERT("full floor capacity preserves every item and reserves reward in a reliquary", game.floor_item_count == MAX_FLOOR_ITEMS && game.floor_items[MAX_FLOOR_ITEMS - 1].item.type == ITEM_POTION_HEALTH && game.lich_reward_state == 1 && find_tile(TILE_LICH_RELIQUARY, &x, &y));
    game.player.x = x;
    game.player.y = y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("full inventory cannot consume the reserved reward", game.lich_reward_state == 1 && game.map.tiles[y][x] == TILE_LICH_RELIQUARY);
    game.inventory_count--;
    int reward_score = game.score;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("reliquary pickup grants original Cryptblade and marks it claimed", game.lich_reward_state == 2 && strcmp(game.inventory[MAX_INVENTORY - 1].name, "Cryptblade") == 0 && game.inventory[MAX_INVENTORY - 1].attack_bonus == 2 && game.score == reward_score + game.inventory[MAX_INVENTORY - 1].value);
    lich_restore_reward(&game);
    ASSERT("claiming cannot recreate a second reward", !find_tile(TILE_LICH_RELIQUARY, &x, &y));
    kill_with(0);
    game.player.x = game.enemies[0].x;
    game.player.y = game.enemies[0].y;
    for (int i = 0; i < 3; i++) {
        action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    }
    ASSERT("normal ground pickup also records a claimed reward", game.lich_reward_state == 2);
    setup();
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_SKELETON, .name = "Lich Shade", .hp = 1, .max_hp = 10, .x = 23, .y = 15, .revived = 1, .experience = 0};
    game.enemy_count = 2;
    game.player.attack = 100;
    int xp = game.player.experience;
    action_resolve_player(&game, (Action){ACTION_MOVE, 23, 15});
    ASSERT("summoned skeleton grants neither loot nor experience", !game.enemies[1].active && game.floor_item_count == 0 && game.player.experience == xp);
}

static void test_persistence(void) {
    Enemy *e = setup();
    e->hp = 60;
    e->attack_phase = 1;
    e->boss_wave_used = 1;
    e->boss_attack = LICH_RING;
    e->move_timer = 2;
    e->attack_target_x = e->x;
    e->attack_target_y = e->y;
    e->boss_aux_x = 22;
    e->boss_aux_y = 12;
    game.level_cache[4].valid = 1;
    game.level_cache[4].map = game.map;
    game.level_cache[4].enemy_count = game.enemy_count;
    game.level_cache[4].enemies[0] = *e;
    int ok = save_game(&game, 99176) && load_game(&loaded, 99176);
    ASSERT("save restores active and cached phase, warning, wave and auxiliary targets", ok && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].boss_attack == LICH_RING && loaded.enemies[0].boss_wave_used && loaded.enemies[0].boss_aux_x == 22 && loaded.level_cache[4].enemies[0].boss_aux_y == 12 && loaded.level_cache[4].enemies[0].attack_phase == 1);
    e->move_timer = -2;
    game.lich_reward_state = 1;
    ok = save_game(&game, 99176) && load_game(&loaded, 99176);
    ASSERT("save preserves recovery and unclaimed reward state", ok && loaded.enemies[0].move_timer == -2 && loaded.lich_reward_state == 1);
    game.lich_reward_state = 2;
    ok = save_game(&game, 99176) && load_game(&loaded, 99176);
    ASSERT("save preserves claimed rewards", ok && loaded.lich_reward_state == 2);
    remove("saves/savegame_99176.json");
}

static void test_travel_and_capacity(void) {
    Enemy *e = setup();
    e->hp = 60;
    e->attack_phase = 1;
    e->boss_wave_used = 1;
    e->boss_attack = LICH_RING;
    e->move_timer = 2;
    e->attack_target_x = e->x;
    e->attack_target_y = e->y;
    game.level_cache[4].valid = 1;
    game.level_cache[4].map = game.map;
    game.level_cache[4].enemy_count = game.enemy_count;
    game.level_cache[4].enemies[0] = *e;
    game.portal_active = 1;
    game.portal_location = LOCATION_DUNGEON;
    game.portal_level = 5;
    game.portal_x = game.player.x;
    game.portal_y = game.player.y;
    game.portal_origin_tile = TILE_FLOOR;
    game.location = LOCATION_TOWN;
    game_use_town_portal(&game);
    ASSERT("return portal preserves a ring already warned on the cached floor", game.location == LOCATION_DUNGEON && game.enemies[0].boss_attack == LICH_RING && game.enemies[0].move_timer == 2 && game.enemies[0].attack_phase == 1 && game.enemies[0].boss_wave_used);

    kill_with(0);
    // A fresh expedition discards floor loot but preserves an earned reward.
    game.location = LOCATION_TOWN;
    game.portal_active = 0;
    game_enter_dungeon(&game);
    for (int level = 1; level < DUNGEON_DEPTH; level++) {
        game_descend(&game);
    }
    int living = 0;
    for (int i = 0; i < game.enemy_count; i++) {
        living |= game.enemies[i].active && game.enemies[i].type == ENEMY_LICH_KING;
    }
    ASSERT("fresh expedition restores unclaimed Cryptblade without respawning the Lich", !living && game.lich_reward_state == 1);
    // The new generated arena has different coordinates; scan the whole map.
    int cache = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            cache |= game.map.tiles[y][x] == TILE_LICH_RELIQUARY;
        }
    }
    ASSERT("unclaimed reward cache exists on the regenerated final floor", cache);

    e = setup();
    e->hp = 60;
    for (int i = 1; i < NON_ROAD_ENEMY_LIMIT; i++) {
        game.enemies[i] = (Enemy){.active = 1, .type = ENEMY_SKELETON, .x = 5, .y = 5, .hp = 10, .max_hp = 10};
    }
    game.enemy_count = NON_ROAD_ENEMY_LIMIT;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("wave respects enemy capacity and is consumed rather than retried forever", game.enemy_count == NON_ROAD_ENEMY_LIMIT && e->boss_wave_used && e->move_timer == -1);
}

static int make_legacy_save(void) {
    if (!save_game(&game, 99176)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99176.json", "r");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 123);
    cJSON_DeleteItemFromObject(root, "lich_reward_state");
    cJSON *arrays[2] = {cJSON_GetObjectItem(root, "enemies"),
        cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "level_cache"), 4), "enemies")};
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
    file = fopen("saves/savegame_99176.json", "w");
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

static void test_migration_and_layouts(void) {
    Enemy *e = setup();
    e->hp = 79;
    e->move_timer = 37;
    game.gold = 321;
    game.elowen_quest_state = 2;
    game.elowen_seals_restored = 7;
    map_mark_explored(&game.map, 22, 12);
    for (int y = 11; y < 21; y++) {
        for (int x = 21; x < 33; x++) {
            if (game.map.tiles[y][x] == TILE_LICH_SEAL || game.map.tiles[y][x] == TILE_LICH_PILLAR) {
                game.map.tiles[y][x] = TILE_FLOOR;
            }
        }
    }
    game.level_cache[4].valid = 1;
    game.level_cache[4].map = game.map;
    game.level_cache[4].enemy_count = 1;
    game.level_cache[4].enemies[0] = *e;
    game.level_cache[4].enemies[0].hp = 51;
    int ok = make_legacy_save() && load_game(&loaded, 99176);
    ASSERT("version 123 migration adds arena mechanics without resetting either boss", ok && loaded.enemies[0].hp == 79 && loaded.level_cache[4].enemies[0].hp == 51 && loaded.enemies[0].move_timer == 0 && lich_active_seals(&loaded) == 2 && loaded.level_cache[4].map.tiles[11][21] == TILE_LICH_SEAL);
    ASSERT("migration preserves gold, player position, exploration and completed quest", loaded.gold == 321 && loaded.player.x == 23 && loaded.player.y == 16 && map_is_explored(&loaded.map, 22, 12) && loaded.elowen_quest_state == 2 && loaded.elowen_seals_restored == 7);
    game.portal_active = 1;
    game.portal_location = LOCATION_DUNGEON;
    game.portal_level = DUNGEON_DEPTH;
    game.portal_x = 25;
    game.portal_y = 17;
    game.portal_origin_tile = TILE_FLOOR;
    ok = make_legacy_save() && load_game(&loaded, 99176);
    ASSERT("migration protects hidden active and cached portal landings from new cover", ok && loaded.map.tiles[17][25] == TILE_FLOOR && loaded.level_cache[4].map.tiles[17][25] == TILE_FLOOR);
    loaded.location = LOCATION_TOWN;
    game_use_town_portal(&loaded);
    ASSERT("migrated portal returns to its exact saved coordinates", loaded.location == LOCATION_DUNGEON && loaded.player.x == 25 && loaded.player.y == 17);
    game.portal_active = 0;
    game.defeated_bosses |= 1 << LOCATION_DUNGEON;
    game.enemies[0].active = 0;
    game.level_cache[4].enemies[0].active = 0;
    ok = make_legacy_save() && load_game(&loaded, 99176);
    ASSERT("legacy victories remain defeated and do not fabricate a claimed Cryptblade", ok && !loaded.enemies[0].active && loaded.lich_reward_state == 2);
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 22, .y = 12, .underlying_tile = TILE_FLOOR, .item = item_make_cryptblade()};
    game.map.tiles[12][22] = TILE_ITEM;
    ok = make_legacy_save() && load_game(&loaded, 99176);
    ASSERT("legacy floor reward survives migration and becomes recoverable", ok && loaded.lich_reward_state == 1 && loaded.floor_items[0].active && strcmp(loaded.floor_items[0].item.name, "Cryptblade") == 0);
    game.floor_items[0].item.sharpened = 1;
    game.floor_items[0].item.attack_bonus++;
    ok = make_legacy_save() && load_game(&loaded, 99176);
    ASSERT("previously sharpened legacy gear is preserved as already claimed", ok && loaded.lich_reward_state == 2 && loaded.floor_items[0].item.sharpened && loaded.floor_items[0].item.attack_bonus == 3);
    remove("saves/savegame_99176.json");

    int valid = 1;
    for (int seed = 0; seed < 100; seed++) {
        setup();
        srand((unsigned)seed);
        map_generate(&game.map, DUNGEON_DEPTH);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        enemies_spawn(&game);
        valid &= lich_active_seals(&game) == 2;
        int pillars = 0;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                pillars += game.map.tiles[y][x] == TILE_LICH_PILLAR;
            }
        }
        valid &= pillars == 2 && map_ensure_dungeon_connectivity(&game.map, DUNGEON_DEPTH);
        for (int i = 0; i < game.enemy_count; i++) {
            if (game.enemies[i].active) {
                valid &= map_is_walkable(&game.map, game.enemies[i].x, game.enemies[i].y);
            }
        }
    }
    ASSERT("100 generated arenas retain two seals, two safe pillars, routes and valid enemy positions", valid);
}

void test_lich(void) {
    printf("Lich King phased encounter tests:\n");
    test_patterns();
    test_seals();
    test_rewards();
    test_persistence();
    test_travel_and_capacity();
    test_migration_and_layouts();
}
