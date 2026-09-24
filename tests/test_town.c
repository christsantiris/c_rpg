#include "test_utils.h"
#include "../src/game/map.h"
#include "../src/game/game.h"
#include "../src/screens/shop.h"
#include "../src/screens/harbor.h"
#include "../src/systems/save_load.h"

void test_town_healer(void) {
    printf("Town healer tests:\n");
    static GameState g;
    static GameState loaded;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    int dx = TOWN_HEALER_DOOR_X;
    int dy = TOWN_HEALER_DOOR_Y;
    ASSERT("healer stands west of the blacksmith along the east-west road",
        TOWN_HEALER_X + TOWN_HEALER_W < TOWN_BLACKSMITH_X &&
        TOWN_HEALER_Y == TOWN_BLACKSMITH_Y &&
        g.map.tiles[dy][dx] == TILE_HEALER_DOOR);
    ASSERT("healer building is solid with a walkable entrance",
        !map_is_walkable(&g.map, TOWN_HEALER_X, TOWN_HEALER_Y) &&
        map_is_walkable(&g.map, dx, dy));
    int plaza = 1;
    for (int x = dx; x <= 30; x++) {
        TileType upper = g.map.tiles[dy + 1][x];
        plaza &= upper == TILE_TOWN_PATH ||
            (x == TOWN_CAIN_X && upper == TILE_NPC_CAIN);
        plaza &= g.map.tiles[dy + 2][x] == TILE_TOWN_PATH;
    }
    ASSERT("two-tile cobblestone plaza reaches every north shop door", plaza &&
        g.map.tiles[TOWN_BLACKSMITH_Y + 4][TOWN_BLACKSMITH_X + 2] == TILE_TOWN_PATH &&
        g.map.tiles[11][30] == TILE_TOWN_PATH);
    ASSERT("former healer lot and lane return to grass",
        g.map.tiles[16][26] == TILE_TOWN_FLOOR && g.map.tiles[20][28] == TILE_TOWN_FLOOR);
    ASSERT("healer lane leaves the harbor road locked",
        !game_harbor_unlocked(&g) &&
        g.map.tiles[TOWN_HARBOR_Y + 1][21] == TILE_TOWN_FLOOR &&
        g.map.tiles[TOWN_ROWAN_Y][TOWN_ROWAN_X] == TILE_NPC_ROWAN);

    ShopScreen shop;
    shop_init(&shop, SHOP_TYPE_HEALER, 0);
    int starts_on_heal = shop.selected == 0 &&
        shop_handle_key(&shop, SDL_SCANCODE_RETURN) == SHOP_HEAL;
    shop_handle_key(&shop, SDL_SCANCODE_DOWN);
    int selects_exit = shop.selected == 1 &&
        shop_handle_key(&shop, SDL_SCANCODE_KP_ENTER) == SHOP_CLOSED;
    shop_handle_key(&shop, SDL_SCANCODE_W);
    ASSERT("healer has selectable treatment and exit options",
        shop.item_count == 0 && starts_on_heal && selects_exit &&
        shop.selected == 0 && shop_handle_key(&shop, SDL_SCANCODE_TAB) == SHOP_NONE &&
        shop.mode == 0 && shop_handle_key(&shop, SDL_SCANCODE_ESCAPE) == SHOP_CLOSED);
    g.player.hp = g.player.max_hp - 30;
    g.gold = 9;
    ASSERT("healing thirty HP costs ten gold", game_healer_price(&g) == 10);
    game_visit_healer(&g);
    ASSERT("insufficient funds leave both gold and HP unchanged",
        g.gold == 9 && g.player.hp == g.player.max_hp - 30);
    g.gold = 10;
    int mp = g.player.mp;
    int items = g.inventory_count;
    game_visit_healer(&g);
    ASSERT("exact payment restores full HP without changing MP or inventory",
        g.gold == 0 && g.player.hp == g.player.max_hp &&
        g.player.mp == mp && g.inventory_count == items);
    g.gold = 100;
    game_visit_healer(&g);
    ASSERT("full health and repeat purchases are free",
        game_healer_price(&g) == 0 && g.gold == 100 && g.player.hp == g.player.max_hp);
    g.player.hp -= 31;
    ASSERT("larger wounds cost more with rounded-up pricing", game_healer_price(&g) == 11);
    game_visit_healer(&g);
    g.player.hp--;
    ASSERT("even a one-HP wound costs one gold", game_healer_price(&g) == 1);
    game_visit_healer(&g);
    ASSERT("separate treatments charge their current cost", g.gold == 88);

    const int slot = 99012;
    if (save_exists(slot)) {
        ASSERT("healer test save slot must be unused", 0);
        return;
    }
    int saved = save_game(&g, slot);
    int restored = saved && load_game(&loaded, slot);
    ASSERT("healing payment, restored HP and healer building survive save/load",
        restored && loaded.gold == 88 && loaded.player.hp == loaded.player.max_hp &&
        loaded.map.tiles[dy][dx] == TILE_HEALER_DOOR);
    remove("saves/savegame_99012.json");
}

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

static void reveal_forest_exit(GameState *g) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_FOREST_LANDMARK) {
                g->player.x = x;
                g->player.y = y;
                Action discover = {ACTION_MOVE, x, y};
                action_resolve_player(g, discover);
                return;
            }
        }
    }
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
    ASSERT("TILE_WATCHTOWER is distinct", TILE_WATCHTOWER != TILE_TAVERN);

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
    ASSERT("blacksmith moves east to make room for the healer",
        TOWN_BLACKSMITH_X > 7 &&
        m.tiles[TOWN_BLACKSMITH_Y][TOWN_BLACKSMITH_X] == TILE_SHOP_BLACKSMITH);
    ASSERT("alchemist at (28,7)",
        m.tiles[7][28] == TILE_SHOP_ALCHEMIST);
    ASSERT("blacksmith has a walk-in doorway",
        m.tiles[TOWN_BLACKSMITH_Y + 3][TOWN_BLACKSMITH_X + 2] == TILE_BLACKSMITH_DOOR &&
        map_is_walkable(&m, TOWN_BLACKSMITH_X + 2, TOWN_BLACKSMITH_Y + 3));
    ASSERT("alchemist has a walk-in doorway",
        m.tiles[10][30] == TILE_ALCHEMIST_DOOR &&
        map_is_walkable(&m, 30, 10));
    ASSERT("cobblestone plaza reaches all three shop doors",
        m.tiles[TOWN_HEALER_DOOR_Y + 1][TOWN_HEALER_DOOR_X] == TILE_TOWN_PATH &&
        m.tiles[TOWN_HEALER_DOOR_Y + 1][9] == TILE_TOWN_PATH &&
        m.tiles[TOWN_BLACKSMITH_Y + 4][TOWN_BLACKSMITH_X + 2] == TILE_TOWN_PATH &&
        m.tiles[11][24] == TILE_TOWN_PATH &&
        m.tiles[11][30] == TILE_TOWN_PATH);
    ASSERT("shop facades remain solid away from their doors",
        !map_is_walkable(&m, TOWN_BLACKSMITH_X, TOWN_BLACKSMITH_Y) &&
        !map_is_walkable(&m, 28, 7));
    ASSERT("tavern occupies southwest town lot",
        m.tiles[16][5] == TILE_TAVERN &&
        m.tiles[20][8] == TILE_TAVERN_DOOR &&
        m.tiles[20][11] == TILE_TAVERN);
    ASSERT("tavern facade remains solid away from its door",
        !map_is_walkable(&m, 5, 16));
    ASSERT("tavern doorway is walkable",
        map_is_walkable(&m, 8, 20));
    ASSERT("cobblestone lane reaches the south-facing tavern door",
        m.tiles[13][12] == TILE_TOWN_PATH &&
        m.tiles[21][12] == TILE_TOWN_PATH &&
        m.tiles[21][8] == TILE_TOWN_PATH);
    ASSERT("harbor reaches the southeast corner of the town green",
        m.tiles[20][34] == TILE_WATCHTOWER &&
        m.tiles[TOWN_H - 2][TOWN_W - 2] == TILE_WATCHTOWER);
    ASSERT("harbor remains closed and solid",
        !map_is_walkable(&m, 34, 20) &&
        !map_is_walkable(&m, TOWN_HARBOR_ENTRANCE_X,
            TOWN_HARBOR_ENTRANCE_Y));
    ASSERT("former watchtower lot is walkable green",
        m.tiles[16][28] == TILE_TOWN_FLOOR && map_is_walkable(&m, 32, 21));

    // Path tiles exist
    ASSERT("vertical path at center",
        m.tiles[10][20] == TILE_TOWN_PATH);
    ASSERT("horizontal path at mid row",
        m.tiles[12][10] == TILE_TOWN_PATH);

    ASSERT("coast exit at south crossroad",
        m.tiles[TOWN_H - 1][20] == TILE_TOWN_EXIT);
}

static void test_necromancer_retaliates_to_arrows(void) {
    static GameState g;
    Item bows[] = {item_make_bow(), item_make_longbow(), item_make_magic_longbow()};
    for (int i = 0; i < 3; i++) {
        g.player.player_class = CLASS_ROGUE;
        game_init(&g);
        g.location = LOCATION_FOREST;
        g.level = FOREST_DEPTH;
        map_generate_forest(&g.map, g.level);
        enemies_spawn(&g);
        ASSERT("Necromancer is available for ranged encounter",
            g.enemy_count > 0 && g.enemies[0].type == ENEMY_FOREST_NECROMANCER);
        if (g.enemy_count == 0 || g.enemies[0].type != ENEMY_FOREST_NECROMANCER) {
            return;
        }
        g.enemy_count = 1;
        Room *grove = &g.map.rooms[g.map.room_count - 1];
        Enemy *boss = &g.enemies[0];
        boss->x = grove->x + 4;
        boss->y = grove->y + grove->h / 2;
        g.player.x = boss->x - bows[i].range;
        g.player.y = boss->y;
        g.player.last_dx = 1;
        g.player.last_dy = 0;
        g.player.hp = 300;
        g.player.max_hp = 300;
        g.inventory_count = 1;
        g.inventory[0] = bows[i];
        g.equipped_main_hand = 0;
        for (int x = g.player.x; x <= boss->x; x++) {
            g.map.tiles[g.player.y][x] = TILE_FOREST_FLOOR;
        }
        action_resolve_enemies(&g);
        ASSERT("unprovoked Necromancer remains dormant outside the grove",
            boss->move_timer == 0 && g.player.hp == 300);
        action_resolve_player(&g, (Action){ACTION_RANGED_ATTACK, 0, 0});
        ASSERT("each bow can hit the Necromancer from outside the grove",
            boss->hp < boss->max_hp);
        action_resolve_enemies(&g);
        ASSERT("a ranged hit triggers the Necromancer's warning turn",
            boss->move_timer == 1 && g.player.hp == 300 &&
            strstr(g.messages[g.message_count - 1], "invokes the forest"));
        action_resolve_enemies(&g);
        ASSERT("Necromancer retaliates at every bow's maximum range",
            boss->move_timer == 2 && g.player.hp < 300);
        int hp = g.player.hp;
        g.player.x = boss->x - 13;
        action_resolve_enemies(&g);
        ASSERT("Necromancer does not attack a distant retreating player",
            boss->move_timer == 2 && g.player.hp == hp);
        g.player.x = boss->x - bows[i].range;
        action_resolve_enemies(&g);
        action_resolve_enemies(&g);
        ASSERT("Necromancer resumes fighting when the player returns",
            boss->move_timer == 4 && g.player.hp < hp);
    }
}

static void test_necromancer_opens_exit(void) {
    Action attacks[] = {
        {ACTION_MOVE, 12, 10},
        {ACTION_RANGED_ATTACK, 0, 0},
        {ACTION_CAST_SPELL, 0, 0}
    };
    for (int i = 0; i < 3; i++) {
        GameState g;
        g.player.player_class = CLASS_WARRIOR;
        game_init(&g);
        g.location = LOCATION_FOREST;
        g.level = FOREST_DEPTH;
        map_generate_forest(&g.map, g.level);
        g.enemy_count = 2;
        g.enemies[0] = (Enemy){
            .x = 12, .y = 10, .active = 1, .hp = 1, .max_hp = 1,
            .type = ENEMY_FOREST_NECROMANCER, .is_boss = 1
        };
        g.enemies[1] = (Enemy){
            .x = 20, .y = 20, .active = 1, .hp = 10,
            .type = ENEMY_DARK_ELF
        };
        g.player.x = i == 0 ? 11 : 10;
        g.player.y = 10;
        g.player.last_dx = 1;
        g.player.last_dy = 0;
        g.inventory_count = 1;
        g.inventory[0] = i == 0 ? item_make_dagger() : item_make_bow();
        g.equipped_main_hand = 0;
        g.player.known_spell_count = 1;
        g.player.known_spells[0] = spell_make_magic_arrow();
        g.player.equipped_spell = 0;
        g.player.mp = 100;
        for (int x = 10; x <= 12; x++) {
            g.map.tiles[10][x] = TILE_FOREST_FLOOR;
        }
        Action exit = outdoor_exit_action(&g.map);
        ASSERT("final forest exit starts hidden without finding the landmark",
            g.map.tiles[exit.target_y][exit.target_x] == TILE_FOREST_WALL);
        action_resolve_player(&g, attacks[i]);
        ASSERT("melee, arrows and spells record the Necromancer defeat",
            !g.enemies[0].active && (g.defeated_bosses & (1 << LOCATION_FOREST)));
        ASSERT("boss defeat reveals the exit without clearing regular enemies",
            g.map.tiles[exit.target_y][exit.target_x] == TILE_FOREST_EXIT &&
            g.enemies[1].active && !g.level_cleared);
        g.player.x = g.map.stairs_down_x;
        g.player.y = g.map.stairs_down_y;
        action_resolve_player(&g, exit);
        ASSERT("player leaves the final forest with a living enemy behind",
            g.location == LOCATION_TOWN &&
            g.forest_cache[FOREST_DEPTH - 1].enemies[1].active &&
            !g.forest_cache[FOREST_DEPTH - 1].level_cleared);

        g.location = LOCATION_FOREST;
        g.level = FOREST_DEPTH - 1;
        g.forest_cache[FOREST_DEPTH - 1].valid = 0;
        game_descend(&g);
        exit = outdoor_exit_action(&g.map);
        ASSERT("regenerated final forest keeps the defeated boss's exit open",
            g.map.tiles[exit.target_y][exit.target_x] == TILE_FOREST_EXIT &&
            g.enemy_count > 0 && !g.level_cleared);
    }
}

void test_forest(void) {
    printf("Forest adventure tests:\n");
    test_necromancer_retaliates_to_arrows();
    test_necromancer_opens_exit();
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
    ASSERT("forest exit begins hidden",
        g.map.tiles[g.map.stairs_down_y][MAP_W - 1] == TILE_FOREST_WALL);
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
    int hidden_trails = 0;
    int hidden_x = -1;
    int hidden_y = -1;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g.map.tiles[y][x] == TILE_FOREST_HIDDEN_TRAIL) {
                hidden_trails++;
                hidden_x = x;
                hidden_y = y;
            }
        }
    }
    ASSERT("forest contains a concealed shortcut", hidden_trails > 0);
    ASSERT("concealed shortcut initially behaves like dense woods",
        hidden_x >= 0 && !map_is_walkable(&g.map, hidden_x, hidden_y));
    reveal_forest_exit(&g);
    ASSERT("forest landmark reveals east stage exit",
        g.map.tiles[g.map.stairs_down_y][MAP_W - 1] == TILE_FOREST_EXIT);
    int concealed_after_landmark = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            concealed_after_landmark +=
                g.map.tiles[y][x] == TILE_FOREST_HIDDEN_TRAIL;
        }
    }
    ASSERT("forest landmark reveals the concealed shortcut",
        hidden_x >= 0 && hidden_y >= 0 && concealed_after_landmark == 0 &&
        g.map.tiles[hidden_y][hidden_x] == TILE_FOREST_FLOOR);

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

    static const int expected_rooms[FOREST_DEPTH] = {7, 9, 9, 8, 10, 10, 10, 10};
    static const int expected_entrances[FOREST_DEPTH] = {0, 2, 1, 2, 0, 1, 2, 0};
    static const int expected_exits[FOREST_DEPTH] = {0, 1, 2, 1, 0, 2, 1, 0};
    for (int level = 1; level <= FOREST_DEPTH; level++) {
        map_generate_forest(&g.map, level);
        ASSERT("forest level uses its distinct topology size",
            g.map.room_count == expected_rooms[level - 1]);
        ASSERT("forest levels vary their entrance edge",
            outdoor_entrance_is_on_side(&g.map, TILE_FOREST_ENTRANCE,
                expected_entrances[level - 1]));
        reveal_forest_exit(&g);
        ASSERT("forest levels vary their exit edge",
            outdoor_exit_is_on_side(&g.map, TILE_FOREST_EXIT,
                expected_exits[level - 1]));
    }
    map_generate_forest(&g.map, 2);
    g.location = LOCATION_FOREST;
    g.enemy_count = 0;
    int false_marker_x = -1;
    int false_marker_y = -1;
    int hidden_before_false_marker = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g.map.tiles[y][x] == TILE_FOREST_FALSE_MARKER) {
                false_marker_x = x;
                false_marker_y = y;
            } else if (g.map.tiles[y][x] == TILE_FOREST_HIDDEN_TRAIL) {
                hidden_before_false_marker++;
            }
        }
    }
    ASSERT("later forest stages contain a misleading trail marker",
        false_marker_x >= 0 && false_marker_y >= 0);
    if (false_marker_x < 0 || false_marker_y < 0) {
        return;
    }
    g.player.x = false_marker_x;
    g.player.y = false_marker_y;
    Action inspect_false_marker = {
        ACTION_MOVE, false_marker_x, false_marker_y
    };
    action_resolve_player(&g, inspect_false_marker);
    int hidden_after_false_marker = 0;
    int revealed_exit = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            hidden_after_false_marker +=
                g.map.tiles[y][x] == TILE_FOREST_HIDDEN_TRAIL;
            revealed_exit += g.map.tiles[y][x] == TILE_FOREST_EXIT;
        }
    }
    ASSERT("misleading marker becomes ordinary forest floor",
        g.map.tiles[false_marker_y][false_marker_x] == TILE_FOREST_FLOOR);
    ASSERT("misleading marker reveals neither shortcut nor exit",
        hidden_after_false_marker == hidden_before_false_marker &&
        revealed_exit == 0);

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
    ASSERT("forest portal appears beside the forest entrance",
        g.map.tiles[13][2] == TILE_PORTAL);
    Action use_portal = {ACTION_MOVE, 2, 13};
    action_resolve_player(&g, use_portal);
    ASSERT("town portal returns to forest", g.location == LOCATION_FOREST);
    ASSERT("forest portal restores exact tile",
        g.player.x == portal_x && g.player.y == portal_y);

    g.level = FOREST_DEPTH;
    g.level_cleared = 0;
    map_generate_forest(&g.map, g.level);
    enemies_spawn(&g);
    reveal_forest_exit(&g);
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

void test_cain_gift(void) {
    printf("Cain gift tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    ASSERT("Cain stands beside the crossroads within talking range of spawn",
        g.map.tiles[TOWN_CAIN_Y][TOWN_CAIN_X] == TILE_NPC_CAIN &&
        abs(g.player.x - TOWN_CAIN_X) <= 1 &&
        abs(g.player.y - TOWN_CAIN_Y) <= 1);
    ASSERT("Cain is solid and leaves both roads open",
        !map_is_walkable(&g.map, TOWN_CAIN_X, TOWN_CAIN_Y) &&
        g.map.tiles[12][19] == TILE_TOWN_PATH &&
        g.map.tiles[11][20] == TILE_TOWN_PATH);
    ASSERT("Cain's gift starts unclaimed", !g.cain_scroll_given);

    int count = g.inventory_count;
    game_talk_to_cain(&g);
    ASSERT("Cain warns about dangers outside town in his dialogue bubble",
        g.dialogue_active && strcmp(g.dialogue_speaker, "Cain") == 0 &&
        strstr(g.dialogue_text, "Undead") &&
        strstr(g.dialogue_text, "goblins") &&
        g.dialogue_x == TOWN_CAIN_X && g.dialogue_y == TOWN_CAIN_Y);
    ASSERT("Cain gives one Return to Town scroll",
        g.cain_scroll_given && g.inventory_count == count + 1 &&
        g.inventory[count].type == ITEM_SCROLL &&
        g.inventory[count].spell_id == SPELL_RETURN_TO_TOWN);
    game_talk_to_cain(&g);
    ASSERT("talking again does not duplicate the gift",
        g.inventory_count == count + 1);
    Action read = {ACTION_USE_ITEM, count, 0};
    action_resolve_player(&g, read);
    ASSERT("Cain's scroll teaches Return to Town",
        g.inventory_count == count && g.player.known_spell_count == 1 &&
        g.player.known_spells[0].id == SPELL_RETURN_TO_TOWN);
    game_enter_tavern(&g);
    game_leave_tavern(&g);
    game_talk_to_cain(&g);
    ASSERT("returning to town after reading the scroll does not renew the gift",
        g.inventory_count == count && g.cain_scroll_given &&
        g.map.tiles[TOWN_CAIN_Y][TOWN_CAIN_X] == TILE_NPC_CAIN);

    game_init(&g);
    while (g.inventory_count < MAX_INVENTORY) {
        g.inventory[g.inventory_count++] = item_make_health_potion();
    }
    game_talk_to_cain(&g);
    ASSERT("a full pack leaves the gift available for later",
        g.inventory_count == MAX_INVENTORY && !g.cain_scroll_given &&
        strstr(g.dialogue_text, "Make room"));
    game_remove_inventory_item(&g, MAX_INVENTORY - 1);
    game_talk_to_cain(&g);
    ASSERT("Cain gives the scroll after the player makes room",
        g.inventory_count == MAX_INVENTORY && g.cain_scroll_given &&
        g.inventory[MAX_INVENTORY - 1].spell_id == SPELL_RETURN_TO_TOWN);
}

void test_harbor_road(void) {
    printf("Harbor road tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    int road_y = TOWN_HARBOR_Y + 1;
    ASSERT("new games have no harbor road",
        !game_harbor_unlocked(&g) && g.map.tiles[road_y][21] == TILE_TOWN_FLOOR);
    int inventory_count = g.inventory_count;
    game_talk_to_rowan(&g);
    ASSERT("Rowan initially gives neither an item nor a quest",
        g.inventory_count == inventory_count && !game_has_treasure_map(&g) &&
        g.elowen_quest_state == 0 && g.dain_quest_state == 0 &&
        g.alder_quest_state == 0 && g.mara_quest_state == 0);

    int bosses = (1 << LOCATION_DUNGEON) | (1 << LOCATION_FOREST) |
        (1 << LOCATION_MOUNTAINS) | (1 << LOCATION_COAST);
    g.elowen_quest_state = 3;
    g.dain_quest_state = 3;
    g.alder_quest_state = 3;
    g.mara_quest_state = 3;
    const Location regions[] = {
        LOCATION_DUNGEON, LOCATION_FOREST, LOCATION_MOUNTAINS, LOCATION_COAST
    };
    for (int i = 0; i < 4; i++) {
        g.defeated_bosses = bosses & ~(1 << regions[i]);
        game_enter_tavern(&g);
        game_leave_tavern(&g);
        ASSERT("each regional boss is required to unlock the road",
            !game_harbor_unlocked(&g) && g.map.tiles[road_y][21] == TILE_TOWN_FLOOR);
        game_talk_to_rowan(&g);
        ASSERT("a missing boss prevents the treasure map gift", !game_has_treasure_map(&g));
    }
    g.defeated_bosses = bosses;
    int *quests[] = {
        &g.elowen_quest_state, &g.dain_quest_state,
        &g.alder_quest_state, &g.mara_quest_state
    };
    for (int i = 0; i < 4; i++) {
        for (int state = 0; state < 3; state++) {
            *quests[i] = state;
            game_enter_tavern(&g);
            game_leave_tavern(&g);
            ASSERT("every quest must be turned in before the road appears",
                !game_harbor_unlocked(&g) && g.map.tiles[road_y][21] == TILE_TOWN_FLOOR);
            game_talk_to_rowan(&g);
            ASSERT("an unfinished quest prevents the treasure map gift", !game_has_treasure_map(&g));
        }
        *quests[i] = 3;
    }

    g.mara_quest_state = 2;
    game_enter_tavern(&g);
    game_talk_to_mara(&g);
    game_leave_tavern(&g);
    for (int x = 20; x < TOWN_HARBOR_X; x++) {
        ASSERT("last quest turn-in connects the south road to the harbor",
            g.map.tiles[road_y][x] == TILE_TOWN_PATH &&
            map_is_walkable(&g.map, x, road_y));
    }
    ASSERT("harbor road bends through a landing onto the dock",
        g.map.tiles[TOWN_HARBOR_ENTRANCE_Y][TOWN_HARBOR_X - 1] == TILE_TOWN_PATH &&
        g.map.tiles[TOWN_HARBOR_ENTRANCE_Y][TOWN_HARBOR_X] == TILE_TOWN_PATH &&
        g.map.tiles[TOWN_HARBOR_ENTRANCE_Y][TOWN_HARBOR_ENTRANCE_X] == TILE_TOWN_PATH);
    g.player.x = TOWN_HARBOR_X - 1;
    g.player.y = road_y;
    game_move_player(&g, 0, 1);
    game_move_player(&g, 1, 0);
    game_move_player(&g, 1, 0);
    game_move_player(&g, 1, 0);
    ASSERT("player can walk from the road into the harbor dock",
        g.player.x == TOWN_HARBOR_ENTRANCE_X &&
        g.player.y == TOWN_HARBOR_ENTRANCE_Y);
    HarborScreen harbor;
    harbor_init(&harbor);
    ASSERT("dock scene blocks boarding without Rowan's treasure map",
        harbor.selected == 0 &&
        harbor_handle_key(&harbor, SDL_SCANCODE_RETURN, 0, 0) == HARBOR_MAP_REQUIRED);
    harbor_handle_key(&harbor, SDL_SCANCODE_DOWN, 0, 0);
    ASSERT("dock scene can return to town by keyboard",
        harbor.selected == 1 &&
        harbor_handle_key(&harbor, SDL_SCANCODE_KP_ENTER, 0, 0) == HARBOR_CLOSED &&
        harbor_handle_key(&harbor, SDL_SCANCODE_ESCAPE, 0, 0) == HARBOR_CLOSED);
    ASSERT("road leaves Rowan beside the route",
        g.map.tiles[TOWN_ROWAN_Y][TOWN_ROWAN_X] == TILE_NPC_ROWAN);
    while (g.inventory_count < MAX_INVENTORY) {
        g.inventory[g.inventory_count++] = item_make_health_potion();
    }
    game_talk_to_rowan(&g);
    ASSERT("a full pack leaves the map available for later",
        !game_has_treasure_map(&g) && g.inventory_count == MAX_INVENTORY &&
        strstr(g.dialogue_text, "Make room"));
    game_remove_inventory_item(&g, MAX_INVENTORY - 1);
    game_talk_to_rowan(&g);
    ASSERT("Rowan gives the treasure map when space is available",
        game_has_treasure_map(&g) && g.inventory_count == MAX_INVENTORY &&
        g.inventory[MAX_INVENTORY - 1].type == ITEM_TREASURE_MAP);
    harbor_init(&harbor);
    ASSERT("treasure map enables the dock's boarding option",
        harbor_handle_key(&harbor, SDL_SCANCODE_RETURN,
            game_has_treasure_map(&g), 0) == HARBOR_BOARD);
    Action read_map = {ACTION_USE_ITEM, MAX_INVENTORY - 1, 0};
    action_resolve_player(&g, read_map);
    ASSERT("reading the map describes the island without consuming it",
        game_has_treasure_map(&g) && g.inventory_count == MAX_INVENTORY &&
        strstr(g.messages[g.message_count - 1], "ruined temple"));
    Action drop_map = {ACTION_DROP_ITEM, MAX_INVENTORY - 1, 0};
    action_resolve_player(&g, drop_map);
    ASSERT("the map cannot be dropped and lost",
        game_has_treasure_map(&g) && g.inventory_count == MAX_INVENTORY &&
        g.floor_item_count == 0);
    game_remove_inventory_item(&g, MAX_INVENTORY - 2);
    game_talk_to_rowan(&g);
    ASSERT("talking again with room in the pack does not duplicate the gift",
        g.inventory_count == MAX_INVENTORY - 1 && game_has_treasure_map(&g));

    game_enter_coast(&g);
    game_open_town_portal(&g);
    ASSERT("returning from an expedition keeps the road and Coast portal",
        g.map.tiles[road_y][TOWN_HARBOR_X - 1] == TILE_TOWN_PATH &&
        g.map.tiles[TOWN_H - 3][21] == TILE_PORTAL);
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

static void test_goblin_king_retaliation(void) {
    static GameState g;
    Item bows[] = {item_make_bow(), item_make_longbow(), item_make_magic_longbow()};
    for (int scenario = 0; scenario < 5; scenario++) {
        g.player.player_class = CLASS_ROGUE;
        game_init(&g);
        g.location = LOCATION_MOUNTAINS;
        g.level = MOUNTAIN_DEPTH;
        map_generate_mountains(&g.map, g.level);
        enemies_spawn(&g);
        ASSERT("Goblin King is available for retaliation test",
            g.enemy_count > 0 && g.enemies[0].type == ENEMY_MOUNTAIN_GOBLIN_KING);
        if (g.enemy_count == 0 || g.enemies[0].type != ENEMY_MOUNTAIN_GOBLIN_KING) {
            return;
        }
        g.enemy_count = 1;
        Enemy *boss = &g.enemies[0];
        // A fixed arena keeps every shot outside its boundary and inside map bounds.
        g.map.rooms[g.map.room_count - 1] = (Room){30, 30, 12, 12};
        boss->x = 32;
        boss->y = 35;
        g.inventory_count = 1;
        g.inventory[0] = bows[scenario < 3 ? scenario : 2];
        g.equipped_main_hand = 0;
        g.player.known_spell_count = 1;
        g.player.known_spells[0] = spell_make_magic_arrow();
        g.player.equipped_spell = 0;
        g.player.mp = 100;
        g.player.hp = 300;
        g.player.max_hp = 300;
        int range = scenario == 4 ? g.player.known_spells[0].range : g.inventory[0].range;
        int diagonal = scenario == 3;
        g.player.x = boss->x - range;
        g.player.y = boss->y - (diagonal ? range : 0);
        g.player.last_dx = 1;
        g.player.last_dy = diagonal;
        int start_x = g.player.x;
        int start_y = g.player.y;
        for (int step = 0; step <= range; step++) {
            int path_x = start_x + step;
            int path_y = start_y + step * diagonal;
            g.map.tiles[path_y][path_x] = TILE_MOUNTAIN_FLOOR;
            if (diagonal && step > 0) {
                g.map.tiles[path_y - 1][path_x] = TILE_MOUNTAIN_FLOOR;
                g.map.tiles[path_y][path_x - 1] = TILE_MOUNTAIN_FLOOR;
            }
        }
        EnemyProjectiles shots = {0};
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("unprovoked Goblin King waits inside his fortress",
            boss->move_timer == 0 && shots.count == 0 && g.player.hp == 300);
        Action attack = {scenario == 4 ? ACTION_CAST_SPELL : ACTION_RANGED_ATTACK, 0, 0};
        action_resolve_player(&g, attack);
        ASSERT("bows and magic can provoke the Goblin King outside his fortress",
            boss->hp < boss->max_hp && boss->active);
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("a ranged hit triggers the Goblin King's axe warning",
            boss->move_timer == 1 && shots.count == 0 &&
            strstr(g.messages[g.message_count - 1], "raises his axe"));
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("Goblin King retaliates with damage and a visible axe projectile",
            boss->move_timer == 2 && g.player.hp < 300 && shots.count == 1 &&
            shots.shots[0].type == ENEMY_MOUNTAIN_GOBLIN_KING &&
            shots.shots[0].target_x == start_x && shots.shots[0].target_y == start_y);
        int hp = g.player.hp;
        g.player.x = boss->x - 13;
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("Goblin King stops attacking a distant retreating player",
            boss->move_timer == 2 && g.player.hp == hp && shots.count == 0);
        g.player.x = start_x;
        action_resolve_enemies_with_projectiles(&g, &shots);
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("Goblin King resumes retaliation when the player returns",
            boss->move_timer == 4 && g.player.hp < hp && shots.count == 1);
    }
}

void test_mountains(void) {
    test_goblin_king_retaliation();
    printf("Goblin Mountains tests:\n");
    static const int expected_rooms[MOUNTAIN_DEPTH] = {7, 8, 8, 9, 9, 10, 10, 10};
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
        ASSERT("mountain stage uses its authored template size",
            g.map.room_count == expected_rooms[level - 1]);
        ASSERT("mountain levels vary their entrance edge",
            outdoor_entrance_is_on_side(&g.map, TILE_MOUNTAIN_ENTRANCE,
                expected_entrances[level - 1]));
        ASSERT("mountain levels vary their exit edge",
            outdoor_exit_is_on_side(&g.map, TILE_MOUNTAIN_EXIT,
                expected_exits[level - 1]));
        int bridges = 0;
        int caves = 0;
        int fortress = 0;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                bridges += g.map.tiles[y][x] == TILE_MOUNTAIN_BRIDGE;
                caves += g.map.tiles[y][x] == TILE_MOUNTAIN_CAVE_FLOOR;
                fortress +=
                    g.map.tiles[y][x] == TILE_MOUNTAIN_FORTRESS_FLOOR;
            }
        }
        if (level == 2 || level == 7) {
            ASSERT("bridge template contains timber crossings", bridges > 0);
        } else if (level == 3 || level == 6) {
            ASSERT("cave template contains underground terrain", caves > 0);
        } else if (level == 4 || level == 5 || level == 8) {
            ASSERT("fortress template contains paved strongholds",
                fortress > 0);
        } else {
            ASSERT("pass template remains exposed basalt",
                bridges == 0 && caves == 0 && fortress == 0);
        }
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
