#include "test_utils.h"
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
    game.inventory_count = 1;
    game.equipped_main_hand = 0;
    game.player.known_spells[0] = spell_make_magic_arrow();
    game.player.known_spell_count = 1;
    game.player.equipped_spell = 0;
    Action action = attack == 0 ? (Action){ACTION_MOVE, 30, 28} :
        (Action){attack == 1 ? ACTION_CAST_SPELL : ACTION_RANGED_ATTACK, 0, 0};
    action_resolve_player(&game, action);
    ASSERT("all three regions prompt for melee, spell, and bow boss victories", !game.enemies[0].active && game_shortcut_prompt_active(&game) && strstr(game.dialogue_text, "beside the defeated boss"));
    int x = -1;
    int y = -1;
    ASSERT("shortcut appears beside the death position despite surviving enemies", find_shortcut(&game.map, markers[region], &x, &y) == 1 && abs(x - 30) + abs(y - 28) == 1 && game.enemies[1].active);
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
    ASSERT("moving an old shortcut preserves its portal without restoring a distant entrance", game.map.tiles[old_y][old_x] == TILE_PORTAL && game.portal_origin_tile == floors[region] && find_shortcut(&game.map, markers[region], &x, &y) == 1 && abs(x - 30) + abs(y - 28) == 1);
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
    memcpy(cache->enemies, game.enemies, sizeof(game.enemies));
    game.location = LOCATION_TOWN3;
    game.floor_item_count = 0;
    game.enemy_count = 0;
    game.dialogue_active = 0;
    map_generate_town3(&game.map, &game.player.x, &game.player.y);
    int ok = save_game(&game, SHORTCUT_TEST_SLOT) && load_game(&loaded, SHORTCUT_TEST_SLOT);
    LevelCache *migrated = region == 0 ? &loaded.forest_cache[3] :
        (region == 1 ? &loaded.swamp_cache[3] : &loaded.mountain_cache[3]);
    ASSERT("loading a town save repairs cached shortcuts near the defeated boss", ok && find_shortcut(&migrated->map, markers[region], &x, &y) == 1 && abs(x - 30) + abs(y - 28) == 1 && migrated->map.tiles[old_y][old_x] == floors[region]);
    ASSERT("shortcut migration preserves character and enemy progress", ok && loaded.player.hp == game.player.hp && loaded.gold == game.gold && loaded.score == game.score && loaded.defeated_bosses == game.defeated_bosses && migrated->enemies[0].hp == game.enemies[0].hp && migrated->enemies[1].active);
    remove("saves/savegame_99125.json");
}

static int single_tile_shortcut(const GameState *g) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            int path;
            if (g->location == LOCATION_FOREST_ROAD) {
                path = y == FOREST_ROAD_Y && x < FOREST_ROAD_W;
            } else if (g->location == LOCATION_SWAMP_ROAD) {
                path = x == SWAMP_ROAD_X && y < SWAMP_ROAD_H;
            } else {
                path = x == HIGH_PASS_X && y < HIGH_PASS_H;
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
    ASSERT("safe town shortcuts have exactly one walkable tile across their width", single_tile_shortcut(&game));
    Location shortcut = game.location;
    int steps = 0;
    while (game.location == shortcut && steps < MAP_W + MAP_H) {
        action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x + dx, game.player.y + dy});
        steps++;
    }
    ASSERT("one-tile shortcuts connect the correct towns in both directions", game.location == destination && game.enemy_count == 0);
}

void test_regional_shortcuts(void) {
    printf("Consistent regional shortcut discovery tests:\n");
    ASSERT("regional shortcut test slot is unused", !save_exists(SHORTCUT_TEST_SLOT));
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
