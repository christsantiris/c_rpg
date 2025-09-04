#ifndef SAVE_LOAD_H
#define SAVE_LOAD_H

#include "../core/types.h"

// Save/Load function declarations
int save_game(const Game* game, const char* filename);
int load_game(Game* game, const char* filename);
int save_file_exists(const char* filename);
void create_save_directory(void);

// Save file constants
#define SAVE_FILENAME "saves/savegame.dat"
#define SAVE_DIRECTORY "saves"

#endif