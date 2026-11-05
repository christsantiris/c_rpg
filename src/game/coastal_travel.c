#include "coastal_travel.h"
#include "eastern_arc.h"
#include "mangrove.h"
#include <string.h>

int coastal_approach(const GameState *g) {
    const TownApproach *route = approach_route(g);
    return route && route->area == LOCATION_COAST;
}

int coastal_stillbury_docks(const GameState *g) {
    return g->location == LOCATION_RIVER_CROSSING && !g->river_boat.aboard;
}

int coastal_at_docks(const GameState *g) {
    return coastal_stillbury_docks(g) &&
        g->player.x == STILLBURY_DOCK_ENTRY_X && g->player.y == STILLBURY_DOCK_ENTRY_Y;
}

int coastal_trade_restored(const GameState *g) {
    return (g->mara_beacons_lit & 7) == 7 && (g->defeated_bosses & (1u << LOCATION_COAST));
}

void coastal_place_docks(Map *map) {
    for (int x = RIVER_NORTH_X; x < STILLBURY_DOCK_X; x++) {
        map->tiles[STILLBURY_ROWAN_Y][x] = TILE_TOWN_PATH;
    }
    for (int y = STILLBURY_ROWAN_Y; y <= STILLBURY_DOCK_ENTRY_Y; y++) {
        map->tiles[y][STILLBURY_DOCK_X - 1] = TILE_TOWN_PATH;
    }
    for (int y = STILLBURY_DOCK_Y; y < STILLBURY_DOCK_Y + TOWN_HARBOR_H; y++) {
        for (int x = STILLBURY_DOCK_X; x < STILLBURY_DOCK_X + TOWN_HARBOR_W; x++) {
            map->tiles[y][x] = TILE_WATCHTOWER;
        }
    }
    for (int x = STILLBURY_DOCK_X; x <= STILLBURY_DOCK_ENTRY_X; x++) {
        map->tiles[STILLBURY_DOCK_ENTRY_Y][x] = TILE_TOWN_PATH;
    }
    map->tiles[STILLBURY_ROWAN_Y][STILLBURY_ROWAN_X] = TILE_NPC_ROWAN;
}

static void retile(GameState *g, int x, int y, TileType tile) {
    g->map.tiles[y][x] = tile;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = tile;
            g->map.tiles[y][x] = TILE_ITEM;
        }
    }
}

void coastal_refresh_docks(GameState *g) {
    if (g->location != LOCATION_RIVER_CROSSING) {
        return;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *item = &g->floor_items[i];
        if (item->active && g->map.tiles[item->y][item->x] == TILE_ITEM) {
            g->map.tiles[item->y][item->x] = item->underlying_tile;
        }
    }
    coastal_place_docks(&g->map);
    if (!g->river_boat.aboard && (!map_is_walkable(&g->map, g->player.x, g->player.y) ||
        (g->player.x == STILLBURY_ROWAN_X && g->player.y == STILLBURY_ROWAN_Y))) {
        g->player.x = STILLBURY_DOCK_X - 1;
        g->player.y = STILLBURY_DOCK_Y - 1;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (!item->active) {
            continue;
        }
        if (!map_is_walkable(&g->map, item->x, item->y) ||
            (item->x == STILLBURY_ROWAN_X && item->y == STILLBURY_ROWAN_Y)) {
            item->x = STILLBURY_DOCK_X - 1;
            item->y = STILLBURY_DOCK_Y - 1;
        }
        item->underlying_tile = g->map.tiles[item->y][item->x];
        for (int n = 0; n < i; n++) {
            const FloorItem *earlier = &g->floor_items[n];
            if (earlier->active && earlier->x == item->x && earlier->y == item->y) {
                item->underlying_tile = earlier->underlying_tile;
                break;
            }
        }
        if (item->underlying_tile != TILE_MANGROVE_EXIT && item->underlying_tile != TILE_MANGROVE_RETURN &&
            item->underlying_tile != TILE_MANGROVE_LANDING) {
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
    map_mark_explored(&g->map, STILLBURY_ROWAN_X, STILLBURY_ROWAN_Y);
    map_mark_explored(&g->map, STILLBURY_DOCK_ENTRY_X, STILLBURY_DOCK_ENTRY_Y);
}

void coastal_return_from_island(GameState *g) {
    eastern_river_enter(g, 1);
    g->player.x = STILLBURY_DOCK_ENTRY_X - 1;
    g->player.y = STILLBURY_DOCK_ENTRY_Y;
    g->player.last_dx = -1;
    g->player.last_dy = 0;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    push_message(g, "Rowan's ship returns to Stillbury's docks. Follow the shore path north into town.");
}

void coastal_migrate_quest(GameState *g, int version) {
    if (version < 244) {
        if (strcmp(g->dialogue_speaker, "Mara") == 0 || strcmp(g->dialogue_speaker, "Captain Rowan") == 0) {
            g->dialogue_active = 0;
        }
        if (coastal_approach(g)) {
            approach_refresh_entrance(g);
        }
        Map shore;
        eastern_river_map(&shore);
        for (int i = 0; i < g->river_boat.loot_count; i++) {
            FloorItem *item = &g->river_boat.loot[i];
            if (item->active) {
                if (!map_is_walkable(&shore, item->x, item->y) ||
                    (item->x == STILLBURY_ROWAN_X && item->y == STILLBURY_ROWAN_Y)) {
                    item->x = STILLBURY_DOCK_X - 1;
                    item->y = STILLBURY_DOCK_Y - 1;
                }
                item->underlying_tile = shore.tiles[item->y][item->x];
            }
        }
        if (g->location == LOCATION_TAVERN) {
            if (g->player.x == MARA_TAVERN_X && g->player.y == MARA_TAVERN_Y) {
                g->player.y++;
            }
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == MARA_TAVERN_X && item->y == MARA_TAVERN_Y) {
                    item->y++;
                    item->underlying_tile = TILE_TAVERN_FLOOR;
                    g->map.tiles[item->y][item->x] = TILE_ITEM;
                }
            }
            g->map.tiles[MARA_TAVERN_Y][MARA_TAVERN_X] = TILE_NPC_MARA;
        }
    }
    coastal_refresh_docks(g);
}

void coastal_migrate_travel(GameState *g) {
    if (g->location == LOCATION_TOWN) {
        for (int y = TOWN_HARBOR_Y; y < TOWN_H - 1; y++) {
            for (int x = 21; x < TOWN_W - 1; x++) {
                TileType tile = g->map.tiles[y][x];
                if (tile == TILE_ITEM) {
                    for (int i = 0; i < g->floor_item_count; i++) {
                        const FloorItem *item = &g->floor_items[i];
                        if (item->active && item->x == x && item->y == y) {
                            tile = item->underlying_tile;
                        }
                    }
                }
                if (tile == TILE_WATCHTOWER || tile == TILE_NPC_ROWAN || tile == TILE_TOWN_PATH) {
                    retile(g, x, y, TILE_TOWN_FLOOR);
                }
            }
        }
        // Very old migrations moved covered loot out of this lot; remove stale item markers too.
        for (int y = TOWN_HARBOR_Y; y < TOWN_H - 1; y++) {
            for (int x = TOWN_HARBOR_X; x < TOWN_W - 1; x++) {
                retile(g, x, y, TILE_TOWN_FLOOR);
            }
        }
        retile(g, TOWN_ROWAN_X, TOWN_ROWAN_Y, TILE_TOWN_FLOOR);
    } else if (g->location == LOCATION_APPROACH && g->approach_route == 7) {
        // Keep the Glassdeep expedition; its approach belongs to Stillbury.
        approach_refresh_entrance(g);
    }
    if (coastal_approach(g)) {
        // The new branch may replace an old ground tile or a dropped-item underlay.
        coastal_refresh_docks(g);
    }
}
