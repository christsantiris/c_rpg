#include "approach_checks.h"
#include "test_utils.h"
#include "shortcut_checks.h"
#include "game/game.h"
#include "game/actions.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

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
    if (g->location == LOCATION_APPROACH) {
        walk_return_approach(g);
    }
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
    ASSERT("wardens occupy stages 7, 6, and 5 near Stillbury", found == (g->level == 7 || g->level == 6 || g->level == 5 ? 1 : 0));
}

static void test_forest_crossing(int reverse, int shortcut) {
    srand(809 + reverse + shortcut * 2);
    forest.player.player_class = CLASS_WARRIOR;
    game_init(&forest);
    game_enter_inn(&forest);
    forest.player.x = ALDER_INN_X;
    forest.player.y = ALDER_INN_Y + 1;
    game_talk_to_alder(&forest);
    game_handle_quest_offer_key(&forest, SDL_SCANCODE_Y, 0);
    game_leave_inn(&forest);
    if (reverse) {
        game_enter_town2(&forest);
    } else {
        forest.location = LOCATION_TOWN;
        map_generate_town(&forest.map, &forest.player.x, &forest.player.y);
    }
    int gate_x = reverse ? TOWN_W - 1 : 0;
    ASSERT("shortcut gate starts closed at either town", forest.map.tiles[TOWN_ROAD_EXIT_Y][gate_x] == TILE_WALL);
    action_resolve_player(&forest, (Action){ACTION_MOVE, gate_x, 12});
    walk_town_approach(&forest);
    ASSERT("either forest gate starts at its own border",
        forest.location == LOCATION_FOREST && forest.level == (reverse ? FOREST_DEPTH : 1) &&
        forest.forest_entry_town == (reverse ? LOCATION_TOWN2 : LOCATION_TOWN));
    forest_step(&forest, !reverse);
    ASSERT("border permits immediate retreat without finding a landmark",
        forest.location == (reverse ? LOCATION_TOWN2 : LOCATION_TOWN));
    game_enter_forest(&forest);
    int guard = 0;
    while (forest.level != FOREST_BOSS_LEVEL && guard++ < FOREST_DEPTH) {
        ASSERT("approaches contain no Necromancer", forest_boss(&forest) < 0);
        forest_rescue(&forest);
        int previous = forest.level;
        int score = forest.score;
        forest_landmark(&forest);
        forest_step(&forest, reverse);
        ASSERT("landmarks open the onward passage from either side",
            forest.location == LOCATION_FOREST && forest.level == previous + (reverse ? -1 : 1));
        ASSERT("either approach rewards first visits equally", forest.score == score + map_forest_difficulty(forest.level) * 100);
    }
    int boss = forest_boss(&forest);
    ASSERT("both approaches reach the central boss grove", forest.level == FOREST_BOSS_LEVEL && boss >= 0);
    ASSERT("Stillbury approach completes all rescues before the boss; Oakhaven approach starts after him", forest.alder_wardens_rescued ==
        (reverse ? 7 : 0));
    if (boss < 0) {
        return;
    }
    forest_landmark(&forest);
    forest_step(&forest, reverse);
    ASSERT("living Necromancer blocks crossing the grove", forest.level == FOREST_BOSS_LEVEL);
    forest_step(&forest, !reverse);
    ASSERT("living Necromancer permits retreat", forest.level == FOREST_BOSS_LEVEL + (reverse ? 1 : -1));
    int revisit_score = forest.score;
    forest_landmark(&forest);
    forest_step(&forest, reverse);
    ASSERT("revisiting a forest stage cannot farm travel score", forest.score == revisit_score);
    boss = forest_boss(&forest);
    ASSERT("returning restores the same living boss", boss >= 0);
    if (boss < 0) {
        return;
    }
    Enemy *enemy = &forest.enemies[boss];
    enemy->hp = 1;
    forest.player.x = enemy->x - 1;
    forest.player.y = enemy->y;
    forest.map.tiles[forest.player.y][forest.player.x] = TILE_FOREST_FLOOR;
    for (int tries = 0; enemy->active && tries < 20; tries++) {
        action_resolve_player(&forest, (Action){ACTION_MOVE, enemy->x, enemy->y});
    }
    ASSERT("boss victory unlocks the shortcut", !enemy->active && (forest.defeated_bosses & (1 << LOCATION_FOREST)));
    ASSERT("boss victory announces the shortcut and destination",
        forest.dialogue_active && strstr(forest.dialogue_text, "You found a shortcut through the forest") &&
        strstr(forest.dialogue_text, reverse ? "OakHaven" : "Stillbury"));
    int x = -1;
    int y = -1;
    for (int row = 0; row < MAP_H; row++) {
        for (int column = 0; column < MAP_W; column++) {
            if (forest.map.tiles[row][column] == TILE_FOREST_SHORTCUT) {
                x = column;
                y = row;
            }
        }
    }
    ASSERT("shortcut opens at the end of a passage into the surrounding terrain", x >= 0 && shortcut_exit_is_recessed(&forest.map, x, y, TILE_FOREST_WALL));
    if (x < 0) {
        return;
    }
    forest.dialogue_active = 0;
    if (shortcut) {
        ASSERT("grove shortcut saves and loads", save_game(&forest, 99031) && load_game(&restored, 99031));
        ASSERT("loaded grove retains its shortcut destination", restored.forest_entry_town == forest.forest_entry_town && restored.map.tiles[y][x] == TILE_FOREST_SHORTCUT);
        ASSERT("grove save retains quest rescues", restored.alder_quest_state == forest.alder_quest_state && restored.alder_wardens_rescued == forest.alder_wardens_rescued);
        action_resolve_player(&restored, (Action){ACTION_MOVE, x, y});
        ASSERT("forest shortcut waits for confirmation", restored.location == LOCATION_FOREST && game_shortcut_prompt_active(&restored));
        game_handle_shortcut_prompt_key(&restored, SDL_SCANCODE_RETURN, 0);
        memcpy(&forest, &restored, sizeof(forest));
        remove("saves/savegame_99031.json");
        ASSERT("boss shortcut enters the road rather than town", forest.location == LOCATION_FOREST_ROAD && forest.player.x == FOREST_ROAD_FORK_X && forest.player.y == FOREST_ROAD_BOSS_Y + 1);
        for (int steps = 0; forest.location == LOCATION_FOREST_ROAD && steps < FOREST_ROAD_W; steps++) {
            int dx = forest.player.y == FOREST_ROAD_Y ? (reverse ? 1 : -1) : 0;
            int dy = forest.player.y < FOREST_ROAD_Y ? 1 : 0;
            action_resolve_player(&forest, (Action){ACTION_MOVE, forest.player.x + dx, forest.player.y + dy});
        }
    } else {
        guard = 0;
        while (forest.location == LOCATION_FOREST && guard++ < FOREST_DEPTH) {
            if (forest.level != FOREST_BOSS_LEVEL) {
                forest_rescue(&forest);
            }
            forest_landmark(&forest);
            forest_step(&forest, reverse);
        }
        ASSERT("full crossings complete the quest in either order", forest.alder_quest_state == 2 && forest.alder_wardens_rescued == 7);
    }
    ASSERT("shortcut and full route reach the opposite town", forest.location == (reverse ? LOCATION_TOWN : LOCATION_TOWN2));
    ASSERT("both routes arrive through the shared forest gate", forest.player.y == 12 && forest.player.x == (reverse ? 1 : TOWN_W - 2));
    gate_x = reverse ? 0 : TOWN_W - 1;
    ASSERT("liberated town keeps only the main forest gate", forest.map.tiles[12][gate_x] == TILE_TOWN_EXIT && forest.map.tiles[TOWN_ROAD_EXIT_Y][gate_x] == TILE_WALL);
    action_resolve_player(&forest, (Action){ACTION_MOVE, gate_x, 12});
    ASSERT("shared forest gate offers a route choice", game_shortcut_prompt_active(&forest));
    game_handle_shortcut_prompt_key(&forest, SDL_SCANCODE_2, 0);
    ASSERT("town shortcut enters the safe road", forest.location == LOCATION_FOREST_ROAD && forest.enemy_count == 0);
    game_leave_forest_road(&forest, reverse ? LOCATION_TOWN2 : LOCATION_TOWN);
    ASSERT("original town also keeps only its main gate", forest.map.tiles[12][reverse ? TOWN_W - 1 : 0] == TILE_TOWN_EXIT && forest.map.tiles[TOWN_ROAD_EXIT_Y][reverse ? TOWN_W - 1 : 0] == TILE_WALL);
    game_enter_forest(&forest);
    ASSERT("revisiting preserves the grove and defeated boss", forest.forest_cache[FOREST_BOSS_LEVEL - 1].valid && (forest.defeated_bosses & (1 << LOCATION_FOREST)));
}

static void test_forest_entry_direction(void) {
    int layouts = 1;
    for (int seed = 0; seed < 32; seed++) {
        srand(1700 + seed);
        map_generate_forest(&forest.map, 1);
        layouts &= forest.map.stairs_up_x == MAP_W - 2 && forest.map.stairs_down_x == 1 &&
            forest.map.tiles[forest.map.stairs_up_y][MAP_W - 1] == TILE_FOREST_ENTRANCE &&
            map_is_walkable(&forest.map, MAP_W - 3, forest.map.stairs_up_y) &&
            forest.map.tiles[forest.map.stairs_down_y][0] == TILE_FOREST_WALL;
    }
    ASSERT("32 forest entry layouts face west from OakHaven with the onward west exit still hidden", layouts);
    for (int unlocked = 0; unlocked < 2; unlocked++) {
        memset(&forest, 0, sizeof(forest));
        game_init(&forest);
        forest.defeated_bosses = unlocked ? 1 << LOCATION_FOREST : 0;
        action_resolve_player(&forest, (Action){ACTION_MOVE, 0, 12});
        if (unlocked) {
            ASSERT("debug-unlocked west gate still offers the normal forest route", game_shortcut_prompt_active(&forest));
            game_handle_shortcut_prompt_key(&forest, SDL_SCANCODE_1, 0);
        }
        walk_town_approach(&forest);
        int y = forest.player.y;
        ASSERT("OakHaven's west gate arrives on the east side of forest stage one", forest.location == LOCATION_FOREST && forest.level == 1 && forest.player.x == MAP_W - 2);
        action_resolve_player(&forest, (Action){ACTION_MOVE, MAP_W - 3, y});
        ASSERT("continuing west immediately walks deeper into the forest", forest.location == LOCATION_FOREST && forest.player.x == MAP_W - 3 && forest.player.y == y);
        action_resolve_player(&forest, (Action){ACTION_MOVE, MAP_W - 2, y});
        action_resolve_player(&forest, (Action){ACTION_MOVE, MAP_W - 1, y});
        walk_return_approach(&forest);
        ASSERT("turning east through the entrance returns to OakHaven's west gate", forest.location == LOCATION_TOWN && forest.player.x == 1 && forest.player.y == 12);
    }
}

void test_forest_travel(void) {
    printf("Forest travel from either town tests:\n");
    test_forest_entry_direction();
    ASSERT("difficulty rises toward the grove from both towns",
        map_forest_difficulty(1) < map_forest_difficulty(2) && map_forest_difficulty(2) < map_forest_difficulty(3) &&
        map_forest_difficulty(3) < map_forest_difficulty(4) && map_forest_difficulty(4) > map_forest_difficulty(5) &&
        map_forest_difficulty(5) > map_forest_difficulty(6) && map_forest_difficulty(6) > map_forest_difficulty(7));
    ASSERT("seven stages place the boss at the exact midpoint", FOREST_DEPTH == 7 && FOREST_BOSS_LEVEL * 2 == FOREST_DEPTH + 1);
    for (int level = 1; level <= FOREST_DEPTH; level++) {
        ASSERT("both approaches have matching difficulty", map_forest_difficulty(level) == map_forest_difficulty(FOREST_DEPTH + 1 - level));
    }
    ASSERT("forest travel test slot is unused", !save_exists(99031));
    for (int reverse = 0; reverse < 2; reverse++) {
        for (int shortcut = 0; shortcut < 2; shortcut++) {
            test_forest_crossing(reverse, shortcut);
        }
    }
    forest.player.player_class = CLASS_WARRIOR;
    game_init(&forest);
    game_enter_town2(&forest);
    game_enter_forest(&forest);
    forest_landmark(&forest);
    forest_step(&forest, 1);
    int x = forest.player.x;
    int y = forest.player.y;
    int hp = --forest.enemies[0].hp;
    game_open_town_portal(&forest);
    ASSERT("reverse Return to Town anchors to Stillbury", forest.location == LOCATION_TOWN2 && forest.forest_portal_town == LOCATION_TOWN2 && forest.map.tiles[13][TOWN_W - 3] == TILE_PORTAL);
    ASSERT("reverse portal saves and loads", save_game(&forest, 99031) && load_game(&restored, 99031));
    action_resolve_player(&restored, (Action){ACTION_MOVE, TOWN_W - 3, 13});
    ASSERT("portal restores stage, position, enemy health and entry town", restored.location == LOCATION_FOREST && restored.level == FOREST_DEPTH - 1 && restored.player.x == x && restored.player.y == y && restored.enemies[0].hp == hp && restored.forest_entry_town == LOCATION_TOWN2);
    remove("saves/savegame_99031.json");

    forest.player.player_class = CLASS_WARRIOR;
    game_init(&forest);
    game_enter_forest(&forest);
    forest_landmark(&forest);
    forest_step(&forest, 0);
    x = forest.player.x;
    y = forest.player.y;
    game_open_town_portal(&forest);
    game_enter_town2(&forest);
    game_enter_forest(&forest);
    game_return_to_town(&forest);
    ASSERT("opposite-end reentry keeps the old portal anchored to OakHaven", forest.location == LOCATION_TOWN2 && forest.forest_entry_town == LOCATION_TOWN2 && forest.forest_portal_town == LOCATION_TOWN && forest.map.tiles[13][TOWN_W - 3] != TILE_PORTAL);
    game_leave_forest(&forest, LOCATION_TOWN, 0);
    ASSERT("original town still exposes the portal", forest.map.tiles[13][2] == TILE_PORTAL);
    ASSERT("portal anchor survives a changed expedition and save/load", save_game(&forest, 99031) && load_game(&restored, 99031));
    action_resolve_player(&restored, (Action){ACTION_MOVE, 2, 13});
    ASSERT("old portal restores its original expedition", restored.location == LOCATION_FOREST && restored.level == 2 && restored.forest_entry_town == LOCATION_TOWN && restored.player.x == x && restored.player.y == y);
    remove("saves/savegame_99031.json");
}
