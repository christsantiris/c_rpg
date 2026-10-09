#ifndef TOWN_LIFE_RENDERER_H
#define TOWN_LIFE_RENDERER_H

#include "renderer.h"
#include "viewport.h"
#include "../game/town_life.h"

void town_life_draw_building(Renderer *r, const Viewport *v, const TownBuilding *b);
void town_life_draw_resident(Renderer *r, int tx, int ty, int style);
void town_life_draw_goods(Renderer *r, int tx, int ty, int style);

#endif
