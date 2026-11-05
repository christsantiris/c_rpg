#include "crown_quest.h"
#include "western_arc.h"
#include "approach.h"
#include <stdio.h>
#include <stdlib.h>

void crown_quest_sync(GameState *g) {
    if (g->giants_crown_defeated) {
        int recovered = !(g->crown_quest_progress & CROWN_TAKEN);
        g->crown_quest_progress |= CROWN_JARL | CROWN_TAKEN;
        if (recovered) {
            push_message(g, "You recover the Last Jarl's crown. Edda waits at the Stillbury end of Giant's Bridge, outside the checkpoint. No pack slot is needed.");
        }
    }
    if (g->crown_quest_state == 1 && (g->crown_quest_progress & CROWN_TAKEN)) {
        g->crown_quest_state = 2;
        push_message(g, "Completed: Giant's Crown. Deliver it to Edda outside Stillbury's western checkpoint for your reward.");
    }
}

static void claim_reward(GameState *g) {
    if (g->crown_reward_state != 1) {
        return;
    }
    if (g->inventory_count >= MAX_INVENTORY) {
        push_message(g, "Make room in your pack. Edda keeps your Stoneguard Shield safe until you return.");
        return;
    }
    g->inventory[g->inventory_count++] = item_make_stoneguard_shield();
    g->crown_reward_state = 2;
    push_message(g, "Received Stoneguard Shield: +8 defense, 20% block, +30 maximum HP while equipped.");
}

int crown_quest_talk(GameState *g) {
    if (!approach_bridge_road(g) || abs(g->player.x - EDDA_STILLBURY_X) + abs(g->player.y - EDDA_STILLBURY_Y) > 1 ||
        g->map.tiles[EDDA_STILLBURY_Y][EDDA_STILLBURY_X] != TILE_NPC_EDDA) {
        return 0;
    }
    crown_quest_sync(g);
    g->dialogue_active = 1;
    g->dialogue_x = EDDA_STILLBURY_X;
    g->dialogue_y = EDDA_STILLBURY_Y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Antiquarian Edda");
    if (!g->crown_quest_state) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Liora at the caravan camp outside Sunscar can tell you about the Last Jarl's crown. She asks you to recover it from him and bring it to me. I remain outside Stillbury's checkpoint, whatever the Crown thinks of you.");
    } else if (g->crown_quest_state == 1) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "The Last Jarl carries the crown in Giant's Peaks 5. Recover it when he falls and bring it here. No inscription, seal or vault key is required.");
    } else {
        if (g->crown_quest_state == 2) {
            g->crown_quest_state = 3;
            g->crown_reward_state = 1;
            g->gold += CROWN_REWARD_GOLD;
            g->score += CROWN_REWARD_SCORE;
            push_message(g, "Giant's Crown reward: 220 gold, 1600 score and a Stoneguard Shield.");
        }
        claim_reward(g);
        snprintf(g->dialogue_text, sizeof(g->dialogue_text), g->crown_reward_state == 1 ?
            "The giant kingdom's crown is safe. Your 220 gold is paid. Make room in your pack and speak to me again; your Stoneguard Shield is safe here." :
            "The crown preserves the giant kingdom's history. Your Stoneguard Shield carries its endurance: +8 defense, 20%% block, and +30 maximum HP while equipped.");
    }
    return 1;
}

// The old quest ward is retired. Its side room remains ordinary optional treasure.
static void repair_vault(Map *map, FloorItem *loot, int count) {
    TileType *door = &map->tiles[CROWN_DOOR_Y][CROWN_DOOR_X];
    if (*door == TILE_GIANTS_WALL) {
        *door = TILE_GIANTS_CRACK;
    }
    for (int i = 0; i < count; i++) {
        if (loot[i].active && loot[i].x == CROWN_DOOR_X && loot[i].y == CROWN_DOOR_Y && loot[i].underlying_tile == TILE_GIANTS_WALL) {
            loot[i].underlying_tile = TILE_GIANTS_CRACK;
        }
    }
}

void crown_quest_prepare(GameState *g) {
    if (g->location != LOCATION_GIANTS_CROWN || g->level != 5) {
        return;
    }
    repair_vault(&g->map, g->floor_items, g->floor_item_count);
    if (g->portal_location == LOCATION_GIANTS_CROWN && g->portal_level == 5 &&
        g->portal_x == CROWN_DOOR_X && g->portal_y == CROWN_DOOR_Y && g->portal_origin_tile == TILE_GIANTS_WALL) {
        g->portal_origin_tile = TILE_GIANTS_CRACK;
    }
    if (g->crown_vault_open || g->debug_passages_unlocked ||
        (g->player.x >= 3 && g->player.x <= 7 && g->player.y >= 21 && g->player.y <= 25)) {
        if (g->map.tiles[CROWN_DOOR_Y][CROWN_DOOR_X] == TILE_GIANTS_CRACK) {
            g->map.tiles[CROWN_DOOR_Y][CROWN_DOOR_X] = TILE_GIANTS_PAVING;
        }
    }
}

void crown_quest_migrate(GameState *g) {
    // Repair cached terrain as well as the live room without replacing either map.
    GiantsStage *stage = &g->giants_crown[4];
    if (stage->valid && stage->tiles[CROWN_DOOR_Y][CROWN_DOOR_X] == TILE_GIANTS_WALL) {
        stage->tiles[CROWN_DOOR_Y][CROWN_DOOR_X] = TILE_GIANTS_CRACK;
    }
    for (int i = 0; i < stage->loot_count; i++) {
        if (stage->loot[i].x == CROWN_DOOR_X && stage->loot[i].y == CROWN_DOOR_Y && stage->loot[i].underlying_tile == TILE_GIANTS_WALL) {
            stage->loot[i].underlying_tile = TILE_GIANTS_CRACK;
        }
    }
    crown_quest_prepare(g);
    crown_quest_sync(g);
}

const char *crown_quest_hint(const GameState *g) {
    (void)g;
    return NULL;
}

int crown_quest_interact(GameState *g) {
    (void)g;
    return 0;
}

int crown_quest_marker(const GameState *g, int *x, int *y) {
    (void)g;
    (void)x;
    (void)y;
    return 0;
}

int crown_quest_valid(const GameState *g) {
    int state = g->crown_quest_state;
    int progress = g->crown_quest_progress;
    int reward = g->crown_reward_state;
    return state >= 0 && state <= 3 && progress >= 0 && progress <= 15 &&
        g->crown_vault_open >= 0 && g->crown_vault_open <= 1 && reward >= 0 && reward <= 2 &&
        (!(progress & (CROWN_JARL | CROWN_TAKEN)) || g->giants_crown_defeated) &&
        (state < 2 || (progress & CROWN_TAKEN)) && ((state == 3) == (reward > 0));
}
