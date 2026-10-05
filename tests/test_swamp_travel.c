#include "test_utils.h"
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

static void test_swamp_crossing(int reverse, int shortcut) {
    swamp.player.player_class = CLASS_WARRIOR;
    game_init(&swamp);
    srand(1037 + reverse + shortcut * 2);
    game_enter_town2(&swamp);
    game_talk_to_innkeeper(&swamp);
    ASSERT("Bram names the revised rescue level", strstr(swamp.dialogue_text, "level 3") != NULL);
    if (reverse) {
        game_enter_town3(&swamp);
    }
    int gate_y = reverse ? TOWN_H - 1 : 0;
    int road_x = reverse ? ROSEMOOR_SWAMP_ROAD_X : TOWN3_ROAD_X;
    ASSERT("swamp shortcut starts closed at either town", swamp.map.tiles[gate_y][road_x] == TILE_WALL);
    action_resolve_player(&swamp, (Action){ACTION_MOVE, 20, gate_y});
    ASSERT("either ordinary gate starts at its own border", swamp.location == LOCATION_SWAMP && swamp.level == (reverse ? SWAMP_DEPTH : 1) && swamp.swamp_entry_town == (reverse ? LOCATION_TOWN3 : LOCATION_TOWN2));
    swamp_step(&swamp, !reverse);
    ASSERT("both border levels permit immediate retreat", swamp.location == (reverse ? LOCATION_TOWN3 : LOCATION_TOWN2) && swamp.player.x == 20 && swamp.player.y == (reverse ? TOWN_H - 2 : 1));
    game_enter_swamp(&swamp);
    int guard = 0;
    while (swamp.level != SWAMP_BOSS_LEVEL && guard++ < SWAMP_DEPTH) {
        ASSERT("approaches contain no swamp boss", swamp_boss(&swamp) < 0);
        ASSERT("Mira appears only at level 3 on the Stillbury side", swamp_mira(&swamp, 1) == (swamp.level == SWAMP_RESCUE_LEVEL));
        int before = swamp.level;
        int score = swamp.score;
        swamp_step(&swamp, reverse);
        ASSERT("both directions generate and enter unvisited swamp levels", swamp.location == LOCATION_SWAMP && swamp.level == before + (reverse ? -1 : 1));
        ASSERT("both directions reward first visits equally", swamp.score == score + map_swamp_difficulty(swamp.level) * 100);
    }
    int boss = swamp_boss(&swamp);
    ASSERT("both approaches reach the central demon", swamp.level == SWAMP_BOSS_LEVEL && boss >= 0);
    ASSERT("Stillbury rescue precedes the demon and Rosemoor rescue follows it", swamp.innkeeper_quest_state == (reverse ? 1 : 2) && !swamp_mira(&swamp, 0));
    if (boss < 0) {
        return;
    }
    swamp_step(&swamp, reverse);
    ASSERT("living demon blocks crossing the central level", swamp.level == SWAMP_BOSS_LEVEL);
    swamp_step(&swamp, !reverse);
    ASSERT("living demon permits retreat toward the entry town", swamp.level == SWAMP_BOSS_LEVEL + (reverse ? 1 : -1));
    int score = swamp.score;
    swamp_step(&swamp, reverse);
    ASSERT("revisiting levels cannot farm travel score", swamp.score == score);
    boss = swamp_boss(&swamp);
    ASSERT("returning restores the same living demon", boss >= 0);
    if (boss < 0) {
        return;
    }
    Enemy *demon = &swamp.enemies[boss];
    demon->hp = 1;
    swamp.player.x = demon->x - 1;
    swamp.player.y = demon->y;
    swamp.map.tiles[swamp.player.y][swamp.player.x] = TILE_SWAMP_FLOOR;
    for (int tries = 0; demon->active && tries < 20; tries++) {
        action_resolve_player(&swamp, (Action){ACTION_MOVE, demon->x, demon->y});
    }
    ASSERT("victory unlocks the swamp shortcut", !demon->active && (swamp.defeated_bosses & (1 << LOCATION_SWAMP)));
    ASSERT("victory prompts discovery of the opposite-town shortcut", swamp.dialogue_active && strstr(swamp.dialogue_text, "shortcut through the swamp") && strstr(swamp.dialogue_text, reverse ? "Stillbury" : "Rosemoor"));
    int x = -1;
    int y = -1;
    for (int row = 0; row < MAP_H; row++) {
        for (int column = 0; column < MAP_W; column++) {
            if (swamp.map.tiles[row][column] == TILE_SWAMP_SHORTCUT) {
                x = column;
                y = row;
            }
        }
    }
    Enemy *defeated = demon;
    ASSERT("shortcut appears beside the defeated boss", x >= 0 && abs(x - defeated->x) + abs(y - defeated->y) <= 1 && map_is_walkable(&swamp.map, x, y));
    if (x < 0) {
        return;
    }
    swamp.dialogue_active = 0;
    if (shortcut) {
        ASSERT("central shortcut saves and loads", save_game(&swamp, 99122) && load_game(&restored, 99122));
        ASSERT("loaded swamp preserves entry town, quest, and shortcut", restored.swamp_entry_town == swamp.swamp_entry_town && restored.innkeeper_quest_state == swamp.innkeeper_quest_state && restored.map.tiles[y][x] == TILE_SWAMP_SHORTCUT);
        action_resolve_player(&restored, (Action){ACTION_MOVE, x, y});
        memcpy(&swamp, &restored, sizeof(swamp));
        remove("saves/savegame_99122.json");
    } else {
        guard = 0;
        while (swamp.location == LOCATION_SWAMP && guard++ < SWAMP_DEPTH) {
            if (swamp.innkeeper_quest_state == 1 && swamp.level == SWAMP_RESCUE_LEVEL) {
                ASSERT("reverse full crossing reaches Mira after the boss", swamp_mira(&swamp, 1));
            }
            swamp_step(&swamp, reverse);
        }
        ASSERT("full crossings rescue Mira from either direction", swamp.innkeeper_quest_state == 2);
    }
    ASSERT("both exit choices reach the opposite town", swamp.location == (reverse ? LOCATION_TOWN2 : LOCATION_TOWN3));
    gate_y = reverse ? 0 : TOWN_H - 1;
    road_x = reverse ? TOWN3_ROAD_X : ROSEMOOR_SWAMP_ROAD_X;
    ASSERT("arrival uses the chosen town gate", swamp.player.x == (shortcut ? road_x : 20) && swamp.player.y == (reverse ? 1 : TOWN_H - 2));
    ASSERT("town retains ordinary and shortcut swamp gates", swamp.map.tiles[gate_y][20] == TILE_TOWN_EXIT && swamp.map.tiles[gate_y][road_x] == TILE_TOWN_EXIT);
    ASSERT("unlocked town gates save and load", save_game(&swamp, 99122) && load_game(&restored, 99122) && restored.map.tiles[gate_y][20] == TILE_TOWN_EXIT && restored.map.tiles[gate_y][road_x] == TILE_TOWN_EXIT);
    remove("saves/savegame_99122.json");
    action_resolve_player(&swamp, (Action){ACTION_MOVE, road_x, gate_y});
    ASSERT("town shortcut enters an enemy-free road", swamp.location == LOCATION_SWAMP_ROAD && swamp.enemy_count == 0);
    game_leave_swamp_road(&swamp, reverse ? LOCATION_TOWN3 : LOCATION_TOWN2);
    ASSERT("original town also retains both gates", swamp.map.tiles[reverse ? TOWN_H - 1 : 0][20] == TILE_TOWN_EXIT && swamp.map.tiles[reverse ? TOWN_H - 1 : 0][reverse ? ROSEMOOR_SWAMP_ROAD_X : TOWN3_ROAD_X] == TILE_TOWN_EXIT);
    game_enter_swamp(&swamp);
    guard = 0;
    while (swamp.level != SWAMP_BOSS_LEVEL && guard++ < SWAMP_DEPTH) {
        swamp_step(&swamp, reverse);
    }
    ASSERT("revisiting preserves the defeated demon and shortcut", swamp_boss(&swamp) < 0 && swamp.map.tiles[y][x] == TILE_SWAMP_SHORTCUT);
}

void test_swamp_travel(void) {
    printf("Swamp travel from either town tests:\n");
    ASSERT("seven swamp levels place the boss at the exact midpoint", SWAMP_DEPTH == 7 && SWAMP_BOSS_LEVEL * 2 == SWAMP_DEPTH + 1);
    ASSERT("swamp difficulty rises toward the center and falls beyond it", map_swamp_difficulty(1) < map_swamp_difficulty(2) && map_swamp_difficulty(2) < map_swamp_difficulty(3) && map_swamp_difficulty(3) < map_swamp_difficulty(4));
    swamp.player.player_class = CLASS_WARRIOR;
    game_init(&swamp);
    swamp.location = LOCATION_SWAMP;
    for (int level = 1; level <= SWAMP_DEPTH; level++) {
        ASSERT("both swamp approaches have matching difficulty", map_swamp_difficulty(level) == map_swamp_difficulty(SWAMP_DEPTH + 1 - level));
        swamp.level = level;
        map_generate_swamp(&swamp.map, level);
        enemies_spawn(&swamp);
        ASSERT("actual swamp rosters thin out toward either town", swamp.enemy_count == 10 + map_swamp_difficulty(level));
        ASSERT("Demon spawns only in the central level", (swamp_boss(&swamp) >= 0) == (level == SWAMP_BOSS_LEVEL));
    }
    ASSERT("swamp travel test slot is unused", !save_exists(99122));
    for (int reverse = 0; reverse < 2; reverse++) {
        for (int shortcut = 0; shortcut < 2; shortcut++) {
            test_swamp_crossing(reverse, shortcut);
        }
    }
    swamp.player.player_class = CLASS_WARRIOR;
    game_init(&swamp);
    game_enter_town3(&swamp);
    game_enter_swamp(&swamp);
    swamp_step(&swamp, 1);
    int x = swamp.player.x;
    int y = swamp.player.y;
    int hp = --swamp.enemies[0].hp;
    game_open_town_portal(&swamp);
    ASSERT("reverse swamp Return to Town anchors to Rosemoor", swamp.location == LOCATION_TOWN3 && swamp.swamp_portal_town == LOCATION_TOWN3 && swamp.map.tiles[TOWN_H - 3][21] == TILE_PORTAL);
    ASSERT("reverse swamp portal saves and loads", save_game(&swamp, 99122) && load_game(&restored, 99122));
    action_resolve_player(&restored, (Action){ACTION_MOVE, 21, TOWN_H - 3});
    ASSERT("portal restores reverse stage, position, health, and entry town", restored.location == LOCATION_SWAMP && restored.level == SWAMP_DEPTH - 1 && restored.player.x == x && restored.player.y == y && restored.enemies[0].hp == hp && restored.swamp_entry_town == LOCATION_TOWN3);
    remove("saves/savegame_99122.json");

    game_enter_town2(&swamp);
    game_enter_swamp(&swamp);
    game_return_to_town(&swamp);
    ASSERT("opposite-end entry keeps the old portal anchored to Rosemoor", swamp.location == LOCATION_TOWN2 && swamp.swamp_entry_town == LOCATION_TOWN2 && swamp.swamp_portal_town == LOCATION_TOWN3 && swamp.map.tiles[2][21] != TILE_PORTAL);
    game_enter_swamp(&swamp);
    game_leave_swamp(&swamp, LOCATION_TOWN3, 0);
    ASSERT("original town still exposes its swamp portal", swamp.map.tiles[TOWN_H - 3][21] == TILE_PORTAL);
    ASSERT("portal anchor survives a changed expedition and save/load", save_game(&swamp, 99122) && load_game(&restored, 99122));
    action_resolve_player(&restored, (Action){ACTION_MOVE, 21, TOWN_H - 3});
    ASSERT("old portal restores its original Rosemoor expedition", restored.location == LOCATION_SWAMP && restored.level == SWAMP_DEPTH - 1 && restored.swamp_entry_town == LOCATION_TOWN3 && restored.player.x == x && restored.player.y == y);
    remove("saves/savegame_99122.json");
}
