#include "castle_finale.h"
#include "castle.h"
#include "aldric.h"
#include "storm_mage.h"
#include "garran.h"
#include <string.h>

static void fill(Map *m, int x, int y, int w, int h, TileType tile) {
    for (int ty = y; ty < y + h; ty++) {
        for (int tx = x; tx < x + w; tx++) {
            m->tiles[ty][tx] = tile;
        }
    }
}

void finale_generate(Map *m) {
    static const Room rooms[8] = {{8, 6, 15, 12}, {30, 6, 23, 12}, {6, 25, 14, 14}, {30, 25, 24, 12},
        {6, 45, 20, 13}, {34, 44, 22, 14}, {56, 23, 6, 34}, {24, 19, 14, 4}};
    memset(m, 0, sizeof(*m));
    fill(m, 0, 0, MAP_W, MAP_H, TILE_CASTLE_WALL);
    m->room_count = 8;
    memcpy(m->rooms, rooms, sizeof(rooms));
    for (int i = 0; i < 8; i++) {
        const Room *r = &rooms[i];
        fill(m, r->x, r->y, r->w, r->h, TILE_CASTLE_FLOOR);
    }
    fill(m, 14, 10, 30, 3, TILE_CASTLE_CARPET);
    fill(m, 23, 12, 2, 3, TILE_CASTLE_FLOOR);
    fill(m, 10, 17, 2, 10, TILE_CASTLE_FLOOR);
    fill(m, 12, 38, 2, 10, TILE_CASTLE_FLOOR);
    fill(m, 25, 50, 11, 2, TILE_CASTLE_FLOOR);
    fill(m, 40, 17, 3, 10, TILE_CASTLE_CARPET);
    fill(m, 44, 36, 3, 10, TILE_CASTLE_CARPET);
    fill(m, 52, 10, 7, 2, TILE_CASTLE_FLOOR);
    fill(m, 57, 11, 2, 44, TILE_CASTLE_FLOOR);
    fill(m, 53, 53, 6, 2, TILE_CASTLE_FLOOR);
    fill(m, 26, 11, 2, 42, TILE_CASTLE_FLOOR);
    fill(m, 34, 28, 2, 6, TILE_CASTLE_TABLE);
    fill(m, 49, 28, 2, 6, TILE_CASTLE_TABLE);
    fill(m, 8, 47, 8, 1, TILE_CASTLE_BOOKCASE);
    fill(m, 32, 7, 9, 1, TILE_CASTLE_BOOKCASE);
    fill(m, 29, 10, 1, 3, TILE_CASTLE_GATE);
    m->tiles[13][24] = TILE_CASTLE_LEVER;
    fill(m, 26, 40, 2, 1, TILE_CASTLE_GATE);
    m->tiles[48][27] = TILE_CASTLE_LEVER;
    fill(m, 43, 44, 4, 14, TILE_CASTLE_CARPET);
    fill(m, 34, 50, 12, 3, TILE_CASTLE_CARPET);
    m->tiles[45][36] = TILE_CASTLE_BANNER;
    m->tiles[45][53] = TILE_CASTLE_BANNER;
    m->tiles[48][38] = TILE_CASTLE_PILLAR;
    m->tiles[48][51] = TILE_CASTLE_PILLAR;
    fill(m, 40, 47, 1, 3, TILE_CASTLE_GATE_OPEN);
    fill(m, 50, 47, 1, 3, TILE_CASTLE_GATE);
    m->tiles[54][37] = TILE_CASTLE_WARD;
    m->tiles[54][53] = TILE_CASTLE_WARD;
    m->tiles[46][53] = TILE_CASTLE_MUSTER;
    fill(m, 41, 51, 9, 1, TILE_CASTLE_FIRE_RUNE);
    m->tiles[45][45] = TILE_CASTLE_THRONE;
    m->stairs_up_x = 15;
    m->stairs_up_y = 11;
    m->stairs_down_x = 45;
    m->stairs_down_y = 55;
    m->tiles[11][15] = TILE_STAIRS_DOWN;
}

int finale_interact(GameState *g, int x, int y) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 6 || x != 27 || y != 48) {
        return 0;
    }
    for (int gx = 26; gx <= 27; gx++) {
        if (g->map.tiles[40][gx] != TILE_ITEM) {
            g->map.tiles[40][gx] = TILE_CASTLE_GATE_OPEN;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *f = &g->floor_items[i];
            if (f->active && f->x == gx && f->y == 40) {
                f->underlying_tile = TILE_CASTLE_GATE_OPEN;
            }
        }
    }
    push_message(g, "The royal passage locks open. Retreat to the apartments remains available.");
    return 1;
}

int finale_pending(const GameState *g) {
    return g->game_won && g->castle_ending > 0 && g->castle_ending < 3;
}

void finale_sync(GameState *g) {
    if (!finale_pending(g)) {
        return;
    }
    g->dialogue_active = 1;
    g->dialogue_x = g->player.x;
    g->dialogue_y = g->player.y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", g->castle_ending == 1 ? "The King" : "The Kingdom Freed");
    if (g->castle_ending == 1) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text), "The crown falls silent. The King rises, freed from Malakar's binding.\nYou broke his hold over me and ended the terror I could not stop. The realm owes you its freedom.\nEnter / Esc continues.");
    } else {
        const char *aldric = g->aldric_party_state == ALDRIC_DEAD ? "Aldric's sacrifice is honored." : g->aldric_party_state == ALDRIC_FOLLOWING ? "Aldric stands beside you." : "The household honors your courage.";
        const char *soren = g->storm_mage.recruited ? (g->storm_mage.state == STORM_MAGE_DEAD ? "Soren is remembered." : "Soren is free of Malakar's reach.") : "The castle's prisoners are freed.";
        const char *garran = g->garran.recruited ? (g->garran.state == GARRAN_DEAD ? "Garran is remembered." : "Garran can return to the open road.") : "The roads can reopen.";
        snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s\n%s\n%s\nThe King pledges to repair the harm done in his name. Crown of the Unbound earned.\nEnter / Esc: victory.", aldric, soren, garran);
    }
}

int finale_handle_key(GameState *g, int key, int repeat) {
    if (!finale_pending(g)) {
        return 0;
    }
    if (!repeat && (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER || key == SDL_SCANCODE_ESCAPE)) {
        g->castle_ending++;
        g->dialogue_active = 0;
        finale_sync(g);
    }
    return 1;
}

static int migrate_floor(Map *m, Enemy *actors, int count, FloorItem *loot, int items, int px, int py) {
    TileType arena[14][22];
    int underlays[MAX_FLOOR_ITEMS];
    for (int y = 0; y < 14; y++) {
        memcpy(arena[y], &m->tiles[y + 44][34], sizeof(arena[y]));
    }
    for (int i = 0; i < items; i++) {
        underlays[i] = loot[i].underlying_tile;
    }
    if (!castle_remodel(m, actors, count, loot, items, px, py, 6)) {
        return 0;
    }
    for (int y = 0; y < 14; y++) {
        memcpy(&m->tiles[y + 44][34], arena[y], sizeof(arena[y]));
    }
    for (int i = 0; i < items; i++) {
        int arena_loot = loot[i].x >= 34 && loot[i].x < 56 && loot[i].y >= 44 && loot[i].y < 58;
        int shared_gate = (loot[i].x == 29 && loot[i].y >= 10 && loot[i].y <= 12) ||
            (loot[i].y == 40 && (loot[i].x == 26 || loot[i].x == 27));
        int gate_loot = shared_gate && (underlays[i] == TILE_CASTLE_GATE || underlays[i] == TILE_CASTLE_GATE_OPEN);
        if (arena_loot || gate_loot) {
            loot[i].underlying_tile = underlays[i];
        }
    }
    for (int i = 0; i < count; i++) {
        if (actors[i].type == ENEMY_MALAKAR_FINAL) {
            snprintf(actors[i].name, sizeof(actors[i].name), "Malakar, Crown Alchemist");
        }
    }
    return 1;
}

int finale_migrate(GameState *g) {
    if (g->castle_cache[5].valid) {
        LevelCache *c = &g->castle_cache[5];
        if (!migrate_floor(&c->map, c->enemies, c->enemy_count, g->castle_loot[5], g->castle_loot_count[5], -1, -1)) {
            return 0;
        }
    }
    if (g->location == LOCATION_CASTLE_INTERIOR && g->level == 6) {
        if (!migrate_floor(&g->map, g->enemies, g->enemy_count, g->floor_items, g->floor_item_count, g->player.x, g->player.y)) {
            return 0;
        }
    }
    return 1;
}
