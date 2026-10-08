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
    ASSERT("Alder's journal places objectives on both sides of the central grove", entry.stages[0] == 1 && entry.stages[1] == 2 && entry.stages[2] == 5);
    g.rook_quest_state = 1;
    ASSERT("Rook's labyrinth retrieval appears as an active quest",
        quest_journal_count(&g, QUEST_TAB_ACTIVE) == 3 &&
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 2, &entry) &&
        strcmp(entry.title, "The Ivory Rook") == 0 &&
        entry.objective_count == 1 && entry.reward_gold == ROOK_QUEST_REWARD);
    g.rook_quest_state = 2;
    ASSERT("recovering the ivory rook marks its objective ready to return",
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 2, &entry) &&
        entry.objective_complete[0] && entry.state == 2);
    g.rook_quest_state = 3;
    ASSERT("a returned ivory rook is retained in completed quests",
        quest_journal_count(&g, QUEST_TAB_COMPLETED) == 2);
    g.mara_quest_state = 1;
    g.mara_beacons_lit = MARA_BEACON_STAGE_2 | MARA_BEACON_STAGE_4;
    ASSERT("Mara's journal shows stages two through four and preserved completion bits",
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 2, &entry) &&
        strcmp(entry.giver, "Mara") == 0 && entry.stages[0] == 2 &&
        entry.stages[1] == 3 && entry.stages[2] == 4 &&
        entry.objective_complete[0] && !entry.objective_complete[1] && entry.objective_complete[2]);
    g.mara_quest_state = 0;
    g.innkeeper_quest_state = 1;
    ASSERT("Bram's rescue appears as a swamp quest",
        quest_journal_count(&g, QUEST_TAB_ACTIVE) == 3 &&
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 2, &entry) &&
        strcmp(entry.title, "Bring Mira Home") == 0 &&
        entry.stages[0] == SWAMP_RESCUE_LEVEL);
    g.innkeeper_quest_state = 2;
    ASSERT("rescuing Mira marks Bram's quest ready to return",
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 2, &entry) &&
        entry.objective_complete[0]);
    g.innkeeper_quest_state = 3;
    ASSERT("Bram's completed quest moves to the completed tab",
        quest_journal_count(&g, QUEST_TAB_COMPLETED) == 3);
    g.elowen_quest_state = 0;
    g.dain_quest_state = 0;
    g.dragon_treasure_quest_state = 1;
    ASSERT("Ilya's dragon treasure quest appears with the strength reward",
        quest_journal_count(&g, QUEST_TAB_ACTIVE) == 1 &&
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "The Dragon's Hoard") == 0 &&
        strcmp(entry.reward_item, "Potion of Strength") == 0 && strstr(entry.summary_line_2, "Oakhaven's Tavern"));
    g.dragon_treasure_quest_state = 2;
    ASSERT("recovered dragon treasure is ready to return",
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 0, &entry) &&
        entry.objective_complete[0] && entry.state == 2);
    g.dragon_treasure_quest_state = 3;
    ASSERT("Ilya's quest stays in the completed journal",
        quest_journal_count(&g, QUEST_TAB_COMPLETED) == 4);

    QuestJournalScreen screen;
    quest_journal_init(&screen);
    quest_journal_handle_key(&screen, SDL_SCANCODE_DOWN, 2, SDL_SCANCODE_Q);
    ASSERT("journal moves selection", screen.selected == 1);
    quest_journal_handle_key(&screen, SDL_SCANCODE_TAB, 2, SDL_SCANCODE_Q);
    ASSERT("journal changes tabs and resets selection",
        screen.tab == QUEST_TAB_COMPLETED && screen.selected == 0);
    quest_journal_handle_key(&screen, SDL_SCANCODE_TAB, 1, SDL_SCANCODE_Q);
    ASSERT("boss progress is reachable after completed quests",
        screen.tab == QUEST_TAB_BOSSES &&
        quest_journal_count(&g, screen.tab) == JOURNAL_BOSS_COUNT);
    quest_journal_handle_key(&screen, SDL_SCANCODE_RIGHT, JOURNAL_BOSS_COUNT, SDL_SCANCODE_Q);
    ASSERT("right wraps from bosses to active quests", screen.tab == QUEST_TAB_ACTIVE);
    quest_journal_handle_key(&screen, SDL_SCANCODE_LEFT, 2, SDL_SCANCODE_Q);
    ASSERT("left wraps from active quests to bosses", screen.tab == QUEST_TAB_BOSSES);
    const Location regions[JOURNAL_BOSS_COUNT] = {
        LOCATION_DUNGEON, LOCATION_FOREST, LOCATION_MOUNTAINS, LOCATION_COAST,
        LOCATION_TEMPLE, LOCATION_SWAMP, LOCATION_DRAGONSPINE, LOCATION_FROSTFELL,
        LOCATION_DESERT, LOCATION_MOONVEIL, LOCATION_ASHEN, LOCATION_GLASSDEEP, LOCATION_LABYRINTH, LOCATION_CATACOMBS, LOCATION_CASTLE_INTERIOR, LOCATION_CASTLE_INTERIOR, LOCATION_CASTLE_INTERIOR
    };
    const char *names[JOURNAL_BOSS_COUNT] = {
        "Lich King", "Necromancer", "Goblin King", "Drowned Queen",
        "Fallen Sun Guardian", "Swamp Demon", "Red Dragon", "Polar Kraken", "Desert Pharaoh", "Thorn Regent", "Cinder Lord", "Prism Sovereign", "Minotaur", "Grave Marshal", "Castellan", "Royal Arcanist", "Lord Veyr"
    };
    for (int defeated = 0; defeated < JOURNAL_BOSS_COUNT; defeated++) {
        g.defeated_bosses = defeated < 14 || defeated == 16 ? 1 << regions[defeated] : 0;
        g.castle_minibosses = defeated == 14 ? 1 : defeated == 15 ? 2 : 0;
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
        quest_journal_handle_key(&screen, SDL_SCANCODE_Q, 1, SDL_SCANCODE_Q) ==
            QUEST_JOURNAL_CLOSED);
}
