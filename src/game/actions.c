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

static Item random_weapon(int level) {
    if (level <= 3) {
        int r = rand() % 2;
        if (r == 0) return item_make_rusty_sword();
        return item_make_short_sword();
    } else if (level <= 6) {
        int r = rand() % 3;
        if (r == 0) return item_make_short_sword();
        if (r == 1) return item_make_long_sword();
        return item_make_bow();
    } else {
        int r = rand() % 4;
        if (r == 0) return item_make_long_sword();
        if (r == 1) return item_make_battle_axe();
        if (r == 2) return item_make_bow();
        return item_make_staff();
    }
}

static void drop_loot(GameState *g, int x, int y, EnemyType type, int is_boss) {
    // Gold dropstatic void drop_loot(GameState *g, int x, int y, EnemyType type) {
    int gold = 0;
    switch (type) {
        case ENEMY_SKELETON: gold = 2 + rand() % 4;  break;
        case ENEMY_GOBLIN:   gold = 3 + rand() % 5;  break;
        case ENEMY_ZOMBIE:   gold = 4 + rand() % 6;  break;
        case ENEMY_ORC:      gold = 6 + rand() % 8;  break;
        case ENEMY_TROLL:    gold = 10 + rand() % 10; break;
        case ENEMY_GIANT:    gold = 15 + rand() % 15; break;
        case ENEMY_GOBLIN_KING: break;
        case ENEMY_LICH_KING:  break;
        case ENEMY_DEMON_LORD:  break;
        case ENEMY_RED_DRAGON: break;
        case ENEMY_TARRASQUE:  break;
    }
    
    // 50% chance to drop gold
    if (rand() % 100 < 20) {
        g->gold += gold;
        char msg[MAX_MESSAGE_LEN];
        snprintf(msg, sizeof(msg), "Found %d gold!", gold);
        push_message(g, msg);
    }

    // Boss guaranteed drop
    if (is_boss) {
        if (g->floor_item_count < MAX_FLOOR_ITEMS) {
            Item boss_drop = rand() % 2 == 0
                ? random_weapon(g->level)
                : item_make_chain_mail();
            FloorItem fi = {0};
            fi.active = 1;
            fi.x = x; fi.y = y;
            fi.item = boss_drop;
            g->map.tiles[y][x] = TILE_ITEM;
            g->floor_items[g->floor_item_count++] = fi;
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "%s dropped!", boss_drop.name);
            push_message(g, msg);
        }
        return;
    }

    // Item drop — 25% chance
    if (rand() % 100 >= 10) return;
    if (g->floor_item_count >= MAX_FLOOR_ITEMS) return;

    Item item;
    int roll = rand() % 100;
    int level = g->level;

    if (level <= 3) {
        // Early levels: potions and magic arrow scrolls
        if (roll < 40)      item = item_make_health_potion();
        else if (roll < 70) item = item_make_mana_potion();
        else if (roll < 90) item = item_make_scroll_magic_arrow();
        else                item = item_make_scroll_heal();
    } else if (level <= 6) {
        // Mid levels: weapons, armor, heal scrolls
        if (roll < 25)      item = item_make_health_potion();
        else if (roll < 45) item = item_make_mana_potion();
        else if (roll < 60) item = random_weapon(level);
        else if (roll < 75) item = item_make_leather_armor();
        else if (roll < 88) item = item_make_scroll_magic_arrow();
        else if (roll < 95) item = item_make_scroll_heal();
        else                item = item_make_scroll_fireball();
    } else {
        // Deep levels: better drops, fireball scrolls
        if (roll < 20)      item = item_make_health_potion();
        else if (roll < 35) item = item_make_mana_potion();
        else if (roll < 60) item = random_weapon(level);
        else if (roll < 65) item = item_make_leather_armor();
        else if (roll < 75) item = item_make_scroll_magic_arrow();
        else if (roll < 88) item = item_make_scroll_heal();
        else                item = item_make_scroll_fireball();
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

static void set_trail(GameState *g, int sx, int sy,
                      int tx, int ty, int dx, int dy,
                      int range, Uint8 r, Uint8 gr, Uint8 b) {
    g->trail_count  = 0;
    g->trail_frames = 4;
    int cx = sx;
    int cy = sy;
    for (int step = 1; step <= range; step++) {
        cx = sx + dx * step;
        cy = sy + dy * step;
        if (cx < 0 || cx >= MAP_W || cy < 0 || cy >= MAP_H) break;
        if (!map_is_walkable(&g->map, cx, cy)) break;
        if (g->trail_count >= MAX_TRAIL) break;
        TrailTile *t      = &g->trail[g->trail_count++];
        t->active         = 1;
        t->x              = cx;
        t->y              = cy;
        t->r              = r;
        t->g              = gr;
        t->b              = b;
        t->is_impact      = (cx == tx && cy == ty);
    }
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
        } else if (item->type == ITEM_SCROLL) {
            for (int i = 0; i < g->player.known_spell_count; i++) {
                if (g->player.known_spells[i].id == item->spell_id) {
                    push_message(g, "Already know that spell");
                    return;
                }
            }
            if (g->player.known_spell_count >= MAX_SPELLS) {
                push_message(g, "Cannot learn more spells");
                return;
            }
            Spell learned;
            switch (item->spell_id) {
                case SPELL_MAGIC_ARROW: learned = spell_make_magic_arrow(); break;
                case SPELL_FIREBALL:    learned = spell_make_fireball();    break;
                case SPELL_HEAL:        learned = spell_make_heal();        break;
                default: return;
            }
            g->player.known_spells[g->player.known_spell_count++] = learned;
            snprintf(msg, sizeof(msg), "Learned %s!", learned.name);
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
            if (g->equipped_weapon >= 0 &&
                g->equipped_weapon < g->inventory_count)
                g->player.attack -= g->inventory[g->equipped_weapon].attack_bonus;
            g->equipped_weapon = idx;
            g->player.attack  += item->attack_bonus;
            snprintf(msg, sizeof(msg), "Equipped %s", item->name);
            push_message(g, msg);
        } else if (item->type == ITEM_ARMOR) {
            if (g->equipped_armor >= 0 &&
                g->equipped_armor < g->inventory_count)
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
    if (a.type == ACTION_DROP_ITEM) {
        int idx = a.target_x;
        if (idx < 0 || idx >= g->inventory_count) return;
        if (g->floor_item_count >= MAX_FLOOR_ITEMS) {
            push_message(g, "No room to drop item!");
            return;
        }

        Item *item = &g->inventory[idx];

        // Unequip if equipped
        if (g->equipped_weapon == idx) {
            g->player.attack   -= item->attack_bonus;
            g->equipped_weapon  = -1;
        } else if (g->equipped_armor == idx) {
            g->player.defense  -= item->defense_bonus;
            g->equipped_armor   = -1;
        }

        // Adjust equipped indices if needed
        if (g->equipped_weapon > idx) g->equipped_weapon--;
        if (g->equipped_armor  > idx) g->equipped_armor--;

        // Place on floor
        FloorItem fi = {0};
        fi.active = 1;
        fi.x      = g->player.x;
        fi.y      = g->player.y;
        fi.item   = *item;
        g->map.tiles[fi.y][fi.x] = TILE_ITEM;
        g->floor_items[g->floor_item_count++] = fi;

        // Remove from inventory
        for (int i = idx; i < g->inventory_count - 1; i++)
            g->inventory[i] = g->inventory[i + 1];
        g->inventory_count--;

        char msg[MAX_MESSAGE_LEN];
        snprintf(msg, sizeof(msg), "Dropped %s", fi.item.name);
        push_message(g, msg);
        return;
    }

    if (a.type == ACTION_CAST_SPELL) {
        if (g->player.equipped_spell < 0 ||
            g->player.equipped_spell >= g->player.known_spell_count) {
            push_message(g, "No spell equipped!");
            return;
        }

        Spell *sp = &g->player.known_spells[g->player.equipped_spell];

        if (g->player.mp < sp->mp_cost) {
            push_message(g, "Not enough MP!");
            return;
        }

        if (g->player.last_dx == 0 && g->player.last_dy == 0) {
            push_message(g, "Move first to aim!");
            return;
        }

        g->player.mp -= sp->mp_cost;

        // Set trail based on spell type
        if (sp->type == SPELL_TYPE_DAMAGE_RANGED) {
            int ex = g->player.x + g->player.last_dx * sp->range;
            int ey = g->player.y + g->player.last_dy * sp->range;
            set_trail(g, g->player.x, g->player.y,
                ex, ey,
                g->player.last_dx, g->player.last_dy,
                sp->range, 40, 120, 220);
        } else if (sp->type == SPELL_TYPE_DAMAGE_AREA) {
            int ex = g->player.x + g->player.last_dx * sp->range;
            int ey = g->player.y + g->player.last_dy * sp->range;
            set_trail(g, g->player.x, g->player.y,
                ex, ey,
                g->player.last_dx, g->player.last_dy,
                sp->range, 220, 100, 20);
        } else if (sp->type == SPELL_TYPE_HEAL) {
            // Heal — green ring on player tile
            g->trail_count  = 0;
            g->trail_frames = 4;
            TrailTile *t = &g->trail[g->trail_count++];
            t->active    = 1;
            t->x         = g->player.x;
            t->y         = g->player.y;
            t->r         = 40;
            t->g         = 180;
            t->b         = 80;
            t->is_impact = 1;
        }

        if (sp->type == SPELL_TYPE_DAMAGE_RANGED) {
            // Travel in last direction, hit first enemy
            int cx = g->player.x;
            int cy = g->player.y;
            int hit = 0;
            for (int step = 1; step <= sp->range && !hit; step++) {
                cx = g->player.x + g->player.last_dx * step;
                cy = g->player.y + g->player.last_dy * step;
                if (!map_is_walkable(&g->map, cx, cy)) break;
                for (int i = 0; i < g->enemy_count; i++) {
                    Enemy *e = &g->enemies[i];
                    if (!e->active) continue;
                    if (e->x == cx && e->y == cy) {
                        int dmg = sp->damage + g->player.level * 2;
                        e->hp -= dmg;
                        char msg[MAX_MESSAGE_LEN];
                        if (e->hp <= 0) {
                            e->active = 0;
                            int all_clear = 1;
                            for (int j = 0; j < g->enemy_count; j++)
                                if (g->enemies[j].active) { all_clear = 0; break; }
                            if (all_clear) g->level_cleared = 1;
                            drop_loot(g, e->x, e->y, e->type, e->is_boss);
                            player_gain_xp(g, e->experience);
                            snprintf(msg, sizeof(msg), "%s killed %s!",
                                sp->name, e->name);
                        } else {
                            snprintf(msg, sizeof(msg), "%s hit %s: %d dmg",
                                sp->name, e->name, dmg);
                        }
                        push_message(g, msg);
                        hit = 1;
                    }
                }
            }
            if (!hit) push_message(g, "Spell missed!");

        } else if (sp->type == SPELL_TYPE_HEAL) {
            int healed = sp->heal_hp + g->player.level * 2;
            g->player.hp += healed;
            if (g->player.hp > g->player.max_hp)
                g->player.hp = g->player.max_hp;
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Healed %d HP!", healed);
            push_message(g, msg);

        } else if (sp->type == SPELL_TYPE_DAMAGE_AREA) {
            // Travel then explode in radius
            int cx = g->player.x + g->player.last_dx * sp->range;
            int cy = g->player.y + g->player.last_dy * sp->range;
            int hits = 0;
            for (int i = 0; i < g->enemy_count; i++) {
                Enemy *e = &g->enemies[i];
                if (!e->active) continue;
                int dx = e->x - cx;
                int dy = e->y - cy;
                int dist = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
                if (dist <= sp->radius) {
                    int dmg = sp->damage + g->player.level * 2;
                    e->hp -= dmg;
                    if (e->hp <= 0) {
                        e->active = 0;
                        drop_loot(g, e->x, e->y, e->type, e->is_boss);
                        player_gain_xp(g, e->experience);
                    }
                    hits++;
                }
            }
            int all_clear = 1;
            for (int j = 0; j < g->enemy_count; j++)
                if (g->enemies[j].active) { all_clear = 0; break; }
            if (all_clear) g->level_cleared = 1;
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Fireball hit %d enemies!", hits);
            push_message(g, msg);
        }
        return;
    }

    if (a.type == ACTION_RANGED_ATTACK) {
        if (g->equipped_weapon < 0 ||
            g->equipped_weapon >= g->inventory_count) {
            push_message(g, "No weapon equipped!");
            return;
        }

        Item *wpn = &g->inventory[g->equipped_weapon];
        if (!wpn->is_ranged) {
            push_message(g, "No ranged weapon equipped!");
            return;
        }

        if (g->player.last_dx == 0 && g->player.last_dy == 0) {
            push_message(g, "Move first to aim!");
            return;
        }

        int hit = 0;
        for (int step = 1; step <= wpn->range && !hit; step++) {
            int tx = g->player.x + g->player.last_dx * step;
            int ty = g->player.y + g->player.last_dy * step;
            if (!map_is_walkable(&g->map, tx, ty)) break;
            for (int i = 0; i < g->enemy_count; i++) {
                Enemy *e = &g->enemies[i];
                if (!e->active) continue;
                if (e->x != tx || e->y != ty) continue;
                int dmg = g->player.attack - e->defense;
                if (dmg < 1) dmg = 1;
                e->hp -= dmg;
                char msg[MAX_MESSAGE_LEN];
                if (e->hp <= 0) {
                    e->active = 0;
                    int all_clear = 1;
                    for (int j = 0; j < g->enemy_count; j++)
                        if (g->enemies[j].active) { all_clear = 0; break; }
                    if (all_clear) g->level_cleared = 1;
                    drop_loot(g, e->x, e->y, e->type, e->is_boss);
                    player_gain_xp(g, e->experience);
                    snprintf(msg, sizeof(msg), "Attack killed %s!", e->name);
                } else {
                    snprintf(msg, sizeof(msg), "Attack hit %s: %d dmg",
                        e->name, dmg);
                }
                push_message(g, msg);
                hit = 1;
            }
        }
        // Gray trail for ranged weapon
        int ex = g->player.x + g->player.last_dx * wpn->range;
        int ey = g->player.y + g->player.last_dy * wpn->range;
        set_trail(g, g->player.x, g->player.y,
            ex, ey,
            g->player.last_dx, g->player.last_dy,
            wpn->range, 160, 160, 160);
        if (!hit) push_message(g, "Attack missed!");
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
                    drop_loot(g, e->x, e->y, e->type, e->is_boss);
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
        // Track last direction for ranged attacks
        if (map_is_walkable(&g->map, tx, ty)) {
            g->player.last_dx = tx - g->player.x;
            g->player.last_dy = ty - g->player.y;
            game_move_player(g, tx - g->player.x, ty - g->player.y);
        }
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