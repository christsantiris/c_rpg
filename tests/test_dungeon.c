#include "test_utils.h"
#include "../src/game/map.h"
#include "../src/game/game.h"
#include "../src/game/actions.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

void test_dungeon(void) {
    printf("Dungeon generation tests:\n");

    srand((unsigned)time(NULL));

    for (int run = 0; run < 5; run++) {
        Map m;
        map_generate(&m, 1);

        ASSERT("at least min rooms generated", m.room_count >= MIN_ROOMS);
        ASSERT("at most max rooms generated",  m.room_count <= MAX_ROOMS);

        ASSERT("stairs up tile is set",
            m.tiles[m.stairs_up_y][m.stairs_up_x] == TILE_STAIRS_UP);
        ASSERT("stairs down tile is set",
            m.tiles[m.stairs_down_y][m.stairs_down_x] == TILE_STAIRS_DOWN);

        ASSERT("stairs up and down are not on same tile",
            !(m.stairs_up_x == m.stairs_down_x &&
              m.stairs_up_y == m.stairs_down_y));

        ASSERT("stairs up within bounds",
            m.stairs_up_x >= 0 && m.stairs_up_x < MAP_W &&
            m.stairs_up_y >= 0 && m.stairs_up_y < MAP_H);
        ASSERT("stairs down within bounds",
            m.stairs_down_x >= 0 && m.stairs_down_x < MAP_W &&
            m.stairs_down_y >= 0 && m.stairs_down_y < MAP_H);
    }

    GameState g = {0};
    g.location = LOCATION_DUNGEON;
    g.level = DUNGEON_DEPTH;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    int matching_bosses = 0;
    for (int j = 0; j < g.enemy_count; j++) {
        if (g.enemies[j].is_boss && g.enemies[j].type == ENEMY_LICH_KING) {
            matching_bosses++;
        }
    }
    ASSERT("final floor contains the Lich King", matching_bosses == 1);
    ASSERT("boss floor respects enemy capacity", g.enemy_count <= AREA_ENEMY_LIMIT);
    int locked_doors = 0;
    int door_x = 0;
    int door_y = 0;
    int dungeon_keys = 0;
    int key_x = 0;
    int key_y = 0;
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            if (g.map.tiles[y][x] == TILE_LOCKED_DOOR) {
                locked_doors++;
                door_x = x;
                door_y = y;
            } else if (g.map.tiles[y][x] == TILE_DUNGEON_KEY) {
                dungeon_keys++;
                key_x = x;
                key_y = y;
            }
    ASSERT("final boss room has one locked door", locked_doors == 1);
    ASSERT("final floor contains one dungeon key", dungeon_keys == 1);

    Room *boss_room = &g.map.rooms[g.map.room_count - 1];
    int perimeter_openings = 0;
    for (int y = boss_room->y; y < boss_room->y + boss_room->h; y++) {
        for (int x = boss_room->x; x < boss_room->x + boss_room->w; x++) {
            int perimeter = x == boss_room->x ||
                x == boss_room->x + boss_room->w - 1 ||
                y == boss_room->y ||
                y == boss_room->y + boss_room->h - 1;
            if (perimeter && g.map.tiles[y][x] != TILE_WALL)
                perimeter_openings++;
        }
    }
    ASSERT("boss chamber has exactly one perimeter opening",
        perimeter_openings == 1);

    int boss_index = -1;
    for (int i = 0; i < g.enemy_count; i++)
        if (g.enemies[i].is_boss) boss_index = i;
    int boss_x = g.enemies[boss_index].x;
    int boss_y = g.enemies[boss_index].y;
    ASSERT("Lich King starts inside the sealed chamber",
        boss_x > boss_room->x && boss_x < boss_room->x + boss_room->w - 1 &&
        boss_y > boss_room->y && boss_y < boss_room->y + boss_room->h - 1);
    int enemy_on_key = 0;
    for (int i = 0; i < g.enemy_count; i++)
        if (g.enemies[i].active &&
            g.enemies[i].x == key_x && g.enemies[i].y == key_y)
            enemy_on_key = 1;
    ASSERT("dungeon key is not hidden beneath an enemy", !enemy_on_key);
    g.player.x = key_x;
    g.player.y = key_y;
    ASSERT("walking onto dungeon key does not collect it",
        !g.dungeon_key_found &&
        g.map.tiles[key_y][key_x] == TILE_DUNGEON_KEY);
    Action pick_up_key = {ACTION_PICK_UP, 0, 0};
    action_resolve_player(&g, pick_up_key);
    ASSERT("P picks up dungeon key", g.dungeon_key_found);
    ASSERT("picked-up dungeon key leaves a floor tile",
        g.map.tiles[key_y][key_x] == TILE_FLOOR);
    action_resolve_enemies(&g);
    ASSERT("Lich King remains dormant behind locked door",
        g.enemies[boss_index].x == boss_x && g.enemies[boss_index].y == boss_y &&
        g.enemies[boss_index].move_timer == 0);

    for (int i = 0; i < g.enemy_count; i++)
        if (!g.enemies[i].is_boss) g.enemies[i].active = 0;
    game_update_level_progress(&g);
    locked_doors = 0;
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            if (g.map.tiles[y][x] == TILE_LOCKED_DOOR) locked_doors++;
    ASSERT("clearing regular enemies does not unlock boss door",
        locked_doors == 1);

    g.dungeon_key_found = 1;
    g.player.x = door_x;
    g.player.y = door_y + 1;
    Action unlock = {ACTION_MOVE, door_x, door_y};
    action_resolve_player(&g, unlock);
    ASSERT("dungeon key unlocks boss door",
        g.map.tiles[door_y][door_x] == TILE_FLOOR);
    ASSERT("dungeon key is consumed", g.dungeon_key_found == 0);

    for (int i = 0; i < g.enemy_count; i++)
        if (!g.enemies[i].is_boss) g.enemies[i].active = 0;
    g.player.x = boss_room->x + 1;
    g.player.y = boss_room->y + 1;
    if (g.player.x == boss_x && g.player.y == boss_y) g.player.y++;
    g.player.hp = 100;
    g.player.defense = 0;
    action_resolve_enemies(&g);
    action_resolve_enemies(&g);
    ASSERT("Lich King attacks after player enters chamber", g.player.hp < 100);
    ASSERT("Lich King holds position inside his chamber",
        g.enemies[boss_index].x == boss_x &&
        g.enemies[boss_index].y == boss_y);

    for (int level = 1; level < DUNGEON_DEPTH; level++) {
        g.level = level;
        map_generate(&g.map, level);
        enemies_spawn(&g);
        int bosses = 0;
        for (int j = 0; j < g.enemy_count; j++)
            if (g.enemies[j].is_boss) bosses++;
        ASSERT("early dungeon floor has no boss", bosses == 0);
    }
}

void test_dungeon_exit_distance(void) {
    printf("Dungeon exit distance tests:\n");
    int exits_separated = 1;
    for (int seed = 1; seed <= 256 && exits_separated; seed++) {
        srand(seed);
        for (int level = 1; level <= DUNGEON_DEPTH; level++) {
            Map m;
            map_generate(&m, level);
            int distance = abs(m.stairs_down_x - m.stairs_up_x) +
                abs(m.stairs_down_y - m.stairs_up_y);
            if (distance < 45 || m.room_count < MIN_ROOMS) {
                exits_separated = 0;
                break;
            }
        }
    }
    ASSERT("dungeon exits stay away from entrances across generated floors",
        exits_separated);
}

static int dungeon_tile_reachable(const Map *m, int target_x, int target_y) {
    unsigned char seen[MAP_H][MAP_W] = {{0}};
    int queue[MAP_W * MAP_H];
    int head = 0;
    int tail = 0;
    int start_x = m->stairs_up_x;
    int start_y = m->stairs_up_y;
    if (!map_is_walkable(m, start_x, start_y)) {
        return 0;
    }
    seen[start_y][start_x] = 1;
    queue[tail++] = start_y * MAP_W + start_x;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail) {
        int cell = queue[head++];
        int x = cell % MAP_W;
        int y = cell / MAP_W;
        if (x == target_x && y == target_y) {
            return 1;
        }
        for (int side = 0; side < 4; side++) {
            int nx = x + dx[side];
            int ny = y + dy[side];
            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H ||
                seen[ny][nx] || !map_is_walkable(m, nx, ny)) {
                continue;
            }
            seen[ny][nx] = 1;
            queue[tail++] = ny * MAP_W + nx;
        }
    }
    if (target_x < 0 && target_y < 0) {
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (map_is_walkable(m, x, y) && !seen[y][x]) {
                    return 0;
                }
            }
        }
        return 1;
    }
    return 0;
}

static int dungeon_walk_to(GameState *g, int tx, int ty) {
    int parents[MAP_W * MAP_H];
    int queue[MAP_W * MAP_H];
    for (int i = 0; i < MAP_W * MAP_H; i++) {
        parents[i] = -1;
    }
    int start = g->player.y * MAP_W + g->player.x;
    int target = ty * MAP_W + tx;
    int head = 0;
    int tail = 0;
    parents[start] = start;
    queue[tail++] = start;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail && parents[target] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + dx[side];
            int y = cell / MAP_W + dy[side];
            if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
                continue;
            }
            int next = y * MAP_W + x;
            int unlock = next == target && g->dungeon_key_found && g->map.tiles[y][x] == TILE_LOCKED_DOOR;
            if (parents[next] >= 0 || (!map_is_walkable(&g->map, x, y) && !unlock)) {
                continue;
            }
            parents[next] = cell;
            queue[tail++] = next;
        }
    }
    if (parents[target] < 0) {
        return 0;
    }
    int steps = 0;
    for (int cell = target; cell != start; cell = parents[cell]) {
        queue[steps++] = cell;
    }
    while (steps > 0) {
        int cell = queue[--steps];
        action_resolve_player(g, (Action){ACTION_MOVE, cell % MAP_W, cell / MAP_W});
        if (g->player.x != cell % MAP_W || g->player.y != cell / MAP_W) {
            return 0;
        }
    }
    return 1;
}

void test_dungeon_exit_reachability(void) {
    printf("Dungeon exit reachability tests:\n");
    int all_routes_open = 1;
    int no_gates = 1;
    for (int seed = 1; seed <= 512; seed++) {
        srand(seed);
        for (int level = 1; level < DUNGEON_DEPTH; level++) {
            Map m;
            map_generate(&m, level);
            if (!dungeon_tile_reachable(&m, m.stairs_down_x, m.stairs_down_y)) {
                all_routes_open = 0;
            }
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    if (m.tiles[y][x] == TILE_DUNGEON_GATE ||
                        m.tiles[y][x] == TILE_DUNGEON_SWITCH_OFF ||
                        m.tiles[y][x] == TILE_DUNGEON_SWITCH_ON) {
                        no_gates = 0;
                    }
                }
            }
        }
    }
    ASSERT("generated dungeon exits are reachable from the entrance",
        all_routes_open);
    ASSERT("no generated dungeon floor has a portcullis or floor switch",
        no_gates);

    int keys_reachable = 1;
    int doors_reachable = 1;
    int rooms_reachable = 1;
    int boss_sealed = 1;
    int boss_reachable = 1;
    int first_failure = 0;
    for (int seed = 1; seed <= 2048; seed++) {
        Map m;
        srand(seed);
        map_generate(&m, DUNGEON_DEPTH);
        int key_x = -1;
        int key_y = -1;
        int door_x = -1;
        int door_y = -1;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (m.tiles[y][x] == TILE_DUNGEON_KEY) {
                    key_x = x;
                    key_y = y;
                } else if (m.tiles[y][x] == TILE_LOCKED_DOOR) {
                    door_x = x;
                    door_y = y;
                }
            }
        }
        int key_open = key_x >= 0 && dungeon_tile_reachable(&m, key_x, key_y);
        keys_reachable &= key_open;
        int door_open = 0;
        const int dx[4] = {0, 1, 0, -1};
        const int dy[4] = {-1, 0, 1, 0};
        if (door_x >= 0) {
            for (int side = 0; side < 4; side++) {
                door_open |= dungeon_tile_reachable(&m, door_x + dx[side], door_y + dy[side]);
            }
        }
        doors_reachable &= door_open;
        for (int room = 0; room < m.room_count - 1; room++) {
            int x;
            int y;
            map_room_center(&m.rooms[room], &x, &y);
            rooms_reachable &= dungeon_tile_reachable(&m, x, y);
        }
        boss_sealed &= !dungeon_tile_reachable(&m, m.stairs_down_x, m.stairs_down_y);
        if (!first_failure && (!key_open || !door_open)) {
            first_failure = seed;
            printf("  First blocked boss route: seed %d, key (%d,%d), door (%d,%d)\n", seed, key_x, key_y, door_x, door_y);
        }
        if (door_x >= 0) {
            m.tiles[door_y][door_x] = TILE_FLOOR;
        }
        int boss_open = dungeon_tile_reachable(&m, m.stairs_down_x, m.stairs_down_y);
        if (!first_failure && !boss_open) {
            first_failure = seed;
            printf("  First blocked opened door: seed %d, door (%d,%d)\n", seed, door_x, door_y);
        }
        boss_reachable &= boss_open;
    }
    ASSERT("final-floor key can be reached before opening the boss door across 2048 seeds", keys_reachable);
    ASSERT("final-floor boss door has a reachable approach across 2048 seeds", doors_reachable);
    ASSERT("sealing the boss chamber does not isolate earlier rooms", rooms_reachable);
    ASSERT("the boss remains inaccessible until his door is unlocked", boss_sealed);
    ASSERT("unlocking the door provides a path to the boss and exit across 2048 seeds", boss_reachable);

}

void test_return_to_town_spell(void) {
    printf("Return to Town spell tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    g.player.x = TOWN_W - 2;
    g.player.y = 12;
    action_resolve_player(&g, (Action){ACTION_MOVE, TOWN_W - 1, 12});
    ASSERT("east OakHaven exit enters dungeon", g.location == LOCATION_DUNGEON);
    int origin_x = g.player.x;
    int origin_y = g.player.y;
    g.player.known_spells[0] = spell_make_return_to_town();
    g.player.known_spell_count = 1;
    g.player.equipped_spell = 0;
    g.player.mp = 0;

    Action cast = {ACTION_CAST_SPELL, 0, 0};
    action_resolve_player(&g, cast);
    ASSERT("zero-mana return spell reaches town", g.location == LOCATION_TOWN);
    ASSERT("return spell leaves a portal beside the dungeon entrance",
        g.map.tiles[13][TOWN_W - 3] == TILE_PORTAL);
    ASSERT("dungeon return spell arrives at east OakHaven road",
        g.player.x == TOWN_W - 2 && g.player.y == 12);
    ASSERT("return portal remains active", g.portal_active == 1);
    ASSERT("unusable dungeon end of portal is hidden",
        g.level_cache[0].map.tiles[origin_y][origin_x] != TILE_PORTAL);

    Action enter = {ACTION_MOVE, TOWN_W - 3, 13};
    action_resolve_player(&g, enter);
    ASSERT("town portal returns to dungeon", g.location == LOCATION_DUNGEON);
    ASSERT("portal returns to casting position",
        g.player.x == origin_x && g.player.y == origin_y);
    ASSERT("dungeon portal closes behind player",
        g.map.tiles[origin_y][origin_x] != TILE_PORTAL);
    ASSERT("portal closes after return trip", g.portal_active == 0);
    ASSERT("closed portal is removed from the cached dungeon floor",
        g.level_cache[0].map.tiles[origin_y][origin_x] != TILE_PORTAL);

    game_enter_coast(&g);
    for (int level = 1; level < COAST_DEPTH; level++) {
        game_descend(&g);
    }
    game_open_town_portal(&g);
    ASSERT("coast portal remembers stage five",
        g.portal_active && g.portal_level == COAST_DEPTH);
    game_enter_coast(&g);
    game_return_to_town(&g);
    ASSERT("coast portal survives another expedition",
        g.map.tiles[TOWN_H - 3][21] == TILE_PORTAL);
    game_use_town_portal(&g);
    ASSERT("coast portal returns to stage five",
        g.location == LOCATION_COAST && g.level == COAST_DEPTH);

    map_room_center(&g.map.rooms[0], &g.player.x, &g.player.y);
    int blocked_x = g.player.x;
    int blocked_y = g.player.y;
    game_open_town_portal(&g);
    g.coast_cache[COAST_DEPTH - 1].map.tiles[blocked_y][blocked_x] = TILE_COAST_WALL;
    game_use_town_portal(&g);
    ASSERT("portal avoids a blocked destination in the cached map",
        g.location == LOCATION_COAST && g.level == COAST_DEPTH &&
        (g.player.x != blocked_x || g.player.y != blocked_y) &&
        map_is_walkable(&g.map, g.player.x, g.player.y) &&
        g.map.tiles[blocked_y][blocked_x] == TILE_COAST_WALL);
}

void test_final_dungeon_exit(void) {
    printf("Final dungeon exit tests:\n");
    GameState g;
    game_init(&g);
    g.location = LOCATION_DUNGEON;
    g.level = DUNGEON_DEPTH;
    g.max_level_reached = DUNGEON_DEPTH;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);

    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    Action a = {ACTION_ASCEND, 0, 0};
    action_resolve_player(&g, a);
    ASSERT("living Lich King seals the special upward stairs",
        g.location == LOCATION_DUNGEON && g.level == DUNGEON_DEPTH &&
        g.map.tiles[g.player.y][g.player.x] == TILE_DUNGEON_STAIRS_SEALED);

    g.enemy_count = 2;
    g.enemies[0] = (Enemy){
        .x = 12, .y = 10, .active = 1, .hp = 1, .max_hp = 1,
        .type = ENEMY_LICH_KING, .is_boss = 1
    };
    g.enemies[1] = (Enemy){
        .x = 20, .y = 20, .active = 1, .hp = 10,
        .type = ENEMY_SKELETON
    };
    g.player.x = 10;
    g.player.y = 10;
    g.player.last_dx = 1;
    g.player.last_dy = 0;
    g.inventory_count = 1;
    g.inventory[0] = item_make_bow();
    g.player.arrows = MAX_ARROWS;
    g.equipped_main_hand = 0;
    for (int x = 10; x <= 12; x++) {
        g.map.tiles[10][x] = TILE_FLOOR;
    }
    g.map.stairs_down_x = 12;
    g.map.stairs_down_y = 10;
    g.map.tiles[10][12] = TILE_DUNGEON_STAIRS_SEALED;
    action_resolve_player(&g, (Action){ACTION_RANGED_ATTACK, 0, 0});
    ASSERT("defeating the Lich opens the return exit with regular enemies alive",
        !g.enemies[0].active && g.enemies[1].active && !g.level_cleared &&
        g.floor_items[0].underlying_tile == TILE_DUNGEON_STAIRS_RETURN &&
        g.floor_items[1].underlying_tile == TILE_DUNGEON_STAIRS_RETURN);

    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    action_resolve_player(&g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("picking up boss gold preserves the unlocked exit",
        g.inventory_count == 1 && g.gold == 25 &&
        g.floor_items[1].underlying_tile == TILE_DUNGEON_STAIRS_RETURN);
    action_resolve_player(&g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("picking up boss equipment preserves the unlocked exit",
        g.inventory_count == 2 && g.map.tiles[10][12] == TILE_DUNGEON_STAIRS_RETURN);
    action_resolve_player(&g, (Action){ACTION_DESCEND, 0, 0});
    ASSERT("the stairs-down control does not activate the upward return staircase",
        g.location == LOCATION_DUNGEON && g.level == DUNGEON_DEPTH);
    action_resolve_player(&g, a);
    ASSERT("final exit returns player to town", g.location == LOCATION_TOWN);
    ASSERT("final exit returns at east OakHaven road",
        g.player.x == TOWN_W - 2 && g.player.y == 12);
    ASSERT("final stairs return directly to town without changing the dungeon floor",
        g.level == DUNGEON_DEPTH);
    ASSERT("leaving the dungeon preserves surviving enemies and uncleared status",
        g.level_cache[DUNGEON_DEPTH - 1].enemies[1].active &&
        !g.level_cache[DUNGEON_DEPTH - 1].level_cleared);
}

void test_enemy_movement_collision(void) {
    printf("Enemy movement collision tests:\n");
    static GameState g;
    game_init(&g);
    g.location = LOCATION_DUNGEON;
    map_generate(&g.map, 1);
    g.player.x = 10;
    g.player.y = 10;
    g.enemy_count = 2;
    g.enemies[0] = (Enemy){
        .x = 8, .y = 10, .active = 1, .type = ENEMY_SKELETON
    };
    g.enemies[1] = (Enemy){
        .x = 9, .y = 10, .active = 1, .type = ENEMY_LICH_KING,
        .is_boss = 1
    };

    action_resolve_enemies(&g);

    ASSERT("enemy cannot move onto boss tile",
        !(g.enemies[0].x == g.enemies[1].x &&
          g.enemies[0].y == g.enemies[1].y));
    ASSERT("blocked enemy remains in place",
        g.enemies[0].x == 8 && g.enemies[0].y == 10);

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g.map.tiles[y][x] = TILE_FOREST_WALL;
        }
    }
    g.location = LOCATION_FOREST;
    g.level = 2;
    g.map.tiles[5][5] = TILE_FOREST_FLOOR;
    g.map.tiles[6][6] = TILE_FOREST_FLOOR;
    g.map.tiles[7][7] = TILE_FOREST_FLOOR;
    g.player.x = 7;
    g.player.y = 7;
    g.enemy_count = 1;
    g.enemies[0] = (Enemy){
        .x = 5, .y = 5, .active = 1, .type = ENEMY_GIANT_SPIDER,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    action_resolve_enemies(&g);
    ASSERT("forest enemy cannot cut diagonally through trees",
        g.enemies[0].x == 5 && g.enemies[0].y == 5);
    g.map.tiles[5][6] = TILE_FOREST_FLOOR;
    g.map.tiles[6][7] = TILE_FOREST_FLOOR;
    action_resolve_enemies(&g);
    ASSERT("forest enemy follows an orthogonal path around the trees",
        g.enemies[0].x == 6 && g.enemies[0].y == 5);

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g.map.tiles[y][x] = TILE_WALL;
        }
    }
    for (int x = 4; x <= 8; x++) {
        g.map.tiles[5][x] = TILE_FLOOR;
    }
    for (int y = 5; y <= 8; y++) {
        g.map.tiles[y][8] = TILE_FLOOR;
    }
    for (int x = 8; x <= 12; x++) {
        g.map.tiles[8][x] = TILE_FLOOR;
    }
    g.location = LOCATION_DUNGEON;
    g.level = 1;
    g.player.x = 12;
    g.player.y = 8;
    g.player.hp = 100;
    g.enemy_count = 2;
    g.enemies[0] = (Enemy){
        .x = 6, .y = 5, .active = 1, .type = ENEMY_SKELETON,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    g.enemies[1] = (Enemy){
        .x = 5, .y = 5, .active = 1, .type = ENEMY_SKELETON,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    for (int turn = 0; turn < 3; turn++) {
        action_resolve_enemies(&g);
    }
    ASSERT("enemy rounds an L-shaped corner toward the player",
        g.enemies[0].x == 8 && g.enemies[0].y == 6);
    ASSERT("queued enemy follows through the vacated corridor",
        g.enemies[1].x == 8 && g.enemies[1].y == 5);
    ASSERT("corner pursuit keeps enemies on separate tiles",
        g.enemies[0].x != g.enemies[1].x ||
        g.enemies[0].y != g.enemies[1].y);

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g.map.tiles[y][x] = TILE_FLOOR;
        }
    }
    g.player.x = 10;
    g.player.y = 10;
    g.player.hp = 100;
    g.enemy_count = 4;
    int pursuit_start_x[4] = {10, 8, 12, 10};
    int pursuit_start_y[4] = {7, 10, 10, 13};
    for (int i = 0; i < g.enemy_count; i++) {
        g.enemies[i] = (Enemy){
            .x = pursuit_start_x[i], .y = pursuit_start_y[i],
            .active = 1, .type = ENEMY_SKELETON,
            .hp = 10, .max_hp = 10, .attack = 1
        };
    }
    action_resolve_enemies(&g);
    int moved_pursuers = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        if (g.enemies[i].x != pursuit_start_x[i] ||
            g.enemies[i].y != pursuit_start_y[i]) {
            moved_pursuers++;
        }
    }
    ASSERT("only three nonadjacent enemies join pursuit",
        moved_pursuers == 3);

    g.enemy_count = 1;
    g.enemies[0] = (Enemy){
        .x = 10, .y = 22, .active = 1, .type = ENEMY_SKELETON,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    action_resolve_enemies(&g);
    ASSERT("unaware distant enemy holds position",
        g.enemies[0].x == 10 && g.enemies[0].y == 22);
    g.enemies[0].hp = 9;
    action_resolve_enemies(&g);
    ASSERT("damaged distant enemy pursues its attacker",
        g.enemies[0].x == 10 && g.enemies[0].y == 21);

    g.enemy_count = 2;
    g.enemies[0] = (Enemy){
        .x = 10, .y = 5, .active = 1, .type = ENEMY_GOBLIN_SCOUT,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    g.enemies[1] = (Enemy){
        .x = 10, .y = 4, .active = 1, .type = ENEMY_GIANT_SPIDER,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    action_resolve_enemies(&g);
    ASSERT("flankers split to opposite sides of a direct approach",
        g.enemies[0].x == 9 && g.enemies[0].y == 5 &&
        g.enemies[1].x == 11 && g.enemies[1].y == 4);

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g.map.tiles[y][x] = TILE_WALL;
        }
    }
    for (int y = 5; y <= 10; y++) {
        g.map.tiles[y][10] = TILE_FLOOR;
    }
    g.enemy_count = 1;
    g.enemies[0] = (Enemy){
        .x = 10, .y = 5, .active = 1, .type = ENEMY_GOBLIN_SCOUT,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    action_resolve_enemies(&g);
    ASSERT("flanker follows the corridor when no side route exists",
        g.enemies[0].x == 10 && g.enemies[0].y == 6);

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g.map.tiles[y][x] = TILE_FLOOR;
        }
    }
    g.enemy_count = 4;
    g.enemies[0] = (Enemy){
        .x = 10, .y = 8, .active = 1, .type = ENEMY_SKELETON,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    g.enemies[1] = (Enemy){
        .x = 8, .y = 10, .active = 1, .type = ENEMY_SKELETON,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    g.enemies[2] = (Enemy){
        .x = 10, .y = 4, .active = 1, .type = ENEMY_GOBLIN_ARCHER,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    g.enemies[3] = (Enemy){
        .x = 9, .y = 4, .active = 1, .type = ENEMY_HOBGOBLIN_GUARD,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    action_resolve_enemies(&g);
    ASSERT("protector joins pursuit near a ranged ally",
        abs(g.enemies[3].x - 10) + abs(g.enemies[3].y - 10) < 7);
    ASSERT("ranged ally moves when another enemy blocks its firing lane",
        (g.enemies[2].x != 10 || g.enemies[2].y != 4) &&
        g.enemies[2].move_timer == 1);

    g.enemy_count = 2;
    g.enemies[0] = (Enemy){
        .x = 10, .y = 5, .active = 1, .type = ENEMY_GOBLIN_SHAMAN,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    g.enemies[1] = (Enemy){
        .x = 11, .y = 5, .active = 1, .type = ENEMY_HOBGOBLIN_GUARD,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    action_resolve_enemies(&g);
    ASSERT("support enemy holds position near an ally",
        g.enemies[0].x == 10 && g.enemies[0].y == 5);

    g.enemy_count = 1;
    g.enemies[0] = (Enemy){
        .x = 9, .y = 10, .active = 1, .type = ENEMY_GOBLIN_SHAMAN,
        .hp = 10, .max_hp = 10, .attack = 1
    };
    int hp_before_support_retreat = g.player.hp;
    action_resolve_enemies(&g);
    ASSERT("support enemy retreats instead of entering melee",
        g.enemies[0].x == 8 && g.enemies[0].y == 10 &&
        g.player.hp == hp_before_support_retreat);

    Location locations[4] = {
        LOCATION_DUNGEON,
        LOCATION_FOREST,
        LOCATION_MOUNTAINS,
        LOCATION_COAST
    };
    for (int region = 0; region < 4; region++) {
        g.location = locations[region];
        g.level = DUNGEON_DEPTH;
        if (g.location == LOCATION_DUNGEON) {
            map_generate(&g.map, g.level);
        } else if (g.location == LOCATION_FOREST) {
            map_generate_forest(&g.map, g.level);
        } else if (g.location == LOCATION_MOUNTAINS) {
            map_generate_mountains(&g.map, g.level);
        } else {
            map_generate_coast(&g.map, g.level);
        }
        enemies_spawn(&g);
        int enemies_on_open_tiles = 1;
        for (int i = 0; i < g.enemy_count; i++) {
            if (g.enemies[i].active && !map_is_walkable(&g.map,
                g.enemies[i].x, g.enemies[i].y)) {
                enemies_on_open_tiles = 0;
            }
        }
        ASSERT("spawned enemies are never inside walls",
            enemies_on_open_tiles);
        if (g.location == LOCATION_FOREST) {
            int clear_of_trees = 1;
            for (int i = 0; i < g.enemy_count; i++) {
                Enemy *enemy = &g.enemies[i];
                if (!enemy->active) {
                    continue;
                }
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (abs(dx) + abs(dy) != 1) {
                            continue;
                        }
                        TileType tile = g.map.tiles[enemy->y + dy]
                            [enemy->x + dx];
                        if (tile == TILE_FOREST_WALL ||
                            tile == TILE_FOREST_HIDDEN_TRAIL) {
                            clear_of_trees = 0;
                        }
                    }
                }
            }
            ASSERT("forest enemies spawn clear of tree tiles", clear_of_trees);
        }
    }

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g.map.tiles[y][x] = TILE_FLOOR;
        }
    }
    g.location = LOCATION_DUNGEON;
    g.enemy_count = 0;
    g.player.x = 5;
    g.player.y = 5;
    g.player.hp = 100;
    g.map.tiles[5][7] = TILE_TRAP_HIDDEN;
    Action approach_trap = {ACTION_MOVE, 6, 5};
    action_resolve_player(&g, approach_trap);
    ASSERT("approaching a dungeon trap leaves it hidden",
        g.map.tiles[5][7] == TILE_TRAP_HIDDEN);

    Action step_on_trap = {ACTION_MOVE, 7, 5};
    action_resolve_player(&g, step_on_trap);
    TileType sprung_trap = g.map.tiles[5][7];
    ASSERT("stepping on a hidden trap reveals and triggers it",
        sprung_trap == TILE_TRAP_SPIKE ||
        sprung_trap == TILE_TRAP_FIRE ||
        sprung_trap == TILE_TRAP_POISON);

    g.map.tiles[5][8] = TILE_TRAP_REVEALED;
    action_resolve_player(&g, (Action){ACTION_MOVE, 8, 5});
    sprung_trap = g.map.tiles[5][8];
    ASSERT("deliberately visible pressure plates still trigger",
        sprung_trap == TILE_TRAP_SPIKE ||
        sprung_trap == TILE_TRAP_FIRE ||
        sprung_trap == TILE_TRAP_POISON);

    g.message_count = 0;
    g.location = LOCATION_TEMPLE;
    g.temple_alignment = 0;
    g.player.x = 5;
    g.player.y = 5;
    g.player.hp = 1;
    g.player.poison_turns = 0;
    g.map.tiles[5][5] = TILE_TEMPLE_FLOOR;
    g.map.tiles[5][6] = TILE_TEMPLE_SOLAR_TRAP;
    action_resolve_player(&g, (Action){ACTION_MOVE, 6, 5});
    ASSERT("fatal solar trap records its cause",
        g.player.hp <= 0 && g.message_count > 0 &&
        strstr(g.messages[g.message_count - 1], "Solar flame") != NULL);
    int death_messages = g.message_count;
    action_resolve_enemies(&g);
    ASSERT("enemy turns do not overwrite a recorded death cause",
        g.message_count == death_messages &&
        strstr(g.messages[g.message_count - 1], "Solar flame") != NULL);

    g.message_count = 0;
    g.location = LOCATION_DUNGEON;
    g.player.x = 5;
    g.player.y = 5;
    g.player.hp = 3;
    g.player.poison_turns = 1;
    g.map.tiles[5][5] = TILE_FLOOR;
    g.map.tiles[5][6] = TILE_FLOOR;
    action_resolve_player(&g, (Action){ACTION_MOVE, 6, 5});
    ASSERT("fatal poison records its cause",
        g.player.hp == 0 && g.message_count > 0 &&
        strstr(g.messages[g.message_count - 1], "Poison!") != NULL);
}

void test_new_dungeon_enemies(void) {
    printf("New dungeon enemy tests:\n");
    GameState g = {0};
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            g.map.tiles[y][x] = TILE_FLOOR;
    g.player.x = 10;
    g.player.y = 10;
    g.player.hp = 100;
    g.player.mp = 20;

    g.enemy_count = 1;
    g.enemies[0] = (Enemy){
        .x = 4, .y = 10, .active = 1, .type = ENEMY_CRYPT_BAT
    };
    action_resolve_enemies(&g);
    ASSERT("crypt bat flanks while moving two tiles",
        g.enemies[0].x == 5 && g.enemies[0].y == 9);

    g.enemies[0] = (Enemy){
        .x = 8, .y = 8, .active = 1, .type = ENEMY_WRAITH,
        .attack = 7
    };
    g.map.tiles[9][9] = TILE_WALL;
    action_resolve_enemies(&g);
    ASSERT("wraith routes around rather than passing through a wall",
        (g.enemies[0].x == 9 && g.enemies[0].y == 8) ||
        (g.enemies[0].x == 8 && g.enemies[0].y == 9));

    g.player.defense = 2;
    g.enemies[0] = (Enemy){
        .x = 10, .y = 7, .active = 1, .type = ENEMY_CRYPT_CONJURER,
        .attack = 7, .move_timer = 1
    };
    int hp_before = g.player.hp;
    action_resolve_enemies(&g);
    ASSERT("necromancer fires an aligned ranged bolt", g.player.hp < hp_before);

    g.enemy_count = 2;
    g.enemies[0] = (Enemy){
        .x = 6, .y = 6, .active = 1, .type = ENEMY_CRYPT_CONJURER,
        .attack = 7, .move_timer = 3
    };
    g.enemies[1] = (Enemy){
        .x = 3, .y = 3, .active = 0, .type = ENEMY_SKELETON,
        .hp = 0, .max_hp = 10
    };
    action_resolve_enemies(&g);
    ASSERT("necromancer revives a fallen skeleton", g.enemies[1].active == 1);
    ASSERT("revived skeleton returns at full health", g.enemies[1].hp == 10);
}

void test_stairs_locked(void) {
    printf("Dungeon stairs progression tests:\n");

    GameState g;
    game_init(&g);
    game_enter_dungeon(&g);

    ASSERT("level not cleared on start", g.level_cleared == 0);

    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    int level_before = g.level;

    Action a = {ACTION_DESCEND, 0, 0};
    action_resolve_player(&g, a);
    ASSERT("can descend without clearing the floor",
        g.level == level_before + 1);

    g.level = 2;
    map_generate(&g.map, g.level);
    int key_x = -1;
    int key_y = -1;
    int door_x = -1;
    int door_y = -1;
    int cache_x = -1;
    int cache_y = -1;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g.map.tiles[y][x] == TILE_CRYPT_KEY) {
                key_x = x;
                key_y = y;
            } else if (g.map.tiles[y][x] == TILE_CRYPT_DOOR) {
                door_x = x;
                door_y = y;
            } else if (g.map.tiles[y][x] == TILE_CRYPT_CACHE) {
                cache_x = x;
                cache_y = y;
            }
        }
    }
    ASSERT("dungeon floor two contains an optional locked crypt",
        key_x >= 0 && door_x >= 0 && cache_x >= 0);
    if (key_x < 0 || door_x < 0 || cache_x < 0) {
        return;
    }

    g.player.x = door_x - 1;
    g.player.y = door_y;
    Action enter_crypt = {ACTION_MOVE, door_x, door_y};
    action_resolve_player(&g, enter_crypt);
    ASSERT("crypt door remains locked without its key",
        g.map.tiles[door_y][door_x] == TILE_CRYPT_DOOR);

    g.player.x = key_x;
    g.player.y = key_y;
    Action collect_key = {ACTION_PICK_UP, 0, 0};
    action_resolve_player(&g, collect_key);
    ASSERT("P collects the crypt key", g.dungeon_crypt_keys == 1);

    g.player.x = door_x - 1;
    g.player.y = door_y;
    action_resolve_player(&g, enter_crypt);
    ASSERT("crypt key opens and is consumed by the door",
        g.map.tiles[door_y][door_x] == TILE_FLOOR &&
        g.dungeon_crypt_keys == 0);

    int gold_before = g.gold;
    g.player.x = cache_x;
    g.player.y = cache_y;
    Action loot_cache = {ACTION_PICK_UP, 0, 0};
    action_resolve_player(&g, loot_cache);
    ASSERT("P loots the crypt cache once",
        g.gold > gold_before &&
        g.map.tiles[cache_y][cache_x] == TILE_FLOOR);
}

static int save_old_dungeon(const GameState *g, int version) {
    const int slot = 99151;
    const char *path = "saves/savegame_99151.json";
    if (!save_game(g, slot)) {
        return 0;
    }
    FILE *file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc((size_t)size + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    size_t read = fread(buffer, 1, (size_t)size, file);
    buffer[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json) {
        return 0;
    }
    file = fopen(path, "wb");
    int written = file && fputs(json, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(json);
    return written;
}

static void blocked_dungeon(Map *m, int corner) {
    memset(m, 0, sizeof(*m));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_WALL;
        }
    }
    m->room_count = 3;
    m->rooms[0] = (Room){2, 18, 8, 8};
    m->rooms[1] = (Room){20, 2, 8, 8};
    m->rooms[2] = (Room){20, 14, 10, 14};
    for (int i = 0; i < m->room_count; i++) {
        const Room *room = &m->rooms[i];
        for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
            for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
                m->tiles[y][x] = TILE_FLOOR;
            }
        }
    }
    // An older corridor reached the boss room's side before it was sealed.
    for (int x = 6; x < 20; x++) {
        m->tiles[22][x] = TILE_FLOOR;
    }
    for (int y = 6; y < 14; y++) {
        m->tiles[y][25] = TILE_FLOOR;
    }
    m->stairs_up_x = 6;
    m->stairs_up_y = 22;
    m->stairs_down_x = 25;
    m->stairs_down_y = 21;
    m->tiles[22][6] = TILE_STAIRS_UP;
    m->tiles[21][25] = TILE_STAIRS_DOWN;
    m->tiles[6][24] = TILE_DUNGEON_KEY;
    m->tiles[14][corner ? 20 : 25] = corner == 2 ? TILE_FLOOR : TILE_LOCKED_DOOR;
    map_mark_explored(m, 19, 22);
}

void test_dungeon_boss_save_migration(void) {
    printf("Dungeon boss access and save migration tests:\n");
    static GameState g;
    static GameState loaded;
    for (int seed = 17; seed <= 35; seed += 18) {
        memset(&g, 0, sizeof(g));
        g.player.player_class = CLASS_WARRIOR;
        game_init(&g);
        g.location = LOCATION_DUNGEON;
        g.level = DUNGEON_DEPTH;
        srand(seed);
        map_generate(&g.map, g.level);
        g.player.x = g.map.stairs_up_x;
        g.player.y = g.map.stairs_up_y;
        int key_x = -1;
        int key_y = -1;
        int door_x = -1;
        int door_y = -1;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (g.map.tiles[y][x] == TILE_DUNGEON_KEY) {
                    key_x = x;
                    key_y = y;
                } else if (g.map.tiles[y][x] == TILE_LOCKED_DOOR) {
                    door_x = x;
                    door_y = y;
                }
            }
        }
        ASSERT("regression layout can be walked from its entrance to the key", key_x >= 0 && dungeon_walk_to(&g, key_x, key_y));
        action_resolve_player(&g, (Action){ACTION_PICK_UP, 0, 0});
        ASSERT("regression key is picked up with P and unlocks the reachable door", g.dungeon_key_found && door_x >= 0 &&
            dungeon_walk_to(&g, door_x, door_y) && !g.dungeon_key_found && g.map.tiles[door_y][door_x] == TILE_FLOOR);
        ASSERT("the unlocked chamber can be entered through normal movement", dungeon_walk_to(&g, g.map.stairs_down_x, g.map.stairs_down_y));
    }
    for (int cached = 0; cached < 2; cached++) {
        for (int corner = 0; corner < 3; corner++) {
            memset(&g, 0, sizeof(g));
            g.player.player_class = CLASS_WARRIOR;
            game_init(&g);
            g.location = LOCATION_DUNGEON;
            g.level = DUNGEON_DEPTH;
            blocked_dungeon(&g.map, corner);
            g.player.x = corner == 2 ? 20 : 6;
            g.player.y = corner == 2 ? 14 : 22;
            g.gold = 123;
            g.score = 456;
            g.elowen_quest_state = 2;
            g.elowen_seals_restored = 7;
            g.enemy_count = 1;
            g.enemies[0] = (Enemy){.type = ENEMY_LICH_KING, .is_boss = 1, .active = 1, .x = 26, .y = 21, .hp = 73, .max_hp = 100};
            snprintf(g.enemies[0].name, sizeof(g.enemies[0].name), "Lich King");
            g.floor_item_count = 1;
            g.floor_items[0] = (FloorItem){.active = 1, .x = 19, .y = 22, .underlying_tile = TILE_FLOOR, .item = item_make_health_potion()};
            g.map.tiles[22][19] = TILE_ITEM;
            LevelCache *cache = &g.level_cache[DUNGEON_DEPTH - 1];
            cache->valid = 1;
            cache->map = g.map;
            cache->enemy_count = 1;
            cache->enemies[0] = g.enemies[0];
            ASSERT("legacy fixture reproduces a key isolated by the boss chamber", !dungeon_tile_reachable(&g.map, 24, 6));
            if (cached) {
                game_return_to_town(&g);
            }
            ASSERT("version 105 dungeon saves load with repaired routes", save_old_dungeon(&g, 105) && load_game(&loaded, 99151));
            Map *map = cached ? &loaded.level_cache[DUNGEON_DEPTH - 1].map : &loaded.map;
            ASSERT("migration reconnects the old key without moving or collecting it", dungeon_tile_reachable(map, 24, 6) && map->tiles[6][24] == TILE_DUNGEON_KEY && !loaded.dungeon_key_found);
            ASSERT("migration preserves boss health, quest progress, money, and exploration", loaded.gold == g.gold && loaded.score == g.score &&
                loaded.elowen_quest_state == 2 && loaded.elowen_seals_restored == 7 && map_is_explored(map, 19, 22) &&
                loaded.level_cache[DUNGEON_DEPTH - 1].enemies[0].hp == 73 && loaded.level_cache[DUNGEON_DEPTH - 1].enemies[0].active);
            if (!cached) {
                ASSERT("active-floor migration preserves the player and dropped item", loaded.player.x == g.player.x && loaded.player.y == g.player.y &&
                    map_is_walkable(map, loaded.player.x, loaded.player.y) && loaded.enemies[0].hp == 73 &&
                    loaded.floor_items[0].active && map->tiles[22][19] == TILE_ITEM && loaded.floor_items[0].underlying_tile == TILE_FLOOR);
            }
            if (corner == 2) {
                ASSERT("an already unlocked corner door remains open and leads into the chamber", map->tiles[14][20] == TILE_FLOOR &&
                    dungeon_tile_reachable(map, map->stairs_down_x, map->stairs_down_y));
            } else {
                int x = corner ? 20 : 25;
                int y = corner ? 15 : 14;
                ASSERT("migration keeps the boss locked behind an approachable door", map->tiles[y][x] == TILE_LOCKED_DOOR &&
                    !dungeon_tile_reachable(map, map->stairs_down_x, map->stairs_down_y));
                map->tiles[y][x] = TILE_FLOOR;
                ASSERT("unlocking a migrated door makes the boss reachable", dungeon_tile_reachable(map, map->stairs_down_x, map->stairs_down_y));
            }
        }
    }
    remove("saves/savegame_99151.json");
}

void test_level_cache_cleared(void) {
    printf("Level cache cleared tests:\n");

    GameState g;
    game_init(&g);

    // Set up dungeon
    g.location = LOCATION_DUNGEON;
    g.level    = 1;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    g.player.x = g.map.stairs_up_x;
    g.player.y = g.map.stairs_up_y;

    // Kill all enemies to clear level
    for (int i = 0; i < g.enemy_count; i++)
        g.enemies[i].active = 0;
    g.level_cleared = 1;

    // Descend to level 2 — level 1 should be cached as cleared
    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    game_descend(&g);
    ASSERT("level is now 2",                    g.level == 2);
    ASSERT("level 1 cached as cleared",         g.level_cache[0].level_cleared == 1);
    ASSERT("level 2 not cleared",               g.level_cleared == 0);

    // Ascend back to level 1 — should restore cleared state
    g.player.x = g.map.stairs_up_x;
    g.player.y = g.map.stairs_up_y;
    game_ascend(&g);
    ASSERT("back on level 1",                   g.level == 1);
    ASSERT("level 1 restored as cleared",       g.level_cleared == 1);
}

static void open_test_dungeon_doors(Map *m) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (m->tiles[y][x] == TILE_LOCKED_DOOR || m->tiles[y][x] == TILE_CRYPT_DOOR) {
                m->tiles[y][x] = TILE_FLOOR;
            }
        }
    }
}

static void add_isolated_dungeon_room(Map *m, int x, int y) {
    for (int cy = y; cy < y + 5; cy++) {
        for (int cx = x; cx < x + 5; cx++) {
            m->tiles[cy][cx] = TILE_FLOOR;
        }
    }
}

void test_dungeon_all_room_connectivity(void) {
    printf("All dungeon room connectivity tests:\n");
    int all_tiles = 1;
    int keys_before_doors = 1;
    int caches_locked = 1;
    int unchanged = 1;
    for (int seed = 1; seed <= 1024; seed++) {
        srand(seed);
        for (int level = 1; level <= DUNGEON_DEPTH; level++) {
            Map m;
            map_generate(&m, level);
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    if (m.tiles[y][x] == TILE_DUNGEON_KEY || m.tiles[y][x] == TILE_CRYPT_KEY) {
                        keys_before_doors &= dungeon_tile_reachable(&m, x, y);
                    } else if (m.tiles[y][x] == TILE_CRYPT_CACHE) {
                        caches_locked &= !dungeon_tile_reachable(&m, x, y);
                    }
                }
            }
            Map opened = m;
            open_test_dungeon_doors(&opened);
            all_tiles &= dungeon_tile_reachable(&opened, -1, -1);
            unsigned char tiles_before[sizeof(m.tiles)];
            memcpy(tiles_before, m.tiles, sizeof(m.tiles));
            unchanged &= map_ensure_dungeon_connectivity(&m, level) && memcmp(tiles_before, m.tiles, sizeof(m.tiles)) == 0;
        }
    }
    ASSERT("all rooms, crypts, and corridor tiles are reachable across 5120 generated floors", all_tiles);
    ASSERT("every dungeon and crypt key is reachable before unlocking its door", keys_before_doors);
    ASSERT("connectivity repair does not bypass locked crypts", caches_locked);
    ASSERT("valid dungeon layouts are unchanged by another connectivity check", unchanged);

    static GameState g;
    static GameState loaded;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    g.elowen_quest_state = 1;
    game_enter_dungeon(&g);
    int quest_routes = 1;
    for (int level = 1; level <= DUNGEON_DEPTH; level++) {
        Map opened = g.map;
        open_test_dungeon_doors(&opened);
        quest_routes &= dungeon_tile_reachable(&opened, -1, -1);
        if (level < DUNGEON_DEPTH) {
            game_descend(&g);
        }
    }
    ASSERT("placing Elowen's quest encounters preserves full room connectivity", quest_routes);

    // A disconnected room need not have a room-list entry to be detected.
    g.location = LOCATION_DUNGEON;
    g.level = 3;
    srand(7107);
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    add_isolated_dungeon_room(&g.map, 170, 80);
    add_isolated_dungeon_room(&g.map, 185, 90);
    g.player.x = 172;
    g.player.y = 82;
    g.floor_item_count = 1;
    g.floor_items[0] = (FloorItem){.active = 1, .x = 172, .y = 82, .underlying_tile = TILE_FLOOR, .item = item_make_health_potion()};
    g.map.tiles[82][172] = TILE_ITEM;
    map_mark_explored(&g.map, 172, 82);
    g.enemies[0].hp = 7;
    int gold = g.gold;
    int score = g.score;
    for (int level = 1; level <= DUNGEON_DEPTH; level++) {
        LevelCache *cache = &g.level_cache[level - 1];
        cache->valid = 1;
        map_generate(&cache->map, level);
        add_isolated_dungeon_room(&cache->map, 170, 80);
    }
    ASSERT("a legacy floor has unreachable ordinary rooms before repair", !dungeon_tile_reachable(&g.map, 172, 82) &&
        !dungeon_tile_reachable(&g.map, 187, 92));
    ASSERT("version 106 saves load with all dungeon floors repaired", save_old_dungeon(&g, 106) && load_game(&loaded, 99151));
    Map opened = loaded.map;
    open_test_dungeon_doors(&opened);
    ASSERT("active-floor repair connects every isolated room and passage", dungeon_tile_reachable(&opened, -1, -1));
    int cached_routes = 1;
    for (int level = 1; level <= DUNGEON_DEPTH; level++) {
        opened = loaded.level_cache[level - 1].map;
        open_test_dungeon_doors(&opened);
        cached_routes &= dungeon_tile_reachable(&opened, -1, -1);
    }
    ASSERT("migration repairs every cached dungeon floor, including early floors", cached_routes);
    ASSERT("repair preserves a player and dropped loot inside an isolated room", loaded.player.x == 172 && loaded.player.y == 82 &&
        loaded.floor_items[0].active && loaded.floor_items[0].x == 172 && loaded.floor_items[0].y == 82 &&
        loaded.map.tiles[82][172] == TILE_ITEM && loaded.floor_items[0].underlying_tile == TILE_FLOOR && map_is_explored(&loaded.map, 172, 82));
    ASSERT("connectivity repair preserves enemies, quest progress, gold, and score", loaded.enemies[0].hp == 7 &&
        loaded.elowen_quest_state == g.elowen_quest_state && loaded.elowen_seals_restored == g.elowen_seals_restored &&
        loaded.gold == gold && loaded.score == score);
    ASSERT("the repaired map survives saving without another migration", save_game(&loaded, 99151) && load_game(&g, 99151) &&
        dungeon_tile_reachable(&g.map, 172, 82));

    add_isolated_dungeon_room(&g.map, 180, 70);
    ASSERT("a later disconnected section is detectable", !dungeon_tile_reachable(&g.map, 182, 72));
    game_refresh_quest_encounters(&g);
    ASSERT("cached-floor refresh enforces connectivity before play resumes", dungeon_tile_reachable(&g.map, 182, 72));
    remove("saves/savegame_99151.json");
}

static int lich_guard_count(const Enemy *enemies, int count) {
    int guards = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(enemies[i].name, "Lich Guard") == 0) {
            guards++;
        }
    }
    return guards;
}

void test_dungeon_return_stairs_migration(void) {
    printf("Special dungeon return stairs save migration tests:\n");
    static GameState g;
    static GameState loaded;
    for (int open = 0; open <= 1; open++) {
        game_init(&g);
        g.location = LOCATION_DUNGEON;
        g.level = DUNGEON_DEPTH;
        map_generate(&g.map, g.level);
        enemies_spawn(&g);
        if (open) {
            g.defeated_bosses |= 1 << LOCATION_DUNGEON;
            g.enemies[0].active = 0;
            g.enemies[0].hp = 0;
        }
        int x = g.map.stairs_down_x;
        int y = g.map.stairs_down_y;
        TileType old_stair = open ? TILE_RETURN_EXIT : TILE_STAIRS_DOWN;
        TileType stair = open ? TILE_DUNGEON_STAIRS_RETURN : TILE_DUNGEON_STAIRS_SEALED;
        g.map.tiles[y][x] = old_stair;
        LevelCache *cache = &g.level_cache[DUNGEON_DEPTH - 1];
        cache->map = g.map;
        memcpy(cache->enemies, g.enemies, sizeof(g.enemies));
        cache->enemy_count = g.enemy_count;
        cache->valid = 1;
        g.level_cache[0].valid = 1;
        map_generate(&g.level_cache[0].map, 1);
        g.player.x = x;
        g.player.y = y;
        g.gold = 77;
        g.elowen_quest_state = 1;
        g.elowen_seals_restored = 7;
        g.floor_item_count = 1;
        g.floor_items[0] = (FloorItem){
            .active = 1, .x = x, .y = y, .underlying_tile = old_stair,
            .item = item_make_health_potion()
        };
        g.map.tiles[y][x] = TILE_ITEM;
        ASSERT("legacy return stairs fixture saves", save_old_dungeon(&g, 108));
        ASSERT("legacy return stairs migrate while retaining the boss lock state", load_game(&loaded, 99151) &&
            loaded.map.tiles[y][x] == stair && loaded.level_cache[DUNGEON_DEPTH - 1].map.tiles[y][x] == stair);
        ASSERT("stairs migration preserves loot, player position, enemies and quest progress",
            loaded.floor_items[0].active && loaded.floor_items[0].x == x && loaded.floor_items[0].y == y &&
            loaded.floor_items[0].underlying_tile == stair &&
            loaded.floor_items[0].item.type == ITEM_POTION_HEALTH &&
            loaded.player.x == x && loaded.player.y == y && loaded.gold == 77 &&
            loaded.enemies[0].hp == g.enemies[0].hp && loaded.enemy_count == g.enemy_count &&
            loaded.elowen_quest_state == 1 && loaded.elowen_seals_restored == 7);
        const Map *first = &loaded.level_cache[0].map;
        ASSERT("ordinary downward stairs remain unchanged", first->tiles[first->stairs_down_y][first->stairs_down_x] == TILE_STAIRS_DOWN);
        ASSERT("special stairs and loot survive a current-version save/load", save_game(&loaded, 99151) &&
            load_game(&g, 99151) && g.map.tiles[y][x] == stair && g.floor_items[0].underlying_tile == stair);
        int message_count = g.message_count;
        game_update_level_progress(&g);
        game_update_level_progress(&g);
        ASSERT("loot on open stairs does not cause repeated unlock announcements", g.message_count == message_count);
        action_resolve_player(&g, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("stairs-down never activates migrated return stairs", g.location == LOCATION_DUNGEON && g.level == DUNGEON_DEPTH);
        action_resolve_player(&g, (Action){ACTION_ASCEND, 0, 0});
        ASSERT("stairs-up returns to Oakhaven only after the Lich is defeated, even with loot on the stairs",
            open ? g.location == LOCATION_TOWN && g.player.x == TOWN_W - 2 && g.player.y == 12
            : g.location == LOCATION_DUNGEON && g.level == DUNGEON_DEPTH);
    }

    // A Return to Town portal may have been cast while standing on the old exit.
    game_init(&g);
    game_enter_dungeon(&g);
    while (g.level < DUNGEON_DEPTH) {
        game_descend(&g);
    }
    g.defeated_bosses |= 1 << LOCATION_DUNGEON;
    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    int x = g.player.x;
    int y = g.player.y;
    g.map.tiles[y][x] = TILE_RETURN_EXIT;
    game_open_town_portal(&g);
    g.level_cache[DUNGEON_DEPTH - 1].map.tiles[y][x] = TILE_RETURN_EXIT;
    g.portal_origin_tile = TILE_RETURN_EXIT;
    ASSERT("legacy return portal fixture saves", save_old_dungeon(&g, 108));
    ASSERT("portal destination retains the new return stairs", load_game(&loaded, 99151) &&
        loaded.portal_active && loaded.portal_origin_tile == TILE_DUNGEON_STAIRS_RETURN);
    game_use_town_portal(&loaded);
    ASSERT("returning through the portal restores the special upward stairs at the exact tile",
        loaded.location == LOCATION_DUNGEON && loaded.player.x == x && loaded.player.y == y &&
        loaded.map.tiles[y][x] == TILE_DUNGEON_STAIRS_RETURN);
    action_resolve_player(&loaded, (Action){ACTION_ASCEND, 0, 0});
    ASSERT("stairs-up works after restoring a migrated portal destination", loaded.location == LOCATION_TOWN);
    remove("saves/savegame_99151.json");
}

void test_lich_minions(void) {
    printf("Lich King minion and town guidance migration tests:\n");
    static GameState g;
    static GameState loaded;
    game_init(&g);
    g.location = LOCATION_DUNGEON;
    g.level = DUNGEON_DEPTH;
    int valid = 1;
    for (int seed = 0; seed < 256; seed++) {
        srand((unsigned)seed);
        map_generate(&g.map, g.level);
        g.player.x = g.map.stairs_up_x;
        g.player.y = g.map.stairs_up_y;
        enemies_spawn(&g);
        Room *room = &g.map.rooms[g.map.room_count - 1];
        if (lich_guard_count(g.enemies, g.enemy_count) != 2 ||
            g.enemy_count > AREA_ENEMY_LIMIT) {
            valid = 0;
        }
        for (int i = 0; i < g.enemy_count; i++) {
            Enemy *e = &g.enemies[i];
            if (strcmp(e->name, "Lich Guard") == 0 &&
                (!e->active || e->is_boss || e->type != ENEMY_SKELETON ||
                e->x <= room->x || e->x >= room->x + room->w - 1 ||
                e->y <= room->y || e->y >= room->y + room->h - 1 ||
                g.map.tiles[e->y][e->x] != TILE_FLOOR ||
                dungeon_tile_reachable(&g.map, e->x, e->y))) {
                valid = 0;
            }
            for (int j = 0; j < i; j++) {
                if (g.enemies[j].x == e->x && g.enemies[j].y == e->y) {
                    valid = 0;
                }
            }
        }
    }
    ASSERT("256 boss layouts have exactly two Skeleton guards inside the lock, without overlaps or blocked stairs", valid);
    int count = g.enemy_count;
    game_add_lich_minions(&g);
    ASSERT("adding minions again does not duplicate them", g.enemy_count == count);
    g.enemies[1].active = 0;
    g.enemies[1].hp = 0;
    game_add_lich_minions(&g);
    ASSERT("fallen minions are not resurrected by encounter setup", g.enemy_count == count && !g.enemies[1].active);

    // Remove the new guards to model a final floor saved before version 108.
    int legacy_count = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        if (strcmp(g.enemies[i].name, "Lich Guard") != 0) {
            g.enemies[legacy_count++] = g.enemies[i];
        }
    }
    g.enemy_count = legacy_count;
    g.enemies[0].hp = 73;
    g.gold = 121;
    g.elowen_quest_state = 1;
    g.elowen_seals_restored = 7;
    LevelCache *cache = &g.level_cache[DUNGEON_DEPTH - 1];
    cache->map = g.map;
    memcpy(cache->enemies, g.enemies, sizeof(g.enemies));
    cache->enemy_count = g.enemy_count;
    cache->valid = 1;
    ASSERT("legacy boss floor fixture saves", save_old_dungeon(&g, 107));
    ASSERT("legacy boss floor gains two minions on load", load_game(&loaded, 99151) &&
        lich_guard_count(loaded.enemies, loaded.enemy_count) == 2);
    ASSERT("cached boss floor also gains two minions", lich_guard_count(
        loaded.level_cache[DUNGEON_DEPTH - 1].enemies,
        loaded.level_cache[DUNGEON_DEPTH - 1].enemy_count) == 2);
    ASSERT("minion migration preserves player, boss health, money and completed seals",
        loaded.player.x == g.player.x && loaded.player.y == g.player.y &&
        loaded.enemies[0].hp == 73 && loaded.gold == 121 &&
        loaded.elowen_quest_state == 1 && loaded.elowen_seals_restored == 7);
    loaded.enemies[legacy_count].hp = 0;
    loaded.enemies[legacy_count].active = 0;
    ASSERT("saving and loading does not respawn a defeated guard", save_game(&loaded, 99151) &&
        load_game(&g, 99151) && lich_guard_count(g.enemies, g.enemy_count) == 2 &&
        !g.enemies[legacy_count].active && g.enemies[legacy_count].hp == 0);

    game_init(&g);
    g.map.tiles[TOWN_BRAM_Y][TOWN_BRAM_X] = TILE_ITEM;
    g.player.x = TOWN_BRAM_X;
    g.player.y = TOWN_BRAM_Y;
    g.floor_item_count = 1;
    g.floor_items[0].active = 1;
    g.floor_items[0].x = TOWN_BRAM_X;
    g.floor_items[0].y = TOWN_BRAM_Y;
    g.floor_items[0].underlying_tile = TILE_TOWN_PATH;
    g.floor_items[0].item = item_make_health_potion();
    g.level_cache[DUNGEON_DEPTH - 1] = loaded.level_cache[DUNGEON_DEPTH - 1];
    // Model an old cache with no guards while the player is in town.
    g.level_cache[DUNGEON_DEPTH - 1].enemy_count = legacy_count;
    ASSERT("legacy town fixture saves", save_old_dungeon(&g, 107));
    ASSERT("old town save gains Bram", load_game(&loaded, 99151) &&
        loaded.map.tiles[TOWN_BRAM_Y][TOWN_BRAM_X] == TILE_NPC_BRAM);
    ASSERT("town migration keeps the player and dropped potion on the adjoining cobblestone",
        loaded.player.x == TOWN_BRAM_X && loaded.player.y == TOWN_BRAM_Y + 1 &&
        loaded.floor_items[0].active && loaded.floor_items[0].item.type == ITEM_POTION_HEALTH &&
        loaded.floor_items[0].x == TOWN_BRAM_X && loaded.floor_items[0].y == TOWN_BRAM_Y + 1 &&
        loaded.floor_items[0].underlying_tile == TILE_TOWN_PATH &&
        loaded.map.tiles[TOWN_BRAM_Y + 1][TOWN_BRAM_X] == TILE_ITEM);
    ASSERT("town save migration adds cached guards without changing the active town or its enemies",
        loaded.location == LOCATION_TOWN && loaded.level == g.level &&
        loaded.enemy_count == g.enemy_count && lich_guard_count(
        loaded.level_cache[DUNGEON_DEPTH - 1].enemies,
        loaded.level_cache[DUNGEON_DEPTH - 1].enemy_count) == 2);

    game_init(&g);
    g.location = LOCATION_DUNGEON;
    g.level = 1;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    ASSERT("early floors do not have Lich guards", lich_guard_count(g.enemies, g.enemy_count) == 0);
    g.level = DUNGEON_DEPTH;
    g.defeated_bosses |= 1 << LOCATION_DUNGEON;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    ASSERT("a defeated Lich King does not generate guards on a new expedition", lich_guard_count(g.enemies, g.enemy_count) == 0);
    g.level_cache[DUNGEON_DEPTH - 1].map = g.map;
    memcpy(g.level_cache[DUNGEON_DEPTH - 1].enemies, g.enemies, sizeof(g.enemies));
    g.level_cache[DUNGEON_DEPTH - 1].enemy_count = g.enemy_count;
    g.level_cache[DUNGEON_DEPTH - 1].valid = 1;
    ASSERT("completed dungeon fixture saves", save_old_dungeon(&g, 107));
    ASSERT("old completed dungeon stays completed without guards", load_game(&loaded, 99151) &&
        (loaded.defeated_bosses & (1 << LOCATION_DUNGEON)) &&
        lich_guard_count(loaded.enemies, loaded.enemy_count) == 0 &&
        lich_guard_count(loaded.level_cache[DUNGEON_DEPTH - 1].enemies,
        loaded.level_cache[DUNGEON_DEPTH - 1].enemy_count) == 0);
    remove("saves/savegame_99151.json");
}
