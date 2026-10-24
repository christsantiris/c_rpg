#include "coastal_travel.h"

int coastal_approach(const GameState *g) {
    const TownApproach *route = approach_route(g);
    return route && route->area == LOCATION_COAST;
}

int coastal_at_docks(const GameState *g) {
    return coastal_approach(g) && game_harbor_unlocked(g) &&
        g->player.x == COAST_DOCK_ENTRY_X && g->player.y == COAST_DOCK_ENTRY_Y;
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
    if (!coastal_approach(g)) {
        return;
    }
    for (int x = APPROACH_W / 2; x <= COAST_DOCK_X - 1; x++) {
        retile(g, x, 16, TILE_TOWN_PATH);
    }
    for (int y = 17; y <= COAST_DOCK_ENTRY_Y; y++) {
        retile(g, COAST_DOCK_X - 1, y, y < COAST_DOCK_Y || game_harbor_unlocked(g) ? TILE_TOWN_PATH : TILE_COAST_WALL);
    }
    retile(g, COAST_ROWAN_X, COAST_ROWAN_Y, TILE_NPC_ROWAN);
    for (int y = COAST_DOCK_Y; y < COAST_DOCK_Y + TOWN_HARBOR_H; y++) {
        for (int x = COAST_DOCK_X; x < COAST_DOCK_X + TOWN_HARBOR_W; x++) {
            retile(g, x, y, TILE_WATCHTOWER);
        }
    }
    if (game_harbor_unlocked(g)) {
        for (int x = COAST_DOCK_X; x <= COAST_DOCK_ENTRY_X; x++) {
            retile(g, x, COAST_DOCK_ENTRY_Y, TILE_TOWN_PATH);
        }
    }
}

void coastal_return_from_island(GameState *g) {
    approach_enter_region(g, 3);
    coastal_refresh_docks(g);
    g->player.x = game_harbor_unlocked(g) ? COAST_DOCK_ENTRY_X : COAST_DOCK_X - 1;
    g->player.y = game_harbor_unlocked(g) ? COAST_DOCK_ENTRY_Y : COAST_DOCK_Y - 1;
    g->player.last_dx = -1;
    g->player.last_dy = 0;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    push_message(g, "The ship returns to the docks beside the Sunken Coast entrance. Oakhaven is north.");
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
