#include "jail.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int near(int x, int y, int tx, int ty) {
    return abs(x - tx) <= 1 && abs(y - ty) <= 1;
}

static void dialogue(GameState *g, const char *speaker, const char *text, int x, int y) {
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", speaker);
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s", text);
    g->dialogue_x = x;
    g->dialogue_y = y;
}

void jail_place_castle(Map *m) {
    for (int y = JAIL_Y; y < JAIL_Y + JAIL_H; y++) {
        for (int x = JAIL_X; x < JAIL_X + JAIL_W; x++) {
            m->tiles[y][x] = TILE_JAIL_BUILDING;
        }
    }
    m->tiles[JAIL_DOOR_Y][JAIL_DOOR_X] = TILE_JAIL_DOOR;
    for (int y = CASTLE_ROAD_Y + 1; y <= JAIL_DOOR_Y + 1; y++) {
        if (m->tiles[y][JAIL_X - 1] != TILE_ITEM) {
            m->tiles[y][JAIL_X - 1] = TILE_TOWN_PATH;
        }
    }
    for (int x = JAIL_X - 1; x <= JAIL_DOOR_X; x++) {
        if (m->tiles[JAIL_DOOR_Y + 1][x] != TILE_ITEM) {
            m->tiles[JAIL_DOOR_Y + 1][x] = TILE_TOWN_PATH;
        }
    }
    m->tiles[INFORMANT_Y][INFORMANT_X] = TILE_NPC_INFORMANT;
}

void jail_refresh_hatch(GameState *g) {
    if (g->location != LOCATION_JAIL) {
        return;
    }
    TileType tile = g->jail_quest_state >= 2 ? TILE_JAIL_HATCH : TILE_CASTLE_FLOOR;
    if (g->map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] != TILE_ITEM) {
        g->map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] = tile;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == JAIL_HATCH_X && item->y == JAIL_HATCH_Y) {
            item->underlying_tile = tile;
        }
    }
}

static int new_obstacle(int x, int y) {
    return (x >= JAIL_X && x < JAIL_X + JAIL_W && y >= JAIL_Y && y < JAIL_Y + JAIL_H) ||
        (x == INFORMANT_X && y == INFORMANT_Y);
}

void jail_migrate_castle(GameState *g) {
    if (g->location != LOCATION_CASTLE) {
        return;
    }
    if (new_obstacle(g->player.x, g->player.y)) {
        g->player.x = JAIL_DOOR_X;
        g->player.y = JAIL_DOOR_Y + 1;
    }
    jail_place_castle(&g->map);
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && new_obstacle(item->x, item->y)) {
            item->x = JAIL_DOOR_X;
            item->y = JAIL_DOOR_Y + 1;
            item->underlying_tile = TILE_TOWN_PATH;
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        } else if (item->active && ((item->x == JAIL_X - 1 && item->y > CASTLE_ROAD_Y && item->y <= JAIL_DOOR_Y + 1) ||
            (item->y == JAIL_DOOR_Y + 1 && item->x >= JAIL_X - 1 && item->x <= JAIL_DOOR_X))) {
            item->underlying_tile = TILE_TOWN_PATH;
        }
    }
}

static int equipment(const Item *item) {
    return item->type == ITEM_WEAPON || item->type == ITEM_ARMOR || item->type == ITEM_SHIELD || item->type == ITEM_HELMET || item->type == ITEM_ACCESSORY;
}

static void confiscate_gear(GameState *g) {
    if (g->jail_gear_state) {
        return;
    }
    for (int slot = EQUIPMENT_STRONG_HAND; slot < MAX_CARRIED_ITEMS; slot++) {
        const Item *item = game_equipped_item(g, slot);
        if (!item) {
            continue;
        }
        int index = g->jail_gear_count++;
        g->jail_gear[index] = *item;
        if (slot == EQUIPMENT_STRONG_HAND) {
            g->jail_main_hand = index;
        } else if (slot == EQUIPMENT_WEAK_HAND) {
            g->jail_off_hand = index;
        } else if (slot == EQUIPMENT_ARMOR) {
            g->jail_armor = index;
        } else {
            int extra = slot == EQUIPMENT_HELMET ? 0 : slot - EQUIPMENT_ACCESSORY_1 + 1;
            g->jail_extra_slots[extra] = index;
        }
        game_clear_equipment_slot(g, slot);
    }
    for (int i = 0; i < g->inventory_count; i++) {
        if (equipment(&g->inventory[i])) {
            g->jail_gear[g->jail_gear_count++] = g->inventory[i];
        }
    }
    for (int i = g->inventory_count - 1; i >= 0; i--) {
        if (equipment(&g->inventory[i])) {
            game_remove_inventory_item(g, i);
        }
    }
    if (g->jail_gear_count) {
        g->jail_gear_state = 1;
        push_message(g, "Assigned: Recover Your Equipment. The guards locked your gear in a storeroom.");
    }
}

static void enter_armory(GameState *g) {
    g->location = LOCATION_JAIL_ARMORY;
    g->level = 1;
    g->dialogue_active = 0;
    memset(&g->map, 0, sizeof(g->map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = x > 1 && x < 22 && y > 1 && y < 17 ? TILE_CASTLE_FLOOR : TILE_CASTLE_WALL;
        }
    }
    g->map.tiles[JAIL_GEAR_Y][JAIL_GEAR_X] = TILE_JAIL_GEAR_CHEST;
    g->map.tiles[17][11] = TILE_TUNNEL_EXIT;
    g->player.x = 11;
    g->player.y = 16;
    g->player.last_dx = 0;
    g->player.last_dy = -1;
    g->enemy_count = 3;
    if (!g->jail_armory_initialized) {
        for (int i = 0; i < 3; i++) {
            g->jail_guards[i] = (Enemy){.active = 1, .type = ENEMY_ROYAL_GUARD,
                .x = i == 0 ? 8 : i == 1 ? 15 : 12, .y = i == 2 ? 7 : 10,
                .hp = 90, .max_hp = 90, .attack = 16, .defense = 4, .experience = 65,
                .attack_target_x = -1, .attack_target_y = -1};
            snprintf(g->jail_guards[i].name, sizeof(g->jail_guards[i].name), "Royal Guard");
        }
        g->jail_armory_initialized = 1;
    }
    memcpy(g->enemies, g->jail_guards, sizeof(g->jail_guards));
    g->floor_item_count = g->jail_armory_loot_count;
    memcpy(g->floor_items, g->jail_armory_loot, sizeof(g->jail_armory_loot));
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].active) {
            g->map.tiles[g->floor_items[i].y][g->floor_items[i].x] = TILE_ITEM;
        }
    }
    push_message(g, "Royal Guards defend the dungeon's evidence room. Your confiscated equipment is in the chest.");
}

static void leave_armory(GameState *g) {
    memcpy(g->jail_guards, g->enemies, sizeof(g->jail_guards));
    memcpy(g->jail_armory_loot, g->floor_items, sizeof(g->jail_armory_loot));
    g->jail_armory_loot_count = g->floor_item_count;
    game_enter_town_gate(g, LOCATION_CASTLE, JAIL_DOOR_X, JAIL_DOOR_Y + 1);
    g->player.last_dy = 1;
}

int jail_gear_nearby(const GameState *g) {
    return g->location == LOCATION_JAIL_ARMORY && g->jail_gear_state == 1 &&
        near(g->player.x, g->player.y, JAIL_GEAR_X, JAIL_GEAR_Y);
}

int jail_recover_gear(GameState *g) {
    if (!jail_gear_nearby(g)) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].type == ENEMY_ROYAL_GUARD) {
            push_message(g, "Defeat the Royal Guards before opening the evidence chest.");
            return 1;
        }
    }
    int slots[] = {g->jail_main_hand, g->jail_off_hand, g->jail_extra_slots[0], g->jail_armor, g->jail_extra_slots[1], g->jail_extra_slots[2]};
    // Restore worn gear directly, even when the bag is full. Revisit unclaimed
    // bag items after all six slots so accessory constraints see the full set.
    for (int pass = 0; pass < g->jail_gear_count + EQUIPMENT_SLOT_COUNT; pass++) {
        int index = pass < EQUIPMENT_SLOT_COUNT ? slots[pass] : pass - EQUIPMENT_SLOT_COUNT;
        if (index < 0 || index >= g->jail_gear_count || (g->jail_gear_claimed & (1 << index))) {
            continue;
        }
        const Item *item = &g->jail_gear[index];
        int restored = pass < EQUIPMENT_SLOT_COUNT && game_restore_equipment(g, item, MAX_INVENTORY + pass);
        if (!restored && g->inventory_count < MAX_INVENTORY) {
            g->inventory[g->inventory_count++] = *item;
            restored = 1;
        }
        if (restored) {
            g->jail_gear_claimed |= 1 << index;
        }
    }
    if (g->jail_gear_claimed == (1 << g->jail_gear_count) - 1) {
        g->jail_gear_state = 3;
        push_message(g, "Completed: Recover Your Equipment. Every confiscated item has been returned.");
    } else {
        push_message(g, "Make room in your pack, then Pick Up again. Unclaimed gear stays safely on the ground.");
    }
    return 1;
}

static void enter_jail(GameState *g, int arrest) {
    if (arrest) {
        confiscate_gear(g);
    }
    g->location = LOCATION_JAIL;
    g->level = 1;
    g->jail_quest_state = 1;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->level_cleared = 0;
    g->castle_prompt = 0;
    g->trail_count = 0;
    g->trail_frames = 0;
    memset(&g->map, 0, sizeof(g->map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = TILE_CASTLE_WALL;
        }
    }
    for (int y = 4; y <= 12; y++) {
        for (int x = 5; x <= 18; x++) {
            g->map.tiles[y][x] = TILE_CASTLE_FLOOR;
        }
    }
    for (int x = 5; x <= 18; x++) {
        g->map.tiles[13][x] = TILE_JAIL_BARS;
        g->map.tiles[15][x] = TILE_CASTLE_FLOOR;
    }
    g->map.tiles[15][9] = TILE_NPC_ROYAL_GUARD;
    g->map.tiles[15][15] = TILE_NPC_ROYAL_GUARD;
    g->map.tiles[JAIL_PRISONER_Y][JAIL_PRISONER_X] = TILE_NPC_PRISONER;
    g->player.x = 9;
    g->player.y = 8;
    g->prisoner_x = JAIL_PRISONER_X;
    g->prisoner_y = JAIL_PRISONER_Y;
    g->player.poison_turns = 0;
    g->player.frozen_turns = 0;
    g->player.freeze_recovery = 0;
    dialogue(g, "Royal Guards", "The townsman was a royal informant. The guards locked you in the Royal Dungeon and confiscated your weapons, armor, and shields. Find your gear and a way out.", g->player.x, g->player.y);
    push_message(g, "The townsman reports you. Royal Guards escort you to the Royal Dungeon!");
}

static int tunnel_y(int x) {
    if (x >= 76 && x <= 98) {
        return 8;
    }
    if (x >= 113 && x <= 135) {
        return 20;
    }
    if (x >= 151 && x <= 174) {
        return 8;
    }
    return 14 + (x >= 12 && x <= 25 ? -3 : x >= 38 && x <= 49 ? 3 : 0);
}

static void carve_tunnel(Map *m, int start_x) {
    for (int x = start_x; x < ESCAPE_TUNNEL_W - 1; x++) {
        int radius = x < ESCAPE_TUNNEL_LEGACY_W ? 2 : 1;
        int from = tunnel_y(x - 1) - radius;
        int to = tunnel_y(x) + radius;
        if (from > tunnel_y(x) - radius) {
            from = tunnel_y(x) - radius;
        }
        if (to < tunnel_y(x - 1) + radius) {
            to = tunnel_y(x - 1) + radius;
        }
        for (int y = from; y <= to; y++) {
            m->tiles[y][x] = TILE_CASTLE_FLOOR;
        }
    }
    m->tiles[14][ESCAPE_TUNNEL_W - 1] = TILE_TUNNEL_EXIT;
}

static void populate_tunnel(GameState *g) {
    static const EnemyType deeper_types[] = {
        ENEMY_HOBGOBLIN_GUARD, ENEMY_GIANT_SPIDER, ENEMY_ROAD_ARCHER,
        ENEMY_BANDIT, ENEMY_GIANT_SPIDER, ENEMY_ROAD_ARCHER
    };
    for (int i = g->enemy_count; i < NON_ROAD_ENEMY_LIMIT; i++) {
        int deeper = i >= 12;
        int x = deeper ? 69 + (i - 12) * 7 : 18 + i * 4;
        EnemyType type = deeper ? deeper_types[(i - 12) % 6] :
            i % 3 == 0 ? ENEMY_BANDIT : i % 3 == 1 ? ENEMY_GIANT_SPIDER : ENEMY_GIANT_RAT;
        int hp = deeper ? (type == ENEMY_HOBGOBLIN_GUARD ? 80 : 48) : 32;
        Enemy *enemy = &g->enemies[g->enemy_count++];
        *enemy = (Enemy){.active = 1, .type = type, .x = x, .y = tunnel_y(x), .max_hp = hp, .hp = hp,
            .attack = deeper ? 16 : 10, .defense = deeper ? 5 : 3, .experience = deeper ? 60 : 35,
            .attack_target_x = -1, .attack_target_y = -1, .facing_dy = -1};
        const char *name = type == ENEMY_BANDIT ? "Tunnel Smuggler" : type == ENEMY_GIANT_SPIDER ? "Giant Spider" :
            type == ENEMY_HOBGOBLIN_GUARD ? "Smuggler Bodyguard" : type == ENEMY_ROAD_ARCHER ? "Smuggler Archer" : "Giant Rat";
        snprintf(enemy->name, sizeof(enemy->name), "%s", name);
    }
}

void jail_migrate_tunnel(GameState *g) {
    if (g->location != LOCATION_ESCAPE_TUNNEL) {
        return;
    }
    // Keep the original passage, escort, loot, and enemy state; open the deeper section.
    carve_tunnel(&g->map, ESCAPE_TUNNEL_LEGACY_W - 1);
    if (g->enemy_count < NON_ROAD_ENEMY_LIMIT) {
        g->level_cleared = 0;
        populate_tunnel(g);
    }
}

static void enter_tunnel(GameState *g) {
    g->location = LOCATION_ESCAPE_TUNNEL;
    g->level = 1;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->level_cleared = 0;
    g->dialogue_active = 0;
    memset(&g->map, 0, sizeof(g->map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = TILE_CASTLE_WALL;
        }
    }
    carve_tunnel(&g->map, 1);
    g->map.tiles[14][0] = TILE_JAIL_HATCH;
    g->player.x = 2;
    g->player.y = 14;
    g->prisoner_x = 1;
    g->prisoner_y = 14;
    populate_tunnel(g);
    if (g->jail_gear_state == 1 && !g->jail_supplies_taken && g->inventory_count < MAX_INVENTORY) {
        int slot = g->inventory_count++;
        g->inventory[slot] = item_make_smuggled_dagger();
        game_equip_main_hand(g, slot);
        g->jail_supplies_taken = 1;
        push_message(g, "Tomas gives you a smuggled weapon for the escape. Your own gear is still guarded in the dungeon.");
    }
    push_message(g, "Tomas follows. Clear the tunnel east to Ridgeshire; Space lets him catch up.");
    push_message(g, "The passage narrows deeper in; armed smugglers guard the long route home.");
    push_message(g, "Tomas pulls the hatch shut behind you so the guards cannot follow.");
}

int jail_prisoner_at(const GameState *g, int x, int y) {
    return ((g->location == LOCATION_ESCAPE_TUNNEL && g->jail_quest_state == 2) ||
        (g->location == LOCATION_TOWN4 && g->jail_quest_state == 3)) && g->prisoner_x == x && g->prisoner_y == y;
}

int jail_talk_nearby(GameState *g) {
    if (g->location == LOCATION_CASTLE && near(g->player.x, g->player.y, INFORMANT_X, INFORMANT_Y)) {
        if (g->jail_quest_state == 0) {
            enter_jail(g, 1);
        } else {
            dialogue(g, "Townsman", "The guards already know your face. I have nothing more to say to you.", INFORMANT_X, INFORMANT_Y);
        }
        return 1;
    }
    if (g->location == LOCATION_JAIL && near(g->player.x, g->player.y, JAIL_PRISONER_X, JAIL_PRISONER_Y)) {
        if (g->jail_quest_state == 0 && g->debug_passages_unlocked) {
            g->jail_quest_state = 1;
        }
        dialogue(g, "Tomas", g->jail_quest_state == 1 ?
            "A flagstone hides a tunnel to central Ridgeshire. Smugglers, spiders, and rats make it too dangerous for me alone. Lead me home for a reward; I will reveal the hatch and follow you." :
            "Step onto the revealed hatch to enter the tunnel. I will follow you to Ridgeshire. Clear the way; Space lets me catch up.", JAIL_PRISONER_X, JAIL_PRISONER_Y);
        return 1;
    }
    if (jail_prisoner_at(g, g->prisoner_x, g->prisoner_y) && near(g->player.x, g->player.y, g->prisoner_x, g->prisoner_y)) {
        dialogue(g, "Tomas", g->location == LOCATION_TOWN4 ? "Home at last. Thank you for guiding me out of that dungeon." :
            "I will stay behind you. Clear the way; Space lets me catch up if you move too far ahead.", g->prisoner_x, g->prisoner_y);
        return 1;
    }
    return 0;
}

int jail_move_exit(GameState *g, int x, int y) {
    if (g->debug_passages_unlocked && g->location == LOCATION_JAIL && x == 4 && y == 8) {
        g->location = LOCATION_CASTLE;
        map_generate_castle(&g->map, &g->player.x, &g->player.y);
        g->player.x = JAIL_DOOR_X;
        g->player.y = JAIL_DOOR_Y + 1;
        g->enemy_count = 0;
        g->floor_item_count = 0;
        g->dialogue_active = 0;
        return 1;
    }
    if (g->location == LOCATION_JAIL_ARMORY && x == 11 && y == 17) {
        leave_armory(g);
        return 1;
    }
    if (g->location == LOCATION_CASTLE && x == JAIL_DOOR_X && y == JAIL_DOOR_Y) {
        if (g->jail_quest_state == 3 && g->jail_gear_state) {
            enter_armory(g);
            return 1;
        }
        if (g->debug_passages_unlocked) {
            int quest = g->jail_quest_state;
            enter_jail(g, 0);
            g->jail_quest_state = quest;
            g->dialogue_active = 0;
            game_refresh_debug_passages(g);
            return 1;
        }
        push_message(g, "The Royal Dungeon is locked and guarded.");
        return 1;
    }
    if (g->location == LOCATION_JAIL && g->jail_quest_state >= 2 && x == JAIL_HATCH_X && y == JAIL_HATCH_Y) {
        enter_tunnel(g);
        return 1;
    }
    if (g->location != LOCATION_ESCAPE_TUNNEL) {
        return 0;
    }
    if (x == 0 && y == 14) {
        if (g->debug_passages_unlocked) {
            int quest = g->jail_quest_state;
            enter_jail(g, 0);
            g->jail_quest_state = quest;
            g->dialogue_active = 0;
            game_refresh_debug_passages(g);
            return 1;
        }
        push_message(g, "The hatch is barred from this side. The way out is east, to Ridgeshire.");
        return 1;
    }
    if (x != ESCAPE_TUNNEL_W - 1 || y != 14) {
        return 0;
    }
    if (!g->debug_passages_unlocked && (g->jail_quest_state != 2 || !near(g->player.x, g->player.y, g->prisoner_x, g->prisoner_y))) {
        push_message(g, "Wait for Tomas. You must reach Ridgeshire together.");
        return 1;
    }
    game_enter_town4(g);
    g->level = 1;
    g->player.x = 20;
    g->player.y = 12;
    g->prisoner_x = 21;
    g->prisoner_y = 12;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->level_cleared = 0;
    g->player.poison_turns = 0;
    if (g->debug_passages_unlocked && g->jail_quest_state != 2) {
        return 1;
    }
    g->jail_quest_state = 3;
    g->gold += ESCAPE_REWARD_GOLD;
    g->score += ESCAPE_REWARD_SCORE;
    dialogue(g, "Tomas", "We made it to Ridgeshire! Take 100 gold as thanks for bringing me home. I can finally see my family again.", g->prisoner_x, g->prisoner_y);
    push_message(g, "Completed: Guide Tomas Home. +100 gold, +750 score.");
    return 1;
}

void jail_follow(GameState *g) {
    if (g->location != LOCATION_ESCAPE_TUNNEL || g->jail_quest_state != 2 || g->player.hp <= 0 ||
        near(g->player.x, g->player.y, g->prisoner_x, g->prisoner_y)) {
        return;
    }
    // Find a clear route to the player, including bends and teleport-created gaps.
    int distance[ESCAPE_TUNNEL_H][ESCAPE_TUNNEL_W];
    int queue[ESCAPE_TUNNEL_H * ESCAPE_TUNNEL_W];
    memset(distance, -1, sizeof(distance));
    int head = 0;
    int tail = 0;
    queue[tail++] = g->player.y * ESCAPE_TUNNEL_W + g->player.x;
    distance[g->player.y][g->player.x] = 0;
    const int dx[4] = {1, 0, -1, 0};
    const int dy[4] = {0, 1, 0, -1};
    while (head < tail) {
        int cell = queue[head++];
        int x = cell % ESCAPE_TUNNEL_W;
        int y = cell / ESCAPE_TUNNEL_W;
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (nx < 1 || nx >= ESCAPE_TUNNEL_W - 1 || ny < 1 || ny >= ESCAPE_TUNNEL_H - 1 ||
                distance[ny][nx] >= 0 || !map_is_walkable(&g->map, nx, ny)) {
                continue;
            }
            int occupied = 0;
            for (int j = 0; j < g->enemy_count; j++) {
                const Enemy *enemy = &g->enemies[j];
                occupied |= enemy->active && enemy->x == nx && enemy->y == ny;
            }
            if (!occupied) {
                distance[ny][nx] = distance[y][x] + 1;
                queue[tail++] = ny * ESCAPE_TUNNEL_W + nx;
            }
        }
    }
    int best = distance[g->prisoner_y][g->prisoner_x];
    for (int i = 0; i < 4; i++) {
        int x = g->prisoner_x + dx[i];
        int y = g->prisoner_y + dy[i];
        if (x < 1 || x >= ESCAPE_TUNNEL_W - 1 || y < 1 || y >= ESCAPE_TUNNEL_H - 1) {
            continue;
        }
        int d = distance[y][x];
        if (d >= 0 && (best < 0 || d < best)) {
            g->prisoner_x = x;
            g->prisoner_y = y;
            break;
        }
    }
}

int jail_blocks_portal(GameState *g) {
    if (g->location != LOCATION_JAIL && g->location != LOCATION_ESCAPE_TUNNEL && g->location != LOCATION_JAIL_ARMORY) {
        return 0;
    }
    push_message(g, g->location == LOCATION_JAIL_ARMORY ?
        "The dungeon's royal wards block town portals. Leave through the front door." :
        "The dungeon's royal wards block town portals. Escape through the tunnel with Tomas.");
    return 1;
}
