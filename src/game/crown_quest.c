#include "crown_quest.h"
#include <stdio.h>
#include <stdlib.h>

static int near(const GameState *g, int x, int y) {
    return abs(g->player.x - x) + abs(g->player.y - y) <= 1;
}

static TileType door_tile(const GameState *g) {
    TileType tile = g->map.tiles[CROWN_DOOR_Y][CROWN_DOOR_X];
    if (tile == TILE_ITEM) {
        for (int n = 0; n < g->floor_item_count; n++) {
            const FloorItem *item = &g->floor_items[n];
            if (item->active && item->x == CROWN_DOOR_X && item->y == CROWN_DOOR_Y) {
                return item->underlying_tile;
            }
        }
    }
    if (tile == TILE_PORTAL && g->portal_location == LOCATION_GIANTS_CROWN && g->portal_level == 5) {
        return g->portal_origin_tile;
    }
    return tile;
}

static void put_door(GameState *g, TileType tile) {
    if (g->map.tiles[CROWN_DOOR_Y][CROWN_DOOR_X] == TILE_ITEM) {
        for (int n = 0; n < g->floor_item_count; n++) {
            FloorItem *item = &g->floor_items[n];
            if (item->active && item->x == CROWN_DOOR_X && item->y == CROWN_DOOR_Y) {
                item->underlying_tile = tile;
            }
        }
    } else if (g->map.tiles[CROWN_DOOR_Y][CROWN_DOOR_X] == TILE_PORTAL &&
        g->portal_location == LOCATION_GIANTS_CROWN && g->portal_level == 5) {
        g->portal_origin_tile = tile;
    } else {
        g->map.tiles[CROWN_DOOR_Y][CROWN_DOOR_X] = tile;
    }
}

void crown_quest_sync(GameState *g) {
    if (g->giants_crown_defeated) {
        g->crown_quest_progress |= CROWN_JARL;
    }
    if (g->crown_quest_state == 1 && g->crown_quest_progress == 15) {
        g->crown_quest_state = 2;
        push_message(g, "The royal crown is safe. Return to Antiquarian Edda in Rosemoor's Adventurer's Guild.");
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
    if (g->location != LOCATION_GUILD || !near(g, EDDA_X, EDDA_Y) || g->map.tiles[EDDA_Y][EDDA_X] != TILE_NPC_EDDA) {
        return 0;
    }
    crown_quest_sync(g);
    g->dialogue_active = 1;
    g->dialogue_x = EDDA_X;
    g->dialogue_y = EDDA_Y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Antiquarian Edda");
    if (!g->crown_quest_state) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Find the giant kingdom's crown. Read stage 2's inscription, recover stage 3's seal, then defeat the Last Jarl and open stage 5's western vault. Return for 220 gold and a Stoneguard Shield.");
    } else if (g->crown_quest_state == 1) {
        if (!(g->crown_quest_progress & CROWN_INSCRIPTION)) {
            snprintf(g->dialogue_text, sizeof(g->dialogue_text),
                "Read the royal inscription in Giant's Peaks 2's northwest village hall. It names the seal chamber behind the cracked western masonry on stage 3.");
        } else if (!(g->crown_quest_progress & CROWN_SEAL)) {
            snprintf(g->dialogue_text, sizeof(g->dialogue_text),
                "Stage 3: follow the western path to the cracked wall. Bait a giant's warned strike against it, or clear it with Interact. Recover the royal seal from the chamber's pedestal.");
        } else if (!(g->crown_quest_progress & CROWN_JARL)) {
            snprintf(g->dialogue_text, sizeof(g->dialogue_text),
                "Defeat the Last Jarl on Giant's Peaks 5. Then take the western path to the royal vault. The seal opens its warded doorway after his defeat.");
        } else {
            snprintf(g->dialogue_text, sizeof(g->dialogue_text),
                "Your seal and the Jarl's defeat open Giant's Peaks 5's western royal vault. Take the crown from its pedestal and bring it here. Its ordinary stone coffer holds separate treasure.");
        }
    } else {
        if (g->crown_quest_state == 2) {
            g->crown_quest_state = 3;
            g->crown_reward_state = 1;
            g->gold += CROWN_REWARD_GOLD;
            g->score += CROWN_REWARD_SCORE;
            push_message(g, "Completed: Giant's Crown. +220 gold, +1600 score.");
        }
        claim_reward(g);
        snprintf(g->dialogue_text, sizeof(g->dialogue_text), g->crown_reward_state == 1 ?
            "The crown will preserve the giant kingdom's history. Your 220 gold is paid. Make room in your pack and speak to me again; your Stoneguard Shield is safe here." :
            "The crown belongs in the Guild's collection. Your Stoneguard Shield carries the old kingdom's endurance: +8 defense, 20%% block, and +30 maximum HP while equipped.");
    }
    return 1;
}

void crown_quest_prepare(GameState *g) {
    if (g->location != LOCATION_GIANTS_CROWN || g->level != 5) {
        return;
    }
    TileType tile = door_tile(g);
    if ((g->crown_vault_open || g->debug_passages_unlocked) && (tile == TILE_GIANTS_WALL || tile == TILE_GIANTS_CRACK)) {
        put_door(g, TILE_GIANTS_PAVING);
    } else if (!g->crown_vault_open && tile == TILE_GIANTS_CRACK) {
        // A warded door cannot be shattered by a heavy strike. Old already-open
        // rooms stay accessible; the crown itself still requires seal and victory.
        put_door(g, TILE_GIANTS_WALL);
    }
}

void crown_quest_migrate(GameState *g) {
    if (g->location == LOCATION_GUILD && g->map.tiles[EDDA_Y][EDDA_X] == TILE_TAVERN_FLOOR &&
        (g->player.x != EDDA_X || g->player.y != EDDA_Y)) {
        g->map.tiles[EDDA_Y][EDDA_X] = TILE_NPC_EDDA;
    }
    if (g->location == LOCATION_GIANTS_CROWN && g->level == 5) {
        int occupied_vault = g->player.x >= 3 && g->player.x <= 7 && g->player.y >= 21 && g->player.y <= 25;
        occupied_vault |= g->portal_active && g->portal_location == LOCATION_GIANTS_CROWN && g->portal_level == 5 &&
            g->portal_x >= 3 && g->portal_x <= 7 && g->portal_y >= 21 && g->portal_y <= 25;
        if (occupied_vault && (door_tile(g) == TILE_GIANTS_CRACK || door_tile(g) == TILE_GIANTS_WALL)) {
            put_door(g, TILE_GIANTS_PAVING);
        }
    }
    // Cached stages receive the ward only when entered. No map is regenerated.
    crown_quest_prepare(g);
    crown_quest_sync(g);
}

const char *crown_quest_hint(const GameState *g) {
    if (g->location != LOCATION_GIANTS_CROWN) {
        return NULL;
    }
    if (g->level == 2 && near(g, CROWN_INSCRIPTION_X, CROWN_INSCRIPTION_Y)) {
        return g->crown_quest_progress & CROWN_INSCRIPTION ? "A: REREAD ROYAL INSCRIPTION" : "A: READ ROYAL INSCRIPTION";
    }
    if (g->level == 3 && !(g->crown_quest_progress & CROWN_SEAL) && near(g, CROWN_RELIC_X, CROWN_RELIC_Y)) {
        return "A: RECOVER ROYAL SEAL";
    }
    if (g->level == 5) {
        if (!g->crown_vault_open && near(g, CROWN_DOOR_X, CROWN_DOOR_Y)) {
            return "A: OPEN ROYAL VAULT";
        }
        if (!(g->crown_quest_progress & CROWN_TAKEN) && near(g, CROWN_RELIC_X, CROWN_RELIC_Y)) {
            return "A: RECOVER ROYAL CROWN";
        }
    }
    return NULL;
}

int crown_quest_interact(GameState *g) {
    if (!crown_quest_hint(g)) {
        return 0;
    }
    crown_quest_sync(g);
    if (g->level == 2) {
        g->crown_quest_progress |= CROWN_INSCRIPTION;
        push_message(g, "Royal inscription: the seal rests behind the western cracked masonry of the Broken Aqueduct, stage 3.");
        push_message(g, "The seal opens stage 5's western royal vault only after the Last Jarl falls. The crown rests within, apart from ordinary treasure.");
    } else if (g->level == 3) {
        g->crown_quest_progress |= CROWN_SEAL;
        push_message(g, "You recover the royal seal. Use it at the western vault on Giant's Peaks 5 after defeating the Last Jarl. No pack slot is needed.");
    } else if (!(g->crown_quest_progress & CROWN_SEAL)) {
        push_message(g, "The royal ward requires its seal. Find it behind Giant's Peaks 3's western cracked masonry.");
        return 1;
    } else if (!g->giants_crown_defeated) {
        push_message(g, "The Last Jarl's ward still binds the crown. Defeat him before opening the royal vault.");
        return 1;
    } else if (near(g, CROWN_DOOR_X, CROWN_DOOR_Y)) {
        g->crown_vault_open = 1;
        put_door(g, TILE_GIANTS_PAVING);
        push_message(g, "The royal seal releases the ward. The vault door opens; its return passage stays clear.");
    } else {
        // An older save may already have opened this room before the quest existed.
        g->crown_vault_open = 1;
        g->crown_quest_progress |= CROWN_TAKEN;
        crown_quest_prepare(g);
        push_message(g, "You lift the intact royal crown from its stone resting place. It is kept with your quest discoveries, outside the pack.");
    }
    crown_quest_sync(g);
    return 1;
}

int crown_quest_marker(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_GIANTS_CROWN) {
        return 0;
    }
    if (g->level == 2 && !(g->crown_quest_progress & CROWN_INSCRIPTION)) {
        *x = CROWN_INSCRIPTION_X;
        *y = CROWN_INSCRIPTION_Y;
        return 1;
    }
    if ((g->level == 3 && !(g->crown_quest_progress & CROWN_SEAL)) ||
        (g->level == 5 && !(g->crown_quest_progress & CROWN_TAKEN))) {
        *x = g->level == 5 && !g->crown_vault_open ? CROWN_DOOR_X : CROWN_RELIC_X;
        *y = CROWN_RELIC_Y;
        return 1;
    }
    return 0;
}

int crown_quest_valid(const GameState *g) {
    int state = g->crown_quest_state;
    int progress = g->crown_quest_progress;
    int vault = g->crown_vault_open;
    int reward = g->crown_reward_state;
    return state >= 0 && state <= 3 && progress >= 0 && progress <= 15 && vault >= 0 && vault <= 1 && reward >= 0 && reward <= 2 &&
        (!(progress & CROWN_JARL) || g->giants_crown_defeated) &&
        (!vault || ((progress & (CROWN_SEAL | CROWN_JARL)) == (CROWN_SEAL | CROWN_JARL))) &&
        (!(progress & CROWN_TAKEN) || vault) && (state != 1 || progress != 15) &&
        (state < 2 || progress == 15) && ((state == 3) == (reward > 0));
}
