#include "game.h"

void game_init(GameState *g, int map_w, int map_h) {
    g->map_w    = map_w;
    g->map_h    = map_h;
    g->player.x = map_w / 2;
    g->player.y = map_h / 2;
}

void game_move_player(GameState *g, int dx, int dy) {
    int nx = g->player.x + dx;
    int ny = g->player.y + dy;

    // Keep player inside the wall border
    if (nx < 1 || nx >= g->map_w - 1) return;
    if (ny < 1 || ny >= g->map_h - 1) return;

    g->player.x = nx;
    g->player.y = ny;
}