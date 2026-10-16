#include "test_utils.h"
#include "game/castle.h"
#include "game/lord_veyr.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "systems/highscore.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define VEYR_SLOT 99205
#define VEYR_SCORE_PATH "saves/highscores_veyr_test.json"
static GameState game;
static GameState loaded;

static Enemy *find_boss(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        if (enemies[i].type == ENEMY_LORD_VEYR) {
            return &enemies[i];
        }
    }
    return NULL;
}

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    castle_enter(&game, 6);
    Enemy *e = find_boss(game.enemies, game.enemy_count);
    for (int i = 0; i < game.enemy_count; i++) {
        game.enemies[i].active = &game.enemies[i] == e;
    }
    e->revived = 1;
    game.player.x = 45;
    game.player.y = 53;
    game.player.hp = 1000;
    game.player.max_hp = 1000;
    game.player.defense = 0;
    game.player.experience_next = 100000;
    return e;
}

static void prepare(Enemy *e, int attack) {
    lord_veyr_cancel(&game, e);
    e->attack_phase = attack <= VEYR_ADVANCE ? 1 : attack <= VEYR_EAST_LANE ? 2 : 3;
    e->hp = e->attack_phase == 1 ? e->max_hp : e->attack_phase == 2 ? e->max_hp / 2 : e->max_hp / 4;
    e->boss_wave_used = attack == VEYR_ADVANCE || attack == VEYR_WEST_LANE || attack == VEYR_FIRE ? 1 : attack == VEYR_EAST_LANE ? 2 : 0;
    if (attack == VEYR_SWING) {
        game.player.x = e->x;
        game.player.y = e->y + 1;
    }
    lord_veyr_tick(&game, e);
}

static void test_veyr_phases(void) {
    Enemy *e = setup();
    ASSERT("Veyr retains original health, attack and defense", e->max_hp == 950 && e->attack == 45 && e->defense == 14);
    game.player.y = 50;
    ASSERT("Armored Ruler warns a fixed close swing", castle_tick(&game) == 0 && e->attack_phase == 1 && e->boss_attack == VEYR_SWING && e->move_timer == 1 && lord_veyr_marks(e, 45, 50));
    ASSERT("front armor combines with original ward protection", castle_enemy_damage(&game, e, 100) == 30);
    e->hp = e->max_hp / 2;
    e->frozen_turns = 1;
    ASSERT("freeze and threshold crossing preserve the pending armored attack", castle_tick(&game) == 0 && e->attack_phase == 1 && e->move_timer == 1 && castle_enemy_damage(&game, e, 100) == 30);
    game.player.x++;
    ASSERT("sidestepping the fixed swing avoids a hit", castle_tick(&game) == 0 && e->move_timer == 2);
    ASSERT("normal recovery permits a full action without retaliation", castle_tick(&game) == 0 && e->move_timer == 0);
    castle_tick(&game);
    ASSERT("Broken Throne begins only after the queued attack and recovery", e->attack_phase == 2 && e->boss_attack == VEYR_BLAST && e->move_timer == 3);
    ASSERT("phase two drops frontal armor while retaining ward protection", castle_enemy_damage(&game, e, 100) == 60);
    int mask = e->boss_aux_x;
    e->hp = e->max_hp / 4;
    e->frozen_turns = 1;
    castle_tick(&game);
    ASSERT("lower threshold cannot enlarge a frozen phase-two blast", e->attack_phase == 2 && e->boss_aux_x == mask && !lord_veyr_marks(e, e->attack_target_x, e->attack_target_y + 2));
    game.player.x += 2;
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("Unbound Crown warns its larger diamond for three actions", e->attack_phase == 3 && e->boss_attack == VEYR_CROWN && e->move_timer == 4 && lord_veyr_marks(e, e->attack_target_x, e->attack_target_y + 2));
    e->hp = e->max_hp;
    game.player.x -= 3;
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("healing cannot reverse the third phase", e->attack_phase == 3);
}

static void test_veyr_advance(void) {
    Enemy *e = setup();
    prepare(e, VEYR_ADVANCE);
    ASSERT("Throne Advance locks a three-stone cardinal approach", e->boss_attack == VEYR_ADVANCE && e->move_timer == 1 && lord_veyr_marks(e, 45, 50) && lord_veyr_marks(e, 45, 52) && !lord_veyr_marks(e, 45, 53));
    game.player.x++;
    ASSERT("sidestepping advances Veyr only along the marked path and opens recovery", castle_tick(&game) == 0 && e->x == 45 && e->y == 52 && e->move_timer == 2);
    ASSERT("advance recovery lowers frontal armor", castle_enemy_damage(&game, e, 100) == 60);
    e = setup();
    game.player.y = 51;
    prepare(e, VEYR_ADVANCE);
    ASSERT("standing on the advance path takes one hit without actor overlap", castle_tick(&game) == e->attack + 8 && e->y == 50 && game.player.y == 51);
    e = setup();
    e->x = 35;
    e->y = 51;
    game.player.x = 34;
    game.player.y = 51;
    prepare(e, VEYR_ADVANCE);
    game.player.y = 52;
    castle_tick(&game);
    ASSERT("advance never carries Veyr out of the arena into an approach corridor", e->x >= 34 && e->x < 56 && e->y >= 44 && e->y < 58);
}

static void test_veyr_wards_and_fire(void) {
    Enemy *e = setup();
    prepare(e, VEYR_WEST_LANE);
    ASSERT("phase-two west lane is a fixed horizontal row", lord_veyr_marks(e, 34, 53) && lord_veyr_marks(e, 55, 53) && !lord_veyr_marks(e, 45, 54));
    game.player.y++;
    castle_tick(&game);
    ASSERT("west lane waits its full two-action warning", e->move_timer == 1 && e->attack_target_y == 53);
    ASSERT("west lane resolution leaves normal recovery", castle_tick(&game) == 0 && e->move_timer == 2);
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("east lane receives a separate two-action warning", e->boss_attack == VEYR_EAST_LANE && e->move_timer == 3 && lord_veyr_marks(e, 45, 57));
    game.player.x++;
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("ward-linked lanes cycle back to a separately warned Royal Edict", e->boss_attack == VEYR_BLAST && e->move_timer == 3);
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        e = setup();
        game.player.player_class = cls;
        game.player.x = 37;
        game.player.y = 55;
        prepare(e, VEYR_WEST_LANE);
        castle_interact(&game);
        ASSERT("every class can remove a prepared linked ward lane and its gate changes", !e->boss_aux_x && !e->revive_timer && game.map.tiles[54][37] == TILE_CASTLE_WARD_SPENT);
        game.player.x = 53;
        castle_interact(&game);
        ASSERT("both disabled wards remove protection without creating damage immunity", castle_enemy_damage(&game, e, 100) == 100);
        prepare(e, VEYR_CROWN);
        ASSERT("crown warning never overlaps the throne fire hazard", game.castle_fire_phase[5] == 0 && !castle_fire_warning(&game, 45, 51));
        game.player.x -= 3;
        castle_tick(&game);
        castle_tick(&game);
        castle_tick(&game);
        ASSERT("completing a crown with both wards disabled opens two full recovery actions", e->move_timer == 2 && e->revive_timer == 1);
        ASSERT("the two crown counterattack actions are safe", castle_tick(&game) == 0 && e->move_timer == 2 && castle_tick(&game) == 0 && e->move_timer == 0);
    }
    e = setup();
    prepare(e, VEYR_CROWN);
    game.player.x -= 3;
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("a resolved crown alternates with separately warned throne fire", e->boss_attack == VEYR_FIRE && e->move_timer == 3 && game.castle_fire_phase[5] == 1 && castle_fire_warning(&game, 45, 51));
    e->frozen_turns = 2;
    int plan = e->revive_timer;
    castle_tick(&game);
    ASSERT("freeze pauses both fire warning and planned terrain changes", e->move_timer == 3 && game.castle_fire_phase[5] == 1 && e->revive_timer == plan);
    castle_tick(&game);
    game.player.x = 45;
    game.player.y = 51;
    ASSERT("throne fire has a second harmless warning", castle_tick(&game) == 0 && game.castle_fire_phase[5] == 2);
    ASSERT("throne fire hits once with its existing damage instead of adding a crown hit", castle_tick(&game) == 24 && e->move_timer == 2 && game.castle_fire_phase[5] == 3);
    ASSERT("fire impact clears on the next action without repeat damage", castle_tick(&game) == 0 && game.castle_fire_phase[5] == 0);
    castle_tick(&game);
    ASSERT("fire cycles back to a new three-action crown", e->boss_attack == VEYR_CROWN && e->move_timer == 4);
}

static void test_veyr_escorts(void) {
    Enemy *e = setup();
    e->revived = 0;
    game.player.y = 50;
    int count = game.enemy_count;
    castle_tick(&game);
    ASSERT("first engagement marks and announces two finite escorts with at least two actions", e->revived && (e->boss_wave_used & VEYR_ESCORT_PENDING) && e->move_timer == 3 && lord_veyr_escort_marks(e, 42, 46) && lord_veyr_escort_marks(e, 48, 46) && game.enemy_count == count);
    game.player.x++;
    castle_tick(&game);
    ASSERT("escort arrivals do not happen on the first warning action", game.enemy_count == count);
    castle_tick(&game);
    ASSERT("exactly one Warden and Marksman arrive after their warning", game.enemy_count == count + 2 && game.enemies[count].type == ENEMY_IRON_WARDEN && game.enemies[count + 1].type == ENEMY_ROYAL_MARKSMAN);
    ASSERT("new escorts wait until the next turn and yield no loot, XP or score", game.enemies[count].move_timer == 0 && game.enemies[count].experience == 0 && game.enemies[count].revived && game.enemies[count + 1].move_timer == 0);
    int gold = game.gold;
    castle_record_death(&game, &game.enemies[count]);
    ASSERT("escort deaths cannot produce gold", game.gold == gold);
    game.enemies[count].active = 0;
    game.enemies[count + 1].active = 0;
    prepare(e, VEYR_ADVANCE);
    ASSERT("the same attempt never declares another escort call", !(e->boss_wave_used & VEYR_ESCORT_PENDING) && game.enemy_count == count + 2);
    e = setup();
    e->revived = 0;
    castle_tick(&game);
    game.player.x = 42;
    game.player.y = 46;
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("occupied escort stones skip arrivals without moving or crushing their occupants", game.enemy_count == count + 1 && game.player.x == 42 && game.player.y == 46);
    e = setup();
    e->revived = 0;
    game.enemy_count = NON_ROAD_ENEMY_LIMIT;
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("a full enemy roster consumes the finite call without overflow or retry", game.enemy_count == NON_ROAD_ENEMY_LIMIT && e->revived && !(e->boss_wave_used & VEYR_ESCORT_PENDING));
}

static int can_walk_out(const Enemy *e, int x, int y, int turns) {
    if (!lord_veyr_marks(e, x, y)) {
        return 1;
    }
    if (!turns) {
        return 0;
    }
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    for (int d = 0; d < 4; d++) {
        int nx = x + dx[d];
        int ny = y + dy[d];
        if (map_is_walkable(&game.map, nx, ny) && (nx != e->x || ny != e->y) && lord_veyr_barrier_change(e, nx, ny) >= 0 && can_walk_out(e, nx, ny, turns - 1)) {
            return 1;
        }
    }
    return 0;
}

static void test_veyr_barriers(void) {
    Enemy *e = setup();
    prepare(e, VEYR_BLAST);
    ASSERT("Veyr locks explicit closing and opening plans before damage", lord_veyr_barrier_change(e, 40, 47) == -1 && lord_veyr_barrier_change(e, 50, 47) == 1);
    Enemy *guard = &game.enemies[0];
    guard->active = 1;
    guard->x = 40;
    guard->y = 47;
    guard->frozen_turns = 3;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 40, .y = 48, .underlying_tile = TILE_CASTLE_GATE_OPEN, .item = item_make_magic_shield()};
    game.map.tiles[48][40] = TILE_ITEM;
    game.player.x = 44;
    game.player.y = 54;
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("Veyr's barrier changes preserve occupied gates and loot underlays", game.map.tiles[47][40] == TILE_CASTLE_GATE_OPEN && game.map.tiles[48][40] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_CASTLE_GATE_OPEN);
    ASSERT("only empty announced gate cells close or open", game.map.tiles[49][40] == TILE_CASTLE_GATE && game.map.tiles[47][50] == TILE_CASTLE_GATE_OPEN && game.map.tiles[46][40] == TILE_CASTLE_FLOOR);
}

static void test_veyr_escapes_and_range(void) {
    Enemy *e = setup();
    int safe = 1;
    int cases = 0;
    for (int attack = VEYR_SWING; attack <= VEYR_FIRE; attack++) {
        for (int y = 44; y < 58; y++) {
            for (int x = 34; x < 56; x++) {
                if (!map_is_walkable(&game.map, x, y) || (x == e->x && y == e->y)) {
                    continue;
                }
                game.player.x = x;
                game.player.y = y;
                prepare(e, attack);
                safe &= can_walk_out(e, game.player.x, game.player.y, e->move_timer >= 3 ? e->move_timer - 1 : 1);
                cases++;
            }
        }
    }
    ASSERT("every signature leaves an ordinary walking escape throughout the throne room, including planned closures", safe && cases > 1500);
    Item bows[4] = {item_make_bow(), item_make_longbow(), item_make_magic_longbow(), item_make_krakenbone_bow()};
    for (int i = 0; i < 4; i++) {
        e = setup();
        e->x = 39;
        e->y = 51;
        game.player.x = e->x - bows[i].range;
        game.player.y = e->y;
        game.player.last_dx = 1;
        game.player.last_dy = 0;
        game.player.player_class = CLASS_ROGUE;
        game.inventory[0] = bows[i];
        game.inventory_count = 1;
        game.equipped_main_hand = 0;
        game.player.arrows = MAX_ARROWS;
        game.player.attack = 50;
        action_resolve_player(&game, (Action){ACTION_RANGED_ATTACK, 0, 0});
        ASSERT("all bows can damage Veyr along a clear approach", e->hp < e->max_hp);
        castle_tick(&game);
        ASSERT("Veyr answers corridor fire with a marked warning while staying in his arena", e->move_timer == 3 && lord_veyr_marks(e, game.player.x, game.player.y) && e->x == 39 && e->y == 51);
    }
}

static void test_veyr_victory(void) {
    for (int method = 0; method < 5; method++) {
        Enemy *e = setup();
        game.inventory_count = MAX_INVENTORY;
        game.floor_item_count = MAX_FLOOR_ITEMS;
        e->hp = 1;
        game.player.y = e->y + 1;
        game.player.last_dx = 0;
        game.player.last_dy = -1;
        game.player.attack = 1000;
        int score = game.score;
        if (method == 0) {
            action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
        } else if (method == 1) {
            game.player.player_class = CLASS_ROGUE;
            game.player.y++;
            game.inventory[0] = item_make_bow();
            game.equipped_main_hand = 0;
            game.player.arrows = MAX_ARROWS;
            action_resolve_player(&game, (Action){ACTION_RANGED_ATTACK, 0, 0});
        } else {
            game.player.player_class = CLASS_MAGE;
            game.player.known_spells[0] = method == 2 ? spell_make_magic_arrow() : method == 3 ? spell_make_fireball() : spell_make_frost_bolt();
            game.player.known_spell_count = 1;
            game.player.equipped_spell = 0;
            game.player.mp = 100;
            action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
        }
        ASSERT("every damage kill path awards the trophy before victory without needing any inventory or loot space", !e->active && game.game_won && game.crown_of_unbound && game.score == score + 10000 && (game.defeated_bosses & (1 << LOCATION_CASTLE_INTERIOR)) && game.inventory_count == MAX_INVENTORY);
        castle_record_death(&game, e);
        ASSERT("repeat death processing cannot duplicate the victory bonus", game.score == score + 10000);
        int hp = game.player.hp;
        action_resolve_enemies(&game);
        ASSERT("victory stops combat and floor hazards immediately", game.player.hp == hp && game.castle_fire_phase[5] == 0);
    }
    BossJournalEntry entry;
    quest_journal_get_boss(&game, 16, &entry);
    ASSERT("boss journal records the awarded Crown of the Unbound", entry.defeated && entry.trophy && strcmp(entry.trophy, "Crown of the Unbound") == 0);
    ASSERT("completed campaign and permanent trophy survive save/load", save_game(&game, VEYR_SLOT) && load_game(&loaded, VEYR_SLOT) && loaded.game_won && loaded.crown_of_unbound);
}

static cJSON *read_fixture(void) {
    FILE *f = fopen("saves/savegame_99205.json", "rb");
    if (!f) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *buf = malloc((size_t)size + 1);
    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);
    cJSON *root = cJSON_Parse(buf);
    free(buf);
    return root;
}

static int write_fixture(cJSON *root) {
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    FILE *f = fopen("saves/savegame_99205.json", "wb");
    if (!f) {
        free(json);
        return 0;
    }
    fputs(json, f);
    fclose(f);
    free(json);
    return 1;
}

static int legacy_fixture(void) {
    if (!save_game(&game, VEYR_SLOT)) {
        return 0;
    }
    cJSON *root = read_fixture();
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 138);
    cJSON_DeleteItemFromObject(cJSON_GetObjectItem(root, "castle"), "crown_of_unbound");
    return write_fixture(root);
}

static void test_veyr_saves(void) {
    for (int attack = VEYR_SWING; attack <= VEYR_FIRE; attack++) {
        Enemy *e = setup();
        prepare(e, attack);
        e->frozen_turns = 2;
        ASSERT("every signature retains phase, locked geometry, countdown, barrier plan and freeze through save/load", save_game(&game, VEYR_SLOT) && load_game(&loaded, VEYR_SLOT) &&
            memcmp(e, find_boss(loaded.enemies, loaded.enemy_count), sizeof(*e)) == 0 && loaded.castle_fire_phase[5] == game.castle_fire_phase[5]);
    }
    Enemy *e = setup();
    e->revived = 0;
    castle_tick(&game);
    ASSERT("a queued finite escort call survives save/load without early arrivals", save_game(&game, VEYR_SLOT) && load_game(&loaded, VEYR_SLOT) &&
        (find_boss(loaded.enemies, loaded.enemy_count)->boss_wave_used & VEYR_ESCORT_PENDING) && loaded.enemy_count == game.enemy_count);
    e = setup();
    prepare(e, VEYR_CROWN);
    game.map.tiles[54][37] = TILE_CASTLE_WARD_SPENT;
    game.map.tiles[54][53] = TILE_CASTLE_WARD_SPENT;
    game.player.x += 3;
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    castle_enter(&game, 5);
    ASSERT("cached throne room saves spent wards and long crown recovery", save_game(&game, VEYR_SLOT) && load_game(&loaded, VEYR_SLOT) && loaded.castle_cache[5].map.tiles[54][37] == TILE_CASTLE_WARD_SPENT &&
        find_boss(loaded.castle_cache[5].enemies, loaded.castle_cache[5].enemy_count)->revive_timer == 1);
    castle_enter(&loaded, 6);
    ASSERT("floor revisit resumes the same boss health and recovery", find_boss(loaded.enemies, loaded.enemy_count)->hp == e->hp && find_boss(loaded.enemies, loaded.enemy_count)->move_timer == 2);
    castle_leave(&loaded, 0);
    castle_enter(&loaded, 6);
    Enemy *fresh = find_boss(loaded.enemies, loaded.enemy_count);
    ASSERT("castle retreat starts a new attempt with restored wards, health and escort allowance", fresh->hp == fresh->max_hp && !fresh->attack_phase && !fresh->revived && !fresh->move_timer && loaded.map.tiles[54][37] == TILE_CASTLE_WARD && !loaded.castle_fire_phase[5]);
    e = setup();
    e->hp = 222;
    e->attack_phase = 2;
    e->attack_target_x = 45;
    e->attack_target_y = 53;
    e->move_timer = 3;
    e->frozen_turns = 1;
    game.castle_fire_phase[5] = 2;
    game.gold = 321;
    ASSERT("legacy live migration preserves pending blast, timing, damage, freeze and progress", legacy_fixture() && load_game(&loaded, VEYR_SLOT) && find_boss(loaded.enemies, loaded.enemy_count)->hp == 222 &&
        find_boss(loaded.enemies, loaded.enemy_count)->move_timer == 3 && find_boss(loaded.enemies, loaded.enemy_count)->frozen_turns == 1 && loaded.gold == 321 && !loaded.crown_of_unbound);
    Enemy *old = find_boss(loaded.enemies, loaded.enemy_count);
    ASSERT("legacy overlapping fire is deferred without widening the original blast", (old->boss_wave_used & VEYR_DEFERRED_FIRE) && !loaded.castle_fire_phase[5] && lord_veyr_marks(old, 45, 54) && !lord_veyr_marks(old, 45, 55));
    castle_enter(&game, 5);
    ASSERT("legacy cached throne migration preserves health and queued geometry", legacy_fixture() && load_game(&loaded, VEYR_SLOT) && find_boss(loaded.castle_cache[5].enemies, loaded.castle_cache[5].enemy_count)->hp == 222);
    game.game_won = 1;
    game.defeated_bosses |= 1 << LOCATION_CASTLE_INTERIOR;
    find_boss(game.castle_cache[5].enemies, game.castle_cache[5].enemy_count)->active = 0;
    int score = game.score;
    ASSERT("historical victory receives the trophy once without respawning or repeating score", legacy_fixture() && load_game(&loaded, VEYR_SLOT) && loaded.game_won && loaded.crown_of_unbound && loaded.score == score);
    ASSERT("migrated victory remains claimed after rewriting the save", save_game(&loaded, VEYR_SLOT) && load_game(&game, VEYR_SLOT) && game.crown_of_unbound && game.score == score);
    cJSON *root = read_fixture();
    cJSON_SetNumberValue(cJSON_GetObjectItem(cJSON_GetObjectItem(root, "castle"), "crown_of_unbound"), 2);
    ASSERT("invalid trophy state is rejected", write_fixture(root) && !load_game(&loaded, VEYR_SLOT));
    remove("saves/savegame_99205.json");
}

static void test_veyr_highscores(void) {
    HighScoreTable table = {0};
    HighScoreTable restored = {0};
    highscore_insert(&table, "Old hero", 20000, 6, 0);
    highscore_insert(&table, "Crown hero", 30000, 6, 1);
    highscore_save_to(&table, VEYR_SCORE_PATH);
    highscore_load_from(&restored, VEYR_SCORE_PATH);
    ASSERT("Hall of Fame preserves per-run trophies and permanent trophy history", restored.count == 2 && restored.crown_of_unbound && restored.entries[0].crown_of_unbound && !restored.entries[1].crown_of_unbound && strcmp(restored.entries[0].name, "Crown hero") == 0);
    table = (HighScoreTable){0};
    for (int i = 0; i < MAX_HIGH_SCORES; i++) {
        highscore_insert(&table, "Prior run", 50000 + i, 6, 0);
    }
    highscore_insert(&table, "Unranked crown", 10000, 6, 1);
    highscore_save_to(&table, VEYR_SCORE_PATH);
    highscore_load_from(&restored, VEYR_SCORE_PATH);
    ASSERT("an unranked victory retains the trophy without corrupting the top ten", restored.count == MAX_HIGH_SCORES && restored.crown_of_unbound && !restored.entries[0].crown_of_unbound && restored.entries[9].score == 50000);
    FILE *f = fopen(VEYR_SCORE_PATH, "wb");
    fputs("{\"entries\":[{\"name\":\"Legacy hero\",\"score\":12345,\"level\":6}]}", f);
    fclose(f);
    highscore_load_from(&restored, VEYR_SCORE_PATH);
    ASSERT("legacy high scores preserve names and scores without inventing victories", restored.count == 1 && restored.entries[0].score == 12345 && !restored.crown_of_unbound && !restored.entries[0].crown_of_unbound);
    remove(VEYR_SCORE_PATH);
}

void test_lord_veyr(void) {
    printf("Lord Veyr encounter\n");
    test_veyr_phases();
    test_veyr_advance();
    test_veyr_wards_and_fire();
    test_veyr_escorts();
    test_veyr_barriers();
    test_veyr_escapes_and_range();
    test_veyr_victory();
    test_veyr_saves();
    test_veyr_highscores();
}
