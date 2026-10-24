#include "test_utils.h"
#include "game/mangrove.h"
#include "game/town_road.h"
#include "systems/save_load.h"
#include "screens/quest_journal.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define DELTA_SLOT 99148

static int walk_to(GameState *g, int tx, int ty, int afloat) {
    int previous[MANGROVE_H][MANGROVE_W];
    int queue[MANGROVE_W * MANGROVE_H];
    memset(previous, -1, sizeof(previous));
    int start = g->player.y * MANGROVE_W + g->player.x;
    int target = ty * MANGROVE_W + tx;
    int head = 0;
    int tail = 1;
    queue[0] = start;
    previous[g->player.y][g->player.x] = start;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail && previous[ty][tx] < 0) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % MANGROVE_W + dx[i];
            int y = cell / MANGROVE_W + dy[i];
            if (x < 1 || x >= MANGROVE_W - 1 || y < 1 || y >= MANGROVE_H - 1 || previous[y][x] >= 0) {
                continue;
            }
            int open = afloat ? g->map.tiles[y][x] == TILE_MANGROVE_WATER : map_is_walkable(&g->map, x, y);
            if (open) {
                previous[y][x] = cell;
                queue[tail++] = y * MANGROVE_W + x;
            }
        }
    }
    if (previous[ty][tx] < 0) {
        return 0;
    }
    int count = 0;
    for (int cell = target; cell != start; cell = previous[cell / MANGROVE_W][cell % MANGROVE_W]) {
        queue[count++] = cell;
    }
    for (int i = count - 1; i >= 0; i--) {
        action_resolve_player(g, (Action){ACTION_MOVE, queue[i] % MANGROVE_W, queue[i] / MANGROVE_W});
        if (g->player.x != queue[i] % MANGROVE_W || g->player.y != queue[i] / MANGROVE_W) {
            return 0;
        }
    }
    return 1;
}


static void flood(const Map *map, unsigned char reached[MANGROVE_H][MANGROVE_W], int sx, int sy, int afloat) {
    int queue[MANGROVE_W * MANGROVE_H];
    int head = 0;
    int tail = 1;
    queue[0] = sy * MANGROVE_W + sx;
    reached[sy][sx] = 1;
    const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % MANGROVE_W + steps[i][0];
            int y = cell / MANGROVE_W + steps[i][1];
            if (x < 0 || y < 0 || x >= MANGROVE_W || y >= MANGROVE_H || reached[y][x] ||
                (afloat && (x == 0 || y == 0 || x == MANGROVE_W - 1 || y == MANGROVE_H - 1))) {
                continue;
            }
            if (afloat ? map->tiles[y][x] == TILE_MANGROVE_WATER : map_is_walkable(map, x, y)) {
                reached[y][x] = 1;
                queue[tail++] = y * MANGROVE_W + x;
            }
        }
    }
}

static int sign_delta(int value) {
    return (value > 0) - (value < 0);
}

static void test_full_delta(GameState *g, GameState *loaded) {
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, TOWN_W - 1, 12);
    mangrove_enter(g, 0);
    unsigned int silhouettes[MANGROVE_DEPTH] = {0};
    for (int stage = 1; stage <= MANGROVE_DEPTH; stage++) {
        ASSERT("every Saltwind stage starts with its own shore encounters", g->location == LOCATION_MANGROVES && g->level == stage && g->enemy_count >= 6 && mangrove_state_valid(g));
        unsigned char shore[MANGROVE_H][MANGROVE_W] = {{0}};
        unsigned char channel[MANGROVE_H][MANGROVE_W] = {{0}};
        flood(&g->map, channel, g->mangroves.boat_x, g->mangroves.boat_y, 1);
        int ports = 1;
        for (int island = 0; island < 4; island++) {
            int x;
            int y;
            int cx;
            int cy;
            mangrove_landing(stage, island, &x, &y);
            mangrove_island(stage, island, &cx, &cy);
            ports &= channel[y + sign_delta(y - cy)][x + sign_delta(x - cx)] && g->map.tiles[y][x] == TILE_MANGROVE_LANDING;
            // Each landing adds its reachable land component to the same union.
            flood(&g->map, shore, x, y, 0);
        }
        ASSERT("all four landings connect to the same navigable water network", ports);
        int encounters = 1;
        int bosses = 0;
        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *enemy = &g->enemies[i];
            encounters &= shore[enemy->y][enemy->x] && map_is_walkable(&g->map, enemy->x, enemy->y);
            bosses += enemy->is_boss;
        }
        ASSERT("every enemy can be reached on foot after docking; only stage five has a boss", encounters && bosses == (stage == MANGROVE_DEPTH));
        ASSERT("both boundary jetties can be reached from a landing", shore[g->map.stairs_up_y][g->map.stairs_up_x] && shore[g->map.stairs_down_y][g->map.stairs_down_x] &&
            (g->map.stairs_down_x == 0 || g->map.stairs_down_x == MANGROVE_W - 1 || g->map.stairs_down_y == 0 || g->map.stairs_down_y == MANGROVE_H - 1));
        int caches = 1;
        for (int i = 0; i < 3; i++) {
            int x;
            int y;
            mangrove_cache(stage, i, &x, &y);
            caches &= shore[y][x - 1] || shore[y][x + 1] || shore[y - 1][x] || shore[y + 1][x];
        }
        ASSERT("every optional supply cache has a reachable shore approach", caches);
        for (int y = 0; y < MANGROVE_H; y++) {
            for (int x = 0; x < MANGROVE_W; x++) {
                silhouettes[stage - 1] = silhouettes[stage - 1] * 33u + g->map.tiles[y][x];
            }
        }
        g->enemies[0].hp = 7;
        g->enemies[0].frozen_turns = 3;
        ASSERT("all five active stages can be saved with their enemies and skiff", save_game(g, DELTA_SLOT) && load_game(loaded, DELTA_SLOT) && loaded->level == stage && loaded->enemies[0].hp == 7 && loaded->enemies[0].frozen_turns == 3 && mangrove_state_valid(loaded));
        // Exercise physical boarding, water routing, docking and boundary progression separately from combat.
        for (int i = 0; i < g->enemy_count; i++) {
            g->enemies[i].active = 0;
        }
        int lx;
        int ly;
        mangrove_landing(stage, 0, &lx, &ly);
        ASSERT("the entry jetty reaches its skiff landing", walk_to(g, lx, ly, 0));
        action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        int cx;
        int cy;
        mangrove_landing(stage, 3, &lx, &ly);
        mangrove_island(stage, 3, &cx, &cy);
        int bx = lx + sign_delta(lx - cx);
        int by = ly + sign_delta(ly - cy);
        ASSERT("the onward island is reached by legal skiff movement", g->mangroves.aboard && walk_to(g, bx, by, 1));
        action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        int ex = g->map.stairs_down_x;
        int ey = g->map.stairs_down_y;
        int ax = ex == 0 ? 1 : ex == MANGROVE_W - 1 ? ex - 1 : ex;
        int ay = ey == 0 ? 1 : ey == MANGROVE_H - 1 ? ey - 1 : ey;
        ASSERT("docking reaches the onward boundary on foot", !g->mangroves.aboard && walk_to(g, ax, ay, 0));
        action_resolve_player(g, (Action){ACTION_MOVE, ex, ey});
        if (stage == MANGROVE_DEPTH) {
            ASSERT("Sszareth's permanent victory flag gates the final departure", g->location == LOCATION_MANGROVES && g->level == MANGROVE_DEPTH && !g->mangrove_defeated);
            g->debug_passages_unlocked = 1;
            action_resolve_player(g, (Action){ACTION_MOVE, ex, ey});
            ASSERT("debug passage override opens the sanctuary departure without awarding boss credit", g->location == LOCATION_MANGROVE_PATH && !g->mangrove_defeated);
        }
    }
    int varied = 1;
    for (int i = 0; i < MANGROVE_DEPTH; i++) {
        for (int j = i + 1; j < MANGROVE_DEPTH; j++) {
            varied &= silhouettes[i] != silhouettes[j];
        }
    }
    ASSERT("the five stages have distinct terrain silhouettes", varied);
    g->debug_passages_unlocked = 0;
    mangrove_enter(g, 0);
    ASSERT("fresh physical entry begins at stage one and respawns its ordinary enemies", g->level == 1 && g->enemies[0].active && g->enemies[0].hp == g->enemies[0].max_hp);
    g->enemies[0].hp = 8;
    g->enemies[0].frozen_turns = 2;
    mangrove_change_level(g, 1);
    mangrove_change_level(g, 0);
    ASSERT("backtracking preserves enemy health and freeze instead of respawning", g->level == 1 && g->enemies[0].hp == 8 && g->enemies[0].frozen_turns == 2);
    mangrove_change_level(g, 1);
    mangrove_change_level(g, 1);
    g->enemies[0].hp = 9;
    g->enemies[0].attack_phase = 1;
    g->enemies[0].move_timer = 2;
    int enemy_x = g->enemies[0].x;
    game_open_town_portal(g);
    ASSERT("a later-stage portal still visits the last town actually entered", g->location == LOCATION_DAWNMERE && g->portal_level == 3);
    ASSERT("town saves retain all five stage records and pending encounter state", save_game(g, DELTA_SLOT) && load_game(loaded, DELTA_SLOT));
    *g = *loaded;
    game_use_town_portal(g);
    ASSERT("a stage-three portal resumes the original enemy wound, position and wind-up", g->location == LOCATION_MANGROVES && g->level == 3 && g->enemies[0].hp == 9 && g->enemies[0].x == enemy_x && g->enemies[0].attack_phase == 1 && g->enemies[0].move_timer == 2);
}

static void test_marsh_fights(GameState *g) {
    game_init(g);
    mangrove_enter(g, 0);
    for (int i = 1; i < MANGROVE_DEPTH; i++) {
        mangrove_change_level(g, 1);
    }
    int boss_index = -1;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == ENEMY_SSZARETH) {
            boss_index = i;
        } else {
            g->enemies[i].active = 0;
        }
    }
    ASSERT("Heartroot Sanctuary contains Sszareth", boss_index >= 0);
    if (boss_index < 0) {
        return;
    }
    Enemy *boss = &g->enemies[boss_index];
    g->player.x = boss->x + 3;
    g->player.y = boss->y;
    int damage = 0;
    mangrove_tick(g, boss, &damage);
    ASSERT("Sszareth warns before charging along a fixed lane", !damage && boss->attack_phase && mangrove_attack_marks(boss, g->player.x, g->player.y));
    g->player.y++;
    mangrove_tick(g, boss, &damage);
    mangrove_tick(g, boss, &damage);
    ASSERT("leaving the committed warning lane avoids the charge and opens the boss guard", !damage && !boss->attack_phase && boss->defense == 0 && boss->move_timer < 0);
    boss->move_timer = 0;
    boss->hp = boss->max_hp / 2 - 1;
    g->player.x = boss->x;
    g->player.y = boss->y + 3;
    mangrove_tick(g, boss, &damage);
    mangrove_tick(g, boss, &damage);
    mangrove_tick(g, boss, &damage);
    int summons = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        summons += g->enemies[i].active && g->enemies[i].revived;
    }
    ASSERT("the half-health call creates only a finite pair of reachable spearhunters", boss->boss_wave_used == 1 && summons == 2);
    boss->hp = 1;
    boss->attack_phase = 0;
    g->player.x = boss->x - 1;
    g->player.y = boss->y;
    g->player.attack = 100;
    int gold = g->gold;
    int score = g->score;
    action_resolve_player(g, (Action){ACTION_MOVE, boss->x, boss->y});
    BossJournalEntry entry;
    ASSERT("an actual combat victory awards Sszareth's gold, score and journal entry", !boss->active && g->mangrove_defeated && g->gold == gold + 200 && g->score >= score + 2400 && quest_journal_get_boss(g, 18, &entry) && entry.defeated);
    gold = g->gold;
    score = g->score;
    mangrove_defeat(g);
    ASSERT("the boss bounty cannot be claimed twice", gold == g->gold && score == g->score);
    mangrove_leave(g);
    mangrove_enter(g, 0);
    for (int i = 1; i < MANGROVE_DEPTH; i++) {
        mangrove_change_level(g, 1);
    }
    int bosses = 0;
    int regulars = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        bosses += g->enemies[i].active && g->enemies[i].is_boss;
        regulars += g->enemies[i].active && !g->enemies[i].is_boss;
    }
    ASSERT("a completed boss stays defeated while normal sanctuary encounters respawn", !bosses && regulars >= 6);
}

static int make_legacy_delta_save(int version, int boat_on_jetty) {
    FILE *file = fopen("saves/savegame_99148.json", "rb");
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
    text[count] = '\0';
    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    if (version == 190) {
        cJSON_DeleteItemFromObject(root, "mangrove_stages");
        cJSON_DeleteItemFromObject(root, "mangrove_defeated");
        cJSON *delta = cJSON_GetObjectItem(root, "mangroves");
        cJSON_DeleteItemFromObject(delta, "stage");
        cJSON_DeleteItemFromObject(delta, "enemies");
    }
    cJSON *delta = cJSON_GetObjectItem(root, "mangroves");
    if (boat_on_jetty) {
        // This water square in the pilot is now covered by the onward jetty.
        cJSON_SetNumberValue(cJSON_GetObjectItem(delta, "boat_x"), 59);
        cJSON_SetNumberValue(cJSON_GetObjectItem(delta, "boat_y"), 30);
        cJSON *player = cJSON_GetObjectItem(root, "player");
        cJSON_SetNumberValue(cJSON_GetObjectItem(player, "x"), 59);
        cJSON_SetNumberValue(cJSON_GetObjectItem(player, "y"), 30);
    }
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99148.json", "wb");
    int ok = file && text && fputs(text, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(text);
    return ok;
}

static void test_region_restoration(GameState *g, GameState *loaded) {
    game_init(g);
    // A version-191 marsh explorer last visited Stillbury before the routes moved back.
    game_enter_town_gate(g, LOCATION_TOWN2, TOWN_W - 1, 12);
    mangrove_enter(g, 0);
    mangrove_change_level(g, 1);
    mangrove_change_level(g, 1);
    g->mangrove_defeated = 1;
    g->gold = 177;
    g->mangroves.searched = 1;
    mangrove_generate_level(&g->map, 3, 1);
    g->floor_items[0] = (FloorItem){.active = 1, .x = 32, .y = 12,
        .underlying_tile = TILE_MANGROVE_LANDING, .item = item_make_health_potion()};
    g->floor_item_count = 1;
    g->map.tiles[12][32] = TILE_ITEM;
    g->player.x = 32;
    g->player.y = 12;
    g->enemies[0].hp = 13;
    g->enemies[0].frozen_turns = 2;
    mangrove_interact(g);
    ASSERT("the restored region still boards its original stage-three skiff", g->mangroves.aboard);
    game_open_town_portal(g);
    int ok = save_game(g, DELTA_SLOT) && make_legacy_delta_save(191, 0) && load_game(loaded, DELTA_SLOT);
    ASSERT("version-191 saves keep their existing town and marsh portal after the route restoration", ok && loaded->location == LOCATION_TOWN2 && loaded->last_town_visited == LOCATION_TOWN2 && loaded->portal_active && loaded->portal_level == 3 && loaded->gold == 177);
    if (ok) {
        game_use_town_portal(loaded);
        ASSERT("restoring the road does not reset the occupied skiff, supplies, gear, boss victory or enemy wounds", loaded->location == LOCATION_MANGROVES && loaded->level == 3 && loaded->mangroves.aboard && loaded->player.x == 32 && loaded->player.y == 13 && loaded->enemies[0].hp == 13 && loaded->enemies[0].frozen_turns == 2 && loaded->mangroves.searched == 1 && loaded->floor_item_count == 1 && loaded->mangrove_defeated);
        ASSERT("the restored expedition rewrites as the current save format", save_game(loaded, DELTA_SLOT) && load_game(g, DELTA_SLOT));
    }
    game_init(g);
    game_enter_glassdeep(g);
    g->enemies[0].hp = 7;
    game_return_to_approach(g);
    g->last_town_visited = LOCATION_DAWNMERE;
    int x = g->player.x;
    int y = g->player.y;
    g->floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y,
        .underlying_tile = g->map.tiles[y][x], .item = item_make_health_potion()};
    g->floor_item_count = 1;
    g->map.tiles[y][x] = TILE_ITEM;
    map_mark_explored(&g->map, x, y);
    ok = save_game(g, DELTA_SLOT) && make_legacy_delta_save(191, 0) && load_game(loaded, DELTA_SLOT);
    ASSERT("a version-191 Glassdeep approach migrates to Stillbury with its expedition and dropped gear", ok && approach_route(loaded) && approach_route(loaded)->town == LOCATION_TOWN2 && loaded->glassdeep_cache[0].enemies[0].hp == 7 && loaded->last_town_visited == LOCATION_DAWNMERE && loaded->floor_item_count == 1 && loaded->floor_items[0].x == x && loaded->floor_items[0].y == y && loaded->map.tiles[y][x] == TILE_ITEM && map_is_explored(&loaded->map, x, y));
    if (ok) {
        approach_travel(loaded, loaded->map.stairs_up_x, loaded->map.stairs_up_y);
        ASSERT("retreat from the restored Glassdeep approach reaches Stillbury's exterior", loaded->location == LOCATION_TOWN_ROAD && loaded->road_town == LOCATION_TOWN2);
    }
    game_init(g);
    mangrove_path(g, 1);
    ok = save_game(g, DELTA_SLOT) && make_legacy_delta_save(191, 0) && load_game(loaded, DELTA_SLOT);
    if (ok) {
        mangrove_travel(loaded, MANGROVE_GATE_X, 0);
    }
    ASSERT("a saved marsh approach now leads back to Dawnmere", ok && loaded->location == LOCATION_TOWN_ROAD && loaded->road_town == LOCATION_DAWNMERE);
}

static void test_pilot_migration(GameState *g, GameState *loaded) {
    for (int scenario = 0; scenario < 4; scenario++) {
        game_init(g);
        game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
        mangrove_enter(g, 0);
        g->enemy_count = 0;
        g->gold = 117;
        g->mangroves.searched = 1;
        mangrove_generate_level(&g->map, 1, 1);
        g->player.x = 14;
        g->player.y = 14;
        map_mark_explored(&g->map, 44, 9);
        action_resolve_player(g, (Action){ACTION_DROP_ITEM, 0, 0});
        if (scenario) {
            action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        }
        if (scenario == 2) {
            game_open_town_portal(g);
        }
        int x = g->player.x;
        int y = g->player.y;
        int ok = save_game(g, DELTA_SLOT) && make_legacy_delta_save(190, scenario == 3) && load_game(loaded, DELTA_SLOT);
        ASSERT("version-190 pilot saves migrate with supplies, exploration, gear and gold intact", ok && loaded->gold == 117 && loaded->mangroves.searched == 1 && loaded->mangrove_stages[0].initialized && loaded->mangrove_stages[0].loot_count == 1 && mangrove_state_valid(loaded));
        if (!ok) {
            continue;
        }
        if (scenario == 2) {
            ASSERT("a pilot portal retains Dawnmere as the last visited town after relocation", loaded->location == LOCATION_DAWNMERE && loaded->portal_active && loaded->last_town_visited == LOCATION_DAWNMERE);
            game_use_town_portal(loaded);
            ASSERT("the migrated pilot portal resumes its occupied skiff and populated shore", loaded->location == LOCATION_MANGROVES && loaded->mangroves.aboard && loaded->enemy_count > 0 && loaded->player.x == 14 && loaded->player.y == 15);
        } else if (scenario == 3) {
            ASSERT("a pilot skiff covered by the new jetty moves with its rider to adjacent water", loaded->mangroves.aboard && loaded->player.x == loaded->mangroves.boat_x && loaded->player.y == loaded->mangroves.boat_y && abs(loaded->player.x - 59) + abs(loaded->player.y - 30) == 1 && loaded->map.tiles[loaded->player.y][loaded->player.x] == TILE_MANGROVE_WATER);
        } else {
            ASSERT("pilot migration keeps exact valid player coordinates and adds encounters plus the boundary exit", loaded->player.x == x && loaded->player.y == y && loaded->enemy_count > 0 && loaded->map.tiles[30][63] == TILE_MANGROVE_EXIT && map_is_explored(&loaded->map, 44, 9));
        }
        ASSERT("the upgraded expedition can be saved and loaded again", save_game(loaded, DELTA_SLOT) && load_game(g, DELTA_SLOT) && mangrove_state_valid(g));
    }
}

void test_mangrove(void) {
    printf("Saltwind landing and physical skiff exploration:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("skiff fixtures allocate on the heap", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    g->player.player_class = CLASS_WARRIOR;
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    ASSERT("Dawnmere has its southern wilderness exit", g->map.tiles[TOWN_H - 1][20] == TILE_TOWN_EXIT);
    action_resolve_player(g, (Action){ACTION_MOVE, 20, TOWN_H - 1});
    ASSERT("the southern gate still leaves town through its matching exterior", g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_DAWNMERE);
    int x;
    int y;
    town_road_exit(TOWN_SIDE_SOUTH, &x, &y);
    town_road_travel(g, x, y);
    ASSERT("the exterior southern road reaches Saltwind's separate safe approach", g->location == LOCATION_MANGROVE_PATH && g->player.last_dy == 1 && !g->enemy_count);
    int road = 1;
    for (int row = 1; row <= MANGROVE_GATE_Y - 4; row++) {
        road &= g->map.tiles[row][MANGROVE_GATE_X] == TILE_TOWN_PATH && g->map.tiles[row][MANGROVE_GATE_X - 1] != TILE_TOWN_PATH && g->map.tiles[row][MANGROVE_GATE_X + 1] != TILE_TOWN_PATH;
        action_resolve_player(g, (Action){ACTION_MOVE, MANGROVE_GATE_X, row});
    }
    ASSERT("the safe approach road is one tile wide", road && g->location == LOCATION_MANGROVE_PATH);
    action_resolve_player(g, (Action){ACTION_MOVE, MANGROVE_GATE_X, MANGROVE_GATE_Y - 3});
    ASSERT("walking into the mangrove landmark starts the landing facing south", g->location == LOCATION_MANGROVES && g->level == 1 && g->player.x == 14 && g->player.y == 1 && g->player.last_dy == 1 && mangrove_state_valid(g));
    // Isolate the established skiff controls here; encounter coverage follows below.
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
    ASSERT("the starting landing is reachable on foot", walk_to(g, 14, 14, 0));
    int py = g->player.y;
    action_resolve_player(g, (Action){ACTION_MOVE, 14, 15});
    ASSERT("deep water blocks walking until the skiff is boarded", g->player.y == py && !map_is_walkable(&g->map, 14, 15));
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("interacting boards the actual skiff on its water tile", g->mangroves.aboard && g->player.x == 14 && g->player.y == 15);
    int hp = g->player.hp;
    int original_type = g->enemies[0].type;
    g->enemies[0].active = 1;
    g->enemies[0].x = 14;
    g->enemies[0].y = 14;
    g->enemies[0].hp = 29;
    g->enemies[0].frozen_turns = 3;
    action_resolve_player(g, (Action){ACTION_MOVE, 14, 14});
    ASSERT("a skiff cannot attack ashore or advance shore combat", g->mangroves.aboard && g->enemies[0].hp == 29 && g->enemies[0].frozen_turns == 3 && g->player.hp == hp);
    g->enemies[0].active = 0;
    g->enemies[0].type = original_type;
    int inventory = g->inventory_count;
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, 0, 0});
    ASSERT("items cannot be dropped into deep water from the skiff", g->inventory_count == inventory && !g->floor_item_count && mangrove_state_valid(g));
    action_resolve_player(g, (Action){ACTION_MOVE, 14, 14});
    ASSERT("paddling onto land cannot bypass docking", g->mangroves.aboard && g->player.y == 15);

    const int mangrove_landings[4][2] = {{14, 14}, {32, 11}, {23, 24}, {51, 25}};
    const int mangrove_caches[3][2] = {{44, 9}, {25, 31}, {52, 31}};
    const int moorings[4][2] = {{14, 15}, {31, 11}, {23, 23}, {51, 24}};
    int gold = g->gold;
    for (int i = 1; i < 4; i++) {
        ASSERT("each island has a reachable water approach", walk_to(g, moorings[i][0], moorings[i][1], 1));
        ASSERT("aboard saves preserve skiff coordinates and facing", save_game(g, DELTA_SLOT) && load_game(loaded, DELTA_SLOT) && loaded->mangroves.aboard && loaded->mangroves.boat_x == g->player.x && loaded->mangroves.boat_y == g->player.y && loaded->mangroves.boat_dx == g->mangroves.boat_dx);
        *g = *loaded;
        action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("interacting at the pier goes ashore and leaves the skiff moored", !g->mangroves.aboard && g->player.x == mangrove_landings[i][0] && g->player.y == mangrove_landings[i][1] && g->mangroves.boat_x == moorings[i][0] && g->mangroves.boat_y == moorings[i][1]);
        ASSERT("island supplies have a reachable adjacent foot tile", walk_to(g, mangrove_caches[i - 1][0] - 1, mangrove_caches[i - 1][1], 0));
        action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("optional supplies pay once and record exploration", g->gold == gold + i * 15 && g->mangroves.searched == (unsigned)((1 << i) - 1));
        action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("repeating a search cannot repeat its reward", g->gold == gold + i * 15);
        ASSERT("the moored skiff can be reached again on foot", walk_to(g, mangrove_landings[i][0], mangrove_landings[i][1], 0));
        action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the same skiff can be reboarded at every landing", g->mangroves.aboard);
    }
    g->last_town_visited = LOCATION_DAWNMERE;
    int bx = g->player.x;
    int by = g->player.y;
    game_open_town_portal(g);
    ASSERT("a portal cast aboard visits the last town and keeps the skiff occupied", g->location == LOCATION_DAWNMERE && g->portal_active && g->portal_location == LOCATION_MANGROVES && g->mangroves.aboard);
    ASSERT("boat expedition state survives saving in town", save_game(g, DELTA_SLOT) && load_game(loaded, DELTA_SLOT) && loaded->portal_active && loaded->mangroves.aboard && loaded->mangroves.searched == 7);
    *g = *loaded;
    game_use_town_portal(g);
    ASSERT("the return portal restores the exact boat and water position without duplication", g->location == LOCATION_MANGROVES && !g->portal_active && g->mangroves.aboard && g->player.x == bx && g->player.y == by && g->mangroves.boat_x == bx && g->mangroves.boat_y == by && mangrove_state_valid(g));
    ASSERT("the starting shore is reachable after exploring every island", walk_to(g, 14, 15, 1));
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("disembarking returns to the starting landing", !g->mangroves.aboard && g->player.y == 14);
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, 0, 0});
    ASSERT("gear can be left on a landing while its boarding interaction remains available", g->floor_item_count == 1 && mangrove_hint(g));
    game_open_town_portal(g);
    ASSERT("a shore portal retains the parked skiff and dropped gear", g->location == LOCATION_DAWNMERE && !g->mangroves.aboard && g->mangroves.loot_count == 1);
    game_use_town_portal(g);
    ASSERT("a shore return restores the landing, its gear and the same skiff", g->location == LOCATION_MANGROVES && !g->mangroves.aboard && g->player.x == 14 && g->player.y == 14 && g->floor_item_count == 1 && g->mangroves.boat_x == 14 && g->mangroves.boat_y == 15 && mangrove_state_valid(g));
    ASSERT("the original boundary exit is reachable on foot", walk_to(g, 14, 1, 0));
    action_resolve_player(g, (Action){ACTION_MOVE, 14, 0});
    ASSERT("retreat returns to the original approach instead of directly to town", g->location == LOCATION_MANGROVE_PATH && g->player.y == MANGROVE_GATE_Y - 4 && g->mangroves.loot_count == 1);
    for (int row = g->player.y - 1; row >= 0; row--) {
        action_resolve_player(g, (Action){ACTION_MOVE, MANGROVE_GATE_X, row});
    }
    ASSERT("walking north returns to Dawnmere's southern exterior", g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_DAWNMERE);
    town_road_entrance(TOWN_SIDE_SOUTH, &x, &y);
    while (g->location == LOCATION_TOWN_ROAD && g->player.y > y) {
        action_resolve_player(g, (Action){ACTION_MOVE, x, g->player.y - 1});
    }
    ASSERT("entering town from the mangroves places the player beside its south gate", g->location == LOCATION_DAWNMERE && g->player.x == 20 && g->player.y == TOWN_H - 2 && g->player.last_dy == -1);
    mangrove_enter(g, 0);
    ASSERT("a new visit preserves explored islands, claimed supplies and dropped gear", g->location == LOCATION_MANGROVES && !g->mangroves.aboard && g->mangroves.searched == 7 && g->floor_item_count == 1 && g->map.tiles[14][14] == TILE_ITEM && map_is_explored(&g->map, 51, 31));
    TileType terrain = g->map.tiles[7][7];
    g->map.tiles[7][7] = TILE_TOWN_FLOOR;
    ASSERT("a town map cannot be saved as the Saltwind adventure", !save_game(g, DELTA_SLOT));
    g->map.tiles[7][7] = terrain;
    g->mangroves.boat_x = 14;
    g->mangroves.boat_y = 14;
    ASSERT("saving rejects an impossible skiff parked on land", !save_game(g, DELTA_SLOT));
    test_full_delta(g, loaded);
    test_marsh_fights(g);
    test_pilot_migration(g, loaded);
    test_region_restoration(g, loaded);
    remove("saves/savegame_99148.json");
    free(g);
    free(loaded);
}
