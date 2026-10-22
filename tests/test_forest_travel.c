#include "approach_checks.h"
#include "test_utils.h"
#include "shortcut_checks.h"
#include "game/game.h"
#include "game/actions.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

#define FOREST_TRAVEL_SLOT 99031
static GameState forest;
static GameState restored;

static void forest_landmark(GameState *g) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_FOREST_LANDMARK) {
                g->player.x = x;
                g->player.y = y;
                action_resolve_player(g, (Action){ACTION_MOVE, x, y});
                return;
            }
        }
    }
}

static void forest_step(GameState *g, int reverse) {
    int x = reverse ? g->map.stairs_up_x : g->map.stairs_down_x;
    int y = reverse ? g->map.stairs_up_y : g->map.stairs_down_y;
    g->player.x = x;
    g->player.y = y;
    if (x == 1) {
        x = 0;
    } else if (x == MAP_W - 2) {
        x = MAP_W - 1;
    } else if (y == 1) {
        y = 0;
    } else {
        y = MAP_H - 1;
    }
    action_resolve_player(g, (Action){ACTION_MOVE, x, y});
}

static int forest_boss(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].type == ENEMY_FOREST_NECROMANCER) {
            return i;
        }
    }
    return -1;
}

static void forest_rescue(GameState *g) {
    int found = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_FOREST_WARDEN) {
                found++;
                game_rescue_forest_warden(g, x, y);
            }
        }
    }
    ASSERT("Alder's wardens occupy forest levels two, three and four", found == (g->level >= 2 && g->level <= 4));
}

static void forest_walk_road(GameState *g, int west) {
    for (int step = 0; g->location == LOCATION_APPROACH && step < APPROACH_W + 4; step++) {
        int dy = (g->map.stairs_up_y > g->player.y) - (g->map.stairs_up_y < g->player.y);
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + (dy ? 0 : west ? -1 : 1), g->player.y + dy});
    }
    walk_into_arrived_town(g);
}

static void test_forest_crossing(int stillbury, int home) {
    srand(809 + stillbury + home * 2);
    game_init(&forest);
    if (stillbury) {
        game_enter_town2(&forest);
    }
    Location origin = forest.location;
    forest.alder_quest_state = 1;
    int gate_x = stillbury ? TOWN_W - 1 : 0;
    action_resolve_player(&forest, (Action){ACTION_MOVE, gate_x, 12});
    finish_town_departure(&forest);
    ASSERT("either forest gate first opens a safe approach", forest.location == LOCATION_APPROACH && forest.enemy_count == 0);
    int x;
    int y;
    ASSERT("forest has no town shortcut or boss branch", !approach_shortcut_exit(&forest, &x, &y) && !approach_boss_exit(&forest, &x, &y));
    ASSERT("the far town stays locked before Necromancer victory", !approach_forest_town_exit(&forest, &x, &y));
    ASSERT("the forest landmark has the same coordinates from either town", forest.map.stairs_down_x == APPROACH_W / 2 && forest.map.stairs_down_y == APPROACH_H / 2 - 2);
    walk_town_approach(&forest);
    ASSERT("both forest entrances start at level one", forest.location == LOCATION_FOREST && forest.level == 1 && forest.forest_entry_town == origin && forest.player.x == forest.map.stairs_up_x && forest.player.y == forest.map.stairs_up_y);
    forest_step(&forest, 1);
    ASSERT("retreat at forest one returns onto the path", forest.location == LOCATION_APPROACH);
    walk_return_approach(&forest);
    ASSERT("walking the path returns through the original town gate", forest.location == origin);
    game_enter_forest(&forest);
    for (int level = 1; level < FOREST_DEPTH; level++) {
        ASSERT("forest progression moves forward without an early boss", forest.level == level && forest_boss(&forest) < 0);
        forest_rescue(&forest);
        forest.dialogue_active = 0;
        forest_landmark(&forest);
        int score = forest.score;
        forest_step(&forest, 0);
        ASSERT("new forest levels grant their travel reward once", forest.level == level + 1 && forest.score == score + map_forest_difficulty(forest.level) * 100);
    }
    ASSERT("all three wardens are rescued before the final grove", forest.alder_wardens_rescued == 7 && forest.alder_quest_state == 2);
    int boss = forest_boss(&forest);
    ASSERT("the Necromancer waits on forest five", forest.level == 5 && boss >= 0);
    forest_landmark(&forest);
    forest_step(&forest, 0);
    ASSERT("the living Necromancer seals the final exit", forest.location == LOCATION_FOREST && forest.level == 5);
    forest_step(&forest, 1);
    ASSERT("the living Necromancer permits ordinary backtracking", forest.location == LOCATION_FOREST && forest.level == 4);
    forest_landmark(&forest);
    forest_step(&forest, 0);
    boss = forest_boss(&forest);
    if (boss < 0) {
        return;
    }
    Enemy *enemy = &forest.enemies[boss];
    enemy->hp = 1;
    forest.player.attack = 10000;
    forest.player.x = enemy->x - 1;
    forest.player.y = enemy->y;
    forest.map.tiles[forest.player.y][forest.player.x] = TILE_FOREST_FLOOR;
    for (int tries = 0; enemy->active && tries < 30; tries++) {
        action_resolve_player(&forest, (Action){ACTION_MOVE, enemy->x, enemy->y});
    }
    ASSERT("normal combat records victory without a shortcut prompt", !enemy->active && (forest.defeated_bosses & (1 << LOCATION_FOREST)) && !game_shortcut_prompt_active(&forest));
    int markers = 0;
    for (int row = 0; row < MAP_H; row++) {
        for (int col = 0; col < MAP_W; col++) {
            markers += forest.map.tiles[row][col] == TILE_FOREST_SHORTCUT;
        }
    }
    ASSERT("the boss never creates a forest shortcut", markers == 0);
    forest_step(&forest, 0);
    ASSERT("the boss exit returns to the straight town road", forest.location == LOCATION_APPROACH && forest.player.y == APPROACH_H / 2 && forest.enemy_count == 0);
    ASSERT("victory opens the far town and a one-tile road", approach_forest_town_exit(&forest, &x, &y) && forest.map.tiles[y][forest.player.x] == TILE_TOWN_PATH && forest.map.tiles[y + 1][forest.player.x + 1] == TILE_TOWN_FLOOR);
    ASSERT("road unlock and rescues survive saving", save_game(&forest, FOREST_TRAVEL_SLOT) && load_game(&restored, FOREST_TRAVEL_SLOT) && approach_forest_town_exit(&restored, &x, &y) && restored.alder_wardens_rescued == 7);
    forest = restored;
    forest_walk_road(&forest, home ? stillbury : !stillbury);
    Location destination = home ? origin : stillbury ? LOCATION_TOWN : LOCATION_TOWN2;
    ASSERT("the road leads home or onward in the correct direction", forest.location == destination);
    action_resolve_player(&forest, (Action){ACTION_MOVE, destination == LOCATION_TOWN ? 0 : TOWN_W - 1, 12});
    walk_town_approach(&forest);
    ASSERT("revisiting from either town always starts at forest one", forest.location == LOCATION_FOREST && forest.level == 1);
    forest_landmark(&forest);
    forest_step(&forest, 0);
    int px = forest.player.x;
    int py = forest.player.y;
    forest.enemies[0].hp = 7;
    game_open_town_portal(&forest);
    ASSERT("Return to Town remembers the forest entrance town", forest.location == destination && forest.forest_portal_town == destination);
    ASSERT("the forest portal survives saving", save_game(&forest, FOREST_TRAVEL_SLOT) && load_game(&restored, FOREST_TRAVEL_SLOT));
    game_use_town_portal(&restored);
    ASSERT("the portal restores the level, position and wounded enemy", restored.location == LOCATION_FOREST && restored.level == 2 && restored.player.x == px && restored.player.y == py && restored.enemies[0].hp == 7 && restored.forest_entry_town == destination);
    remove("saves/savegame_99031.json");
}

void test_forest_travel(void) {
    printf("Forward Haunted Forest adventure and town road tests:\n");
    ASSERT("the forest has five levels ending at the Necromancer", FOREST_DEPTH == 5 && FOREST_BOSS_LEVEL == 5);
    ASSERT("forest travel test slot is unused", !save_exists(FOREST_TRAVEL_SLOT));
    for (int stillbury = 0; stillbury < 2; stillbury++) {
        for (int home = 0; home < 2; home++) {
            test_forest_crossing(stillbury, home);
        }
    }
}
