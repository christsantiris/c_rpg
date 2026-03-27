#ifndef VIEWPORT_HEADER_H
#define VIEWPORT_HEADER_H

typedef struct {
    int cam_x, cam_y;
    int tiles_x, tiles_y;
    int map_w, map_h;
} Viewport;

void viewport_init(Viewport *v, int tiles_x, int tiles_y, int map_w, int map_h);
void viewport_center_on(Viewport *v, int player_x, int player_y);
void viewport_on_resize(Viewport *v, int tiles_x, int tiles_y);
int  viewport_to_screen_x(const Viewport *v, int world_x);
int  viewport_to_screen_y(const Viewport *v, int world_y);
int  viewport_is_visible(const Viewport *v, int world_x, int world_y);

#endif