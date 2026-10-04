#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/screens/quest_journal.h"
#include "../src/systems/save_load.h"
#include <stdlib.h>
#include <string.h>

static Map desert_layout;
static unsigned char visited[MAP_H][MAP_W];
static int queue[MAP_W * MAP_H];
static GameState desert_game;
static GameState desert_loaded;

static void walk_desert_edge(GameState *g, int west) {
    int x = west ? g->map.stairs_down_x : g->map.stairs_up_x;
    int y = west ? g->map.stairs_down_y : g->map.stairs_up_y;
    g->player.x = west ? x + 1 : x - 1;
    g->player.y = y;
    action_resolve_player(g, (Action){ACTION_MOVE, x, y});
}

static void test_desert_travel(void) {
    GameState *g = &desert_game;
    memset(g, 0, sizeof(*g));
    g->player.player_class = CLASS_WARRIOR;
    game_init(g);
    srand(4242);
    g->defeated_bosses = 1 << LOCATION_FOREST;
    game_enter_town2(g);
    g->max_swamp_level_reached = 4;
    g->max_frostfell_level_reached = 3;
    g->max_level_reached = 7;
    g->player.x = 1;
    g->player.y = 12;
    action_resolve_player(g, (Action){ACTION_MOVE, 0, 12});
    ASSERT("Stillbury's west gate enters Sunscar level one from the east",
        g->location == LOCATION_DESERT && g->level == 1 &&
        g->player.x == DESERT_MAP_W - 1 && g->player.y == g->map.stairs_up_y);
    ASSERT("Sunscar opens without a swamp or frost boss defeat and spawns its regular roster",
        g->enemy_count == 11 && g->max_swamp_level_reached == 4 &&
        g->max_frostfell_level_reached == 3 && g->max_level_reached == 7);
    walk_desert_edge(g, 0);
    ASSERT("level one's east entrance returns to Stillbury's west gate",
        g->location == LOCATION_TOWN2 && g->player.x == 1 && g->player.y == 12);
    action_resolve_player(g, (Action){ACTION_MOVE, 0, 12});

    int x;
    int y;
    map_room_center(&g->map.rooms[0], &x, &y);
    map_mark_explored(&g->map, x, y);
    g->enemies[0] = (Enemy){
        .active = 1, .type = ENEMY_SKELETON, .name = "Skeleton",
        .x = x, .y = y, .hp = 22, .max_hp = 30
    };
    g->enemy_count = 1;
    g->level_cleared = 0;
    walk_desert_edge(g, 1);
    ASSERT("a west exit advances even with a living regular enemy behind",
        g->location == LOCATION_DESERT && g->level == 2 &&
        g->player.x == g->map.stairs_up_x && g->desert_cache[0].valid);
    walk_desert_edge(g, 0);
    ASSERT("backtracking restores the desert map, exploration and enemy state",
        g->level == 1 && g->player.x == g->map.stairs_down_x &&
        map_is_explored(&g->map, x, y) && g->enemy_count == 1 &&
        g->enemies[0].hp == 22 && !g->level_cleared);
    for (int level = 1; level < DESERT_DEPTH; level++) {
        walk_desert_edge(g, 1);
    }
    ASSERT("western exits reach level five using only the desert's depth tracking",
        g->level == DESERT_DEPTH && g->max_desert_level_reached == DESERT_DEPTH &&
        g->max_swamp_level_reached == 4 && g->max_frostfell_level_reached == 3 &&
        g->max_level_reached == 7);
    walk_desert_edge(g, 0);
    ASSERT("level five's east entrance returns to level four's west exit",
        g->level == DESERT_DEPTH - 1 && g->player.x == g->map.stairs_down_x &&
        g->desert_cache[DESERT_DEPTH - 1].valid);
    walk_desert_edge(g, 1);

    // Portal travel is checked without creatures occupying the landing tile.
    g->enemy_count = 0;
    map_room_center(&g->map.rooms[0], &x, &y);
    g->player.x = x;
    g->player.y = y;
    g->inventory[0] = item_make_health_potion();
    g->inventory_count = 1;
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, 0, 0});
    g->player.known_spells[0] = spell_make_return_to_town();
    g->player.known_spell_count = 1;
    g->player.equipped_spell = 0;
    g->player.mp = 100;
    action_resolve_player(g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("Return to Town opens a desert portal at Stillbury's west gate",
        g->location == LOCATION_TOWN2 && g->player.x == 1 && g->player.y == 12 &&
        g->portal_active && g->portal_location == LOCATION_DESERT &&
        g->portal_origin_tile == TILE_DESERT_FLOOR &&
        g->map.tiles[12][2] == TILE_PORTAL && g->floor_item_count == 0 &&
        g->desert_cache[DESERT_DEPTH - 1].map.tiles[y][x] == TILE_DESERT_FLOOR);

    const int slot = 99021;
    int saved = save_game(g, slot) && load_game(&desert_loaded, slot);
    ASSERT("desert progress and Stillbury's return portal survive save/load",
        saved && desert_loaded.location == LOCATION_TOWN2 &&
        desert_loaded.map.tiles[12][0] == TILE_TOWN_EXIT &&
        desert_loaded.map.tiles[12][2] == TILE_PORTAL &&
        desert_loaded.max_desert_level_reached == DESERT_DEPTH &&
        desert_loaded.desert_cache[0].enemies[0].hp == 22);
    if (saved) {
        action_resolve_player(&desert_loaded, (Action){ACTION_MOVE, 2, 12});
    }
    ASSERT("the saved portal restores the exact desert level and landing tile",
        saved && desert_loaded.location == LOCATION_DESERT &&
        desert_loaded.level == DESERT_DEPTH && desert_loaded.player.x == x &&
        desert_loaded.player.y == y && !desert_loaded.portal_active &&
        desert_loaded.map.tiles[y][x] == TILE_DESERT_FLOOR);
    remove("saves/savegame_99021.json");
    g->desert_cache[DESERT_DEPTH - 1].map.tiles[y][x] = TILE_PORTAL;
    game_hide_portal_destination(g);
    ASSERT("hiding a desert portal restores its cached sand tile",
        g->desert_cache[DESERT_DEPTH - 1].map.tiles[y][x] == TILE_DESERT_FLOOR);
    action_resolve_player(g, (Action){ACTION_MOVE, 2, 12});

    map_room_center(&g->map.rooms[DESERT_DEPTH], &x, &y);
    g->enemies[0] = (Enemy){
        .active = 1, .is_boss = 1, .type = ENEMY_SKELETON,
        .name = "Test Boss", .x = x, .y = y, .hp = 100, .max_hp = 100
    };
    g->enemy_count = 1;
    walk_desert_edge(g, 1);
    ASSERT("a living boss blocks level five's west exit",
        g->location == LOCATION_DESERT && g->level == DESERT_DEPTH &&
        strcmp(g->messages[g->message_count - 1],
            "The Desert Pharaoh bars the way west!") == 0);
    g->enemies[0].active = 0;
    g->enemies[1] = g->enemies[0];
    g->enemies[1].is_boss = 0;
    g->enemies[1].active = 1;
    g->enemies[1].x++;
    g->enemy_count = 2;
    walk_desert_edge(g, 1);
    ASSERT("the final west exit returns to Stillbury with regular enemies still alive",
        g->location == LOCATION_TOWN2 && g->player.x == 1 && g->player.y == 12 &&
        g->desert_cache[DESERT_DEPTH - 1].enemies[1].active);

    g->portal_active = 1;
    g->portal_location = LOCATION_DESERT;
    g->portal_level = DESERT_DEPTH + 1;
    game_hide_portal_destination(g);
    game_use_town_portal(g);
    ASSERT("desert portals cannot index a sixth cache entry", g->location == LOCATION_TOWN2);
}

static void test_desert_rosters(void) {
    GameState *g = &desert_game;
    memset(g, 0, sizeof(*g));
    game_init(g);
    g->location = LOCATION_DESERT;
    int roster_ok = 1;
    int placed_ok = 1;
    int progression_ok = 1;
    int boss_ok = 1;
    const EnemyType last_type[DESERT_DEPTH] = {
        ENEMY_VIPER, ENEMY_MUMMY, ENEMY_DJINN, ENEMY_GOLEM, ENEMY_GOLEM
    };
    for (int level = 1; level <= DESERT_DEPTH; level++) {
        int seen[5] = {0};
        for (int tier = 0; tier < 2; tier++) {
            g->defeated_bosses = 1 << LOCATION_FOREST;
            if (tier) {
                g->defeated_bosses |= (1 << LOCATION_DUNGEON) |
                    (1 << LOCATION_MOUNTAINS) | (1 << LOCATION_COAST);
            }
            for (int seed = 1; seed <= 20; seed++) {
                srand((unsigned int)(seed * 31 + level));
                g->level = level;
                map_generate_desert(&g->map, level);
                g->player.x = g->map.stairs_up_x;
                g->player.y = g->map.stairs_up_y;
                enemies_spawn(g);
                roster_ok &= g->enemy_count == 10 + level;
                int bosses = 0;
                int boss_x;
                int boss_y;
                map_room_center(&g->map.rooms[g->map.room_count - 1], &boss_x, &boss_y);
                for (int i = 0; i < g->enemy_count; i++) {
                    const Enemy *e = &g->enemies[i];
                    if (e->type == ENEMY_DESERT_PHARAOH) {
                        bosses++;
                        boss_ok &= level == DESERT_DEPTH && e->is_boss && e->active &&
                            e->x == boss_x && e->y == boss_y && e->hp == e->max_hp &&
                            e->hp > 0 && e->attack > 0 && e->experience > 0 &&
                            strcmp(e->name, "Desert Pharaoh") == 0;
                        continue;
                    }
                    int desert_type = e->type >= ENEMY_SCARAB && e->type <= ENEMY_GOLEM;
                    roster_ok &= desert_type && !e->is_boss && e->active &&
                        e->hp == e->max_hp && e->hp > 0 && e->attack > 0 &&
                        e->experience > 0 && e->name[0] != '\0';
                    if (desert_type) {
                        seen[e->type - ENEMY_SCARAB]++;
                        progression_ok &= e->type <= last_type[level - 1];
                    }
                    placed_ok &= g->map.tiles[e->y][e->x] == TILE_DESERT_FLOOR;
                    for (int j = 0; j < i; j++) {
                        placed_ok &= e->x != g->enemies[j].x || e->y != g->enemies[j].y;
                    }
                }
                boss_ok &= bosses == (level == DESERT_DEPTH);
            }
        }
        for (int type = ENEMY_SCARAB; type <= last_type[level - 1]; type++) {
            progression_ok &= seen[type - ENEMY_SCARAB] > 0;
        }
    }
    ASSERT("all five Sunscar levels spawn only desert enemies with usable stats", roster_ok);
    ASSERT("desert enemies occupy separate sand tiles and leave exits open", placed_ok);
    ASSERT("desert roles unlock by stage even after other regional victories", progression_ok);
    ASSERT("only level five spawns one Desert Pharaoh at the final clearing's center", boss_ok);

    g->location = LOCATION_DESERT;
    g->level = DESERT_DEPTH;
    for (int type = ENEMY_SCARAB; type <= ENEMY_GOLEM; type++) {
        int index = type - ENEMY_SCARAB;
        map_room_center(&g->map.rooms[index], &g->enemies[index].x, &g->enemies[index].y);
        g->enemies[index].type = (EnemyType)type;
        g->enemies[index].move_timer = index + 1;
        g->enemies[index].hp--;
    }
    g->enemies[0].active = 0;
    g->enemies[3].frozen_turns = 2;
    g->enemy_count = 5;
    walk_desert_edge(g, 0);
    const int slot = 99022;
    int saved = save_game(g, slot) && load_game(&desert_loaded, slot);
    if (saved) {
        walk_desert_edge(&desert_loaded, 1);
    }
    int preserved = saved && desert_loaded.location == LOCATION_DESERT &&
        desert_loaded.level == DESERT_DEPTH && desert_loaded.enemy_count == 5;
    for (int i = 0; i < 5 && saved; i++) {
        const Enemy *expected = &g->desert_cache[DESERT_DEPTH - 1].enemies[i];
        const Enemy *actual = &desert_loaded.enemies[i];
        preserved &= actual->type == expected->type && actual->hp == expected->hp &&
            actual->active == expected->active && actual->move_timer == expected->move_timer &&
            actual->frozen_turns == expected->frozen_turns;
    }
    ASSERT("cached desert enemy types, damage, deaths and timers survive save/load", preserved);
    remove("saves/savegame_99022.json");
}

static void setup_desert_enemy(EnemyType type, int x, int y) {
    GameState *g = &desert_game;
    memset(g, 0, sizeof(*g));
    g->player.player_class = CLASS_WARRIOR;
    game_init(g);
    g->location = LOCATION_DESERT;
    g->level = 3;
    for (int ty = 0; ty < MAP_H; ty++) {
        for (int tx = 0; tx < MAP_W; tx++) {
            g->map.tiles[ty][tx] = TILE_DESERT_WALL;
        }
    }
    for (int ty = 10; ty < 30; ty++) {
        for (int tx = 10; tx < 30; tx++) {
            g->map.tiles[ty][tx] = TILE_DESERT_FLOOR;
        }
    }
    g->player.x = 20;
    g->player.y = 20;
    g->player.hp = 100;
    g->player.defense = 4;
    g->equipped_armor = -1;
    g->equipped_off_hand = -1;
    g->enemy_count = 1;
    g->enemies[0] = (Enemy){
        .type = type, .x = x, .y = y, .active = 1,
        .name = "Desert Enemy", .hp = 100, .max_hp = 100, .attack = 12
    };
}

static void test_desert_combat(void) {
    GameState *g = &desert_game;
    setup_desert_enemy(ENEMY_VIPER, 21, 20);
    action_resolve_enemies(g);
    ASSERT("a Viper bite damages and poisons the player", g->player.hp == 92 && g->player.poison_turns == 3);
    setup_desert_enemy(ENEMY_VIPER, 21, 20);
    g->inventory[0] = item_make_shadow_armor();
    g->inventory[0].evasion_chance = 100;
    g->inventory_count = 1;
    g->equipped_armor = 0;
    action_resolve_enemies(g);
    ASSERT("a dodged Viper bite does not poison the player", g->player.hp == 100 && !g->player.poison_turns);

    const EnemyType slow[2] = {ENEMY_MUMMY, ENEMY_GOLEM};
    int slow_ok = 1;
    for (int i = 0; i < 2; i++) {
        setup_desert_enemy(slow[i], 24, 20);
        action_resolve_enemies(g);
        slow_ok &= g->enemies[0].x == 24;
        action_resolve_enemies(g);
        slow_ok &= g->enemies[0].x == 23;
    }
    ASSERT("Mummies and Golems advance only every other turn", slow_ok);
    setup_desert_enemy(ENEMY_SCARAB, 24, 21);
    action_resolve_enemies(g);
    ASSERT("Scarabs close distance on their first turn",
        abs(g->enemies[0].x - 20) + abs(g->enemies[0].y - 20) < 5);

    EnemyProjectiles shots = {0};
    setup_desert_enemy(ENEMY_DJINN, 24, 20);
    action_resolve_enemies_with_projectiles(g, &shots);
    int recovered = g->player.hp == 100 && shots.count == 0 && g->enemies[0].x == 24;
    action_resolve_enemies_with_projectiles(g, &shots);
    ASSERT("Djinn alternate recovery with magic bolts using half the player's defense",
        recovered && g->player.hp == 90 && shots.count == 1 &&
        shots.shots[0].type == ENEMY_DJINN && g->enemies[0].x == 24);
    setup_desert_enemy(ENEMY_DJINN, 24, 20);
    g->map.tiles[20][22] = TILE_DESERT_WALL;
    g->enemies[0].move_timer = 1;
    action_resolve_enemies_with_projectiles(g, &shots);
    ASSERT("sandstone blocks a Djinn bolt", g->player.hp == 100 && shots.count == 0);
    setup_desert_enemy(ENEMY_DJINN, 24, 20);
    g->enemies[1] = g->enemies[0];
    g->enemies[1].type = ENEMY_MUMMY;
    g->enemies[1].x = 22;
    g->enemies[0].move_timer = 1;
    g->enemy_count = 2;
    action_resolve_enemies_with_projectiles(g, &shots);
    ASSERT("a living enemy blocks a Djinn bolt", g->player.hp == 100 && shots.count == 0);

    int rewards_ok = 1;
    int loot_ok = 1;
    for (int type = ENEMY_SCARAB; type <= ENEMY_GOLEM; type++) {
        setup_desert_enemy((EnemyType)type, 21, 20);
        srand((unsigned int)(4242 + type));
        g->player.known_spells[0] = spell_make_magic_arrow();
        g->player.known_spell_count = 1;
        g->player.equipped_spell = 0;
        g->player.last_dx = 1;
        g->player.last_dy = 0;
        g->enemies[0].experience = 10;
        int gold = 0;
        for (int kill = 0; kill < 200; kill++) {
            g->map.tiles[20][21] = TILE_DESERT_FLOOR;
            g->floor_item_count = 0;
            g->enemies[0].active = 1;
            g->enemies[0].hp = 1;
            g->player.mp = 100;
            int score = g->score;
            action_resolve_player(g, (Action){ACTION_CAST_SPELL, 0, 0});
            rewards_ok &= !g->enemies[0].active && g->score > score;
            gold += g->gold;
            g->gold = 0;
            for (int i = 0; i < g->floor_item_count; i++) {
                const FloorItem *item = &g->floor_items[i];
                loot_ok &= item->underlying_tile == TILE_DESERT_FLOOR;
                if (item->item.type == ITEM_GOLD) {
                    gold += item->item.value;
                }
            }
        }
        rewards_ok &= g->score > 0 && g->player.level > 1 && gold > 0;
    }
    ASSERT("every desert enemy awards score, experience and occasional gold", rewards_ok);
    ASSERT("desert loot preserves the sand underneath", loot_ok);
}

static void setup_pharaoh_combat(void) {
    setup_desert_enemy(ENEMY_DESERT_PHARAOH, 24, 20);
    desert_game.level = DESERT_DEPTH;
    desert_game.map.room_count = 1;
    desert_game.map.rooms[0] = (Room){20, 18, 10, 8};
    desert_game.enemies[0].is_boss = 1;
}

static void test_pharaoh_combat(void) {
    GameState *g = &desert_game;
    EnemyProjectiles shots = {0};
    setup_pharaoh_combat();
    g->player.x = 19;
    action_resolve_enemies_with_projectiles(g, &shots);
    ASSERT("the Pharaoh waits for the player to enter his clearing",
        g->player.hp == 100 && shots.count == 0 && g->enemies[0].move_timer == 0);
    g->player.x = 20;
    action_resolve_enemies_with_projectiles(g, &shots);
    int warned = g->player.hp == 100 && shots.count == 0 &&
        strcmp(g->messages[g->message_count - 1], "The Pharaoh gathers desert magic...") == 0;
    action_resolve_enemies_with_projectiles(g, &shots);
    ASSERT("the Pharaoh warns then fires a magic bolt using half the player's defense",
        warned && g->player.hp == 90 && shots.count == 1 &&
        shots.shots[0].type == ENEMY_DESERT_PHARAOH &&
        g->enemies[0].x == 24 && g->enemies[0].y == 20);

    setup_pharaoh_combat();
    g->enemies[0].move_timer = 1;
    g->map.tiles[20][22] = TILE_DESERT_WALL;
    action_resolve_enemies_with_projectiles(g, &shots);
    ASSERT("sandstone blocks the Pharaoh's magic bolt", g->player.hp == 100 && shots.count == 0);
    setup_pharaoh_combat();
    g->enemies[0].move_timer = 1;
    g->enemies[1] = (Enemy){
        .type = ENEMY_MUMMY, .active = 1, .x = 22, .y = 20,
        .hp = 100, .max_hp = 100, .frozen_turns = 2
    };
    g->enemy_count = 2;
    action_resolve_enemies_with_projectiles(g, &shots);
    ASSERT("living enemies block the Pharaoh's magic bolt", g->player.hp == 100 && shots.count == 0);

    setup_pharaoh_combat();
    g->enemies[0].hp--;
    g->player.x = 19;
    action_resolve_enemies_with_projectiles(g, &shots);
    int awakened = g->enemies[0].move_timer == 1;
    action_resolve_enemies_with_projectiles(g, &shots);
    ASSERT("a ranged hit awakens the Pharaoh outside his clearing",
        awakened && g->player.hp == 90 && shots.count == 1);
    g->player.x = 10;
    action_resolve_enemies_with_projectiles(g, &shots);
    ASSERT("the Pharaoh stops attacking when the player retreats far away",
        g->player.hp == 90 && shots.count == 0 && g->enemies[0].move_timer == 2);

    setup_pharaoh_combat();
    g->player.x = 23;
    action_resolve_enemies_with_projectiles(g, &shots);
    ASSERT("an adjacent player takes a normal melee hit from the Pharaoh",
        g->player.hp == 92 && shots.count == 0);
}

static int find_pharaoh(const GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].type == ENEMY_DESERT_PHARAOH) {
            return i;
        }
    }
    return -1;
}

static int sandstorm_staves_on_floor(const GameState *g) {
    int count = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        count += g->floor_items[i].active &&
            strcmp(g->floor_items[i].item.name, "Sandstorm Staff") == 0;
    }
    return count;
}

static void test_pharaoh_reward(void) {
    GameState *g = &desert_game;
    memset(g, 0, sizeof(*g));
    g->player.player_class = CLASS_MAGE;
    game_init(g);
    ASSERT("a fresh game starts without an unclaimed Sandstorm Staff", !g->sandstorm_staff_unclaimed);
    srand(707);
    g->defeated_bosses = 1 << LOCATION_FOREST;
    game_enter_town2(g);
    game_enter_desert(g);
    while (g->level < DESERT_DEPTH) {
        game_descend(g);
    }
    int k = find_pharaoh(g);
    ASSERT("the final Sunscar level has a living Pharaoh", k >= 0);
    if (k < 0) {
        return;
    }
    walk_desert_edge(g, 1);
    ASSERT("the actual Pharaoh blocks the final west exit", g->location == LOCATION_DESERT);
    g->enemies[k].hp--;
    g->enemies[k].move_timer = 1;
    game_ascend(g);
    const int slot = 99023;
    int saved = save_game(g, slot) && load_game(&desert_loaded, slot);
    if (saved) {
        *g = desert_loaded;
        game_descend(g);
    }
    k = find_pharaoh(g);
    ASSERT("the Pharaoh's health, boss flag and attack timer survive cached save/load",
        saved && k >= 0 && g->enemies[k].is_boss &&
        g->enemies[k].hp == g->enemies[k].max_hp - 1 && g->enemies[k].move_timer == 1);
    if (!saved || k < 0) {
        remove("saves/savegame_99023.json");
        return;
    }
    Enemy boss = g->enemies[k];
    g->enemies[0] = boss;
    g->enemy_count = 1;
    g->enemies[0].hp = 1;
    g->player.x = boss.x + 1;
    g->player.y = boss.y;
    action_resolve_player(g, (Action){ACTION_MOVE, boss.x, boss.y});
    BossJournalEntry entry;
    ASSERT("defeating the Pharaoh grants his staff, experience and boss journal completion",
        !g->enemies[0].active && g->player.level > 1 &&
        g->sandstorm_staff_unclaimed && sandstorm_staves_on_floor(g) == 1 &&
        (g->defeated_bosses & (1 << LOCATION_DESERT)) &&
        quest_journal_get_boss(g, JOURNAL_BOSS_COUNT - 1, &entry) && entry.defeated &&
        strcmp(entry.name, "Desert Pharaoh") == 0 && strcmp(entry.area, "Sunscar Wastes") == 0);

    game_open_town_portal(g);
    saved = save_game(g, slot) && load_game(&desert_loaded, slot);
    if (saved) {
        *g = desert_loaded;
        game_use_town_portal(g);
    }
    game_refresh_quest_encounters(g);
    game_refresh_quest_encounters(g);
    ASSERT("an unclaimed staff survives saving in Stillbury and returning through the portal once",
        saved && g->location == LOCATION_DESERT && g->sandstorm_staff_unclaimed &&
        sandstorm_staves_on_floor(g) == 1 && find_pharaoh(g) < 0);
    game_ascend(g);
    game_descend(g);
    ASSERT("backtracking preserves the unclaimed staff on level five",
        g->level == DESERT_DEPTH && sandstorm_staves_on_floor(g) == 1);
    walk_desert_edge(g, 1);
    ASSERT("defeating the Pharaoh opens the final west exit to Stillbury",
        g->location == LOCATION_TOWN2 && g->player.x == 1 && g->player.y == 12);
    game_enter_desert(g);
    while (g->level < DESERT_DEPTH) {
        game_descend(g);
    }
    ASSERT("a fresh desert visit restores the staff without respawning the Pharaoh",
        sandstorm_staves_on_floor(g) == 1 && find_pharaoh(g) < 0);

    int x;
    int y;
    map_room_center(&g->map.rooms[g->map.room_count - 1], &x, &y);
    g->player.x = x;
    g->player.y = y;
    int inventory_count = g->inventory_count;
    g->inventory_count = MAX_INVENTORY;
    action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("a full inventory leaves the staff available and unclaimed",
        g->sandstorm_staff_unclaimed && sandstorm_staves_on_floor(g) == 1);
    g->inventory_count = inventory_count;
    action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("picking up the staff clears its claim and restores the sand tile",
        !g->sandstorm_staff_unclaimed && sandstorm_staves_on_floor(g) == 0 &&
        g->map.tiles[y][x] == TILE_DESERT_FLOOR);
    int staff_index = g->inventory_count - 1;
    int base_mp = g->player.max_mp;
    if (g->equipped_main_hand >= 0) {
        base_mp -= g->inventory[g->equipped_main_hand].max_mp_bonus;
    }
    action_resolve_player(g, (Action){ACTION_EQUIP_ITEM, staff_index, 0});
    ASSERT("a Mage can equip the two-handed Sandstorm Staff",
        g->equipped_main_hand == staff_index && g->equipped_off_hand == -1 &&
        g->player.max_mp == base_mp + 25);
    game_open_town_portal(g);
    saved = save_game(g, slot) && load_game(&desert_loaded, slot);
    if (saved) {
        *g = desert_loaded;
        game_use_town_portal(g);
    }
    int in_pack = 0;
    int metadata_ok = 0;
    for (int i = 0; i < g->inventory_count; i++) {
        const Item *item = &g->inventory[i];
        if (strcmp(item->name, "Sandstorm Staff") == 0) {
            in_pack++;
            metadata_ok = item->attack_bonus == 7 && item->spell_power_bonus == 6 &&
                item->max_mp_bonus == 25 && item->spell_cost_reduction_percent == 7 &&
                item->weapon_family == WEAPON_FAMILY_STAFF && item->weapon_hands == WEAPON_HANDS_TWO &&
                item->class_mask == ITEM_CLASS_MAGE && item->rarity == ITEM_RARITY_RARE &&
                item->visual_id == ITEM_VISUAL_SANDSTORM_STAFF;
        }
    }
    ASSERT("saving after pickup preserves staff bonuses and equipment without duplicating it",
        saved && in_pack == 1 && metadata_ok && g->equipped_main_hand == staff_index &&
        !g->sandstorm_staff_unclaimed && sandstorm_staves_on_floor(g) == 0 && find_pharaoh(g) < 0);
    remove("saves/savegame_99023.json");

    setup_pharaoh_combat();
    g->enemies[0].x = 21;
    g->enemies[0].hp = 1;
    g->floor_item_count = MAX_FLOOR_ITEMS;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        g->floor_items[i] = (FloorItem){
            .active = 1, .x = 10 + i % 20, .y = 10 + i / 20,
            .underlying_tile = TILE_DESERT_FLOOR, .item = item_make_health_potion()
        };
        g->map.tiles[g->floor_items[i].y][g->floor_items[i].x] = TILE_ITEM;
    }
    g->player.known_spells[0] = spell_make_magic_arrow();
    g->player.known_spell_count = 1;
    g->player.equipped_spell = 0;
    g->player.last_dx = 1;
    g->player.last_dy = 0;
    g->player.mp = 100;
    action_resolve_player(g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("a spell kill still guarantees the staff when all floor item slots are occupied",
        !g->enemies[0].active && g->sandstorm_staff_unclaimed &&
        sandstorm_staves_on_floor(g) == 1 && g->floor_item_count == MAX_FLOOR_ITEMS &&
        g->gold > 0 && g->score >= 1600);
}

static int count_desert_lamps(const Map *m) {
    int count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            count += m->tiles[y][x] == TILE_DESERT_LAMP;
        }
    }
    return count;
}

static void test_desert_lamp_quest(void) {
    GameState *g = &desert_game;
    memset(g, 0, sizeof(*g));
    game_init(g);
    srand(811);
    g->defeated_bosses = (1 << LOCATION_FOREST) | (1 << LOCATION_SWAMP) | (1 << LOCATION_DESERT);
    game_enter_desert(g);
    while (g->level < DESERT_LAMP_LEVEL) {
        game_descend(g);
    }
    ASSERT("the magic lamp does not appear before accepting Zara's quest", count_desert_lamps(&g->map) == 0);
    int x;
    int y;
    map_room_center(&g->map.rooms[g->map.room_count - 1], &x, &y);
    map_mark_explored(&g->map, x, y);
    g->player.x = x;
    g->player.y = y;
    g->enemies[0].hp--;
    int hp = g->enemies[0].hp;
    game_open_town_portal(g);
    game_enter_town3(g);
    g->player.x = TOWN_GUILD_DOOR_X;
    g->player.y = TOWN_GUILD_DOOR_Y + 1;
    action_resolve_player(g, (Action){ACTION_MOVE, TOWN_GUILD_DOOR_X, TOWN_GUILD_DOOR_Y});
    ASSERT("Rosemoor's Guild doorway enters a safe hall with Zara and no inn NPCs",
        g->location == LOCATION_GUILD && g->enemy_count == 0 &&
        g->map.tiles[GUILD_ZARA_Y][GUILD_ZARA_X] == TILE_NPC_GUILD_SEEKER &&
        !map_is_walkable(&g->map, GUILD_ZARA_X, GUILD_ZARA_Y) &&
        g->map.tiles[18][10] == TILE_TAVERN_FLOOR);
    g->player.x = GUILD_ZARA_X;
    g->player.y = GUILD_ZARA_Y + 1;
    game_talk_to_guild_seeker(g);
    ASSERT("Zara assigns the lamp quest without resetting desert progress or its portal",
        g->sunscar_lamp_quest_state == 1 && g->portal_active &&
        g->max_desert_level_reached == DESERT_LAMP_LEVEL &&
        g->desert_cache[DESERT_LAMP_LEVEL - 1].enemies[0].hp == hp &&
        map_is_explored(&g->desert_cache[DESERT_LAMP_LEVEL - 1].map, x, y));
    QuestJournalEntry entry;
    ASSERT("the journal shows Zara's lamp objective, stage and rewards",
        quest_journal_count(g, QUEST_TAB_ACTIVE) == 1 &&
        quest_journal_get_entry(g, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "The Lost Magic Lamp") == 0 && strcmp(entry.giver, "Zara") == 0 &&
        entry.stages[0] == DESERT_LAMP_LEVEL && entry.reward_gold == 80 && entry.reward_score == 600);
    game_talk_to_guild_seeker(g);
    const int slot = 99025;
    int saved = save_game(g, slot) && load_game(&desert_loaded, slot);
    ASSERT("saving inside the Guild preserves the quest, NPC and desert portal",
        saved && desert_loaded.location == LOCATION_GUILD && desert_loaded.sunscar_lamp_quest_state == 1 &&
        desert_loaded.portal_active && desert_loaded.map.tiles[GUILD_ZARA_Y][GUILD_ZARA_X] == TILE_NPC_GUILD_SEEKER);
    if (saved) {
        *g = desert_loaded;
    }
    g->player.x = 20;
    g->player.y = 21;
    action_resolve_player(g, (Action){ACTION_MOVE, 20, 22});
    ASSERT("leaving the Guild returns outside its Rosemoor door",
        g->location == LOCATION_TOWN3 && g->player.x == TOWN_GUILD_DOOR_X &&
        g->player.y == TOWN_GUILD_DOOR_Y + 1);
    game_enter_town2(g);
    game_use_town_portal(g);
    game_refresh_quest_encounters(g);
    game_refresh_quest_encounters(g);
    ASSERT("returning to an already explored desert stage places exactly one lamp on reachable sand",
        g->location == LOCATION_DESERT && g->level == DESERT_LAMP_LEVEL &&
        count_desert_lamps(&g->map) == 1 && g->map.tiles[y][x] == TILE_DESERT_LAMP &&
        map_is_walkable(&g->map, x, y));
    game_ascend(g);
    game_descend(g);
    ASSERT("backtracking preserves the uncollected lamp", count_desert_lamps(&g->map) == 1);
    g->player.x = x;
    g->player.y = y;
    game_open_town_portal(g);
    saved = save_game(g, slot) && load_game(&desert_loaded, slot);
    if (saved) {
        *g = desert_loaded;
        game_use_town_portal(g);
    }
    ASSERT("a saved portal opened on the lamp restores the lamp without duplication",
        saved && g->map.tiles[y][x] == TILE_DESERT_LAMP && count_desert_lamps(&g->map) == 1);

    g->inventory[g->inventory_count++] = item_make_health_potion();
    g->inventory[g->inventory_count++] = item_make_mana_potion();
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, g->inventory_count - 1, 0});
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, g->inventory_count - 1, 0});
    game_refresh_quest_encounters(g);
    action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
    action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("stacked loot on the lamp restores its quest marker after pickup", g->map.tiles[y][x] == TILE_DESERT_LAMP);
    g->inventory_count = MAX_INVENTORY;
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("recovering the lamp works with a full pack and marks the quest ready to return",
        g->sunscar_lamp_quest_state == 2 && g->map.tiles[y][x] == TILE_DESERT_FLOOR &&
        quest_journal_get_entry(g, QUEST_TAB_ACTIVE, 0, &entry) && entry.state == 2 && entry.objective_complete[0]);
    game_open_town_portal(g);
    saved = save_game(g, slot) && load_game(&desert_loaded, slot);
    if (saved) {
        *g = desert_loaded;
        game_use_town_portal(g);
    }
    game_refresh_quest_encounters(g);
    ASSERT("saving and revisiting after lamp pickup cannot spawn a second lamp",
        saved && g->sunscar_lamp_quest_state == 2 && count_desert_lamps(&g->map) == 0);
    game_return_to_town(g);
    game_enter_town3(g);
    game_enter_guild(g);
    int gold = g->gold;
    int score = g->score;
    game_talk_to_guild_seeker(g);
    game_talk_to_guild_seeker(g);
    ASSERT("returning the lamp pays exactly once and moves the quest to the completed journal",
        g->sunscar_lamp_quest_state == 3 && g->gold == gold + 80 && g->score == score + 600 &&
        quest_journal_count(g, QUEST_TAB_ACTIVE) == 0 && quest_journal_count(g, QUEST_TAB_COMPLETED) == 1);
    saved = save_game(g, slot) && load_game(&desert_loaded, slot);
    if (saved) {
        game_talk_to_guild_seeker(&desert_loaded);
    }
    ASSERT("loading a completed lamp quest cannot repeat its reward",
        saved && desert_loaded.sunscar_lamp_quest_state == 3 &&
        desert_loaded.gold == gold + 80 && desert_loaded.score == score + 600);
    remove("saves/savegame_99025.json");
}

static int desert_layout_connected(const Map *m) {
    memset(visited, 0, sizeof(visited));
    int head = 0;
    int tail = 0;
    queue[tail++] = m->stairs_up_y * MAP_W + m->stairs_up_x;
    visited[m->stairs_up_y][m->stairs_up_x] = 1;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail) {
        int pos = queue[head++];
        int x = pos % MAP_W;
        int y = pos / MAP_W;
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (map_is_walkable(m, nx, ny) && !visited[ny][nx]) {
                visited[ny][nx] = 1;
                queue[tail++] = ny * MAP_W + nx;
            }
        }
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (map_is_walkable(m, x, y) && !visited[y][x]) {
                return 0;
            }
        }
    }
    return 1;
}

void test_desert(void) {
    printf("Sunscar Wastes map tests:\n");
    int connected = 1;
    int edges = 1;
    int terrain = 1;
    int rooms = 1;
    int unexplored = 1;
    for (int level = 1; level <= DESERT_DEPTH; level++) {
        for (int seed = 1; seed <= 100; seed++) {
            srand((unsigned int)(seed * 31 + level));
            memset(desert_layout.explored, 0xff, sizeof(desert_layout.explored));
            map_generate_desert(&desert_layout, level);
            connected &= desert_layout_connected(&desert_layout);
            edges &= desert_layout.stairs_up_x == DESERT_MAP_W - 1 &&
                desert_layout.stairs_down_x == 0 &&
                desert_layout.tiles[desert_layout.stairs_up_y][DESERT_MAP_W - 1] ==
                    TILE_DESERT_ENTRANCE &&
                desert_layout.tiles[desert_layout.stairs_down_y][0] == TILE_DESERT_EXIT;
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    TileType tile = desert_layout.tiles[y][x];
                    terrain &= tile == TILE_DESERT_FLOOR || tile == TILE_DESERT_WALL ||
                        tile == TILE_DESERT_ENTRANCE || tile == TILE_DESERT_EXIT;
                    int border = x == 0 || x >= DESERT_MAP_W - 1 ||
                        y == 0 || y >= DESERT_MAP_H - 1;
                    int entrance = x == desert_layout.stairs_up_x &&
                        y == desert_layout.stairs_up_y;
                    int exit = x == desert_layout.stairs_down_x &&
                        y == desert_layout.stairs_down_y;
                    if (border && !entrance && !exit) {
                        edges &= !map_is_walkable(&desert_layout, x, y);
                    }
                }
            }
            for (int i = 0; i < desert_layout.room_count; i++) {
                const Room *room = &desert_layout.rooms[i];
                int x;
                int y;
                map_room_center(room, &x, &y);
                rooms &= room->x >= 0 && room->y >= 0 &&
                    room->x + room->w <= DESERT_MAP_W &&
                    room->y + room->h <= DESERT_MAP_H &&
                    x > 0 && x < DESERT_MAP_W - 1 &&
                    y > 0 && y < DESERT_MAP_H - 1 &&
                    desert_layout.tiles[y][x] == TILE_DESERT_FLOOR;
            }
            for (int i = 0; i < MAP_EXPLORED_BYTES; i++) {
                unexplored &= desert_layout.explored[i] == 0;
            }
        }
    }
    ASSERT("all five desert levels have connected sand paths across 100 seeds", connected);
    ASSERT("desert levels have only an east entrance and west exit at the borders", edges);
    ASSERT("desert maps contain only their own terrain and no frost hazards", terrain);
    ASSERT("desert clearing centers stay on sand inside the map", rooms);
    ASSERT("generating a desert level resets its exploration", unexplored);
    test_desert_travel();
    test_desert_rosters();
    test_desert_combat();
    test_pharaoh_combat();
    test_pharaoh_reward();
    test_desert_lamp_quest();
}
