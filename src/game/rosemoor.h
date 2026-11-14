#ifndef ROSEMOOR_H
#define ROSEMOOR_H

#include "game.h"

#define ROSE_HADRIN_X 10
#define ROSE_HADRIN_Y 14
#define ROSE_FROST_SPEAKER "Mira: Broken Ward"
#define ROSE_FORGE_SPEAKER "Forgekeeper Hadrin"
#define ROSE_HUNT_SPEAKER "Selene: Rimefang"

enum { ROSE_INFO, ROSE_MIRA_MENU, ROSE_POTION, ROSE_WEAPON };
enum { ROSE_WEAPON_NONE, ROSE_WEAPON_PENDING, ROSE_BLADE, ROSE_BOW, ROSE_STAFF };

void rosemoor_sync(GameState *g);
void rosemoor_prepare(GameState *g);
void rosemoor_migrate(GameState *g, int version);
int rosemoor_talk(GameState *g);
int rosemoor_dialogue(const GameState *g);
const char *rosemoor_hint(const GameState *g);
int rosemoor_handle_key(GameState *g, int key, int repeat);
int rosemoor_open_shop(const GameState *g, int key);
void rosemoor_kraken_reward(GameState *g);
void rosemoor_weapon_offer(GameState *g);
int rosemoor_valid(const GameState *g);

#endif
