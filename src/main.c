#include "../include/game.h"

int main() {
    Game game;
    
    // Initialize game
    init_ncurses();
    init_game(&game);
    
    // Main game loop
    int running = 1;
    while (running) {
        // Clear screen and redraw world
        clear();
        draw_map(&game);
        draw_player(&game);
        
        // Show instructions at the bottom
        mvprintw(MAP_HEIGHT + 2, 0, "Use arrow keys to move, 'q' to quit");
        
        // Refresh screen to render changes
        refresh();
        
        // Handle player input
        running = handle_input(&game);
    }
    
    // Clean up and exit
    cleanup_ncurses();
    printf("Thanks for playing!\n");
    return 0;
}

void init_ncurses() {
    // Initialize ncurses
    initscr();
    
    // Don't echo pressed keys to screen
    noecho();
    
    // Enable special keys
    keypad(stdscr, TRUE);
    
    // Hide cursor
    curs_set(0);
}

void cleanup_ncurses() {
    endwin();
}

void init_game(Game *game) {
    // Random number generator
    srand(time(NULL));
    
    // Initialize player
    game->player.symbol = PLAYER;
    
    // Initialize room count
    game->room_count = 0;
    
    // Generate the dungeon
    generate_dungeon(game);
    
    // Place player in the center of the first room
    if (game->room_count > 0) {
        game->player.x = game->rooms[0].x + game->rooms[0].width / 2;
        game->player.y = game->rooms[0].y + game->rooms[0].height / 2;
    } else {
        // Fallback if no rooms generated
        game->player.x = MAP_WIDTH / 2;
        game->player.y = MAP_HEIGHT / 2;
    }
}

void draw_map(Game *game) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            // mvaddch moves cursor to (y,x) and adds character
            mvaddch(y, x, game->map[y][x]);
        }
    }
}

void draw_player(Game *game) {
    // Draw player on top of the map
    mvaddch(game->player.y, game->player.x, game->player.symbol);
}

int handle_input(Game *game) {
    int ch = getch(); // Get a character from user
    
    switch (ch) {
        case KEY_UP:
        case 'w':
        case 'W':
            move_player(game, 0, -1); // Move up (y decreases)
            break;
            
        case KEY_DOWN:
        case 's':
        case 'S':
            move_player(game, 0, 1); // Move down (y increases)
            break;
            
        case KEY_LEFT:
        case 'a':
        case 'A':
            move_player(game, -1, 0); // Move left (x decreases)
            break;
            
        case KEY_RIGHT:
        case 'd':
        case 'D':
            move_player(game, 1, 0); // Move right (x increases)
            break;
            
        case 'q':
        case 'Q':
        case KEY_ESC:
            return 0; // Quit game
            
        default:
            // Invalid key, do nothing
            break;
    }
    
    return 1; // Continue game
}

int is_valid_move(Game *game, int new_x, int new_y) {
    // Check bounds
    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT) {
        return 0;
    }
    
    // Check if the tile is walkable (not a wall)
    if (game->map[new_y][new_x] == WALL) {
        return 0;
    }
    
    return 1; // Valid move
}

void move_player(Game *game, int dx, int dy) {
    int new_x = game->player.x + dx;
    int new_y = game->player.y + dy;
    
    // Only move if the new position is valid
    if (is_valid_move(game, new_x, new_y)) {
        game->player.x = new_x;
        game->player.y = new_y;
    }
    // If invalid move, just ignore it (player doesn't move)
}


int random_range(int min, int max) {
    if (min > max) {
        // Swap if min > max
        int temp = min;
        min = max;
        max = temp;
    }
    if (min == max) {
        return min;
    }
    return min + rand() % (max - min + 1);
}

int rooms_overlap(Rectangle r1, Rectangle r2) {
    return (r1.x < r2.x + r2.width && r1.x + r1.width > r2.x &&
            r1.y < r2.y + r2.height && r1.y + r1.height > r2.y);
}

// Create room in dungeon
void create_room(Game *game, Rectangle room) {
    for (int y = room.y; y < room.y + room.height; y++) {
        for (int x = room.x; x < room.x + room.width; x++) {
            if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
                game->map[y][x] = FLOOR;
            }
        }
    }
}

void create_corridor(Game *game, int x1, int y1, int x2, int y2) {
    // Create horizontal corridor first
    int start_x = (x1 < x2) ? x1 : x2;
    int end_x = (x1 > x2) ? x1 : x2;
    for (int x = start_x; x <= end_x; x++) {
        if (x >= 0 && x < MAP_WIDTH && y1 >= 0 && y1 < MAP_HEIGHT) {
            game->map[y1][x] = FLOOR;
        }
    }
    
    // Create vertical corridor
    int start_y = (y1 < y2) ? y1 : y2;
    int end_y = (y1 > y2) ? y1 : y2;
    for (int y = start_y; y <= end_y; y++) {
        if (x2 >= 0 && x2 < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
            game->map[y][x2] = FLOOR;
        }
    }
}

void generate_dungeon(Game *game) {
    // Fill entire map with walls
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            game->map[y][x] = WALL;
        }
    }
    
    game->room_count = 0;
    
    // Try to create rooms
    for (int attempts = 0; attempts < 30 && game->room_count < MAX_ROOMS; attempts++) {
        Rectangle room;
        
        // Random room size
        room.width = random_range(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
        room.height = random_range(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
        
        // Random position (leave space for walls)
        room.x = random_range(1, MAP_WIDTH - room.width - 1);
        room.y = random_range(1, MAP_HEIGHT - room.height - 1);
        
        // Check if room overlaps with existing rooms
        int overlaps = 0;
        for (int i = 0; i < game->room_count; i++) {
            if (rooms_overlap(room, game->rooms[i])) {
                overlaps = 1;
                break;
            }
        }
        
        if (!overlaps) {
            // Create the room
            create_room(game, room);
            
            // Connect to previous room with corridor
            if (game->room_count > 0) {
                Rectangle prev_room = game->rooms[game->room_count - 1];
                int prev_center_x = prev_room.x + prev_room.width / 2;
                int prev_center_y = prev_room.y + prev_room.height / 2;
                int curr_center_x = room.x + room.width / 2;
                int curr_center_y = room.y + room.height / 2;
                
                create_corridor(game, prev_center_x, prev_center_y, curr_center_x, curr_center_y);
            }
            
            // Save the room
            game->rooms[game->room_count] = room;
            game->room_count++;
        }
    }
}