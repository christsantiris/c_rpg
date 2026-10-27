#include "war_machine.h"
#include "combat_feedback.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int beside(const GameState *g, int x, int y) {
    return abs(g->player.x - x) + abs(g->player.y - y) <= 1;
}

static int near_veyra(const GameState *g) {
    return g->location == LOCATION_TOWN_HALL && abs(g->player.x - HALL_VEYRA_X) <= 1 && abs(g->player.y - HALL_VEYRA_Y) <= 1;
}

void war_machine_accept(GameState *g) {
    if (g->war_machine_quest_state == 1 && g->war_machine_progress == 7) {
        g->war_machine_quest_state = 2;
        push_message(g, "The weapons shipment is destroyed and the command seal is safe. Report to Veyra in Ridgeshire's Town Hall.");
    }
}

int war_machine_talk(GameState *g) {
    if (!near_veyra(g)) {
        return 0;
    }
    g->dialogue_active = 1;
    g->dialogue_x = HALL_VEYRA_X;
    g->dialogue_y = HALL_VEYRA_Y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", WAR_SPEAKER);
    if (!g->war_machine_quest_state) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Ironfang arms the goblins on Crown Road North. Read stage 2's office orders, dump stage 4's weapons by minecart, and take the stage 5 Mine Boss's command seal. Return for 160 gold.");
    } else if (g->war_machine_quest_state == 1) {
        if (!(g->war_machine_progress & WAR_ORDERS)) {
            snprintf(g->dialogue_text, sizeof(g->dialogue_text),
                "Find the shipment orders in Ironfang 2's northwest side office. The junction and eastern repair lead to stage 4's flooded supply platform. Bring the Mine Boss's seal from stage 5.");
        } else if (!(g->war_machine_progress & WAR_SABOTAGE)) {
            snprintf(g->dialogue_text, sizeof(g->dialogue_text),
                "Ironfang 4: repair the eastern rail, set the junction east, and ride the cart over the flooded trestle. Disembark beside the salvage cache; release the loading winch to sink the weapons.");
        } else {
            snprintf(g->dialogue_text, sizeof(g->dialogue_text),
                "The weapons are lost. Defeat Ironfang 5's Goblin Mine Boss and take his command seal from the coffer in his gallery. A previous victory counts. Bring the seal and orders back here.");
        }
    } else if (g->war_machine_quest_state == 2) {
        g->war_machine_quest_state = 3;
        g->gold += WAR_REWARD_GOLD;
        g->score += WAR_REWARD_SCORE;
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Their weapons lie at the bottom of the shaft, and this seal proves their commander is dead. You have broken Ironfang's next war shipment. Take 160 gold with Ridgeshire's thanks.");
        push_message(g, "Completed: Break the War Machine. +160 gold, +1000 score.");
    } else {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Ironfang's shipment will never reach the goblin soldiers. Their patrols remain dangerous, but your sabotage denied them fresh weapons.");
    }
    return 1;
}

// The existing Watchfires conversation keeps its own offer and payout. Topic
// keys are handled before Y/N so either quest can be chosen independently.
int war_machine_dialogue(const GameState *g) {
    return g->dialogue_active && near_veyra(g) &&
        (strcmp(g->dialogue_speaker, "Marshal Veyra") == 0 || strcmp(g->dialogue_speaker, WAR_SPEAKER) == 0);
}

int war_machine_handle_key(GameState *g, int key, int repeat) {
    if (!war_machine_dialogue(g) || (key != SDL_SCANCODE_1 && key != SDL_SCANCODE_2)) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (key == SDL_SCANCODE_2) {
        war_machine_talk(g);
    } else {
        game_talk_to_veyra(g);
    }
    return 1;
}

const char *war_machine_hint(const GameState *g) {
    if (g->location != LOCATION_IRONFANG || g->level < 1 || g->level > IRON_DEPTH || g->ironfang[g->level - 1].aboard) {
        return NULL;
    }
    if (g->level == 2 && beside(g, WAR_ORDERS_X, WAR_ORDERS_Y)) {
        return g->war_machine_progress & WAR_ORDERS ? "A: REREAD SHIPMENT ORDERS" : "A: READ SHIPMENT ORDERS";
    }
    if (g->level == 4 && !(g->war_machine_progress & WAR_SABOTAGE) && beside(g, WAR_WINCH_X, WAR_WINCH_Y)) {
        return "A: RELEASE WEAPONS WINCH";
    }
    if (g->level == 5 && !(g->war_machine_progress & WAR_SEAL) && beside(g, WAR_SEAL_X, WAR_SEAL_Y)) {
        return g->ironfang_defeated ? "A: RECOVER COMMAND SEAL" : "A: INSPECT COMMAND COFFER";
    }
    return NULL;
}

int war_machine_interact(GameState *g) {
    if (!war_machine_hint(g)) {
        return 0;
    }
    if (g->level == 2) {
        g->war_machine_progress |= WAR_ORDERS;
        push_message(g, "Shipment orders: swords, spearheads and armor for Crown Road North. The stockpile hangs over Ironfang 4's flooded shaft.");
        push_message(g, "Set the junction east, repair the eastern rail, then ride to the supply platform. Its loading winch can sink the weapons.");
    } else if (g->level == 4) {
        g->war_machine_progress |= WAR_SABOTAGE;
        combat_feedback_add(g, FEEDBACK_WEAPONS_DUMP, FEEDBACK_NOW, WAR_CARGO_X, WAR_CARGO_Y, 0);
        push_message(g, "You release the loading winch. Crates of goblin weapons crash into the flooded shaft! The return rail remains intact.");
        if (!(g->war_machine_progress & WAR_ORDERS)) {
            push_message(g, "Find the shipment orders in Ironfang 2's northwest side office to identify who armed the goblin soldiers.");
        }
    } else if (!g->ironfang_defeated) {
        push_message(g, "The Goblin Mine Boss guards his command seal. Defeat him before opening the coffer.");
        return 1;
    } else {
        g->war_machine_progress |= WAR_SEAL;
        push_message(g, "You recover the Goblin Mine Boss's command seal as proof. It travels with your quest discoveries, outside the pack.");
    }
    war_machine_accept(g);
    return 1;
}

int war_machine_marker(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_IRONFANG) {
        return 0;
    }
    if (g->level == 2 && !(g->war_machine_progress & WAR_ORDERS)) {
        *x = WAR_ORDERS_X;
        *y = WAR_ORDERS_Y;
        return 1;
    }
    if (g->level == 4 && !(g->war_machine_progress & WAR_SABOTAGE)) {
        *x = WAR_WINCH_X;
        *y = WAR_WINCH_Y;
        return 1;
    }
    if (g->level == 5 && !(g->war_machine_progress & WAR_SEAL)) {
        *x = WAR_SEAL_X;
        *y = WAR_SEAL_Y;
        return 1;
    }
    return 0;
}

int war_machine_valid(const GameState *g) {
    int state = g->war_machine_quest_state;
    int progress = g->war_machine_progress;
    return state >= 0 && state <= 3 && progress >= 0 && progress <= 7 &&
        (!(progress & WAR_SEAL) || g->ironfang_defeated) && (state != 1 || progress != 7) && (state < 2 || progress == 7);
}
