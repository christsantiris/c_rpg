#include "heartseed.h"
#include "elderwood.h"
#include <string.h>

// Legacy geometry is retained only when repairing old saved clearings.
void heartseed_carve_grove(Map *map) {
    // Extend only the eastern clearing, retaining cut paths, logs, actors and loot.
    for (int y = 28; y <= 40; y++) {
        for (int x = 47; x <= 59; x++) {
            if (map->tiles[y][x] == TILE_ELDERWOOD_WALL && elderwood_in_clearing(x, y, 47, 28, 59, 40)) {
                map->tiles[y][x] = TILE_ELDERWOOD_FLOOR;
            }
        }
    }
    // A short trail joins the rounded grove to the northern flank.
    for (int y = 28; y <= 30; y++) {
        if (map->tiles[y][58] == TILE_ELDERWOOD_WALL) {
            map->tiles[y][58] = TILE_ELDERWOOD_FLOOR;
        }
    }
    // A bramble passage beside the permanent southern route offers a flank.
    for (int x = 53; x <= 58; x++) {
        if (map->tiles[25][x] == TILE_ELDERWOOD_WALL) {
            map->tiles[25][x] = TILE_ELDERWOOD_FLOOR;
        }
    }
    for (int y = 26; y <= 28; y++) {
        if (map->tiles[y][58] == TILE_ELDERWOOD_WALL) {
            map->tiles[y][58] = TILE_ELDERWOOD_BRAMBLE;
        }
    }
}

static TileType retired_tile(TileType tile) {
    if (tile == TILE_HEARTSEED_ORDERS || tile == TILE_HEARTSEED_STAKE ||
        tile == TILE_HEARTSEED_CUT_STAKE || tile == TILE_HEARTSEED_ALTAR) {
        return TILE_ELDERWOOD_FLOOR;
    }
    return tile;
}

static void retire_names(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        Enemy *enemy = &enemies[i];
        if (strncmp(enemy->name, "Heartseed ", 10) == 0 || strcmp(enemy->name, "Bound Treefolk") == 0) {
            snprintf(enemy->name, sizeof(enemy->name), "%s", enemy->type == ENEMY_TREEFOLK ? "Treefolk" : "Dark Elf Archer");
        }
    }
}

void heartseed_migrate(GameState *g) {
    if (strcmp(g->dialogue_speaker, "Forester Tamsin") == 0) {
        g->dialogue_active = 0;
    }
    if (g->location == LOCATION_TOWN && g->map.tiles[TAMSIN_Y][TAMSIN_X] == TILE_NPC_TAMSIN) {
        g->map.tiles[TAMSIN_Y][TAMSIN_X] = TILE_TOWN_FLOOR;
    }
    for (int i = 0; i < ELDERWOOD_DEPTH; i++) {
        ElderwoodCache *cache = &g->elderwood_cache[i];
        if (!cache->valid) {
            continue;
        }
        for (int y = 0; y < ELDERWOOD_H; y++) {
            for (int x = 0; x < ELDERWOOD_W; x++) {
                cache->tiles[y][x] = retired_tile(cache->tiles[y][x]);
            }
        }
        retire_names(cache->enemies, cache->enemy_count);
    }
    if (g->location == LOCATION_ELDERWOOD) {
        for (int y = 0; y < ELDERWOOD_H; y++) {
            for (int x = 0; x < ELDERWOOD_W; x++) {
                g->map.tiles[y][x] = retired_tile(g->map.tiles[y][x]);
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            g->floor_items[i].underlying_tile = retired_tile(g->floor_items[i].underlying_tile);
        }
        retire_names(g->enemies, g->enemy_count);
    }
    if (g->portal_location == LOCATION_ELDERWOOD) {
        g->portal_origin_tile = retired_tile(g->portal_origin_tile);
    }
}
