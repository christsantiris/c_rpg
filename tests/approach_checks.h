#ifndef APPROACH_CHECKS_H
#define APPROACH_CHECKS_H

#include "test_utils.h"
#include "game/approach.h"
#include "game/actions.h"

static inline void walk_return_approach(GameState *g) {
    ASSERT("the area exit returns to the safe approach path", g->location == LOCATION_APPROACH && g->enemy_count == 0);
    int steps = 0;
    while (g->location == LOCATION_APPROACH && steps++ < APPROACH_LENGTH) {
        int dx = (g->map.stairs_up_x > g->player.x) - (g->map.stairs_up_x < g->player.x);
        int dy = (g->map.stairs_up_y > g->player.y) - (g->map.stairs_up_y < g->player.y);
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
    }
    ASSERT("walking the return path reaches the town gate", g->location != LOCATION_APPROACH);
}

static inline void walk_town_approach(GameState *g) {
    ASSERT("town departure begins on a safe approach path", g->location == LOCATION_APPROACH && g->enemy_count == 0);
    int steps = 0;
    while (g->location == LOCATION_APPROACH && steps++ < APPROACH_LENGTH) {
        int dx = (g->map.stairs_down_x > g->player.x) - (g->map.stairs_down_x < g->player.x);
        int dy = (g->map.stairs_down_y > g->player.y) - (g->map.stairs_down_y < g->player.y);
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
    }
    ASSERT("walking the approach reaches the enemy area entrance", g->location != LOCATION_APPROACH);
}

#endif
