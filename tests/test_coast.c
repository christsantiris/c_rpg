#include "test_utils.h"
#include "../src/game/game.h"

static Action coast_edge_action(const Map *m, int entrance) {
    int x = entrance ? m->stairs_up_x : m->stairs_down_x;
    int y = entrance ? m->stairs_up_y : m->stairs_down_y;
    Action action = {ACTION_MOVE, x, y};
    if (x == 1) {
        action.target_x--;
    } else if (x == MAP_W - 2) {
        action.target_x++;
    } else if (y == 1) {
        action.target_y--;
    } else {
        action.target_y++;
    }
    return action;
}

static int coast_enemy(EnemyType type) {
    return type >= ENEMY_ILLUSION && type <= ENEMY_DROWNED_QUEEN;
}

void test_coast(void) {
    printf("Sunken Coast tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);

    Action south = {ACTION_MOVE, 20, TOWN_H - 1};
    g.player.x = 20;
    g.player.y = TOWN_H - 2;
    action_resolve_player(&g, south);
    ASSERT("south town exit enters coast", g.location == LOCATION_COAST);
    ASSERT("coast begins on stage one", g.level == 1);
    ASSERT("coast uses blue ruin terrain",
        g.map.tiles[g.player.y][g.player.x] == TILE_COAST_FLOOR);

    for (int level = 1; level <= COAST_DEPTH; level++) {
        g.location = LOCATION_COAST;
        g.level = level;
        map_generate_coast(&g.map, level);
        enemies_spawn(&g);
        int roster_ok = 1;
        int queen_count = 0;
        for (int i = 0; i < g.enemy_count; i++) {
            if (!coast_enemy(g.enemies[i].type)) {
                roster_ok = 0;
            }
            if (g.enemies[i].type == ENEMY_DROWNED_QUEEN) {
                queen_count++;
            }
        }
        ASSERT("coast stages use only coast enemies", roster_ok);
        ASSERT("Drowned Queen appears only on stage eight",
            queen_count == (level == COAST_DEPTH ? 1 : 0));
        ASSERT("coast entry and exit use different edges",
            !((g.map.stairs_up_x == 1 && g.map.stairs_down_x == 1) ||
            (g.map.stairs_up_x == MAP_W - 2 &&
                g.map.stairs_down_x == MAP_W - 2) ||
            (g.map.stairs_up_y == 1 && g.map.stairs_down_y == 1) ||
            (g.map.stairs_up_y == MAP_H - 2 &&
                g.map.stairs_down_y == MAP_H - 2)));
    }

    g.level = 1;
    map_generate_coast(&g.map, 1);
    enemies_spawn(&g);
    Action exit = coast_edge_action(&g.map, 0);
    action_resolve_player(&g, exit);
    ASSERT("coast advances without clearing enemies", g.level == 2);

    g.level = COAST_DEPTH;
    map_generate_coast(&g.map, COAST_DEPTH);
    enemies_spawn(&g);
    exit = coast_edge_action(&g.map, 0);
    action_resolve_player(&g, exit);
    ASSERT("Drowned Queen blocks the final exit",
        g.location == LOCATION_COAST);
    for (int i = 0; i < g.enemy_count; i++) {
        if (g.enemies[i].type == ENEMY_DROWNED_QUEEN) {
            g.enemies[i].active = 0;
        }
    }
    action_resolve_player(&g, exit);
    ASSERT("defeating Drowned Queen returns to town",
        g.location == LOCATION_TOWN);
    ASSERT("coast completion returns at south road",
        g.player.x == 20 && g.player.y == TOWN_H - 2);
}
