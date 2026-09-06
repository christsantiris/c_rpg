#include "test_utils.h"
#include "../src/game/game.h"

void test_map(void) {
    printf("Map border tests:\n");

    GameState g;
    game_init(&g);

    ASSERT("new map begins unexplored",
        !map_is_explored(&g.map, g.player.x, g.player.y));
    map_mark_explored(&g.map, g.player.x, g.player.y);
    ASSERT("seen map tile remains explored",
        map_is_explored(&g.map, g.player.x, g.player.y));

    ASSERT("player does not start on top border",    g.player.y != 0);
    ASSERT("player does not start on bottom border", g.player.y != MAP_H - 1);
    ASSERT("player does not start on left border",   g.player.x != 0);
    ASSERT("player does not start on right border",  g.player.x != MAP_W - 1);

    int explored_x = g.player.x;
    int explored_y = g.player.y;
    game_descend(&g);
    game_ascend(&g);
    ASSERT("exploration follows a cached dungeon floor",
        map_is_explored(&g.map, explored_x, explored_y));

    // Walk all the way left
    GameState g2;
    game_init(&g2);
    g2.player.y = 5;
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
