#include "test_utils.h"
#include "../src/game/map.h"
#include "../src/game/game.h"
#include "../src/game/actions.h"
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
    ASSERT("boss floor respects enemy capacity", g.enemy_count <= MAX_ENEMIES);
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

static int dungeon_tile_reachable(const Map *m, int target_x, int target_y, int gates_open) {
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
                seen[ny][nx] || (!map_is_walkable(m, nx, ny) &&
                !(gates_open && m->tiles[ny][nx] == TILE_DUNGEON_GATE))) {
                continue;
            }
            seen[ny][nx] = 1;
            queue[tail++] = ny * MAP_W + nx;
        }
    }
    return 0;
}

void test_dungeon_gate_reachability(void) {
    printf("Dungeon gate reachability tests:\n");
    int all_routes_open = 1;
    int all_switches_accessible = 1;
    for (int seed = 1; seed <= 512; seed++) {
        srand(seed);
        for (int level = 1; level < DUNGEON_DEPTH; level++) {
            Map m;
            map_generate(&m, level);
            if (!dungeon_tile_reachable(&m, m.stairs_down_x,
                m.stairs_down_y, 1)) {
                all_routes_open = 0;
            }
            int has_gate = 0;
            int has_reachable_switch = 0;
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    if (m.tiles[y][x] == TILE_DUNGEON_GATE) {
                        has_gate = 1;
                    } else if (m.tiles[y][x] == TILE_DUNGEON_SWITCH_OFF &&
                        dungeon_tile_reachable(&m, x, y, 0)) {
                        has_reachable_switch = 1;
                    }
                }
            }
            if (has_gate && !has_reachable_switch) {
                all_switches_accessible = 0;
            }
        }
    }
    ASSERT("generated dungeon exits remain reachable after opening gates",
        all_routes_open);
    ASSERT("every closed portcullis has an accessible switch",
        all_switches_accessible);
}

void test_return_to_town_spell(void) {
    printf("Return to Town spell tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    game_enter_dungeon(&g);
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
        g.map.tiles[2][21] == TILE_PORTAL);
    ASSERT("return portal remains active", g.portal_active == 1);
    ASSERT("unusable dungeon end of portal is hidden",
        g.level_cache[0].map.tiles[origin_y][origin_x] != TILE_PORTAL);

    Action enter = {ACTION_MOVE, 21, 2};
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
    for (int level = 1; level < 6; level++) {
        game_descend(&g);
    }
    game_open_town_portal(&g);
    ASSERT("coast portal remembers stage six",
        g.portal_active && g.portal_level == 6);
    game_enter_coast(&g);
    game_return_to_town(&g);
    ASSERT("coast portal survives another expedition",
        g.map.tiles[TOWN_H - 3][21] == TILE_PORTAL);
    game_use_town_portal(&g);
    ASSERT("coast portal returns to stage six",
        g.location == LOCATION_COAST && g.level == 6);

    map_room_center(&g.map.rooms[0], &g.player.x, &g.player.y);
    int blocked_x = g.player.x;
    int blocked_y = g.player.y;
    game_open_town_portal(&g);
    g.coast_cache[5].map.tiles[blocked_y][blocked_x] = TILE_COAST_WALL;
    game_use_town_portal(&g);
    ASSERT("portal avoids a blocked destination in the cached map",
        g.location == LOCATION_COAST && g.level == 6 &&
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
    Action a = {ACTION_DESCEND, 0, 0};
    action_resolve_player(&g, a);
    ASSERT("living Lich King blocks the final dungeon exit",
        g.location == LOCATION_DUNGEON);

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
    g.equipped_main_hand = 0;
    for (int x = 10; x <= 12; x++) {
        g.map.tiles[10][x] = TILE_FLOOR;
    }
    g.map.stairs_down_x = 12;
    g.map.stairs_down_y = 10;
    g.map.tiles[10][12] = TILE_STAIRS_DOWN;
    action_resolve_player(&g, (Action){ACTION_RANGED_ATTACK, 0, 0});
    ASSERT("defeating the Lich opens the return exit with regular enemies alive",
        !g.enemies[0].active && g.enemies[1].active && !g.level_cleared &&
        g.map.tiles[g.map.stairs_down_y][g.map.stairs_down_x] == TILE_RETURN_EXIT);

    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    action_resolve_player(&g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("picking up boss loot on the stairs preserves the unlocked exit",
        g.inventory_count == 2 && g.map.tiles[10][12] == TILE_RETURN_EXIT);
    action_resolve_player(&g, a);
    ASSERT("final exit returns player to town", g.location == LOCATION_TOWN);
    ASSERT("final exit returns at north town road", g.player.x == 20 && g.player.y == 1);
    ASSERT("final exit does not create a ninth floor",
        g.level == DUNGEON_DEPTH);
    ASSERT("leaving the dungeon preserves surviving enemies and uncleared status",
        g.level_cache[DUNGEON_DEPTH - 1].enemies[1].active &&
        !g.level_cache[DUNGEON_DEPTH - 1].level_cleared);
}

void test_enemy_movement_collision(void) {
    printf("Enemy movement collision tests:\n");
    GameState g;
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

    Location locations[4] = {
        LOCATION_DUNGEON,
        LOCATION_FOREST,
        LOCATION_MOUNTAINS,
        LOCATION_COAST
    };
    for (int region = 0; region < 4; region++) {
        g.location = locations[region];
        g.level = 8;
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
    Action loot_cache = {ACTION_INTERACT, 0, 0};
    action_resolve_player(&g, loot_cache);
    ASSERT("A loots the crypt cache once",
        g.gold > gold_before &&
        g.map.tiles[cache_y][cache_x] == TILE_FLOOR);

    g.level = 3;
    map_generate(&g.map, g.level);
    int switch_x = -1;
    int switch_y = -1;
    int gate_x = -1;
    int gate_y = -1;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g.map.tiles[y][x] == TILE_DUNGEON_SWITCH_OFF) {
                switch_x = x;
                switch_y = y;
            } else if (g.map.tiles[y][x] == TILE_DUNGEON_GATE) {
                gate_x = x;
                gate_y = y;
            }
        }
    }
    ASSERT("dungeon floor three contains a switch and portcullis",
        switch_x >= 0 && gate_x >= 0);
    if (switch_x < 0 || gate_x < 0) {
        return;
    }
    ASSERT("closed portcullis blocks movement",
        !map_is_walkable(&g.map, gate_x, gate_y));

    g.player.x = switch_x;
    g.player.y = switch_y;
    Action activate_switch = {ACTION_INTERACT, 0, 0};
    action_resolve_player(&g, activate_switch);
    ASSERT("A activates the dungeon switch",
        g.map.tiles[switch_y][switch_x] == TILE_DUNGEON_SWITCH_ON);
    ASSERT("activated switch opens the portcullis shortcut",
        g.map.tiles[gate_y][gate_x] == TILE_FLOOR &&
        map_is_walkable(&g.map, gate_x, gate_y));
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
