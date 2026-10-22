#include "test_utils.h"
#include "../src/game/goblin_king.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = LOCATION_MOUNTAINS;
    game.level = MOUNTAIN_BOSS_LEVEL;
    game.max_mountain_level_reached = MOUNTAIN_BOSS_LEVEL;
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
                game.map.tiles[y][x] = TILE_MOUNTAIN_FORTRESS_FLOOR;
            }
        }
    }
    for (int x = 5; x <= 20; x++) {
        game.map.tiles[16][x] = TILE_MOUNTAIN_FORTRESS_FLOOR;
    }
    for (int y = 5; y <= 16; y++) {
        game.map.tiles[y][5] = TILE_MOUNTAIN_FORTRESS_FLOOR;
    }
    game.map.stairs_up_x = 5;
    game.map.stairs_up_y = 5;
    game.map.tiles[5][5] = TILE_STAIRS_UP;
    game.map.stairs_down_x = 27;
    game.map.stairs_down_y = 16;
    game.map.tiles[16][27] = TILE_MOUNTAIN_FORTRESS_FLOOR;
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
    game.enemies[0] = (Enemy){.type = ENEMY_MOUNTAIN_GOBLIN_KING, .name = "Goblin King", .x = 28, .y = 16,
        .active = 1, .is_boss = 1, .hp = 180, .max_hp = 180, .attack = 20, .defense = 8,
        .experience = 500, .attack_target_x = -1, .attack_target_y = -1, .boss_aux_x = -1, .boss_aux_y = -1};
    goblin_king_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
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
        int shields = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            shields += game.floor_items[i].active && strcmp(game.floor_items[i].item.name, "Goblin King's Shield") == 0;
        }
        victories &= !game.enemies[0].active && shields == 1 && game.goblin_king_reward_state == 1 && (game.defeated_bosses & (1 << LOCATION_MOUNTAINS));
    }
    ASSERT("melee, bow, Magic Arrow, Fireball and Frost Bolt each guarantee one Goblin King's Shield", victories);
    Enemy *e = setup();
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 100;
    e->hp = 1;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 22, .y = 20, .underlying_tile = TILE_MOUNTAIN_FORTRESS_FLOOR, .item = item_make_health_potion()};
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
    ASSERT("full floor capacity preserves every item and reserves reward in a reliquary", game.floor_item_count == MAX_FLOOR_ITEMS && game.floor_items[MAX_FLOOR_ITEMS - 1].item.type == ITEM_POTION_HEALTH && game.goblin_king_reward_state == 1 && find_tile(TILE_GOBLIN_KING_RELIQUARY, &x, &y));
    game.player.x = x;
    game.player.y = y;
    game.map.tiles[y][x + 1] = TILE_GOBLIN_KEG;
    game.map.tiles[y - 1][x] = TILE_GOBLIN_KEG;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("full inventory cannot consume the reserved reward", game.goblin_king_reward_state == 1 && game.map.tiles[y][x] == TILE_GOBLIN_KING_RELIQUARY);
    game.inventory_count--;
    int reward_score = game.score;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("reliquary pickup grants original Goblin King's Shield and marks it claimed", game.goblin_king_reward_state == 2 && strcmp(game.inventory[MAX_INVENTORY - 1].name, "Goblin King's Shield") == 0 && game.inventory[MAX_INVENTORY - 1].defense_bonus == item_make_goblin_king_shield().defense_bonus && game.score == reward_score + game.inventory[MAX_INVENTORY - 1].value);
    goblin_king_restore_reward(&game);
    ASSERT("claiming cannot recreate a second reward or prime adjacent kegs", !find_tile(TILE_GOBLIN_KING_RELIQUARY, &x, &y) && game.map.tiles[game.player.y - 1][game.player.x] == TILE_GOBLIN_KEG);
    kill_with(0);
    game.player.x = game.enemies[0].x;
    game.player.y = game.enemies[0].y;
    for (int i = 0; i < 3; i++) {
        action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    }
    ASSERT("normal ground pickup also records a claimed reward", game.goblin_king_reward_state == 2);
}

static int make_legacy_save(void) {
    if (!save_game(&game, 99179)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99179.json", "r");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 125);
    cJSON_DeleteItemFromObject(root, "goblin_king_reward_state");
    cJSON *arrays[2] = {cJSON_GetObjectItem(root, "enemies"),
        cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "mountain_cache"), 3), "enemies")};
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
    file = fopen("saves/savegame_99179.json", "w");
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

static void spend_kegs(void) {
    for (int y = 11; y < 21; y++) {
        for (int x = 21; x < 33; x++) {
            if (game.map.tiles[y][x] == TILE_GOBLIN_KEG) {
                game.map.tiles[y][x] = TILE_GOBLIN_KEG_SPENT;
            }
        }
    }
}

static void test_patterns(void) {
    Enemy *e = setup();
    spend_kegs();
    action_resolve_enemies(&game);
    ASSERT("King declares one fixed axe lane without immediate damage", e->boss_attack == GOBLIN_KING_AXE && e->move_timer == 1 && game.player.hp == 1000 && goblin_king_attack_marks(&game, e, 23, 16));
    e->hp = 90;
    e->frozen_turns = 2;
    game.player.y++;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze and half-health crossing preserve pending axe shape and countdown", e->move_timer == 1 && e->attack_phase == 0 && e->attack_target_x == 23 && e->attack_target_y == 16);
    action_resolve_enemies(&game);
    ASSERT("one ordinary sidestep avoids the fixed throw and opens recovery", game.player.hp == 1000 && e->move_timer == -1);
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 10;
    int hp = e->hp;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    action_resolve_enemies(&game);
    ASSERT("safe melee counterattack during recovery has no extra King retaliation", e->hp < hp && e->move_timer == 0 && game.player.hp == 1000);
    action_resolve_enemies(&game);
    ASSERT("next preparation declares Powder King and two-turn bomb pockets", e->attack_phase == 1 && e->boss_attack == GOBLIN_KING_BOMBS && e->move_timer == 2 && e->boss_aux_x >= 0);
    int tx = e->attack_target_x;
    int ty = e->attack_target_y;
    int ax = e->boss_aux_x;
    int ay = e->boss_aux_y;
    game.player.x++;
    action_resolve_enemies(&game);
    ASSERT("bombs keep both landing sites fixed after player movement", e->move_timer == 1 && e->attack_target_x == tx && e->attack_target_y == ty && e->boss_aux_x == ax && e->boss_aux_y == ay);
    game.player.y++;
    action_resolve_enemies(&game);
    ASSERT("two cardinal moves escape a bomb pocket without special abilities", game.player.hp == 1000 && e->move_timer == -1);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("Powder King alternates bombardment with axe throws", e->boss_attack == GOBLIN_KING_AXE && e->move_timer == 1);

    e = setup();
    spend_kegs();
    e->hp = 90;
    e->attack_phase = 1;
    e->boss_attack = GOBLIN_KING_AXE;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("remaining in marked bombardment deals one signature hit", game.player.hp == 1000 - e->attack && e->move_timer == -1);

    e = setup();
    spend_kegs();
    game.map.tiles[16][26] = TILE_MOUNTAIN_WALL;
    EnemyProjectiles shots;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("solid cover clips axe warning before the player", !goblin_king_attack_marks(&game, e, 23, 16) && e->attack_target_x == 27);
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("cover also clips projectile and prevents axe damage", shots.count == 1 && shots.shots[0].target_x == 27 && game.player.hp == 1000);
    e = setup();
    spend_kegs();
    game.player.x = 23;
    game.player.y = 11;
    game.map.tiles[16][27] = TILE_MOUNTAIN_WALL;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("a closed diagonal corner clips the initial warning to King", e->attack_target_x == e->x && e->attack_target_y == e->y && !goblin_king_attack_marks(&game, e, 27, 15));
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("axe projectile cannot cut through a closed diagonal corner", shots.count == 0 && game.player.hp == 1000);
    e = setup();
    spend_kegs();
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_GOBLIN_SCOUT, .x = 26, .y = 16, .hp = 10, .max_hp = 10, .frozen_turns = 5};
    game.enemy_count = 2;
    action_resolve_enemies(&game);
    game.enemies[1].active = 0;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("removing a blocker never extends an axe beyond its declared endpoint", game.player.hp == 1000 && shots.count == 1 && shots.shots[0].target_x == 27);

    e = setup();
    spend_kegs();
    e->hp--;
    action_resolve_enemies(&game);
    game.player.x = e->x - 13;
    action_resolve_enemies(&game);
    ASSERT("retreat beyond range pauses exact warning without healing or moving King", e->move_timer == 1 && e->hp == 179 && e->x == 28 && e->attack_target_x == 23);
    game.player.x = 23;
    action_resolve_enemies(&game);
    ASSERT("returning resumes the original warned throw", game.player.hp == 980 && e->move_timer == -1);
}

static void test_kegs(void) {
    Enemy *e = setup();
    ASSERT("three finite kegs start walkable and unlit", goblin_king_remaining_kegs(&game) == 3 && map_is_walkable(&game.map, 27, 16));
    action_resolve_enemies(&game);
    game.player.y++;
    action_resolve_enemies(&game);
    ASSERT("warned axe lights only the keg in its fixed path with a two-turn fuse", goblin_king_keg_fuse(&game, 27, 16) == 2 && !goblin_king_keg_fuse(&game, 29, 16) && game.player.hp == 1000);
    action_resolve_enemies(&game);
    ASSERT("keg footprint and countdown remain visible during first fuse turn", goblin_king_keg_fuse(&game, 27, 16) == 1 && goblin_king_keg_marks(&game, 28, 16) == 1 && !goblin_king_keg_marks(&game, 25, 16));
    action_resolve_enemies(&game);
    ASSERT("keg catching King damages him, consumes itself and gives two complete recovery turns", e->hp == 150 && e->move_timer == -2 && game.map.tiles[16][27] == TILE_GOBLIN_KEG_SPENT && goblin_king_remaining_kegs(&game) == 2);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("stagger stays safe for both player actions", e->move_timer == 0 && game.player.hp == 1000);
    goblin_king_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    ASSERT("spent keg is never replenished by map preparation", game.map.tiles[16][27] == TILE_GOBLIN_KEG_SPENT && goblin_king_remaining_kegs(&game) == 2);

    e = setup();
    game.player.x = 26;
    game.player.y = 16;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 27, .y = 16, .underlying_tile = TILE_GOBLIN_KEG, .item = item_make_health_potion()};
    game.map.tiles[16][27] = TILE_ITEM;
    ASSERT("contextual Interact recognizes a keg beneath loot", game_has_regional_interaction(&game));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("interaction leaves two complete fuse turns without deleting loot", goblin_king_keg_fuse(&game, 27, 16) == 2 && game.floor_items[0].active && game.floor_items[0].underlying_tile == TILE_GOBLIN_KEG_FUSE_2 && game.player.hp == 1000);
    game.player.x--;
    action_resolve_enemies(&game);
    game.player.x--;
    action_resolve_enemies(&game);
    ASSERT("ordinary movement escapes own keg and preserves spent underlay", game.player.hp == 1000 && game.floor_items[0].underlying_tile == TILE_GOBLIN_KEG_SPENT && e->hp == 150);

    e = setup();
    game.player.x = 27;
    game.player.y = 16;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, 26, 16});
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, 25, 16});
    action_resolve_enemies(&game);
    ASSERT("two actual cardinal moves escape a keg from its center", game.player.x == 25 && game.player.y == 16 && game.player.hp == 1000 && e->hp == 150);
    int all_classes = 1;
    for (int c = 0; c < 3; c++) {
        setup();
        game.player.player_class = c == 0 ? CLASS_WARRIOR : c == 1 ? CLASS_ROGUE : CLASS_MAGE;
        game.player.x = 26;
        game.player.y = 16;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        action_resolve_enemies(&game);
        all_classes &= goblin_king_keg_fuse(&game, 27, 16) == 2 && game.inventory_count == 0;
    }
    ASSERT("every class primes kegs without equipment, ammunition or consumables", all_classes);

    e = setup();
    game.map.tiles[16][27] = TILE_GOBLIN_KEG_FUSE_2;
    e->frozen_turns = 3;
    e->move_timer = 2;
    e->boss_attack = GOBLIN_KING_BOMBS;
    e->attack_target_x = 23;
    e->attack_target_y = 16;
    e->boss_aux_x = 27;
    e->boss_aux_y = 16;
    action_resolve_enemies(&game);
    ASSERT("freezing King pauses pending bombs while active keg fuse keeps ticking", e->move_timer == 2 && e->frozen_turns == 2 && goblin_king_keg_fuse(&game, 27, 16) == 1);
    action_resolve_enemies(&game);
    ASSERT("explosion cancels frozen warning and safely staggers King", e->hp == 150 && e->move_timer == -3 && e->attack_target_x == -1 && e->boss_aux_x == -1);

    e = setup();
    game.player.x = 27;
    game.player.y = 16;
    game.map.tiles[16][27] = TILE_GOBLIN_KEG_FUSE_1;
    game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_GOBLIN_SCOUT, .x = 27, .y = 15, .hp = 1, .max_hp = 10, .experience = 7};
    game.enemy_count = 2;
    int xp = game.player.experience;
    action_resolve_enemies(&game);
    ASSERT("marked keg blast can hit player and kill nearby regular enemies with normal XP", game.player.hp == 980 && !game.enemies[1].active && game.player.experience == xp + 7);

    e = setup();
    game.player.x = 27;
    game.player.y = 16;
    game.map.tiles[16][27] = TILE_GOBLIN_KEG_FUSE_1;
    game.map.tiles[15][27] = TILE_GOBLIN_KEG_FUSE_1;
    action_resolve_enemies(&game);
    ASSERT("overlapping warned blasts hit each actor once per turn", game.player.hp == 980 && e->hp == 150);

    e = setup();
    spend_kegs();
    game.map.tiles[12][22] = TILE_GOBLIN_KEG_FUSE_1;
    e->boss_attack = GOBLIN_KING_BOMBS;
    e->move_timer = 2;
    e->attack_target_x = 22;
    e->attack_target_y = 13;
    e->boss_aux_x = 30;
    e->boss_aux_y = 19;
    action_resolve_enemies(&game);
    ASSERT("blast removes nearby bomb pocket while preserving remote locked pocket", e->hp == 180 && e->move_timer == 1 && e->attack_target_x == -1 && e->boss_aux_x == 30 && e->boss_aux_y == 19);

    e = setup();
    e->hp = 1;
    game.map.tiles[16][27] = TILE_GOBLIN_KEG_FUSE_1;
    action_resolve_enemies(&game);
    int shields = 0;
    for (int i = 0; i < game.floor_item_count; i++) {
        shields += game.floor_items[i].active && strcmp(game.floor_items[i].item.name, "Goblin King's Shield") == 0;
    }
    ASSERT("keg victory records defeat, original shield and shortcut discovery exactly once", !e->active && shields == 1 && game.goblin_king_reward_state == 1 && (game.defeated_bosses & (1 << LOCATION_MOUNTAINS)) && !game_shortcut_prompt_active(&game));
    int victory_xp = game.player.experience;
    action_resolve_enemies(&game);
    ASSERT("spent kegs cannot duplicate boss XP or reward", shields == 1 && game.player.experience == victory_xp && game.goblin_king_reward_state == 1);
}

static void test_saves_and_layouts(void) {
    Enemy *e = setup();
    game.map.tiles[16][27] = TILE_GOBLIN_KEG_FUSE_1;
    game.map.tiles[16][29] = TILE_GOBLIN_KEG_SPENT;
    e->hp = 90;
    e->attack_phase = 1;
    e->boss_attack = GOBLIN_KING_BOMBS;
    e->move_timer = 2;
    e->frozen_turns = 2;
    e->attack_target_x = 23;
    e->attack_target_y = 16;
    e->boss_aux_x = 30;
    e->boss_aux_y = 19;
    game.mountain_cache[MOUNTAIN_BOSS_LEVEL - 1].valid = 1;
    game.mountain_cache[MOUNTAIN_BOSS_LEVEL - 1].map = game.map;
    game.mountain_cache[MOUNTAIN_BOSS_LEVEL - 1].enemy_count = 1;
    game.mountain_cache[MOUNTAIN_BOSS_LEVEL - 1].enemies[0] = *e;
    game.goblin_king_reward_state = 1;
    int ok = save_game(&game, 99179) && load_game(&loaded, 99179);
    ASSERT("save preserves active phase, frozen bomb countdown and both fixed pockets", ok && loaded.enemies[0].hp == 90 && loaded.enemies[0].attack_phase == 1 && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].frozen_turns == 2 && loaded.enemies[0].attack_target_x == 23 && loaded.enemies[0].boss_aux_x == 30 && loaded.enemies[0].boss_aux_y == 19);
    ASSERT("save preserves active/cached keg fuses, spent state and reward claim state", ok && loaded.map.tiles[16][27] == TILE_GOBLIN_KEG_FUSE_1 && loaded.mountain_cache[MOUNTAIN_BOSS_LEVEL - 1].map.tiles[16][29] == TILE_GOBLIN_KEG_SPENT && loaded.mountain_cache[MOUNTAIN_BOSS_LEVEL - 1].enemies[0].boss_aux_y == 19 && loaded.goblin_king_reward_state == 1);
    game.goblin_king_reward_state = 2;
    e->move_timer = -2;
    ok = save_game(&game, 99179) && load_game(&loaded, 99179);
    ASSERT("save also retains claimed shield and recovery timing", ok && loaded.goblin_king_reward_state == 2 && loaded.enemies[0].move_timer == -2);
    game.portal_active = 1;
    game.portal_location = LOCATION_MOUNTAINS;
    game.portal_level = MOUNTAIN_BOSS_LEVEL;
    game.portal_x = 23;
    game.portal_y = 16;
    game.portal_origin_tile = TILE_MOUNTAIN_FORTRESS_FLOOR;
    game.location = LOCATION_TOWN;
    game_use_town_portal(&game);
    ASSERT("portal revisit restores fixed bomb pockets and never resets spent kegs", game.location == LOCATION_MOUNTAINS && game.enemies[0].boss_aux_x == 30 && game.enemies[0].move_timer == 2 && game.map.tiles[16][29] == TILE_GOBLIN_KEG_SPENT && goblin_king_keg_fuse(&game, 27, 16) == 1);

    e = setup();
    game.level = 4;
    e->hp = 79;
    e->move_timer = 37;
    spend_kegs();
    for (int y = 11; y < 21; y++) {
        for (int x = 21; x < 33; x++) {
            if (game.map.tiles[y][x] == TILE_GOBLIN_KEG_SPENT) {
                game.map.tiles[y][x] = TILE_MOUNTAIN_FORTRESS_FLOOR;
            }
        }
    }
    game.gold = 321;
    game.dain_quest_state = 2;
    game.dain_map_fragments = 7;
    map_mark_explored(&game.map, 22, 12);
    game.mountain_cache[3].valid = 1;
    game.mountain_cache[3].map = game.map;
    game.mountain_cache[3].enemy_count = 1;
    game.mountain_cache[3].enemies[0] = *e;
    game.mountain_cache[3].enemies[0].hp = 51;
    game.portal_active = 1;
    game.portal_location = LOCATION_MOUNTAINS;
    game.portal_level = 4;
    game.portal_x = 27;
    game.portal_y = 16;
    game.portal_origin_tile = TILE_MOUNTAIN_FORTRESS_FLOOR;
    ok = make_legacy_save() && load_game(&loaded, 99179);
    ASSERT("version 125 migration adds finite kegs without healing active or cached King", ok && loaded.enemies[0].hp == 79 && loaded.mountain_cache[4].enemies[0].hp == 51 && loaded.enemies[0].move_timer == 0 && goblin_king_remaining_kegs(&loaded) == 3);
    ASSERT("migration preserves quests, fragments, exploration, gold, position and portal landing", loaded.dain_quest_state == 2 && loaded.dain_map_fragments == 7 && loaded.gold == 321 && loaded.player.x == 23 && loaded.player.y == 16 && map_is_explored(&loaded.map, 22, 12) && loaded.map.tiles[16][27] == TILE_MOUNTAIN_FORTRESS_FLOOR && loaded.mountain_cache[4].map.tiles[16][27] == TILE_MOUNTAIN_FORTRESS_FLOOR);
    game.map.tiles[16][27] = TILE_MOUNTAIN_FLOOR;
    game.mountain_cache[3].map.tiles[16][27] = TILE_MOUNTAIN_FLOOR;
    game.portal_origin_tile = TILE_MOUNTAIN_FLOOR;
    ok = make_legacy_save() && load_game(&loaded, 99179);
    ASSERT("migration also protects older plain mountain portal floor in active and cached arenas", ok && loaded.map.tiles[16][27] == TILE_MOUNTAIN_FLOOR && loaded.mountain_cache[4].map.tiles[16][27] == TILE_MOUNTAIN_FLOOR);
    game.enemies[0].active = 0;
    game.mountain_cache[3].enemies[0].active = 0;
    game.defeated_bosses |= 1 << LOCATION_MOUNTAINS;
    ok = make_legacy_save() && load_game(&loaded, 99179);
    ASSERT("legacy victories stay defeated without manufacturing a new shield", ok && !loaded.enemies[0].active && loaded.goblin_king_reward_state == 2);
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 24, .y = 12, .underlying_tile = TILE_MOUNTAIN_FORTRESS_FLOOR, .item = item_make_goblin_king_shield()};
    game.map.tiles[12][24] = TILE_ITEM;
    ok = make_legacy_save() && load_game(&loaded, 99179);
    ASSERT("known legacy dropped shield becomes persistently recoverable", ok && loaded.goblin_king_reward_state == 1 && loaded.floor_items[0].active);
    remove("saves/savegame_99179.json");

    int valid = 1;
    for (int seed = 0; seed < 100; seed++) {
        setup();
        srand((unsigned)seed);
        map_generate_mountains(&game.map, MOUNTAIN_BOSS_LEVEL);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        enemies_spawn(&game);
        valid &= goblin_king_remaining_kegs(&game) == 3;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (game.map.tiles[y][x] == TILE_GOBLIN_KEG) {
                    valid &= map_is_walkable(&game.map, x, y) && abs(x - game.enemies[0].x) <= 1 && abs(y - game.enemies[0].y) <= 1;
                    for (int i = 0; i < game.enemy_count; i++) {
                        valid &= !game.enemies[i].active || game.enemies[i].x != x || game.enemies[i].y != y;
                    }
                }
            }
        }
    }
    ASSERT("100 summit arenas retain three walkable kegs in range of King without actor overlap", valid);
    kill_with(0);
    game.floor_item_count = 0;
    map_generate_mountains(&game.map, MOUNTAIN_BOSS_LEVEL);
    enemies_spawn(&game);
    game_refresh_quest_encounters(&game);
    int reward = 0;
    int living = 0;
    for (int i = 0; i < game.enemy_count; i++) {
        living |= game.enemies[i].active && game.enemies[i].type == ENEMY_MOUNTAIN_GOBLIN_KING;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            reward |= game.map.tiles[y][x] == TILE_GOBLIN_KING_RELIQUARY;
        }
    }
    ASSERT("regenerated summit restores an unclaimed shield without respawning King", !living && reward && game.goblin_king_reward_state == 1);
}

void test_goblin_king(void) {
    printf("Mountain Goblin King phased encounter tests:\n");
    test_patterns();
    test_kegs();
    test_rewards();
    test_saves_and_layouts();
}
