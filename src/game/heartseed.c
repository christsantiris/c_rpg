#include "heartseed.h"
#include "elderwood.h"
#include <stdlib.h>
#include <string.h>

const int heartseed_stakes[3][2] = {{49, 30}, {57, 30}, {53, 37}};

static int near(const GameState *g, int x, int y) {
    return abs(g->player.x - x) + abs(g->player.y - y) <= 1;
}

static TileType object_tile(const GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            const FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == x && item->y == y) {
                return item->underlying_tile;
            }
        }
    }
    if (tile == TILE_PORTAL && g->portal_location == g->location && g->portal_level == g->level) {
        return g->portal_origin_tile;
    }
    return tile;
}

static void put_object(GameState *g, int x, int y, TileType tile) {
    if (g->map.tiles[y][x] == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == x && item->y == y) {
                item->underlying_tile = tile;
            }
        }
    } else if (g->map.tiles[y][x] == TILE_PORTAL && g->portal_location == g->location && g->portal_level == g->level) {
        g->portal_origin_tile = tile;
    } else {
        g->map.tiles[y][x] = tile;
    }
}

static void sync_state(GameState *g) {
    if (g->heartseed_quest_state == 1 && g->heartseed_progress == 7) {
        g->heartseed_quest_state = 2;
        push_message(g, "The heartseed and orders are safe. Return to Forester Tamsin in Oakhaven.");
    }
}

static int occupied(const GameState *g, int x, int y) {
    if (g->player.x == x && g->player.y == y) {
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 1;
        }
    }
    return g->map.tiles[y][x] != TILE_ELDERWOOD_FLOOR;
}

static int add_guard(GameState *g, EnemyType type, const char *name, int tx, int ty) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (strcmp(g->enemies[i].name, name) == 0) {
            return 1;
        }
    }
    int slot = g->enemy_count;
    if (slot >= AREA_ENEMY_LIMIT) {
        for (int i = 0; i < g->enemy_count; i++) {
            if (!g->enemies[i].active && strncmp(g->enemies[i].name, "Heartseed ", 10) != 0 &&
                strcmp(g->enemies[i].name, "Bound Treefolk") != 0) {
                slot = i;
                break;
            }
        }
    }
    if (slot >= AREA_ENEMY_LIMIT) {
        return 0;
    }
    for (int radius = 0; radius < 5; radius++) {
        for (int y = ty - radius; y <= ty + radius; y++) {
            for (int x = tx - radius; x <= tx + radius; x++) {
                if (x <= 0 || y <= 0 || x >= ELDERWOOD_W - 1 || y >= ELDERWOOD_H - 1 || occupied(g, x, y)) {
                    continue;
                }
                Enemy *e = &g->enemies[slot];
                memset(e, 0, sizeof(*e));
                game_spawn_enemy(g, e, type, x, y);
                snprintf(e->name, sizeof(e->name), "%s", name);
                if (type == ENEMY_TREEFOLK) {
                    e->hp = 113;
                    e->max_hp = 113;
                    e->attack = 18;
                    e->defense = 8;
                    e->experience = 106;
                    g->heartseed_guardian = HEARTSEED_BOUND;
                }
                if (slot == g->enemy_count) {
                    g->enemy_count++;
                }
                g->level_cleared = 0;
                return 1;
            }
        }
    }
    return 0;
}

void heartseed_prepare(GameState *g) {
    if (g->location != LOCATION_ELDERWOOD || (g->level != 2 && g->level != 4)) {
        return;
    }
    if (g->level == 2) {
        put_object(g, HEARTSEED_ORDERS_X, HEARTSEED_ORDERS_Y, TILE_HEARTSEED_ORDERS);
        if (!(g->heartseed_sites & 1) &&
            add_guard(g, ENEMY_DARK_ELF, "Heartseed Lookout", 10, 10) &&
            add_guard(g, ENEMY_DARK_ELF, "Heartseed Sentry", 14, 10)) {
            g->heartseed_sites |= 1;
        }
        return;
    }
    // Extend only the eastern clearing, retaining cut paths, logs, actors and loot.
    if (!(g->heartseed_sites & 2)) {
        for (int y = 28; y <= 40; y++) {
            for (int x = 47; x <= 59; x++) {
                if (g->map.tiles[y][x] == TILE_ELDERWOOD_WALL) {
                    g->map.tiles[y][x] = TILE_ELDERWOOD_FLOOR;
                }
            }
        }
        // A bramble passage beside the permanent southern route offers a flank.
        for (int x = 53; x <= 58; x++) {
            if (g->map.tiles[25][x] == TILE_ELDERWOOD_WALL) {
                g->map.tiles[25][x] = TILE_ELDERWOOD_FLOOR;
            }
        }
        for (int y = 26; y <= 28; y++) {
            if (g->map.tiles[y][58] == TILE_ELDERWOOD_WALL) {
                g->map.tiles[y][58] = TILE_ELDERWOOD_BRAMBLE;
            }
        }
        if ((!g->heartseed_guardian && !add_guard(g, ENEMY_TREEFOLK, "Bound Treefolk", 53, 32)) ||
            !add_guard(g, ENEMY_DARK_ELF, "Heartseed Captor", 48, 34) ||
            !add_guard(g, ENEMY_DARK_ELF, "Heartseed Binder", 57, 35)) {
            return;
        }
        g->heartseed_sites |= 2;
    }
    for (int i = 0; i < 3; i++) {
        put_object(g, heartseed_stakes[i][0], heartseed_stakes[i][1],
            g->heartseed_bindings & (1 << i) ? TILE_HEARTSEED_CUT_STAKE : TILE_HEARTSEED_STAKE);
    }
    put_object(g, HEARTSEED_X, HEARTSEED_Y, TILE_HEARTSEED_ALTAR);
}

void heartseed_migrate(GameState *g) {
    if (g->location == LOCATION_TOWN) {
        // Keep an old save's player and dropped items beside the new resident.
        if (g->player.x == TAMSIN_X && g->player.y == TAMSIN_Y) {
            g->player.x++;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == TAMSIN_X && item->y == TAMSIN_Y) {
                item->x++;
                item->underlying_tile = TILE_TOWN_FLOOR;
                g->map.tiles[item->y][item->x] = TILE_ITEM;
            }
        }
        g->map.tiles[TAMSIN_Y][TAMSIN_X] = TILE_NPC_TAMSIN;
    }
    // Unvisited or cached forest stages receive their objects when unpacked.
    heartseed_prepare(g);
}

int heartseed_talk(GameState *g) {
    if (g->location != LOCATION_TOWN || abs(g->player.x - TAMSIN_X) > 1 || abs(g->player.y - TAMSIN_Y) > 1) {
        return 0;
    }
    g->dialogue_active = 1;
    g->dialogue_x = TAMSIN_X;
    g->dialogue_y = TAMSIN_Y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Forester Tamsin");
    if (!g->heartseed_quest_state) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Dark Elves stole Elderwood's heartseed and bound its Treefolk guardian. Take the north road. Find their orders in the stage 2 western camp and the golden-root grove in stage 4's eastern clearing. Cut its three bindings, recover the seed and bring both seed and orders to me for 120 gold.");
    } else if (g->heartseed_quest_state == 1) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Orders in Elderwood 2: %s. Bindings cut: %d/3. Heartseed: %s. The grove is in stage 4's eastern clearing, beneath a golden root arch. Cutting every stake frees its Treefolk; defeating it also leaves the seed recoverable. Bring the orders and seed back here.",
            g->heartseed_progress & HEARTSEED_ORDERS ? "found" : "missing",
            !!(g->heartseed_bindings & 1) + !!(g->heartseed_bindings & 2) + !!(g->heartseed_bindings & 4),
            g->heartseed_progress & HEARTSEED_TAKEN ? "safe" : "missing");
    } else if (g->heartseed_quest_state == 2) {
        g->heartseed_quest_state = 3;
        g->gold += HEARTSEED_REWARD_GOLD;
        g->score += HEARTSEED_REWARD_SCORE;
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "The heartseed is safe, and these orders reveal who bound the grove. I will plant it where the forest can recover. Take 120 gold with Oakhaven's thanks.");
        push_message(g, "Completed: The Stolen Heartseed. +120 gold, +800 score.");
    } else {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text), "The heartseed is growing again. Elderwood remembers your kindness.");
    }
    return 1;
}

void heartseed_accept(GameState *g) {
    sync_state(g);
}

const char *heartseed_hint(const GameState *g) {
    if (g->location != LOCATION_ELDERWOOD) {
        return NULL;
    }
    if (g->level == 2 && near(g, HEARTSEED_ORDERS_X, HEARTSEED_ORDERS_Y) &&
        object_tile(g, HEARTSEED_ORDERS_X, HEARTSEED_ORDERS_Y) == TILE_HEARTSEED_ORDERS) {
        return "A: READ DARK ELF ORDERS";
    }
    if (g->level != 4) {
        return NULL;
    }
    for (int i = 0; i < 3; i++) {
        if (!(g->heartseed_bindings & (1 << i)) && near(g, heartseed_stakes[i][0], heartseed_stakes[i][1]) &&
            object_tile(g, heartseed_stakes[i][0], heartseed_stakes[i][1]) == TILE_HEARTSEED_STAKE) {
            return "A: CUT ROOT BINDING";
        }
    }
    if (near(g, HEARTSEED_X, HEARTSEED_Y) && !(g->heartseed_progress & HEARTSEED_TAKEN) &&
        object_tile(g, HEARTSEED_X, HEARTSEED_Y) == TILE_HEARTSEED_ALTAR) {
        return g->heartseed_bindings == 7 && g->heartseed_guardian >= HEARTSEED_SLAIN ?
            "A: RECOVER HEARTSEED" : "A: EXAMINE BOUND HEARTSEED";
    }
    return NULL;
}

int heartseed_is_guardian(const GameState *g, const Enemy *e) {
    return g->location == LOCATION_ELDERWOOD && g->level == 4 &&
        e->type == ENEMY_TREEFOLK && strcmp(e->name, "Bound Treefolk") == 0;
}

void heartseed_record_death(GameState *g, const Enemy *e) {
    if (heartseed_is_guardian(g, e) && g->heartseed_guardian == HEARTSEED_BOUND) {
        g->heartseed_guardian = HEARTSEED_SLAIN;
        push_message(g, "The bound Treefolk falls. Cut the remaining stakes to release the heartseed.");
    }
}

int heartseed_interact(GameState *g) {
    if (!heartseed_hint(g)) {
        return 0;
    }
    heartseed_prepare(g);
    if (g->level == 2) {
        g->heartseed_progress |= HEARTSEED_ORDERS;
        push_message(g, "Dark Elf orders: carry the heartseed to Elderwood 4's eastern golden-root grove. Bind its Treefolk with three stakes.");
        sync_state(g);
        return 1;
    }
    for (int i = 0; i < 3; i++) {
        if (!(g->heartseed_bindings & (1 << i)) && near(g, heartseed_stakes[i][0], heartseed_stakes[i][1])) {
            g->heartseed_bindings |= 1 << i;
            put_object(g, heartseed_stakes[i][0], heartseed_stakes[i][1], TILE_HEARTSEED_CUT_STAKE);
            push_message(g, "You sever a thorn binding. Its violet light fades.");
            if (g->heartseed_bindings == 7) {
                g->heartseed_progress |= HEARTSEED_BINDINGS;
                if (g->heartseed_guardian == HEARTSEED_BOUND) {
                    for (int n = 0; n < g->enemy_count; n++) {
                        Enemy *e = &g->enemies[n];
                        if (e->active && heartseed_is_guardian(g, e)) {
                            e->active = 0;
                            e->attack_phase = 0;
                            e->move_timer = 0;
                            g->heartseed_guardian = HEARTSEED_FREED;
                            player_gain_xp(g, e->experience);
                            push_message(g, "The Treefolk is free! It bows and returns to the woods. The heartseed is released.");
                            game_update_level_progress(g);
                            break;
                        }
                    }
                } else {
                    push_message(g, "The final binding falls. The heartseed is released.");
                }
            }
            return 1;
        }
    }
    if (g->heartseed_bindings != 7 || g->heartseed_guardian < HEARTSEED_SLAIN) {
        push_message(g, "Three thorn bindings hold the heartseed. Cut the marked stakes around the grove.");
        return 1;
    }
    g->heartseed_progress |= HEARTSEED_TAKEN;
    push_message(g, "You recover the living heartseed. It is kept safely with your quest discoveries.");
    sync_state(g);
    return 1;
}
