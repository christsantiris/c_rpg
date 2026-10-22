#ifndef APPROACH_CHECKS_H
#define APPROACH_CHECKS_H

#include "test_utils.h"
#include "game/approach.h"
#include "game/actions.h"

static inline void walk_return_approach(GameState *g) {
    ASSERT("the area exit returns to the safe approach path", g->location == LOCATION_APPROACH && g->enemy_count == 0);
    int steps = 0;
    const TownApproach *route = approach_route(g);
    int dungeon_road = route && route->area == LOCATION_DUNGEON;
    while (g->location == LOCATION_APPROACH && steps++ < APPROACH_W + APPROACH_BRANCH_LENGTH) {
        int dx = (g->map.stairs_up_x > g->player.x) - (g->map.stairs_up_x < g->player.x);
        int dy = (g->map.stairs_up_y > g->player.y) - (g->map.stairs_up_y < g->player.y);
        if (dungeon_road) {
            if (dy) {
                dx = 0;
            }
        } else if (g->map.stairs_up_x == g->map.stairs_down_x && dx) {
            dy = 0;
        } else if (g->map.stairs_up_y == g->map.stairs_down_y && dy) {
            dx = 0;
        }
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
    }
    ASSERT("walking the return path reaches the town gate", g->location != LOCATION_APPROACH);
}

static inline void walk_approach_branch(GameState *g, int boss) {
    int x;
    int y;
    const TownApproach *route = approach_route(g);
    Location destination = route ? (boss ? route->area : approach_shortcut_town(route)) : LOCATION_TOWN;
    int open = boss ? approach_boss_exit(g, &x, &y) : approach_shortcut_exit(g, &x, &y);
    ASSERT("the unlocked approach exposes a physical shortcut branch", open);
    if (!open) {
        return;
    }
    int vertical = g->map.stairs_up_x == g->map.stairs_down_x;
    int jx = vertical ? g->map.stairs_up_x : x;
    int jy = vertical ? y : g->map.stairs_up_y;
    for (int step = 0; step < APPROACH_LENGTH && (g->player.x != jx || g->player.y != jy); step++) {
        int dx = (jx > g->player.x) - (jx < g->player.x);
        int dy = (jy > g->player.y) - (jy < g->player.y);
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
    }
    for (int step = 0; g->location == LOCATION_APPROACH && step < APPROACH_BRANCH_LENGTH; step++) {
        int dx = (x > g->player.x) - (x < g->player.x);
        int dy = (y > g->player.y) - (y < g->player.y);
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
    }
    ASSERT("the crossroads branch reaches its destination directly", g->location == destination && (!boss || g->level == 4));
}

static inline void walk_shortcut_approach(GameState *g) {
    walk_approach_branch(g, 0);
}

static inline void walk_boss_approach(GameState *g) {
    walk_approach_branch(g, 1);
}

static inline void walk_town_approach(GameState *g) {
    ASSERT("town departure begins on a safe approach path", g->location == LOCATION_APPROACH && g->enemy_count == 0);
    int steps = 0;
    const TownApproach *route = approach_route(g);
    int dungeon_road = route && route->area == LOCATION_DUNGEON;
    while (g->location == LOCATION_APPROACH && steps++ < APPROACH_LENGTH + 4) {
        int dx = (g->map.stairs_down_x > g->player.x) - (g->map.stairs_down_x < g->player.x);
        int dy = (g->map.stairs_down_y > g->player.y) - (g->map.stairs_down_y < g->player.y);
        if (dungeon_road && dx) {
            dy = 0;
        }
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
    }
    ASSERT("walking the approach reaches the enemy area entrance", g->location != LOCATION_APPROACH);
}

#endif
