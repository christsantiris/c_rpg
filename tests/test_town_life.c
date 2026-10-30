#include "expedition_fixture.h"
#include "test_utils.h"
#include "../src/game/town_life.h"
#include "../src/game/actions.h"
#include "../src/game/mangrove.h"
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
    if (town == LOCATION_DAWNMERE) {
        game_enter_town_gate(&game, LOCATION_DAWNMERE, 0, 12);
    } else if (town == LOCATION_TOWN) {
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

static int downgrade_town_save(int version) {
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    test_mark_legacy_portal(root);
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

static void test_alehouse(void) {
    start_town(LOCATION_DAWNMERE);
    const TownBuilding *b = town_life_building(LOCATION_ALEHOUSE);
    int x = b->x + b->w / 2;
    int y = b->y + b->h - 1;
    ASSERT("Dawnmere has separate reachable Alehouse and merchant doors", b->town == LOCATION_DAWNMERE &&
        town_life_building(LOCATION_DAWNMERE)->interior == LOCATION_DAWNMERE_MERCHANT &&
        game.map.tiles[y][x] == TILE_LOCAL_DOOR && all_walkable_connected(&game.map, 20, 12));
    ASSERT("Dain leaves the whole cobblestone plaza intact", game.map.tiles[DAIN_TOWN_Y][DAIN_TOWN_X] == TILE_TOWN_PATH);
    game.player.x = 20;
    game.player.y = 12;
    for (int step = 21; step <= x; step++) {
        action_resolve_player(&game, (Action){ACTION_MOVE, step, 12});
    }
    for (int step = 11; step >= y; step--) {
        action_resolve_player(&game, (Action){ACTION_MOVE, x, step});
    }
    ASSERT("walking from the plaza selects the Alehouse rather than the merchant", game.location == LOCATION_ALEHOUSE &&
        !game.enemy_count && !game.dialogue_active && game.map.tiles[DAIN_ALEHOUSE_Y][DAIN_ALEHOUSE_X] == TILE_NPC_DAIN &&
        all_walkable_connected(&game.map, game.player.x, game.player.y));
    game.player.x = DAIN_ALEHOUSE_X;
    game.player.y = DAIN_ALEHOUSE_Y + 1;
    ASSERT("Dain can be offered his quest through Alehouse Talk", town_life_talk(&game, 0) && game_quest_offer_active(&game));
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    game.dain_map_fragments = 5;
    ASSERT("Alehouse saving retains Dain, quest progress and cask shelves", save_game(&game, TOWN_LIFE_SLOT) &&
        load_game(&loaded, TOWN_LIFE_SLOT) && loaded.location == LOCATION_ALEHOUSE && loaded.dain_quest_state == 1 &&
        loaded.dain_map_fragments == 5 && loaded.map.tiles[7][9] == TILE_TAVERN_TABLE);
    loaded.player.x = 20;
    loaded.player.y = 21;
    action_resolve_player(&loaded, (Action){ACTION_MOVE, 20, 22});
    ASSERT("the Alehouse returns to its own doorstep with quests intact", loaded.location == LOCATION_DAWNMERE &&
        loaded.player.x == x && loaded.player.y == y + 1 && loaded.dain_map_fragments == 5);
    start_town(LOCATION_DAWNMERE);
    // An old town save can contain both the player and stacked loot in the new lot.
    game.player.x = b->x;
    game.player.y = b->y;
    game.floor_item_count = 3;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = b->x, .y = b->y,
            .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
    }
    game.floor_items[2] = (FloorItem){.active = 1, .x = DAIN_TOWN_X, .y = DAIN_TOWN_Y,
        .underlying_tile = TILE_NPC_DAIN, .item = item_make_health_potion()};
    game.map.tiles[b->y][b->x] = TILE_ITEM;
    game.map.tiles[DAIN_TOWN_Y][DAIN_TOWN_X] = TILE_ITEM;
    game.dain_quest_state = 2;
    game.dain_map_fragments = 7;
    map_mark_explored(&game.map, 3, 3);
    ASSERT("old saves gain the Alehouse without losing player or stacked loot", save_game(&game, TOWN_LIFE_SLOT) &&
        downgrade_town_save(214) && load_game(&loaded, TOWN_LIFE_SLOT) && loaded.player.x == x && loaded.player.y == y + 1 &&
        loaded.floor_items[0].x == x && loaded.floor_items[1].x == x && loaded.floor_items[0].y == y + 1 &&
        loaded.floor_items[1].y == y + 1 && loaded.floor_items[2].active &&
        loaded.floor_items[2].underlying_tile == TILE_TOWN_PATH && loaded.dain_quest_state == 2 &&
        loaded.dain_map_fragments == 7 && loaded.gold == 123 && loaded.score == 456 && map_is_explored(&loaded.map, 3, 3));
    remove(TOWN_LIFE_PATH);
}

static int walk_to(GameState *g, int tx, int ty) {
    int previous[MAP_W * MAP_H];
    int queue[MAP_W * MAP_H];
    for (int i = 0; i < MAP_W * MAP_H; i++) {
        previous[i] = -1;
    }
    int start = g->player.y * MAP_W + g->player.x;
    int target = ty * MAP_W + tx;
    int head = 0;
    int tail = 0;
    queue[tail++] = start;
    previous[start] = start;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail && previous[target] < 0) {
        int current = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = current % MAP_W + dx[i];
            int y = current / MAP_W + dy[i];
            int next = y * MAP_W + x;
            if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H && previous[next] < 0 &&
                (next == target || (map_is_walkable(&g->map, x, y) && g->map.tiles[y][x] != TILE_LOCAL_DOOR))) {
                previous[next] = current;
                queue[tail++] = next;
            }
        }
    }
    if (previous[target] < 0) {
        return 0;
    }
    int count = 0;
    for (int next = target; next != start; next = previous[next]) {
        queue[count++] = next;
    }
    for (int i = count - 1; i >= 0; i--) {
        action_resolve_player(g, (Action){ACTION_MOVE, queue[i] % MAP_W, queue[i] / MAP_W});
    }
    return 1;
}

static void test_goldsmith(void) {
    const TownBuilding *b = town_life_building(LOCATION_GOLDSMITH);
    int x = b->x + b->w / 2;
    int y = b->y + b->h - 1;
    for (int seed = 1; seed <= 8; seed++) {
        srand(seed);
        start_town(LOCATION_TOWN);
        ASSERT("Goldsmith preserves all Oakhaven paths and gate approaches", all_walkable_connected(&game.map, 20, 12));
        ASSERT("walking from the plaza actually enters the Goldsmith", walk_to(&game, x, y) && game.location == LOCATION_GOLDSMITH);
        ASSERT("Goldsmith counter can be approached through real movement", walk_to(&game, RESIDENT_X, RESIDENT_Y + 1) && town_life_near_goldsmith(&game));
        ASSERT("Goldsmith interior and inventory survive save/load", save_game(&game, TOWN_LIFE_SLOT) && load_game(&loaded, TOWN_LIFE_SLOT) && loaded.location == LOCATION_GOLDSMITH && town_life_near_goldsmith(&loaded));
        ASSERT("Goldsmith exit actually returns to its Oakhaven doorstep", walk_to(&loaded, 20, 22) && loaded.location == LOCATION_TOWN && loaded.player.x == x && loaded.player.y == y + 1);
    }
    start_town(LOCATION_TOWN);
    for (int row = b->y; row <= y; row++) {
        for (int col = b->x; col < b->x + b->w; col++) {
            game.map.tiles[row][col] = TILE_TOWN_FLOOR;
        }
    }
    game.player.x = b->x;
    game.player.y = b->y;
    game.player.hp = 37;
    game.floor_item_count = 2;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = b->x, .y = b->y, .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_ring_of_vigor()};
    }
    game.map.tiles[b->y][b->x] = TILE_ITEM;
    map_mark_explored(&game.map, 20, 12);
    ASSERT("version-225 Oakhaven saves gain a usable Goldsmith", save_game(&game, TOWN_LIFE_SLOT) && downgrade_town_save(225) && load_game(&loaded, TOWN_LIFE_SLOT) && loaded.map.tiles[y][x] == TILE_LOCAL_DOOR);
    ASSERT("Goldsmith migration preserves wounds, progress, exploration and stacked jewelry", loaded.player.hp == 37 && loaded.gold == 123 && loaded.elowen_quest_state == 1 && map_is_explored(&loaded.map, 20, 12) && loaded.player.x == x && loaded.player.y == y + 1 && loaded.floor_items[0].x == x && loaded.floor_items[1].y == y + 1 && loaded.floor_items[0].item.max_hp_bonus == 10);
    ASSERT("migrated Goldsmith entrance remains an actual transition", walk_to(&loaded, x, y) && loaded.location == LOCATION_GOLDSMITH);
    remove(TOWN_LIFE_PATH);
}

void test_town_life(void) {
    test_goldsmith();
    test_alehouse();
    printf("Town resident building tests:\n");
    const Location towns[5] = {LOCATION_TOWN, LOCATION_TOWN2, LOCATION_TOWN4, LOCATION_TOWN3, LOCATION_DAWNMERE};
    for (int i = 0; i < 5; i++) {
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
        if (towns[i] == LOCATION_DAWNMERE) {
            game.player.y = 12;
            action_resolve_player(&game, (Action){ACTION_MOVE, door_x, 11});
            ASSERT("Dawnmere's merchant has a single-tile spur from the road to its door", game.player.x == door_x &&
                game.player.y == 11 && game.map.tiles[11][door_x] == TILE_TOWN_PATH &&
                game.map.tiles[11][door_x - 1] == TILE_TOWN_FLOOR && game.map.tiles[11][door_x + 1] == TILE_TOWN_FLOOR);
            // Create the portal through normal play so its saved stage is valid.
            mangrove_enter(&game, 0);
            game_open_town_portal(&game);
            game.player.x = door_x;
            game.player.y = door_y + 1;
        }
        int inventory = game.inventory_count;
        game.dialogue_active = 1;
        action_resolve_player(&game, (Action){ACTION_MOVE, door_x, door_y});
        ASSERT("walking into local buildings enters a safe interior",
            game.location == b->interior && game.enemy_count == 0);
        ASSERT("town residents wait for Talk on entry and clear previous dialogue", !game.dialogue_active);
        ASSERT("resident and exit have connected approaches",
            all_walkable_connected(&game.map, game.player.x, game.player.y) &&
            !map_is_walkable(&game.map, RESIDENT_X, RESIDENT_Y) && map_is_walkable(&game.map, RESIDENT_X, RESIDENT_Y + 1));
        ASSERT("talking requires being beside the resident", !town_life_talk(&game, 0));
        game.player.x = RESIDENT_X - 1;
        game.player.y = RESIDENT_Y + 1;
        action_resolve_player(&game, (Action){ACTION_MOVE, RESIDENT_X, RESIDENT_Y + 1});
        ASSERT("approaching a town resident does not initiate dialogue", !game.dialogue_active);
        ASSERT("talking explicitly opens resident dialogue without granting quests or items", town_life_talk(&game, 0) &&
            game.dialogue_active && strcmp(game.dialogue_speaker, b->speaker) == 0 && strcmp(game.dialogue_text, b->before) == 0 &&
            game.gold == 123 && game.score == 456 && game.inventory_count == inventory && game.elowen_quest_state == 1);
        if (b->victory) {
            game.defeated_bosses |= b->victory;
            ASSERT("baker and abbot react to their regional boss victories", town_life_talk(&game, 0) && strcmp(game.dialogue_text, b->after) == 0);
        }
        ASSERT("all five interiors and their dialogue survive save and load", save_game(&game, TOWN_LIFE_SLOT) &&
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
        if (towns[i] == LOCATION_DAWNMERE) {
            ASSERT("the merchant returns to Dawnmere with the marsh return portal intact", loaded.last_town_visited == LOCATION_DAWNMERE &&
                !loaded.portal_active && loaded.portal_location == LOCATION_MANGROVES && loaded.map.tiles[TOWN_H - 3][21] != TILE_PORTAL);
        }

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
        int ok = save_game(&game, TOWN_LIFE_SLOT) && downgrade_town_save(game.location == LOCATION_DAWNMERE ? 192 : 110) && load_game(&loaded, TOWN_LIFE_SLOT);
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

    start_town(LOCATION_DAWNMERE);
    // Reproduce version 192's decorative merchant and grass-covered doorstep.
    game.player.x = 9;
    game.player.y = 12;
    game.map.tiles[10][9] = TILE_LOCAL_BUILDING;
    game.floor_item_count = 2;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = 9 - i, .y = 11,
            .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
        game.map.tiles[11][9 - i] = TILE_ITEM;
    }
    game.portal_active = 1;
    game.portal_location = LOCATION_DUNGEON;
    game.dungeon_portal_town = LOCATION_DAWNMERE;
    game.return_portal_town = LOCATION_DAWNMERE;
    game.map.tiles[13][2] = TILE_PORTAL;
    map_mark_explored(&game.map, 9, 11);
    ASSERT("version 192 restores the merchant door without moving the road player or portal", save_game(&game, TOWN_LIFE_SLOT) &&
        downgrade_town_save(192) && load_game(&loaded, TOWN_LIFE_SLOT) && loaded.player.x == 9 && loaded.player.y == 12 &&
        loaded.map.tiles[10][9] == TILE_LOCAL_DOOR && loaded.map.tiles[13][2] != TILE_PORTAL && !loaded.portal_active &&
        loaded.gold == 123 && loaded.elowen_quest_state == 1 && map_is_explored(&loaded.map, 9, 11));
    ASSERT("merchant migration repairs loot's path underlay without paving neighboring grass", loaded.map.tiles[11][9] == TILE_ITEM &&
        loaded.floor_items[0].underlying_tile == TILE_TOWN_PATH && loaded.floor_items[1].underlying_tile == TILE_TOWN_FLOOR);
    action_resolve_player(&loaded, (Action){ACTION_MOVE, 9, 11});
    action_resolve_player(&loaded, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("the migrated doorstep is walkable and reveals cobblestone after picking up loot", loaded.player.x == 9 &&
        loaded.player.y == 11 && loaded.map.tiles[11][9] == TILE_TOWN_PATH);
    action_resolve_player(&loaded, (Action){ACTION_MOVE, 9, 10});
    ASSERT("the repaired legacy door enters the merchant instead of remaining decorative", loaded.location == LOCATION_DAWNMERE_MERCHANT);
    remove(TOWN_LIFE_PATH);
}
