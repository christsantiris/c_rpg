#ifndef HALLOFFAME_RENDERER_HEADER_H
#define HALLOFFAME_RENDERER_HEADER_H

#include "renderer.h"
#include "../systems/highscore.h"

void halloffame_draw(Renderer *r, const HighScoreTable *t);

#endif