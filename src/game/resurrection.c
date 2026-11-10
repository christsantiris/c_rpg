#include "resurrection.h"
#include "aldric.h"
#include "party.h"
#include "storm_mage.h"
#include "garran.h"
#include "elowen.h"
#include "town_road.h"
#include "combat_feedback.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char *speaker = "Soulstone Warlock";

int resurrection_available(const GameState *g) {
    return g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN;
}

static int near_warlock(const GameState *g) {
    return resurrection_available(g) && g->map.tiles[SOULSTONE_WARLOCK_Y][SOULSTONE_WARLOCK_X] == TILE_NPC_WARLOCK &&
        abs(g->player.x - SOULSTONE_WARLOCK_X) <= 1 &&
        abs(g->player.y - SOULSTONE_WARLOCK_Y) <= 1;
}

static int loot_at(const GameState *g, int x, int y) {
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
            return 1;
        }
    }
    return 0;
}

static void move_landmark_loot(GameState *g, FloorItem *item) {
    for (int radius = 1; radius <= 8; radius++) {
        for (int y = item->y - radius; y <= item->y + radius; y++) {
            for (int x = item->x - radius; x <= item->x + radius; x++) {
                if (!map_is_walkable(&g->map, x, y) || map_is_travel_tile(g->map.tiles[y][x]) || loot_at(g, x, y) ||
                    (x == SOULSTONE_X && y == SOULSTONE_Y) || (x == SOULSTONE_WARLOCK_X && y == SOULSTONE_WARLOCK_Y)) {
                    continue;
                }
                item->x = x;
                item->y = y;
                item->underlying_tile = g->map.tiles[y][x];
                g->map.tiles[y][x] = TILE_ITEM;
                return;
            }
        }
    }
}

static void marker(GameState *g, int x, int y, TileType tile) {
    g->map.tiles[y][x] = tile;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            if (tile == TILE_SOULSTONE_SHRINE || tile == TILE_NPC_WARLOCK) {
                move_landmark_loot(g, item);
                continue;
            }
            item->underlying_tile = tile;
            // Preserve the landmark while retaining the item for normal pickup.
            if (tile == TILE_TOWN_PATH) {
                g->map.tiles[y][x] = TILE_ITEM;
            }
        }
    }
}

void resurrection_place(GameState *g) {
    if (!resurrection_available(g)) {
        return;
    }
    for (int x = SOULSTONE_RITUAL_X; x < OAKHAVEN_FERRY_X; x++) {
        marker(g, x, SOULSTONE_RITUAL_Y, TILE_TOWN_PATH);
    }
    marker(g, SOULSTONE_RITUAL_X, SOULSTONE_RITUAL_Y, TILE_SOULSTONE_RITUAL);
    marker(g, SOULSTONE_X, SOULSTONE_Y, TILE_SOULSTONE_SHRINE);
    marker(g, SOULSTONE_WARLOCK_X, SOULSTONE_WARLOCK_Y, TILE_NPC_WARLOCK);
    // Only relocate an actor if an older save stood on a newly solid landmark.
    if (!map_is_walkable(&g->map, g->player.x, g->player.y)) {
        g->player.x = SOULSTONE_RITUAL_X;
        g->player.y = SOULSTONE_RITUAL_Y;
    }
    if (g->aldric_party_state == ALDRIC_FOLLOWING && !map_is_walkable(&g->map, g->aldric_x, g->aldric_y)) {
        aldric_follow(g, -1, -1);
    }
    if (g->storm_mage.state == STORM_MAGE_FOLLOWING && !map_is_walkable(&g->map, g->storm_mage.x, g->storm_mage.y)) {
        storm_mage_follow(g, -1, -1);
    }
    if (g->garran.state == GARRAN_FOLLOWING && !map_is_walkable(&g->map, g->garran.x, g->garran.y)) {
        garran_follow(g, -1, -1);
    }
    if (g->elowen_quest_state == ELOWEN_RESCUED && !map_is_walkable(&g->map, g->elowen_x, g->elowen_y)) {
        elowen_follow(g, -1, -1);
    }
}

static int peaceful(const GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active) {
            return 0;
        }
    }
    return g->player.hp > 0 && g->player.frozen_turns == 0;
}

static int knows_resurrection(const GameState *g) {
    for (int i = 0; i < g->player.known_spell_count; i++) {
        if (g->player.known_spells[i].id == SPELL_RESURRECTION) {
            return 1;
        }
    }
    return 0;
}

static int has_party(const GameState *g) {
    for (int i = 0; i < PARTY_MEMBER_COUNT; i++) {
        if (party_recruited(g, i)) {
            return 1;
        }
    }
    return 0;
}

static void offer(GameState *g, int prompt, const char *text) {
    g->resurrection_prompt = prompt;
    g->dialogue_active = 1;
    g->dialogue_x = (prompt == RESURRECTION_SPELL_CONFIRM || prompt == RESURRECTION_SPELL_TARGET) ? SOULSTONE_X : SOULSTONE_WARLOCK_X;
    g->dialogue_y = (prompt == RESURRECTION_SPELL_CONFIRM || prompt == RESURRECTION_SPELL_TARGET) ? SOULSTONE_Y : SOULSTONE_WARLOCK_Y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", (prompt == RESURRECTION_SPELL_CONFIRM || prompt == RESURRECTION_SPELL_TARGET) ? "Soulstone Shrine" : speaker);
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s", text);
}

int resurrection_talk(GameState *g) {
    if (!near_warlock(g)) {
        return 0;
    }
    char text[MAX_DIALOGUE_LEN];
    const char *status = !has_party(g) ? "Recruit a companion before using these services." :
        party_fallen_count(g) ? "Fallen companions await resurrection." : "Your companions are alive.";
    snprintf(text, sizeof(text),
        "This shrine can restore a fallen companion to life.\n%s\nGold: %d\n%s2 Resurrection scroll: %d gold (Mage level %d)\nEnter / Esc: Leave",
        status, g->gold, party_fallen_count(g) ? "1 Resurrect fallen companion: 200 gold\n" : "",
        RESURRECTION_SCROLL_GOLD, RESURRECTION_LEVEL);
    offer(g, RESURRECTION_SERVICE, text);
    return 1;
}

static const Spell *ritual_spell(const GameState *g) {
    int index = g->player.equipped_spell;
    if (index < 0 || index >= g->player.known_spell_count || g->player.known_spells[index].id != SPELL_RESURRECTION) {
        return NULL;
    }
    return &g->player.known_spells[index];
}

static int spell_ready(GameState *g) {
    if (!resurrection_available(g) || g->player.x != SOULSTONE_RITUAL_X || g->player.y != SOULSTONE_RITUAL_Y ||
        g->map.tiles[SOULSTONE_RITUAL_Y][SOULSTONE_RITUAL_X] != TILE_SOULSTONE_RITUAL) {
        push_message(g, "Resurrection requires the ritual circle at Soulstone Shrine, north of Oakhaven.");
        return 0;
    }
    if (g->player.player_class != CLASS_MAGE || g->player.level < RESURRECTION_LEVEL || !ritual_spell(g)) {
        push_message(g, "Only a level 10 Mage who knows and equips Resurrection can perform this ritual.");
        return 0;
    }
    if (!peaceful(g)) {
        push_message(g, "Resurrection requires peace at the shrine.");
        return 0;
    }
    if (!party_fallen_count(g)) {
        push_message(g, "No fallen party member needs resurrection.");
        return 0;
    }
    if (g->player.mp < action_spell_mp_cost(g, ritual_spell(g))) {
        push_message(g, "Not enough MP for Resurrection.");
        return 0;
    }
    return 1;
}

static void confirm_target(GameState *g, int magic) {
    char text[MAX_DIALOGUE_LEN];
    int hp = party_max_hp(g, g->resurrection_target);
    if (magic) {
        snprintf(text, sizeof(text), "Resurrect %s with %d / %d HP for %d MP?\nY: Perform ritual\nN / Esc: Cancel",
            party_name(g->resurrection_target), hp / 2, hp, action_spell_mp_cost(g, ritual_spell(g)));
    } else {
        snprintf(text, sizeof(text), "Resurrect %s at full health for %d gold?\nY: Pay and resurrect\nN / Esc: Cancel",
            party_name(g->resurrection_target), RESURRECTION_GOLD);
    }
    offer(g, magic ? RESURRECTION_SPELL_CONFIRM : RESURRECTION_PAID_CONFIRM, text);
}

static void select_target(GameState *g, int magic) {
    int count = party_fallen_count(g);
    if (count == 1) {
        g->resurrection_target = party_fallen_at(g, 0);
        confirm_target(g, magic);
        return;
    }
    char text[MAX_DIALOGUE_LEN];
    int used = snprintf(text, sizeof(text), "Choose a fallen companion:\n");
    for (int i = 0; i < count && used < (int)sizeof(text); i++) {
        used += snprintf(text + used, sizeof(text) - used, "%d %s\n", i + 1, party_name(party_fallen_at(g, i)));
    }
    if (used < (int)sizeof(text)) {
        snprintf(text + used, sizeof(text) - used, "Enter / Esc: Cancel");
    }
    offer(g, magic ? RESURRECTION_SPELL_TARGET : RESURRECTION_PAID_TARGET, text);
}

int resurrection_offer_spell(GameState *g) {
    if (!spell_ready(g)) {
        return 0;
    }
    select_target(g, 1);
    return 1;
}

int resurrection_prompt_active(const GameState *g) {
    return resurrection_available(g) && g->resurrection_prompt >= RESURRECTION_SERVICE && g->resurrection_prompt <= RESURRECTION_SPELL_TARGET &&
        g->dialogue_active && (strcmp(g->dialogue_speaker, speaker) == 0 || strcmp(g->dialogue_speaker, "Soulstone Shrine") == 0);
}

static int scroll_ready(GameState *g) {
    if (!has_party(g)) {
        push_message(g, "Recruit a companion before using the warlock's services.");
        return 0;
    }
    if (g->player.player_class != CLASS_MAGE || g->player.level < RESURRECTION_LEVEL) {
        push_message(g, "The Resurrection scroll requires a level 10 Mage.");
        return 0;
    }
    if (knows_resurrection(g)) {
        push_message(g, "You already know Resurrection.");
        return 0;
    }
    if (g->inventory_count >= MAX_INVENTORY) {
        push_message(g, "Make room in your bag for the Resurrection scroll.");
        return 0;
    }
    if (g->gold < RESURRECTION_SCROLL_GOLD) {
        push_message(g, "The Resurrection scroll costs 400 gold.");
        return 0;
    }
    return 1;
}

int resurrection_handle_key(GameState *g, int key, int repeat) {
    if (!resurrection_prompt_active(g)) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    int prompt = g->resurrection_prompt;
    if (key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_N || key == SDL_SCANCODE_RETURN) {
        g->resurrection_prompt = RESURRECTION_NONE;
        g->dialogue_active = 0;
        return 1;
    }
    if (prompt == RESURRECTION_SERVICE) {
        if (!near_warlock(g) || !peaceful(g)) {
            return 1;
        }
        if (key == SDL_SCANCODE_1) {
            if (!party_fallen_count(g)) {
                push_message(g, "No fallen party member needs resurrection.");
            } else {
                select_target(g, 0);
            }
        } else if (key == SDL_SCANCODE_2 && scroll_ready(g)) {
            offer(g, RESURRECTION_SCROLL_CONFIRM, "Buy Scroll: Resurrection for 400 gold? Read it from your inventory to learn the spell.\nY: Buy\nN / Esc: Cancel");
        }
        return 1;
    }
    if (prompt == RESURRECTION_PAID_TARGET || prompt == RESURRECTION_SPELL_TARGET) {
        int index = key - SDL_SCANCODE_1;
        int target = index >= 0 && index < 9 ? party_fallen_at(g, index) : -1;
        if (target >= 0) {
            g->resurrection_target = target;
            confirm_target(g, prompt == RESURRECTION_SPELL_TARGET);
        }
        return 1;
    }
    if (key != SDL_SCANCODE_Y) {
        return 1;
    }
    if (prompt == RESURRECTION_SPELL_CONFIRM) {
        if (!spell_ready(g)) {
            return 1;
        }
        int cost = action_spell_mp_cost(g, ritual_spell(g));
        if (!party_fallen(g, g->resurrection_target) ||
            !party_revive(g, g->resurrection_target, party_max_hp(g, g->resurrection_target) / 2)) {
            return 1;
        }
        g->player.mp -= cost;
        combat_feedback_add(g, FEEDBACK_MANA_LOSS, FEEDBACK_NOW, g->player.x, g->player.y, cost);
    } else {
        if (!near_warlock(g) || !peaceful(g)) {
            return 1;
        }
        if (prompt == RESURRECTION_SCROLL_CONFIRM) {
            if (!scroll_ready(g)) {
                return 1;
            }
            g->inventory[g->inventory_count++] = item_make_scroll_resurrection();
            g->gold -= RESURRECTION_SCROLL_GOLD;
            push_message(g, "Bought Scroll: Resurrection for 400 gold.");
        } else {
            if (g->gold < RESURRECTION_GOLD) {
                push_message(g, "Resurrection costs 200 gold. Return when you can pay.");
                return 1;
            }
            if (!party_fallen(g, g->resurrection_target) ||
                !party_revive(g, g->resurrection_target, party_max_hp(g, g->resurrection_target))) {
                return 1;
            }
            g->gold -= RESURRECTION_GOLD;
        }
    }
    g->resurrection_prompt = RESURRECTION_NONE;
    g->dialogue_active = 0;
    return (prompt == RESURRECTION_SPELL_CONFIRM || prompt == RESURRECTION_SPELL_TARGET) ? 2 : 1;
}
