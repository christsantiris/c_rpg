#include "test_utils.h"
#include "game/pirate.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define HARBOR_SLOT 99287
#define HARBOR_PATH "saves/savegame_99287.json"

static int distance_to(const Map *map, int x, int y, int tx, int ty) {
    int distance[PIRATE_H][PIRATE_W];
    memset(distance, -1, sizeof(distance));
    int queue[PIRATE_H * PIRATE_W];
    int head = 0;
    int tail = 1;
    queue[0] = y * PIRATE_W + x;
    distance[y][x] = 0;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail) {
        int cell = queue[head++];
        x = cell % PIRATE_W;
        y = cell / PIRATE_W;
        if (x == tx && y == ty) {
            return distance[y][x];
        }
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (nx >= 0 && nx < PIRATE_W && ny >= 0 && ny < PIRATE_H &&
                distance[ny][nx] < 0 && map_is_walkable(map, nx, ny)) {
                distance[ny][nx] = distance[y][x] + 1;
                queue[tail++] = ny * PIRATE_W + nx;
            }
        }
    }
    return -1;
}

static int edit_save(int version, double state) {
    FILE *file = fopen(HARBOR_PATH, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = malloc(size + 1);
    if (!text) {
        fclose(file);
        return 0;
    }
    size_t count = fread(text, 1, size, file);
    fclose(file);
    text[count] = 0;
    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    cJSON *cache = cJSON_GetObjectItem(root, "pirate_cache");
    for (int i = 0; i < PIRATE_DEPTH; i++) {
        cJSON *entry = cJSON_GetArrayItem(cache, i);
        if (version < 187 || (i == 2 && state < 0)) {
            cJSON_DeleteItemFromObject(entry, "harbor_state");
        } else if (i == 2) {
            cJSON_SetNumberValue(cJSON_GetObjectItem(entry, "harbor_state"), state);
        }
    }
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(HARBOR_PATH, "wb");
    int ok = file && text && fputs(text, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(text);
    return ok;
}

void test_pirate_harbor(void) {
    printf("Pirate harbor exploration, cargo and boarding:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    Map *map = malloc(sizeof(*map));
    ASSERT("harbor fixtures allocate without adding static game states", g && loaded && map);
    if (!g || !loaded || !map) {
        free(g); free(loaded); free(map);
        return;
    }
    ASSERT("harbor fixture will not overwrite a character", !save_exists(HARBOR_SLOT));
    if (save_exists(HARBOR_SLOT)) {
        free(g); free(loaded); free(map);
        return;
    }
    pirate_generate(map, 3, 17, 0);
    for (int x = 29; x <= 31; x++) {
        map->tiles[22][x] = TILE_PIRATE_RAIL;
        map->tiles[22][x + 15] = TILE_PIRATE_RAIL;
    }
    ASSERT("ships are optional discoveries: sealing both boarding ramps still leaves the exit reachable",
        distance_to(map, map->stairs_up_x, map->stairs_up_y, map->stairs_down_x, map->stairs_down_y) > 0 &&
        distance_to(map, map->stairs_up_x, map->stairs_up_y, 30, 10) < 0 &&
        distance_to(map, map->stairs_up_x, map->stairs_up_y, 45, 10) < 0);
    // Either side of the public quay loop can be explored independently.
    pirate_generate(map, 3, 17, 0);
    for (int y = 2; y <= 4; y++) {
        map->tiles[y][19] = TILE_PIRATE_RAIL;
    }
    ASSERT("the harbor loop still reaches the continuation when its northern leg is blocked",
        distance_to(map, map->stairs_up_x, map->stairs_up_y, map->stairs_down_x, map->stairs_down_y) > 0);
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 20, 12);
    pirate_enter(g);
    pirate_change_level(g, 1);
    pirate_change_level(g, 1);
    ASSERT("both ships have a deck pirate and a cargo-room guard on reachable ground",
        g->enemy_count == 9 && g->enemies[0].x == 28 && g->enemies[0].y == 11 &&
        g->enemies[1].x == 32 && g->enemies[2].x == 43 && g->enemies[3].x == 47 &&
        distance_to(&g->map, 30, 23, 31, 10) > 0 && distance_to(&g->map, 45, 23, 46, 10) > 0);
    *loaded = *g;
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
    int boarded = 1;
    for (int ship = 0; ship < 2; ship++) {
        int x = 31 + ship * 15;
        g->player.x = x;
        g->player.y = 25;
        for (int y = 24; y >= 10; y--) {
            action_resolve_player(g, (Action){ACTION_MOVE, x, y});
            boarded &= g->player.x == x && g->player.y == y && g->level == 3;
        }
    }
    ASSERT("both boarding ramps lead into cargo rooms using ordinary movement", boarded);
    *g = *loaded;
    g->player.x = 46;
    g->player.y = 10;
    int gold = g->gold;
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("the eastern cargo gives a clue and stays sealed before finding the western key",
        !g->pirate_cache[2].harbor_state && g->gold == gold && g->map.tiles[9][47] == TILE_PIRATE_CARGO &&
        strstr(pirate_interaction_hint(g), "FIND ITS KEY"));
    g->player.x = 31;
    g->player.y = 10;
    ASSERT("nearby cargo activates the bound interaction key", game_has_regional_interaction(g));
    int inventory = g->inventory_count;
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("searching western sailcloth gives a persistent key and coins without an inventory slot",
        g->pirate_cache[2].harbor_state == PIRATE_CARGO_KEY && g->gold == gold + 35 &&
        g->inventory_count == inventory && g->map.tiles[9][32] == TILE_PIRATE_DECK);
    ASSERT("the discovered key survives an active harbor save", save_game(g, HARBOR_SLOT) && load_game(loaded, HARBOR_SLOT) &&
        loaded->pirate_cache[2].harbor_state == PIRATE_CARGO_KEY && loaded->map.tiles[9][32] == TILE_PIRATE_DECK);
    *g = *loaded;
    pirate_interact(g);
    ASSERT("searched cargo cannot pay twice", g->gold == gold + 35 && !pirate_interaction_hint(g));
    g->player.x = 46;
    g->player.y = 10;
    ASSERT("finding the key updates the nearby strongbox action", strstr(pirate_interaction_hint(g), "UNLOCK") != NULL);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    pirate_interact(g);
    ASSERT("the optional eastern strongbox pays once and leaves Corvin's quest untouched",
        g->gold == gold + 110 && g->pirate_cache[2].harbor_state == (PIRATE_CARGO_KEY | PIRATE_STRONGBOX) &&
        !g->pirate_quest_state && !g->pirate_cache[2].treasure_claimed);
    g->player.x = 32;
    g->player.y = 16;
    int before = distance_to(&g->map, 34, 16, 41, 16);
    ASSERT("the raised gangplank leaves actual water between ships", !map_is_walkable(&g->map, 37, 16) && before > 25);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("operating the deck winch opens a shorter physical crossing between ships",
        g->pirate_cache[2].harbor_state == 7 && map_is_walkable(&g->map, 37, 16) &&
        distance_to(&g->map, 34, 16, 41, 16) == 7 && !pirate_interaction_hint(g));
    g->player.x = 34;
    g->player.y = 16;
    int crossed = 1;
    for (int x = 35; x <= 41; x++) {
        action_resolve_player(g, (Action){ACTION_MOVE, x, 16});
        crossed &= g->player.x == x && g->player.y == 16;
    }
    ASSERT("ordinary movement crosses the lowered gangplank without a menu or teleport", crossed);
    g->enemies[0].hp = 7;
    g->enemies[1].active = 0;
    g->player.x = 37;
    g->player.y = 16;
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, 0, 0});
    map_mark_explored(&g->map, 37, 16);
    pirate_change_level(g, 1);
    pirate_change_level(g, 0);
    ASSERT("backtracking retains cargo, gangplank, crew wounds, deaths, exploration and dropped gear",
        g->pirate_cache[2].harbor_state == 7 && g->map.tiles[16][37] == TILE_ITEM &&
        g->floor_items[0].underlying_tile == TILE_PIRATE_DECK && g->enemies[0].hp == 7 &&
        !g->enemies[1].active && map_is_explored(&g->map, 37, 16));
    g->player.x = 36;
    g->player.y = 16;
    game_open_town_portal(g);
    ASSERT("a portal from the boarding gangplank returns to Dawnmere", g->portal_active && g->location == LOCATION_DAWNMERE);
    ASSERT("harbor state survives saving while away from the coast", save_game(g, HARBOR_SLOT) && load_game(loaded, HARBOR_SLOT) &&
        loaded->pirate_cache[2].harbor_state == 7);
    *g = *loaded;
    game_use_town_portal(g);
    ASSERT("return portal restores the open crossing and its dropped equipment",
        g->location == LOCATION_PIRATE_COAST && g->level == 3 && map_is_walkable(&g->map, g->player.x, g->player.y) &&
        g->map.tiles[16][37] == TILE_ITEM && map_is_walkable(&g->map, 39, 16) && g->enemies[0].hp == 7);
    // Simulate a version-186 expedition on the retired southern pier.
    g->player.x = 40;
    g->player.y = 31;
    g->map.tiles[31][40] = TILE_PIRATE_DECK;
    g->enemies[0].x = 41;
    g->enemies[0].y = 31;
    g->map.tiles[31][41] = TILE_PIRATE_DECK;
    g->pirate_cache[2].treasure_claimed = 2;
    g->pirate_quest_state = 2;
    gold = g->gold;
    ASSERT("version-186 active expeditions migrate onto the harbor without resetting progress",
        save_game(g, HARBOR_SLOT) && edit_save(186, 0) && load_game(loaded, HARBOR_SLOT) &&
        !loaded->pirate_cache[2].harbor_state && loaded->gold == gold && loaded->pirate_quest_state == 2 &&
        loaded->pirate_cache[2].treasure_claimed == 2 && loaded->enemies[0].hp == 7 && !loaded->enemies[1].active &&
        distance_to(&loaded->map, loaded->map.stairs_up_x, loaded->map.stairs_up_y, loaded->player.x, loaded->player.y) > 0 &&
        distance_to(&loaded->map, loaded->player.x, loaded->player.y, loaded->enemies[0].x, loaded->enemies[0].y) >= 0 &&
        loaded->floor_items[0].active && map_is_walkable(&loaded->map, loaded->floor_items[0].x, loaded->floor_items[0].y));
    *g = *loaded;
    // Also migrate a cached harbor and its anchor while the player is in town.
    g->player.x = 40;
    g->player.y = 31;
    g->map.tiles[31][40] = TILE_PIRATE_DECK;
    g->enemies[0].x = 41;
    g->enemies[0].y = 31;
    g->floor_items[0].x = 42;
    g->floor_items[0].y = 31;
    g->floor_items[0].underlying_tile = TILE_PIRATE_DECK;
    unsigned int first_seed = g->pirate_cache[0].seed;
    int first_x = g->pirate_cache[0].enemies[0].x;
    game_open_town_portal(g);
    ASSERT("version-186 saves also migrate cached harbors while retaining the town and other stages",
        save_game(g, HARBOR_SLOT) && edit_save(186, 0) && load_game(loaded, HARBOR_SLOT) &&
        loaded->location == LOCATION_DAWNMERE && loaded->portal_active && loaded->gold == gold &&
        loaded->pirate_cache[0].seed == first_seed && loaded->pirate_cache[0].enemies[0].x == first_x);
    *g = *loaded;
    pirate_generate(map, 3, g->pirate_cache[2].seed, g->pirate_cache[2].treasure_claimed);
    ASSERT("cached migration repairs crew, dropped gear and portal anchors onto the new harbor",
        distance_to(map, map->stairs_up_x, map->stairs_up_y, g->portal_x, g->portal_y) > 0 &&
        distance_to(map, map->stairs_up_x, map->stairs_up_y, g->pirate_cache[2].enemies[0].x, g->pirate_cache[2].enemies[0].y) > 0 &&
        distance_to(map, map->stairs_up_x, map->stairs_up_y, g->pirate_cache[2].loot[0].x, g->pirate_cache[2].loot[0].y) > 0 &&
        g->pirate_cache[2].enemies[0].hp == 7 && !g->pirate_cache[2].enemies[1].active && g->pirate_quest_state == 2);
    ASSERT("current saves reject missing, impossible or fractional cargo progress",
        save_game(g, HARBOR_SLOT) && edit_save(187, -1) && !load_game(loaded, HARBOR_SLOT) &&
        save_game(g, HARBOR_SLOT) && edit_save(187, PIRATE_STRONGBOX) && !load_game(loaded, HARBOR_SLOT) &&
        save_game(g, HARBOR_SLOT) && edit_save(187, 1.5) && !load_game(loaded, HARBOR_SLOT));
    remove(HARBOR_PATH);
    free(g); free(loaded); free(map);
}
