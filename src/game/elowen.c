#include "elowen.h"
#include "aldric.h"
#include "storm_mage.h"
#include "garran.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int escort_space(const GameState *g, int x, int y) {
    if (!map_is_walkable(&g->map, x, y) || (x == g->player.x && y == g->player.y) || aldric_at(g, x, y) || storm_mage_at(g, x, y) || garran_at(g, x, y)) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 0;
        }
    }
    return 1;
}

int elowen_at(const GameState *g, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
        return 0;
    }
    if (g->elowen_quest_state == ELOWEN_RESCUED) {
        return x == g->elowen_x && y == g->elowen_y;
    }
    return g->map.tiles[y][x] == TILE_NPC_ELOWEN &&
        ((g->location == LOCATION_TAVERN && g->elowen_quest_state == ELOWEN_HOME) ||
        (g->location == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH &&
        g->elowen_quest_state < ELOWEN_RESCUED));
}

void elowen_follow(GameState *g, int old_x, int old_y) {
    if (g->elowen_quest_state != ELOWEN_RESCUED) {
        return;
    }
    if (abs(old_x - g->player.x) + abs(old_y - g->player.y) == 1 &&
        escort_space(g, old_x, old_y)) {
        g->elowen_x = old_x;
        g->elowen_y = old_y;
    } else if (abs(g->elowen_x - g->player.x) + abs(g->elowen_y - g->player.y) != 1 ||
        !escort_space(g, g->elowen_x, g->elowen_y)) {
        const int dx[] = {0, -1, 1, 0};
        const int dy[] = {1, 0, 0, -1};
        // A one-tile landing can temporarily share the player's square.
        g->elowen_x = g->player.x;
        g->elowen_y = g->player.y;
        for (int i = 0; i < 4; i++) {
            int x = g->player.x + dx[i];
            int y = g->player.y + dy[i];
            if (escort_space(g, x, y)) {
                g->elowen_x = x;
                g->elowen_y = y;
                break;
            }
        }
    }
    if (g->dialogue_active && strcmp(g->dialogue_speaker, "Elowen") == 0) {
        g->dialogue_x = g->elowen_x;
        g->dialogue_y = g->elowen_y;
    }
}

static int prisoner_space(const GameState *g, int x, int y) {
    if (g->map.tiles[y][x] != TILE_FLOOR || !escort_space(g, x, y)) {
        return 0;
    }
    // The surrounding ring guarantees routes can pass around the captive.
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (!map_is_walkable(&g->map, x + dx, y + dy)) {
                return 0;
            }
        }
    }
    return 1;
}

void elowen_prepare(GameState *g) {
    if (g->location != LOCATION_DUNGEON || g->level != DUNGEON_DEPTH ||
        g->map.room_count < 1 || g->elowen_quest_state >= ELOWEN_RESCUED ||
        (g->defeated_bosses & (1 << LOCATION_DUNGEON))) {
        return;
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    for (int y = room->y + 1; y < room->y + room->h - 1; y++) {
        for (int x = room->x + 1; x < room->x + room->w - 1; x++) {
            if (g->map.tiles[y][x] == TILE_NPC_ELOWEN) {
                return;
            }
        }
    }
    // Keep her inside the chamber, away from its door and existing props.
    for (int y = room->y + room->h - 2; y > room->y; y--) {
        for (int x = room->x + room->w - 2; x > room->x; x--) {
            if (prisoner_space(g, x, y)) {
                g->map.tiles[y][x] = TILE_NPC_ELOWEN;
                return;
            }
        }
    }
}

void elowen_rescue(GameState *g) {
    if (g->location != LOCATION_DUNGEON || g->level != DUNGEON_DEPTH ||
        g->elowen_quest_state >= ELOWEN_RESCUED) {
        return;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_NPC_ELOWEN) {
                g->map.tiles[y][x] = TILE_FLOOR;
            }
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].underlying_tile == TILE_NPC_ELOWEN) {
            g->floor_items[i].underlying_tile = TILE_FLOOR;
        }
    }
    g->elowen_quest_state = ELOWEN_RESCUED;
    g->elowen_x = -1;
    g->elowen_y = -1;
    elowen_follow(g, -1, -1);
    game_talk_to_elowen(g);
    push_message(g, "Completed: Save Elowen. Bring her to Bram in Oakhaven's Inn for your reward.");
}

void elowen_home(GameState *g) {
    if (g->location != LOCATION_TAVERN || g->elowen_quest_state != ELOWEN_RESCUED) {
        return;
    }
    // Keep anything dropped at her future home available beside her.
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == ELOWEN_TAVERN_X && item->y == ELOWEN_TAVERN_Y) {
            item->y++;
            item->underlying_tile = TILE_TAVERN_FLOOR;
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
    g->elowen_quest_state = ELOWEN_HOME;
    g->elowen_x = -1;
    g->elowen_y = -1;
    g->gold += ELOWEN_REWARD_GOLD;
    map_place_tavern_elowen(&g->map);
    push_message(g, "Elowen is home. Bram rewards you with 80 gold.");
}

void game_talk_to_elowen(GameState *g) {
    int x = -1;
    int y = -1;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int tx = g->player.x + dx;
            int ty = g->player.y + dy;
            if (elowen_at(g, tx, ty)) {
                x = tx;
                y = ty;
            }
        }
    }
    if (x < 0) {
        return;
    }
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Elowen");
    g->dialogue_x = x;
    g->dialogue_y = y;
    const char *text = "The Lich King has bound me here. Defeat him and I can escape!";
    if (g->elowen_quest_state == ELOWEN_RESCUED) {
        text = "You freed me! My father Bram runs Oakhaven's Inn. He will reward you for saving me. I will follow you; please take me home to him.";
    } else if (g->elowen_quest_state == ELOWEN_HOME) {
        text = "Thank you for bringing me home. I overheard the Lich King say he was in league with the King. Could the Crown be behind the creatures threatening Oakhaven?";
    }
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "%s", text);
}
