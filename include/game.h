#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

// Game states
#define STATE_MENU 0
#define STATE_PLAYING 1
#define MAX_ENEMIES 10

// Menu options
#define MENU_NEW_GAME 0
#define MENU_LOAD_GAME 1
#define MENU_QUIT 2
#define MENU_OPTIONS 3

// Map dimensions
#define MAP_WIDTH 60
#define MAP_HEIGHT 30

// Symbols
#define WALL '#'
#define FLOOR '.'
#define PLAYER 'C'
#define ENEMY 'E'

// Room generation constants
#define MIN_ROOM_SIZE 4
#define MAX_ROOM_SIZE 10
#define MAX_ROOMS 8

// Colors
#define COLOR_WALL 1
#define COLOR_FLOOR 2
#define COLOR_PLAYER 3
#define COLOR_ENEMY 4
#define COLOR_TEXT 5

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
} Player;

// Enemy structure
typedef struct {
    int ID;
    int x;
    int y;
    char symbol;
    char name[32];
    int active;  // 0 = dead/removed, 1 = alive
} Enemy;

// Game structure
typedef struct {
    Player player;
    Enemy enemies[MAX_ENEMIES];
    Rectangle rooms[MAX_ROOMS];
    char map[MAP_HEIGHT][MAP_WIDTH];
    int room_count;
    int game_over;  // 0 = playing, 1 = game over
    int turn_count;     
    int enemies_killed; 
    int enemy_count;
    int game_state;
    int selected_menu;
    char recentlyDefeated[15];
    int showMessage;
} Game;

// Function declarations

// Menu functions
void draw_title_screen(Game *game);
void handle_menu_input(Game *game);
void start_new_game(Game *game);

void init_game(Game *game);
void init_ncurses(void);
void cleanup_ncurses(void);

void draw_map(Game *game);
void draw_player(Game *game);
void draw_enemy(Game *game, int enemy_index);

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
void move_enemy(Game *game, int enemy_index);

#endif