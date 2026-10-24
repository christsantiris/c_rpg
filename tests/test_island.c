#include "test_utils.h"
#include "game/coastal_travel.h"
#include "../src/game/game.h"
#include "../src/screens/harbor.h"
#include "../src/systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

static int legacy_island_save(int slot) {
    char path[64];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", slot);
    FILE *file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *text = malloc((size_t)length + 1);
    if (!text) {
        fclose(file);
        return 0;
    }
    size_t count = fread(text, 1, (size_t)length, file);
    text[count] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 180);
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(path, "wb");
    int ok = file && text && fputs(text, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(text);
    return ok;
}

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
    ASSERT("return voyage restores the unlocked coastal dock beside Sunken Coast",
        coastal_at_docks(&g) && g.map.tiles[COAST_DOCK_ENTRY_Y][COAST_DOCK_X - 1] == TILE_TOWN_PATH);

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
    for (int side = 0; side < 4; side++) {
        game_enter_island(&g);
        int tx = side == 1 ? ISLAND_TEMPLE_X + ISLAND_TEMPLE_W - 1 : side == 3 ? ISLAND_TEMPLE_X : ISLAND_GATE_X;
        int ty = side == 0 ? ISLAND_TEMPLE_Y : side == 2 ? ISLAND_GATE_Y : ISLAND_TEMPLE_Y + 2;
        g.player.x = tx + (side == 1) - (side == 3);
        g.player.y = ty + (side == 2) - (side == 0);
        ASSERT("the centered temple is reachable from each cardinal side", island_reaches_adjacent(&g.map, tx, ty));
        action_resolve_player(&g, (Action){ACTION_MOVE, tx, ty});
        ASSERT("touching any side of the temple landmark enters stage one", g.location == LOCATION_TEMPLE && g.level == 1 && g.enemy_count > 0);
        game_leave_temple(&g);
        ASSERT("temple retreat returns to the island path beside Nahla", g.location == LOCATION_ISLAND && g.player.x == ISLAND_GATE_X && g.player.y == ISLAND_GATE_Y + 1 && map_is_walkable(&g.map, g.player.x, g.player.y) && island_reaches_adjacent(&g.map, ISLAND_NAHLA_X, ISLAND_NAHLA_Y));
    }
    // A former island save can contain players and loot on the new footprint.
    game_enter_island(&g);
    for (int y = ISLAND_TEMPLE_Y; y < ISLAND_TEMPLE_Y + ISLAND_TEMPLE_H; y++) {
        for (int x = ISLAND_TEMPLE_X; x < ISLAND_TEMPLE_X + ISLAND_TEMPLE_W; x++) {
            g.map.tiles[y][x] = TILE_ISLAND_PATH;
        }
    }
    g.map.tiles[7][19] = TILE_ISLAND_TEMPLE_GATE;
    g.map.tiles[7][20] = TILE_ISLAND_TEMPLE_GATE;
    g.map.stairs_down_x = 19;
    g.map.stairs_down_y = 7;
    g.player.x = 20;
    g.player.y = 11;
    g.gold = 393;
    g.temple_treasure_state = 1;
    g.floor_item_count = 2;
    g.floor_items[0] = (FloorItem){.active = 1, .x = 20, .y = 11, .underlying_tile = TILE_ISLAND_PATH, .item = item_make_health_potion()};
    g.floor_items[1] = (FloorItem){.active = 1, .x = 10, .y = 15, .underlying_tile = TILE_ISLAND_GRASS, .item = item_make_mana_potion()};
    g.map.tiles[11][20] = TILE_ITEM;
    g.map.tiles[15][10] = TILE_ITEM;
    map_mark_explored(&g.map, 19, 7);
    int hp = g.temple_cache[0].enemies[0].hp;
    ASSERT("old island saves move players out of the new landmark without resetting progress", save_game(&g, slot) && legacy_island_save(slot) && load_game(&loaded, slot) && loaded.player.x == ISLAND_GATE_X && loaded.player.y == ISLAND_GATE_Y + 1 && loaded.gold == 393 && loaded.temple_treasure_state == 1 && loaded.temple_cache[0].enemies[0].hp == hp && map_is_explored(&loaded.map, 19, 7));
    ASSERT("island migration retires the old doorway and keeps both covered and unrelated loot reachable", loaded.map.tiles[7][19] == TILE_ISLAND_PATH && loaded.map.tiles[ISLAND_TEMPLE_Y][ISLAND_TEMPLE_X] == TILE_ISLAND_TEMPLE_GATE && loaded.floor_items[0].active && loaded.floor_items[0].x == ISLAND_GATE_X && loaded.floor_items[0].y == ISLAND_GATE_Y + 1 && loaded.map.tiles[loaded.floor_items[0].y][loaded.floor_items[0].x] == TILE_ITEM && loaded.floor_items[1].active && loaded.floor_items[1].x == 10 && loaded.floor_items[1].y == 15);
    ASSERT("migrated island positions and loot survive another save round trip", save_game(&loaded, slot) && load_game(&g, slot) && g.player.x == loaded.player.x && g.player.y == loaded.player.y && g.floor_items[0].y == ISLAND_GATE_Y + 1);
    remove("saves/savegame_99013.json");
}
