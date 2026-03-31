#include "game.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

static void spawn_enemy(Enemy *e, EnemyType type, int x, int y) {
    e->active  = 1;
    e->type    = type;
    e->x       = x;
    e->y       = y;
    switch (type) {
        case ENEMY_GOBLIN:
            strncpy(e->name, "Goblin", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 15; e->hp = 15;
            e->attack = 4;  e->defense = 1;
            e->experience = 10;
            break;
        case ENEMY_SKELETON:
            strncpy(e->name, "Skeleton", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 20; e->hp = 20;
            e->attack = 6;  e->defense = 0;
            e->experience = 15;
            break;
        case ENEMY_ORC:
            strncpy(e->name, "Orc", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 25; e->hp = 25;
            e->attack = 7;  e->defense = 2;
            e->experience = 20;
            break;
        case ENEMY_TROLL:
            strncpy(e->name, "Troll", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 40; e->hp = 40;
            e->attack = 10; e->defense = 4;
            e->experience = 30;
            break;
    }
}

static void enemies_spawn(GameState *g) {
    g->enemy_count = 0;
    int num_enemies = 3 + g->level;
    if (num_enemies > MAX_ENEMIES) num_enemies = MAX_ENEMIES;

    for (int i = 0; i < num_enemies; i++) {
        int room_idx = rand() % (g->map.room_count - 1) + 1;
        Room *room   = &g->map.rooms[room_idx];
        int ex = room->x + rand() % room->w;
        int ey = room->y + rand() % room->h;
        EnemyType type = (EnemyType)(rand() % 4);
        spawn_enemy(&g->enemies[i], type, ex, ey);
        g->enemy_count++;
    }
}

void game_init(GameState *g) {
    srand((unsigned)time(NULL));
    g->level = 1;
    for (int i = 0; i < MAX_DEPTH; i++)
        g->level_cache[i].valid = 0;
    map_generate(&g->map, g->level);
    g->player.name[0]       = '\0';
    g->player.hp            = 100;
    g->player.max_hp        = 100;
    g->player.mp            = 50;
    g->player.max_mp        = 50;
    g->player.attack        = 10;
    g->player.defense       = 5;
    g->player.level         = 1;
    g->player.experience    = 0;
    g->player.experience_next = 100;

    // Spawn enemies in random rooms
    enemies_spawn(g);
    // g->enemy_count = 0;
    // int num_enemies = 3 + g->level;
    // if (num_enemies > MAX_ENEMIES) num_enemies = MAX_ENEMIES;

    // for (int i = 0; i < num_enemies; i++) {
    //     int room_idx = rand() % (g->map.room_count - 1) + 1;
    //     Room *room   = &g->map.rooms[room_idx];
    //     int ex = room->x + rand() % room->w;
    //     int ey = room->y + rand() % room->h;
    //     EnemyType type = (EnemyType)(rand() % 4);
    //     spawn_enemy(&g->enemies[i], type, ex, ey);
    //     g->enemy_count++;
    // }

    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
}

void game_move_player(GameState *g, int dx, int dy) {
    int nx = g->player.x + dx;
    int ny = g->player.y + dy;
    if (!map_is_walkable(&g->map, nx, ny)) return;
    g->player.x = nx;
    g->player.y = ny;
}

void game_descend(GameState *g) {
    if (g->level >= 1 && g->level <= MAX_DEPTH) {
        g->level_cache[g->level - 1].map         = g->map;
        g->level_cache[g->level - 1].enemy_count = g->enemy_count;
        for (int i = 0; i < g->enemy_count; i++)
            g->level_cache[g->level - 1].enemies[i] = g->enemies[i];
        g->level_cache[g->level - 1].valid = 1;
    }

    g->level++;
    if (g->level <= MAX_DEPTH && g->level_cache[g->level - 1].valid) {
        g->map         = g->level_cache[g->level - 1].map;
        g->enemy_count = g->level_cache[g->level - 1].enemy_count;
        for (int i = 0; i < g->enemy_count; i++)
            g->enemies[i] = g->level_cache[g->level - 1].enemies[i];
    } else {
        map_generate(&g->map, g->level);
        enemies_spawn(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
}

void game_ascend(GameState *g) {
    if (g->level <= 1) return;

    if (g->level <= MAX_DEPTH) {
        g->level_cache[g->level - 1].map         = g->map;
        g->level_cache[g->level - 1].enemy_count = g->enemy_count;
        for (int i = 0; i < g->enemy_count; i++)
            g->level_cache[g->level - 1].enemies[i] = g->enemies[i];
        g->level_cache[g->level - 1].valid = 1;
    }

    g->level--;

    if (g->level_cache[g->level - 1].valid) {
        g->map         = g->level_cache[g->level - 1].map;
        g->enemy_count = g->level_cache[g->level - 1].enemy_count;
        for (int i = 0; i < g->enemy_count; i++)
            g->enemies[i] = g->level_cache[g->level - 1].enemies[i];
    }

    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y;
}