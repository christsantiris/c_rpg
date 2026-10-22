#include "healer.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int near_healer(const GameState *g) {
    return g->location == LOCATION_HEALER &&
        abs(g->player.x - HEALER_X) <= 1 && abs(g->player.y - HEALER_Y) <= 1;
}

static int heal_price(int hp) {
    return hp / HEALER_HP_PER_GOLD + (hp % HEALER_HP_PER_GOLD != 0);
}

void healer_enter(GameState *g) {
    if (g->location != LOCATION_TOWN2) {
        return;
    }
    g->location = LOCATION_HEALER;
    map_generate_workshop(&g->map, &g->player.x, &g->player.y);
    for (int y = TAVERN_Y + 1; y < TAVERN_Y + TAVERN_H - 1; y++) {
        for (int x = TAVERN_X + 1; x < TAVERN_X + TAVERN_W - 1; x++) {
            g->map.tiles[y][x] = TILE_TAVERN_FLOOR;
        }
    }
    for (int x = 10; x <= 13; x++) {
        g->map.tiles[9][x] = TILE_TAVERN_TABLE;
        g->map.tiles[9][x + 17] = TILE_TAVERN_TABLE;
    }
    g->map.tiles[HEALER_Y][HEALER_X] = TILE_NPC_HEALER;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    g->player.frozen_turns = 0;
    g->player.freeze_recovery = 0;
    push_message(g, "You enter Lysa's healing house. Speak with her for treatment.");
}

void healer_leave(GameState *g) {
    if (g->location != LOCATION_HEALER) {
        return;
    }
    game_enter_town2(g);
    g->player.x = TOWN_HEALER_DOOR_X;
    g->player.y = TOWN_HEALER_DOOR_Y + 1;
    push_message(g, "You step out of the healing house.");
}

int healer_offer(const GameState *g, int choice) {
    int missing = g->player.max_hp - g->player.hp;
    if (missing <= 0 || g->player.hp <= 0) {
        return 0;
    }
    if (choice == 1) {
        return missing;
    }
    if (choice == 2) {
        return missing / 2 + missing % 2;
    }
    if (choice == 3 && g->gold > 0) {
        return g->gold >= heal_price(missing) ? missing : g->gold * HEALER_HP_PER_GOLD;
    }
    return 0;
}

int healer_talk(GameState *g) {
    if (!near_healer(g)) {
        return 0;
    }
    g->dialogue_active = 1;
    g->dialogue_x = HEALER_X;
    g->dialogue_y = HEALER_Y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Lysa the Healer");
    if (g->player.hp >= g->player.max_hp) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text), "You are already at full health. No treatment is needed.");
    } else {
        int full = healer_offer(g, 1);
        int half = healer_offer(g, 2);
        int budget = healer_offer(g, 3);
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "1 gold per 10 HP.\nHP: %d/%d   Gold: %d\n1 Full: +%d HP, %dg%s\n2 Half: +%d HP, %dg%s\n3 Affordable: +%d HP, %dg",
            g->player.hp, g->player.max_hp, g->gold,
            full, heal_price(full), g->gold < heal_price(full) ? " *" : "",
            half, heal_price(half), g->gold < heal_price(half) ? " *" : "",
            budget, heal_price(budget));
    }
    return 1;
}

int healer_purchase(GameState *g, int choice) {
    if (!near_healer(g)) {
        return 0;
    }
    int hp = healer_offer(g, choice);
    int price = heal_price(hp);
    if (hp == 0 || g->gold < price) {
        push_message(g, g->player.hp >= g->player.max_hp ? "You are already at full health." : "Lysa: You do not have enough gold for that treatment.");
        return 0;
    }
    g->gold -= price;
    g->player.hp += hp;
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Lysa restores %d HP for %d gold.", hp, price);
    push_message(g, message);
    healer_talk(g);
    return 1;
}

int healer_handle_key(GameState *g, int scancode, int repeat) {
    if (g->location != LOCATION_HEALER || !g->dialogue_active ||
        strcmp(g->dialogue_speaker, "Lysa the Healer") != 0) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (scancode == SDL_SCANCODE_ESCAPE || scancode == SDL_SCANCODE_RETURN) {
        g->dialogue_active = 0;
    } else if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_3) {
        healer_purchase(g, scancode - SDL_SCANCODE_1 + 1);
    }
    return 1;
}
