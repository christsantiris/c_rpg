#include "test_utils.h"
#include "../src/screens/quest_journal.h"
#include <string.h>

void test_quest_journal(void) {
    printf("Quest journal tests:\n");
    static GameState g;
    memset(&g, 0, sizeof(g));
    g.elowen_quest_state = 1;
    g.dain_quest_state = 2;
    g.dain_map_fragments = 7;
    g.alder_quest_state = 3;
    g.alder_wardens_rescued = 7;

    ASSERT("journal lists accepted active quests only",
        quest_journal_count(&g, QUEST_TAB_ACTIVE) == 1);
    ASSERT("finished objectives free an active slot before reward collection",
        game_active_quest_count(&g) == 1);
    ASSERT("completed quests include pending and collected rewards",
        quest_journal_count(&g, QUEST_TAB_COMPLETED) == 2);

    QuestJournalEntry entry;
    ASSERT("journal exposes Elowen quest details",
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "Save Elowen") == 0 &&
        strcmp(entry.giver, "Bram") == 0 &&
        entry.reward_gold == 80 && entry.reward_score == 0 && entry.objective_count == 1 && entry.stages[0] == DUNGEON_DEPTH);
    ASSERT("the rescue objective stays unfinished until the Lich falls",
        !entry.objective_complete[0]);
    ASSERT("completed quests retain pending reward details",
        quest_journal_get_entry(&g, QUEST_TAB_COMPLETED, 0, &entry) &&
        entry.state == 2 && strcmp(entry.giver, "Dain") == 0);
    ASSERT("completed tab retains turned-in quests",
        quest_journal_get_entry(&g, QUEST_TAB_COMPLETED, 1, &entry) &&
        entry.state == 3 && strcmp(entry.giver, "Alder") == 0);
    ASSERT("Alder's journal places both rangers on the first two stages", entry.stages[0] == 1 && entry.stages[1] == 2 && entry.objective_count == 2);
    g.rook_quest_state = 1;
    ASSERT("Rook's Minotaur hunt appears as an active quest",
        quest_journal_count(&g, QUEST_TAB_ACTIVE) == 2 &&
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 1, &entry) &&
        strcmp(entry.title, "Slay the Minotaur") == 0 &&
        entry.objective_count == 1 && entry.reward_gold == ROOK_QUEST_REWARD &&
        strstr(entry.summary_line_1, "Stillbury") && strstr(entry.summary_line_2, "no return to Rook"));
    g.rook_quest_state = 2;
    ASSERT("legacy completed labyrinth quests remain completed",
        quest_journal_get_entry(&g, QUEST_TAB_COMPLETED, 2, &entry) &&
        entry.objective_complete[0] && entry.state == 2 &&
        quest_journal_count(&g, QUEST_TAB_COMPLETED) == 3);
    g.rook_quest_state = 3;
    ASSERT("a slain Minotaur is retained in completed quests",
        quest_journal_count(&g, QUEST_TAB_COMPLETED) == 3);
    g.mara_quest_state = 1;
    g.mara_beacons_lit = MARA_BEACON_STAGE_2 | MARA_BEACON_STAGE_4;
    ASSERT("Rowan's journal shows stages two through four and preserved completion bits",
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 1, &entry) &&
        strcmp(entry.giver, "Captain Rowan") == 0 && strstr(entry.summary_line_2, "Stillbury") && entry.stages[0] == 2 &&
        entry.stages[1] == 3 && entry.stages[2] == 4 &&
        entry.objective_complete[0] && !entry.objective_complete[1] && entry.objective_complete[2]);
    g.mara_quest_state = 0;
    for (int state = 1; state <= 3; state++) {
        g.innkeeper_quest_state = state;
        ASSERT("the retired Mira quest never appears in either journal tab",
            quest_journal_count(&g, QUEST_TAB_ACTIVE) == 1 &&
            quest_journal_count(&g, QUEST_TAB_COMPLETED) == 3 &&
            game_active_quest_count(&g) == 1);
    }
    g.elowen_quest_state = 0;
    g.dain_quest_state = 0;
    g.dragon_treasure_quest_state = 1;
    ASSERT("Ilya's dragon treasure quest appears with the strength reward",
        quest_journal_count(&g, QUEST_TAB_ACTIVE) == 1 &&
        quest_journal_get_entry(&g, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "The Dragon's Hoard") == 0 &&
        strcmp(entry.reward_item, "Potion of Strength") == 0 && strstr(entry.summary_line_2, "Oakhaven's Inn"));
    g.dragon_treasure_quest_state = 2;
    ASSERT("recovered dragon treasure completes the quest with its reward pending",
        quest_journal_get_entry(&g, QUEST_TAB_COMPLETED, 2, &entry) &&
        entry.objective_complete[0] && entry.state == 2);
    g.dragon_treasure_quest_state = 3;
    ASSERT("Ilya's quest stays in the completed journal",
        quest_journal_count(&g, QUEST_TAB_COMPLETED) == 3);

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
    ASSERT("evidence follows bosses without changing existing quest tabs", screen.tab == QUEST_TAB_EVIDENCE);
    quest_journal_handle_key(&screen, SDL_SCANCODE_RIGHT, 0, SDL_SCANCODE_Q);
    ASSERT("right wraps from evidence to active quests", screen.tab == QUEST_TAB_ACTIVE);
    quest_journal_handle_key(&screen, SDL_SCANCODE_LEFT, 2, SDL_SCANCODE_Q);
    ASSERT("left wraps from active quests to evidence", screen.tab == QUEST_TAB_EVIDENCE);
    const Location regions[JOURNAL_BOSS_COUNT] = {
        LOCATION_DUNGEON, LOCATION_FOREST, LOCATION_MOUNTAINS, LOCATION_COAST,
        LOCATION_TEMPLE, LOCATION_SWAMP, LOCATION_DRAGONSPINE, LOCATION_FROSTFELL,
        LOCATION_DESERT, LOCATION_ASHEN, LOCATION_GLASSDEEP, LOCATION_LABYRINTH, LOCATION_CATACOMBS, LOCATION_CASTLE_INTERIOR, LOCATION_CASTLE_INTERIOR, LOCATION_CASTLE_INTERIOR, LOCATION_ELDERWOOD, LOCATION_MANGROVES, LOCATION_STORMWATCH, LOCATION_IRONFANG, LOCATION_GIANTS_CROWN
    };
    const char *names[JOURNAL_BOSS_COUNT] = {
        "Lich King", "Necromancer", "Goblin King", "Drowned Queen",
        "Fallen Sun Guardian", "Swamp Demon", "Red Dragon", "Frost Kraken", "Desert Pharaoh", "Cinder Lord", "Quarry Colossus", "Minotaur", "Grave Marshal", "Castellan", "Royal Arcanist", "Lord Veyr", "Elderthorn", "Sszareth, Keeper of the Roots", "Tempest Matriarch", "Goblin Mine Boss", "The Last Jarl"
    };
    for (int defeated = 0; defeated < JOURNAL_BOSS_COUNT; defeated++) {
        g.defeated_bosses = defeated < 13 || defeated == 15 ? 1 << regions[defeated] : 0;
        g.elderwood_defeated = defeated == 16;
        g.mangrove_defeated = defeated == 17;
        g.stormwatch_defeated = defeated == 18;
        g.ironfang_defeated = defeated == 19;
        g.giants_crown_defeated = defeated == 20;
        g.castle_minibosses = defeated == 13 ? 1 : defeated == 14 ? 2 : 0;
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
