#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

// Map dimensions
#define MAP_WIDTH 40
#define MAP_HEIGHT 20

// Symbols
#define WALL '#'
#define FLOOR '.'
#define PLAYER 'C'
#define ENEMY 'E'

// Room generation constants
#define MIN_ROOM_SIZE 4
#define MAX_ROOM_SIZE 10
#define MAX_ROOMS 6

// Key codes
#ifndef KEY_ESC
#define KEY_ESC 27
#endif

// Rectangle structure for rooms
typedef struct {
    int x, y, width, height;
} Rectangle;

// Player structure
typedef struct {
    int x;
    int y;
    char symbol;
    int active;
} Player;

// Enemy structure
typedef struct {
    int x;
    int y;
    char symbol;
    char name[32];
    int active;
} Enemy;

// Game structure
typedef struct {
    Player player;
    char map[MAP_HEIGHT][MAP_WIDTH];
    Rectangle rooms[MAX_ROOMS];
    int room_count;
    Enemy enemy;
} Game;

// Function declarations
void init_game(Game *game);
void init_ncurses(void);
void cleanup_ncurses(void);
void draw_map(Game *game);
void draw_player(Game *game);
void draw_enemy(Game *game);
int handle_input(Game *game);
int is_valid_move(Game *game, int new_x, int new_y);
int is_valid_player_move(Game *game, int new_x, int new_y);
int is_valid_enemy_move(Game *game, int new_x, int new_y);
void move_player(Game *game, int dx, int dy);

// Dungeon generation functions
void generate_dungeon(Game *game);
void create_room(Game *game, Rectangle room);
void create_corridor(Game *game, int x1, int y1, int x2, int y2);
int rooms_overlap(Rectangle r1, Rectangle r2);
int random_range(int min, int max);

// Enemy Moves
void move_enemy(Game *game);

#endif