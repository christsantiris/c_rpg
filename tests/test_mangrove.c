#include "test_utils.h"
#include "game/mangrove.h"
#include "game/town_road.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

#define DELTA_SLOT 99148

static int walk_to(GameState *g, int tx, int ty, int afloat) {
    int previous[MANGROVE_H][MANGROVE_W];
    int queue[MANGROVE_W * MANGROVE_H];
    memset(previous, -1, sizeof(previous));
    int start = g->player.y * MANGROVE_W + g->player.x;
    int target = ty * MANGROVE_W + tx;
    int head = 0;
    int tail = 1;
    queue[0] = start;
    previous[g->player.y][g->player.x] = start;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail && previous[ty][tx] < 0) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % MANGROVE_W + dx[i];
            int y = cell / MANGROVE_W + dy[i];
            if (x < 1 || x >= MANGROVE_W - 1 || y < 1 || y >= MANGROVE_H - 1 || previous[y][x] >= 0) {
                continue;
            }
            int open = afloat ? g->map.tiles[y][x] == TILE_MANGROVE_WATER : map_is_walkable(&g->map, x, y);
            if (open) {
                previous[y][x] = cell;
                queue[tail++] = y * MANGROVE_W + x;
            }
        }
    }
    if (previous[ty][tx] < 0) {
        return 0;
    }
    int count = 0;
    for (int cell = target; cell != start; cell = previous[cell / MANGROVE_W][cell % MANGROVE_W]) {
        queue[count++] = cell;
    }
    for (int i = count - 1; i >= 0; i--) {
        action_resolve_player(g, (Action){ACTION_MOVE, queue[i] % MANGROVE_W, queue[i] / MANGROVE_W});
        if (g->player.x != queue[i] % MANGROVE_W || g->player.y != queue[i] / MANGROVE_W) {
            return 0;
        }
    }
    return 1;
}

void test_mangrove(void) {
    printf("Saltwind landing and physical skiff exploration:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("skiff fixtures allocate on the heap", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    g->player.player_class = CLASS_WARRIOR;
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    ASSERT("Dawnmere has a single southern exit tile", g->map.tiles[TOWN_H - 1][20] == TILE_TOWN_EXIT && g->map.tiles[TOWN_H - 1][19] != TILE_TOWN_EXIT && g->map.tiles[TOWN_H - 1][21] != TILE_TOWN_EXIT);
    action_resolve_player(g, (Action){ACTION_MOVE, 20, TOWN_H - 1});
    ASSERT("the southern gate still leaves town through its matching exterior", g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_DAWNMERE);
    int x;
    int y;
    town_road_exit(TOWN_SIDE_SOUTH, &x, &y);
    town_road_travel(g, x, y);
    ASSERT("the exterior southern road reaches Saltwind's separate safe approach", g->location == LOCATION_MANGROVE_PATH && g->player.last_dy == 1 && !g->enemy_count);
    int road = 1;
    for (int row = 1; row <= MANGROVE_GATE_Y - 4; row++) {
        road &= g->map.tiles[row][MANGROVE_GATE_X] == TILE_TOWN_PATH && g->map.tiles[row][MANGROVE_GATE_X - 1] != TILE_TOWN_PATH && g->map.tiles[row][MANGROVE_GATE_X + 1] != TILE_TOWN_PATH;
        action_resolve_player(g, (Action){ACTION_MOVE, MANGROVE_GATE_X, row});
    }
    ASSERT("the safe approach road is one tile wide", road && g->location == LOCATION_MANGROVE_PATH);
    action_resolve_player(g, (Action){ACTION_MOVE, MANGROVE_GATE_X, MANGROVE_GATE_Y - 3});
    ASSERT("walking into the mangrove landmark starts the landing facing south", g->location == LOCATION_MANGROVES && g->level == 1 && g->player.x == 14 && g->player.y == 1 && g->player.last_dy == 1 && mangrove_state_valid(g));
    ASSERT("the starting landing is reachable on foot", walk_to(g, 14, 14, 0));
    int py = g->player.y;
    action_resolve_player(g, (Action){ACTION_MOVE, 14, 15});
    ASSERT("deep water blocks walking until the skiff is boarded", g->player.y == py && !map_is_walkable(&g->map, 14, 15));
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("interacting boards the actual skiff on its water tile", g->mangroves.aboard && g->player.x == 14 && g->player.y == 15);
    int inventory = g->inventory_count;
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, 0, 0});
    ASSERT("items cannot be dropped into deep water from the skiff", g->inventory_count == inventory && !g->floor_item_count && mangrove_state_valid(g));
    action_resolve_player(g, (Action){ACTION_MOVE, 14, 14});
    ASSERT("paddling onto land cannot bypass docking", g->mangroves.aboard && g->player.y == 15);

    const int moorings[4][2] = {{14, 15}, {31, 11}, {23, 23}, {51, 24}};
    int gold = g->gold;
    for (int i = 1; i < 4; i++) {
        ASSERT("each island has a reachable water approach", walk_to(g, moorings[i][0], moorings[i][1], 1));
        ASSERT("aboard saves preserve skiff coordinates and facing", save_game(g, DELTA_SLOT) && load_game(loaded, DELTA_SLOT) && loaded->mangroves.aboard && loaded->mangroves.boat_x == g->player.x && loaded->mangroves.boat_y == g->player.y && loaded->mangroves.boat_dx == g->mangroves.boat_dx);
        *g = *loaded;
        action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("interacting at the pier goes ashore and leaves the skiff moored", !g->mangroves.aboard && g->player.x == mangrove_landings[i][0] && g->player.y == mangrove_landings[i][1] && g->mangroves.boat_x == moorings[i][0] && g->mangroves.boat_y == moorings[i][1]);
        ASSERT("island supplies have a reachable adjacent foot tile", walk_to(g, mangrove_caches[i - 1][0] - 1, mangrove_caches[i - 1][1], 0));
        action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("optional supplies pay once and record exploration", g->gold == gold + i * 15 && g->mangroves.searched == (unsigned)((1 << i) - 1));
        action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("repeating a search cannot repeat its reward", g->gold == gold + i * 15);
        ASSERT("the moored skiff can be reached again on foot", walk_to(g, mangrove_landings[i][0], mangrove_landings[i][1], 0));
        action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the same skiff can be reboarded at every landing", g->mangroves.aboard);
    }
    g->last_town_visited = LOCATION_TOWN2;
    int bx = g->player.x;
    int by = g->player.y;
    game_open_town_portal(g);
    ASSERT("a portal cast aboard visits the last town and keeps the skiff occupied", g->location == LOCATION_TOWN2 && g->portal_active && g->portal_location == LOCATION_MANGROVES && g->mangroves.aboard);
    ASSERT("boat expedition state survives saving in town", save_game(g, DELTA_SLOT) && load_game(loaded, DELTA_SLOT) && loaded->portal_active && loaded->mangroves.aboard && loaded->mangroves.searched == 7);
    *g = *loaded;
    game_use_town_portal(g);
    ASSERT("the return portal restores the exact boat and water position without duplication", g->location == LOCATION_MANGROVES && !g->portal_active && g->mangroves.aboard && g->player.x == bx && g->player.y == by && g->mangroves.boat_x == bx && g->mangroves.boat_y == by && mangrove_state_valid(g));
    ASSERT("the starting shore is reachable after exploring every island", walk_to(g, 14, 15, 1));
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("disembarking returns to the starting landing", !g->mangroves.aboard && g->player.y == 14);
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, 0, 0});
    ASSERT("gear can be left on a landing while its boarding interaction remains available", g->floor_item_count == 1 && mangrove_hint(g));
    game_open_town_portal(g);
    ASSERT("a shore portal retains the parked skiff and dropped gear", g->location == LOCATION_TOWN2 && !g->mangroves.aboard && g->mangroves.loot_count == 1);
    game_use_town_portal(g);
    ASSERT("a shore return restores the landing, its gear and the same skiff", g->location == LOCATION_MANGROVES && !g->mangroves.aboard && g->player.x == 14 && g->player.y == 14 && g->floor_item_count == 1 && g->mangroves.boat_x == 14 && g->mangroves.boat_y == 15 && mangrove_state_valid(g));
    ASSERT("the original boundary exit is reachable on foot", walk_to(g, 14, 1, 0));
    action_resolve_player(g, (Action){ACTION_MOVE, 14, 0});
    ASSERT("retreat returns to the original approach instead of directly to town", g->location == LOCATION_MANGROVE_PATH && g->player.y == MANGROVE_GATE_Y - 4 && g->mangroves.loot_count == 1);
    for (int row = g->player.y - 1; row >= 0; row--) {
        action_resolve_player(g, (Action){ACTION_MOVE, MANGROVE_GATE_X, row});
    }
    ASSERT("walking north returns to Dawnmere's southern exterior", g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_DAWNMERE);
    town_road_entrance(TOWN_SIDE_SOUTH, &x, &y);
    while (g->location == LOCATION_TOWN_ROAD && g->player.y > y) {
        action_resolve_player(g, (Action){ACTION_MOVE, x, g->player.y - 1});
    }
    ASSERT("entering town from the mangroves places the player beside its south gate", g->location == LOCATION_DAWNMERE && g->player.x == 20 && g->player.y == TOWN_H - 2 && g->player.last_dy == -1);
    mangrove_enter(g, 0);
    ASSERT("a new visit preserves explored islands, claimed supplies and dropped gear", g->location == LOCATION_MANGROVES && !g->mangroves.aboard && g->mangroves.searched == 7 && g->floor_item_count == 1 && g->map.tiles[14][14] == TILE_ITEM && map_is_explored(&g->map, 51, 31));
    TileType terrain = g->map.tiles[7][7];
    g->map.tiles[7][7] = TILE_TOWN_FLOOR;
    ASSERT("a town map cannot be saved as the Saltwind adventure", !save_game(g, DELTA_SLOT));
    g->map.tiles[7][7] = terrain;
    g->mangroves.boat_x = 14;
    g->mangroves.boat_y = 14;
    ASSERT("saving rejects an impossible skiff parked on land", !save_game(g, DELTA_SLOT));
    remove("saves/savegame_99148.json");
    free(g);
    free(loaded);
}
