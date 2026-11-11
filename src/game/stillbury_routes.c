#include "stillbury_routes.h"
#include "approach.h"
#include "boss_gates.h"
#include "elderwood.h"
#include "giants_crown.h"
#include "jail.h"
#include "quarry_quest.h"
#include "town_road.h"
#include <stdlib.h>

static void trail_tile(GameState *g, int x, int y) {
    g->map.tiles[y][x] = TILE_TOWN_PATH;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = TILE_TOWN_PATH;
            g->map.tiles[y][x] = TILE_ITEM;
        }
    }
}

void stillbury_routes_prepare(GameState *g) {
    if (g->location != LOCATION_APPROACH) {
        return;
    }
    if (g->approach_route == APPROACH_QUARRY_ROAD || g->approach_route == APPROACH_ELDERWOOD_ROAD || g->approach_route == APPROACH_COAST_ROAD) {
        // The escarpment has no northern passage. Retile only solid scenery so
        // existing trails, actors, loot and return anchors stay usable.
        for (int y = 0; y <= 12; y++) {
            for (int x = 0; x < APPROACH_W; x++) {
                TileType tile = g->map.tiles[y][x];
                if (y <= 8 + x / 4 % 5 && (tile == TILE_WALL || tile == TILE_FOREST_WALL || tile == TILE_ELDERWOOD_WALL || tile == TILE_ROAD_BANK)) {
                    g->map.tiles[y][x] = TILE_MOUNTAIN_WALL;
                }
            }
        }
    }
    quarry_prepare(g);
    if (northern_approach(g)) {
        if (boss_gate_open(g, LOCATION_SWAMP)) {
            // The eastern exit emerges above the demon's checkpoint, then bends toward the Catacombs.
            for (int x = NORTH_SWAMP_X; x <= NORTH_SWAMP_X + 4; x++) {
                trail_tile(g, x, NORTH_SWAMP_Y - 6);
            }
            for (int y = NORTH_SWAMP_Y - 8; y <= NORTH_SWAMP_Y - 6; y++) {
                trail_tile(g, NORTH_SWAMP_X + 4, y);
            }
            for (int x = NORTH_ROAD_X; x <= NORTH_SWAMP_X + 4; x++) {
                trail_tile(g, x, NORTH_SWAMP_Y - 8);
            }
        }
        map_mark_explored(&g->map, NORTH_ROAD_X, 0);
        map_mark_explored(&g->map, NORTH_ROAD_X, APPROACH_H - 1);
        map_mark_explored(&g->map, NORTH_SWAMP_X, NORTH_SWAMP_Y);
        map_mark_explored(&g->map, NORTH_CAT_X, NORTH_CAT_Y);
    } else if (approach_bridge_road(g)) {
        map_mark_explored(&g->map, BRIDGE_QUARRY_X, 0);
    } else if (g->approach_route == APPROACH_ELDERWOOD_ROAD || g->approach_route == APPROACH_COAST_ROAD) {
        if (g->approach_route == APPROACH_ELDERWOOD_ROAD && boss_gate_open(g, LOCATION_ELDERWOOD)) {
            // The final northbound exit has a short inward trail before the coastward turn.
            for (int y = 14; y <= 16; y++) {
                trail_tile(g, 32, y);
            }
            for (int x = 32; x <= 48; x++) {
                trail_tile(g, x, 14);
            }
            for (int y = 14; y <= APPROACH_H / 2; y++) {
                trail_tile(g, 48, y);
            }
        }
        map_mark_explored(&g->map, g->map.stairs_up_x, g->map.stairs_up_y);
        map_mark_explored(&g->map, g->map.stairs_down_x, g->map.stairs_down_y);
        if (g->approach_route == APPROACH_ELDERWOOD_ROAD) {
            map_mark_explored(&g->map, APPROACH_W - 1, APPROACH_H / 2);
        }
    }
}

int stillbury_routes_travel(GameState *g, int x, int y) {
    if (g->location != LOCATION_APPROACH) {
        return 0;
    }
    TileType tile = g->map.tiles[y][x];
    if (approach_bridge_road(g) && x == BRIDGE_QUARRY_X && y == 0) {
        approach_enter_region(g, APPROACH_QUARRY_ROAD);
        push_message(g, "North: Stonewake Quarry. The labyrinth is an optional stop beside this road.");
    } else if (g->approach_route == APPROACH_QUARRY_ROAD && tile == TILE_APPROACH_TOWN) {
        approach_enter_region(g, APPROACH_PEAKS_NORTH_ROAD);
        g->player.x = BRIDGE_QUARRY_X;
        g->player.y = 1;
        g->player.last_dx = 0;
        g->player.last_dy = 1;
        push_message(g, "The road forks: west to Giant's Bridge, east to Stillbury.");
    } else if (g->approach_route == APPROACH_ELDERWOOD_ROAD && tile == TILE_APPROACH_DUNGEON_TOWN) {
        if (boss_gate_crossing(g, LOCATION_ELDERWOOD)) {
            approach_enter_region(g, APPROACH_COAST_ROAD);
            push_message(g, "Elderwood opens onto the Sunken Coast. Stillbury lies west through the forest.");
        }
    } else if (g->approach_route == APPROACH_COAST_ROAD && tile == TILE_APPROACH_TOWN) {
        approach_enter_region(g, APPROACH_ELDERWOOD_ROAD);
        // Legacy coast expeditions predate Elderwood's gate; keep their retreat usable.
        g->player.x = boss_gate_open(g, LOCATION_ELDERWOOD) ? APPROACH_W - 2 : APPROACH_W - 5;
        g->player.y = APPROACH_H / 2;
        g->player.last_dx = -1;
        g->player.last_dy = 0;
    } else {
        return 0;
    }
    map_mark_explored(&g->map, g->player.x, g->player.y);
    return 1;
}

void stillbury_routes_migrate(GameState *g, int version) {
    if (g->location == LOCATION_CASTLE) {
        // Remove the retired Ashen spur without replacing the grounds or loot.
        for (int y = 1; y < CASTLE_ROAD_Y; y++) {
            if (g->map.tiles[y][CASTLE_ASHEN_GATE_X] == TILE_TOWN_PATH) {
                g->map.tiles[y][CASTLE_ASHEN_GATE_X] = TILE_TOWN_FLOOR;
            }
        }
        g->map.tiles[0][CASTLE_ASHEN_GATE_X] = TILE_WALL;
        if (g->player.x == CASTLE_ASHEN_GATE_X && g->player.y == 0) {
            g->player.y = 1;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == CASTLE_ASHEN_GATE_X && item->y < CASTLE_ROAD_Y) {
                item->y = item->y ? item->y : 1;
                item->underlying_tile = TILE_TOWN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
    }
    if (version < 248) {
        if (g->location == LOCATION_SWAMP_ROAD || g->location == LOCATION_HIGH_PASS) {
            g->swamp_entry_town = LOCATION_TOWN2;
            g->mountain_entry_town = LOCATION_TOWN;
            approach_migrate_roads(g);
        }
        if (g->location == LOCATION_APPROACH && g->approach_route == 13) {
            // The existing Ashen expedition and caches remain intact; only its
            // safe approach now returns west through Rosemoor instead of the castle.
            approach_refresh_entrance(g);
        }
        if (g->location == LOCATION_TOWN3) {
            map_set_rosemoor_swamp_road(&g->map, 0);
            map_place_town3_ashen_gate(&g->map);
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == 0 && abs(item->y - ROSEMOOR_ASHEN_GATE_Y) <= 2) {
                    item->underlying_tile = TILE_TOWN_EXIT;
                }
            }
        }
        if (g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN3) {
            town_road_migrate(g);
        }
    }
    if (g->swamp_cache[0].valid) {
        map_swamp_south_entry(&g->swamp_cache[0].map);
    }
    if (g->location == LOCATION_SWAMP && g->level == 1) {
        int old_x = g->map.stairs_up_x;
        int old_y = g->map.stairs_up_y;
        map_swamp_south_entry(&g->map);
        if (old_x == 0 && g->map.stairs_up_y == SWAMP_MAP_H - 1) {
            if (g->player.x == old_x && g->player.y == old_y) {
                g->player.x++;
            }
            TileType bank = g->map.tiles[old_y][old_x + 1];
            for (int i = 0; i < g->floor_item_count; i++) {
                const FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == old_x + 1 && item->y == old_y) {
                    bank = item->underlying_tile;
                    break;
                }
            }
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == old_x && item->y == old_y) {
                    item->x++;
                    item->underlying_tile = bank;
                    g->map.tiles[item->y][item->x] = TILE_ITEM;
                }
            }
        }
    }
    // Extend the old interior Elderwood markers onto the outdoor boundary in place.
    for (int i = 0; i < ELDERWOOD_DEPTH; i++) {
        ElderwoodCache *cache = &g->elderwood_cache[i];
        if (cache->valid) {
            if (cache->tiles[46][32] == TILE_ELDERWOOD_ENTRANCE) {
                cache->tiles[46][32] = TILE_ELDERWOOD_FLOOR;
            }
            if (cache->tiles[1][32] == TILE_ELDERWOOD_EXIT) {
                cache->tiles[1][32] = TILE_ELDERWOOD_FLOOR;
            }
            cache->tiles[47][32] = TILE_ELDERWOOD_ENTRANCE;
            cache->tiles[0][32] = TILE_ELDERWOOD_EXIT;
        }
    }
    if (g->location == LOCATION_ELDERWOOD) {
        if (g->map.tiles[46][32] == TILE_ELDERWOOD_ENTRANCE) {
            g->map.tiles[46][32] = TILE_ELDERWOOD_FLOOR;
        }
        if (g->map.tiles[1][32] == TILE_ELDERWOOD_EXIT) {
            g->map.tiles[1][32] = TILE_ELDERWOOD_FLOOR;
        }
        g->map.tiles[47][32] = TILE_ELDERWOOD_ENTRANCE;
        g->map.tiles[0][32] = TILE_ELDERWOOD_EXIT;
        g->map.stairs_up_y = 47;
        g->map.stairs_down_y = 0;
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->x == 32 && ((item->y == 46 && item->underlying_tile == TILE_ELDERWOOD_ENTRANCE) ||
                (item->y == 1 && item->underlying_tile == TILE_ELDERWOOD_EXIT))) {
                item->underlying_tile = TILE_ELDERWOOD_FLOOR;
            }
        }
    }
    if (g->portal_location == LOCATION_ELDERWOOD && g->portal_x == 32 &&
        ((g->portal_y == 46 && g->portal_origin_tile == TILE_ELDERWOOD_ENTRANCE) ||
        (g->portal_y == 1 && g->portal_origin_tile == TILE_ELDERWOOD_EXIT))) {
        g->portal_origin_tile = TILE_ELDERWOOD_FLOOR;
    }
    if (version < 248) {
        g->elderwood_entry_town = LOCATION_TOWN2;
        if (g->catacombs_entry_town == LOCATION_TOWN) {
            g->catacombs_entry_town = LOCATION_TOWN2;
        }
        if (g->swamp_entry_town == LOCATION_TOWN3) {
            g->swamp_entry_town = LOCATION_TOWN2;
        }
        if (g->portal_location == LOCATION_ELDERWOOD) {
            g->northern_portal_town = LOCATION_TOWN2;
        }
        if (g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN2) {
            town_road_migrate(g);
        }
        if (g->location == LOCATION_APPROACH && (g->approach_route == 3 || g->approach_route == 5 || g->approach_route == 7 || g->approach_route == 9 || g->approach_route == 14 || g->approach_route == APPROACH_QUARRY_ROAD || approach_bridge_road(g))) {
            int coastal_origin_changed = g->approach_route == 3;
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && g->map.tiles[item->y][item->x] == TILE_ITEM) {
                    g->map.tiles[item->y][item->x] = item->underlying_tile;
                }
            }
            if (g->approach_route == 5 || g->approach_route == 9) {
                g->approach_route = APPROACH_BLACKWATER_ROAD;
            } else if (g->approach_route == 3) {
                g->approach_route = APPROACH_COAST_ROAD;
            } else if (g->approach_route == 7) {
                g->approach_route = APPROACH_QUARRY_ROAD;
            }
            if (northern_approach(g)) {
                northern_refresh(g);
            } else if (g->approach_route == APPROACH_COAST_ROAD || g->approach_route == APPROACH_QUARRY_ROAD || approach_bridge_road(g)) {
                approach_refresh_entrance(g);
            }
            int below_demon = g->approach_route == APPROACH_BLACKWATER_ROAD && !boss_gate_open(g, LOCATION_SWAMP);
            if (coastal_origin_changed || !map_is_walkable(&g->map, g->player.x, g->player.y) || (below_demon && g->player.y <= NORTH_SWAMP_BARRIER_Y)) {
                g->player.x = g->map.stairs_up_x;
                g->player.y = g->map.stairs_up_y;
                g->player.x += g->player.x == 0 ? 1 : g->player.x == APPROACH_W - 1 ? -1 : 0;
                g->player.y += g->player.y == 0 ? 1 : g->player.y == APPROACH_H - 1 ? -1 : 0;
            }
            map_mark_explored(&g->map, g->player.x, g->player.y);
            // Generic approach repairs have already restored item overlays.
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && g->map.tiles[item->y][item->x] == TILE_ITEM) {
                    g->map.tiles[item->y][item->x] = item->underlying_tile;
                }
            }
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (!item->active) {
                    continue;
                }
                if (coastal_origin_changed || !map_is_walkable(&g->map, item->x, item->y) || (below_demon && item->y <= NORTH_SWAMP_BARRIER_Y)) {
                    item->x = g->player.x;
                    item->y = g->player.y;
                }
                item->underlying_tile = g->map.tiles[item->y][item->x];
            }
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && !map_is_travel_tile(item->underlying_tile)) {
                    g->map.tiles[item->y][item->x] = TILE_ITEM;
                }
            }
        }
    }
    stillbury_routes_prepare(g);
    jail_migrate_destination(g, version);
}
