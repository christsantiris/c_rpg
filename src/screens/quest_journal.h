#ifndef QUEST_JOURNAL_HEADER_H
#define QUEST_JOURNAL_HEADER_H

#include "../game/game.h"

typedef enum {
    QUEST_TAB_ACTIVE = 0,
    QUEST_TAB_COMPLETED
} QuestJournalTab;

typedef enum {
    QUEST_JOURNAL_NONE = 0,
    QUEST_JOURNAL_CLOSED
} QuestJournalResult;

typedef struct {
    const char *title;
    const char *giver;
    const char *summary_line_1;
    const char *summary_line_2;
    const char *objectives[3];
    const char *area;
    int stages[3];
    int objective_complete[3];
    int objective_count;
    int reward_gold;
    int reward_score;
    int state;
} QuestJournalEntry;

typedef struct {
    QuestJournalTab tab;
    int selected;
} QuestJournalScreen;

void quest_journal_init(QuestJournalScreen *screen);
int quest_journal_count(const GameState *g, QuestJournalTab tab);
int quest_journal_get_entry(const GameState *g, QuestJournalTab tab, int index, QuestJournalEntry *entry);
QuestJournalResult quest_journal_handle_key(QuestJournalScreen *screen, int scancode, int entry_count);

#endif
