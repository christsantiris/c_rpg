#ifndef SPRITES_H
#define SPRITES_H

#include "renderer.h"

void draw_floor(Renderer *r, int tile_x, int tile_y);
void draw_wall(Renderer *r, int tile_x, int tile_y);
void draw_player(Renderer *r, int tile_x, int tile_y);
void draw_stairs_up(Renderer *r, int tile_x, int tile_y);
void draw_stairs_down(Renderer *r, int tile_x, int tile_y);

#endif