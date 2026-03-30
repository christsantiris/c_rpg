#ifndef SPRITES_H
#define SPRITES_H
#include "../game/enemy.h"
#include "renderer.h"

// Map draw functions
void draw_floor(Renderer *r, int tile_x, int tile_y);
void draw_wall(Renderer *r, int tile_x, int tile_y);
void draw_player(Renderer *r, int tile_x, int tile_y);
void draw_stairs_up(Renderer *r, int tile_x, int tile_y);
void draw_stairs_down(Renderer *r, int tile_x, int tile_y);

// Enemey draw functions
void draw_goblin(Renderer *r, int tile_x, int tile_y);
void draw_skeleton(Renderer *r, int tile_x, int tile_y);
void draw_orc(Renderer *r, int tile_x, int tile_y);
void draw_troll(Renderer *r, int tile_x, int tile_y);
void draw_enemy(Renderer *r, int tile_x, int tile_y, EnemyType type);

#endif