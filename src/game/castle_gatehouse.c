#include "castle_gatehouse.h"
#include "castle.h"
#include <stdlib.h>
#include <string.h>

static const Room rooms[MAX_ROOMS] = {
    {8, 6, 15, 12}, {32, 6, 22, 12}, {6, 25, 17, 13},
    {30, 25, 23, 11}, {30, 39, 12, 9}, {38, 50, 16, 8},
    {6, 44, 13, 12}, {21, 44, 13, 12}, {24, 24, 5, 9},
    {56, 23, 5, 33}
};

static void floor_rect(Map *m, int x, int y, int w, int h, TileType tile) {
    for (int ty = y; ty < y + h; ty++) {
        for (int tx = x; tx < x + w; tx++) {
            m->tiles[ty][tx] = tile;
        }
    }
}

void gatehouse_generate(Map *m) {
    memset(m, 0, sizeof(*m));
    floor_rect(m, 0, 0, MAP_W, MAP_H, TILE_CASTLE_WALL);
    m->room_count = MAX_ROOMS;
    memcpy(m->rooms, rooms, sizeof(rooms));
    for (int i = 0; i < m->room_count; i++) {
        const Room *r = &rooms[i];
        floor_rect(m, r->x, r->y, r->w, r->h, TILE_CASTLE_FLOOR);
    }
    // Narrow, offset doorways connect rooms; masonry separates the wings.
    floor_rect(m, 14, 10, 30, 3, TILE_CASTLE_CARPET);
    floor_rect(m, 14, 17, 3, 15, TILE_CASTLE_CARPET);
    floor_rect(m, 11, 37, 3, 9, TILE_CASTLE_FLOOR);
    floor_rect(m, 18, 50, 5, 2, TILE_CASTLE_FLOOR);
    floor_rect(m, 25, 11, 3, 34, TILE_CASTLE_FLOOR);
    floor_rect(m, 22, 30, 10, 2, TILE_CASTLE_FLOOR);
    floor_rect(m, 33, 42, 4, 10, TILE_CASTLE_FLOOR);
    floor_rect(m, 34, 51, 7, 2, TILE_CASTLE_FLOOR);
    floor_rect(m, 35, 35, 2, 6, TILE_CASTLE_FLOOR);
    floor_rect(m, 48, 17, 2, 7, TILE_CASTLE_FLOOR);
    floor_rect(m, 48, 22, 11, 2, TILE_CASTLE_FLOOR);
    floor_rect(m, 52, 29, 6, 2, TILE_CASTLE_FLOOR);
    floor_rect(m, 52, 53, 6, 2, TILE_CASTLE_FLOOR);
    floor_rect(m, 44, 51, 3, 6, TILE_CASTLE_CARPET);
    // Furnishings leave room to flank; service rooms have different footprints.
    for (int y = 27; y <= 33; y += 3) {
        floor_rect(m, 8, y, 2, 1, TILE_CASTLE_TABLE);
        floor_rect(m, 19, y, 2, 1, TILE_CASTLE_TABLE);
    }
    floor_rect(m, 35, 28, 2, 5, TILE_CASTLE_TABLE);
    floor_rect(m, 45, 28, 2, 5, TILE_CASTLE_TABLE);
    floor_rect(m, 32, 40, 6, 1, TILE_CASTLE_TABLE);
    floor_rect(m, 7, 45, 4, 1, TILE_CASTLE_BOOKCASE);
    floor_rect(m, 22, 45, 5, 1, TILE_CASTLE_BOOKCASE);
    floor_rect(m, 52, 8, 1, 5, TILE_CASTLE_BOOKCASE);
    m->tiles[7][10] = TILE_CASTLE_BANNER;
    m->tiles[7][20] = TILE_CASTLE_BANNER;
    m->tiles[51][40] = TILE_CASTLE_PILLAR;
    m->tiles[51][51] = TILE_CASTLE_PILLAR;
    floor_rect(m, 29, 10, 1, 3, TILE_CASTLE_GATE);
    floor_rect(m, 14, 21, 3, 1, TILE_CASTLE_GATE_OPEN);
    floor_rect(m, 25, GATEHOUSE_SHORTCUT_Y, 3, 1, TILE_CASTLE_GATE);
    m->tiles[GATEHOUSE_LEVER_Y][GATEHOUSE_LEVER_X] = TILE_CASTLE_LEVER;
    // Keep the existing entry, onward stair and upper-floor trapdoor landing.
    floor_rect(m, 23, 12, 2, 3, TILE_CASTLE_FLOOR);
    floor_rect(m, 8, 14, 3, 2, TILE_CASTLE_FLOOR);
    m->tiles[15][10] = TILE_CASTLE_LEVER;
    m->tiles[13][24] = TILE_CASTLE_LEVER;
    m->stairs_up_x = 15;
    m->stairs_up_y = 11;
    m->stairs_down_x = 45;
    m->stairs_down_y = 55;
    m->tiles[11][15] = TILE_STAIRS_DOWN;
    m->tiles[55][45] = TILE_STAIRS_UP;
}

void gatehouse_supply(GameState *g) {
    // Called only when this floor is first generated, never when defenders regroup.
    FloorItem *loot = g->castle_loot[0];
    const int positions[2][2] = {{15, 52}, {39, 46}};
    for (int i = 0; i < 2; i++) {
        int x = positions[i][0];
        int y = positions[i][1];
        loot[i] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = g->map.tiles[y][x],
            .item = i == 0 ? item_make_health_potion() : item_make_mana_potion()};
        g->map.tiles[y][x] = TILE_ITEM;
    }
    g->castle_loot_count[0] = 2;
}

int gatehouse_open_shortcut(GameState *g, int x, int y) {
    if (g->level != 1 || x != GATEHOUSE_LEVER_X || y != GATEHOUSE_LEVER_Y) {
        return 0;
    }
    for (int gx = 25; gx <= 27; gx++) {
        if (g->map.tiles[GATEHOUSE_SHORTCUT_Y][gx] != TILE_ITEM) {
            g->map.tiles[GATEHOUSE_SHORTCUT_Y][gx] = TILE_CASTLE_GATE_OPEN;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == gx && item->y == GATEHOUSE_SHORTCUT_Y) {
                item->underlying_tile = TILE_CASTLE_GATE_OPEN;
            }
        }
    }
    push_message(g, "The service portcullis locks open. The lower keep now connects directly to the entrance hall.");
    return 1;
}

int gatehouse_migrate(GameState *g) {
    if (g->castle_cache[0].valid) {
        LevelCache *cache = &g->castle_cache[0];
        if (!castle_remodel(&cache->map, cache->enemies, cache->enemy_count, g->castle_loot[0], g->castle_loot_count[0], -1, -1, 1)) {
            return 0;
        }
    }
    if (g->location == LOCATION_CASTLE_INTERIOR && g->level == 1) {
        if (!castle_remodel(&g->map, g->enemies, g->enemy_count, g->floor_items, g->floor_item_count, g->player.x, g->player.y, 1)) {
            return 0;
        }
    }
    return 1;
}
