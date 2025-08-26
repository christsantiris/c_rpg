#ifndef DUNGEON_H
#define DUNGEON_H

#include "types.h"

// Dungeon generation functions
void generate_dungeon(Game *game);
void create_room(Game *game, Rectangle room);
void create_corridor(Game *game, int x1, int y1, int x2, int y2);
int rooms_overlap(Rectangle r1, Rectangle r2);
int random_range(int min, int max);

#endif