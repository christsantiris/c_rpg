#include "test_utils.h"
#include "game/game.h"
#include "game/aldric.h"
#include "game/enemy_tactics.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

static GameState g;
static GameState loaded;

static void arena(Location area) {
    memset(&g, 0, sizeof(g));
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    g.location = area;
    g.level = 1;
    g.enemy_count = 0;
    g.player.x = 20;
    g.player.y = 20;
    g.player.hp = 10000;
    g.player.max_hp = 10000;
    g.player.defense = 0;
    g.map.room_count = 1;
    g.map.rooms[0] = (Room){4, 4, 34, 34};
    g.map.stairs_up_x = 5;
    g.map.stairs_up_y = 5;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g.map.tiles[y][x] = x >= 4 && x < 38 && y >= 4 && y < 38 ? TILE_FLOOR : TILE_WALL;
        }
    }
}

static void actor(EnemyType type, int x, int y) {
    Enemy *e = &g.enemies[g.enemy_count++];
    game_spawn_enemy(&g, e, type, x, y);
    e->hp = 999;
    e->max_hp = 1000;
    e->attack = 10;
    e->defense = 0;
}

static int actors_valid(void) {
    for (int i = 0; i < g.enemy_count; i++) {
        if (!g.enemies[i].active) {
            continue;
        }
        if (!map_is_walkable(&g.map, g.enemies[i].x, g.enemies[i].y) ||
            (g.enemies[i].x == g.player.x && g.enemies[i].y == g.player.y) ||
            aldric_at(&g, g.enemies[i].x, g.enemies[i].y)) {
            return 0;
        }
        for (int j = 0; j < i; j++) {
            if (g.enemies[j].active && g.enemies[i].x == g.enemies[j].x && g.enemies[i].y == g.enemies[j].y) {
                return 0;
            }
        }
    }
    return 1;
}

static void generated_encounters(void) {
    static TileType terrain[MAP_H][MAP_W];
    int valid = 1;
    int unchanged = 1;
    int cadence = 1;
    for (int area = 0; area < 2; area++) {
        for (int stage = 1; stage <= 5; stage++) {
            for (int seed = 9100; seed < 9104; seed++) {
                arena(area ? LOCATION_MOUNTAINS : LOCATION_FOREST);
                srand(seed);
                g.level = stage;
                g.player.level = 5;
                g.player.hp = 100000;
                if (area) {
                    map_generate_mountains(&g.map, stage);
                } else {
                    map_generate_forest(&g.map, stage);
                }
                enemies_spawn(&g);
                // Exercise regular squads without altering either boss encounter.
                for (int i = 0; i < g.enemy_count; i++) {
                    if (g.enemies[i].is_boss) {
                        g.enemies[i].frozen_turns = 1000;
                    }
                }
                g.player.x = g.map.stairs_up_x;
                g.player.y = g.map.stairs_up_y;
                for (int i = 0; i < g.enemy_count; i++) {
                    int x = g.enemies[i].x - 1;
                    int y = g.enemies[i].y;
                    if (!g.enemies[i].is_boss && map_is_walkable(&g.map, x, y)) {
                        g.player.x = x;
                        g.player.y = y;
                        if (actors_valid()) {
                            break;
                        }
                    }
                }
                memcpy(terrain, g.map.tiles, sizeof(terrain));
                for (int turn = 0; turn < 24; turn++) {
                    Enemy before[MAX_ENEMIES];
                    memcpy(before, g.enemies, sizeof(before));
                    action_resolve_enemies(&g);
                    valid &= actors_valid();
                    for (int i = 0; i < g.enemy_count; i++) {
                        const Enemy *e = &g.enemies[i];
                        int squad = enemy_goblin_frontline(e) || e->type == ENEMY_GOBLIN_ARCHER || e->type == ENEMY_GOBLIN_SHAMAN ||
                            e->type == ENEMY_DARK_ELF_GUARD || e->type == ENEMY_DARK_ELF_ARCHER || e->type == ENEMY_DARK_ELF_RANGER;
                        if (squad && e->active) {
                            cadence &= abs(e->x - before[i].x) + abs(e->y - before[i].y) <= 1;
                            unchanged &= e->attack == before[i].attack && e->defense == before[i].defense && e->max_hp == before[i].max_hp;
                        }
                    }
                }
                unchanged &= memcmp(terrain, g.map.tiles, sizeof(terrain)) == 0;
            }
        }
    }
    ASSERT("40 generated forest and mountain encounters keep actors on terrain without overlaps", valid);
    ASSERT("squad turns preserve generated terrain, transitions and spawned combat stats", unchanged);
    ASSERT("generated squad members never gain extra movement steps", cadence);
}

void test_act2_tactics(void) {
    printf("Act 2 squads, flanking and companion combat regressions:\n");
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    int rotated = 1;
    for (int side = 0; side < 4; side++) {
        arena(LOCATION_MOUNTAINS);
        actor(ENEMY_HOBGOBLIN_GUARD, 20 + dx[side] * 4, 20 + dy[side] * 4);
        actor(ENEMY_GOBLIN_ARCHER, 20 + dx[side] * 5, 20 + dy[side] * 5);
        actor(ENEMY_GOBLIN_SHAMAN, 20 + dx[side] * 6 - dy[side], 20 + dy[side] * 6 + dx[side]);
        for (int turn = 0; turn < 16; turn++) {
            action_resolve_enemies(&g);
            rotated &= actors_valid();
        }
        int front = (g.enemies[0].x - 20) * dx[side] + (g.enemies[0].y - 20) * dy[side];
        int rear = (g.enemies[1].x - 20) * dx[side] + (g.enemies[1].y - 20) * dy[side];
        rotated &= front == 1 && rear >= 3;
    }
    ASSERT("goblin formations work when approaching from all four directions", rotated);

    arena(LOCATION_MOUNTAINS);
    actor(ENEMY_HOBGOBLIN_GUARD, 16, 20);
    actor(ENEMY_GOBLIN_ARCHER, 15, 20);
    actor(ENEMY_GOBLIN_SHAMAN, 14, 21);
    int valid = 1;
    int arrows = 0;
    EnemyProjectiles shots = {0};
    for (int turn = 0; turn < 12; turn++) {
        action_resolve_enemies_with_projectiles(&g, &shots);
        valid &= actors_valid();
        arrows += shots.count;
    }
    ASSERT("goblins form a melee screen with archer and shaman behind it", g.enemies[0].x == 19 && g.enemies[0].y != 20 && g.enemies[1].x < g.enemies[0].x && g.enemies[2].x < g.enemies[0].x);
    ASSERT("the goblin screen leaves a functioning firing lane and never overlaps actors", arrows > 0 && valid);
    action_resolve_player(&g, (Action){ACTION_MOVE, 21, 20});
    for (int turn = 0; turn < 12; turn++) {
        action_resolve_enemies(&g);
        valid &= actors_valid();
    }
    ASSERT("the goblin frontline follows a moving player instead of holding a stale position", g.enemies[0].x == 20 && g.enemies[0].y != 20 && valid);

    arena(LOCATION_MOUNTAINS);
    for (int y = 4; y < 38; y++) {
        for (int x = 4; x < 38; x++) {
            g.map.tiles[y][x] = y == 20 ? TILE_FLOOR : TILE_WALL;
        }
    }
    actor(ENEMY_HOBGOBLIN_GUARD, 16, 20);
    actor(ENEMY_GOBLIN_ARCHER, 15, 20);
    actor(ENEMY_GOBLIN_SHAMAN, 14, 20);
    for (int turn = 0; turn < 8; turn++) {
        action_resolve_enemies(&g);
        valid &= actors_valid();
    }
    ASSERT("a narrow pass breaks formation and the frontline still engages", g.enemies[0].x == 19 && g.player.hp < 10000 && valid);

    arena(LOCATION_FOREST);
    actor(ENEMY_DARK_ELF_GUARD, 14, 20);
    actor(ENEMY_DARK_ELF_ARCHER, 13, 21);
    actor(ENEMY_DARK_ELF_RANGER, 13, 19);
    int archer_shots = 0;
    int ranger_shots = 0;
    int bow_cadence = 1;
    for (int turn = 0; turn < 24; turn++) {
        action_resolve_enemies_with_projectiles(&g, &shots);
        valid &= actors_valid();
        for (int i = 0; i < shots.count; i++) {
            archer_shots += shots.shots[i].type == ENEMY_DARK_ELF_ARCHER;
            ranger_shots += shots.shots[i].type == ENEMY_DARK_ELF_RANGER;
            if (shots.shots[i].type == ENEMY_DARK_ELF_ARCHER) {
                bow_cadence &= g.enemies[1].move_timer % 2 == 0;
            } else if (shots.shots[i].type == ENEMY_DARK_ELF_RANGER) {
                bow_cadence &= g.enemies[2].move_timer % 3 == 0;
            }
        }
    }
    ASSERT("Dark Elf bow units establish opposing flanks while their guard engages", g.enemies[1].y < g.player.y && g.enemies[2].y > g.player.y && abs(g.enemies[0].x - g.player.x) <= 1);
    ASSERT("both elf bow roles keep their shot cadences after flanking without collisions", archer_shots > 0 && ranger_shots > 0 && bow_cadence && valid);
    ASSERT("squad save slot is unused", !save_exists(99242));
    ASSERT("a flanking encounter can be saved and loaded", save_game(&g, 99242) && load_game(&loaded, 99242));
    srand(1234);
    action_resolve_enemies(&g);
    srand(1234);
    action_resolve_enemies(&loaded);
    int same = 1;
    for (int i = 0; i < g.enemy_count; i++) {
        same &= g.enemies[i].x == loaded.enemies[i].x && g.enemies[i].y == loaded.enemies[i].y && g.enemies[i].move_timer == loaded.enemies[i].move_timer;
    }
    ASSERT("save/load retains the next tactical moves and attack cadence", same && g.player.hp == loaded.player.hp);
    remove("saves/savegame_99242.json");

    arena(LOCATION_FOREST);
    actor(ENEMY_DARK_ELF_GUARD, 14, 20);
    actor(ENEMY_DARK_ELF_ARCHER, 13, 21);
    actor(ENEMY_DARK_ELF_RANGER, 13, 19);
    for (int y = 17; y <= 23; y++) {
        g.map.tiles[y][17] = TILE_FOREST_WALL;
    }
    for (int turn = 0; turn < 30; turn++) {
        action_resolve_enemies(&g);
        valid &= actors_valid();
    }
    ASSERT("elves route around a tree barrier to the player's side rather than crossing it", g.enemies[0].x > 17 && g.enemies[1].x > 17 && g.enemies[2].x > 17 && valid);

    arena(LOCATION_DUNGEON);
    actor(ENEMY_GOBLIN_ARCHER, 14, 20);
    g.enemies[0].move_timer = 1;
    g.aldric_party_state = ALDRIC_FOLLOWING;
    g.aldric_x = 14;
    g.aldric_y = 19;
    g.aldric_hp = ALDRIC_MAX_HP;
    action_resolve_enemies_with_projectiles(&g, &shots);
    ASSERT("an archer beside Aldric retains its scheduled clear shot instead of losing its ranged role", shots.count == 1 && g.player.hp == 9990 && g.aldric_hp == ALDRIC_MAX_HP);

    arena(LOCATION_DUNGEON);
    actor(ENEMY_GOBLIN_SCOUT, 14, 18);
    actor(ENEMY_GOBLIN_SHAMAN, 14, 20);
    g.enemies[0].hp = 10;
    g.enemies[1].move_timer = 2;
    g.aldric_party_state = ALDRIC_FOLLOWING;
    g.aldric_x = 15;
    g.aldric_y = 20;
    g.aldric_hp = ALDRIC_MAX_HP;
    action_resolve_enemies(&g);
    ASSERT("a shaman beside Aldric still performs its scheduled ally heal", g.enemies[0].hp == 16 && g.aldric_hp == ALDRIC_MAX_HP);

    arena(LOCATION_DUNGEON);
    actor(ENEMY_GOBLIN_ARCHER, 14, 20);
    g.enemies[0].move_timer = 1;
    g.aldric_party_state = ALDRIC_FOLLOWING;
    g.aldric_x = 15;
    g.aldric_y = 20;
    g.aldric_hp = ALDRIC_MAX_HP;
    action_resolve_enemies_with_projectiles(&g, &shots);
    ASSERT("enemy arrows cannot pass through Aldric to hit the player", shots.count == 0 && g.player.hp == 10000);

    arena(LOCATION_FOREST);
    actor(ENEMY_DARK_ELF_GUARD, 14, 20);
    ASSERT("an isolated elf uses its ordinary combat movement", enemy_tactics_move(&g, 0) == -1);
    actor(ENEMY_DARK_ELF_ARCHER, 13, 21);
    g.location = LOCATION_DUNGEON;
    ASSERT("forest tactics do not change elves in other regions", enemy_tactics_move(&g, 0) == -1);
    arena(LOCATION_DUNGEON);
    actor(ENEMY_HOBGOBLIN_GUARD, 16, 20);
    actor(ENEMY_GOBLIN_ARCHER, 15, 20);
    ASSERT("mountain formations do not change goblins in other regions", enemy_tactics_move(&g, 0) == -1);
    generated_encounters();
}
