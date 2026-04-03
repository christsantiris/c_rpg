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
void draw_skeleton(Renderer *r, int tile_x, int tile_y);
void draw_goblin(Renderer *r, int tile_x, int tile_y);
void draw_zombie(Renderer *r, int tile_x, int tile_y);
void draw_orc(Renderer *r, int tile_x, int tile_y);
void draw_troll(Renderer *r, int tile_x, int tile_y);
void draw_giant(Renderer *r, int tile_x, int tile_y);
void draw_enemy(Renderer *r, int tile_x, int tile_y, EnemyType type);

void draw_goblin_king(Renderer *r, int tile_x, int tile_y);
void draw_lich_king(Renderer *r, int tile_x, int tile_y);
void draw_demon_lord(Renderer *r, int tile_x, int tile_y);
void draw_red_dragon(Renderer *r, int tile_x, int tile_y);
void draw_tarrasque(Renderer *r, int tile_x, int tile_y);

// Town Draw functions
void draw_town_floor(Renderer *r, int tile_x, int tile_y);
void draw_town_path(Renderer *r, int tile_x, int tile_y);
void draw_town_exit(Renderer *r, int tile_x, int tile_y);
void draw_shop_blacksmith(Renderer *r, int tile_x, int tile_y);
void draw_shop_alchemist(Renderer *r, int tile_x, int tile_y);

void draw_floor_item(Renderer *r, int tile_x, int tile_y);
void draw_floor_gold(Renderer *r, int tile_x, int tile_y);

#endif