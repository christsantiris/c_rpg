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
        entry.reward_gold == 40 && entry.reward_score == 300);
    ASSERT("journal translates objective progress bits",
        entry.objective_complete[0] && !entry.objective_complete[1] &&
        entry.objective_complete[2]);
    ASSERT("ready quests remain active until turned in",
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 1, &entry) &&
        entry.state == 2 && strcmp(entry.giver, "Dain") == 0);
    ASSERT("completed tab retains turned-in quests",
        quest_journal_get_entry(&g, QUEST_TAB_COMPLETED, 0, &entry) &&
        entry.state == 3 && strcmp(entry.giver, "Alder") == 0);
    g.rook_quest_state = 1;
    ASSERT("Rook's labyrinth retrieval appears as an active quest",
        quest_journal_count(&g, QUEST_TAB_ACTIVE) == 3 &&
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 2, &entry) &&
        strcmp(entry.title, "Rook's Marker") == 0 &&
        entry.objective_count == 1 && entry.reward_gold == ROOK_QUEST_REWARD);
    g.rook_quest_state = 2;
    ASSERT("recovering the ivory rook marks its objective ready to return",
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 2, &entry) &&
        entry.objective_complete[0] && entry.state == 2);
    g.rook_quest_state = 3;
    ASSERT("a returned ivory rook is retained in completed quests",
        quest_journal_count(&g, QUEST_TAB_COMPLETED) == 2);

    QuestJournalScreen screen;
    quest_journal_init(&screen);
    quest_journal_handle_key(&screen, SDL_SCANCODE_DOWN, 2);
    ASSERT("journal moves selection", screen.selected == 1);
    quest_journal_handle_key(&screen, SDL_SCANCODE_TAB, 2);
    ASSERT("journal changes tabs and resets selection",
        screen.tab == QUEST_TAB_COMPLETED && screen.selected == 0);
    quest_journal_handle_key(&screen, SDL_SCANCODE_TAB, 1);
    ASSERT("boss progress is reachable after completed quests",
        screen.tab == QUEST_TAB_BOSSES &&
        quest_journal_count(&g, screen.tab) == JOURNAL_BOSS_COUNT);
    quest_journal_handle_key(&screen, SDL_SCANCODE_RIGHT, JOURNAL_BOSS_COUNT);
    ASSERT("right wraps from bosses to active quests", screen.tab == QUEST_TAB_ACTIVE);
    quest_journal_handle_key(&screen, SDL_SCANCODE_LEFT, 2);
    ASSERT("left wraps from active quests to bosses", screen.tab == QUEST_TAB_BOSSES);
    const Location regions[JOURNAL_BOSS_COUNT] = {
        LOCATION_DUNGEON, LOCATION_FOREST, LOCATION_MOUNTAINS, LOCATION_COAST,
        LOCATION_TEMPLE
    };
    const char *names[JOURNAL_BOSS_COUNT] = {
        "Lich King", "Necromancer", "Goblin King", "Drowned Queen",
        "Fallen Sun Guardian"
    };
    for (int defeated = 0; defeated < JOURNAL_BOSS_COUNT; defeated++) {
        g.defeated_bosses = 1 << regions[defeated];
        for (int i = 0; i < JOURNAL_BOSS_COUNT; i++) {
            BossJournalEntry boss;
            ASSERT("journal maps each regional defeat to the correct boss",
                quest_journal_get_boss(&g, i, &boss) &&
                strcmp(boss.name, names[i]) == 0 &&
                boss.defeated == (i == defeated));
        }
    }
    BossJournalEntry boss;
    ASSERT("boss journal rejects invalid rows",
        !quest_journal_get_boss(&g, -1, &boss) &&
        !quest_journal_get_boss(&g, JOURNAL_BOSS_COUNT, &boss));
    ASSERT("boss rows are not presented as quests",
        !quest_journal_get_entry(&g, QUEST_TAB_BOSSES, 0, &entry));
    ASSERT("Q closes the quest journal",
        quest_journal_handle_key(&screen, SDL_SCANCODE_Q, 1) ==
            QUEST_JOURNAL_CLOSED);
}
