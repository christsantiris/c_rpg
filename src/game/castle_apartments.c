#include "castle_apartments.h"
#include "castle.h"
#include <string.h>

static void fill(Map *m, int x, int y, int w, int h, TileType tile) {
    for (int ty = y; ty < y + h; ty++) {
        for (int tx = x; tx < x + w; tx++) {
            m->tiles[ty][tx] = tile;
        }
    }
}

void apartments_generate(Map *m) {
    static const Room rooms[MAX_ROOMS] = {
        {8, 6, 15, 12}, {6, 26, 17, 13}, {29, 24, 25, 15},
        {34, 5, 22, 12}, {6, 46, 16, 12}, {38, 50, 16, 8},
        {27, 43, 9, 13}, {6, 19, 12, 5}, {56, 23, 6, 24},
        {25, 18, 16, 4}
    };
    memset(m, 0, sizeof(*m));
    fill(m, 0, 0, MAP_W, MAP_H, TILE_CASTLE_WALL);
    m->room_count = MAX_ROOMS;
    memcpy(m->rooms, rooms, sizeof(rooms));
    for (int i = 0; i < MAX_ROOMS; i++) {
        const Room *r = &rooms[i];
        fill(m, r->x, r->y, r->w, r->h, TILE_CASTLE_FLOOR);
    }
    fill(m, 14, 10, 32, 3, TILE_CASTLE_CARPET);
    fill(m, 23, 12, 2, 3, TILE_CASTLE_FLOOR);
    fill(m, 10, 17, 2, 11, TILE_CASTLE_FLOOR);
    fill(m, 12, 38, 2, 11, TILE_CASTLE_FLOOR);
    fill(m, 21, 51, 23, 2, TILE_CASTLE_FLOOR);
    fill(m, 22, 31, 9, 2, TILE_CASTLE_FLOOR);
    fill(m, 46, 16, 2, 10, TILE_CASTLE_FLOOR);
    fill(m, 40, 21, 2, 5, TILE_CASTLE_FLOOR);
    fill(m, 44, 38, 3, 15, TILE_CASTLE_CARPET);
    fill(m, 53, 31, 6, 2, TILE_CASTLE_FLOOR);
    fill(m, 58, 46, 2, 9, TILE_CASTLE_FLOOR);
    fill(m, 53, 53, 7, 2, TILE_CASTLE_FLOOR);
    fill(m, 26, 11, 2, 43, TILE_CASTLE_FLOOR);
    fill(m, 44, 51, 3, 6, TILE_CASTLE_CARPET);
    fill(m, 8, 28, 2, 1, TILE_CASTLE_TABLE);
    fill(m, 19, 35, 2, 1, TILE_CASTLE_TABLE);
    fill(m, 8, 48, 2, 1, TILE_CASTLE_TABLE);
    fill(m, 18, 54, 2, 1, TILE_CASTLE_TABLE);
    for (int x = 31; x <= 51; x += 3) {
        m->tiles[26][x] = TILE_CASTLE_BOOKCASE;
        m->tiles[36][x] = TILE_CASTLE_BOOKCASE;
    }
    for (int y = 28; y <= 32; y++) {
        if (y != 30) {
            m->tiles[y][34] = TILE_CASTLE_BOOKCASE;
            m->tiles[y][50] = TILE_CASTLE_BOOKCASE;
        }
    }
    fill(m, 36, 6, 15, 1, TILE_CASTLE_BOOKCASE);
    fill(m, 29, 44, 5, 1, TILE_CASTLE_BOOKCASE);
    m->tiles[27][12] = TILE_CASTLE_BOOKCASE;
    fill(m, 38, 34, 11, 1, TILE_CASTLE_FIRE_RUNE);
    fill(m, 44, 42, 3, 1, TILE_CASTLE_FIRE_RUNE);
    fill(m, 29, 10, 1, 3, TILE_CASTLE_GATE);
    m->tiles[13][24] = TILE_CASTLE_LEVER;
    fill(m, 26, APARTMENTS_GATE_Y, 2, 1, TILE_CASTLE_GATE);
    m->tiles[APARTMENTS_LEVER_Y][APARTMENTS_LEVER_X] = TILE_CASTLE_LEVER;
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

void apartments_supply(GameState *g) {
    const int positions[3][2] = {{15, 36}, {48, 13}, {31, 52}};
    for (int i = 0; i < 3; i++) {
        int x = positions[i][0];
        int y = positions[i][1];
        Item item = i == 0 ? item_make_health_potion() : i == 1 ? item_make_strength_potion() : item_make_mana_potion();
        g->castle_loot[4][i] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = g->map.tiles[y][x], .item = item};
        g->map.tiles[y][x] = TILE_ITEM;
    }
    g->castle_loot_count[4] = 3;
}

int apartments_interact(GameState *g, int x, int y) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 5) {
        return 0;
    }
    if (x == 12 && y == 27) {
        push_message(g, "Royal bedside orders: The King's attendants were dismissed. All medicines and audiences require Malakar's seal.");
        return 1;
    }
    if (x == 40 && y == 26) {
        push_message(g, "Crown diagrams: the crown binds its wearer and feeds power to Malakar. Its conduits lead to the throne chamber.");
        return 1;
    }
    if (x == 31 && y == 44) {
        push_message(g, "Recovery ledger: Malakar drank the restoratives. The crown will restore him to full strength in the King's chamber.");
        push_message(g, "Empty vessels offer no way to weaken him. Reach the final confrontation upstairs; the household passage shortens retreat.");
        return 1;
    }
    if (x != APARTMENTS_LEVER_X || y != APARTMENTS_LEVER_Y) {
        return 0;
    }
    for (int gx = 26; gx <= 27; gx++) {
        if (g->map.tiles[APARTMENTS_GATE_Y][gx] != TILE_ITEM) {
            g->map.tiles[APARTMENTS_GATE_Y][gx] = TILE_CASTLE_GATE_OPEN;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == gx && item->y == APARTMENTS_GATE_Y) {
                item->underlying_tile = TILE_CASTLE_GATE_OPEN;
            }
        }
    }
    push_message(g, "The household passage locks open, connecting the royal suites directly to the receiving hall.");
    return 1;
}

int apartments_migrate(GameState *g) {
    if (g->castle_cache[4].valid) {
        LevelCache *c = &g->castle_cache[4];
        if (!castle_remodel(&c->map, c->enemies, c->enemy_count, g->castle_loot[4], g->castle_loot_count[4], -1, -1, 5)) {
            return 0;
        }
    }
    if (g->location == LOCATION_CASTLE_INTERIOR && g->level == 5) {
        if (!castle_remodel(&g->map, g->enemies, g->enemy_count, g->floor_items, g->floor_item_count, g->player.x, g->player.y, 5)) {
            return 0;
        }
    }
    return 1;
}
