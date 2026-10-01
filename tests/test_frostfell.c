#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/game/actions.h"
#include "../src/systems/save_load.h"

#include <stdio.h>
#include <string.h>

static GameState frost_game;
static GameState frost_loaded;
static Map frost_layout;
static unsigned char visited[MAP_H][MAP_W];
static int queue[MAP_W * MAP_H];

// Every snow tile, plus the exit, must be reachable from the entrance.
static int frost_layout_connected(const Map *map) {
    memset(visited, 0, sizeof(visited));
    int head = 0;
    int tail = 0;
    queue[tail++] = map->stairs_up_y * MAP_W + map->stairs_up_x;
    visited[map->stairs_up_y][map->stairs_up_x] = 1;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail) {
        int tile = queue[head++];
        int x = tile % MAP_W;
        int y = tile / MAP_W;
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (nx >= 0 && nx < MAP_W && ny >= 0 && ny < MAP_H &&
                !visited[ny][nx] && map_is_walkable(map, nx, ny)) {
                visited[ny][nx] = 1;
                queue[tail++] = ny * MAP_W + nx;
            }
        }
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = map->tiles[y][x];
            if ((tile == TILE_FROST_FLOOR || tile == TILE_FROST_EXIT) && !visited[y][x]) {
                return 0;
            }
        }
    }
    return 1;
}

static int frost_layout_snowed_over(const Map *map) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = map->tiles[y][x];
            if (tile == TILE_SWAMP_FLOOR || tile == TILE_SWAMP_WALL ||
                tile == TILE_SWAMP_ENTRANCE || tile == TILE_SWAMP_EXIT) {
                return 0;
            }
        }
    }
    return 1;
}

static void walk_onto(GameState *g, int x, int y) {
    action_resolve_player(g, (Action){ACTION_MOVE, x, y});
}

void test_frostfell(void) {
    printf("Frostfell Wastes tests:\n");

    int layouts_ok = 1;
    for (int level = 1; level <= FROSTFELL_DEPTH; level++) {
        for (int seed = 1; seed <= 20; seed++) {
            srand((unsigned int)(seed * 31 + level));
            map_generate_frostfell(&frost_layout, level);
            layouts_ok &= frost_layout.stairs_up_x == SWAMP_MAP_W - 1 &&
                frost_layout.stairs_down_x == 0 &&
                frost_layout.tiles[frost_layout.stairs_up_y][SWAMP_MAP_W - 1] ==
                    TILE_FROST_ENTRANCE &&
                frost_layout.tiles[frost_layout.stairs_down_y][0] == TILE_FROST_EXIT &&
                frost_layout_snowed_over(&frost_layout) &&
                frost_layout_connected(&frost_layout);
        }
    }
    ASSERT("Frostfell stages run east to west on connected snowfields",
        layouts_ok);

    memset(&frost_game, 0, sizeof(frost_game));
    frost_game.player.player_class = CLASS_WARRIOR;
    game_init(&frost_game);
    game_enter_town2(&frost_game);
    int west_gate = 1;
    for (int y = 10; y <= 14; y++) {
        west_gate &= frost_game.map.tiles[y][0] == TILE_TOWN_EXIT;
    }
    frost_game.player.x = 1;
    frost_game.player.y = 12;
    walk_onto(&frost_game, 0, 12);
    ASSERT("Town 2's west gate opens onto the first Frostfell stage",
        west_gate && frost_game.location == LOCATION_FROSTFELL &&
        frost_game.level == 1 &&
        frost_game.player.x == frost_game.map.stairs_up_x &&
        frost_game.player.y == frost_game.map.stairs_up_y);

    for (int stage = 1; stage < FROSTFELL_DEPTH; stage++) {
        frost_game.player.x = frost_game.map.stairs_down_x + 1;
        frost_game.player.y = frost_game.map.stairs_down_y;
        walk_onto(&frost_game, frost_game.map.stairs_down_x, frost_game.map.stairs_down_y);
    }
    int deepest = frost_game.level == FROSTFELL_DEPTH &&
        frost_game.max_frostfell_level_reached == FROSTFELL_DEPTH;
    frost_game.player.x = frost_game.map.stairs_up_x - 1;
    frost_game.player.y = frost_game.map.stairs_up_y;
    walk_onto(&frost_game, frost_game.map.stairs_up_x, frost_game.map.stairs_up_y);
    ASSERT("the western exits descend to stage 5 and the east entrance climbs back",
        deepest && frost_game.level == FROSTFELL_DEPTH - 1 &&
        frost_game.frostfell_cache[FROSTFELL_DEPTH - 1].valid &&
        frost_game.player.x == frost_game.map.stairs_down_x);

    frost_game.player.x = frost_game.map.stairs_down_x + 1;
    frost_game.player.y = frost_game.map.stairs_down_y;
    int portal_x = frost_game.player.x;
    int portal_y = frost_game.player.y;
    game_open_town_portal(&frost_game);
    int in_town = frost_game.location == LOCATION_TOWN2 &&
        frost_game.player.x == 1 && frost_game.player.y == 12 &&
        frost_game.map.tiles[13][2] == TILE_PORTAL;
    game_use_town_portal(&frost_game);
    ASSERT("a return portal from Frostfell opens in Town 2 and leads back",
        in_town && frost_game.location == LOCATION_FROSTFELL &&
        frost_game.level == FROSTFELL_DEPTH - 1 &&
        frost_game.player.x == portal_x && frost_game.player.y == portal_y);

    const int slot = 99017;
    int loaded_ok = save_game(&frost_game, slot) && load_game(&frost_loaded, slot);
    ASSERT("Frostfell progress and stage caches survive save and load",
        loaded_ok && frost_loaded.location == LOCATION_FROSTFELL &&
        frost_loaded.level == FROSTFELL_DEPTH - 1 &&
        frost_loaded.max_frostfell_level_reached == FROSTFELL_DEPTH &&
        frost_loaded.frostfell_cache[FROSTFELL_DEPTH - 1].valid &&
        memcmp(frost_loaded.frostfell_cache[FROSTFELL_DEPTH - 1].map.tiles,
            frost_game.frostfell_cache[FROSTFELL_DEPTH - 1].map.tiles,
            sizeof(frost_game.map.tiles)) == 0);
    remove("saves/savegame_99017.json");

    for (int stage = frost_game.level; stage < FROSTFELL_DEPTH; stage++) {
        frost_game.player.x = frost_game.map.stairs_down_x + 1;
        frost_game.player.y = frost_game.map.stairs_down_y;
        walk_onto(&frost_game, frost_game.map.stairs_down_x, frost_game.map.stairs_down_y);
    }
    frost_game.player.x = frost_game.map.stairs_down_x + 1;
    frost_game.player.y = frost_game.map.stairs_down_y;
    walk_onto(&frost_game, frost_game.map.stairs_down_x, frost_game.map.stairs_down_y);
    ASSERT("the final western exit returns to Town 2's west gate",
        frost_game.location == LOCATION_TOWN2 &&
        frost_game.player.x == 1 && frost_game.player.y == 12);
}
