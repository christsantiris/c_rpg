#ifndef COMBAT_H
#define COMBAT_H

#include "types.h"

// Combat functions
int calculate_damage(int attack, int defense);
int player_attack_enemy(Player* player, Enemy* enemy);
int enemy_attack_player(Enemy* enemy, Player* player);

#endif