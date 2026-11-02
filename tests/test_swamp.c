#include "test_utils.h"
#include "approach_checks.h"
#include "../src/game/game.h"
#include "../src/game/actions.h"
#include "../src/systems/save_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GameState swamp_game;
static GameState swamp_loaded;
static Map swamp_layout;
static unsigned char visited[MAP_H][MAP_W];
static int queue[MAP_W * MAP_H];

static int swamp_exit_reachable(const Map *map) {
    memset(visited, 0, sizeof(visited));
    int head = 0;
    int tail = 0;
    int start = map->stairs_up_y * MAP_W + map->stairs_up_x;
    queue[tail++] = start;
    visited[map->stairs_up_y][map->stairs_up_x] = 1;
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
                !visited[ny][nx] && map_is_walkable(map, nx, ny)) {
                visited[ny][nx] = 1;
                queue[tail++] = ny * MAP_W + nx;
            }
        }
    }
    if (!visited[map->stairs_down_y][map->stairs_down_x]) {
        return 0;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = map->tiles[y][x];
            if ((tile == TILE_SWAMP_FLOOR || tile == TILE_SWAMP_ENTRANCE ||
                tile == TILE_SWAMP_EXIT) && !visited[y][x]) {
                return 0;
            }
        }
    }
    return 1;
}

static int swamp_exits_at_edges(const Map *map) {
    if (map->stairs_up_x != 0 ||
        map->stairs_down_x != SWAMP_MAP_W - 1 ||
        map->tiles[map->stairs_up_y][0] != TILE_SWAMP_ENTRANCE ||
        map->tiles[map->stairs_down_y][SWAMP_MAP_W - 1] != TILE_SWAMP_EXIT ||
        !map_is_walkable(map, 1, map->stairs_up_y) ||
        !map_is_walkable(map, SWAMP_MAP_W - 2, map->stairs_down_y)) {
        return 0;
    }
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        if (y != map->stairs_up_y && map_is_walkable(map, 0, y)) {
            return 0;
        }
        if (y != map->stairs_down_y &&
            map_is_walkable(map, SWAMP_MAP_W - 1, y)) {
            return 0;
        }
    }
    return 1;
}

void test_swamp(void) {
    printf("Swamp tests:\n");
    swamp_game.player.player_class = CLASS_WARRIOR;
    game_init(&swamp_game);
    srand(19);
    game_enter_town2(&swamp_game);
    ASSERT("Town 2 north gate enters the swamp and the shortcut starts closed",
        swamp_game.map.tiles[0][20] == TILE_TOWN_EXIT &&
        swamp_game.map.tiles[TOWN_H - 1][SOUTH_AREA_GATE_X] == TILE_TOWN_EXIT &&
        swamp_game.map.tiles[0][TOWN3_ROAD_X] == TILE_WALL);
    swamp_game.player.x = 20;
    swamp_game.player.y = 1;
    action_resolve_player(&swamp_game, (Action){ACTION_MOVE, 20, 0});
    walk_town_approach(&swamp_game);
    ASSERT("north gate enters swamp level one",
        swamp_game.location == LOCATION_SWAMP && swamp_game.level == 1);
    int rounded_clearings = 0;
    for (int i = 0; i < swamp_game.map.room_count; i++) {
        const Room *room = &swamp_game.map.rooms[i];
        int corners = 0;
        corners += !map_is_walkable(&swamp_game.map, room->x, room->y);
        corners += !map_is_walkable(&swamp_game.map, room->x + room->w - 1, room->y);
        corners += !map_is_walkable(&swamp_game.map, room->x, room->y + room->h - 1);
        corners += !map_is_walkable(&swamp_game.map, room->x + room->w - 1, room->y + room->h - 1);
        rounded_clearings += corners >= 2;
    }
    ASSERT("swamp clearings have irregular shorelines instead of square rooms",
        rounded_clearings >= 5);

    for (int level = 1; level <= SWAMP_BOSS_LEVEL; level++) {
        ASSERT("every swamp path tile connects to the entry and exit",
            swamp_game.level == level &&
            swamp_exit_reachable(&swamp_game.map));
        ASSERT("swamp entrance and exit are single openings at opposite edges",
            swamp_exits_at_edges(&swamp_game.map));
        ASSERT("swamp tiles use their own black-green terrain",
            swamp_game.map.tiles[0][0] == TILE_SWAMP_WALL &&
            swamp_game.map.tiles[swamp_game.map.stairs_up_y]
                [swamp_game.map.stairs_up_x] == TILE_SWAMP_ENTRANCE);
        if (level < SWAMP_BOSS_LEVEL) {
            int exit_x = swamp_game.map.stairs_down_x;
            int exit_y = swamp_game.map.stairs_down_y;
            swamp_game.player.x = exit_x - 1;
            swamp_game.player.y = exit_y;
            action_resolve_player(&swamp_game,
                (Action){ACTION_MOVE, exit_x, exit_y});
        }
    }
    int demon_count = 0;
    int regular_count = 0;
    for (int i = 0; i < swamp_game.enemy_count; i++) {
        EnemyType type = swamp_game.enemies[i].type;
        demon_count += type == ENEMY_SWAMP_DEMON;
        regular_count += type == ENEMY_GIANT_RAT || type == ENEMY_BANDIT ||
            type == ENEMY_VAMPIRE || type == ENEMY_ZOMBIE ||
            type == ENEMY_WRAITH;
    }
    ASSERT("final level five has one demon and the swamp enemy roster",
        demon_count == 1 && regular_count > 0 &&
        regular_count + demon_count == swamp_game.enemy_count);
    int final_exit_x = swamp_game.map.stairs_down_x;
    int final_exit_y = swamp_game.map.stairs_down_y;
    swamp_game.player.x = final_exit_x - 1;
    swamp_game.player.y = final_exit_y;
    action_resolve_player(&swamp_game,
        (Action){ACTION_MOVE, final_exit_x, final_exit_y});
    ASSERT("the central exit is blocked while the demon lives",
        swamp_game.level == SWAMP_BOSS_LEVEL &&
        swamp_game.location == LOCATION_SWAMP);
    Item reward = boss_equipment_reward(ENEMY_SWAMP_DEMON);
    ASSERT("demon reward matches long sword power and reaches two tiles",
        strcmp(reward.name, "Demonic Sword") == 0 &&
        reward.attack_bonus == item_make_long_sword().attack_bonus &&
        reward.range == 2 && reward.is_ranged &&
        reward.class_mask == ITEM_CLASS_ALL);
    for (int i = 0; i < swamp_game.enemy_count; i++) {
        Enemy *enemy = &swamp_game.enemies[i];
        if (enemy->type == ENEMY_SWAMP_DEMON) {
            enemy->hp = 1;
            swamp_game.player.x = enemy->x - 1;
            swamp_game.player.y = enemy->y;
            for (int tries = 0; enemy->active && tries < 20; tries++) {
                action_resolve_player(&swamp_game,
                    (Action){ACTION_MOVE, enemy->x, enemy->y});
            }
            break;
        }
    }
    int sword_dropped = 0;
    for (int i = 0; i < swamp_game.floor_item_count; i++) {
        sword_dropped += swamp_game.floor_items[i].active &&
            strcmp(swamp_game.floor_items[i].item.name, "Demonic Sword") == 0;
    }
    ASSERT("defeating the demon drops its one-time sword reward",
        sword_dropped == 1 &&
        (swamp_game.defeated_bosses & (1 << LOCATION_SWAMP)));
    swamp_game.player.x = final_exit_x - 1;
    swamp_game.player.y = final_exit_y;
    action_resolve_player(&swamp_game,
        (Action){ACTION_MOVE, final_exit_x, final_exit_y});
    ASSERT("victory exits the final swamp level onto its town road", swamp_game.location == LOCATION_APPROACH && swamp_game.enemy_count == 0);
    walk_swamp_road(&swamp_game);
    ASSERT("walking north reaches Rosemoor", swamp_game.location == LOCATION_TOWN3 && swamp_game.player.y == TOWN_H - 2);
    action_resolve_player(&swamp_game, (Action){ACTION_MOVE, 20, TOWN_H - 1});
    walk_swamp_road(&swamp_game);
    ASSERT("the open road returns south to Stillbury", swamp_game.location == LOCATION_TOWN2);
    ASSERT("the open swamp road survives saving", save_game(&swamp_game, 99121) && load_game(&swamp_loaded, 99121) && (swamp_loaded.defeated_bosses & (1 << LOCATION_SWAMP)));
    remove("saves/savegame_99121.json");

    for (int class_id = CLASS_WARRIOR; class_id <= CLASS_ROGUE; class_id++) {
        swamp_game.player.player_class = (PlayerClass)class_id;
        game_init(&swamp_game);
        int slot = swamp_game.inventory_count++;
        swamp_game.inventory[slot] = reward;
        action_resolve_player(&swamp_game,
            (Action){ACTION_EQUIP_ITEM, slot, 0});
        ASSERT("every class can equip the demonic sword",
            swamp_game.equipped_main_hand == EQUIPMENT_STRONG_HAND);
        game_enter_swamp(&swamp_game);
        swamp_game.enemy_count = 1;
        swamp_game.enemies[0] = (Enemy){0};
        swamp_game.enemies[0].active = 1;
        swamp_game.enemies[0].hp = 100;
        swamp_game.enemies[0].max_hp = 100;
        swamp_game.enemies[0].x = 12;
        swamp_game.enemies[0].y = 10;
        swamp_game.player.x = 10;
        swamp_game.player.y = 10;
        swamp_game.player.last_dx = 1;
        swamp_game.player.last_dy = 0;
        for (int x = 10; x <= 13; x++) {
            swamp_game.map.tiles[10][x] = TILE_SWAMP_FLOOR;
        }
        action_resolve_player(&swamp_game,
            (Action){ACTION_RANGED_ATTACK, 0, 0});
        ASSERT("F hits two tiles away with long sword damage",
            swamp_game.enemies[0].hp == 100 - swamp_game.player.attack);
        ASSERT("demonic sword sends its strike to the hit target",
            swamp_game.trail_effect == TRAIL_EFFECT_DEMONIC_SWORD &&
            swamp_game.trail_count == 2 &&
            swamp_game.trail[1].is_impact);
        swamp_game.enemies[0].hp = 100;
        swamp_game.enemies[0].x = 11;
        action_resolve_player(&swamp_game,
            (Action){ACTION_RANGED_ATTACK, 0, 0});
        ASSERT("F also hits an adjacent enemy with long sword damage",
            swamp_game.enemies[0].hp == 100 - swamp_game.player.attack);
        swamp_game.enemies[0].hp = 100;
        action_resolve_player(&swamp_game,
            (Action){ACTION_MOVE, 11, 10});
        ASSERT("demonic sword retains its full melee attack bonus",
            swamp_game.enemies[0].hp == 100 - swamp_game.player.attack);
        swamp_game.enemies[0].hp = 100;
        swamp_game.enemies[0].x = 12;
        swamp_game.map.tiles[10][11] = TILE_SWAMP_WALL;
        action_resolve_player(&swamp_game,
            (Action){ACTION_RANGED_ATTACK, 0, 0});
        ASSERT("a wall blocks the demonic sword's ranged attack",
            swamp_game.enemies[0].hp == 100);
    }

    swamp_game.player.player_class = CLASS_WARRIOR;
    game_init(&swamp_game);
    game_enter_town_gate(&swamp_game, LOCATION_TOWN2, 20, 0);
    int sword_slot = swamp_game.inventory_count++;
    swamp_game.inventory[sword_slot] = reward;
    action_resolve_player(&swamp_game,
        (Action){ACTION_EQUIP_ITEM, sword_slot, 0});
    game_enter_swamp(&swamp_game);
    game_descend(&swamp_game);
    game_open_town_portal(&swamp_game);
    ASSERT("swamp escape reaches Stillbury without a return portal",
        swamp_game.location == LOCATION_TOWN2 &&
        swamp_game.map.tiles[2][21] != TILE_PORTAL);
    int saved = save_game(&swamp_game, 99121);
    int loaded = saved && load_game(&swamp_loaded, 99121);
    ASSERT("swamp portal and floor cache survive save/load",
        loaded && swamp_loaded.portal_location == LOCATION_SWAMP &&
        swamp_loaded.swamp_cache[1].valid &&
        swamp_loaded.max_swamp_level_reached == 2);
    ASSERT("equipped demonic sword retains its class and range after load",
        loaded && swamp_loaded.equipped_main_hand == EQUIPMENT_STRONG_HAND &&
        swamp_loaded.inventory[EQUIPMENT_STRONG_HAND].class_mask == ITEM_CLASS_ALL &&
        swamp_loaded.inventory[EQUIPMENT_STRONG_HAND].range == 2);
    if (loaded) {
        game_enter_forest_road(&swamp_loaded);
        game_leave_forest_road(&swamp_loaded, LOCATION_TOWN);
        game_enter_forest_road(&swamp_loaded);
        game_leave_forest_road(&swamp_loaded, LOCATION_TOWN2);
        ASSERT("the retired swamp return portal stays absent after traveling between towns",
            !swamp_loaded.portal_active &&
            swamp_loaded.map.tiles[2][21] != TILE_PORTAL);
        swamp_loaded.player.x = 22;
        swamp_loaded.player.y = 2;
        action_resolve_player(&swamp_loaded,
            (Action){ACTION_MOVE, 21, 2});
        ASSERT("walking onto the former swamp portal stays in Stillbury",
            swamp_loaded.location == LOCATION_TOWN2 && !swamp_loaded.portal_active);
    }
    int connected_layouts = 1;
    for (int seed = 0; seed < 25; seed++) {
        srand(1000 + seed);
        for (int level = 1; level <= SWAMP_DEPTH; level++) {
            map_generate_swamp(&swamp_layout, level);
            if (!swamp_exit_reachable(&swamp_layout) ||
                !swamp_exits_at_edges(&swamp_layout)) {
                connected_layouts = 0;
            }
        }
    }
    ASSERT("random swamp shorelines never isolate traversable ground",
        connected_layouts);

    for (int seed = 0; seed < 3; seed++) {
        game_init(&swamp_game);
        srand(2100 + seed);
        // Historical quest data must not reactivate the retired encounter.
        swamp_game.innkeeper_quest_state = 1;
        game_enter_town2(&swamp_game);
        game_enter_swamp(&swamp_game);
        for (int level = 1; level <= SWAMP_DEPTH; level++) {
            int retired_encounter = 0;
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    retired_encounter |= swamp_game.map.tiles[y][x] == TILE_SWAMP_DAUGHTER;
                }
            }
            for (int i = 0; i < swamp_game.enemy_count; i++) {
                retired_encounter |= strcmp(swamp_game.enemies[i].name, "Vampire Captor") == 0;
            }
            ASSERT("fresh swamp stages never spawn Mira or her retired captor", !retired_encounter);
            ASSERT("retiring the quest preserves the swamp's entrance and exit route",
                swamp_game.level == level && swamp_exit_reachable(&swamp_game.map));
            if (level < SWAMP_DEPTH) {
                game_descend(&swamp_game);
            }
        }
    }
    remove("saves/savegame_99121.json");
}
