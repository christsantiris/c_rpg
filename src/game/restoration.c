#include "restoration.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Location location;
    TileType npc;
    int door_x;
    int door_y;
    int mana;
    const char *speaker;
    const char *unit;
} RestorationService;

static const RestorationService services[] = {
    {LOCATION_HEALER, TILE_NPC_HEALER, TOWN_HEALER_DOOR_X, TOWN_HEALER_DOOR_Y, 0, "Lysa the Healer", "HP"},
    {LOCATION_WITCH, TILE_NPC_WITCH, TOWN_WITCH_DOOR_X, TOWN_WITCH_DOOR_Y, 1, "Morwen the Witch", "MP"}
};

static const RestorationService *service_at(Location location) {
    for (int i = 0; i < (int)(sizeof(services) / sizeof(services[0])); i++) {
        if (services[i].location == location) {
            return &services[i];
        }
    }
    return NULL;
}

static int near_service(const GameState *g) {
    return service_at(g->location) && abs(g->player.x - RESTORATION_X) <= 1 &&
        abs(g->player.y - RESTORATION_Y) <= 1;
}

static int missing_resource(const GameState *g, const RestorationService *service) {
    return service->mana ? g->player.max_mp - g->player.mp : g->player.max_hp - g->player.hp;
}

static int treatment_price(int amount) {
    return amount / RESTORATION_PER_GOLD + (amount % RESTORATION_PER_GOLD != 0);
}

void restoration_enter(GameState *g, Location location) {
    const RestorationService *service = service_at(location);
    if (g->location != LOCATION_TOWN2 || !service) {
        return;
    }
    g->location = location;
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
    g->map.tiles[RESTORATION_Y][RESTORATION_X] = service->npc;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    g->player.poison_turns = 0;
    g->player.frozen_turns = 0;
    g->player.freeze_recovery = 0;
    push_message(g, service->mana ? "You enter Morwen's hut. Speak with her to restore your mana." :
        "You enter Lysa's healing house. Speak with her for treatment.");
}

void restoration_leave(GameState *g) {
    const RestorationService *service = service_at(g->location);
    if (!service) {
        return;
    }
    game_enter_town2(g);
    g->player.x = service->door_x;
    g->player.y = service->door_y + 1;
    push_message(g, service->mana ? "You step out of the witch's hut." : "You step out of the healing house.");
}

int restoration_offer(const GameState *g, int choice) {
    const RestorationService *service = service_at(g->location);
    if (!service) {
        return 0;
    }
    int missing = missing_resource(g, service);
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
        return g->gold >= treatment_price(missing) ? missing : g->gold * RESTORATION_PER_GOLD;
    }
    return 0;
}

int restoration_talk(GameState *g) {
    if (!near_service(g)) {
        return 0;
    }
    const RestorationService *service = service_at(g->location);
    g->dialogue_active = 1;
    g->dialogue_x = RESTORATION_X;
    g->dialogue_y = RESTORATION_Y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", service->speaker);
    if (missing_resource(g, service) <= 0) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text), "Your %s is already full. No treatment is needed.", service->unit);
    } else {
        int full = restoration_offer(g, 1);
        int half = restoration_offer(g, 2);
        int budget = restoration_offer(g, 3);
        int current = service->mana ? g->player.mp : g->player.hp;
        int maximum = service->mana ? g->player.max_mp : g->player.max_hp;
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "1 gold per 10 %s.\n%s: %d/%d   Gold: %d\n1 Full: +%d %s, %dg%s\n2 Half: +%d %s, %dg%s\n3 Affordable: +%d %s, %dg",
            service->unit, service->unit, current, maximum, g->gold,
            full, service->unit, treatment_price(full), g->gold < treatment_price(full) ? " *" : "",
            half, service->unit, treatment_price(half), g->gold < treatment_price(half) ? " *" : "",
            budget, service->unit, treatment_price(budget));
    }
    return 1;
}

int restoration_purchase(GameState *g, int choice) {
    if (!near_service(g)) {
        return 0;
    }
    const RestorationService *service = service_at(g->location);
    int amount = restoration_offer(g, choice);
    int price = treatment_price(amount);
    if (amount == 0 || g->gold < price) {
        push_message(g, missing_resource(g, service) <= 0 ? "No treatment is needed." : "You do not have enough gold for that treatment.");
        return 0;
    }
    g->gold -= price;
    if (service->mana) {
        g->player.mp += amount;
    } else {
        g->player.hp += amount;
    }
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "%s restores %d %s for %d gold.", service->speaker, amount, service->unit, price);
    push_message(g, message);
    restoration_talk(g);
    return 1;
}

int restoration_prompt_active(const GameState *g) {
    const RestorationService *service = service_at(g->location);
    return service && g->dialogue_active && strcmp(g->dialogue_speaker, service->speaker) == 0;
}

int restoration_handle_key(GameState *g, int scancode, int repeat) {
    if (!restoration_prompt_active(g)) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (scancode == SDL_SCANCODE_ESCAPE || scancode == SDL_SCANCODE_RETURN) {
        g->dialogue_active = 0;
    } else if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_3) {
        restoration_purchase(g, scancode - SDL_SCANCODE_1 + 1);
    }
    return 1;
}
