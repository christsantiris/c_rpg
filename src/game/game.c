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
        case ENEMY_GOBLIN_KING:
            strncpy(e->name, "Goblin King", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 100; e->hp = 100;
            e->attack = 15;  e->defense = 5;
            e->experience = 200;
            e->is_boss = 1;
            break;
        case ENEMY_LICH_KING:
            strncpy(e->name, "Lich King", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 200; e->hp = 200;
            e->attack = 25;  e->defense = 10;
            e->experience = 400;
            e->is_boss = 1;
            break;
        case ENEMY_DEMON_LORD:
            strncpy(e->name, "Demon Lord", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 350; e->hp = 350;
            e->attack = 38;  e->defense = 15;
            e->experience = 700;
            e->is_boss = 1;
            break;
        case ENEMY_RED_DRAGON:
            strncpy(e->name, "Red Dragon", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 500; e->hp = 500;
            e->attack = 55;  e->defense = 22;
            e->experience = 1200;
            e->is_boss = 1;
            break;
        case ENEMY_TARRASQUE:
            strncpy(e->name, "Tarrasque", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 800; e->hp = 800;
            e->attack = 80;  e->defense = 35;
            e->experience = 2000;
            e->is_boss = 1;
            break;
    }
}

static void spawn_boss(GameState *g) {
    // Find largest room
    int largest = 0;
    for (int i = 1; i < g->map.room_count; i++) {
        int area = g->map.rooms[i].w * g->map.rooms[i].h;
        int best = g->map.rooms[largest].w * g->map.rooms[largest].h;
        if (area > best) largest = i;
    }

    Room *room = &g->map.rooms[largest];
    int bx = room->x + room->w / 2;
    int by = room->y + room->h / 2;

    EnemyType type;
    switch (g->level) {
        case 5:  type = ENEMY_GOBLIN_KING; break;
        case 10: type = ENEMY_LICH_KING;   break;
        case 15: type = ENEMY_DEMON_LORD;  break;
        case 20: type = ENEMY_RED_DRAGON;  break;
        case 25: type = ENEMY_TARRASQUE;   break;
        default: return;
    }

    int idx = g->enemy_count;
    if (idx >= MAX_ENEMIES) return;
    spawn_enemy(&g->enemies[idx], type, bx, by);
    g->enemy_count++;
}

void enemies_spawn(GameState *g) {
    g->enemy_count = 0;
    if (g->map.room_count == 0) return;

    int num_enemies = 5 + g->level;
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
    // Spawn boss on boss levels
    if (g->level == 5  || g->level == 10 || g->level == 15 ||
        g->level == 20 || g->level == 25) {
        spawn_boss(g);
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

    g->player.known_spell_count = 0;
    g->player.equipped_spell    = -1;
    g->player.last_dx           = 0;
    g->player.last_dy           = 0;
    g->trail_count              = 0;
    g->trail_frames             = 0;

    #ifdef DEBUG
    g->inventory[g->inventory_count++] = item_make_scroll_magic_arrow();
    g->inventory[g->inventory_count++] = item_make_bow();
    #endif

    // Starting weapon
    g->inventory[g->inventory_count++] = item_make_rusty_sword();

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
        g->level_cache[g->level - 1].level_cleared = g->level_cleared;
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
        g->level_cleared = g->level_cache[g->level - 1].level_cleared;
    } else {
        g->level_cleared = 0;
        map_generate(&g->map, g->level);
        enemies_spawn(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
}

void game_ascend(GameState *g) {
    if (g->level <= 1) return;

    if (g->level <= MAX_DEPTH) {
        g->level_cache[g->level - 1].map           = g->map;
        g->level_cache[g->level - 1].enemy_count   = g->enemy_count;
        g->level_cache[g->level - 1].level_cleared = g->level_cleared;
        for (int i = 0; i < g->enemy_count; i++)
            g->level_cache[g->level - 1].enemies[i] = g->enemies[i];
        g->level_cache[g->level - 1].valid = 1;
    }

    g->level--;

    if (g->level_cache[g->level - 1].valid) {
        g->map         = g->level_cache[g->level - 1].map;
        g->enemy_count = g->level_cache[g->level - 1].enemy_count;
        g->level_cleared = g->level_cache[g->level - 1].level_cleared;
        for (int i = 0; i < g->enemy_count; i++)
            g->enemies[i] = g->level_cache[g->level - 1].enemies[i];
    } else {
        g->level_cleared = 0;
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