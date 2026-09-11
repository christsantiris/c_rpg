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

Item random_enemy_item(int level) {
    int roll = rand() % 100;
    if (level <= 3) {
        if (roll < 40) {
            return item_make_health_potion();
        }
        if (roll < 70) {
            return item_make_mana_potion();
        }
        if (roll < 90) {
            return item_make_scroll_magic_arrow();
        }
        return item_make_scroll_heal();
    }
    if (level <= 6) {
        if (roll < 35) {
            return item_make_health_potion();
        }
        if (roll < 65) {
            return item_make_mana_potion();
        }
        if (roll < 82) {
            return item_make_scroll_magic_arrow();
        }
        if (roll < 93) {
            return item_make_scroll_heal();
        }
        return item_make_scroll_fireball();
    }
    if (roll < 30) {
        return item_make_health_potion();
    }
    if (roll < 55) {
        return item_make_mana_potion();
    }
    if (roll < 63) {
        return item_make_scroll_magic_arrow();
    }
    if (roll < 82) {
        return item_make_scroll_heal();
    }
    return item_make_scroll_fireball();
}

static void mark_item_tile(GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile != TILE_MOUNTAIN_WEAK_BRIDGE && tile != TILE_MOUNTAIN_CACHE &&
        !map_is_coast_tidal_tile(tile) && !map_is_coast_object(tile)) {
        g->map.tiles[y][x] = TILE_ITEM;
    }
}

Item boss_equipment_reward(EnemyType type) {
    switch (type) {
        case ENEMY_LICH_KING:
            return item_make_cryptblade();
        case ENEMY_FOREST_NECROMANCER:
            return item_make_necromancer_cloak();
        case ENEMY_MOUNTAIN_GOBLIN_KING:
        case ENEMY_GOBLIN_KING:
            return item_make_goblin_king_shield();
        case ENEMY_DROWNED_QUEEN:
            return item_make_tidecaller_robes();
        default:
            return item_make_cryptblade();
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
        case ENEMY_ILLUSION: return 30;
        case ENEMY_MERFOLK: return 55;
        case ENEMY_SIREN: return 80;
        case ENEMY_GIANT_CRAB: return 100;
        case ENEMY_ANIMATED_STATUE: return 145;
        case ENEMY_WATER_ELEMENTAL: return 135;
        case ENEMY_SEA_SERPENT: return 180;
        case ENEMY_DROWNED_QUEEN: return 1600;
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

static void drop_loot(GameState *g, Enemy *enemy) {
    int x = enemy->x;
    int y = enemy->y;
    EnemyType type = enemy->type;
    int is_boss = enemy->is_boss;
    if (enemy->dain_fragment) {
        game_record_dain_kill(g, type);
    }
    if (is_boss) {
        g->defeated_bosses |= 1 << g->location;
    }
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
        case ENEMY_ILLUSION: gold = 2; break;
        case ENEMY_MERFOLK: gold = 5; break;
        case ENEMY_SIREN: gold = 8; break;
        case ENEMY_GIANT_CRAB: gold = 7; break;
        case ENEMY_ANIMATED_STATUE: gold = 12; break;
        case ENEMY_WATER_ELEMENTAL: gold = 10; break;
        case ENEMY_SEA_SERPENT: gold = 15; break;
        case ENEMY_DROWNED_QUEEN: gold = 75; break;
        case ENEMY_ORC:      gold = 6 + rand() % 8;  break;
        case ENEMY_TROLL:    gold = 10 + rand() % 10; break;
        case ENEMY_GIANT:    gold = 15 + rand() % 15; break;
        case ENEMY_GOBLIN_KING: break;
        case ENEMY_LICH_KING:  break;
        case ENEMY_DEMON_LORD:  break;
        case ENEMY_RED_DRAGON: break;
        case ENEMY_TARRASQUE:  break;
    }
    
    // Preserve the existing coin chance and values alongside the new loot
    // table; quest rewards remain the larger source of purchasing power.
    if (is_boss || rand() % 100 < 25) {
        g->gold += gold;
        g->score += gold;
        char msg[MAX_MESSAGE_LEN];
        snprintf(msg, sizeof(msg), "Found %d gold!", gold);
        push_message(g, msg);
    }

    // Each boss leaves a fixed regional reward instead of rolling ordinary
    // equipment, so capstone weapons remain Blacksmith progression.
    if (is_boss) {
        if (g->floor_item_count >= MAX_FLOOR_ITEMS) {
            FloorItem *discarded = &g->floor_items[MAX_FLOOR_ITEMS - 1];
            if (g->map.tiles[discarded->y][discarded->x] == TILE_ITEM) {
                g->map.tiles[discarded->y][discarded->x] =
                    discarded->underlying_tile;
            }
            g->floor_item_count--;
        }
        if (g->floor_item_count < MAX_FLOOR_ITEMS) {
            Item boss_drop = boss_equipment_reward(type);
            FloorItem fi = {0};
            fi.active = 1;
            fi.x = x;
            fi.y = y;
            fi.underlying_tile = g->map.tiles[y][x];
            fi.item = boss_drop;
            mark_item_tile(g, x, y);
            g->floor_items[g->floor_item_count++] = fi;
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "%s dropped!", boss_drop.name);
            push_message(g, msg);
        }
        return;
    }

    // Item drop — 5% chance
    if (rand() % 100 >= 5) {
        return;
    }
    if (g->floor_item_count >= MAX_FLOOR_ITEMS) {
        return;
    }

    Item item = random_enemy_item(g->level);

    FloorItem fi = {0};
    fi.active = 1;
    fi.x = x;
    fi.y = y;
    fi.underlying_tile = g->map.tiles[y][x];
    fi.item = item;
    g->floor_items[g->floor_item_count++] = fi;

    mark_item_tile(g, x, y);
    char item_msg[MAX_MESSAGE_LEN];
    snprintf(item_msg, sizeof(item_msg), "%s dropped!", item.name);
    push_message(g, item_msg);
}

static int apply_melee_cleave(GameState *g, Enemy *target, int attack, int percent) {
    int hits = 0;
    int defeated = 0;

    if (percent <= 0) {
        return 0;
    }

    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *enemy = &g->enemies[i];
        if (!enemy->active || enemy == target) {
            continue;
        }
        if (abs(enemy->x - target->x) > 1 ||
            abs(enemy->y - target->y) > 1) {
            continue;
        }

        int damage = attack * percent / 100 - enemy->defense;
        if (damage < 1) {
            damage = 1;
        }
        enemy->hp -= damage;
        hits++;
        if (enemy->hp <= 0) {
            enemy->active = 0;
            drop_loot(g, enemy);
            player_gain_xp(g, enemy->experience);
            defeated = 1;
        }
    }

    if (defeated) {
        game_update_level_progress(g);
    }
    return hits;
}

static int equipped_spell_power(const GameState *g) {
    if (g->equipped_main_hand < 0 ||
        g->equipped_main_hand >= g->inventory_count) {
        return 0;
    }
    return g->inventory[g->equipped_main_hand].spell_power_bonus;
}

static const Item *equipped_armor(const GameState *g) {
    if (g->equipped_armor < 0 ||
        g->equipped_armor >= g->inventory_count) {
        return NULL;
    }
    return &g->inventory[g->equipped_armor];
}

static int equipped_spell_cost(const GameState *g, const Spell *spell) {
    const Item *armor = equipped_armor(g);
    if (spell->mp_cost == 0) {
        return spell->mp_cost;
    }
    int reduction = armor ? armor->spell_cost_reduction_percent : 0;
    if (g->equipped_main_hand >= 0 &&
        g->equipped_main_hand < g->inventory_count) {
        reduction += g->inventory[g->equipped_main_hand]
            .spell_cost_reduction_percent;
    }
    if (reduction > 50) {
        reduction = 50;
    }
    int cost = spell->mp_cost * (100 - reduction) / 100;
    return cost < 1 ? 1 : cost;
}

static int apply_enemy_damage(GameState *g, int damage) {
    const Item *armor = equipped_armor(g);
    if (armor && armor->evasion_chance > 0 &&
        rand() % 100 < armor->evasion_chance) {
        push_message(g, "Dodged!");
        return 0;
    }
    if (g->equipped_off_hand >= 0 &&
        g->equipped_off_hand < g->inventory_count) {
        const Item *shield = &g->inventory[g->equipped_off_hand];
        if (shield->type == ITEM_SHIELD && shield->block_chance > 0 &&
            rand() % 100 < shield->block_chance) {
            damage = (damage * (100 - shield->block_reduction_percent) + 99)
                / 100;
            if (damage < 1) {
                damage = 1;
            }
            push_message(g, "Blocked!");
        }
    }
    g->player.hp -= damage;
    return damage;
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

static int reveal_adjacent_dungeon_traps(GameState *g) {
    if (g->location != LOCATION_DUNGEON) {
        return 0;
    }
    int revealed = 0;
    for (int y = g->player.y - 1; y <= g->player.y + 1; y++) {
        for (int x = g->player.x - 1; x <= g->player.x + 1; x++) {
            if (x == g->player.x && y == g->player.y) {
                continue;
            }
            if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
                continue;
            }
            if (g->map.tiles[y][x] == TILE_TRAP_HIDDEN) {
                g->map.tiles[y][x] = TILE_TRAP_REVEALED;
                revealed++;
            }
        }
    }
    return revealed;
}

static void change_mountain_tile(GameState *g, int x, int y, TileType tile) {
    g->map.tiles[y][x] = tile;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = tile;
        }
    }
    map_mark_explored(&g->map, x, y);
}

static int mountain_obstacle(TileType tile) {
    return tile == TILE_MOUNTAIN_GATE || tile == TILE_MOUNTAIN_ROCKFALL ||
        tile == TILE_MOUNTAIN_CHASM;
}

int game_has_regional_interaction(const GameState *g) {
    if (g->location == LOCATION_COAST) {
        return map_is_coast_object(g->map.tiles[g->player.y][g->player.x]);
    }
    if (g->location != LOCATION_MOUNTAINS) {
        return 0;
    }
    if (g->map.tiles[g->player.y][g->player.x] == TILE_MOUNTAIN_CACHE) {
        return 1;
    }
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < 4; i++) {
        int x = g->player.x + offsets[i][0];
        int y = g->player.y + offsets[i][1];
        if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
            mountain_obstacle(g->map.tiles[y][x])) {
            return 1;
        }
    }
    return 0;
}

static int interact_mountain(GameState *g) {
    if (g->location != LOCATION_MOUNTAINS) {
        return 0;
    }
    int px = g->player.x;
    int py = g->player.y;
    if (g->map.tiles[py][px] == TILE_MOUNTAIN_CACHE) {
        int gold = 40 + g->level * 10;
        g->gold += gold;
        g->score += gold;
        change_mountain_tile(g, px, py, TILE_MOUNTAIN_CAVE_FLOOR);
        push_message(g, "You recover the buried goblin hoard!");
        return 1;
    }
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < 4; i++) {
        int x = px + offsets[i][0];
        int y = py + offsets[i][1];
        if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
            continue;
        }
        TileType tile = g->map.tiles[y][x];
        if (!mountain_obstacle(tile)) {
            continue;
        }
        if (tile == TILE_MOUNTAIN_GATE) {
            change_mountain_tile(g, x, y, TILE_MOUNTAIN_FORTRESS_FLOOR);
            push_message(g, "The goblin gate opens. Watch for traps!");
        } else if (tile == TILE_MOUNTAIN_CHASM) {
            change_mountain_tile(g, x, y, TILE_MOUNTAIN_BRIDGE);
            push_message(g, "You lash a new crossing into place.");
        } else {
            // Each generated fort has a straight buried tunnel with two ends.
            int left = x;
            int right = x;
            while (left > 0 && (g->map.tiles[y][left - 1] == TILE_MOUNTAIN_HIDDEN_CAVE ||
                g->map.tiles[y][left - 1] == TILE_MOUNTAIN_ROCKFALL)) {
                left--;
            }
            while (right < MAP_W - 1 && (g->map.tiles[y][right + 1] == TILE_MOUNTAIN_HIDDEN_CAVE ||
                g->map.tiles[y][right + 1] == TILE_MOUNTAIN_ROCKFALL)) {
                right++;
            }
            for (int cx = left; cx <= right; cx++) {
                g->map.tiles[y][cx] = TILE_MOUNTAIN_CAVE_FLOOR;
                map_mark_explored(&g->map, cx, y);
            }
            g->map.tiles[y][(left + right) / 2] = TILE_MOUNTAIN_CACHE;
            g->player.hp -= 4 + g->level;
            push_message(g, "Falling rocks hurt! A cave is exposed.");
        }
        map_mark_explored(&g->map, x, y);
        return 1;
    }
    return 0;
}

static void coast_find_bank(GameState *g, Enemy *enemy) {
    // A rising channel carries its occupants to the nearest unoccupied bank.
    unsigned char seen[MAP_H][MAP_W] = {{0}};
    int queue[MAP_W * MAP_H];
    int head = 0;
    int tail = 0;
    queue[tail++] = enemy->y * MAP_W + enemy->x;
    seen[enemy->y][enemy->x] = 1;
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail) {
        int cell = queue[head++];
        int x = cell % MAP_W;
        int y = cell / MAP_W;
        int occupied = g->player.x == x && g->player.y == y;
        for (int i = 0; i < g->enemy_count && !occupied; i++) {
            Enemy *other = &g->enemies[i];
            occupied = other->active && other != enemy && other->x == x && other->y == y;
        }
        if (map_is_walkable(&g->map, x, y) && !occupied) {
            enemy->x = x;
            enemy->y = y;
            return;
        }
        for (int i = 0; i < 4; i++) {
            int nx = x + offsets[i][0];
            int ny = y + offsets[i][1];
            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H || seen[ny][nx]) {
                continue;
            }
            if (!map_is_walkable(&g->map, nx, ny) && !map_is_coast_tidal_tile(g->map.tiles[ny][nx])) {
                continue;
            }
            seen[ny][nx] = 1;
            queue[tail++] = ny * MAP_W + nx;
        }
    }
}

static void coast_toggle_tide(GameState *g) {
    // Restore water covered by loot in older saves before swapping the basins.
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x >= 0 && item->x < MAP_W && item->y >= 0 &&
            item->y < MAP_H && g->map.tiles[item->y][item->x] == TILE_ITEM &&
            map_is_coast_tidal_tile(item->underlying_tile)) {
            g->map.tiles[item->y][item->x] = item->underlying_tile;
        }
    }
    int blue_drained = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = g->map.tiles[y][x];
            TileType next = map_coast_swapped_tile(tile);
            if (next == tile) {
                continue;
            }
            g->map.tiles[y][x] = next;
            blue_drained |= next == TILE_COAST_DRAINED_WATER;
            // Operating the sluice reveals its connected channels and banks.
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    map_mark_explored(&g->map, x + dx, y + dy);
                }
            }
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            item->underlying_tile = map_coast_swapped_tile(item->underlying_tile);
        }
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *enemy = &g->enemies[i];
        if (enemy->active && map_is_coast_tidal_tile(g->map.tiles[enemy->y][enemy->x]) &&
            !map_is_walkable(&g->map, enemy->x, enemy->y)) {
            coast_find_bank(g, enemy);
        }
    }
    push_message(g, blue_drained ? "Blue channels drain; amber channels rise." :
        "Blue channels rise; amber channels drain.");
}

void action_resolve_player(GameState *g, Action a) {
    game_repair_equipment_indices(g);
    if (a.type == ACTION_NONE) {
        return;
    }

    if (a.type == ACTION_DESCEND) {
        TileType tile = g->map.tiles[g->player.y][g->player.x];
        if (tile == TILE_RETURN_EXIT && g->level_cleared) {
            int forest = g->location == LOCATION_FOREST;
            g->score += g->level * 100;
            game_return_to_town(g);
            push_message(g, forest ? "The forest is freed!" :
                "The Lich is defeated!");
        } else if (tile == TILE_STAIRS_DOWN) {
            if (g->level < DUNGEON_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            }
        }
        return;
    }

    if (a.type == ACTION_ASCEND) {
        if (g->map.tiles[g->player.y][g->player.x] == TILE_STAIRS_UP) {
            if (g->level == 1) {
                game_return_to_town(g);
            } else {
                game_ascend(g);
            }
        }
        return;
    }

    if (a.type == ACTION_INTERACT) {
        if (interact_mountain(g)) {
            return;
        }
        TileType tile = g->map.tiles[g->player.y][g->player.x];
        if (tile == TILE_DUNGEON_SWITCH_OFF) {
            int opened = 0;
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    if (g->map.tiles[y][x] == TILE_DUNGEON_GATE) {
                        g->map.tiles[y][x] = TILE_FLOOR;
                        opened++;
                    }
                }
            }
            g->map.tiles[g->player.y][g->player.x] =
                TILE_DUNGEON_SWITCH_ON;
            push_message(g, opened > 0 ?
                "The portcullis opens a shortcut!" :
                "The switch grinds into place.");
            return;
        }
        if (tile == TILE_DUNGEON_SWITCH_ON) {
            push_message(g, "The switch has already been activated.");
            return;
        }
        if (tile == TILE_CRYPT_CACHE) {
            int gold = 20 + g->level * 5;
            g->gold += gold;
            g->score += gold;
            g->map.tiles[g->player.y][g->player.x] = TILE_FLOOR;
            char message[MAX_MESSAGE_LEN];
            snprintf(message, sizeof(message),
                "The crypt cache holds %d gold!", gold);
            push_message(g, message);
            return;
        }
        if (tile == TILE_BROKEN_BURIAL_SEAL) {
            if (g->elowen_quest_state != 1) {
                push_message(g, "A shattered burial seal lies here.");
                return;
            }
            int seal_index = g->level == 2 ? 0 : (g->level == 4 ? 1 : 2);
            g->elowen_seals_restored |= 1 << seal_index;
            g->map.tiles[g->player.y][g->player.x] =
                TILE_RESTORED_BURIAL_SEAL;
            push_message(g, "Burial seal restored.");
            if ((g->elowen_seals_restored & 7) == 7) {
                g->elowen_quest_state = 2;
                push_message(g, "All seals restored. Return to Elowen.");
            }
            return;
        }
        if (tile == TILE_COAST_BEACON_UNLIT) {
            game_light_coast_beacon(g, g->player.x, g->player.y);
            return;
        }
        if (tile == TILE_COAST_BEACON_LIT) {
            push_message(g, "The beacon already burns brightly.");
            return;
        }
        if (tile == TILE_COAST_TIDE_CONTROL || tile == TILE_COAST_SLUICE_CONTROL) {
            coast_toggle_tide(g);
            return;
        }
        if (tile == TILE_COAST_CACHE) {
            int gold = 50 + g->level * 10;
            g->gold += gold;
            g->score += gold;
            g->map.tiles[g->player.y][g->player.x] = TILE_COAST_FLOOR;
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == g->player.x && item->y == g->player.y) {
                    item->underlying_tile = TILE_COAST_FLOOR;
                    mark_item_tile(g, item->x, item->y);
                }
            }
            push_message(g, "You recover the sunken chamber's hoard!");
            return;
        }
        push_message(g, "There is nothing to interact with here.");
        return;
    }

    if (a.type == ACTION_PICK_UP) {
        if (g->map.tiles[g->player.y][g->player.x] == TILE_CRYPT_KEY) {
            g->dungeon_crypt_keys++;
            g->map.tiles[g->player.y][g->player.x] = TILE_FLOOR;
            push_message(g, "Picked up a crypt key.");
            return;
        }
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
        } else if (item->type == ITEM_SPELL_TOME) {
            if (!item_class_allowed(item, g->player.player_class)) {
                push_message(g, "Only a Mage can study that tome");
                return;
            }
            Spell *known = NULL;
            for (int i = 0; i < g->player.known_spell_count; i++) {
                if (g->player.known_spells[i].id == item->spell_id) {
                    known = &g->player.known_spells[i];
                    break;
                }
            }
            if (!known) {
                push_message(g, "Learn the spell before upgrading it");
                return;
            }
            if (!spell_upgrade(known)) {
                push_message(g, "That spell cannot be upgraded further");
                return;
            }
            snprintf(msg, sizeof(msg), "%s reached rank %d!", known->name,
                known->rank);
            push_message(g, msg);
        } else if (item->type == ITEM_SCROLL) {
            if (!item_class_allowed(item, g->player.player_class)) {
                push_message(g, "Your class cannot learn that spell");
                return;
            }
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
                case SPELL_FROST_BOLT:
                    learned = spell_make_frost_bolt();
                    break;
                case SPELL_TELEPORT: learned = spell_make_teleport(); break;
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

        game_remove_inventory_item(g, idx);
        return;
    }

    if (a.type == ACTION_EQUIP_ITEM) {
        int idx = a.target_x;
        if (idx < 0 || idx >= g->inventory_count) {
            return;
        }
        Item *item = &g->inventory[idx];
        char msg[MAX_MESSAGE_LEN];

        if (item->type == ITEM_WEAPON) {
            if (!game_equip_main_hand(g, idx)) {
                push_message(g, "Your class cannot equip that");
                return;
            }
            snprintf(msg, sizeof(msg), "Equipped %s", item->name);
            push_message(g, msg);
        } else if (item->type == ITEM_ARMOR) {
            if (!item_class_allowed(item, g->player.player_class)) {
                push_message(g, "Your class cannot equip that");
                return;
            }
            if (g->equipped_main_hand == idx) {
                g->equipped_main_hand = -1;
            }
            if (g->equipped_off_hand == idx) {
                g->equipped_off_hand = -1;
            }
            if (g->equipped_armor >= 0 &&
                g->equipped_armor < g->inventory_count) {
                game_remove_armor_bonuses(g,
                    &g->inventory[g->equipped_armor]);
            }
            g->equipped_armor  = idx;
            game_apply_armor_bonuses(g, item);
            snprintf(msg, sizeof(msg), "Equipped %s", item->name);
            push_message(g, msg);
        } else if (item->type == ITEM_SHIELD) {
            if (!game_equip_shield(g, idx)) {
                push_message(g, "Shield requires one-handed weapon");
                return;
            }
            snprintf(msg, sizeof(msg), "Off-hand: %s", item->name);
            push_message(g, msg);
        } else {
            push_message(g, "Cannot equip that item");
        }
        return;
    }
    if (a.type == ACTION_EQUIP_OFF_HAND) {
        int idx = a.target_x;
        if (game_equip_off_hand(g, idx)) {
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), "Off-hand: %s",
                g->inventory[idx].name);
            push_message(g, msg);
        } else {
            push_message(g, "Requires two one-handed weapons");
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

        Item item = g->inventory[idx];

        // Place on floor
        FloorItem fi = {0};
        fi.active = 1;
        fi.x      = g->player.x;
        fi.y      = g->player.y;
        fi.underlying_tile = g->map.tiles[fi.y][fi.x];
        fi.item   = item;
        mark_item_tile(g, fi.x, fi.y);
        g->floor_items[g->floor_item_count++] = fi;

        game_remove_inventory_item(g, idx);

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
        int spell_power = equipped_spell_power(g);
        int mana_cost = equipped_spell_cost(g, sp);

        if (g->player.mp < mana_cost) {
            push_message(g, "Not enough MP!");
            return;
        }

        if (sp->id == SPELL_RETURN_TO_TOWN) {
            if (g->location == LOCATION_TOWN) {
                push_message(g, "Already in town!");
                return;
            }
            game_open_town_portal(g);
            return;
        }

        if (sp->id == SPELL_TELEPORT) {
            int start_x = g->player.x;
            int start_y = g->player.y;
            int destination_x = start_x;
            int destination_y = start_y;
            for (int step = 1; step <= sp->range; step++) {
                int x = start_x + g->player.last_dx * step;
                int y = start_y + g->player.last_dy * step;
                if (!map_is_walkable(&g->map, x, y)) {
                    break;
                }
                int occupied = 0;
                for (int i = 0; i < g->enemy_count; i++) {
                    if (g->enemies[i].active && g->enemies[i].x == x &&
                        g->enemies[i].y == y) {
                        occupied = 1;
                        break;
                    }
                }
                if (occupied) {
                    break;
                }
                destination_x = x;
                destination_y = y;
            }
            if (destination_x == start_x && destination_y == start_y) {
                push_message(g, "Teleport path is blocked");
                return;
            }
            g->player.mp -= mana_cost;
            int distance = abs_int(destination_x - start_x) +
                abs_int(destination_y - start_y);
            set_trail(g, start_x, start_y, destination_x, destination_y,
                g->player.last_dx, g->player.last_dy, distance,
                155, 90, 235, TRAIL_EFFECT_GENERIC);
            g->player.x = destination_x;
            g->player.y = destination_y;
            push_message(g, "Teleported!");
            return;
        }

        if (g->player.last_dx == 0 && g->player.last_dy == 0) {
            push_message(g, "Move first to aim!");
            return;
        }

        g->player.mp -= mana_cost;

        // Set trail based on spell type
        if (sp->type == SPELL_TYPE_DAMAGE_RANGED) {
            #ifndef TEST_BUILD
            if (sp->id == SPELL_MAGIC_ARROW) sfx_play_magic_arrow();
            #endif
            int ex = g->player.x + g->player.last_dx * sp->range;
            int ey = g->player.y + g->player.last_dy * sp->range;
            if (sp->id == SPELL_FROST_BOLT) {
                set_trail(g, g->player.x, g->player.y, ex, ey,
                    g->player.last_dx, g->player.last_dy,
                    sp->range, 120, 225, 255, TRAIL_EFFECT_MAGIC_ARROW);
            } else {
                set_trail(g, g->player.x, g->player.y, ex, ey,
                    g->player.last_dx, g->player.last_dy,
                    sp->range, 40, 120, 220, TRAIL_EFFECT_MAGIC_ARROW);
            }
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
                        int dmg = sp->damage + g->player.level * 2 +
                            spell_power;
                        e->hp -= dmg;
                        char msg[MAX_MESSAGE_LEN];
                        if (e->hp <= 0) {
                            e->active = 0;
                            game_update_level_progress(g);
                            drop_loot(g, e);
                            player_gain_xp(g, e->experience);
                            g->score += enemy_score(e->type);
                            snprintf(msg, sizeof(msg), "%s killed %s!",
                                sp->name, e->name);
                        } else if (sp->id == SPELL_FROST_BOLT) {
                            e->frozen_turns = 2;
                            snprintf(msg, sizeof(msg),
                                "Frost Bolt froze %s: %d dmg", e->name,
                                dmg);
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
            int healed = sp->heal_hp + g->player.level * 2 + spell_power;
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
                    int dmg = sp->damage + g->player.level * 2 + spell_power;
                    e->hp -= dmg;
                    if (e->hp <= 0) {
                        e->active = 0;
                        drop_loot(g, e);
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
        if (g->equipped_main_hand < 0 ||
            g->equipped_main_hand >= g->inventory_count) {
            push_message(g, "No weapon equipped!");
            return;
        }

        Item *wpn = &g->inventory[g->equipped_main_hand];
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
        for (int step = 1;
            step <= wpn->range && (!hit || wpn->pierces_targets); step++) {
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
                    drop_loot(g, e);
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
                if (!wpn->pierces_targets) {
                    impact_x = tx;
                    impact_y = ty;
                }
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
                Item *melee_weapon = NULL;
                if (g->equipped_main_hand >= 0 &&
                    g->equipped_main_hand < g->inventory_count) {
                    melee_weapon = &g->inventory[g->equipped_main_hand];
                    if (melee_weapon->is_ranged) {
                        melee_attack -= melee_weapon->attack_bonus;
                    }
                }
                int effective_defense = e->defense;
                if (melee_weapon &&
                    melee_weapon->armor_penetration_percent > 0) {
                    effective_defense = effective_defense *
                        (100 - melee_weapon->armor_penetration_percent) / 100;
                }
                int dmg = melee_attack - effective_defense;
                if (dmg < 1) {
                    dmg = 1;
                }
                int critical_chance = melee_weapon &&
                    !melee_weapon->is_ranged
                    ? melee_weapon->critical_chance_bonus : 0;
                if (g->equipped_off_hand >= 0 &&
                    g->equipped_off_hand < g->inventory_count) {
                    critical_chance +=
                        g->inventory[g->equipped_off_hand]
                            .critical_chance_bonus / 2;
                }
                int critical = critical_chance > 0 &&
                    rand() % 100 < critical_chance;
                if (critical) {
                    dmg = (dmg * 3 + 1) / 2;
                }
                e->hp -= dmg;
                int cleave_hits = melee_weapon
                    ? apply_melee_cleave(g, e, melee_attack,
                        melee_weapon->cleave_percent)
                    : 0;
                #ifndef TEST_BUILD
                sfx_play_attack();
                #endif
                if (e->hp <= 0) {
                    e->active = 0;
                    drop_loot(g, e);
                    player_gain_xp(g, e->experience);
                    game_update_level_progress(g);
                    char msg[MAX_MESSAGE_LEN];
                    snprintf(msg, sizeof(msg), critical ?
                        "Critical killed %s!" : "Killed %s!", e->name);
                    push_message(g, msg);
                } else {
                    char msg[MAX_MESSAGE_LEN];
                    snprintf(msg, sizeof(msg), critical ?
                        "Critical hit %s: %d dmg" : "Hit %s: %d dmg",
                        e->name, dmg);
                    push_message(g, msg);
                }
                if (cleave_hits > 0) {
                    char msg[MAX_MESSAGE_LEN];
                    snprintf(msg, sizeof(msg), "Cleave struck %d nearby!",
                        cleave_hits);
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
            g->map.tiles[ty][tx] == TILE_TAVERN_DOOR) {
            game_enter_tavern(g);
            return;
        }

        if (g->location == LOCATION_TAVERN &&
            g->map.tiles[ty][tx] == TILE_TAVERN_EXIT) {
            game_leave_tavern(g);
            return;
        }

        if (g->location == LOCATION_TOWN &&
            g->map.tiles[ty][tx] == TILE_TOWN_EXIT) {
            if (tx == 0) {
                game_enter_forest(g);
            } else if (tx == TOWN_W - 1) {
                game_enter_mountains(g);
            } else if (ty == TOWN_H - 1) {
                game_enter_coast(g);
            } else {
                game_enter_dungeon(g);
            }
            return;
        }

        if (g->location == LOCATION_COAST &&
            g->map.tiles[ty][tx] == TILE_COAST_ENTRANCE) {
            if (g->level == 1) {
                game_return_to_town(g);
            } else {
                game_ascend(g);
            }
            return;
        }

        if (g->location == LOCATION_COAST &&
            g->map.tiles[ty][tx] == TILE_COAST_EXIT) {
            if (g->player.x != g->map.stairs_down_x ||
                g->player.y != g->map.stairs_down_y) {
                return;
            }
            if (g->level < COAST_DEPTH) {
                game_descend(g);
                g->score += g->level * 100;
            } else {
                int queen_alive = 0;
                for (int i = 0; i < g->enemy_count; i++) {
                    if (g->enemies[i].active &&
                        g->enemies[i].type == ENEMY_DROWNED_QUEEN) {
                        queen_alive = 1;
                        break;
                    }
                }
                if (queen_alive) {
                    push_message(g, "The Drowned Queen commands the tide!");
                    return;
                }
                g->score += g->level * 100;
                game_return_to_town(g);
                push_message(g, "The Sunken Coast is reclaimed!");
            }
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

        if (g->map.tiles[ty][tx] == TILE_CRYPT_DOOR) {
            if (g->dungeon_crypt_keys < 1) {
                push_message(g, "The crypt is locked. Find its key.");
                return;
            }
            g->dungeon_crypt_keys--;
            g->map.tiles[ty][tx] = TILE_FLOOR;
            push_message(g, "The crypt key unlocks the door!");
        }

        if (g->location == LOCATION_COAST &&
            tx == g->map.stairs_down_x && ty == g->map.stairs_down_y &&
            g->map.tiles[ty][tx] == TILE_COAST_DEEP_WATER) {
            push_message(g, "Lower the tide to reach the exit.");
            return;
        }

        // Move if walkable
        // Track last direction for ranged attacks
        if (map_is_walkable(&g->map, tx, ty)) {
            g->player.last_dx = tx - g->player.x;
            g->player.last_dy = ty - g->player.y;
            int old_x = g->player.x;
            int old_y = g->player.y;
            game_move_player(g, tx - g->player.x, ty - g->player.y);
            if ((old_x != g->player.x || old_y != g->player.y) &&
                g->map.tiles[old_y][old_x] == TILE_MOUNTAIN_WEAK_BRIDGE) {
                change_mountain_tile(g, old_x, old_y, TILE_MOUNTAIN_CHASM);
                push_message(g, "Bridge collapsed! Press A to repair.");
            }
            if (g->map.tiles[g->player.y][g->player.x] == TILE_MOUNTAIN_WEAK_BRIDGE) {
                push_message(g, "The bridge creaks beneath your feet!");
            }
        }
        if (reveal_adjacent_dungeon_traps(g) > 0) {
            push_message(g, "You notice a suspicious pressure plate.");
        }
        // Check for trap on new tile
        int px = g->player.x;
        int py = g->player.y;
        TileType tile = g->map.tiles[py][px];

        if (g->location == LOCATION_FOREST &&
            tile == TILE_FOREST_FALSE_MARKER) {
            g->map.tiles[py][px] = TILE_FOREST_FLOOR;
            push_message(g, "The broken marker points to a dead trail.");
            tile = TILE_FOREST_FLOOR;
        }

        if (g->location == LOCATION_FOREST &&
            tile == TILE_FOREST_LANDMARK) {
            g->map.tiles[py][px] = TILE_FOREST_FLOOR;
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    if (g->map.tiles[y][x] == TILE_FOREST_HIDDEN_TRAIL) {
                        g->map.tiles[y][x] = TILE_FOREST_FLOOR;
                    }
                }
            }
            if (g->map.stairs_down_x == 1) {
                g->map.tiles[g->map.stairs_down_y][0] = TILE_FOREST_EXIT;
            } else if (g->map.stairs_down_x == MAP_W - 2) {
                g->map.tiles[g->map.stairs_down_y][MAP_W - 1] =
                    TILE_FOREST_EXIT;
            } else if (g->map.stairs_down_y == 1) {
                g->map.tiles[0][g->map.stairs_down_x] = TILE_FOREST_EXIT;
            } else {
                g->map.tiles[MAP_H - 1][g->map.stairs_down_x] =
                    TILE_FOREST_EXIT;
            }
        push_message(g, "The landmark reveals hidden trails!");
            tile = TILE_FOREST_FLOOR;
        }

        if (tile == TILE_TRAP_HIDDEN || tile == TILE_TRAP_REVEALED) {
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
        if (e->frozen_turns > 0) {
            e->frozen_turns--;
            continue;
        }

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
        if (e->type == ENEMY_DROWNED_QUEEN) {
            Room *throne = &g->map.rooms[g->map.room_count - 1];
            int player_in_throne =
                g->player.x >= throne->x &&
                g->player.x < throne->x + throne->w &&
                g->player.y >= throne->y &&
                g->player.y < throne->y + throne->h;
            if (!player_in_throne && e->move_timer == 0) {
                continue;
            }
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
                dmg = apply_enemy_damage(g, dmg);
                if (dmg == 0) {
                    continue;
                }
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
                dmg = apply_enemy_damage(g, dmg);
                if (dmg == 0) {
                    continue;
                }
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
                dmg = apply_enemy_damage(g, dmg);
                if (dmg == 0) {
                    continue;
                }
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
                dmg = apply_enemy_damage(g, dmg);
                if (dmg == 0) {
                    continue;
                }
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Goblin King axe: %d dmg", dmg);
                push_message(g, msg);
            } else push_message(g, "The Goblin King raises his axe...");
            continue;
        }

        if (e->type == ENEMY_DROWNED_QUEEN) {
            if (e->move_timer % 3 == 0) {
                int dmg = e->attack - g->player.defense / 2;
                if (dmg < 5) {
                    dmg = 5;
                }
                dmg = apply_enemy_damage(g, dmg);
                if (dmg == 0) {
                    continue;
                }
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Queen's tidal wave: %d dmg", dmg);
                push_message(g, msg);
            } else {
                push_message(g, "The Drowned Queen summons the tide...");
            }
            continue;
        }

        if ((e->type == ENEMY_SIREN ||
            e->type == ENEMY_WATER_ELEMENTAL) &&
            e->move_timer % 2 == 0 && clear_orthogonal_path(g, e)) {
            int dmg = e->attack - g->player.defense / 2;
            if (dmg < 1) {
                dmg = 1;
            }
            dmg = apply_enemy_damage(g, dmg);
            if (dmg == 0) {
                continue;
            }
            char msg[MAX_MESSAGE_LEN];
            snprintf(msg, sizeof(msg), e->type == ENEMY_SIREN
                ? "Siren song: %d dmg" : "Water surge: %d dmg", dmg);
            push_message(g, msg);
            continue;
        }

        if ((e->type == ENEMY_GOBLIN_ARCHER ||
            e->type == ENEMY_GOBLIN_BOMBER) &&
            e->move_timer % 2 == 0 && clear_orthogonal_path(g, e)) {
            int dmg = e->attack - g->player.defense / 2;
            if (dmg < 1) dmg = 1;
            dmg = apply_enemy_damage(g, dmg);
            if (dmg == 0) {
                continue;
            }
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
            dmg = apply_enemy_damage(g, dmg);
            if (dmg == 0) {
                continue;
            }
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
                dmg = apply_enemy_damage(g, dmg);
                if (dmg == 0) {
                    continue;
                }
                char msg[MAX_MESSAGE_LEN];
                snprintf(msg, sizeof(msg), "Conjurer bolt: %d dmg", dmg);
                push_message(g, msg);
                continue;
            }
        }

        if (e->type == ENEMY_ZOMBIE || e->type == ENEMY_GIANT_WURM ||
            e->type == ENEMY_FOREST_TROLL || e->type == ENEMY_CAVE_TROLL ||
            e->type == ENEMY_GIANT_CRAB ||
            e->type == ENEMY_ANIMATED_STATUE) {
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
