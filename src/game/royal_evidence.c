#include "ridgeshire.h"
#include "royal_evidence.h"
#include "war_machine.h"
#include <SDL2/SDL.h>
#include <stdio.h>

static const RoyalEvidence documents[] = {
    {ROYAL_EVIDENCE_REQUISITIONS, "Malakar's Requisitions", "Recovered from Ironfang's side office",
        "Divert royal steel to the goblin forces. Supply components for the royal binding apparatus. Send reports only to Malakar, Royal Arcanist; withhold copies from the King's household."},
    {ROYAL_EVIDENCE_BINDING, "The Dragon's Runic Shackle", "Recovered after defeating the Red Dragon",
        "The runes suppress another creature's will. Edric identifies the pattern described in Malakar's binding requisitions. The dragon's violence was directed by magic."},
    {ROYAL_EVIDENCE_CROWN, "Royal Crown Inventory", "Examined by Archivist Oswin in Ridgeshire",
        "Malakar commissioned runes for the King's ceremonial crown. Oswin recognizes a binding pattern. Compare the inventory with Ironfang's requisitions and the dragon's shackle to establish whether the crown suppresses the King's will."},

    {ROYAL_EVIDENCE_FUNDING, "Royal Payment Records", "Recovered from the Goblin King",
        "Royal Treasury: release gold and weapons to the Goblin King for raids on Oakhaven's caravans. Continue payment while his forces hold the western road closed. By order of the King."},
    {ROYAL_EVIDENCE_PROTECTION, "Royal Writ of Protection", "Recovered from the Necromancer",
        "By royal command, the Necromancer's work in the eastern forest is under the King's protection. Patrols shall leave his grove undisturbed and permit his servants to pass."},
    {ROYAL_EVIDENCE_CONFISCATION, "Royal Confiscation Orders", "Recovered from Captain Rusk in Dawnmere",
        "By the King's seal: Dawnmere's refusal of the levies is rebellion. Seize its cargo and detain those who resist. Send goods and prisoners to Stillbury's customs house. Captain Rusk shall hold the river waterfront until relieved."},
    {ROYAL_EVIDENCE_TRIBUTE, "The Pharaoh's Agreement", "Recovered from the Pharaoh in Sunscar Wastes",
        "By the King's seal: confiscated goods shall be sent as tribute to the Pharaoh. Stillbury customs will record the shipments and release payment to the royal caravans. In return, the Pharaoh shall maintain his forces in Sunscar."},
    {ROYAL_EVIDENCE_CHARTER, "The Jarl's Royal Charter", "Recovered from the Last Jarl in Giant's Peaks",
        "By royal command: the Last Jarl shall hold the mountain road south of Giant's Bridge. Permit only shipments bearing a Stillbury customs seal. Keep unlicensed travelers and merchants from the crossing. The Crown guarantees his claim to the pass."},
    {ROYAL_EVIDENCE_PRISONERS, "Labyrinth Intake Register", "Recovered from the Minotaur's lair",
        "Prisoners received under royal authority, transferred from Stillbury customs. Authorization: Malakar, Royal Arcanist. "
        "Deliver the condemned to the inner maze. Send all returns to the Arcanist's private office; no copies to the royal court. "
        "The last column records their delivery. No departures are recorded."}
};

int royal_evidence_found(const GameState *g) {
    return ((g->war_machine_progress & WAR_ORDERS) ? ROYAL_EVIDENCE_REQUISITIONS : 0) |
        ((g->defeated_bosses & (1u << LOCATION_DRAGONSPINE)) ? ROYAL_EVIDENCE_BINDING : 0) |
        (g->ridgeshire.crown_record ? ROYAL_EVIDENCE_CROWN : 0) |
        ((g->defeated_bosses & (1u << LOCATION_MOUNTAINS)) ? ROYAL_EVIDENCE_FUNDING : 0) |
        ((g->defeated_bosses & (1u << LOCATION_FOREST)) ? ROYAL_EVIDENCE_PROTECTION : 0) |
        (g->dawnmere_liberated ? ROYAL_EVIDENCE_CONFISCATION : 0) |
        ((g->defeated_bosses & (1u << LOCATION_DESERT)) ? ROYAL_EVIDENCE_TRIBUTE : 0) |
        (g->giants_crown_defeated ? ROYAL_EVIDENCE_CHARTER : 0) |
        ((g->defeated_bosses & (1u << LOCATION_LABYRINTH)) ? ROYAL_EVIDENCE_PRISONERS : 0);
}

int royal_evidence_count(const GameState *g) {
    int found = royal_evidence_found(g);
    int count = 0;
    for (int i = 0; i < (int)(sizeof(documents) / sizeof(documents[0])); i++) {
        count += !!(found & documents[i].id);
    }
    return count;
}

const RoyalEvidence *royal_evidence_document(int id) {
    for (int i = 0; i < (int)(sizeof(documents) / sizeof(documents[0])); i++) {
        if (documents[i].id == id) {
            return &documents[i];
        }
    }
    return NULL;
}

const RoyalEvidence *royal_evidence_entry(const GameState *g, int index) {
    int found = royal_evidence_found(g);
    for (int i = 0; i < (int)(sizeof(documents) / sizeof(documents[0])); i++) {
        if ((found & documents[i].id) && index-- == 0) {
            return &documents[i];
        }
    }
    return NULL;
}

const char *royal_evidence_conclusion(const GameState *g) {
    if (g->ridgeshire.crown_record && (g->war_machine_progress & WAR_ORDERS) && (g->defeated_bosses & (1u << LOCATION_DRAGONSPINE))) {
        return "Malakar's requisitions, the dragon's shackle and the crown inventory show that he controls the King through binding magic.";
    }
    if ((g->war_machine_progress & WAR_ORDERS) || (g->defeated_bosses & (1u << LOCATION_DRAGONSPINE))) {
        return "Malakar's binding magic directs creatures and diverts royal supplies. Archivist Oswin in Ridgeshire can compare the evidence with the King's crown records.";
    }
    if (royal_evidence_found(g) & ROYAL_EVIDENCE_PRISONERS) {
        return "Malakar's private office directed the disappearances under royal authority and kept the records from the royal court. What does the King know?";
    }
    if (royal_evidence_found(g) & (ROYAL_EVIDENCE_TRIBUTE | ROYAL_EVIDENCE_CHARTER)) {
        return "Royal tribute and control of the crossing point to Stillbury customs. The King is financing evil and restricting travel.";
    }
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
