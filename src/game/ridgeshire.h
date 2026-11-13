#ifndef RIDGESHIRE_H
#define RIDGESHIRE_H

#include "game.h"

#define RENN_X 34
#define RENN_Y 14
#define RIDGE_ILYA_X 10
#define RIDGE_ILYA_Y 14
#define RIDGE_DRAGON_SPEAKER "Edric: Dragon Binding"
#define RIDGE_HOARD_SPEAKER "Ilya: Dragon's Hoard"
#define RIDGE_REST_GOLD 100

enum { RIDGE_MENU, RIDGE_EDRIC_MENU, RIDGE_INN_MENU, RIDGE_REST, RIDGE_ARMOR };
enum { RIDGE_ARMOR_NONE, RIDGE_ARMOR_PENDING, RIDGE_DRAGONPLATE, RIDGE_DRAGONLEATHER, RIDGE_EMBERWEAVE };

void ridgeshire_sync(GameState *g);
void ridgeshire_prepare(GameState *g);
void ridgeshire_migrate(GameState *g, int version);
int ridgeshire_talk(GameState *g);
int ridgeshire_dialogue(const GameState *g);
int ridgeshire_handle_key(GameState *g, int key, int repeat);
const char *ridgeshire_hint(const GameState *g);
void ridgeshire_armor_offer(GameState *g);
void ridgeshire_claim_ring(GameState *g);
int ridgeshire_valid(const GameState *g);

#endif
