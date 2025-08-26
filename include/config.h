#ifndef CONFIG_H
#define CONFIG_H

// Configuration structure
typedef struct {
    // Display settings
    int use_colors;           // 1 = use colors, 0 = no colors
    int show_tutorial_tips;   // 1 = show tips, 0 = hide tips
    
    // Gameplay settings
    int starting_hp;          // Player starting health (future feature)
    int enemy_spawn_rate;     // How many enemies to spawn (1-3 = easy, 4-6 = normal, 7-10 = hard)
    
    // Control settings
    char quit_key;           // Key to quit ('q' by default)
    char help_key;           // Key for help ('?' by default)
    
    // Game info
    char player_name[32];    // Player's name
} GameConfig;

// Configuration functions
void config_load_defaults(GameConfig* config);
void config_load_from_file(GameConfig* config, const char* filename);
void config_save_to_file(const GameConfig* config, const char* filename);
void config_print_current(const GameConfig* config);

#endif