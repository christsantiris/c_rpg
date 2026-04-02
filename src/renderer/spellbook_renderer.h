#ifndef SPELLBOOK_RENDERER_HEADER_H
#define SPELLBOOK_RENDERER_HEADER_H

#include "renderer.h"
#include "../game/game.h"
#include "../screens/spellbook.h"

void spellbook_draw(Renderer *r, const GameState *g, const SpellbookScreen *s);

#endif