#include "test_utils.h"
#include "game/actions.h"
#include "game/aldric.h"
#include "game/approach.h"
#include "game/eastern_arc.h"
#include "game/garran.h"
#include "game/giants_crown.h"
#include "game/mangrove.h"
#include "game/party.h"
#include "game/resurrection.h"
#include "game/storm_mage.h"
#include "game/town_road.h"
#include "game/western_arc.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define ROGUE_SLOT 99281
#define ROGUE_CELLS (MAP_W * MAP_H)

static int route_path(GameState *g, int tx, int ty, int travel) {
    int previous[ROGUE_CELLS];
    int queue[ROGUE_CELLS];
    for (int i = 0; i < ROGUE_CELLS; i++) {
        previous[i] = -1;
    }
    int start = g->player.y * MAP_W + g->player.x;
    int target = ty * MAP_W + tx;
    int head = 0;
    int tail = 1;
    queue[0] = start;
    previous[start] = start;
    const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail && previous[target] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + steps[side][0];
            int y = cell / MAP_W + steps[side][1];
            if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
                continue;
            }
            int next = y * MAP_W + x;
            TileType tile = g->map.tiles[y][x];
            int transition = map_is_travel_tile(tile) && tile != TILE_ROAD_GATE_OPEN && tile != TILE_ROAD_GATE_CLOSED;
            int afloat = mangrove_aboard(g);
            int walkable = afloat ? tile == TILE_MANGROVE_WATER && x > 0 && y > 0 && x < MANGROVE_W - 1 && y < MANGROVE_H - 1 : map_is_walkable(&g->map, x, y);
            if (previous[next] >= 0 || !walkable || (transition && next != target)) {
                continue;
            }
            int occupied = g->garran.state == GARRAN_WAITING && garran_at(g, x, y);
            for (int i = 0; i < g->enemy_count; i++) {
                occupied |= g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y;
            }
            if (occupied && next != target) {
                continue;
            }
            previous[next] = cell;
            queue[tail++] = next;
        }
    }
    if (previous[target] < 0 || !travel) {
        return previous[target] >= 0;
    }
    int count = 0;
    for (int cell = target; cell != start; cell = previous[cell]) {
        queue[count++] = cell;
    }
    Location area = g->location;
    int level = g->level;
    int route = g->approach_route;
    for (int i = count - 1; i >= 0; i--) {
        int x = queue[i] % MAP_W;
        int y = queue[i] / MAP_W;
        action_resolve_player(g, (Action){ACTION_MOVE, x, y});
        if (g->location != area || g->level != level || g->approach_route != route) {
            return i == 0;
        }
        if (g->player.x != x || g->player.y != y) {
            return 0;
        }
    }
    return 1;
}

static void camp(GameState *g) {
    game_init(g);
    approach_enter_region(g, APPROACH_SUNSCAR_ROAD);
    g->player.x = CAMP_GARRAN_X;
    g->player.y = CAMP_GARRAN_Y - 1;
}

static void recruit(GameState *g) {
    int x;
    int y;
    ASSERT("Garran can be approached at his current waiting place", garran_position(g, &x, &y) && route_path(g, x, y - 1, 1) && garran_talk(g, x, y));
    garran_handle_key(g, SDL_SCANCODE_2, 0);
    garran_handle_key(g, SDL_SCANCODE_Y, 0);
}

static void companions(GameState *g) {
    g->aldric_party_state = ALDRIC_FOLLOWING;
    g->aldric_recruited = 1;
    g->aldric_hp = ALDRIC_MAX_HP;
    g->aldric_x = -1;
    g->aldric_y = -1;
    g->storm_mage.state = STORM_MAGE_FOLLOWING;
    g->storm_mage.recruited = 1;
    g->storm_mage.x = -1;
    g->storm_mage.y = -1;
    aldric_follow(g, -1, -1);
    storm_mage_follow(g, -1, -1);
    garran_follow(g, -1, -1);
}

static int placed(const GameState *g) {
    return g->garran.state == GARRAN_FOLLOWING && map_is_walkable(&g->map, g->garran.x, g->garran.y) &&
        !aldric_at(g, g->garran.x, g->garran.y) && !storm_mage_at(g, g->garran.x, g->garran.y) &&
        (g->garran.x != g->player.x || g->garran.y != g->player.y);
}

static void offers(GameState *g, GameState *loaded) {
    camp(g);
    ASSERT("Garran is available before either Sunscar or the Jarl is defeated", !g->defeated_bosses && !g->giants_crown_defeated && garran_talk(g, CAMP_GARRAN_X, CAMP_GARRAN_Y));
    ASSERT("his initial menu accepts neither a quest nor a companion automatically", !game_quest_offer_active(g) && !g->garran.recruited && !game_active_quest_count(g));
    garran_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("Y at the menu cannot recruit Garran or accept his quest", !g->garran.recruited && !g->jarl_quest_state);
    garran_handle_key(g, SDL_SCANCODE_2, 0);
    ASSERT("recruitment has its own persistent confirmation", g->garran.prompt == GARRAN_JOIN && save_game(g, ROGUE_SLOT) && load_game(loaded, ROGUE_SLOT) && garran_dialogue(loaded) && loaded->garran.prompt == GARRAN_JOIN);
    garran_handle_key(loaded, SDL_SCANCODE_Y, 1);
    ASSERT("a held key cannot recruit", !loaded->garran.recruited);
    garran_handle_key(loaded, SDL_SCANCODE_N, 0);
    ASSERT("declining leaves Garran at camp for another visit", garran_at(loaded, CAMP_GARRAN_X, CAMP_GARRAN_Y) && !loaded->garran.recruited);
    g->dialogue_active = 0;
    g->tribute_quest_state = 1;
    g->dain_quest_state = 1;
    g->alder_quest_state = 1;
    int gold = g->gold;
    recruit(g);
    ASSERT("recruitment is free and works with all three quest slots occupied", g->garran.recruited && g->garran.state == GARRAN_FOLLOWING && g->gold == gold && game_active_quest_count(g) == 3 && !g->jarl_quest_state && !g->royal_reported_actions);
    ASSERT("the recruited scout leaves no duplicate NPC at camp", g->map.tiles[CAMP_GARRAN_Y][CAMP_GARRAN_X] != TILE_NPC_GARRAN && placed(g));
    garran_talk(g, g->garran.x, g->garran.y);
    garran_handle_key(g, SDL_SCANCODE_1, 0);
    ASSERT("the Jarl quest remains separate and respects the active quest limit", game_quest_offer_active(g) && game_quest_offer_at_limit(g));
    garran_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("a full journal cannot accept the Jarl quest through recruitment dialogue", !g->jarl_quest_state && g->garran.state == GARRAN_FOLLOWING);
    garran_handle_key(g, SDL_SCANCODE_ESCAPE, 0);
    g->tribute_quest_state = 0;
    garran_talk(g, g->garran.x, g->garran.y);
    garran_handle_key(g, SDL_SCANCODE_1, 0);
    garran_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("a following Garran can assign his existing boss quest", g->jarl_quest_state == 1 && game_active_quest_count(g) == 3);
    g->garran.hp = 37;
    garran_talk(g, g->garran.x, g->garran.y);
    garran_handle_key(g, SDL_SCANCODE_2, 0);
    garran_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("dismissal at camp preserves wounds and restores one waiting NPC", g->garran.state == GARRAN_WAITING && g->garran.hp == 37 && garran_at(g, CAMP_GARRAN_X, CAMP_GARRAN_Y));
    recruit(g);
    ASSERT("recruiting again does not heal the Rogue", g->garran.state == GARRAN_FOLLOWING && g->garran.hp == 37);
    game_enter_inn(g);
    g->garran.x = -1;
    g->garran.y = -1;
    garran_follow(g, -1, -1);
    ASSERT("reaching Stillbury together changes his waiting home", g->garran.home == GARRAN_STILLBURY && map_is_walkable(&g->map, GARRAN_INN_X, GARRAN_INN_Y));
    ASSERT("his waiting position in Stillbury can be approached", route_path(g, GARRAN_INN_X, GARRAN_INN_Y - 1, 1));
    garran_follow(g, -1, -1);
    garran_talk(g, g->garran.x, g->garran.y);
    garran_handle_key(g, SDL_SCANCODE_2, 0);
    garran_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("Stillbury's Tavern becomes the persistent recruitment location", g->garran.state == GARRAN_WAITING && g->garran.home == GARRAN_STILLBURY && g->garran.hp == 37 && save_game(g, ROGUE_SLOT) && load_game(loaded, ROGUE_SLOT) && garran_at(loaded, GARRAN_INN_X, GARRAN_INN_Y));
    recruit(loaded);
    ASSERT("Garran rejoins from Stillbury without resetting his wounds", loaded->garran.state == GARRAN_FOLLOWING && loaded->garran.hp == 37);
}

static void arena(GameState *g) {
    camp(g);
    recruit(g);
    g->location = LOCATION_DUNGEON;
    g->level = 1;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = TILE_FLOOR;
        }
    }
    g->player.x = 10;
    g->player.y = 10;
    g->garran.x = 9;
    g->garran.y = 10;
    g->enemy_count = 1;
    g->enemies[0] = (Enemy){.active = 1, .type = ENEMY_SKELETON, .x = 9, .y = 6, .hp = 100, .max_hp = 100, .defense = 2, .attack = 20, .experience = 10};
    strcpy(g->enemies[0].name, "Test foe");
}

static void waiting_with_party(GameState *g) {
    camp(g);
    recruit(g);
    companions(g);
    g->garran.hp = 37;
    g->garran.x = CAMP_GARRAN_X - 1;
    g->garran.y = CAMP_GARRAN_Y - 1;
    g->map.tiles[CAMP_GARRAN_Y][CAMP_GARRAN_X] = TILE_PORTAL;
    garran_talk(g, g->garran.x, g->garran.y);
    garran_handle_key(g, SDL_SCANCODE_2, 0);
    garran_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("a portal at camp cannot hide Garran when asked to wait", g->garran.state == GARRAN_FOLLOWING && g->garran.hp == 37 && g->map.tiles[CAMP_GARRAN_Y][CAMP_GARRAN_X] == TILE_PORTAL);
    g->map.tiles[CAMP_GARRAN_Y][CAMP_GARRAN_X] = TILE_TOWN_FLOOR;
    g->storm_mage.x = CAMP_GARRAN_X;
    g->storm_mage.y = CAMP_GARRAN_Y;
    garran_talk(g, g->garran.x, g->garran.y);
    garran_handle_key(g, SDL_SCANCODE_2, 0);
    garran_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("another companion steps aside when Garran waits at camp", g->garran.state == GARRAN_WAITING && g->garran.hp == 37 && garran_at(g, CAMP_GARRAN_X, CAMP_GARRAN_Y) && !storm_mage_at(g, CAMP_GARRAN_X, CAMP_GARRAN_Y));
    recruit(g);
    game_enter_inn(g);
    g->player.x = GARRAN_INN_X;
    g->player.y = GARRAN_INN_Y - 1;
    g->garran.x = GARRAN_INN_X - 1;
    g->garran.y = GARRAN_INN_Y - 1;
    g->aldric_x = GARRAN_INN_X + 1;
    g->aldric_y = GARRAN_INN_Y - 1;
    g->storm_mage.x = GARRAN_INN_X;
    g->storm_mage.y = GARRAN_INN_Y;
    garran_talk(g, g->garran.x, g->garran.y);
    garran_handle_key(g, SDL_SCANCODE_2, 0);
    garran_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("waiting in Stillbury also moves a companion aside without healing Garran", g->garran.state == GARRAN_WAITING && g->garran.hp == 37 && garran_at(g, GARRAN_INN_X, GARRAN_INN_Y) && !storm_mage_at(g, GARRAN_INN_X, GARRAN_INN_Y));
    recruit(g);
    ASSERT("all three companions resume traveling together", placed(g) && g->garran.hp == 37 && g->aldric_party_state == ALDRIC_FOLLOWING && g->storm_mage.state == STORM_MAGE_FOLLOWING);
}

static void combat(GameState *g) {
    arena(g);
    g->player.arrows = 0;
    g->garran.x = 7;
    ASSERT("a Rogue at the back of a three-companion party can still be addressed", garran_talk(g, g->garran.x, g->garran.y));
    g->dialogue_active = 0;
    g->garran.x = 9;
    garran_turn(g);
    ASSERT("Garran fires a bow at range without borrowing the player's arrows", g->enemies[0].hp == 88 && !g->player.arrows);
    g->enemies[0].y = 9;
    garran_turn(g);
    ASSERT("an adjacent enemy receives a dagger strike instead", g->enemies[0].hp == 80);
    arena(g);
    g->enemies[0].y = 4;
    garran_turn(g);
    ASSERT("the shortbow cannot hit beyond its five-tile range", g->enemies[0].hp == 100);
    arena(g);
    g->map.tiles[8][9] = TILE_WALL;
    garran_turn(g);
    ASSERT("arrows never pass through walls", g->enemies[0].hp == 100);
    arena(g);
    g->aldric_party_state = ALDRIC_FOLLOWING;
    g->aldric_x = 9;
    g->aldric_y = 8;
    garran_turn(g);
    ASSERT("Garran never fires through another companion", g->enemies[0].hp == 100);
    arena(g);
    g->location = LOCATION_DESERT;
    g->level = 2;
    g->enemies[0].type = ENEMY_ROYAL_GUARD;
    g->enemies[0].boss_aux_y = TRIBUTE_SQUAD;
    garran_turn(g);
    ASSERT("Garran leaves the neutral tribute convoy alone", g->enemies[0].hp == 100 && !g->royal_reported_actions);
    g->enemies[0].boss_aux_x = 1;
    garran_turn(g);
    ASSERT("he supports the player after the convoy is challenged", g->enemies[0].hp == 88);
    arena(g);
    g->location = LOCATION_DAWNMERE;
    garran_turn(g);
    ASSERT("he also leaves unalerted Dawnmere soldiers alone", g->enemies[0].hp == 100 && !g->royal_reported_actions);
    arena(g);
    g->enemies[0].hp = 1;
    int xp = g->player.experience;
    garran_turn(g);
    ASSERT("companion kills grant XP to the player", !g->enemies[0].active && g->player.experience == xp + 10);
    arena(g);
    g->enemies[0].type = ENEMY_LAST_JARL;
    g->enemies[0].hp = 1;
    g->jarl_quest_state = 1;
    garran_turn(g);
    ASSERT("Garran's killing blow opens the Jarl's pass and completes the existing quest", !g->enemies[0].active && g->giants_crown_defeated && g->jarl_quest_state == 3);
    arena(g);
    g->enemies[0].y = 9;
    g->garran.hp = 1;
    g->player.x = 12;
    action_resolve_enemies(g);
    ASSERT("enemy turns can kill Garran and remove him from the battlefield", g->garran.state == GARRAN_DEAD && !g->garran.hp && !garran_at(g, 9, 10));
    garran_follow(g, -1, -1);
    ASSERT("following and town visits never revive a dead Rogue", g->garran.state == GARRAN_DEAD && !g->garran.hp);
}

static void resurrection(GameState *g, GameState *loaded) {
    camp(g);
    recruit(g);
    companions(g);
    g->aldric_party_state = ALDRIC_DEAD;
    g->aldric_hp = 0;
    g->storm_mage.state = STORM_MAGE_DEAD;
    g->storm_mage.hp = 0;
    g->garran.state = GARRAN_DEAD;
    g->garran.hp = 0;
    g->garran.x = -1;
    g->garran.y = -1;
    town_road_enter(g, LOCATION_TOWN, TOWN_SIDE_NORTH, 1);
    g->player.x = SOULSTONE_WARLOCK_X;
    g->player.y = SOULSTONE_WARLOCK_Y - 1;
    g->gold = 600;
    resurrection_talk(g);
    resurrection_handle_key(g, SDL_SCANCODE_1, 0);
    ASSERT("the shrine lists all three fallen companions", party_fallen_count(g) == 3 && strstr(g->dialogue_text, "1 Aldric") && strstr(g->dialogue_text, "2 Soren") && strstr(g->dialogue_text, "3 Garran"));
    resurrection_handle_key(g, SDL_SCANCODE_3, 0);
    ASSERT("a Garran resurrection confirmation survives saving", g->resurrection_target == PARTY_GARRAN && save_game(g, ROGUE_SLOT) && load_game(loaded, ROGUE_SLOT) && loaded->resurrection_target == PARTY_GARRAN);
    resurrection_handle_key(loaded, SDL_SCANCODE_Y, 0);
    ASSERT("200 gold resurrects only Garran at full health", loaded->gold == 400 && loaded->garran.state == GARRAN_FOLLOWING && loaded->garran.hp == 90 && loaded->aldric_party_state == ALDRIC_DEAD && loaded->storm_mage.state == STORM_MAGE_DEAD);
    g->player.player_class = CLASS_MAGE;
    g->player.level = 10;
    g->player.max_mp = g->player.mp = 200;
    g->player.known_spell_count = 1;
    g->player.known_spells[0] = spell_make_resurrection();
    g->player.equipped_spell = 0;
    g->player.x = SOULSTONE_RITUAL_X;
    g->player.y = SOULSTONE_RITUAL_Y;
    g->dialogue_active = 0;
    resurrection_offer_spell(g);
    resurrection_handle_key(g, SDL_SCANCODE_3, 0);
    resurrection_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("the Mage ritual revives Garran at half health for 80 MP", g->garran.state == GARRAN_FOLLOWING && g->garran.hp == 45 && g->player.mp == 120 && g->aldric_party_state == ALDRIC_DEAD && g->storm_mage.state == STORM_MAGE_DEAD);
    g->aldric_party_state = ALDRIC_WAITING;
    g->storm_mage.state = STORM_MAGE_WAITING;
    g->player.x = SOULSTONE_WARLOCK_X;
    g->player.y = SOULSTONE_WARLOCK_Y - 1;
    resurrection_talk(g);
    ASSERT("a living Garran is never offered for resurrection", !party_fallen_count(g) && !strstr(g->dialogue_text, "1 Resurrect"));
}

static void journeys(GameState *g, int seed) {
    camp(g);
    srand(seed);
    recruit(g);
    companions(g);
    ASSERT("all companions can reach the real Sunscar entrance", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y, 1) && g->location == LOCATION_DESERT && placed(g));
    for (int stage = 1; stage <= DESERT_DEPTH; stage++) {
        g->enemy_count = 0;
        ASSERT("the arriving party can step inward before retreating", route_path(g, DESERT_MAP_W - 2, g->map.stairs_up_y, 1));
        ASSERT("Garran travels through each desert stage's actual retreat boundary", route_path(g, DESERT_MAP_W - 1, g->map.stairs_up_y, 1) && placed(g));
        if (stage > 1) {
            ASSERT("cached desert landings allow a step inward before returning", route_path(g, 1, g->map.stairs_down_y, 1));
        }
        int x = stage == 1 ? g->map.stairs_down_x : 0;
        ASSERT("the party can reenter the cached desert stage", route_path(g, x, g->map.stairs_down_y, 1) && g->location == LOCATION_DESERT && g->level == stage && placed(g));
        g->enemy_count = 0;
        ASSERT("the party traverses each onward desert boundary", route_path(g, 0, g->map.stairs_down_y, 1) && placed(g));
    }
    ASSERT("Garran returns to the caravan road after all five desert stages", approach_sunscar_road(g));
    ASSERT("the party follows the northward desert road into Giant's Peaks", route_path(g, APPROACH_PEAKS_BRANCH_X, 0, 1) && approach_peaks_road(g) && route_path(g, g->map.stairs_down_x, g->map.stairs_down_y, 1) && g->location == LOCATION_GIANTS_CROWN && placed(g));
    for (int stage = 1; stage <= GIANTS_DEPTH; stage++) {
        g->enemy_count = 0;
        ASSERT("each giant stage retains a usable retreat with three companions", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y, 1) && placed(g));
        ASSERT("the party reenters each giant stage through its real transition", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y, 1) && g->location == LOCATION_GIANTS_CROWN && g->level == stage && placed(g));
        g->enemy_count = 0;
        if (stage == GIANTS_DEPTH) {
            giants_defeat(g);
        }
        ASSERT("Garran follows through every northward giant transition", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y, 1) && placed(g));
    }
    ASSERT("the party crosses Giant's Bridge onto Stillbury's road", approach_bridge_road(g) && route_path(g, APPROACH_W - 1, APPROACH_BRIDGE_EXIT_Y, 1) && g->location == LOCATION_TOWN_ROAD && placed(g) && g->garran.home == GARRAN_STILLBURY);
    int x;
    int y;
    town_road_entrance(TOWN_SIDE_WEST, &x, &y);
    ASSERT("the party enters Stillbury through the real western gate", route_path(g, x, y, 1) && g->location == LOCATION_TOWN2 && g->act4_arrived && placed(g));
    ASSERT("Garran enters Stillbury's Tavern through its actual door", route_path(g, TOWN_INN_DOOR_X, TOWN_INN_DOOR_Y, 1) && g->location == LOCATION_INN && placed(g));
}

static void voyages(GameState *g, GameState *loaded) {
    camp(g);
    recruit(g);
    companions(g);
    eastern_river_enter(g, 0);
    action_resolve_player(g, (Action){ACTION_WAIT, 0, 0});
    ASSERT("the full party reaches the river skiff", route_path(g, RIVER_SOUTH_X, RIVER_SOUTH_Y, 1));
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Garran boards without remaining attackable on the bank", mangrove_aboard(g) && g->garran.x == g->player.x && g->garran.y == g->player.y && !garran_at(g, g->player.x, g->player.y));
    g->garran.hp = 37;
    ASSERT("the Rogue and his wounds persist while aboard", route_path(g, 30, 20, 1) && save_game(g, ROGUE_SLOT) && load_game(loaded, ROGUE_SLOT) && mangrove_aboard(loaded) && loaded->garran.hp == 37);
    ASSERT("the loaded party sails to Stillbury's dock", route_path(loaded, RIVER_NORTH_X, RIVER_NORTH_Y + 1, 1));
    action_resolve_player(loaded, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("docking places all three companions without overlap", !mangrove_aboard(loaded) && placed(loaded) && loaded->garran.hp == 37);
    loaded->defeated_bosses |= 1u << LOCATION_COAST;
    game_enter_island(loaded);
    ASSERT("Garran accompanies a sea voyage to the Ruined Isle", loaded->location == LOCATION_ISLAND && placed(loaded) && loaded->garran.hp == 37);
    game_leave_island(loaded);
    ASSERT("the return voyage preserves his wounds and places him at the docks", loaded->location == LOCATION_RIVER_CROSSING && placed(loaded) && loaded->garran.hp == 37);
    loaded->player.known_spells[0] = spell_make_return_to_town();
    loaded->last_town_visited = LOCATION_TOWN2;
    loaded->player.known_spell_count = 1;
    loaded->player.equipped_spell = 0;
    loaded->player.mp = loaded->player.max_mp = 200;
    action_resolve_player(loaded, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("the Rogue follows the player's one-way town portal", loaded->location == LOCATION_TOWN2 && placed(loaded));
}

static int change_save(int mode) {
    FILE *file = fopen("saves/savegame_99281.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = calloc((size_t)size + 1, 1);
    int ok = data && fread(data, 1, (size_t)size, file) == (size_t)size;
    fclose(file);
    cJSON *root = ok ? cJSON_Parse(data) : NULL;
    free(data);
    if (!root) {
        return 0;
    }
    cJSON *rogue = cJSON_GetObjectItem(root, "garran");
    if (mode == 0) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 246);
        cJSON_DeleteItemFromObject(root, "garran");
    } else if (mode == 1) {
        cJSON_DeleteItemFromObject(rogue, "hp");
    } else {
        cJSON_SetNumberValue(cJSON_GetObjectItem(rogue, "state"), 99);
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99281.json", "wb");
    ok = data && file && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static void saves(GameState *g, GameState *loaded) {
    camp(g);
    recruit(g);
    g->garran.hp = 41;
    ASSERT("membership, wounds and camp home survive saving", save_game(g, ROGUE_SLOT) && load_game(loaded, ROGUE_SLOT) && loaded->garran.state == GARRAN_FOLLOWING && loaded->garran.recruited && loaded->garran.hp == 41 && loaded->garran.home == GARRAN_CAMP);
    g->garran.state = GARRAN_DEAD;
    g->garran.hp = 0;
    g->garran.x = -1;
    g->garran.y = -1;
    ASSERT("Garran's death survives saving without creating a camp duplicate", save_game(g, ROGUE_SLOT) && load_game(loaded, ROGUE_SLOT) && loaded->garran.state == GARRAN_DEAD && !garran_at(loaded, CAMP_GARRAN_X, CAMP_GARRAN_Y));
    ASSERT("new saves reject a missing companion field", save_game(g, ROGUE_SLOT) && change_save(1) && !load_game(loaded, ROGUE_SLOT));
    ASSERT("new saves reject an invalid companion state", save_game(g, ROGUE_SLOT) && change_save(2) && !load_game(loaded, ROGUE_SLOT));
    game_init(g);
    game_enter_desert(g);
    g->enemies[0].hp = 11;
    game_descend(g);
    g->enemies[0].hp = 7;
    int gold = g->gold;
    int count = g->enemy_count;
    int x = g->player.x;
    int y = g->player.y;
    ASSERT("older saves gain an unrecruited scout while preserving active and cached wounds", save_game(g, ROGUE_SLOT) && change_save(0) && load_game(loaded, ROGUE_SLOT) && !loaded->garran.recruited && loaded->garran.hp == 90 && loaded->enemies[0].hp == 7 && loaded->desert_cache[0].enemies[0].hp == 11 && loaded->gold == gold && loaded->enemy_count == count && loaded->player.x == x && loaded->player.y == y);
    camp(g);
    garran_talk(g, CAMP_GARRAN_X, CAMP_GARRAN_Y);
    garran_handle_key(g, SDL_SCANCODE_1, 0);
    ASSERT("an old pending Jarl offer remains a quest offer after migration", save_game(g, ROGUE_SLOT) && change_save(0) && load_game(loaded, ROGUE_SLOT) && game_quest_offer_active(loaded) && !loaded->garran.recruited);
}

void test_garran(void) {
    printf("Caravan Rogue companion:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("companion fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    offers(g, loaded);
    waiting_with_party(g);
    combat(g);
    resurrection(g, loaded);
    const int seeds[] = {19, 107, 1301};
    for (int i = 0; i < 3; i++) {
        journeys(g, seeds[i]);
    }
    voyages(g, loaded);
    saves(g, loaded);
    remove("saves/savegame_99281.json");
    free(g);
    free(loaded);
}

