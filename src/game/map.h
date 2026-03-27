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

typedef enum {
    TILE_FLOOR = 0,
    TILE_WALL,
    TILE_STAIRS_UP,
    TILE_STAIRS_DOWN
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

#endif