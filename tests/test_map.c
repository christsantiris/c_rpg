#include "test_utils.h"
#include "../src/game/game.h"

void test_map(void) {
    printf("Map border tests:\n");

    GameState g;
    game_init(&g, 20, 20);

    // Top border is all wall
    int top_ok = 1;
    for (int x = 0; x < 20; x++)
        if (g.player.x == x && g.player.y == 0) { top_ok = 0; break; }
    ASSERT("player does not start on top border", g.player.y != 0);
    ASSERT("player does not start on bottom border", g.player.y != 19);
    ASSERT("player does not start on left border", g.player.x != 0);
    ASSERT("player does not start on right border", g.player.x != 19);

    // Verify border cells are not reachable
    GameState g2;
    game_init(&g2, 20, 20);
    g2.player.x = 1; g2.player.y = 1;

    // Walk all the way left
    for (int i = 0; i < 20; i++) game_move_player(&g2, -1, 0);
    ASSERT("player cannot reach x=0", g2.player.x >= 1);

    // Walk all the way up
    for (int i = 0; i < 20; i++) game_move_player(&g2, 0, -1);
    ASSERT("player cannot reach y=0", g2.player.y >= 1);

    // Walk all the way right
    g2.player.x = 18;
    for (int i = 0; i < 20; i++) game_move_player(&g2, 1, 0);
    ASSERT("player cannot reach x=map_w-1", g2.player.x <= 18);

    // Walk all the way down
    g2.player.y = 18;
    for (int i = 0; i < 20; i++) game_move_player(&g2, 0, 1);
    ASSERT("player cannot reach y=map_h-1", g2.player.y <= 18);
}