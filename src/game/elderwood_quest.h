#ifndef ELDERWOOD_QUEST_H
#define ELDERWOOD_QUEST_H

#include "game.h"

#define RANGER_TAMSIN_X 34
#define RANGER_TAMSIN_Y 13
#define ELDERWOOD_QUEST_GOLD 120
#define ELDERWOOD_QUEST_SCORE 800
#define RANGER_TAMSIN "Ranger Tamsin"

void elderwood_quest_sync(GameState *g);
int elderwood_quest_talk(GameState *g);
void elderwood_quest_migrate(GameState *g);

#endif
