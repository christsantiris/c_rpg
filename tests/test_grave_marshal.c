#include "test_utils.h"
#include "approach_checks.h"
#include "../src/game/grave_marshal.h"
#include "../src/game/catacombs.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = LOCATION_CATACOMBS;
    game.level = CATACOMBS_DEPTH;
    game.max_catacombs_level_reached = CATACOMBS_DEPTH;
    srand(19);
    map_generate_catacombs(&game.map, CATACOMBS_DEPTH);
    int x;
    int y;
    map_room_center(&game.map.rooms[7], &x, &y);
    game.player.x = x - 1;
    game.player.y = y;
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
    game.enemies[0] = (Enemy){.type = ENEMY_GRAVE_MARSHAL, .name = "Grave Marshal", .x = x, .y = y,
        .active = 1, .is_boss = 1, .hp = 280, .max_hp = 280, .attack = 26, .defense = 8,
        .experience = 750, .attack_target_x = -1, .attack_target_y = -1, .boss_aux_x = -1, .boss_aux_y = -1};
    grave_marshal_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    return &game.enemies[0];
}

static int find_tile(TileType tile, int *x, int *y) {
    const Room *room = &game.map.rooms[7];
    for (int row = room->y; row < room->y + room->h; row++) {
        for (int col = room->x; col < room->x + room->w; col++) {
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
        int mantles = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            mantles += game.floor_items[i].active && strcmp(game.floor_items[i].item.name, "Gravekeeper's Mantle") == 0;
        }
        victories &= !game.enemies[0].active && mantles == 1 && game.grave_marshal_reward_state == 1 && (game.defeated_bosses & (1 << LOCATION_CATACOMBS));
    }
    ASSERT("melee, bow, Magic Arrow, Fireball and Frost Bolt each guarantee one Gravekeeper's Mantle", victories);
    Enemy *e = setup();
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 100;
    e->hp = 1;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = game.map.rooms[0].x + 2, .y = game.map.rooms[0].y + 2, .underlying_tile = TILE_CATACOMBS_FLOOR, .item = item_make_health_potion()};
    }
    game.floor_item_count = MAX_FLOOR_ITEMS;
    game.map.tiles[game.map.rooms[0].y + 2][game.map.rooms[0].x + 2] = TILE_ITEM;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        game.inventory[i] = item_make_health_potion();
    }
    game.inventory_count = MAX_INVENTORY;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    int x = 0;
    int y = 0;
    ASSERT("full floor capacity preserves every item and reserves reward in a reliquary", game.floor_item_count == MAX_FLOOR_ITEMS && game.floor_items[MAX_FLOOR_ITEMS - 1].item.type == ITEM_POTION_HEALTH && game.grave_marshal_reward_state == 1 && find_tile(TILE_MARSHAL_RELIQUARY, &x, &y));
    int saved = save_game(&game, 99187) && load_game(&loaded, 99187);
    ASSERT("a full floor-loot save retains the reserved mantle, both claim flags and every existing item", saved && loaded.grave_marshal_reward_state == 1 && loaded.catacombs_mantle_unclaimed && loaded.map.tiles[y][x] == TILE_MARSHAL_RELIQUARY && loaded.floor_item_count == MAX_FLOOR_ITEMS);
    game.player.x = x;
    game.player.y = y;
    game.map.tiles[y][x + 1] = TILE_OSSUARY_BRAZIER;
    game.map.tiles[y - 1][x] = TILE_OSSUARY_BRAZIER;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("full inventory cannot consume the reserved reward", game.grave_marshal_reward_state == 1 && game.map.tiles[y][x] == TILE_MARSHAL_RELIQUARY);
    game.inventory_count--;
    int reward_score = game.score;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("reliquary pickup grants original Gravekeeper's Mantle and marks it claimed", game.grave_marshal_reward_state == 2 && strcmp(game.inventory[MAX_INVENTORY - 1].name, "Gravekeeper's Mantle") == 0 && game.inventory[MAX_INVENTORY - 1].defense_bonus == item_make_gravekeeper_mantle().defense_bonus && game.score == reward_score + game.inventory[MAX_INVENTORY - 1].value);
    grave_marshal_restore_reward(&game);
    ASSERT("claiming cannot recreate a second reward or operate adjacent braziers", !find_tile(TILE_MARSHAL_RELIQUARY, &x, &y) && game.map.tiles[game.player.y - 1][game.player.x] == TILE_OSSUARY_BRAZIER);
    kill_with(0);
    game.player.x = game.enemies[0].x;
    game.player.y = game.enemies[0].y;
    for (int i = 0; i < 3; i++) {
        action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    }
    ASSERT("normal ground pickup also records a claimed reward", game.grave_marshal_reward_state == 2);
}

static int make_legacy_save(void) {
    if (!save_game(&game, 99187)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99187.json", "r");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 129);
    cJSON_DeleteItemFromObject(root, "grave_marshal_reward_state");
    cJSON *arrays[2] = {cJSON_GetObjectItem(root, "enemies"),
        cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "catacombs_cache"), CATACOMBS_DEPTH - 1), "enemies")};
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
    file = fopen("saves/savegame_99187.json", "w");
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
    ASSERT("Honored Commander warns one-action polearm front without damage", e->boss_attack == MARSHAL_SWEEP && e->move_timer == 1 && catacombs_sweep_marks(&game.map, e, game.player.x, game.player.y) && game.player.hp == 1000);
    int tx = e->attack_target_x;
    int ty = e->attack_target_y;
    e->hp = 140;
    e->frozen_turns = 2;
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x - 1, game.player.y});
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze and threshold crossing preserve original sweep and phase", e->move_timer == 1 && e->attack_phase == 0 && e->attack_target_x == tx && e->attack_target_y == ty);
    action_resolve_enemies(&game);
    ASSERT("one ordinary backstep escapes with a full recovery action", game.player.hp == 1000 && e->move_timer == -1);
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 20;
    int hp = e->hp;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    action_resolve_enemies(&game);
    ASSERT("adjacent counterattack during recovery has no extra retaliation", e->hp < hp && game.player.hp == 1000 && e->move_timer == 0);
    e->boss_wave_used = 1;
    action_resolve_enemies(&game);
    ASSERT("next preparation latches Funeral March and warns a fixed two-action cross", e->attack_phase == 1 && e->boss_attack == MARSHAL_CROSS && e->move_timer == 2 && e->attack_target_x == game.player.x);
    tx = e->attack_target_x;
    ty = e->attack_target_y;
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x, game.player.y + 1});
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x - 1, game.player.y});
    action_resolve_enemies(&game);
    ASSERT("two actual cardinal moves leave the locked cross without damage", game.player.hp == 1000 && e->move_timer == -1 && game.player.x != tx && game.player.y != ty);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("Funeral March alternates cross with a separately warned funeral channel", e->boss_attack == MARSHAL_CHANNEL && e->move_timer == 2);
    e = setup();
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("standing in the short sweep takes one base-strength hit", game.player.hp == 974 && e->move_timer == -1);
    e = setup();
    e->attack_phase = 1;
    e->boss_wave_used = 1;
    e->hp = 140;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("standing in a cross takes exactly one signature hit and no extra melee", game.player.hp == 974 && e->move_timer == -1);
    e = setup();
    game.player.x = e->x - 4;
    int x = e->x;
    action_resolve_enemies(&game);
    ASSERT("phase one retains an ordinary approach step inside the royal tomb", e->x == x - 1 && e->move_timer == 0 && game.player.hp == 1000);
    e = setup();
    e->hp = 140;
    e->attack_phase = 1;
    e->boss_wave_used = 1;
    game.map.tiles[e->y][e->x - 1] = TILE_CATACOMBS_WALL;
    game.player.x = e->x - 2;
    action_resolve_enemies(&game);
    ASSERT("solid cover prevents preparing an attack behind it", e->attack_target_x == -1);
    game.map.tiles[e->y][e->x - 1] = TILE_CATACOMBS_FLOOR;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("removing cover cannot invent a target during a saved warning", game.player.hp == 1000 && e->move_timer == -1);
    e = setup();
    e->hp = 140;
    e->attack_phase = 1;
    e->boss_wave_used = 1;
    game.player.x -= 2;
    action_resolve_enemies(&game);
    tx = e->attack_target_x;
    ty = e->attack_target_y;
    game.map.tiles[ty][tx - 1] = TILE_CATACOMBS_SARCOPHAGUS;
    ASSERT("tomb cover clips cross arms before stones behind it", !grave_marshal_attack_marks(&game.map, e, tx - 2, ty));
    game.player.x = e->x - 13;
    action_resolve_enemies(&game);
    ASSERT("retreat beyond range pauses exact countdown without resetting health", e->move_timer == 2 && e->hp == 140 && e->attack_target_x == tx);
}

static void test_guards(void) {
    Enemy *e = setup();
    e->hp = 140;
    action_resolve_enemies(&game);
    int ax = e->attack_target_x;
    int ay = e->attack_target_y;
    int bx = e->boss_aux_x;
    int by = e->boss_aux_y;
    ASSERT("one finite honor wave warns two distant fixed tomb niches for two actions", e->boss_attack == MARSHAL_GUARDS && e->move_timer == 2 && e->boss_wave_used == 1 && ax >= 0 && bx >= 0 && abs(ax - game.player.x) + abs(ay - game.player.y) >= 4 && !catacombs_sweep_marks(&game.map, e, ax, ay));
    e->frozen_turns = 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freeze pauses the honor call and cannot spawn guards early", e->move_timer == 2 && game.enemy_count == 1 && e->boss_aux_x == bx);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("two guards spawn at the warned niches and do not move or attack immediately", game.enemy_count == 3 && game.enemies[1].x == ax && game.enemies[1].y == ay && game.enemies[2].x == bx && game.enemies[2].y == by && game.player.hp == 1000 && e->move_timer == -1);
    ASSERT("honor guards carry no experience, loot or second resurrection allowance", game.enemies[1].experience == 0 && game.enemies[1].revived && game.enemies[2].experience == 0 && game.enemies[2].revived);
    LevelCache *cache = &game.catacombs_cache[4];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = game.enemy_count;
    memcpy(cache->enemies, game.enemies, sizeof(Enemy) * (size_t)game.enemy_count);
    int saved = save_game(&game, 99187) && load_game(&loaded, 99187);
    ASSERT("saved active and cached honor guards retain no-reward and one-wave state", saved && loaded.enemies[1].revived && loaded.enemies[1].experience == 0 && loaded.catacombs_cache[4].enemies[2].revived && loaded.catacombs_cache[4].enemies[0].boss_wave_used == 1 && strcmp(loaded.enemies[1].name, "Royal Honor Guard") == 0);
    for (int i = 1; i < 3; i++) {
        Enemy *guard = &game.enemies[i];
        game.player.x = guard->x - 1;
        game.player.y = guard->y;
        game.player.attack = 1000;
        action_resolve_player(&game, (Action){ACTION_MOVE, guard->x, guard->y});
    }
    ASSERT("killing the honor guard awards no repeat XP, score, coins or items", !game.enemies[1].active && !game.enemies[2].active && game.score == 0 && game.floor_item_count == 0);
    game.player.x = e->x - 1;
    game.player.y = e->y;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("the next attack is a cross and never repeats the spent honor wave", e->boss_attack == MARSHAL_CROSS && e->boss_wave_used == 1 && game.enemy_count == 3);

    e = setup();
    e->hp = 140;
    action_resolve_enemies(&game);
    ax = e->attack_target_x;
    ay = e->attack_target_y;
    game.player.x = ax;
    game.player.y = ay;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("occupying a warned niche skips that guard without postponing an unannounced spawn", game.enemy_count == 2 && game.enemies[1].x != ax && game.map.tiles[ay][ax] == TILE_MARSHAL_NICHE_SPENT && e->boss_wave_used == 1);
    e = setup();
    e->hp = 140;
    game.enemies[1] = (Enemy){.type = ENEMY_ANCIENT_SKELETON, .active = 0, .hp = 0, .max_hp = 48, .x = 8, .y = 7};
    game.enemy_count = 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("summoning preserves every existing corpse and its resurrection allowance", game.enemy_count == 4 && game.enemies[1].type == ENEMY_ANCIENT_SKELETON && !game.enemies[1].active && !game.enemies[1].revived && game.enemies[1].hp == 0);
    e = setup();
    e->hp = 140;
    for (int i = 1; i < NON_ROAD_ENEMY_LIMIT; i++) {
        game.enemies[i] = (Enemy){.type = ENEMY_SKELETON, .active = 1, .hp = 10, .max_hp = 10, .x = 8, .y = 7, .frozen_turns = 20};
    }
    game.enemy_count = NON_ROAD_ENEMY_LIMIT;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("a full enemy list consumes the finite warning without overflow or replacing old actors", game.enemy_count == NON_ROAD_ENEMY_LIMIT && game.enemies[29].type == ENEMY_SKELETON && e->boss_wave_used == 1 && e->move_timer == -1);
    e = setup();
    e->hp = 140;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    e->hp = 1;
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 1000;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    game.player.x = game.map.stairs_down_x;
    game.player.y = game.map.stairs_down_y;
    action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
    walk_return_approach(&game);
    ASSERT("Marshal victory opens the castle return even with both honor guards still alive", game.location == LOCATION_CASTLE && game.catacombs_cache[4].enemies[1].active && game.catacombs_cache[4].enemies[2].active && game.grave_marshal_reward_state == 1);
}

static void test_braziers(void) {
    Enemy *e = setup();
    const Room *room = &game.map.rooms[7];
    int x = room->x + 1;
    int y = room->y + 1;
    game.player.x = x + 1;
    game.player.y = y;
    e->hp = 140;
    e->attack_phase = 1;
    e->boss_wave_used = 1;
    e->boss_attack = MARSHAL_CROSS;
    ASSERT("two braziers preserve 50 percent protection", catacombs_enemy_damage(&game, e, 100) == 50 && catacombs_enemy_damage(&game, e, 1) == 1);
    action_resolve_enemies(&game);
    ASSERT("a new funeral channel is marked for two actions before the counter", e->boss_attack == MARSHAL_CHANNEL && e->move_timer == 2 && grave_marshal_attack_marks(&game.map, e, game.player.x, game.player.y));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("extinguishing a tomb brazier interrupts only the channel and leaves two full recovery actions", game.map.tiles[y][x] == TILE_OSSUARY_COLD && e->move_timer == -2 && e->attack_target_x == -1 && game.player.hp == 1000 && catacombs_enemy_damage(&game, e, 100) == 75);
    game.player.x = e->x - 1;
    game.player.y = e->y;
    game.player.attack = 20;
    int hp = e->hp;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    action_resolve_enemies(&game);
    ASSERT("both counter actions permit melee without extra Marshal retaliation", e->hp < hp && e->move_timer == 0 && game.player.hp == 1000);
    game.player.x = room->x + room->w - 3;
    game.player.y = y;
    e->boss_attack = MARSHAL_CROSS;
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("second tomb flame stays cold, grants its own interruption and removes protection", catacombs_lit_braziers(&game.map, 7) == 0 && e->move_timer == -2 && catacombs_enemy_damage(&game, e, 100) == 100);
    grave_marshal_prepare_map(&game.map, game.enemies, game.enemy_count, game.player.x, game.player.y);
    ASSERT("arena preparation never relights spent braziers", catacombs_lit_braziers(&game.map, 7) == 0);
    int classes = 1;
    for (int c = 0; c < 3; c++) {
        e = setup();
        room = &game.map.rooms[7];
        game.player.x = room->x + 2;
        game.player.y = room->y + 1;
        game.player.player_class = c == 0 ? CLASS_WARRIOR : c == 1 ? CLASS_ROGUE : CLASS_MAGE;
        e->boss_attack = MARSHAL_CHANNEL;
        e->move_timer = 2;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        classes &= e->move_timer == -3 && catacombs_lit_braziers(&game.map, 7) == 1 && game.inventory_count == 0;
    }
    ASSERT("every class interrupts the channel without gear or consumables", classes);
    e = setup();
    room = &game.map.rooms[7];
    game.player.x = room->x + 2;
    game.player.y = room->y + 1;
    e->boss_attack = MARSHAL_GUARDS;
    e->move_timer = 2;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("ordinary brazier use cannot erase the honor guard warning or replenish it", e->boss_attack == MARSHAL_GUARDS && e->move_timer == 2 && catacombs_lit_braziers(&game.map, 7) == 1);
}
static void test_persistence(void) {
    Enemy *e = setup();
    e->hp = 101;
    e->attack_phase = 1;
    e->boss_wave_used = 1;
    e->boss_attack = MARSHAL_GUARDS;
    e->move_timer = 2;
    e->frozen_turns = 2;
    int x = 0;
    int y = 0;
    find_tile(TILE_MARSHAL_NICHE, &x, &y);
    e->attack_target_x = x;
    e->attack_target_y = y;
    game.map.tiles[y][x] = TILE_MARSHAL_NICHE_SPENT;
    const Room *room = &game.map.rooms[7];
    e->boss_aux_x = room->x + room->w - 3;
    e->boss_aux_y = room->y + room->h - 3;
    game.map.tiles[room->y + 1][room->x + 1] = TILE_OSSUARY_COLD;
    game.catacombs_quest_state = 1;
    game.catacombs_quest_progress = 7;
    game.catacombs_quest_encounters = 7;
    game.grave_marshal_reward_state = 1;
    game.catacombs_mantle_unclaimed = 1;
    LevelCache *cache = &game.catacombs_cache[4];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = 1;
    cache->enemies[0] = *e;
    int ok = save_game(&game, 99187) && load_game(&loaded, 99187);
    ASSERT("save restores active HP, phase, frozen countdown, finite wave and both guard targets", ok && loaded.enemies[0].hp == 101 && loaded.enemies[0].attack_phase == 1 && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].frozen_turns == 2 && loaded.enemies[0].boss_wave_used == 1 && loaded.enemies[0].attack_target_x == x && loaded.enemies[0].boss_aux_y == e->boss_aux_y);
    ASSERT("save preserves cached niches, cold braziers, unclaimed mantle and memorial progress", ok && loaded.catacombs_cache[4].map.tiles[y][x] == TILE_MARSHAL_NICHE_SPENT && loaded.catacombs_cache[4].enemies[0].move_timer == 2 && loaded.map.tiles[room->y + 1][room->x + 1] == TILE_OSSUARY_COLD && loaded.grave_marshal_reward_state == 1 && loaded.catacombs_mantle_unclaimed && loaded.catacombs_quest_progress == 7);
    e->move_timer = -2;
    game.grave_marshal_reward_state = 2;
    game.catacombs_mantle_unclaimed = 0;
    ok = save_game(&game, 99187) && load_game(&loaded, 99187);
    ASSERT("save retains two-action recovery and claimed mantle", ok && loaded.enemies[0].move_timer == -2 && loaded.grave_marshal_reward_state == 2 && !loaded.catacombs_mantle_unclaimed);
    e->move_timer = 2;
    e->boss_attack = MARSHAL_CHANNEL;
    e->attack_target_x = game.player.x;
    e->attack_target_y = game.player.y;
    cache->enemies[0] = *e;
    game_open_town_portal(&game);
    game_use_town_portal(&game);
    ASSERT("portal wards cancel a threatening new channel on landing without changing health, wave or cold flame", game.location == LOCATION_CATACOMBS && game.enemies[0].attack_target_x == -1 && game.enemies[0].move_timer == -1 && game.enemies[0].hp == 101 && game.enemies[0].boss_wave_used == 1 && game.player.hp == 1000);
    e = setup();
    e->hp = 140;
    action_resolve_enemies(&game);
    x = e->attack_target_x;
    y = e->attack_target_y;
    game_open_town_portal(&game);
    game_use_town_portal(&game);
    ASSERT("non-damaging honor guard warnings survive portal travel without rearming a second wave", game.enemies[0].boss_attack == MARSHAL_GUARDS && game.enemies[0].move_timer == 2 && game.enemies[0].attack_target_x == x && game.enemies[0].boss_wave_used == 1);

    e = setup();
    room = &game.map.rooms[7];
    for (int yy = room->y; yy < room->y + room->h; yy++) {
        for (int xx = room->x; xx < room->x + room->w; xx++) {
            if (game.map.tiles[yy][xx] == TILE_MARSHAL_NICHE) {
                game.map.tiles[yy][xx] = TILE_CATACOMBS_FLOOR;
            }
        }
    }
    e->hp = 99;
    e->move_timer = 1;
    e->attack_target_x = e->x - 1;
    e->attack_target_y = e->y;
    game.map.tiles[room->y + 1][room->x + 1] = TILE_OSSUARY_COLD;
    game.catacombs_quest_state = 1;
    game.catacombs_quest_progress = 3;
    game.catacombs_quest_encounters = 7;
    game.gold = 321;
    map_mark_explored(&game.map, e->x, e->y);
    game.portal_active = 1;
    game.portal_location = LOCATION_CATACOMBS;
    game.portal_level = CATACOMBS_DEPTH;
    game.portal_x = room->x + 2;
    game.portal_y = room->y + 2;
    game.portal_origin_tile = TILE_CATACOMBS_FLOOR;
    cache = &game.catacombs_cache[4];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = 1;
    cache->enemies[0] = *e;
    cache->enemies[0].hp = 51;
    ok = make_legacy_save() && load_game(&loaded, 99187);
    ASSERT("version 129 migration preserves active/cached health and safely restarts old unmarked attacks", ok && loaded.enemies[0].hp == 99 && loaded.catacombs_cache[4].enemies[0].hp == 51 && loaded.enemies[0].move_timer == 0 && loaded.enemies[0].attack_target_x == -1);
    ASSERT("migration preserves cold flames, memorials, gold, exploration and hidden portal landings", ok && loaded.map.tiles[room->y + 1][room->x + 1] == TILE_OSSUARY_COLD && loaded.catacombs_quest_progress == 3 && loaded.gold == 321 && map_is_explored(&loaded.map, e->x, e->y) && loaded.map.tiles[game.portal_y][game.portal_x] == TILE_CATACOMBS_FLOOR && loaded.catacombs_cache[4].map.tiles[game.portal_y][game.portal_x] == TILE_CATACOMBS_FLOOR);
    e->active = 0;
    cache->enemies[0].active = 0;
    game.defeated_bosses |= 1 << LOCATION_CATACOMBS;
    game.catacombs_mantle_unclaimed = 0;
    ok = make_legacy_save() && load_game(&loaded, 99187);
    ASSERT("legacy victories remain defeated and cannot fabricate a previously claimed mantle", ok && !loaded.enemies[0].active && loaded.grave_marshal_reward_state == 2 && !loaded.catacombs_mantle_unclaimed);
    game.catacombs_mantle_unclaimed = 1;
    ok = make_legacy_save() && load_game(&loaded, 99187);
    ASSERT("known old unclaimed-mantle flag migrates to recoverable earned state", ok && loaded.grave_marshal_reward_state == 1 && loaded.catacombs_mantle_unclaimed);
    remove("saves/savegame_99187.json");
}

static int escapes(Enemy *e, int x, int y, int turns) {
    if (!grave_marshal_attack_marks(&game.map, e, x, y)) {
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
    static int queue[MAP_H * MAP_W];
    int valid = 1;
    int safe = 1;
    for (int seed = 0; seed < 100; seed++) {
        setup();
        srand((unsigned)seed);
        map_generate_catacombs(&game.map, CATACOMBS_DEPTH);
        game.player.x = game.map.stairs_up_x;
        game.player.y = game.map.stairs_up_y;
        enemies_spawn(&game);
        Enemy *e = &game.enemies[0];
        const Room *room = &game.map.rooms[7];
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
        int niches = 0;
        for (int y = room->y; y < room->y + room->h; y++) {
            for (int x = room->x; x < room->x + room->w; x++) {
                if (game.map.tiles[y][x] == TILE_MARSHAL_NICHE) {
                    niches++;
                    valid &= seen[y][x] && abs(x - e->x) + abs(y - e->y) >= 3;
                }
                if (!map_is_walkable(&game.map, x, y) || (x == e->x && y == e->y)) {
                    continue;
                }
                game.player.x = x;
                game.player.y = y;
                for (int kind = MARSHAL_CROSS; kind <= MARSHAL_CHANNEL; kind++) {
                    e->hp = e->max_hp / 2;
                    e->attack_phase = 1;
                    e->boss_wave_used = 1;
                    e->move_timer = 0;
                    e->boss_attack = kind == MARSHAL_CROSS ? MARSHAL_CHANNEL : MARSHAL_CROSS;
                    grave_marshal_tick(&game, e);
                    safe &= escapes(e, x, y, e->move_timer);
                }
            }
        }
        valid &= niches == 2 && catacombs_lit_braziers(&game.map, 7) == 2 && seen[game.map.stairs_down_y][game.map.stairs_down_x];
    }
    ASSERT("100 tombs retain two reachable niches, two native braziers and an accessible return route", valid);
    ASSERT("all cross/channel preparations from every walkable royal-tomb tile allow normal walking escapes", safe);
}

void test_grave_marshal(void) {
    printf("Grave Marshal phased encounter tests:\n");
    test_patterns();
    test_guards();
    test_braziers();
    test_rewards();
    test_persistence();
    test_layouts();
}
