#include "test_utils.h"
#include "../src/game/necromancer.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = LOCATION_FOREST;
    game.level = FOREST_BOSS_LEVEL;
    game.max_forest_level_reached = FOREST_BOSS_LEVEL;
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
                game.map.tiles[y][x] = TILE_FOREST_FLOOR;
            }
        }
    }
    for (int x = 5; x <= 20; x++) {
        game.map.tiles[16][x] = TILE_FOREST_FLOOR;
    }
    for (int y = 5; y <= 16; y++) {
        game.map.tiles[y][5] = TILE_FOREST_FLOOR;
    }
    game.map.stairs_up_x = 5;
    game.map.stairs_up_y = 5;
    game.map.tiles[5][5] = TILE_STAIRS_UP;
    game.map.stairs_down_x = 27;
    game.map.stairs_down_y = 16;
    game.map.tiles[16][27] = TILE_FOREST_FLOOR;
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
    game.enemies[0] = (Enemy){.type = ENEMY_FOREST_NECROMANCER, .name = "Necromancer", .x = 28, .y = 16,
        .active = 1, .is_boss = 1, .hp = 140, .max_hp = 140, .attack = 18, .defense = 6,
        .experience = 400, .attack_target_x = -1, .attack_target_y = -1, .boss_aux_x = -1, .boss_aux_y = -1};
    necromancer_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
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
        int cloaks = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            cloaks += game.floor_items[i].active && strcmp(game.floor_items[i].item.name, "Necromancer's Cloak") == 0;
        }
        victories &= !game.enemies[0].active && cloaks == 1 && game.necromancer_reward_state == 1 && (game.defeated_bosses & (1 << LOCATION_FOREST));
    }
    ASSERT("melee, bow, Magic Arrow, Fireball and Frost Bolt each guarantee one Necromancer's Cloak", victories);
    Enemy *e = setup();
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 100;
    e->hp = 1;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 22, .y = 20, .underlying_tile = TILE_FOREST_FLOOR, .item = item_make_health_potion()};
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
    ASSERT("full floor capacity preserves every item and reserves reward in a reliquary", game.floor_item_count == MAX_FLOOR_ITEMS && game.floor_items[MAX_FLOOR_ITEMS - 1].item.type == ITEM_POTION_HEALTH && game.necromancer_reward_state == 1 && find_tile(TILE_NECRO_RELIQUARY, &x, &y));
    game.player.x = x;
    game.player.y = y;
    game.map.tiles[y][x + 1] = TILE_NECRO_GRAVE;
    game.map.tiles[y - 1][x] = TILE_NECRO_GRAVE;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("full inventory cannot consume the reserved reward", game.necromancer_reward_state == 1 && game.map.tiles[y][x] == TILE_NECRO_RELIQUARY);
    game.inventory_count--;
    int reward_score = game.score;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("reliquary pickup grants original Necromancer's Cloak and marks it claimed", game.necromancer_reward_state == 2 && strcmp(game.inventory[MAX_INVENTORY - 1].name, "Necromancer's Cloak") == 0 && game.inventory[MAX_INVENTORY - 1].defense_bonus == item_make_necromancer_cloak().defense_bonus && game.score == reward_score + game.inventory[MAX_INVENTORY - 1].value);
    necromancer_restore_reward(&game);
    ASSERT("claiming cannot recreate a second reward or consecrate adjacent graves", !find_tile(TILE_NECRO_RELIQUARY, &x, &y) && game.map.tiles[game.player.y - 1][game.player.x] == TILE_NECRO_GRAVE);
    kill_with(0);
    game.player.x = game.enemies[0].x;
    game.player.y = game.enemies[0].y;
    for (int i = 0; i < 3; i++) {
        action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    }
    ASSERT("normal ground pickup also records a claimed reward", game.necromancer_reward_state == 2);
    setup();
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_BLIGHTED_WOLF, .name = "Grove Revenant", .hp = 1, .max_hp = 10, .x = 23, .y = 15, .revived = 1, .experience = 0};
    game.enemy_count = 2;
    game.player.attack = 100;
    int xp = game.player.experience;
    action_resolve_player(&game, (Action){ACTION_MOVE, 23, 15});
    ASSERT("raised servant grants neither loot nor experience", !game.enemies[1].active && game.floor_item_count == 0 && game.player.experience == xp);
}

static int make_legacy_save(void) {
    if (!save_game(&game, 99177)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99177.json", "r");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 124);
    cJSON_DeleteItemFromObject(root, "necromancer_reward_state");
    cJSON *arrays[2] = {cJSON_GetObjectItem(root, "enemies"),
        cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "forest_cache"), 3), "enemies")};
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
    file = fopen("saves/savegame_99177.json", "w");
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

static void test_patterns_and_graves(void) {
    Enemy *e = setup();
    int hp = game.player.hp;
    action_resolve_enemies(&game);
    ASSERT("Necromancer warns a fixed lane without immediate damage", e->boss_attack == NECRO_BOLT && e->move_timer == 1 && game.player.hp == hp && necromancer_attack_marks(&game, e, 23, 16));
    int tx = e->attack_target_x;
    e->hp = e->max_hp / 2;
    e->frozen_turns = 2;
    game.player.y++;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze and half-health crossing preserve the declared bolt", e->move_timer == 1 && e->attack_phase == 0 && e->attack_target_x == tx && e->attack_target_y == 16);
    action_resolve_enemies(&game);
    ASSERT("ordinary sidestep avoids bolt and opens recovery", game.player.hp == hp && e->move_timer == -1);
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 10;
    int boss_hp = e->hp;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    action_resolve_enemies(&game);
    ASSERT("melee recovery hit has no extra Necromancer retaliation", e->hp < boss_hp && game.player.hp == hp && e->move_timer == 0);
    action_resolve_enemies(&game);
    ASSERT("next preparation announces phase two and spends its finite wave", e->attack_phase == 1 && e->boss_attack == NECRO_RAISE && e->boss_wave_used == 3 && e->move_timer == 2);

    e = setup();
    e->hp--;
    e->attack_phase = 1;
    e->boss_wave_used = 3;
    action_resolve_enemies(&game);
    ASSERT("crossing roots warn both fixed lanes for two turns", e->boss_attack == NECRO_ROOTS && e->move_timer == 2 && necromancer_attack_marks(&game, e, 23, 17) && necromancer_attack_marks(&game, e, 24, 16));
    game.player.x++;
    action_resolve_enemies(&game);
    ASSERT("roots keep the original intersection after first escape step", e->move_timer == 1 && e->attack_target_x == 23 && e->attack_target_y == 16);
    game.player.y++;
    action_resolve_enemies(&game);
    ASSERT("two ordinary cardinal steps escape roots without special abilities", game.player.hp == hp && e->move_timer == -1 && game.enemy_count == 1);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("phase two alternates roots with spirit bolts", e->boss_attack == NECRO_BOLT && e->move_timer == 1);

    e = setup();
    e->attack_phase = 1;
    e->boss_wave_used = 3;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("remaining on marked roots deals exactly one signature hit", game.player.hp == hp - e->attack && e->move_timer == -1);

    e = setup();
    ASSERT("grove contains three traversable graves", necromancer_active_graves(&game) == 3);
    action_resolve_enemies(&game);
    game.player.y++;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    int x = 0;
    int y = 0;
    find_tile(TILE_NECRO_GRAVE, &x, &y);
    game.floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_NECRO_GRAVE, .item = item_make_health_potion()};
    game.floor_item_count = 1;
    game.map.tiles[y][x] = TILE_ITEM;
    game.player.x = x;
    game.player.y = y + 1;
    ASSERT("contextual interaction recognizes warned grave under loot", game_has_regional_interaction(&game) && necromancer_attack_marks(&game, e, x, y));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("consecration cancels entire wave and leaves two complete recovery turns", necromancer_active_graves(&game) == 2 && e->move_timer == -2 && game.enemy_count == 1 && e->boss_wave_used == 1);
    ASSERT("consecrating beneath loot preserves the item and its spent underlay", game.floor_items[0].active && game.floor_items[0].underlying_tile == TILE_NECRO_GRAVE_SPENT && game.map.tiles[y][x] == TILE_ITEM);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("interrupted wave is not retried during recovery", e->move_timer == 0 && game.enemy_count == 1);
    game.map.tiles[y][x] = TILE_NECRO_GRAVE_SPENT;
    game.floor_item_count = 0;
    while (find_tile(TILE_NECRO_GRAVE, &x, &y)) {
        game.player.x = x;
        game.player.y = y;
        necromancer_interact(&game);
    }
    necromancer_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    e->hp = e->max_hp / 2;
    action_resolve_enemies(&game);
    ASSERT("all consecrated graves prevent phase-two raises and stay spent", necromancer_active_graves(&game) == 0 && e->boss_wave_used == 3 && e->boss_attack == NECRO_ROOTS);
}

static void test_finite_waves(void) {
    Enemy *e = setup();
    e->boss_attack = NECRO_RAISE;
    e->boss_wave_used = 1;
    e->move_timer = 1;
    game.enemy_count = 2;
    game.enemies[1] = (Enemy){.type = ENEMY_FOREST_TROLL, .max_hp = 48, .hp = 0, .attack = 13, .defense = 5, .experience = 48};
    action_resolve_enemies(&game);
    ASSERT("wave revives a fallen forest servant at half health without repeat XP", game.enemies[1].active && game.enemies[1].type == ENEMY_FOREST_TROLL && game.enemies[1].hp == 24 && game.enemies[1].experience == 0 && game.enemies[1].revived);
    ASSERT("up to three grave servants appear and skip their spawning turn", game.enemy_count == 4 && game.player.hp == 1000 && game.enemies[1].x == 22 && game.enemies[1].y == 12);
    for (int i = 1; i < game.enemy_count; i++) {
        game.enemies[i].active = 0;
    }
    e->move_timer = 0;
    e->hp = e->max_hp / 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("phase two permits one second wave and persists both consumed flags", e->boss_wave_used == 3 && e->move_timer == -1 && game.enemy_count == 4);
    for (int i = 1; i < game.enemy_count; i++) {
        game.enemies[i].active = 0;
    }
    for (int i = 0; i < 30; i++) {
        action_resolve_enemies(&game);
    }
    ASSERT("later attack cycles never refill exhausted grave waves", !game.enemies[1].active && !game.enemies[2].active && !game.enemies[3].active && e->boss_wave_used == 3);

    e = setup();
    e->boss_attack = NECRO_RAISE;
    e->boss_wave_used = 1;
    e->move_timer = 1;
    int x;
    int y;
    find_tile(TILE_NECRO_GRAVE, &x, &y);
    game.player.x = x;
    game.player.y = y;
    action_resolve_enemies(&game);
    ASSERT("standing on a warned grave prevents spawn overlap without retargeting", game.enemy_count == 3 && game.player.hp == 1000 && game.enemies[1].x != game.player.x);
    e = setup();
    for (int i = 1; i < NON_ROAD_ENEMY_LIMIT; i++) {
        game.enemies[i] = (Enemy){.active = 1, .type = ENEMY_SKELETON, .x = 5, .y = 5, .hp = 10, .max_hp = 10};
    }
    game.enemy_count = NON_ROAD_ENEMY_LIMIT;
    e->boss_attack = NECRO_RAISE;
    e->boss_wave_used = 3;
    e->move_timer = 1;
    action_resolve_enemies(&game);
    ASSERT("full enemy capacity skips the finite wave without deleting an actor", game.enemy_count == NON_ROAD_ENEMY_LIMIT && e->move_timer == -1 && e->boss_wave_used == 3);
    int no_farming = 1;
    for (int method = 0; method < 5; method++) {
        setup();
        Enemy *servant = game_raise_grove_servant(&game, 22, 12, 0);
        if (!servant) {
            no_farming = 0;
            continue;
        }
        servant->hp = 1;
        game.player.x = method == 0 ? 21 : 20;
        game.player.y = 12;
        game.player.last_dx = 1;
        game.player.last_dy = 0;
        game.player.attack = 100;
        int xp = game.player.experience;
        int score = game.score;
        int gold = game.gold;
        if (method == 0) {
            action_resolve_player(&game, (Action){ACTION_MOVE, 22, 12});
        } else if (method == 1) {
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
        no_farming &= !servant->active && game.floor_item_count == 0 && game.player.experience == xp && game.score == score && game.gold == gold;
    }
    ASSERT("all five attack paths defeat raised servants without repeat gold, loot, XP or score", no_farming);
}

static void test_persistence_and_migration(void) {
    Enemy *e = setup();
    int x;
    int y;
    find_tile(TILE_NECRO_GRAVE, &x, &y);
    game.map.tiles[y][x] = TILE_NECRO_GRAVE_SPENT;
    e->hp = 60;
    e->attack_phase = 1;
    e->boss_wave_used = 3;
    e->boss_attack = NECRO_ROOTS;
    e->move_timer = 2;
    e->frozen_turns = 2;
    e->attack_target_x = 23;
    e->attack_target_y = 16;
    game.forest_cache[FOREST_BOSS_LEVEL - 1].valid = 1;
    game.forest_cache[FOREST_BOSS_LEVEL - 1].map = game.map;
    game.forest_cache[FOREST_BOSS_LEVEL - 1].enemy_count = 1;
    game.forest_cache[FOREST_BOSS_LEVEL - 1].enemies[0] = *e;
    game.necromancer_reward_state = 1;
    int ok = save_game(&game, 99177) && load_game(&loaded, 99177);
    ASSERT("save restores phase, exact frozen warning and both finite waves", ok && loaded.enemies[0].hp == 60 && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].frozen_turns == 2 && loaded.enemies[0].boss_attack == NECRO_ROOTS && loaded.enemies[0].boss_wave_used == 3 && loaded.enemies[0].attack_target_x == 23 && loaded.enemies[0].attack_target_y == 16);
    ASSERT("save preserves grave consecrations, reward state and cached waves", ok && loaded.map.tiles[y][x] == TILE_NECRO_GRAVE_SPENT && loaded.forest_cache[FOREST_BOSS_LEVEL - 1].map.tiles[y][x] == TILE_NECRO_GRAVE_SPENT && loaded.forest_cache[FOREST_BOSS_LEVEL - 1].enemies[0].boss_wave_used == 3 && loaded.necromancer_reward_state == 1);
    e->move_timer = -2;
    game.necromancer_reward_state = 2;
    ok = save_game(&game, 99177) && load_game(&loaded, 99177);
    ASSERT("save also preserves recovery timing and claimed cloak state", ok && loaded.enemies[0].move_timer == -2 && loaded.necromancer_reward_state == 2);
    e->move_timer = 2;
    game.portal_active = 1;
    game.portal_location = LOCATION_FOREST;
    game.portal_level = FOREST_BOSS_LEVEL;
    game.portal_x = 23;
    game.portal_y = 16;
    game.portal_origin_tile = TILE_FOREST_FLOOR;
    game.location = LOCATION_TOWN;
    game_use_town_portal(&game);
    ASSERT("portal revisit preserves consumed waves, pending roots and spent graves", game.location == LOCATION_FOREST && game.enemies[0].move_timer == 2 && game.enemies[0].boss_wave_used == 3 && necromancer_active_graves(&game) == 2);

    e = setup();
    game.level = 4;
    e->hp = 79;
    e->move_timer = 37;
    game.gold = 321;
    game.alder_quest_state = 2;
    map_mark_explored(&game.map, 22, 12);
    for (int row = 11; row < 21; row++) {
        for (int col = 21; col < 33; col++) {
            if (game.map.tiles[row][col] == TILE_NECRO_GRAVE) {
                game.map.tiles[row][col] = TILE_FOREST_FLOOR;
            }
        }
    }
    game.forest_cache[3].valid = 1;
    game.forest_cache[3].map = game.map;
    game.forest_cache[3].enemy_count = 1;
    game.forest_cache[3].enemies[0] = *e;
    game.forest_cache[3].enemies[0].hp = 51;
    game.portal_active = 1;
    game.portal_location = LOCATION_FOREST;
    game.portal_level = 4;
    game.portal_x = 22;
    game.portal_y = 12;
    game.portal_origin_tile = TILE_FOREST_FLOOR;
    ok = make_legacy_save() && load_game(&loaded, 99177);
    ASSERT("version 124 migration adds graves and visible timing while preserving boss health", ok && loaded.enemies[0].hp == 79 && loaded.forest_cache[4].enemies[0].hp == 51 && loaded.enemies[0].move_timer == 0 && necromancer_active_graves(&loaded) == 3);
    ASSERT("migration preserves quests, gold, exploration, position and portal landing", loaded.alder_quest_state == 2 && loaded.gold == 321 && map_is_explored(&loaded.map, 22, 12) && loaded.player.x == 23 && loaded.player.y == 16 && loaded.map.tiles[12][22] == TILE_FOREST_FLOOR && loaded.forest_cache[4].map.tiles[12][22] == TILE_FOREST_FLOOR);
    game.enemies[0].active = 0;
    game.forest_cache[3].enemies[0].active = 0;
    game.defeated_bosses |= 1 << LOCATION_FOREST;
    ok = make_legacy_save() && load_game(&loaded, 99177);
    ASSERT("legacy victories remain defeated and do not fabricate a reward", ok && !loaded.enemies[0].active && loaded.necromancer_reward_state == 2);
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 24, .y = 12, .underlying_tile = TILE_FOREST_FLOOR, .item = item_make_necromancer_cloak()};
    game.map.tiles[12][24] = TILE_ITEM;
    ok = make_legacy_save() && load_game(&loaded, 99177);
    ASSERT("known legacy dropped cloak becomes persistently recoverable", ok && loaded.necromancer_reward_state == 1 && loaded.floor_items[0].active);
    remove("saves/savegame_99177.json");
}

static void test_layouts_and_reward_refresh(void) {
    int valid = 1;
    for (int seed = 0; seed < 100; seed++) {
        setup();
        srand((unsigned)seed);
        map_generate_forest(&game.map, FOREST_BOSS_LEVEL);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        enemies_spawn(&game);
        valid &= necromancer_active_graves(&game) == 3;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (game.map.tiles[y][x] == TILE_NECRO_GRAVE) {
                    valid &= map_is_walkable(&game.map, x, y);
                    for (int i = 0; i < game.enemy_count; i++) {
                        valid &= !game.enemies[i].active || game.enemies[i].x != x || game.enemies[i].y != y;
                    }
                }
            }
        }
    }
    ASSERT("100 forest arenas have three walkable graves without actor overlap", valid);
    kill_with(0);
    game.floor_item_count = 0;
    map_generate_forest(&game.map, FOREST_BOSS_LEVEL);
    enemies_spawn(&game);
    game_refresh_quest_encounters(&game);
    int reward = 0;
    int living = 0;
    for (int i = 0; i < game.enemy_count; i++) {
        living |= game.enemies[i].active && game.enemies[i].type == ENEMY_FOREST_NECROMANCER;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            reward |= game.map.tiles[y][x] == TILE_NECRO_RELIQUARY;
        }
    }
    ASSERT("regenerated grove restores earned cloak without reviving a defeated boss", !living && reward && game.necromancer_reward_state == 1);
}

void test_necromancer(void) {
    printf("Forest Necromancer phased encounter tests:\n");
    test_patterns_and_graves();
    test_finite_waves();
    test_rewards();
    test_persistence_and_migration();
    test_layouts_and_reward_refresh();
}
