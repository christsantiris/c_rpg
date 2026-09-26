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

static const QuestDefinition quest_definitions[6] = {
    {
        "The Broken Seals", "Elowen",
        "Break through the undead guarding three shattered",
        "burial seals, then restore each seal.",
        {"Repair burial seal", "Repair burial seal", "Repair burial seal"},
        "Dungeon", {2, 4, 6}, 40, 300
    },
    {
        "Recover the Treasure Map", "Dain",
        "Recover the three treasure-map fragments carried",
        "by the Goblin Map Bearers.",
        {"Defeat Archer Map Bearer", "Defeat Bomber Map Bearer",
            "Defeat Shaman Map Bearer"},
        "Mountains", {2, 3, 5}, 60, 400
    },
    {
        "The Lost Wardens", "Alder",
        "Defeat the hunting parties guarding three lost",
        "wardens, then help each warden escape.",
        {"Rescue forest warden", "Rescue forest warden",
            "Rescue forest warden"},
        "Forest", {2, 5, 7}, 70, 500
    },
    {
        "Relight the Drowned Beacons", "Mara",
        "Lower the tides, fight through drowned guardians,",
        "and relight the three old coast beacons.",
        {"Light drowned beacon", "Light drowned beacon",
            "Light drowned beacon"},
        "Coast", {1, 3, 6}, 80, 600
    },
    {
        "The Buried Sun", "Nahla",
        "Defeat the temple guardian and recover the",
        "treasure buried beneath the solar vault.",
        {"Recover the buried treasure", "", ""},
        "Ruined Temple", {4, 0, 0}, 150, 2500
    },
    {
        "The Ivory Rook", "Rook",
        "Navigate the enemy-free labyrinth, light three",
        "runes, and recover Rook's stolen ivory rook.",
        {"Recover the ivory rook", "", ""},
        "Town Labyrinth", {1, 0, 0}, ROOK_QUEST_REWARD, 500
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
    if (quest == 3) {
        return g->mara_quest_state;
    }
    if (quest == 4) {
        return g->temple_treasure_state;
    }
    return g->rook_quest_state;
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
    if (quest == 3) {
        return g->mara_beacons_lit;
    }
    if (quest == 4) {
        return g->temple_treasure_state >= 2 ? 1 : 0;
    }
    return g->rook_quest_state >= 2 ? 1 : 0;
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
    if (tab == QUEST_TAB_BOSSES) {
        return JOURNAL_BOSS_COUNT;
    }
    int count = 0;
    for (int quest = 0; quest < 6; quest++) {
        if (quest_in_tab(quest_state(g, quest), tab)) {
            count++;
        }
    }
    return count;
}

int quest_journal_get_boss(const GameState *g, int index, BossJournalEntry *entry) {
    static const char *names[JOURNAL_BOSS_COUNT] = {
        "Lich King", "Necromancer", "Goblin King", "Drowned Queen",
        "Fallen Sun Guardian"
    };
    static const char *areas[JOURNAL_BOSS_COUNT] = {
        "Dungeon", "Forest", "Goblin Mountains", "Sunken Coast",
        "Ruined Temple"
    };
    static const Location regions[JOURNAL_BOSS_COUNT] = {
        LOCATION_DUNGEON, LOCATION_FOREST, LOCATION_MOUNTAINS, LOCATION_COAST,
        LOCATION_TEMPLE
    };
    if (index < 0 || index >= JOURNAL_BOSS_COUNT) {
        return 0;
    }
    entry->name = names[index];
    entry->area = areas[index];
    entry->defeated = (g->defeated_bosses & (1 << regions[index])) != 0;
    return 1;
}

int quest_journal_get_entry(const GameState *g, QuestJournalTab tab, int index, QuestJournalEntry *entry) {
    if (tab == QUEST_TAB_BOSSES) {
        return 0;
    }
    int visible_index = 0;
    for (int quest = 0; quest < 6; quest++) {
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
        entry->objective_count = quest >= 4 ? 1 : 3;
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
            screen->tab = (screen->tab + 2) % 3;
            screen->selected = 0;
            break;
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_TAB:
            screen->tab = (screen->tab + 1) % 3;
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
