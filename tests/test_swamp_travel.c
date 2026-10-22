#include "approach_checks.h"
#include "test_utils.h"
#include "shortcut_checks.h"
#include "game/game.h"
#include "game/actions.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

static GameState swamp;
static GameState restored;

static void swamp_step(GameState *g, int reverse) {
    int x = reverse ? g->map.stairs_up_x : g->map.stairs_down_x;
    int y = reverse ? g->map.stairs_up_y : g->map.stairs_down_y;
    g->player.x = x + (reverse ? 1 : -1);
    g->player.y = y;
    action_resolve_player(g, (Action){ACTION_MOVE, x, y});

}

static int swamp_boss(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].type == ENEMY_SWAMP_DEMON) {
            return i;
        }
    }
    return -1;
}

static int swamp_mira(GameState *g, int rescue) {
    int x = -1;
    int y = -1;
    for (int row = 0; row < MAP_H; row++) {
        for (int column = 0; column < MAP_W; column++) {
            if (g->map.tiles[row][column] == TILE_SWAMP_DAUGHTER) {
                x = column;
                y = row;
            }
        }
    }
    if (rescue && x >= 0) {
        game_rescue_innkeeper_daughter(g, x, y);
        ASSERT("Mira waits for the vampire captor's defeat", g->innkeeper_quest_state == 1);
        int captor = -1;
        for (int i = 0; i < g->enemy_count; i++) {
            if (g->enemies[i].active && strcmp(g->enemies[i].name, "Vampire Captor") == 0) {
                captor = i;
                g->enemies[i].active = 0;
            }
        }
        ASSERT("Mira's level has its vampire captor", captor >= 0);
        game_rescue_innkeeper_daughter(g, x, y);
        ASSERT("Mira can be rescued from either direction", g->innkeeper_quest_state == 2 && g->map.tiles[y][x] == TILE_SWAMP_FLOOR);
    }
    return x >= 0;
}

static void test_swamp_crossing(int rosemoor, int home) {
    game_init(&swamp);
    srand(1037 + rosemoor + home * 2);
    game_enter_town2(&swamp);
    game_enter_inn(&swamp);
    game_talk_to_innkeeper(&swamp);
    game_handle_quest_offer_key(&swamp, SDL_SCANCODE_Y, 0);
    ASSERT("Bram sends the player to Mira on level three", strstr(swamp.dialogue_text, "level 3") != NULL);
    game_leave_inn(&swamp);
    if (rosemoor) {
        game_enter_town3(&swamp);
    }
    Location origin = swamp.location;
    action_resolve_player(&swamp, (Action){ACTION_MOVE, 20, rosemoor ? TOWN_H - 1 : 0});
    int x;
    int y;
    ASSERT("the swamp's far town is locked before the Demon falls", !approach_swamp_town_exit(&swamp, &x, &y));
    ASSERT("the swamp has no shortcut or boss branch", !approach_shortcut_exit(&swamp, &x, &y) && !approach_boss_exit(&swamp, &x, &y));
    walk_town_approach(&swamp);
    ASSERT("either town entrance starts swamp level one", swamp.location == LOCATION_SWAMP && swamp.level == 1 && swamp.swamp_entry_town == origin);
    swamp_step(&swamp, 1);
    walk_return_approach(&swamp);
    ASSERT("retreat from level one walks back to the original town", swamp.location == origin);
    game_enter_swamp(&swamp);
    for (int level = 1; level < SWAMP_DEPTH; level++) {
        ASSERT("the swamp progresses forward with no early boss", swamp.level == level && swamp_boss(&swamp) < 0);
        ASSERT("Mira and her captor are on swamp three", swamp_mira(&swamp, 1) == (level == SWAMP_RESCUE_LEVEL));
        swamp.dialogue_active = 0;
        int score = swamp.score;
        swamp_step(&swamp, 0);
        ASSERT("first visits reward increasing swamp difficulty", swamp.level == level + 1 && swamp.score == score + map_swamp_difficulty(swamp.level) * 100);
    }
    int boss = swamp_boss(&swamp);
    ASSERT("Mira is rescued before the fifth-level Demon", swamp.innkeeper_quest_state == 2 && swamp.level == 5 && boss >= 0);
    if (boss < 0) {
        return;
    }
    swamp_step(&swamp, 0);
    ASSERT("the living Demon seals the final exit", swamp.location == LOCATION_SWAMP && swamp.level == 5);
    swamp_step(&swamp, 1);
    ASSERT("the living Demon permits backtracking", swamp.location == LOCATION_SWAMP && swamp.level == 4);
    swamp_step(&swamp, 0);
    boss = swamp_boss(&swamp);
    if (boss < 0) {
        return;
    }
    Enemy *demon = &swamp.enemies[boss];
    demon->hp = 1;
    swamp.player.attack = 10000;
    swamp.player.x = demon->x - 1;
    swamp.player.y = demon->y;
    swamp.map.tiles[swamp.player.y][swamp.player.x] = TILE_SWAMP_FLOOR;
    for (int tries = 0; demon->active && tries < 30; tries++) {
        action_resolve_player(&swamp, (Action){ACTION_MOVE, demon->x, demon->y});
    }
    ASSERT("combat unlocks the road without a shortcut prompt", !demon->active && (swamp.defeated_bosses & (1 << LOCATION_SWAMP)) && !game_shortcut_prompt_active(&swamp));
    int markers = 0;
    for (int row = 0; row < MAP_H; row++) {
        for (int col = 0; col < MAP_W; col++) {
            markers += swamp.map.tiles[row][col] == TILE_SWAMP_SHORTCUT;
        }
    }
    ASSERT("victory creates no swamp shortcut", markers == 0);
    swamp_step(&swamp, 0);
    ASSERT("the final exit returns onto the main road", swamp.location == LOCATION_APPROACH && swamp.player.x == APPROACH_W / 2 && swamp.player.y == APPROACH_H / 2 && swamp.enemy_count == 0);
    ASSERT("road access and Mira's rescue survive saving", save_game(&swamp, 99122) && load_game(&restored, 99122) && approach_swamp_town_exit(&restored, &x, &y) && restored.innkeeper_quest_state == 2);
    swamp = restored;
    if (home) {
        walk_return_approach(&swamp);
    } else {
        walk_swamp_road(&swamp);
    }
    Location destination = home ? origin : rosemoor ? LOCATION_TOWN2 : LOCATION_TOWN3;
    ASSERT("the road lets the player walk home or onward", swamp.location == destination);
    action_resolve_player(&swamp, (Action){ACTION_MOVE, 20, destination == LOCATION_TOWN2 ? 0 : TOWN_H - 1});
    walk_town_approach(&swamp);
    ASSERT("reentry always starts at swamp one", swamp.location == LOCATION_SWAMP && swamp.level == 1);
    swamp_step(&swamp, 0);
    x = swamp.player.x;
    y = swamp.player.y;
    swamp.enemies[0].hp = 7;
    game_open_town_portal(&swamp);
    ASSERT("Return to Town remembers the expedition's town", swamp.location == destination && swamp.swamp_portal_town == destination);
    ASSERT("swamp portal saves and loads", save_game(&swamp, 99122) && load_game(&restored, 99122));
    game_use_town_portal(&restored);
    ASSERT("the portal restores level, position and wounded enemies", restored.location == LOCATION_SWAMP && restored.level == 2 && restored.player.x == x && restored.player.y == y && restored.enemies[0].hp == 7 && restored.swamp_entry_town == destination);
    remove("saves/savegame_99122.json");
}

void test_swamp_travel(void) {
    printf("Forward swamp adventure and town road tests:\n");
    ASSERT("the swamp ends with its fifth-level Demon", SWAMP_DEPTH == 5 && SWAMP_BOSS_LEVEL == 5);
    ASSERT("swamp travel slot is unused", !save_exists(99122));
    for (int rosemoor = 0; rosemoor < 2; rosemoor++) {
        for (int home = 0; home < 2; home++) {
            test_swamp_crossing(rosemoor, home);
        }
    }
}
