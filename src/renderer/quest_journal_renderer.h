#ifndef QUEST_JOURNAL_RENDERER_HEADER_H
#define QUEST_JOURNAL_RENDERER_HEADER_H

#include "renderer.h"
#include "../game/game.h"
#include "../screens/quest_journal.h"

void quest_journal_draw(Renderer *r, const GameState *g, const QuestJournalScreen *screen);

#endif
