#ifndef ROYAL_EVIDENCE_RENDERER_H
#define ROYAL_EVIDENCE_RENDERER_H

#include "renderer.h"
#include "../game/royal_evidence.h"

void royal_evidence_draw(Renderer *r, const GameState *g, const RoyalEvidence *document, SDL_Rect paper, const char *footer);

#endif
