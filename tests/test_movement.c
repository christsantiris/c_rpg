#include "test_utils.h"
#include "../src/game/game.h"

void test_movement(void) {
    printf("Movement tests:\n");

    GameState g;
    game_init(&g, 20, 20);
    int start_x = g.player.x;
    int start_y = g.player.y;

    // Basic movement
    game_move_player(&g, 1, 0);
    ASSERT("move right increases x by 1", g.player.x == start_x + 1);

    game_move_player(&g, -1, 0);
    ASSERT("move left decreases x by 1", g.player.x == start_x);

    game_move_player(&g, 0, 1);
    ASSERT("move down increases y by 1", g.player.y == start_y + 1);

    game_move_player(&g, 0, -1);
    ASSERT("move up decreases y by 1", g.player.y == start_y);

    // Wall collision - left border
    GameState g2;
    game_init(&g2, 20, 20);
    g2.player.x = 1;
    g2.player.y = 10;
    game_move_player(&g2, -1, 0);
    ASSERT("player cannot move into left border", g2.player.x == 1);

    // Wall collision - right border
    GameState g3;
    game_init(&g3, 20, 20);
    g3.player.x = 18;
    g3.player.y = 10;
    game_move_player(&g3, 1, 0);
    ASSERT("player cannot move into right border", g3.player.x == 18);

    // Wall collision - top border
    GameState g4;
    game_init(&g4, 20, 20);
    g4.player.x = 10;
    g4.player.y = 1;
    game_move_player(&g4, 0, -1);
    ASSERT("player cannot move into top border", g4.player.y == 1);

    // Wall collision - bottom border
    GameState g5;
    game_init(&g5, 20, 20);
    g5.player.x = 10;
    g5.player.y = 18;
    game_move_player(&g5, 0, 1);
    ASSERT("player cannot move into bottom border", g5.player.y == 18);
}