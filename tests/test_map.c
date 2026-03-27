#include "test_utils.h"
#include "../src/game/game.h"

void test_map(void) {
    printf("Map border tests:\n");

    GameState g;
    game_init(&g);

    ASSERT("player does not start on top border",    g.player.y != 0);
    ASSERT("player does not start on bottom border", g.player.y != MAP_H - 1);
    ASSERT("player does not start on left border",   g.player.x != 0);
    ASSERT("player does not start on right border",  g.player.x != MAP_W - 1);

    // Walk all the way left
    GameState g2;
    game_init(&g2);
    for (int i = 0; i < MAP_W; i++) game_move_player(&g2, -1, 0);
    ASSERT("player cannot reach x=0", g2.player.x >= 1);

    // Walk all the way up
    for (int i = 0; i < MAP_H; i++) game_move_player(&g2, 0, -1);
    ASSERT("player cannot reach y=0", g2.player.y >= 1);

    // Walk all the way right
    for (int i = 0; i < MAP_W; i++) game_move_player(&g2, 1, 0);
    ASSERT("player cannot reach x=MAP_W-1", g2.player.x <= MAP_W - 2);

    // Walk all the way down
    for (int i = 0; i < MAP_H; i++) game_move_player(&g2, 0, 1);
    ASSERT("player cannot reach y=MAP_H-1", g2.player.y <= MAP_H - 2);
}