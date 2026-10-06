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
    const char *reward_item;
} QuestDefinition;

static const QuestDefinition quest_definitions[9] = {
    {
        "The Broken Seals", "Elowen",
        "Break through the undead guarding three shattered",
        "burial seals, then restore each seal.",
        {"Repair burial seal", "Repair burial seal", "Repair burial seal"},
        "Dungeon", {2, 3, 4}, 40, 300
    },
    {
        "Recover the Treasure Map", "Dain",
        "Recover the three treasure-map fragments carried",
        "by the Goblin Map Bearers.",
        {"Defeat Archer Map Bearer", "Defeat Bomber Map Bearer",
            "Defeat Shaman Map Bearer"},
        "Mountains", {1, 2, 3}, 60, 400
    },
    {
        "The Lost Wardens", "Alder",
        "Defeat the hunting parties guarding three lost",
        "wardens, then help each warden escape.",
        {"Rescue forest warden", "Rescue forest warden",
            "Rescue forest warden"},
        "Forest", {1, 2, 3}, 70, 500
    },
    {
        "Relight the Drowned Beacons", "Mara",
        "Lower the tides, fight through drowned guardians,",
        "and relight the three old coast beacons.",
        {"Light drowned beacon", "Light drowned beacon",
            "Light drowned beacon"},
        "Coast", {2, 3, 4}, 80, 600
    },
    {
        "The Buried Sun", "Nahla",
        "Defeat the temple guardian and recover the",
        "treasure buried beneath the solar vault.",
        {"Recover the buried treasure", "", ""},
        "Ruined Temple", {5, 0, 0}, 150, 2500
    },
    {
        "The Ivory Rook", "Rook",
        "Navigate the labyrinth, light five runes,",
        "defeat the Minotaur, and recover the ivory rook.",
        {"Recover the ivory rook", "", ""},
        "Stillbury Labyrinth", {5, 0, 0}, ROOK_QUEST_REWARD, 500
    },
    {
        "Bring Mira Home", "Bram",
        "Defeat the vampire holding Mira in the swamp,",
        "then speak to her and return to Bram at the inn.",
        {"Rescue Mira", "", ""},
        "Blackwater Swamp", {SWAMP_RESCUE_LEVEL, 0, 0}, 80, 600
    },
    {
        "The Dragon's Hoard", "Ilya",
        "Recover the golden goblet from the hoard",
        "on Dragonspine's fifth stage. Return to Ridgeshire.",
        {"Recover the golden goblet", "", ""},
        "Dragonspine", {5, 0, 0}, 0, 600, "Potion of Strength"
    },
    {
        "The Lost Magic Lamp", "Zara",
        "Recover a magic lamp from Sunscar Wastes",
        "and return it to the Guild in Rosemoor.",
        {"Recover the magic lamp", "", ""},
        "Sunscar Wastes", {DESERT_LAMP_LEVEL, 0, 0}, 80, 600
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
    if (quest == 5) {
        return g->rook_quest_state;
    }
    if (quest == 6) {
        return g->innkeeper_quest_state;
    }
    if (quest == 7) {
        return g->dragon_treasure_quest_state;
    }
    return g->sunscar_lamp_quest_state;
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
    if (quest == 5) {
        return g->rook_quest_state >= 2;
    }
    if (quest == 6) {
        return g->innkeeper_quest_state >= 2;
    }
    if (quest == 7) {
        return g->dragon_treasure_quest_state >= 2;
    }
    return g->sunscar_lamp_quest_state >= 2;
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
    for (int quest = 0; quest < 9; quest++) {
        if (quest_in_tab(quest_state(g, quest), tab)) {
            count++;
        }
    }
    return count;
}

int quest_journal_get_boss(const GameState *g, int index, BossJournalEntry *entry) {
    static const char *names[JOURNAL_BOSS_COUNT] = {
        "Lich King", "Necromancer", "Goblin King", "Drowned Queen",
        "Fallen Sun Guardian", "Swamp Demon", "Red Dragon", "Polar Kraken", "Desert Pharaoh", "Thorn Regent", "Cinder Lord", "Prism Sovereign", "Minotaur", "Grave Marshal"
    };
    static const char *areas[JOURNAL_BOSS_COUNT] = {
        "Dungeon", "Forest", "Goblin Mountains", "Sunken Coast",
        "Ruined Temple", "Blackwater Swamp", "Dragonspine", "Frostfell Wastes", "Sunscar Wastes", "Moonveil Gardens", "Ashen Hollow", "Glassdeep Caverns", "Labyrinth", "Royal Catacombs"
    };
    static const Location regions[JOURNAL_BOSS_COUNT] = {
        LOCATION_DUNGEON, LOCATION_FOREST, LOCATION_MOUNTAINS, LOCATION_COAST,
        LOCATION_TEMPLE, LOCATION_SWAMP, LOCATION_DRAGONSPINE, LOCATION_FROSTFELL,
        LOCATION_DESERT, LOCATION_MOONVEIL, LOCATION_ASHEN, LOCATION_GLASSDEEP, LOCATION_LABYRINTH, LOCATION_CATACOMBS
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
    for (int quest = 0; quest < 9; quest++) {
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
        entry->reward_item = definition->reward_item;
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

// close_key is the character's journal key; the journal's own keys win.
QuestJournalResult quest_journal_handle_key(QuestJournalScreen *screen, int scancode, int entry_count, int close_key) {
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
            return QUEST_JOURNAL_CLOSED;
        default:
            if (scancode == close_key) {
                return QUEST_JOURNAL_CLOSED;
            }
            break;
    }
    return QUEST_JOURNAL_NONE;
}
