#include "test_utils.h"
#include "game/castle.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define CASTELLAN_SLOT 99201
static GameState game;
static GameState loaded;

static Enemy *find_boss(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == ENEMY_CASTELLAN) {
            return &g->enemies[i];
        }
    }
    return NULL;
}

static Enemy *setup(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    castle_enter(&game, 2);
    Enemy *e = find_boss(&game);
    for (int i = 0; i < game.enemy_count; i++) {
        game.enemies[i].active = &game.enemies[i] == e;
    }
    game.player.x = 45;
    game.player.y = 46;
    game.player.hp = 1000;
    game.player.max_hp = 1000;
    game.player.defense = 0;
    game.player.experience_next = 100000;
    return e;
}

static void test_castellan_phases(void) {
    Enemy *e = setup();
    ASSERT("Castellan starts with unchanged health and damage", e->max_hp == 420 && e->attack == 35);
    ASSERT("Shield Commander warns a fixed one-action charge", castle_tick(&game) == 0 && e->boss_attack == 1 &&
        e->attack_phase == CASTELLAN_CHARGE && e->move_timer == 1 && castle_attack_marks(&game.map, e, 45, 46));
    int mask = e->boss_aux_x;
    e->hp = e->max_hp / 2;
    e->frozen_turns = 1;
    ASSERT("freeze pauses the warning and half-health transition", castle_tick(&game) == 0 && e->move_timer == 1 && e->boss_attack == 1 && e->boss_aux_x == mask);
    game.player.x++;
    ASSERT("crossing half health finishes the original attack without retargeting", castle_tick(&game) == 0 && e->boss_attack == 1 && e->move_timer == 2);
    ASSERT("normal recovery permits a full action with no retaliation", castle_tick(&game) == 0 && e->move_timer == 0);
    ASSERT("Broken Formation announces one two-action marked rally", castle_tick(&game) == 0 && e->boss_attack == 2 &&
        e->attack_phase == CASTELLAN_RALLY && e->move_timer == 3 && e->boss_wave_used && castle_castellan_marks(e, 42, 46) &&
        !castle_attack_marks(&game.map, e, 42, 46));
    int count = game.enemy_count;
    game.player.x = 42;
    game.player.y = 46;
    ASSERT("rally gives its second harmless warning", castle_tick(&game) == 0 && game.enemy_count == count && e->move_timer == 1);
    ASSERT("occupying one muster stone prevents that guard without damaging player", castle_tick(&game) == 0 && game.enemy_count == count + 1);
    Enemy *guard = &game.enemies[count];
    ASSERT("the finite arrival has no loot or XP and starts acting next turn", guard->type == ENEMY_ROYAL_MARKSMAN &&
        guard->x == 48 && guard->y == 46 && guard->experience == 0 && guard->move_timer == 0);
    int gold = game.gold;
    castle_record_death(&game, guard);
    ASSERT("rally guards cannot produce gold", game.gold == gold);
    guard->active = 0;
    e->x = 45;
    e->y = 49;
    game.player.x = e->x;
    game.player.y = e->y - 1;
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("phase two starts with a wider two-action sweep", e->attack_phase == CASTELLAN_SWEEP && e->move_timer == 3 && castle_attack_marks(&game.map, e, e->x + 2, e->y - 2));
    mask = e->boss_aux_x;
    game.player.y = e->y + 1;
    castle_tick(&game);
    ASSERT("the sweep locks its facing and cells while the player moves", e->move_timer == 1 && e->boss_aux_x == mask && castle_attack_marks(&game.map, e, e->x + 2, e->y - 2));
    ASSERT("walking behind the sweep avoids damage", castle_tick(&game) == 0 && e->move_timer == 2);
    castle_tick(&game);
    e->hp = e->max_hp;
    castle_tick(&game);
    ASSERT("healing cannot reverse phase two; the next attack is a newly warned charge", e->boss_attack == 2 && e->attack_phase == CASTELLAN_CHARGE && e->move_timer == 1);

    e = setup();
    game.player.y = e->y - 1;
    castle_tick(&game);
    ASSERT("phase one retains its smaller shield swing", e->attack_phase == CASTELLAN_SWING && e->move_timer == 1);
    ASSERT("standing on the swing footprint takes exactly one hit", castle_tick(&game) == e->attack && e->move_timer == 2);
    ASSERT("shield is lowered during recovery", castle_enemy_damage(&game, e, 100) == 100);
    castle_tick(&game);
    ASSERT("front armor remains active outside recovery", castle_enemy_damage(&game, e, 100) == 50);
    game.player.y = e->y + 1;
    ASSERT("flanking still bypasses the shield", castle_enemy_damage(&game, e, 100) == 100);
}

static void test_castellan_sweep_escape(void) {
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    for (int d = 0; d < 4; d++) {
        Enemy *e = setup();
        e->boss_attack = 2;
        e->boss_wave_used = 1;
        game.player.x = e->x + dx[d];
        game.player.y = e->y + dy[d];
        castle_tick(&game);
        int escaped = 1;
        for (int y = 44; y < 58; y++) {
            for (int x = 34; x < 56; x++) {
                if (!castle_attack_marks(&game.map, e, x, y)) {
                    continue;
                }
                int safe = 0;
                for (int first = 0; first < 4; first++) {
                    int ax = x + dx[first];
                    int ay = y + dy[first];
                    if (!map_is_walkable(&game.map, ax, ay) || (ax == e->x && ay == e->y)) {
                        continue;
                    }
                    safe |= !castle_attack_marks(&game.map, e, ax, ay);
                    for (int second = 0; second < 4; second++) {
                        int bx = ax + dx[second];
                        int by = ay + dy[second];
                        safe |= map_is_walkable(&game.map, bx, by) && (bx != e->x || by != e->y) && !castle_attack_marks(&game.map, e, bx, by);
                    }
                }
                escaped &= safe;
            }
        }
        ASSERT("every marked sweep tile has an ordinary walking escape within two actions", escaped);
        castle_tick(&game);
        ASSERT("sweep damage uses exactly its visible fixed footprint", castle_tick(&game) == e->attack && e->move_timer == 2);
    }
}

static void test_castellan_arena_escape(void) {
    Enemy *e = setup();
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    int safe = 1;
    int cases = 0;
    for (int y = 44; y < 58; y++) {
        for (int x = 34; x < 56; x++) {
            if (!map_is_walkable(&game.map, x, y)) {
                continue;
            }
            for (int d = 0; d < 4; d++) {
                int px = x + dx[d];
                int py = y + dy[d];
                if (px < 34 || px >= 56 || py < 44 || py >= 58 || !map_is_walkable(&game.map, px, py)) {
                    continue;
                }
                e->x = x;
                e->y = y;
                e->boss_attack = 2;
                e->boss_wave_used = 1;
                e->boss_aux_y = 0;
                e->move_timer = 0;
                e->attack_target_x = -1;
                game.player.x = px;
                game.player.y = py;
                castle_tick(&game);
                int escape = 0;
                for (int first = 0; first < 4; first++) {
                    int ax = px + dx[first];
                    int ay = py + dy[first];
                    if (!map_is_walkable(&game.map, ax, ay) || (ax == x && ay == y)) {
                        continue;
                    }
                    escape |= !castle_attack_marks(&game.map, e, ax, ay);
                    for (int second = 0; second < 4; second++) {
                        int bx = ax + dx[second];
                        int by = ay + dy[second];
                        escape |= map_is_walkable(&game.map, bx, by) && (bx != x || by != y) && !castle_attack_marks(&game.map, e, bx, by);
                    }
                }
                safe &= escape;
                cases++;
            }
        }
    }
    ASSERT("sweep can be escaped at every walkable boss position and facing in Iron Keep", safe && cases > 1000);
}

static void test_castellan_controls(void) {
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        Enemy *e = setup();
        game.player.player_class = cls;
        e->x = 42;
        game.player.x = 37;
        game.player.y = 50;
        castle_tick(&game);
        ASSERT("side controls have an accessible charge lane", e->attack_phase == CASTELLAN_CHARGE && castle_attack_marks(&game.map, e, 40, 49));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        action_resolve_enemies(&game);
        ASSERT("every class can use a side lever to interrupt a warned charge",
            e->attack_target_x == -1 && e->move_timer == 2 && e->revive_timer == 1 && game.map.tiles[49][40] == TILE_CASTLE_GATE);
        ASSERT("the interrupted charge cannot damage or move the boss during its long opening", castle_tick(&game) == 0 && e->x == 42 && e->move_timer == 2 && e->revive_timer == 0);
        ASSERT("the second counterattack action is also safe", castle_tick(&game) == 0 && e->move_timer == 0);
        castle_tick(&game);
        int mask = e->boss_aux_x;
        castle_interact(&game);
        ASSERT("opening a portcullis cannot extend an already declared footprint", e->boss_aux_x == mask && !castle_attack_marks(&game.map, e, 39, 49));
    }
    Enemy *e = setup();
    Enemy *guard = &game.enemies[0];
    guard->active = 1;
    guard->x = 40;
    guard->y = 48;
    game.player.x = 37;
    game.player.y = 50;
    castle_interact(&game);
    ASSERT("occupied gate cells stay open while adjacent empty cells close", game.map.tiles[48][40] == TILE_CASTLE_GATE_OPEN && game.map.tiles[47][40] == TILE_CASTLE_GATE && game.map.tiles[49][40] == TILE_CASTLE_GATE);
    guard->active = 0;
    castle_interact(&game);
    ASSERT("partially closed gates can close after an actor leaves", game.map.tiles[48][40] == TILE_CASTLE_GATE);
    castle_interact(&game);
    ASSERT("the same lever reopens a fully closed gate", game.map.tiles[48][40] == TILE_CASTLE_GATE_OPEN);
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 40, .y = 47, .underlying_tile = TILE_CASTLE_GATE_OPEN, .item = item_make_magic_shield()};
    game.map.tiles[47][40] = TILE_ITEM;
    castle_interact(&game);
    ASSERT("loot-covered gate stones remain visibly open without changing their item", game.map.tiles[47][40] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_CASTLE_GATE_OPEN && game.map.tiles[48][40] == TILE_CASTLE_GATE);
    game.map.tiles[47][40] = TILE_CASTLE_GATE_OPEN;
    game.floor_item_count = 0;
    castle_interact(&game);
    castle_interact(&game);
    e->x = 42;
    game.player.x = 37;
    game.player.y = 49;
    guard->active = 1;
    guard->x = 40;
    guard->y = 49;
    guard->move_timer = 2;
    guard->frozen_turns = 3;
    castle_tick(&game);
    ASSERT("a guard collision protects a player beyond the guard", castle_tick(&game) == 0 && e->x == 41 && e->move_timer == 2 && e->revive_timer == 1 && guard->frozen_turns == 2);
    guard->active = 0;
    game.player.x = 15;
    game.player.y = 11;
    int hp = e->hp;
    castle_tick(&game);
    ASSERT("leaving the arena cancels warnings without healing or moving the boss", e->hp == hp && e->attack_target_x == -1 && e->x == 41);
}

static void test_castellan_reward(void) {
    Item shield = item_make_castellan_bulwark();
    Item magic = item_make_magic_shield();
    ASSERT("Bulwark trades one defense for five block chance against Magic Shield", shield.defense_bonus == 8 && shield.block_chance == 30 && shield.block_reduction_percent == 50 &&
        shield.defense_bonus == magic.defense_bonus - 1 && shield.block_chance == magic.block_chance + 5);
    ASSERT("Bulwark is a rare Warrior shield with its own saved visual", shield.type == ITEM_SHIELD && shield.rarity == ITEM_RARITY_RARE &&
        item_class_allowed(&shield, CLASS_WARRIOR) && !item_class_allowed(&shield, CLASS_MAGE) && !item_class_allowed(&shield, CLASS_ROGUE) && shield.visual_id == ITEM_VISUAL_CASTELLAN_BULWARK);
    for (int method = 0; method < 5; method++) {
        Enemy *e = setup();
        e->hp = 1;
        game.player.x = e->x;
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
        ASSERT("melee, arrows and every damage spell award Bulwark plus the existing gold/stair unlock", !e->active && game.castellan_reward_state == 1 && game.gold == gold + 100 && (game.castle_minibosses & 1) && !game.game_won);
        castle_record_death(&game, e);
        ASSERT("recording a repeated death cannot duplicate rewards", game.gold == gold + 100 && game.castellan_reward_state == 1);
    }
    game.inventory_count = MAX_INVENTORY;
    game.floor_item_count = MAX_FLOOR_ITEMS;
    game.player.x = 53;
    game.player.y = 54;
    ASSERT("full inventory and floor loot leave the guaranteed shield reserved", castle_interact(&game) && game.castellan_reward_state == 1 && game.inventory_count == MAX_INVENTORY);
    ASSERT("unclaimed armory reservation survives save/load with maximum loot", save_game(&game, CASTELLAN_SLOT) && load_game(&loaded, CASTELLAN_SLOT) && loaded.castellan_reward_state == 1 && loaded.floor_item_count == MAX_FLOOR_ITEMS);
    castle_leave(&game, 0);
    castle_enter(&game, 2);
    game.player.x = 53;
    game.player.y = 54;
    game.inventory_count = 0;
    ASSERT("unclaimed Bulwark survives retreat and is collectible by every class", castle_interact(&game) && game.castellan_reward_state == 2 && game.inventory_count == 1 && strcmp(game.inventory[0].name, "Castellan's Bulwark") == 0);
    ASSERT("claiming the armory cannot create a second shield", !castle_interact(&game) && game.inventory_count == 1);
    ASSERT("claimed shield metadata and progress survive save/load", save_game(&game, CASTELLAN_SLOT) && load_game(&loaded, CASTELLAN_SLOT) && loaded.castellan_reward_state == 2 && loaded.inventory[0].visual_id == shield.visual_id && loaded.inventory[0].block_chance == 30);
}

static int legacy_fixture(void) {
    if (!save_game(&game, CASTELLAN_SLOT)) {
        return 0;
    }
    FILE *f = fopen("saves/savegame_99201.json", "rb");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *buffer = malloc((size_t)size + 1);
    fread(buffer, 1, (size_t)size, f);
    buffer[size] = '\0';
    fclose(f);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 136);
    cJSON_DeleteItemFromObject(cJSON_GetObjectItem(root, "castle"), "castellan_reward");
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    f = fopen("saves/savegame_99201.json", "wb");
    fputs(json, f);
    fclose(f);
    free(json);
    return 1;
}

static void test_castellan_saves(void) {
    Enemy *e = setup();
    e->hp = 205;
    e->boss_attack = 2;
    e->boss_wave_used = 1;
    castle_tick(&game);
    e->frozen_turns = 2;
    int mask = e->boss_aux_x;
    ASSERT("save/load retains phase, alternation, locked sweep, countdown and freeze", save_game(&game, CASTELLAN_SLOT) && load_game(&loaded, CASTELLAN_SLOT) &&
        find_boss(&loaded)->boss_attack == 2 && find_boss(&loaded)->boss_aux_y == 1 && find_boss(&loaded)->boss_aux_x == mask &&
        find_boss(&loaded)->move_timer == 3 && find_boss(&loaded)->frozen_turns == 2);
    castle_enter(&game, 3);
    ASSERT("cached Iron Keep also preserves the queued attack", save_game(&game, CASTELLAN_SLOT) && load_game(&loaded, CASTELLAN_SLOT) && loaded.castle_cache[1].enemies[10].boss_aux_x == mask);
    castle_enter(&loaded, 2);
    ASSERT("floor revisit resumes warning without restarting it", find_boss(&loaded)->move_timer == 3 && find_boss(&loaded)->frozen_turns == 2);
    castle_leave(&loaded, 0);
    castle_enter(&loaded, 2);
    Enemy *restored = find_boss(&loaded);
    ASSERT("castle retreat resets health and phases but does not replenish the finite rally", restored->hp == restored->max_hp && !restored->boss_attack && restored->boss_wave_used && restored->attack_target_x == -1);

    e = setup();
    castle_tick(&game);
    e->hp = 123;
    game.gold = 321;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 40, .y = 48, .underlying_tile = TILE_CASTLE_FLOOR, .item = item_make_magic_shield()};
    game.map.tiles[48][40] = TILE_ITEM;
    game.map.tiles[49][40] = TILE_CASTLE_FLOOR;
    map_mark_explored(&game.map, 40, 48);
    ASSERT("version 136 live castle migrates without resetting health, loot, exploration or gold", legacy_fixture() && load_game(&loaded, CASTELLAN_SLOT) &&
        find_boss(&loaded)->hp == 123 && loaded.gold == 321 && loaded.map.tiles[48][40] == TILE_ITEM &&
        loaded.floor_items[0].underlying_tile == TILE_CASTLE_GATE_OPEN && loaded.map.tiles[49][40] == TILE_CASTLE_GATE_OPEN &&
        map_is_explored(&loaded.map, 40, 48) && !loaded.castellan_reward_state);
    castle_enter(&game, 3);
    ASSERT("version 136 cached Iron Keep migrates without regenerating guards", legacy_fixture() && load_game(&loaded, CASTELLAN_SLOT) && loaded.castle_cache[1].enemy_count == 11 && loaded.castle_loot[1][0].underlying_tile == TILE_CASTLE_GATE_OPEN && loaded.castle_cache[1].map.tiles[49][40] == TILE_CASTLE_GATE_OPEN);
    game.castle_minibosses = 1;
    game.castle_cache[1].enemies[10].active = 0;
    ASSERT("historical victories keep gold/progression and reserve the new thematic shield once", legacy_fixture() && load_game(&loaded, CASTELLAN_SLOT) && loaded.castellan_reward_state == 1 && loaded.castle_minibosses == 1 && loaded.gold == 321);
    castle_enter(&loaded, 2);
    loaded.player.x = 53;
    loaded.player.y = 54;
    loaded.inventory_count = 0;
    castle_interact(&loaded);
    ASSERT("migrated victory can claim its new shield without respawning Castellan", loaded.castellan_reward_state == 2 && !find_boss(&loaded)->active && loaded.gold == 321);
    ASSERT("migrated reward stays claimed after rewriting the save", save_game(&loaded, CASTELLAN_SLOT) && load_game(&game, CASTELLAN_SLOT) && game.castellan_reward_state == 2);
    remove("saves/savegame_99201.json");
}

void test_castellan(void) {
    printf("Castellan encounter\n");
    test_castellan_phases();
    test_castellan_sweep_escape();
    test_castellan_arena_escape();
    test_castellan_controls();
    test_castellan_reward();
    test_castellan_saves();
}
