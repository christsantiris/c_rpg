#include "test_utils.h"
#include "../src/screens/quest_journal.h"
#include <string.h>

void test_quest_journal(void) {
    printf("Quest journal tests:\n");
    static GameState g;
    memset(&g, 0, sizeof(g));
    g.elowen_quest_state = 1;
    g.elowen_seals_restored = 5;
    g.dain_quest_state = 2;
    g.dain_map_fragments = 7;
    g.alder_quest_state = 3;
    g.alder_wardens_rescued = 7;

    ASSERT("journal lists accepted active quests only",
        quest_journal_count(&g, QUEST_TAB_ACTIVE) == 2);
    ASSERT("journal lists completed quests separately",
        quest_journal_count(&g, QUEST_TAB_COMPLETED) == 1);

    QuestJournalEntry entry;
    ASSERT("journal exposes Elowen quest details",
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "The Broken Seals") == 0 &&
        strcmp(entry.giver, "Elowen") == 0 &&
        entry.reward_gold == 100 && entry.reward_score == 300);
    ASSERT("journal translates objective progress bits",
        entry.objective_complete[0] && !entry.objective_complete[1] &&
        entry.objective_complete[2]);
    ASSERT("ready quests remain active until turned in",
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 1, &entry) &&
        entry.state == 2 && strcmp(entry.giver, "Dain") == 0);
    ASSERT("completed tab retains turned-in quests",
        quest_journal_get_entry(&g, QUEST_TAB_COMPLETED, 0, &entry) &&
        entry.state == 3 && strcmp(entry.giver, "Alder") == 0);

    QuestJournalScreen screen;
    quest_journal_init(&screen);
    quest_journal_handle_key(&screen, SDL_SCANCODE_DOWN, 2);
    ASSERT("journal moves selection", screen.selected == 1);
    quest_journal_handle_key(&screen, SDL_SCANCODE_TAB, 2);
    ASSERT("journal changes tabs and resets selection",
        screen.tab == QUEST_TAB_COMPLETED && screen.selected == 0);
    ASSERT("Q closes the quest journal",
        quest_journal_handle_key(&screen, SDL_SCANCODE_Q, 1) ==
            QUEST_JOURNAL_CLOSED);
}
