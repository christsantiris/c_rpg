#include "game.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "../game/actions.h"

static void spawn_enemy(Enemy *e, EnemyType type, int x, int y) {
    e->active  = 1;
    e->type    = type;
    e->x       = x;
    e->y       = y;
switch (type) {
        case ENEMY_SKELETON:
            strncpy(e->name, "Skeleton", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 10; e->hp = 10;
            e->attack = 3;  e->defense = 0;
            e->experience = 8;
            break;
        case ENEMY_GOBLIN:
            strncpy(e->name, "Goblin", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 15; e->hp = 15;
            e->attack = 4;  e->defense = 1;
            e->experience = 10;
            break;
        case ENEMY_ZOMBIE:
            strncpy(e->name, "Zombie", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 22; e->hp = 22;
            e->attack = 6;  e->defense = 1;
            e->experience = 14;
            e->move_timer = 0;
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
        case ENEMY_GIANT:
            strncpy(e->name, "Giant", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 60; e->hp = 60;
            e->attack = 14; e->defense = 6;
            e->experience = 50;
            break;
    }
}

void enemies_spawn(GameState *g) {
    g->enemy_count = 0;
    if (g->map.room_count == 0) return;

    int num_enemies = 3 + g->level;
    if (num_enemies > MAX_ENEMIES) num_enemies = MAX_ENEMIES;

    for (int i = 0; i < num_enemies; i++) {
        int room_idx = rand() % (g->map.room_count - 1) + 1;
        if (room_idx >= g->map.room_count) room_idx = 0;
        Room *room = &g->map.rooms[room_idx];
        if (room->w < 3 || room->h < 3) continue;
        int ex = room->x + 1 + rand() % (room->w - 2);
        int ey = room->y + 1 + rand() % (room->h - 2);
        if (!map_is_walkable(&g->map, ex, ey)) continue;

        EnemyType type;
        int roll = rand() % 100;
        int level = g->level;

        if (level <= 2) {
            type = roll < 60 ? ENEMY_SKELETON : ENEMY_GOBLIN;
        } else if (level <= 4) {
            if (roll < 30)      type = ENEMY_SKELETON;
            else if (roll < 70) type = ENEMY_GOBLIN;
            else                type = ENEMY_ZOMBIE;
        } else if (level <= 6) {
            if (roll < 20)      type = ENEMY_GOBLIN;
            else if (roll < 60) type = ENEMY_ZOMBIE;
            else                type = ENEMY_ORC;
        } else if (level <= 8) {
            if (roll < 20)      type = ENEMY_ZOMBIE;
            else if (roll < 60) type = ENEMY_ORC;
            else                type = ENEMY_TROLL;
        } else if (level <= 10) {
            if (roll < 20)      type = ENEMY_ORC;
            else if (roll < 60) type = ENEMY_TROLL;
            else                type = ENEMY_GIANT;
        } else {
            type = roll < 50 ? ENEMY_TROLL : ENEMY_GIANT;
        }

        spawn_enemy(&g->enemies[i], type, ex, ey);
        g->enemy_count++;
    }
}

void game_init(GameState *g) {
    srand((unsigned)time(NULL));
    g->level = 1;
    for (int i = 0; i < MAX_DEPTH; i++)
        g->level_cache[i].valid = 0;
    g->message_count = 0;
    g->level_cleared = 0;
    g->max_level_reached = 1;
    g->location = LOCATION_TOWN;
    int spawn_x, spawn_y;
    map_generate_town(&g->map, &spawn_x, &spawn_y);
    g->player.x = spawn_x;
    g->player.y = spawn_y;
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
    g->inventory_count  = 0;
    g->equipped_weapon  = -1;
    g->equipped_armor   = -1;
    g->gold             = 0;
    g->floor_item_count = 0;
    for (int i = 0; i < MAX_INVENTORY; i++)
        g->inventory[i].active = 0;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++)
        g->floor_items[i].active = 0;

    // Spawn enemies in random rooms
    enemies_spawn(g);
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
    if (g->level > g->max_level_reached)
        g->max_level_reached = g->level;
    g->level_cleared = 0;
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
    g->level_cleared = 0;

    if (g->level_cache[g->level - 1].valid) {
        g->map         = g->level_cache[g->level - 1].map;
        g->enemy_count = g->level_cache[g->level - 1].enemy_count;
        for (int i = 0; i < g->enemy_count; i++)
            g->enemies[i] = g->level_cache[g->level - 1].enemies[i];
    }

    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y;
}

void game_enter_dungeon(GameState *g) {
    g->location = LOCATION_DUNGEON;

    if (g->max_level_reached > 1 &&
        g->level_cache[g->max_level_reached - 1].valid) {
        g->level = g->max_level_reached;
        g->map         = g->level_cache[g->level - 1].map;
        g->enemy_count = g->level_cache[g->level - 1].enemy_count;
        for (int i = 0; i < g->enemy_count; i++)
            g->enemies[i] = g->level_cache[g->level - 1].enemies[i];
        g->level_cleared = 1;
        g->player.x = g->map.stairs_up_x;
        g->player.y = g->map.stairs_up_y;
    } else {
        g->level         = 1;
        g->level_cleared = 0;
        for (int i = 0; i < MAX_DEPTH; i++)
            g->level_cache[i].valid = 0;
        map_generate(&g->map, g->level);
        enemies_spawn(g);
        g->player.x = g->map.stairs_up_x;
        g->player.y = g->map.stairs_up_y;
    }
}

void player_gain_xp(GameState *g, int xp) {
    g->player.experience += xp;

    while (g->player.experience >= g->player.experience_next &&
           g->player.level < 50) {
        g->player.experience    -= g->player.experience_next;
        g->player.level++;
        g->player.max_hp        += 10;
        g->player.hp             = g->player.max_hp;
        g->player.attack        += 2;

        // Defense grows at half rate, capped at 50% of attack
        int new_defense = g->player.defense + 1;
        int defense_cap = g->player.attack / 2;
        g->player.defense = new_defense > defense_cap ? defense_cap : new_defense;

        g->player.experience_next = g->player.level * 100;

        char msg[MAX_MESSAGE_LEN];
        snprintf(msg, sizeof(msg), "Level up! Now level %d", g->player.level);
        push_message(g, msg);
    }
}