#include "test_utils.h"
#include "game/castle.h"
#include "game/royal_arcanist.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define ARCANIST_SLOT 99203
static GameState game;
static GameState loaded;

static Enemy *find_boss(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        if (enemies[i].type == ENEMY_ROYAL_ARCANIST) {
            return &enemies[i];
        }
    }
    return NULL;
}

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    castle_enter(&game, 4);
    Enemy *e = find_boss(game.enemies, game.enemy_count);
    for (int i = 0; i < game.enemy_count; i++) {
        game.enemies[i].active = &game.enemies[i] == e;
    }
    game.player.x = 45;
    game.player.y = 53;
    game.player.hp = 1000;
    game.player.max_hp = 1000;
    game.player.defense = 0;
    game.player.experience_next = 100000;
    return e;
}

static void prepare(Enemy *e, int attack) {
    royal_arcanist_cancel(e);
    e->attack_phase = attack == ARCANIST_BLAST ? 1 : 2;
    e->boss_wave_used = attack >= ARCANIST_WEST_LANE;
    if (attack == ARCANIST_EAST_LANE) {
        game.map.tiles[54][37] = TILE_CASTLE_WARD_SPENT;
    }
    royal_arcanist_tick(&game, e);
}

static void test_arcanist_phases(void) {
    Enemy *e = setup();
    ASSERT("Arcanist keeps original health and attack", e->max_hp == 470 && e->attack == 38);
    ASSERT("Warded Scholar declares a harmless two-action blast", castle_tick(&game) == 0 && e->attack_phase == 1 && e->boss_attack == ARCANIST_BLAST && e->move_timer == 3);
    ASSERT("blast footprint is the fixed five-stone diamond", castle_attack_marks(&game.map, e, 45, 53) && castle_attack_marks(&game.map, e, 45, 54) &&
        castle_attack_marks(&game.map, e, 46, 53) && !castle_attack_marks(&game.map, e, 46, 54) && !castle_attack_marks(&game.map, e, 45, 55));
    ASSERT("both active wards retain their damage reduction", castle_enemy_damage(&game, e, 100) == 60);
    int mask = e->boss_aux_x;
    int high = e->boss_aux_y;
    int barriers = e->revive_timer;
    e->hp = e->max_hp / 2;
    e->frozen_turns = 1;
    ASSERT("freeze pauses warning, footprint, health transition and planned barriers", castle_tick(&game) == 0 && e->move_timer == 3 &&
        e->attack_phase == 1 && e->boss_aux_x == mask && e->boss_aux_y == high && e->revive_timer == barriers);
    ASSERT("blast gives its second harmless warning", castle_tick(&game) == 0 && e->move_timer == 1);
    ASSERT("crossing half health cannot widen the pending blast", !castle_attack_marks(&game.map, e, 45, 56) && e->attack_phase == 1);
    ASSERT("warned hit includes the original two ward attack bonuses", castle_tick(&game) == e->attack + 8 && e->move_timer == 2 && e->attack_target_x == -1);
    ASSERT("normal recovery is one full harmless action", castle_tick(&game) == 0 && e->move_timer == 0);
    ASSERT("half health declares Shattered Liturgy and a new cross warning", castle_tick(&game) == 0 && e->attack_phase == 2 && e->boss_attack == ARCANIST_CROSS && e->move_timer == 3);
    ASSERT("cross reaches three stones along each axis and leaves diagonals safe", castle_attack_marks(&game.map, e, 42, 53) && castle_attack_marks(&game.map, e, 45, 56) && !castle_attack_marks(&game.map, e, 44, 54));
    game.player.x = 44;
    castle_tick(&game);
    game.player.y = 54;
    ASSERT("two ordinary cardinal steps escape the cross", castle_tick(&game) == 0 && e->move_timer == 2);
    castle_tick(&game);
    e->hp = e->max_hp;
    castle_tick(&game);
    ASSERT("healing cannot reverse phase two; the next signature is ward-linked", e->attack_phase == 2 && e->boss_attack == ARCANIST_WEST_LANE && e->move_timer == 3);
}

static void test_arcanist_lanes(void) {
    Enemy *e = setup();
    prepare(e, ARCANIST_WEST_LANE);
    ASSERT("west pedestal powers a fixed horizontal arena lane", e->boss_attack == ARCANIST_WEST_LANE && royal_arcanist_marks(e, 34, 53) && royal_arcanist_marks(e, 55, 53) && !royal_arcanist_marks(e, 45, 52));
    game.player.y = 54;
    castle_tick(&game);
    ASSERT("west lane stays on its original row", e->move_timer == 1 && e->attack_target_y == 53 && !royal_arcanist_marks(e, 45, 54));
    ASSERT("west resolution warns the east follow-up instead of hitting immediately", castle_tick(&game) == 0 && e->boss_attack == ARCANIST_EAST_LANE && e->move_timer == 3 && e->attack_target_x == 45);
    ASSERT("east pedestal powers a separately fixed vertical arena lane", royal_arcanist_marks(e, 45, 44) && royal_arcanist_marks(e, 45, 57) && !royal_arcanist_marks(e, 46, 54));
    game.player.x = 46;
    castle_tick(&game);
    ASSERT("east warning remains fixed while player walks away", e->move_timer == 1 && e->attack_target_x == 45);
    ASSERT("finishing the lane sequence exposes normal recovery", castle_tick(&game) == 0 && e->move_timer == 2 && !e->boss_wave_used);
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("cross and lane sequences alternate", e->boss_attack == ARCANIST_CROSS && e->move_timer == 3);

    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        for (int east = 0; east < 2; east++) {
            e = setup();
            game.player.player_class = cls;
            game.player.x = east ? 53 : 37;
            game.player.y = 55;
            prepare(e, east ? ARCANIST_EAST_LANE : ARCANIST_WEST_LANE);
            int gate = game.map.tiles[48][40];
            action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
            action_resolve_enemies(&game);
            ASSERT("every class can disable either lane's source and cancel its barrier changes", game.map.tiles[54][east ? 53 : 37] == TILE_CASTLE_WARD_SPENT &&
                e->attack_target_x == -1 && e->move_timer == 2 && e->revive_timer == 1 && game.map.tiles[48][40] == gate);
            ASSERT("ward counter leaves two full recovery actions after Interact", castle_tick(&game) == 0 && e->move_timer == 2 && castle_tick(&game) == 0 && e->move_timer == 0);
        }
    }
    e = setup();
    prepare(e, ARCANIST_WEST_LANE);
    game.player.x = 53;
    game.player.y = 55;
    castle_interact(&game);
    ASSERT("disabling the other pedestal does not retarget the pending west lane", e->boss_attack == ARCANIST_WEST_LANE && e->move_timer == 3 && e->attack_target_y == 53 && castle_enemy_damage(&game, e, 100) == 80);
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("a spent east pedestal removes its future follow-up", e->move_timer == 2 && e->attack_target_x == -1);
    game.player.x = 37;
    castle_interact(&game);
    ASSERT("both disabled pedestals remove all ward protection", castle_enemy_damage(&game, e, 100) == 100);
    e->attack_phase = 2;
    e->boss_wave_used = 1;
    e->move_timer = 0;
    castle_tick(&game);
    ASSERT("both spent wards leave a vulnerable boss with a normal blast fallback", e->boss_attack == ARCANIST_BLAST && e->move_timer == 3);

    e = setup();
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 37, .y = 54, .underlying_tile = TILE_CASTLE_WARD, .item = item_make_health_potion()};
    game.map.tiles[54][37] = TILE_ITEM;
    game.player.x = 37;
    game.player.y = 55;
    prepare(e, ARCANIST_WEST_LANE);
    ASSERT("loot cannot disable a ward or conceal its linked preparation", e->boss_attack == ARCANIST_WEST_LANE && e->move_timer == 3);
    castle_interact(&game);
    ASSERT("ward interaction updates loot underlays and interrupts its lane", game.map.tiles[54][37] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_CASTLE_WARD_SPENT && e->move_timer == 2);
}

static void test_arcanist_barriers(void) {
    Enemy *e = setup();
    castle_tick(&game);
    ASSERT("barrier warning distinguishes declared closing and opening cells", royal_arcanist_barrier_change(e, 40, 47) == -1 && royal_arcanist_barrier_change(e, 50, 47) == 1);
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
    ASSERT("warned closures never crush actors or hide a closed gate beneath loot", game.map.tiles[47][40] == TILE_CASTLE_GATE_OPEN && game.map.tiles[48][40] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_CASTLE_GATE_OPEN);
    ASSERT("empty warned gates change and unannounced cells remain untouched", game.map.tiles[49][40] == TILE_CASTLE_GATE && game.map.tiles[47][50] == TILE_CASTLE_GATE_OPEN && game.map.tiles[46][40] == TILE_CASTLE_FLOOR);
    e = setup();
    game.player.x = 49;
    game.player.y = 48;
    prepare(e, ARCANIST_BLAST);
    ASSERT("closed gate cells are absent from the locked blast footprint", !royal_arcanist_marks(e, 50, 48));
    game.map.tiles[48][50] = TILE_CASTLE_GATE_OPEN;
    ASSERT("opening cover cannot extend an already advertised attack", !royal_arcanist_marks(e, 50, 48));
}

static int has_escape(const Enemy *e, int x, int y, int turns) {
    if (!royal_arcanist_marks(e, x, y)) {
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
        if (map_is_walkable(&game.map, nx, ny) && royal_arcanist_barrier_change(e, nx, ny) >= 0 &&
            (nx != e->x || ny != e->y) && has_escape(e, nx, ny, turns - 1)) {
            return 1;
        }
    }
    return 0;
}

static void test_arcanist_escapes(void) {
    int safe = 1;
    int cases = 0;
    Enemy *e = setup();
    for (int attack = ARCANIST_BLAST; attack <= ARCANIST_EAST_LANE; attack++) {
        for (int y = 44; y < 58; y++) {
            for (int x = 34; x < 56; x++) {
                if (!map_is_walkable(&game.map, x, y) || (x == e->x && y == e->y)) {
                    continue;
                }
                game.player.x = x;
                game.player.y = y;
                prepare(e, attack);
                safe &= has_escape(e, x, y, 2);
                cases++;
            }
        }
    }
    ASSERT("every signature leaves an ordinary two-action escape throughout the chapel, including future gate closures", safe && cases > 1000);
}

static void test_arcanist_ranged(void) {
    Item bows[6] = {item_make_bow(), item_make_shortbow(), item_make_longbow(), item_make_crossbow(), item_make_magic_longbow(), item_make_krakenbone_bow()};
    for (int i = 0; i < 6; i++) {
        Enemy *e = setup();
        game.player.x = e->x;
        game.player.y = e->y - bows[i].range;
        game.player.last_dx = 0;
        game.player.last_dy = 1;
        game.player.player_class = CLASS_ROGUE;
        game.inventory[0] = bows[i];
        game.inventory_count = 1;
        game.equipped_main_hand = 0;
        game.player.arrows = MAX_ARROWS;
        game.player.attack = 40;
        action_resolve_player(&game, (Action){ACTION_RANGED_ATTACK, 0, 0});
        ASSERT("all bow ranges can damage the Arcanist from a clear approach", e->hp < e->max_hp);
        castle_tick(&game);
        ASSERT("Arcanist answers corridor bow fire with a warning while remaining inside his arena", e->move_timer == 3 &&
            royal_arcanist_marks(e, game.player.x, game.player.y) && e->x == 45 && e->y == 49);
    }
    Enemy *e = setup();
    game.player.x = 15;
    game.player.y = 11;
    castle_tick(&game);
    ASSERT("a distant player does not activate an unprovoked Arcanist", e->attack_phase == 0 && e->move_timer == 0 && e->x == 45);
}

static void test_arcanist_reward(void) {
    Item robes = item_make_crown_chapel_vestments();
    Item archmage = item_make_archmage_robes();
    ASSERT("Vestments have approved Mage-only armor stats below Archmage resource bonuses", robes.defense_bonus == 4 && robes.max_mp_bonus == 45 && robes.spell_cost_reduction_percent == 15 &&
        robes.max_mp_bonus < archmage.max_mp_bonus && robes.spell_cost_reduction_percent < archmage.spell_cost_reduction_percent && robes.class_mask == ITEM_CLASS_MAGE && robes.armor_family == ARMOR_FAMILY_ROBE);
    for (int method = 0; method < 5; method++) {
        Enemy *e = setup();
        e->hp = 1;
        game.player.y = e->y + 1;
        game.player.last_dx = 0;
        game.player.last_dy = -1;
        game.player.attack = 1000;
        int gold = game.gold;
        if (method == 0) {
            action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
        } else if (method == 1) {
            game.player.player_class = CLASS_ROGUE;
            game.player.y = e->y + 2;
            game.inventory[0] = item_make_bow();
            game.inventory_count = 1;
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
        ASSERT("melee, arrows and all damage spells reserve Vestments with the original 100 gold and floor-five unlock", !e->active && game.arcanist_reward_state == 1 && game.gold == gold + 100 && (game.castle_minibosses & 2) && !game.game_won);
        castle_record_death(&game, e);
        ASSERT("duplicate death handling cannot repeat the payout", game.gold == gold + 100 && game.arcanist_reward_state == 1);
    }
    game.inventory_count = MAX_INVENTORY;
    game.floor_item_count = MAX_FLOOR_ITEMS;
    game.player.x = 53;
    game.player.y = 57;
    castle_interact(&game);
    ASSERT("full inventory and maximum floor loot leave the guaranteed reward reserved", game.arcanist_reward_state == 1 && game.inventory_count == MAX_INVENTORY);
    ASSERT("unclaimed reward saves without requiring a free loot slot", save_game(&game, ARCANIST_SLOT) && load_game(&loaded, ARCANIST_SLOT) && loaded.arcanist_reward_state == 1);
    castle_leave(&game, 0);
    castle_enter(&game, 4);
    game.player.x = 53;
    game.player.y = 57;
    game.inventory_count = 0;
    castle_interact(&game);
    ASSERT("reserved robes remain recoverable after castle retreat", game.arcanist_reward_state == 2 && game.inventory_count == 1 && game.inventory[0].visual_id == ITEM_VISUAL_CROWN_CHAPEL_VESTMENTS);
    int max_mp = game.player.max_mp;
    action_resolve_player(&game, (Action){ACTION_EQUIP_ITEM, 0, 0});
    ASSERT("Mage can equip Vestments and receives their maximum MP bonus", game.equipped_armor == EQUIPMENT_ARMOR && game.player.max_mp == max_mp + 45);
    ASSERT("claim and equipped item metadata survive save/load without another award", save_game(&game, ARCANIST_SLOT) && load_game(&loaded, ARCANIST_SLOT) && loaded.arcanist_reward_state == 2 && loaded.equipped_armor == EQUIPMENT_ARMOR && loaded.inventory[EQUIPMENT_ARMOR].max_mp_bonus == 45 && !castle_interact(&loaded));
}

static cJSON *read_fixture(void) {
    FILE *f = fopen("saves/savegame_99203.json", "rb");
    if (!f) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *buffer = malloc((size_t)size + 1);
    fread(buffer, 1, (size_t)size, f);
    buffer[size] = '\0';
    fclose(f);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    return root;
}

static int write_fixture(cJSON *root) {
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    FILE *f = fopen("saves/savegame_99203.json", "wb");
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
    if (!save_game(&game, ARCANIST_SLOT)) {
        return 0;
    }
    cJSON *root = read_fixture();
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 137);
    cJSON_DeleteItemFromObject(cJSON_GetObjectItem(root, "castle"), "arcanist_reward");
    return write_fixture(root);
}

static void test_arcanist_saves(void) {
    for (int attack = ARCANIST_BLAST; attack <= ARCANIST_EAST_LANE; attack++) {
        Enemy *e = setup();
        prepare(e, attack);
        e->frozen_turns = 2;
        ASSERT("save/load preserves each signature's phase, masks, target, barrier plan, countdown and freeze", save_game(&game, ARCANIST_SLOT) && load_game(&loaded, ARCANIST_SLOT) &&
            memcmp(e, find_boss(loaded.enemies, loaded.enemy_count), sizeof(*e)) == 0);
    }
    Enemy *e = setup();
    game.player.x = 37;
    game.player.y = 55;
    prepare(e, ARCANIST_WEST_LANE);
    castle_interact(&game);
    castle_tick(&game);
    ASSERT("two-action ward-counter recovery survives save/load", save_game(&game, ARCANIST_SLOT) && load_game(&loaded, ARCANIST_SLOT) &&
        find_boss(loaded.enemies, loaded.enemy_count)->revive_timer == 1 && find_boss(loaded.enemies, loaded.enemy_count)->move_timer == 2);
    castle_enter(&game, 5);
    ASSERT("cached chapel retains spent wards and recovery", save_game(&game, ARCANIST_SLOT) && load_game(&loaded, ARCANIST_SLOT) && loaded.castle_cache[3].map.tiles[54][37] == TILE_CASTLE_WARD_SPENT && find_boss(loaded.castle_cache[3].enemies, loaded.castle_cache[3].enemy_count)->revive_timer == 1);
    castle_enter(&loaded, 4);
    ASSERT("revisiting chapel resumes the same attempt", loaded.map.tiles[54][37] == TILE_CASTLE_WARD_SPENT && find_boss(loaded.enemies, loaded.enemy_count)->move_timer == 2);
    e = setup();
    prepare(e, ARCANIST_EAST_LANE);
    castle_enter(&game, 5);
    castle_travel(&game, 0);
    e = find_boss(game.enemies, game.enemy_count);
    ASSERT("a protected stair arrival cancels both its threatened lane and barrier plan", game.level == 4 && e->attack_target_x == -1 && e->move_timer == 2 && e->revive_timer == 0 && !e->boss_aux_x && !e->boss_aux_y);
    ASSERT("arrival protection leaves one recovery action instead of interpreting gate bits as a timer", castle_tick(&game) == 0 && e->move_timer == 0);
    castle_leave(&loaded, 0);
    castle_enter(&loaded, 4);
    Enemy *restored = find_boss(loaded.enemies, loaded.enemy_count);
    ASSERT("castle retreat restores surviving Arcanist and wards without replaying a warning", restored->hp == restored->max_hp && restored->move_timer == 0 && !restored->attack_phase && loaded.map.tiles[54][37] == TILE_CASTLE_WARD);

    e = setup();
    e->hp = 111;
    e->attack_target_x = 45;
    e->attack_target_y = 53;
    e->move_timer = 3;
    e->frozen_turns = 1;
    game.gold = 321;
    game.enemies[0].hp = 7;
    game.map.tiles[54][37] = TILE_CASTLE_WARD_SPENT;
    map_mark_explored(&game.map, 45, 53);
    ASSERT("version 137 live migration preserves old blast timing, health, freeze, guards, wards and gold", legacy_fixture() && load_game(&loaded, ARCANIST_SLOT) &&
        find_boss(loaded.enemies, loaded.enemy_count)->hp == 111 && find_boss(loaded.enemies, loaded.enemy_count)->move_timer == 3 &&
        find_boss(loaded.enemies, loaded.enemy_count)->frozen_turns == 1 && loaded.enemies[0].hp == 7 && !loaded.enemies[0].active &&
        loaded.map.tiles[54][37] == TILE_CASTLE_WARD_SPENT && loaded.gold == 321 && map_is_explored(&loaded.map, 45, 53) && !loaded.arcanist_reward_state);
    ASSERT("migration locks the legacy five-stone footprint without widening it at low HP", royal_arcanist_marks(find_boss(loaded.enemies, loaded.enemy_count), 45, 54) &&
        !royal_arcanist_marks(find_boss(loaded.enemies, loaded.enemy_count), 45, 56) && find_boss(loaded.enemies, loaded.enemy_count)->attack_phase == 1);
    castle_enter(&game, 5);
    ASSERT("version 137 cached migration preserves the same queued attack", legacy_fixture() && load_game(&loaded, ARCANIST_SLOT) &&
        find_boss(loaded.castle_cache[3].enemies, loaded.castle_cache[3].enemy_count)->hp == 111 && find_boss(loaded.castle_cache[3].enemies, loaded.castle_cache[3].enemy_count)->move_timer == 3);
    game.castle_minibosses = 2;
    find_boss(game.castle_cache[3].enemies, game.castle_cache[3].enemy_count)->active = 0;
    ASSERT("historical victory reserves only the newly introduced robes without repeating gold or respawning boss", legacy_fixture() && load_game(&loaded, ARCANIST_SLOT) &&
        loaded.arcanist_reward_state == 1 && loaded.gold == 321 && loaded.castle_minibosses == 2 && !find_boss(loaded.castle_cache[3].enemies, loaded.castle_cache[3].enemy_count)->active);
    castle_enter(&loaded, 4);
    loaded.player.x = 53;
    loaded.player.y = 57;
    loaded.inventory_count = 0;
    castle_interact(&loaded);
    ASSERT("new reward for an old victory can be claimed exactly once", save_game(&loaded, ARCANIST_SLOT) && load_game(&game, ARCANIST_SLOT) && game.arcanist_reward_state == 2 && !castle_interact(&game));
    cJSON *root = read_fixture();
    cJSON_SetNumberValue(cJSON_GetObjectItem(cJSON_GetObjectItem(root, "castle"), "arcanist_reward"), 3);
    ASSERT("save loader rejects an invalid new reward state", write_fixture(root) && !load_game(&loaded, ARCANIST_SLOT));
    remove("saves/savegame_99203.json");
}

void test_royal_arcanist(void) {
    printf("Royal Arcanist encounter\n");
    test_arcanist_phases();
    test_arcanist_lanes();
    test_arcanist_barriers();
    test_arcanist_escapes();
    test_arcanist_ranged();
    test_arcanist_reward();
    test_arcanist_saves();
}
