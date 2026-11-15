#include "test_utils.h"
#include "castle_route_checks.h"
#include "game/castle.h"
#include "game/castle_finale.h"
#include "game/malakar_final.h"
#include "game/aldric.h"
#include "game/storm_mage.h"
#include "game/garran.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define FINALE_SLOT 99370
#define FINALE_SAVE "saves/savegame_99370.json"
static GameState game;
static GameState loaded;

static Enemy *find_boss(GameState *g, EnemyType type) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == type) {
            return &g->enemies[i];
        }
    }
    return NULL;
}

static void clear_guards(GameState *g, Enemy *boss) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (&g->enemies[i] != boss) {
            g->enemies[i].active = 0;
        }
    }
}

static Enemy *setup(void) {
    game_init(&game);
    game.castle_minibosses = 3;
    castle_enter(&game, 6);
    Enemy *e = find_boss(&game, ENEMY_MALAKAR_FINAL);
    clear_guards(&game, e);
    game.player.x = 45;
    game.player.y = 52;
    return e;
}

static int legacy_version(void) {
    FILE *file = fopen(FINALE_SAVE, "rb");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 257);
    cJSON_DeleteItemFromObject(cJSON_GetObjectItem(root, "castle"), "ending");
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = text ? fopen(FINALE_SAVE, "wb") : NULL;
    int ok = file != NULL;
    if (file) {
        fputs(text, file);
        fclose(file);
    }
    free(text);
    return ok;
}

static void test_migration(void) {
    for (int sleeping = 0; sleeping < 2; sleeping++) {
        Enemy *e = setup();
        Map arena = game.map;
        castle_legacy_layout(&game.map, 6);
        for (int y = 44; y < 58; y++) {
            memcpy(&game.map.tiles[y][34], &arena.tiles[y][34], 22 * sizeof(TileType));
        }
        e->hp = 237;
        snprintf(e->name, sizeof(e->name), "Lord Veyr");
        malakar_final_tick(&game, e);
        e->frozen_turns = 3;
        game.map.tiles[54][37] = TILE_CASTLE_WARD_SPENT;
        game.floor_item_count = 2;
        game.floor_items[0] = (FloorItem){.active = 1, .x = 53, .y = 54, .underlying_tile = TILE_CASTLE_WARD, .item = item_make_health_potion()};
        game.map.tiles[54][53] = TILE_ITEM;
        game.floor_items[1] = (FloorItem){.active = 1, .x = 29, .y = 11, .underlying_tile = TILE_CASTLE_GATE_OPEN, .item = item_make_mana_potion()};
        game.map.tiles[11][29] = TILE_ITEM;
        game.player.hp = 41;
        game.gold = 713;
        map_mark_explored(&game.map, 45, 52);
        Map before = game.map;
        Enemy old = *e;
        castle_store(&game);
        if (sleeping) {
            game.location = LOCATION_TOWN;
            game.level = 1;
            game.enemy_count = 0;
            game.floor_item_count = 0;
            map_generate_town(&game.map, &game.player.x, &game.player.y);
        }
        int ok = save_game(&game, FINALE_SLOT) && legacy_version() && load_game(&loaded, FINALE_SLOT);
        ASSERT("active and cached legacy finales remodel without resetting player progress", ok && loaded.castle_cache[5].map.room_count == 8 && loaded.player.hp == 41 && loaded.gold == 713);
        if (!ok) {
            continue;
        }
        if (sleeping) {
            castle_enter(&loaded, 6);
        }
        int identical = 1;
        for (int y = 44; y < 58; y++) {
            identical &= !memcmp(&loaded.map.tiles[y][34], &before.tiles[y][34], 22 * sizeof(TileType));
            identical &= !memcmp(&loaded.castle_cache[5].map.tiles[y][34], &before.tiles[y][34], 22 * sizeof(TileType));
        }
        e = find_boss(&loaded, ENEMY_MALAKAR_FINAL);
        ASSERT("the saved arena retains exact wards, gates, warning geometry and loot overlays", identical && loaded.floor_items[0].underlying_tile == TILE_CASTLE_WARD);
        ASSERT("loot cannot silently reclose the council gate during migration", loaded.floor_item_count == 2 && loaded.floor_items[1].underlying_tile == TILE_CASTLE_GATE_OPEN && loaded.map.tiles[11][29] == TILE_ITEM);
        ASSERT("serialized final boss 94 becomes Malakar with wounds, freeze and preparation intact", e && e->type == 94 && strstr(e->name, "Malakar") && e->hp == old.hp && e->attack_phase == old.attack_phase && e->boss_attack == old.boss_attack && e->boss_aux_x == old.boss_aux_x && e->boss_aux_y == old.boss_aux_y && e->move_timer == old.move_timer && e->frozen_turns == 3);
        ASSERT("exploration and unfinished campaign remain intact", !memcmp(loaded.map.explored, before.explored, sizeof(before.explored)) && !loaded.game_won && !loaded.castle_ending);
        int timer = e->move_timer;
        castle_tick(&loaded);
        ASSERT("a restored frozen final attack remains paused", e->move_timer == timer && e->frozen_turns == 2);
        clear_guards(&loaded, NULL);
        ASSERT("migrated occupants can walk to and use the actual retreat stair", castle_walk_to(&loaded, 15, 11));
        action_resolve_player(&loaded, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("migrated retreat reaches Royal Apartments", loaded.level == 5 && loaded.player.x == 45 && loaded.player.y == 55);
        ASSERT("the remodeled cached finale survives another save round trip", save_game(&loaded, FINALE_SLOT) && load_game(&game, FINALE_SLOT) && game.castle_cache[5].map.room_count == 8);
    }
    Enemy *e = setup();
    e->active = 0;
    e->hp = 0;
    game.game_won = 1;
    game.crown_of_unbound = 1;
    game.score = 12345;
    ASSERT("historical victories load completed without replaying rewards or the new ending", save_game(&game, FINALE_SLOT) && legacy_version() && load_game(&loaded, FINALE_SLOT) && loaded.game_won && loaded.crown_of_unbound && loaded.castle_ending == 3 && loaded.score == 12345 && !finale_pending(&loaded));
}

static void test_phases(void) {
    Enemy *e = setup();
    ASSERT("Malakar returns fully restored, distinct from his laboratory encounter", e && e->hp == 950 && e->max_hp == 950 && e->type != ENEMY_MALAKAR);
    castle_tick(&game);
    ASSERT("phase one announces a fixed two-action bursting vial and finite escorts", e->attack_phase == 1 && e->boss_attack == FINAL_BLAST && e->move_timer == 3 && malakar_final_marks(e, 45, 52) && malakar_final_escort_marks(e, 42, 46));
    int mask = e->boss_aux_x;
    ASSERT("final preparation round trips with its footprint and countdown", save_game(&game, FINALE_SLOT) && load_game(&loaded, FINALE_SLOT) && find_boss(&loaded, ENEMY_MALAKAR_FINAL)->boss_aux_x == mask && find_boss(&loaded, ENEMY_MALAKAR_FINAL)->move_timer == 3);
    e->frozen_turns = 2;
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("freeze pauses vial preparation and escort arrival", !e->frozen_turns && e->move_timer == 3 && game.enemy_count == 8);
    action_resolve_player(&game, (Action){ACTION_MOVE, 44, 52});
    int damage = castle_tick(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, 43, 52});
    damage += castle_tick(&game);
    ASSERT("two actual walking actions dodge the vial; the two escorts arrive without reward", !damage && e->move_timer == 2 && game.enemy_count == 10 && !game.enemies[8].experience && !game.enemies[9].experience);
    clear_guards(&game, e);
    castle_tick(&game);
    ASSERT("a resolved vial grants a full recovery action", !e->move_timer);
    castle_tick(&game);
    ASSERT("phase one alternates vial and warned volatile surge without another escort call", e->boss_attack == FINAL_ADVANCE && game.enemy_count == 10);

    e = setup();
    e->revived = 1;
    e->hp = 600;
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("phase two prepares a linked west conduit lane", e->attack_phase == 2 && e->boss_attack == FINAL_WEST_LANE && e->boss_aux_x);
    game.player.x = 37;
    game.player.y = 53;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Interact breaks the conduit and cancels its pending lane and barriers", game.map.tiles[54][37] == TILE_CASTLE_WARD_SPENT && !e->boss_aux_x && !e->boss_aux_y);
    game.player.x = 53;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("both conduits remain optional and usable by every class", game.map.tiles[54][53] == TILE_CASTLE_WARD_SPENT && castle_enemy_damage(&game, e, 100) == 100);

    e = setup();
    e->revived = 1;
    e->hp = 250;
    game.map.tiles[54][37] = TILE_CASTLE_WARD_SPENT;
    game.map.tiles[54][53] = TILE_CASTLE_WARD_SPENT;
    castle_tick(&game);
    ASSERT("phase three announces three actions before a crown rupture", e->attack_phase == 3 && e->boss_attack == FINAL_CROWN && e->move_timer == 4);
    int dodged = 1;
    for (int x = 44; x >= 42; x--) {
        action_resolve_player(&game, (Action){ACTION_MOVE, x, 52});
        dodged &= game.player.x == x && castle_tick(&game) == 0;
    }
    ASSERT("three real steps dodge the crown and earn two recovery actions with spent conduits", dodged && e->move_timer == 2 && e->revive_timer == 1);
    castle_tick(&game);
    ASSERT("first extended recovery action remains available", e->move_timer == 2 && !e->revive_timer);
    castle_tick(&game);
    ASSERT("second extended recovery action completes without another attack", !e->move_timer);
    castle_tick(&game);
    ASSERT("throne fire receives a separate two-action warning", e->boss_attack == FINAL_FIRE && game.castle_fire_phase[5] == 1 && e->move_timer == 3);
    castle_tick(&game);
    ASSERT("the second fire warning is still harmless off the lane", game.castle_fire_phase[5] == 2 && e->move_timer == 1);
    game.player.x = 45;
    game.player.y = 51;
    ASSERT("the warned fire resolves once through the real damage handler", castle_tick(&game) > 0 && game.castle_fire_phase[5] == 3 && e->move_timer == 2);
    e->frozen_turns = 1;
    ASSERT("resolved fire clears even while Malakar is frozen", castle_tick(&game) == 0 && !game.castle_fire_phase[5]);
}

static void finish_with_class(GameState *g, Enemy *e, int kind) {
    clear_guards(g, e);
    g->player.x = kind == 0 ? 44 : 42;
    g->player.y = e->y;
    g->player.last_dx = 1;
    g->player.last_dy = 0;
    g->player.attack = 5000;
    if (kind == 0) {
        action_resolve_player(g, (Action){ACTION_MOVE, e->x, e->y});
    } else if (kind == 1) {
        g->player.player_class = CLASS_ROGUE;
        g->inventory[0] = item_make_bow();
        g->inventory_count = 1;
        game_equip_main_hand(g, 0);
        g->player.arrows = MAX_ARROWS;
        action_resolve_player(g, (Action){ACTION_RANGED_ATTACK, 0, 0});
    } else {
        g->player.player_class = CLASS_MAGE;
        g->player.known_spell_count = 1;
        g->player.known_spells[0] = spell_make_magic_arrow();
        g->player.known_spells[0].damage = 5000;
        g->player.equipped_spell = 0;
        g->player.mp = 100;
        action_resolve_player(g, (Action){ACTION_CAST_SPELL, 0, 0});
    }
}

static void test_campaign(void) {
    for (unsigned seed = 1; seed <= 8; seed++) {
        game_init(&game);
        srand(seed);
        castle_enter(&game, 1);
        for (int level = 1; level <= 6; level++) {
            int placed = 1;
            for (int i = 0; i < game.enemy_count; i++) {
                placed &= map_is_walkable(&game.map, game.enemies[i].x, game.enemies[i].y);
            }
            ASSERT("campaign defenders occupy usable tiles after all encounter placement", placed && game.level == level);
            if (level == 2) {
                Enemy *e = find_boss(&game, ENEMY_MALAKAR);
                clear_guards(&game, e);
                ASSERT("the full castle route reaches the early Malakar fight", castle_walk_to(&game, 44, 14));
                // Apply an actual player attack large enough to cross the escape threshold.
                game.player.attack = 5000;
                action_resolve_player(&game, (Action){ACTION_MOVE, 45, 14});
                ASSERT("the laboratory encounter flees without ending the campaign", !e->active && e->hp > 0 && (game.castle_minibosses & 1) && !game.game_won);
            } else if (level == 4) {
                Enemy *e = find_boss(&game, ENEMY_ROYAL_ARCANIST);
                clear_guards(&game, e);
                ASSERT("the full castle route reaches the chapel boss", castle_walk_to(&game, 44, 49));
                game.player.attack = 5000;
                action_resolve_player(&game, (Action){ACTION_MOVE, 45, 49});
                ASSERT("defeating the Arcanist unlocks the apartments", !e->active && (game.castle_minibosses & 2));
            } else if (level == 6) {
                Enemy *e = find_boss(&game, ENEMY_MALAKAR_FINAL);
                ASSERT("laboratory wounds never carry into the restored final encounter", e && e->active && e->hp == e->max_hp && e->hp == 950);
                clear_guards(&game, e);
                ASSERT("the throne objective is reachable along the western royal passage", castle_walk_to(&game, 35, 51));
                ASSERT("the final level has a usable retreat and a throne objective instead of a false stair", game.map.tiles[11][15] == TILE_STAIRS_DOWN && game.map.tiles[45][45] == TILE_CASTLE_THRONE && game.map.tiles[55][45] != TILE_STAIRS_UP);
                ASSERT("the late retreat lever is reachable", castle_walk_to(&game, 27, 48));
                action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
                ASSERT("the shortcut opens a continuous path to the entrance", game.map.tiles[40][26] == TILE_CASTLE_GATE_OPEN && castle_walk_to(&game, 15, 11));
                action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
                ASSERT("finale retreat uses the actual apartments stair", game.level == 5 && game.player.x == 45 && game.player.y == 55);
                action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
                ASSERT("the cached finale retains its opened shortcut", game.level == 6 && game.map.tiles[40][26] == TILE_CASTLE_GATE_OPEN);
                e = find_boss(&game, ENEMY_MALAKAR_FINAL);
                int score = game.score;
                finish_with_class(&game, e, seed % 3);
                ASSERT("warrior melee, rogue arrows and mage spells can kill final Malakar without another escape", !e->active && e->hp <= 0 && game.game_won && game.crown_of_unbound && game.castle_ending == 1 && game.score >= score + 10000);
                int won_score = game.score;
                castle_record_death(&game, e);
                ASSERT("the crown, victory score and King ending are awarded once", game.score == won_score && finale_pending(&game) && game.dialogue_active && strstr(game.dialogue_text, "King rises"));
                int hp = game.player.hp;
                action_resolve_enemies(&game);
                ASSERT("victory immediately stops all enemy and fire damage", game.player.hp == hp);
                ASSERT("the King's first ending page survives saving and loading", save_game(&game, FINALE_SLOT) && load_game(&loaded, FINALE_SLOT) && finale_pending(&loaded) && loaded.castle_ending == 1 && loaded.dialogue_active);
                ASSERT("movement and repeated confirmation cannot skip an ending page", finale_handle_key(&loaded, SDL_SCANCODE_RIGHT, 0) && finale_handle_key(&loaded, SDL_SCANCODE_RETURN, 1) && loaded.castle_ending == 1);
                finale_handle_key(&loaded, SDL_SCANCODE_RETURN, 0);
                ASSERT("the aftermath page also survives saving and loading", save_game(&loaded, FINALE_SLOT) && load_game(&game, FINALE_SLOT) && game.castle_ending == 2 && strstr(game.dialogue_text, "Crown of the Unbound"));
                finale_handle_key(&game, SDL_SCANCODE_ESCAPE, 0);
                ASSERT("confirmation completes the ending and returns control to the existing victory screen", !finale_pending(&game) && game.castle_ending == 3 && !game.dialogue_active && game.game_won);
                ASSERT("completed endings do not replay after load", save_game(&game, FINALE_SLOT) && load_game(&loaded, FINALE_SLOT) && loaded.castle_ending == 3 && !finale_pending(&loaded));
                break;
            }
            clear_guards(&game, NULL);
            ASSERT("each completed stage has a continuous route to its real onward transition", castle_walk_to(&game, 45, 55));
            action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
            ASSERT("ascending actually travels to the next castle stage", game.level == level + 1);
            action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
            ASSERT("cached retreat reaches the correct original staircase", game.level == level && game.player.x == 45 && game.player.y == 55);
            action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
        }
    }
}

static void test_finale_routes(void) {
    for (unsigned seed = 1; seed <= 8; seed++) {
        game_init(&game);
        srand(seed);
        game.castle_minibosses = 3;
        castle_enter(&game, 6);
        clear_guards(&game, NULL);
        for (int state = 0; state < 4; state++) {
            for (int y = 47; y <= 49; y++) {
                game.map.tiles[y][40] = state & 1 ? TILE_CASTLE_GATE_OPEN : TILE_CASTLE_GATE;
                game.map.tiles[y][50] = state & 2 ? TILE_CASTLE_GATE_OPEN : TILE_CASTLE_GATE;
            }
            int connected = 1;
            for (int i = 0; i < game.map.room_count; i++) {
                int x;
                int y;
                map_room_center(&game.map.rooms[i], &x, &y);
                connected &= castle_walk_to(&game, x, y);
            }
            ASSERT("all eight final spaces and the real retreat remain connected in every arena gate state", connected && castle_walk_to(&game, 15, 11));
        }
        ASSERT("the council lever is reachable with the entrance gate closed", castle_walk_to(&game, 24, 13));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the council lever opens its own gate through the real interaction", game.map.tiles[11][29] == TILE_CASTLE_GATE_OPEN);
        ASSERT("the royal passage lever is accessible from the chamber side", castle_walk_to(&game, 27, 48));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        castle_leave(&game, 0);
        castle_enter(&game, 6);
        ASSERT("full retreat restores the living boss but preserves the permanent final shortcut", find_boss(&game, ENEMY_MALAKAR_FINAL)->hp == 950 && game.map.tiles[40][26] == TILE_CASTLE_GATE_OPEN);
        clear_guards(&game, NULL);
        ASSERT("regrouping cannot introduce the retired southern gate or block retreat", game.map.tiles[21][15] != TILE_CASTLE_GATE_OPEN && castle_walk_to(&game, 27, 48) && castle_walk_to(&game, 15, 11));
    }
}

void test_castle_finale(void) {
    printf("Castle V2 final Malakar and campaign integration tests:\n");
    test_phases();
    test_migration();
    test_finale_routes();
    test_campaign();
    setup();
    game.game_won = 1;
    game.castle_ending = 2;
    game.aldric_party_state = ALDRIC_DEAD;
    game.storm_mage.recruited = 1;
    game.storm_mage.state = STORM_MAGE_DEAD;
    game.garran.recruited = 1;
    game.garran.state = GARRAN_DEAD;
    finale_sync(&game);
    ASSERT("the ending honors fallen companions without claiming they survived", strstr(game.dialogue_text, "Aldric's sacrifice") && strstr(game.dialogue_text, "Soren is remembered") && strstr(game.dialogue_text, "Garran is remembered"));
    game.aldric_party_state = ALDRIC_FOLLOWING;
    game.storm_mage.state = STORM_MAGE_FOLLOWING;
    game.garran.state = GARRAN_FOLLOWING;
    finale_sync(&game);
    ASSERT("the ending acknowledges the surviving party", strstr(game.dialogue_text, "Aldric stands") && strstr(game.dialogue_text, "Soren is free") && strstr(game.dialogue_text, "Garran can return") && strstr(game.dialogue_text, "victory."));
    remove(FINALE_SAVE);
}
