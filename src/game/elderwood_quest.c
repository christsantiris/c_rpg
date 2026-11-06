#include "elderwood_quest.h"
#include "aldric.h"
#include "elowen.h"
#include "storm_mage.h"
#include <stdlib.h>
#include <string.h>

void elderwood_quest_sync(GameState *g) {
    if (g->elderwood_defeated && g->elderwood_quest_state < 2) {
        g->elderwood_quest_state = 2;
        push_message(g, "Completed: Clear Elderwood. Ranger Tamsin has your reward near Stillbury's east gate.");
    }
}

int elderwood_quest_talk(GameState *g) {
    if (g->location != LOCATION_TOWN2 || g->map.tiles[RANGER_TAMSIN_Y][RANGER_TAMSIN_X] != TILE_NPC_TAMSIN ||
        abs(g->player.x - RANGER_TAMSIN_X) > 1 || abs(g->player.y - RANGER_TAMSIN_Y) > 1) {
        return 0;
    }
    elderwood_quest_sync(g);
    g->dialogue_active = 1;
    g->dialogue_x = RANGER_TAMSIN_X;
    g->dialogue_y = RANGER_TAMSIN_Y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", RANGER_TAMSIN);
    const char *text;
    if (g->elderwood_quest_state == 0) {
        text = "Evil has taken hold of Elderwood, east of Stillbury. Defeat Elderthorn in its fifth stage to clear the forest and open the coastal road. Return to me for 120 gold and 800 score. Will you help?";
    } else if (g->elderwood_quest_state == 1) {
        text = "Elderthorn still rules Elderwood. Take Stillbury's east road and defeat him in the fifth stage. Return here for your reward once the forest is clear.";
    } else if (g->elderwood_quest_state == 2) {
        g->elderwood_quest_state = 3;
        g->gold += ELDERWOOD_QUEST_GOLD;
        g->score += ELDERWOOD_QUEST_SCORE;
        text = "Elderthorn is defeated, and the road to Sunken Coast is open. Thank you for clearing Elderwood. Take 120 gold and 800 score with Stillbury's thanks.";
        push_message(g, "Ranger Tamsin rewards you for clearing Elderwood: +120 gold, +800 score.");
    } else {
        text = "Elderwood is safer thanks to you. Follow the eastern road through the forest to Sunken Coast, and speak to Captain Rowan at Stillbury's docks when you return.";
    }
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s", text);
    return 1;
}

static void open_tile(const GameState *g, int *x, int *y) {
    for (int radius = 1; radius < TOWN_W + TOWN_H; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int px = RANGER_TAMSIN_X + dx;
                int py = RANGER_TAMSIN_Y + dy;
                if (abs(dx) + abs(dy) != radius || !map_is_walkable(&g->map, px, py) ||
                    map_is_travel_tile(g->map.tiles[py][px]) || g->map.tiles[py][px] == TILE_ITEM ||
                    g->map.tiles[py][px] == TILE_PORTAL || (g->player.x == px && g->player.y == py) ||
                    aldric_at(g, px, py) || storm_mage_at(g, px, py) || elowen_at(g, px, py)) {
                    continue;
                }
                int occupied = 0;
                for (int i = 0; i < g->enemy_count; i++) {
                    occupied |= g->enemies[i].active && g->enemies[i].x == px && g->enemies[i].y == py;
                }
                if (!occupied) {
                    *x = px;
                    *y = py;
                    return;
                }
            }
        }
    }
}

void elderwood_quest_migrate(GameState *g) {
    elderwood_quest_sync(g);
    if (g->location != LOCATION_TOWN2) {
        return;
    }
    int x = RANGER_TAMSIN_X;
    int y = RANGER_TAMSIN_Y;
    if (g->player.x == x && g->player.y == y) {
        open_tile(g, &g->player.x, &g->player.y);
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            open_tile(g, &item->x, &item->y);
            item->underlying_tile = g->map.tiles[item->y][item->x];
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
    if (g->portal_active && g->portal_location == LOCATION_TOWN2 && g->portal_x == x && g->portal_y == y) {
        open_tile(g, &g->portal_x, &g->portal_y);
        g->portal_origin_tile = g->map.tiles[g->portal_y][g->portal_x];
        g->map.tiles[g->portal_y][g->portal_x] = TILE_PORTAL;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *enemy = &g->enemies[i];
        if (enemy->active && enemy->x == x && enemy->y == y) {
            open_tile(g, &enemy->x, &enemy->y);
        }
    }
    g->map.tiles[y][x] = TILE_NPC_TAMSIN;
    if (aldric_at(g, x, y)) {
        aldric_follow(g, -1, -1);
    }
    if (storm_mage_at(g, x, y)) {
        storm_mage_follow(g, -1, -1);
    }
    if (elowen_at(g, x, y)) {
        elowen_follow(g, -1, -1);
    }
}
