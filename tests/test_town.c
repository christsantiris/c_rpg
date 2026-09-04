#include "test_utils.h"
#include "../src/game/map.h"
#include "../src/game/game.h"

static int forest_path_exists_around(const Map *m, int blocked_room) {
    unsigned char visited[MAP_H][MAP_W] = {{0}};
    int qx[MAP_W * MAP_H];
    int qy[MAP_W * MAP_H];
    int head = 0, tail = 0;
    qx[tail] = m->stairs_up_x;
    qy[tail++] = m->stairs_up_y;
    visited[m->stairs_up_y][m->stairs_up_x] = 1;
    static const int dx[4] = {1, -1, 0, 0};
    static const int dy[4] = {0, 0, 1, -1};
    const Room *blocked = &m->rooms[blocked_room];
    while (head < tail) {
        int x = qx[head], y = qy[head++];
        if (x == m->stairs_down_x && y == m->stairs_down_y) return 1;
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i], ny = y + dy[i];
            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H ||
                visited[ny][nx] || !map_is_walkable(m, nx, ny)) continue;
            if (nx >= blocked->x && nx < blocked->x + blocked->w &&
                ny >= blocked->y && ny < blocked->y + blocked->h) continue;
            visited[ny][nx] = 1;
            qx[tail] = nx;
            qy[tail++] = ny;
        }
    }
    return 0;
}

static Action outdoor_exit_action(const Map *m) {
    Action action = {ACTION_MOVE, m->stairs_down_x, m->stairs_down_y};
    if (m->stairs_down_x == MAP_W - 2) {
        action.target_x++;
    } else if (m->stairs_down_y == 1) {
        action.target_y--;
    } else {
        action.target_y++;
    }
    return action;
}

static Action outdoor_entrance_action(const Map *m) {
    Action action = {ACTION_MOVE, m->stairs_up_x, m->stairs_up_y};
    if (m->stairs_up_x == 1) {
        action.target_x--;
    } else if (m->stairs_up_y == 1) {
        action.target_y--;
    } else {
        action.target_y++;
    }
    return action;
}

static int outdoor_exit_is_on_side(const Map *m, TileType exit_tile, int side) {
    if (side == 0) {
        return m->tiles[m->stairs_down_y][MAP_W - 1] == exit_tile;
    }
    if (side == 1) {
        return m->tiles[0][m->stairs_down_x] == exit_tile;
    }
    return m->tiles[MAP_H - 1][m->stairs_down_x] == exit_tile;
}

static int outdoor_entrance_is_on_side(const Map *m, TileType entrance_tile, int side) {
    if (side == 0) {
        return m->tiles[m->stairs_up_y][0] == entrance_tile;
    }
    if (side == 1) {
        return m->tiles[0][m->stairs_up_x] == entrance_tile;
    }
    return m->tiles[MAP_H - 1][m->stairs_up_x] == entrance_tile;
}

void test_town_tiles(void) {
    printf("Town tile tests:\n");

    // Tile types are distinct
    ASSERT("TILE_TOWN_FLOOR is distinct",    TILE_TOWN_FLOOR    != TILE_WALL);
    ASSERT("TILE_TOWN_PATH is distinct",     TILE_TOWN_PATH     != TILE_TOWN_FLOOR);
    ASSERT("TILE_TOWN_EXIT is distinct",     TILE_TOWN_EXIT     != TILE_TOWN_PATH);
    ASSERT("TILE_SHOP_BLACKSMITH distinct",  TILE_SHOP_BLACKSMITH != TILE_TOWN_EXIT);
    ASSERT("TILE_SHOP_ALCHEMIST distinct",   TILE_SHOP_ALCHEMIST  != TILE_SHOP_BLACKSMITH);
    ASSERT("TILE_TAVERN is distinct", TILE_TAVERN != TILE_SHOP_ALCHEMIST);

    // Constants are defined
    ASSERT("TOWN_W is 40", TOWN_W == 40);
    ASSERT("TOWN_H is 25", TOWN_H == 25);
}

void test_town_map(void) {
    printf("Town map tests:\n");

    Map m;
    int spawn_x, spawn_y;
    map_generate_town(&m, &spawn_x, &spawn_y);

    // Spawn is walkable
    ASSERT("spawn tile is walkable",
        map_is_walkable(&m, spawn_x, spawn_y));
    ASSERT("town spawn is at the central crossroads",
        spawn_x == 20 && spawn_y == 12);

    // Exit tiles at north edge
    ASSERT("exit tile at north center",
        m.tiles[0][20] == TILE_TOWN_EXIT);
    ASSERT("forest exit at west crossroad",
        m.tiles[12][0] == TILE_TOWN_EXIT);
    ASSERT("mountain exit at east crossroad",
        m.tiles[12][TOWN_W - 1] == TILE_TOWN_EXIT);

    // Shop tiles in correct positions
    ASSERT("blacksmith at (7,7)",
        m.tiles[7][7] == TILE_SHOP_BLACKSMITH);
    ASSERT("alchemist at (28,7)",
        m.tiles[7][28] == TILE_SHOP_ALCHEMIST);
    ASSERT("tavern occupies southwest town lot",
        m.tiles[16][5] == TILE_TAVERN && m.tiles[20][11] == TILE_TAVERN);
    ASSERT("tavern is not walkable before interiors are implemented",
        !map_is_walkable(&m, 5, 16));

    // Path tiles exist
    ASSERT("vertical path at center",
        m.tiles[10][20] == TILE_TOWN_PATH);
    ASSERT("horizontal path at mid row",
        m.tiles[12][10] == TILE_TOWN_PATH);

    // Border is wall
    ASSERT("south border is wall",
        m.tiles[TOWN_H-1][20] == TILE_WALL);
}

void test_forest(void) {
    printf("Forest adventure tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);

    g.player.x = 1;
    g.player.y = 12;
    Action west = {ACTION_MOVE, 0, 12};
    action_resolve_player(&g, west);
    ASSERT("west town exit enters forest", g.location == LOCATION_FOREST);
    ASSERT("forest begins on level one", g.level == 1);
    ASSERT("forest begins just inside west edge", g.player.x == 1);
    ASSERT("forest has west entrance",
        g.map.tiles[g.map.stairs_up_y][0] == TILE_FOREST_ENTRANCE);
    ASSERT("forest has east stage exit",
        g.map.tiles[g.map.stairs_down_y][MAP_W - 1] == TILE_FOREST_EXIT);
    ASSERT("forest generation includes branching clearings",
        g.map.room_count == 7);
    ASSERT("lower route reaches exit when upper route is blocked",
        forest_path_exists_around(&g.map, 3));
    ASSERT("upper route reaches exit when lower route is blocked",
        forest_path_exists_around(&g.map, 4));
    int forest_terrain = 0;
    for (int y = 0; y < MAP_H && !forest_terrain; y++)
        for (int x = 0; x < MAP_W; x++)
            if (g.map.tiles[y][x] == TILE_FOREST_FLOOR) {
                forest_terrain = 1;
                break;
            }
    ASSERT("forest contains forest floor tiles", forest_terrain);

    int pickup_x = g.player.x;
    int pickup_y = g.player.y;
    g.floor_item_count = 1;
    g.floor_items[0] = (FloorItem){
        .active = 1,
        .x = pickup_x,
        .y = pickup_y,
        .underlying_tile = TILE_FOREST_FLOOR,
        .item = item_make_health_potion()
    };
    g.map.tiles[pickup_y][pickup_x] = TILE_ITEM;
    Action pickup = {ACTION_PICK_UP, 0, 0};
    action_resolve_player(&g, pickup);
    ASSERT("forest pickup restores forest floor",
        g.map.tiles[pickup_y][pickup_x] == TILE_FOREST_FLOOR);

    g.player.x = MAP_W - 2;
    g.player.y = g.map.stairs_down_y;
    Action east = {ACTION_MOVE, MAP_W - 1, g.player.y};
    action_resolve_player(&g, east);
    ASSERT("forest exit advances without clearing enemies", g.level == 2);
    ASSERT("next forest stage starts at south edge",
        g.player.y == MAP_H - 2);
    Action back = outdoor_entrance_action(&g.map);
    action_resolve_player(&g, back);
    ASSERT("forest entrance returns to previous stage", g.level == 1);
    ASSERT("backtracking arrives inside east edge", g.player.x == MAP_W - 2);

    for (int level = 1; level <= FOREST_DEPTH; level++) {
        g.location = LOCATION_FOREST;
        g.level = level;
        map_generate_forest(&g.map, level);
        enemies_spawn(&g);
        int bosses = 0;
        int invalid_enemy = 0;
        for (int i = 0; i < g.enemy_count; i++) {
            EnemyType type = g.enemies[i].type;
            if (g.enemies[i].is_boss) bosses++;
            if (type < ENEMY_PIXIE || type > ENEMY_FOREST_NECROMANCER)
                invalid_enemy = 1;
        }
        ASSERT("forest floors use only forest roster", !invalid_enemy);
        if (level < FOREST_DEPTH)
            ASSERT("forest stages 1-7 have no boss", bosses == 0);
        else
            ASSERT("forest stage 8 has Necromancer boss",
                bosses == 1 && g.enemies[0].type == ENEMY_FOREST_NECROMANCER);
    }

    static const int expected_rooms[FOREST_DEPTH] = {7, 8, 9, 8, 9, 10, 10, 10};
    static const int expected_entrances[FOREST_DEPTH] = {0, 2, 1, 2, 0, 1, 2, 0};
    static const int expected_exits[FOREST_DEPTH] = {0, 1, 2, 1, 0, 2, 1, 0};
    for (int level = 1; level <= FOREST_DEPTH; level++) {
        map_generate_forest(&g.map, level);
        ASSERT("forest level uses its distinct topology size",
            g.map.room_count == expected_rooms[level - 1]);
        ASSERT("forest levels vary their entrance edge",
            outdoor_entrance_is_on_side(&g.map, TILE_FOREST_ENTRANCE,
                expected_entrances[level - 1]));
        ASSERT("forest levels vary their exit edge",
            outdoor_exit_is_on_side(&g.map, TILE_FOREST_EXIT,
                expected_exits[level - 1]));
    }
    map_generate_forest(&g.map, 2);
    int l2y3, l2y4, l2y5, unused;
    map_room_center(&g.map.rooms[3], &unused, &l2y3);
    map_room_center(&g.map.rooms[4], &unused, &l2y4);
    map_room_center(&g.map.rooms[5], &unused, &l2y5);
    ASSERT("forest level 2 has three vertically distinct routes",
        l2y3 < l2y4 && l2y4 < l2y5);
    map_generate_forest(&g.map, 4);
    int hub_y, upper_y, lower_y;
    map_room_center(&g.map.rooms[1], &unused, &hub_y);
    map_room_center(&g.map.rooms[2], &unused, &upper_y);
    map_room_center(&g.map.rooms[4], &unused, &lower_y);
    ASSERT("forest level 4 uses a central branching hub",
        upper_y < hub_y && hub_y < lower_y);

    g.location = LOCATION_FOREST;
    g.level = 1;
    map_generate_forest(&g.map, 1);
    enemies_spawn(&g);
    g.level_cleared = 1;
    game_descend(&g);
    ASSERT("forest progress uses forest cache", g.forest_cache[0].valid);
    ASSERT("forest progress does not overwrite dungeon cache",
        !g.level_cache[0].valid);

    int portal_x = g.player.x;
    int portal_y = g.player.y;
    game_open_town_portal(&g);
    ASSERT("return spell works from forest", g.location == LOCATION_TOWN);
    ASSERT("portal remembers forest location",
        g.portal_location == LOCATION_FOREST);
    game_use_town_portal(&g);
    ASSERT("town portal returns to forest", g.location == LOCATION_FOREST);
    ASSERT("forest portal restores exact tile",
        g.player.x == portal_x && g.player.y == portal_y);

    g.level = FOREST_DEPTH;
    map_generate_forest(&g.map, g.level);
    enemies_spawn(&g);
    g.player.x = MAP_W - 2;
    g.player.y = g.map.stairs_down_y;
    east = (Action){ACTION_MOVE, MAP_W - 1, g.player.y};
    action_resolve_player(&g, east);
    ASSERT("living Necromancer blocks final forest exit",
        g.location == LOCATION_FOREST);
    for (int i = 0; i < g.enemy_count; i++)
        if (g.enemies[i].type == ENEMY_FOREST_NECROMANCER)
            g.enemies[i].active = 0;
    action_resolve_player(&g, east);
    ASSERT("final east forest exit returns to town",
        g.location == LOCATION_TOWN);
    ASSERT("forest completion returns at west town road",
        g.player.x == 1 && g.player.y == 12);
}

void test_town_spawn(void) {
    printf("Town spawn tests:\n");

    GameState g;
    game_init(&g);

    ASSERT("new game starts in town",     g.location == LOCATION_TOWN);
    ASSERT("player spawn is walkable",
        map_is_walkable(&g.map, g.player.x, g.player.y));
    ASSERT("player not on exit tile",
        g.map.tiles[g.player.y][g.player.x] != TILE_TOWN_EXIT);
    ASSERT("player not on shop tile",
        g.map.tiles[g.player.y][g.player.x] != TILE_SHOP_BLACKSMITH &&
        g.map.tiles[g.player.y][g.player.x] != TILE_SHOP_ALCHEMIST);
}

void test_mountains(void) {
    printf("Goblin Mountains tests:\n");
    static const int expected_entrances[MOUNTAIN_DEPTH] = {0, 2, 1, 2, 0, 1, 2, 0};
    static const int expected_exits[MOUNTAIN_DEPTH] = {1, 0, 2, 1, 2, 0, 1, 2};
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    g.player.x = TOWN_W - 2;
    g.player.y = 12;
    Action enter = {ACTION_MOVE, TOWN_W - 1, 12};
    action_resolve_player(&g, enter);
    ASSERT("east town exit enters mountains", g.location == LOCATION_MOUNTAINS);
    ASSERT("mountains begin on level one", g.level == 1);
    ASSERT("mountains use red-black terrain",
        g.map.tiles[g.player.y][g.player.x] == TILE_MOUNTAIN_FLOOR);

    for (int level = 1; level <= MOUNTAIN_DEPTH; level++) {
        g.level = level;
        map_generate_mountains(&g.map, level);
        ASSERT("mountain levels vary their entrance edge",
            outdoor_entrance_is_on_side(&g.map, TILE_MOUNTAIN_ENTRANCE,
                expected_entrances[level - 1]));
        ASSERT("mountain levels vary their exit edge",
            outdoor_exit_is_on_side(&g.map, TILE_MOUNTAIN_EXIT,
                expected_exits[level - 1]));
        enemies_spawn(&g);
        int bosses = 0, invalid = 0;
        for (int i = 0; i < g.enemy_count; i++) {
            EnemyType type = g.enemies[i].type;
            if (g.enemies[i].is_boss) bosses++;
            if (type < ENEMY_GOBLIN_SCOUT ||
                type > ENEMY_MOUNTAIN_GOBLIN_KING) invalid = 1;
        }
        ASSERT("mountains use only mountain enemy roster", !invalid);
        if (level < MOUNTAIN_DEPTH)
            ASSERT("mountain stages 1-7 have no boss", bosses == 0);
        else
            ASSERT("mountain stage 8 has Goblin King",
                bosses == 1 &&
                g.enemies[0].type == ENEMY_MOUNTAIN_GOBLIN_KING);
    }

    g.level = 1;
    map_generate_mountains(&g.map, 1);
    enemies_spawn(&g);
    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    Action exit = outdoor_exit_action(&g.map);
    action_resolve_player(&g, exit);
    ASSERT("mountain stage advances without full clear", g.level == 2);
    ASSERT("mountain progress uses independent cache",
        g.mountain_cache[0].valid && !g.level_cache[0].valid &&
        !g.forest_cache[0].valid);

    g.level = MOUNTAIN_DEPTH;
    map_generate_mountains(&g.map, g.level);
    enemies_spawn(&g);
    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    exit = outdoor_exit_action(&g.map);
    action_resolve_player(&g, exit);
    ASSERT("Goblin King blocks final mountain exit",
        g.location == LOCATION_MOUNTAINS);
    for (int i = 0; i < g.enemy_count; i++)
        if (g.enemies[i].type == ENEMY_MOUNTAIN_GOBLIN_KING)
            g.enemies[i].active = 0;
    action_resolve_player(&g, exit);
    ASSERT("defeating Goblin King returns to town",
        g.location == LOCATION_TOWN);
    ASSERT("mountain completion returns at east town road",
        g.player.x == TOWN_W - 2 && g.player.y == 12);
}

void test_return_to_town(void) {
    printf("Return to town tests:\n");

    GameState g;
    game_init(&g);

    // Set up dungeon state
    g.location = LOCATION_DUNGEON;
    g.level = 3;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    g.level_cleared = 1;
    g.player.x = g.map.stairs_up_x;
    g.player.y = g.map.stairs_up_y;

    int enemies_before = g.enemy_count;
    ASSERT("enemies exist before return", enemies_before > 0);

    game_return_to_town(&g);

    ASSERT("location is town after return",
        g.location == LOCATION_TOWN);
    ASSERT("enemy count is zero after return",
        g.enemy_count == 0);
    ASSERT("player spawn is walkable",
        map_is_walkable(&g.map, g.player.x, g.player.y));
    ASSERT("player not on wall tile",
        g.map.tiles[g.player.y][g.player.x] != TILE_WALL);
    ASSERT("level 3 cached after return",
        g.level_cache[2].valid == 1);
    ASSERT("level 3 cleared state cached",
        g.level_cache[2].level_cleared == 1);
    ASSERT("floor items cleared",
        g.floor_item_count == 0);
}
