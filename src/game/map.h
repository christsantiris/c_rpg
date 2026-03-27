#ifndef MAP_HEADER_H
#define MAP_HEADER_H

#define MAP_W 200
#define MAP_H 100

typedef enum {
    TILE_FLOOR = 0,
    TILE_WALL
} TileType;

typedef struct {
    TileType tiles[MAP_H][MAP_W];
} Map;

void map_init(Map *m);
int  map_is_walkable(const Map *m, int x, int y);

#endif