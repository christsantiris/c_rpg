#include "test_utils.h"
#include "game/actions.h"
#include "game/aldric.h"
#include "game/approach.h"
#include "game/eastern_arc.h"
#include "game/mangrove.h"
#include "game/party.h"
#include "game/resurrection.h"
#include "game/storm_mage.h"
#include "game/stormwatch.h"
#include "game/town_life.h"
#include "game/town_road.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define ROUTE_CELLS (MAP_W * MAP_H)
#define MAGE_SLOT 99278

static int route_path(GameState *g, int tx, int ty) {
    int *previous = malloc(sizeof(int) * ROUTE_CELLS);
    int *queue = malloc(sizeof(int) * ROUTE_CELLS);
    if (!previous || !queue) {
        free(previous);
        free(queue);
        return 0;
    }
    for (int i = 0; i < ROUTE_CELLS; i++) {
        previous[i] = -1;
    }
    int start = g->player.y * MAP_W + g->player.x;
    int target = ty * MAP_W + tx;
    int head = 0;
    int tail = 1;
    int afloat = mangrove_aboard(g);
    previous[start] = start;
    queue[0] = start;
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
            int transition = (map_is_travel_tile(tile) && tile != TILE_ROAD_GATE_OPEN) || tile == TILE_SWAMP_ENTRANCE || tile == TILE_SWAMP_EXIT || tile == TILE_COAST_ENTRANCE || tile == TILE_COAST_EXIT || tile == TILE_STORM_ENTRANCE || tile == TILE_STORM_EXIT;
            int occupied = g->storm_mage.state != STORM_MAGE_FOLLOWING && storm_mage_at(g, x, y);
            for (int i = 0; i < g->enemy_count; i++) {
                occupied |= g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y;
            }
            int entrance = tile == TILE_NORTH_SWAMP || tile == TILE_NORTH_CATACOMBS;
            int passable = afloat ? tile == TILE_MANGROVE_WATER && x > 0 && x < MANGROVE_W - 1 && y > 0 && y < MANGROVE_H - 1 :
                (map_is_walkable(&g->map, x, y) || (entrance && next == target)) && (!transition || next == target);
            if (previous[next] >= 0 || occupied || !passable) {
                continue;
            }
            previous[next] = cell;
            queue[tail++] = next;
        }
    }
    int ok = previous[target] >= 0;
    int count = 0;
    if (ok) {
        for (int cell = target; cell != start; cell = previous[cell]) {
            queue[count++] = cell;
        }
        Location location = g->location;
        int level = g->level;
        int route = g->approach_route;
        for (int i = count - 1; i >= 0; i--) {
            int x = queue[i] % MAP_W;
            int y = queue[i] / MAP_W;
            action_resolve_player(g, (Action){ACTION_MOVE, x, y});
            if (g->location != location || g->level != level || g->approach_route != route) {
                ok = i == 0;
                break;
            }
            if (g->player.x != x || g->player.y != y) {
                ok = 0;
                break;
            }
        }
    }
    free(previous);
    free(queue);
    return ok;
}

static void clear_enemies(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
}

static void follow_aldric(GameState *g) {
    g->aldric_party_state = ALDRIC_FOLLOWING;
    g->aldric_recruited = 1;
    g->aldric_hp = ALDRIC_MAX_HP;
    g->aldric_x = g->player.x - 1;
    g->aldric_y = g->player.y;
}

static void recruit(GameState *g) {
    g->stormwatch_defeated = 1;
    g->storm_mage.state = STORM_MAGE_WAITING;
    town_life_generate(&g->map, LOCATION_ALEHOUSE, &g->player.x, &g->player.y);
    g->location = LOCATION_ALEHOUSE;
    g->enemy_count = 0;
    g->player.x = STORM_MAGE_INN_X - 1;
    g->player.y = STORM_MAGE_INN_Y;
    storm_mage_talk(g, STORM_MAGE_INN_X, STORM_MAGE_INN_Y);
    storm_mage_handle_key(g, SDL_SCANCODE_Y, 0);
}

static void storm_journey(GameState *g, unsigned seed) {
    game_init(g);
    srand(seed);
    town_road_enter(g, LOCATION_DAWNMERE, TOWN_SIDE_WEST, 0);
    int x;
    int y;
    town_road_exit(TOWN_SIDE_EAST, &x, &y);
    ASSERT("occupied Dawnmere can be bypassed to reach Stormwatch without attacking the Crown", route_path(g, x, y) && g->location == LOCATION_APPROACH && g->approach_route == 18 && !g->royal_reported_actions && !g->dawnmere_liberated);
    ASSERT("the headland's visible fort doorway is reachable from Dawnmere", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_STORMWATCH && g->level == 1);
    for (int stage = 1; stage <= STORM_DEPTH; stage++) {
        clear_enemies(g);
        ASSERT("every Stormwatch stage has a real retreat route", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && (stage == 1 ? g->location == LOCATION_APPROACH : g->level == stage - 1));
        if (stage == 1) {
            ASSERT("retreating preserves the fort's working reentry", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_STORMWATCH);
        } else {
            clear_enemies(g);
            ASSERT("cached Stormwatch stages retain their actual onward transition", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->level == stage);
        }
        clear_enemies(g);
        if (stage >= 2) {
            ASSERT("the shutter crank is reachable", route_path(g, 31, 22));
            stormwatch_interact(g);
            ASSERT("a lowered shutter preserves retreat", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && g->level == stage - 1);
            clear_enemies(g);
            ASSERT("a lowered shutter preserves onward travel through the cached stage", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->level == stage);
            clear_enemies(g);
        }
        if (stage == STORM_DEPTH) {
            ASSERT("the captive mage is approachable before the boss is defeated", route_path(g, STORM_MAGE_X - 1, STORM_MAGE_Y) && storm_mage_talk(g, STORM_MAGE_X, STORM_MAGE_Y));
            ASSERT("recruitment cannot bypass the Matriarch", !storm_mage_prompt_active(g) && !storm_mage_handle_key(g, SDL_SCANCODE_Y, 0) && g->storm_mage.state == STORM_MAGE_CAPTIVE);
        }
        ASSERT("the mage does not block any onward boundary, including the final departure", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && (stage < STORM_DEPTH ? g->location == LOCATION_STORMWATCH && g->level == stage + 1 : g->location == LOCATION_APPROACH));
    }
}

static void combat(GameState *g) {
    game_init(g);
    recruit(g);
    g->location = LOCATION_DUNGEON;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = TILE_FLOOR;
        }
    }
    g->player.x = 10;
    g->player.y = 10;
    g->storm_mage.x = 9;
    g->storm_mage.y = 10;
    g->enemy_count = 3;
    for (int i = 0; i < 3; i++) {
        g->enemies[i] = (Enemy){.active = 1, .type = ENEMY_SKELETON, .x = 9 + i, .y = 7, .hp = 200, .max_hp = 200, .experience = 0};
        snprintf(g->enemies[i].name, sizeof(g->enemies[i].name), "Guard %d", i);
    }
    g->map.tiles[7][10] = TILE_WALL;
    g->enemies[1].x = 9;
    g->enemies[1].y = 6;
    int hp = g->player.hp;
    follow_aldric(g);
    g->aldric_x = 11;
    g->aldric_y = 10;
    int ally_hp = g->aldric_hp;
    storm_mage_turn(g);
    ASSERT("the level 1 mage casts the existing Fireball and spends his own MP", g->storm_mage.mp == 80 && g->enemies[0].hp == 173 && g->enemies[1].hp == 173);
    ASSERT("Fireball respects walls and never damages party members", g->enemies[2].hp == 200 && g->player.hp == hp && g->aldric_hp == ally_hp);
    g->map.tiles[8][9] = TILE_WALL;
    int mp = g->storm_mage.mp;
    storm_mage_turn(g);
    ASSERT("a blocked firing lane spends no mana", g->storm_mage.mp == mp);
    g->map.tiles[8][9] = TILE_FLOOR;
    g->location = LOCATION_DAWNMERE;
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].boss_aux_x = 0;
    }
    storm_mage_turn(g);
    ASSERT("Soren never starts a fight with Dawnmere's unalerted soldiers", g->storm_mage.mp == mp && !g->royal_reported_actions);
    g->location = LOCATION_DUNGEON;
    g->storm_mage.x = 9;
    g->storm_mage.y = 10;
    g->enemies[0].x = 9;
    g->enemies[0].y = 9;
    g->enemies[0].hp = 200;
    g->enemy_count = 1;
    g->storm_mage.mp = 0;
    storm_mage_turn(g);
    ASSERT("an exhausted mage uses his staff instead of casting without mana", g->enemies[0].hp == 195 && !g->storm_mage.mp);
    player_gain_xp(g, 100);
    ASSERT("Soren grows from level 1 with the player's XP without reducing the player's reward", g->storm_mage.level == 2 && g->storm_mage.experience == 0 && storm_mage_max_hp(g) == 80 && storm_mage_max_mp(g) == 115 && g->storm_mage.hp == 80);
    g->storm_mage.hp = 1;
    g->player.x = 12;
    g->aldric_x = 14;
    g->enemies[0].attack = 999;
    g->enemies[0].hp = 999;
    action_resolve_enemies(g);
    ASSERT("a recruited mage can die and requires resurrection", g->storm_mage.state == STORM_MAGE_DEAD && !g->storm_mage.hp && !storm_mage_at(g, 9, 10));
}

static void resurrection_targets(GameState *g, GameState *loaded) {
    game_init(g);
    recruit(g);
    g->storm_mage.state = STORM_MAGE_DEAD;
    g->storm_mage.hp = 0;
    g->storm_mage.x = -1;
    g->storm_mage.y = -1;
    g->aldric_party_state = ALDRIC_DEAD;
    g->aldric_recruited = 1;
    g->aldric_hp = 0;
    town_road_enter(g, LOCATION_TOWN, TOWN_SIDE_NORTH, 1);
    ASSERT("both fallen companions can reach the fixed resurrection service", route_path(g, SOULSTONE_WARLOCK_X, SOULSTONE_WARLOCK_Y - 1) && resurrection_talk(g));
    g->gold = 600;
    resurrection_handle_key(g, SDL_SCANCODE_1, 0);
    ASSERT("the shrine lists both fallen targets", g->resurrection_prompt == RESURRECTION_PAID_TARGET && strstr(g->dialogue_text, "1 Aldric") && strstr(g->dialogue_text, "2 Soren"));
    resurrection_handle_key(g, SDL_SCANCODE_2, 0);
    ASSERT("the selected resurrection target survives saving and loading", g->resurrection_target == PARTY_SOREN && save_game(g, MAGE_SLOT) && load_game(loaded, MAGE_SLOT) && loaded->resurrection_target == PARTY_SOREN && loaded->resurrection_prompt == RESURRECTION_PAID_CONFIRM);
    resurrection_handle_key(loaded, SDL_SCANCODE_Y, 0);
    ASSERT("payment revives only the selected mage", loaded->gold == 400 && loaded->storm_mage.state == STORM_MAGE_FOLLOWING && loaded->storm_mage.hp == 70 && loaded->aldric_party_state == ALDRIC_DEAD);
    loaded->aldric_party_state = ALDRIC_WAITING;
    loaded->aldric_hp = ALDRIC_MAX_HP;
    resurrection_talk(loaded);
    ASSERT("living companions are never offered as resurrection targets", !strstr(loaded->dialogue_text, "1 Resurrect") && !party_fallen_count(loaded));
    g->player.player_class = CLASS_MAGE;
    g->player.level = 10;
    g->player.known_spell_count = 1;
    g->player.known_spells[0] = spell_make_resurrection();
    g->player.equipped_spell = 0;
    g->player.mp = 200;
    g->player.max_mp = 200;
    g->dialogue_active = 0;
    ASSERT("the ritual circle remains reachable with two fallen companions", route_path(g, SOULSTONE_RITUAL_X, SOULSTONE_RITUAL_Y));
    resurrection_offer_spell(g);
    ASSERT("the Mage ritual also offers target selection", g->resurrection_prompt == RESURRECTION_SPELL_TARGET);
    resurrection_handle_key(g, SDL_SCANCODE_2, 0);
    resurrection_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("the player's ritual resurrects the selected mage at half HP for the normal MP cost", g->storm_mage.state == STORM_MAGE_FOLLOWING && g->storm_mage.hp == 35 && g->aldric_party_state == ALDRIC_DEAD && g->player.mp == 120);
}

static int rewrite_save(int legacy) {
    FILE *file = fopen("saves/savegame_99278.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = malloc(size + 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    size_t count = fread(data, 1, size, file);
    fclose(file);
    data[count] = 0;
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    if (legacy) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 244);
        cJSON_DeleteItemFromObject(root, "storm_mage");
        cJSON_DeleteItemFromObject(root, "resurrection_target");
    } else {
        cJSON_SetNumberValue(cJSON_GetObjectItem(cJSON_GetObjectItem(root, "storm_mage"), "level"), 0);
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99278.json", "wb");
    int ok = file && data && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static void persistence_and_travel(GameState *g, GameState *loaded) {
    game_init(g);
    g->player.x = STORM_GUIDE_OAK_X;
    g->player.y = STORM_GUIDE_OAK_Y;
    g->map.tiles[g->player.y][g->player.x] = TILE_ITEM;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = g->player.x, .y = g->player.y, .underlying_tile = TILE_TOWN_PATH, .item = item_make_ring_of_vigor()};
    g->gold = 777;
    g->player.hp = 37;
    map_mark_explored(&g->map, 5, 12);
    ASSERT("old Oakhaven saves gain the guide without trapping the player or losing dropped equipment", save_game(g, MAGE_SLOT) && rewrite_save(1) && load_game(loaded, MAGE_SLOT) && loaded->map.tiles[STORM_GUIDE_OAK_Y][STORM_GUIDE_OAK_X] == TILE_NPC_STORM_GUIDE && map_is_walkable(&loaded->map, loaded->player.x, loaded->player.y) && loaded->floor_items[0].active && loaded->floor_items[0].y == STORM_GUIDE_OAK_Y + 1 && loaded->gold == 777 && loaded->player.hp == 37 && map_is_explored(&loaded->map, 5, 12));
    town_road_enter(g, LOCATION_DAWNMERE, TOWN_SIDE_WEST, 0);
    int x;
    int y;
    town_road_exit(TOWN_SIDE_EAST, &x, &y);
    g->map.tiles[y][x] = TILE_WALL;
    ASSERT("old Dawnmere road saves acquire the real eastern departure without changing their progress", save_game(g, MAGE_SLOT) && rewrite_save(1) && load_game(loaded, MAGE_SLOT) && loaded->gold == 777 && loaded->player.hp == 37 && route_path(loaded, x, y) && loaded->location == LOCATION_APPROACH && loaded->approach_route == 18);
    game_init(g);
    stormwatch_enter(g, 0);
    g->enemies[0].hp = 11;
    stormwatch_change_level(g, 1);
    g->enemies[0].hp = 7;
    g->stormwatch_defeated = 1;
    int gold = g->gold;
    int score = g->score;
    ASSERT("existing Matriarch victories free the mage while preserving active and cached enemy wounds", save_game(g, MAGE_SLOT) && rewrite_save(1) && load_game(loaded, MAGE_SLOT) && loaded->storm_mage.state == STORM_MAGE_FREED && !loaded->storm_mage.recruited && loaded->enemies[0].hp == 7 && loaded->stormwatch[0].enemies[0].hp == 11 && loaded->gold == gold && loaded->score == score);
    recruit(loaded);
    player_gain_xp(loaded, 144);
    storm_mage_talk(loaded, loaded->storm_mage.x, loaded->storm_mage.y);
    storm_mage_handle_key(loaded, SDL_SCANCODE_Y, 0);
    ASSERT("dismissal preserves the mage's earned level and experience", loaded->storm_mage.state == STORM_MAGE_WAITING && loaded->storm_mage.level == 2 && loaded->storm_mage.experience == 44 && save_game(loaded, MAGE_SLOT) && load_game(g, MAGE_SLOT) && g->storm_mage.state == STORM_MAGE_WAITING);
    recruit(g);
    follow_aldric(g);
    ASSERT("both companions cross the Alehouse's actual doorway together", route_path(g, 20, 22) && g->location == LOCATION_DAWNMERE && g->storm_mage.level == 2 && g->storm_mage.experience == 44 && map_is_walkable(&g->map, g->storm_mage.x, g->storm_mage.y) && !aldric_at(g, g->storm_mage.x, g->storm_mage.y));
    ASSERT("the returning party can enter the Alehouse through its real doorway", route_path(g, 28, 11) && route_path(g, 28, 10) && g->location == LOCATION_ALEHOUSE && route_path(g, STORM_GUIDE_DAWN_X + 1, STORM_GUIDE_DAWN_Y));
    ASSERT("rumor NPCs acknowledge Soren's rescue instead of still calling him a prisoner", storm_mage_talk(g, STORM_GUIDE_DAWN_X, STORM_GUIDE_DAWN_Y) && strstr(g->dialogue_text, "traveling in your party"));
    g->dialogue_active = 0;
    eastern_river_enter(g, 0);
    ASSERT("the two-companion party can reach the river skiff", route_path(g, RIVER_SOUTH_X, RIVER_SOUTH_Y));
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("boarding carries both companions without leaving attackable actors on the bank", mangrove_aboard(g) && g->storm_mage.x == g->player.x && g->storm_mage.y == g->player.y && !storm_mage_at(g, g->player.x, g->player.y));
    ASSERT("the mage's level and party membership survive saving mid-crossing", route_path(g, 30, 20) && save_game(g, MAGE_SLOT) && load_game(loaded, MAGE_SLOT) && mangrove_aboard(loaded) && loaded->storm_mage.level == 2 && loaded->aldric_party_state == ALDRIC_FOLLOWING);
    ASSERT("the loaded skiff reaches Stillbury's shore", route_path(loaded, RIVER_NORTH_X, RIVER_NORTH_Y + 1));
    action_resolve_player(loaded, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("docking places both companions on walkable ground without overlap", !mangrove_aboard(loaded) && map_is_walkable(&loaded->map, loaded->storm_mage.x, loaded->storm_mage.y) && !aldric_at(loaded, loaded->storm_mage.x, loaded->storm_mage.y));
    loaded->defeated_bosses |= 1u << LOCATION_COAST;
    loaded->storm_mage.hp = 43;
    loaded->storm_mage.mp = 21;
    game_enter_island(loaded);
    ASSERT("sea voyages place both companions at the new landing before the next player action", abs(loaded->storm_mage.x - loaded->player.x) + abs(loaded->storm_mage.y - loaded->player.y) <= 3 && abs(loaded->aldric_x - loaded->player.x) + abs(loaded->aldric_y - loaded->player.y) <= 2 && map_is_walkable(&loaded->map, loaded->storm_mage.x, loaded->storm_mage.y) && !aldric_at(loaded, loaded->storm_mage.x, loaded->storm_mage.y));
    game_leave_island(loaded);
    ASSERT("return voyages retain the mage's wounds and mana and place both companions beside the docks", loaded->location == LOCATION_RIVER_CROSSING && loaded->storm_mage.hp == 43 && loaded->storm_mage.mp == 21 && abs(loaded->storm_mage.x - loaded->player.x) + abs(loaded->storm_mage.y - loaded->player.y) <= 3 && map_is_walkable(&loaded->map, loaded->storm_mage.x, loaded->storm_mage.y) && !aldric_at(loaded, loaded->storm_mage.x, loaded->storm_mage.y));
    ASSERT("a malformed mage level is rejected instead of resetting the companion", save_game(loaded, MAGE_SLOT) && rewrite_save(0) && !load_game(g, MAGE_SLOT));
}

void test_storm_mage(void) {
    printf("Stormwatch rescue and Mage companion:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("Stormwatch companion fixtures allocate and the isolated slot is unused", g && loaded && !save_exists(MAGE_SLOT));
    if (!g || !loaded || save_exists(MAGE_SLOT)) {
        free(g);
        free(loaded);
        return;
    }
    game_init(g);
    g->player.x = STORM_GUIDE_OAK_X + 1;
    g->player.y = STORM_GUIDE_OAK_Y;
    ASSERT("Oakhaven's new NPC introduces the mage without occupying a quest slot", storm_mage_talk(g, STORM_GUIDE_OAK_X, STORM_GUIDE_OAK_Y) && strstr(g->dialogue_text, "Stormwatch") && !game_active_quest_count(g));
    town_life_generate(&g->map, LOCATION_ALEHOUSE, &g->player.x, &g->player.y);
    g->location = LOCATION_ALEHOUSE;
    g->player.x = STORM_GUIDE_DAWN_X + 1;
    g->player.y = STORM_GUIDE_DAWN_Y;
    ASSERT("Dawnmere's new NPC independently explains the rescue", storm_mage_talk(g, STORM_GUIDE_DAWN_X, STORM_GUIDE_DAWN_Y) && strstr(g->dialogue_text, "Tempest Matriarch") && !game_active_quest_count(g));
    const unsigned seeds[] = {19, 107, 1301};
    for (int i = 0; i < 3; i++) {
        storm_journey(g, seeds[i]);
    }
    stormwatch_enter(g, 0);
    for (int level = 1; level < STORM_DEPTH; level++) {
        stormwatch_change_level(g, 1);
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (!e->is_boss) {
            e->active = 0;
        } else {
            e->hp = 1;
            g->player.attack = 999;
            g->player.x = e->x - 1;
            g->player.y = e->y;
            action_resolve_player(g, (Action){ACTION_MOVE, e->x, e->y});
        }
    }
    ASSERT("a real boss kill frees Soren without either rumor conversation or quest acceptance", g->stormwatch_defeated && g->storm_mage.state == STORM_MAGE_FREED && !g->storm_mage.recruited);
    ASSERT("the freed mage can be approached and offers recruitment", route_path(g, STORM_MAGE_X - 1, STORM_MAGE_Y) && storm_mage_talk(g, STORM_MAGE_X, STORM_MAGE_Y) && storm_mage_prompt_active(g));
    ASSERT("the unanswered recruitment offer survives saving", save_game(g, MAGE_SLOT) && load_game(loaded, MAGE_SLOT) && storm_mage_prompt_active(loaded));
    storm_mage_handle_key(loaded, SDL_SCANCODE_Y, 1);
    ASSERT("holding Y cannot recruit automatically", loaded->storm_mage.state == STORM_MAGE_FREED);
    storm_mage_handle_key(loaded, SDL_SCANCODE_N, 0);
    ASSERT("declining sends Soren to Dawnmere's Alehouse for later recruitment", loaded->storm_mage.state == STORM_MAGE_WAITING && !loaded->storm_mage.recruited);
    recruit(loaded);
    ASSERT("recruitment grants a level 1 Mage with his own health and mana", loaded->storm_mage.state == STORM_MAGE_FOLLOWING && loaded->storm_mage.level == 1 && loaded->storm_mage.hp == 70 && loaded->storm_mage.mp == 100 && loaded->storm_mage.recruited);
    follow_aldric(loaded);
    for (int step = 0; step < 6; step++) {
        action_resolve_player(loaded, (Action){ACTION_MOVE, loaded->player.x - 1, loaded->player.y});
    }
    ASSERT("Aldric and Soren can follow together without occupying the same tile", loaded->aldric_party_state == ALDRIC_FOLLOWING && loaded->storm_mage.state == STORM_MAGE_FOLLOWING && !aldric_at(loaded, loaded->storm_mage.x, loaded->storm_mage.y));
    loaded->storm_mage.hp = 43;
    loaded->storm_mage.mp = 21;
    ASSERT("both companions and the mage's wounds and mana survive saving", save_game(loaded, MAGE_SLOT) && load_game(g, MAGE_SLOT) && g->storm_mage.hp == 43 && g->storm_mage.mp == 21 && g->aldric_party_state == ALDRIC_FOLLOWING);
    combat(g);
    ASSERT("death, earned level and mana remain persistent", save_game(g, MAGE_SLOT) && load_game(loaded, MAGE_SLOT) && loaded->storm_mage.state == STORM_MAGE_DEAD && loaded->storm_mage.level == 2 && !loaded->storm_mage.hp && loaded->storm_mage.mp == g->storm_mage.mp);
    resurrection_targets(g, loaded);
    persistence_and_travel(g, loaded);
    remove("saves/savegame_99278.json");
    free(g);
    free(loaded);
}

