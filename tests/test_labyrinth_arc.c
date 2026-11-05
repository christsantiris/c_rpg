#include "test_utils.h"
#include "game/actions.h"
#include "game/approach.h"
#include "game/eastern_arc.h"
#include "game/labyrinth_arc.h"
#include "game/royal_evidence.h"
#include "game/town_road.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define LABYRINTH_SLOT 99273
#define LABYRINTH_CELLS (MAP_W * MAP_H)

static int lab_path(GameState *g, int tx, int ty) {
    int *previous = malloc(sizeof(int) * LABYRINTH_CELLS);
    int *queue = malloc(sizeof(int) * LABYRINTH_CELLS);
    if (!previous || !queue) {
        free(previous);
        free(queue);
        return 0;
    }
    for (int i = 0; i < LABYRINTH_CELLS; i++) {
        previous[i] = -1;
    }
    int start = g->player.y * MAP_W + g->player.x;
    int target = ty * MAP_W + tx;
    int head = 0;
    int tail = 1;
    previous[start] = start;
    queue[0] = start;
    const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail && previous[target] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + steps[side][0];
            int y = cell / MAP_W + steps[side][1];
            if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
                continue;
            }
            int next = y * MAP_W + x;
            TileType tile = g->map.tiles[y][x];
            int transition = map_is_travel_tile(tile) || tile == TILE_LABYRINTH_EXIT || tile == TILE_LABYRINTH_STAIRS;
            int occupied = 0;
            for (int i = 0; i < g->enemy_count; i++) {
                occupied |= g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y;
            }
            if (previous[next] >= 0 || occupied || !map_is_walkable(&g->map, x, y) || (transition && next != target)) {
                continue;
            }
            previous[next] = cell;
            queue[tail++] = next;
        }
    }
    int ok = previous[target] >= 0;
    int count = 0;
    if (ok) {
        for (int cell = target; cell != start; cell = previous[cell]) {
            queue[count++] = cell;
        }
        Location location = g->location;
        int level = g->level;
        int route = g->approach_route;
        for (int i = count - 1; i >= 0; i--) {
            int x = queue[i] % MAP_W;
            int y = queue[i] / MAP_W;
            action_resolve_player(g, (Action){ACTION_MOVE, x, y});
            if (g->location != location || g->level != level || g->approach_route != route) {
                ok = i == 0;
                break;
            }
            if (g->player.x != x || g->player.y != y) {
                ok = 0;
                break;
            }
        }
    }
    free(previous);
    free(queue);
    return ok;
}

static void clear_regulars(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type != ENEMY_MINOTAUR) {
            g->enemies[i].active = 0;
        }
    }
}

static int puzzle_count(const Map *map) {
    int count = 0;
    for (int y = 0; y < LABYRINTH_H; y++) {
        for (int x = 0; x < LABYRINTH_W; x++) {
            TileType tile = map->tiles[y][x];
            count += tile == TILE_LABYRINTH_SWITCH_OFF || tile == TILE_LABYRINTH_SWITCH_ON || tile == TILE_LABYRINTH_GATE || tile == TILE_LABYRINTH_RELIC;
            count += tile == TILE_LABYRINTH_STAIRS && (x != map->stairs_down_x || y != map->stairs_down_y);
            count += tile == TILE_LABYRINTH_EXIT && (x != map->stairs_up_x || y != map->stairs_up_y);
        }
    }
    return count;
}

static int old_version(void) {
    const char *path = "saves/savegame_99273.json";
    FILE *file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = calloc((size_t)size + 1, 1);
    int ok = data && fread(data, 1, (size_t)size, file) == (size_t)size;
    fclose(file);
    cJSON *root = ok ? cJSON_Parse(data) : NULL;
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 241);
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(path, "wb");
    ok = data && file && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

void test_labyrinth_arc(void) {
    printf("Stillbury labyrinth, optional quarry road and Minotaur quest:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("labyrinth fixtures allocate and save slot is unused", g && loaded && !save_exists(LABYRINTH_SLOT));
    if (!g || !loaded || save_exists(LABYRINTH_SLOT)) {
        free(g);
        free(loaded);
        return;
    }
    const int seeds[] = {19, 107, 1301};
    for (int seed = 0; seed < 3; seed++) {
        srand(seeds[seed]);
        game_init(g);
        game_enter_town2(g);
        ASSERT("the northern quarry exit works while every regional boss and Rook quest remain unfinished", lab_path(g, 20, 0) && g->location == LOCATION_TOWN_ROAD && !g->rook_quest_state && !g->defeated_bosses);
        int x;
        int y;
        town_road_exit(TOWN_SIDE_NORTH, &x, &y);
        ASSERT("the real north exterior boundary reaches the signed quarry road facing north", lab_path(g, x, y) && g->location == LOCATION_APPROACH && g->approach_route == APPROACH_QUARRY_ROAD && g->player.last_dy == -1);
        ASSERT("the Minotaur never blocks entry into the quarry", lab_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_GLASSDEEP);
        game_return_to_approach(g);
        ASSERT("quarry retreat returns to the same new road beside the optional maze", g->location == LOCATION_APPROACH && g->approach_route == APPROACH_QUARRY_ROAD);
        ASSERT("the maze doorway is usable before meeting Rook and preserves its inward heading", lab_path(g, QUARRY_LABYRINTH_X, QUARRY_LABYRINTH_Y) && g->location == LOCATION_LABYRINTH && g->level == 1 && g->player.last_dy == -1 && !g->rook_quest_state);
        for (int stage = 1; stage <= LABYRINTH_DEPTH; stage++) {
            clear_regulars(g);
            ASSERT("postprocessed maze floors have no runes, relic, puzzle gate or false stairs", !puzzle_count(&g->map));
            if (stage < LABYRINTH_DEPTH) {
                int px = g->player.x;
                int py = g->player.y;
                g->player.x = g->map.stairs_down_x;
                g->player.y = g->map.stairs_down_y;
                int item = g->inventory_count;
                g->inventory[g->inventory_count++] = item_make_health_potion();
                action_resolve_player(g, (Action){ACTION_DROP_ITEM, item, 0});
                ASSERT("dropped treasure cannot hide a working labyrinth stair", g->map.tiles[g->player.y][g->player.x] == TILE_LABYRINTH_STAIRS);
                g->player.x = px;
                g->player.y = py;
                ASSERT("every real downstairs can be reached and crossed without puzzle objectives", lab_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_LABYRINTH && g->level == stage + 1);
            }
        }
        Enemy *boss = NULL;
        for (int i = 0; i < g->enemy_count; i++) {
            if (g->enemies[i].type == ENEMY_MINOTAUR && g->enemies[i].active) {
                boss = &g->enemies[i];
            }
        }
        ASSERT("the living Minotaur is reachable without a quest or rune collection", boss && lab_path(g, boss->x - 1, boss->y));
        for (int stage = LABYRINTH_DEPTH; stage >= 1; stage--) {
            clear_regulars(g);
            ASSERT("every real upstairs permits actual retreat while the Minotaur lives", lab_path(g, g->map.stairs_up_x, g->map.stairs_up_y) &&
                (stage > 1 ? g->location == LOCATION_LABYRINTH && g->level == stage - 1 : g->location == LOCATION_APPROACH && g->approach_route == APPROACH_QUARRY_ROAD));
        }
        ASSERT("the quarry-road boundary remains a usable retreat to Stillbury", lab_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN2);
    }
    game_init(g);
    g->royal_reported_actions = 1;
    game_enter_town2(g);
    g->player.x = 20;
    g->player.y = 2;
    action_resolve_player(g, (Action){ACTION_MOVE, 20, 3});
    action_resolve_player(g, (Action){ACTION_MOVE, 20, 4});
    ASSERT("the new north gate preserves the wanted checkpoint instead of bypassing Crown hostility", g->stillbury_prompt && g->dialogue_y == 5);
    eastern_handle_key(g, SDL_SCANCODE_ESCAPE, 0);
    ASSERT("a wanted traveler can withdraw safely toward the quarry gate", !g->stillbury_prompt && g->player.y == 2);
    game_init(g);
    game_enter_inn(g);
    ASSERT("Rook is present in Stillbury's tavern", g->map.tiles[ROOK_TAVERN_Y][ROOK_TAVERN_X] == TILE_NPC_ROOK);
    g->player.x = ROOK_TAVERN_X;
    g->player.y = ROOK_TAVERN_Y - 1;
    game_talk_to_rook(g);
    ASSERT("Rook offers the prisoner-disappearance hunt with explicit acceptance", game_quest_offer_active(g) && strstr(g->dialogue_text, "disappear") && !g->rook_quest_state);
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("acceptance creates one active Minotaur quest", g->rook_quest_state == 1 && game_active_quest_count(g) == 1);
    game_enter_labyrinth(g);
    for (int stage = 1; stage < LABYRINTH_DEPTH; stage++) {
        game_change_labyrinth_floor(g, 1, 0);
    }
    clear_regulars(g);
    Enemy *boss = NULL;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == ENEMY_MINOTAUR && g->enemies[i].active) {
            boss = &g->enemies[i];
        }
    }
    ASSERT("quest finale can reach the real Minotaur", boss && lab_path(g, boss->x - 1, boss->y));
    int gold = g->gold;
    if (boss) {
        g->player.attack = 100000;
        action_resolve_player(g, (Action){ACTION_MOVE, boss->x, boss->y});
    }
    ASSERT("a real boss kill completes and pays the quest immediately", g->rook_quest_state == 3 && g->rook_quest_completions == 1 && g->gold == gold + ROOK_QUEST_REWARD && !game_active_quest_count(g));
    ASSERT("boss victory reveals Malakar's intake register without consuming pack space", (royal_evidence_found(g) & ROYAL_EVIDENCE_PRISONERS) && strstr(royal_evidence_document(ROYAL_EVIDENCE_PRISONERS)->text, "Malakar") && strstr(royal_evidence_conclusion(g), "Malakar"));
    gold = g->gold;
    labyrinth_complete_quest(g);
    ASSERT("quest reward, victory, evidence and loot survive loading without paying twice", save_game(g, LABYRINTH_SLOT) && load_game(loaded, LABYRINTH_SLOT) && loaded->gold == gold && loaded->rook_quest_state == 3 && loaded->rook_quest_completions == 1 && (royal_evidence_found(loaded) & ROYAL_EVIDENCE_PRISONERS));
    loaded->royal_evidence_seen |= ROYAL_EVIDENCE_PRISONERS;
    ASSERT("the new evidence read-state round trips", save_game(loaded, LABYRINTH_SLOT) && load_game(g, LABYRINTH_SLOT) && (g->royal_evidence_seen & ROYAL_EVIDENCE_PRISONERS));
    g->rook_quest_state = 0;
    g->elowen_quest_state = 1;
    g->dain_quest_state = 1;
    g->alder_quest_state = 1;
    game_enter_inn(g);
    g->player.x = ROOK_TAVERN_X;
    g->player.y = ROOK_TAVERN_Y - 1;
    game_talk_to_rook(g);
    ASSERT("an already defeated boss can be acknowledged even with three active quests", !game_quest_offer_at_limit(g) && game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0) && g->rook_quest_state == 3 && game_active_quest_count(g) == 3);
    game_init(g);
    g->rook_quest_state = 1;
    game_enter_labyrinth(g);
    g->enemies[0].hp = 37;
    game_change_labyrinth_floor(g, 1, 0);
    int sy = g->map.stairs_up_y;
    g->map.tiles[sy][4] = TILE_LABYRINTH_GATE;
    g->map.tiles[sy][5] = TILE_LABYRINTH_RELIC;
    g->map.tiles[sy][6] = TILE_LABYRINTH_SWITCH_OFF;
    g->labyrinth_cache[0].map.tiles[3][4] = TILE_LABYRINTH_SWITCH_ON;
    g->labyrinth_cache[0].map.tiles[3][5] = TILE_LABYRINTH_STAIRS;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 4, .y = sy, .underlying_tile = TILE_LABYRINTH_GATE, .item = item_make_health_potion()};
    g->map.tiles[sy][4] = TILE_ITEM;
    map_mark_explored(&g->map, 4, sy);
    g->player.hp = 43;
    g->gold = 321;
    ASSERT("old active and cached maze puzzles migrate without regenerating their maps or losing wounds and loot", save_game(g, LABYRINTH_SLOT) && old_version() && load_game(loaded, LABYRINTH_SLOT) &&
        loaded->player.hp == 43 && loaded->gold == 321 && loaded->rook_quest_state == 1 && loaded->labyrinth_cache[0].enemies[0].hp == 37 && !puzzle_count(&loaded->map) && !puzzle_count(&loaded->labyrinth_cache[0].map) &&
        loaded->floor_items[0].active && loaded->floor_items[0].underlying_tile == TILE_LABYRINTH_FLOOR && loaded->map.tiles[sy][4] == TILE_ITEM && map_is_explored(&loaded->map, 4, sy));
    remove("saves/savegame_99273.json");
    free(g);
    free(loaded);
}
