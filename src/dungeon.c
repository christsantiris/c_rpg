#include "../include/game.h"

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

int random_range(int min, int max) {
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

