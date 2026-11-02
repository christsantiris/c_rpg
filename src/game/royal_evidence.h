#ifndef ROYAL_EVIDENCE_H
#define ROYAL_EVIDENCE_H

#include "game.h"

enum {
    ROYAL_EVIDENCE_NONE = 0,
    ROYAL_EVIDENCE_FUNDING = 1,
    ROYAL_EVIDENCE_PROTECTION = 2
};

typedef struct {
    int id;
    const char *title;
    const char *source;
    const char *text;
} RoyalEvidence;

int royal_evidence_found(const GameState *g);
int royal_evidence_count(const GameState *g);
const RoyalEvidence *royal_evidence_document(int id);
const RoyalEvidence *royal_evidence_entry(const GameState *g, int index);
const char *royal_evidence_conclusion(const GameState *g);
int royal_evidence_pending(const GameState *g);
int royal_evidence_handle_key(GameState *g, int key, int repeat);

#endif
