#ifndef SUN_GUARDIAN_H
#define SUN_GUARDIAN_H

#include "game.h"
#include "actions.h"

enum { GUARDIAN_LANCE = 1, GUARDIAN_RING, GUARDIAN_LANES, GUARDIAN_LUNAR };

void sun_guardian_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int sun_guardian_attack_marks(const GameState *g, const Enemy *e, int x, int y);
int sun_guardian_sanctuary_protects(const GameState *g, const Enemy *e, int x, int y);
int sun_guardian_tick(GameState *g, Enemy *e, EnemyProjectiles *shots);
int sun_guardian_enemy_damage(const GameState *g, const Enemy *e, int damage);
void sun_guardian_award_reward(GameState *g);
void sun_guardian_restore_reward(GameState *g);
int sun_guardian_has_interaction(const GameState *g);
int sun_guardian_interact(GameState *g, int pickup);
void sun_guardian_cancel_landing(GameState *g, int x, int y);
void sun_guardian_migrate(GameState *g);

#endif
