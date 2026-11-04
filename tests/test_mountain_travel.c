#include "expedition_fixture.h"
#include "approach_checks.h"
#include "test_utils.h"
#include "shortcut_checks.h"
#include "game/game.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

#define MOUNTAIN_TRAVEL_SLOT 99124
static GameState mountain;
static GameState restored;

static void mountain_step(GameState *g, int reverse) {
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

static int mountain_king(const GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].type == ENEMY_MOUNTAIN_GOBLIN_KING) {
            return i;
        }
    }
    return -1;
}

static void mountain_kill(GameState *g, int index) {
    Enemy *enemy = &g->enemies[index];
    enemy->hp = 1;
    g->player.x = enemy->x - 1;
    g->player.y = enemy->y;
    g->map.tiles[g->player.y][g->player.x] = TILE_MOUNTAIN_FLOOR;
    for (int tries = 0; enemy->active && tries < 30; tries++) {
        action_resolve_player(g, (Action){ACTION_MOVE, enemy->x, enemy->y});
    }
    ASSERT("mountain target dies through normal melee combat", !enemy->active);
}

static void mountain_walk_road(GameState *g, int west) {
    int y = g->map.stairs_up_y;
    for (int step = 0; g->location == LOCATION_APPROACH && step < APPROACH_W + APPROACH_H; step++) {
        int dy = (y > g->player.y) - (y < g->player.y);
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + (dy ? 0 : west ? -1 : 1), g->player.y + dy});
    }
    walk_into_arrived_town(g);
}

static void mountain_crossing(int return_home) {
    const int stillbury = 0;
    mountain.player.player_class = CLASS_WARRIOR;
    game_init(&mountain);
    srand(2701 + stillbury);
    game_leave_mountains(&mountain, stillbury ? LOCATION_TOWN2 : LOCATION_TOWN, 0);
    mountain.dain_quest_state = 1;
    mountain.watchfire_quest_state = 1;
    Location home = mountain.location;
    action_resolve_player(&mountain, (Action){ACTION_MOVE, stillbury ? TOWN_W - 1 : 0, 12});
    finish_town_departure(&mountain);
    ASSERT("Oakhaven’s gate leads to a safe mountain road", mountain.location == LOCATION_APPROACH && mountain.enemy_count == 0);
    int x;
    int y;
    ASSERT("the same mountain landmark stands at the same road position from Oakhaven", mountain.map.stairs_down_x == APPROACH_W / 2 && mountain.map.stairs_down_y == APPROACH_H / 2 - 2);
    ASSERT("mountain road has no town branch or direct boss shortcut", !approach_shortcut_exit(&mountain, &x, &y) && !approach_boss_exit(&mountain, &x, &y));
    ASSERT("the road beyond the mountain is locked before victory", !approach_mountain_town_exit(&mountain, &x, &y));
    walk_town_approach(&mountain);
    ASSERT("Oakhaven always enters the foot of the mountain at level one", mountain.location == LOCATION_MOUNTAINS && mountain.level == 1 && mountain.mountain_entry_town == home);
    mountain_step(&mountain, 1);
    ASSERT("retreating at the foot returns to the pathway", mountain.location == LOCATION_APPROACH);
    walk_return_approach(&mountain);
    ASSERT("walking back reaches the original town", mountain.location == home);
    game_enter_mountains(&mountain);
    for (int level = 1; level < MOUNTAIN_DEPTH; level++) {
        ASSERT("climb visits levels in ascending order", mountain.level == level && mountain_king(&mountain) < 0);
        int fragments = 0;
        for (int i = 0; i < mountain.enemy_count; i++) {
            if (mountain.enemies[i].active && mountain.enemies[i].dain_fragment) {
                ASSERT("Dain's halves are on mountain levels one and two", mountain.enemies[i].dain_fragment == (level == 1 ? DAIN_FRAGMENT_ARCHER : DAIN_FRAGMENT_WARCHIEF));
                fragments++;
                mountain_kill(&mountain, i);
            }
        }
        ASSERT("each quest level has exactly one pending bearer", fragments == (level <= 2));
        int score = mountain.score;
        mountain_step(&mountain, 0);
        ASSERT("first climbs award the next level's travel score", mountain.score == score + 100 * map_mountain_difficulty(mountain.level));
    }
    int king = mountain_king(&mountain);
    ASSERT("all map fragments are recoverable before the summit", mountain.dain_map_fragments == DAIN_MAP_COMPLETE && mountain.dain_quest_state == 2);
    ASSERT("the Goblin King occupies level five", mountain.level == 5 && king >= 0);
    int fires = 0;
    for (int row = 0; row < MAP_H; row++) {
        for (int col = 0; col < MAP_W; col++) {
            fires += mountain.map.tiles[row][col] == TILE_WATCHFIRE_COLD;
        }
    }
    ASSERT("Veyra's mountain watchfire is on level five", fires == 1);
    mountain_step(&mountain, 0);
    ASSERT("the living King seals the summit exit", mountain.location == LOCATION_MOUNTAINS && mountain.level == 5);
    if (king < 0) {
        return;
    }
    mountain_kill(&mountain, king);
    ASSERT("King victory has no shortcut prompt", !game_shortcut_prompt_active(&mountain));
    int shortcuts = 0;
    for (int row = 0; row < MAP_H; row++) {
        for (int col = 0; col < MAP_W; col++) {
            shortcuts += mountain.map.tiles[row][col] == TILE_MOUNTAIN_SHORTCUT;
        }
    }
    ASSERT("King victory creates no shortcut tile", shortcuts == 0);
    mountain_step(&mountain, 0);
    ASSERT("the summit exit returns to the straight safe road", mountain.location == LOCATION_APPROACH && mountain.enemy_count == 0 && mountain.player.y == mountain.map.stairs_up_y);
    ASSERT("victory opens the Sunscar gate", approach_mountain_town_exit(&mountain, &x, &y));
    ASSERT("the unlocked road stays one tile wide", mountain.map.tiles[y][mountain.player.x] == TILE_TOWN_PATH && mountain.map.tiles[y + 1][mountain.player.x + 1] == TILE_TOWN_FLOOR);
    ASSERT("the unlocked road and quest progress survive saving", save_game(&mountain, MOUNTAIN_TRAVEL_SLOT) && load_game(&restored, MOUNTAIN_TRAVEL_SLOT) && restored.dain_map_fragments == DAIN_MAP_COMPLETE && approach_mountain_town_exit(&restored, &x, &y));
    mountain = restored;
    if (return_home) {
        mountain_walk_road(&mountain, stillbury);
        ASSERT("the road permits walking home", mountain.location == home);
    } else {
        walk_approach_to(&mountain, 0, APPROACH_H / 2);
        ASSERT("the open pass continues west onto the Sunscar road", approach_sunscar_road(&mountain) && mountain.player.x == APPROACH_W - 2 && mountain.player.last_dx == -1);
        walk_approach_to(&mountain, mountain.map.stairs_down_x, mountain.map.stairs_down_y);
        ASSERT("the Sunscar road's desert entrance leads into the first stage", mountain.location == LOCATION_DESERT && mountain.level == 1);
        mountain.player.x = mountain.map.stairs_up_x - 1;
        mountain.player.y = mountain.map.stairs_up_y;
        action_resolve_player(&mountain, (Action){ACTION_MOVE, mountain.map.stairs_up_x, mountain.map.stairs_up_y});
        ASSERT("Sunscar's real eastern entrance returns to the Sunscar road outside its desert entrance", approach_sunscar_road(&mountain) &&
            mountain.player.x == mountain.map.stairs_down_x && mountain.player.y == mountain.map.stairs_down_y + 1);
        walk_approach_to(&mountain, mountain.map.stairs_up_x, mountain.map.stairs_up_y);
        ASSERT("the Sunscar road's east end returns to the mountain pass", mountain.location == LOCATION_APPROACH && mountain.approach_route == 2 && mountain.player.x == 1 && mountain.player.last_dx == 1);
        mountain_walk_road(&mountain, 0);
    }
    Location target = LOCATION_TOWN;
    action_resolve_player(&mountain, (Action){ACTION_MOVE, target == LOCATION_TOWN ? 0 : TOWN_W - 1, 12});
    walk_town_approach(&mountain);
    ASSERT("reentry after victory still starts at level one", mountain.location == LOCATION_MOUNTAINS && mountain.level == 1);
    game_open_town_portal(&mountain);
    ASSERT("mountain portals anchor to the town of entry", mountain.location == target && mountain.mountain_portal_town == target);
    int py = 12;
    int px = target == LOCATION_TOWN ? 2 : TOWN_W - 3;
    ASSERT("the mountain portal appears beside the correct gate", mountain.map.tiles[py][px] != TILE_PORTAL);
    action_resolve_player(&mountain, (Action){ACTION_MOVE, px, py});
    ASSERT("walking the former mountain portal tile stays in town", mountain.location == target && !mountain.portal_active);
    test_restore_cached_floor(&mountain);
    ASSERT("the mountain cache retains its stage and original entry town", mountain.location == LOCATION_MOUNTAINS && mountain.level == 1 && mountain.mountain_entry_town == target);
    remove("saves/savegame_99124.json");
}

void test_mountain_travel(void) {
    printf("Forward mountain climb and road tests:\n");
    ASSERT("mountains have five levels with the King and watchfire at the summit", MOUNTAIN_DEPTH == 5 && MOUNTAIN_BOSS_LEVEL == 5 && WATCHFIRE_MOUNTAINS_LEVEL == 5);
    ASSERT("mountain travel test slot is unused", !save_exists(MOUNTAIN_TRAVEL_SLOT));
    for (int home = 0; home < 2; home++) {
        mountain_crossing(home);
    }
}
