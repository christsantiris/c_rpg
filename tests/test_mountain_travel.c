#include "test_utils.h"
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

static void mountain_fragments(GameState *g) {
    int bearers = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *enemy = &g->enemies[i];
        if (enemy->active && enemy->dain_fragment) {
            ASSERT("map bearers occupy levels 1-3 on OakHaven's side", g->level >= 1 && g->level <= 3 && enemy->dain_fragment == (1 << (g->level - 1)));
            bearers++;
            mountain_kill(g, i);
        }
    }
    ASSERT("each pending fragment has one bearer on its own stage", bearers == (g->level <= 3 ? 1 : 0));
}

static void mountain_crossing(int reverse, int shortcut) {
    mountain.player.player_class = CLASS_WARRIOR;
    game_init(&mountain);
    srand(2701 + reverse + 2 * shortcut);
    game_enter_guild(&mountain);
    mountain.player.x = GUILD_DAIN_X;
    mountain.player.y = GUILD_DAIN_Y + 1;
    game_talk_to_dain(&mountain);
    ASSERT("Dain directs the player to revised stages", strstr(mountain.dialogue_text, "1, 2, and 3") != NULL);
    game_leave_guild(&mountain);
    game_leave_mountains(&mountain, reverse ? LOCATION_TOWN4 : LOCATION_TOWN, 0);
    int gate_y = reverse ? TOWN_H - 1 : 0;
    int road_x = reverse ? RIDGESHIRE_MOUNTAIN_ROAD_X : TOWN4_ROAD_X;
    ASSERT("High Pass starts locked at either town", mountain.map.tiles[gate_y][road_x] == TILE_WALL);
    action_resolve_player(&mountain, (Action){ACTION_MOVE, 20, gate_y});
    ASSERT("ordinary gates enter the appropriate mountain end", mountain.location == LOCATION_MOUNTAINS && mountain.level == (reverse ? MOUNTAIN_DEPTH : 1) && mountain.mountain_entry_town == (reverse ? LOCATION_TOWN4 : LOCATION_TOWN));
    mountain_step(&mountain, !reverse);
    ASSERT("either entry permits immediate retreat", mountain.location == (reverse ? LOCATION_TOWN4 : LOCATION_TOWN) && mountain.player.x == 20 && mountain.player.y == (reverse ? TOWN_H - 2 : 1));
    game_enter_mountains(&mountain);
    int guard = 0;
    while (mountain.level != MOUNTAIN_BOSS_LEVEL && guard++ < MOUNTAIN_DEPTH) {
        ASSERT("approach levels contain no King", mountain_king(&mountain) < 0);
        mountain_fragments(&mountain);
        int level = mountain.level;
        int score = mountain.score;
        mountain_step(&mountain, reverse);
        ASSERT("both directions generate the next mountain stage", mountain.location == LOCATION_MOUNTAINS && mountain.level == level + (reverse ? -1 : 1));
        ASSERT("first visits use symmetric travel rewards", mountain.score == score + 100 * map_mountain_difficulty(mountain.level));
    }
    int king = mountain_king(&mountain);
    ASSERT("either approach reaches the King at the central peak", mountain.level == MOUNTAIN_BOSS_LEVEL && king >= 0);
    ASSERT("map quest precedes the peak only from OakHaven", mountain.dain_map_fragments == (reverse ? 0 : 7));
    if (king < 0) {
        return;
    }
    mountain_step(&mountain, reverse);
    ASSERT("living King blocks crossing the peak", mountain.level == MOUNTAIN_BOSS_LEVEL);
    mountain_step(&mountain, !reverse);
    ASSERT("living King permits retreat to the approach", mountain.level == MOUNTAIN_BOSS_LEVEL + (reverse ? 1 : -1));
    int score = mountain.score;
    mountain_step(&mountain, reverse);
    ASSERT("revisiting cannot farm mountain travel score", mountain.score == score);
    king = mountain_king(&mountain);
    ASSERT("cached peak retains its living King", king >= 0);
    if (king < 0) {
        return;
    }
    mountain_kill(&mountain, king);
    ASSERT("King victory prompts the opposite-town shortcut", mountain.dialogue_active && strstr(mountain.dialogue_text, "shortcut through the mountains") && strstr(mountain.dialogue_text, reverse ? "OakHaven" : "Ridgeshire"));
    int x = -1;
    int y = -1;
    for (int row = 0; row < MAP_H; row++) {
        for (int column = 0; column < MAP_W; column++) {
            if (mountain.map.tiles[row][column] == TILE_MOUNTAIN_SHORTCUT) {
                x = column;
                y = row;
            }
        }
    }
    Enemy *defeated = &mountain.enemies[king];
    ASSERT("shortcut appears beside the defeated boss", x >= 0 && abs(x - defeated->x) + abs(y - defeated->y) <= 1 && map_is_walkable(&mountain.map, x, y));
    if (x < 0) {
        return;
    }
    mountain.dialogue_active = 0;
    if (shortcut) {
        ASSERT("central mountain shortcut survives save/load", save_game(&mountain, MOUNTAIN_TRAVEL_SLOT) && load_game(&restored, MOUNTAIN_TRAVEL_SLOT) && restored.mountain_entry_town == mountain.mountain_entry_town && restored.map.tiles[y][x] == TILE_MOUNTAIN_SHORTCUT);
        action_resolve_player(&restored, (Action){ACTION_MOVE, x, y});
        mountain = restored;
        remove("saves/savegame_99124.json");
    } else {
        mountain_step(&mountain, reverse);
        guard = 0;
        while (mountain.location == LOCATION_MOUNTAINS && guard++ < MOUNTAIN_DEPTH) {
            if (reverse) {
                mountain_fragments(&mountain);
            }
            mountain_step(&mountain, reverse);
        }
        ASSERT("full crossings permit completing Dain's map from either side", mountain.dain_map_fragments == 7 && mountain.dain_quest_state == 2);
    }
    ASSERT("both exit choices reach the opposite town", mountain.location == (reverse ? LOCATION_TOWN : LOCATION_TOWN4));
    ASSERT("arrival matches the chosen gate", mountain.player.x == (shortcut ? (reverse ? TOWN4_ROAD_X : RIDGESHIRE_MOUNTAIN_ROAD_X) : 20) && mountain.player.y == (reverse ? 1 : TOWN_H - 2));
    gate_y = reverse ? 0 : TOWN_H - 1;
    road_x = reverse ? TOWN4_ROAD_X : RIDGESHIRE_MOUNTAIN_ROAD_X;
    ASSERT("victory keeps both ordinary and shortcut town gates", mountain.map.tiles[gate_y][20] == TILE_TOWN_EXIT && mountain.map.tiles[gate_y][road_x] == TILE_TOWN_EXIT);
    ASSERT("town mountain gates survive save/load", save_game(&mountain, MOUNTAIN_TRAVEL_SLOT) && load_game(&restored, MOUNTAIN_TRAVEL_SLOT) && restored.map.tiles[gate_y][road_x] == TILE_TOWN_EXIT);
    remove("saves/savegame_99124.json");
    action_resolve_player(&mountain, (Action){ACTION_MOVE, road_x, gate_y});
    ASSERT("unlocked town shortcut enters the safe High Pass", mountain.location == LOCATION_HIGH_PASS && mountain.enemy_count == 0);
    game_leave_high_pass(&mountain, reverse ? LOCATION_TOWN4 : LOCATION_TOWN);
    game_enter_mountains(&mountain);
    guard = 0;
    while (mountain.level != MOUNTAIN_BOSS_LEVEL && guard++ < MOUNTAIN_DEPTH) {
        mountain_step(&mountain, reverse);
    }
    ASSERT("revisiting the peak preserves the dead King and shortcut", mountain_king(&mountain) < 0 && mountain.map.tiles[y][x] == TILE_MOUNTAIN_SHORTCUT);
}

void test_mountain_travel(void) {
    printf("Bidirectional mountain travel tests:\n");
    ASSERT("seven mountain stages have an exact central peak", MOUNTAIN_DEPTH == 7 && MOUNTAIN_BOSS_LEVEL * 2 == MOUNTAIN_DEPTH + 1);
    ASSERT("mountain travel test slot is unused", !save_exists(MOUNTAIN_TRAVEL_SLOT));
    mountain.player.player_class = CLASS_WARRIOR;
    game_init(&mountain);
    mountain.location = LOCATION_MOUNTAINS;
    for (int level = 1; level <= MOUNTAIN_DEPTH; level++) {
        ASSERT("mountain difficulty matches on both approaches", map_mountain_difficulty(level) == map_mountain_difficulty(MOUNTAIN_DEPTH + 1 - level));
        mountain.level = level;
        map_generate_mountains(&mountain.map, level);
        enemies_spawn(&mountain);
        int expected = 10 + map_mountain_difficulty(level);
        if (expected > AREA_ENEMY_LIMIT) {
            expected = AREA_ENEMY_LIMIT;
        }
        ASSERT("mountain enemy density falls away from the peak", mountain.enemy_count == expected);
        ASSERT("King spawns only at the central peak", (mountain_king(&mountain) >= 0) == (level == MOUNTAIN_BOSS_LEVEL));
    }
    for (int reverse = 0; reverse < 2; reverse++) {
        for (int shortcut = 0; shortcut < 2; shortcut++) {
            mountain_crossing(reverse, shortcut);
        }
    }
    mountain.player.player_class = CLASS_WARRIOR;
    game_init(&mountain);
    game_enter_town4(&mountain);
    game_enter_mountains(&mountain);
    mountain_step(&mountain, 1);
    int x = mountain.player.x;
    int y = mountain.player.y;
    int hp = --mountain.enemies[0].hp;
    game_open_town_portal(&mountain);
    ASSERT("reverse mountain Return to Town anchors to Ridgeshire", mountain.location == LOCATION_TOWN4 && mountain.mountain_portal_town == LOCATION_TOWN4 && mountain.map.tiles[TOWN_H - 3][21] == TILE_PORTAL);
    ASSERT("reverse mountain portal survives save/load", save_game(&mountain, MOUNTAIN_TRAVEL_SLOT) && load_game(&restored, MOUNTAIN_TRAVEL_SLOT));
    action_resolve_player(&restored, (Action){ACTION_MOVE, 21, TOWN_H - 3});
    ASSERT("portal restores reverse level, position, wounded enemies, and entry town", restored.location == LOCATION_MOUNTAINS && restored.level == 6 && restored.player.x == x && restored.player.y == y && restored.enemies[0].hp == hp && restored.mountain_entry_town == LOCATION_TOWN4);
    remove("saves/savegame_99124.json");
    game_leave_mountains(&mountain, LOCATION_TOWN, 0);
    game_enter_mountains(&mountain);
    game_return_to_town(&mountain);
    ASSERT("opposite entry does not move the old mountain portal", mountain.location == LOCATION_TOWN && mountain.mountain_entry_town == LOCATION_TOWN && mountain.mountain_portal_town == LOCATION_TOWN4 && mountain.map.tiles[2][21] != TILE_PORTAL);
    game_leave_mountains(&mountain, LOCATION_TOWN4, 0);
    ASSERT("Ridgeshire keeps the portal's original anchor", mountain.map.tiles[TOWN_H - 3][21] == TILE_PORTAL);
}
