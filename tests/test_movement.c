#include "test_utils.h"
#include "../src/game/game.h"

static void setup_ranged_enemy(GameState *g, EnemyType type) {
    g->player.player_class = CLASS_WARRIOR;
    game_init(g);
    g->location = LOCATION_FOREST;
    g->map.room_count = 1;
    g->map.rooms[0] = (Room){10, 10, 20, 20};
    for (int y = 10; y < 30; y++) {
        for (int x = 10; x < 30; x++) {
            g->map.tiles[y][x] = TILE_FOREST_FLOOR;
        }
    }
    g->player.x = 20;
    g->player.y = 20;
    g->player.hp = 100;
    g->player.defense = 0;
    g->equipped_armor = -1;
    g->equipped_off_hand = -1;
    g->enemy_count = 1;
    g->enemies[0] = (Enemy){
        .type = type, .x = 14, .y = 20, .active = 1,
        .hp = 100, .max_hp = 100, .attack = 10,
        .move_timer = type == ENEMY_DROWNED_QUEEN ? 2 : 1
    };
}

void test_enemy_projectiles(void) {
    printf("Enemy projectile tests:\n");
    static GameState g;
    EnemyProjectiles shots = {0};
    EnemyType ranged[] = {
        ENEMY_LICH_KING, ENEMY_FOREST_NECROMANCER, ENEMY_MOUNTAIN_GOBLIN_KING,
        ENEMY_DROWNED_QUEEN, ENEMY_SIREN, ENEMY_WATER_ELEMENTAL,
        ENEMY_GOBLIN_ARCHER, ENEMY_GOBLIN_BOMBER, ENEMY_DARK_ELF, ENEMY_CRYPT_CONJURER
    };
    for (int i = 0; i < (int)(sizeof(ranged) / sizeof(ranged[0])); i++) {
        setup_ranged_enemy(&g, ranged[i]);
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("every ranged enemy emits its own projectile toward the player",
            shots.count == 1 && shots.shots[0].type == ranged[i] &&
            shots.shots[0].start_x == 14 && shots.shots[0].start_y == 20 &&
            shots.shots[0].target_x == 20 && shots.shots[0].target_y == 20);
        ASSERT("projectile attacks retain their existing damage", g.player.hp == 90);
    }

    setup_ranged_enemy(&g, ENEMY_GOBLIN_ARCHER);
    g.enemies[1] = g.enemies[0];
    g.enemies[1].type = ENEMY_DARK_ELF;
    g.enemies[1].x = 20;
    g.enemies[1].y = 14;
    g.enemy_count = 2;
    g.trail_count = 1;
    g.trail_frames = 4;
    g.trail_effect = TRAIL_EFFECT_WEAPON_ARROW;
    action_resolve_enemies_with_projectiles(&g, &shots);
    ASSERT("two shooters retain both projectile paths in the same turn",
        shots.count == 2 && shots.shots[0].start_x == 14 &&
        shots.shots[1].start_y == 14 && g.player.hp == 80);
    ASSERT("enemy projectiles do not overwrite the player's projectile trail",
        g.trail_count == 1 && g.trail_frames == 4 && g.trail_effect == TRAIL_EFFECT_WEAPON_ARROW);

    setup_ranged_enemy(&g, ENEMY_GOBLIN_ARCHER);
    g.map.tiles[20][17] = TILE_FOREST_WALL;
    action_resolve_enemies_with_projectiles(&g, &shots);
    ASSERT("blocked shots create no projectile and clear the previous batch",
        shots.count == 0 && g.player.hp == 100);

    setup_ranged_enemy(&g, ENEMY_GOBLIN_ARCHER);
    g.enemies[0].move_timer = 0;
    action_resolve_enemies_with_projectiles(&g, &shots);
    ASSERT("ranged enemy holds a clear firing lane during recovery",
        g.enemies[0].x == 14 && g.enemies[0].y == 20 &&
        g.player.hp == 100);
    action_resolve_enemies_with_projectiles(&g, &shots);
    ASSERT("ranged enemy attacks without abandoning its firing lane",
        g.enemies[0].x == 14 && g.enemies[0].y == 20 &&
        g.player.hp == 90 && shots.count == 1);

    setup_ranged_enemy(&g, ENEMY_GOBLIN_ARCHER);
    g.enemies[0].x = 19;
    g.enemies[0].move_timer = 0;
    action_resolve_enemies_with_projectiles(&g, &shots);
    ASSERT("ranged enemy retreats instead of entering melee",
        g.enemies[0].x == 18 && g.enemies[0].y == 20 &&
        g.player.hp == 100 && shots.count == 0);

    setup_ranged_enemy(&g, ENEMY_GOBLIN_ARCHER);
    g.enemies[0].y = 19;
    g.enemies[0].move_timer = 0;
    action_resolve_enemies_with_projectiles(&g, &shots);
    ASSERT("ranged enemy steps into an available firing lane",
        g.enemies[0].x == 14 && g.enemies[0].y == 20);

    setup_ranged_enemy(&g, ENEMY_FOREST_NECROMANCER);
    g.enemies[0].move_timer = 0;
    action_resolve_enemies_with_projectiles(&g, &shots);
    ASSERT("boss warning turns do not create a projectile", shots.count == 0);
    g.enemies[0].x = 19;
    action_resolve_enemies_with_projectiles(&g, &shots);
    ASSERT("melee attacks do not create a projectile", shots.count == 0 && g.player.hp == 90);

    setup_ranged_enemy(&g, ENEMY_GOBLIN_ARCHER);
    g.inventory[0] = item_make_shadow_armor();
    g.inventory[0].evasion_chance = 100;
    g.equipped_armor = 0;
    action_resolve_enemies_with_projectiles(&g, &shots);
    ASSERT("dodged ranged attacks still have a visible projectile",
        shots.count == 1 && g.player.hp == 100);

    setup_ranged_enemy(&g, ENEMY_FOREST_NECROMANCER);
    g.enemies[0].y = 14;
    action_resolve_enemies_with_projectiles(&g, &shots);
    ASSERT("boss shots preserve diagonal paths",
        shots.count == 1 && shots.shots[0].start_x == 14 &&
        shots.shots[0].start_y == 14 && shots.shots[0].target_x == 20 &&
        shots.shots[0].target_y == 20);

    EnemyType retreating[] = {
        ENEMY_CRYPT_CONJURER, ENEMY_DARK_ELF, ENEMY_GOBLIN_ARCHER,
        ENEMY_GOBLIN_BOMBER, ENEMY_SIREN, ENEMY_WATER_ELEMENTAL,
        ENEMY_BLOWDART_HUNTER, ENEMY_SUN_PRIEST, ENEMY_SERPENT_SPIRIT,
        ENEMY_MOONBOUND_SENTINEL, ENEMY_GOBLIN_SHAMAN
    };
    for (int i = 0; i < (int)(sizeof(retreating) / sizeof(retreating[0])); i++) {
        for (int corridor = 0; corridor < 2; corridor++) {
            for (int timer = 0; timer < 2; timer++) {
                setup_ranged_enemy(&g, retreating[i]);
                if (corridor) {
                    for (int x = 10; x < 30; x++) {
                        g.map.tiles[19][x] = TILE_FOREST_WALL;
                        g.map.tiles[21][x] = TILE_FOREST_WALL;
                    }
                }
                g.player.attack = 50;
                g.enemies[0].x = 17;
                g.enemies[0].move_timer = timer;
                for (int turn = 0; turn < 8 && g.enemies[0].active; turn++) {
                    int dx = (g.enemies[0].x > g.player.x) -
                        (g.enemies[0].x < g.player.x);
                    int dy = (g.enemies[0].y > g.player.y) -
                        (g.enemies[0].y < g.player.y);
                    action_resolve_player(&g, (Action){ACTION_MOVE,
                        g.player.x + dx, g.player.y + dy});
                    if (g.enemies[0].active) {
                        action_resolve_enemies_with_projectiles(&g, &shots);
                    }
                }
                ASSERT("a warrior can catch and kill a retreating enemy before cornering it",
                    !g.enemies[0].active && g.enemies[0].x > 10 &&
                    g.player.hp > 0);
            }
        }
    }
}

void test_movement(void) {
    printf("Movement tests:\n");

    GameState g;
    game_init(&g);
    // Find a walkable position with room to move in all directions
    int start_x = -1, start_y = -1;
    for (int y = 2; y < MAP_H - 2 && start_x == -1; y++) {
        for (int x = 2; x < MAP_W - 2 && start_x == -1; x++) {
            if (map_is_walkable(&g.map, x, y) &&
                map_is_walkable(&g.map, x+1, y) &&
                map_is_walkable(&g.map, x-1, y) &&
                map_is_walkable(&g.map, x, y+1) &&
                map_is_walkable(&g.map, x, y-1)) {
                start_x = x;
                start_y = y;
            }
        }
    }
    ASSERT("found valid test position", start_x != -1);
    g.player.x = start_x;
    g.player.y = start_y;

    // Basic movement
    game_move_player(&g, 1, 0);
    ASSERT("move right increases x by 1", g.player.x == start_x + 1);

    game_move_player(&g, -1, 0);
    ASSERT("move left decreases x by 1", g.player.x == start_x);

    game_move_player(&g, 0, 1);
    ASSERT("move down increases y by 1", g.player.y == start_y + 1);

    game_move_player(&g, 0, -1);
    ASSERT("move up decreases y by 1", g.player.y == start_y);

    // Wall collision - left border
    game_init(&g);
    g.player.x = 1;
    g.player.y = MAP_H / 2;
    game_move_player(&g, -1, 0);
    ASSERT("player cannot move into left border", g.player.x == 1);

    // Wall collision - right border
    game_init(&g);
    printf("DEBUG after game_init: equipped_armor=%d defense=%d\n",
        g.equipped_armor, g.player.defense);
    g.player.x = MAP_W - 2;
    g.player.y = MAP_H / 2;
    game_move_player(&g, 1, 0);
    ASSERT("player cannot move into right border", g.player.x == MAP_W - 2);

    // Wall collision - top border
    game_init(&g);
    g.player.x = MAP_W / 2;
    g.player.y = 1;
    game_move_player(&g, 0, -1);
    ASSERT("player cannot move into top border", g.player.y == 1);

    // Wall collision - bottom border
    game_init(&g);
    g.player.x = MAP_W / 2;
    g.player.y = MAP_H - 2;
    game_move_player(&g, 0, 1);
    ASSERT("player cannot move into bottom border", g.player.y == MAP_H - 2);
}
