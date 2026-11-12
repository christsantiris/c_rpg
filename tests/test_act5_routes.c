#include "test_utils.h"
#include "town_road_checks.h"
#include "game/act5_routes.h"
#include "game/approach.h"
#include "game/eastern_arc.h"
#include "game/ironfang.h"
#include "game/stillbury_routes.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define ROUTE_CELLS (MAP_W * MAP_H)
#define ACT5_SLOT 99292

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
    if (start == target) {
        return 1;
    }
    int head = 0;
    int tail = 1;
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
            int transition = (map_is_travel_tile(tile) && tile != TILE_ROAD_GATE_OPEN) || tile == TILE_SWAMP_ENTRANCE || tile == TILE_SWAMP_EXIT || tile == TILE_COAST_ENTRANCE || tile == TILE_COAST_EXIT || tile == TILE_ASHEN_ENTRANCE || tile == TILE_ASHEN_EXIT || tile == TILE_TOWN_EXIT || tile == TILE_FROST_ENTRANCE || tile == TILE_FROST_EXIT || tile == TILE_DRAGON_ENTRANCE || tile == TILE_DRAGON_EXIT || tile == TILE_IRON_ENTRANCE || tile == TILE_IRON_EXIT;
            int occupied = 0;
            for (int i = 0; i < g->enemy_count; i++) {
                occupied |= g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y;
            }
            int entrance = tile == TILE_NORTH_SWAMP || tile == TILE_NORTH_CATACOMBS;
            if (previous[next] >= 0 || occupied || tile == TILE_FROST_ICE || (!map_is_walkable(&g->map, x, y) && !(entrance && next == target)) || (transition && next != target)) {
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
            if (location == LOCATION_ESCAPE_TUNNEL && g->location == location) {
                action_resolve_enemies(g);
            }
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

static void inward(GameState *g) {
    int x = g->player.x;
    int y = g->player.y;
    if (x == 0) {
        x++;
    } else if (x == SWAMP_MAP_W - 1) {
        x--;
    } else if (y == 0) {
        y++;
    } else if (y == SWAMP_MAP_H - 1) {
        y--;
    }
    if (x != g->player.x || y != g->player.y) {
        action_resolve_player(g, (Action){ACTION_MOVE, x, y});
    }
}

static void branch(GameState *g, int west, Location area) {
    game_init(g);
    game_enter_town_gate(g, LOCATION_CASTLE, CROWNROAD_X, TOWN_H - 1);
    ASSERT("both Castle Grounds road exits are discovered", map_is_explored(&g->map, 0, CASTLE_ROAD_Y) && map_is_explored(&g->map, TOWN_W - 1, CASTLE_ROAD_Y));
    ASSERT("the Castle Grounds boundary enters the correct compass road", route_path(g, west ? 0 : TOWN_W - 1, CASTLE_ROAD_Y) && g->location == (west ? LOCATION_CROWNROAD : LOCATION_CROWNROAD_WEST) && g->player.last_dx == (west ? -1 : 1));
    ASSERT("Crown Road names match their geographic sides", strcmp(approach_area_name(g->location), west ? "CROWN ROAD WEST" : "CROWN ROAD EAST") == 0);
    clear_enemies(g);
    ASSERT("the actual road boundary leads to the correct town exterior", route_path(g, west ? 0 : CROWNROAD_W - 1, CROWNROAD_Y) && g->location == LOCATION_TOWN_ROAD && g->road_town == (west ? LOCATION_TOWN3 : LOCATION_TOWN4));
    walk_into_arrived_town(g);
    ASSERT("town arrival retains the crossing direction", g->location == (west ? LOCATION_TOWN3 : LOCATION_TOWN4) && g->player.last_dx == (west ? -1 : 1));
    if (west) {
        game_enter_guild(g);
        game_leave_guild(g);
        eastern_refresh(g);
        ASSERT("entering interiors and refreshing Rosemoor preserve the Ashen gate", g->map.tiles[ROSEMOOR_ASHEN_GATE_Y][0] == TILE_TOWN_EXIT);
    }
    TownSide side = area == LOCATION_ASHEN ? TOWN_SIDE_WEST : area == LOCATION_IRONFANG ? TOWN_SIDE_EAST : TOWN_SIDE_NORTH;
    TownApproach connection;
    ASSERT("the requested regional route belongs to its town", town_road_connection(g->location, side, &connection) && connection.area == area && map_is_explored(&g->map, connection.gate_x, connection.gate_y));
    ASSERT("a real town path reaches the regional exterior", route_path(g, connection.gate_x, connection.gate_y) && g->location == LOCATION_TOWN_ROAD);
    int x;
    int y;
    town_road_exit(side, &x, &y);
    ASSERT("the town exterior reaches its regional approach", route_path(g, x, y) && g->location == LOCATION_APPROACH && approach_route(g)->area == area);
    ASSERT("the approach's actual doorway enters its adventure without a quest or boss gate", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == area && g->level == 1 && !g->defeated_bosses);
    if (area == LOCATION_FROSTFELL || area == LOCATION_DRAGONSPINE) {
        ASSERT("northbound arrival has a south-boundary entrance and an inward path", g->map.stairs_up_y == SWAMP_MAP_H - 1 && g->player.last_dy == -1 && map_is_walkable(&g->map, g->map.stairs_up_x, SWAMP_MAP_H - 2));
    }
    for (int stage = 1; stage <= 5; stage++) {
        clear_enemies(g);
        inward(g);
        ASSERT("each stage exposes both discovered transition tiles", map_is_explored(&g->map, g->map.stairs_up_x, g->map.stairs_up_y) && map_is_explored(&g->map, g->map.stairs_down_x, g->map.stairs_down_y));
        ASSERT("every stage has an actual usable retreat", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && (stage == 1 ? g->location == LOCATION_APPROACH : g->location == area && g->level == stage - 1));
        clear_enemies(g);
        inward(g);
        ASSERT("retreat preserves a real route back into the same stage", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == area && g->level == stage);
        clear_enemies(g);
        inward(g);
        if (stage < 5) {
            ASSERT("the actual onward transition advances to the next stage", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == area && g->level == stage + 1);
        }
    }
    clear_enemies(g);
    ASSERT("the final exit returns to the same town's regional road", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_APPROACH && approach_route(g)->town == (west ? LOCATION_TOWN3 : LOCATION_TOWN4));
    ASSERT("the regional road has a continuous retreat to its town exterior", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && g->location == LOCATION_TOWN_ROAD);
    walk_into_arrived_town(g);
    ASSERT("the adventure returns to its correct hub", g->location == (west ? LOCATION_TOWN3 : LOCATION_TOWN4));
    ASSERT("the hub's Crown Road gate still departs toward the castle", route_path(g, west ? TOWN_W - 1 : 0, west ? TOWN3_CROWN_GATE_Y : TOWN4_CROWN_GATE_Y) && g->location == LOCATION_TOWN_ROAD);
    finish_town_departure(g);
    clear_enemies(g);
    ASSERT("the cached Crown Road returns to Castle Grounds", route_path(g, west ? CROWNROAD_W - 1 : 0, CROWNROAD_Y) && g->location == LOCATION_CASTLE && g->player.last_dx == (west ? 1 : -1));
}

static int downgrade(int version) {
    const char *path = "saves/savegame_99292.json";
    FILE *file = fopen(path, "rb");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(path, "wb");
    ok = data && file && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static void legacy(GameState *g) {
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("legacy Act 5 fixture allocates", loaded != NULL);
    if (!loaded) {
        return;
    }
    game_init(g);
    game_enter_town_gate(g, LOCATION_TOWN3, TOWN_W - 1, TOWN3_CROWN_GATE_Y);
    g->map.tiles[ROSEMOOR_ASHEN_GATE_Y][0] = TILE_WALL;
    map_mark_explored(&g->map, 20, 12);
    ASSERT("version-249 Rosemoor saves reopen Ashen without losing exploration", save_game(g, ACT5_SLOT) && downgrade(249) && load_game(loaded, ACT5_SLOT) && loaded->map.tiles[ROSEMOOR_ASHEN_GATE_Y][0] == TILE_TOWN_EXIT && map_is_explored(&loaded->map, 20, 12));
    ASSERT("a restored old town can really depart through its Ashen gate", route_path(loaded, 0, ROSEMOOR_ASHEN_GATE_Y) && loaded->location == LOCATION_TOWN_ROAD);
    const Location areas[2] = {LOCATION_FROSTFELL, LOCATION_DRAGONSPINE};
    for (int i = 0; i < 2; i++) {
        game_init(g);
        if (!i) {
            game_enter_frostfell(g);
        } else {
            game_enter_dragonspine(g);
        }
        int cx;
        int cy;
        map_room_center(&g->map.rooms[0], &cx, &cy);
        int ox = i ? 0 : SWAMP_MAP_W - 1;
        g->map.tiles[g->map.stairs_up_y][g->map.stairs_up_x] = i ? TILE_DRAGON_WALL : TILE_FROST_WALL;
        g->map.stairs_up_x = ox;
        g->map.stairs_up_y = cy;
        g->map.tiles[cy][ox] = i ? TILE_DRAGON_ENTRANCE : TILE_FROST_ENTRANCE;
        g->player.x = ox;
        g->player.y = cy;
        g->enemies[0].hp = 7;
        g->floor_item_count = 1;
        g->floor_items[0] = (FloorItem){0};
        g->floor_items[0].active = 1;
        g->floor_items[0].x = ox;
        g->floor_items[0].y = cy;
        g->floor_items[0].underlying_tile = i ? TILE_DRAGON_ENTRANCE : TILE_FROST_ENTRANCE;
        g->floor_items[0].item = item_make_stone_warhammer();
        g->floor_items[0].item.sharpened = 1;
        g->map.tiles[cy][ox] = TILE_ITEM;
        g->inventory[g->inventory_count++] = item_make_stone_warhammer();
        map_mark_explored(&g->map, cx, cy);
        LevelCache *cache = i ? &g->dragonspine_cache[0] : &g->frostfell_cache[0];
        cache->valid = 1;
        cache->map = g->map;
        cache->enemy_count = g->enemy_count;
        memcpy(cache->enemies, g->enemies, sizeof(cache->enemies));
        ASSERT("old active and cached side entrances migrate to a south entrance", save_game(g, ACT5_SLOT) && downgrade(249) && load_game(loaded, ACT5_SLOT) && loaded->map.stairs_up_y == SWAMP_MAP_H - 1 && (i ? loaded->dragonspine_cache[0].map.stairs_up_y : loaded->frostfell_cache[0].map.stairs_up_y) == SWAMP_MAP_H - 1);
        ASSERT("entry migration preserves enemy wounds, equipment and exploration", loaded->enemies[0].hp == 7 && map_is_explored(&loaded->map, cx, cy) && loaded->inventory[loaded->inventory_count - 1].max_hp_bonus == 30);
        ASSERT("entry migration preserves floor loot and its metadata in place", loaded->floor_items[0].active && loaded->floor_items[0].x == ox && loaded->floor_items[0].y == cy && loaded->floor_items[0].item.sharpened && loaded->floor_items[0].underlying_tile == (i ? TILE_DRAGON_FLOOR : TILE_FROST_FLOOR));
        ASSERT("a player saved on the retired side entrance has usable ground after migration", map_is_walkable(&loaded->map, loaded->player.x, loaded->player.y));
        clear_enemies(loaded);
        int ex = loaded->map.stairs_up_x;
        int ey = loaded->map.stairs_up_y;
        loaded->floor_item_count = 2;
        loaded->floor_items[1] = loaded->floor_items[0];
        loaded->floor_items[1].x = ex;
        loaded->floor_items[1].y = ey;
        loaded->floor_items[1].underlying_tile = i ? TILE_DRAGON_ENTRANCE : TILE_FROST_ENTRANCE;
        loaded->map.tiles[ey][ex] = TILE_ITEM;
        act5_reveal_exits(loaded);
        ASSERT("floor loot cannot hide or disable the actual retreat transition", loaded->map.tiles[ey][ex] == (i ? TILE_DRAGON_ENTRANCE : TILE_FROST_ENTRANCE) && loaded->floor_items[1].active && loaded->floor_items[1].item.sharpened);
        ASSERT("a migrated expedition can use the actual new retreat tile", route_path(loaded, loaded->map.stairs_up_x, loaded->map.stairs_up_y) && loaded->location == LOCATION_APPROACH);
    }
    game_init(g);
    game_enter_crownroad(g, LOCATION_CROWNROAD, 1);
    g->enemies[0].hp = 7;
    ASSERT("correcting a road label preserves a wounded patrol through save/load", save_game(g, ACT5_SLOT) && downgrade(249) && load_game(loaded, ACT5_SLOT) && loaded->enemies[0].hp == 7 && loaded->location == LOCATION_CROWNROAD && strcmp(approach_area_name(loaded->location), "CROWN ROAD WEST") == 0);
    remove("saves/savegame_99292.json");
    free(loaded);
}

void test_act5_routes(void) {
    printf("Act 5 Castle Grounds, Rosemoor and Ridgeshire navigation:\n");
    GameState *g = calloc(1, sizeof(*g));
    ASSERT("Act 5 fixture allocates", g != NULL);
    if (!g) {
        return;
    }
    const Location areas[] = {LOCATION_ASHEN, LOCATION_FROSTFELL, LOCATION_DRAGONSPINE, LOCATION_IRONFANG};
    for (int seed = 0; seed < 3; seed++) {
        for (int area = 0; area < 4; area++) {
            srand(19 + seed * 97);
            branch(g, area < 2, areas[area]);
        }
    }
    legacy(g);
    free(g);
}
