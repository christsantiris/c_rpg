#include "royal_evidence.h"
#include <SDL2/SDL.h>
#include <stdio.h>

static const RoyalEvidence documents[] = {
    {ROYAL_EVIDENCE_FUNDING, "Royal Payment Records", "Recovered from the Goblin King",
        "Royal Treasury: release gold and weapons to the Goblin King for raids on Oakhaven's caravans. Continue payment while his forces hold the western road closed. By order of the King."},
    {ROYAL_EVIDENCE_PROTECTION, "Royal Writ of Protection", "Recovered from the Necromancer",
        "By royal command, the Necromancer's work in the eastern forest is under the King's protection. Patrols shall leave his grove undisturbed and permit his servants to pass."},
    {ROYAL_EVIDENCE_CONFISCATION, "Royal Confiscation Orders", "Recovered from Captain Rusk in Dawnmere",
        "By the King's seal: Dawnmere's refusal of the levies is rebellion. Seize its cargo and detain those who resist. Send goods and prisoners to Stillbury's customs house. Captain Rusk shall hold the river waterfront until relieved."}
};

int royal_evidence_found(const GameState *g) {
    return ((g->defeated_bosses & (1u << LOCATION_MOUNTAINS)) ? ROYAL_EVIDENCE_FUNDING : 0) |
        ((g->defeated_bosses & (1u << LOCATION_FOREST)) ? ROYAL_EVIDENCE_PROTECTION : 0) |
        (g->dawnmere_liberated ? ROYAL_EVIDENCE_CONFISCATION : 0);
}

int royal_evidence_count(const GameState *g) {
    int found = royal_evidence_found(g);
    return !!(found & ROYAL_EVIDENCE_FUNDING) + !!(found & ROYAL_EVIDENCE_PROTECTION) + !!(found & ROYAL_EVIDENCE_CONFISCATION);
}

const RoyalEvidence *royal_evidence_document(int id) {
    for (int i = 0; i < 3; i++) {
        if (documents[i].id == id) {
            return &documents[i];
        }
    }
    return NULL;
}

const RoyalEvidence *royal_evidence_entry(const GameState *g, int index) {
    int found = royal_evidence_found(g);
    for (int i = 0; i < 3; i++) {
        if ((found & documents[i].id) && index-- == 0) {
            return &documents[i];
        }
    }
    return NULL;
}

const char *royal_evidence_conclusion(const GameState *g) {
    if (g->dawnmere_liberated) {
        return "The King ordered Dawnmere punished. Seized goods and prisoners were sent to Stillbury customs.";
    }
    if (royal_evidence_count(g) == 2) {
        return "Both parchments bear the King's seal: the Crown funds the raids and protects the Necromancer.";
    }
    return "The King's seal implicates the Crown. Why is the King aiding these creatures?";
}

int royal_evidence_pending(const GameState *g) {
    if (g->player.hp <= 0 || g->game_won) {
        return ROYAL_EVIDENCE_NONE;
    }
    int unread = royal_evidence_found(g) & ~g->royal_evidence_seen;
    return unread & -unread;
}

int royal_evidence_handle_key(GameState *g, int key, int repeat) {
    int id = royal_evidence_pending(g);
    if (!id) {
        return 0;
    }
    if (!repeat && (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER || key == SDL_SCANCODE_SPACE || key == SDL_SCANCODE_ESCAPE)) {
        g->royal_evidence_seen |= id;
        char message[MAX_MESSAGE_LEN];
        snprintf(message, sizeof(message), "%s saved. Press %s and choose Evidence to read it again.",
            royal_evidence_document(id)->title, SDL_GetScancodeName((SDL_Scancode)g->key_bindings[CONTROL_QUEST_JOURNAL]));
        push_message(g, message);
    }
    return 1;
}
