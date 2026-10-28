#include "expedition_fixture.h"
#include "town_road_checks.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define TOWN_ROAD_TEST_SLOT 99231

static int rewrite_road_save(GameState *g, int version, int town) {
    if (!save_game(g, TOWN_ROAD_TEST_SLOT)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99231.json", "rb");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    test_mark_legacy_portal(root);
    if (town < 0) {
        cJSON_DeleteItemFromObject(root, "road_town");
    } else {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "road_town"), town);
    }
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99231.json", "wb");
    int ok = text && file && fputs(text, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(text);
    return ok;
}

void test_town_roads(void) {
    printf("Town exteriors, directional gates and bypass roads:\n");
    ASSERT("town road save slot is unused", !save_exists(TOWN_ROAD_TEST_SLOT));
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("town exterior fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    const Location towns[TOWN_ROAD_COUNT] = {LOCATION_TOWN, LOCATION_TOWN2, LOCATION_TOWN3, LOCATION_TOWN4, LOCATION_DAWNMERE};
    for (int town = 0; town < TOWN_ROAD_COUNT; town++) {
        for (int side = 0; side < 4; side++) {
            TownApproach connection;
            game_init(g);
            if (!town_road_connection(towns[town], side, &connection)) {
                ASSERT("unconnected sides cannot create a town road or reopen a closed gate", !town_road_enter(g, towns[town], side, 0) && g->location == LOCATION_TOWN);
                continue;
            }
            g->gold = 393;
            g->score = 1234;
            g->elowen_quest_state = 1;
            g->elowen_seals_restored = 3;
            g->level_cache[1].valid = 1;
            map_generate(&g->level_cache[1].map, 2);
            g->level_cache[1].enemy_count = 1;
            g->level_cache[1].enemies[0] = (Enemy){.active = 1, .type = ENEMY_SKELETON, .hp = 7};
            Map dungeon = g->level_cache[1].map;
            ASSERT("arrival from every linked direction reaches a safe exterior", town_road_enter(g, towns[town], side, 0) && g->location == LOCATION_TOWN_ROAD && g->road_town == towns[town] && g->enemy_count == 0);
            int x;
            int y;
            town_road_entrance(side, &x, &y);
            map_mark_explored(&g->map, g->player.x, g->player.y);
            ASSERT("town road saves retain town identity, position and exploration", save_game(g, TOWN_ROAD_TEST_SLOT) && load_game(loaded, TOWN_ROAD_TEST_SLOT) && loaded->location == LOCATION_TOWN_ROAD && loaded->road_town == towns[town] && loaded->player.x == g->player.x && loaded->player.y == g->player.y && map_is_explored(&loaded->map, g->player.x, g->player.y));
            walk_town_road_to(loaded, x, y);
            int px = connection.gate_x == 0 ? 1 : connection.gate_x == TOWN_W - 1 ? TOWN_W - 2 : connection.gate_x;
            int py = connection.gate_y == 0 ? 1 : connection.gate_y == TOWN_H - 1 ? TOWN_H - 2 : connection.gate_y;
            ASSERT("town entry stands beside the gate used, facing inward", loaded->location == towns[town] && loaded->player.x == px && loaded->player.y == py && loaded->player.last_dx == (side == TOWN_SIDE_WEST) - (side == TOWN_SIDE_EAST) && loaded->player.last_dy == (side == TOWN_SIDE_NORTH) - (side == TOWN_SIDE_SOUTH));
            ASSERT("entering a town preserves wounded combat maps, money and quests", loaded->gold == 393 && loaded->score == 1234 && loaded->elowen_quest_state == 1 && loaded->elowen_seals_restored == 3 && loaded->level_cache[1].enemies[0].hp == 7 && memcmp(&dungeon, &loaded->level_cache[1].map, sizeof(Map)) == 0);
            action_resolve_player(loaded, (Action){ACTION_MOVE, connection.gate_x, connection.gate_y});
            ASSERT("leaving any interior gate returns outside the matching side of the town", loaded->location == LOCATION_TOWN_ROAD && loaded->road_town == towns[town] && abs(loaded->player.x - x) + abs(loaded->player.y - y) == 1 && loaded->map.tiles[y][x] == TILE_TOWN_ROAD_ENTRY_NORTH + side);
            town_road_exit(side, &x, &y);
            walk_town_road_to(loaded, x, y);
            ASSERT("the regional exit resumes the correct approach or Crown Road", loaded->location == (connection.area == LOCATION_MANGROVES ? LOCATION_MANGROVE_PATH : connection.area == LOCATION_CROWNROAD || connection.area == LOCATION_KING_ROAD_WEST ? connection.area : LOCATION_APPROACH) && (loaded->location != LOCATION_APPROACH || (approach_route(loaded)->town == towns[town] && approach_route(loaded)->area == connection.area)));
            for (int offset = -3; offset <= 3; offset += 6) {
                town_road_enter(g, towns[town], side, 0);
                town_road_entrance(side, &x, &y);
                x += side == TOWN_SIDE_NORTH || side == TOWN_SIDE_SOUTH ? offset : 0;
                y += side == TOWN_SIDE_EAST || side == TOWN_SIDE_WEST ? offset : 0;
                g->player.x = x + (side == TOWN_SIDE_EAST) - (side == TOWN_SIDE_WEST);
                g->player.y = y + (side == TOWN_SIDE_SOUTH) - (side == TOWN_SIDE_NORTH);
                ASSERT("off-center sprite contact does not require an exterior gate tile", g->map.tiles[y][x] == TILE_WALL && map_is_walkable(&g->map, g->player.x, g->player.y));
                action_resolve_player(g, (Action){ACTION_MOVE, x, y});
                ASSERT("walking into a cardinal side of the sprite enters at that side's interior exit", g->location == towns[town] && g->player.x == px && g->player.y == py);
            }
            for (int onward = 0; onward < 4; onward++) {
                TownApproach next;
                if (onward == side || !town_road_connection(towns[town], onward, &next)) {
                    continue;
                }
                town_road_enter(g, towns[town], side, 0);
                town_road_entrance(onward, &x, &y);
                walk_town_road_to(g, x, y);
                int gx = next.gate_x == 0 ? 1 : next.gate_x == TOWN_W - 1 ? TOWN_W - 2 : next.gate_x;
                int gy = next.gate_y == 0 ? 1 : next.gate_y == TOWN_H - 1 ? TOWN_H - 2 : next.gate_y;
                ASSERT("walking around town to a different entrance uses that entrance's interior gate", g->location == towns[town] && g->player.x == gx && g->player.y == gy);
                town_road_enter(g, towns[town], side, 0);
                int hp = g->player.hp;
                town_road_exit(onward, &x, &y);
                walk_town_road_to(g, x, y);
                ASSERT("the signed road bypasses town to every other connected direction", g->location == (next.area == LOCATION_MANGROVES ? LOCATION_MANGROVE_PATH : next.area == LOCATION_CROWNROAD || next.area == LOCATION_KING_ROAD_WEST ? next.area : LOCATION_APPROACH) && (g->location != LOCATION_APPROACH || approach_route(g)->area == next.area));
                ASSERT("bypassing town neither starts combat nor changes character, dungeon or quest progress", g->player.hp == hp && g->gold == 393 && g->score == 1234 && g->elowen_seals_restored == 3 && g->level_cache[1].enemies[0].hp == 7 && !g->defeated_bosses);
            }
        }
    }
    game_init(g);
    town_road_enter(g, LOCATION_TOWN, TOWN_SIDE_WEST, 0);
    int x;
    int y;
    town_road_exit(TOWN_SIDE_EAST, &x, &y);
    walk_town_road_to(g, x, y);
    ASSERT("bypassing Oakhaven does not unlock the Lich road", g->location == LOCATION_APPROACH && !approach_dungeon_town_exit(g, &x, &y));
    for (int town = 0; town < TOWN_ROAD_COUNT; town++) {
        game_init(g);
        game_enter_town_gate(g, towns[town], 0, 12);
        g->gold = 393;
        g->rook_quest_state = 1;
        g->rook_labyrinth_switches = 3;
        Map old = g->map;
        ASSERT("older interior saves load in place with their town, exploration and quests intact", rewrite_road_save(g, 177, -1) && load_game(loaded, TOWN_ROAD_TEST_SLOT) && loaded->location == towns[town] && loaded->player.x == g->player.x && loaded->player.y == g->player.y && loaded->gold == 393 && loaded->rook_labyrinth_switches == 3 && memcmp(old.tiles, loaded->map.tiles, sizeof(old.tiles)) == 0 && memcmp(old.explored, loaded->map.explored, sizeof(old.explored)) == 0);
    }
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    ASSERT("Dawnmere has single-tile roads outside its rectangular plaza", g->map.tiles[12][5] == TILE_TOWN_PATH && g->map.tiles[11][5] == TILE_TOWN_FLOOR && g->map.tiles[13][5] == TILE_TOWN_FLOOR && g->map.tiles[14][35] == TILE_TOWN_FLOOR && g->map.tiles[18][20] == TILE_TOWN_PATH && g->map.tiles[18][19] == TILE_TOWN_FLOOR && g->map.tiles[11][12] == TILE_TOWN_PATH && g->map.tiles[15][28] == TILE_TOWN_PATH && g->map.tiles[10][12] == TILE_TOWN_FLOOR && g->map.tiles[16][28] == TILE_TOWN_FLOOR);
    // Reproduce the former broad west road and offset east-west paving.
    g->player.x = 5;
    g->player.y = 11;
    g->gold = 393;
    g->rook_quest_state = 1;
    g->map.tiles[11][5] = TILE_TOWN_PATH;
    g->map.tiles[14][35] = TILE_ITEM;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 35, .y = 14, .underlying_tile = TILE_TOWN_PATH, .item = item_make_health_potion()};
    g->portal_active = 1;
    g->portal_location = LOCATION_DUNGEON;
    g->dungeon_portal_town = LOCATION_DAWNMERE;
    g->map.tiles[13][6] = TILE_PORTAL;
    map_mark_explored(&g->map, 35, 14);
    ASSERT("older Dawnmere saves adopt the narrow roads without moving the player or losing progress", rewrite_road_save(g, 178, LOCATION_TOWN) && load_game(loaded, TOWN_ROAD_TEST_SLOT) && loaded->player.x == 5 && loaded->player.y == 11 && loaded->gold == 393 && loaded->rook_quest_state == 1 && loaded->map.tiles[11][5] == TILE_TOWN_FLOOR && map_is_explored(&loaded->map, 35, 14));
    ASSERT("Dawnmere paving migration preserves dropped loot, portals and the merchant", loaded->floor_items[0].active && loaded->floor_items[0].underlying_tile == TILE_TOWN_FLOOR && loaded->map.tiles[14][35] == TILE_ITEM && !loaded->portal_active && loaded->map.tiles[13][6] != TILE_PORTAL && loaded->map.tiles[7][7] == TILE_LOCAL_BUILDING);
    ASSERT("repaired Dawnmere paving survives a current-version save round trip", save_game(loaded, TOWN_ROAD_TEST_SLOT) && load_game(g, TOWN_ROAD_TEST_SLOT) && g->map.tiles[11][5] == TILE_TOWN_FLOOR && g->floor_items[0].underlying_tile == TILE_TOWN_FLOOR && g->map.tiles[13][6] != TILE_PORTAL);
    game_init(g);
    town_road_enter(g, LOCATION_TOWN4, TOWN_SIDE_WEST, 0);
    ASSERT("modern exterior saves require their town identity", rewrite_road_save(g, 178, -1) && !load_game(loaded, TOWN_ROAD_TEST_SLOT));
    ASSERT("an unrelated location cannot be loaded as a town exterior", rewrite_road_save(g, 178, LOCATION_CASTLE) && !load_game(loaded, TOWN_ROAD_TEST_SLOT));
    remove("saves/savegame_99231.json");
    free(g);
    free(loaded);
}
