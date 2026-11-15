#include "test_utils.h"
#include "castle_route_checks.h"
#include "game/castle.h"
#include "game/malakar.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define MALAKAR_SLOT 99362
#define MALAKAR_SAVE "saves/savegame_99362.json"

static GameState game;
static GameState loaded;

static Enemy *setup(void) {
    game_init(&game);
    castle_enter(&game, 2);
    for (int i = 0; i < game.enemy_count; i++) {
        if (game.enemies[i].type != ENEMY_MALAKAR) {
            game.enemies[i].active = 0;
        }
    }
    return &game.enemies[0];
}

static int legacy_version(void) {
    FILE *file = fopen(MALAKAR_SAVE, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *text = malloc(length + 1);
    if (!text) {
        fclose(file);
        return 0;
    }
    size_t read = fread(text, 1, length, file);
    text[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 253);
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = text ? fopen(MALAKAR_SAVE, "wb") : NULL;
    int ok = file != NULL;
    if (file) {
        fputs(text, file);
        fclose(file);
    }
    free(text);
    return ok;
}

static void test_damage_paths(void) {
    for (int kind = 0; kind < 4; kind++) {
        Enemy *e = setup();
        game.player.x = kind == 0 ? 44 : 42;
        game.player.y = 14;
        game.player.last_dx = 1;
        game.player.last_dy = 0;
        game.player.attack = 5000;
        if (kind == 0) {
            action_resolve_player(&game, (Action){ACTION_MOVE, 45, 14});
        } else if (kind == 1) {
            game.player.player_class = CLASS_ROGUE;
            game.inventory[0] = item_make_bow();
            game.inventory_count = 1;
            game_equip_main_hand(&game, 0);
            game.player.arrows = MAX_ARROWS;
            action_resolve_player(&game, (Action){ACTION_RANGED_ATTACK, 0, 0});
        } else if (kind == 2) {
            game.player.player_class = CLASS_MAGE;
            game.player.known_spell_count = 1;
            game.player.known_spells[0] = spell_make_magic_arrow();
            game.player.known_spells[0].damage = 5000;
            game.player.equipped_spell = 0;
            game.player.mp = 100;
            action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
        } else {
            e->hp = e->max_hp * 2 / 5 + 1;
            e->defense = 0;
            action_aldric_strike(&game, e);
        }
        ASSERT("melee, arrows, magic and companion damage force escape without killing Malakar", !e->active && e->hp == e->max_hp * 2 / 5 && !game.game_won && (game.castle_minibosses & 1));
        ASSERT("even a lethal-sized hit opens the seal and reserves the existing equipment reward", game.map.tiles[22][35] == TILE_CASTLE_GATE_OPEN && game.gold == 100 && game.castellan_reward_state == 1);
        int gold = game.gold;
        int xp = game.player.experience;
        int applied = castle_enemy_damage(&game, e, 9999);
        e->hp -= applied;
        ASSERT("an escaped Malakar cannot award duplicate money or experience", !applied && game.gold == gold && game.player.experience == xp && e->hp > 0);
    }
}

static void test_preparations(void) {
    Enemy *e = setup();
    game.player.x = 45;
    game.player.y = 16;
    int damage = castle_tick(&game);
    ASSERT("overpressure locks a marked footprint with three full warning actions", !damage && e->boss_attack == MALAKAR_PRESSURE && e->move_timer == 3 && castle_attack_marks(&game.map, e, 45, 16));
    int mask = e->boss_aux_x;
    ASSERT("the preparation's footprint and source survive save/load", save_game(&game, MALAKAR_SLOT) && load_game(&loaded, MALAKAR_SLOT) &&
        loaded.enemies[0].boss_aux_x == mask && loaded.enemies[0].boss_aux_y == 38 && loaded.enemies[0].move_timer == 3);
    e->frozen_turns = 2;
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("freezing pauses the mixture countdown and fixed footprint", !e->frozen_turns && e->move_timer == 3 && e->boss_aux_x == mask);
    int dodged = 1;
    for (int x = 46; x <= 48; x++) {
        action_resolve_player(&game, (Action){ACTION_MOVE, x, 16});
        dodged &= game.player.x == x && castle_tick(&game) == 0;
    }
    ASSERT("three actual movement actions dodge the resolving overpressure", dodged && e->attack_target_x == -1 && e->move_timer == 1);
    castle_tick(&game);
    ASSERT("a resolved mixture provides a full recovery action", !e->move_timer && e->attack_target_x < 0);

    e = setup();
    game.player.x = 37;
    game.player.y = 16;
    castle_tick(&game);
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("disabling the linked condenser cancels a warned overpressure", game.map.tiles[16][38] == TILE_CASTLE_WARD_SPENT && e->attack_target_x < 0 && e->move_timer == 2);

    e = setup();
    game.player.x = 45;
    game.player.y = 16;
    game.map.tiles[16][44] = TILE_CASTLE_WALL;
    game.map.tiles[16][46] = TILE_CASTLE_WALL;
    game.map.tiles[17][45] = TILE_CASTLE_WALL;
    game.enemies[1].active = 1;
    game.enemies[1].x = 45;
    game.enemies[1].y = 15;
    castle_tick(&game);
    ASSERT("a preparation without a usable escape route vents harmlessly", e->attack_target_x < 0 && !e->boss_attack && e->move_timer == 1);

    e = setup();
    game.player.x = 45;
    game.player.y = 16;
    e->hp = 280;
    castle_tick(&game);
    ASSERT("a wounded Malakar announces a restorative linked to a condenser", e->boss_attack == MALAKAR_RESTORE && e->move_timer == 2 && e->attack_target_x == 38);
    int applied = castle_enemy_damage(&game, e, 10);
    e->hp -= applied;
    ASSERT("ordinary damage interrupts the restorative and exposes Malakar", e->hp == 270 && e->boss_attack == 0 && e->move_timer == 2 && e->attack_target_x < 0);
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("an interrupted restorative cannot be attempted repeatedly", e->hp == 270 && e->boss_attack != MALAKAR_RESTORE);

    e = setup();
    game.player.x = 37;
    game.player.y = 16;
    e->hp = 280;
    castle_tick(&game);
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("every class can interrupt the linked preparation through Interact", game.map.tiles[16][38] == TILE_CASTLE_WARD_SPENT && e->attack_target_x < 0 && e->move_timer == 2);
    castle_store(&game);
    ASSERT("spent apparatus and interrupted preparation persist in both save records", save_game(&game, MALAKAR_SLOT) && load_game(&loaded, MALAKAR_SLOT) &&
        loaded.map.tiles[16][38] == TILE_CASTLE_WARD_SPENT && loaded.castle_cache[1].map.tiles[16][38] == TILE_CASTLE_WARD_SPENT && loaded.enemies[0].move_timer == 2);

    e = setup();
    game.player.x = 45;
    game.player.y = 16;
    e->hp = 280;
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("an uninterrupted restorative resolves once without exceeding maximum health", e->hp == 380 && (e->boss_wave_used & 1) && e->move_timer == 1);
    game.player.x = 15;
    game.player.y = 11;
    castle_tick(&game);
    ASSERT("retreat clears pending mixtures without blocking the entrance", e->attack_target_x < 0 && game.map.tiles[11][15] == TILE_STAIRS_DOWN);
    remove(MALAKAR_SAVE);
}

static void test_migration(void) {
    for (int completed = 0; completed < 2; completed++) {
        Enemy *e = setup();
        castle_legacy_layout(&game.map, 2);
        game.player.x = 25;
        game.player.y = 17;
        game.player.hp = 41;
        game.gold = 721;
        game.castle_minibosses = completed;
        game.castellan_reward_state = completed ? 2 : 0;
        e->type = ENEMY_VAMPIRE;
        snprintf(e->name, sizeof(e->name), "Vampire");
        e->x = 45;
        e->y = 49;
        e->hp = completed ? 0 : 63;
        e->max_hp = 150;
        e->active = !completed;
        game.floor_item_count = 1;
        game.floor_items[0] = (FloorItem){.active = 1, .x = 28, .y = 36, .underlying_tile = TILE_CASTLE_FLOOR, .item = item_make_health_potion()};
        game.map.tiles[36][28] = TILE_ITEM;
        map_mark_explored(&game.map, 15, 11);
        map_mark_explored(&game.map, 28, 36);
        unsigned char explored[MAP_EXPLORED_BYTES];
        memcpy(explored, game.map.explored, sizeof(explored));
        castle_store(&game);
        int ok = save_game(&game, MALAKAR_SLOT) && legacy_version() && load_game(&loaded, MALAKAR_SLOT);
        ASSERT("active and cached legacy Iron Keeps become the ten-room alchemy wing", ok && loaded.map.room_count == 10 && loaded.castle_cache[1].map.room_count == 10);
        ASSERT("the legacy encounter retains its position, wounds or historical victory", ok &&
            loaded.enemies[0].type == ENEMY_MALAKAR && loaded.enemies[0].x == 45 && loaded.enemies[0].y == 49 &&
            loaded.enemies[0].hp == (completed ? 0 : 63) && loaded.enemies[0].max_hp == 150 &&
            loaded.enemies[0].active == !completed && loaded.castle_cache[1].enemies[0].type == ENEMY_MALAKAR);
        ASSERT("migration retains player progress, rewards, exploration and loose loot", ok &&
            loaded.player.x == 25 && loaded.player.y == 17 && loaded.player.hp == 41 && loaded.gold == 721 &&
            loaded.castle_minibosses == completed && loaded.castellan_reward_state == (completed ? 2 : 0) &&
            !memcmp(explored, loaded.map.explored, sizeof(explored)) && loaded.floor_item_count == 1 && loaded.map.tiles[36][28] == TILE_ITEM);
        if (ok) {
            for (int i = 0; i < loaded.enemy_count; i++) {
                loaded.enemies[i].active = 0;
            }
            ASSERT("legacy occupants retain a continuous retreat through the new seal", castle_walk_to(&loaded, 15, 11));
            ASSERT("legacy occupants retain a continuous route to the real onward stair", castle_walk_to(&loaded, 45, 55));
            ASSERT("the migrated wing survives a current-format round trip", save_game(&loaded, MALAKAR_SLOT) && load_game(&game, MALAKAR_SLOT) && game.map.room_count == 10 && game.gold == 721);
        }
    }
    remove(MALAKAR_SAVE);
}

void test_malakar(void) {
    printf("Castle V2 Royal alchemy Wing tests:\n");
    for (unsigned seed = 1; seed <= 8; seed++) {
        srand(seed);
        Enemy *e = setup();
        int placed = game.enemy_count == 9;
        for (int i = 0; i < game.enemy_count; i++) {
            placed &= map_is_walkable(&game.map, game.enemies[i].x, game.enemies[i].y);
        }
        ASSERT("all nine laboratory-wing actors occupy usable tiles after placement", placed);
        ASSERT("Malakar occupies the early laboratory, with the rest of the wing initially sealed", e->active && e->type == ENEMY_MALAKAR && e->x == 45 && e->y == 14 && game.map.tiles[22][35] == TILE_CASTLE_GATE);
        ASSERT("real movement reaches the first encounter directly from the entry", castle_walk_to(&game, 44, 14));
        int applied = castle_enemy_damage(&game, e, 10000);
        e->hp -= applied;
        int connected = 1;
        for (int i = 0; i < game.map.room_count; i++) {
            int x;
            int y;
            map_room_center(&game.map.rooms[i], &x, &y);
            connected &= castle_walk_to(&game, x, y);
        }
        ASSERT("all ten rooms remain explorable after the escape and encounter placement", connected);
        ASSERT("the wing's real onward stair remains reachable", castle_walk_to(&game, 45, 55));
        action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
        ASSERT("forcing Malakar to flee unlocks actual travel to the Court and Household", game.level == 3);
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("cached revisits preserve Malakar's escape and the opened seal", game.level == 2 && !game.enemies[0].active && game.map.tiles[22][35] == TILE_CASTLE_GATE_OPEN);
        ASSERT("the wing retains a continuous route to the gatehouse stair", castle_walk_to(&game, 15, 11));
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("the retreat stair actually enters the gatehouse", game.level == 1);
        castle_leave(&game, 0);
        castle_enter(&game, 2);
        int returned = 0;
        for (int i = 0; i < game.enemy_count; i++) {
            returned |= game.enemies[i].active && game.enemies[i].type == ENEMY_MALAKAR;
        }
        ASSERT("castle regrouping cannot replay the completed laboratory fight", !returned && game.map.tiles[22][35] == TILE_CASTLE_GATE_OPEN);
    }
    test_damage_paths();
    test_preparations();
    test_migration();
}
