#include "viewport.h"

void viewport_init(Viewport *v, int tiles_x, int tiles_y, int map_w, int map_h) {
    v->tiles_x = tiles_x;
    v->tiles_y = tiles_y;
    v->map_w   = map_w;
    v->map_h   = map_h;
    v->cam_x   = 0;
    v->cam_y   = 0;
}

void viewport_center_on(Viewport *v, int player_x, int player_y) {
    v->cam_x = player_x - v->tiles_x / 2;
    v->cam_y = player_y - v->tiles_y / 2;

    if (v->cam_x < 0) v->cam_x = 0;
    if (v->cam_y < 0) v->cam_y = 0;
    if (v->cam_x + v->tiles_x > v->map_w) v->cam_x = v->map_w - v->tiles_x;
    if (v->cam_y + v->tiles_y > v->map_h) v->cam_y = v->map_h - v->tiles_y;
}

void viewport_on_resize(Viewport *v, int tiles_x, int tiles_y) {
    v->tiles_x = tiles_x;
    v->tiles_y = tiles_y;
}

int viewport_to_screen_x(const Viewport *v, int world_x) {
    return world_x - v->cam_x;
}

int viewport_to_screen_y(const Viewport *v, int world_y) {
    return world_y - v->cam_y;
}

int viewport_is_visible(const Viewport *v, int world_x, int world_y) {
    return world_x >= v->cam_x && world_x < v->cam_x + v->tiles_x &&
           world_y >= v->cam_y && world_y < v->cam_y + v->tiles_y;
}