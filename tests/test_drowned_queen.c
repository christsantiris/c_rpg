#include "test_utils.h"
#include "approach_checks.h"
#include "../src/game/drowned_queen.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = LOCATION_COAST;
    game.level = COAST_DEPTH;
    game.max_coast_level_reached = COAST_DEPTH;
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
                game.map.tiles[y][x] = TILE_COAST_FLOOR;
            }
        }
    }
    for (int x = 5; x <= 20; x++) {
        game.map.tiles[16][x] = TILE_COAST_FLOOR;
    }
    for (int y = 5; y <= 16; y++) {
        game.map.tiles[y][5] = TILE_COAST_FLOOR;
    }
    game.map.stairs_up_x = 5;
    game.map.stairs_up_y = 5;
    game.map.tiles[5][5] = TILE_STAIRS_UP;
    game.map.stairs_down_x = 27;
    game.map.stairs_down_y = 16;
    game.map.tiles[16][27] = TILE_COAST_FLOOR;
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
    game.enemies[0] = (Enemy){.type = ENEMY_DROWNED_QUEEN, .name = "Drowned Queen", .x = 28, .y = 16,
        .active = 1, .is_boss = 1, .hp = 220, .max_hp = 220, .attack = 22, .defense = 8,
        .experience = 650, .attack_target_x = -1, .attack_target_y = -1, .boss_aux_x = -1, .boss_aux_y = -1};
    drowned_queen_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
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
        int robes = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            robes += game.floor_items[i].active && strcmp(game.floor_items[i].item.name, "Tidecaller Robes") == 0;
        }
        victories &= !game.enemies[0].active && robes == 1 && game.drowned_queen_reward_state == 1 && (game.defeated_bosses & (1 << LOCATION_COAST));
    }
    ASSERT("melee, bow, Magic Arrow, Fireball and Frost Bolt each guarantee one Tidecaller Robes", victories);
    Enemy *e = setup();
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 100;
    e->hp = 1;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 22, .y = 20, .underlying_tile = TILE_COAST_FLOOR, .item = item_make_health_potion()};
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
    ASSERT("full floor capacity preserves every item and reserves reward in a reliquary", game.floor_item_count == MAX_FLOOR_ITEMS && game.floor_items[MAX_FLOOR_ITEMS - 1].item.type == ITEM_POTION_HEALTH && game.drowned_queen_reward_state == 1 && find_tile(TILE_QUEEN_RELIQUARY, &x, &y));
    game.player.x = x;
    game.player.y = y;
    game.map.tiles[y][x + 1] = TILE_QUEEN_CONTROL_H;
    game.map.tiles[y - 1][x] = TILE_QUEEN_CONTROL_H;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("full inventory cannot consume the reserved reward", game.drowned_queen_reward_state == 1 && game.map.tiles[y][x] == TILE_QUEEN_RELIQUARY);
    game.inventory_count--;
    int reward_score = game.score;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("reliquary pickup grants original Tidecaller Robes and marks it claimed", game.drowned_queen_reward_state == 2 && strcmp(game.inventory[MAX_INVENTORY - 1].name, "Tidecaller Robes") == 0 && game.inventory[MAX_INVENTORY - 1].defense_bonus == item_make_tidecaller_robes().defense_bonus && game.score == reward_score + game.inventory[MAX_INVENTORY - 1].value);
    drowned_queen_restore_reward(&game);
    ASSERT("claiming cannot recreate a second reward or operate adjacent tide controls", !find_tile(TILE_QUEEN_RELIQUARY, &x, &y) && game.map.tiles[game.player.y - 1][game.player.x] == TILE_QUEEN_CONTROL_H);
    kill_with(0);
    game.player.x = game.enemies[0].x;
    game.player.y = game.enemies[0].y;
    for (int i = 0; i < 3; i++) {
        action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    }
    ASSERT("normal ground pickup also records a claimed reward", game.drowned_queen_reward_state == 2);
}

static int make_legacy_save(int version) {
    if (!save_game(&game, 99181)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99181.json", "r");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    if (version < 127) {
        cJSON_DeleteItemFromObject(root, "drowned_queen_reward_state");
    }
    cJSON *arrays[2] = {cJSON_GetObjectItem(root, "enemies"),
        cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "coast_cache"), 4), "enemies")};
    for (int group = 0; group < 2 && version < 127; group++) {
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
    file = fopen("saves/savegame_99181.json", "w");
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
    ASSERT("Queen declares a two-turn tidal row without immediate damage", e->boss_attack == QUEEN_ROW && e->move_timer == 2 && game.player.hp == 1000 && drowned_queen_attack_marks(&game, e, 23, 16));
    int low = e->boss_aux_x;
    int high = e->boss_aux_y;
    e->hp = 110;
    e->frozen_turns = 2;
    game.player.y++;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze and crossing half health preserve warning phase, row and bounds", e->move_timer == 2 && e->attack_phase == 0 && e->attack_target_y == 16 && e->boss_aux_x == low && e->boss_aux_y == high);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("ordinary sidestep escapes wave and opens safe recovery", game.player.hp == 1000 && e->move_timer == -1);
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 10;
    int hp = e->hp;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    action_resolve_enemies(&game);
    ASSERT("melee counterattack during recovery has no extra Queen retaliation", e->hp < hp && game.player.hp == 1000 && e->move_timer == 0);
    action_resolve_enemies(&game);
    ASSERT("next preparation announces Storm Throne and a separate perpendicular wave", e->attack_phase == 1 && e->boss_attack == QUEEN_COLUMN && e->move_timer == 2 && e->attack_target_x == 27);
    action_resolve_player(&game, (Action){ACTION_MOVE, 26, 16});
    action_resolve_enemies(&game);
    ASSERT("column warning stays fixed after player moves", e->attack_target_x == 27 && e->move_timer == 1 && !drowned_queen_attack_marks(&game, e, 26, 16));
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("phase two adds two separately warned flooded pockets", e->boss_attack == QUEEN_POCKETS && e->move_timer == 2 && e->boss_aux_x >= 0 && e->boss_aux_y >= 0);
    int ax = e->boss_aux_x;
    int ay = e->boss_aux_y;
    action_resolve_player(&game, (Action){ACTION_MOVE, 26, 17});
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, 25, 17});
    action_resolve_enemies(&game);
    ASSERT("two actual cardinal moves escape pockets without retargeting or lingering damage", game.player.hp == 1000 && e->move_timer == -1 && e->boss_aux_x == -1 && game.map.tiles[17][25] == TILE_QUEEN_WATER && ax >= 0 && ay >= 0);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("Storm Throne repeats row, column and pocket sequence", e->boss_attack == QUEEN_ROW && e->move_timer == 2);

    e = setup();
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("remaining in a marked wave takes exactly one signature hit", game.player.hp == 978 && e->move_timer == -1);
    e = setup();
    e->attack_phase = 1;
    e->hp = 110;
    e->boss_attack = QUEEN_COLUMN;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("remaining in a storm pocket takes exactly one hit and no extra melee", e->boss_attack == QUEEN_POCKETS && e->move_timer == -1 && game.player.hp == 978);

    e = setup();
    game.map.tiles[16][26] = TILE_COAST_WALL;
    action_resolve_enemies(&game);
    ASSERT("solid cover prevents a wave target behind the wall", e->attack_target_x == -1 && !drowned_queen_attack_marks(&game, e, 23, 16));
    game.map.tiles[16][26] = TILE_QUEEN_WATER;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("removing cover during a warning cannot create an unannounced wave", game.player.hp == 1000 && e->move_timer == -1);

    e = setup();
    game.map.tiles[16][21] = TILE_COAST_DEEP_WATER;
    action_resolve_enemies(&game);
    low = e->boss_aux_x;
    game.map.tiles[16][21] = TILE_COAST_DRAINED_WATER;
    ASSERT("draining old deep water cannot extend a locked wave beyond its original bounds", low == 22 && !drowned_queen_attack_marks(&game, e, 21, 16));
    e = setup();
    game.player.x = 23;
    game.player.y = 11;
    game.map.tiles[16][27] = TILE_COAST_WALL;
    action_resolve_enemies(&game);
    ASSERT("closed diagonal corners preserve Queen line-of-sight rules", e->attack_target_x == -1);
    e = setup();
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_MERFOLK, .x = 26, .y = 16, .hp = 10, .max_hp = 10, .frozen_turns = 5};
    game.enemy_count = 2;
    action_resolve_enemies(&game);
    game.enemies[1].active = 0;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("an ally blocks preparation and its removal cannot retarget the warning", e->attack_target_x == -1 && game.player.hp == 1000);

    e = setup();
    e->hp--;
    action_resolve_enemies(&game);
    game.player.x = e->x - 13;
    action_resolve_enemies(&game);
    ASSERT("distant retreat pauses exact warning without healing or moving Queen", e->move_timer == 2 && e->hp == 219 && e->x == 28 && e->attack_target_x == 23);
    game.player.x = 23;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("returning resumes original two-turn wave", game.player.hp == 978 && e->move_timer == -1);
}

static void test_controls(void) {
    Enemy *e = setup();
    ASSERT("arena starts with two independent controls", drowned_queen_controls(&game) == 2);
    int hx = 0;
    int hy = 0;
    int vx = 0;
    int vy = 0;
    find_tile(TILE_QUEEN_CONTROL_H, &hx, &hy);
    find_tile(TILE_QUEEN_CONTROL_V, &vx, &vy);
    game.mara_quest_state = 1;
    game.mara_beacons_lit = 3;
    game.map.tiles[6][6] = TILE_COAST_TIDE_CONTROL;
    game.map.tiles[6][5] = TILE_COAST_DEEP_WATER;
    game.map.tiles[7][6] = TILE_COAST_CHANNEL_DRY;
    game.map.tiles[6][7] = TILE_COAST_BEACON_LIT;
    game.player.x = hx + 1;
    game.player.y = hy;
    action_resolve_enemies(&game);
    ASSERT("wave can warn the lane before its control drains it", drowned_queen_attack_marks(&game, e, game.player.x, game.player.y));
    game.floor_item_count = 2;
    game.floor_items[0] = (FloorItem){.active = 1, .x = hx + 2, .y = hy, .underlying_tile = TILE_QUEEN_WATER, .item = item_make_health_potion()};
    game.floor_items[1] = (FloorItem){.active = 1, .x = hx, .y = hy, .underlying_tile = TILE_QUEEN_CONTROL_H, .item = item_make_health_potion()};
    game.map.tiles[hy][hx + 2] = TILE_ITEM;
    game.map.tiles[hy][hx] = TILE_ITEM;
    ASSERT("contextual interaction finds control beneath loot", game_has_regional_interaction(&game));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("horizontal control drains its own permanent safe lane during a warning", game.map.tiles[hy][hx + 1] == TILE_QUEEN_DRY && !drowned_queen_attack_marks(&game, e, hx + 1, hy) && drowned_queen_controls(&game) == 1 && game.player.hp == 1000);
    ASSERT("draining preserves both loot entries and updates their underlays", game.floor_items[0].active && game.floor_items[0].underlying_tile == TILE_QUEEN_DRY && game.floor_items[1].active && game.floor_items[1].underlying_tile == TILE_QUEEN_CONTROL_H_DRAINED);
    ASSERT("Queen control preserves ordinary basins, lit beacons and quest progress", game.map.tiles[6][5] == TILE_COAST_DEEP_WATER && game.map.tiles[7][6] == TILE_COAST_CHANNEL_DRY && game.map.tiles[6][7] == TILE_COAST_BEACON_LIT && game.mara_beacons_lit == 3);
    action_resolve_enemies(&game);
    ASSERT("drained lane remains safe when queued wave resolves", game.player.hp == 1000 && e->move_timer == -1);
    game.player.x = vx;
    game.player.y = vy;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("vertical control drains its own crossing path without reflooding horizontal path", drowned_queen_controls(&game) == 0 && game.map.tiles[16][vx] == TILE_QUEEN_DRY && game.map.tiles[hy][hx + 1] == TILE_QUEEN_DRY);
    game.player.x = 6;
    game.player.y = 6;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("old basin control still swaps its water without changing Queen's drained lanes", game.map.tiles[6][5] == TILE_COAST_DRAINED_WATER && game.map.tiles[7][6] == TILE_COAST_CHANNEL_WATER && game.map.tiles[hy][hx + 1] == TILE_QUEEN_DRY && drowned_queen_controls(&game) == 0 && game.mara_beacons_lit == 3);
    drowned_queen_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    ASSERT("map preparation never resets drained controls or lanes", drowned_queen_controls(&game) == 0 && game.map.tiles[16][vx] == TILE_QUEEN_DRY);
    int ix = 28;
    int iy = 13;
    game.map.tiles[iy][ix] = TILE_QUEEN_ISLAND;
    game.player.x = ix;
    game.player.y = iy;
    e->attack_target_x = ix;
    e->attack_target_y = iy;
    e->boss_aux_x = ix + 4;
    e->boss_aux_y = iy;
    e->boss_attack = QUEEN_POCKETS;
    e->move_timer = 2;
    ASSERT("retired island tiles no longer grant immunity inside a storm pocket", map_is_walkable(&game.map, ix, iy) && drowned_queen_attack_marks(&game, e, ix, iy));
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("remaining on a retired island takes storm damage", game.player.hp == 978);
    setup();
    for (int yy = 11; yy < 21; yy++) {
        for (int xx = 21; xx < 33; xx++) {
            if (game.map.tiles[yy][xx] >= TILE_QUEEN_WATER && game.map.tiles[yy][xx] <= TILE_QUEEN_CONTROL_V_DRAINED) {
                game.map.tiles[yy][xx] = TILE_COAST_FLOOR;
            }
        }
    }
    game.map.tiles[14][24] = TILE_COAST_DRAINED_WATER;
    game.map.tiles[14][25] = TILE_COAST_DEEP_WATER;
    game.map.tiles[14][26] = TILE_COAST_CHANNEL_DRY;
    game.map.tiles[14][27] = TILE_COAST_BEACON_LIT;
    drowned_queen_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    ASSERT("arena migration preserves pre-existing basin and beacon tile identities inside the throne", game.map.tiles[14][24] == TILE_COAST_DRAINED_WATER && game.map.tiles[14][25] == TILE_COAST_DEEP_WATER && game.map.tiles[14][26] == TILE_COAST_CHANNEL_DRY && game.map.tiles[14][27] == TILE_COAST_BEACON_LIT && drowned_queen_controls(&game) == 2);
    int classes = 1;
    for (int c = 0; c < 3; c++) {
        setup();
        find_tile(TILE_QUEEN_CONTROL_H, &hx, &hy);
        game.player.player_class = c == 0 ? CLASS_WARRIOR : c == 1 ? CLASS_ROGUE : CLASS_MAGE;
        game.player.x = hx;
        game.player.y = hy;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        classes &= drowned_queen_controls(&game) == 1 && game.inventory_count == 0;
    }
    ASSERT("every class drains lanes without equipment or consumables", classes);
}

static void test_persistence(void) {
    Enemy *e = setup();
    int x = 0;
    int y = 0;
    find_tile(TILE_QUEEN_CONTROL_H, &x, &y);
    game.player.x = x;
    game.player.y = y;
    drowned_queen_interact(&game);
    e->hp = 79;
    e->attack_phase = 1;
    e->boss_attack = QUEEN_POCKETS;
    e->move_timer = 2;
    e->frozen_turns = 2;
    e->attack_target_x = 23;
    e->attack_target_y = 16;
    e->boss_aux_x = 30;
    e->boss_aux_y = 19;
    game.coast_cache[4].valid = 1;
    game.coast_cache[4].map = game.map;
    game.coast_cache[4].enemy_count = 1;
    game.coast_cache[4].enemies[0] = *e;
    game.drowned_queen_reward_state = 1;
    int ok = save_game(&game, 99181) && load_game(&loaded, 99181);
    ASSERT("save preserves active phase, frozen countdown and both locked pool targets", ok && loaded.enemies[0].hp == 79 && loaded.enemies[0].attack_phase == 1 && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].frozen_turns == 2 && loaded.enemies[0].attack_target_x == 23 && loaded.enemies[0].boss_aux_x == 30 && loaded.enemies[0].boss_aux_y == 19);
    ASSERT("save preserves active/cached drained lane, control and unclaimed robes", ok && loaded.map.tiles[y][x] == TILE_QUEEN_CONTROL_H_DRAINED && loaded.coast_cache[4].map.tiles[y][x + 1] == TILE_QUEEN_DRY && loaded.coast_cache[4].enemies[0].boss_aux_y == 19 && loaded.drowned_queen_reward_state == 1);
    e->boss_attack = QUEEN_ROW;
    e->boss_aux_x = 21;
    e->boss_aux_y = 32;
    game.coast_cache[4].enemies[0] = *e;
    ok = save_game(&game, 99181) && load_game(&loaded, 99181);
    ASSERT("save restores exact row bounds in active and cached warnings", ok && loaded.enemies[0].boss_attack == QUEEN_ROW && loaded.enemies[0].boss_aux_x == 21 && loaded.enemies[0].boss_aux_y == 32 && loaded.coast_cache[4].enemies[0].boss_aux_y == 32 && !drowned_queen_attack_marks(&loaded, &loaded.enemies[0], 20, 16));
    e->boss_attack = QUEEN_POCKETS;
    e->boss_aux_x = 30;
    e->boss_aux_y = 19;
    game.drowned_queen_reward_state = 2;
    e->move_timer = -2;
    ok = save_game(&game, 99181) && load_game(&loaded, 99181);
    ASSERT("save also retains claimed robes and recovery timing", ok && loaded.drowned_queen_reward_state == 2 && loaded.enemies[0].move_timer == -2);
    e->move_timer = 2;
    game.coast_cache[4].enemies[0] = *e;
    game.portal_active = 1;
    game.portal_location = LOCATION_COAST;
    game.portal_level = COAST_DEPTH;
    game.portal_x = 23;
    game.portal_y = 16;
    game.portal_origin_tile = TILE_QUEEN_WATER;
    game.location = LOCATION_TOWN;
    game_use_town_portal(&game);
    ASSERT("portal revisit restores queued pools and keeps the drained control spent", game.location == LOCATION_COAST && game.enemies[0].move_timer == 2 && game.enemies[0].boss_aux_x == 30 && drowned_queen_controls(&game) == 1);

    e = setup();
    for (int yy = 11; yy < 21; yy++) {
        for (int xx = 21; xx < 33; xx++) {
            if (game.map.tiles[yy][xx] >= TILE_QUEEN_WATER && game.map.tiles[yy][xx] <= TILE_QUEEN_CONTROL_V_DRAINED) {
                game.map.tiles[yy][xx] = TILE_COAST_FLOOR;
            }
        }
    }
    e->hp = 79;
    e->move_timer = 37;
    game.gold = 321;
    game.mara_quest_state = 2;
    game.mara_beacons_lit = 7;
    map_mark_explored(&game.map, 22, 12);
    game.coast_cache[4].valid = 1;
    game.coast_cache[4].map = game.map;
    game.coast_cache[4].enemy_count = 1;
    game.coast_cache[4].enemies[0] = *e;
    game.coast_cache[4].enemies[0].hp = 51;
    game.portal_active = 1;
    game.portal_location = LOCATION_COAST;
    game.portal_level = COAST_DEPTH;
    game.portal_x = 26;
    game.portal_y = 12;
    game.portal_origin_tile = TILE_COAST_FLOOR;
    ok = make_legacy_save(126) && load_game(&loaded, 99181);
    ASSERT("version 126 migration adds two controls without healing active or cached Queen", ok && loaded.enemies[0].hp == 79 && loaded.coast_cache[4].enemies[0].hp == 51 && loaded.enemies[0].move_timer == 0 && drowned_queen_controls(&loaded) == 2);
    ASSERT("migration preserves quests, beacon completion, exploration, gold, position and portal", loaded.mara_quest_state == 2 && loaded.mara_beacons_lit == 7 && loaded.gold == 321 && loaded.player.x == 23 && loaded.player.y == 16 && map_is_explored(&loaded.map, 22, 12) && loaded.map.tiles[12][26] == TILE_COAST_FLOOR && loaded.coast_cache[4].map.tiles[12][26] == TILE_COAST_FLOOR);
    game.enemies[0].active = 0;
    game.coast_cache[4].enemies[0].active = 0;
    game.defeated_bosses |= 1 << LOCATION_COAST;
    ok = make_legacy_save(126) && load_game(&loaded, 99181);
    ASSERT("legacy victories remain defeated without fabricating another robes reward", ok && !loaded.enemies[0].active && loaded.drowned_queen_reward_state == 2);
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 24, .y = 12, .underlying_tile = TILE_COAST_FLOOR, .item = item_make_tidecaller_robes()};
    game.map.tiles[12][24] = TILE_ITEM;
    ok = make_legacy_save(126) && load_game(&loaded, 99181);
    ASSERT("known legacy dropped robes become persistently recoverable", ok && loaded.drowned_queen_reward_state == 1 && loaded.floor_items[0].active);
    remove("saves/savegame_99181.json");
}

static void test_island_removal_migration(void) {
    for (int scenario = 0; scenario < 3; scenario++) {
        Enemy *e = setup();
        e->hp = 79;
        e->attack_phase = 1;
        e->boss_attack = QUEEN_POCKETS;
        e->move_timer = 2;
        e->frozen_turns = 2;
        e->attack_target_x = 28;
        e->attack_target_y = 13;
        e->boss_aux_x = 31;
        e->boss_aux_y = 16;
        e->active = scenario != 2;
        game.map.tiles[13][28] = TILE_ITEM;
        game.map.tiles[16][31] = TILE_QUEEN_ISLAND;
        game.floor_item_count = 1;
        game.floor_items[0] = (FloorItem){.active = 1, .x = 28, .y = 13,
            .underlying_tile = TILE_QUEEN_ISLAND, .item = item_make_health_potion()};
        game.player.x = 28;
        game.player.y = 13;
        game.gold = 321;
        game.mara_quest_state = 2;
        game.mara_beacons_lit = 7;
        game.drowned_queen_reward_state = 2;
        if (scenario == 2) {
            game.defeated_bosses |= 1 << LOCATION_COAST;
        }
        map_mark_explored(&game.map, 31, 16);
        game.coast_cache[4].valid = 1;
        game.coast_cache[4].map = game.map;
        game.coast_cache[4].enemy_count = 1;
        game.coast_cache[4].enemies[0] = *e;
        game.portal_active = 1;
        game.portal_location = LOCATION_COAST;
        game.portal_level = COAST_DEPTH;
        game.portal_x = 31;
        game.portal_y = 16;
        game.portal_origin_tile = TILE_QUEEN_ISLAND;
        if (scenario == 1) {
            game_return_to_town(&game);
        }
        game.coast_cache[4].enemies[0].hp = 51;
        int ok = make_legacy_save(140) && load_game(&loaded, 99181);
        ASSERT("version 140 saves remove cached islands and portal island underlays", ok &&
            loaded.coast_cache[4].map.tiles[16][31] == TILE_QUEEN_WATER &&
            loaded.portal_origin_tile == TILE_QUEEN_WATER);
        ASSERT("removing islands preserves cached Queen damage, warning targets and freeze", ok &&
            loaded.coast_cache[4].enemies[0].hp == 51 &&
            loaded.coast_cache[4].enemies[0].move_timer == 2 &&
            loaded.coast_cache[4].enemies[0].frozen_turns == 2 &&
            loaded.coast_cache[4].enemies[0].boss_attack == QUEEN_POCKETS &&
            loaded.coast_cache[4].enemies[0].boss_aux_x == 31);
        if (scenario != 1) {
            ASSERT("active islands and loot underlays become water without losing items or moving the player", ok &&
                loaded.map.tiles[16][31] == TILE_QUEEN_WATER && loaded.map.tiles[13][28] == TILE_ITEM &&
                loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == TILE_QUEEN_WATER &&
                loaded.player.x == 28 && loaded.player.y == 13 && map_is_explored(&loaded.map, 31, 16));
            ASSERT("removing islands preserves the active encounter and defeated Queen state", ok &&
                loaded.enemies[0].hp == 79 && loaded.enemies[0].move_timer == 2 &&
                loaded.enemies[0].active == (scenario != 2) && loaded.defeated_bosses == game.defeated_bosses);
        }
        ASSERT("removing islands preserves quest completion, gold and claimed robes", ok &&
            loaded.mara_quest_state == 2 && loaded.mara_beacons_lit == 7 &&
            loaded.gold == 321 && loaded.drowned_queen_reward_state == 2);
        ASSERT("island removal persists when the migrated save is loaded again", save_game(&loaded, 99181) &&
            load_game(&loaded, 99181) && loaded.coast_cache[4].map.tiles[16][31] == TILE_QUEEN_WATER);
    }
    remove("saves/savegame_99181.json");
}

static void test_layouts_and_travel(void) {
    static unsigned char seen[MAP_H][MAP_W];
    static int queue[MAP_W * MAP_H];
    int valid = 1;
    for (int seed = 0; seed < 100; seed++) {
        setup();
        srand((unsigned)seed);
        map_generate_coast(&game.map, COAST_DEPTH);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        enemies_spawn(&game);
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
        int islands = 0;
        valid &= drowned_queen_controls(&game) == 2;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                TileType tile = game.map.tiles[y][x];
                if (tile == TILE_QUEEN_ISLAND || tile == TILE_QUEEN_CONTROL_H || tile == TILE_QUEEN_CONTROL_V) {
                    islands += tile == TILE_QUEEN_ISLAND;
                    valid &= seen[y][x] && map_is_walkable(&game.map, x, y);
                    for (int i = 0; i < game.enemy_count; i++) {
                        valid &= !game.enemies[i].active || game.enemies[i].x != x || game.enemies[i].y != y;
                    }
                }
            }
        }
        valid &= islands == 0 && seen[game.map.stairs_down_y][game.map.stairs_down_x];
    }
    ASSERT("100 thrones have no shelter islands and retain two reachable controls and accessible exits", valid);
    kill_with(0);
    game.floor_item_count = 0;
    map_generate_coast(&game.map, COAST_DEPTH);
    enemies_spawn(&game);
    game_refresh_quest_encounters(&game);
    int reward = 0;
    int living = 0;
    for (int i = 0; i < game.enemy_count; i++) {
        living |= game.enemies[i].active && game.enemies[i].type == ENEMY_DROWNED_QUEEN;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            reward |= game.map.tiles[y][x] == TILE_QUEEN_RELIQUARY;
        }
    }
    ASSERT("regenerated throne restores unclaimed robes without reviving defeated Queen", !living && reward && game.drowned_queen_reward_state == 1);
    game.player.x = game.map.stairs_down_x;
    game.player.y = game.map.stairs_down_y;
    Action exit = {ACTION_MOVE, game.player.x, game.player.y};
    if (exit.target_x == 1) {
        exit.target_x--;
    } else if (exit.target_x == MAP_W - 2) {
        exit.target_x++;
    } else if (exit.target_y == 1) {
        exit.target_y--;
    } else {
        exit.target_y++;
    }
    action_resolve_player(&game, exit);
    walk_return_approach(&game);
    ASSERT("Queen victory still opens coast exit and harbor access with regular enemies alive", game.location == LOCATION_TOWN && game_harbor_unlocked(&game));
}

void test_drowned_queen(void) {
    printf("Drowned Queen phased encounter tests:\n");
    test_patterns();
    test_controls();
    test_rewards();
    test_persistence();
    test_island_removal_migration();
    test_layouts_and_travel();
}
