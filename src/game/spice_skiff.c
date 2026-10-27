#include "spice_skiff.h"
#include "mangrove.h"
#include "town_life.h"
#include <stdio.h>
#include <stdlib.h>

const char spice_skiff_offer[] = "My spice skiff vanished in Saltwind Marsh, south of Dawnmere. "
    "Search stage 2's northern island. Bring its manifest and sealed spice casket here for 150 gold.";

static void sync_state(GameState *g) {
    if (g->spice_skiff_quest_state == 1 && g->spice_skiff_progress == 3) {
        g->spice_skiff_quest_state = 2;
        push_message(g, "The manifest and spice casket are safe. Return to Corvin in Dawnmere's Spice Merchant shop.");
    }
}

void spice_skiff_talk(GameState *g) {
    if (g->location != LOCATION_DAWNMERE_MERCHANT) {
        return;
    }
    g->dialogue_active = 1;
    g->dialogue_x = RESIDENT_X;
    g->dialogue_y = RESIDENT_Y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Spice Merchant Corvin");
    if (!g->spice_skiff_quest_state) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s", spice_skiff_offer);
    } else if (g->spice_skiff_quest_state == 1) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Manifest: %s; cargo: %s. Saltwind 2: northern island. Saltwind 3: western root arch; dock from the east. "
            "Defeat the island guards, then return both here.",
            g->spice_skiff_progress & SPICE_MANIFEST ? "found" : "missing",
            g->spice_skiff_progress & SPICE_CARGO ? "recovered" : "missing");
    } else if (g->spice_skiff_quest_state == 2) {
        g->spice_skiff_quest_state = 3;
        g->gold += SPICE_REWARD_GOLD;
        g->score += SPICE_REWARD_SCORE;
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "My shipment, still sealed! With the manifest I can account for every sack. "
            "Take 150 gold with my thanks. Dawnmere's kitchens will smell of good spices again.");
        push_message(g, "Completed: The Missing Spice Skiff. +150 gold, +1000 score.");
    } else {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Your recovered spices are safely in my storeroom. Thank you for following the channels when others turned back.");
    }
}

void spice_skiff_accept(GameState *g) {
    sync_state(g);
}

const char *spice_skiff_hint(const GameState *g) {
    if (g->location != LOCATION_MANGROVES || g->mangroves.aboard) {
        return NULL;
    }
    if (g->level == 2 && abs(g->player.x - SPICE_WRECK_X) + abs(g->player.y - SPICE_WRECK_Y) <= 1) {
        return g->spice_skiff_progress & SPICE_MANIFEST ? "REREAD CARGO MANIFEST" : "SEARCH STRANDED SKIFF";
    }
    if (g->level == 3 && !(g->spice_skiff_progress & SPICE_CARGO) &&
        abs(g->player.x - SPICE_CASKET_X) + abs(g->player.y - SPICE_CASKET_Y) <= 1) {
        return "RECOVER SPICE CASKET";
    }
    return NULL;
}

static int cargo_guarded(const GameState *g) {
    unsigned char seen[MANGROVE_H][MANGROVE_W] = {{0}};
    int queue[MANGROVE_W * MANGROVE_H];
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    int head = 0;
    int tail = 1;
    queue[0] = SPICE_CASKET_Y * MANGROVE_W + SPICE_CASKET_X;
    seen[SPICE_CASKET_Y][SPICE_CASKET_X] = 1;
    while (head < tail) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % MANGROVE_W + dx[i];
            int y = cell / MANGROVE_W + dy[i];
            if (x < 1 || x >= MANGROVE_W - 1 || y < 1 || y >= MANGROVE_H - 1 ||
                seen[y][x] || !map_is_walkable(&g->map, x, y)) {
                continue;
            }
            seen[y][x] = 1;
            queue[tail++] = y * MANGROVE_W + x;
        }
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (e->active && e->x >= 0 && e->x < MANGROVE_W && e->y >= 0 && e->y < MANGROVE_H && seen[e->y][e->x]) {
            return 1;
        }
    }
    return 0;
}

int spice_skiff_interact(GameState *g) {
    if (!spice_skiff_hint(g)) {
        return 0;
    }
    if (g->level == 2) {
        g->spice_skiff_progress |= SPICE_MANIFEST;
        push_message(g, "The skiff's manifest: spices moved to the western island of Saltwind 3, beneath an arched mangrove root.");
        push_message(g, "Reach its east-facing timber landing by skiff. The manifest is kept with your quest discoveries.");
    } else {
        if (cargo_guarded(g)) {
            push_message(g, "Giant Crabs and Lizardfolk guard this island. Defeat its shore guards before recovering the spices.");
            return 1;
        }
        g->spice_skiff_progress |= SPICE_CARGO;
        push_message(g, "You recover Corvin's sealed spice casket. It travels safely with your quest discoveries, outside the pack.");
    }
    sync_state(g);
    return 1;
}

int spice_skiff_marker(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_MANGROVES) {
        return 0;
    }
    if (g->level == 2 && !(g->spice_skiff_progress & SPICE_MANIFEST)) {
        *x = SPICE_WRECK_X;
        *y = SPICE_WRECK_Y;
        return 1;
    }
    if (g->level == 3 && !(g->spice_skiff_progress & SPICE_CARGO)) {
        *x = SPICE_CASKET_X;
        *y = SPICE_CASKET_Y;
        return 1;
    }
    return 0;
}
