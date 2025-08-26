#ifndef TYPES_H
#define TYPES_H

#include "../utils/config.h"

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
#define STAIRS_DOWN '>'

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
typedef struct Rectangle {
    int x, y, width, height;
} Rectangle;

// Player structure
typedef struct {
    int x;
    int y;
    char symbol;
    int max_hp;
    int current_hp;
    int attack;
    int defense;
    int level;              
    int experience;         
    int experience_to_next;
} Player;

// Enemy Type Enum
typedef enum {
    ENEMY_GOBLIN = 0,
    ENEMY_ORC,
    ENEMY_SKELETON,
    ENEMY_TROLL
} EnemyType;

// Enemy structure
typedef struct {
    int ID;
    int x;
    int y;
    char symbol;
    char name[32];
    int active;  // 0 = dead/removed, 1 = alive
    int max_hp;
    int current_hp;
    int attack;
    int defense;
    EnemyType type; 
    int experience;
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
    GameConfig config;
    int current_level;
    int waiting_for_stairs;  
} Game;

#endif