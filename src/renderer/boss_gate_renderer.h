#ifndef BOSS_GATE_RENDERER_H
#define BOSS_GATE_RENDERER_H

#include "renderer.h"
#include "viewport.h"
#include "../game/boss_gates.h"

void boss_gate_draw_bank(Renderer *r, const GameState *g, int sx, int sy, int x, int y);
void boss_gate_draw_banks(Renderer *r, const GameState *g, const Viewport *v);
void boss_gate_draw(Renderer *r, const GameState *g, const Viewport *v);

#endif
