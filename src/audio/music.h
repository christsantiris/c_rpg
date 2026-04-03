#ifndef MUSIC_HEADER_H
#define MUSIC_HEADER_H

#include "../screens/landing.h"

void music_init(void);
void music_update(GameScreen screen, int dungeon_level);
void music_free(void);

#endif