#include "test_utils.h"
#include "../src/game/map.h"
#include "../src/game/game.h"
#include "../src/game/actions.h"
#include <stdlib.h>
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
    g.level = MAX_DEPTH;
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

    for (int level = 1; level < MAX_DEPTH; level++) {
        g.level = level;
        map_generate(&g.map, level);
        enemies_spawn(&g);
        int bosses = 0;
        for (int j = 0; j < g.enemy_count; j++)
            if (g.enemies[j].is_boss) bosses++;
        ASSERT("early dungeon floor has no boss", bosses == 0);
    }
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
    ASSERT("return spell leaves a town portal", g.map.tiles[2][20] == TILE_PORTAL);
    ASSERT("return portal remains active", g.portal_active == 1);
    ASSERT("dungeon end of portal remains in cached floor",
        g.level_cache[0].map.tiles[origin_y][origin_x] == TILE_PORTAL);

    Action enter = {ACTION_MOVE, 20, 2};
    action_resolve_player(&g, enter);
    ASSERT("town portal returns to dungeon", g.location == LOCATION_DUNGEON);
    ASSERT("portal returns to casting position",
        g.player.x == origin_x && g.player.y == origin_y);
    ASSERT("dungeon portal closes behind player",
        g.map.tiles[origin_y][origin_x] != TILE_PORTAL);
    ASSERT("portal closes after return trip", g.portal_active == 0);
}

void test_final_dungeon_exit(void) {
    printf("Final dungeon exit tests:\n");
    GameState g;
    game_init(&g);
    g.location = LOCATION_DUNGEON;
    g.level = MAX_DEPTH;
    g.max_level_reached = MAX_DEPTH;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);

    game_mark_level_cleared(&g);
    ASSERT("final stairs become a return exit",
        g.map.tiles[g.map.stairs_down_y][g.map.stairs_down_x] == TILE_RETURN_EXIT);

    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    Action a = {ACTION_DESCEND, 0, 0};
    action_resolve_player(&g, a);
    ASSERT("final exit returns player to town", g.location == LOCATION_TOWN);
    ASSERT("final exit returns at north town road", g.player.x == 20 && g.player.y == 1);
    ASSERT("final exit does not create a sixth floor", g.level == MAX_DEPTH);
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
    ASSERT("crypt bat moves two tiles", g.enemies[0].x == 6);

    g.enemies[0] = (Enemy){
        .x = 8, .y = 8, .active = 1, .type = ENEMY_WRAITH,
        .attack = 7
    };
    g.map.tiles[9][9] = TILE_WALL;
    action_resolve_enemies(&g);
    ASSERT("wraith cannot pass through a wall",
        g.enemies[0].x == 8 && g.enemies[0].y == 8);

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
    printf("Stairs lock tests:\n");

    GameState g;
    game_init(&g);
    game_descend(&g);

    ASSERT("level not cleared on start", g.level_cleared == 0);

    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    int level_before = g.level;

    Action a = {ACTION_DESCEND, 0, 0};
    action_resolve_player(&g, a);
    ASSERT("cannot descend when level not cleared", g.level == level_before);

    for (int i = 0; i < g.enemy_count; i++)
        g.enemies[i].active = 0;
    g.level_cleared = 1;

    action_resolve_player(&g, a);
    ASSERT("can descend when level cleared", g.level == level_before + 1);
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
