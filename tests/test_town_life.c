#include "test_utils.h"
#include "../src/game/town_life.h"
#include "../src/game/actions.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define TOWN_LIFE_SLOT 99151
#define TOWN_LIFE_PATH "saves/savegame_99151.json"

static GameState game;
static GameState loaded;

static void start_town(Location town) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    game.location = town;
    game.defeated_bosses = (1 << LOCATION_FOREST) | (1 << LOCATION_MOUNTAINS) | (1 << LOCATION_SWAMP);
    if (town == LOCATION_TOWN) {
        map_set_town2_road(&game.map, 1);
        map_set_town4_road(&game.map, 1);
    } else if (town == LOCATION_TOWN2) {
        map_generate_town2(&game.map, &game.player.x, &game.player.y);
        map_set_town3_road(&game.map, 1);
        map_set_stillbury_forest_road(&game.map, 1);
    } else if (town == LOCATION_TOWN3) {
        map_generate_town3(&game.map, &game.player.x, &game.player.y);
        map_set_rosemoor_swamp_road(&game.map, 1);
    } else {
        map_generate_town4(&game.map, &game.player.x, &game.player.y);
        map_set_ridgeshire_mountain_road(&game.map, 1);
    }
    game.level_cache[0].valid = 1;
    game.level_cache[0].map.tiles[3][3] = TILE_STAIRS_UP;
    game.gold = 123;
    game.score = 456;
    game.elowen_quest_state = 1;
}

static int all_walkable_connected(const Map *map, int sx, int sy) {
    unsigned char seen[MAP_H][MAP_W] = {{0}};
    int queue[MAP_W * MAP_H];
    int head = 0;
    int tail = 0;
    queue[tail++] = sy * MAP_W + sx;
    seen[sy][sx] = 1;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail) {
        int tile = queue[head++];
        int x = tile % MAP_W;
        int y = tile / MAP_W;
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (nx >= 0 && nx < MAP_W && ny >= 0 && ny < MAP_H &&
                !seen[ny][nx] && map_is_walkable(map, nx, ny)) {
                seen[ny][nx] = 1;
                queue[tail++] = ny * MAP_W + nx;
            }
        }
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (map_is_walkable(map, x, y) && !seen[y][x]) {
                return 0;
            }
        }
    }
    return 1;
}

static int downgrade_town_save(void) {
    FILE *file = fopen(TOWN_LIFE_PATH, "rb");
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
    size_t length = fread(text, 1, size, file);
    fclose(file);
    text[length] = '\0';
    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 110);
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!text) {
        return 0;
    }
    file = fopen(TOWN_LIFE_PATH, "wb");
    if (!file) {
        free(text);
        return 0;
    }
    fputs(text, file);
    fclose(file);
    free(text);
    return 1;
}

void test_town_life(void) {
    printf("Town resident building tests:\n");
    const Location towns[4] = {LOCATION_TOWN, LOCATION_TOWN2, LOCATION_TOWN4, LOCATION_TOWN3};
    for (int i = 0; i < 4; i++) {
        start_town(towns[i]);
        const TownBuilding *b = town_life_building(game.location);
        int door_x = b->x + b->w / 2;
        int door_y = b->y + b->h - 1;
        ASSERT("new town buildings leave every path and gate reachable",
            all_walkable_connected(&game.map, game.player.x, game.player.y));
        ASSERT("building bodies block movement while doors are accessible",
            !map_is_walkable(&game.map, b->x, b->y) &&
            game.map.tiles[door_y][door_x] == TILE_LOCAL_DOOR && map_is_walkable(&game.map, door_x, door_y + 1));
        game.player.x = door_x;
        game.player.y = door_y + 1;
        int inventory = game.inventory_count;
        game.dialogue_active = 1;
        action_resolve_player(&game, (Action){ACTION_MOVE, door_x, door_y});
        ASSERT("walking into local buildings enters a safe interior",
            game.location == b->interior && game.enemy_count == 0);
        if (b->interior == LOCATION_BAKERY || b->interior == LOCATION_MONASTERY) {
            ASSERT("the baker and abbot do not greet on entry or retain previous dialogue", !game.dialogue_active);
        } else {
            ASSERT("the butcher and spice merchant retain their entry greetings", game.dialogue_active &&
                strcmp(game.dialogue_speaker, b->speaker) == 0 && strcmp(game.dialogue_text, b->before) == 0);
        }
        ASSERT("resident and exit have connected approaches",
            all_walkable_connected(&game.map, game.player.x, game.player.y) &&
            !map_is_walkable(&game.map, RESIDENT_X, RESIDENT_Y) && map_is_walkable(&game.map, RESIDENT_X, RESIDENT_Y + 1));
        ASSERT("talking requires being beside the resident", !town_life_talk(&game, 0));
        game.player.x = RESIDENT_X - 1;
        game.player.y = RESIDENT_Y + 1;
        action_resolve_player(&game, (Action){ACTION_MOVE, RESIDENT_X, RESIDENT_Y + 1});
        if (b->interior == LOCATION_BAKERY || b->interior == LOCATION_MONASTERY) {
            ASSERT("approaching the baker or abbot does not initiate dialogue", !game.dialogue_active);
        }
        ASSERT("talking explicitly opens resident dialogue without granting quests or items", town_life_talk(&game, 0) &&
            game.dialogue_active && strcmp(game.dialogue_speaker, b->speaker) == 0 && strcmp(game.dialogue_text, b->before) == 0 &&
            game.gold == 123 && game.score == 456 && game.inventory_count == inventory && game.elowen_quest_state == 1);
        if (b->victory) {
            game.defeated_bosses |= b->victory;
            ASSERT("baker and abbot react to their regional boss victories", town_life_talk(&game, 0) && strcmp(game.dialogue_text, b->after) == 0);
        }
        ASSERT("all four interiors and their dialogue survive save and load", save_game(&game, TOWN_LIFE_SLOT) &&
            load_game(&loaded, TOWN_LIFE_SLOT) && loaded.location == b->interior &&
            strcmp(loaded.dialogue_text, game.dialogue_text) == 0 && loaded.map.tiles[RESIDENT_Y][RESIDENT_X] == TILE_NPC_RESIDENT);
        loaded.player.x = 20;
        loaded.player.y = 21;
        action_resolve_player(&loaded, (Action){ACTION_MOVE, 20, 22});
        ASSERT("walking out returns to the correct town outside the door", loaded.location == b->town &&
            loaded.player.x == door_x && loaded.player.y == door_y + 1 && !loaded.dialogue_active);
        ASSERT("visiting local buildings preserves regional caches and unlocked shortcuts",
            loaded.level_cache[0].map.tiles[3][3] == TILE_STAIRS_UP &&
            all_walkable_connected(&loaded.map, loaded.player.x, loaded.player.y) &&
            (loaded.defeated_bosses & game.defeated_bosses) == game.defeated_bosses);

        start_town(towns[i]);
        // Reproduce an old save with the player and loot inside the new footprint.
        for (int y = b->y; y < b->y + b->h; y++) {
            for (int x = b->x; x < b->x + b->w; x++) {
                game.map.tiles[y][x] = TILE_TOWN_FLOOR;
            }
        }
        game.player.x = b->x;
        game.player.y = b->y;
        game.floor_item_count = 2;
        for (int item = 0; item < 2; item++) {
            game.floor_items[item] = (FloorItem){.active = 1, .x = b->x + item, .y = b->y,
                .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
            game.map.tiles[b->y][b->x + item] = TILE_ITEM;
        }
        map_mark_explored(&game.map, 20, 12);
        int ok = save_game(&game, TOWN_LIFE_SLOT) && downgrade_town_save() && load_game(&loaded, TOWN_LIFE_SLOT);
        ASSERT("old town saves gain the new building without blocking the player", ok &&
            loaded.map.tiles[door_y][door_x] == TILE_LOCAL_DOOR &&
            loaded.player.x == door_x && loaded.player.y == door_y + 1);
        if (ok) {
            ASSERT("town migration preserves progress, exploration and stacked dropped items",
                loaded.gold == 123 && loaded.score == 456 && loaded.elowen_quest_state == 1 &&
                loaded.defeated_bosses == game.defeated_bosses && map_is_explored(&loaded.map, 20, 12) &&
                loaded.floor_items[0].x == door_x && loaded.floor_items[1].x == door_x &&
                loaded.floor_items[0].y == door_y + 1 && loaded.floor_items[1].y == door_y + 1);
            int count = loaded.inventory_count;
            action_resolve_player(&loaded, (Action){ACTION_PICK_UP, 0, 0});
            action_resolve_player(&loaded, (Action){ACTION_PICK_UP, 0, 0});
            ASSERT("relocated stacked items remain recoverable on cobblestone", loaded.inventory_count == count + 2 &&
                loaded.map.tiles[door_y + 1][door_x] == TILE_TOWN_PATH);
        }
        remove(TOWN_LIFE_PATH);
    }
}
