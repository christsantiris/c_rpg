#include "actions.h"
#include "game.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "item.h"

static int abs_int(int n) { return n < 0 ? -n : n; }

static Action make_move_or_attack(int tx, int ty) {
    Action a;
    a.target_x = tx;
    a.target_y = ty;
    a.type     = ACTION_MOVE;
    return a;
}

void push_message(GameState *g, const char *msg) {
    if (g->message_count < MAX_MESSAGES) {
        strncpy(g->messages[g->message_count], msg, MAX_MESSAGE_LEN - 1);
        g->messages[g->message_count][MAX_MESSAGE_LEN - 1] = '\0';
        g->message_count++;
    } else {
        for (int i = 0; i < MAX_MESSAGES - 1; i++)
            strncpy(g->messages[i], g->messages[i + 1], MAX_MESSAGE_LEN);
        strncpy(g->messages[MAX_MESSAGES - 1], msg, MAX_MESSAGE_LEN - 1);
        g->messages[MAX_MESSAGES - 1][MAX_MESSAGE_LEN - 1] = '\0';
    }
}

static void drop_loot(GameState *g, int x, int y, EnemyType type) {
    // Gold drop
    int gold = 0;
    switch (type) {
        case ENEMY_SKELETON: gold = 2 + rand() % 4;  break;
        case ENEMY_GOBLIN:   gold = 3 + rand() % 5;  break;
        case ENEMY_ZOMBIE:   gold = 4 + rand() % 6;  break;
        case ENEMY_ORC:      gold = 6 + rand() % 8;  break;
        case ENEMY_TROLL:    gold = 10 + rand() % 10; break;
        case ENEMY_GIANT:    gold = 15 + rand() % 15; break;
    }
    
    // 50% chance to drop gold
    if (rand() % 100 < 50) {
        g->gold += gold;
        char msg[MAX_MESSAGE_LEN];
        snprintf(msg, sizeof(msg), "Found %d gold!", gold);
        push_message(g, msg);
    }

    // Item drop — 25% chance
    if (rand() % 100 >= 25) return;
    if (g->floor_item_count >= MAX_FLOOR_ITEMS) return;

    Item item;
    switch (rand() % 4) {
        case 0: item = item_make_health_potion(); break;
        case 1: item = item_make_mana_potion();   break;
        case 2: item = item_make_weapon("Iron Sword", 3, 20); break;
        default: item = item_make_armor("Leather Armor", 2, 15); break;
    }

    FloorItem fi = {0};
    fi.active = 1;
    fi.x      = x;
    fi.y      = y;
    fi.item   = item;
    g->floor_items[g->floor_item_count++] = fi;

    g->map.tiles[y][x] = TILE_ITEM;
    char item_msg[MAX_MESSAGE_LEN];
    snprintf(item_msg, sizeof(item_msg), "%s dropped!", item.name);
    push_message(g, item_msg);
}

void action_resolve_player(GameState *g, Action a) {
    if (a.type == ACTION_NONE) return;

    if (a.type == ACTION_DESCEND) {
        if (g->map.tiles[g->player.y][g->player.x] == TILE_STAIRS_DOWN) {
            if (g->level_cleared) {
                game_descend(g);
            } else {
                push_message(g, "Clear the level first!");
            }
        }
        return;
    }

    if (a.type == ACTION_ASCEND) {
        if (g->map.tiles[g->player.y][g->player.x] == TILE_STAIRS_UP) {
            if (g->level == 1) {
                g->location = LOCATION_TOWN;
                int spawn_x, spawn_y;
                map_generate_town(&g->map, &spawn_x, &spawn_y);
                g->player.x = spawn_x;
                g->player.y = spawn_y;
            } else {
                game_ascend(g);
            }
        }
        return;
    }

    if (a.type == ACTION_PICK_UP) {
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *fi = &g->floor_items[i];
            if (!fi->active) continue;
            if (fi->x != g->player.x || fi->y != g->player.y) continue;
            if (g->inventory_count >= MAX_INVENTORY) {
                push_message(g, "Inventory full!");
                return;
            }
            g->inventory[g->inventory_count++] = fi->item;
            fi->active = 0;
            g->map.tiles[fi->y][fi->x] = TILE_FLOOR;
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Picked up %s", fi->item.name);
            push_message(g, msg);
            return;
        }
        push_message(g, "Nothing to pick up");
        return;
    }
    if (a.type == ACTION_USE_ITEM) {
        int idx = a.target_x;
        if (idx < 0 || idx >= g->inventory_count) return;
        Item *item = &g->inventory[idx];
        char msg[MAX_MESSAGE_LEN];

        if (item->type == ITEM_POTION_HEALTH) {
            int healed = item->heal_hp;
            g->player.hp += healed;
            if (g->player.hp > g->player.max_hp)
                g->player.hp = g->player.max_hp;
            snprintf(msg, sizeof(msg), "Drank %s +%d HP", item->name, healed);
            push_message(g, msg);
        } else if (item->type == ITEM_POTION_MANA) {
            int restored = item->heal_mp;
            g->player.mp += restored;
            if (g->player.mp > g->player.max_mp)
                g->player.mp = g->player.max_mp;
            snprintf(msg, sizeof(msg), "Drank %s +%d MP", item->name, restored);
            push_message(g, msg);
        } else {
            push_message(g, "Cannot use that item");
            return;
        }

        // Remove item from inventory
        for (int i = idx; i < g->inventory_count - 1; i++)
            g->inventory[i] = g->inventory[i + 1];
        g->inventory_count--;
        return;
    }

    if (a.type == ACTION_EQUIP_ITEM) {
        int idx = a.target_x;
        if (idx < 0 || idx >= g->inventory_count) return;
        Item *item = &g->inventory[idx];
        char msg[MAX_MESSAGE_LEN];

        if (item->type == ITEM_WEAPON) {
            // Unequip old weapon bonus
            if (g->equipped_weapon >= 0)
                g->player.attack -= g->inventory[g->equipped_weapon].attack_bonus;
            g->equipped_weapon = idx;
            g->player.attack  += item->attack_bonus;
            snprintf(msg, sizeof(msg), "Equipped %s", item->name);
            push_message(g, msg);
        } else if (item->type == ITEM_ARMOR) {
            // Unequip old armor bonus
            if (g->equipped_armor >= 0)
                g->player.defense -= g->inventory[g->equipped_armor].defense_bonus;
            g->equipped_armor  = idx;
            g->player.defense += item->defense_bonus;
            snprintf(msg, sizeof(msg), "Equipped %s", item->name);
            push_message(g, msg);
        } else {
            push_message(g, "Cannot equip that item");
        }
        return;
    }

    if (a.type == ACTION_MOVE) {
        int tx = a.target_x;
        int ty = a.target_y;

        // Check for enemy at target
        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *e = &g->enemies[i];
            if (!e->active) continue;
            if (e->x == tx && e->y == ty) {
                // Melee attack
                int dmg = g->player.attack - e->defense;
                if (dmg < 1) dmg = 1;
                e->hp -= dmg;
                char msg[MAX_MESSAGE_LEN];
                if (e->hp <= 0) {
                    e->active = 0;
                    drop_loot(g, e->x, e->y, e->type);
                    player_gain_xp(g, e->experience);
                    int all_clear = 1;
                    for (int j = 0; j < g->enemy_count; j++) {
                        if (g->enemies[j].active) { all_clear = 0; break; }
                    }
                    if (all_clear) g->level_cleared = 1;
                    char msg[MAX_MESSAGE_LEN];
                    snprintf(msg, sizeof(msg), "Killed %s!", e->name);
                    push_message(g, msg);
                } else {
                    char msg[MAX_MESSAGE_LEN];
                    snprintf(msg, sizeof(msg), "Hit %s: %d dmg", e->name, dmg);
                    push_message(g, msg);
                }
                return;
            }
        }
        // Check for town exit
        if (g->location == LOCATION_TOWN &&
            g->map.tiles[ty][tx] == TILE_TOWN_EXIT) {
            game_enter_dungeon(g);
            return;
        }

        // Move if walkable
        if (map_is_walkable(&g->map, tx, ty))
            game_move_player(g, tx - g->player.x, ty - g->player.y);
    }
}

void action_resolve_enemies(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (!e->active) continue;

        int dx = g->player.x - e->x;
        int dy = g->player.y - e->y;

        // Adjacent to player — melee attack
        if (abs_int(dx) <= 1 && abs_int(dy) <= 1 &&
            !(dx == 0 && dy == 0)) {
                int dmg = e->attack - g->player.defense;
                if (dmg < 1) dmg = 1;
                g->player.hp -= dmg;
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "%s: %d dmg", e->name, dmg);
                push_message(g, msg);
                continue;
        }

        // Move toward player
        if (e->type == ENEMY_ZOMBIE) { // Zombies move every other turn
            e->move_timer++;
            if (e->move_timer % 2 != 0) continue;
        }

        int mx = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
        int my = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
        int tx = e->x + mx;
        int ty = e->y + my;

        if (map_is_walkable(&g->map, tx, ty) &&
            !(tx == g->player.x && ty == g->player.y)) {
            e->x = tx;
            e->y = ty;
        }
    }
}