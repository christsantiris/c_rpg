#include "town_life.h"
#include "spice_skiff.h"
#include <stdio.h>
#include <stdlib.h>

static const TownBuilding buildings[] = {
    {LOCATION_TOWN, LOCATION_BAKERY, 7, 17, 5, 4, 0, 1 << LOCATION_DUNGEON,
        "BAKERY", "Baker Nessa",
        "Business has been slow since the evil Lich King took over Oakhaven's dungeon. Folk are too afraid to leave home, even for fresh bread.",
        "Business is so much better since you defeated the evil Lich King! The ovens are busy again, and Oakhaven finally smells of fresh bread instead of fear."},
    {LOCATION_TOWN2, LOCATION_BUTCHER, 7, 17, 5, 4, 1, 0,
        "BUTCHER", "Butcher Holt",
        "There is evil lurking in the lands north of Stillbury. Hunters return from the swamp pale and shaken, if they return at all. Mind your step beyond the north gate.", NULL},
    {LOCATION_TOWN4, LOCATION_MONASTERY, 6, 16, 7, 5, 2, 1 << LOCATION_DRAGONSPINE,
        "MONASTERY", "Abbot Edric",
        "The dragons of Dragonspine, north of Ridgeshire, terrorize our town. We hear their cries across the peaks and pray for those caught outside the walls.",
        "Since you defeated the dragon ruling Dragonspine, Ridgeshire is safer. Our bells ring in gratitude rather than alarm. May you find peace beneath this roof."},
    {LOCATION_TOWN3, LOCATION_SPICE_SHOP, 7, 17, 5, 4, 3, 0,
        "SPICE MERCHANT", "Spice Merchant Suri",
        "The Crown Road has become dangerous. Bandits prey on caravans, and every sack of spice that reaches Rosemoor has a tale of narrow escape. Traders travel in company now.", NULL},
    {LOCATION_DAWNMERE, LOCATION_DAWNMERE_MERCHANT, 7, 7, 5, 4, 0, 0,
        "SPICE MERCHANT", "Spice Merchant Corvin",
        spice_skiff_offer, NULL},
    {LOCATION_DAWNMERE, LOCATION_ALEHOUSE, 25, 5, 7, 6, 4, 0,
        "ALEHOUSE", "Dain", NULL, NULL}
};

const TownBuilding *town_life_building(Location location) {
    return town_life_next_building(location, NULL);
}

const TownBuilding *town_life_next_building(Location location, const TownBuilding *previous) {
    int found = previous == NULL;
    for (int i = 0; i < (int)(sizeof(buildings) / sizeof(buildings[0])); i++) {
        if (found && (buildings[i].town == location || buildings[i].interior == location)) {
            return &buildings[i];
        }
        if (&buildings[i] == previous) {
            found = 1;
        }
    }
    return NULL;
}

int town_life_is_interior(Location location) {
    const TownBuilding *b = town_life_building(location);
    return b && b->interior == location;
}

static void place_building(Map *map, Location town, const TownBuilding *b) {
    int door_x = b->x + b->w / 2;
    int door_y = b->y + b->h - 1;
    for (int y = b->y; y <= door_y; y++) {
        for (int x = b->x; x < b->x + b->w; x++) {
            map->tiles[y][x] = TILE_LOCAL_BUILDING;
        }
    }
    map->tiles[door_y][door_x] = TILE_LOCAL_DOOR;
    int road_y = town == LOCATION_TOWN3 ? CASTLE_ROAD_Y : 12;
    if (door_y < road_y) {
        for (int y = door_y + 1; y < road_y; y++) {
            if (map->tiles[y][door_x] != TILE_ITEM) {
                map->tiles[y][door_x] = TILE_TOWN_PATH;
            }
        }
        return;
    }
    for (int y = road_y + 1; y <= door_y + 1; y++) {
        if (map->tiles[y][b->x - 1] != TILE_ITEM) {
            map->tiles[y][b->x - 1] = TILE_TOWN_PATH;
        }
    }
    for (int x = b->x - 1; x <= door_x; x++) {
        if (map->tiles[door_y + 1][x] != TILE_ITEM) {
            map->tiles[door_y + 1][x] = TILE_TOWN_PATH;
        }
    }
}

void town_life_place(Map *map, Location town) {
    for (const TownBuilding *b = town_life_building(town); b && b->town == town; b = town_life_next_building(town, b)) {
        place_building(map, town, b);
    }
}

void town_life_generate(Map *map, Location interior, int *sx, int *sy) {
    map_generate_workshop(map, sx, sy);
    for (int y = TAVERN_Y + 1; y < TAVERN_Y + TAVERN_H - 1; y++) {
        for (int x = TAVERN_X + 1; x < TAVERN_X + TAVERN_W - 1; x++) {
            map->tiles[y][x] = TILE_TAVERN_FLOOR;
        }
    }
    for (int x = 9; x <= 13; x++) {
        map->tiles[7][x] = TILE_TAVERN_TABLE;
        map->tiles[7][x + 17] = TILE_TAVERN_TABLE;
    }
    if (interior == LOCATION_MONASTERY) {
        for (int y = 11; y <= 14; y += 3) {
            for (int x = 12; x <= 16; x++) {
                map->tiles[y][x] = TILE_TAVERN_TABLE;
                map->tiles[y][x + 12] = TILE_TAVERN_TABLE;
            }
        }
    } else {
        for (int x = 15; x <= 25; x++) {
            map->tiles[17][x] = TILE_TAVERN_TABLE;
        }
    }
    if (interior == LOCATION_ALEHOUSE) {
        for (int x = 10; x <= 11; x++) {
            map->tiles[12][x] = TILE_TAVERN_TABLE;
            map->tiles[12][x + 16] = TILE_TAVERN_TABLE;
        }
        map->tiles[DAIN_ALEHOUSE_Y][DAIN_ALEHOUSE_X] = TILE_NPC_DAIN;
    } else {
        map->tiles[RESIDENT_Y][RESIDENT_X] = TILE_NPC_RESIDENT;
    }
}

int town_life_talk(GameState *g, int greeting) {
    if (g->location == LOCATION_ALEHOUSE) {
        if (greeting || abs(g->player.x - DAIN_ALEHOUSE_X) > 1 || abs(g->player.y - DAIN_ALEHOUSE_Y) > 1) {
            return 0;
        }
        game_talk_to_dain(g);
        return 1;
    }
    const TownBuilding *b = town_life_building(g->location);
    if (!b || b->interior != g->location ||
        (!greeting && (abs(g->player.x - RESIDENT_X) > 1 || abs(g->player.y - RESIDENT_Y) > 1))) {
        return 0;
    }
    if (g->location == LOCATION_DAWNMERE_MERCHANT) {
        spice_skiff_talk(g);
        return 1;
    }
    g->dialogue_active = 1;
    g->dialogue_x = RESIDENT_X;
    g->dialogue_y = RESIDENT_Y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", b->speaker);
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s",
        b->after && (g->defeated_bosses & b->victory) ? b->after : b->before);
    return 1;
}

static void migrate_building(GameState *g, const TownBuilding *b) {
    int door_x = b->x + b->w / 2;
    int outside_y = b->y + b->h;
    int road_y = g->location == LOCATION_TOWN3 ? CASTLE_ROAD_Y : 12;
    if (g->player.x >= b->x && g->player.x < b->x + b->w &&
        g->player.y >= b->y && g->player.y < outside_y) {
        g->player.x = door_x;
        g->player.y = outside_y;
    }
    place_building(&g->map, g->location, b);
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (!item->active) {
            continue;
        }
        if (item->x >= b->x && item->x < b->x + b->w &&
            item->y >= b->y && item->y < outside_y) {
            item->x = door_x;
            item->y = outside_y;
            item->underlying_tile = TILE_TOWN_PATH;
        } else if ((outside_y <= road_y && item->x == door_x && item->y >= outside_y && item->y <= road_y) ||
            (outside_y > road_y && item->x == b->x - 1 && item->y > road_y && item->y <= outside_y) ||
            (outside_y > road_y && item->y == outside_y && item->x >= b->x - 1 && item->x <= door_x)) {
            item->underlying_tile = TILE_TOWN_PATH;
        }
        if (item->x >= 0 && item->x < MAP_W && item->y >= 0 && item->y < MAP_H) {
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
}

void town_life_migrate(GameState *g) {
    for (const TownBuilding *b = town_life_building(g->location); b && b->town == g->location; b = town_life_next_building(g->location, b)) {
        migrate_building(g, b);
    }
}
