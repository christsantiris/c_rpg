#ifndef EXPEDITION_FIXTURE_H
#define EXPEDITION_FIXTURE_H

#include "game/game.h"
#include "cJSON.h"

// Restore an archived floor for isolated encounter/save tests. This is a test
// fixture, not a playable return trip; escape behavior has its own tests.
void test_restore_cached_floor(GameState *g);
void test_mark_legacy_portal(cJSON *root);

#endif
