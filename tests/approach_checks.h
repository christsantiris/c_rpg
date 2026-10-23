#ifndef APPROACH_CHECKS_H
#define APPROACH_CHECKS_H

#include "test_utils.h"
#include "town_road_checks.h"
#include "game/approach.h"
#include "game/elderwood.h"
#include "game/actions.h"

// Entrance landmarks use the forest layout from side gates and the mountain layout otherwise.
static inline int approach_check_entrance_vertical(const TownApproach *route) {
    return route && approach_entrance_rows(route->area) && (route->gate_y == 0 || route->gate_y == TOWN_H - 1);
}

static inline int approach_check_entrance_horizontal(const TownApproach *route) {
    return route && approach_entrance_rows(route->area) && !approach_check_entrance_vertical(route);
}

static inline void walk_return_approach(GameState *g) {
    if (g->location == LOCATION_TOWN_ROAD) {
        walk_into_arrived_town(g);
        return;
    }
    ASSERT("the area exit returns to the safe approach path", g->location == LOCATION_APPROACH && g->enemy_count == 0);
    int steps = 0;
    const TownApproach *route = approach_route(g);
    if (northern_approach(g)) {
        while (g->location == LOCATION_APPROACH && steps++ < APPROACH_W + APPROACH_H) {
            int dx = (NORTH_ROAD_X > g->player.x) - (NORTH_ROAD_X < g->player.x);
            int dy = dx ? 0 : (g->map.stairs_up_y > g->player.y) - (g->map.stairs_up_y < g->player.y);
            action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
        }
        walk_into_arrived_town(g);
        ASSERT("northern return path reaches its original town", g->location == (route ? route->town : LOCATION_CASTLE));
        return;
    }
    int dungeon_road = route && (route->area == LOCATION_DUNGEON || route->area == LOCATION_FOREST || approach_check_entrance_horizontal(route));
    int mountain_road = route && (route->area == LOCATION_MOUNTAINS || route->area == LOCATION_SWAMP || approach_check_entrance_vertical(route));
    while (g->location == LOCATION_APPROACH && steps++ < APPROACH_W + APPROACH_BRANCH_LENGTH) {
        int dx = (g->map.stairs_up_x > g->player.x) - (g->map.stairs_up_x < g->player.x);
        int dy = (g->map.stairs_up_y > g->player.y) - (g->map.stairs_up_y < g->player.y);
        if (mountain_road && dx) {
            dy = 0;
        } else if (dungeon_road) {
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
    walk_into_arrived_town(g);
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

static inline void walk_forest_road(GameState *g) {
    finish_town_departure(g);
    int x;
    int y;
    int open = approach_forest_town_exit(g, &x, &y);
    ASSERT("forest victory opens the other town gate", open);
    for (int step = 0; open && g->location == LOCATION_APPROACH && step < APPROACH_W + 4; step++) {
        int dy = (y > g->player.y) - (y < g->player.y);
        int dx = dy ? 0 : (x > g->player.x) - (x < g->player.x);
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
    }
    walk_into_arrived_town(g);
}

static inline void walk_to_labyrinth(GameState *g) {
    finish_town_departure(g);
    for (int step = 0; g->location == LOCATION_APPROACH && step < APPROACH_W + 4; step++) {
        if (g->player.x == APPROACH_LABYRINTH_X && g->player.y == APPROACH_LABYRINTH_Y + 1) {
            break;
        }
        int dx = (APPROACH_LABYRINTH_X > g->player.x) - (APPROACH_LABYRINTH_X < g->player.x);
        int dy = dx ? 0 : (APPROACH_LABYRINTH_Y + 1 > g->player.y) - (APPROACH_LABYRINTH_Y + 1 < g->player.y);
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
    }
    ASSERT("the forest road reaches the labyrinth's south-facing door", g->location == LOCATION_APPROACH && g->player.x == APPROACH_LABYRINTH_X && g->player.y == APPROACH_LABYRINTH_Y + 1);
}

static inline void walk_swamp_road(GameState *g) {
    finish_town_departure(g);
    int x;
    int y;
    int open = approach_swamp_town_exit(g, &x, &y);
    ASSERT("swamp victory opens the other town gate", open);
    for (int step = 0; open && g->location == LOCATION_APPROACH && step < APPROACH_W + 4; step++) {
        int dx = (x > g->player.x) - (x < g->player.x);
        int dy = dx ? 0 : (y > g->player.y) - (y < g->player.y);
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
    }
    walk_into_arrived_town(g);
}

static inline void walk_shortcut_approach(GameState *g) {
    walk_approach_branch(g, 0);
}

static inline void walk_boss_approach(GameState *g) {
    walk_approach_branch(g, 1);
}

static inline void walk_town_approach(GameState *g) {
    finish_town_departure(g);
    ASSERT("town departure begins on a safe approach path", g->location == LOCATION_APPROACH && g->enemy_count == 0);
    int steps = 0;
    const TownApproach *route = approach_route(g);
    int dungeon_road = route && (route->area == LOCATION_DUNGEON || route->area == LOCATION_FOREST || approach_check_entrance_horizontal(route));
    int mountain_road = route && (route->area == LOCATION_MOUNTAINS || route->area == LOCATION_SWAMP || approach_check_entrance_vertical(route));
    while (g->location == LOCATION_APPROACH && steps++ < APPROACH_W + 4) {
        int dx = (g->map.stairs_down_x > g->player.x) - (g->map.stairs_down_x < g->player.x);
        int dy = (g->map.stairs_down_y > g->player.y) - (g->map.stairs_down_y < g->player.y);
        if (mountain_road && g->player.y != g->map.stairs_down_y + 1 && g->player.x == g->map.stairs_up_x) {
            dx = 0;
            dy = (g->map.stairs_down_y + 1 > g->player.y) - (g->map.stairs_down_y + 1 < g->player.y);
        } else if (mountain_road && dx) {
            dy = 0;
        } else if (dungeon_road && dx) {
            dy = 0;
        }
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
    }
    ASSERT("walking the approach reaches the enemy area entrance", g->location != LOCATION_APPROACH);
}

#endif
