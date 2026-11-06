#include "party.h"
#include "aldric.h"
#include "storm_mage.h"

const char *party_name(int member) {
    const char *names[PARTY_MEMBER_COUNT] = {"Aldric", STORM_MAGE_NAME};
    return member >= 0 && member < PARTY_MEMBER_COUNT ? names[member] : "Companion";
}

int party_recruited(const GameState *g, int member) {
    if (member == PARTY_ALDRIC) {
        return g->aldric_recruited || g->aldric_party_state == ALDRIC_FOLLOWING || g->aldric_party_state == ALDRIC_DEAD;
    }
    return member == PARTY_SOREN && g->storm_mage.recruited;
}

int party_fallen(const GameState *g, int member) {
    return member == PARTY_ALDRIC ? g->aldric_party_state == ALDRIC_DEAD :
        member == PARTY_SOREN && g->storm_mage.state == STORM_MAGE_DEAD;
}

int party_fallen_count(const GameState *g) {
    int count = 0;
    for (int i = 0; i < PARTY_MEMBER_COUNT; i++) {
        count += party_fallen(g, i);
    }
    return count;
}

int party_fallen_at(const GameState *g, int index) {
    for (int i = 0; i < PARTY_MEMBER_COUNT; i++) {
        if (party_fallen(g, i) && index-- == 0) {
            return i;
        }
    }
    return -1;
}

int party_max_hp(const GameState *g, int member) {
    return member == PARTY_ALDRIC ? ALDRIC_MAX_HP : member == PARTY_SOREN ? storm_mage_max_hp(g) : 0;
}

int party_revive(GameState *g, int member, int hp) {
    return member == PARTY_ALDRIC ? aldric_revive(g, hp) : member == PARTY_SOREN ? storm_mage_revive(g, hp) : 0;
}
