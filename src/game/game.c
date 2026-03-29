#include "game.h"
#include <stdlib.h>
#include <time.h>

void game_init(GameState *g) {
    srand((unsigned)time(NULL));
    g->level = 1;
    map_generate(&g->map, g->level);
    g->player.name[0] = '\0';
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
}

void game_move_player(GameState *g, int dx, int dy) {
    int nx = g->player.x + dx;
    int ny = g->player.y + dy;
    if (!map_is_walkable(&g->map, nx, ny)) return;
    g->player.x = nx;
    g->player.y = ny;
}

void game_descend(GameState *g) {
    g->level++;
    map_generate(&g->map, g->level);
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
}

void game_ascend(GameState *g) {
    if (g->level <= 1) return;
    g->level--;
    map_generate(&g->map, g->level);
    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y;
}