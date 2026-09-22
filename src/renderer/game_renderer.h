#ifndef GAME_RENDERER_HEADER_H
#define GAME_RENDERER_HEADER_H

#include "renderer.h"
#include "viewport.h"
#include "../game/game.h"

#define SPELL_TRAVEL_MS 240
#define SPELL_ARROW_MS 340
#define SPELL_FIREBALL_MS 460
#define ENEMY_PROJECTILE_TRAVEL_MS 280
#define ENEMY_PROJECTILE_TOTAL_MS 400

void game_draw(Renderer *r, GameState *g, Viewport *v);
void game_draw_enemy_projectiles(Renderer *r, const EnemyProjectiles *shots, const Viewport *v, Uint32 elapsed);

#endif
