#ifndef MAP_HEADER_H
#define MAP_HEADER_H

#define MAP_W 200
#define MAP_H 100
#define MAP_EXPLORED_BYTES ((MAP_W * MAP_H + 7) / 8)

#define MIN_ROOMS 6
#define MAX_ROOMS 10
#define MIN_ROOM_W 8 // dungeon room size
#define MAX_ROOM_W 20 // dungeon room size
#define MIN_ROOM_H 8 // dungeon room size
#define MAX_ROOM_H 14 // dungeon room size

#define TOWN_W 44 // town dimensions
#define TOWN_H 25 // town dimensions
#define TAVERN_X 4
#define TAVERN_Y 2
#define TAVERN_W 32
#define TAVERN_H 21
#define TOWN_CAIN_X 19
#define TOWN_CAIN_Y 11
#define TOWN_HARBOR_W 5
#define TOWN_HARBOR_H 4
#define TOWN_HARBOR_X (TOWN_W - 1 - TOWN_HARBOR_W)
#define TOWN_HARBOR_Y (TOWN_H - 1 - TOWN_HARBOR_H)
#define TOWN_HARBOR_ENTRANCE_X (TOWN_HARBOR_X + 2)
#define TOWN_HARBOR_ENTRANCE_Y (TOWN_HARBOR_Y + 2)
#define TOWN_ROWAN_X (TOWN_HARBOR_X - 2)
#define TOWN_ROWAN_Y TOWN_HARBOR_Y
#define TOWN_BLACKSMITH_X 11
#define TOWN_BLACKSMITH_Y 7
#define TOWN_HEALER_X 4
#define TOWN_HEALER_Y 7
#define TOWN_HEALER_W 5
#define TOWN_HEALER_H 4
#define TOWN_HEALER_DOOR_X (TOWN_HEALER_X + 2)
#define TOWN_HEALER_DOOR_Y (TOWN_HEALER_Y + TOWN_HEALER_H - 1)
#define TOWN_ALCHEMIST_X 29
#define TOWN_ALCHEMIST_Y 7
#define TOWN_WITCH_X 35
#define TOWN_WITCH_Y 7
#define TOWN_WITCH_W 5
#define TOWN_WITCH_H 4
#define TOWN_WITCH_DOOR_X (TOWN_WITCH_X + 2)
#define TOWN_WITCH_DOOR_Y (TOWN_WITCH_Y + TOWN_WITCH_H - 1)
#define TOWN_LABYRINTH_X 15
#define TOWN_LABYRINTH_Y 18

#define LABYRINTH_W 43
#define LABYRINTH_H 25
#define LABYRINTH_SWITCH_COUNT 3

#define ISLAND_W 40
#define ISLAND_H 25
#define ISLAND_GATE_X 19
#define ISLAND_GATE_Y 7
#define ISLAND_CAMP_X 12
#define ISLAND_CAMP_Y 16
#define ISLAND_MARKER_X 10
#define ISLAND_MARKER_Y 10
#define ISLAND_STATUE_X 29
#define ISLAND_STATUE_Y 10
#define ISLAND_LAGOON_X 29
#define ISLAND_LAGOON_Y 15
#define ISLAND_SHIP_X 25
#define ISLAND_SHIP_Y 22
#define ISLAND_CAPTAIN_X 18
#define ISLAND_CAPTAIN_Y 21
#define ISLAND_NAHLA_X 15
#define ISLAND_NAHLA_Y 14
#define ISLAND_SPAWN_X 20
#define ISLAND_SPAWN_Y 21

#define TEMPLE_W 64
#define TEMPLE_H 36
#define TEMPLE_ENTRANCE_X 32
#define TEMPLE_ENTRANCE_Y 34
#define TEMPLE_TREASURE_X 32
#define TEMPLE_TREASURE_Y 3
#define TEMPLE_DEPTH 4

#define DUNGEON_DEPTH 8
#define FOREST_DEPTH 8
#define MOUNTAIN_DEPTH 8
#define COAST_DEPTH 8
#define MAX_REGION_DEPTH 8

typedef enum {
    TILE_FLOOR = 0,
    TILE_WALL,
    TILE_STAIRS_UP,
    TILE_STAIRS_DOWN,
    TILE_TOWN_FLOOR,
    TILE_TOWN_PATH,
    TILE_TOWN_EXIT,
    TILE_SHOP_BLACKSMITH,
    TILE_SHOP_ALCHEMIST,
    TILE_ITEM,
    TILE_GOLD,
    TILE_TRAP_HIDDEN,
    TILE_TRAP_REVEALED,
    TILE_TRAP_SPIKE,
    TILE_TRAP_FIRE,
    TILE_TRAP_POISON,
    TILE_RETURN_EXIT,
    TILE_LOCKED_DOOR,
    TILE_DUNGEON_KEY,
    TILE_CRYPT_DOOR,
    TILE_CRYPT_KEY,
    TILE_CRYPT_CACHE,
    TILE_DUNGEON_GATE,
    TILE_DUNGEON_SWITCH_OFF,
    TILE_DUNGEON_SWITCH_ON,
    TILE_PORTAL,
    TILE_FOREST_FLOOR,
    TILE_FOREST_WALL,
    TILE_FOREST_HIDDEN_TRAIL,
    TILE_FOREST_ENTRANCE,
    TILE_FOREST_EXIT,
    TILE_FOREST_FALSE_MARKER,
    TILE_MOUNTAIN_FLOOR,
    TILE_MOUNTAIN_WALL,
    TILE_MOUNTAIN_ENTRANCE,
    TILE_MOUNTAIN_EXIT,
    TILE_TAVERN,
    TILE_COAST_FLOOR,
    TILE_COAST_WALL,
    TILE_COAST_ENTRANCE,
    TILE_COAST_EXIT,
    TILE_FOREST_LANDMARK,
    TILE_MOUNTAIN_BRIDGE,
    TILE_MOUNTAIN_CAVE_FLOOR,
    TILE_MOUNTAIN_FORTRESS_FLOOR,
    TILE_COAST_SHALLOW_WATER,
    TILE_COAST_DEEP_WATER,
    TILE_COAST_TIDE_CONTROL,
    TILE_BROKEN_BURIAL_SEAL,
    TILE_RESTORED_BURIAL_SEAL,
    TILE_TAVERN_DOOR,
    TILE_TAVERN_FLOOR,
    TILE_TAVERN_WALL,
    TILE_TAVERN_EXIT,
    TILE_TAVERN_TABLE,
    TILE_NPC_ELOWEN,
    TILE_NPC_DAIN,
    TILE_NPC_ALDER,
    TILE_FOREST_WARDEN,
    TILE_NPC_MARA,
    TILE_COAST_BEACON_UNLIT,
    TILE_COAST_BEACON_LIT,
    TILE_COAST_DRAINED_WATER,
    TILE_BLACKSMITH_DOOR,
    TILE_ALCHEMIST_DOOR,
    TILE_WATCHTOWER,
    // Append terrain IDs: existing saves store these numeric values.
    TILE_MOUNTAIN_WEAK_BRIDGE,
    TILE_MOUNTAIN_CHASM,
    TILE_MOUNTAIN_GATE,
    TILE_MOUNTAIN_ROCKFALL,
    TILE_MOUNTAIN_HIDDEN_CAVE,
    TILE_MOUNTAIN_CACHE,
    TILE_COAST_CHANNEL_WATER,
    TILE_COAST_CHANNEL_DRY,
    TILE_COAST_SLUICE_CONTROL,
    TILE_COAST_CACHE,
    TILE_NPC_CAIN,
    TILE_NPC_ROWAN,
    TILE_HEALER,
    TILE_HEALER_DOOR,
    // Append island IDs: saved maps store these numeric values.
    TILE_ISLAND_WATER,
    TILE_ISLAND_SAND,
    TILE_ISLAND_GRASS,
    TILE_ISLAND_JUNGLE,
    TILE_ISLAND_PATH,
    TILE_ISLAND_DOCK,
    TILE_ISLAND_SHIP,
    TILE_ISLAND_CAMP,
    TILE_ISLAND_MARKER,
    TILE_ISLAND_STATUE,
    TILE_ISLAND_LAGOON,
    TILE_ISLAND_TEMPLE_GATE,
    TILE_NPC_ISLAND_CAPTAIN,
    // Append temple IDs: saved maps store these numeric values.
    TILE_TEMPLE_FLOOR,
    TILE_TEMPLE_WALL,
    TILE_TEMPLE_ENTRANCE,
    TILE_TEMPLE_ALTAR,
    TILE_TEMPLE_MOON_DOOR_CLOSED,
    TILE_TEMPLE_MOON_DOOR_OPEN,
    TILE_TEMPLE_SOLAR_TRAP,
    TILE_TEMPLE_DORMANT_SENTINEL,
    TILE_TEMPLE_VAULT_DOOR,
    TILE_TEMPLE_TREASURE,
    TILE_TEMPLE_WATER,
    TILE_TEMPLE_RUBBLE,
    TILE_NPC_ISLAND_NAHLA,
    TILE_WITCH,
    TILE_WITCH_DOOR,
    TILE_NPC_GAMBLER,
    TILE_LABYRINTH_ENTRANCE,
    TILE_LABYRINTH_FLOOR,
    TILE_LABYRINTH_WALL,
    TILE_LABYRINTH_EXIT,
    TILE_LABYRINTH_SWITCH_OFF,
    TILE_LABYRINTH_SWITCH_ON,
    TILE_LABYRINTH_GATE,
    TILE_LABYRINTH_RELIC
} TileType;

typedef struct {
    int x, y, w, h;
} Room;

typedef struct {
    TileType tiles[MAP_H][MAP_W];
    unsigned char explored[MAP_EXPLORED_BYTES];
    Room     rooms[MAX_ROOMS];
    int      room_count;
    int      stairs_up_x,   stairs_up_y;
    int      stairs_down_x, stairs_down_y;
} Map;

void map_generate(Map *m, int level);
int map_repair_dungeon_routes(Map *m);
int  map_is_walkable(const Map *m, int x, int y);
void map_room_center(const Room *r, int *cx, int *cy);
void map_generate_town(Map *m, int *spawn_x, int *spawn_y);
void map_place_town_labyrinth(Map *m);
void map_place_town_harbor(Map *m);
void map_generate_tavern(Map *m, int *spawn_x, int *spawn_y);
void map_generate_island(Map *m, int *spawn_x, int *spawn_y);
void map_generate_temple(Map *m, int level, int *spawn_x, int *spawn_y);
void map_generate_labyrinth(Map *m, int switches, int *spawn_x, int *spawn_y);
void map_generate_forest(Map *m, int level);
void map_reveal_forest_exit(Map *m);
void map_generate_mountains(Map *m, int level);
void map_generate_coast(Map *m, int level);
int map_remove_coast_sluice(Map *m);
TileType map_coast_trap_underlay(const Map *m, int x, int y);
int map_is_coast_tidal_tile(TileType tile);
int map_is_coast_object(TileType tile);
TileType map_coast_swapped_tile(TileType tile);
void map_clear_exploration(Map *m);
void map_mark_explored(Map *m, int x, int y);
int map_is_explored(const Map *m, int x, int y);

#endif
