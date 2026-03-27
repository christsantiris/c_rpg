#ifndef GAME_H
#define GAME_H

typedef struct {
    int x, y;
} Player;

typedef struct {
    Player player;
    int map_w, map_h;
} GameState;

void game_init(GameState *g, int map_w, int map_h);
void game_move_player(GameState *g, int dx, int dy);

#endif