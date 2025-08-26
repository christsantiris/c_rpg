#include "../include/utils/config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void config_load_defaults(GameConfig* config) {
    // Set default values
    config->use_colors = 1;              // Colors enabled
    config->show_tutorial_tips = 1;      // Show tips
    config->starting_hp = 100;           // Default health
    config->enemy_spawn_rate = 4;        // Normal difficulty
    config->quit_key = 'q';              // 'q' to quit
    config->help_key = '?';              // '?' for help
    strcpy(config->player_name, "Hero"); // Default name
    
    printf("Loaded default configuration.\n");
}

void config_load_from_file(GameConfig* config, const char* filename) {
    FILE* file = fopen(filename, "r");
    
    if (!file) {
        printf("Config file '%s' not found, using defaults.\n", filename);
        return;
    }
    
    // Start with defaults, then override with file values
    config_load_defaults(config);
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;
        
        // Parse key=value pairs
        char key[64], value[64];
        if (sscanf(line, "%63[^=]=%63[^\n]", key, value) == 2) {
            
            if (strcmp(key, "use_colors") == 0) {
                config->use_colors = atoi(value);
            }
            else if (strcmp(key, "show_tutorial_tips") == 0) {
                config->show_tutorial_tips = atoi(value);
            }
            else if (strcmp(key, "starting_hp") == 0) {
                config->starting_hp = atoi(value);
            }
            else if (strcmp(key, "enemy_spawn_rate") == 0) {
                config->enemy_spawn_rate = atoi(value);
            }
            else if (strcmp(key, "quit_key") == 0) {
                config->quit_key = value[0];  // First character
            }
            else if (strcmp(key, "help_key") == 0) {
                config->help_key = value[0];
            }
            else if (strcmp(key, "player_name") == 0) {
                strncpy(config->player_name, value, 31);
                config->player_name[31] = '\0';  // Ensure null termination
            }
        }
    }
    
    fclose(file);
    printf("Loaded configuration from '%s'.\n", filename);
}

void config_save_to_file(const GameConfig* config, const char* filename) {
    FILE* file = fopen(filename, "w");
    
    if (!file) {
        printf("Config file '%s' not found, creating default configuration.\n", filename);
        
        // Save the defaults to create the file
        config_save_to_file(config, filename);
        return;
    }
    
    fprintf(file, "# Castle of no Return - Configuration File\n");
    fprintf(file, "# Edit values below to customize your game\n\n");
    
    fprintf(file, "# Display Settings\n");
    fprintf(file, "use_colors=%d\n", config->use_colors);
    fprintf(file, "show_tutorial_tips=%d\n", config->show_tutorial_tips);
    
    fprintf(file, "\n# Gameplay Settings\n");
    fprintf(file, "starting_hp=%d\n", config->starting_hp);
    fprintf(file, "enemy_spawn_rate=%d\n", config->enemy_spawn_rate);
    
    fprintf(file, "\n# Controls (single character)\n");
    fprintf(file, "quit_key=%c\n", config->quit_key);
    fprintf(file, "help_key=%c\n", config->help_key);
    
    fprintf(file, "\n# Player Info\n");
    fprintf(file, "player_name=%s\n", config->player_name);
    
    fclose(file);
    printf("Configuration saved to '%s'.\n", filename);
}

void config_print_current(const GameConfig* config) {
    printf("\n=== Current Configuration ===\n");
    printf("Use Colors: %s\n", config->use_colors ? "Yes" : "No");
    printf("Show Tips: %s\n", config->show_tutorial_tips ? "Yes" : "No");
    printf("Starting HP: %d\n", config->starting_hp);
    printf("Enemy Spawn Rate: %d\n", config->enemy_spawn_rate);
    printf("Quit Key: '%c'\n", config->quit_key);
    printf("Help Key: '%c'\n", config->help_key);
    printf("Player Name: %s\n", config->player_name);
    printf("============================\n\n");
}