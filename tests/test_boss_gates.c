#include "test_utils.h"
#include "game/actions.h"
#include "game/approach.h"
#include "game/boss_gates.h"
#include "game/crownroad_north.h"
#include "game/elderwood.h"
#include "game/mangrove.h"
#include "game/stormwatch.h"
#include "game/ironfang.h"
#include "game/giants_crown.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define GATE_SLOT 99311

static int path(GameState *g, int tx, int ty, int walk) {
    int previous[APPROACH_W * APPROACH_H];
    int queue[APPROACH_W * APPROACH_H];
    memset(previous, -1, sizeof(previous));
    int start = g->player.y * APPROACH_W + g->player.x;
    int target = ty * APPROACH_W + tx;
    int head = 0;
    int tail = 1;
    previous[start] = start;
    queue[0] = start;
    const int steps[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    while (head < tail && previous[target] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % APPROACH_W + steps[side][0];
            int y = cell / APPROACH_W + steps[side][1];
            if (x < 0 || y < 0 || x >= APPROACH_W || y >= APPROACH_H) {
                continue;
            }
            int next = y * APPROACH_W + x;
            TileType tile = g->map.tiles[y][x];
            int transition = tile == TILE_APPROACH_AREA || tile == TILE_LABYRINTH_ENTRANCE ||
                tile == TILE_NORTH_ELDERWOOD || tile == TILE_NORTH_CATACOMBS;
            if (previous[next] >= 0 || (transition && next != target) || (!map_is_walkable(&g->map, x, y) && !(transition && next == target))) {
                continue;
            }
            previous[next] = cell;
            queue[tail++] = next;
        }
    }
    if (previous[target] < 0) {
        return 0;
    }
    int count = 0;
    for (int next = target; next != start; next = previous[next]) {
        queue[count++] = next;
    }
    if (walk) {
        for (int i = count - 1; i >= 0 && g->location == LOCATION_APPROACH; i--) {
            action_resolve_player(g, (Action){ACTION_MOVE, queue[i] % APPROACH_W, queue[i] / APPROACH_W});
        }
    }
    return 1;
}

static void town_connections(GameState *g) {
    const int routes[] = {0, 4, 1, 15, 2, 11, 5, 9};
    for (int i = 0; i < 8; i++) {
        game_init(g);
        approach_enter_region(g, routes[i]);
        const TownApproach *route = approach_route(g);
        RoadCheckpoint point[2];
        int count = boss_gate_checkpoints(g, point);
        int vertical = point[0].vertical;
        int ex = vertical ? point[0].x : point[0].x < APPROACH_W / 2 ? 0 : APPROACH_W - 1;
        int ey = !vertical ? point[0].y : point[0].y < APPROACH_H / 2 ? 0 : APPROACH_H - 1;
        ASSERT("every main route has one closed checkpoint and cannot be bypassed across grass", count == 1 &&
            g->map.tiles[point[0].y][point[0].x] == TILE_ROAD_GATE_CLOSED && !path(g, ex, ey, 0));
        ASSERT("either approach still reaches its boss adventure and its original town", path(g, g->map.stairs_down_x, g->map.stairs_down_y, 0) &&
            path(g, g->map.stairs_up_x, g->map.stairs_up_y, 0));
        if (route->area == LOCATION_FOREST) {
            ASSERT("the optional labyrinth stays reachable from both forest approaches before the Necromancer falls", path(g, APPROACH_LABYRINTH_X, APPROACH_LABYRINTH_Y, 0));
            g->defeated_bosses |= 1 << LOCATION_LABYRINTH;
            ASSERT("a Minotaur victory never opens the Haunted Forest road", !boss_gate_open(g, LOCATION_FOREST));
        }
        g->player.x = point[0].x + (vertical ? 0 : point[0].x < APPROACH_W / 2 ? 1 : -1);
        g->player.y = point[0].y + (!vertical ? 0 : point[0].y < APPROACH_H / 2 ? 1 : -1);
        int x = g->player.x;
        int y = g->player.y;
        action_resolve_player(g, (Action){ACTION_MOVE, point[0].x, point[0].y});
        ASSERT("bumping a closed checkpoint names its boss and leaves the player on the approach", g->player.x == x && g->player.y == y &&
            strstr(g->messages[g->message_count - 1], boss_gate_rule(route->area)->boss) && game_has_regional_interaction(g));
        action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("interacting explains the exact adventure required to secure passage", strstr(g->messages[g->message_count - 1], approach_area_name(route->area)) != NULL);
        g->debug_passages_unlocked = 1;
        boss_gate_refresh(g);
        ASSERT("debug unlocks open the physical crossing without awarding a boss victory", path(g, ex, ey, 0) && !(g->defeated_bosses & (1 << route->area)));
        g->debug_passages_unlocked = 0;
        g->defeated_bosses |= 1 << route->area;
        boss_gate_refresh(g);
        ASSERT("a saved victory opens the same checkpoint without depending on the approach", g->map.tiles[point[0].y][point[0].x] == TILE_ROAD_GATE_OPEN && path(g, ex, ey, 0));
        ASSERT("walking through the secured checkpoint crosses to the other town exterior", path(g, ex, ey, 1) && g->location == LOCATION_TOWN_ROAD);
    }
}

static void northern_connections(GameState *g) {
    for (int castle = 0; castle < 2; castle++) {
        for (int flags = 0; flags < 4; flags++) {
            game_init(g);
            g->elderwood_defeated = (flags & 1) != 0;
            g->defeated_bosses = flags & 2 ? 1 << LOCATION_CATACOMBS : 0;
            approach_enter_region(g, castle ? 14 : 16);
            int ex = NORTH_ROAD_X;
            int ey = castle ? APPROACH_H - 1 : 0;
            ASSERT("the northern road requires both victories for a complete crossing from either end", path(g, ex, ey, 0) == (flags == 3));
            ASSERT("a northern expedition always retains a retreat to its original endpoint", path(g, NORTH_ROAD_X, castle ? 0 : APPROACH_H - 1, 0));
            ASSERT("the first uncleared northern adventure can be entered from its incoming side", castle ?
                path(g, NORTH_CAT_X, NORTH_CAT_Y - 5, 0) : path(g, NORTH_ELDER_X, NORTH_ELDER_Y, 0));
            if (castle && (flags & 2)) {
                ASSERT("a castle-side Catacombs victory exposes Elderwood without bypassing Elderthorn", path(g, NORTH_ELDER_X, NORTH_ELDER_Y - 5, 0));
            }
            if (!castle && (flags & 1)) {
                ASSERT("an Oakhaven-side Elderwood victory exposes the Catacombs without bypassing the Marshal", path(g, NORTH_CAT_X, NORTH_CAT_Y, 0));
            }
            if (flags == 3) {
                ASSERT("an entirely secured northern approach can be crossed on foot", path(g, ex, ey, 1) &&
                    g->location == (castle ? LOCATION_TOWN_ROAD : LOCATION_CROWNROAD_NORTH));
            }
        }
    }
    game_init(g);
    northroad_enter(g, 1, 0);
    ASSERT("Crown Road North has no independent boss gate from Castle Grounds", g->location == LOCATION_CROWNROAD_NORTH);
    northroad_leave(g, 0);
    ASSERT("castle-side travelers reach the Catacombs approach without being granted either victory", northern_approach(g) &&
        !g->elderwood_defeated && !(g->defeated_bosses & (1 << LOCATION_CATACOMBS)) && !path(g, NORTH_ROAD_X, APPROACH_H - 1, 0));
    path(g, NORTH_CAT_X, NORTH_CAT_Y - 5, 1);
    ASSERT("the Catacombs can be challenged from the castle side while their road remains closed", g->location == LOCATION_CATACOMBS && g->catacombs_entry_town == LOCATION_CASTLE);
    game_leave_catacombs(g);
    ASSERT("retreat from an undefeated castle-side Catacombs attempt returns north of its checkpoint", northern_approach(g) &&
        g->player.y < NORTH_CAT_Y - 2 && path(g, NORTH_ROAD_X, 0, 0));
}

static int set_legacy_version(void) {
    FILE *file = fopen("saves/savegame_99311.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = malloc(size + 1);
    size_t read = fread(data, 1, size, file);
    fclose(file);
    data[read] = 0;
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 210);
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99311.json", "wb");
    int ok = file && data && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static void saved_connections(GameState *g, GameState *loaded) {
    ASSERT("boss-gate migration test slot is unused", !save_exists(GATE_SLOT));
    if (save_exists(GATE_SLOT)) {
        return;
    }
    for (int castle = 0; castle < 2; castle++) {
        game_init(g);
        approach_enter_region(g, castle ? 14 : 16);
        g->gold = 789;
        g->player.hp = 43;
        g->heartseed_quest_state = 1;
        g->floor_item_count = 1;
        g->floor_items[0] = (FloorItem){.active = 1, .x = NORTH_ROAD_X, .y = castle ? 3 : 44,
            .underlying_tile = TILE_TOWN_PATH, .item = item_make_health_potion()};
        g->map.tiles[g->floor_items[0].y][NORTH_ROAD_X] = TILE_ITEM;
        map_mark_explored(&g->map, NORTH_ROAD_X, 20);
        ASSERT("older approach saves acquire both checkpoints while retaining health, money, quests and loot", save_game(g, GATE_SLOT) &&
            set_legacy_version() && load_game(loaded, GATE_SLOT) && loaded->gold == 789 && loaded->player.hp == 43 &&
            loaded->heartseed_quest_state == 1 && loaded->floor_items[0].active && map_is_explored(&loaded->map, NORTH_ROAD_X, 20) &&
            loaded->map.tiles[NORTH_FOREST_BARRIER_Y][NORTH_ROAD_X] == TILE_ROAD_GATE_CLOSED &&
            loaded->map.tiles[NORTH_CAT_Y - 2][NORTH_ROAD_X] == TILE_ROAD_GATE_CLOSED);
        ASSERT("migrated checkpoints survive another save without opening a road or moving the player", save_game(loaded, GATE_SLOT) &&
            load_game(g, GATE_SLOT) && g->player.x == loaded->player.x && g->player.y == loaded->player.y &&
            !path(g, NORTH_ROAD_X, castle ? APPROACH_H - 1 : 0, 0));
    }
    game_init(g);
    game_enter_dungeon(g);
    g->enemies[0].hp = 37;
    game_return_to_approach(g);
    approach_enter_region(g, 14);
    g->player.x = NORTH_ROAD_X;
    g->player.y = 20;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = NORTH_ROAD_X - 1, .y = NORTH_CAT_Y - 2,
        .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
    g->map.tiles[NORTH_CAT_Y - 2][NORTH_ROAD_X - 1] = TILE_ITEM;
    ASSERT("legacy travelers beyond a newly closed gate regain a retreat, with bank-covered loot kept accessible", save_game(g, GATE_SLOT) &&
        set_legacy_version() && load_game(loaded, GATE_SLOT) && loaded->player.y < NORTH_CAT_Y - 2 &&
        path(loaded, NORTH_ROAD_X, 0, 0) && loaded->floor_items[0].active &&
        path(loaded, loaded->floor_items[0].x, loaded->floor_items[0].y, 0) &&
        loaded->map.tiles[0][NORTH_ROAD_X] == TILE_NORTH_CASTLE);
    ASSERT("adding road gates preserves unrelated combat caches and enemy wounds", loaded->level_cache[0].valid &&
        loaded->level_cache[0].enemies[0].hp == 37 &&
        !memcmp(loaded->level_cache[0].map.tiles, g->level_cache[0].map.tiles, sizeof(g->map.tiles)));
    remove("saves/savegame_99311.json");
}

void test_boss_gates(void) {
    printf("Shared boss gates and two-way road passage:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    if (!g || !loaded) {
        ASSERT("boss-gate fixtures allocate", 0);
        free(g);
        free(loaded);
        return;
    }
    const Location optional[] = {LOCATION_LABYRINTH, LOCATION_GLASSDEEP, LOCATION_MANGROVES, LOCATION_STORMWATCH,
        LOCATION_IRONFANG, LOCATION_GIANTS_CROWN, LOCATION_DESERT, LOCATION_FROSTFELL, LOCATION_DRAGONSPINE,
        LOCATION_ASHEN, LOCATION_TEMPLE, LOCATION_ISLAND, LOCATION_CROWNROAD, LOCATION_KING_ROAD_WEST,
        LOCATION_CROWNROAD_NORTH, LOCATION_CASTLE};
    game_init(g);
    for (unsigned int i = 0; i < sizeof(optional) / sizeof(optional[0]); i++) {
        ASSERT("every optional region and ordinary Crown Road explicitly permits travel before boss victory",
            boss_gate_rule(optional[i]) && boss_gate_rule(optional[i])->policy == BOSS_GATE_NONE && boss_gate_open(g, optional[i]));
    }
    ASSERT("sailing and castle floor progression retain their separate policies", boss_gate_rule(LOCATION_COAST)->policy == BOSS_GATE_DESTINATION &&
        !game_harbor_unlocked(g) && boss_gate_rule(LOCATION_CASTLE_INTERIOR)->policy == BOSS_GATE_INTERNAL);
    town_connections(g);
    northern_connections(g);
    saved_connections(g, loaded);
    free(g);
    free(loaded);
}
