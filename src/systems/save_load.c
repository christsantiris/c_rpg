#include "../game/giants_crown.h"
#include "../game/ironfang.h"
#include "../game/crownroad_north.h"
#include "../game/stonewake.h"
#include "save_load.h"
#include "../game/game.h"
#include "../game/town_road.h"
#include "../game/castle.h"
#include "../game/royal_arcanist.h"
#include "../game/lord_veyr.h"
#include "../game/lich.h"
#include "../game/necromancer.h"
#include "../game/goblin_king.h"
#include "../game/drowned_queen.h"
#include "../game/swamp_demon.h"
#include "../game/minotaur.h"
#include "../game/grave_marshal.h"
#include "../game/desert_pharaoh.h"
#include "../game/cinder_lord.h"
#include "../game/prism_sovereign.h"
#include "../game/sun_guardian.h"
#include "../game/red_dragon.h"
#include "../game/regional_terrain.h"
#include "../game/landscapes.h"
#include "../game/wilds.h"
#include "../game/jail.h"
#include "../game/town_life.h"
#include "../game/approach.h"
#include "../game/elderwood.h"
#include "../game/heartseed.h"
#include "../game/spice_skiff.h"
#include "../game/mangrove.h"
#include "../game/stormwatch.h"
#include "../game/stormbell.h"
#include "../game/coastal_travel.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const char b64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static TileType retired_seal_floor(Location location) {
    if (location == LOCATION_GLASSDEEP) {
        return TILE_GLASSDEEP_FLOOR;
    }

    if (location == LOCATION_ASHEN) {
        return TILE_ASHEN_FLOOR;
    }
    return TILE_FLOOR;
}

static void remove_royal_seals(Map *map, Enemy *enemies, int count, TileType floor) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (map->tiles[y][x] == TILE_CASTLE_SEAL) {
                map->tiles[y][x] = floor;
            }
        }
    }
    for (int i = 0; i < count; i++) {
        if (strcmp(enemies[i].name, "Royal Seal Guardian") == 0) {
            enemies[i].active = 0;
        }
    }
}

static void migrate_removed_royal_seals(GameState *g) {
    TileType floor = retired_seal_floor(g->location);
    remove_royal_seals(&g->map, g->enemies, g->enemy_count, floor);
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].underlying_tile == TILE_CASTLE_SEAL) {
            g->floor_items[i].underlying_tile = floor;
        }
    }
    LevelCache *caches[] = {g->level_cache, g->glassdeep_cache, g->ashen_cache};
    const int depths[] = {MAX_REGION_DEPTH, GLASSDEEP_DEPTH, ASHEN_DEPTH};
    const Location regions[] = {LOCATION_DUNGEON, LOCATION_GLASSDEEP, LOCATION_ASHEN};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < depths[i]; j++) {
            LevelCache *cache = &caches[i][j];
            if (cache->valid) {
                remove_royal_seals(&cache->map, cache->enemies, cache->enemy_count, retired_seal_floor(regions[i]));
            }
        }
    }
    if (g->portal_origin_tile == TILE_CASTLE_SEAL) {
        g->portal_origin_tile = retired_seal_floor(g->portal_location);
    }
    if (strcmp(g->dialogue_speaker, "Royal seals") == 0) {
        g->dialogue_active = 0;
        g->dialogue_speaker[0] = '\0';
        g->dialogue_text[0] = '\0';
    }
}

static void migrate_ridgeshire_portal(GameState *g) {
    if (g->location != LOCATION_TOWN4 || !g->portal_active ||
        (g->portal_location != LOCATION_DRAGONSPINE && g->portal_location != LOCATION_ASHEN)) {
        return;
    }
    int dragon = g->portal_location == LOCATION_DRAGONSPINE;
    int old_x = dragon ? TOWN_W - 3 : 21;
    int old_y = dragon ? 13 : 2;
    int x = dragon ? RIDGESHIRE_DRAGON_PORTAL_X : (TOWN_W - 3);
    int y = dragon ? RIDGESHIRE_DRAGON_PORTAL_Y : 13;
    if (g->map.tiles[old_y][old_x] == TILE_PORTAL) {
        g->map.tiles[old_y][old_x] = TILE_TOWN_FLOOR;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (!item->active) {
            continue;
        }
        if (item->x == old_x && item->y == old_y && item->underlying_tile == TILE_PORTAL) {
            item->underlying_tile = TILE_TOWN_FLOOR;
        }
        if (item->x == x && item->y == y) {
            item->underlying_tile = TILE_PORTAL;
        }
    }
    g->map.tiles[y][x] = TILE_PORTAL;
}

static void migrate_shared_town_gates(GameState *g) {
    if (g->location != LOCATION_TOWN && g->location != LOCATION_TOWN2 && g->location != LOCATION_TOWN3 && g->location != LOCATION_TOWN4) {
        return;
    }
    // Expose terrain first so dropped items receive the retired paths' new floor.
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && g->map.tiles[item->y][item->x] == TILE_ITEM) {
            g->map.tiles[item->y][item->x] = item->underlying_tile;
        }
    }
    if (g->location == LOCATION_TOWN) {
        map_set_town2_road(&g->map, 0);
        map_set_town4_road(&g->map, 0);
    } else if (g->location == LOCATION_TOWN2) {
        map_set_stillbury_forest_road(&g->map, 0);
        map_set_town3_road(&g->map, 0);
    } else if (g->location == LOCATION_TOWN3) {
        map_set_rosemoor_swamp_road(&g->map, 0);
    } else {
        map_set_ridgeshire_mountain_road(&g->map, 0);
    }
    // A player or item saved on a removed boundary gate moves one tile inside.
    if (!map_is_walkable(&g->map, g->player.x, g->player.y)) {
        if (g->player.x == 0 || g->player.x == TOWN_W - 1) {
            g->player.x += g->player.x == 0 ? 1 : -1;
        }
        if (g->player.y == 0 || g->player.y == TOWN_H - 1) {
            g->player.y += g->player.y == 0 ? 1 : -1;
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (!item->active) {
            continue;
        }
        if (!map_is_walkable(&g->map, item->x, item->y)) {
            if (item->x == 0 || item->x == TOWN_W - 1) {
                item->x += item->x == 0 ? 1 : -1;
            }
            if (item->y == 0 || item->y == TOWN_H - 1) {
                item->y += item->y == 0 ? 1 : -1;
            }
        }
        item->underlying_tile = g->map.tiles[item->y][item->x];
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
}

static void mirror_forest_enemies(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        Enemy *enemy = &enemies[i];
        enemy->x = MAP_W - 1 - enemy->x;
        if (enemy->attack_target_x >= 0 && enemy->attack_target_x < MAP_W) {
            enemy->attack_target_x = MAP_W - 1 - enemy->attack_target_x;
        }
        if (enemy->boss_aux_x >= 0 && enemy->boss_aux_x < MAP_W) {
            enemy->boss_aux_x = MAP_W - 1 - enemy->boss_aux_x;
        }
        enemy->facing_dx = -enemy->facing_dx;
    }
}

static void migrate_forest_entry_direction(GameState *g) {
    LevelCache *cache = &g->forest_cache[0];
    int active = g->location == LOCATION_FOREST && g->level == 1 && g->map.stairs_up_x == 1;
    int cached = cache->valid && cache->map.stairs_up_x == 1;
    if (!active && !cached) {
        return;
    }
    if (active) {
        map_mirror_horizontal(&g->map);
        mirror_forest_enemies(g->enemies, g->enemy_count);
        g->player.x = MAP_W - 1 - g->player.x;
        g->player.last_dx = -g->player.last_dx;
        g->dialogue_x = MAP_W - 1 - g->dialogue_x;
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].active) {
                g->floor_items[i].x = MAP_W - 1 - g->floor_items[i].x;
            }
        }
    }
    if (cached) {
        map_mirror_horizontal(&cache->map);
        mirror_forest_enemies(cache->enemies, cache->enemy_count);
    }
    if (g->portal_location == LOCATION_FOREST && g->portal_level == 1) {
        g->portal_x = MAP_W - 1 - g->portal_x;
    }
}

static int shortcut_row(int y, int height) {
    if (y < 1) {
        return 1;
    }
    if (y > height - 2) {
        return height - 2;
    }
    return y;
}

static void migrate_narrow_shortcuts(GameState *g) {
    Map map;
    int x;
    int height;
    TileType floor;
    if (g->location == LOCATION_SWAMP_ROAD) {
        map_generate_swamp_road(&map);
        x = SWAMP_ROAD_X;
        height = SWAMP_ROAD_H;
        floor = TILE_SWAMP_FLOOR;
    } else if (g->location == LOCATION_HIGH_PASS) {
        map_generate_high_pass(&map);
        x = HIGH_PASS_X;
        height = HIGH_PASS_H;
        floor = TILE_DRAGON_FLOOR;
    } else {
        return;
    }
    // Preserve exploration and progress while replacing the old wider terrain.
    memcpy(g->map.tiles, map.tiles, sizeof(g->map.tiles));
    g->player.x = x;
    g->player.y = shortcut_row(g->player.y, height);
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (!item->active) {
            continue;
        }
        item->x = x;
        item->y = shortcut_row(item->y, height);
        item->underlying_tile = floor;
        g->map.tiles[item->y][item->x] = TILE_ITEM;
    }
}

static char *bytes_to_base64(const unsigned char *src, int src_len) {
    int dst_len = ((src_len + 2) / 3) * 4 + 1;
    char *out = malloc(dst_len);
    if (!out) {
        return NULL;
    }

    int i = 0, j = 0;
    while (i < src_len) {
        unsigned int a = i < src_len ? src[i++] : 0;
        unsigned int b = i < src_len ? src[i++] : 0;
        unsigned int c = i < src_len ? src[i++] : 0;
        unsigned int t = (a << 16) | (b << 8) | c;
        out[j++] = b64[(t >> 18) & 0x3f];
        out[j++] = b64[(t >> 12) & 0x3f];
        out[j++] = b64[(t >>  6) & 0x3f];
        out[j++] = b64[(t >>  0) & 0x3f];
    }
    int pad = src_len % 3;
    if (pad == 1) {
        out[j - 1] = '=';
        out[j - 2] = '=';
    }
    if (pad == 2) {
        out[j - 1] = '=';
    }
    out[j] = '\0';
    return out;
}

static char *tiles_to_base64(const TileType tiles[MAP_H][MAP_W]) {
    return bytes_to_base64((const unsigned char *)tiles,
        MAP_H * MAP_W * sizeof(TileType));
}

static void base64_to_bytes(const char *src, unsigned char *dst, int dst_len) {
    static const unsigned char dec[256] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,62,0,0,0,63,
        52,53,54,55,56,57,58,59,60,61,0,0,0,0,0,0,
        0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,
        0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,0,0,0,0,0
    };
    int len = strlen(src);
    int j = 0;
    for (int i = 0; i < len && j < dst_len; i += 4) {
        unsigned int a = dec[(unsigned char)src[i]];
        unsigned int b = dec[(unsigned char)src[i+1]];
        unsigned int c = src[i+2] == '=' ? 0 : dec[(unsigned char)src[i+2]];
        unsigned int d = src[i+3] == '=' ? 0 : dec[(unsigned char)src[i+3]];
        unsigned int t = (a << 18) | (b << 12) | (c << 6) | d;
        dst[j++] = (t >> 16) & 0xff;
        if (src[i+2] != '=' && j < dst_len) {
            dst[j++] = (t >> 8) & 0xff;
        }
        if (src[i+3] != '=' && j < dst_len) {
            dst[j++] = t & 0xff;
        }
    }
}

static void base64_to_tiles(const char *src, TileType tiles[MAP_H][MAP_W]) {
    base64_to_bytes(src, (unsigned char *)tiles,
        MAP_H * MAP_W * sizeof(TileType));
}

static const char *slot_path(int slot) {
    static char path[64];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", slot);
    return path;
}

int save_exists(int slot) {
    FILE *f = fopen(slot_path(slot), "r");
    if (f) { fclose(f); return 1; }
    return 0;
}

static cJSON *serialize_map(const Map *m) {
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "room_count",    m->room_count);
    cJSON_AddNumberToObject(obj, "stairs_up_x",   m->stairs_up_x);
    cJSON_AddNumberToObject(obj, "stairs_up_y",   m->stairs_up_y);
    cJSON_AddNumberToObject(obj, "stairs_down_x", m->stairs_down_x);
    cJSON_AddNumberToObject(obj, "stairs_down_y", m->stairs_down_y);

    cJSON *traps = cJSON_CreateArray();
    for (int i = 0; i < m->burial_trap_count; i++) {
        const BurialTrap *trap = &m->burial_traps[i];
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "x", trap->x);
        cJSON_AddNumberToObject(entry, "y", trap->y);
        cJSON_AddNumberToObject(entry, "vertical", trap->vertical);
        cJSON_AddNumberToObject(entry, "timer", trap->timer);
        cJSON_AddNumberToObject(entry, "spent", trap->spent);
        cJSON_AddItemToArray(traps, entry);
    }
    cJSON_AddItemToObject(obj, "burial_traps", traps);

    // Rooms
    cJSON *rooms = cJSON_CreateArray();
    for (int i = 0; i < m->room_count; i++) {
        cJSON *r = cJSON_CreateObject();
        cJSON_AddNumberToObject(r, "x", m->rooms[i].x);
        cJSON_AddNumberToObject(r, "y", m->rooms[i].y);
        cJSON_AddNumberToObject(r, "w", m->rooms[i].w);
        cJSON_AddNumberToObject(r, "h", m->rooms[i].h);
        cJSON_AddItemToArray(rooms, r);
    }
    cJSON_AddItemToObject(obj, "rooms", rooms);

    // Tiles as base64 string
    char *b64tiles = tiles_to_base64(m->tiles);
    cJSON_AddStringToObject(obj, "tiles_b64", b64tiles);
    free(b64tiles);

    char *b64explored = bytes_to_base64(m->explored,
        MAP_EXPLORED_BYTES);
    cJSON_AddStringToObject(obj, "explored_b64", b64explored);
    free(b64explored);

    return obj;
}

static void deserialize_map(const cJSON *obj, Map *m) {
    m->room_count    = cJSON_GetObjectItem(obj, "room_count")->valueint;
    m->stairs_up_x   = cJSON_GetObjectItem(obj, "stairs_up_x")->valueint;
    m->stairs_up_y   = cJSON_GetObjectItem(obj, "stairs_up_y")->valueint;
    m->stairs_down_x = cJSON_GetObjectItem(obj, "stairs_down_x")->valueint;
    m->stairs_down_y = cJSON_GetObjectItem(obj, "stairs_down_y")->valueint;

    cJSON *rooms = cJSON_GetObjectItem(obj, "rooms");
    for (int i = 0; i < m->room_count; i++) {
        cJSON *r = cJSON_GetArrayItem(rooms, i);
        m->rooms[i].x = cJSON_GetObjectItem(r, "x")->valueint;
        m->rooms[i].y = cJSON_GetObjectItem(r, "y")->valueint;
        m->rooms[i].w = cJSON_GetObjectItem(r, "w")->valueint;
        m->rooms[i].h = cJSON_GetObjectItem(r, "h")->valueint;
    }

    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    cJSON *traps = cJSON_GetObjectItem(obj, "burial_traps");
    int count = cJSON_GetArraySize(traps);
    if (count > MAX_ROOMS) {
        count = MAX_ROOMS;
    }
    for (int i = 0; i < count; i++) {
        cJSON *entry = cJSON_GetArrayItem(traps, i);
        cJSON *x = cJSON_GetObjectItem(entry, "x");
        cJSON *y = cJSON_GetObjectItem(entry, "y");
        cJSON *vertical = cJSON_GetObjectItem(entry, "vertical");
        cJSON *timer = cJSON_GetObjectItem(entry, "timer");
        cJSON *spent = cJSON_GetObjectItem(entry, "spent");
        if (cJSON_IsNumber(x) && cJSON_IsNumber(y) && cJSON_IsNumber(vertical) && cJSON_IsNumber(timer) && cJSON_IsNumber(spent) &&
            x->valueint >= 0 && x->valueint < MAP_W && y->valueint >= 0 && y->valueint < MAP_H) {
            m->burial_traps[m->burial_trap_count++] = (BurialTrap){x->valueint, y->valueint, vertical->valueint != 0, timer->valueint > 0 ? 1 : 0, spent->valueint != 0};
        }
    }
    base64_to_tiles(cJSON_GetObjectItem(obj, "tiles_b64")->valuestring, m->tiles);
    map_clear_exploration(m);
    cJSON *explored = cJSON_GetObjectItem(obj, "explored_b64");
    if (explored && cJSON_IsString(explored)) {
        base64_to_bytes(explored->valuestring, m->explored,
            MAP_EXPLORED_BYTES);
    }
}

static void hide_legacy_fort_plate(Map *m) {
    if (m->room_count < 2) {
        return;
    }
    int cx;
    int cy;
    map_room_center(&m->rooms[1], &cx, &cy);
    if (cx < 0 || cx + 1 >= MAP_W || cy < 0 || cy >= MAP_H) {
        return;
    }
    if (m->tiles[cy][cx + 1] == TILE_TRAP_REVEALED) {
        m->tiles[cy][cx + 1] = TILE_TRAP_HIDDEN;
    }
}

static void move_ashore_from_town3_moat(int *x, int *y) {
    int moat_right = TOWN_MOAT_X + TOWN_MOAT_W - 1;
    int moat_bottom = TOWN_MOAT_Y + TOWN_MOAT_H - 1;
    if (*x < TOWN_MOAT_X || *x > moat_right ||
        *y < TOWN_MOAT_Y || *y > moat_bottom ||
        (*x == CROWNROAD_X && *y == moat_bottom)) {
        return;
    }
    if (*y == TOWN_MOAT_Y) {
        *y -= 1;
    } else if (*y == moat_bottom) {
        *y += 1;
    }
    if (*x == TOWN_MOAT_X) {
        *x -= 1;
    } else if (*x == moat_right) {
        *x += 1;
    }
}

// Older Crown Road saves ran north to south; Crown Road East runs west to east.
// This is the same turn map_generate_crownroad applies to the road's layout.
static void rotate_crownroad_position(int *x, int *y) {
    int old_x = *x;
    *x = CROWNROAD_LEGACY_W - 1 - *y;
    *y = old_x;
}

static void stretch_crownroad_x(int *x) {
    if (*x >= 0 && *x < CROWNROAD_LEGACY_W) {
        *x *= CROWNROAD_LENGTH_SCALE;
    }
}

static void stretch_crownroad_enemies(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        stretch_crownroad_x(&enemies[i].x);
        stretch_crownroad_x(&enemies[i].attack_target_x);
    }
}

static void migrate_longer_crownroads(GameState *g, int version) {
    if (game_is_king_road(g)) {
        unsigned char explored[MAP_EXPLORED_BYTES];
        memcpy(explored, g->map.explored, sizeof(explored));
        map_generate_crownroad(&g->map);
        // Before version 69 the orientation migration already regenerated the new map.
        if (version >= 69) {
            for (int y = 0; y < CROWNROAD_H; y++) {
                for (int x = 0; x < CROWNROAD_W; x++) {
                    int index = y * MAP_W + x / CROWNROAD_LENGTH_SCALE;
                    if (explored[index / 8] & (1u << (index % 8))) {
                        map_mark_explored(&g->map, x, y);
                    }
                }
            }
        }
        stretch_crownroad_x(&g->player.x);
        stretch_crownroad_enemies(g->enemies, g->enemy_count);
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            stretch_crownroad_x(&item->x);
            if (item->active && item->x >= 0 && item->x < MAP_W && item->y >= 0 && item->y < MAP_H) {
                item->underlying_tile = g->map.tiles[item->y][item->x];
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x >= 0 && item->x < MAP_W && item->y >= 0 && item->y < MAP_H) {
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->trail_count = 0;
        g->trail_frames = 0;
    }
    if (g->crownroad_cache.valid) {
        stretch_crownroad_enemies(g->crownroad_cache.enemies, g->crownroad_cache.enemy_count);
    }
    if (g->kingroad_west_cache.valid) {
        stretch_crownroad_enemies(g->kingroad_west_cache.enemies, g->kingroad_west_cache.enemy_count);
    }
}

static int in_lot(int x, int y, int lot_x, int lot_y, int w, int h) {
    return x >= lot_x && x < lot_x + w && y >= lot_y && y < lot_y + h;
}

// Move the player and loot standing on a new building lot onto the square in
// front of it, just below the lot.
static void clear_new_town_lot(GameState *g, int lot_x, int lot_y, int w, int h) {
    int front_y = lot_y + h;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && in_lot(item->x, item->y, lot_x, lot_y, w, h)) {
            item->y = front_y;
            item->underlying_tile = TILE_TOWN_PATH;
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
    if (in_lot(g->player.x, g->player.y, lot_x, lot_y, w, h)) {
        g->player.y = front_y;
    }
}

// Swap a town tile between grass and cobblestone, leaving buildings, NPCs and
// portals alone. Loot keeps its marker and records the new ground beneath it.
static void retile_town_ground(GameState *g, int x, int y, TileType ground) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_TOWN_FLOOR || tile == TILE_TOWN_PATH) {
        g->map.tiles[y][x] = ground;
        return;
    }
    if (tile != TILE_ITEM) {
        return;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y &&
            (item->underlying_tile == TILE_TOWN_FLOOR ||
            item->underlying_tile == TILE_TOWN_PATH)) {
            item->underlying_tile = ground;
        }
    }
}

static void migrate_island_temple(GameState *g) {
    if (g->location != LOCATION_ISLAND) {
        return;
    }
    map_place_island_temple(&g->map);
    if (in_lot(g->player.x, g->player.y, ISLAND_TEMPLE_X, ISLAND_TEMPLE_Y, ISLAND_TEMPLE_W, ISLAND_TEMPLE_H)) {
        g->player.x = ISLAND_GATE_X;
        g->player.y = ISLAND_GATE_Y + 1;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (!item->active) {
            continue;
        }
        if (in_lot(item->x, item->y, ISLAND_TEMPLE_X, ISLAND_TEMPLE_Y, ISLAND_TEMPLE_W, ISLAND_TEMPLE_H)) {
            item->x = ISLAND_GATE_X;
            item->y = ISLAND_GATE_Y + 1;
            item->underlying_tile = TILE_ISLAND_PATH;
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        } else if (item->underlying_tile == TILE_ISLAND_TEMPLE_GATE) {
            item->underlying_tile = TILE_ISLAND_PATH;
        }
    }
    if (g->portal_active && g->portal_location == LOCATION_ISLAND) {
        if (in_lot(g->portal_x, g->portal_y, ISLAND_TEMPLE_X, ISLAND_TEMPLE_Y, ISLAND_TEMPLE_W, ISLAND_TEMPLE_H)) {
            g->portal_x = ISLAND_GATE_X;
            g->portal_y = ISLAND_GATE_Y + 1;
            g->portal_origin_tile = TILE_ISLAND_PATH;
            g->map.tiles[g->portal_y][g->portal_x] = TILE_PORTAL;
        } else if (g->portal_origin_tile == TILE_ISLAND_TEMPLE_GATE) {
            g->portal_origin_tile = TILE_ISLAND_PATH;
        }
    }
}

static void clear_town3_old_lot(GameState *g, int lot_x, int lot_y, int w, int h) {
    for (int y = lot_y; y < lot_y + h; y++) {
        for (int x = lot_x; x < lot_x + w; x++) {
            if (g->map.tiles[y][x] == TILE_ITEM) {
                for (int i = 0; i < g->floor_item_count; i++) {
                    FloorItem *item = &g->floor_items[i];
                    if (item->active && item->x == x && item->y == y) {
                        item->underlying_tile = TILE_TOWN_FLOOR;
                    }
                }
            } else {
                g->map.tiles[y][x] = TILE_TOWN_FLOOR;
            }
        }
    }
}

static cJSON *serialize_enemies(const Enemy *enemies, int count) {
    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < count; i++) {
        const Enemy *e = &enemies[i];
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "x",          e->x);
        cJSON_AddNumberToObject(obj, "y",          e->y);
        cJSON_AddNumberToObject(obj, "active",     e->active);
        cJSON_AddNumberToObject(obj, "type",       e->type);
        cJSON_AddStringToObject(obj, "name",       e->name);
        cJSON_AddNumberToObject(obj, "hp",         e->hp);
        cJSON_AddNumberToObject(obj, "max_hp",     e->max_hp);
        cJSON_AddNumberToObject(obj, "attack",     e->attack);
        cJSON_AddNumberToObject(obj, "defense",    e->defense);
        cJSON_AddNumberToObject(obj, "experience", e->experience);
        cJSON_AddNumberToObject(obj, "move_timer", e->move_timer);
        cJSON_AddNumberToObject(obj, "is_boss",    e->is_boss);
        cJSON_AddNumberToObject(obj, "dain_fragment", e->dain_fragment);
        cJSON_AddNumberToObject(obj, "frozen_turns", e->frozen_turns);
        cJSON_AddNumberToObject(obj, "attack_target_x", e->attack_target_x);
        cJSON_AddNumberToObject(obj, "attack_target_y", e->attack_target_y);
        cJSON_AddNumberToObject(obj, "revived", e->revived);
        cJSON_AddNumberToObject(obj, "revive_timer", e->revive_timer);
        cJSON_AddNumberToObject(obj, "facing_dx", e->facing_dx);
        cJSON_AddNumberToObject(obj, "facing_dy", e->facing_dy);
        cJSON_AddNumberToObject(obj, "attack_phase", e->attack_phase);
        cJSON_AddNumberToObject(obj, "boss_attack", e->boss_attack);
        cJSON_AddNumberToObject(obj, "boss_wave_used", e->boss_wave_used);
        cJSON_AddNumberToObject(obj, "boss_aux_x", e->boss_aux_x);
        cJSON_AddNumberToObject(obj, "boss_aux_y", e->boss_aux_y);
        if (e->type == ENEMY_RED_DRAGON) {
            cJSON *warning = cJSON_CreateArray();
            for (int row = 0; row < DRAGON_WARNING_SIZE; row++) {
                cJSON_AddItemToArray(warning, cJSON_CreateNumber(e->dragon_warning[row]));
            }
            cJSON_AddItemToObject(obj, "dragon_warning", warning);
        }
        cJSON_AddItemToArray(arr, obj);
    }
    return arr;
}

static void migrate_retired_enemy(Enemy *e) {
    // Only serialized IDs remain here; retired types never enter runtime state.
    const char *name;
    switch ((int)e->type) {
        case 1:
            e->type = ENEMY_GOBLIN_SCOUT;
            name = "Goblin Scout";
            break;
        case 3:
            e->type = ENEMY_HOBGOBLIN_GUARD;
            name = "Hobgoblin Guard";
            break;
        case 4:
            e->type = ENEMY_CAVE_TROLL;
            name = "Cave Troll";
            break;
        case 6:
        case 8:
        case 10:
            e->type = ENEMY_LICH_KING;
            name = "Lich King";
            break;
        case 84:
            e->type = ENEMY_VAMPIRE;
            name = "Vampire";
            break;
        case 85:
            e->type = ENEMY_CRYPT_CONJURER;
            name = "Crypt Conjurer";
            break;
        case 110:
            e->type = ENEMY_GIANT_CRAB;
            name = "Giant Crab";
            break;
        default:
            return;
    }
    if (!strstr(e->name, "Memorial Guard")) {
        snprintf(e->name, sizeof(e->name), "%s", name);
    }
}

static void deserialize_enemies(const cJSON *arr, Enemy *enemies, int *count, int capacity) {
    *count = cJSON_GetArraySize(arr);
    if (*count > capacity) {
        *count = capacity;
    }
    for (int i = 0; i < *count; i++) {
        cJSON *obj = cJSON_GetArrayItem(arr, i);
        Enemy *e = &enemies[i];
        e->x          = cJSON_GetObjectItem(obj, "x")->valueint;
        e->y          = cJSON_GetObjectItem(obj, "y")->valueint;
        e->active     = cJSON_GetObjectItem(obj, "active")->valueint;
        e->type       = cJSON_GetObjectItem(obj, "type")->valueint;
        const char *name = cJSON_GetObjectItem(obj, "name")->valuestring;
        // Normalize renamed actors in both active and cached encounters.
        if (e->type == ENEMY_STONE_GIANT && strcmp(name, "Giant") == 0) {
            name = "Stone Giant";
        } else if (e->type == ENEMY_FROST_GIANT && strcmp(name, "Ice Giant") == 0) {
            name = "Frost Giant";
        } else if (e->type == ENEMY_MINOTAUR) {
            name = "Minotaur";
        } else if (e->type == ENEMY_FROST_KRAKEN) {
            name = "Frost Kraken";
        }
        snprintf(e->name, sizeof(e->name), "%s", name);
        migrate_retired_enemy(e);
        e->hp         = cJSON_GetObjectItem(obj, "hp")->valueint;
        e->max_hp     = cJSON_GetObjectItem(obj, "max_hp")->valueint;
        e->attack     = cJSON_GetObjectItem(obj, "attack")->valueint;
        e->defense    = cJSON_GetObjectItem(obj, "defense")->valueint;
        e->experience = cJSON_GetObjectItem(obj, "experience")->valueint;
        e->move_timer = cJSON_GetObjectItem(obj, "move_timer")->valueint;
        cJSON *boss = cJSON_GetObjectItem(obj, "is_boss");
        e->is_boss = boss ? boss->valueint :
            (e->type == ENEMY_LICH_KING || e->type == ENEMY_RED_DRAGON ||
             e->type == ENEMY_FOREST_NECROMANCER ||
             e->type == ENEMY_MOUNTAIN_GOBLIN_KING ||
             e->type == ENEMY_DROWNED_QUEEN);
        cJSON *dain_fragment = cJSON_GetObjectItem(obj, "dain_fragment");
        e->dain_fragment = dain_fragment ? dain_fragment->valueint : 0;
        e->frozen_turns = cJSON_GetObjectItem(obj, "frozen_turns")->valueint;
        e->attack_target_x = cJSON_GetObjectItem(obj, "attack_target_x")->valueint;
        e->attack_target_y = cJSON_GetObjectItem(obj, "attack_target_y")->valueint;
        cJSON *revived = cJSON_GetObjectItem(obj, "revived");
        cJSON *revive_timer = cJSON_GetObjectItem(obj, "revive_timer");
        e->revived = revived ? revived->valueint : 0;
        e->revive_timer = revive_timer ? revive_timer->valueint : 0;
        cJSON *fx = cJSON_GetObjectItem(obj, "facing_dx");
        cJSON *fy = cJSON_GetObjectItem(obj, "facing_dy");
        e->facing_dx = fx ? fx->valueint : 0;
        e->facing_dy = fy ? fy->valueint : -1;
        cJSON *phase = cJSON_GetObjectItem(obj, "attack_phase");
        e->attack_phase = phase ? phase->valueint : 0;
        cJSON *attack = cJSON_GetObjectItem(obj, "boss_attack");
        cJSON *wave = cJSON_GetObjectItem(obj, "boss_wave_used");
        cJSON *aux_x = cJSON_GetObjectItem(obj, "boss_aux_x");
        cJSON *aux_y = cJSON_GetObjectItem(obj, "boss_aux_y");
        e->boss_attack = attack ? attack->valueint : 0;
        e->boss_wave_used = wave ? wave->valueint : 0;
        e->boss_aux_x = aux_x ? aux_x->valueint : -1;
        e->boss_aux_y = aux_y ? aux_y->valueint : -1;
        cJSON *warning = cJSON_GetObjectItem(obj, "dragon_warning");
        for (int row = 0; row < DRAGON_WARNING_SIZE; row++) {
            cJSON *bits = cJSON_GetArrayItem(warning, row);
            e->dragon_warning[row] = bits ? (unsigned int)bits->valueint & ((1u << DRAGON_WARNING_WORD_BITS) - 1u) : 0;
        }
        if (cJSON_GetObjectItem(obj, "type")->valueint == 110) {
            // Retire the old charge without moving or healing the saved actor.
            e->attack_phase = 0;
            e->boss_attack = 0;
            e->move_timer = 0;
            e->attack_target_x = -1;
            e->attack_target_y = -1;
            e->defense = 6;
        }
    }
}

static void deserialize_item_metadata(const cJSON *obj, Item *item) {
    // Migrate bow names and ranges in inventory, floor loot and castle loot.
    if (item->type == ITEM_WEAPON && strcmp(item->name, "Bow") == 0) {
        snprintf(item->name, sizeof(item->name), "Hunting Bow");
    }
    if (item->type == ITEM_WEAPON) {
        if (strcmp(item->name, "Hunting Bow") == 0) {
            item->range = 3;
        } else if (strcmp(item->name, "Shortbow") == 0 || strcmp(item->name, "Crossbow") == 0) {
            item->range = 5;
        } else if (strcmp(item->name, "Longbow") == 0 || strcmp(item->name, "Magic Longbow") == 0 ||
            strcmp(item->name, "Krakenbone Bow") == 0) {
            item->range = MAX_BOW_RANGE;
        }
    }
    cJSON *sharpened = cJSON_GetObjectItem(obj, "sharpened");
    item->sharpened = sharpened ? !!sharpened->valueint : 0;
    cJSON *family = cJSON_GetObjectItem(obj, "weapon_family");
    cJSON *critical = cJSON_GetObjectItem(obj, "critical_chance_bonus");
    cJSON *gold_bonus = cJSON_GetObjectItem(obj, "gold_drop_bonus_percent");
    item->gold_drop_bonus_percent = gold_bonus ? gold_bonus->valueint : 0;
    cJSON *cleave = cJSON_GetObjectItem(obj, "cleave_percent");
    cJSON *piercing = cJSON_GetObjectItem(obj, "pierces_targets");
    cJSON *spell_power = cJSON_GetObjectItem(obj, "spell_power_bonus");
    cJSON *armor_penetration = cJSON_GetObjectItem(obj,
        "armor_penetration_percent");
    cJSON *armor_family = cJSON_GetObjectItem(obj, "armor_family");
    cJSON *max_hp_bonus = cJSON_GetObjectItem(obj, "max_hp_bonus");
    cJSON *max_mp_bonus = cJSON_GetObjectItem(obj, "max_mp_bonus");
    cJSON *evasion = cJSON_GetObjectItem(obj, "evasion_chance");
    cJSON *spell_cost_reduction = cJSON_GetObjectItem(obj,
        "spell_cost_reduction_percent");
    cJSON *block_chance = cJSON_GetObjectItem(obj, "block_chance");
    cJSON *block_reduction = cJSON_GetObjectItem(obj,
        "block_reduction_percent");
    if (!block_reduction && item->type == ITEM_SHIELD) {
        int old_defense = item->defense_bonus;
        item_apply_legacy_shield_metadata(item);
        if (strcmp(item->name, "Goblin King's Shield") == 0) {
            item->defense_bonus = old_defense;
        }
        return;
    }
    if (!family && item->type == ITEM_WEAPON) {
        int old_attack = item->attack_bonus;
        item_apply_legacy_metadata(item);
        // Keep saved attack until the equipped-stat migrations adjust the player.
        if (strcmp(item->name, "Cryptblade") == 0 || item->weapon_family == WEAPON_FAMILY_DAGGER) {
            item->attack_bonus = old_attack;
        }
        cJSON *legacy_two_handed = cJSON_GetObjectItem(obj,
            "is_two_handed");
        if (legacy_two_handed && legacy_two_handed->valueint) {
            item->weapon_hands = WEAPON_HANDS_TWO;
        }
        if (critical) {
            item->critical_chance_bonus = critical->valueint;
        }
        if (cleave) {
            item->cleave_percent = cleave->valueint;
        }
        if (piercing) {
            item->pierces_targets = piercing->valueint;
        }
        if (spell_power) {
            item->spell_power_bonus = spell_power->valueint;
        }
        if (armor_penetration) {
            item->armor_penetration_percent =
                armor_penetration->valueint;
        }
        return;
    }
    cJSON *hands = cJSON_GetObjectItem(obj, "weapon_hands");
    cJSON *rarity = cJSON_GetObjectItem(obj, "rarity");
    cJSON *class_mask = cJSON_GetObjectItem(obj, "class_mask");
    cJSON *visual_id = cJSON_GetObjectItem(obj, "visual_id");
    item->weapon_family = family ? family->valueint : WEAPON_FAMILY_NONE;
    item->weapon_hands = hands ? hands->valueint : WEAPON_HANDS_NONE;
    item->rarity = rarity ? rarity->valueint : ITEM_RARITY_COMMON;
    item->class_mask = class_mask ? class_mask->valueint : 0;
    item->visual_id = visual_id ? visual_id->valueint : ITEM_VISUAL_NONE;
    item->critical_chance_bonus = critical ? critical->valueint : 0;
    item->cleave_percent = cleave ? cleave->valueint : 0;
    item->pierces_targets = piercing ? piercing->valueint : 0;
    item->spell_power_bonus = spell_power ? spell_power->valueint : 0;
    item->armor_penetration_percent = armor_penetration
        ? armor_penetration->valueint : 0;
    item->armor_family = armor_family
        ? armor_family->valueint : ARMOR_FAMILY_NONE;
    item->max_hp_bonus = max_hp_bonus ? max_hp_bonus->valueint : 0;
    item->max_mp_bonus = max_mp_bonus ? max_mp_bonus->valueint : 0;
    item->evasion_chance = evasion ? evasion->valueint : 0;
    item->spell_cost_reduction_percent = spell_cost_reduction
        ? spell_cost_reduction->valueint : 0;
    item->block_chance = block_chance ? block_chance->valueint : 0;
    item->block_reduction_percent = block_reduction
        ? block_reduction->valueint : 0;
}

static cJSON *serialize_castle_loot(const FloorItem *items, int count) {
    cJSON *floor_items = cJSON_CreateArray();
    for (int i = 0; i < count; i++) {
        const FloorItem *fi = &items[i];
        cJSON *f = cJSON_CreateObject();
        cJSON_AddNumberToObject(f, "active", fi->active);
        cJSON_AddNumberToObject(f, "x", fi->x);
        cJSON_AddNumberToObject(f, "y", fi->y);
        cJSON_AddNumberToObject(f, "underlying_tile", fi->underlying_tile);
        cJSON *it = cJSON_CreateObject();
        cJSON_AddNumberToObject(it, "active", fi->item.active);
        cJSON_AddNumberToObject(it, "type", fi->item.type);
        cJSON_AddStringToObject(it, "name", fi->item.name);
        cJSON_AddNumberToObject(it, "attack_bonus", fi->item.attack_bonus);
        cJSON_AddNumberToObject(it, "sharpened", fi->item.sharpened);
        cJSON_AddNumberToObject(it, "defense_bonus", fi->item.defense_bonus);
        cJSON_AddNumberToObject(it, "value", fi->item.value);
        cJSON_AddNumberToObject(it, "spell_id", fi->item.spell_id);
        cJSON_AddNumberToObject(it, "is_ranged", fi->item.is_ranged);
        cJSON_AddNumberToObject(it, "range", fi->item.range);
        cJSON_AddNumberToObject(it, "weapon_family",
            fi->item.weapon_family);
        cJSON_AddNumberToObject(it, "weapon_hands", fi->item.weapon_hands);
        cJSON_AddNumberToObject(it, "rarity", fi->item.rarity);
        cJSON_AddNumberToObject(it, "class_mask", fi->item.class_mask);
        cJSON_AddNumberToObject(it, "visual_id", fi->item.visual_id);
        cJSON_AddNumberToObject(it, "critical_chance_bonus",
            fi->item.critical_chance_bonus);
        cJSON_AddNumberToObject(it, "gold_drop_bonus_percent", fi->item.gold_drop_bonus_percent);
        cJSON_AddNumberToObject(it, "cleave_percent",
            fi->item.cleave_percent);
        cJSON_AddNumberToObject(it, "pierces_targets",
            fi->item.pierces_targets);
        cJSON_AddNumberToObject(it, "spell_power_bonus",
            fi->item.spell_power_bonus);
        cJSON_AddNumberToObject(it, "armor_penetration_percent",
            fi->item.armor_penetration_percent);
        cJSON_AddNumberToObject(it, "armor_family", fi->item.armor_family);
        cJSON_AddNumberToObject(it, "max_hp_bonus", fi->item.max_hp_bonus);
        cJSON_AddNumberToObject(it, "max_mp_bonus", fi->item.max_mp_bonus);
        cJSON_AddNumberToObject(it, "evasion_chance",
            fi->item.evasion_chance);
        cJSON_AddNumberToObject(it, "spell_cost_reduction_percent",
            fi->item.spell_cost_reduction_percent);
        cJSON_AddNumberToObject(it, "block_chance",
            fi->item.block_chance);
        cJSON_AddNumberToObject(it, "block_reduction_percent",
            fi->item.block_reduction_percent);
        cJSON_AddItemToObject(f, "item", it);
        cJSON_AddItemToArray(floor_items, f);
    }
    return floor_items;
}

static int deserialize_castle_loot(const cJSON *floor_items, FloorItem *items, int *count) {
    if (!cJSON_IsArray(floor_items) || cJSON_GetArraySize(floor_items) > MAX_FLOOR_ITEMS) {
        return 0;
    }
    *count = cJSON_GetArraySize(floor_items);
    for (int i = 0; i < *count; i++) {
        cJSON *f = cJSON_GetArrayItem(floor_items, i);
        FloorItem *fi = &items[i];
        fi->active = cJSON_GetObjectItem(f, "active")->valueint;
        fi->x = cJSON_GetObjectItem(f, "x")->valueint;
        fi->y = cJSON_GetObjectItem(f, "y")->valueint;
        cJSON *underlying = cJSON_GetObjectItem(f, "underlying_tile");
        fi->underlying_tile = underlying ? underlying->valueint : TILE_CASTLE_FLOOR;
        cJSON *it = cJSON_GetObjectItem(f, "item");
        fi->item.active = cJSON_GetObjectItem(it, "active")->valueint;
        fi->item.type = cJSON_GetObjectItem(it, "type")->valueint;
        strncpy(fi->item.name, cJSON_GetObjectItem(it, "name")->valuestring,
            sizeof(fi->item.name) - 1);
        fi->item.attack_bonus = cJSON_GetObjectItem(it, "attack_bonus")->valueint;
        fi->item.defense_bonus = cJSON_GetObjectItem(it, "defense_bonus")->valueint;
        fi->item.value = cJSON_GetObjectItem(it, "value")->valueint;
        fi->item.spell_id = cJSON_GetObjectItem(it, "spell_id")->valueint;
        fi->item.is_ranged = cJSON_GetObjectItem(it, "is_ranged")->valueint;
        fi->item.range = cJSON_GetObjectItem(it, "range")->valueint;
        deserialize_item_metadata(it, &fi->item);
    }
    return 1;
}

static int elderwood_load(const cJSON *root, GameState *g, int version) {
    memset(g->elderwood_cache, 0, sizeof(g->elderwood_cache));
    g->max_elderwood_level_reached = 1;
    g->elderwood_defeated = 0;
    g->elderwood_entry_town = LOCATION_TOWN;
    g->catacombs_entry_town = LOCATION_CASTLE;
    g->northern_portal_town = LOCATION_TOWN3; // Keep existing Rosemoor return portals usable.
    g->last_town_visited = LOCATION_TOWN;
    g->return_portal_town = -1;
    if (version < 182) {
        return 1;
    }
    const char *names[] = {"max_elderwood_level_reached", "elderwood_defeated", "elderwood_entry_town", "catacombs_entry_town", "northern_portal_town", "last_town_visited", "return_portal_town"};
    int values[7];
    for (int i = 0; i < 7; i++) {
        const cJSON *item = cJSON_GetObjectItem(root, names[i]);
        if (!cJSON_IsNumber(item) || item->valuedouble != item->valueint) {
            return 0;
        }
        values[i] = item->valueint;
    }
    g->max_elderwood_level_reached = values[0];
    g->elderwood_defeated = values[1];
    g->elderwood_entry_town = values[2];
    g->catacombs_entry_town = values[3];
    g->northern_portal_town = values[4];
    g->last_town_visited = values[5];
    g->return_portal_town = values[6];
    if (town_road_index(g->last_town_visited) < 0 || (g->return_portal_town != -1 && town_road_index(g->return_portal_town) < 0)) {
        return 0;
    }
    if (g->max_elderwood_level_reached < 1 || g->max_elderwood_level_reached > ELDERWOOD_DEPTH ||
        g->elderwood_defeated < 0 || g->elderwood_defeated > 1 ||
        (g->elderwood_entry_town != LOCATION_TOWN && g->elderwood_entry_town != LOCATION_CASTLE) ||
        (g->catacombs_entry_town != LOCATION_TOWN && g->catacombs_entry_town != LOCATION_CASTLE) ||
        (g->northern_portal_town != LOCATION_TOWN && g->northern_portal_town != LOCATION_CASTLE && g->northern_portal_town != LOCATION_TOWN3)) {
        return 0;
    }
    const cJSON *array = cJSON_GetObjectItem(root, "elderwood_cache");
    if (!cJSON_IsArray(array) || cJSON_GetArraySize(array) != ELDERWOOD_DEPTH) {
        return 0;
    }
    for (int i = 0; i < ELDERWOOD_DEPTH; i++) {
        ElderwoodCache *cache = &g->elderwood_cache[i];
        const cJSON *entry = cJSON_GetArrayItem(array, i);
        const cJSON *valid = cJSON_GetObjectItem(entry, "valid");
        const cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
        if (!cJSON_IsNumber(valid) || (valid->valueint != 0 && valid->valueint != 1) ||
            !cJSON_IsNumber(cleared) || (cleared->valueint != 0 && cleared->valueint != 1)) {
            return 0;
        }
        cache->valid = valid->valueint;
        cache->level_cleared = cleared->valueint;
        if (!cache->valid) {
            continue;
        }
        const cJSON *map = cJSON_GetObjectItem(entry, "map");
        const cJSON *enemies = cJSON_GetObjectItem(entry, "enemies");
        const char *fields[] = {"room_count", "stairs_up_x", "stairs_up_y", "stairs_down_x", "stairs_down_y"};
        for (int n = 0; n < 5; n++) {
            if (!cJSON_IsNumber(cJSON_GetObjectItem(map, fields[n]))) {
                return 0;
            }
        }
        if (cJSON_GetObjectItem(map, "room_count")->valueint != 0 ||
            !cJSON_IsString(cJSON_GetObjectItem(map, "tiles_b64")) ||
            !cJSON_IsString(cJSON_GetObjectItem(map, "explored_b64")) ||
            !cJSON_IsArray(enemies) || cJSON_GetArraySize(enemies) > AREA_ENEMY_LIMIT) {
            return 0;
        }
        const char *enemy_fields[] = {"x", "y", "active", "type", "hp", "max_hp", "attack", "defense", "experience", "is_boss", "move_timer", "frozen_turns", "attack_target_x", "attack_target_y"};
        for (int n = 0; n < cJSON_GetArraySize(enemies); n++) {
            const cJSON *enemy = cJSON_GetArrayItem(enemies, n);
            if (!cJSON_IsString(cJSON_GetObjectItem(enemy, "name"))) {
                return 0;
            }
            for (int f = 0; f < 14; f++) {
                if (!cJSON_IsNumber(cJSON_GetObjectItem(enemy, enemy_fields[f]))) {
                    return 0;
                }
            }
        }
        Map scratch = {0};
        deserialize_map(map, &scratch);
        elderwood_pack(cache, &scratch);
        deserialize_enemies(enemies, cache->enemies, &cache->enemy_count, AREA_ENEMY_LIMIT);
    }
    return 1;
}

static int load_heartseed(const cJSON *root, GameState *g, int version) {
    int *values[] = {&g->heartseed_quest_state, &g->heartseed_progress, &g->heartseed_bindings,
        &g->heartseed_sites, &g->heartseed_guardian};
    const char *names[] = {"heartseed_quest_state", "heartseed_progress", "heartseed_bindings",
        "heartseed_sites", "heartseed_guardian"};
    const int limits[] = {3, 7, 7, 3, 3};
    for (int i = 0; i < 5; i++) {
        *values[i] = 0;
        if (version < 204) {
            continue;
        }
        const cJSON *value = cJSON_GetObjectItem(root, names[i]);
        if (!cJSON_IsNumber(value) || value->valuedouble != value->valueint || value->valueint < 0 || value->valueint > limits[i]) {
            return 0;
        }
        *values[i] = value->valueint;
    }
    return !!(g->heartseed_progress & HEARTSEED_BINDINGS) == (g->heartseed_bindings == 7) &&
        (!(g->heartseed_progress & HEARTSEED_TAKEN) || (g->heartseed_bindings == 7 && g->heartseed_guardian >= HEARTSEED_SLAIN)) &&
        (g->heartseed_quest_state != 1 || g->heartseed_progress != 7) &&
        (g->heartseed_quest_state < 2 || g->heartseed_progress == 7) &&
        (g->heartseed_guardian != HEARTSEED_FREED || g->heartseed_bindings == 7);
}

static int load_stormbell(const cJSON *root, GameState *g, int version) {
    g->stormbell_quest_state = 0;
    g->stormbell_progress = 0;
    g->stormbell_phase = 0;
    memset(g->stormbell_guards, 0, sizeof(g->stormbell_guards));
    if (version < 206) {
        return 1;
    }
    const char *fields[] = {"stormbell_quest_state", "stormbell_progress", "stormbell_phase"};
    int *values[] = {&g->stormbell_quest_state, &g->stormbell_progress, &g->stormbell_phase};
    const int limits[] = {3, 7, 3};
    for (int i = 0; i < 3; i++) {
        const cJSON *value = cJSON_GetObjectItem(root, fields[i]);
        if (!cJSON_IsNumber(value) || value->valuedouble != value->valueint || value->valueint < 0 || value->valueint > limits[i]) {
            return 0;
        }
        *values[i] = value->valueint;
    }
    const cJSON *guards = cJSON_GetObjectItem(root, "stormbell_guards");
    int count = g->stormbell_phase == 1 ? 2 : g->stormbell_phase ? 3 : 0;
    if (!cJSON_IsArray(guards) || cJSON_GetArraySize(guards) != count) {
        return 0;
    }
    for (int i = 0; i < count; i++) {
        const cJSON *e = cJSON_GetArrayItem(guards, i);
        const char *required[] = {"x", "y", "active", "type", "hp", "max_hp", "attack", "defense", "experience", "move_timer", "frozen_turns", "attack_target_x", "attack_target_y", "revived", "is_boss"};
        if (!cJSON_IsObject(e) || !cJSON_IsString(cJSON_GetObjectItem(e, "name"))) {
            return 0;
        }
        for (int f = 0; f < (int)(sizeof(required) / sizeof(required[0])); f++) {
            const cJSON *value = cJSON_GetObjectItem(e, required[f]);
            if (!cJSON_IsNumber(value) || value->valuedouble != value->valueint) {
                return 0;
            }
        }
    }
    deserialize_enemies(guards, g->stormbell_guards, &count, 3);
    return stormbell_valid(g);
}

static int load_spice_skiff(const cJSON *root, GameState *g, int version) {
    g->spice_skiff_quest_state = 0;
    g->spice_skiff_progress = 0;
    if (version < 205) {
        return 1;
    }
    const cJSON *state = cJSON_GetObjectItem(root, "spice_skiff_quest_state");
    const cJSON *progress = cJSON_GetObjectItem(root, "spice_skiff_progress");
    if (!cJSON_IsNumber(state) || state->valuedouble != state->valueint || state->valueint < 0 || state->valueint > 3 ||
        !cJSON_IsNumber(progress) || progress->valuedouble != progress->valueint || progress->valueint < 0 || progress->valueint > 3 ||
        (state->valueint == 1 && progress->valueint == 3) || (state->valueint >= 2 && progress->valueint != 3)) {
        return 0;
    }
    g->spice_skiff_quest_state = state->valueint;
    g->spice_skiff_progress = progress->valueint;
    return 1;
}

static int load_jail_equipment(const cJSON *root, GameState *g, int version) {
    memset(g->jail_gear, 0, sizeof(g->jail_gear));
    memset(g->jail_guards, 0, sizeof(g->jail_guards));
    memset(g->jail_armory_loot, 0, sizeof(g->jail_armory_loot));
    g->jail_gear_count = 0;
    g->jail_gear_claimed = 0;
    g->jail_gear_state = 0;
    g->jail_main_hand = -1;
    g->jail_off_hand = -1;
    g->jail_armor = -1;
    g->jail_supplies_taken = 0;
    g->jail_armory_initialized = 0;
    g->jail_armory_loot_count = 0;
    if (version < 183) {
        return 1;
    }
    const char *names[] = {"jail_gear_claimed", "jail_gear_state", "jail_main_hand", "jail_off_hand", "jail_armor", "jail_supplies_taken", "jail_armory_initialized"};
    int *fields[] = {&g->jail_gear_claimed, &g->jail_gear_state, &g->jail_main_hand, &g->jail_off_hand, &g->jail_armor, &g->jail_supplies_taken, &g->jail_armory_initialized};
    for (int i = 0; i < 7; i++) {
        const cJSON *value = cJSON_GetObjectItem(root, names[i]);
        if (!cJSON_IsNumber(value) || value->valuedouble != value->valueint) {
            return 0;
        }
        *fields[i] = value->valueint;
    }
    const cJSON *gear = cJSON_GetObjectItem(root, "jail_gear");
    if (!cJSON_IsArray(gear) || cJSON_GetArraySize(gear) > MAX_INVENTORY) {
        return 0;
    }
    const char *required[] = {"active", "type", "attack_bonus", "defense_bonus", "value", "spell_id", "is_ranged", "range"};
    for (int i = 0; i < cJSON_GetArraySize(gear); i++) {
        const cJSON *entry = cJSON_GetArrayItem(gear, i);
        const cJSON *item = cJSON_GetObjectItem(entry, "item");
        if (!cJSON_IsString(cJSON_GetObjectItem(item, "name"))) {
            return 0;
        }
        for (int n = 0; n < 8; n++) {
            if (!cJSON_IsNumber(cJSON_GetObjectItem(item, required[n]))) {
                return 0;
            }
        }
        if (!cJSON_IsNumber(cJSON_GetObjectItem(entry, "active")) ||
            !cJSON_IsNumber(cJSON_GetObjectItem(entry, "x")) || !cJSON_IsNumber(cJSON_GetObjectItem(entry, "y"))) {
            return 0;
        }
    }
    FloorItem stored[MAX_INVENTORY] = {0};
    if (!deserialize_castle_loot(gear, stored, &g->jail_gear_count)) {
        return 0;
    }
    for (int i = 0; i < g->jail_gear_count; i++) {
        g->jail_gear[i] = stored[i].item;
        if (stored[i].item.type != ITEM_WEAPON && stored[i].item.type != ITEM_ARMOR && stored[i].item.type != ITEM_SHIELD) {
            return 0;
        }
    }
    int mask = (1 << g->jail_gear_count) - 1;
    if (g->jail_gear_claimed < 0 || (g->jail_gear_claimed & ~mask) ||
        (g->jail_gear_state != 0 && g->jail_gear_state != 1 && g->jail_gear_state != 3) ||
        (g->jail_gear_state == 0 && g->jail_gear_count) ||
        (g->jail_gear_state == 1 && (!g->jail_gear_count || g->jail_gear_claimed == mask)) ||
        (g->jail_gear_state == 3 && (!g->jail_gear_count || g->jail_gear_claimed != mask)) ||
        g->jail_main_hand < -1 || g->jail_main_hand >= g->jail_gear_count ||
        g->jail_off_hand < -1 || g->jail_off_hand >= g->jail_gear_count ||
        g->jail_armor < -1 || g->jail_armor >= g->jail_gear_count ||
        g->jail_supplies_taken < 0 || g->jail_supplies_taken > 1 ||
        g->jail_armory_initialized < 0 || g->jail_armory_initialized > 1) {
        return 0;
    }
    const cJSON *guards = cJSON_GetObjectItem(root, "jail_guards");
    if (!cJSON_IsArray(guards) || cJSON_GetArraySize(guards) != (g->jail_armory_initialized ? 3 : 0)) {
        return 0;
    }
    const char *guard_fields[] = {"x", "y", "active", "type", "hp", "max_hp", "attack", "defense", "experience", "move_timer", "frozen_turns", "attack_target_x", "attack_target_y"};
    for (int i = 0; i < cJSON_GetArraySize(guards); i++) {
        const cJSON *guard = cJSON_GetArrayItem(guards, i);
        if (!cJSON_IsString(cJSON_GetObjectItem(guard, "name"))) {
            return 0;
        }
        for (int n = 0; n < 13; n++) {
            if (!cJSON_IsNumber(cJSON_GetObjectItem(guard, guard_fields[n]))) {
                return 0;
            }
        }
    }
    int count;
    deserialize_enemies(guards, g->jail_guards, &count, 3);
    for (int i = 0; i < count; i++) {
        const Enemy *guard = &g->jail_guards[i];
        if (guard->type != ENEMY_ROYAL_GUARD || guard->x < 0 || guard->x >= JAIL_ROOM_W || guard->y < 0 || guard->y >= JAIL_ROOM_H) {
            return 0;
        }
    }
    if (!deserialize_castle_loot(cJSON_GetObjectItem(root, "jail_armory_loot"), g->jail_armory_loot, &g->jail_armory_loot_count)) {
        return 0;
    }
    for (int i = 0; i < g->jail_armory_loot_count; i++) {
        const FloorItem *item = &g->jail_armory_loot[i];
        if (item->x < 0 || item->x >= JAIL_ROOM_W || item->y < 0 || item->y >= JAIL_ROOM_H) {
            return 0;
        }
    }
    return 1;
}

static cJSON *serialize_mangrove(const MangroveState *state) {
    cJSON *delta = cJSON_CreateObject();
    cJSON_AddNumberToObject(delta, "initialized", state->initialized);
    cJSON_AddNumberToObject(delta, "stage", state->stage);
    cJSON_AddNumberToObject(delta, "boat_x", state->boat_x);
    cJSON_AddNumberToObject(delta, "boat_y", state->boat_y);
    cJSON_AddNumberToObject(delta, "boat_dx", state->boat_dx);
    cJSON_AddNumberToObject(delta, "boat_dy", state->boat_dy);
    cJSON_AddNumberToObject(delta, "aboard", state->aboard);
    cJSON_AddNumberToObject(delta, "searched", state->searched);
    char *explored = bytes_to_base64(state->explored, sizeof(state->explored));
    cJSON_AddStringToObject(delta, "explored", explored);
    free(explored);
    cJSON_AddItemToObject(delta, "loot", serialize_castle_loot(state->loot, state->loot_count));
    cJSON_AddItemToObject(delta, "enemies", serialize_enemies(state->enemies, state->enemy_count));
    return delta;
}

static cJSON *serialize_giants(const GameState *g) {
    cJSON *array = cJSON_CreateArray();
    for (int n = 0; n < GIANTS_DEPTH; n++) {
        GiantsStage state = g->giants_crown[n];
        if (g->location == LOCATION_GIANTS_CROWN && g->level == n + 1) {
            giants_capture(g, &state);
        }
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", state.valid);
        if (state.valid) {
            Map map;
            giants_unpack(&state, &map);
            cJSON_AddItemToObject(entry, "map", serialize_map(&map));
            cJSON_AddItemToObject(entry, "enemies", serialize_enemies(state.enemies, state.enemy_count));
            cJSON_AddItemToObject(entry, "loot", serialize_castle_loot(state.loot, state.loot_count));
        }
        cJSON_AddItemToArray(array, entry);
    }
    return array;
}

static cJSON *serialize_ironfang(const GameState *g) {
    cJSON *array = cJSON_CreateArray();
    for (int n = 0; n < IRON_DEPTH; n++) {
        IronfangStage state = g->ironfang[n];
        if (g->location == LOCATION_IRONFANG && g->level == n + 1) {
            ironfang_capture(g, &state);
        }
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", state.valid);
        cJSON_AddNumberToObject(entry, "cart_x", state.cart_x);
        cJSON_AddNumberToObject(entry, "cart_y", state.cart_y);
        cJSON_AddNumberToObject(entry, "aboard", state.aboard);
        cJSON_AddNumberToObject(entry, "route", state.route);
        cJSON_AddNumberToObject(entry, "ore_loads", state.ore_loads);
        cJSON_AddNumberToObject(entry, "ore_fuse", state.ore_fuse);
        if (state.valid) {
            Map map;
            ironfang_unpack(&state, &map);
            cJSON_AddItemToObject(entry, "map", serialize_map(&map));
            cJSON_AddItemToObject(entry, "enemies", serialize_enemies(state.enemies, state.enemy_count));
            cJSON_AddItemToObject(entry, "loot", serialize_castle_loot(state.loot, state.loot_count));
        }
        cJSON_AddItemToArray(array, entry);
    }
    return array;
}

static cJSON *serialize_stormwatch(const GameState *g) {
    cJSON *array = cJSON_CreateArray();
    for (int n = 0; n < STORM_DEPTH; n++) {
        StormwatchStage state = g->stormwatch[n];
        if (g->location == LOCATION_STORMWATCH && g->level == n + 1) {
            stormwatch_capture(g, &state);
        }
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", state.valid);
        cJSON_AddNumberToObject(entry, "braced", state.braced);
        cJSON_AddNumberToObject(entry, "wind_elapsed_ms", state.wind_elapsed_ms);
        for (int lane = 0; lane < 2; lane++) {
            char key[32];
            snprintf(key, sizeof(key), "phase%d", lane);
            cJSON_AddNumberToObject(entry, key, state.phase[lane]);
            snprintf(key, sizeof(key), "direction%d", lane);
            cJSON_AddNumberToObject(entry, key, state.direction[lane]);
            snprintf(key, sizeof(key), "next_direction%d", lane);
            cJSON_AddNumberToObject(entry, key, state.next_direction[lane]);
            snprintf(key, sizeof(key), "warned%d", lane);
            char *encoded = bytes_to_base64(state.warned[lane], STORM_MASK_BYTES);
            cJSON_AddStringToObject(entry, key, encoded);
            free(encoded);
        }
        if (state.valid) {
            Map map;
            stormwatch_unpack(&state, &map);
            cJSON_AddItemToObject(entry, "map", serialize_map(&map));
            cJSON_AddItemToObject(entry, "enemies", serialize_enemies(state.enemies, state.enemy_count));
            cJSON_AddItemToObject(entry, "loot", serialize_castle_loot(state.loot, state.loot_count));
        }
        cJSON_AddItemToArray(array, entry);
    }
    return array;
}

static int stormwatch_number(const cJSON *object, const char *key, int *value) {
    const cJSON *field = cJSON_GetObjectItem(object, key);
    if (!cJSON_IsNumber(field) || field->valuedouble != field->valueint) {
        return 0;
    }
    *value = field->valueint;
    return 1;
}


static cJSON *serialize_northroad(const GameState *g) {
    NorthRoadCache state = g->northroad;
    northroad_capture(g, &state);
    cJSON *entry = cJSON_CreateObject();
    cJSON_AddNumberToObject(entry, "valid", state.valid);
    if (state.valid) {
        cJSON_AddNumberToObject(entry, "from_castle", state.from_castle);
        cJSON_AddNumberToObject(entry, "level_cleared", state.level_cleared);
        cJSON_AddItemToObject(entry, "map", serialize_map(&state.map));
        cJSON_AddItemToObject(entry, "enemies", serialize_enemies(state.enemies, state.enemy_count));
        cJSON_AddItemToObject(entry, "loot", serialize_castle_loot(state.loot, state.loot_count));
    }
    return entry;
}

static int load_northroad(const cJSON *root, GameState *g, int version) {
    NorthRoadCache *state = &g->northroad;
    memset(state, 0, sizeof(*state));
    if (version < 202) {
        return 1;
    }
    const cJSON *entry = cJSON_GetObjectItem(root, "northroad");
    if (!cJSON_IsObject(entry) || !stormwatch_number(entry, "valid", &state->valid) ||
        (state->valid != 0 && state->valid != 1)) {
        return 0;
    }
    if (!state->valid) {
        return 1;
    }
    const cJSON *map = cJSON_GetObjectItem(entry, "map");
    const cJSON *enemies = cJSON_GetObjectItem(entry, "enemies");
    if (!stormwatch_number(entry, "from_castle", &state->from_castle) ||
        !stormwatch_number(entry, "level_cleared", &state->level_cleared) ||
        !cJSON_IsString(cJSON_GetObjectItem(map, "tiles_b64")) ||
        !cJSON_IsString(cJSON_GetObjectItem(map, "explored_b64")) ||
        !cJSON_IsArray(enemies) || cJSON_GetArraySize(enemies) > NORTH_ENEMIES ||
        !deserialize_castle_loot(cJSON_GetObjectItem(entry, "loot"), state->loot, &state->loot_count)) {
        return 0;
    }
    const char *fields[] = {"room_count", "stairs_up_x", "stairs_up_y", "stairs_down_x", "stairs_down_y"};
    for (int i = 0; i < 5; i++) {
        if (!cJSON_IsNumber(cJSON_GetObjectItem(map, fields[i]))) {
            return 0;
        }
    }
    for (int i = 0; i < cJSON_GetArraySize(enemies); i++) {
        const cJSON *enemy = cJSON_GetArrayItem(enemies, i);
        const char *required[] = {"x", "y", "active", "type", "hp", "max_hp", "attack", "defense", "experience", "move_timer", "frozen_turns", "attack_target_x", "attack_target_y", "boss_aux_x", "boss_aux_y"};
        if (!cJSON_IsString(cJSON_GetObjectItem(enemy, "name"))) {
            return 0;
        }
        for (int n = 0; n < (int)(sizeof(required) / sizeof(required[0])); n++) {
            int value;
            if (!stormwatch_number(enemy, required[n], &value)) {
                return 0;
            }
        }
    }
    deserialize_map(map, &state->map);
    deserialize_enemies(enemies, state->enemies, &state->enemy_count, NORTH_ENEMIES);
    return 1;
}

static int load_giants(const cJSON *root, GameState *g, int version) {
    memset(g->giants_crown, 0, sizeof(g->giants_crown));
    g->giants_crown_defeated = 0;
    if (version < 203) {
        return 1;
    }
    const cJSON *array = cJSON_GetObjectItem(root, "giants_crown");
    if (!cJSON_IsArray(array) || cJSON_GetArraySize(array) != GIANTS_DEPTH ||
        !stormwatch_number(root, "giants_crown_defeated", &g->giants_crown_defeated)) {
        return 0;
    }
    for (int n = 0; n < GIANTS_DEPTH; n++) {
        GiantsStage *state = &g->giants_crown[n];
        const cJSON *entry = cJSON_GetArrayItem(array, n);
        if (!stormwatch_number(entry, "valid", &state->valid) || state->valid < 0 || state->valid > 1) {
            return 0;
        }
        if (!state->valid) {
            continue;
        }
        const cJSON *map = cJSON_GetObjectItem(entry, "map");
        const cJSON *enemies = cJSON_GetObjectItem(entry, "enemies");
        const char *fields[] = {"room_count", "stairs_up_x", "stairs_up_y", "stairs_down_x", "stairs_down_y"};
        for (int f = 0; f < 5; f++) {
            int value;
            if (!stormwatch_number(map, fields[f], &value) || (f == 0 && value != 0)) {
                return 0;
            }
        }
        if (!cJSON_IsString(cJSON_GetObjectItem(map, "tiles_b64")) || !cJSON_IsString(cJSON_GetObjectItem(map, "explored_b64")) ||
            !cJSON_IsArray(enemies) || cJSON_GetArraySize(enemies) > AREA_ENEMY_LIMIT ||
            !deserialize_castle_loot(cJSON_GetObjectItem(entry, "loot"), state->loot, &state->loot_count)) {
            return 0;
        }
        for (int e = 0; e < cJSON_GetArraySize(enemies); e++) {
            const cJSON *enemy = cJSON_GetArrayItem(enemies, e);
            const char *required[] = {"x", "y", "active", "type", "hp", "max_hp", "attack", "defense", "experience", "move_timer", "frozen_turns", "attack_target_x", "attack_target_y", "attack_phase"};
            if (!cJSON_IsString(cJSON_GetObjectItem(enemy, "name"))) {
                return 0;
            }
            for (int f = 0; f < (int)(sizeof(required) / sizeof(required[0])); f++) {
                int value;
                if (!stormwatch_number(enemy, required[f], &value)) {
                    return 0;
                }
            }
        }
        Map scratch = {0};
        deserialize_map(map, &scratch);
        giants_pack(state, &scratch);
        deserialize_enemies(enemies, state->enemies, &state->enemy_count, AREA_ENEMY_LIMIT);
    }
    return 1;
}

static int load_ironfang(const cJSON *root, GameState *g, int version) {
    memset(g->ironfang, 0, sizeof(g->ironfang));
    g->ironfang_defeated = 0;
    if (version < 200) {
        return 1;
    }
    const cJSON *array = cJSON_GetObjectItem(root, "ironfang");
    if (!cJSON_IsArray(array) || cJSON_GetArraySize(array) != IRON_DEPTH ||
        !stormwatch_number(root, "ironfang_defeated", &g->ironfang_defeated)) {
        return 0;
    }
    for (int n = 0; n < IRON_DEPTH; n++) {
        IronfangStage *state = &g->ironfang[n];
        const cJSON *entry = cJSON_GetArrayItem(array, n);
        if (!stormwatch_number(entry, "valid", &state->valid) ||
            !stormwatch_number(entry, "cart_x", &state->cart_x) || !stormwatch_number(entry, "cart_y", &state->cart_y) ||
            !stormwatch_number(entry, "aboard", &state->aboard) || !stormwatch_number(entry, "route", &state->route) ||
            !stormwatch_number(entry, "ore_loads", &state->ore_loads) || !stormwatch_number(entry, "ore_fuse", &state->ore_fuse)) {
            return 0;
        }
        if (!state->valid) {
            continue;
        }
        const cJSON *map = cJSON_GetObjectItem(entry, "map");
        const cJSON *enemies = cJSON_GetObjectItem(entry, "enemies");
        const char *fields[] = {"room_count", "stairs_up_x", "stairs_up_y", "stairs_down_x", "stairs_down_y"};
        for (int f = 0; f < 5; f++) {
            if (!cJSON_IsNumber(cJSON_GetObjectItem(map, fields[f]))) {
                return 0;
            }
        }
        if (!cJSON_IsString(cJSON_GetObjectItem(map, "tiles_b64")) || !cJSON_IsString(cJSON_GetObjectItem(map, "explored_b64")) ||
            !cJSON_IsArray(enemies) || cJSON_GetArraySize(enemies) > AREA_ENEMY_LIMIT ||
            !deserialize_castle_loot(cJSON_GetObjectItem(entry, "loot"), state->loot, &state->loot_count)) {
            return 0;
        }
        for (int e = 0; e < cJSON_GetArraySize(enemies); e++) {
            const cJSON *enemy = cJSON_GetArrayItem(enemies, e);
            const char *required[] = {"x", "y", "active", "type", "hp", "max_hp", "attack", "defense", "experience", "move_timer", "frozen_turns", "attack_target_x", "attack_target_y"};
            if (!cJSON_IsString(cJSON_GetObjectItem(enemy, "name"))) {
                return 0;
            }
            for (int f = 0; f < (int)(sizeof(required) / sizeof(required[0])); f++) {
                if (!cJSON_IsNumber(cJSON_GetObjectItem(enemy, required[f]))) {
                    return 0;
                }
            }
        }
        Map scratch = {0};
        deserialize_map(map, &scratch);
        ironfang_pack(state, &scratch);
        deserialize_enemies(enemies, state->enemies, &state->enemy_count, AREA_ENEMY_LIMIT);
    }
    return 1;
}

static int load_stormwatch(const cJSON *root, GameState *g, int version) {
    memset(g->stormwatch, 0, sizeof(g->stormwatch));
    g->stormwatch_defeated = 0;
    if (version < 198) {
        return 1;
    }
    const cJSON *array = cJSON_GetObjectItem(root, "stormwatch");
    if (!cJSON_IsArray(array) || cJSON_GetArraySize(array) != STORM_DEPTH ||
        !stormwatch_number(root, "stormwatch_defeated", &g->stormwatch_defeated)) {
        return 0;
    }
    for (int n = 0; n < STORM_DEPTH; n++) {
        StormwatchStage *state = &g->stormwatch[n];
        const cJSON *entry = cJSON_GetArrayItem(array, n);
        if (!stormwatch_number(entry, "valid", &state->valid) || !stormwatch_number(entry, "braced", &state->braced)) {
            return 0;
        }
        // Earlier Stormwatch saves start with a full second until the next step.
        if (version >= 199 && !stormwatch_number(entry, "wind_elapsed_ms", &state->wind_elapsed_ms)) {
            return 0;
        }
        for (int lane = 0; lane < 2; lane++) {
            char key[32];
            snprintf(key, sizeof(key), "phase%d", lane);
            if (!stormwatch_number(entry, key, &state->phase[lane])) {
                return 0;
            }
            snprintf(key, sizeof(key), "direction%d", lane);
            if (!stormwatch_number(entry, key, &state->direction[lane])) {
                return 0;
            }
            snprintf(key, sizeof(key), "next_direction%d", lane);
            if (!stormwatch_number(entry, key, &state->next_direction[lane])) {
                return 0;
            }
            snprintf(key, sizeof(key), "warned%d", lane);
            const cJSON *warning = cJSON_GetObjectItem(entry, key);
            if (!cJSON_IsString(warning) || strlen(warning->valuestring) != 4 * ((STORM_MASK_BYTES + 2) / 3)) {
                return 0;
            }
            base64_to_bytes(warning->valuestring, state->warned[lane], STORM_MASK_BYTES);
        }
        if (!state->valid) {
            continue;
        }
        const cJSON *map = cJSON_GetObjectItem(entry, "map");
        const char *fields[] = {"room_count", "stairs_up_x", "stairs_up_y", "stairs_down_x", "stairs_down_y"};
        for (int f = 0; f < 5; f++) {
            if (!cJSON_IsNumber(cJSON_GetObjectItem(map, fields[f]))) {
                return 0;
            }
        }
        const cJSON *enemies = cJSON_GetObjectItem(entry, "enemies");
        if (!cJSON_IsString(cJSON_GetObjectItem(map, "tiles_b64")) || !cJSON_IsString(cJSON_GetObjectItem(map, "explored_b64")) ||
            !cJSON_IsArray(enemies) || cJSON_GetArraySize(enemies) > AREA_ENEMY_LIMIT ||
            !deserialize_castle_loot(cJSON_GetObjectItem(entry, "loot"), state->loot, &state->loot_count)) {
            return 0;
        }
        for (int i = 0; i < cJSON_GetArraySize(enemies); i++) {
            const cJSON *enemy = cJSON_GetArrayItem(enemies, i);
            const char *required[] = {"x", "y", "active", "type", "hp", "max_hp", "attack", "defense", "experience", "move_timer", "frozen_turns", "attack_target_x", "attack_target_y"};
            if (!cJSON_IsString(cJSON_GetObjectItem(enemy, "name"))) {
                return 0;
            }
            for (int f = 0; f < (int)(sizeof(required) / sizeof(required[0])); f++) {
                if (!cJSON_IsNumber(cJSON_GetObjectItem(enemy, required[f]))) {
                    return 0;
                }
            }
        }
        Map scratch = {0};
        deserialize_map(map, &scratch);
        stormwatch_pack(state, &scratch);
        deserialize_enemies(enemies, state->enemies, &state->enemy_count, AREA_ENEMY_LIMIT);
    }
    return 1;
}

// Existing expeditions remain intact. Only the previously closed east road is
// opened, preserving exploration, portals and item underlays on its paving.
static void migrate_stormwatch_road(GameState *g) {
    int from = 0;
    int to = -1;
    int y = 0;
    if (g->location == LOCATION_DAWNMERE) {
        for (int row = 10; row <= 14; row++) {
            g->map.tiles[row][TOWN_W - 1] = TILE_TOWN_EXIT;
        }
        from = 21;
        to = TOWN_W - 2;
        y = 12;
    } else if (g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_DAWNMERE) {
        from = TOWN_SPRITE_X + TOWN_SPRITE_W;
        to = TOWN_ROAD_W - 2;
        y = TOWN_ROAD_H / 2;
        g->map.tiles[y][from - 1] = TILE_TOWN_ROAD_ENTRY_EAST;
        g->map.tiles[y][TOWN_ROAD_W - 1] = TILE_TOWN_ROAD_EXIT_EAST;
    }
    for (int x = from; x <= to; x++) {
        if (g->map.tiles[y][x] != TILE_PORTAL) {
            g->map.tiles[y][x] = TILE_TOWN_PATH;
        }
        for (int n = 0; n < g->floor_item_count; n++) {
            FloorItem *item = &g->floor_items[n];
            if (item->active && item->x == x && item->y == y) {
                item->underlying_tile = TILE_TOWN_PATH;
                g->map.tiles[y][x] = TILE_ITEM;
            }
        }
        if (g->portal_active && g->portal_location == g->location && g->portal_x == x && g->portal_y == y) {
            g->portal_origin_tile = TILE_TOWN_PATH;
        }
    }
}

int save_game(const GameState *g, int slot) {
    if (!stormbell_valid(g) || !giants_valid(g) || !northroad_valid(g) || !mangrove_state_valid(g) || !stormwatch_valid(g) || !ironfang_valid(g)) {
        return 0;
    }
    if (g->retired_coast_item_count < 0 || g->retired_coast_item_count > RETIRED_LOOT_STAGES * MAX_FLOOR_ITEMS) {
        return 0;
    }
    mkdir("saves", 0755);
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "save_version", 206);
    cJSON_AddItemToObject(root, "giants_crown", serialize_giants(g));
    cJSON_AddNumberToObject(root, "giants_crown_defeated", g->giants_crown_defeated);
    cJSON_AddItemToObject(root, "northroad", serialize_northroad(g));
    cJSON_AddItemToObject(root, "ironfang", serialize_ironfang(g));
    cJSON_AddNumberToObject(root, "ironfang_defeated", g->ironfang_defeated);
    cJSON_AddItemToObject(root, "stormwatch", serialize_stormwatch(g));
    cJSON_AddNumberToObject(root, "stormwatch_defeated", g->stormwatch_defeated);
    cJSON_AddNumberToObject(root, "stormbell_quest_state", g->stormbell_quest_state);
    cJSON_AddNumberToObject(root, "stormbell_progress", g->stormbell_progress);
    cJSON_AddNumberToObject(root, "stormbell_phase", g->stormbell_phase);
    Enemy bell_guards[3];
    stormbell_capture(g, bell_guards);
    cJSON_AddItemToObject(root, "stormbell_guards", serialize_enemies(bell_guards, g->stormbell_phase == 1 ? 2 : g->stormbell_phase ? 3 : 0));
    MangroveState mangrove = g->mangroves;
    mangrove_capture(g, &mangrove);
    cJSON_AddItemToObject(root, "mangroves", serialize_mangrove(&mangrove));
    cJSON *stages = cJSON_CreateArray();
    for (int i = 0; i < MANGROVE_DEPTH; i++) {
        const MangroveState *state = g->location == LOCATION_MANGROVES && g->level == i + 1 ? &mangrove : &g->mangrove_stages[i];
        cJSON_AddItemToArray(stages, serialize_mangrove(state));
    }
    cJSON_AddItemToObject(root, "mangrove_stages", stages);
    cJSON_AddNumberToObject(root, "mangrove_defeated", g->mangrove_defeated);
    cJSON_AddNumberToObject(root, "spice_skiff_quest_state", g->spice_skiff_quest_state);
    cJSON_AddNumberToObject(root, "spice_skiff_progress", g->spice_skiff_progress);
    cJSON *retired = cJSON_CreateArray();
    for (int stage = 0; stage < RETIRED_LOOT_STAGES; stage++) {
        FloorItem items[MAX_FLOOR_ITEMS] = {0};
        int count = 0;
        for (int i = stage * MAX_FLOOR_ITEMS; i < g->retired_coast_item_count && i < (stage + 1) * MAX_FLOOR_ITEMS; i++) {
            items[count].active = 1;
            items[count++].item = g->retired_coast_items[i];
        }
        cJSON_AddItemToArray(retired, serialize_castle_loot(items, count));
    }
    cJSON_AddItemToObject(root, "retired_coast_loot", retired);
    cJSON_AddNumberToObject(root, "jail_quest_state", g->jail_quest_state);
    cJSON_AddNumberToObject(root, "prisoner_x", g->prisoner_x);
    cJSON_AddNumberToObject(root, "prisoner_y", g->prisoner_y);
    cJSON_AddNumberToObject(root, "jail_gear_claimed", g->jail_gear_claimed);
    cJSON_AddNumberToObject(root, "jail_gear_state", g->jail_gear_state);
    cJSON_AddNumberToObject(root, "jail_main_hand", g->jail_main_hand);
    cJSON_AddNumberToObject(root, "jail_off_hand", g->jail_off_hand);
    cJSON_AddNumberToObject(root, "jail_armor", g->jail_armor);
    cJSON_AddNumberToObject(root, "jail_supplies_taken", g->jail_supplies_taken);
    cJSON_AddNumberToObject(root, "jail_armory_initialized", g->jail_armory_initialized);
    FloorItem stored_gear[MAX_INVENTORY] = {0};
    for (int i = 0; i < g->jail_gear_count; i++) {
        stored_gear[i].item = g->jail_gear[i];
    }
    cJSON_AddItemToObject(root, "jail_gear", serialize_castle_loot(stored_gear, g->jail_gear_count));
    cJSON_AddItemToObject(root, "jail_guards", serialize_enemies(g->jail_guards, g->jail_armory_initialized ? 3 : 0));
    cJSON_AddItemToObject(root, "jail_armory_loot", serialize_castle_loot(g->jail_armory_loot, g->jail_armory_loot_count));
    cJSON_AddNumberToObject(root, "dungeon_entry_town", g->dungeon_entry_town);
    cJSON_AddNumberToObject(root, "dungeon_portal_town", g->dungeon_portal_town);
    cJSON_AddNumberToObject(root, "forest_entry_town", g->forest_entry_town);
    cJSON_AddNumberToObject(root, "forest_portal_town", g->forest_portal_town);
    cJSON_AddNumberToObject(root, "swamp_entry_town", g->swamp_entry_town);
    cJSON_AddNumberToObject(root, "swamp_portal_town", g->swamp_portal_town);
    cJSON_AddNumberToObject(root, "mountain_entry_town", g->mountain_entry_town);
    cJSON_AddNumberToObject(root, "mountain_portal_town", g->mountain_portal_town);

    // Player
    cJSON *player = cJSON_CreateObject();
    cJSON_AddStringToObject(player, "name",              g->player.name);
    cJSON_AddNumberToObject(player, "x",                 g->player.x);
    cJSON_AddNumberToObject(player, "y",                 g->player.y);
    cJSON_AddNumberToObject(player, "hp",                g->player.hp);
    cJSON_AddNumberToObject(player, "max_hp",            g->player.max_hp);
    cJSON_AddNumberToObject(player, "mp",                g->player.mp);
    cJSON_AddNumberToObject(player, "max_mp",            g->player.max_mp);
    cJSON_AddNumberToObject(player, "attack",            g->player.attack);
    cJSON_AddNumberToObject(player, "defense",           g->player.defense);
    cJSON_AddNumberToObject(player, "level",             g->player.level);
    cJSON_AddNumberToObject(player, "experience",        g->player.experience);
    cJSON_AddNumberToObject(player, "experience_next",   g->player.experience_next);
    cJSON_AddNumberToObject(player, "last_dx",           g->player.last_dx);
    cJSON_AddNumberToObject(player, "last_dy",           g->player.last_dy);
    cJSON_AddNumberToObject(player, "equipped_spell",    g->player.equipped_spell);
    cJSON_AddNumberToObject(player, "frozen_turns",      g->player.frozen_turns);
    cJSON_AddNumberToObject(player, "freeze_recovery", g->player.freeze_recovery);
    cJSON_AddNumberToObject(root, "kraken_bow_unclaimed", g->kraken_bow_unclaimed);
    cJSON_AddNumberToObject(root, "sandstorm_staff_unclaimed", g->sandstorm_staff_unclaimed);
    cJSON_AddNumberToObject(root, "desert_pharaoh_reward_state", g->desert_pharaoh_reward_state);
    cJSON_AddNumberToObject(root, "cinder_lord_reward_pending", g->cinder_lord_reward_pending);
    cJSON_AddNumberToObject(root, "prism_sovereign_reward_pending", g->prism_sovereign_reward_pending);
    cJSON_AddNumberToObject(root, "stonewake_warhammer_state", g->stonewake_warhammer_state);
    cJSON_AddNumberToObject(root, "sun_guardian_reward_state", g->sun_guardian_reward_state);
    cJSON_AddNumberToObject(root, "red_dragon_reward_state", g->red_dragon_reward_state);
    cJSON_AddNumberToObject(player, "known_spell_count", g->player.known_spell_count);
    cJSON_AddNumberToObject(player, "player_class",      g->player.player_class);
    cJSON_AddNumberToObject(player, "arrows", g->player.arrows);

    // Known spells
    cJSON *spells = cJSON_CreateArray();
    for (int i = 0; i < g->player.known_spell_count; i++) {
        const Spell *sp = &g->player.known_spells[i];
        cJSON *s = cJSON_CreateObject();
        cJSON_AddStringToObject(s, "name",     sp->name);
        cJSON_AddNumberToObject(s, "id",       sp->id);
        cJSON_AddNumberToObject(s, "type",     sp->type);
        cJSON_AddNumberToObject(s, "mp_cost",  sp->mp_cost);
        cJSON_AddNumberToObject(s, "damage",   sp->damage);
        cJSON_AddNumberToObject(s, "range",    sp->range);
        cJSON_AddNumberToObject(s, "radius",   sp->radius);
        cJSON_AddNumberToObject(s, "rank",     sp->rank);
        cJSON_AddItemToArray(spells, s);
    }
    cJSON_AddItemToObject(player, "spells", spells);
    cJSON_AddItemToObject(root, "player", player);

    // Game state
    cJSON_AddNumberToObject(root, "level",             g->level);
    cJSON_AddNumberToObject(root, "level_cleared",     g->level_cleared);
    cJSON_AddNumberToObject(root, "max_level_reached", g->max_level_reached);
    cJSON_AddNumberToObject(root, "max_forest_level_reached",
        g->max_forest_level_reached);
    cJSON_AddNumberToObject(root, "max_mountain_level_reached",
        g->max_mountain_level_reached);
    cJSON_AddNumberToObject(root, "max_coast_level_reached",
        g->max_coast_level_reached);
    cJSON_AddNumberToObject(root, "max_swamp_level_reached",
        g->max_swamp_level_reached);
    cJSON_AddNumberToObject(root, "max_dragonspine_level_reached",
        g->max_dragonspine_level_reached);
    cJSON_AddNumberToObject(root, "max_frostfell_level_reached",
        g->max_frostfell_level_reached);
    cJSON_AddNumberToObject(root, "max_desert_level_reached",
        g->max_desert_level_reached);
    cJSON_AddNumberToObject(root, "max_ashen_level_reached", g->max_ashen_level_reached);
    cJSON_AddNumberToObject(root, "max_glassdeep_level_reached", g->max_glassdeep_level_reached);
    cJSON_AddNumberToObject(root, "max_catacombs_level_reached", g->max_catacombs_level_reached);
    cJSON_AddNumberToObject(root, "catacombs_mantle_unclaimed", g->catacombs_mantle_unclaimed);
    cJSON_AddNumberToObject(root, "max_temple_level_reached",
        g->max_temple_level_reached);
    cJSON_AddNumberToObject(root, "message_count",     g->message_count);
    cJSON_AddNumberToObject(root, "gold",              g->gold);
    cJSON_AddNumberToObject(root, "labyrinth_entry_town", g->labyrinth_entry_town);
    cJSON_AddNumberToObject(root, "rook_quest_state", g->rook_quest_state);
    cJSON_AddNumberToObject(root, "innkeeper_quest_state", g->innkeeper_quest_state);
    cJSON_AddNumberToObject(root, "rook_labyrinth_switches",
        g->rook_labyrinth_switches);
    cJSON_AddNumberToObject(root, "rook_quest_completions",
        g->rook_quest_completions);
    cJSON_AddNumberToObject(root, "score",             g->score);
    cJSON_AddNumberToObject(root, "equipped_main_hand",
        g->equipped_main_hand);
    cJSON_AddNumberToObject(root, "equipped_off_hand",
        g->equipped_off_hand);
    cJSON_AddNumberToObject(root, "equipped_armor",    g->equipped_armor);
    cJSON_AddNumberToObject(root, "location",          g->location);
    cJSON_AddNumberToObject(root, "road_town", g->road_town);
    cJSON_AddNumberToObject(root, "debug_passages_unlocked", g->debug_passages_unlocked);
    cJSON_AddNumberToObject(root, "max_elderwood_level_reached", g->max_elderwood_level_reached);
    cJSON_AddNumberToObject(root, "elderwood_defeated", g->elderwood_defeated);
    cJSON_AddNumberToObject(root, "heartseed_quest_state", g->heartseed_quest_state);
    cJSON_AddNumberToObject(root, "heartseed_progress", g->heartseed_progress);
    cJSON_AddNumberToObject(root, "heartseed_bindings", g->heartseed_bindings);
    cJSON_AddNumberToObject(root, "heartseed_sites", g->heartseed_sites);
    cJSON_AddNumberToObject(root, "heartseed_guardian", g->heartseed_guardian);
    cJSON_AddNumberToObject(root, "elderwood_entry_town", g->elderwood_entry_town);
    cJSON_AddNumberToObject(root, "catacombs_entry_town", g->catacombs_entry_town);
    cJSON_AddNumberToObject(root, "northern_portal_town", g->northern_portal_town);
    cJSON_AddNumberToObject(root, "last_town_visited", g->last_town_visited);
    cJSON_AddNumberToObject(root, "return_portal_town", g->return_portal_town);
    cJSON *elderwood = cJSON_CreateArray();
    for (int i = 0; i < ELDERWOOD_DEPTH; i++) {
        const ElderwoodCache *cache = &g->elderwood_cache[i];
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", cache->valid);
        cJSON_AddNumberToObject(entry, "level_cleared", cache->level_cleared);
        if (cache->valid) {
            Map scratch;
            elderwood_unpack(cache, &scratch);
            cJSON_AddItemToObject(entry, "map", serialize_map(&scratch));
            cJSON_AddItemToObject(entry, "enemies", serialize_enemies(cache->enemies, cache->enemy_count));
        }
        cJSON_AddItemToArray(elderwood, entry);
    }
    cJSON_AddItemToObject(root, "elderwood_cache", elderwood);
    cJSON_AddNumberToObject(root, "approach_route", g->approach_route);
    cJSON_AddNumberToObject(root, "lich_reward_state", g->lich_reward_state);
    cJSON_AddNumberToObject(root, "necromancer_reward_state", g->necromancer_reward_state);
    cJSON_AddNumberToObject(root, "goblin_king_reward_state", g->goblin_king_reward_state);
    cJSON_AddNumberToObject(root, "grave_marshal_reward_state", g->grave_marshal_reward_state);
    cJSON_AddNumberToObject(root, "minotaur_reward_state", g->minotaur_reward_state);
    cJSON_AddNumberToObject(root, "swamp_demon_reward_state", g->swamp_demon_reward_state);
    cJSON_AddNumberToObject(root, "drowned_queen_reward_state", g->drowned_queen_reward_state);
    cJSON_AddNumberToObject(root, "dungeon_key_found", g->dungeon_key_found);
    cJSON_AddNumberToObject(root, "dungeon_crypt_keys",
        g->dungeon_crypt_keys);
    cJSON_AddNumberToObject(root, "portal_active",     g->portal_active);
    cJSON_AddNumberToObject(root, "portal_level",      g->portal_level);
    cJSON_AddNumberToObject(root, "portal_location",   g->portal_location);
    cJSON_AddNumberToObject(root, "portal_x",          g->portal_x);
    cJSON_AddNumberToObject(root, "portal_y",          g->portal_y);
    cJSON_AddNumberToObject(root, "portal_origin_tile", g->portal_origin_tile);
    cJSON_AddNumberToObject(root, "defeated_bosses", g->defeated_bosses);
    cJSON_AddNumberToObject(root, "elowen_quest_state",
        g->elowen_quest_state);
    cJSON_AddNumberToObject(root, "elowen_seals_restored",
        g->elowen_seals_restored);
    cJSON_AddNumberToObject(root, "dain_quest_state", g->dain_quest_state);
    cJSON_AddNumberToObject(root, "dain_map_fragments",
        g->dain_map_fragments);
    cJSON_AddNumberToObject(root, "alder_quest_state",
        g->alder_quest_state);
    cJSON_AddNumberToObject(root, "alder_wardens_rescued",
        g->alder_wardens_rescued);
    cJSON_AddNumberToObject(root, "mara_quest_state", g->mara_quest_state);
    cJSON_AddNumberToObject(root, "mara_beacons_lit", g->mara_beacons_lit);
    cJSON_AddNumberToObject(root, "cain_scroll_given", g->cain_scroll_given);
    cJSON_AddNumberToObject(root, "island_travel_unlocked",
        g->island_travel_unlocked);
    cJSON_AddNumberToObject(root, "dragon_treasure_quest_state",
        g->dragon_treasure_quest_state);
    cJSON_AddNumberToObject(root, "sunscar_lamp_quest_state", g->sunscar_lamp_quest_state);
    cJSON_AddNumberToObject(root, "emberforge_quest_state", g->emberforge_quest_state);
    cJSON_AddNumberToObject(root, "emberforge_progress", g->emberforge_progress);
    cJSON_AddNumberToObject(root, "emberforge_encounters", g->emberforge_encounters);
    cJSON_AddNumberToObject(root, "frostfell_quest_state", g->frostfell_quest_state);
    cJSON_AddNumberToObject(root, "frostfell_quest_progress", g->frostfell_quest_progress);
    cJSON_AddNumberToObject(root, "frostfell_quest_encounters", g->frostfell_quest_encounters);
    cJSON_AddNumberToObject(root, "glassdeep_quest_state", g->glassdeep_quest_state);
    cJSON_AddNumberToObject(root, "glassdeep_quest_progress", g->glassdeep_quest_progress);
    cJSON_AddNumberToObject(root, "glassdeep_quest_encounters", g->glassdeep_quest_encounters);
    cJSON_AddNumberToObject(root, "catacombs_quest_state", g->catacombs_quest_state);
    cJSON_AddNumberToObject(root, "catacombs_quest_progress", g->catacombs_quest_progress);
    cJSON_AddNumberToObject(root, "catacombs_quest_encounters", g->catacombs_quest_encounters);
    cJSON_AddNumberToObject(root, "watchfire_quest_state", g->watchfire_quest_state);
    cJSON_AddNumberToObject(root, "watchfire_quest_progress", g->watchfire_quest_progress);
    cJSON_AddNumberToObject(root, "watchfire_quest_encounters", g->watchfire_quest_encounters);
    cJSON_AddNumberToObject(root, "hunt_quest_state", g->hunt_quest_state);
    cJSON_AddNumberToObject(root, "hunt_quest_progress", g->hunt_quest_progress);
    cJSON_AddNumberToObject(root, "hunt_quest_encounters", g->hunt_quest_encounters);
    cJSON_AddNumberToObject(root, "temple_alignment", g->temple_alignment);
    cJSON_AddNumberToObject(root, "temple_sentinels_awakened",
        g->temple_sentinels_awakened);
    cJSON_AddNumberToObject(root, "temple_treasure_state",
        g->temple_treasure_state);
    cJSON_AddNumberToObject(root, "dialogue_active", g->dialogue_active);
    cJSON_AddStringToObject(root, "dialogue_speaker", g->dialogue_speaker);
    cJSON_AddStringToObject(root, "dialogue_text", g->dialogue_text);
    cJSON_AddNumberToObject(root, "dialogue_x", g->dialogue_x);
    cJSON_AddNumberToObject(root, "dialogue_y", g->dialogue_y);

    // Messages
    cJSON *messages = cJSON_CreateArray();
    for (int i = 0; i < g->message_count; i++)
        cJSON_AddItemToArray(messages, cJSON_CreateString(g->messages[i]));
    cJSON_AddItemToObject(root, "messages", messages);
    cJSON *message_kinds = cJSON_CreateArray();
    for (int i = 0; i < g->message_count; i++) {
        cJSON_AddItemToArray(message_kinds, cJSON_CreateNumber(g->message_kinds[i]));
    }
    cJSON_AddItemToObject(root, "message_kinds", message_kinds);

    // Controls
    cJSON *key_bindings = cJSON_CreateArray();
    for (int i = 0; i < CONTROL_COUNT; i++) {
        cJSON_AddItemToArray(key_bindings, cJSON_CreateNumber(g->key_bindings[i]));
    }
    cJSON_AddItemToObject(root, "key_bindings", key_bindings);

    // Inventory
    cJSON *inventory = cJSON_CreateArray();
    for (int i = 0; i < g->inventory_count; i++) {
        const Item *item = &g->inventory[i];
        cJSON *it = cJSON_CreateObject();
        cJSON_AddNumberToObject(it, "active",        item->active);
        cJSON_AddNumberToObject(it, "type",          item->type);
        cJSON_AddStringToObject(it, "name",          item->name);
        cJSON_AddNumberToObject(it, "attack_bonus",  item->attack_bonus);
        cJSON_AddNumberToObject(it, "sharpened", item->sharpened);
        cJSON_AddNumberToObject(it, "defense_bonus", item->defense_bonus);
        cJSON_AddNumberToObject(it, "value",         item->value);
        cJSON_AddNumberToObject(it, "spell_id",      item->spell_id);
        cJSON_AddNumberToObject(it, "is_ranged",     item->is_ranged);
        cJSON_AddNumberToObject(it, "range",         item->range);
        cJSON_AddNumberToObject(it, "weapon_family", item->weapon_family);
        cJSON_AddNumberToObject(it, "weapon_hands",  item->weapon_hands);
        cJSON_AddNumberToObject(it, "rarity",        item->rarity);
        cJSON_AddNumberToObject(it, "class_mask",    item->class_mask);
        cJSON_AddNumberToObject(it, "visual_id",     item->visual_id);
        cJSON_AddNumberToObject(it, "critical_chance_bonus",
            item->critical_chance_bonus);
        cJSON_AddNumberToObject(it, "gold_drop_bonus_percent", item->gold_drop_bonus_percent);
        cJSON_AddNumberToObject(it, "cleave_percent", item->cleave_percent);
        cJSON_AddNumberToObject(it, "pierces_targets",
            item->pierces_targets);
        cJSON_AddNumberToObject(it, "spell_power_bonus",
            item->spell_power_bonus);
        cJSON_AddNumberToObject(it, "armor_penetration_percent",
            item->armor_penetration_percent);
        cJSON_AddNumberToObject(it, "armor_family", item->armor_family);
        cJSON_AddNumberToObject(it, "max_hp_bonus", item->max_hp_bonus);
        cJSON_AddNumberToObject(it, "max_mp_bonus", item->max_mp_bonus);
        cJSON_AddNumberToObject(it, "evasion_chance", item->evasion_chance);
        cJSON_AddNumberToObject(it, "spell_cost_reduction_percent",
            item->spell_cost_reduction_percent);
        cJSON_AddNumberToObject(it, "block_chance", item->block_chance);
        cJSON_AddNumberToObject(it, "block_reduction_percent",
            item->block_reduction_percent);
        cJSON_AddItemToArray(inventory, it);
    }
    cJSON_AddItemToObject(root, "inventory", inventory);
    cJSON_AddNumberToObject(root, "inventory_count", g->inventory_count);

    // Floor items
    cJSON *floor_items = cJSON_CreateArray();
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *fi = &g->floor_items[i];
        cJSON *f = cJSON_CreateObject();
        cJSON_AddNumberToObject(f, "active", fi->active);
        cJSON_AddNumberToObject(f, "x",      fi->x);
        cJSON_AddNumberToObject(f, "y",      fi->y);
        cJSON_AddNumberToObject(f, "underlying_tile", fi->underlying_tile);
        cJSON *it = cJSON_CreateObject();
        cJSON_AddNumberToObject(it, "active",        fi->item.active);
        cJSON_AddNumberToObject(it, "type",          fi->item.type);
        cJSON_AddStringToObject(it, "name",          fi->item.name);
        cJSON_AddNumberToObject(it, "attack_bonus",  fi->item.attack_bonus);
        cJSON_AddNumberToObject(it, "sharpened", fi->item.sharpened);
        cJSON_AddNumberToObject(it, "defense_bonus", fi->item.defense_bonus);
        cJSON_AddNumberToObject(it, "value",         fi->item.value);
        cJSON_AddNumberToObject(it, "spell_id",      fi->item.spell_id);
        cJSON_AddNumberToObject(it, "is_ranged",     fi->item.is_ranged);
        cJSON_AddNumberToObject(it, "range",         fi->item.range);
        cJSON_AddNumberToObject(it, "weapon_family",
            fi->item.weapon_family);
        cJSON_AddNumberToObject(it, "weapon_hands", fi->item.weapon_hands);
        cJSON_AddNumberToObject(it, "rarity", fi->item.rarity);
        cJSON_AddNumberToObject(it, "class_mask", fi->item.class_mask);
        cJSON_AddNumberToObject(it, "visual_id", fi->item.visual_id);
        cJSON_AddNumberToObject(it, "critical_chance_bonus",
            fi->item.critical_chance_bonus);
        cJSON_AddNumberToObject(it, "gold_drop_bonus_percent", fi->item.gold_drop_bonus_percent);
        cJSON_AddNumberToObject(it, "cleave_percent",
            fi->item.cleave_percent);
        cJSON_AddNumberToObject(it, "pierces_targets",
            fi->item.pierces_targets);
        cJSON_AddNumberToObject(it, "spell_power_bonus",
            fi->item.spell_power_bonus);
        cJSON_AddNumberToObject(it, "armor_penetration_percent",
            fi->item.armor_penetration_percent);
        cJSON_AddNumberToObject(it, "armor_family", fi->item.armor_family);
        cJSON_AddNumberToObject(it, "max_hp_bonus", fi->item.max_hp_bonus);
        cJSON_AddNumberToObject(it, "max_mp_bonus", fi->item.max_mp_bonus);
        cJSON_AddNumberToObject(it, "evasion_chance",
            fi->item.evasion_chance);
        cJSON_AddNumberToObject(it, "spell_cost_reduction_percent",
            fi->item.spell_cost_reduction_percent);
        cJSON_AddNumberToObject(it, "block_chance",
            fi->item.block_chance);
        cJSON_AddNumberToObject(it, "block_reduction_percent",
            fi->item.block_reduction_percent);
        cJSON_AddItemToObject(f, "item", it);
        cJSON_AddItemToArray(floor_items, f);
    }
    cJSON_AddItemToObject(root, "floor_items", floor_items);
    cJSON_AddNumberToObject(root, "floor_item_count", g->floor_item_count);

    // Current map
    cJSON_AddItemToObject(root, "map", serialize_map(&g->map));

    // Current enemies
    cJSON_AddItemToObject(root, "enemies",
        serialize_enemies(g->enemies, g->enemy_count));
    cJSON_AddNumberToObject(root, "enemy_count", g->enemy_count);

    // Level cache
    cJSON *cache = cJSON_CreateArray();
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid",         g->level_cache[i].valid);
        cJSON_AddNumberToObject(entry, "level_cleared", g->level_cache[i].level_cleared);
        if (g->level_cache[i].valid) {
            cJSON_AddItemToObject(entry, "map",
                serialize_map(&g->level_cache[i].map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(g->level_cache[i].enemies,
                                  g->level_cache[i].enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count",
                g->level_cache[i].enemy_count);
        }
        cJSON_AddItemToArray(cache, entry);
    }
    cJSON_AddItemToObject(root, "level_cache", cache);

    cJSON *forest_cache = cJSON_CreateArray();
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", g->forest_cache[i].valid);
        cJSON_AddNumberToObject(entry, "level_cleared",
            g->forest_cache[i].level_cleared);
        if (g->forest_cache[i].valid) {
            cJSON_AddItemToObject(entry, "map",
                serialize_map(&g->forest_cache[i].map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(g->forest_cache[i].enemies,
                    g->forest_cache[i].enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count",
                g->forest_cache[i].enemy_count);
        }
        cJSON_AddItemToArray(forest_cache, entry);
    }
    cJSON_AddItemToObject(root, "forest_cache", forest_cache);

    cJSON *mountain_cache = cJSON_CreateArray();
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", g->mountain_cache[i].valid);
        cJSON_AddNumberToObject(entry, "level_cleared",
            g->mountain_cache[i].level_cleared);
        if (g->mountain_cache[i].valid) {
            cJSON_AddItemToObject(entry, "map",
                serialize_map(&g->mountain_cache[i].map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(g->mountain_cache[i].enemies,
                    g->mountain_cache[i].enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count",
                g->mountain_cache[i].enemy_count);
        }
        cJSON_AddItemToArray(mountain_cache, entry);
    }
    cJSON_AddItemToObject(root, "mountain_cache", mountain_cache);

    cJSON *coast_cache = cJSON_CreateArray();
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", g->coast_cache[i].valid);
        cJSON_AddNumberToObject(entry, "level_cleared",
            g->coast_cache[i].level_cleared);
        if (g->coast_cache[i].valid) {
            cJSON_AddItemToObject(entry, "map",
                serialize_map(&g->coast_cache[i].map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(g->coast_cache[i].enemies,
                    g->coast_cache[i].enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count",
                g->coast_cache[i].enemy_count);
        }
        cJSON_AddItemToArray(coast_cache, entry);
    }
    cJSON_AddItemToObject(root, "coast_cache", coast_cache);

    cJSON *swamp_cache = cJSON_CreateArray();
    for (int i = 0; i < SWAMP_CACHE_DEPTH; i++) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", g->swamp_cache[i].valid);
        cJSON_AddNumberToObject(entry, "level_cleared",
            g->swamp_cache[i].level_cleared);
        if (g->swamp_cache[i].valid) {
            cJSON_AddItemToObject(entry, "map",
                serialize_map(&g->swamp_cache[i].map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(g->swamp_cache[i].enemies,
                    g->swamp_cache[i].enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count",
                g->swamp_cache[i].enemy_count);
        }
        cJSON_AddItemToArray(swamp_cache, entry);
    }
    cJSON_AddItemToObject(root, "swamp_cache", swamp_cache);

    cJSON *dragonspine_cache = cJSON_CreateArray();
    for (int i = 0; i < DRAGONSPINE_DEPTH; i++) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", g->dragonspine_cache[i].valid);
        cJSON_AddNumberToObject(entry, "level_cleared",
            g->dragonspine_cache[i].level_cleared);
        if (g->dragonspine_cache[i].valid) {
            cJSON_AddItemToObject(entry, "map",
                serialize_map(&g->dragonspine_cache[i].map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(g->dragonspine_cache[i].enemies,
                    g->dragonspine_cache[i].enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count",
                g->dragonspine_cache[i].enemy_count);
        }
        cJSON_AddItemToArray(dragonspine_cache, entry);
    }
    cJSON_AddItemToObject(root, "dragonspine_cache", dragonspine_cache);

    cJSON *frostfell_cache = cJSON_CreateArray();
    for (int i = 0; i < FROSTFELL_DEPTH; i++) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", g->frostfell_cache[i].valid);
        cJSON_AddNumberToObject(entry, "level_cleared",
            g->frostfell_cache[i].level_cleared);
        if (g->frostfell_cache[i].valid) {
            cJSON_AddItemToObject(entry, "map",
                serialize_map(&g->frostfell_cache[i].map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(g->frostfell_cache[i].enemies,
                    g->frostfell_cache[i].enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count",
                g->frostfell_cache[i].enemy_count);
        }
        cJSON_AddItemToArray(frostfell_cache, entry);
    }
    cJSON_AddItemToObject(root, "frostfell_cache", frostfell_cache);

    cJSON *desert_cache = cJSON_CreateArray();
    for (int i = 0; i < DESERT_DEPTH; i++) {
        const LevelCache *cache = &g->desert_cache[i];
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", cache->valid);
        cJSON_AddNumberToObject(entry, "level_cleared", cache->level_cleared);
        if (cache->valid) {
            cJSON_AddItemToObject(entry, "map", serialize_map(&cache->map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(cache->enemies, cache->enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count", cache->enemy_count);
        }
        cJSON_AddItemToArray(desert_cache, entry);
    }
    cJSON_AddItemToObject(root, "desert_cache", desert_cache);

    cJSON *ashen_cache = cJSON_CreateArray();
    for (int i = 0; i < ASHEN_DEPTH; i++) {
        const LevelCache *cache = &g->ashen_cache[i];
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", cache->valid);
        cJSON_AddNumberToObject(entry, "level_cleared", cache->level_cleared);
        if (cache->valid) {
            cJSON_AddItemToObject(entry, "map", serialize_map(&cache->map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(cache->enemies, cache->enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count", cache->enemy_count);
        }
        cJSON_AddItemToArray(ashen_cache, entry);
    }
    cJSON_AddItemToObject(root, "ashen_cache", ashen_cache);

    cJSON *glassdeep_cache = cJSON_CreateArray();
    for (int i = 0; i < GLASSDEEP_DEPTH; i++) {
        const LevelCache *cache = &g->glassdeep_cache[i];
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", cache->valid);
        cJSON_AddNumberToObject(entry, "level_cleared", cache->level_cleared);
        if (cache->valid) {
            cJSON_AddItemToObject(entry, "map", serialize_map(&cache->map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(cache->enemies, cache->enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count", cache->enemy_count);
        }
        cJSON_AddItemToArray(glassdeep_cache, entry);
    }
    cJSON_AddItemToObject(root, "glassdeep_cache", glassdeep_cache);

    cJSON *catacombs_cache = cJSON_CreateArray();
    for (int i = 0; i < CATACOMBS_DEPTH; i++) {
        const LevelCache *cache = &g->catacombs_cache[i];
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", cache->valid);
        cJSON_AddNumberToObject(entry, "level_cleared", cache->level_cleared);
        if (cache->valid) {
            cJSON_AddItemToObject(entry, "map", serialize_map(&cache->map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(cache->enemies, cache->enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count", cache->enemy_count);
        }
        cJSON_AddItemToArray(catacombs_cache, entry);
    }
    cJSON_AddItemToObject(root, "catacombs_cache", catacombs_cache);

    cJSON *castle = cJSON_CreateObject();
    cJSON_AddNumberToObject(castle, "minibosses", g->castle_minibosses);
    cJSON_AddNumberToObject(castle, "castellan_reward", g->castellan_reward_state);
    cJSON_AddNumberToObject(castle, "arcanist_reward", g->arcanist_reward_state);
    cJSON_AddNumberToObject(castle, "prompt", g->castle_prompt);
    cJSON_AddNumberToObject(castle, "won", g->game_won);
    cJSON_AddNumberToObject(castle, "crown_of_unbound", g->crown_of_unbound);
    cJSON *floors = cJSON_CreateArray();
    for (int i = 0; i < CASTLE_DEPTH; i++) {
        const LevelCache *cache = &g->castle_cache[i];
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", cache->valid);
        cJSON_AddNumberToObject(entry, "level_cleared", cache->level_cleared);
        cJSON_AddNumberToObject(entry, "fire_phase", g->castle_fire_phase[i]);
        if (cache->valid) {
            cJSON_AddItemToObject(entry, "map", serialize_map(&cache->map));
            cJSON_AddItemToObject(entry, "enemies", serialize_enemies(cache->enemies, cache->enemy_count));
        }
        cJSON_AddItemToObject(entry, "loot", serialize_castle_loot(g->castle_loot[i], g->castle_loot_count[i]));
        cJSON_AddItemToArray(floors, entry);
    }
    cJSON_AddItemToObject(castle, "floors", floors);
    cJSON_AddItemToObject(root, "castle", castle);

    const char *road_keys[2] = {"crownroad_cache", "kingroad_west_cache"};
    const CrownroadCache *road_caches[2] = {&g->crownroad_cache, &g->kingroad_west_cache};
    for (int i = 0; i < 2; i++) {
        const CrownroadCache *cache = road_caches[i];
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", cache->valid);
        cJSON_AddNumberToObject(entry, "level_cleared", cache->level_cleared);
        if (cache->valid) {
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(cache->enemies, cache->enemy_count));
            cJSON_AddNumberToObject(entry, "enemy_count", cache->enemy_count);
        }
        cJSON_AddItemToObject(root, road_keys[i], entry);
    }

    cJSON *temple_cache = cJSON_CreateArray();
    for (int i = 0; i < TEMPLE_DEPTH; i++) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", g->temple_cache[i].valid);
        cJSON_AddNumberToObject(entry, "level_cleared",
            g->temple_cache[i].level_cleared);
        if (g->temple_cache[i].valid) {
            cJSON_AddItemToObject(entry, "map",
                serialize_map(&g->temple_cache[i].map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(g->temple_cache[i].enemies,
                    g->temple_cache[i].enemy_count));
        }
        cJSON_AddItemToArray(temple_cache, entry);
    }
    cJSON_AddItemToObject(root, "temple_cache", temple_cache);

    cJSON *labyrinth_cache = cJSON_CreateArray();
    for (int i = 0; i < LABYRINTH_DEPTH; i++) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "valid", g->labyrinth_cache[i].valid);
        cJSON_AddNumberToObject(entry, "level_cleared",
            g->labyrinth_cache[i].level_cleared);
        if (g->labyrinth_cache[i].valid) {
            cJSON_AddItemToObject(entry, "map",
                serialize_map(&g->labyrinth_cache[i].map));
            cJSON_AddItemToObject(entry, "enemies",
                serialize_enemies(g->labyrinth_cache[i].enemies,
                    g->labyrinth_cache[i].enemy_count));
        }
        cJSON_AddItemToArray(labyrinth_cache, entry);
    }
    cJSON_AddItemToObject(root, "labyrinth_cache", labyrinth_cache);

    char *json = cJSON_Print(root);
    cJSON_Delete(root);

    FILE *f = fopen(slot_path(slot), "w");
    if (!f) { free(json); return 0; }
    fputs(json, f);
    fclose(f);
    free(json);
    return 1;
}

static void repair_floor_item_underlays(GameState *g) {
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->underlying_tile != TILE_ITEM) {
            continue;
        }
        int occupied = 0;
        for (int j = 0; j < g->floor_item_count; j++) {
            const FloorItem *other = &g->floor_items[j];
            if (other->x == item->x && other->y == item->y) {
                occupied |= other->active;
                if (other->underlying_tile != TILE_ITEM) {
                    item->underlying_tile = other->underlying_tile;
                }
            }
        }
        if (!occupied && item->underlying_tile != TILE_ITEM &&
            item->x >= 0 && item->x < MAP_W && item->y >= 0 && item->y < MAP_H &&
            g->map.tiles[item->y][item->x] == TILE_ITEM) {
            g->map.tiles[item->y][item->x] = item->underlying_tile;
        }
    }
}

// IDs used by the retired Pirate Coast implementation (saves 184-188).
enum { RETIRED_PIRATE_LOCATION = 44, RETIRED_MERCHANT_LOCATION = 45, RETIRED_PIRATE_APPROACH = 17 };

static int is_pirate_prototype_save(const cJSON *root, int version) {
    return version >= 184 && version <= 188 &&
        cJSON_IsArray(cJSON_GetObjectItem(root, "pirate_cache"));
}

static int load_mangrove_record(const cJSON *delta, MangroveState *state, int version) {
    *state = (MangroveState){0};
    const char *names[] = {"initialized", "boat_x", "boat_y", "boat_dx", "boat_dy", "aboard", "searched"};
    int *values[] = {&state->initialized, &state->boat_x, &state->boat_y, &state->boat_dx, &state->boat_dy, &state->aboard};
    for (int i = 0; i < 7; i++) {
        const cJSON *field = cJSON_GetObjectItem(delta, names[i]);
        if (!cJSON_IsNumber(field) || field->valuedouble != field->valueint ||
            (i == 6 && (field->valueint < 0 || field->valueint > 7))) {
            return 0;
        }
        if (i < 6) {
            *values[i] = field->valueint;
        } else {
            state->searched = field->valueint;
        }
    }
    const cJSON *explored = cJSON_GetObjectItem(delta, "explored");
    if ((state->initialized != 0 && state->initialized != 1) || (state->aboard != 0 && state->aboard != 1) ||
        !cJSON_IsString(explored) || strlen(explored->valuestring) != 4 * ((sizeof(state->explored) + 2) / 3) ||
        !deserialize_castle_loot(cJSON_GetObjectItem(delta, "loot"), state->loot, &state->loot_count)) {
        return 0;
    }
    base64_to_bytes(explored->valuestring, state->explored, sizeof(state->explored));
    state->stage = state->initialized ? 1 : 0;
    if (version >= 191) {
        const cJSON *stage = cJSON_GetObjectItem(delta, "stage");
        const cJSON *enemies = cJSON_GetObjectItem(delta, "enemies");
        if (!cJSON_IsNumber(stage) || stage->valuedouble != stage->valueint ||
            !cJSON_IsArray(enemies) || cJSON_GetArraySize(enemies) > AREA_ENEMY_LIMIT) {
            return 0;
        }
        state->stage = stage->valueint;
        const char *required[] = {"x", "y", "active", "type", "hp", "max_hp", "attack", "defense", "experience",
            "move_timer", "frozen_turns", "attack_target_x", "attack_target_y"};
        for (int i = 0; i < cJSON_GetArraySize(enemies); i++) {
            const cJSON *enemy = cJSON_GetArrayItem(enemies, i);
            if (!cJSON_IsString(cJSON_GetObjectItem(enemy, "name"))) {
                return 0;
            }
            for (int n = 0; n < (int)(sizeof(required) / sizeof(required[0])); n++) {
                const cJSON *field = cJSON_GetObjectItem(enemy, required[n]);
                if (!cJSON_IsNumber(field) || field->valuedouble != field->valueint) {
                    return 0;
                }
            }
        }
        deserialize_enemies(enemies, state->enemies, &state->enemy_count, AREA_ENEMY_LIMIT);
    }
    return 1;
}

static int load_mangroves(const cJSON *root, GameState *g, int version) {
    g->mangroves = (MangroveState){0};
    memset(g->mangrove_stages, 0, sizeof(g->mangrove_stages));
    g->mangrove_defeated = 0;
    if (version < 190) {
        return 1;
    }
    if (!load_mangrove_record(cJSON_GetObjectItem(root, "mangroves"), &g->mangroves, version)) {
        return 0;
    }
    if (version >= 191) {
        const cJSON *stages = cJSON_GetObjectItem(root, "mangrove_stages");
        const cJSON *defeated = cJSON_GetObjectItem(root, "mangrove_defeated");
        if (!cJSON_IsArray(stages) || cJSON_GetArraySize(stages) != MANGROVE_DEPTH || !cJSON_IsNumber(defeated) ||
            (defeated->valuedouble != 0 && defeated->valuedouble != 1)) {
            return 0;
        }
        g->mangrove_defeated = defeated->valueint;
        for (int i = 0; i < MANGROVE_DEPTH; i++) {
            if (!load_mangrove_record(cJSON_GetArrayItem(stages, i), &g->mangrove_stages[i], version)) {
                return 0;
            }
        }
    }
    return 1;
}

static int load_retired_coast_loot(const cJSON *root, GameState *g, int version) {
    int legacy = is_pirate_prototype_save(root, version);
    const cJSON *stages = cJSON_GetObjectItem(root, legacy ? "pirate_cache" : "retired_coast_loot");
    if (!legacy && version < 189) {
        return 1;
    }
    int size = cJSON_GetArraySize(stages);
    if (!cJSON_IsArray(stages) || (legacy ? size != 5 : size != 5 && size != RETIRED_LOOT_STAGES)) {
        return 0;
    }
    int location = cJSON_GetObjectItem(root, "location")->valueint;
    int level = cJSON_GetObjectItem(root, "level")->valueint;
    for (int stage = 0; stage < size; stage++) {
        if (legacy && location == RETIRED_PIRATE_LOCATION && level == stage + 1) {
            continue; // The active floor is authoritative; do not duplicate its cached loot.
        }
        const cJSON *entry = cJSON_GetArrayItem(stages, stage);
        const cJSON *loot = legacy ? cJSON_GetObjectItem(entry, "loot") : entry;
        FloorItem items[MAX_FLOOR_ITEMS] = {0};
        int count = 0;
        if (!deserialize_castle_loot(loot, items, &count)) {
            return 0;
        }
        for (int i = 0; i < count; i++) {
            if (items[i].active) {
                g->retired_coast_items[g->retired_coast_item_count++] = items[i].item;
            }
        }
    }
    return 1;
}

static void migrate_removed_pirate_coast(GameState *g, int returning) {
    int removed_portal = g->portal_location == RETIRED_PIRATE_LOCATION;
    if (removed_portal) {
        g->portal_active = 0;
        g->portal_location = LOCATION_DUNGEON;
        g->portal_level = 0;
        g->portal_x = 0;
        g->portal_y = 0;
        g->portal_origin_tile = TILE_FLOOR;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (town_road_index(g->location) >= 0 && g->map.tiles[y][x] == TILE_PORTAL) {
                    g->map.tiles[y][x] = TILE_TOWN_FLOOR;
                }
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].underlying_tile == TILE_PORTAL) {
                g->floor_items[i].underlying_tile = TILE_TOWN_FLOOR;
            }
        }
    }
    if (!returning && g->location != LOCATION_DAWNMERE &&
        (g->location != LOCATION_TOWN_ROAD || g->road_town != LOCATION_DAWNMERE)) {
        return;
    }

    FloorItem loot[MAX_FLOOR_ITEMS];
    int loot_count = g->floor_item_count;
    memcpy(loot, g->floor_items, sizeof(loot));
    unsigned char explored[MAP_EXPLORED_BYTES];
    memcpy(explored, g->map.explored, sizeof(explored));
    int x = g->player.x;
    int y = g->player.y;
    int dx = g->player.last_dx;
    int dy = g->player.last_dy;
    int portal_x = -1;
    int portal_y = -1;
    if (!returning && g->location == LOCATION_DAWNMERE && g->portal_active) {
        for (int row = 0; row < TOWN_H; row++) {
            for (int col = 0; col < TOWN_W; col++) {
                if (g->map.tiles[row][col] == TILE_PORTAL) {
                    portal_x = col;
                    portal_y = row;
                }
            }
        }
        for (int i = 0; i < loot_count; i++) {
            if (loot[i].active && loot[i].underlying_tile == TILE_PORTAL) {
                portal_x = loot[i].x;
                portal_y = loot[i].y;
            }
        }
    }
    if (returning) {
        // The retired approach and merchant room have no surviving exit.
        g->location = LOCATION_DAWNMERE;
        g->level = 1;
        game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
        push_message(g, "Pirate Coast has been retired. You return to Dawnmere.");
    } else if (g->location == LOCATION_TOWN_ROAD) {
        town_road_enter(g, LOCATION_DAWNMERE, TOWN_SIDE_WEST, 0);
    } else {
        int spawn_x;
        int spawn_y;
        map_generate_dawnmere(&g->map, &spawn_x, &spawn_y);
        if (portal_x >= 0 && portal_x < TOWN_W && portal_y >= 0 && portal_y < TOWN_H) {
            g->map.tiles[portal_y][portal_x] = TILE_PORTAL;
        }
    }
    if (!returning) {
        memcpy(g->map.explored, explored, sizeof(explored));
        if (map_is_walkable(&g->map, x, y)) {
            g->player.x = x;
            g->player.y = y;
        } else if (g->location == LOCATION_DAWNMERE) {
            g->player.x = 20;
            g->player.y = 12;
        }
        g->player.last_dx = dx;
        g->player.last_dy = dy;
    }
    g->floor_item_count = loot_count;
    memcpy(g->floor_items, loot, sizeof(loot));
    for (int i = 0; i < loot_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (!item->active) {
            continue;
        }
        if (returning || !map_is_walkable(&g->map, item->x, item->y)) {
            // Separate walkable squares near town's western road keep loot reachable.
            item->x = 2 + i % 10;
            item->y = 12 + i / 10;
        }
        item->underlying_tile = g->map.tiles[item->y][item->x];
    }
    for (int i = 0; i < loot_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
    g->approach_route = -1;
    g->enemy_count = 0;
    g->dialogue_active = 0;
    map_mark_explored(&g->map, g->player.x, g->player.y);
}

// Only migration code knows retired garden location/route/NPC IDs: 24, 10, 206.
static void migrate_western_geography(GameState *g, int removed_garden, int rewards) {
    Map town_map = {0};
    int town = town_road_index(g->location) >= 0;
    int x;
    int y;
    if (town) {
        switch (g->location) {
            case LOCATION_TOWN: map_generate_town(&town_map, &x, &y); break;
            case LOCATION_TOWN2: map_generate_town2(&town_map, &x, &y); break;
            case LOCATION_TOWN3: map_generate_town3(&town_map, &x, &y); break;
            case LOCATION_TOWN4: map_generate_town4(&town_map, &x, &y); break;
            case LOCATION_DAWNMERE: map_generate_dawnmere(&town_map, &x, &y); break;
            default: break;
        }
    }
    // Preserve already earned, unclaimed rewards through the existing recovery queue.
    if (rewards & 1) {
        g->retired_coast_items[g->retired_coast_item_count++] = item_make_briarweave_cloak();
    }
    if (rewards & 2) {
        g->retired_coast_items[g->retired_coast_item_count++] = item_make_strength_potion();
    }
    int removed_portal = (int)g->portal_location == 24;
    g->defeated_bosses &= ~(1 << 24);
    if (removed_portal) {
        g->portal_active = 0;
        g->portal_location = LOCATION_DUNGEON;
        g->portal_level = 0;
        g->portal_origin_tile = TILE_FLOOR;
    }
    if (strcmp(g->dialogue_speaker, "Botanist Liora") == 0) {
        g->dialogue_active = 0;
        g->dialogue_speaker[0] = '\0';
        g->dialogue_text[0] = '\0';
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if ((int)g->map.tiles[y][x] == 206 || (removed_portal && g->map.tiles[y][x] == TILE_PORTAL)) {
                g->map.tiles[y][x] = town ? town_map.tiles[y][x] : TILE_TAVERN_FLOOR;
            }
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if ((int)item->underlying_tile == 206 || (removed_portal && item->underlying_tile == TILE_PORTAL)) {
            item->underlying_tile = town ? town_map.tiles[item->y][item->x] : TILE_TAVERN_FLOOR;
        }
    }
    if (removed_garden) {
        g->map = town_map;
        map_set_rosemoor_swamp_road(&g->map, g->defeated_bosses & (1 << LOCATION_SWAMP));
        g->location = LOCATION_TOWN3;
        g->last_town_visited = LOCATION_TOWN3;
        g->approach_route = -1;
        g->level = 1;
        g->level_cleared = 0;
        g->enemy_count = 0;
        g->player.x = 1;
        g->player.y = ROSEMOOR_ASHEN_GATE_Y;
        TileType ground = g->map.tiles[ROSEMOOR_ASHEN_GATE_Y][2];
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active) {
                item->x = 2;
                item->y = ROSEMOOR_ASHEN_GATE_Y;
                item->underlying_tile = ground;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        map_mark_explored(&g->map, g->player.x, g->player.y);
        push_message(g, "You return to Rosemoor. The western road leads to Giant's Peaks.");
    }
    if (g->location == LOCATION_APPROACH && g->approach_route == 13 && g->map.stairs_up_x == 0) {
        // Mirror the old eastbound approach, preserving distance and exploration.
        unsigned char explored[MAP_EXPLORED_BYTES];
        memcpy(explored, g->map.explored, sizeof(explored));
        for (int y = 0; y < APPROACH_H; y++) {
            for (int x = 0; x < APPROACH_W; x++) {
                int source = y * MAP_W + x;
                int target = y * MAP_W + APPROACH_W - 1 - x;
                unsigned char bit = 1 << (target % 8);
                if (explored[source / 8] & (1 << (source % 8))) {
                    g->map.explored[target / 8] |= bit;
                } else {
                    g->map.explored[target / 8] &= ~bit;
                }
            }
        }
        g->player.x = APPROACH_W - 1 - g->player.x;
        g->player.last_dx = -g->player.last_dx;
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].active) {
                g->floor_items[i].x = APPROACH_W - 1 - g->floor_items[i].x;
            }
        }
        approach_refresh_entrance(g);
    }
    if (g->location == LOCATION_TOWN4) {
        map_place_town4_region_gates(&g->map);
        if (!map_is_walkable(&g->map, g->player.x, g->player.y)) {
            g->player.x = TOWN_W - 2;
        }
    }
    if (g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN4) {
        FloorItem loot[MAX_FLOOR_ITEMS];
        int count = g->floor_item_count;
        memcpy(loot, g->floor_items, sizeof(loot));
        unsigned char explored[MAP_EXPLORED_BYTES];
        memcpy(explored, g->map.explored, sizeof(explored));
        int x = g->player.x;
        int y = g->player.y;
        int dx = g->player.last_dx;
        int dy = g->player.last_dy;
        town_road_enter(g, LOCATION_TOWN4, TOWN_SIDE_NORTH, 0);
        memcpy(g->map.explored, explored, sizeof(explored));
        if (map_is_walkable(&g->map, x, y)) {
            g->player.x = x;
            g->player.y = y;
        }
        g->player.last_dx = dx;
        g->player.last_dy = dy;
        g->floor_item_count = count;
        memcpy(g->floor_items, loot, sizeof(loot));
        for (int i = 0; i < count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active) {
                if (!map_is_walkable(&g->map, item->x, item->y)) {
                    item->x = g->player.x;
                    item->y = g->player.y;
                }
                item->underlying_tile = g->map.tiles[item->y][item->x];
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
    }
}

static void migrate_testing_save(cJSON *root, int version) {
    if (version < 123) {
        const char *fields[3] = {"hunt_quest_state", "hunt_quest_progress", "hunt_quest_encounters"};
        for (int i = 0; i < 3; i++) {
            if (!cJSON_GetObjectItem(root, fields[i])) {
                cJSON_AddNumberToObject(root, fields[i], 0);
            }
        }
    }
    if (version < 122) {
        const char *fields[3] = {"watchfire_quest_state", "watchfire_quest_progress", "watchfire_quest_encounters"};
        for (int i = 0; i < 3; i++) {
            if (!cJSON_GetObjectItem(root, fields[i])) {
                cJSON_AddNumberToObject(root, fields[i], 0);
            }
        }
    }
    if (version < 101) {
        const char *fields[] = {"jail_quest_state", "prisoner_x", "prisoner_y"};
        for (int i = 0; i < 3; i++) {
            if (!cJSON_GetObjectItem(root, fields[i])) {
                cJSON_AddNumberToObject(root, fields[i], 0);
            }
        }
    }
    if (version < 97) {
        const char *fields[3] = {"catacombs_quest_state", "catacombs_quest_progress", "catacombs_quest_encounters"};
        for (int i = 0; i < 3; i++) {
            if (!cJSON_GetObjectItem(root, fields[i])) {
                cJSON_AddNumberToObject(root, fields[i], 0);
            }
        }
    }

    if (version < 94) {
        const char *fields[3] = {"glassdeep_quest_state", "glassdeep_quest_progress", "glassdeep_quest_encounters"};
        for (int i = 0; i < 3; i++) {
            if (!cJSON_GetObjectItem(root, fields[i])) {
                cJSON_AddNumberToObject(root, fields[i], 0);
            }
        }
    }
    if (version < 91) {
        const char *fields[3] = {"frostfell_quest_state", "frostfell_quest_progress", "frostfell_quest_encounters"};
        for (int i = 0; i < 3; i++) {
            if (!cJSON_GetObjectItem(root, fields[i])) {
                cJSON_AddNumberToObject(root, fields[i], 0);
            }
        }
    }
    if (version < 88) {
        const char *fields[3] = {"emberforge_quest_state", "emberforge_progress", "emberforge_encounters"};
        for (int i = 0; i < 3; i++) {
            if (!cJSON_GetObjectItem(root, fields[i])) {
                cJSON_AddNumberToObject(root, fields[i], 0);
            }
        }
    }
    if (version < 83) {
        if (!cJSON_GetObjectItem(root, "max_catacombs_level_reached")) {
            cJSON_AddNumberToObject(root, "max_catacombs_level_reached", 1);
        }
        if (!cJSON_GetObjectItem(root, "catacombs_mantle_unclaimed")) {
            cJSON_AddNumberToObject(root, "catacombs_mantle_unclaimed", 0);
        }
        if (!cJSON_GetObjectItem(root, "catacombs_cache")) {
            cJSON *cache = cJSON_CreateArray();
            for (int i = 0; i < CATACOMBS_DEPTH; i++) {
                cJSON *entry = cJSON_CreateObject();
                cJSON_AddNumberToObject(entry, "valid", 0);
                cJSON_AddNumberToObject(entry, "level_cleared", 0);
                cJSON_AddItemToArray(cache, entry);
            }
            cJSON_AddItemToObject(root, "catacombs_cache", cache);
        }
    }
    if (version < 82) {
        if (!cJSON_GetObjectItem(root, "max_glassdeep_level_reached")) {
            cJSON_AddNumberToObject(root, "max_glassdeep_level_reached", 1);
        }
        if (!cJSON_GetObjectItem(root, "glassdeep_cache")) {
            cJSON *cache = cJSON_CreateArray();
            for (int i = 0; i < GLASSDEEP_DEPTH; i++) {
                cJSON *entry = cJSON_CreateObject();
                cJSON_AddNumberToObject(entry, "valid", 0);
                cJSON_AddNumberToObject(entry, "level_cleared", 0);
                cJSON_AddItemToArray(cache, entry);
            }
            cJSON_AddItemToObject(root, "glassdeep_cache", cache);
        }
    }
    if (version < 81) {
        if (!cJSON_GetObjectItem(root, "max_ashen_level_reached")) {
            cJSON_AddNumberToObject(root, "max_ashen_level_reached", 1);
        }
        if (!cJSON_GetObjectItem(root, "ashen_cache")) {
            cJSON *cache = cJSON_CreateArray();
            for (int i = 0; i < ASHEN_DEPTH; i++) {
                cJSON *entry = cJSON_CreateObject();
                cJSON_AddNumberToObject(entry, "valid", 0);
                cJSON_AddNumberToObject(entry, "level_cleared", 0);
                cJSON_AddItemToArray(cache, entry);
            }
            cJSON_AddItemToObject(root, "ashen_cache", cache);
        }
    }

    if (version < 79) {
        if (!cJSON_GetObjectItem(root, "mountain_entry_town")) {
            cJSON_AddNumberToObject(root, "mountain_entry_town", LOCATION_TOWN);
        }
        if (!cJSON_GetObjectItem(root, "mountain_portal_town")) {
            cJSON_AddNumberToObject(root, "mountain_portal_town", LOCATION_TOWN);
        }
    }
    if (version < 77) {
        if (!cJSON_GetObjectItem(root, "forest_entry_town")) {
            cJSON_AddNumberToObject(root, "forest_entry_town", LOCATION_TOWN);
        }
        if (!cJSON_GetObjectItem(root, "forest_portal_town")) {
            cJSON_AddNumberToObject(root, "forest_portal_town", LOCATION_TOWN);
        }
    }
    if (version < 78) {
        if (!cJSON_GetObjectItem(root, "swamp_entry_town")) {
            cJSON_AddNumberToObject(root, "swamp_entry_town", LOCATION_TOWN2);
        }
        if (!cJSON_GetObjectItem(root, "swamp_portal_town")) {
            cJSON_AddNumberToObject(root, "swamp_portal_town", LOCATION_TOWN2);
        }
    }
    if (version < 76 && !cJSON_GetObjectItem(root, "sunscar_lamp_quest_state")) {
        cJSON_AddNumberToObject(root, "sunscar_lamp_quest_state", 0);
    }
    if (version < 73) {
        cJSON *player = cJSON_GetObjectItem(root, "player");
        if (!cJSON_GetObjectItem(player, "freeze_recovery")) {
            cJSON_AddNumberToObject(player, "freeze_recovery", 0);
        }
        if (!cJSON_GetObjectItem(root, "kraken_bow_unclaimed")) {
            cJSON_AddNumberToObject(root, "kraken_bow_unclaimed", 0);
        }
    }
    if (version < 74) {
        if (!cJSON_GetObjectItem(root, "max_desert_level_reached")) {
            cJSON_AddNumberToObject(root, "max_desert_level_reached", 1);
        }
        if (!cJSON_GetObjectItem(root, "desert_cache")) {
            cJSON *cache = cJSON_CreateArray();
            for (int i = 0; i < DESERT_DEPTH; i++) {
                cJSON *entry = cJSON_CreateObject();
                cJSON_AddNumberToObject(entry, "valid", 0);
                cJSON_AddNumberToObject(entry, "level_cleared", 0);
                cJSON_AddItemToArray(cache, entry);
            }
            cJSON_AddItemToObject(root, "desert_cache", cache);
        }
    }
    if (version < 75 && !cJSON_GetObjectItem(root, "sandstorm_staff_unclaimed")) {
        cJSON_AddNumberToObject(root, "sandstorm_staff_unclaimed", 0);
    }
}

static void migrate_dungeon_return_stairs(Map *map, int defeated) {
    int x = map->stairs_down_x;
    int y = map->stairs_down_y;
    if (x <= 0 || x >= MAP_W - 1 || y <= 0 || y >= MAP_H - 1) {
        return;
    }
    if (map->tiles[y][x] == TILE_STAIRS_DOWN || map->tiles[y][x] == TILE_RETURN_EXIT || map->tiles[y][x] == TILE_ITEM) {
        map->tiles[y][x] = defeated ? TILE_DUNGEON_STAIRS_RETURN : TILE_DUNGEON_STAIRS_SEALED;
    }
}

static void migrate_lich_minions(GameState *g) {
    game_add_lich_minions(g);
    LevelCache *cache = &g->level_cache[DUNGEON_DEPTH - 1];
    if (!cache->valid || (g->defeated_bosses & (1 << LOCATION_DUNGEON))) {
        return;
    }
    Map active_map = g->map;
    Enemy active_enemies[MAX_ENEMIES];
    memcpy(active_enemies, g->enemies, sizeof(active_enemies));
    int active_count = g->enemy_count;
    Location location = g->location;
    int level = g->level;
    int player_x = g->player.x;
    int player_y = g->player.y;
    g->location = LOCATION_DUNGEON;
    g->level = DUNGEON_DEPTH;
    g->map = cache->map;
    memcpy(g->enemies, cache->enemies, sizeof(cache->enemies));
    g->enemy_count = cache->enemy_count;
    g->player.x = -1;
    g->player.y = -1;
    game_add_lich_minions(g);
    memcpy(cache->enemies, g->enemies, sizeof(cache->enemies));
    cache->enemy_count = g->enemy_count;
    g->map = active_map;
    memcpy(g->enemies, active_enemies, sizeof(active_enemies));
    g->enemy_count = active_count;
    g->location = location;
    g->level = level;
    g->player.x = player_x;
    g->player.y = player_y;
}

static void remove_legacy_coast_beacon(Map *m) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (m->tiles[y][x] == TILE_COAST_BEACON_UNLIT ||
                m->tiles[y][x] == TILE_COAST_BEACON_LIT) {
                m->tiles[y][x] = TILE_COAST_FLOOR;
            }
        }
    }
}

static int migrate_shortened_stages(GameState *g, Location region) {
    static const int levels[8] = {1, 2, 3, 3, 4, 4, 4, 5};
    static const int coast_retained[5] = {0, 1, 2, 5, 7};
    static const int dungeon_retained[5] = {0, 1, 3, 5, 7};
    int coast = region == LOCATION_COAST;
    const int *retained = coast ? coast_retained : dungeon_retained;
    LevelCache *cache = coast ? g->coast_cache : g->level_cache;
    int *max_level = coast ? &g->max_coast_level_reached : &g->max_level_reached;
    LevelCache *old = malloc(MAX_REGION_DEPTH * sizeof(*cache));
    if (!old) {
        return 0;
    }
    memcpy(old, cache, MAX_REGION_DEPTH * sizeof(*cache));
    int active = g->location == region ? g->level : 0;
    int portal = g->portal_active && g->portal_location == region ? g->portal_level : 0;
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        cache[i].valid = 0;
        cache[i].level_cleared = 0;
    }
    // Prefer the retained stages, but keep other explored snapshots when the
    // corresponding retained stage has not been visited yet.
    for (int i = 0; i < 8; i++) {
        if (old[i].valid) {
            cache[levels[i] - 1] = old[i];
        }
    }
    for (int i = 0; i < 5; i++) {
        if (old[retained[i]].valid) {
            cache[i] = old[retained[i]];
        }
    }
    if (portal >= 1 && portal <= 8 && old[portal - 1].valid) {
        cache[levels[portal - 1] - 1] = old[portal - 1];
    }
    free(old);
    if (active >= 1 && active <= 8) {
        g->level = levels[active - 1];
        LevelCache *snapshot = &cache[g->level - 1];
        snapshot->map = g->map;
        snapshot->enemy_count = g->enemy_count;
        memcpy(snapshot->enemies, g->enemies, sizeof(snapshot->enemies));
        snapshot->level_cleared = g->level_cleared;
        snapshot->valid = 1;
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x >= 0 && item->x < MAP_W &&
                item->y >= 0 && item->y < MAP_H &&
                snapshot->map.tiles[item->y][item->x] == TILE_ITEM) {
                snapshot->map.tiles[item->y][item->x] = item->underlying_tile;
            }
        }
    }
    int reached = *max_level;
    if (reached < 1) {
        reached = 1;
    } else if (reached > 8) {
        reached = 8;
    }
    *max_level = levels[reached - 1];
    if (g->portal_location == region && g->portal_level >= 1 && g->portal_level <= 8) {
        g->portal_level = levels[g->portal_level - 1];
        if (active && portal && active != portal && g->level == g->portal_level) {
            // Two old floors now share one stage; land on the retained map's
            // entrance rather than the discarded floor's coordinates.
            g->portal_x = g->map.stairs_up_x;
            g->portal_y = g->map.stairs_up_y;
            g->portal_origin_tile = g->map.tiles[g->portal_y][g->portal_x];
        }
    }
    if (coast && cache[0].valid) {
        remove_legacy_coast_beacon(&cache[0].map);
    }
    if (coast && active == 1) {
        remove_legacy_coast_beacon(&g->map);
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->underlying_tile == TILE_COAST_BEACON_UNLIT ||
                item->underlying_tile == TILE_COAST_BEACON_LIT) {
                item->underlying_tile = TILE_COAST_FLOOR;
            }
        }
    }
    if (coast && g->portal_location == region && g->portal_level == 1 &&
        (g->portal_origin_tile == TILE_COAST_BEACON_UNLIT ||
        g->portal_origin_tile == TILE_COAST_BEACON_LIT)) {
        g->portal_origin_tile = TILE_COAST_FLOOR;
    }
    return 1;
}

static void migrate_expanded_finale(GameState *g, Location region, int old_depth) {
    int labyrinth = region == LOCATION_LABYRINTH;
    LevelCache *cache = labyrinth ? g->labyrinth_cache : g->temple_cache;
    if (labyrinth) {
        int old_switches = g->rook_labyrinth_switches;
        g->rook_labyrinth_switches = (old_switches & 3) | ((old_switches & 4) ? 16 : 0);
        if ((old_switches & 4) || cache[old_depth - 1].valid ||
            (g->location == region && g->level == old_depth)) {
            // Players already at the old vault keep access without having to
            // backtrack through the two newly inserted rune floors.
            g->rook_labyrinth_switches |= 12;
        }
    } else if (g->max_temple_level_reached == old_depth) {
        g->max_temple_level_reached = TEMPLE_DEPTH;
    }
    cache[4] = cache[old_depth - 1];
    cache[old_depth - 1].valid = 0;
    cache[old_depth - 1].level_cleared = 0;
    if (g->location == region && g->level == old_depth) {
        g->level = 5;
    }
    if (g->portal_location == region && g->portal_level == old_depth) {
        g->portal_level = 5;
    }
}

static void remove_misplaced_shaman(Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        Enemy *enemy = &enemies[i];
        if (enemy->dain_fragment == DAIN_FRAGMENT_SHAMAN) {
            enemy->dain_fragment = 0;
            snprintf(enemy->name, sizeof(enemy->name), "Goblin Shaman");
        }
    }
}

static void remove_legacy_wardens(Map *m) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (m->tiles[y][x] == TILE_FOREST_WARDEN) {
                m->tiles[y][x] = TILE_FOREST_FLOOR;
            }
        }
    }
}

static void remove_misplaced_map_bearers(Enemy *enemies, int count, int level) {
    int expected = level == 7 ? DAIN_FRAGMENT_ARCHER :
        (level == 6 ? DAIN_FRAGMENT_BOMBER : (level == 5 ? DAIN_FRAGMENT_SHAMAN : 0));
    for (int i = 0; i < count; i++) {
        Enemy *enemy = &enemies[i];
        if (enemy->dain_fragment && enemy->dain_fragment != expected) {
            enemy->dain_fragment = 0;
            const char *name = enemy->type == ENEMY_GOBLIN_ARCHER ? "Goblin Archer" :
                (enemy->type == ENEMY_GOBLIN_BOMBER ? "Goblin Bomber" : "Goblin Shaman");
            snprintf(enemy->name, sizeof(enemy->name), "%s", name);
        }
    }
}

static void migrate_near_side_quests(GameState *g) {
    for (int i = 0; i < 7; i++) {
        if (i < 4 && g->forest_cache[i].valid) {
            remove_legacy_wardens(&g->forest_cache[i].map);
        }
    }
    if (g->location == LOCATION_FOREST && g->level < 5) {
        remove_legacy_wardens(&g->map);
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].underlying_tile == TILE_FOREST_WARDEN) {
                g->floor_items[i].underlying_tile = TILE_FOREST_FLOOR;
            }
        }
    }
    if (g->portal_location == LOCATION_FOREST && g->portal_level < 5 &&
        g->portal_origin_tile == TILE_FOREST_WARDEN) {
        g->portal_origin_tile = TILE_FOREST_FLOOR;
    }
    for (int i = 0; i < 7; i++) {
        if (g->mountain_cache[i].valid) {
            remove_misplaced_map_bearers(g->mountain_cache[i].enemies, g->mountain_cache[i].enemy_count, i + 1);
        }
    }
    if (g->location == LOCATION_MOUNTAINS) {
        remove_misplaced_map_bearers(g->enemies, g->enemy_count, g->level);
    }
}

// Keep saved terrain, enemies, loot and portal coordinates while moving the
// former end bosses into the center. Forest and mountain stages 4 and 7 merge
// into the outer approach, retaining occupied or portal-linked snapshots.
static void retire_mountain_markers(Map *map) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = map->tiles[y][x];
            if (tile == TILE_MOUNTAIN_SHORTCUT || tile == TILE_WATCHFIRE_COLD || tile == TILE_WATCHFIRE_LIT) {
                map->tiles[y][x] = TILE_MOUNTAIN_FLOOR;
            }
        }
    }
}

static void relocate_mountain_bearers(Enemy *enemies, int count, int level) {
    int expected = level == 2 ? DAIN_FRAGMENT_ARCHER : level == 3 ? DAIN_FRAGMENT_BOMBER : level == 4 ? DAIN_FRAGMENT_SHAMAN : 0;
    for (int i = 0; i < count; i++) {
        Enemy *enemy = &enemies[i];
        if (enemy->dain_fragment && enemy->dain_fragment != expected) {
            enemy->dain_fragment = 0;
            snprintf(enemy->name, sizeof(enemy->name), "%s", enemy->type == ENEMY_GOBLIN_ARCHER ? "Goblin Archer" : enemy->type == ENEMY_GOBLIN_BOMBER ? "Goblin Bomber" : "Goblin Shaman");
        }
    }
}

static void restore_watchfire_guards(GameState *g, const Enemy *guards, int count) {
    int restored = 0;
    for (int i = 0; i < g->enemy_count && restored < count; i++) {
        Enemy *enemy = &g->enemies[i];
        if (strcmp(enemy->name, "Watchfire Defender") == 0) {
            int x = enemy->x;
            int y = enemy->y;
            *enemy = guards[restored++];
            enemy->x = x;
            enemy->y = y;
        }
    }
}

static int migrate_mountain_watchfire(GameState *g, const Enemy *guards, int count) {
    if (!count || !g->watchfire_quest_state || (g->watchfire_quest_progress & 1)) {
        return 1;
    }
    GameState *stage = malloc(sizeof(*stage));
    if (!stage) {
        return 0;
    }
    *stage = *g;
    stage->location = LOCATION_MOUNTAINS;
    stage->level = WATCHFIRE_MOUNTAINS_LEVEL;
    stage->floor_item_count = 0;
    LevelCache *cache = &g->mountain_cache[WATCHFIRE_MOUNTAINS_LEVEL - 1];
    if (cache->valid) {
        stage->map = cache->map;
        stage->enemy_count = cache->enemy_count;
        stage->level_cleared = cache->level_cleared;
        memcpy(stage->enemies, cache->enemies, sizeof(stage->enemies));
    } else {
        stage->watchfire_quest_state = 0;
        map_generate_mountains(&stage->map, stage->level);
        enemies_spawn(stage);
        stage->watchfire_quest_state = g->watchfire_quest_state;
    }
    stage->player.x = stage->map.stairs_up_x;
    stage->player.y = stage->map.stairs_up_y;
    if (stage->portal_active && stage->portal_location == LOCATION_MOUNTAINS && stage->portal_level == stage->level) {
        stage->player.x = stage->portal_x;
        stage->player.y = stage->portal_y;
    }
    stage->watchfire_quest_encounters &= ~1;
    game_refresh_quest_encounters(stage);
    restore_watchfire_guards(stage, guards, count);
    cache->valid = 1;
    cache->map = stage->map;
    cache->enemy_count = stage->enemy_count;
    cache->level_cleared = stage->level_cleared;
    memcpy(cache->enemies, stage->enemies, sizeof(cache->enemies));
    g->watchfire_quest_encounters |= stage->watchfire_quest_encounters & 1;
    free(stage);
    if (g->location == LOCATION_MOUNTAINS && g->level == WATCHFIRE_MOUNTAINS_LEVEL) {
        // The active snapshot can have newer boss damage than its cache.
        g->watchfire_quest_encounters &= ~1;
        game_refresh_quest_encounters(g);
        restore_watchfire_guards(g, guards, count);
    }
    return 1;
}

static int migrate_mountain_climb(GameState *g) {
    // Keep the peak and near-side quest maps. Occupied and portal-linked maps
    // take priority over other snapshots when the two outer slopes merge.
    int levels[7] = {1, 2, 3, 5, 4, 3, 2};
    int active = g->location == LOCATION_MOUNTAINS ? g->level - 1 : -1;
    int portal = g->portal_active && g->portal_location == LOCATION_MOUNTAINS ? g->portal_level - 1 : -1;
    if (active >= 0 && active < 7 && portal >= 0 && portal < 7 && active != portal && levels[active] == levels[portal]) {
        levels[portal] = levels[portal] == 2 ? 3 : 2;
    }
    LevelCache *old = malloc(MAX_REGION_DEPTH * sizeof(*old));
    if (!old) {
        return 0;
    }
    memcpy(old, g->mountain_cache, MAX_REGION_DEPTH * sizeof(*old));
    Enemy defenders[4];
    int defender_count = 0;
    Enemy *source = active == 5 ? g->enemies : old[5].enemies;
    int source_count = active == 5 ? g->enemy_count : old[5].valid ? old[5].enemy_count : 0;
    for (int i = 0; i < source_count && defender_count < 4; i++) {
        if (strcmp(source[i].name, "Watchfire Defender") == 0) {
            defenders[defender_count++] = source[i];
        }
    }
    // Transfer the watchfire's defenders, including damage and deaths, rather
    // than leaving a second encounter on the former northern slope.
    for (int pass = 0; pass < 2; pass++) {
        Enemy *enemies = pass ? g->enemies : old[5].enemies;
        int *count = pass ? &g->enemy_count : &old[5].enemy_count;
        if ((pass && active != 5) || (!pass && !old[5].valid)) {
            continue;
        }
        int kept = 0;
        for (int i = 0; i < *count; i++) {
            if (strcmp(enemies[i].name, "Watchfire Defender") != 0) {
                enemies[kept++] = enemies[i];
            }
        }
        *count = kept;
    }
    memset(g->mountain_cache, 0, sizeof(g->mountain_cache));
    for (int i = 0; i < 7; i++) {
        if (old[i].valid) {
            g->mountain_cache[levels[i] - 1] = old[i];
        }
    }
    // An active floor may differ from its last cached snapshot.
    if (portal >= 0 && portal < 7 && old[portal].valid) {
        g->mountain_cache[levels[portal] - 1] = old[portal];
    }
    if (active >= 0 && active < 7) {
        g->level = levels[active];
        if (old[active].valid) {
            g->mountain_cache[g->level - 1] = old[active];
        }
        retire_mountain_markers(&g->map);
        relocate_mountain_bearers(g->enemies, g->enemy_count, g->level);
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->underlying_tile == TILE_MOUNTAIN_SHORTCUT || item->underlying_tile == TILE_WATCHFIRE_COLD || item->underlying_tile == TILE_WATCHFIRE_LIT) {
                item->underlying_tile = TILE_MOUNTAIN_FLOOR;
            }
        }
        if (strcmp(g->dialogue_speaker, "Shortcut found") == 0 || strcmp(g->dialogue_speaker, "Take shortcut?") == 0) {
            g->dialogue_active = 0;
        }
    }
    if (g->portal_location == LOCATION_MOUNTAINS && g->portal_level >= 1 && g->portal_level <= 7) {
        g->portal_level = levels[g->portal_level - 1];
        if (g->portal_origin_tile == TILE_MOUNTAIN_SHORTCUT || g->portal_origin_tile == TILE_WATCHFIRE_COLD || g->portal_origin_tile == TILE_WATCHFIRE_LIT) {
            g->portal_origin_tile = TILE_MOUNTAIN_FLOOR;
        }
    }
    for (int i = 0; i < MOUNTAIN_DEPTH; i++) {
        if (g->mountain_cache[i].valid) {
            retire_mountain_markers(&g->mountain_cache[i].map);
            relocate_mountain_bearers(g->mountain_cache[i].enemies, g->mountain_cache[i].enemy_count, i + 1);
        }
    }
    free(old);
    int reached = 1;
    for (int i = 0; i < 7 && i < g->max_mountain_level_reached; i++) {
        if (levels[i] > reached) {
            reached = levels[i];
        }
    }
    g->max_mountain_level_reached = reached;
    // Completed fires stay completed; pending defenders move to the summit.
    g->watchfire_quest_encounters &= ~1;
    if (!migrate_mountain_watchfire(g, defenders, defender_count)) {
        return 0;
    }
    if (g->mountain_entry_town == LOCATION_TOWN) {
        g->mountain_entry_town = LOCATION_DAWNMERE;
    }
    if (g->mountain_portal_town == LOCATION_TOWN) {
        g->mountain_portal_town = LOCATION_DAWNMERE;
    }
    if (g->location == LOCATION_TOWN) {
        for (int x = 18; x <= 22; x++) {
            g->map.tiles[0][x] = TILE_WALL;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->y == 0 && item->x >= 18 && item->x <= 22) {
                item->y = 1;
                item->underlying_tile = item->x == 20 ? TILE_TOWN_PATH : TILE_TOWN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        if (g->player.y == 0 && g->player.x >= 18 && g->player.x <= 22) {
            g->player.y = 1;
        }
        // A portal already present in an old Oakhaven save remains usable
        // once, so moving its future anchor cannot strand an expedition.
    } else if (g->location == LOCATION_DAWNMERE) {
        for (int x = 18; x <= 22; x++) {
            g->map.tiles[0][x] = TILE_TOWN_EXIT;
        }
        if (g->portal_active && g->portal_location == LOCATION_MOUNTAINS && g->mountain_portal_town == LOCATION_DAWNMERE) {
            g->map.tiles[2][21] = TILE_PORTAL;
        }
    }
    approach_refresh_mountains(g);
    return 1;
}

static int forest_warden_pending(const GameState *g, int level) {
    int bit = level >= 2 && level <= 4 ? 1 << (level - 2) : 0;
    return bit && g->alder_quest_state == 1 && !(g->alder_wardens_rescued & bit);
}

static void retire_forest_markers(Map *map, int keep_warden) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = map->tiles[y][x];
            if (tile == TILE_FOREST_SHORTCUT || (!keep_warden && tile == TILE_FOREST_WARDEN)) {
                map->tiles[y][x] = TILE_FOREST_FLOOR;
            }
        }
    }
}

static int migrate_forest_adventure(GameState *g) {
    int levels[7] = {1, 2, 3, 5, 4, 3, 2};
    int active = g->location == LOCATION_FOREST ? g->level - 1 : -1;
    int portal = g->portal_active && g->portal_location == LOCATION_FOREST ? g->portal_level - 1 : -1;
    if (active >= 0 && active < 7 && portal >= 0 && portal < 7 && active != portal && levels[active] == levels[portal]) {
        levels[portal] = levels[portal] == 2 ? 3 : 2;
    }
    LevelCache *old = malloc(MAX_REGION_DEPTH * sizeof(*old));
    if (!old) {
        return 0;
    }
    memcpy(old, g->forest_cache, MAX_REGION_DEPTH * sizeof(*old));
    memset(g->forest_cache, 0, sizeof(g->forest_cache));
    for (int i = 0; i < 7; i++) {
        if (old[i].valid) {
            g->forest_cache[levels[i] - 1] = old[i];
        }
    }
    // Linked and occupied snapshots win when the old outer trails merge.
    if (portal >= 0 && portal < 7 && old[portal].valid) {
        g->forest_cache[levels[portal] - 1] = old[portal];
    }
    if (active >= 0 && active < 7) {
        g->level = levels[active];
        if (old[active].valid) {
            g->forest_cache[g->level - 1] = old[active];
        }
        int keep = forest_warden_pending(g, g->level);
        retire_forest_markers(&g->map, keep);
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->underlying_tile == TILE_FOREST_SHORTCUT || (!keep && item->underlying_tile == TILE_FOREST_WARDEN)) {
                item->underlying_tile = TILE_FOREST_FLOOR;
            }
        }
        if (strcmp(g->dialogue_speaker, "Shortcut found") == 0 || strcmp(g->dialogue_speaker, "Take shortcut?") == 0) {
            g->dialogue_active = 0;
        }
    }
    if (g->portal_location == LOCATION_FOREST && g->portal_level >= 1 && g->portal_level <= 7) {
        g->portal_level = levels[g->portal_level - 1];
        if (g->portal_origin_tile == TILE_FOREST_SHORTCUT ||
            (!forest_warden_pending(g, g->portal_level) && g->portal_origin_tile == TILE_FOREST_WARDEN)) {
            g->portal_origin_tile = TILE_FOREST_FLOOR;
        }
    }
    for (int i = 0; i < FOREST_DEPTH; i++) {
        if (g->forest_cache[i].valid) {
            retire_forest_markers(&g->forest_cache[i].map, forest_warden_pending(g, i + 1));
        }
    }
    free(old);
    int reached = 1;
    for (int i = 0; i < 7 && i < g->max_forest_level_reached; i++) {
        if (levels[i] > reached) {
            reached = levels[i];
        }
    }
    g->max_forest_level_reached = reached;
    approach_refresh_forest(g);
    return 1;
}

static int swamp_rescue_pending(const GameState *g, int level) {
    return level == SWAMP_RESCUE_LEVEL && g->innkeeper_quest_state == 1;
}

static void retire_swamp_markers(Map *map, int keep_mira) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = map->tiles[y][x];
            if (tile == TILE_SWAMP_SHORTCUT || (!keep_mira && tile == TILE_SWAMP_DAUGHTER)) {
                map->tiles[y][x] = TILE_SWAMP_FLOOR;
            }
        }
    }
}

static int migrate_swamp_adventure(GameState *g) {
    int levels[7] = {1, 2, 3, 5, 4, 2, 1};
    int active = g->location == LOCATION_SWAMP ? g->level - 1 : -1;
    int portal = g->portal_active && g->portal_location == LOCATION_SWAMP ? g->portal_level - 1 : -1;
    if (active >= 0 && active < 7 && portal >= 0 && portal < 7 && active != portal && levels[active] == levels[portal]) {
        levels[portal] = levels[portal] == 1 ? 2 : 1;
    }
    LevelCache *old = malloc(SWAMP_CACHE_DEPTH * sizeof(*old));
    if (!old) {
        return 0;
    }
    memcpy(old, g->swamp_cache, SWAMP_CACHE_DEPTH * sizeof(*old));
    memset(g->swamp_cache, 0, sizeof(g->swamp_cache));
    for (int i = 0; i < 7; i++) {
        if (old[i].valid) {
            g->swamp_cache[levels[i] - 1] = old[i];
        }
    }
    // Linked and occupied snapshots win when the old outer trails merge.
    if (portal >= 0 && portal < 7 && old[portal].valid) {
        g->swamp_cache[levels[portal] - 1] = old[portal];
    }
    if (active >= 0 && active < 7) {
        g->level = levels[active];
        if (old[active].valid) {
            g->swamp_cache[g->level - 1] = old[active];
        }
        int keep = swamp_rescue_pending(g, g->level);
        retire_swamp_markers(&g->map, keep);
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->underlying_tile == TILE_SWAMP_SHORTCUT || (!keep && item->underlying_tile == TILE_SWAMP_DAUGHTER)) {
                item->underlying_tile = TILE_SWAMP_FLOOR;
            }
        }
        if (strcmp(g->dialogue_speaker, "Shortcut found") == 0 || strcmp(g->dialogue_speaker, "Take shortcut?") == 0) {
            g->dialogue_active = 0;
        }
    }
    if (g->portal_location == LOCATION_SWAMP && g->portal_level >= 1 && g->portal_level <= 7) {
        g->portal_level = levels[g->portal_level - 1];
        if (g->portal_origin_tile == TILE_SWAMP_SHORTCUT ||
            (!swamp_rescue_pending(g, g->portal_level) && g->portal_origin_tile == TILE_SWAMP_DAUGHTER)) {
            g->portal_origin_tile = TILE_SWAMP_FLOOR;
        }
    }
    for (int i = 0; i < SWAMP_DEPTH; i++) {
        if (g->swamp_cache[i].valid) {
            retire_swamp_markers(&g->swamp_cache[i].map, swamp_rescue_pending(g, i + 1));
        }
    }
    free(old);
    int reached = 1;
    for (int i = 0; i < 7 && i < g->max_swamp_level_reached; i++) {
        if (levels[i] > reached) {
            reached = levels[i];
        }
    }
    g->max_swamp_level_reached = reached;
    approach_refresh_swamp(g);
    return 1;
}

static int migrate_region_routes(GameState *g, Location region) {
    static const int forest_levels[8] = {1, 2, 3, 7, 5, 6, 7, 4};
    static const int swamp_levels[5] = {1, 2, 5, 3, 4};
    int forest = region == LOCATION_FOREST;
    int mountain = region == LOCATION_MOUNTAINS;
    int old_depth = forest || mountain ? 8 : 5;
    int depth = 7; // Historical route migration runs before five-level migrations.
    const int *levels = forest || mountain ? forest_levels : swamp_levels;
    LevelCache *cache = forest ? g->forest_cache : (mountain ? g->mountain_cache : g->swamp_cache);
    int *max_level = forest ? &g->max_forest_level_reached :
        (mountain ? &g->max_mountain_level_reached : &g->max_swamp_level_reached);
    LevelCache *old = malloc(old_depth * sizeof(*old));
    if (!old) {
        return 0;
    }
    memcpy(old, cache, old_depth * sizeof(*old));
    for (int i = 0; i < depth; i++) {
        cache[i].valid = 0;
        cache[i].level_cleared = 0;
    }
    int keep_seventh = (g->location == region && g->level == 7) ||
        (g->portal_active && g->portal_location == region && g->portal_level == 7);
    int keep_fourth = (g->location == region && g->level == 4) ||
        (g->portal_active && g->portal_location == region && g->portal_level == 4);
    int reached = 1;
    for (int i = 0; i < old_depth; i++) {
        int level = levels[i];
        if (i < *max_level && level > reached) {
            reached = level;
        }
        if ((forest && i == 6 && !keep_seventh) ||
            (mountain && ((i == 3 && !keep_fourth && old[6].valid) || (i == 6 && keep_fourth && !keep_seventh)))) {
            continue;
        }
        if (old[i].valid) {
            cache[level - 1] = old[i];
        }
    }
    free(old);
    *max_level = reached;
    if (g->location == region && g->level >= 1 && g->level <= old_depth) {
        g->level = levels[g->level - 1];
    }
    if (g->portal_location == region && g->portal_level >= 1 && g->portal_level <= old_depth) {
        g->portal_level = levels[g->portal_level - 1];
    }
    if (forest) {
        if (g->location == region) {
            remove_legacy_wardens(&g->map);
        }
        for (int i = 0; i < depth; i++) {
            if (cache[i].valid) {
                remove_legacy_wardens(&cache[i].map);
            }
        }
        cache[MAX_REGION_DEPTH - 1].valid = 0;
    }
    if (mountain) {
        // Old bearers move with their maps, but fragments now belong to stages
        // 1-3. Keep the enemies and their health; refresh places needed bearers.
        for (int stage = 0; stage <= depth; stage++) {
            Enemy *enemies = stage == depth ? g->enemies : cache[stage].enemies;
            int count = stage == depth ? g->enemy_count : cache[stage].enemy_count;
            if ((stage == depth && g->location != region) || (stage < depth && !cache[stage].valid)) {
                continue;
            }
            for (int i = 0; i < count; i++) {
                Enemy *enemy = &enemies[i];
                if (enemy->dain_fragment) {
                    enemy->dain_fragment = 0;
                    const char *name = enemy->type == ENEMY_GOBLIN_ARCHER ? "Goblin Archer" :
                        (enemy->type == ENEMY_GOBLIN_BOMBER ? "Goblin Bomber" : "Goblin Shaman");
                    snprintf(enemy->name, sizeof(enemy->name), "%s", name);
                }
            }
        }
        cache[MAX_REGION_DEPTH - 1].valid = 0;
    }
    return 1;
}

// Preserve old warnings: version 115 added recovery, and 117 locks the sweep pattern.
static void migrate_kraken_cycle(Enemy *enemies, int count, int version) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (e->type != ENEMY_FROST_KRAKEN) {
            continue;
        }
        e->attack_phase = 0;
        if (version < 115) {
            e->move_timer = e->move_timer % 2 == 1 ? 1 : 0;
            if (e->move_timer == 0) {
                e->attack_target_x = -1;
                e->attack_target_y = -1;
            }
        }
    }
}

// Version 114 relocates quest givers without regenerating maps or quest progress.
static void migrate_quest_giver_locations(GameState *g) {
    static const struct {
        TileType tile;
        const char *speaker;
        Location from;
        Location to;
        int x;
        int y;
    } moves[] = {
        {TILE_NPC_DAIN, "Dain", LOCATION_GUILD, LOCATION_TOWN4, DAIN_TOWN_X, DAIN_TOWN_Y},
        {TILE_NPC_GUILD_SEEKER, "Zara", LOCATION_GUILD, LOCATION_INN, ZARA_INN_X, ZARA_INN_Y}
    };
    for (size_t i = 0; i < sizeof(moves) / sizeof(moves[0]); i++) {
        if (strcmp(g->dialogue_speaker, moves[i].speaker) == 0) {
            g->dialogue_active = 0;
        }
        if (g->location == moves[i].from) {
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    if (g->map.tiles[y][x] == moves[i].tile) {
                        g->map.tiles[y][x] = TILE_TAVERN_FLOOR;
                    }
                }
            }
            for (int j = 0; j < g->floor_item_count; j++) {
                if (g->floor_items[j].underlying_tile == moves[i].tile) {
                    g->floor_items[j].underlying_tile = TILE_TAVERN_FLOOR;
                }
            }
        }
        if (g->location == moves[i].to) {
            int x = moves[i].x;
            int y = moves[i].y;
            if (g->player.x == x && g->player.y == y) {
                g->player.y++;
            }
            for (int j = 0; j < g->floor_item_count; j++) {
                FloorItem *item = &g->floor_items[j];
                if (item->active && item->x == x && item->y == y) {
                    item->y++;
                    item->underlying_tile = g->location == LOCATION_INN ? TILE_TAVERN_FLOOR : TILE_TOWN_PATH;
                    g->map.tiles[item->y][item->x] = TILE_ITEM;
                }
            }
            g->map.tiles[y][x] = moves[i].tile;
        }
    }
}

static void migrate_rook_location(GameState *g) {
    if (strcmp(g->dialogue_speaker, "Rook") == 0) {
        g->dialogue_active = 0;
    }
    if (g->location == LOCATION_INN || g->location == LOCATION_GUILD) {
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (g->map.tiles[y][x] == TILE_NPC_ROOK) {
                    g->map.tiles[y][x] = TILE_TAVERN_FLOOR;
                }
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].underlying_tile == TILE_NPC_ROOK) {
                g->floor_items[i].underlying_tile = TILE_TAVERN_FLOOR;
            }
        }
    }
    if (g->location == LOCATION_TAVERN) {
        if (g->player.x == ROOK_TAVERN_X && g->player.y == ROOK_TAVERN_Y) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == ROOK_TAVERN_X && item->y == ROOK_TAVERN_Y) {
                item->y++;
                item->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->map.tiles[ROOK_TAVERN_Y][ROOK_TAVERN_X] = TILE_NPC_ROOK;
    }
}

static void migrate_goblin_shield(Item *item) {
    if (item->type == ITEM_SHIELD && strcmp(item->name, "Goblin King's Shield") == 0) {
        Item shield = item_make_goblin_king_shield();
        item->defense_bonus = shield.defense_bonus;
        item->block_chance = shield.block_chance;
        item->value = shield.value;
    }
}

static void migrate_goblin_shields(GameState *g) {
    for (int i = 0; i < g->inventory_count; i++) {
        Item *item = &g->inventory[i];
        int old_defense = item->defense_bonus;
        migrate_goblin_shield(item);
        if (g->equipped_off_hand == i) {
            g->player.defense += item->defense_bonus - old_defense;
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        migrate_goblin_shield(&g->floor_items[i].item);
    }
    for (int level = 0; level < CASTLE_DEPTH; level++) {
        for (int i = 0; i < g->castle_loot_count[level]; i++) {
            migrate_goblin_shield(&g->castle_loot[level][i].item);
        }
    }
}

static void migrate_dagger(Item *item) {
    if (item->type != ITEM_WEAPON) {
        return;
    }
    Item definition;
    if (strcmp(item->name, "Rusty Dagger") == 0) {
        definition = item_make_rusty_dagger();
    } else if (strcmp(item->name, "Dagger") == 0) {
        definition = item_make_dagger();
    } else if (strcmp(item->name, "Magic Dagger") == 0) {
        definition = item_make_magic_dagger();
    } else {
        return;
    }
    item->attack_bonus = definition.attack_bonus + item->sharpened;
    item->gold_drop_bonus_percent = definition.gold_drop_bonus_percent;
}

static void migrate_daggers(GameState *g) {
    for (int i = 0; i < g->inventory_count; i++) {
        Item *item = &g->inventory[i];
        int old_attack = item->attack_bonus;
        migrate_dagger(item);
        if (g->equipped_main_hand == i) {
            g->player.attack += item->attack_bonus - old_attack;
        }
        if (g->equipped_off_hand == i && item->type == ITEM_WEAPON) {
            g->player.attack += game_off_hand_attack_bonus(item) - (old_attack + 1) / 2;
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        migrate_dagger(&g->floor_items[i].item);
    }
    for (int level = 0; level < CASTLE_DEPTH; level++) {
        for (int i = 0; i < g->castle_loot_count[level]; i++) {
            migrate_dagger(&g->castle_loot[level][i].item);
        }
    }
}

static void migrate_sunwarden_mantle(Item *item, Player *player) {
    if (item->type != ITEM_ARMOR || strcmp(item->name, "Sunwarden Mantle") != 0) {
        return;
    }
    Item mantle = item_make_sunwarden_mantle();
    if (player) {
        player->max_hp += mantle.max_hp_bonus - item->max_hp_bonus;
        player->max_mp += mantle.max_mp_bonus - item->max_mp_bonus;
        if (player->hp > player->max_hp) {
            player->hp = player->max_hp;
        }
        if (player->mp > player->max_mp) {
            player->mp = player->max_mp;
        }
    }
    item->max_hp_bonus = mantle.max_hp_bonus;
    item->max_mp_bonus = mantle.max_mp_bonus;
}

static void migrate_rebalanced_weapon(Item *item, int retire_goblin) {
    if (item->type != ITEM_WEAPON) {
        return;
    }
    if (strcmp(item->name, "Cryptblade") == 0) {
        Item blade = item_make_cryptblade();
        item->attack_bonus = blade.attack_bonus + item->sharpened;
        item->critical_chance_bonus = blade.critical_chance_bonus;
    } else if (retire_goblin && strcmp(item->name, "Goblin King's Greatsword") == 0) {
        int active = item->active;
        *item = item_make_goblin_king_shield();
        item->active = active;
    }
}

static void migrate_rebalanced_weapons(GameState *g, int retire_goblin) {
    for (int i = 0; i < g->inventory_count; i++) {
        Item *item = &g->inventory[i];
        int old_attack = item->attack_bonus;
        if (retire_goblin && item->type == ITEM_WEAPON && strcmp(item->name, "Goblin King's Greatsword") == 0) {
            if (g->equipped_main_hand == i) {
                game_unequip_main_hand(g);
            }
            if (g->equipped_off_hand == i) {
                game_unequip_off_hand(g);
            }
        }
        migrate_rebalanced_weapon(item, retire_goblin);
        if (item->type == ITEM_WEAPON) {
            if (g->equipped_main_hand == i) {
                g->player.attack += item->attack_bonus - old_attack;
            }
            if (g->equipped_off_hand == i) {
                g->player.attack += game_off_hand_attack_bonus(item) - (old_attack + 1) / 2;
            }
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        migrate_rebalanced_weapon(&g->floor_items[i].item, retire_goblin);
    }
    for (int level = 0; level < CASTLE_DEPTH; level++) {
        for (int i = 0; i < g->castle_loot_count[level]; i++) {
            migrate_rebalanced_weapon(&g->castle_loot[level][i].item, retire_goblin);
        }
    }
}

int load_game(GameState *g, int slot) {
    FILE *f = fopen(slot_path(slot), "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root) return 0;

    cJSON *version_item = cJSON_GetObjectItem(root, "save_version");
    int save_version = version_item ? version_item->valueint : 1;
    if (!load_giants(root, g, save_version) || !load_northroad(root, g, save_version) || !load_ironfang(root, g, save_version) || !load_stormwatch(root, g, save_version) || !load_mangroves(root, g, save_version)) {
        cJSON_Delete(root);
        return 0;
    }
    g->retired_coast_item_count = 0;
    int pirate_prototype = is_pirate_prototype_save(root, save_version);
    int pirate_return = 0;
    int old_forest = save_version < 77 && !cJSON_GetObjectItem(root, "forest_entry_town");
    int old_swamp = save_version < 78 && !cJSON_GetObjectItem(root, "swamp_entry_town");
    int old_mountains = save_version < 79 && !cJSON_GetObjectItem(root, "mountain_entry_town");
    migrate_testing_save(root, save_version);

    cJSON *max_desert = cJSON_GetObjectItem(root, "max_desert_level_reached");
    cJSON *desert_cache = cJSON_GetObjectItem(root, "desert_cache");
    if (!cJSON_IsNumber(max_desert) || !cJSON_IsArray(desert_cache) ||
        cJSON_GetArraySize(desert_cache) != DESERT_DEPTH) {
        cJSON_Delete(root);
        return 0;
    }

    cJSON *max_ashen = cJSON_GetObjectItem(root, "max_ashen_level_reached");
    cJSON *ashen_cache = cJSON_GetObjectItem(root, "ashen_cache");
    if (!cJSON_IsNumber(max_ashen) || !cJSON_IsArray(ashen_cache) ||
        cJSON_GetArraySize(ashen_cache) != ASHEN_DEPTH) {
        cJSON_Delete(root);
        return 0;
    }

    cJSON *max_glassdeep = cJSON_GetObjectItem(root, "max_glassdeep_level_reached");
    cJSON *glassdeep_cache = cJSON_GetObjectItem(root, "glassdeep_cache");
    if (!cJSON_IsNumber(max_glassdeep) || !cJSON_IsArray(glassdeep_cache) ||
        cJSON_GetArraySize(glassdeep_cache) != GLASSDEEP_DEPTH) {
        cJSON_Delete(root);
        return 0;
    }

    cJSON *max_catacombs = cJSON_GetObjectItem(root, "max_catacombs_level_reached");
    cJSON *catacombs_cache = cJSON_GetObjectItem(root, "catacombs_cache");
    if (!cJSON_IsNumber(max_catacombs) || !cJSON_IsArray(catacombs_cache) ||
        cJSON_GetArraySize(catacombs_cache) != CATACOMBS_DEPTH) {
        cJSON_Delete(root);
        return 0;
    }

    cJSON *mantle = cJSON_GetObjectItem(root, "catacombs_mantle_unclaimed");
    if (!cJSON_IsNumber(mantle)) {
        cJSON_Delete(root);
        return 0;
    }
    if (!elderwood_load(root, g, save_version) || !load_heartseed(root, g, save_version) || !load_spice_skiff(root, g, save_version) || !load_stormbell(root, g, save_version)) {
        cJSON_Delete(root);
        return 0;
    }
    if (!load_jail_equipment(root, g, save_version)) {
        cJSON_Delete(root);
        return 0;
    }
    // Player
    cJSON *player = cJSON_GetObjectItem(root, "player");
    cJSON *freeze_recovery = cJSON_GetObjectItem(player, "freeze_recovery");
    cJSON *kraken_bow_unclaimed = cJSON_GetObjectItem(root, "kraken_bow_unclaimed");
    cJSON *sandstorm_staff_unclaimed = cJSON_GetObjectItem(root, "sandstorm_staff_unclaimed");
    cJSON *lamp_quest = cJSON_GetObjectItem(root, "sunscar_lamp_quest_state");
    cJSON *forest_entry = cJSON_GetObjectItem(root, "forest_entry_town");
    cJSON *forest_portal = cJSON_GetObjectItem(root, "forest_portal_town");
    cJSON *swamp_entry = cJSON_GetObjectItem(root, "swamp_entry_town");
    cJSON *swamp_portal = cJSON_GetObjectItem(root, "swamp_portal_town");
    cJSON *mountain_entry = cJSON_GetObjectItem(root, "mountain_entry_town");
    cJSON *mountain_portal = cJSON_GetObjectItem(root, "mountain_portal_town");
    if (!cJSON_IsNumber(freeze_recovery) || !cJSON_IsNumber(kraken_bow_unclaimed) ||
        !cJSON_IsNumber(sandstorm_staff_unclaimed) || !cJSON_IsNumber(lamp_quest) ||
        !cJSON_IsNumber(forest_entry) || !cJSON_IsNumber(forest_portal) ||
        !cJSON_IsNumber(swamp_entry) || !cJSON_IsNumber(swamp_portal) ||
        !cJSON_IsNumber(mountain_entry) || !cJSON_IsNumber(mountain_portal)) {
        cJSON_Delete(root);
        return 0;
    }
    g->player.freeze_recovery = freeze_recovery->valueint;
    g->kraken_bow_unclaimed = kraken_bow_unclaimed->valueint;
    g->sandstorm_staff_unclaimed = sandstorm_staff_unclaimed->valueint;
    g->sunscar_lamp_quest_state = lamp_quest->valueint;
    cJSON *emberforge_quest = cJSON_GetObjectItem(root, "emberforge_quest_state");
    cJSON *emberforge_progress = cJSON_GetObjectItem(root, "emberforge_progress");
    cJSON *emberforge_encounters = cJSON_GetObjectItem(root, "emberforge_encounters");
    if (!cJSON_IsNumber(emberforge_quest) || !cJSON_IsNumber(emberforge_progress) || !cJSON_IsNumber(emberforge_encounters) ||
        emberforge_quest->valueint < 0 || emberforge_quest->valueint > 3 ||
        emberforge_progress->valueint < 0 || emberforge_progress->valueint > 3 ||
        emberforge_encounters->valueint < 0 || emberforge_encounters->valueint > 3) {
        cJSON_Delete(root);
        return 0;
    }
    g->emberforge_quest_state = emberforge_quest->valueint;
    g->emberforge_progress = emberforge_progress->valueint;
    g->emberforge_encounters = emberforge_encounters->valueint;
    cJSON *frostfell_quest = cJSON_GetObjectItem(root, "frostfell_quest_state");
    cJSON *frostfell_progress = cJSON_GetObjectItem(root, "frostfell_quest_progress");
    cJSON *frostfell_encounters = cJSON_GetObjectItem(root, "frostfell_quest_encounters");
    if (!cJSON_IsNumber(frostfell_quest) || !cJSON_IsNumber(frostfell_progress) || !cJSON_IsNumber(frostfell_encounters) ||
        frostfell_quest->valueint < 0 || frostfell_quest->valueint > 3 ||
        frostfell_progress->valueint < 0 || frostfell_progress->valueint > 3 ||
        frostfell_encounters->valueint < 0 || frostfell_encounters->valueint > 3) {
        cJSON_Delete(root);
        return 0;
    }
    g->frostfell_quest_state = frostfell_quest->valueint;
    g->frostfell_quest_progress = frostfell_progress->valueint;
    g->frostfell_quest_encounters = frostfell_encounters->valueint;
    cJSON *glassdeep_quest = cJSON_GetObjectItem(root, "glassdeep_quest_state");
    cJSON *glassdeep_progress = cJSON_GetObjectItem(root, "glassdeep_quest_progress");
    cJSON *glassdeep_encounters = cJSON_GetObjectItem(root, "glassdeep_quest_encounters");
    if (!cJSON_IsNumber(glassdeep_quest) || !cJSON_IsNumber(glassdeep_progress) || !cJSON_IsNumber(glassdeep_encounters) ||
        glassdeep_quest->valueint < 0 || glassdeep_quest->valueint > 3 ||
        glassdeep_progress->valueint < 0 || glassdeep_progress->valueint > 7 ||
        glassdeep_encounters->valueint < 0 || glassdeep_encounters->valueint > 7 ||
        (glassdeep_quest->valueint == 0 && (glassdeep_progress->valueint || glassdeep_encounters->valueint)) ||
        (glassdeep_quest->valueint == 1 && glassdeep_progress->valueint == 7) ||
        (glassdeep_quest->valueint >= 2 && glassdeep_progress->valueint != 7)) {
        cJSON_Delete(root);
        return 0;
    }
    g->glassdeep_quest_state = glassdeep_quest->valueint;
    g->glassdeep_quest_progress = glassdeep_progress->valueint;
    g->glassdeep_quest_encounters = glassdeep_encounters->valueint;
    cJSON *catacombs_quest = cJSON_GetObjectItem(root, "catacombs_quest_state");
    cJSON *catacombs_progress = cJSON_GetObjectItem(root, "catacombs_quest_progress");
    cJSON *catacombs_encounters = cJSON_GetObjectItem(root, "catacombs_quest_encounters");
    if (!cJSON_IsNumber(catacombs_quest) || !cJSON_IsNumber(catacombs_progress) || !cJSON_IsNumber(catacombs_encounters) ||
        catacombs_quest->valueint < 0 || catacombs_quest->valueint > 3 ||
        catacombs_progress->valueint < 0 || catacombs_progress->valueint > 15 ||
        catacombs_encounters->valueint < 0 || catacombs_encounters->valueint > 7 ||
        (catacombs_quest->valueint == 0 && (catacombs_progress->valueint || catacombs_encounters->valueint)) ||
        (catacombs_quest->valueint == 1 && catacombs_progress->valueint == 15) ||
        (catacombs_quest->valueint >= 2 && catacombs_progress->valueint != 15)) {
        cJSON_Delete(root);
        return 0;
    }
    g->catacombs_quest_state = catacombs_quest->valueint;
    g->catacombs_quest_progress = catacombs_progress->valueint;
    g->catacombs_quest_encounters = catacombs_encounters->valueint;
    cJSON *watchfire_state = cJSON_GetObjectItem(root, "watchfire_quest_state");
    cJSON *watchfire_progress = cJSON_GetObjectItem(root, "watchfire_quest_progress");
    cJSON *watchfire_encounters = cJSON_GetObjectItem(root, "watchfire_quest_encounters");
    if (!cJSON_IsNumber(watchfire_state) || !cJSON_IsNumber(watchfire_progress) || !cJSON_IsNumber(watchfire_encounters) ||
        watchfire_state->valuedouble != watchfire_state->valueint || watchfire_state->valueint < 0 || watchfire_state->valueint > 3 ||
        watchfire_progress->valuedouble != watchfire_progress->valueint || watchfire_progress->valueint < 0 || watchfire_progress->valueint > 7 ||
        watchfire_encounters->valuedouble != watchfire_encounters->valueint || watchfire_encounters->valueint < 0 || watchfire_encounters->valueint > 7 ||
        (watchfire_state->valueint == 0 && (watchfire_progress->valueint || watchfire_encounters->valueint)) ||
        (watchfire_state->valueint == 1 && watchfire_progress->valueint == 7) ||
        (watchfire_state->valueint >= 2 && watchfire_progress->valueint != 7)) {
        cJSON_Delete(root);
        return 0;
    }
    g->watchfire_quest_state = watchfire_state->valueint;
    g->watchfire_quest_progress = watchfire_progress->valueint;
    g->watchfire_quest_encounters = watchfire_encounters->valueint;
    cJSON *hunt_state = cJSON_GetObjectItem(root, "hunt_quest_state");
    cJSON *hunt_progress = cJSON_GetObjectItem(root, "hunt_quest_progress");
    cJSON *hunt_encounters = cJSON_GetObjectItem(root, "hunt_quest_encounters");
    if (!cJSON_IsNumber(hunt_state) || !cJSON_IsNumber(hunt_progress) || !cJSON_IsNumber(hunt_encounters) ||
        hunt_state->valuedouble != hunt_state->valueint || hunt_state->valueint < 0 || hunt_state->valueint > 3 ||
        hunt_progress->valuedouble != hunt_progress->valueint || hunt_progress->valueint < 0 || hunt_progress->valueint > 7 ||
        hunt_encounters->valuedouble != hunt_encounters->valueint || hunt_encounters->valueint < 0 || hunt_encounters->valueint > 1023 ||
        (hunt_state->valueint == 0 && (hunt_progress->valueint || hunt_encounters->valueint)) ||
        (hunt_state->valueint == 1 && hunt_progress->valueint == 7) ||
        (hunt_state->valueint >= 2 && hunt_progress->valueint != 7) ||
        ((hunt_progress->valueint & 1) && (hunt_encounters->valueint & 15) != 15) ||
        ((hunt_progress->valueint & 2) && (hunt_encounters->valueint & 112) != 112) ||
        ((hunt_progress->valueint & 4) && (hunt_encounters->valueint & 896) != 896)) {
        cJSON_Delete(root);
        return 0;
    }
    g->hunt_quest_state = hunt_state->valueint;
    g->hunt_quest_progress = hunt_progress->valueint;
    g->hunt_quest_encounters = hunt_encounters->valueint;
    cJSON *jail_state = cJSON_GetObjectItem(root, "jail_quest_state");
    cJSON *prisoner_x = cJSON_GetObjectItem(root, "prisoner_x");
    cJSON *prisoner_y = cJSON_GetObjectItem(root, "prisoner_y");
    int tunnel_width = save_version < 118 ? ESCAPE_TUNNEL_LEGACY_W : ESCAPE_TUNNEL_W;
    if (!cJSON_IsNumber(jail_state) || !cJSON_IsNumber(prisoner_x) || !cJSON_IsNumber(prisoner_y) ||
        jail_state->valueint < 0 || jail_state->valueint > 3 ||
        prisoner_x->valueint < 0 || prisoner_x->valueint >= tunnel_width ||
        prisoner_y->valueint < 0 || prisoner_y->valueint >= ESCAPE_TUNNEL_H) {
        cJSON_Delete(root);
        return 0;
    }
    g->jail_quest_state = jail_state->valueint;
    g->prisoner_x = prisoner_x->valueint;
    g->prisoner_y = prisoner_y->valueint;
    cJSON *dungeon_entry = cJSON_GetObjectItem(root, "dungeon_entry_town");
    cJSON *dungeon_portal = cJSON_GetObjectItem(root, "dungeon_portal_town");
    g->dungeon_entry_town = dungeon_entry ? dungeon_entry->valueint : LOCATION_TOWN;
    g->dungeon_portal_town = dungeon_portal ? dungeon_portal->valueint : LOCATION_TOWN;
    if ((dungeon_entry && !cJSON_IsNumber(dungeon_entry)) || (dungeon_portal && !cJSON_IsNumber(dungeon_portal)) ||
        (g->dungeon_entry_town != LOCATION_TOWN && g->dungeon_entry_town != LOCATION_DAWNMERE) ||
        (g->dungeon_portal_town != LOCATION_TOWN && g->dungeon_portal_town != LOCATION_DAWNMERE)) {
        cJSON_Delete(root);
        return 0;
    }
    g->forest_entry_town = forest_entry->valueint;
    g->forest_portal_town = forest_portal->valueint;
    g->swamp_entry_town = swamp_entry->valueint;
    g->swamp_portal_town = swamp_portal->valueint;
    g->mountain_entry_town = mountain_entry->valueint;
    g->mountain_portal_town = mountain_portal->valueint;
    if ((g->forest_entry_town != LOCATION_TOWN && g->forest_entry_town != LOCATION_TOWN2) ||
        (g->forest_portal_town != LOCATION_TOWN && g->forest_portal_town != LOCATION_TOWN2) ||
        (g->swamp_entry_town != LOCATION_TOWN2 && g->swamp_entry_town != LOCATION_TOWN3) ||
        (g->swamp_portal_town != LOCATION_TOWN2 && g->swamp_portal_town != LOCATION_TOWN3) ||
        (g->mountain_entry_town != LOCATION_DAWNMERE && g->mountain_entry_town != LOCATION_TOWN4 && (save_version >= 171 || g->mountain_entry_town != LOCATION_TOWN)) ||
        (g->mountain_portal_town != LOCATION_DAWNMERE && g->mountain_portal_town != LOCATION_TOWN4 && (save_version >= 171 || g->mountain_portal_town != LOCATION_TOWN))) {
        cJSON_Delete(root);
        return 0;
    }
    strncpy(g->player.name, cJSON_GetObjectItem(player, "name")->valuestring, 20);
    g->player.x                = cJSON_GetObjectItem(player, "x")->valueint;
    g->player.y                = cJSON_GetObjectItem(player, "y")->valueint;
    g->player.hp               = cJSON_GetObjectItem(player, "hp")->valueint;
    g->player.max_hp           = cJSON_GetObjectItem(player, "max_hp")->valueint;
    g->player.mp               = cJSON_GetObjectItem(player, "mp")->valueint;
    g->player.max_mp           = cJSON_GetObjectItem(player, "max_mp")->valueint;
    g->player.attack           = cJSON_GetObjectItem(player, "attack")->valueint;
    g->player.defense          = cJSON_GetObjectItem(player, "defense")->valueint;
    g->player.level            = cJSON_GetObjectItem(player, "level")->valueint;
    g->player.experience       = cJSON_GetObjectItem(player, "experience")->valueint;
    g->player.experience_next  = cJSON_GetObjectItem(player, "experience_next")->valueint;
    g->player.last_dx          = cJSON_GetObjectItem(player, "last_dx")->valueint;
    g->player.last_dy          = cJSON_GetObjectItem(player, "last_dy")->valueint;
    g->player.equipped_spell   = cJSON_GetObjectItem(player, "equipped_spell")->valueint;
    // Saves from before Frost Wraiths load unfrozen.
    cJSON *frozen_turns = cJSON_GetObjectItem(player, "frozen_turns");
    g->player.frozen_turns = frozen_turns ? frozen_turns->valueint : 0;
    g->player.known_spell_count = cJSON_GetObjectItem(player, "known_spell_count")->valueint;
    g->player.player_class      = cJSON_GetObjectItem(player, "player_class")->valueint;
    cJSON *arrows = cJSON_GetObjectItem(player, "arrows");
    if (save_version < 103 && !arrows) {
        g->player.arrows = g->player.player_class == CLASS_ROGUE ? MAX_ARROWS : 0;
    } else if (!cJSON_IsNumber(arrows) || arrows->valuedouble != arrows->valueint || arrows->valueint < 0 || arrows->valueint > MAX_ARROWS) {
        cJSON_Delete(root);
        return 0;
    } else {
        g->player.arrows = arrows->valueint;
    }

    // Known spells
    cJSON *spells = cJSON_GetObjectItem(player, "spells");
    for (int i = 0; i < g->player.known_spell_count; i++) {
        cJSON *s = cJSON_GetArrayItem(spells, i);
        Spell *sp = &g->player.known_spells[i];
        strncpy(sp->name, cJSON_GetObjectItem(s, "name")->valuestring,
            sizeof(sp->name) - 1);
        sp->id      = cJSON_GetObjectItem(s, "id")->valueint;
        sp->type    = cJSON_GetObjectItem(s, "type")->valueint;
        sp->mp_cost = cJSON_GetObjectItem(s, "mp_cost")->valueint;
        sp->damage  = cJSON_GetObjectItem(s, "damage")->valueint;
        sp->range   = cJSON_GetObjectItem(s, "range")->valueint;
        sp->radius  = cJSON_GetObjectItem(s, "radius")->valueint;
        sp->rank    = cJSON_GetObjectItem(s, "rank")->valueint;
        if (sp->id == SPELL_HEAL) {
            sp->mp_cost = HEAL_BASE_MP_COST - 2 * (sp->rank - 1);
        }
    }

    // Game state
    g->level             = cJSON_GetObjectItem(root, "level")->valueint;
    g->level_cleared     = cJSON_GetObjectItem(root, "level_cleared")->valueint;
    g->max_level_reached = cJSON_GetObjectItem(root, "max_level_reached")->valueint;
    cJSON *max_forest = cJSON_GetObjectItem(root, "max_forest_level_reached");
    g->max_forest_level_reached = max_forest ? max_forest->valueint : 1;
    cJSON *max_mountain = cJSON_GetObjectItem(root, "max_mountain_level_reached");
    g->max_mountain_level_reached = max_mountain ? max_mountain->valueint : 1;
    cJSON *max_coast = cJSON_GetObjectItem(root, "max_coast_level_reached");
    g->max_coast_level_reached = max_coast ? max_coast->valueint : 1;
    cJSON *max_swamp = cJSON_GetObjectItem(root, "max_swamp_level_reached");
    g->max_swamp_level_reached = max_swamp ? max_swamp->valueint : 1;
    cJSON *max_dragonspine = cJSON_GetObjectItem(root,
        "max_dragonspine_level_reached");
    g->max_dragonspine_level_reached = max_dragonspine ? max_dragonspine->valueint : 1;
    cJSON *max_frostfell = cJSON_GetObjectItem(root,
        "max_frostfell_level_reached");
    g->max_frostfell_level_reached = max_frostfell ? max_frostfell->valueint : 1;
    g->max_desert_level_reached = max_desert->valueint;
    g->max_ashen_level_reached = max_ashen->valueint;
    g->max_glassdeep_level_reached = max_glassdeep->valueint;
    g->max_catacombs_level_reached = max_catacombs->valueint;
    g->catacombs_mantle_unclaimed = mantle->valueint;
    cJSON *max_temple = cJSON_GetObjectItem(root,
        "max_temple_level_reached");
    g->max_temple_level_reached = max_temple ? max_temple->valueint : 1;
    g->message_count     = cJSON_GetObjectItem(root, "message_count")->valueint;
    g->gold              = cJSON_GetObjectItem(root, "gold")->valueint;
    cJSON *labyrinth_town = cJSON_GetObjectItem(root, "labyrinth_entry_town");
    g->labyrinth_entry_town = labyrinth_town ? labyrinth_town->valueint : LOCATION_TOWN2;
    if ((save_version >= 176 && !cJSON_IsNumber(labyrinth_town)) ||
        (labyrinth_town && (!cJSON_IsNumber(labyrinth_town) || labyrinth_town->valuedouble != labyrinth_town->valueint)) ||
        (g->labyrinth_entry_town != LOCATION_TOWN && g->labyrinth_entry_town != LOCATION_TOWN2)) {
        cJSON_Delete(root);
        return 0;
    }
    g->rook_quest_state = cJSON_GetObjectItem(root,
        "rook_quest_state")->valueint;
    cJSON *innkeeper_quest = cJSON_GetObjectItem(root, "innkeeper_quest_state");
    g->innkeeper_quest_state = innkeeper_quest ? innkeeper_quest->valueint : 0;
    g->rook_labyrinth_switches = cJSON_GetObjectItem(root,
        "rook_labyrinth_switches")->valueint;
    g->rook_quest_completions = cJSON_GetObjectItem(root,
        "rook_quest_completions")->valueint;
    g->score             = cJSON_GetObjectItem(root, "score")->valueint;
    cJSON *main_hand = cJSON_GetObjectItem(root, "equipped_main_hand");
    cJSON *off_hand = cJSON_GetObjectItem(root, "equipped_off_hand");
    // Saves before version 31 stored the main-hand index as equipped_weapon.
    cJSON *legacy_weapon = cJSON_GetObjectItem(root, "equipped_weapon");
    g->equipped_main_hand = main_hand ? main_hand->valueint :
        (legacy_weapon ? legacy_weapon->valueint : -1);
    g->equipped_off_hand = off_hand ? off_hand->valueint : -1;
    g->equipped_armor    = cJSON_GetObjectItem(root, "equipped_armor")->valueint;
    g->location = cJSON_GetObjectItem(root, "location")->valueint;
    int removed_garden = save_version < 197 && ((int)g->location == 24 ||
        (g->location == LOCATION_APPROACH && cJSON_IsNumber(cJSON_GetObjectItem(root, "approach_route")) &&
        cJSON_GetObjectItem(root, "approach_route")->valueint == 10));
    if (removed_garden) {
        g->location = LOCATION_TOWN3;
    }
    if (pirate_prototype) {
        cJSON *route = cJSON_GetObjectItem(root, "approach_route");
        pirate_return = g->location == RETIRED_PIRATE_LOCATION || g->location == RETIRED_MERCHANT_LOCATION ||
            (g->location == LOCATION_APPROACH && cJSON_IsNumber(route) && route->valueint == RETIRED_PIRATE_APPROACH);
        if (pirate_return) {
            g->location = LOCATION_DAWNMERE;
        }
    }
    cJSON *debug_passages = cJSON_GetObjectItem(root, "debug_passages_unlocked");
    g->debug_passages_unlocked = debug_passages ? debug_passages->valueint : 0;
    if (debug_passages && (!cJSON_IsNumber(debug_passages) ||
        debug_passages->valuedouble != g->debug_passages_unlocked ||
        (g->debug_passages_unlocked != 0 && g->debug_passages_unlocked != 1))) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON *road_town = cJSON_GetObjectItem(root, "road_town");
    g->road_town = road_town ? road_town->valueint : LOCATION_TOWN;
    if ((save_version >= 178 && !cJSON_IsNumber(road_town)) ||
        (road_town && (!cJSON_IsNumber(road_town) || road_town->valuedouble != road_town->valueint)) ||
        town_road_index(g->road_town) < 0 ||
        (g->location == LOCATION_TOWN_ROAD && (g->player.x < 0 || g->player.x >= TOWN_ROAD_W || g->player.y < 0 || g->player.y >= TOWN_ROAD_H))) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON *approach = cJSON_GetObjectItem(root, "approach_route");
    g->approach_route = cJSON_IsNumber(approach) ? approach->valueint : -1;
    if (g->location == LOCATION_APPROACH && (!approach_route(g) ||
        g->player.x < 0 || g->player.x >= APPROACH_W || g->player.y < 0 || g->player.y >= APPROACH_H)) {
        cJSON_Delete(root);
        return 0;
    }
    if ((g->location == LOCATION_JAIL && !g->debug_passages_unlocked && (g->jail_quest_state != 1 && g->jail_quest_state != 2)) ||
        (g->location == LOCATION_ESCAPE_TUNNEL && ((!g->debug_passages_unlocked && g->jail_quest_state != 2) ||
        g->player.x < 1 || g->player.x >= tunnel_width || g->player.y < 1 || g->player.y >= ESCAPE_TUNNEL_H - 1 ||
        g->prisoner_x < 1 || g->prisoner_x >= tunnel_width - 1 || g->prisoner_y < 1 || g->prisoner_y >= ESCAPE_TUNNEL_H - 1))) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON *key_found = cJSON_GetObjectItem(root, "dungeon_key_found");
    cJSON *crypt_keys = cJSON_GetObjectItem(root, "dungeon_crypt_keys");
    cJSON *portal_active = cJSON_GetObjectItem(root, "portal_active");
    cJSON *portal_level = cJSON_GetObjectItem(root, "portal_level");
    cJSON *portal_location = cJSON_GetObjectItem(root, "portal_location");
    cJSON *portal_x = cJSON_GetObjectItem(root, "portal_x");
    cJSON *portal_y = cJSON_GetObjectItem(root, "portal_y");
    cJSON *portal_origin_tile = cJSON_GetObjectItem(root, "portal_origin_tile");
    cJSON *defeated_bosses = cJSON_GetObjectItem(root, "defeated_bosses");
    cJSON *elowen_quest = cJSON_GetObjectItem(root, "elowen_quest_state");
    cJSON *elowen_seals = cJSON_GetObjectItem(root,
        "elowen_seals_restored");
    cJSON *legacy_altars = cJSON_GetObjectItem(root,
        "elowen_altars_cleansed");
    cJSON *dain_quest = cJSON_GetObjectItem(root, "dain_quest_state");
    cJSON *dain_fragments = cJSON_GetObjectItem(root,
        "dain_map_fragments");
    cJSON *legacy_dain_targets = cJSON_GetObjectItem(root,
        "dain_targets_defeated");
    cJSON *alder_quest = cJSON_GetObjectItem(root, "alder_quest_state");
    cJSON *alder_wardens = cJSON_GetObjectItem(root,
        "alder_wardens_rescued");
    cJSON *mara_quest = cJSON_GetObjectItem(root, "mara_quest_state");
    cJSON *mara_beacons = cJSON_GetObjectItem(root, "mara_beacons_lit");
    cJSON *cain_scroll = cJSON_GetObjectItem(root, "cain_scroll_given");
    cJSON *island_travel = cJSON_GetObjectItem(root, "island_travel_unlocked");
    cJSON *dragon_treasure = cJSON_GetObjectItem(root,
        "dragon_treasure_quest_state");
    cJSON *temple_alignment = cJSON_GetObjectItem(root, "temple_alignment");
    cJSON *temple_sentinels = cJSON_GetObjectItem(root,
        "temple_sentinels_awakened");
    cJSON *temple_treasure = cJSON_GetObjectItem(root,
        "temple_treasure_state");
    cJSON *dialogue_active = cJSON_GetObjectItem(root, "dialogue_active");
    cJSON *dialogue_speaker = cJSON_GetObjectItem(root, "dialogue_speaker");
    cJSON *dialogue_text = cJSON_GetObjectItem(root, "dialogue_text");
    cJSON *dialogue_x = cJSON_GetObjectItem(root, "dialogue_x");
    cJSON *dialogue_y = cJSON_GetObjectItem(root, "dialogue_y");
    g->dungeon_key_found = key_found ? key_found->valueint : 0;
    g->dungeon_crypt_keys = crypt_keys ? crypt_keys->valueint : 0;
    cJSON *lich_reward = cJSON_GetObjectItem(root, "lich_reward_state");
    g->lich_reward_state = lich_reward ? lich_reward->valueint : 0;
    if (g->lich_reward_state < 0 || g->lich_reward_state > 2) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON *necro_reward = cJSON_GetObjectItem(root, "necromancer_reward_state");
    g->necromancer_reward_state = necro_reward ? necro_reward->valueint : 0;
    if (g->necromancer_reward_state < 0 || g->necromancer_reward_state > 2) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON *king_reward = cJSON_GetObjectItem(root, "goblin_king_reward_state");
    g->goblin_king_reward_state = king_reward ? king_reward->valueint : 0;
    if (g->goblin_king_reward_state < 0 || g->goblin_king_reward_state > 2) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON *hammer = cJSON_GetObjectItem(root, "stonewake_warhammer_state");
    if (save_version >= 194 && (!cJSON_IsNumber(hammer) || hammer->valuedouble != hammer->valueint || hammer->valueint < 0 || hammer->valueint > 2)) {
        cJSON_Delete(root);
        return 0;
    }
    g->stonewake_warhammer_state = save_version >= 194 ? hammer->valueint : 0;
    cJSON *prism_reward = cJSON_GetObjectItem(root, "prism_sovereign_reward_pending");
    cJSON *guardian_reward = cJSON_GetObjectItem(root, "sun_guardian_reward_state");
    cJSON *dragon_reward = cJSON_GetObjectItem(root, "red_dragon_reward_state");
    g->red_dragon_reward_state = dragon_reward ? dragon_reward->valueint : 0;
    if (g->red_dragon_reward_state < 0 || g->red_dragon_reward_state > 2) {
        cJSON_Delete(root);
        return 0;
    }
    g->sun_guardian_reward_state = guardian_reward ? guardian_reward->valueint : 0;
    if (g->sun_guardian_reward_state < 0 || g->sun_guardian_reward_state > 2) {
        cJSON_Delete(root);
        return 0;
    }
    g->prism_sovereign_reward_pending = prism_reward ? prism_reward->valueint : 0;
    if (g->prism_sovereign_reward_pending < 0 || g->prism_sovereign_reward_pending > 3) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON *cinder_reward = cJSON_GetObjectItem(root, "cinder_lord_reward_pending");
    g->cinder_lord_reward_pending = cinder_reward ? cinder_reward->valueint : 0;
    if (g->cinder_lord_reward_pending < 0 || g->cinder_lord_reward_pending > 3) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON *pharaoh_reward = cJSON_GetObjectItem(root, "desert_pharaoh_reward_state");
    g->desert_pharaoh_reward_state = pharaoh_reward ? pharaoh_reward->valueint : 0;
    if (g->desert_pharaoh_reward_state < 0 || g->desert_pharaoh_reward_state > 2) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON *marshal_reward = cJSON_GetObjectItem(root, "grave_marshal_reward_state");
    g->grave_marshal_reward_state = marshal_reward ? marshal_reward->valueint : 0;
    if (g->grave_marshal_reward_state < 0 || g->grave_marshal_reward_state > 2) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON *minotaur_reward = cJSON_GetObjectItem(root, "minotaur_reward_state");
    g->minotaur_reward_state = minotaur_reward ? minotaur_reward->valueint : 0;
    if (g->minotaur_reward_state < 0 || g->minotaur_reward_state > 3) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON *demon_reward = cJSON_GetObjectItem(root, "swamp_demon_reward_state");
    g->swamp_demon_reward_state = demon_reward ? demon_reward->valueint : 0;
    if (g->swamp_demon_reward_state < 0 || g->swamp_demon_reward_state > 2) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON *queen_reward = cJSON_GetObjectItem(root, "drowned_queen_reward_state");
    g->drowned_queen_reward_state = queen_reward ? queen_reward->valueint : 0;
    if (g->drowned_queen_reward_state < 0 || g->drowned_queen_reward_state > 2) {
        cJSON_Delete(root);
        return 0;
    }
    g->portal_active = portal_active ? portal_active->valueint : 0;
    g->portal_level = portal_level ? portal_level->valueint : 0;
    g->portal_location = portal_location ? portal_location->valueint :
        LOCATION_DUNGEON;
    g->portal_x = portal_x ? portal_x->valueint : 0;
    g->portal_y = portal_y ? portal_y->valueint : 0;
    g->portal_origin_tile = portal_origin_tile
        ? portal_origin_tile->valueint : TILE_FLOOR;
    g->defeated_bosses = defeated_bosses ? defeated_bosses->valueint : 0;
    g->elowen_quest_state = elowen_quest ? elowen_quest->valueint : 0;
    g->elowen_seals_restored = elowen_seals ? elowen_seals->valueint :
        (legacy_altars ? legacy_altars->valueint : 0);
    g->dain_quest_state = dain_quest ? dain_quest->valueint : 0;
    g->dain_map_fragments = dain_fragments ? dain_fragments->valueint :
        (legacy_dain_targets ? legacy_dain_targets->valueint : 0);
    g->alder_quest_state = alder_quest ? alder_quest->valueint : 0;
    g->alder_wardens_rescued = alder_wardens ? alder_wardens->valueint : 0;
    g->mara_quest_state = mara_quest ? mara_quest->valueint : 0;
    g->mara_beacons_lit = mara_beacons ? mara_beacons->valueint : 0;
    g->cain_scroll_given = cain_scroll ? cain_scroll->valueint : 0;
    g->island_travel_unlocked = island_travel ? island_travel->valueint : 0;
    g->dragon_treasure_quest_state = dragon_treasure ? dragon_treasure->valueint : 0;
    g->temple_alignment = temple_alignment ? temple_alignment->valueint : 0;
    g->temple_sentinels_awakened = temple_sentinels
        ? temple_sentinels->valueint : 0;
    g->temple_treasure_state = temple_treasure
        ? temple_treasure->valueint : 0;
    g->dialogue_active = dialogue_active ? dialogue_active->valueint : 0;
    strncpy(g->dialogue_speaker,
        dialogue_speaker ? dialogue_speaker->valuestring : "",
        MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    strncpy(g->dialogue_text,
        dialogue_text ? dialogue_text->valuestring : "",
        MAX_DIALOGUE_LEN - 1);
    g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
    g->dialogue_x = dialogue_x ? dialogue_x->valueint : 10;
    g->dialogue_y = dialogue_y ? dialogue_y->valueint : 7;

    // Messages
    cJSON *messages = cJSON_GetObjectItem(root, "messages");
    // Saves made before message colours have no kinds and load as plain text.
    cJSON *message_kinds = cJSON_GetObjectItem(root, "message_kinds");
    for (int i = 0; i < g->message_count && i < MAX_MESSAGES; i++) {
        strncpy(g->messages[i],
            cJSON_GetArrayItem(messages, i)->valuestring,
            MAX_MESSAGE_LEN - 1);
        g->messages[i][MAX_MESSAGE_LEN - 1] = '\0';
        cJSON *kind = message_kinds ? cJSON_GetArrayItem(message_kinds, i) : NULL;
        g->message_kinds[i] = kind && kind->valueint > MESSAGE_NORMAL &&
            kind->valueint < MESSAGE_KIND_COUNT ? (MessageKind)kind->valueint : MESSAGE_NORMAL;
    }

    // Controls. Saves made before remapping, or with an unusable set, get the
    // defaults so another character's keys never carry over.
    controls_reset(g->key_bindings);
    cJSON *key_bindings = cJSON_GetObjectItem(root, "key_bindings");
    if (cJSON_IsArray(key_bindings) &&
        cJSON_GetArraySize(key_bindings) == CONTROL_COUNT) {
        int saved[CONTROL_COUNT];
        for (int i = 0; i < CONTROL_COUNT; i++) {
            cJSON *key = cJSON_GetArrayItem(key_bindings, i);
            saved[i] = cJSON_IsNumber(key) ? key->valueint : 0;
        }
        if (controls_valid(saved)) {
            memcpy(g->key_bindings, saved, sizeof(saved));
        }
    }

    // Inventory
    g->inventory_count = cJSON_GetObjectItem(root, "inventory_count")->valueint;
    cJSON *inventory = cJSON_GetObjectItem(root, "inventory");
    for (int i = 0; i < g->inventory_count; i++) {
        cJSON *it = cJSON_GetArrayItem(inventory, i);
        Item *item = &g->inventory[i];
        item->active        = cJSON_GetObjectItem(it, "active")->valueint;
        item->type          = cJSON_GetObjectItem(it, "type")->valueint;
        strncpy(item->name, cJSON_GetObjectItem(it, "name")->valuestring,
            sizeof(item->name) - 1);
        item->attack_bonus  = cJSON_GetObjectItem(it, "attack_bonus")->valueint;
        item->defense_bonus = cJSON_GetObjectItem(it, "defense_bonus")->valueint;
        item->value         = cJSON_GetObjectItem(it, "value")->valueint;
        item->spell_id      = cJSON_GetObjectItem(it, "spell_id")->valueint;
        item->is_ranged     = cJSON_GetObjectItem(it, "is_ranged")->valueint;
        item->range         = cJSON_GetObjectItem(it, "range")->valueint;
        deserialize_item_metadata(it, item);
    }
    game_repair_equipment_indices(g);
    if (!g->island_travel_unlocked &&
        (g->location == LOCATION_ISLAND || g->location == LOCATION_TEMPLE ||
        g->temple_treasure_state > 0 || g->temple_alignment > 0 ||
        (g->defeated_bosses & (1 << LOCATION_TEMPLE)))) {
        g->island_travel_unlocked = 1;
    }
    if (g->island_travel_unlocked) {
        for (int i = 0; i < g->inventory_count;) {
            if (g->inventory[i].type == ITEM_TREASURE_MAP) {
                game_remove_inventory_item(g, i);
            } else {
                i++;
            }
        }
    }

    // Floor items
    g->floor_item_count = cJSON_GetObjectItem(root, "floor_item_count")->valueint;
    cJSON *floor_items = cJSON_GetObjectItem(root, "floor_items");
    for (int i = 0; i < g->floor_item_count; i++) {
        cJSON *f = cJSON_GetArrayItem(floor_items, i);
        FloorItem *fi = &g->floor_items[i];
        fi->active = cJSON_GetObjectItem(f, "active")->valueint;
        fi->x      = cJSON_GetObjectItem(f, "x")->valueint;
        fi->y      = cJSON_GetObjectItem(f, "y")->valueint;
        cJSON *underlying = cJSON_GetObjectItem(f, "underlying_tile");
        fi->underlying_tile = underlying ? underlying->valueint :
            (g->location == LOCATION_FOREST ? TILE_FOREST_FLOOR :
            (g->location == LOCATION_MOUNTAINS ? TILE_MOUNTAIN_FLOOR :
            (g->location == LOCATION_COAST ? TILE_COAST_FLOOR : TILE_FLOOR)));
        cJSON *it = cJSON_GetObjectItem(f, "item");
        fi->item.active        = cJSON_GetObjectItem(it, "active")->valueint;
        fi->item.type          = cJSON_GetObjectItem(it, "type")->valueint;
        strncpy(fi->item.name, cJSON_GetObjectItem(it, "name")->valuestring,
            sizeof(fi->item.name) - 1);
        fi->item.attack_bonus  = cJSON_GetObjectItem(it, "attack_bonus")->valueint;
        fi->item.defense_bonus = cJSON_GetObjectItem(it, "defense_bonus")->valueint;
        fi->item.value         = cJSON_GetObjectItem(it, "value")->valueint;
        fi->item.spell_id      = cJSON_GetObjectItem(it, "spell_id")->valueint;
        fi->item.is_ranged     = cJSON_GetObjectItem(it, "is_ranged")->valueint;
        fi->item.range         = cJSON_GetObjectItem(it, "range")->valueint;
        deserialize_item_metadata(it, &fi->item);
    }

    // Current map
    deserialize_map(cJSON_GetObjectItem(root, "map"), &g->map);

    // Current enemies
    deserialize_enemies(cJSON_GetObjectItem(root, "enemies"),
                        g->enemies, &g->enemy_count, sizeof(g->enemies) / sizeof(Enemy));

    // Level cache
    cJSON *cache = cJSON_GetObjectItem(root, "level_cache");
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        cJSON *entry = cJSON_GetArrayItem(cache, i);
        g->level_cache[i].valid = 0;
        g->level_cache[i].level_cleared = 0;
        if (!entry) {
            continue;
        }
        g->level_cache[i].valid         = cJSON_GetObjectItem(entry, "valid")->valueint;
        g->level_cache[i].level_cleared = cJSON_GetObjectItem(entry, "level_cleared")->valueint;
        if (g->level_cache[i].valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"),
                            &g->level_cache[i].map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                                g->level_cache[i].enemies,
                                &g->level_cache[i].enemy_count, sizeof(g->level_cache[i].enemies) / sizeof(Enemy));
        }
    }

    cJSON *forest_cache = cJSON_GetObjectItem(root, "forest_cache");
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        g->forest_cache[i].valid = 0;
        g->forest_cache[i].level_cleared = 0;
        if (!forest_cache) continue;
        cJSON *entry = cJSON_GetArrayItem(forest_cache, i);
        if (!entry) continue;
        cJSON *valid = cJSON_GetObjectItem(entry, "valid");
        cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
        g->forest_cache[i].valid = valid ? valid->valueint : 0;
        g->forest_cache[i].level_cleared = cleared ? cleared->valueint : 0;
        if (g->forest_cache[i].valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"),
                &g->forest_cache[i].map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                g->forest_cache[i].enemies,
                &g->forest_cache[i].enemy_count, sizeof(g->forest_cache[i].enemies) / sizeof(Enemy));
        }
    }

    cJSON *mountain_cache = cJSON_GetObjectItem(root, "mountain_cache");
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        g->mountain_cache[i].valid = 0;
        g->mountain_cache[i].level_cleared = 0;
        if (!mountain_cache) continue;
        cJSON *entry = cJSON_GetArrayItem(mountain_cache, i);
        if (!entry) continue;
        cJSON *valid = cJSON_GetObjectItem(entry, "valid");
        cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
        g->mountain_cache[i].valid = valid ? valid->valueint : 0;
        g->mountain_cache[i].level_cleared = cleared ? cleared->valueint : 0;
        if (g->mountain_cache[i].valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"),
                &g->mountain_cache[i].map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                g->mountain_cache[i].enemies,
                &g->mountain_cache[i].enemy_count, sizeof(g->mountain_cache[i].enemies) / sizeof(Enemy));
        }
    }

    cJSON *coast_cache = cJSON_GetObjectItem(root, "coast_cache");
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        g->coast_cache[i].valid = 0;
        g->coast_cache[i].level_cleared = 0;
        if (!coast_cache) {
            continue;
        }
        cJSON *entry = cJSON_GetArrayItem(coast_cache, i);
        if (!entry) {
            continue;
        }
        cJSON *valid = cJSON_GetObjectItem(entry, "valid");
        cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
        g->coast_cache[i].valid = valid ? valid->valueint : 0;
        g->coast_cache[i].level_cleared = cleared ? cleared->valueint : 0;
        if (g->coast_cache[i].valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"),
                &g->coast_cache[i].map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                g->coast_cache[i].enemies,
                &g->coast_cache[i].enemy_count, sizeof(g->coast_cache[i].enemies) / sizeof(Enemy));
        }
    }

    cJSON *swamp_cache = cJSON_GetObjectItem(root, "swamp_cache");
    for (int i = 0; i < SWAMP_CACHE_DEPTH; i++) {
        g->swamp_cache[i].valid = 0;
        g->swamp_cache[i].level_cleared = 0;
        if (!swamp_cache) {
            continue;
        }
        cJSON *entry = cJSON_GetArrayItem(swamp_cache, i);
        if (!entry) {
            continue;
        }
        cJSON *valid = cJSON_GetObjectItem(entry, "valid");
        cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
        g->swamp_cache[i].valid = valid ? valid->valueint : 0;
        g->swamp_cache[i].level_cleared = cleared ? cleared->valueint : 0;
        if (g->swamp_cache[i].valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"),
                &g->swamp_cache[i].map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                g->swamp_cache[i].enemies,
                &g->swamp_cache[i].enemy_count, sizeof(g->swamp_cache[i].enemies) / sizeof(Enemy));
        }
    }

    cJSON *dragonspine_cache = cJSON_GetObjectItem(root, "dragonspine_cache");
    for (int i = 0; i < DRAGONSPINE_DEPTH; i++) {
        g->dragonspine_cache[i].valid = 0;
        g->dragonspine_cache[i].level_cleared = 0;
        if (!dragonspine_cache) {
            continue;
        }
        cJSON *entry = cJSON_GetArrayItem(dragonspine_cache, i);
        if (!entry) {
            continue;
        }
        cJSON *valid = cJSON_GetObjectItem(entry, "valid");
        cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
        g->dragonspine_cache[i].valid = valid ? valid->valueint : 0;
        g->dragonspine_cache[i].level_cleared = cleared ? cleared->valueint : 0;
        if (g->dragonspine_cache[i].valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"),
                &g->dragonspine_cache[i].map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                g->dragonspine_cache[i].enemies,
                &g->dragonspine_cache[i].enemy_count, sizeof(g->dragonspine_cache[i].enemies) / sizeof(Enemy));
        }
    }

    // Saves from before Frostfell have no cache and start it fresh.
    cJSON *frostfell_cache = cJSON_GetObjectItem(root, "frostfell_cache");
    for (int i = 0; i < FROSTFELL_DEPTH; i++) {
        g->frostfell_cache[i].valid = 0;
        g->frostfell_cache[i].level_cleared = 0;
        cJSON *entry = frostfell_cache ? cJSON_GetArrayItem(frostfell_cache, i) : NULL;
        if (!entry) {
            continue;
        }
        cJSON *valid = cJSON_GetObjectItem(entry, "valid");
        cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
        g->frostfell_cache[i].valid = valid ? valid->valueint : 0;
        g->frostfell_cache[i].level_cleared = cleared ? cleared->valueint : 0;
        if (g->frostfell_cache[i].valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"),
                &g->frostfell_cache[i].map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                g->frostfell_cache[i].enemies,
                &g->frostfell_cache[i].enemy_count, sizeof(g->frostfell_cache[i].enemies) / sizeof(Enemy));
        }
    }

    for (int i = 0; i < DESERT_DEPTH; i++) {
        LevelCache *cache = &g->desert_cache[i];
        *cache = (LevelCache){0};
        cJSON *entry = cJSON_GetArrayItem(desert_cache, i);
        cache->valid = cJSON_GetObjectItem(entry, "valid")->valueint;
        cache->level_cleared = cJSON_GetObjectItem(entry, "level_cleared")->valueint;
        if (cache->valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"), &cache->map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                cache->enemies, &cache->enemy_count, sizeof(cache->enemies) / sizeof(Enemy));
        }
    }

    for (int i = 0; i < ASHEN_DEPTH; i++) {
        LevelCache *cache = &g->ashen_cache[i];
        *cache = (LevelCache){0};
        cJSON *entry = cJSON_GetArrayItem(ashen_cache, i);
        cJSON *valid = cJSON_GetObjectItem(entry, "valid");
        cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
        if (!cJSON_IsNumber(valid) || !cJSON_IsNumber(cleared)) {
            cJSON_Delete(root);
            return 0;
        }
        cache->valid = valid->valueint;
        cache->level_cleared = cleared->valueint;
        if (cache->valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"), &cache->map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                cache->enemies, &cache->enemy_count, sizeof(cache->enemies) / sizeof(Enemy));
        }
    }

    for (int i = 0; i < GLASSDEEP_DEPTH; i++) {
        LevelCache *cache = &g->glassdeep_cache[i];
        *cache = (LevelCache){0};
        cJSON *entry = cJSON_GetArrayItem(glassdeep_cache, i);
        cJSON *valid = cJSON_GetObjectItem(entry, "valid");
        cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
        if (!cJSON_IsNumber(valid) || !cJSON_IsNumber(cleared)) {
            cJSON_Delete(root);
            return 0;
        }
        cache->valid = valid->valueint;
        cache->level_cleared = cleared->valueint;
        if (cache->valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"), &cache->map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                cache->enemies, &cache->enemy_count, sizeof(cache->enemies) / sizeof(Enemy));
        }
    }

    for (int i = 0; i < CATACOMBS_DEPTH; i++) {
        LevelCache *cache = &g->catacombs_cache[i];
        *cache = (LevelCache){0};
        cJSON *entry = cJSON_GetArrayItem(catacombs_cache, i);
        cJSON *valid = cJSON_GetObjectItem(entry, "valid");
        cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
        if (!cJSON_IsNumber(valid) || !cJSON_IsNumber(cleared)) {
            cJSON_Delete(root);
            return 0;
        }
        cache->valid = valid->valueint;
        cache->level_cleared = cleared->valueint;
        if (cache->valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"), &cache->map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                cache->enemies, &cache->enemy_count, sizeof(cache->enemies) / sizeof(Enemy));
        }
    }

    memset(g->castle_cache, 0, sizeof(g->castle_cache));
    memset(g->castle_loot, 0, sizeof(g->castle_loot));
    memset(g->castle_loot_count, 0, sizeof(g->castle_loot_count));
    memset(g->castle_fire_phase, 0, sizeof(g->castle_fire_phase));
    g->castle_minibosses = 0;
    g->castellan_reward_state = 0;
    g->arcanist_reward_state = 0;
    g->castle_prompt = 0;
    g->game_won = 0;
    g->crown_of_unbound = 0;
    cJSON *castle = cJSON_GetObjectItem(root, "castle");
    if (castle) {
        cJSON *minis = cJSON_GetObjectItem(castle, "minibosses");
        cJSON *reward = cJSON_GetObjectItem(castle, "castellan_reward");
        cJSON *arcanist_reward = cJSON_GetObjectItem(castle, "arcanist_reward");
        cJSON *prompt = cJSON_GetObjectItem(castle, "prompt");
        cJSON *won = cJSON_GetObjectItem(castle, "won");
        cJSON *crown = cJSON_GetObjectItem(castle, "crown_of_unbound");
        cJSON *floors = cJSON_GetObjectItem(castle, "floors");
        if ((save_version >= 139 || crown) && (!cJSON_IsNumber(crown) || crown->valuedouble != crown->valueint || crown->valueint < 0 || crown->valueint > 1)) {
            cJSON_Delete(root);
            return 0;
        }
        g->crown_of_unbound = crown ? crown->valueint : 0;
        if ((save_version >= 137 || reward) && (!cJSON_IsNumber(reward) || reward->valuedouble != reward->valueint || reward->valueint < 0 || reward->valueint > 2)) {
            cJSON_Delete(root);
            return 0;
        }
        g->castellan_reward_state = reward ? reward->valueint : 0;
        if ((save_version >= 138 || arcanist_reward) && (!cJSON_IsNumber(arcanist_reward) || arcanist_reward->valuedouble != arcanist_reward->valueint || arcanist_reward->valueint < 0 || arcanist_reward->valueint > 2)) {
            cJSON_Delete(root);
            return 0;
        }
        g->arcanist_reward_state = arcanist_reward ? arcanist_reward->valueint : 0;
        if (!cJSON_IsNumber(minis) || !cJSON_IsNumber(prompt) || !cJSON_IsNumber(won) ||
            !cJSON_IsArray(floors) || cJSON_GetArraySize(floors) != CASTLE_DEPTH ||
            minis->valueint < 0 || minis->valueint > 3 || prompt->valueint < 0 || prompt->valueint > 2) {
            cJSON_Delete(root);
            return 0;
        }
        g->castle_minibosses = minis->valueint;
        g->castle_prompt = prompt->valueint;
        g->game_won = won->valueint;
        for (int i = 0; i < CASTLE_DEPTH; i++) {
            LevelCache *cache = &g->castle_cache[i];
            cJSON *entry = cJSON_GetArrayItem(floors, i);
            cJSON *valid = cJSON_GetObjectItem(entry, "valid");
            cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
            cJSON *fire = cJSON_GetObjectItem(entry, "fire_phase");
            if ((save_version >= 120 || fire) && (!cJSON_IsNumber(fire) || fire->valuedouble != fire->valueint || fire->valueint < 0 || fire->valueint > 3)) {
                cJSON_Delete(root);
                return 0;
            }
            g->castle_fire_phase[i] = fire ? fire->valueint : 0;
            if (!cJSON_IsNumber(valid) || !cJSON_IsNumber(cleared) || !deserialize_castle_loot(cJSON_GetObjectItem(entry, "loot"), g->castle_loot[i], &g->castle_loot_count[i])) {
                cJSON_Delete(root);
                return 0;
            }
            cache->valid = valid->valueint;
            cache->level_cleared = cleared->valueint;
            if (cache->valid) {
                cJSON *enemies = cJSON_GetObjectItem(entry, "enemies");
                if (!cJSON_IsObject(cJSON_GetObjectItem(entry, "map")) || !cJSON_IsArray(enemies) || cJSON_GetArraySize(enemies) > NON_ROAD_ENEMY_LIMIT) {
                    cJSON_Delete(root);
                    return 0;
                }
                deserialize_map(cJSON_GetObjectItem(entry, "map"), &cache->map);
                deserialize_enemies(enemies, cache->enemies, &cache->enemy_count, NON_ROAD_ENEMY_LIMIT);
            }
        }
    } else if (save_version >= 86) {
        cJSON_Delete(root);
        return 0;
    }
    if (g->location == LOCATION_CASTLE_INTERIOR && (g->level < 1 || g->level > CASTLE_DEPTH)) {
        cJSON_Delete(root);
        return 0;
    }

    const char *road_keys[2] = {"crownroad_cache", "kingroad_west_cache"};
    CrownroadCache *road_caches[2] = {&g->crownroad_cache, &g->kingroad_west_cache};
    for (int i = 0; i < 2; i++) {
        CrownroadCache *cache = road_caches[i];
        *cache = (CrownroadCache){0};
        cJSON *entry = cJSON_GetObjectItem(root, road_keys[i]);
        if (entry) {
            cJSON *valid = cJSON_GetObjectItem(entry, "valid");
            cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
            cache->valid = valid ? valid->valueint : 0;
            cache->level_cleared = cleared ? cleared->valueint : 0;
            if (cache->valid) {
                deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                    cache->enemies, &cache->enemy_count, sizeof(cache->enemies) / sizeof(Enemy));
            }
        }
    }

    cJSON *temple_cache = cJSON_GetObjectItem(root, "temple_cache");
    for (int i = 0; i < TEMPLE_DEPTH; i++) {
        g->temple_cache[i].valid = 0;
        g->temple_cache[i].level_cleared = 0;
        if (!temple_cache) {
            continue;
        }
        cJSON *entry = cJSON_GetArrayItem(temple_cache, i);
        if (!entry) {
            continue;
        }
        cJSON *valid = cJSON_GetObjectItem(entry, "valid");
        cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
        g->temple_cache[i].valid = valid ? valid->valueint : 0;
        g->temple_cache[i].level_cleared = cleared ? cleared->valueint : 0;
        if (g->temple_cache[i].valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"),
                &g->temple_cache[i].map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                g->temple_cache[i].enemies,
                &g->temple_cache[i].enemy_count, sizeof(g->temple_cache[i].enemies) / sizeof(Enemy));
        }
    }

    cJSON *labyrinth_cache = cJSON_GetObjectItem(root, "labyrinth_cache");
    for (int i = 0; i < LABYRINTH_DEPTH; i++) {
        g->labyrinth_cache[i].valid = 0;
        g->labyrinth_cache[i].level_cleared = 0;
        if (!labyrinth_cache) {
            continue;
        }
        cJSON *entry = cJSON_GetArrayItem(labyrinth_cache, i);
        if (!entry) {
            continue;
        }
        cJSON *valid = cJSON_GetObjectItem(entry, "valid");
        cJSON *cleared = cJSON_GetObjectItem(entry, "level_cleared");
        g->labyrinth_cache[i].valid = valid ? valid->valueint : 0;
        g->labyrinth_cache[i].level_cleared = cleared ? cleared->valueint : 0;
        if (g->labyrinth_cache[i].valid) {
            deserialize_map(cJSON_GetObjectItem(entry, "map"),
                &g->labyrinth_cache[i].map);
            deserialize_enemies(cJSON_GetObjectItem(entry, "enemies"),
                g->labyrinth_cache[i].enemies,
                &g->labyrinth_cache[i].enemy_count, sizeof(g->labyrinth_cache[i].enemies) / sizeof(Enemy));
        }
    }

    if (save_version < 83 && g->location == LOCATION_CASTLE) {
        g->map.tiles[TOWN_H - 1][CROWNROAD_X] = TILE_TOWN_EXIT;
        for (int y = CASTLE_ROAD_Y; y < TOWN_H - 1; y++) {
            if (g->map.tiles[y][CROWNROAD_X] != TILE_ITEM) {
                g->map.tiles[y][CROWNROAD_X] = TILE_TOWN_PATH;
            }
        }
    }

    // Preserve the old numbering until route migrations have moved saved floors.
    int dungeon_depth = save_version < 85 ? 8 : DUNGEON_DEPTH;
    if (g->max_level_reached > dungeon_depth) {
        g->max_level_reached = dungeon_depth;
    }

    if (save_version < 2) {
        g->level_cache[dungeon_depth - 1].valid = 0;
        if (g->location == LOCATION_DUNGEON && g->level == dungeon_depth) {
            g->floor_item_count = 0;
            map_generate(&g->map, g->level);
            enemies_spawn(g);
            g->level_cleared = 0;
            g->player.x = g->map.stairs_up_x;
            g->player.y = g->map.stairs_up_y;
        }
    }

    if (save_version < 3) {
        g->level_cache[dungeon_depth - 1].valid = 0;
        g->dungeon_key_found = 0;
        g->portal_active = 0;
        if (g->location == LOCATION_DUNGEON && g->level == dungeon_depth) {
            g->floor_item_count = 0;
            map_generate(&g->map, g->level);
            enemies_spawn(g);
            g->level_cleared = 0;
            g->player.x = g->map.stairs_up_x;
            g->player.y = g->map.stairs_up_y;
        }

        int has_return = 0;
        for (int i = 0; i < g->player.known_spell_count; i++)
            if (g->player.known_spells[i].id == SPELL_RETURN_TO_TOWN)
                has_return = 1;
        for (int i = 0; i < g->inventory_count; i++)
            if (g->inventory[i].type == ITEM_SCROLL &&
                g->inventory[i].spell_id == SPELL_RETURN_TO_TOWN)
                has_return = 1;
        if (!has_return && g->inventory_count < MAX_INVENTORY) {
            g->inventory[g->inventory_count++] =
                item_make_scroll_return_to_town();
        } else if (!has_return &&
            g->player.known_spell_count < MAX_SPELLS) {
            g->player.known_spells[g->player.known_spell_count++] =
                spell_make_return_to_town();
        }
    }

    // Version 4 replaces the porous random boss room with a sealed arena,
    // guarantees a visible key, and gives the Lich dedicated encounter AI.
    if (save_version < 4) {
        g->level_cache[dungeon_depth - 1].valid = 0;
        g->dungeon_key_found = 0;
        if (g->location == LOCATION_DUNGEON && g->level == dungeon_depth) {
            g->floor_item_count = 0;
            map_generate(&g->map, g->level);
            enemies_spawn(g);
            g->level_cleared = 0;
            g->player.x = g->map.stairs_up_x;
            g->player.y = g->map.stairs_up_y;
        }
    }

    if (save_version < 5) {
        g->max_forest_level_reached = 1;
        for (int i = 0; i < MAX_REGION_DEPTH; i++)
            g->forest_cache[i].valid = 0;
        g->portal_location = LOCATION_DUNGEON;
    }

    // Version 6 replaces forest stairs with a west-to-east stage route.
    if (save_version < 6) {
        g->max_forest_level_reached = 1;
        for (int i = 0; i < MAX_REGION_DEPTH; i++)
            g->forest_cache[i].valid = 0;
        if (g->portal_location == LOCATION_FOREST)
            g->portal_active = 0;
        if (g->location == LOCATION_FOREST) {
            g->level = 1;
            g->level_cleared = 0;
            g->floor_item_count = 0;
            map_generate_forest(&g->map, g->level);
            enemies_spawn(g);
            g->player.x = g->map.stairs_up_x;
            g->player.y = g->map.stairs_up_y;
        }
    }

    // Version 8 gives each forest level its own branching topology.
    if (save_version < 8) {
        g->max_forest_level_reached = 1;
        for (int i = 0; i < MAX_REGION_DEPTH; i++)
            g->forest_cache[i].valid = 0;
        if (g->portal_location == LOCATION_FOREST)
            g->portal_active = 0;
        if (g->location == LOCATION_FOREST) {
            g->level = 1;
            g->level_cleared = 0;
            g->floor_item_count = 0;
            map_generate_forest(&g->map, g->level);
            enemies_spawn(g);
            g->player.x = g->map.stairs_up_x;
            g->player.y = g->map.stairs_up_y;
        }
    }

    if (save_version < 9) {
        g->max_mountain_level_reached = 1;
        for (int i = 0; i < MAX_REGION_DEPTH; i++)
            g->mountain_cache[i].valid = 0;
        if (g->portal_location == LOCATION_MOUNTAINS)
            g->portal_active = 0;
    }

    // Version 10 gives outdoor stages north and south exits. Cached maps from
    // earlier versions must be regenerated because they contain east exits.
    if (save_version < 10) {
        g->max_forest_level_reached = 1;
        g->max_mountain_level_reached = 1;
        for (int i = 0; i < MAX_REGION_DEPTH; i++) {
            g->forest_cache[i].valid = 0;
            g->mountain_cache[i].valid = 0;
        }
        if (g->portal_location == LOCATION_FOREST ||
            g->portal_location == LOCATION_MOUNTAINS) {
            g->portal_active = 0;
        }
        if (g->location == LOCATION_FOREST ||
            g->location == LOCATION_MOUNTAINS) {
            g->level = 1;
            g->level_cleared = 0;
            g->floor_item_count = 0;
            if (g->location == LOCATION_FOREST) {
                map_generate_forest(&g->map, g->level);
            } else {
                map_generate_mountains(&g->map, g->level);
            }
            enemies_spawn(g);
            g->player.x = g->map.stairs_up_x;
            g->player.y = g->map.stairs_up_y;
        }
    }

    // Version 11 expands every current region from five to eight stages.
    // Regenerate regional progress so former stage-five boss maps cannot be
    // mistaken for the new finales. Character progression remains intact.
    if (save_version < 11) {
        g->max_level_reached = 1;
        g->max_forest_level_reached = 1;
        g->max_mountain_level_reached = 1;
        for (int i = 0; i < MAX_REGION_DEPTH; i++) {
            g->level_cache[i].valid = 0;
            g->forest_cache[i].valid = 0;
            g->mountain_cache[i].valid = 0;
        }
        g->portal_active = 0;
        g->dungeon_key_found = 0;
        if (g->location != LOCATION_TOWN) {
            g->level = 1;
            g->level_cleared = 0;
            g->floor_item_count = 0;
            if (g->location == LOCATION_FOREST) {
                map_generate_forest(&g->map, g->level);
            } else if (g->location == LOCATION_MOUNTAINS) {
                map_generate_mountains(&g->map, g->level);
            } else {
                map_generate(&g->map, g->level);
            }
            enemies_spawn(g);
            g->player.x = g->map.stairs_up_x;
            g->player.y = g->map.stairs_up_y;
        }
    }

    // Version 12 varies outdoor entrance edges and redistributes false paths.
    if (save_version < 12) {
        g->max_forest_level_reached = 1;
        g->max_mountain_level_reached = 1;
        for (int i = 0; i < MAX_REGION_DEPTH; i++) {
            g->forest_cache[i].valid = 0;
            g->mountain_cache[i].valid = 0;
        }
        if (g->portal_location == LOCATION_FOREST ||
            g->portal_location == LOCATION_MOUNTAINS) {
            g->portal_active = 0;
        }
        if (g->location == LOCATION_FOREST ||
            g->location == LOCATION_MOUNTAINS) {
            g->level = 1;
            g->level_cleared = 0;
            g->floor_item_count = 0;
            if (g->location == LOCATION_FOREST) {
                map_generate_forest(&g->map, g->level);
            } else {
                map_generate_mountains(&g->map, g->level);
            }
            enemies_spawn(g);
            g->player.x = g->map.stairs_up_x;
            g->player.y = g->map.stairs_up_y;
        }
    }

    // Version 13 adds the decorative tavern to the town map.
    if (save_version < 13 && g->location == LOCATION_TOWN) {
        int player_x = g->player.x;
        int player_y = g->player.y;
        int spawn_x;
        int spawn_y;
        map_generate_town(&g->map, &spawn_x, &spawn_y);
        if (map_is_walkable(&g->map, player_x, player_y)) {
            g->player.x = player_x;
            g->player.y = player_y;
        } else {
            g->player.x = spawn_x;
            g->player.y = spawn_y;
        }
        if (g->portal_active) {
            g->map.tiles[2][20] = TILE_PORTAL;
        }
    }

    // Version 14 adds the south gate and the Sunken Coast adventure.
    if (save_version < 14) {
        g->max_coast_level_reached = 1;
        for (int i = 0; i < MAX_REGION_DEPTH; i++) {
            g->coast_cache[i].valid = 0;
        }
        if (g->location == LOCATION_TOWN) {
            int player_x = g->player.x;
            int player_y = g->player.y;
            int spawn_x;
            int spawn_y;
            map_generate_town(&g->map, &spawn_x, &spawn_y);
            if (map_is_walkable(&g->map, player_x, player_y)) {
                g->player.x = player_x;
                g->player.y = player_y;
            } else {
                g->player.x = spawn_x;
                g->player.y = spawn_y;
            }
            if (g->portal_active) {
                g->map.tiles[2][20] = TILE_PORTAL;
            }
        }
    }

    // Version 15 replaces rectangular forest rooms with organic trails and
    // landmark-revealed exits. Old cached forests cannot support that flow.
    if (save_version < 15) {
        g->max_forest_level_reached = 1;
        for (int i = 0; i < MAX_REGION_DEPTH; i++) {
            g->forest_cache[i].valid = 0;
        }
        if (g->portal_location == LOCATION_FOREST) {
            g->portal_active = 0;
        }
        if (g->location == LOCATION_FOREST) {
            g->level = 1;
            g->level_cleared = 0;
            g->floor_item_count = 0;
            map_generate_forest(&g->map, g->level);
            enemies_spawn(g);
            g->player.x = g->map.stairs_up_x;
            g->player.y = g->map.stairs_up_y;
        }
    }

    // Version 16 gives the Mountains dedicated pass, bridge, cave, and
    // fortress templates. Regenerate old forest-derived mountain maps.
    if (save_version < 16) {
        g->max_mountain_level_reached = 1;
        for (int i = 0; i < MAX_REGION_DEPTH; i++) {
            g->mountain_cache[i].valid = 0;
        }
        if (g->portal_location == LOCATION_MOUNTAINS) {
            g->portal_active = 0;
        }
        if (g->location == LOCATION_MOUNTAINS) {
            g->level = 1;
            g->level_cleared = 0;
            g->floor_item_count = 0;
            map_generate_mountains(&g->map, g->level);
            enemies_spawn(g);
            g->player.x = g->map.stairs_up_x;
            g->player.y = g->map.stairs_up_y;
        }
    }

    // Version 17 adds persistent coast tides and control mechanisms. Cached
    // coast maps from earlier versions lack the water-state progression.
    if (save_version < 17) {
        g->max_coast_level_reached = 1;
        for (int i = 0; i < MAX_REGION_DEPTH; i++) {
            g->coast_cache[i].valid = 0;
        }
        if (g->portal_location == LOCATION_COAST) {
            g->portal_active = 0;
        }
        if (g->location == LOCATION_COAST) {
            g->level = 1;
            g->level_cleared = 0;
            g->floor_item_count = 0;
            map_generate_coast(&g->map, g->level);
            enemies_spawn(g);
            g->player.x = g->map.stairs_up_x;
            g->player.y = g->map.stairs_up_y;
        }
    }

    // Version 18 adds permanent boss completion and Elowen's persistent
    // quest. Previously defeated cached bosses remain defeated.
    if (save_version < 18) {
        LevelCache *boss_caches[4] = {
            g->level_cache, g->forest_cache, g->mountain_cache, g->coast_cache
        };
        EnemyType boss_types[4] = {
            ENEMY_LICH_KING, ENEMY_FOREST_NECROMANCER,
            ENEMY_MOUNTAIN_GOBLIN_KING, ENEMY_DROWNED_QUEEN
        };
        Location boss_locations[4] = {
            LOCATION_DUNGEON, LOCATION_FOREST,
            LOCATION_MOUNTAINS, LOCATION_COAST
        };
        int boss_depths[4] = {
            // Dungeon and coast caches still use eight-stage numbering here.
            8, FOREST_DEPTH, MOUNTAIN_DEPTH, 8
        };
        for (int region = 0; region < 4; region++) {
            LevelCache *finale = &boss_caches[region][boss_depths[region] - 1];
            if (!finale->valid) {
                continue;
            }
            for (int i = 0; i < finale->enemy_count; i++) {
                if (finale->enemies[i].type == boss_types[region] &&
                    !finale->enemies[i].active) {
                    g->defeated_bosses |= 1 << boss_locations[region];
                }
            }
        }
        g->elowen_quest_state = 0;
        g->elowen_seals_restored = 0;
    }

    // Version 19 makes the Tavern door enterable. Rebuild legacy town maps
    // so their decorative facade receives the doorway tile.
    if (save_version < 19 && g->location == LOCATION_TOWN) {
        int player_x = g->player.x;
        int player_y = g->player.y;
        int spawn_x;
        int spawn_y;
        map_generate_town(&g->map, &spawn_x, &spawn_y);
        if (map_is_walkable(&g->map, player_x, player_y)) {
            g->player.x = player_x;
            g->player.y = player_y;
        } else {
            g->player.x = spawn_x;
            g->player.y = spawn_y;
        }
        if (g->portal_active) {
            g->map.tiles[2][20] = TILE_PORTAL;
        }
    }

    // Version 23 adds Dain and his mountain quest. Existing Tavern saves gain
    // his NPC tile without discarding the rest of the interior state.
    if (save_version < 23 && g->location == LOCATION_TAVERN) {
        if (g->player.x == 18 && g->player.y == 7) {
            g->player.y = 8;
        }
        g->map.tiles[7][18] = TILE_NPC_DAIN;
    }

    // Version 26 adds Alder and The Lost Wardens. Existing Tavern saves gain
    // his NPC tile while older characters begin with the quest unassigned.
    if (save_version < 26) {
        g->alder_quest_state = 0;
        g->alder_wardens_rescued = 0;
        if (g->location == LOCATION_TAVERN) {
            if (g->player.x == 28 && g->player.y == 7) {
                g->player.y = 8;
            }
            g->map.tiles[7][28] = TILE_NPC_ALDER;
        }
    }

    // Version 27 adds Mara and Relight the Drowned Beacons. Existing Tavern
    // saves gain her NPC tile while older characters start without the quest.
    if (save_version < 27) {
        g->mara_quest_state = 0;
        g->mara_beacons_lit = 0;
        if (g->location == LOCATION_TAVERN) {
            if (g->player.x == 31 && g->player.y == 18) {
                g->player.y = 19;
            }
            g->map.tiles[18][31] = TILE_NPC_MARA;
        }
    }

    // Version 28 makes Coast tide controls persistent and reversible. Restart
    // legacy Coast expeditions so every stage uses the new water-state tiles.
    if (save_version < 28) {
        for (int i = 0; i < MAX_REGION_DEPTH; i++) {
            g->coast_cache[i].valid = 0;
        }
        if (g->portal_location == LOCATION_COAST) {
            g->portal_active = 0;
        }
        if (g->location == LOCATION_COAST) {
            g->floor_item_count = 0;
            game_enter_coast(g);
        }
    }

    // Version 29 gives each town shop a walk-in door and makes the remaining
    // facade solid. Rebuild legacy town maps to receive the entrance tiles.
    if (save_version < 29 && g->location == LOCATION_TOWN) {
        int player_x = g->player.x;
        int player_y = g->player.y;
        int spawn_x;
        int spawn_y;
        map_generate_town(&g->map, &spawn_x, &spawn_y);
        if (map_is_walkable(&g->map, player_x, player_y)) {
            g->player.x = player_x;
            g->player.y = player_y;
        } else {
            g->player.x = spawn_x;
            g->player.y = spawn_y;
        }
        if (g->portal_active) {
            g->map.tiles[2][20] = TILE_PORTAL;
        }
    }

    // Version 30 adds the closed watchtower to the southeast town lot.
    if (save_version < 30 && g->location == LOCATION_TOWN) {
        int player_x = g->player.x;
        int player_y = g->player.y;
        int spawn_x;
        int spawn_y;
        map_generate_town(&g->map, &spawn_x, &spawn_y);
        if (map_is_walkable(&g->map, player_x, player_y)) {
            g->player.x = player_x;
            g->player.y = player_y;
        } else {
            g->player.x = spawn_x;
            g->player.y = spawn_y;
        }
        if (g->portal_active) {
            g->map.tiles[2][20] = TILE_PORTAL;
        }
    }

    // Version 38 rebalances named weapons. Update legacy inventory and floor
    // items, then preserve the correct equipped attack total.
    if (save_version < 38) {
        int old_attack_bonus = 0;
        if (g->equipped_main_hand >= 0 &&
            g->equipped_main_hand < g->inventory_count) {
            old_attack_bonus =
                g->inventory[g->equipped_main_hand].attack_bonus;
        }
        for (int i = 0; i < g->inventory_count; i++) {
            if (g->inventory[i].type == ITEM_WEAPON) {
                item_apply_legacy_metadata(&g->inventory[i]);
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].item.type == ITEM_WEAPON) {
                item_apply_legacy_metadata(&g->floor_items[i].item);
            }
        }
        if (g->equipped_main_hand >= 0 &&
            g->equipped_main_hand < g->inventory_count) {
            int new_attack_bonus =
                g->inventory[g->equipped_main_hand].attack_bonus;
            g->player.attack += new_attack_bonus - old_attack_bonus;
        }
    }

    // Version 39 introduces class-specific armor traits and rebalances the
    // existing Leather Armor and Chain Mail definitions.
    if (save_version < 39) {
        int old_defense_bonus = 0;
        int old_hp_bonus = 0;
        int old_mp_bonus = 0;
        if (g->equipped_armor >= 0 &&
            g->equipped_armor < g->inventory_count) {
            Item *old_armor = &g->inventory[g->equipped_armor];
            old_defense_bonus = old_armor->defense_bonus;
            old_hp_bonus = old_armor->max_hp_bonus;
            old_mp_bonus = old_armor->max_mp_bonus;
        }
        for (int i = 0; i < g->inventory_count; i++) {
            if (g->inventory[i].type == ITEM_ARMOR) {
                item_apply_legacy_armor_metadata(&g->inventory[i]);
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].item.type == ITEM_ARMOR) {
                item_apply_legacy_armor_metadata(&g->floor_items[i].item);
            }
        }
        if (g->equipped_armor >= 0 &&
            g->equipped_armor < g->inventory_count) {
            Item *new_armor = &g->inventory[g->equipped_armor];
            g->player.defense +=
                new_armor->defense_bonus - old_defense_bonus;
            int hp_difference = new_armor->max_hp_bonus - old_hp_bonus;
            int mp_difference = new_armor->max_mp_bonus - old_mp_bonus;
            g->player.max_hp += hp_difference;
            g->player.hp += hp_difference;
            g->player.max_mp += mp_difference;
            g->player.mp += mp_difference;
        }
    }

    // Version 40 makes the reserved off-hand index playable. Older saves
    // could contain a stale value there, but never applied an off-hand bonus.
    if (save_version < 40) {
        g->equipped_off_hand = -1;
    }

    // Version 42 makes entry-level Leather Armor usable by every class.
    if (save_version < 42) {
        for (int i = 0; i < g->inventory_count; i++) {
            if (strcmp(g->inventory[i].name, "Leather Armor") == 0) {
                g->inventory[i].class_mask = ITEM_CLASS_ALL;
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            if (strcmp(g->floor_items[i].item.name, "Leather Armor") == 0) {
                g->floor_items[i].item.class_mask = ITEM_CLASS_ALL;
            }
        }
    }

    // Version 46 moves the harbor to the southeast edge of the town green.
    if (save_version < 46 && g->location == LOCATION_TOWN) {
        map_clear_town_harbor(&g->map);
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x >= TOWN_HARBOR_X &&
                item->x < TOWN_W - 1 && item->y >= TOWN_HARBOR_Y &&
                item->y < TOWN_H - 1) {
                // Move covered loot to the cleared former building lot.
                item->x = 28 + item->x - TOWN_HARBOR_X;
                item->y = 16 + item->y - TOWN_HARBOR_Y;
                item->underlying_tile = TILE_TOWN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        if (g->player.x >= TOWN_HARBOR_X && g->player.x < TOWN_W - 1 &&
            g->player.y >= TOWN_HARBOR_Y && g->player.y < TOWN_H - 1) {
            g->player.x = 28 + g->player.x - TOWN_HARBOR_X;
            g->player.y = 16 + g->player.y - TOWN_HARBOR_Y;
        }
    }

    // Version 47 extends cobblestone lanes to the town entrances.
    if (save_version < 47 && g->location == LOCATION_TOWN) {
        int path_x[2] = {9, 30};
        for (int path = 0; path < 2; path++) {
            int occupied = 0;
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == path_x[path] &&
                    item->y == 11) {
                    item->underlying_tile = TILE_TOWN_PATH;
                    occupied = 1;
                }
            }
            if (!occupied) {
                g->map.tiles[11][path_x[path]] = TILE_TOWN_PATH;
            }
        }
        for (int y = 13; y <= 21; y++) {
            int occupied = 0;
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == 12 && item->y == y) {
                    item->underlying_tile = TILE_TOWN_PATH;
                    occupied = 1;
                }
            }
            if (!occupied) {
                g->map.tiles[y][12] = TILE_TOWN_PATH;
            }
        }
        for (int x = 8; x < 12; x++) {
            int occupied = 0;
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == x && item->y == 21) {
                    item->underlying_tile = TILE_TOWN_PATH;
                    occupied = 1;
                }
            }
            if (!occupied) {
                g->map.tiles[21][x] = TILE_TOWN_PATH;
            }
        }
    }

    // Version 48 removes the redundant Coast sluice wall and nearby switch.
    if (save_version < 48) {
        if (g->location == LOCATION_COAST &&
            map_remove_coast_sluice(&g->map)) {
            Room *room = &g->map.rooms[g->map.room_count / 2];
            int cx;
            int cy;
            map_room_center(room, &cx, &cy);
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active &&
                    ((item->x == cx && item->y >= room->y &&
                    item->y < room->y + room->h) ||
                    (item->x == cx + 2 && item->y == cy))) {
                    item->underlying_tile = TILE_COAST_FLOOR;
                    g->map.tiles[item->y][item->x] = TILE_ITEM;
                }
            }
        }
        for (int i = 0; i < MAX_REGION_DEPTH; i++) {
            if (g->coast_cache[i].valid) {
                map_remove_coast_sluice(&g->coast_cache[i].map);
            }
        }
    }

    // Version 61 surrounds the Castle of No Return with a moat. Rebuild
    // legacy Town 3 maps and move the player and loot off the new water.
    if (save_version < 61 && g->location == LOCATION_CASTLE) {
        int spawn_x;
        int spawn_y;
        map_generate_castle(&g->map, &spawn_x, &spawn_y);
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active) {
                move_ashore_from_town3_moat(&item->x, &item->y);
                item->underlying_tile = g->map.tiles[item->y][item->x];
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active) {
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        move_ashore_from_town3_moat(&g->player.x, &g->player.y);
    }

    // Version 62 moves the Tavern east of the Blacksmith, facing the square.
    // Clear its former south-west lot and move anything on the new lot onto
    // the square in front of it.
    if (save_version < 62 && g->location == LOCATION_TOWN) {
        for (int y = 16; y <= 20; y++) {
            for (int x = 5; x <= 11; x++) {
                g->map.tiles[y][x] = TILE_TOWN_FLOOR;
            }
        }
        clear_new_town_lot(g, TOWN_TAVERN_X, TOWN_TAVERN_Y, TOWN_TAVERN_W, TOWN_TAVERN_H);
        map_place_town_tavern(&g->map);
    }

    // Version 63 removes the old Tavern walkway loop and widens the town
    // square to the outer walls of the Blacksmith and Alchemist.
    if (save_version < 63 && g->location == LOCATION_TOWN) {
        for (int y = 13; y <= 21; y++) {
            retile_town_ground(g, 4, y, TILE_TOWN_FLOOR);
            retile_town_ground(g, 12, y, TILE_TOWN_FLOOR);
        }
        for (int x = 5; x < 12; x++) {
            retile_town_ground(g, x, 21, TILE_TOWN_FLOOR);
        }
        for (int y = 11; y <= 14; y++) {
            for (int x = TOWN_BLACKSMITH_X; x <= TOWN_ALCHEMIST_X + 4; x++) {
                retile_town_ground(g, x, y, TILE_TOWN_PATH);
            }
        }
    }

    // Version 64 moves the Town 2 Inn east of the Healer, facing the square.
    // Clear its former south-west lot and lane, and move anything on the new
    // lot onto the square in front of it.
    if (save_version < 64 && g->location == LOCATION_TOWN2) {
        for (int y = 16; y <= 20; y++) {
            for (int x = 5; x <= 11; x++) {
                g->map.tiles[y][x] = TILE_TOWN_FLOOR;
            }
        }
        for (int y = 14; y <= 21; y++) {
            retile_town_ground(g, 12, y, TILE_TOWN_FLOOR);
        }
        for (int x = 8; x < 12; x++) {
            retile_town_ground(g, x, 21, TILE_TOWN_FLOOR);
        }
        clear_new_town_lot(g, TOWN_INN_X, TOWN_INN_Y, TOWN_INN_W, TOWN_INN_H);
        map_place_town_inn(&g->map);
    }

    // Version 66 makes the Fireball scroll Mage-only.
    if (save_version < 66) {
        for (int i = 0; i < g->inventory_count; i++) {
            if (strcmp(g->inventory[i].name, "Scroll: Fireball") == 0) {
                g->inventory[i].class_mask = ITEM_CLASS_MAGE;
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            if (strcmp(g->floor_items[i].item.name, "Scroll: Fireball") == 0) {
                g->floor_items[i].item.class_mask = ITEM_CLASS_MAGE;
            }
        }
    }

    // Version 68 retires the dungeon's portcullis shortcut. Its gates and floor
    // switches become plain floor, including under items dropped on a switch.
    if (save_version < 68) {
        if (g->location == LOCATION_DUNGEON) {
            map_remove_dungeon_gates(&g->map);
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->underlying_tile == TILE_DUNGEON_SWITCH_OFF ||
                    item->underlying_tile == TILE_DUNGEON_SWITCH_ON) {
                    item->underlying_tile = TILE_FLOOR;
                }
            }
        }
        for (int i = 0; i < MAX_REGION_DEPTH; i++) {
            if (g->level_cache[i].valid) {
                map_remove_dungeon_gates(&g->level_cache[i].map);
            }
        }
    }

    // Version 69 moves the swamp to Town 2's north gate and the Royal Guards to
    // Town 3, and turns the Crown Road into Crown Road East between Town 3 and
    // the new castle grounds. Older Town 2 maps lose their guards and south
    // gate, older Town 3 maps (the old castle town) become the new Town 3, and
    // the road, its creatures and its saved progress turn to run west to east.
    if (save_version < 69) {
        if (g->location == LOCATION_TOWN2) {
            for (int y = 0; y < TOWN_H; y++) {
                for (int x = 0; x < TOWN_W; x++) {
                    if (g->map.tiles[y][x] == TILE_NPC_ROYAL_GUARD) {
                        g->map.tiles[y][x] = TILE_TOWN_FLOOR;
                    }
                }
            }
            if (g->map.tiles[TOWN_H - 1][20] == TILE_TOWN_EXIT) {
                g->map.tiles[TOWN_H - 1][20] = TILE_WALL;
            }
        } else if (g->location == LOCATION_TOWN3) {
            // The moat and castle walls become open ground, so the player and
            // loot stay where they stand unless that tile is no longer open.
            int spawn_x;
            int spawn_y;
            map_generate_town3(&g->map, &spawn_x, &spawn_y);
            if (!map_is_walkable(&g->map, g->player.x, g->player.y)) {
                g->player.x = spawn_x;
                g->player.y = spawn_y;
            }
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                TileType ground = g->map.tiles[item->y][item->x];
                if (!item->active) {
                    continue;
                }
                if (ground == TILE_TOWN_FLOOR || ground == TILE_TOWN_PATH) {
                    item->underlying_tile = ground;
                    g->map.tiles[item->y][item->x] = TILE_ITEM;
                } else {
                    item->active = 0;
                }
            }
        } else if (g->location == LOCATION_CROWNROAD) {
            // Road loot is cleared whenever the road is left, so none is kept.
            map_generate_crownroad(&g->map);
            rotate_crownroad_position(&g->player.x, &g->player.y);
            for (int i = 0; i < g->enemy_count; i++) {
                rotate_crownroad_position(&g->enemies[i].x, &g->enemies[i].y);
            }
            g->floor_item_count = 0;
        }
        if (g->crownroad_cache.valid) {
            for (int i = 0; i < g->crownroad_cache.enemy_count; i++) {
                Enemy *e = &g->crownroad_cache.enemies[i];
                rotate_crownroad_position(&e->x, &e->y);
            }
        }
    }

    // Version 70 moves Rosemoor's Apothecary and Adventurer's Guild to the
    // north edge of the central square. Clear their former lots and lanes,
    // preserve loot and move anything covered by the new footprints.
    if (save_version < 70 && g->location == LOCATION_TOWN3) {
        clear_new_town_lot(g, 3, 5, 7, 5);
        clear_new_town_lot(g, 31, 8, 5, 4);
        clear_town3_old_lot(g, 3, 5, 7, 5);
        clear_town3_old_lot(g, 31, 8, 5, 4);
        for (int y = 10; y < CASTLE_ROAD_Y; y++) {
            retile_town_ground(g, 6, y, TILE_TOWN_FLOOR);
        }
        for (int x = 29; x <= 35; x++) {
            retile_town_ground(g, x, 12, TILE_TOWN_FLOOR);
        }
        map_place_town3_guild(&g->map);
        map_place_town_apothecary(&g->map);
        clear_new_town_lot(g, TOWN_GUILD_X, TOWN_GUILD_Y,
            TOWN_GUILD_W, TOWN_GUILD_H);
        clear_new_town_lot(g, TOWN_APOTHECARY_X, TOWN_APOTHECARY_Y,
            TOWN_APOTHECARY_W, TOWN_APOTHECARY_H);
    }

    if ((old_forest && !migrate_region_routes(g, LOCATION_FOREST)) ||
        (old_swamp && !migrate_region_routes(g, LOCATION_SWAMP)) ||
        (old_mountains && !migrate_region_routes(g, LOCATION_MOUNTAINS)) ||
        (save_version < 84 && !migrate_shortened_stages(g, LOCATION_COAST)) ||
        (save_version < 85 && !migrate_shortened_stages(g, LOCATION_DUNGEON))) {
        cJSON_Delete(root);
        return 0;
    }
    if (save_version < 85 && cJSON_GetArraySize(temple_cache) == 4) {
        migrate_expanded_finale(g, LOCATION_TEMPLE, 4);
    }
    if (save_version < 85 && cJSON_GetArraySize(labyrinth_cache) == 3) {
        migrate_expanded_finale(g, LOCATION_LABYRINTH, 3);
    }

    if (g->location == LOCATION_DUNGEON &&
        (g->level > DUNGEON_DEPTH || (save_version < 4 && g->level == DUNGEON_DEPTH))) {
        g->level = DUNGEON_DEPTH;
        g->floor_item_count = 0;
        g->level_cache[DUNGEON_DEPTH - 1].valid = 0;
        map_generate(&g->map, g->level);
        enemies_spawn(g);
        g->level_cleared = 0;
        g->player.x = g->map.stairs_up_x;
        g->player.y = g->map.stairs_up_y;
    }

    if (g->location == LOCATION_MOUNTAINS) {
        hide_legacy_fort_plate(&g->map);
    }
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        if (g->mountain_cache[i].valid) {
            hide_legacy_fort_plate(&g->mountain_cache[i].map);
        }
    }

    if (g->location == LOCATION_FOREST) {
        game_repair_forest_enemy_positions(&g->map, g->enemies,
            g->enemy_count, g->player.x, g->player.y);
    }
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        if (g->forest_cache[i].valid) {
            game_repair_forest_enemy_positions(&g->forest_cache[i].map,
                g->forest_cache[i].enemies, g->forest_cache[i].enemy_count,
                -1, -1);
        }
    }

    if (save_version < 155) {
        migrate_shared_town_gates(g);
    }
    if (g->location == LOCATION_TOWN) {
        map_set_town2_road(&g->map,
            g->defeated_bosses & (1 << LOCATION_FOREST));
        map_set_town4_road(&g->map,
            g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
    }
    if (g->location == LOCATION_TOWN2) {
        map_place_town2_center(&g->map);
        map_remove_town_labyrinth(&g->map);
        map_set_town3_road(&g->map, g->defeated_bosses & (1 << LOCATION_SWAMP));
        map_set_stillbury_forest_road(&g->map, g->defeated_bosses & (1 << LOCATION_FOREST));
    }

    if (g->location == LOCATION_TOWN3) {
        map_place_town3_ashen_gate(&g->map);
        map_place_town3_frost_gate(&g->map);
        map_set_rosemoor_swamp_road(&g->map, g->defeated_bosses & (1 << LOCATION_SWAMP));
        map_place_town3_guild(&g->map);
        map_place_town_apothecary(&g->map);
        if (!map_is_walkable(&g->map, g->player.x, g->player.y)) {
            g->player.x = TOWN_GUILD_DOOR_X;
            g->player.y = TOWN_GUILD_DOOR_Y + 1;
        }
        map_place_town3_guards(&g->map, g->player.x, g->player.y);
    }

    if (g->location == LOCATION_TOWN4) {
        map_place_town4_region_gates(&g->map);
        map_set_ridgeshire_mountain_road(&g->map, g->defeated_bosses & (1 << LOCATION_MOUNTAINS));
        // Extend saved town paving, including terrain covered by dropped items.
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->underlying_tile == TILE_TOWN_FLOOR &&
                in_lot(item->x, item->y, TOWN4_SQUARE_X, TOWN4_SQUARE_Y, TOWN4_SQUARE_W, TOWN4_SQUARE_H)) {
                item->underlying_tile = TILE_TOWN_PATH;
            }
        }
        map_place_town4_workshop(&g->map);
        map_place_town4_hall(&g->map);
        if (!map_is_walkable(&g->map, g->player.x, g->player.y)) {
            if (in_lot(g->player.x, g->player.y, TOWN4_HALL_X, TOWN4_HALL_Y, TOWN4_HALL_W, TOWN4_HALL_H)) {
                g->player.x = TOWN4_HALL_DOOR_X;
                g->player.y = TOWN4_HALL_DOOR_Y + 1;
            } else {
                g->player.x = TOWN4_WORKSHOP_DOOR_X;
                g->player.y = TOWN4_WORKSHOP_DOOR_Y + 1;
            }
        }
        map_place_town4_guards(&g->map, g->player.x, g->player.y);
    }

    if (save_version < 73 && g->location == LOCATION_KING_ROAD_WEST) {
        for (int y = 0; y < CROWNROAD_H; y++) {
            for (int x = 0; x < CROWNROAD_W; x++) {
                if (g->map.tiles[y][x] == TILE_NPC_ROYAL_GUARD) {
                    g->map.tiles[y][x] = TILE_TOWN_FLOOR;
                }
            }
        }
    }

    if (save_version < 89) {
        migrate_removed_royal_seals(g);
    }
    if (save_version < 90) {
        migrate_narrow_shortcuts(g);
    }
    if (save_version < 91 && g->location == LOCATION_TAVERN) {
        if (g->player.x == BRENNA_X && g->player.y == BRENNA_Y) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == BRENNA_X && item->y == BRENNA_Y) {
                item->y++;
                item->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        map_place_tavern_brenna(&g->map);
    }
    if (save_version < 94 && g->location == LOCATION_GUILD) {
        if (g->player.x == GUILD_ORIN_X && g->player.y == GUILD_ORIN_Y) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == GUILD_ORIN_X && item->y == GUILD_ORIN_Y) {
                item->y++;
                item->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->map.tiles[GUILD_ORIN_Y][GUILD_ORIN_X] = TILE_NPC_ORIN;
    }
    if (save_version < 95 && g->location == LOCATION_TAVERN) {
        if (g->player.x == 18 && g->player.y == 18) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == 18 && item->y == 18) {
                item->y++;
                item->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->map.tiles[18][18] = 206;
    }
    // Version 96 moves Mara to the Town Hall without resetting her quest or coast caches.
    if (save_version < 96 && g->location == LOCATION_TAVERN) {
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (g->map.tiles[y][x] == TILE_NPC_MARA) {
                    g->map.tiles[y][x] = TILE_TAVERN_FLOOR;
                }
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].underlying_tile == TILE_NPC_MARA) {
                g->floor_items[i].underlying_tile = TILE_TAVERN_FLOOR;
            }
        }
    }
    if (save_version < 96 && g->location == LOCATION_TOWN_HALL) {
        if (g->player.x == HALL_MARA_X && g->player.y == HALL_MARA_Y) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == HALL_MARA_X && item->y == HALL_MARA_Y) {
                item->y++;
                item->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->map.tiles[HALL_MARA_Y][HALL_MARA_X] = TILE_NPC_MARA;
    }
    if (save_version < 97 && g->location == LOCATION_TOWN_HALL) {
        if (g->player.x == HALL_OSWIN_X && g->player.y == HALL_OSWIN_Y) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == HALL_OSWIN_X && item->y == HALL_OSWIN_Y) {
                item->y++;
                item->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->map.tiles[HALL_OSWIN_Y][HALL_OSWIN_X] = TILE_NPC_OSWIN;
    }
    if (save_version < 98 && g->location == LOCATION_TAVERN) {
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (g->map.tiles[y][x] == TILE_NPC_ALDER) {
                    g->map.tiles[y][x] = TILE_TAVERN_FLOOR;
                }
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].underlying_tile == TILE_NPC_ALDER) {
                g->floor_items[i].underlying_tile = TILE_TAVERN_FLOOR;
            }
        }
    }
    if (save_version < 98 && g->location == LOCATION_INN) {
        if (g->player.x == ALDER_INN_X && g->player.y == ALDER_INN_Y) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == ALDER_INN_X && item->y == ALDER_INN_Y) {
                item->y++;
                item->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->map.tiles[ALDER_INN_Y][ALDER_INN_X] = TILE_NPC_ALDER;
    }
    if (save_version < 99) {
        // The third rescue keeps its completion bit but moves across the boss to stage 5.
        if (g->forest_cache[2].valid) {
            remove_legacy_wardens(&g->forest_cache[2].map);
        }
        if (g->location == LOCATION_FOREST && g->level == 3) {
            remove_legacy_wardens(&g->map);
            for (int i = 0; i < g->floor_item_count; i++) {
                if (g->floor_items[i].underlying_tile == TILE_FOREST_WARDEN) {
                    g->floor_items[i].underlying_tile = TILE_FOREST_FLOOR;
                }
            }
        }
        if (g->portal_location == LOCATION_FOREST && g->portal_level == 3 && g->portal_origin_tile == TILE_FOREST_WARDEN) {
            g->portal_origin_tile = TILE_FOREST_FLOOR;
        }
    }
    if (save_version < 100) {
        // Preserve actors and collected fragments; pending Shaman fragments now belong to stage 5.
        for (int i = 0; i < 7; i++) {
            LevelCache *cache = &g->mountain_cache[i];
            if (i != 4 && cache->valid) {
                remove_misplaced_shaman(cache->enemies, cache->enemy_count);
            }
        }
        if (g->location == LOCATION_MOUNTAINS && g->level != 5) {
            remove_misplaced_shaman(g->enemies, g->enemy_count);
        }
    }
    if (g->castle_prompt) {
        castle_request(g, g->castle_prompt == 2);
    }
    if (save_version < 101) {
        jail_migrate_castle(g);
    }
    if (save_version < 102 && g->location == LOCATION_TOWN4) {
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (g->map.tiles[y][x] == TILE_NPC_DRAGON_SEEKER) {
                    g->map.tiles[y][x] = TILE_TOWN_FLOOR;
                }
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].underlying_tile == TILE_NPC_DRAGON_SEEKER) {
                g->floor_items[i].underlying_tile = TILE_TOWN_FLOOR;
            }
        }
        if (strcmp(g->dialogue_speaker, "Ilya") == 0) {
            g->dialogue_active = 0;
        }
    }
    if (save_version < 102 && g->location == LOCATION_TAVERN) {
        if (g->player.x == ILYA_TAVERN_X && g->player.y == ILYA_TAVERN_Y) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == ILYA_TAVERN_X && item->y == ILYA_TAVERN_Y) {
                item->y++;
                item->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->map.tiles[ILYA_TAVERN_Y][ILYA_TAVERN_X] = TILE_NPC_DRAGON_SEEKER;
    }
    // Version 104 returns Elowen to Oakhaven without resetting dungeon progress.
    if (save_version < 104 && g->location == LOCATION_INN) {
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (g->map.tiles[y][x] == TILE_NPC_ELOWEN) {
                    g->map.tiles[y][x] = TILE_TAVERN_FLOOR;
                }
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].underlying_tile == TILE_NPC_ELOWEN) {
                g->floor_items[i].underlying_tile = TILE_TAVERN_FLOOR;
            }
        }
        if (strcmp(g->dialogue_speaker, "Elowen") == 0) {
            g->dialogue_active = 0;
        }
    }
    if (save_version < 104 && g->location == LOCATION_TAVERN) {
        if (g->player.x == ELOWEN_TAVERN_X && g->player.y == ELOWEN_TAVERN_Y) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == ELOWEN_TAVERN_X && item->y == ELOWEN_TAVERN_Y) {
                item->y++;
                item->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        map_place_tavern_elowen(&g->map);
    }
    // Version 105 moves Mara to Oakhaven and Liora to Stillbury without resetting quests.
    if (save_version < 105 && (g->location == LOCATION_TAVERN || g->location == LOCATION_TOWN_HALL)) {
        TileType removed = g->location == LOCATION_TAVERN ? 206 : TILE_NPC_MARA;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (g->map.tiles[y][x] == removed) {
                    g->map.tiles[y][x] = TILE_TAVERN_FLOOR;
                }
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].underlying_tile == removed) {
                g->floor_items[i].underlying_tile = TILE_TAVERN_FLOOR;
            }
        }
        if ((removed == 206 && strcmp(g->dialogue_speaker, "Botanist Liora") == 0) ||
            (removed == TILE_NPC_MARA && strcmp(g->dialogue_speaker, "Mara") == 0)) {
            g->dialogue_active = 0;
        }
    }
    if (save_version < 105 && (g->location == LOCATION_TAVERN || g->location == LOCATION_INN)) {
        int x = g->location == LOCATION_TAVERN ? MARA_TAVERN_X : 18;
        int y = g->location == LOCATION_TAVERN ? MARA_TAVERN_Y : 18;
        if (g->player.x == x && g->player.y == y) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == x && item->y == y) {
                item->y++;
                item->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->map.tiles[y][x] = g->location == LOCATION_TAVERN ? TILE_NPC_MARA : 206;
    }
    // Version 107 checks every dungeon floor, including saved side rooms.
    if (save_version < 107) {
        if (g->location == LOCATION_DUNGEON) {
            map_ensure_dungeon_connectivity(&g->map, g->level);
        }
        for (int level = 1; level <= DUNGEON_DEPTH; level++) {
            LevelCache *cache = &g->level_cache[level - 1];
            if (cache->valid) {
                map_ensure_dungeon_connectivity(&cache->map, level);
            }
        }
    }
    if (save_version < 108) {
        if (g->location == LOCATION_TOWN) {
            if (g->player.x == TOWN_BRAM_X && g->player.y == TOWN_BRAM_Y) {
                g->player.y++;
            }
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == TOWN_BRAM_X && item->y == TOWN_BRAM_Y) {
                    item->y++;
                    item->underlying_tile = TILE_TOWN_PATH;
                    g->map.tiles[item->y][item->x] = TILE_ITEM;
                }
            }
            g->map.tiles[TOWN_BRAM_Y][TOWN_BRAM_X] = TILE_NPC_BRAM;
        }
        migrate_lich_minions(g);
    }
    if (save_version < 109) {
        int defeated = g->defeated_bosses & (1 << LOCATION_DUNGEON);
        TileType stair = defeated ? TILE_DUNGEON_STAIRS_RETURN : TILE_DUNGEON_STAIRS_SEALED;
        if (g->location == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH) {
            migrate_dungeon_return_stairs(&g->map, defeated);
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == g->map.stairs_down_x &&
                    item->y == g->map.stairs_down_y) {
                    item->underlying_tile = stair;
                }
            }
        }
        LevelCache *cache = &g->level_cache[DUNGEON_DEPTH - 1];
        if (cache->valid) {
            migrate_dungeon_return_stairs(&cache->map, defeated);
        }
        if (g->portal_active && g->portal_location == LOCATION_DUNGEON &&
            g->portal_level == DUNGEON_DEPTH &&
            (g->portal_origin_tile == TILE_STAIRS_DOWN || g->portal_origin_tile == TILE_RETURN_EXIT)) {
            g->portal_origin_tile = stair;
        }
    }
    // Version 140 lowers Cryptblade attack again; only pre-110 saves also
    // retire Goblin greatswords. Include inventory, dropped and cached copies.
    if (save_version < 140) {
        migrate_rebalanced_weapons(g, save_version < 110);
    }
    repair_floor_item_underlays(g);
    if (save_version < 111) {
        town_life_migrate(g);
    }
    if (save_version < 112) {
        migrate_goblin_shields(g);
        if (g->location == LOCATION_TAVERN) {
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    if (g->map.tiles[y][x] == TILE_NPC_BRENNA) {
                        g->map.tiles[y][x] = TILE_TAVERN_FLOOR;
                    }
                }
            }
            for (int i = 0; i < g->floor_item_count; i++) {
                if (g->floor_items[i].underlying_tile == TILE_NPC_BRENNA) {
                    g->floor_items[i].underlying_tile = TILE_TAVERN_FLOOR;
                }
            }
            if (strcmp(g->dialogue_speaker, "Quartermaster Brenna") == 0) {
                g->dialogue_active = 0;
            }
        }
        if (g->location == LOCATION_INN) {
            if (g->player.x == BRENNA_INN_X && g->player.y == BRENNA_INN_Y) {
                g->player.y++;
            }
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == BRENNA_INN_X && item->y == BRENNA_INN_Y) {
                    item->y++;
                    item->underlying_tile = TILE_TAVERN_FLOOR;
                    g->map.tiles[item->y][item->x] = TILE_ITEM;
                }
            }
            g->map.tiles[BRENNA_INN_Y][BRENNA_INN_X] = TILE_NPC_BRENNA;
        }
    }
    if (save_version < 113) {
        migrate_near_side_quests(g);
    }
    if (save_version < 114) {
        migrate_quest_giver_locations(g);
    }
    if (save_version < 117) {
        migrate_kraken_cycle(g->enemies, g->enemy_count, save_version);
        for (int i = 0; i < FROSTFELL_DEPTH; i++) {
            if (g->frostfell_cache[i].valid) {
                migrate_kraken_cycle(g->frostfell_cache[i].enemies, g->frostfell_cache[i].enemy_count, save_version);
            }
        }
    }
    if (save_version < 118) {
        migrate_longer_crownroads(g, save_version);
        jail_migrate_tunnel(g);
    }
    if (save_version < 119) {
        castle_migrate_layout(g);
    }
    if (save_version < 120 && !castle_migrate_encounters(g)) {
        cJSON_Delete(root);
        return 0;
    }
    if (save_version < 121) {
        game_migrate_crownroad_density(g);
    }
    if (save_version < 122 && g->location == LOCATION_TOWN_HALL) {
        if (g->player.x == HALL_VEYRA_X && g->player.y == HALL_VEYRA_Y) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == HALL_VEYRA_X && item->y == HALL_VEYRA_Y) {
                item->y++;
                item->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->map.tiles[HALL_VEYRA_Y][HALL_VEYRA_X] = TILE_NPC_VEYRA;
    }
    if (save_version < 123 && g->location == LOCATION_GUILD) {
        if (g->player.x == GUILD_SELENE_X && g->player.y == GUILD_SELENE_Y) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == GUILD_SELENE_X && item->y == GUILD_SELENE_Y) {
                item->y++;
                item->underlying_tile = TILE_TAVERN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->map.tiles[GUILD_SELENE_Y][GUILD_SELENE_X] = TILE_NPC_SELENE;
    }
    if (save_version < 171 && !migrate_mountain_climb(g)) {
        cJSON_Delete(root);
        return 0;
    }
    if (save_version < 172 && !migrate_forest_adventure(g)) {
        cJSON_Delete(root);
        return 0;
    }
    if (save_version < 173 && !migrate_swamp_adventure(g)) {
        cJSON_Delete(root);
        return 0;
    }
    game_hide_portal_destination(g);
    if (save_version < 124) {
        lich_migrate(g);
    }
    if (save_version < 125) {
        necromancer_migrate(g);
    }
    if (save_version < 126) {
        goblin_king_migrate(g);
    }
    if (save_version < 127) {
        drowned_queen_migrate(g);
    }
    if (save_version < 128) {
        swamp_demon_migrate(g);
    }
    if (save_version < 129) {
        minotaur_migrate(g);
    }
    if (save_version < 130) {
        grave_marshal_migrate(g);
    }
    if (save_version < 131) {
        desert_pharaoh_migrate(g);
    }

    if (save_version < 133) {
        cinder_lord_migrate(g);
    }
    if (save_version < 134) {
        prism_sovereign_migrate(g);
    }
    if (save_version < 135) {
        sun_guardian_migrate(g);
    }
    if (save_version < 136) {
        red_dragon_migrate(g);
    }
    if (save_version < 137) {
        castle_migrate_castellan(g);
    }
    if (save_version < 138) {
        royal_arcanist_migrate(g);
    }
    if (save_version < 139) {
        lord_veyr_migrate(g);
    }
    if (save_version < 141) {
        drowned_queen_remove_islands(g);
    }
    // Version 143 opens the old coast's corridors into sandbanks and flats.
    // Expand in place so cached deaths, beacons, loot and portals stay intact.
    if (save_version < 143) {
        if (g->location == LOCATION_COAST) {
            map_shape_coast(&g->map, g->level);
        }
        for (int i = 0; i < COAST_DEPTH; i++) {
            if (g->coast_cache[i].valid) {
                map_shape_coast(&g->coast_cache[i].map, i + 1);
            }
        }
    }
    // Version 145 appends diverted-valve and stable-crossing tile IDs. Existing
    // version 144 maps use the new valve behavior directly; no terrain reset.
    if (save_version < 144) {
        regional_terrain_migrate(g);
    }
    if (save_version < 146) {
        landscape_migrate(g);
    }
    if (save_version < 147) {
        sunscar_migrate(g);
    }
    if (save_version < 148) {
        wilds_migrate(g);
    }
    // Add Aldric to saved Oakhaven maps without losing the player or dropped loot.
    if (save_version < 149 && g->location == LOCATION_TOWN) {
        if (g->player.x == TOWN_ALDRIC_X && g->player.y == TOWN_ALDRIC_Y) {
            g->player.y++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == TOWN_ALDRIC_X && item->y == TOWN_ALDRIC_Y) {
                item->y++;
                item->underlying_tile = TILE_TOWN_PATH;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->map.tiles[TOWN_ALDRIC_Y][TOWN_ALDRIC_X] = TILE_NPC_ALDRIC;
    }
    // Tessa replaces an impassable bench, preserving player and loot positions.
    if (save_version < 150 && g->location == LOCATION_WORKSHOP) {
        g->map.tiles[WORKSHOP_FLETCHER_Y][WORKSHOP_FLETCHER_X] = TILE_NPC_FLETCHER;
    }
    if (save_version < 151) {
        for (int i = 0; i < g->inventory_count; i++) {
            migrate_sunwarden_mantle(&g->inventory[i], g->equipped_armor == i ? &g->player : NULL);
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            migrate_sunwarden_mantle(&g->floor_items[i].item, NULL);
        }
        for (int level = 0; level < CASTLE_DEPTH; level++) {
            for (int i = 0; i < g->castle_loot_count[level]; i++) {
                migrate_sunwarden_mantle(&g->castle_loot[level][i].item, NULL);
            }
        }
    }
    if (save_version < 152) {
        migrate_rook_location(g);
    }
    if (save_version < 153) {
        migrate_daggers(g);
    }
    if (save_version < 154) {
        // Add the fork without replacing exploration, loot, or player progress.
        if (g->location == LOCATION_FOREST_ROAD) {
            map_add_shortcut_branch(&g->map, TILE_FOREST_SHORTCUT);
        } else if (g->location == LOCATION_SWAMP_ROAD) {
            map_add_shortcut_branch(&g->map, TILE_SWAMP_SHORTCUT);
        } else if (g->location == LOCATION_HIGH_PASS) {
            map_add_shortcut_branch(&g->map, TILE_MOUNTAIN_SHORTCUT);
        }
    }
    if (save_version < 158) {
        migrate_forest_entry_direction(g);
    }
    if (save_version < 162) {
        approach_refresh_dungeon(g);
    }
    if (save_version < 163) {
        approach_relocate_ridgeshire(g);
        migrate_ridgeshire_portal(g);
    }
    if (save_version < 165) {
        approach_migrate_roads(g);
        approach_refresh_fork(g);
        if (strcmp(g->dialogue_speaker, "Choose route") == 0) {
            g->dialogue_active = 0;
            g->dialogue_speaker[0] = '\0';
            g->dialogue_text[0] = '\0';
        }
    }
    if (save_version < 169) {
        approach_refresh_dungeon(g);
    }
    if (save_version < 170 && g->location == LOCATION_DAWNMERE) {
        for (int y = 6; y < 11; y++) {
            for (int x = 26; x < 33; x++) {
                if (g->map.tiles[y][x] == TILE_LOCAL_BUILDING) {
                    g->map.tiles[y][x] = TILE_TOWN_FLOOR;
                }
            }
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x >= 26 && item->x < 33 && item->y >= 6 && item->y < 11 &&
                item->underlying_tile == TILE_LOCAL_BUILDING) {
                item->underlying_tile = TILE_TOWN_FLOOR;
            }
        }
    }
    // Saved corridor approaches adopt their area's entrance landmark.
    if (save_version < 175) {
        approach_refresh_entrance(g);
    }
    if (save_version < 176) {
        approach_refresh_forest(g);
    }
    if (save_version < 177 && approach_route(g) && approach_route(g)->area == LOCATION_COAST) {
        approach_refresh_entrance(g);
    }
    if (save_version < 179 && g->location == LOCATION_DAWNMERE) {
        for (int y = 1; y < TOWN_H - 1; y++) {
            for (int x = 1; x < TOWN_W - 1; x++) {
                retile_town_ground(g, x, y, map_dawnmere_ground(x, y));
            }
        }
    }
    if (save_version < 181) {
        migrate_island_temple(g);
    }
    if (save_version < 182) {
        if (town_road_index(g->location) >= 0) {
            g->last_town_visited = g->location;
        } else if (g->location == LOCATION_DUNGEON) {
            g->last_town_visited = g->dungeon_entry_town;
        } else if (g->location == LOCATION_FOREST) {
            g->last_town_visited = g->forest_entry_town;
        } else if (g->location == LOCATION_MOUNTAINS) {
            g->last_town_visited = g->mountain_entry_town;
        } else if (g->location == LOCATION_SWAMP) {
            g->last_town_visited = g->swamp_entry_town;
        } else if (g->location == LOCATION_INN || g->location == LOCATION_HEALER || g->location == LOCATION_WITCH ||
            g->location == LOCATION_DESERT || g->location == LOCATION_GLASSDEEP) {
            g->last_town_visited = LOCATION_TOWN2;
        } else if (g->location == LOCATION_GUILD || g->location == LOCATION_FROSTFELL) {
            g->last_town_visited = LOCATION_TOWN3;
        } else if (g->location == LOCATION_TOWN_HALL || g->location == LOCATION_WORKSHOP ||
            g->location == LOCATION_ASHEN || g->location == LOCATION_DRAGONSPINE) {
            g->last_town_visited = LOCATION_TOWN4;
        } else if (town_life_is_interior(g->location)) {
            g->last_town_visited = town_life_building(g->location)->town;
        } else if (g->location == LOCATION_APPROACH && approach_route(g) && town_road_index(approach_route(g)->town) >= 0) {
            g->last_town_visited = approach_route(g)->town;
        } else if (g->location == LOCATION_TOWN_ROAD && town_road_index(g->road_town) >= 0) {
            g->last_town_visited = g->road_town;
        }
        if (g->location == LOCATION_TOWN) {
            for (int x = 18; x <= 22; x++) {
                g->map.tiles[0][x] = TILE_TOWN_EXIT;
            }
        }
        if (northern_approach(g)) {
            northern_refresh(g);
            g->player.x = NORTH_CAT_X;
            g->player.y = NORTH_CAT_Y + 1;
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active) {
                    if (!map_is_walkable(&g->map, item->x, item->y)) {
                        item->x = NORTH_ROAD_X + i / 10;
                        item->y = NORTH_CAT_Y + 1 + i % 10;
                    }
                    item->underlying_tile = g->map.tiles[item->y][item->x];
                    g->map.tiles[item->y][item->x] = TILE_ITEM;
                }
            }
        }
    }
    if (g->location == LOCATION_ELDERWOOD && (g->level < 1 || g->level > ELDERWOOD_DEPTH ||
        g->player.x < 0 || g->player.x >= ELDERWOOD_W || g->player.y < 0 || g->player.y >= ELDERWOOD_H)) {
        cJSON_Delete(root);
        return 0;
    }
    // Recover saves containing a town interior or shortcut in a dungeon slot.
    game_repair_dungeon_maps(g);
    // Recess legacy floor shortcuts into terrain in active and cached boss maps.
    game_migrate_boss_shortcuts(g);
    game_refresh_quest_encounters(g);
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && g->location == LOCATION_TOWN2 &&
            (item->underlying_tile == TILE_LABYRINTH_ENTRANCE ||
            (item->underlying_tile == TILE_TOWN_PATH &&
            ((item->x == TOWN_LABYRINTH_X - 2 && item->y >= 14 && item->y <= TOWN_LABYRINTH_Y + 1) ||
            (item->y == TOWN_LABYRINTH_Y + 1 && item->x >= TOWN_LABYRINTH_X - 2 && item->x <= TOWN_LABYRINTH_X))))) {
            item->underlying_tile = TILE_TOWN_FLOOR;
        }
        if (save_version < 157 && item->active && item->underlying_tile == TILE_TOWN_FLOOR &&
            ((g->location == LOCATION_TOWN2 && item->x == TOWN3_ROAD_X && item->y == 11) ||
            (g->location == LOCATION_TOWN3 && item->x == ROSEMOOR_SWAMP_ROAD_X && item->y == 15))) {
            item->underlying_tile = TILE_TOWN_PATH;
        }
        if (item->active && g->location == LOCATION_TOWN3 &&
            item->x >= TOWN_GUILD_X && item->x < TOWN_GUILD_X + TOWN_GUILD_W &&
            item->y >= TOWN_GUILD_Y && item->y < TOWN_GUILD_Y + TOWN_GUILD_H &&
            !map_is_walkable(&g->map, item->x, item->y)) {
            item->x = TOWN_GUILD_DOOR_X;
            item->y = TOWN_GUILD_DOOR_Y + 1;
            item->underlying_tile = TILE_TOWN_PATH;
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
        if (item->active && g->location == LOCATION_TOWN3 &&
            item->x >= TOWN_APOTHECARY_X &&
            item->x < TOWN_APOTHECARY_X + TOWN_APOTHECARY_W &&
            item->y >= TOWN_APOTHECARY_Y &&
            item->y < TOWN_APOTHECARY_Y + TOWN_APOTHECARY_H &&
            !map_is_walkable(&g->map, item->x, item->y)) {
            item->x = TOWN_APOTHECARY_DOOR_X;
            item->y = TOWN_APOTHECARY_DOOR_Y + 1;
            item->underlying_tile = TILE_TOWN_PATH;
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
        if (item->active && g->location == LOCATION_TOWN4 &&
            in_lot(item->x, item->y, TOWN4_HALL_X, TOWN4_HALL_Y, TOWN4_HALL_W, TOWN4_HALL_H)) {
            item->x = TOWN4_HALL_DOOR_X;
            item->y = TOWN4_HALL_DOOR_Y + 1;
            item->underlying_tile = TILE_TOWN_PATH;
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
        if (item->active && g->location == LOCATION_TOWN4 &&
            item->x >= TOWN4_WORKSHOP_X &&
            item->x < TOWN4_WORKSHOP_X + TOWN4_WORKSHOP_W &&
            item->y >= TOWN4_WORKSHOP_Y &&
            item->y < TOWN4_WORKSHOP_Y + TOWN4_WORKSHOP_H &&
            !map_is_walkable(&g->map, item->x, item->y)) {
            item->x = TOWN4_WORKSHOP_DOOR_X;
            item->y = TOWN4_WORKSHOP_DOOR_Y + 1;
        }
        if (item->active && item->item.type == ITEM_GOLD &&
            (item->x < 0 || item->x >= MAP_W ||
            item->y < 0 || item->y >= MAP_H ||
            !map_is_walkable(&g->map, item->x, item->y))) {
            g->gold += item->item.value;
            g->score += item->item.value;
            item->active = 0;
        }
    }
    if (g->location == LOCATION_JAIL_ARMORY && (g->level != 1 || !g->jail_armory_initialized ||
        g->enemy_count != 3 || g->player.x < 0 || g->player.x >= JAIL_ROOM_W || g->player.y < 0 || g->player.y >= JAIL_ROOM_H)) {
        cJSON_Delete(root);
        return 0;
    }
    if (!load_retired_coast_loot(root, g, save_version)) {
        cJSON_Delete(root);
        return 0;
    }
    if (pirate_prototype) {
        migrate_removed_pirate_coast(g, pirate_return);
    }
    game_recover_retired_items(g);
    if (save_version < 191) {
        mangrove_migrate_town(g);
        coastal_migrate_travel(g);
        if (!mangrove_migrate_pilot(g)) {
            cJSON_Delete(root);
            return 0;
        }
    }
    // Version 192 restores Glassdeep to Stillbury. Route indices and expeditions remain stable.
    if (save_version == 191 && g->location == LOCATION_APPROACH && g->approach_route == 7) {
        approach_refresh_entrance(g);
    }
    if (save_version < 193 && g->location == LOCATION_DAWNMERE) {
        town_life_migrate(g);
    }
    if (save_version < 198) {
        migrate_stormwatch_road(g);
    }
    if (!giants_valid(g) || !northroad_valid(g) || !ironfang_valid(g) || !stormwatch_valid(g)) {
        cJSON_Delete(root);
        return 0;
    }
    if (!mangrove_state_valid(g) || (g->location == LOCATION_MANGROVE_PATH &&
        (g->player.x < 0 || g->player.x >= MANGROVE_PATH_W || g->player.y < 0 || g->player.y >= MANGROVE_PATH_H))) {
        cJSON_Delete(root);
        return 0;
    }
    if (save_version < 194) {
        stonewake_migrate(g);
        if (g->glassdeep_quest_state >= 2) {
            g->stonewake_warhammer_state = 1;
        }
    }
    if (save_version < 195) {
        stonewake_migrate_cranes(g);
    }
    if (save_version < 197) {
        const cJSON *rewards = cJSON_GetObjectItem(root, "thorn_regent_reward_pending");
        if (rewards && (!cJSON_IsNumber(rewards) || rewards->valuedouble != rewards->valueint || rewards->valueint < 0 || rewards->valueint > 3)) {
            cJSON_Delete(root);
            return 0;
        }
        int pending = rewards ? rewards->valueint : 0;
        int count = !!(pending & 1) + !!(pending & 2);
        if (g->retired_coast_item_count + count > RETIRED_LOOT_STAGES * MAX_FLOOR_ITEMS) {
            cJSON_Delete(root);
            return 0;
        }
        migrate_western_geography(g, removed_garden, pending);
        game_recover_retired_items(g);
    }
    if (save_version < 200) {
        ironfang_migrate_road(g);
    }
    if (save_version < 202 && g->location == LOCATION_DAWNMERE_MERCHANT &&
        strcmp(g->dialogue_speaker, "Merchant Corvin") == 0) {
        snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Spice Merchant Corvin");
    }
    if (save_version < 203) {
        giants_migrate_geography(g);
    }
    heartseed_migrate(g);
    stormbell_migrate(g);
    if (save_version < 206) {
        stormwatch_repair_boss(g);
    }
    if (save_version < 205 && g->location == LOCATION_DAWNMERE_MERCHANT && g->dialogue_active &&
        strcmp(g->dialogue_speaker, "Spice Merchant Corvin") == 0) {
        spice_skiff_talk(g);
    }
    game_refresh_debug_passages(g);
    cJSON_Delete(root);
    return 1;
}

int get_save_preview(int slot, char *name_out, int *level_out) {
    FILE *f = fopen(slot_path(slot), "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if (!root) return 0;

    cJSON *player = cJSON_GetObjectItem(root, "player");
    cJSON *name = cJSON_GetObjectItem(player, "name");
    cJSON *level = cJSON_GetObjectItem(player, "level");
    if (!cJSON_IsString(name) || !cJSON_IsNumber(level)) {
        cJSON_Delete(root);
        return 0;
    }
    strncpy(name_out, name->valuestring, 20);
    name_out[20] = '\0';
    *level_out = level->valueint;

    cJSON_Delete(root);
    return 1;
}
