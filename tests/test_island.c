#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/screens/harbor.h"
#include "../src/systems/save_load.h"
#include <stdlib.h>
#include <string.h>

static int island_reaches_adjacent(const Map *map, int target_x, int target_y) {
    unsigned char seen[ISLAND_H][ISLAND_W] = {{0}};
    int queue[ISLAND_W * ISLAND_H];
    int head = 0;
    int tail = 0;
    queue[tail++] = ISLAND_SPAWN_Y * ISLAND_W + ISLAND_SPAWN_X;
    seen[ISLAND_SPAWN_Y][ISLAND_SPAWN_X] = 1;
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};

    while (head < tail) {
        int cell = queue[head++];
        int x = cell % ISLAND_W;
        int y = cell / ISLAND_W;
        if (abs(x - target_x) + abs(y - target_y) == 1) {
            return 1;
        }
        for (int side = 0; side < 4; side++) {
            int nx = x + dx[side];
            int ny = y + dy[side];
            if (nx < 0 || nx >= ISLAND_W || ny < 0 || ny >= ISLAND_H ||
                seen[ny][nx] || !map_is_walkable(map, nx, ny)) {
                continue;
            }
            seen[ny][nx] = 1;
            queue[tail++] = ny * ISLAND_W + nx;
        }
    }
    return 0;
}

void test_island(void) {
    printf("Ruined Isle tests:\n");
    static GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    g.defeated_bosses = (1 << LOCATION_DUNGEON) | (1 << LOCATION_FOREST) |
        (1 << LOCATION_MOUNTAINS) | (1 << LOCATION_COAST);
    g.elowen_quest_state = 3;
    g.dain_quest_state = 3;
    g.alder_quest_state = 3;
    g.mara_quest_state = 3;
    game_enter_island(&g);

    ASSERT("boarding reaches the peaceful Ruined Isle landing",
        g.location == LOCATION_ISLAND && g.level == 1 &&
        g.player.x == ISLAND_SPAWN_X && g.player.y == ISLAND_SPAWN_Y &&
        g.enemy_count == 0 && g.floor_item_count == 0);
    ASSERT("island stays inside its fixed ocean boundary",
        g.map.tiles[0][0] == TILE_ISLAND_WATER &&
        g.map.tiles[ISLAND_H][ISLAND_W] == TILE_ISLAND_WATER &&
        !map_is_walkable(&g.map, 0, 0));
    ASSERT("island route connects every exploration landmark",
        island_reaches_adjacent(&g.map, ISLAND_GATE_X, ISLAND_GATE_Y) &&
        island_reaches_adjacent(&g.map, ISLAND_CAMP_X, ISLAND_CAMP_Y) &&
        island_reaches_adjacent(&g.map, ISLAND_MARKER_X, ISLAND_MARKER_Y) &&
        island_reaches_adjacent(&g.map, ISLAND_STATUE_X, ISLAND_STATUE_Y) &&
        island_reaches_adjacent(&g.map, ISLAND_LAGOON_X, ISLAND_LAGOON_Y) &&
        island_reaches_adjacent(&g.map, ISLAND_SHIP_X, ISLAND_SHIP_Y) &&
        island_reaches_adjacent(&g.map, ISLAND_CAPTAIN_X, ISLAND_CAPTAIN_Y));
    ASSERT("Captain Rowan waits beside the island dock",
        g.map.tiles[ISLAND_CAPTAIN_Y][ISLAND_CAPTAIN_X] ==
        TILE_NPC_ISLAND_CAPTAIN &&
        !map_is_walkable(&g.map, ISLAND_CAPTAIN_X, ISLAND_CAPTAIN_Y));

    g.player.x = ISLAND_CAMP_X;
    g.player.y = ISLAND_CAMP_Y - 1;
    ASSERT("camp journal is available from the path",
        game_has_regional_interaction(&g) && game_interact_island(&g) &&
        strstr(g.messages[g.message_count - 1], "journal"));
    g.player.x = ISLAND_MARKER_X + 1;
    g.player.y = ISLAND_MARKER_Y;
    ASSERT("carved marker connects the island to the treasure map",
        game_has_island_interaction(&g) && game_interact_island(&g) &&
        strstr(g.messages[g.message_count - 1], "treasure map"));
    g.player.x = ISLAND_GATE_X;
    g.player.y = ISLAND_GATE_Y + 1;
    ASSERT("temple gate points toward the next area",
        game_has_island_interaction(&g) && game_interact_island(&g) &&
        strstr(g.messages[g.message_count - 1], "ruined temple"));

    HarborScreen harbor;
    harbor_init(&harbor);
    ASSERT("island captain offers the return-to-town choice",
        harbor_handle_key(&harbor, SDL_SCANCODE_RETURN, 1, 1) ==
        HARBOR_SAIL_TOWN);
    game_leave_island(&g);
    ASSERT("return voyage restores the unlocked town harbor",
        g.location == LOCATION_TOWN &&
        g.player.x == TOWN_HARBOR_ENTRANCE_X &&
        g.player.y == TOWN_HARBOR_ENTRANCE_Y &&
        g.map.tiles[TOWN_HARBOR_ENTRANCE_Y][TOWN_HARBOR_X - 1] ==
        TILE_TOWN_PATH);

    const int slot = 99013;
    if (save_exists(slot)) {
        ASSERT("island test save slot must be unused", 0);
        return;
    }
    game_enter_island(&g);
    ASSERT("island state saves successfully", save_game(&g, slot));
    static GameState loaded;
    ASSERT("saved island state loads at the dock",
        load_game(&loaded, slot) && loaded.location == LOCATION_ISLAND &&
        loaded.player.x == ISLAND_SPAWN_X &&
        loaded.map.tiles[ISLAND_GATE_Y][ISLAND_GATE_X] ==
        TILE_ISLAND_TEMPLE_GATE &&
        loaded.map.tiles[ISLAND_CAPTAIN_Y][ISLAND_CAPTAIN_X] ==
        TILE_NPC_ISLAND_CAPTAIN);
    remove("saves/savegame_99013.json");
}
