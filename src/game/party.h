#ifndef PARTY_H
#define PARTY_H

#include "game.h"

enum { PARTY_ALDRIC, PARTY_SOREN, PARTY_GARRAN, PARTY_MEMBER_COUNT };

const char *party_name(int member);
int party_recruited(const GameState *g, int member);
int party_fallen(const GameState *g, int member);
int party_fallen_count(const GameState *g);
int party_fallen_at(const GameState *g, int index);
int party_max_hp(const GameState *g, int member);
int party_revive(GameState *g, int member, int hp);

#endif
