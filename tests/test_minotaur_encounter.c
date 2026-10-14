#include "test_utils.h"
#include "../src/game/minotaur.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = LOCATION_LABYRINTH;
    game.level = LABYRINTH_DEPTH;
    game.rook_quest_state = 1;
    game.rook_labyrinth_switches = 31;
    int x;
    int y;
    map_generate_labyrinth(&game.map, LABYRINTH_DEPTH, 31, &x, &y);
    game.player.x = 37;
    game.player.y = 20;
    game.player.player_class = CLASS_WARRIOR;
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
    game.enemies[0] = (Enemy){.type = ENEMY_MINOTAUR, .name = "Minotaur", .x = 39, .y = 20,
        .active = 1, .is_boss = 1, .hp = 120, .max_hp = 120, .attack = 18, .defense = 7,
        .experience = 250, .attack_target_x = -1, .attack_target_y = -1, .boss_aux_x = -1, .boss_aux_y = -1};
    minotaur_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    return &game.enemies[0];
}

static int find_tile(TileType tile, int *x, int *y) {
    for (int row = 0; row < LABYRINTH_H; row++) {
        for (int col = 0; col < LABYRINTH_W; col++) {
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
        int shields = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            shields += game.floor_items[i].active && strcmp(game.floor_items[i].item.name, "Labyrinth Aegis") == 0;
        }
        victories &= !game.enemies[0].active && shields == 1 && game.minotaur_reward_state == 1 && (game.defeated_bosses & (1 << LOCATION_LABYRINTH));
    }
    ASSERT("melee, bow, Magic Arrow, Fireball and Frost Bolt each guarantee one Labyrinth Aegis", victories);
    Enemy *e = setup();
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 100;
    e->hp = 1;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 22, .y = 20, .underlying_tile = TILE_LABYRINTH_FLOOR, .item = item_make_health_potion()};
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
    ASSERT("full floor capacity preserves every item and reserves reward in a reliquary", game.floor_item_count == MAX_FLOOR_ITEMS && game.floor_items[MAX_FLOOR_ITEMS - 1].item.type == ITEM_POTION_HEALTH && game.minotaur_reward_state == 1 && find_tile(TILE_MINOTAUR_RELIQUARY, &x, &y));
    game.player.x = x;
    game.player.y = y;
    game.map.tiles[y][x + 1] = TILE_LABYRINTH_SWITCH_ON;
    game.map.tiles[y - 1][x] = TILE_LABYRINTH_SWITCH_ON;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("full inventory cannot consume the reserved reward", game.minotaur_reward_state == 1 && game.map.tiles[y][x] == TILE_MINOTAUR_RELIQUARY);
    game.inventory_count--;
    int reward_score = game.score;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("reliquary pickup grants original Labyrinth Aegis and marks it claimed", game.minotaur_reward_state == 2 && strcmp(game.inventory[MAX_INVENTORY - 1].name, "Labyrinth Aegis") == 0 && game.inventory[MAX_INVENTORY - 1].defense_bonus == item_make_labyrinth_aegis().defense_bonus && game.score == reward_score + game.inventory[MAX_INVENTORY - 1].value);
    minotaur_restore_reward(&game);
    ASSERT("claiming cannot recreate a second reward or operate adjacent quest runes", !find_tile(TILE_MINOTAUR_RELIQUARY, &x, &y) && game.map.tiles[game.player.y - 1][game.player.x] == TILE_LABYRINTH_SWITCH_ON);
    kill_with(0);
    game.player.x = game.enemies[0].x;
    game.player.y = game.enemies[0].y;
    for (int i = 0; i < 3; i++) {
        action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    }
    ASSERT("normal ground pickup also records a claimed reward", game.minotaur_reward_state == 2);
    e = setup();
    e->active = 0;
    game.defeated_bosses |= 1 << LOCATION_LABYRINTH;
    game.minotaur_reward_state = MINOTAUR_REWARD_AEGIS;
    for (int yy = 18; yy <= 22; yy++) {
        for (int xx = 37; xx <= 41; xx++) {
            if (game.map.tiles[yy][xx] != TILE_LABYRINTH_FLOOR) {
                continue;
            }
            if (game.floor_item_count < MAX_FLOOR_ITEMS) {
                game.floor_items[game.floor_item_count++] = (FloorItem){.active = 1, .x = xx, .y = yy, .underlying_tile = TILE_LABYRINTH_FLOOR, .item = item_make_health_potion()};
                game.map.tiles[yy][xx] = TILE_ITEM;
            } else {
                game.enemies[game.enemy_count++] = (Enemy){.active = 1, .type = ENEMY_SKELETON, .x = xx, .y = yy, .hp = 10};
            }
        }
    }
    minotaur_restore_reward(&game);
    int reserved = -1;
    for (int i = 0; i < game.floor_item_count; i++) {
        if (game.floor_items[i].underlying_tile == TILE_MINOTAUR_RELIQUARY) {
            reserved = i;
        }
    }
    ASSERT("a saturated vault safely reserves its reward beneath existing loot", reserved >= 0 && game.floor_item_count == MAX_FLOOR_ITEMS && game.minotaur_reward_state == MINOTAUR_REWARD_AEGIS);
    int saved = save_game(&game, 99185) && load_game(&loaded, 99185);
    ASSERT("reserved reward under loot survives save/load with all existing items", saved && reserved >= 0 && loaded.floor_item_count == MAX_FLOOR_ITEMS && loaded.floor_items[reserved].underlying_tile == TILE_MINOTAUR_RELIQUARY && loaded.minotaur_reward_state == MINOTAUR_REWARD_AEGIS);
    if (reserved >= 0) {
        game.player.x = game.floor_items[reserved].x;
        game.player.y = game.floor_items[reserved].y;
        game.inventory_count = 0;
        action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
        int preserved = 1;
        for (int i = 0; i < game.floor_item_count; i++) {
            preserved &= game.floor_items[i].active && game.floor_items[i].item.type == ITEM_POTION_HEALTH;
        }
        ASSERT("P recovers the cache beneath loot without deleting or picking up any existing item", preserved && game.inventory_count == 1 && strcmp(game.inventory[0].name, "Labyrinth Aegis") == 0 && game.minotaur_reward_state == MINOTAUR_REWARD_CLAIMED);
    }

}

static int make_legacy_save(void) {
    if (!save_game(&game, 99185)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99185.json", "r");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 128);
    cJSON_DeleteItemFromObject(root, "minotaur_reward_state");
    cJSON *arrays[2] = {cJSON_GetObjectItem(root, "enemies"),
        cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "labyrinth_cache"), LABYRINTH_DEPTH - 1), "enemies")};
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
    file = fopen("saves/savegame_99185.json", "w");
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

static void test_patterns(void) {
    Enemy *e = setup();
    action_resolve_enemies(&game);
    ASSERT("charge receives a fixed one-action lane before moving or damaging", e->boss_attack == MINOTAUR_CHARGE && e->move_timer == 1 && e->x == 39 && game.player.hp == 1000 && minotaur_attack_marks(&game, e, 37, 20));
    int tx = e->attack_target_x;
    int ty = e->attack_target_y;
    e->hp = 60;
    e->frozen_turns = 2;
    action_resolve_player(&game, (Action){ACTION_MOVE, 37, 21});
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze and half-health crossing preserve charge endpoints, orientation and phase", e->move_timer == 1 && e->attack_phase == 0 && e->attack_target_x == tx && e->attack_target_y == ty && e->facing_dx == -1 && e->x == 39);
    action_resolve_enemies(&game);
    ASSERT("one normal sidestep escapes and the charge moves only through its warned lane", game.player.hp == 1000 && e->x == tx && e->y == ty && e->move_timer == -1);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("next preparation latches Bloodied Bull and independently warns its first charge", e->attack_phase == 1 && e->boss_wave_used == 0 && e->boss_attack == MINOTAUR_CHARGE && e->move_timer == 1);
    action_resolve_enemies(&game);
    ASSERT("phase two first charge resolves separately and retains recovery", e->boss_wave_used == 1 && e->move_timer < 0);
    while (e->move_timer < 0) {
        action_resolve_enemies(&game);
    }
    action_resolve_enemies(&game);
    ASSERT("second charge has a fresh full warning instead of resolving with the first", e->boss_wave_used == 1 && e->move_timer == 1 && e->boss_attack == MINOTAUR_CHARGE);
    action_resolve_enemies(&game);
    while (e->move_timer < 0) {
        action_resolve_enemies(&game);
    }
    action_resolve_enemies(&game);
    ASSERT("two charges lead to a separately warned two-action wide axe", e->boss_wave_used == 2 && e->boss_attack == MINOTAUR_WIDE_AXE && e->move_timer == 2);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("wide axe resolves once, opens recovery and resets the two-charge cycle", e->move_timer == -1 && e->boss_wave_used == 0);

    e = setup();
    game.player.x = 38;
    action_resolve_enemies(&game);
    ASSERT("close phase-one attacks warn a fixed three-tile axe front", e->boss_attack == MINOTAUR_AXE && e->move_timer == 1 && minotaur_attack_marks(&game, e, 38, 20) && minotaur_attack_marks(&game, e, 38, 21) && !minotaur_attack_marks(&game, e, 37, 20));
    action_resolve_player(&game, (Action){ACTION_MOVE, 37, 20});
    action_resolve_enemies(&game);
    game.player.x = 38;
    game.player.attack = 10;
    int hp = e->hp;
    action_resolve_player(&game, (Action){ACTION_MOVE, 39, 20});
    action_resolve_enemies(&game);
    ASSERT("short axe escape and adjacent recovery counterattack have no extra retaliation", game.player.hp == 1000 && e->hp < hp && e->move_timer == 0);
    e = setup();
    game.player.x = 38;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("standing in the short axe receives exactly one base-strength hit", game.player.hp == 982 && e->move_timer == -1);
    e = setup();
    game.player.x = 36;
    action_resolve_enemies(&game);
    ASSERT("single-file approach shortens a charge to permit one backward step", e->attack_target_x == 36 && !minotaur_attack_marks(&game, e, 35, 20));
    action_resolve_player(&game, (Action){ACTION_MOVE, 35, 20});
    action_resolve_enemies(&game);
    ASSERT("actual corridor backstep escapes without an invisible wall or reset", game.player.hp == 1000 && e->x == 36 && e->move_timer == -1);
    e = setup();
    game.player.x = 36;
    game.player.defense = 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("charge damage uses normal armor, stops before the player and never shares a tile", game.player.hp == 984 && e->x == 37 && e->y == 20);
    e = setup();
    e->hp = 60;
    e->attack_phase = 1;
    e->boss_wave_used = 2;
    game.player.x = 38;
    action_resolve_enemies(&game);
    tx = e->facing_dx;
    action_resolve_player(&game, (Action){ACTION_MOVE, 37, 20});
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, 37, 21});
    action_resolve_enemies(&game);
    ASSERT("two ordinary cardinal moves escape the wide arc without retargeting", game.player.hp == 1000 && e->facing_dx == tx && e->move_timer == -1);

    e = setup();
    game.map.tiles[20][38] = TILE_LABYRINTH_GATE;
    action_resolve_enemies(&game);
    ASSERT("closed gate prevents engagement without being damaged", e->boss_attack == 0 && game.player.hp == 1000 && game.map.tiles[20][38] == TILE_LABYRINTH_GATE);
    game.map.tiles[20][38] = TILE_LABYRINTH_FLOOR;
    action_resolve_enemies(&game);
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_SKELETON, .x = 38, .y = 20, .hp = 10, .max_hp = 10, .frozen_turns = 4};
    game.enemy_count = 2;
    action_resolve_enemies(&game);
    ASSERT("an actor entering a warned charge blocks movement and prevents damage beyond it", e->x == 39 && game.player.hp == 1000);
    e = setup();
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_SKELETON, .x = 38, .y = 20, .hp = 10, .max_hp = 10, .frozen_turns = 4};
    game.enemy_count = 2;
    action_resolve_enemies(&game);
    ASSERT("an actor blocks initial targeting just like stone cover", e->boss_attack == 0);
    e = setup();
    action_resolve_enemies(&game);
    game.player.x = 20;
    action_resolve_enemies(&game);
    ASSERT("distant retreat pauses the pending lane without healing or moving", e->move_timer == 1 && e->hp == 120 && e->x == 39);
}

static void bait_pillar(Enemy *e, int phase) {
    e->x = 38;
    e->y = 22;
    e->move_timer = 0;
    e->boss_attack = 0;
    e->attack_phase = phase;
    e->boss_wave_used = 0;
    e->hp = phase ? 60 : 120;
    game.player.x = 38;
    game.player.y = 20;
    action_resolve_enemies(&game);
}

static void test_pillars(void) {
    Enemy *e = setup();
    ASSERT("vault gains two reinforced pillars without changing gate or relic", game.map.tiles[19][38] == TILE_MINOTAUR_PILLAR && game.map.tiles[21][40] == TILE_MINOTAUR_PILLAR && game.map.tiles[20][40] == TILE_LABYRINTH_RELIC && !map_is_walkable(&game.map, 38, 19));
    bait_pillar(e, 0);
    ASSERT("a pillar collision is announced separately from the exact damage lane", e->boss_aux_x == 38 && e->boss_aux_y == 19 && e->attack_target_y == 20 && strstr(game.messages[game.message_count - 1], "CRACK") != NULL);
    action_resolve_player(&game, (Action){ACTION_MOVE, 39, 20});
    action_resolve_enemies(&game);
    ASSERT("baited impact cracks the cap, grants two actions of stagger and keeps player safe", e->move_timer == -2 && game.map.tiles[19][38] == TILE_MINOTAUR_CRACKED && !map_is_walkable(&game.map, 38, 19) && game.player.hp == 1000);
    game.player.x = 39;
    game.player.y = 20;
    game.player.attack = 10;
    int hp = e->hp;
    action_resolve_player(&game, (Action){ACTION_MOVE, 38, 20});
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, 38, 20});
    action_resolve_enemies(&game);
    ASSERT("both stagger actions permit safe melee counterattacks", e->hp < hp && game.player.hp == 1000 && e->move_timer == 0);
    bait_pillar(e, 0);
    e->hp = 60;
    action_resolve_player(&game, (Action){ACTION_MOVE, 39, 20});
    action_resolve_enemies(&game);
    ASSERT("crossing half health cannot break a cap during an already warned phase-one charge", e->attack_phase == 0 && game.map.tiles[19][38] == TILE_MINOTAUR_CRACKED);
    bait_pillar(e, 1);
    ASSERT("a phase-two cracked cap receives an explicit break warning", e->boss_aux_x == 38 && e->boss_aux_y == 19);
    action_resolve_player(&game, (Action){ACTION_MOVE, 39, 20});
    action_resolve_enemies(&game);
    ASSERT("only the designated warned cap breaks into a walkable flank", game.map.tiles[19][38] == TILE_MINOTAUR_BROKEN && map_is_walkable(&game.map, 38, 19) && game.map.tiles[21][40] == TILE_MINOTAUR_PILLAR && game.map.tiles[20][40] == TILE_LABYRINTH_RELIC && e->move_timer == -2);
    minotaur_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    ASSERT("arena preparation never rebuilds a broken cap", game.map.tiles[19][38] == TILE_MINOTAUR_BROKEN);
    e = setup();
    game.player.x = 39;
    game.player.y = 18;
    action_resolve_enemies(&game);
    ASSERT("ordinary wall impacts are telegraphed without marking the wall as damage floor", e->boss_aux_x == 39 && e->boss_aux_y == 17 && !minotaur_attack_marks(&game, e, 39, 17));
    action_resolve_player(&game, (Action){ACTION_MOVE, 38, 18});
    action_resolve_enemies(&game);
    ASSERT("ordinary maze walls remain intact while granting two actions of stagger", e->move_timer == -2 && game.map.tiles[17][39] == TILE_LABYRINTH_WALL && game.player.hp == 1000);
}
static void test_persistence(void) {
    Enemy *e = setup();
    e->hp = 47;
    e->attack_phase = 1;
    e->boss_wave_used = 1;
    e->boss_attack = MINOTAUR_CHARGE;
    e->facing_dx = 0;
    e->facing_dy = -1;
    e->move_timer = 1;
    e->frozen_turns = 2;
    e->attack_target_x = 39;
    e->attack_target_y = 18;
    e->boss_aux_x = 39;
    e->boss_aux_y = 17;
    game.map.tiles[19][38] = TILE_MINOTAUR_CRACKED;
    game.map.tiles[21][40] = TILE_MINOTAUR_BROKEN;
    LevelCache *cache = &game.labyrinth_cache[LABYRINTH_DEPTH - 1];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = 1;
    cache->enemies[0] = *e;
    game.minotaur_reward_state = MINOTAUR_REWARD_AEGIS;
    game.inventory[0] = item_make_labyrinth_aegis();
    game.inventory[1] = item_make_magic_shield();
    game.inventory_count = 2;
    int ok = save_game(&game, 99185) && load_game(&loaded, 99185);
    ASSERT("save preserves active health, phase, charge sequence, orientation, targets and freeze", ok && loaded.enemies[0].hp == 47 && loaded.enemies[0].attack_phase == 1 && loaded.enemies[0].boss_wave_used == 1 && loaded.enemies[0].move_timer == 1 && loaded.enemies[0].frozen_turns == 2 && loaded.enemies[0].facing_dy == -1 && loaded.enemies[0].attack_target_y == 18 && loaded.enemies[0].boss_aux_y == 17);
    ASSERT("active and cached maps retain cracked/broken pillars and reward state", ok && loaded.map.tiles[19][38] == TILE_MINOTAUR_CRACKED && loaded.labyrinth_cache[4].map.tiles[21][40] == TILE_MINOTAUR_BROKEN && loaded.labyrinth_cache[4].enemies[0].boss_wave_used == 1 && loaded.minotaur_reward_state == 1);
    ASSERT("named Aegis visual persists without renaming existing Magic Shields", ok && loaded.inventory[0].visual_id == ITEM_VISUAL_LABYRINTH_AEGIS && strcmp(loaded.inventory[0].name, "Labyrinth Aegis") == 0 && loaded.inventory[1].visual_id == ITEM_VISUAL_MAGIC_SHIELD && strcmp(loaded.inventory[1].name, "Magic Shield") == 0);
    e->move_timer = -2;
    game.minotaur_reward_state = MINOTAUR_REWARD_CLAIMED;
    ok = save_game(&game, 99185) && load_game(&loaded, 99185);
    ASSERT("save retains two-action stagger and permanently claimed reward", ok && loaded.enemies[0].move_timer == -2 && loaded.minotaur_reward_state == 2);
    e->move_timer = 1;
    game_change_labyrinth_floor(&game, 0, 0);
    game_change_labyrinth_floor(&game, 1, 0);
    ASSERT("backtracking preserves frozen charge, health and broken pillar", game.level == 5 && game.enemies[0].hp == 47 && game.enemies[0].move_timer == 1 && game.enemies[0].boss_wave_used == 1 && game.map.tiles[21][40] == TILE_MINOTAUR_BROKEN);
    while (game.level > 1) {
        game_change_labyrinth_floor(&game, 0, 0);
    }
    game_leave_labyrinth(&game);
    game_enter_labyrinth(&game);
    while (game.level < LABYRINTH_DEPTH) {
        game_change_labyrinth_floor(&game, 1, 0);
    }
    ASSERT("retreat to Stillbury and re-entry cannot reset the living boss's cached encounter", game.enemies[0].hp == 47 && game.enemies[0].move_timer == 1 && game.enemies[0].frozen_turns == 2 && game.map.tiles[21][40] == TILE_MINOTAUR_BROKEN);

    e = setup();
    game.map.tiles[19][38] = TILE_LABYRINTH_FLOOR;
    game.map.tiles[21][40] = TILE_LABYRINTH_FLOOR;
    e->hp = 57;
    e->move_timer = 7;
    e->frozen_turns = 1;
    game.gold = 321;
    game.rook_quest_state = 1;
    game.rook_labyrinth_switches = 15;
    game.map.tiles[20][35] = TILE_LABYRINTH_GATE;
    map_mark_explored(&game.map, 38, 20);
    cache = &game.labyrinth_cache[4];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = 1;
    cache->enemies[0] = *e;
    cache->enemies[0].hp = 41;
    ok = make_legacy_save() && load_game(&loaded, 99185);
    ASSERT("version 128 migration adds pillars without healing active or cached boss", ok && loaded.enemies[0].hp == 57 && loaded.labyrinth_cache[4].enemies[0].hp == 41 && loaded.enemies[0].move_timer == 0 && loaded.map.tiles[19][38] == TILE_MINOTAUR_PILLAR && loaded.labyrinth_cache[4].map.tiles[21][40] == TILE_MINOTAUR_PILLAR);
    ASSERT("migration preserves all five-rune quest flags, gate, relic, position, gold and exploration", ok && loaded.rook_quest_state == 1 && loaded.rook_labyrinth_switches == 15 && loaded.map.tiles[20][35] == TILE_LABYRINTH_GATE && loaded.map.tiles[20][40] == TILE_LABYRINTH_RELIC && loaded.player.x == 37 && loaded.player.y == 20 && loaded.gold == 321 && map_is_explored(&loaded.map, 38, 20));
    e->active = 0;
    cache->enemies[0].active = 0;
    game.defeated_bosses |= 1 << LOCATION_LABYRINTH;
    ok = make_legacy_save() && load_game(&loaded, 99185);
    ASSERT("old victories remain defeated without fabricating or renaming a shield", ok && !loaded.enemies[0].active && loaded.minotaur_reward_state == MINOTAUR_REWARD_CLAIMED);
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 39, .y = 20, .underlying_tile = TILE_LABYRINTH_FLOOR, .item = item_make_magic_shield()};
    game.map.tiles[20][39] = TILE_ITEM;
    ok = make_legacy_save() && load_game(&loaded, 99185);
    ASSERT("identifiable old unclaimed Magic Shield retains its original name and tracked variant", ok && loaded.minotaur_reward_state == MINOTAUR_REWARD_LEGACY && strcmp(loaded.floor_items[0].item.name, "Magic Shield") == 0);
    loaded.floor_item_count = 0;
    loaded.map.tiles[20][39] = TILE_LABYRINTH_FLOOR;
    minotaur_restore_reward(&loaded);
    ok = save_game(&loaded, 99185) && load_game(&loaded, 99185);
    ASSERT("a new-format save retains the unclaimed legacy shield variant and cache", ok && loaded.minotaur_reward_state == MINOTAUR_REWARD_LEGACY && loaded.map.tiles[22][37] == TILE_MINOTAUR_RELIQUARY);
    loaded.player.x = 37;
    loaded.player.y = 22;
    minotaur_interact(&loaded);
    ASSERT("legacy cache recovers the original Magic Shield, not a retroactively renamed Aegis", loaded.minotaur_reward_state == 2 && loaded.inventory_count == 1 && strcmp(loaded.inventory[0].name, "Magic Shield") == 0 && loaded.inventory[0].visual_id == ITEM_VISUAL_MAGIC_SHIELD);
    remove("saves/savegame_99185.json");
}

static int escapes(Enemy *e, int x, int y, int turns) {
    if (!minotaur_attack_marks(&game, e, x, y)) {
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

static void test_routes_and_reward(void) {
    Enemy *e = setup();
    Item original = item_make_magic_shield();
    Item reward = item_make_labyrinth_aegis();
    ASSERT("Aegis matches Magic Shield stats, value, rarity, type and Warrior restriction", reward.defense_bonus == original.defense_bonus && reward.block_chance == original.block_chance && reward.value == original.value && reward.rarity == original.rarity && reward.type == original.type && reward.class_mask == original.class_mask && item_class_allowed(&reward, CLASS_WARRIOR) && !item_class_allowed(&reward, CLASS_ROGUE) && !item_class_allowed(&reward, CLASS_MAGE));
    int safe = 1;
    int checked = 0;
    for (int by = 18; by <= 22; by++) {
        for (int bx = 37; bx <= 41; bx++) {
            if (!map_is_walkable(&game.map, bx, by) || game.map.tiles[by][bx] == TILE_LABYRINTH_RELIC) {
                continue;
            }
            for (int y = 15; y <= 22; y++) {
                for (int x = 30; x <= 41; x++) {
                    if (!map_is_walkable(&game.map, x, y) || (x == bx && y == by)) {
                        continue;
                    }
                    for (int kind = 0; kind < 3; kind++) {
                        e->x = bx;
                        e->y = by;
                        game.player.x = x;
                        game.player.y = y;
                        e->hp = kind ? 60 : 120;
                        e->attack_phase = kind ? 1 : 0;
                        e->boss_wave_used = kind == 2 ? 2 : 0;
                        e->boss_attack = 0;
                        e->move_timer = 0;
                        minotaur_tick(&game, e);
                        safe &= escapes(e, x, y, e->move_timer);
                        checked++;
                    }
                }
            }
        }
    }
    ASSERT("all tested vault positions and approach lanes have ordinary walking escapes within each countdown", safe && checked > 1000);
    setup();
    int paths = 1;
    for (int floor = 1; floor <= 5; floor++) {
        int sx;
        int sy;
        map_generate_labyrinth(&game.map, floor, 31, &sx, &sy);
        if (floor == 5) {
            minotaur_prepare_map(&game.map, game.enemies, game.enemy_count, sx, sy);
        }
        static unsigned char seen[LABYRINTH_H][LABYRINTH_W];
        static int queue[LABYRINTH_H * LABYRINTH_W];
        memset(seen, 0, sizeof(seen));
        int head = 0;
        int tail = 0;
        queue[tail++] = sy * LABYRINTH_W + sx;
        seen[sy][sx] = 1;
        static const int steps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        while (head < tail) {
            int cell = queue[head++];
            int x = cell % LABYRINTH_W;
            int y = cell / LABYRINTH_W;
            for (int n = 0; n < 4; n++) {
                int nx = x + steps[n][0];
                int ny = y + steps[n][1];
                if (nx < 0 || nx >= LABYRINTH_W || ny < 0 || ny >= LABYRINTH_H || seen[ny][nx] || !map_is_walkable(&game.map, nx, ny)) {
                    continue;
                }
                seen[ny][nx] = 1;
                queue[tail++] = ny * LABYRINTH_W + nx;
            }
        }
        paths &= seen[game.map.stairs_down_y][game.map.stairs_down_x];
        for (int y = 0; y < LABYRINTH_H; y++) {
            for (int x = 0; x < LABYRINTH_W; x++) {
                if (game.map.tiles[y][x] == TILE_LABYRINTH_SWITCH_ON) {
                    paths &= seen[y][x];
                }
            }
        }
    }
    ASSERT("all five maze floors preserve accessible rune, stair and relic routes", paths);
    kill_with(0);
    game.floor_item_count = 0;
    game.map.tiles[20][39] = TILE_LABYRINTH_FLOOR;
    minotaur_restore_reward(&game);
    int x = 0;
    int y = 0;
    ASSERT("unclaimed Aegis returns to its named cache after floor loot clears", find_tile(TILE_MINOTAUR_RELIQUARY, &x, &y));
    game.player.x = x;
    game.player.y = y;
    minotaur_interact(&game);
    game.inventory_count = 0;
    minotaur_restore_reward(&game);
    ASSERT("selling a claimed shield cannot regenerate a replacement", game.minotaur_reward_state == 2 && !find_tile(TILE_MINOTAUR_RELIQUARY, &x, &y));
}

void test_minotaur_encounter(void) {
    printf("Minotaur phased encounter tests:\n");
    test_patterns();
    test_pillars();
    test_rewards();
    test_persistence();
    test_routes_and_reward();
}
