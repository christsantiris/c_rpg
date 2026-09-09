#include "quest_journal.h"
#include <SDL2/SDL.h>

typedef struct {
    const char *title;
    const char *giver;
    const char *summary_line_1;
    const char *summary_line_2;
    const char *objectives[3];
    const char *area;
    int stages[3];
    int reward_gold;
    int reward_score;
} QuestDefinition;

static const QuestDefinition quest_definitions[4] = {
    {
        "The Broken Seals", "Elowen",
        "Repair the shattered burial seals before their",
        "necrotic power spreads through the crypt.",
        {"Repair burial seal", "Repair burial seal", "Repair burial seal"},
        "Dungeon", {2, 4, 6}, 100, 300
    },
    {
        "Recover the Treasure Map", "Dain",
        "Recover the three treasure-map fragments carried",
        "by the Goblin Map Bearers.",
        {"Defeat Archer Map Bearer", "Defeat Bomber Map Bearer",
            "Defeat Shaman Map Bearer"},
        "Mountains", {2, 3, 5}, 150, 400
    },
    {
        "The Lost Wardens", "Alder",
        "Find the wardens trapped in the forest's",
        "dead-end groves and help them escape.",
        {"Rescue forest warden", "Rescue forest warden",
            "Rescue forest warden"},
        "Forest", {2, 5, 7}, 175, 500
    },
    {
        "Relight the Drowned Beacons", "Mara",
        "Lower the tides and relight the old beacons",
        "that once guided travelers along the coast.",
        {"Light drowned beacon", "Light drowned beacon",
            "Light drowned beacon"},
        "Coast", {1, 3, 6}, 200, 600
    }
};

static int quest_state(const GameState *g, int quest) {
    if (quest == 0) {
        return g->elowen_quest_state;
    }
    if (quest == 1) {
        return g->dain_quest_state;
    }
    if (quest == 2) {
        return g->alder_quest_state;
    }
    return g->mara_quest_state;
}

static int quest_progress(const GameState *g, int quest) {
    if (quest == 0) {
        return g->elowen_seals_restored;
    }
    if (quest == 1) {
        return g->dain_map_fragments;
    }
    if (quest == 2) {
        return g->alder_wardens_rescued;
    }
    return g->mara_beacons_lit;
}

static int quest_in_tab(int state, QuestJournalTab tab) {
    if (tab == QUEST_TAB_COMPLETED) {
        return state == 3;
    }
    return state == 1 || state == 2;
}

void quest_journal_init(QuestJournalScreen *screen) {
    screen->tab = QUEST_TAB_ACTIVE;
    screen->selected = 0;
}

int quest_journal_count(const GameState *g, QuestJournalTab tab) {
    int count = 0;
    for (int quest = 0; quest < 4; quest++) {
        if (quest_in_tab(quest_state(g, quest), tab)) {
            count++;
        }
    }
    return count;
}

int quest_journal_get_entry(const GameState *g, QuestJournalTab tab, int index, QuestJournalEntry *entry) {
    int visible_index = 0;
    for (int quest = 0; quest < 4; quest++) {
        int state = quest_state(g, quest);
        if (!quest_in_tab(state, tab)) {
            continue;
        }
        if (visible_index++ != index) {
            continue;
        }
        const QuestDefinition *definition = &quest_definitions[quest];
        int progress = quest_progress(g, quest);
        entry->title = definition->title;
        entry->giver = definition->giver;
        entry->summary_line_1 = definition->summary_line_1;
        entry->summary_line_2 = definition->summary_line_2;
        entry->area = definition->area;
        entry->objective_count = 3;
        entry->reward_gold = definition->reward_gold;
        entry->reward_score = definition->reward_score;
        entry->state = state;
        for (int objective = 0; objective < 3; objective++) {
            entry->objectives[objective] =
                definition->objectives[objective];
            entry->stages[objective] = definition->stages[objective];
            entry->objective_complete[objective] =
                (progress & (1 << objective)) != 0;
        }
        return 1;
    }
    return 0;
}

QuestJournalResult quest_journal_handle_key(QuestJournalScreen *screen, int scancode, int entry_count) {
    switch (scancode) {
        case SDL_SCANCODE_UP:
            if (screen->selected > 0) {
                screen->selected--;
            }
            break;
        case SDL_SCANCODE_DOWN:
            if (screen->selected + 1 < entry_count) {
                screen->selected++;
            }
            break;
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_TAB:
            screen->tab = screen->tab == QUEST_TAB_ACTIVE
                ? QUEST_TAB_COMPLETED : QUEST_TAB_ACTIVE;
            screen->selected = 0;
            break;
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_Q:
            return QUEST_JOURNAL_CLOSED;
        default:
            break;
    }
    return QUEST_JOURNAL_NONE;
}
