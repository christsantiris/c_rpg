#ifndef JAIL_HEADER_H
#define JAIL_HEADER_H

#include "game.h"

#define JAIL_X 5
#define JAIL_Y 17
#define JAIL_W 7
#define JAIL_H 5
#define JAIL_DOOR_X (JAIL_X + JAIL_W / 2)
#define JAIL_DOOR_Y (JAIL_Y + JAIL_H - 1)
#define INFORMANT_X 30
#define INFORMANT_Y 15
#define JAIL_ROOM_W 24
#define JAIL_ROOM_H 18
#define JAIL_PRISONER_X 13
#define JAIL_PRISONER_Y 8
#define JAIL_HATCH_X 17
#define JAIL_HATCH_Y 11
#define ESCAPE_TUNNEL_LEGACY_W 64
#define ESCAPE_TUNNEL_W 190
#define ESCAPE_TUNNEL_H 28
#define ESCAPE_REWARD_GOLD 100
#define ESCAPE_REWARD_SCORE 750

void jail_place_castle(Map *m);
void jail_migrate_castle(GameState *g);
void jail_migrate_tunnel(GameState *g);
int jail_talk_nearby(GameState *g);
int jail_move_exit(GameState *g, int x, int y);
void jail_follow(GameState *g);
int jail_prisoner_at(const GameState *g, int x, int y);
int jail_blocks_portal(GameState *g);

#endif
