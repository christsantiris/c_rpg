#include "actions.h"
#include "game.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "item.h"
// sfx.h is excluded from the test runner because it links SDL2_mixer,
// which is not available in the test build. TEST_BUILD is defined in CMakeLists.txt.
#ifndef TEST_BUILD
#include "../audio/sfx.h"
#endif

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

static int enemy_score(EnemyType type) {
    switch (type) {
        case ENEMY_SKELETON:    return 10;
        case ENEMY_GOBLIN:      return 15;
        case ENEMY_ZOMBIE:      return 20;
        case ENEMY_CRYPT_BAT:   return 18;
        case ENEMY_WRAITH:      return 40;
        case ENEMY_CRYPT_CONJURER: return 65;
        case ENEMY_PIXIE: return 15;
        case ENEMY_BLIGHTED_WOLF: return 22;
        case ENEMY_GIANT_SPIDER: return 32;
        case ENEMY_DARK_ELF: return 48;
        case ENEMY_GIANT_WURM: return 70;
        case ENEMY_FOREST_TROLL: return 80;
        case ENEMY_FOREST_NECROMANCER: return 1100;
        case ENEMY_GOBLIN_SCOUT: return 18;
        case ENEMY_GOBLIN_ARCHER: return 28;
        case ENEMY_GOBLIN_BOMBER: return 36;
        case ENEMY_TUNNEL_SPIDER: return 34;
        case ENEMY_CAVE_TROLL: return 85;
        case ENEMY_HOBGOBLIN_GUARD: return 70;
        case ENEMY_GOBLIN_SHAMAN: return 76;
        case ENEMY_MOUNTAIN_GOBLIN_KING: return 1300;
        case ENEMY_ORC:         return 30;
        case ENEMY_TROLL:       return 50;
        case ENEMY_GIANT:       return 80;
        case ENEMY_GOBLIN_KING: return 500;
        case ENEMY_LICH_KING:   return 1000;
        case ENEMY_DEMON_LORD:  return 2000;
        case ENEMY_RED_DRAGON:  return 3500;
        case ENEMY_TARRASQUE:   return 5000;
        default:                return 0;
    }
}

static void drop_loot(GameState *g, int x, int y, EnemyType type, int is_boss) {
    // Gold dropstatic void drop_loot(GameState *g, int x, int y, EnemyType type) {
    int gold = 0;
    switch (type) {
        case ENEMY_SKELETON: gold = 2 + rand() % 4;  break;
        case ENEMY_GOBLIN:   gold = 3 + rand() % 5;  break;
        case ENEMY_ZOMBIE:   gold = 4 + rand() % 6;  break;
        case ENEMY_CRYPT_BAT: gold = 2 + rand() % 4; break;
        case ENEMY_WRAITH: gold = 7 + rand() % 7; break;
        case ENEMY_CRYPT_CONJURER: gold = 10 + rand() % 9; break;
        case ENEMY_PIXIE: gold = 2 + rand() % 4; break;
        case ENEMY_BLIGHTED_WOLF: gold = 3 + rand() % 5; break;
        case ENEMY_GIANT_SPIDER: gold = 4 + rand() % 6; break;
        case ENEMY_DARK_ELF: gold = 7 + rand() % 8; break;
        case ENEMY_GIANT_WURM: gold = 10 + rand() % 10; break;
        case ENEMY_FOREST_TROLL: gold = 12 + rand() % 12; break;
        case ENEMY_FOREST_NECROMANCER: gold = 50; break;
        case ENEMY_GOBLIN_SCOUT: gold = 3 + rand() % 5; break;
        case ENEMY_GOBLIN_ARCHER: gold = 5 + rand() % 7; break;
        case ENEMY_GOBLIN_BOMBER: gold = 6 + rand() % 8; break;
        case ENEMY_TUNNEL_SPIDER: gold = 4 + rand() % 6; break;
        case ENEMY_CAVE_TROLL: gold = 12 + rand() % 12; break;
        case ENEMY_HOBGOBLIN_GUARD: gold = 10 + rand() % 10; break;
        case ENEMY_GOBLIN_SHAMAN: gold = 11 + rand() % 11; break;
        case ENEMY_MOUNTAIN_GOBLIN_KING: gold = 60; break;
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
    if (is_boss || rand() % 100 < 20) {
        g->gold += gold;
        g->score += gold;
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
            fi.underlying_tile = g->map.tiles[y][x];
            fi.item = boss_drop;
            g->map.tiles[y][x] = TILE_ITEM;
            g->floor_items[g->floor_item_count++] = fi;
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "%s dropped!", boss_drop.name);
            push_message(g, msg);
        }
        return;
    }

    // Item drop — 5% chance
    if (rand() % 100 >= 5) return;
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
    fi.underlying_tile = g->map.tiles[y][x];
    fi.item   = item;
    g->floor_items[g->floor_item_count++] = fi;

    g->map.tiles[y][x] = TILE_ITEM;
    char item_msg[MAX_MESSAGE_LEN];
    snprintf(item_msg, sizeof(item_msg), "%s dropped!", item.name);
    push_message(g, item_msg);
}

static void set_trail(GameState *g, int sx, int sy,
                      int tx, int ty, int dx, int dy,
                      int range, Uint8 r, Uint8 gr, Uint8 b,
                      TrailEffect effect) {
    g->trail_count  = 0;
    g->trail_frames = 4;
    g->trail_effect = effect;
    #ifndef TEST_BUILD
    g->trail_started_at = SDL_GetTicks();
    #else
    g->trail_started_at = 0;
    #endif
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
        if (t->is_impact) break;
    }
}

void action_resolve_player(GameState *g, Action a) {
    if (a.type == ACTION_NONE) return;

    if (a.type == ACTION_DESCEND) {
        TileType tile = g->map.tiles[g->player.y][g->player.x];
        if (tile == TILE_RETURN_EXIT && g->level_cleared) {
            int forest = g->location == LOCATION_FOREST;
            g->score += g->level * 100;
            game_return_to_town(g);
            push_message(g, forest ? "The forest is freed!" :
                "The Lich is defeated!");
        } else if (tile == TILE_STAIRS_DOWN) {
            if (g->level_cleared) {
                if (g->level < DUNGEON_DEPTH) {
                    game_descend(g);
                    g->score += g->level * 100;
                }
            } else {
                push_message(g, "Clear the level first!");
            }
        }
        return;
    }

    if (a.type == ACTION_ASCEND) {
        if (g->map.tiles[g->player.y][g->player.x] == TILE_STAIRS_UP) {
            if (g->level == 1) {
                Location leaving = g->location;
                g->location = LOCATION_TOWN;
                int spawn_x, spawn_y;
                map_generate_town(&g->map, &spawn_x, &spawn_y);
                g->player.x = leaving == LOCATION_FOREST ? 1 : spawn_x;
                g->player.y = leaving == LOCATION_FOREST ? 12 : spawn_y;
            } else {
                game_ascend(g);
            }
        }
        return;
    }

    if (a.type == ACTION_PICK_UP) {
        if (g->map.tiles[g->player.y][g->player.x] == TILE_DUNGEON_KEY) {
            g->dungeon_key_found = 1;
            g->map.tiles[g->player.y][g->player.x] = TILE_FLOOR;
            push_message(g, "Picked up the Lich King's door key!");
            return;
        }
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
            g->map.tiles[fi->y][fi->x] = (TileType)fi->underlying_tile;
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
                case SPELL_RETURN_TO_TOWN:
                    learned = spell_make_return_to_town();
                    break;
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
        fi.underlying_tile = g->map.tiles[fi.y][fi.x];
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

        if (sp->type == SPELL_TYPE_UTILITY) {
            if (g->location == LOCATION_TOWN) {
                push_message(g, "Already in town!");
                return;
            }
            game_open_town_portal(g);
            return;
        }

        if (g->player.last_dx == 0 && g->player.last_dy == 0) {
            push_message(g, "Move first to aim!");
            return;
        }

        g->player.mp -= sp->mp_cost;

        // Set trail based on spell type
        if (sp->type == SPELL_TYPE_DAMAGE_RANGED) {
            #ifndef TEST_BUILD
            if (sp->id == SPELL_MAGIC_ARROW) sfx_play_magic_arrow();
            #endif
            int ex = g->player.x + g->player.last_dx * sp->range;
            int ey = g->player.y + g->player.last_dy * sp->range;
            set_trail(g, g->player.x, g->player.y,
                ex, ey,
                g->player.last_dx, g->player.last_dy,
                sp->range, 40, 120, 220, TRAIL_EFFECT_MAGIC_ARROW);
        } else if (sp->type == SPELL_TYPE_DAMAGE_AREA) {
            #ifndef TEST_BUILD
            sfx_play_fireball();
            #endif
            int ex = g->player.x + g->player.last_dx * sp->range;
            int ey = g->player.y + g->player.last_dy * sp->range;
            set_trail(g, g->player.x, g->player.y,
                ex, ey,
                g->player.last_dx, g->player.last_dy,
                sp->range, 220, 100, 20, TRAIL_EFFECT_FIREBALL);
        } else if (sp->type == SPELL_TYPE_HEAL) {
            #ifndef TEST_BUILD
            sfx_play_heal();
            #endif
            // Heal — green ring on player tile
            g->trail_count  = 0;
            g->trail_frames = 4;
            g->trail_effect = TRAIL_EFFECT_GENERIC;
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
                            game_update_level_progress(g);
                            drop_loot(g, e->x, e->y, e->type, e->is_boss);
                            player_gain_xp(g, e->experience);
                            g->score += enemy_score(e->type);
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
            game_update_level_progress(g);
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

        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *e = &g->enemies[i];
            if (!e->active) continue;
            int dx = abs_int(e->x - g->player.x);
            int dy = abs_int(e->y - g->player.y);
            if (dx <= 1 && dy <= 1) {
                push_message(g, "Too close to use bow!");
                return;
            }
        }

        #ifndef TEST_BUILD
        sfx_play_arrow();
        #endif

        int hit = 0;
        int impact_x = g->player.x + g->player.last_dx * wpn->range;
        int impact_y = g->player.y + g->player.last_dy * wpn->range;
        for (int step = 1; step <= wpn->range && !hit; step++) {
            int tx = g->player.x + g->player.last_dx * step;
            int ty = g->player.y + g->player.last_dy * step;
            if (!map_is_walkable(&g->map, tx, ty)) break;
            for (int i = 0; i < g->enemy_count; i++) {
                Enemy *e = &g->enemies[i];
                if (!e->active) continue;
                if (e->x != tx || e->y != ty) continue;
                if (step < 2) continue;
                int dmg = g->player.attack - e->defense;
                if (dmg < 1) dmg = 1;
                int critical = rand() % 100 < 15;
                if (critical) dmg = dmg * 3 / 2;
                e->hp -= dmg;
                char msg[MAX_MESSAGE_LEN];
                if (e->hp <= 0) {
                    e->active = 0;
                    game_update_level_progress(g);
                    drop_loot(g, e->x, e->y, e->type, e->is_boss);
                    player_gain_xp(g, e->experience);
                    snprintf(msg, sizeof(msg), "Attack killed %s!", e->name);
                } else if (critical) {
                    snprintf(msg, sizeof(msg), "Critical hit %s: %d dmg",
                        e->name, dmg);
                } else {
                    snprintf(msg, sizeof(msg), "Attack hit %s: %d dmg",
                        e->name, dmg);
                }
                push_message(g, msg);
                impact_x = tx;
                impact_y = ty;
                hit = 1;
            }
        }
        set_trail(g, g->player.x, g->player.y,
            impact_x, impact_y,
            g->player.last_dx, g->player.last_dy,
            wpn->range, 160, 160, 160, TRAIL_EFFECT_WEAPON_ARROW);
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
                int melee_attack = g->player.attack;
                if (g->equipped_weapon >= 0 &&
                    g->equipped_weapon < g->inventory_count) {
                    Item *wpn = &g->inventory[g->equipped_weapon];
                    if (wpn->is_ranged)
                        melee_attack -= wpn->attack_bonus;
                }
                int dmg = melee_attack - e->defense;
                if (dmg < 1) dmg = 1;
                e->hp -= dmg;
                #ifndef TEST_BUILD
                sfx_play_attack();
                #endif
                char msg[MAX_MESSAGE_LEN];
                if (e->hp <= 0) {
                    e->active = 0;
                    drop_loot(g, e->x, e->y, e->type, e->is_boss);
                    player_gain_xp(g, e->experience);
                    game_update_level_progress(g);
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
            g->map.tiles[ty][tx] == TILE_PORTAL && g->portal_active) {
            game_use_town_portal(g);
            return;
        }

        if (g->location == LOCATION_TOWN &&
            g->map.tiles[ty][tx] == TILE_TOWN_EXIT) {
            if (tx == 0) game_enter_forest(g);
            else if (tx == TOWN_W - 1) game_enter_mountains(g);
            else game_enter_dungeon(g);
            return;
        }

        if (g->location == LOCATION_FOREST &&
            g->map.tiles[ty][tx] == TILE_FOREST_ENTRANCE) {
            if (g->level == 1) game_return_to_town(g);
            else game_ascend(g);
            return;
        }

        if (g->location == LOCATION_MOUNTAINS &&
            g->map.tiles[ty][tx] == TILE_MOUNTAIN_ENTRANCE) {
            if (g->level == 1) game_return_to_town(g);
            else game_ascend(g);
            return;
        }

        if (g->location == LOCATION_MOUNTAINS &&
            g->map.tiles[ty][tx] == TILE_MOUNTAIN_EXIT) {
            if (g->level < MOUNTAIN_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            } else {
                int king_alive = 0;
                for (int i = 0; i < g->enemy_count; i++)
                    if (g->enemies[i].active &&
                        g->enemies[i].type == ENEMY_MOUNTAIN_GOBLIN_KING) {
                        king_alive = 1;
                        break;
                    }
                if (king_alive) {
                    push_message(g, "The Goblin King bars the pass!");
                    return;
                }
                g->score += g->level * 100;
                game_return_to_town(g);
                push_message(g, "The mountain pass is liberated!");
            }
            return;
        }

        if (g->location == LOCATION_FOREST &&
            g->map.tiles[ty][tx] == TILE_FOREST_EXIT) {
            if (g->level < FOREST_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            } else {
                int boss_alive = 0;
                for (int i = 0; i < g->enemy_count; i++)
                    if (g->enemies[i].active &&
                        g->enemies[i].type == ENEMY_FOREST_NECROMANCER) {
                        boss_alive = 1;
                        break;
                    }
                if (boss_alive) {
                    push_message(g, "The Necromancer seals the path!");
                    return;
                }
                g->score += g->level * 100;
                game_return_to_town(g);
                push_message(g, "The forest is freed!");
            }
            return;
        }

        if (g->map.tiles[ty][tx] == TILE_LOCKED_DOOR) {
            if (!g->dungeon_key_found) {
                push_message(g, "The Lich King's door is locked.");
                return;
            }
            g->dungeon_key_found = 0;
            g->map.tiles[ty][tx] = TILE_FLOOR;
            push_message(g, "The dungeon key unlocks the door!");
        }

        // Move if walkable
        // Track last direction for ranged attacks
        if (map_is_walkable(&g->map, tx, ty)) {
            g->player.last_dx = tx - g->player.x;
            g->player.last_dy = ty - g->player.y;
            game_move_player(g, tx - g->player.x, ty - g->player.y);
        }
        // Check for trap on new tile
        int px = g->player.x;
        int py = g->player.y;
        TileType tile = g->map.tiles[py][px];

        if (tile == TILE_TRAP_HIDDEN) {
            int roll = rand() % 3;
            TileType trap_type;
            if (roll == 0)      trap_type = TILE_TRAP_SPIKE;
            else if (roll == 1) trap_type = TILE_TRAP_FIRE;
            else                trap_type = TILE_TRAP_POISON;
            g->map.tiles[py][px] = trap_type;

            int dmg = 0;
            char msg[MAX_MESSAGE_LEN];

            if (trap_type == TILE_TRAP_SPIKE) {
                dmg = 5 + rand() % 10;
                g->player.hp -= dmg;
                snprintf(msg, sizeof(msg), "Spike trap! -%d HP", dmg);
                // Red flash
                g->trail_count  = 0;
                g->trail_frames = 4;
                g->trail_effect = TRAIL_EFFECT_GENERIC;
                TrailTile *t = &g->trail[g->trail_count++];
                t->active = 1; t->x = px; t->y = py;
                t->r = 200; t->g = 20; t->b = 20;
                t->is_impact = 1;
            } else if (trap_type == TILE_TRAP_FIRE) {
                dmg = 4 + rand() % 8;
                g->player.hp -= dmg;
                snprintf(msg, sizeof(msg), "Fire trap! -%d HP", dmg);
                // Orange flash
                g->trail_count  = 0;
                g->trail_frames = 4;
                g->trail_effect = TRAIL_EFFECT_GENERIC;
                TrailTile *t = &g->trail[g->trail_count++];
                t->active = 1; t->x = px; t->y = py;
                t->r = 220; t->g = 100; t->b = 20;
                t->is_impact = 1;
            } else if (trap_type == TILE_TRAP_POISON) {
                g->player.poison_turns = 3;
                snprintf(msg, sizeof(msg), "Poison trap! 3 turns");
                // Green flash
                g->trail_count  = 0;
                g->trail_frames = 4;
                g->trail_effect = TRAIL_EFFECT_GENERIC;
                TrailTile *t = &g->trail[g->trail_count++];
                t->active = 1; t->x = px; t->y = py;
                t->r = 40; t->g = 180; t->b = 40;
                t->is_impact = 1;
            }
            push_message(g, msg);
        }

        // Apply poison damage each turn
        if (g->player.poison_turns > 0) {
            int dmg = 3;
            g->player.hp -= dmg;
            g->player.poison_turns--;
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Poison! -%d HP (%d left)",
                dmg, g->player.poison_turns);
            push_message(g, msg);
        }
    }
}

static int enemy_position_occupied(const GameState *g, int skip, int x, int y) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (i == skip || !g->enemies[i].active) continue;
        if (g->enemies[i].x == x && g->enemies[i].y == y) return 1;
    }
    return 0;
}

static int enemy_move_toward(GameState *g, int index) {
    Enemy *e = &g->enemies[index];
    int dx = g->player.x - e->x;
    int dy = g->player.y - e->y;
    int mx = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    int my = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
    int tx = e->x + mx;
    int ty = e->y + my;
    int in_bounds = tx >= 0 && tx < MAP_W && ty >= 0 && ty < MAP_H;

    if (in_bounds && map_is_walkable(&g->map, tx, ty) &&
        !enemy_position_occupied(g, index, tx, ty) &&
        !(tx == g->player.x && ty == g->player.y)) {
        e->x = tx;
        e->y = ty;
        return 1;
    }
    return 0;
}

static int clear_orthogonal_path(const GameState *g, const Enemy *e) {
    int dx = g->player.x - e->x;
    int dy = g->player.y - e->y;
    if (dx != 0 && dy != 0) return 0;
    int distance = abs_int(dx) + abs_int(dy);
    if (distance < 2 || distance > 6) return 0;
    int sx = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    int sy = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
    for (int step = 1; step < distance; step++) {
        int x = e->x + sx * step;
        int y = e->y + sy * step;
        if (!map_is_walkable(&g->map, x, y)) return 0;
    }
    return 1;
}

static int necromancer_revive(GameState *g, int necromancer_index) {
    Enemy *caster = &g->enemies[necromancer_index];
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *dead = &g->enemies[i];
        if (dead->active || dead->type != ENEMY_SKELETON) continue;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                int x = caster->x + dx;
                int y = caster->y + dy;
                if (!map_is_walkable(&g->map, x, y) ||
                    enemy_position_occupied(g, i, x, y) ||
                    (x == g->player.x && y == g->player.y)) continue;
                dead->x = x;
                dead->y = y;
                dead->active = 1;
                dead->hp = dead->max_hp;
                push_message(g, "Crypt Conjurer raises a Skeleton!");
                return 1;
            }
        }
    }
    return 0;
}

static int forest_necromancer_raise(GameState *g, int caster_index) {
    Enemy *caster = &g->enemies[caster_index];
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *dead = &g->enemies[i];
        if (dead->active || dead->is_boss ||
            dead->type < ENEMY_PIXIE ||
            dead->type > ENEMY_FOREST_TROLL) continue;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                int x = caster->x + dx;
                int y = caster->y + dy;
                if (!map_is_walkable(&g->map, x, y) ||
                    enemy_position_occupied(g, i, x, y) ||
                    (x == g->player.x && y == g->player.y)) continue;
                dead->x = x;
                dead->y = y;
                dead->active = 1;
                dead->hp = dead->max_hp / 2;
                if (dead->hp < 1) dead->hp = 1;
                push_message(g, "Necromancer recalls a fallen servant!");
                return 1;
            }
        }
    }
    return 0;
}

void action_resolve_enemies(GameState *g) {
    int boss_locked = 0;
    if (g->location == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH) {
        for (int y = 0; y < MAP_H && !boss_locked; y++)
            for (int x = 0; x < MAP_W; x++)
                if (g->map.tiles[y][x] == TILE_LOCKED_DOOR) {
                    boss_locked = 1;
                    break;
                }
    }

    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (!e->active) continue;
        if (e->is_boss && boss_locked) continue;

        if (e->type == ENEMY_LICH_KING) {
            Room *chamber = &g->map.rooms[g->map.room_count - 1];
            int player_in_chamber =
                g->player.x > chamber->x &&
                g->player.x < chamber->x + chamber->w - 1 &&
                g->player.y > chamber->y &&
                g->player.y < chamber->y + chamber->h - 1;
            // Unlocking the door does not pull the boss into the corridor.
            // The encounter begins only when the player crosses the threshold.
            if (!player_in_chamber && e->move_timer == 0) continue;
        }
        if (e->type == ENEMY_FOREST_NECROMANCER) {
            Room *grove = &g->map.rooms[g->map.room_count - 1];
            int player_in_grove =
                g->player.x >= grove->x &&
                g->player.x < grove->x + grove->w &&
                g->player.y >= grove->y &&
                g->player.y < grove->y + grove->h;
            if (!player_in_grove && e->move_timer == 0) continue;
        }
        if (e->type == ENEMY_MOUNTAIN_GOBLIN_KING) {
            Room *fortress = &g->map.rooms[g->map.room_count - 1];
            int player_in_fortress =
                g->player.x >= fortress->x &&
                g->player.x < fortress->x + fortress->w &&
                g->player.y >= fortress->y &&
                g->player.y < fortress->y + fortress->h;
            if (!player_in_fortress && e->move_timer == 0) continue;
        }

        int dx = g->player.x - e->x;
        int dy = g->player.y - e->y;

        // Adjacent to player — melee attack
        if (abs_int(dx) <= 1 && abs_int(dy) <= 1 &&
            !(dx == 0 && dy == 0)) {
                int defense = g->player.defense;
                if (e->type == ENEMY_WRAITH) defense /= 2;
                int dmg = e->attack - defense;
                if (dmg < 1) dmg = 1;
                g->player.hp -= dmg;
                if (e->type == ENEMY_GIANT_SPIDER ||
                    e->type == ENEMY_TUNNEL_SPIDER) {
                    g->player.poison_turns = 3;
                    push_message(g, "Giant Spider venom poisons you!");
                }
                if (e->type == ENEMY_WRAITH && g->player.mp > 0) {
                    int drained = g->player.mp < 3 ? g->player.mp : 3;
                    g->player.mp -= drained;
                }
                char msg[MAX_MESSAGE_LEN];
                if (e->type == ENEMY_WRAITH)
                    snprintf(msg, sizeof(msg), "Wraith: %d dmg, drains MP", dmg);
                else
                    snprintf(msg, sizeof(msg), "%s: %d dmg", e->name, dmg);
                push_message(g, msg);
                continue;
        }

        e->move_timer++;

        if (e->type == ENEMY_LICH_KING) {
            // The Lich holds the center of his chamber and alternates a ranged
            // necrotic attack with a telegraphed recovery turn. This prevents
            // ordinary pathfinding from walking him out of his own arena.
            if (e->move_timer % 2 == 0) {
                int dmg = e->attack - g->player.defense / 2;
                if (dmg < 4) dmg = 4;
                g->player.hp -= dmg;
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Lich necrotic bolt: %d dmg", dmg);
                push_message(g, msg);
            } else {
                push_message(g, "The Lich gathers dark power...");
            }
            continue;
        }

        if (e->type == ENEMY_FOREST_NECROMANCER) {
            if (e->move_timer % 4 == 0 &&
                forest_necromancer_raise(g, i)) continue;
            if (e->move_timer % 2 == 0) {
                int dmg = e->attack - g->player.defense / 2;
                if (dmg < 3) dmg = 3;
                g->player.hp -= dmg;
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Necromancer spirit bolt: %d dmg", dmg);
                push_message(g, msg);
            } else {
                push_message(g, "The Necromancer invokes the forest...");
            }
            continue;
        }

        if (e->type == ENEMY_MOUNTAIN_GOBLIN_KING) {
            if (e->move_timer % 2 == 0) {
                int dmg = e->attack - g->player.defense / 2;
                if (dmg < 4) dmg = 4;
                g->player.hp -= dmg;
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Goblin King axe: %d dmg", dmg);
                push_message(g, msg);
            } else push_message(g, "The Goblin King raises his axe...");
            continue;
        }

        if ((e->type == ENEMY_GOBLIN_ARCHER ||
            e->type == ENEMY_GOBLIN_BOMBER) &&
            e->move_timer % 2 == 0 && clear_orthogonal_path(g, e)) {
            int dmg = e->attack - g->player.defense / 2;
            if (dmg < 1) dmg = 1;
            g->player.hp -= dmg;
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), e->type == ENEMY_GOBLIN_BOMBER
                ? "Goblin bomb: %d dmg" : "Goblin arrow: %d dmg", dmg);
            push_message(g, msg);
            continue;
        }

        if (e->type == ENEMY_GOBLIN_SHAMAN && e->move_timer % 3 == 0) {
            int healed = 0;
            for (int j = 0; j < g->enemy_count; j++) {
                Enemy *ally = &g->enemies[j];
                if (ally->active && ally->hp < ally->max_hp &&
                    abs_int(ally->x - e->x) <= 4 &&
                    abs_int(ally->y - e->y) <= 4) {
                    ally->hp += 6;
                    if (ally->hp > ally->max_hp) ally->hp = ally->max_hp;
                    healed = 1;
                    push_message(g, "Goblin Shaman heals an ally!");
                    break;
                }
            }
            if (healed) continue;
        }

        if (e->type == ENEMY_DARK_ELF && e->move_timer % 2 == 0 &&
            clear_orthogonal_path(g, e)) {
            int dmg = e->attack - g->player.defense / 2;
            if (dmg < 1) dmg = 1;
            g->player.hp -= dmg;
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Dark Elf arrow: %d dmg", dmg);
            push_message(g, msg);
            continue;
        }

        if (e->type == ENEMY_CRYPT_CONJURER) {
            if (e->move_timer % 4 == 0 && necromancer_revive(g, i)) continue;
            if (e->move_timer % 2 == 0 && clear_orthogonal_path(g, e)) {
                int dmg = e->attack - g->player.defense / 2;
                if (dmg < 1) dmg = 1;
                g->player.hp -= dmg;
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Conjurer bolt: %d dmg", dmg);
                push_message(g, msg);
                continue;
            }
        }

        if (e->type == ENEMY_ZOMBIE || e->type == ENEMY_GIANT_WURM ||
            e->type == ENEMY_FOREST_TROLL || e->type == ENEMY_CAVE_TROLL) {
            if (e->move_timer % 2 != 0) continue;
        }

        int moved = enemy_move_toward(g, i);
        if (e->type == ENEMY_CRYPT_BAT && moved) {
            // Bats close distance quickly, but never attack on their second move.
            enemy_move_toward(g, i);
        }
        if ((e->type == ENEMY_PIXIE || e->type == ENEMY_BLIGHTED_WOLF) && moved)
            enemy_move_toward(g, i);
    }
}
