#ifndef ALDRIC_H
#define ALDRIC_H

#include "game.h"

#define ALDRIC_SPEAKER "Veteran Aldric"

enum {
    ALDRIC_WAITING,
    ALDRIC_FOLLOWING
};

int aldric_at(const GameState *g, int x, int y);
void aldric_place(GameState *g);
void aldric_follow(GameState *g, int lead_old_x, int lead_old_y);
void aldric_talk(GameState *g, int x, int y);
int aldric_prompt_active(const GameState *g);
int aldric_handle_key(GameState *g, int key, int repeat);

#endif
