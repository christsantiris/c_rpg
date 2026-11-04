#include "test_utils.h"
#include "game/actions.h"
#include "game/aldric.h"
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
    const int routes[] = {0, 4, 2};
    for (int i = 0; i < 3; i++) {
        game_init(g);
        approach_enter_region(g, routes[i]);
        const TownApproach *route = approach_route(g);
        if (route->unplaced) {
            continue;
        }
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
            ASSERT("the deferred labyrinth is absent from both forest approaches", g->map.tiles[APPROACH_LABYRINTH_Y][APPROACH_LABYRINTH_X] != TILE_LABYRINTH_ENTRANCE);
            g->defeated_bosses |= 1 << LOCATION_LABYRINTH;
            ASSERT("a Minotaur victory never opens the Haunted Forest road", !boss_gate_open(g, LOCATION_FOREST));
        }
        g->player.x = point[0].x + (vertical ? 0 : point[0].x < APPROACH_W / 2 ? 1 : -1);
        g->player.y = point[0].y + (!vertical ? 0 : point[0].y < APPROACH_H / 2 ? 1 : -1);
        int tx = point[0].x;
        int ty = point[0].y;
        RoadCheckpoint guards[2];
        if (boss_gate_guards(g, guards) == 2) {
            tx = guards[1].x;
            ty = guards[1].y;
            g->player.x = tx + (tx < APPROACH_W / 2 ? 1 : -1);
            g->player.y = ty;
        }
        int x = g->player.x;
        int y = g->player.y;
        action_resolve_player(g, (Action){ACTION_MOVE, tx, ty});
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
        ASSERT("walking through the secured checkpoint crosses to the other town exterior", path(g, ex, ey, 1) && (g->location == LOCATION_TOWN_ROAD || approach_sunscar_road(g)));
    }
}

static void northern_connections(GameState *g) {
    for (int flags = 0; flags < 4; flags++) {
        game_init(g);
        g->elderwood_defeated = (flags & 1) != 0;
        g->defeated_bosses = flags & 2 ? 1 << LOCATION_CATACOMBS : 0;
        approach_enter_region(g, 14);
        ASSERT("northern rapids never become a crossing to Oakhaven after either victory", !path(g, NORTH_ROAD_X, APPROACH_H - 1, 0));
        ASSERT("a northern expedition retains its castle retreat", path(g, NORTH_ROAD_X, 0, 0));
        ASSERT("castle-side travelers can enter the Catacombs before its checkpoint", path(g, NORTH_CAT_X, NORTH_CAT_Y - 5, 0));
        if (flags & 2) {
            ASSERT("Catacombs victory gives access to Elderwood from the castle", path(g, NORTH_ELDER_X, NORTH_ELDER_Y - 5, 0));
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

static void guarded_roads(GameState *g, GameState *loaded) {
    const int routes[] = {0, 2};
    const int seeds[] = {7, 71, 107, 907};
    for (int n = 0; n < 2; n++) {
        for (int seed = 0; seed < 4; seed++) {
            srand(seeds[seed]);
            game_init(g);
            approach_enter_region(g, routes[n]);
            Location area = approach_route(g)->area;
            const char *destination = area == LOCATION_FOREST ? "Dawnmere" : "Sunscar Wastes";
            TileType tile = area == LOCATION_FOREST ? TILE_DARK_ELF_ROAD_GUARD : TILE_GOBLIN_ROAD_GUARD;
            RoadCheckpoint guards[2];
            RoadCheckpoint gates[2];
            int count = boss_gate_guards(g, guards);
            boss_gate_checkpoints(g, gates);
            int dx = guards[1].x < APPROACH_W / 2 ? 1 : -1;
            int front_x = guards[1].x + dx;
            ASSERT("two noncombat sentries stand two and three tiles in front of the existing gate", count == 2 &&
                abs(guards[0].x - gates[0].x) == 2 && abs(guards[1].x - gates[0].x) == 3 &&
                g->map.tiles[guards[0].y][guards[0].x] == tile && g->map.tiles[guards[1].y][guards[1].x] == tile && g->enemy_count == 0);
            ASSERT("the narrow road reaches the front sentry through real movement", path(g, front_x, guards[1].y, 1) &&
                g->location == LOCATION_APPROACH && g->player.x == front_x && g->player.y == guards[1].y && game_has_regional_interaction(g));
            int xp = g->player.experience;
            int gold = g->gold;
            int hp = g->player.hp;
            action_resolve_player(g, (Action){ACTION_MOVE, guards[1].x, guards[1].y});
            action_resolve_enemies(g);
            ASSERT("bumping the sentry explains its destination without combat, rewards or retaliation", g->player.x == front_x &&
                strstr(g->messages[g->message_count - 1], destination) && g->player.experience == xp && g->gold == gold && g->player.hp == hp);
            action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
            ASSERT("the normal interaction key explains the boss required to remove the sentries", strstr(g->messages[g->message_count - 1], boss_gate_rule(area)->boss) != NULL);
            g->player.known_spells[0] = spell_make_teleport();
            g->player.equipped_spell = 0;
            g->player.known_spell_count = 1;
            g->player.mp = 500;
            g->player.last_dx = -dx;
            g->player.last_dy = 0;
            action_resolve_player(g, (Action){ACTION_CAST_SPELL, 0, 0});
            ASSERT("Teleport cannot cross the sentries or spend MP on the blocked step", g->player.x == front_x && g->player.mp == 500);
            g->player.known_spells[0] = spell_make_fireball();
            action_resolve_player(g, (Action){ACTION_CAST_SPELL, 0, 0});
            ASSERT("area magic cannot kill sentries or produce loot and XP", boss_gate_guards(g, guards) == 2 &&
                g->map.tiles[guards[1].y][guards[1].x] == tile && g->player.experience == xp && g->floor_item_count == 0 && g->enemy_count == 0);
            ASSERT("the guards preserve the real retreat to Oakhaven", path(g, g->map.stairs_up_x, g->map.stairs_up_y, 1) && g->location == LOCATION_TOWN_ROAD);
            approach_enter_region(g, routes[n]);
            ASSERT("guards leave the area's adventure entrance usable before boss victory", path(g, g->map.stairs_down_x, g->map.stairs_down_y, 1) && g->location == area);
            game_return_to_approach(g);
            ASSERT("returning from an unfinished adventure keeps its guards and retreat", g->location == LOCATION_APPROACH && boss_gate_guards(g, guards) == 2 &&
                path(g, g->map.stairs_up_x, g->map.stairs_up_y, 0));
            g->defeated_bosses |= 1u << (area == LOCATION_FOREST ? LOCATION_MOUNTAINS : LOCATION_FOREST);
            boss_gate_refresh(g);
            ASSERT("defeating the other area's boss does not remove this road's guards", boss_gate_guards(g, guards) == 2);
            g->debug_passages_unlocked = 1;
            boss_gate_refresh(g);
            ASSERT("debug passage unlock removes the noncombat sentries", boss_gate_guards(g, guards) == 0);
            g->debug_passages_unlocked = 0;
            boss_gate_refresh(g);
            g->defeated_bosses |= 1u << area;
            boss_gate_refresh(g);
            ASSERT("the correct boss victory removes both guards and restores their road tiles", boss_gate_guards(g, guards) == 0 &&
                map_is_walkable(&g->map, guards[0].x, guards[0].y) && map_is_walkable(&g->map, guards[1].x, guards[1].y));
            int ex = area == LOCATION_FOREST ? APPROACH_W - 1 : 0;
            ASSERT("the opened road reaches the onward town by actual travel", path(g, ex, APPROACH_H / 2, 1) && (area == LOCATION_FOREST ? g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_DAWNMERE : approach_sunscar_road(g)));
        }
    }
    if (save_exists(GATE_SLOT)) {
        ASSERT("guard save test slot is unused", 0);
        return;
    }
    for (int n = 0; n < 2; n++) {
        game_init(g);
        approach_enter_region(g, routes[n]);
        Location area = approach_route(g)->area;
        RoadCheckpoint guards[2];
        boss_gate_guards(g, guards);
        int dx = guards[1].x < APPROACH_W / 2 ? 1 : -1;
        // Simulate a pre-sentry save with its player and loot beside the beam.
        for (int i = 0; i < 2; i++) {
            g->map.tiles[guards[i].y][guards[i].x] = TILE_TOWN_PATH;
        }
        g->player.x = guards[0].x - dx;
        g->player.y = guards[0].y;
        g->player.hp = 43;
        g->gold = 789;
        g->aldric_party_state = ALDRIC_FOLLOWING;
        g->aldric_recruited = 1;
        g->aldric_x = guards[0].x;
        g->aldric_y = guards[0].y;
        g->aldric_hp = 77;
        g->floor_item_count = 2;
        for (int i = 0; i < 2; i++) {
            int x = i ? g->player.x : guards[1].x;
            g->floor_items[i] = (FloorItem){.active = 1, .x = x, .y = guards[1].y, .underlying_tile = TILE_TOWN_PATH, .item = item_make_health_potion()};
            g->map.tiles[guards[1].y][x] = TILE_ITEM;
        }
        map_mark_explored(&g->map, 30, 5);
        ASSERT("loading a pre-sentry save adds its guards and moves the player safely in front", save_game(g, GATE_SLOT) && load_game(loaded, GATE_SLOT) &&
            boss_gate_guards(loaded, guards) == 2 && loaded->player.x == guards[1].x + dx && loaded->player.y == guards[1].y &&
            loaded->gold == 789 && loaded->player.hp == 43 && map_is_explored(&loaded->map, 30, 5));
        ASSERT("save restoration preserves Aldric's wounds without leaving him in a guard", loaded->aldric_hp == 77 &&
            map_is_walkable(&loaded->map, loaded->aldric_x, loaded->aldric_y));
        ASSERT("saved loot on or behind sentries moves onto reachable road without ghost items", loaded->floor_items[0].active && loaded->floor_items[1].active &&
            path(loaded, loaded->floor_items[0].x, loaded->floor_items[0].y, 0) && path(loaded, loaded->floor_items[1].x, loaded->floor_items[1].y, 0) &&
            loaded->map.tiles[g->player.y][g->player.x] == TILE_TOWN_PATH);
        ASSERT("closed sentries survive a current save round trip", save_game(loaded, GATE_SLOT) && load_game(g, GATE_SLOT) && boss_gate_guards(g, guards) == 2);
        g->defeated_bosses |= 1u << area;
        boss_gate_refresh(g);
        ASSERT("save/load cannot restore guards after their boss was defeated", save_game(g, GATE_SLOT) && load_game(loaded, GATE_SLOT) && boss_gate_guards(loaded, guards) == 0 && boss_gate_open(loaded, area));
    }
    remove("saves/savegame_99311.json");
}

static void saved_connections(GameState *g, GameState *loaded) {
    ASSERT("boss-gate migration test slot is unused", !save_exists(GATE_SLOT));
    if (save_exists(GATE_SLOT)) {
        return;
    }
    for (int castle = 1; castle < 2; castle++) {
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

static int labyrinth_entrances(const GameState *g) {
    int count = 0;
    for (int y = 0; y < APPROACH_H; y++) {
        for (int x = 0; x < APPROACH_W; x++) {
            count += g->map.tiles[y][x] == TILE_LABYRINTH_ENTRANCE;
        }
    }
    return count;
}

static void deferred_labyrinth(GameState *g, GameState *loaded) {
    const int routes[] = {0, 4};
    const int seeds[] = {17, 107, 907};
    for (int n = 0; n < 2; n++) {
        for (int seed = 0; seed < 3; seed++) {
            for (int flags = 0; flags < 4; flags++) {
                game_init(g);
                srand(seeds[seed]);
                g->rook_quest_state = flags & 1;
                g->debug_passages_unlocked = flags == 3;
                g->defeated_bosses = flags == 2 ? 1u << LOCATION_FOREST : 0;
                approach_enter_region(g, routes[n]);
                ASSERT("normal, old-quest, victorious and debug forest roads have no labyrinth entrance or spur", !labyrinth_entrances(g) &&
                    g->map.tiles[APPROACH_LABYRINTH_Y + 1][APPROACH_LABYRINTH_X] != TILE_TOWN_PATH);
                ASSERT("the forest entrance remains usable through actual movement after deferring the labyrinth", path(g, g->map.stairs_down_x, g->map.stairs_down_y, 1) &&
                    g->location == LOCATION_FOREST && g->level == 1);
                game_return_to_approach(g);
                ASSERT("returning from the forest never restores the retired entrance", g->location == LOCATION_APPROACH && !labyrinth_entrances(g));
                int open = boss_gate_open(g, LOCATION_FOREST);
                int onward_x = n ? 0 : APPROACH_W - 1;
                ASSERT("only boss victory or debug opens onward travel after labyrinth removal", path(g, onward_x, APPROACH_H / 2, 0) == open);
                if (open) {
                    ASSERT("onward forest travel still reaches the opposite town exterior", path(g, onward_x, APPROACH_H / 2, 1) &&
                        g->location == LOCATION_TOWN_ROAD && g->road_town == (n ? LOCATION_TOWN : LOCATION_DAWNMERE));
                    approach_enter_region(g, routes[n]);
                }
                ASSERT("the actual retreat to the original town remains usable", path(g, g->map.stairs_up_x, g->map.stairs_up_y, 1) &&
                    g->location == LOCATION_TOWN_ROAD && g->road_town == (n ? LOCATION_DAWNMERE : LOCATION_TOWN));
            }
        }
    }
    ASSERT("deferred labyrinth save slot is unused", !save_exists(GATE_SLOT));
    if (save_exists(GATE_SLOT)) {
        return;
    }
    for (int n = 0; n < 2; n++) {
        game_init(g);
        approach_enter_region(g, routes[n]);
        game_enter_labyrinth(g);
        g->enemies[0].hp = 37;
        game_leave_labyrinth(g);
        g->rook_quest_state = 1;
        g->rook_labyrinth_switches = 3;
        g->gold = 789;
        g->player.hp = 43;
        g->player.x = APPROACH_LABYRINTH_X;
        g->player.y = APPROACH_LABYRINTH_Y + 1;
        for (int y = APPROACH_LABYRINTH_Y - 2; y <= APPROACH_LABYRINTH_Y; y++) {
            for (int x = APPROACH_LABYRINTH_X - 1; x <= APPROACH_LABYRINTH_X + 1; x++) {
                g->map.tiles[y][x] = TILE_WALL;
            }
        }
        g->map.tiles[APPROACH_LABYRINTH_Y][APPROACH_LABYRINTH_X] = TILE_LABYRINTH_ENTRANCE;
        g->map.tiles[g->player.y][g->player.x] = TILE_TOWN_PATH;
        map_mark_explored(&g->map, 30, 5);
        g->floor_item_count = 2;
        for (int i = 0; i < 2; i++) {
            int y = APPROACH_LABYRINTH_Y - i * 2;
            g->floor_items[i] = (FloorItem){.active = 1, .x = APPROACH_LABYRINTH_X, .y = y,
                .underlying_tile = i ? TILE_WALL : TILE_LABYRINTH_ENTRANCE, .item = item_make_health_potion()};
            g->map.tiles[y][APPROACH_LABYRINTH_X] = TILE_ITEM;
        }
        TileType unchanged = g->map.tiles[5][30];
        ASSERT("saved forest roads lose only the retired entrance while retaining wounds, gold, exploration and quest progress", save_game(g, GATE_SLOT) &&
            load_game(loaded, GATE_SLOT) && !labyrinth_entrances(loaded) && loaded->player.hp == 43 && loaded->gold == 789 &&
            loaded->rook_quest_state == 1 && loaded->rook_labyrinth_switches == 3 && map_is_explored(&loaded->map, 30, 5) && loaded->map.tiles[5][30] == unchanged);
        ASSERT("deferred labyrinth caches retain their saved enemies and wounds", loaded->labyrinth_cache[0].valid && loaded->labyrinth_cache[0].enemies[0].hp == 37);
        ASSERT("loot on the retired entrance and covered by new trees remains reachable", loaded->floor_items[0].active && loaded->floor_items[1].active &&
            loaded->floor_items[0].underlying_tile == TILE_TOWN_FLOOR && loaded->floor_items[1].underlying_tile == TILE_TOWN_PATH &&
            path(loaded, loaded->floor_items[0].x, loaded->floor_items[0].y, 0) && path(loaded, loaded->floor_items[1].x, loaded->floor_items[1].y, 0));
        ASSERT("saving the repaired approach cannot restore a labyrinth entrance", save_game(loaded, GATE_SLOT) && load_game(g, GATE_SLOT) && !labyrinth_entrances(g));
        game_enter_labyrinth(g);
        ASSERT("a legacy save inside the deferred labyrinth remains loadable", save_game(g, GATE_SLOT) && load_game(loaded, GATE_SLOT) && loaded->location == LOCATION_LABYRINTH);
        game_leave_labyrinth(loaded);
        ASSERT("a legacy labyrinth expedition can leave onto the real forest road and retreat", loaded->location == LOCATION_APPROACH && !labyrinth_entrances(loaded) &&
            map_is_walkable(&loaded->map, loaded->player.x, loaded->player.y) && path(loaded, loaded->map.stairs_up_x, loaded->map.stairs_up_y, 1) && loaded->location == LOCATION_TOWN_ROAD);
    }
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
    const Location optional[] = {LOCATION_LABYRINTH, LOCATION_GLASSDEEP, LOCATION_STORMWATCH,
        LOCATION_IRONFANG, LOCATION_GIANTS_CROWN, LOCATION_DESERT, LOCATION_FROSTFELL, LOCATION_DRAGONSPINE,
        LOCATION_ASHEN, LOCATION_TEMPLE, LOCATION_ISLAND, LOCATION_CROWNROAD, LOCATION_CROWNROAD_WEST,
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
    guarded_roads(g, loaded);
    deferred_labyrinth(g, loaded);
    free(g);
    free(loaded);
}
