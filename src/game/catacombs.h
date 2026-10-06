#ifndef CATACOMBS_HEADER_H
#define CATACOMBS_HEADER_H

#include "game.h"

void map_generate_catacombs(Map *m, int level);
int catacombs_room_at(const Map *m, int x, int y);
int catacombs_lit_braziers(const Map *m, int room);
int catacombs_has_interaction(const GameState *g);
int catacombs_interact(GameState *g);
void catacombs_record_death(GameState *g, Enemy *e);
int catacombs_cantor_raise(GameState *g, const Enemy *caster);
int catacombs_tick(GameState *g);
int catacombs_trap_marks(const Map *m, const BurialTrap *trap, int x, int y);
int catacombs_sweep_marks(const Map *m, const Enemy *e, int x, int y);
int catacombs_enemy_damage(const GameState *g, const Enemy *e, int damage);
void catacombs_restore_reward(GameState *g);

#endif
