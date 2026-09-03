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
    e->is_boss = 0;
    e->move_timer = 0;
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
        case ENEMY_CRYPT_BAT:
            strncpy(e->name, "Crypt Bat", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 7; e->hp = 7;
            e->attack = 4; e->defense = 0;
            e->experience = 10;
            break;
        case ENEMY_WRAITH:
            strncpy(e->name, "Wraith", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 18; e->hp = 18;
            e->attack = 7; e->defense = 2;
            e->experience = 22;
            break;
        case ENEMY_NECROMANCER:
            strncpy(e->name, "Necromancer", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 24; e->hp = 24;
            e->attack = 7; e->defense = 1;
            e->experience = 35;
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
            e->max_hp = 140; e->hp = 140;
            e->attack = 18;  e->defense = 6;
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

static int boss_for_level(int level, EnemyType *type) {
    switch (level) {
        case MAX_DEPTH: *type = ENEMY_LICH_KING; return 1;
        default: return 0;
    }
}

static int enemy_tile_open(const GameState *g, int x, int y) {
    // Keep keys, stairs, traps, and portals visible and unobstructed.
    if (!map_is_walkable(&g->map, x, y) ||
        g->map.tiles[y][x] != TILE_FLOOR) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active &&
            g->enemies[i].x == x && g->enemies[i].y == y) {
            return 0;
        }
    }
    return 1;
}

static int find_enemy_tile(GameState *g, int *x, int *y, int room_limit) {
    for (int attempt = 0; attempt < 100; attempt++) {
        int room_idx = rand() % room_limit;
        Room *room = &g->map.rooms[room_idx];
        if (room->w < 3 || room->h < 3) {
            continue;
        }
        int tx = room->x + 1 + rand() % (room->w - 2);
        int ty = room->y + 1 + rand() % (room->h - 2);
        if (enemy_tile_open(g, tx, ty)) {
            *x = tx;
            *y = ty;
            return 1;
        }
    }

    for (int i = 0; i < room_limit; i++) {
        Room *room = &g->map.rooms[i];
        for (int ty = room->y + 1; ty < room->y + room->h - 1; ty++) {
            for (int tx = room->x + 1; tx < room->x + room->w - 1; tx++) {
                if (enemy_tile_open(g, tx, ty)) {
                    *x = tx;
                    *y = ty;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int spawn_into_open_tile(GameState *g, EnemyType type, int room_limit) {
    if (g->enemy_count >= MAX_ENEMIES) {
        return 0;
    }
    int x;
    int y;
    if (!find_enemy_tile(g, &x, &y, room_limit)) {
        return 0;
    }
    spawn_enemy(&g->enemies[g->enemy_count], type, x, y);
    g->enemy_count++;
    return 1;
}

void enemies_spawn(GameState *g) {
    g->enemy_count = 0;
    if (g->map.room_count == 0) {
        return;
    }

    int num_enemies = 10 + g->level;
    if (num_enemies > MAX_ENEMIES) {
        num_enemies = MAX_ENEMIES;
    }

    EnemyType boss_type;
    if (boss_for_level(g->level, &boss_type)) {
        int boss_x = g->map.stairs_down_x + 1;
        int boss_y = g->map.stairs_down_y;
        spawn_enemy(&g->enemies[g->enemy_count++], boss_type,
            boss_x, boss_y);
    }

    int regular_room_limit = g->level == MAX_DEPTH
        ? g->map.room_count - 1 : g->map.room_count;
    while (g->enemy_count < num_enemies) {
        EnemyType type;
        int roll = rand() % 100;
        int level = g->level;

        if (level == 1) {
            type = ENEMY_SKELETON;
        } else if (level == 2) {
            if (roll < 50) type = ENEMY_SKELETON;
            else if (roll < 80) type = ENEMY_ZOMBIE;
            else type = ENEMY_CRYPT_BAT;
        } else if (level == 3) {
            if (roll < 30) type = ENEMY_SKELETON;
            else if (roll < 65) type = ENEMY_ZOMBIE;
            else if (roll < 85) type = ENEMY_CRYPT_BAT;
            else type = ENEMY_WRAITH;
        } else if (level == 4) {
            if (roll < 20) type = ENEMY_SKELETON;
            else if (roll < 55) type = ENEMY_ZOMBIE;
            else if (roll < 70) type = ENEMY_CRYPT_BAT;
            else if (roll < 90) type = ENEMY_WRAITH;
            else type = ENEMY_NECROMANCER;
        } else {
            if (roll < 15) type = ENEMY_SKELETON;
            else if (roll < 45) type = ENEMY_ZOMBIE;
            else if (roll < 60) type = ENEMY_CRYPT_BAT;
            else if (roll < 80) type = ENEMY_WRAITH;
            else type = ENEMY_NECROMANCER;
        }
        if (!spawn_into_open_tile(g, type, regular_room_limit)) {
            break;
        }
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
    g->player.name[0] = '\0';
    g->player.level = 1;
    g->player.experience = 0;
    g->player.experience_next = 100;
    g->inventory_count = 0;
    g->equipped_weapon = -1;
    g->equipped_armor = -1;
    g->gold = 0;
    g->score = 0;
    g->dungeon_key_found = 0;
    g->portal_active = 0;
    g->portal_level = 0;
    g->portal_x = 0;
    g->portal_y = 0;
    g->portal_origin_tile = TILE_FLOOR;
    g->floor_item_count = 0;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        g->inventory[i].active = 0;
    }
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        g->floor_items[i].active = 0;
    }

    g->player.known_spell_count = 0;
    g->player.equipped_spell = -1;
    g->player.last_dx = 0;
    g->player.last_dy = 0;
    g->player.poison_turns = 0;
    g->trail_count = 0;
    g->trail_frames = 0;
    g->trail_effect = TRAIL_EFFECT_GENERIC;
    g->trail_started_at = 0;

    switch (g->player.player_class) {
        case CLASS_WARRIOR:
            g->player.max_hp = 150;
            g->player.max_mp = 20;
            g->player.attack = 14;
            g->player.defense = 6;
            g->inventory[g->inventory_count++] = item_make_rusty_sword();
            break;
        case CLASS_MAGE:
            g->player.max_hp = 70;
            g->player.max_mp = 100;
            g->player.attack = 4;
            g->player.defense = 2;
            g->inventory[g->inventory_count++] = item_make_staff();
            g->inventory[g->inventory_count++] = item_make_scroll_magic_arrow();
            break;
        case CLASS_ROGUE:
            g->player.max_hp = 100;
            g->player.max_mp = 40;
            g->player.attack = 10;
            g->player.defense = 4;
            g->inventory[g->inventory_count++] = item_make_bow();
            break;
    }
    g->inventory[g->inventory_count++] = item_make_scroll_return_to_town();
    g->player.hp = g->player.max_hp;
    g->player.mp = g->player.max_mp;

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
    if (g->level >= MAX_DEPTH) return;

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

void game_return_to_town(GameState *g) {
    // Cache current level before leaving
    if (g->level >= 1 && g->level <= MAX_DEPTH) {
        g->level_cache[g->level - 1].map = g->map;
        g->level_cache[g->level - 1].enemy_count   = g->enemy_count;
        g->level_cache[g->level - 1].level_cleared = g->level_cleared;
        for (int i = 0; i < g->enemy_count; i++)
            g->level_cache[g->level - 1].enemies[i] = g->enemies[i];
        g->level_cache[g->level - 1].valid = 1;
    }

    g->location = LOCATION_TOWN;
    int spawn_x, spawn_y;
    map_generate_town(&g->map, &spawn_x, &spawn_y);
    g->player.x = 20;
    g->player.y = 1;
    g->floor_item_count = 0;
    g->enemy_count = 0;
}

void game_open_town_portal(GameState *g) {
    if (g->location != LOCATION_DUNGEON) return;
    g->portal_active = 1;
    g->portal_level = g->level;
    g->portal_x = g->player.x;
    g->portal_y = g->player.y;
    g->portal_origin_tile = g->map.tiles[g->player.y][g->player.x];
    g->map.tiles[g->player.y][g->player.x] = TILE_PORTAL;
    game_return_to_town(g);
    g->map.tiles[2][20] = TILE_PORTAL;
    push_message(g, "A return portal remains open.");
}

void game_use_town_portal(GameState *g) {
    if (!g->portal_active || g->portal_level < 1 ||
        g->portal_level > MAX_DEPTH) return;
    int level = g->portal_level;
    if (!g->level_cache[level - 1].valid) return;

    g->location = LOCATION_DUNGEON;
    g->level = level;
    g->map = g->level_cache[level - 1].map;
    g->enemy_count = g->level_cache[level - 1].enemy_count;
    g->level_cleared = g->level_cache[level - 1].level_cleared;
    for (int i = 0; i < g->enemy_count; i++)
        g->enemies[i] = g->level_cache[level - 1].enemies[i];
    g->player.x = g->portal_x;
    g->player.y = g->portal_y;
    g->map.tiles[g->portal_y][g->portal_x] = g->portal_origin_tile;
    g->portal_active = 0;
    push_message(g, "Returned through the portal.");
}

void game_mark_level_cleared(GameState *g) {
    g->level_cleared = 1;
    if (g->location == LOCATION_DUNGEON && g->level == MAX_DEPTH) {
        g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] =
            TILE_RETURN_EXIT;
        push_message(g, "A passage to town opens!");
    }
}

void game_update_level_progress(GameState *g) {
    int active_enemies = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].active) continue;
        active_enemies++;
    }

    if (active_enemies == 0) game_mark_level_cleared(g);
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
