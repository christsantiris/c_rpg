#include "approach_checks.h"
#include "test_utils.h"
#include "shortcut_checks.h"
#include "game/game.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

#define SHORTCUT_TEST_SLOT 99125
static GameState game;
static GameState loaded;
static const Location regions[3] = {LOCATION_FOREST, LOCATION_SWAMP, LOCATION_MOUNTAINS};
static const EnemyType bosses[3] = {ENEMY_FOREST_NECROMANCER, ENEMY_SWAMP_DEMON, ENEMY_MOUNTAIN_GOBLIN_KING};
static const TileType markers[3] = {TILE_FOREST_SHORTCUT, TILE_SWAMP_SHORTCUT, TILE_MOUNTAIN_SHORTCUT};
static const TileType walls[3] = {TILE_FOREST_WALL, TILE_SWAMP_WALL, TILE_MOUNTAIN_WALL};
static const TileType floors[3] = {TILE_FOREST_FLOOR, TILE_SWAMP_FLOOR, TILE_MOUNTAIN_FORTRESS_FLOOR};

static int find_shortcut(const Map *map, TileType marker, int *x, int *y) {
    int count = 0;
    for (int row = 0; row < MAP_H; row++) {
        for (int column = 0; column < MAP_W; column++) {
            if (map->tiles[row][column] == marker) {
                *x = column;
                *y = row;
                count++;
            }
        }
    }
    return count;
}

static void shortcut_victory(int region, int attack) {
    game.player.player_class = CLASS_MAGE;
    game_init(&game);
    srand(631 + region);
    game.location = regions[region];
    game.level = 4;
    if (region == 0) {
        map_generate_forest(&game.map, FOREST_BOSS_LEVEL);
    } else if (region == 1) {
        map_generate_swamp(&game.map, SWAMP_BOSS_LEVEL);
    } else {
        map_generate_mountains(&game.map, MOUNTAIN_BOSS_LEVEL);
    }
    // Fight far from the original boss room, as when the Demon pursues the player.
    for (int y = 24; y <= 32; y++) {
        for (int x = 26; x <= 36; x++) {
            game.map.tiles[y][x] = floors[region];
        }
    }
    game.player.x = attack == 0 ? 29 : 28;
    game.player.y = 28;
    game.player.last_dx = 1;
    game.player.last_dy = 0;
    game.player.mp = 100;
    game.enemy_count = 2;
    game.enemies[0] = (Enemy){.type = bosses[region], .active = 1, .is_boss = 1, .x = 30, .y = 28, .hp = 1, .max_hp = 100};
    game.enemies[1] = (Enemy){.type = ENEMY_GOBLIN, .active = 1, .x = 35, .y = 30, .hp = 100, .max_hp = 100};
    game.inventory[0] = attack == 2 ? item_make_bow() : item_make_staff();
    game.player.arrows = MAX_ARROWS;
    game.inventory_count = 1;
    game.equipped_main_hand = 0;
    game.player.known_spells[0] = spell_make_magic_arrow();
    game.player.known_spell_count = 1;
    game.player.equipped_spell = 0;
    Action action = attack == 0 ? (Action){ACTION_MOVE, 30, 28} :
        (Action){attack == 1 ? ACTION_CAST_SPELL : ACTION_RANGED_ATTACK, 0, 0};
    action_resolve_player(&game, action);
    ASSERT("all three regions prompt for melee, spell, and bow boss victories", !game.enemies[0].active && game_shortcut_prompt_active(&game) && strstr(game.dialogue_text, "recessed into the surrounding terrain"));
    int x = -1;
    int y = -1;
    ASSERT("shortcut is recessed into reachable terrain despite surviving enemies", find_shortcut(&game.map, markers[region], &x, &y) == 1 && shortcut_exit_is_recessed(&game.map, x, y, walls[region]) && game.enemies[1].active);
    ASSERT("movement input cannot dismiss the discovery prompt", game_handle_shortcut_prompt_key(&game, SDL_SCANCODE_LEFT, 0) && game_shortcut_prompt_active(&game));
    ASSERT("held Enter cannot dismiss a newly discovered shortcut", game_handle_shortcut_prompt_key(&game, SDL_SCANCODE_RETURN, 1) && game_shortcut_prompt_active(&game));
    ASSERT("discovery prompt survives save/load in every region", save_game(&game, SHORTCUT_TEST_SLOT) && load_game(&loaded, SHORTCUT_TEST_SLOT) && game_shortcut_prompt_active(&loaded));
    ASSERT("fresh Enter dismisses the prompt without advancing a turn", game_handle_shortcut_prompt_key(&loaded, SDL_SCANCODE_RETURN, 0) && !game_shortcut_prompt_active(&loaded) && loaded.player.hp == game.player.hp && loaded.player.mp == game.player.mp);
    ASSERT("normal gameplay keys resume after acknowledgment", !game_handle_shortcut_prompt_key(&loaded, SDL_SCANCODE_LEFT, 0));
    remove("saves/savegame_99125.json");
    if (x < 0) {
        return;
    }
    game.dialogue_active = 0;
    int px = game.player.x;
    int py = game.player.y;
    int reward = -1;
    const char *reward_name = region == 0 ? "Necromancer's Cloak" :
        (region == 1 ? "Demonic Sword" : "Goblin King's Shield");
    for (int i = 0; i < game.floor_item_count; i++) {
        if (game.floor_items[i].active && strcmp(game.floor_items[i].item.name, reward_name) == 0) {
            reward = i;
        }
    }
    ASSERT("regional boss leaves its unique reward on the floor", reward >= 0);
    if (reward < 0) {
        return;
    }
    action_resolve_player(&game, (Action){ACTION_MOVE, x, y});
    ASSERT("stepping into the shortcut keeps the player and reward on the boss floor", game.location == regions[region] && game.player.x == px && game.player.y == py && game.floor_items[reward].active && game_shortcut_prompt_active(&game));
    ASSERT("shortcut confirmation warns that remaining loot will be lost", strstr(game.dialogue_text, "Items left on this floor will be lost"));
    ASSERT("held Enter cannot confirm travel", game_handle_shortcut_prompt_key(&game, SDL_SCANCODE_RETURN, 1) && game.location == regions[region] && game_shortcut_prompt_active(&game));
    ASSERT("held Escape and movement cannot dismiss travel confirmation", game_handle_shortcut_prompt_key(&game, SDL_SCANCODE_ESCAPE, 1) && game_handle_shortcut_prompt_key(&game, SDL_SCANCODE_LEFT, 0) && game_shortcut_prompt_active(&game));
    ASSERT("pending travel and unclaimed reward survive save/load", save_game(&game, SHORTCUT_TEST_SLOT) && load_game(&loaded, SHORTCUT_TEST_SLOT) && loaded.location == regions[region] && game_shortcut_prompt_active(&loaded) && loaded.floor_items[reward].active);
    remove("saves/savegame_99125.json");
    ASSERT("Escape cancels travel without moving or losing the reward", game_handle_shortcut_prompt_key(&loaded, SDL_SCANCODE_ESCAPE, 0) && !game_shortcut_prompt_active(&loaded) && loaded.location == regions[region] && loaded.player.x == px && loaded.player.y == py && loaded.floor_items[reward].active);
    for (int i = 0; i < loaded.floor_item_count; i++) {
        if (loaded.floor_items[i].active) {
            action_resolve_player(&loaded, (Action){ACTION_MOVE, loaded.floor_items[i].x, loaded.floor_items[i].y});
            action_resolve_player(&loaded, (Action){ACTION_PICK_UP, 0, 0});
        }
    }
    int collected = 0;
    for (int i = 0; i < loaded.inventory_count; i++) {
        collected |= strcmp(loaded.inventory[i].name, reward_name) == 0;
    }
    ASSERT("canceling lets the player collect the original boss reward", collected && !loaded.floor_items[reward].active);
    action_resolve_player(&loaded, (Action){ACTION_MOVE, x, y});
    ASSERT("a cleared floor still asks before travel without a loot warning", game_shortcut_prompt_active(&loaded) && !strstr(loaded.dialogue_text, "will be lost"));
    Location destination = region == 0 ? LOCATION_FOREST_ROAD : (region == 1 ? LOCATION_SWAMP_ROAD : LOCATION_HIGH_PASS);
    int hp = loaded.player.hp;
    ASSERT("fresh keypad Enter enters the safe path without spending a combat turn", game_handle_shortcut_prompt_key(&loaded, SDL_SCANCODE_KP_ENTER, 0) && loaded.location == destination && !game_shortcut_prompt_active(&loaded) && loaded.player.hp == hp && loaded.enemy_count == 0);
    collected = 0;
    for (int i = 0; i < loaded.inventory_count; i++) {
        collected |= strcmp(loaded.inventory[i].name, reward_name) == 0;
    }
    ASSERT("confirmed travel preserves the collected boss reward", collected);
    ASSERT("saving on the path preserves the boss chamber for a return trip", save_game(&loaded, SHORTCUT_TEST_SLOT) && load_game(&loaded, SHORTCUT_TEST_SLOT) && loaded.location == destination);
    remove("saves/savegame_99125.json");
    int branch_x = region == 0 ? FOREST_ROAD_FORK_X : (region == 1 ? SWAMP_ROAD_BOSS_X : HIGH_PASS_BOSS_X);
    int branch_y = region == 0 ? FOREST_ROAD_BOSS_Y : (region == 1 ? SWAMP_ROAD_FORK_Y : HIGH_PASS_FORK_Y);
    action_resolve_player(&loaded, (Action){ACTION_MOVE, branch_x, branch_y});
    ASSERT("return branch restores the boss chamber and surviving enemies", loaded.location == regions[region] && loaded.level == 4 && !loaded.enemies[0].active && loaded.enemies[1].active && loaded.enemies[1].hp == game.enemies[1].hp);
    ASSERT("return arrival uses the original nearby shortcut", loaded.player.x == x && loaded.player.y == y && loaded.map.tiles[y][x] == markers[region] && loaded.player.hp == hp && !game_shortcut_prompt_active(&loaded));
    loaded.enemies[1].x = x;
    loaded.enemies[1].y = y;
    game_request_shortcut(&loaded);
    game_handle_shortcut_prompt_key(&loaded, SDL_SCANCODE_RETURN, 0);
    action_resolve_player(&loaded, (Action){ACTION_MOVE, branch_x, branch_y});
    ASSERT("a surviving enemy on the shortcut cannot overlap the returning player", loaded.location == regions[region] && loaded.enemies[1].active && (loaded.player.x != x || loaded.player.y != y) && map_is_walkable(&loaded.map, loaded.player.x, loaded.player.y) && loaded.player.hp == hp);
    loaded.enemies[1].x = 35;
    loaded.enemies[1].y = 30;
    loaded.portal_active = 1;
    loaded.portal_location = regions[region];
    loaded.portal_level = 4;
    loaded.portal_x = x;
    loaded.portal_y = y;
    loaded.portal_origin_tile = markers[region];
    loaded.map.tiles[y][x] = TILE_PORTAL;
    game_request_shortcut(&loaded);
    game_handle_shortcut_prompt_key(&loaded, SDL_SCANCODE_RETURN, 0);
    action_resolve_player(&loaded, (Action){ACTION_MOVE, branch_x, branch_y});
    ASSERT("a portal covering the boss shortcut preserves its landing and remains usable", loaded.location == regions[region] && loaded.player.x == x && loaded.player.y == y && loaded.map.tiles[y][x] == TILE_PORTAL && loaded.portal_active && loaded.portal_origin_tile == markers[region]);
    game_handle_shortcut_prompt_key(&game, SDL_SCANCODE_ESCAPE, 0);
    int index = game.floor_item_count++;
    game.floor_items[index] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = markers[region], .item = item_make_health_potion()};
    game.map.tiles[y][x] = TILE_ITEM;
    game_refresh_quest_encounters(&game);
    ASSERT("refresh preserves loot covering a nearby shortcut", game.map.tiles[y][x] == TILE_ITEM && game.floor_items[index].underlying_tile == markers[region]);
    game.player.x = x;
    game.player.y = y;
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("picking up loot restores the same nearby shortcut", game.map.tiles[y][x] == markers[region]);
    game.portal_active = 1;
    game.portal_location = regions[region];
    game.portal_level = 4;
    game.portal_x = x;
    game.portal_y = y;
    game.portal_origin_tile = markers[region];
    game.map.tiles[y][x] = TILE_PORTAL;
    game_refresh_quest_encounters(&game);
    ASSERT("refresh preserves a return portal covering the shortcut", game.map.tiles[y][x] == TILE_PORTAL && game.portal_origin_tile == markers[region]);
    game.map.tiles[y][x] = floors[region];
    int old_x;
    int old_y;
    map_room_center(&game.map.rooms[game.map.room_count - 1], &old_x, &old_y);
    old_x += 3;
    old_y += region == 0 ? 3 : 2;
    game.portal_x = old_x;
    game.portal_y = old_y;
    game.map.tiles[old_y][old_x] = TILE_PORTAL;
    game_refresh_quest_encounters(&game);
    ASSERT("moving an old shortcut preserves its portal without restoring a distant entrance", game.map.tiles[old_y][old_x] == TILE_PORTAL && game.portal_origin_tile == floors[region] && find_shortcut(&game.map, markers[region], &x, &y) == 1 && shortcut_exit_is_recessed(&game.map, x, y, walls[region]));
}

static void shortcut_migration(int region) {
    shortcut_victory(region, 1);
    game.portal_active = 0;
    int x = -1;
    int y = -1;
    game.map.tiles[game.portal_y][game.portal_x] = floors[region];
    find_shortcut(&game.map, markers[region], &x, &y);
    if (x >= 0) {
        game.map.tiles[y][x] = floors[region];
    }
    int old_x;
    int old_y;
    map_room_center(&game.map.rooms[game.map.room_count - 1], &old_x, &old_y);
    old_x += 3;
    old_y += region == 0 ? 3 : 2;
    game.map.tiles[old_y][old_x] = markers[region];
    LevelCache *cache = region == 0 ? &game.forest_cache[3] :
        (region == 1 ? &game.swamp_cache[3] : &game.mountain_cache[3]);
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = game.enemy_count;
    memcpy(cache->enemies, game.enemies, sizeof(cache->enemies));
    game.location = LOCATION_TOWN3;
    game.floor_item_count = 0;
    game.enemy_count = 0;
    game.dialogue_active = 0;
    map_generate_town3(&game.map, &game.player.x, &game.player.y);
    int ok = save_game(&game, SHORTCUT_TEST_SLOT) && load_game(&loaded, SHORTCUT_TEST_SLOT);
    LevelCache *migrated = region == 0 ? &loaded.forest_cache[3] :
        (region == 1 ? &loaded.swamp_cache[3] : &loaded.mountain_cache[3]);
    ASSERT("loading a town save moves cached floor shortcuts into terrain passages", ok && find_shortcut(&migrated->map, markers[region], &x, &y) == 1 && shortcut_exit_is_recessed(&migrated->map, x, y, walls[region]) && migrated->map.tiles[old_y][old_x] == floors[region]);
    ASSERT("shortcut migration preserves character and enemy progress", ok && loaded.player.hp == game.player.hp && loaded.gold == game.gold && loaded.score == game.score && loaded.defeated_bosses == game.defeated_bosses && migrated->enemies[0].hp == game.enemies[0].hp && migrated->enemies[1].active);
    remove("saves/savegame_99125.json");
}

static int single_tile_shortcut(const GameState *g) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            int path;
            if (g->location == LOCATION_FOREST_ROAD) {
                path = (y == FOREST_ROAD_Y && x < FOREST_ROAD_W) ||
                    (x == FOREST_ROAD_FORK_X && y >= FOREST_ROAD_BOSS_Y && y < FOREST_ROAD_Y);
            } else if (g->location == LOCATION_SWAMP_ROAD) {
                path = (x == SWAMP_ROAD_X && y < SWAMP_ROAD_H) ||
                    (y == SWAMP_ROAD_FORK_Y && x > SWAMP_ROAD_X && x <= SWAMP_ROAD_BOSS_X);
            } else {
                path = (x == HIGH_PASS_X && y < HIGH_PASS_H) ||
                    (y == HIGH_PASS_FORK_Y && x > HIGH_PASS_X && x <= HIGH_PASS_BOSS_X);
            }
            if (!!map_is_walkable(&g->map, x, y) != path) {
                return 0;
            }
        }
    }
    return 1;
}

static void town_shortcut_crossing(int road, int reverse) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.defeated_bosses = (1 << LOCATION_FOREST) | (1 << LOCATION_SWAMP) | (1 << LOCATION_MOUNTAINS);
    Location destination;
    int dx = 0;
    int dy = reverse ? 1 : -1;
    if (road == 0) {
        game.location = reverse ? LOCATION_TOWN2 : LOCATION_TOWN;
        destination = reverse ? LOCATION_TOWN : LOCATION_TOWN2;
        game_enter_forest_road(&game);
        dx = reverse ? 1 : -1;
        dy = 0;
    } else if (road == 1) {
        game.location = reverse ? LOCATION_TOWN3 : LOCATION_TOWN2;
        destination = reverse ? LOCATION_TOWN2 : LOCATION_TOWN3;
        game_enter_swamp_road(&game);
    } else {
        destination = reverse ? LOCATION_TOWN : LOCATION_TOWN4;
        game_enter_high_pass(&game, !reverse);
    }
    ASSERT("safe town shortcuts retain narrow paths with one boss branch", single_tile_shortcut(&game));
    Location shortcut = game.location;
    int steps = 0;
    while (game.location == shortcut && steps < MAP_W + MAP_H) {
        action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x + dx, game.player.y + dy});
        steps++;
    }
    ASSERT("one-tile shortcuts connect the correct towns in both directions", game.location == destination && game.enemy_count == 0);
}

static void shortcut_layouts(void) {
    static Map before;
    for (int region = 0; region < 3; region++) {
        int recessed = 1;
        int stable = 1;
        for (int seed = 0; seed < 64; seed++) {
            memset(&game, 0, sizeof(game));
            game_init(&game);
            srand(527 + seed);
            game.location = regions[region];
            game.level = 4;
            game.defeated_bosses = 1 << regions[region];
            if (region == 0) {
                map_generate_forest(&game.map, FOREST_BOSS_LEVEL);
                game_reveal_forest_shortcut(&game);
            } else if (region == 1) {
                map_generate_swamp(&game.map, SWAMP_BOSS_LEVEL);
                game_reveal_swamp_shortcut(&game);
            } else {
                map_generate_mountains(&game.map, MOUNTAIN_BOSS_LEVEL);
                game_reveal_mountain_shortcut(&game);
            }
            int x = -1;
            int y = -1;
            recessed &= find_shortcut(&game.map, markers[region], &x, &y) == 1 && shortcut_exit_is_recessed(&game.map, x, y, walls[region]);
            recessed &= region != 1 || (x < SWAMP_MAP_W - 1 && y < SWAMP_MAP_H - 1);
            before = game.map;
            if (region == 0) {
                game_reveal_forest_shortcut(&game);
            } else if (region == 1) {
                game_reveal_swamp_shortcut(&game);
            } else {
                game_reveal_mountain_shortcut(&game);
            }
            stable &= memcmp(before.tiles, game.map.tiles, sizeof(before.tiles)) == 0;
        }
        ASSERT("64 generated layouts place the shortcut down a three-tile passage within the rendered terrain", recessed);
        ASSERT("refreshing a recessed shortcut never moves or lengthens its passage", stable);
    }
}

static void town_route_choice(int route) {
    const Location towns[6] = {LOCATION_TOWN, LOCATION_TOWN2, LOCATION_TOWN2, LOCATION_TOWN3, LOCATION_TOWN, LOCATION_TOWN4};
    const Location areas[6] = {LOCATION_FOREST, LOCATION_FOREST, LOCATION_SWAMP, LOCATION_SWAMP, LOCATION_MOUNTAINS, LOCATION_MOUNTAINS};
    const Location roads[6] = {LOCATION_FOREST_ROAD, LOCATION_FOREST_ROAD, LOCATION_SWAMP_ROAD, LOCATION_SWAMP_ROAD, LOCATION_HIGH_PASS, LOCATION_HIGH_PASS};
    const char *names[6] = {"Stillbury", "OakHaven", "Rosemoor", "Stillbury", "Ridgeshire", "OakHaven"};
    const int gate_x[6] = {0, TOWN_W - 1, 20, 20, 20, 20};
    const int gate_y[6] = {12, 12, 0, TOWN_H - 1, 0, TOWN_H - 1};
    const int old_x[6] = {0, TOWN_W - 1, TOWN3_ROAD_X, ROSEMOOR_SWAMP_ROAD_X, TOWN4_ROAD_X, RIDGESHIRE_MOUNTAIN_ROAD_X};
    const int old_y[6] = {TOWN_ROAD_EXIT_Y, TOWN_ROAD_EXIT_Y, 0, TOWN_H - 1, 0, TOWN_H - 1};
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game.location = towns[route];
    if (game.location == LOCATION_TOWN) {
        map_generate_town(&game.map, &game.player.x, &game.player.y);
    } else if (game.location == LOCATION_TOWN2) {
        map_generate_town2(&game.map, &game.player.x, &game.player.y);
    } else if (game.location == LOCATION_TOWN3) {
        map_generate_town3(&game.map, &game.player.x, &game.player.y);
    } else {
        map_generate_town4(&game.map, &game.player.x, &game.player.y);
    }
    ASSERT("shared gate offers no shortcut before its boss is defeated", !game_town_route_available(&game, gate_x[route], gate_y[route]));
    // Match the debug script: mark every shortcut unlocked, then refresh the town.
    game.defeated_bosses = (1 << LOCATION_FOREST) | (1 << LOCATION_SWAMP) | (1 << LOCATION_MOUNTAINS);
    if (game.location == LOCATION_TOWN) {
        map_set_town2_road(&game.map, 1);
        map_set_town4_road(&game.map, 1);
    } else if (game.location == LOCATION_TOWN2) {
        map_set_stillbury_forest_road(&game.map, 1);
        map_set_town3_road(&game.map, 1);
    } else if (game.location == LOCATION_TOWN3) {
        map_set_rosemoor_swamp_road(&game.map, 1);
    } else {
        map_set_ridgeshire_mountain_road(&game.map, 1);
    }
    int px = game.player.x;
    int py = game.player.y;
    action_resolve_player(&game, (Action){ACTION_MOVE, gate_x[route], gate_y[route]});
    ASSERT("debug-unlocked towns retain only the main area gate", game.map.tiles[old_y[route]][old_x[route]] == TILE_WALL && game.map.tiles[gate_y[route]][gate_x[route]] == TILE_TOWN_EXIT);
    if (towns[route] == LOCATION_TOWN2) {
        int paved = 1;
        for (int row = 11; row <= 13; row++) {
            for (int column = TOWN_HEALER_DOOR_X; column <= TOWN_WITCH_DOOR_X; column++) {
                paved &= game.map.tiles[row][column] == TILE_TOWN_PATH;
            }
        }
        ASSERT("retiring Stillbury's shortcut spurs preserves every town-square cobblestone", paved);
    }
    ASSERT("walking to the shared gate opens both choices without moving", game.location == towns[route] && game.player.x == px && game.player.y == py && game_shortcut_prompt_active(&game) && strstr(game.dialogue_text, "1. Travel to") && strstr(game.dialogue_text, "2. Take shortcut to") && strstr(game.dialogue_text, names[route]));
    ASSERT("held shortcut and movement keys cannot select a route", game_handle_shortcut_prompt_key(&game, SDL_SCANCODE_2, 1) && game_handle_shortcut_prompt_key(&game, SDL_SCANCODE_LEFT, 0) && game.location == towns[route] && game_shortcut_prompt_active(&game));
    ASSERT("Escape cancels the town route without moving", game_handle_shortcut_prompt_key(&game, SDL_SCANCODE_ESCAPE, 0) && !game_shortcut_prompt_active(&game) && game.location == towns[route] && game.player.x == px && game.player.y == py);
    action_resolve_player(&game, (Action){ACTION_MOVE, gate_x[route], gate_y[route]});
    ASSERT("pending town route choices survive save/load", save_game(&game, SHORTCUT_TEST_SLOT) && load_game(&loaded, SHORTCUT_TEST_SLOT) && game_shortcut_prompt_active(&loaded) && loaded.dialogue_x == gate_x[route] && loaded.dialogue_y == gate_y[route]);
    remove("saves/savegame_99125.json");
    game_handle_shortcut_prompt_key(&loaded, route % 2 ? SDL_SCANCODE_RETURN : SDL_SCANCODE_KP_1, 0);
    walk_town_approach(&loaded);
    ASSERT("the area choice enters the normal region from the selected town", loaded.location == areas[route] && loaded.level == (route % 2 ? 7 : 1) && !game_shortcut_prompt_active(&loaded));
    game_handle_shortcut_prompt_key(&game, SDL_SCANCODE_KP_2, 0);
    ASSERT("the shortcut choice enters the safe path rather than the destination town", game.location == roads[route] && game.enemy_count == 0 && !game_shortcut_prompt_active(&game));
}

void test_regional_shortcuts(void) {
    printf("Consistent regional shortcut discovery tests:\n");
    ASSERT("regional shortcut test slot is unused", !save_exists(SHORTCUT_TEST_SLOT));
    shortcut_layouts();
    for (int route = 0; route < 6; route++) {
        town_route_choice(route);
    }
    for (int road = 0; road < 3; road++) {
        for (int reverse = 0; reverse < 2; reverse++) {
            town_shortcut_crossing(road, reverse);
        }
    }
    for (int region = 0; region < 3; region++) {
        for (int attack = 0; attack < 3; attack++) {
            shortcut_victory(region, attack);
        }
        shortcut_migration(region);
    }
}
