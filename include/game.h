#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>

// Map dimensions
#define MAP_WIDTH 20
#define MAP_HEIGHT 10

// Symbols
#define WALL '#'
#define FLOOR '.'
#define PLAYER 'C'

// Key codes (in case your system doesn't define them)
#ifndef KEY_ESC
#define KEY_ESC 27
#endif

// Player structure
typedef struct {
    int x;
    int y;
    char symbol;
} Player;

// Game structure
typedef struct {
    Player player;
    char map[MAP_HEIGHT][MAP_WIDTH];
} Game;

// Function declarations
void init_game(Game *game);
void init_ncurses(void);
void cleanup_ncurses(void);
void draw_map(Game *game);
void draw_player(Game *game);
int handle_input(Game *game);
int is_valid_move(Game *game, int new_x, int new_y);
void move_player(Game *game, int dx, int dy);

#endif