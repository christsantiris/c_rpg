#ifndef MAP_HEADER_H
#define MAP_HEADER_H

#define MAP_W 200
#define MAP_H 100

#define MIN_ROOMS 6
#define MAX_ROOMS 10
#define MIN_ROOM_W 8 // dungeon room size
#define MAX_ROOM_W 20 // dungeon room size
#define MIN_ROOM_H 8 // dungeon room size
#define MAX_ROOM_H 14 // dungeon room size

#define TOWN_W 40 // town dimensions
#define TOWN_H 25 // town dimensions

#define MAX_DEPTH 25

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
    TILE_TRAP_SPIKE,
    TILE_TRAP_FIRE,
    TILE_TRAP_POISON
} TileType;

typedef struct {
    int x, y, w, h;
} Room;

typedef struct {
    TileType tiles[MAP_H][MAP_W];
    Room     rooms[MAX_ROOMS];
    int      room_count;
    int      stairs_up_x,   stairs_up_y;
    int      stairs_down_x, stairs_down_y;
} Map;

void map_generate(Map *m, int level);
int  map_is_walkable(const Map *m, int x, int y);
void map_room_center(const Room *r, int *cx, int *cy);
void map_generate_town(Map *m, int *spawn_x, int *spawn_y);

#endif