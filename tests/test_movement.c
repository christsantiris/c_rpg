#include "test_utils.h"
#include "../src/game/game.h"

void test_movement(void) {
    printf("Movement tests:\n");

    GameState g;
    game_init(&g);
    // Find a walkable position with room to move in all directions
    int start_x = -1, start_y = -1;
    for (int y = 2; y < MAP_H - 2 && start_x == -1; y++) {
        for (int x = 2; x < MAP_W - 2 && start_x == -1; x++) {
            if (map_is_walkable(&g.map, x, y) &&
                map_is_walkable(&g.map, x+1, y) &&
                map_is_walkable(&g.map, x-1, y) &&
                map_is_walkable(&g.map, x, y+1) &&
                map_is_walkable(&g.map, x, y-1)) {
                start_x = x;
                start_y = y;
            }
        }
    }
    ASSERT("found valid test position", start_x != -1);
    g.player.x = start_x;
    g.player.y = start_y;

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
    game_init(&g);
    g.player.x = 1;
    g.player.y = MAP_H / 2;
    game_move_player(&g, -1, 0);
    ASSERT("player cannot move into left border", g.player.x == 1);

    // Wall collision - right border
    game_init(&g);
    printf("DEBUG after game_init: equipped_armor=%d defense=%d\n",
        g.equipped_armor, g.player.defense);
    g.player.x = MAP_W - 2;
    g.player.y = MAP_H / 2;
    game_move_player(&g, 1, 0);
    ASSERT("player cannot move into right border", g.player.x == MAP_W - 2);

    // Wall collision - top border
    game_init(&g);
    g.player.x = MAP_W / 2;
    g.player.y = 1;
    game_move_player(&g, 0, -1);
    ASSERT("player cannot move into top border", g.player.y == 1);

    // Wall collision - bottom border
    game_init(&g);
    g.player.x = MAP_W / 2;
    g.player.y = MAP_H - 2;
    game_move_player(&g, 0, 1);
    ASSERT("player cannot move into bottom border", g.player.y == MAP_H - 2);
}
