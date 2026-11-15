#include "castle_chapel.h"
#include "castle.h"
#include <string.h>

static void fill(Map *m, int x, int y, int w, int h, TileType tile) {
    for (int ty = y; ty < y + h; ty++) {
        for (int tx = x; tx < x + w; tx++) {
            m->tiles[ty][tx] = tile;
        }
    }
}

void chapel_generate(Map *m) {
    static const Room rooms[MAX_ROOMS] = {
        {8, 6, 15, 12}, {24, 24, 30, 13}, {6, 25, 14, 13},
        {31, 5, 22, 12}, {6, 43, 18, 15}, {34, 44, 22, 14},
        {26, 41, 6, 15}, {6, 19, 10, 4}, {56, 23, 6, 15},
        {25, 18, 12, 4}
    };
    memset(m, 0, sizeof(*m));
    fill(m, 0, 0, MAP_W, MAP_H, TILE_CASTLE_WALL);
    m->room_count = MAX_ROOMS;
    memcpy(m->rooms, rooms, sizeof(rooms));
    for (int i = 0; i < MAX_ROOMS; i++) {
        const Room *r = &rooms[i];
        fill(m, r->x, r->y, r->w, r->h, TILE_CASTLE_FLOOR);
    }
    fill(m, 14, 10, 30, 3, TILE_CASTLE_CARPET);
    fill(m, 23, 12, 2, 3, TILE_CASTLE_FLOOR);
    fill(m, 10, 17, 2, 10, TILE_CASTLE_FLOOR);
    fill(m, 19, 30, 7, 2, TILE_CASTLE_FLOOR);
    fill(m, 11, 37, 2, 9, TILE_CASTLE_FLOOR);
    fill(m, 23, 50, 13, 2, TILE_CASTLE_FLOOR);
    fill(m, 41, 16, 3, 10, TILE_CASTLE_CARPET);
    fill(m, 38, 24, 4, 13, TILE_CASTLE_CARPET);
    fill(m, 44, 36, 3, 11, TILE_CASTLE_CARPET);
    fill(m, 53, 29, 5, 2, TILE_CASTLE_FLOOR);
    fill(m, 52, 10, 7, 2, TILE_CASTLE_FLOOR);
    fill(m, 57, 11, 2, 14, TILE_CASTLE_FLOOR);
    fill(m, 57, 37, 2, 18, TILE_CASTLE_FLOOR);
    fill(m, 53, 53, 6, 2, TILE_CASTLE_FLOOR);
    fill(m, 26, 11, 2, 41, TILE_CASTLE_FLOOR);
    for (int y = 27; y <= 33; y += 3) {
        fill(m, 29, y, 6, 1, TILE_CASTLE_TABLE);
        fill(m, 45, y, 6, 1, TILE_CASTLE_TABLE);
    }
    fill(m, 33, 6, 7, 1, TILE_CASTLE_BOOKCASE);
    fill(m, 8, 26, 7, 1, TILE_CASTLE_BOOKCASE);
    m->tiles[34][9] = TILE_CASTLE_PILLAR;
    m->tiles[45][8] = TILE_CASTLE_PILLAR;
    m->tiles[55][21] = TILE_CASTLE_PILLAR;
    m->tiles[13][24] = TILE_CASTLE_LEVER;
    fill(m, 29, 10, 1, 3, TILE_CASTLE_GATE);
    fill(m, CHAPEL_GATE_X, CHAPEL_GATE_Y, 2, 1, TILE_CASTLE_GATE);
    m->tiles[CHAPEL_LEVER_Y][CHAPEL_LEVER_X] = TILE_CASTLE_LEVER;
    // Keep the established Arcanist arena and all attack-source coordinates.
    fill(m, 43, 44, 4, 14, TILE_CASTLE_CARPET);
    m->tiles[45][36] = TILE_CASTLE_BANNER;
    m->tiles[45][53] = TILE_CASTLE_BANNER;
    m->tiles[48][38] = TILE_CASTLE_PILLAR;
    m->tiles[48][51] = TILE_CASTLE_PILLAR;
    fill(m, 39, 50, 1, 3, TILE_CASTLE_TABLE);
    fill(m, 50, 50, 1, 3, TILE_CASTLE_TABLE);
    fill(m, 40, 47, 1, 3, TILE_CASTLE_GATE_OPEN);
    fill(m, 50, 47, 1, 3, TILE_CASTLE_GATE);
    m->tiles[54][37] = TILE_CASTLE_WARD;
    m->tiles[54][53] = TILE_CASTLE_WARD;
    m->stairs_up_x = 15;
    m->stairs_up_y = 11;
    m->stairs_down_x = 45;
    m->stairs_down_y = 55;
    m->tiles[11][15] = TILE_STAIRS_DOWN;
    m->tiles[55][45] = TILE_STAIRS_UP;
    m->burial_trap_count = 1;
    m->burial_traps[0] = (BurialTrap){17, 32, 0, 0, 0};
    m->tiles[32][17] = TILE_CASTLE_TRAP_HIDDEN;
}

int chapel_interact(GameState *g, int x, int y) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 4) {
        return 0;
    }
    if (x == 12 && y == 26) {
        push_message(g, "Chapel register: Malakar ordered the crown brought upstairs and placed the royal apartments under the Arcanist's wards.");
        push_message(g, "A keeper's note marks the cloister and choir gallery as alternate approaches. The sacristy lever opens the old royal service passage.");
        return 1;
    }
    if (x != CHAPEL_LEVER_X || y != CHAPEL_LEVER_Y) {
        return 0;
    }
    for (int gx = CHAPEL_GATE_X; gx <= CHAPEL_GATE_X + 1; gx++) {
        if (g->map.tiles[CHAPEL_GATE_Y][gx] != TILE_ITEM) {
            g->map.tiles[CHAPEL_GATE_Y][gx] = TILE_CASTLE_GATE_OPEN;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == gx && item->y == CHAPEL_GATE_Y) {
                item->underlying_tile = TILE_CASTLE_GATE_OPEN;
            }
        }
    }
    push_message(g, "The royal service gate locks open. The sacristy now has a direct retreat to the entrance hall.");
    return 1;
}

static int migrate_floor(Map *m, Enemy *actors, int count, FloorItem *loot, int items, int px, int py) {
    TileType arena[14][22];
    int underlays[MAX_FLOOR_ITEMS];
    for (int y = 0; y < 14; y++) {
        memcpy(arena[y], &m->tiles[44 + y][34], sizeof(arena[y]));
    }
    for (int i = 0; i < items; i++) {
        underlays[i] = loot[i].underlying_tile;
    }
    if (!castle_remodel(m, actors, count, loot, items, px, py, 4)) {
        return 0;
    }
    // A saved fight resumes against exactly the same cover, wards and gates.
    for (int y = 0; y < 14; y++) {
        memcpy(&m->tiles[44 + y][34], arena[y], sizeof(arena[y]));
    }
    for (int i = 0; i < items; i++) {
        if (loot[i].x >= 34 && loot[i].x < 56 && loot[i].y >= 44 && loot[i].y < 58) {
            loot[i].underlying_tile = underlays[i];
        }
    }
    return 1;
}

int chapel_migrate(GameState *g) {
    if (g->castle_cache[3].valid) {
        LevelCache *c = &g->castle_cache[3];
        if (!migrate_floor(&c->map, c->enemies, c->enemy_count, g->castle_loot[3], g->castle_loot_count[3], -1, -1)) {
            return 0;
        }
    }
    if (g->location == LOCATION_CASTLE_INTERIOR && g->level == 4) {
        if (!migrate_floor(&g->map, g->enemies, g->enemy_count, g->floor_items, g->floor_item_count, g->player.x, g->player.y)) {
            return 0;
        }
    }
    return 1;
}
