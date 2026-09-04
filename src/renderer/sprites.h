#ifndef SPRITES_H
#define SPRITES_H
#include "../game/enemy.h"
#include "../game/game.h"
#include "renderer.h"

// Map draw functions
void draw_floor(Renderer *r, int tile_x, int tile_y);
void draw_wall(Renderer *r, int tile_x, int tile_y);
void draw_forest_floor(Renderer *r, int tile_x, int tile_y);
void draw_forest_wall(Renderer *r, int tile_x, int tile_y);
void draw_forest_edge(Renderer *r, int tile_x, int tile_y, int forward);
void draw_forest_landmark(Renderer *r, int tile_x, int tile_y);
void draw_mountain_floor(Renderer *r, int tile_x, int tile_y);
void draw_mountain_wall(Renderer *r, int tile_x, int tile_y);
void draw_mountain_edge(Renderer *r, int tile_x, int tile_y, int forward);
void draw_coast_floor(Renderer *r, int tile_x, int tile_y);
void draw_coast_wall(Renderer *r, int tile_x, int tile_y);
void draw_coast_edge(Renderer *r, int tile_x, int tile_y, int forward);
void draw_player(Renderer *r, int tile_x, int tile_y, PlayerClass player_class);
void draw_stairs_up(Renderer *r, int tile_x, int tile_y);
void draw_stairs_down(Renderer *r, int tile_x, int tile_y);
void draw_return_exit(Renderer *r, int tile_x, int tile_y);
void draw_locked_door(Renderer *r, int tile_x, int tile_y);
void draw_dungeon_key(Renderer *r, int tile_x, int tile_y);
void draw_portal(Renderer *r, int tile_x, int tile_y);

// Enemey draw functions
void draw_skeleton(Renderer *r, int tile_x, int tile_y);
void draw_goblin(Renderer *r, int tile_x, int tile_y);
void draw_zombie(Renderer *r, int tile_x, int tile_y);
void draw_crypt_bat(Renderer *r, int tile_x, int tile_y);
void draw_wraith(Renderer *r, int tile_x, int tile_y);
void draw_necromancer(Renderer *r, int tile_x, int tile_y);
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
typedef enum {
    TOWN_EXIT_DUNGEON,
    TOWN_EXIT_FOREST,
    TOWN_EXIT_MOUNTAINS,
    TOWN_EXIT_COAST
} TownExitStyle;

void draw_town_floor(Renderer *r, int tile_x, int tile_y);
void draw_town_path(Renderer *r, int tile_x, int tile_y);
void draw_town_exit(Renderer *r, int tile_x, int tile_y, TownExitStyle style, int segment);
void draw_shop_blacksmith(Renderer *r, int tile_x, int tile_y);
void draw_shop_alchemist(Renderer *r, int tile_x, int tile_y);
void draw_tavern(Renderer *r, int tile_x, int tile_y);

void draw_floor_item(Renderer *r, int tile_x, int tile_y);
void draw_floor_gold(Renderer *r, int tile_x, int tile_y);

void draw_trap_spike(Renderer *r, int tile_x, int tile_y);
void draw_trap_fire(Renderer *r, int tile_x, int tile_y);
void draw_trap_poison(Renderer *r, int tile_x, int tile_y);

#endif
