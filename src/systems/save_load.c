#include "../../include/systems/save_load.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// Create saves directory if it doesn't exist
void create_save_directory(void) {
    struct stat st = {0};
    if (stat(SAVE_DIRECTORY, &st) == -1) {
        mkdir(SAVE_DIRECTORY, 0755);
    }
}

// Check if save file exists
int save_file_exists(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file) {
        fclose(file);
        return 1; // File exists
    }
    return 0; // File doesn't exist
}

// Save game state to file
int save_game(const Game* game, const char* filename) {
    // Create saves directory if needed
    create_save_directory();
    
    FILE* file = fopen(filename, "wb");
    if (!file) {
        return 0; // Failed to save
    }
    
    // Write a simple header for validation
    const char* magic = "CONR"; // "Castle of no Return" magic bytes
    const int version = 1;
    
    if (fwrite(magic, sizeof(char), 4, file) != 4) {
        fclose(file);
        return 0;
    }
    
    if (fwrite(&version, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    // Write the main game data
    // Note: We save everything except the double buffer and viewport which are runtime-only
    
    // Save player data
    if (fwrite(&game->player, sizeof(Player), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    // Save enemies array
    if (fwrite(game->enemies, sizeof(Enemy), MAX_ENEMIES, file) != MAX_ENEMIES) {
        fclose(file);
        return 0;
    }
    
    // Save rooms array
    if (fwrite(game->rooms, sizeof(Rectangle), MAX_ROOMS, file) != MAX_ROOMS) {
        fclose(file);
        return 0;
    }
    
    // Save map data
    if (fwrite(game->map, sizeof(char), MAP_HEIGHT * MAP_WIDTH, file) != MAP_HEIGHT * MAP_WIDTH) {
        fclose(file);
        return 0;
    }
    
    // Save game state variables
    if (fwrite(&game->room_count, sizeof(int), 1, file) != 1 ||
        fwrite(&game->game_over, sizeof(int), 1, file) != 1 ||
        fwrite(&game->turn_count, sizeof(int), 1, file) != 1 ||
        fwrite(&game->enemies_killed, sizeof(int), 1, file) != 1 ||
        fwrite(&game->enemy_count, sizeof(int), 1, file) != 1 ||
        fwrite(&game->current_level, sizeof(int), 1, file) != 1 ||
        fwrite(&game->waiting_for_stairs, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    // Save message system data
    if (fwrite(game->recentlyDefeated, sizeof(char), 15, file) != 15 ||
        fwrite(&game->showMessage, sizeof(int), 1, file) != 1 ||
        fwrite(game->levelMessage, sizeof(char), 64, file) != 64 ||
        fwrite(&game->showLevelMessage, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    // Save config (though this could be loaded separately)
    if (fwrite(&game->config, sizeof(GameConfig), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    fclose(file);
    return 1; // Success
}

// Load game state from file
int load_game(Game* game, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return 0; // Failed to load
    }
    
    // Read and validate header
    char magic[5] = {0}; // +1 for null terminator
    int version;
    
    if (fread(magic, sizeof(char), 4, file) != 4) {
        fclose(file);
        return 0;
    }
    
    if (strcmp(magic, "CONR") != 0) {
        fclose(file);
        return 0;
    }
    
    if (fread(&version, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    if (version != 1) {
        fclose(file);
        return 0;
    }
    
    // Read the main game data
    
    // Load player data
    if (fread(&game->player, sizeof(Player), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    // Load enemies array
    if (fread(game->enemies, sizeof(Enemy), MAX_ENEMIES, file) != MAX_ENEMIES) {
        fclose(file);
        return 0;
    }
    
    // Load rooms array
    if (fread(game->rooms, sizeof(Rectangle), MAX_ROOMS, file) != MAX_ROOMS) {
        fclose(file);
        return 0;
    }
    
    // Load map data
    if (fread(game->map, sizeof(char), MAP_HEIGHT * MAP_WIDTH, file) != MAP_HEIGHT * MAP_WIDTH) {
        fclose(file);
        return 0;
    }
    
    // Load game state variables
    if (fread(&game->room_count, sizeof(int), 1, file) != 1 ||
        fread(&game->game_over, sizeof(int), 1, file) != 1 ||
        fread(&game->turn_count, sizeof(int), 1, file) != 1 ||
        fread(&game->enemies_killed, sizeof(int), 1, file) != 1 ||
        fread(&game->enemy_count, sizeof(int), 1, file) != 1 ||
        fread(&game->current_level, sizeof(int), 1, file) != 1 ||
        fread(&game->waiting_for_stairs, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    // Load message system data
    if (fread(game->recentlyDefeated, sizeof(char), 15, file) != 15 ||
        fread(&game->showMessage, sizeof(int), 1, file) != 1 ||
        fread(game->levelMessage, sizeof(char), 64, file) != 64 ||
        fread(&game->showLevelMessage, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    // Load config
    if (fread(&game->config, sizeof(GameConfig), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    fclose(file);
    
    // Set game state to playing (not menu)
    game->game_state = STATE_PLAYING;
    
    return 1; // Success
}