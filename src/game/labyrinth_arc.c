#include "labyrinth_arc.h"
#include "approach.h"
#include "town_road.h"
#include <stdio.h>
#include <string.h>

static void road_tile(GameState *g, int x, int y, TileType tile) {
    g->map.tiles[y][x] = tile;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *loot = &g->floor_items[i];
        if (loot->active && loot->x == x && loot->y == y) {
            loot->underlying_tile = tile;
            if (!map_is_travel_tile(tile)) {
                g->map.tiles[y][x] = TILE_ITEM;
            }
        }
    }
}

void labyrinth_prepare_road(GameState *g) {
    if (g->location != LOCATION_APPROACH || g->approach_route != APPROACH_QUARRY_ROAD) {
        return;
    }
    for (int y = QUARRY_LABYRINTH_Y - 2; y <= QUARRY_LABYRINTH_Y; y++) {
        for (int x = QUARRY_LABYRINTH_X - 1; x <= QUARRY_LABYRINTH_X + 1; x++) {
            road_tile(g, x, y, TILE_WALL);
        }
    }
    for (int x = QUARRY_LABYRINTH_X; x <= APPROACH_W / 2; x++) {
        road_tile(g, x, QUARRY_LABYRINTH_Y + 1, TILE_TOWN_PATH);
    }
    road_tile(g, QUARRY_LABYRINTH_X, QUARRY_LABYRINTH_Y, TILE_LABYRINTH_ENTRANCE);
    // Entrance and the main junction are discoverable independently of Rook's quest.
    map_mark_explored(&g->map, QUARRY_LABYRINTH_X, QUARRY_LABYRINTH_Y);
    map_mark_explored(&g->map, g->map.stairs_down_x, g->map.stairs_down_y);
}

static TileType retired_tile(TileType tile, const Map *map, int x, int y) {
    if (tile == TILE_LABYRINTH_SWITCH_OFF || tile == TILE_LABYRINTH_SWITCH_ON ||
        tile == TILE_LABYRINTH_GATE || tile == TILE_LABYRINTH_RELIC ||
        (tile == TILE_LABYRINTH_EXIT && (x != map->stairs_up_x || y != map->stairs_up_y)) ||
        (tile == TILE_LABYRINTH_STAIRS && (x != map->stairs_down_x || y != map->stairs_down_y))) {
        return TILE_LABYRINTH_FLOOR;
    }
    return tile;
}

void labyrinth_simplify_map(Map *map, int level) {
    (void)level;
    for (int y = 0; y < LABYRINTH_H; y++) {
        for (int x = 0; x < LABYRINTH_W; x++) {
            map->tiles[y][x] = retired_tile(map->tiles[y][x], map, x, y);
        }
    }
    if (map->tiles[LABYRINTH_FALSE_EXIT_Y][LABYRINTH_FALSE_EXIT_X + 2] == TILE_TRAP_REVEALED) {
        map->tiles[LABYRINTH_FALSE_EXIT_Y][LABYRINTH_FALSE_EXIT_X + 2] = TILE_LABYRINTH_FLOOR;
    }
}

void labyrinth_complete_quest(GameState *g) {
    if (!(g->defeated_bosses & (1u << LOCATION_LABYRINTH)) ||
        (g->rook_quest_state != 1 && g->rook_quest_state != 2)) {
        return;
    }
    g->rook_quest_state = 3;
    g->rook_quest_completions++;
    g->gold += ROOK_QUEST_REWARD;
    g->score += 500;
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Completed: Slay the Minotaur. %d gold and 500 score awarded; no return to Rook needed.", ROOK_QUEST_REWARD);
    push_message(g, message);
}

void labyrinth_migrate(GameState *g, int version) {
    for (int i = 0; i < LABYRINTH_DEPTH; i++) {
        if (g->labyrinth_cache[i].valid) {
            labyrinth_simplify_map(&g->labyrinth_cache[i].map, i + 1);
        }
    }
    if (g->location == LOCATION_LABYRINTH) {
        labyrinth_simplify_map(&g->map, g->level);
        g->labyrinth_entry_town = LOCATION_TOWN2;
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *loot = &g->floor_items[i];
            if (loot->active) {
                loot->underlying_tile = retired_tile(loot->underlying_tile, &g->map, loot->x, loot->y);
            }
        }
    }
    if (g->location == LOCATION_INN) {
        if (g->player.x == ROOK_TAVERN_X && g->player.y == ROOK_TAVERN_Y) {
            g->player.y--;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *loot = &g->floor_items[i];
            if (loot->active && loot->x == ROOK_TAVERN_X && loot->y == ROOK_TAVERN_Y) {
                loot->x++;
                loot->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[loot->y][loot->x] = TILE_ITEM;
            }
        }
        road_tile(g, ROOK_TAVERN_X, ROOK_TAVERN_Y, TILE_NPC_ROOK);
    }
    if (version < 242) {
        if (g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN2) {
            town_road_migrate(g);
        }
        if (strcmp(g->dialogue_speaker, "Rook") == 0) {
            g->dialogue_active = 0;
        }
    }
    labyrinth_complete_quest(g);
    labyrinth_prepare_road(g);
}
