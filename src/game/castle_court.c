#include "castle_court.h"
#include "castle.h"
#include "royal_evidence.h"
#include <stdlib.h>
#include <string.h>

static void fill(Map *m, int x, int y, int w, int h, TileType tile) {
    for (int ty = y; ty < y + h; ty++) {
        for (int tx = x; tx < x + w; tx++) {
            m->tiles[ty][tx] = tile;
        }
    }
}

void court_generate(Map *m) {
    static const Room rooms[MAX_ROOMS] = {
        {8, 6, 15, 12}, {22, 23, 22, 18}, {7, 22, 8, 22},
        {25, 5, 20, 12}, {48, 5, 12, 13}, {38, 50, 16, 8},
        {6, 47, 15, 11}, {24, 45, 10, 11}, {47, 23, 13, 13},
        {48, 39, 11, 8}
    };
    memset(m, 0, sizeof(*m));
    fill(m, 0, 0, MAP_W, MAP_H, TILE_CASTLE_WALL);
    m->room_count = MAX_ROOMS;
    memcpy(m->rooms, rooms, sizeof(rooms));
    for (int i = 0; i < MAX_ROOMS; i++) {
        const Room *r = &rooms[i];
        fill(m, r->x, r->y, r->w, r->h, TILE_CASTLE_FLOOR);
    }
    // The main stair approach enters the hall from the north.
    fill(m, 14, 10, 22, 3, TILE_CASTLE_CARPET);
    fill(m, 14, 17, 3, 4, TILE_CASTLE_CARPET);
    fill(m, 14, 19, 21, 3, TILE_CASTLE_CARPET);
    fill(m, 32, 20, 3, 21, TILE_CASTLE_CARPET);
    fill(m, 41, 40, 3, 13, TILE_CASTLE_CARPET);
    fill(m, 44, 51, 3, 6, TILE_CASTLE_CARPET);
    // Western service route and the lower household loop.
    fill(m, 10, 17, 2, 7, TILE_CASTLE_FLOOR);
    fill(m, 10, 43, 2, 6, TILE_CASTLE_FLOOR);
    fill(m, 14, 31, 10, 2, TILE_CASTLE_FLOOR);
    fill(m, 19, 11, 2, 41, TILE_CASTLE_FLOOR);
    fill(m, 20, 51, 20, 2, TILE_CASTLE_FLOOR);
    fill(m, 28, 40, 2, 7, TILE_CASTLE_FLOOR);
    // Guest chambers and the eastern gallery flank the hall's defenders.
    fill(m, 44, 10, 6, 2, TILE_CASTLE_FLOOR);
    fill(m, 57, 17, 2, 8, TILE_CASTLE_FLOOR);
    fill(m, 43, 29, 6, 2, TILE_CASTLE_FLOOR);
    fill(m, 52, 35, 2, 6, TILE_CASTLE_FLOOR);
    fill(m, 52, 46, 2, 7, TILE_CASTLE_FLOOR);
    // Furnishings establish the occupied household without closing the aisles.
    fill(m, 27, 27, 2, 10, TILE_CASTLE_TABLE);
    fill(m, 38, 27, 2, 10, TILE_CASTLE_TABLE);
    fill(m, 24, 25, 1, 2, TILE_CASTLE_PILLAR);
    fill(m, 41, 36, 1, 2, TILE_CASTLE_PILLAR);
    fill(m, 30, 8, 10, 1, TILE_CASTLE_TABLE);
    fill(m, 27, 6, 5, 1, TILE_CASTLE_BOOKCASE);
    fill(m, 50, 7, 2, 1, TILE_CASTLE_TABLE);
    fill(m, 56, 13, 2, 1, TILE_CASTLE_TABLE);
    fill(m, 8, 49, 2, 1, TILE_CASTLE_TABLE);
    fill(m, 8, 54, 2, 1, TILE_CASTLE_TABLE);
    fill(m, 25, 46, 5, 1, TILE_CASTLE_BOOKCASE);
    fill(m, 49, 24, 4, 1, TILE_CASTLE_BOOKCASE);
    m->tiles[24][25] = TILE_CASTLE_BANNER;
    m->tiles[24][40] = TILE_CASTLE_BANNER;
    m->tiles[25][54] = TILE_CASTLE_CHAMBERLAIN;
    fill(m, COURT_GATE_X, COURT_GATE_Y, 2, 1, TILE_CASTLE_GATE);
    m->tiles[COURT_LEVER_Y][COURT_LEVER_X] = TILE_CASTLE_LEVER;
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

int court_interact(GameState *g, int x, int y) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 3) {
        return 0;
    }
    if (x == 29 && y == 6) {
        push_message(g, "Council orders: Malakar's office has suspended royal audiences. Household staff are barred from the King's apartments.");
        push_message(g, "An unsigned margin note reads: The chapel wards answer to him now. Use the household passages to avoid the hall's firing lanes.");
        return 1;
    }
    if (x != COURT_LEVER_X || y != COURT_LEVER_Y) {
        return 0;
    }
    for (int gx = COURT_GATE_X; gx <= COURT_GATE_X + 1; gx++) {
        if (g->map.tiles[COURT_GATE_Y][gx] != TILE_ITEM) {
            g->map.tiles[COURT_GATE_Y][gx] = TILE_CASTLE_GATE_OPEN;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *loot = &g->floor_items[i];
            if (loot->active && loot->x == gx && loot->y == COURT_GATE_Y) {
                loot->underlying_tile = TILE_CASTLE_GATE_OPEN;
            }
        }
    }
    push_message(g, "The household gate locks open. Its service passage leads directly back to the receiving hall.");
    return 1;
}

int court_talk(GameState *g, int x, int y) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 3 ||
        x < 0 || y < 0 || x >= MAP_W || y >= MAP_H ||
        abs(g->player.x - x) + abs(g->player.y - y) > 1 ||
        g->map.tiles[y][x] != TILE_CASTLE_CHAMBERLAIN) {
        return 0;
    }
    int evidence = royal_evidence_found(g);
    const char *text;
    if (evidence & (ROYAL_EVIDENCE_BINDING | ROYAL_EVIDENCE_CROWN | ROYAL_EVIDENCE_REQUISITIONS)) {
        text = "Your records explain the orders we received. Malakar's attendants control every royal audience, and the King wears the crown even in his chamber. Reach the Crown Chapel; its wards protect the royal apartments. Our service rooms let you skirt the Great Hall's defenders.";
    } else if (evidence & (ROYAL_EVIDENCE_WARD_ORDERS | ROYAL_EVIDENCE_FORGE_LEDGER)) {
        text = "Rosemoor's records match the commissions sent upstairs. Malakar took charge of the crown and dismissed the King's attendants. The Crown Chapel wards protect the royal apartments. I hid the household in the service rooms; their lever opens a shorter retreat.";
    } else {
        text = "I am Chamberlain Edras. Malakar dismissed the royal attendants and sealed the King's apartments. I sheltered the household here. The western service rooms and eastern guest chambers both skirt the Great Hall. The chapel stair lies beyond it; our household lever opens a shorter retreat.";
    }
    g->dialogue_active = 1;
    g->dialogue_x = x;
    g->dialogue_y = y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Chamberlain Edras");
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s\nEnter / Esc closes.", text);
    return 1;
}

int court_dialogue(const GameState *g) {
    return g->location == LOCATION_CASTLE_INTERIOR && g->level == 3 && g->dialogue_active &&
        strcmp(g->dialogue_speaker, "Chamberlain Edras") == 0;
}

int court_handle_key(GameState *g, int key, int repeat) {
    if (!court_dialogue(g)) {
        return 0;
    }
    if (!repeat && (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER || key == SDL_SCANCODE_ESCAPE)) {
        g->dialogue_active = 0;
    }
    return 1;
}

static int restore_chamberlain(Map *m, const Enemy *actors, int count, const FloorItem *loot, int items, int px, int py) {
    int bx = -1;
    int by = -1;
    int best = CASTLE_W + CASTLE_H;
    for (int y = 23; y < 36; y++) {
        for (int x = 47; x < 60; x++) {
            if (m->tiles[y][x] == TILE_CASTLE_CHAMBERLAIN) {
                return 1;
            }
            if (!map_is_walkable(m, x, y) || (x == px && y == py)) {
                continue;
            }
            int occupied = 0;
            for (int i = 0; i < count; i++) {
                occupied |= actors[i].x == x && actors[i].y == y;
            }
            for (int i = 0; i < items; i++) {
                occupied |= loot[i].active && loot[i].x == x && loot[i].y == y;
            }
            int distance = abs(x - 54) + abs(y - 25);
            if (!occupied && distance < best) {
                bx = x;
                by = y;
                best = distance;
            }
        }
    }
    if (bx < 0) {
        return 0;
    }
    m->tiles[by][bx] = TILE_CASTLE_CHAMBERLAIN;
    return 1;
}

int court_migrate(GameState *g) {
    if (g->castle_cache[2].valid) {
        LevelCache *cache = &g->castle_cache[2];
        if (!castle_remodel(&cache->map, cache->enemies, cache->enemy_count, g->castle_loot[2], g->castle_loot_count[2], -1, -1, 3) ||
            !restore_chamberlain(&cache->map, cache->enemies, cache->enemy_count, g->castle_loot[2], g->castle_loot_count[2], -1, -1)) {
            return 0;
        }
    }
    if (g->location == LOCATION_CASTLE_INTERIOR && g->level == 3) {
        if (!castle_remodel(&g->map, g->enemies, g->enemy_count, g->floor_items, g->floor_item_count, g->player.x, g->player.y, 3) ||
            !restore_chamberlain(&g->map, g->enemies, g->enemy_count, g->floor_items, g->floor_item_count, g->player.x, g->player.y)) {
            return 0;
        }
    }
    return 1;
}
