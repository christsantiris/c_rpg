#include "act5_routes.h"
#include "town_road.h"

void act5_shape_entry(Map *map, Location area) {
    int ox = map->stairs_up_x;
    int oy = map->stairs_up_y;
    if (oy == SWAMP_MAP_H - 1) {
        map_mark_explored(map, ox, oy);
        return;
    }
    int x = ox < 1 ? 1 : ox >= SWAMP_MAP_W - 1 ? SWAMP_MAP_W - 2 : ox;
    TileType floor = area == LOCATION_FROSTFELL ? TILE_FROST_FLOOR : TILE_DRAGON_FLOOR;
    TileType entrance = area == LOCATION_FROSTFELL ? TILE_FROST_ENTRANCE : TILE_DRAGON_ENTRANCE;
    for (int y = oy; y < SWAMP_MAP_H; y++) {
        TileType tile = map->tiles[y][x];
        if (tile != TILE_ITEM && tile != TILE_PORTAL) {
            map->tiles[y][x] = floor;
        }
    }
    if (map->tiles[oy][ox] != TILE_ITEM && map->tiles[oy][ox] != TILE_PORTAL) {
        map->tiles[oy][ox] = floor;
    }
    map->stairs_up_x = x;
    map->stairs_up_y = SWAMP_MAP_H - 1;
    if (map->tiles[SWAMP_MAP_H - 1][x] != TILE_ITEM && map->tiles[SWAMP_MAP_H - 1][x] != TILE_PORTAL) {
        map->tiles[SWAMP_MAP_H - 1][x] = entrance;
    }
    map_mark_explored(map, x, SWAMP_MAP_H - 1);
}

static void reveal_stage(Map *map, TileType entrance, TileType exit) {
    const int x[2] = {map->stairs_up_x, map->stairs_down_x};
    const int y[2] = {map->stairs_up_y, map->stairs_down_y};
    for (int i = 0; i < 2; i++) {
        if (map->tiles[y[i]][x[i]] != TILE_PORTAL) {
            map->tiles[y[i]][x[i]] = i ? exit : entrance;
        }
        map_mark_explored(map, x[i], y[i]);
    }
}

void act5_reveal_exits(GameState *g) {
    TileType entrance;
    TileType exit;
    switch (g->location) {
        case LOCATION_FROSTFELL: entrance = TILE_FROST_ENTRANCE; exit = TILE_FROST_EXIT; break;
        case LOCATION_ASHEN: entrance = TILE_ASHEN_ENTRANCE; exit = TILE_ASHEN_EXIT; break;
        case LOCATION_DRAGONSPINE: entrance = TILE_DRAGON_ENTRANCE; exit = TILE_DRAGON_EXIT; break;
        case LOCATION_IRONFANG: entrance = TILE_IRON_ENTRANCE; exit = TILE_IRON_EXIT; break;
        default: return;
    }
    reveal_stage(&g->map, entrance, exit);
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == g->map.stairs_up_x && item->y == g->map.stairs_up_y) {
            item->underlying_tile = entrance;
        } else if (item->active && item->x == g->map.stairs_down_x && item->y == g->map.stairs_down_y) {
            item->underlying_tile = exit;
        }
    }
}

void act5_routes_migrate(GameState *g) {
    const Location areas[2] = {LOCATION_FROSTFELL, LOCATION_DRAGONSPINE};
    LevelCache *caches[2] = {g->frostfell_cache, g->dragonspine_cache};
    for (int i = 0; i < 2; i++) {
        if (caches[i][0].valid) {
            act5_shape_entry(&caches[i][0].map, areas[i]);
        }
        if (g->location != areas[i] || g->level != 1) {
            continue;
        }
        int ox = g->map.stairs_up_x;
        int oy = g->map.stairs_up_y;
        act5_shape_entry(&g->map, areas[i]);
        if (oy != g->map.stairs_up_y) {
            TileType floor = areas[i] == LOCATION_FROSTFELL ? TILE_FROST_FLOOR : TILE_DRAGON_FLOOR;
            for (int n = 0; n < g->floor_item_count; n++) {
                FloorItem *item = &g->floor_items[n];
                if (item->active && item->x == ox && item->y == oy && item->underlying_tile != TILE_PORTAL) {
                    item->underlying_tile = floor;
                }
            }
            if (g->portal_active && g->portal_location == g->location && g->portal_level == 1 && g->portal_x == ox && g->portal_y == oy) {
                g->portal_origin_tile = floor;
            }
        }
    }
    LevelCache *regions[3] = {g->frostfell_cache, g->dragonspine_cache, g->ashen_cache};
    for (int region = 0; region < 3; region++) {
        for (int level = 0; level < 5; level++) {
            Map *map = &regions[region][level].map;
            if (regions[region][level].valid) {
                const TileType entrances[3] = {TILE_FROST_ENTRANCE, TILE_DRAGON_ENTRANCE, TILE_ASHEN_ENTRANCE};
                const TileType exits[3] = {TILE_FROST_EXIT, TILE_DRAGON_EXIT, TILE_ASHEN_EXIT};
                reveal_stage(map, entrances[region], exits[region]);
            }
        }
    }
    act5_reveal_exits(g);
    town_road_reveal_gates(g);
}
