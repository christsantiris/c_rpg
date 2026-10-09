#include "quest_journal.h"
#include <SDL2/SDL.h>
#include "../game/catacombs.h"
#include "../game/jail.h"

typedef struct {
    const char *title;
    const char *giver;
    const char *summary_line_1;
    const char *summary_line_2;
    const char *objectives[4];
    const char *area;
    int stages[4];
    int reward_gold;
    int reward_score;
    const char *reward_item;
} QuestDefinition;

static const QuestDefinition quest_definitions[15] = {
    {
        "The Broken Seals", "Elowen",
        "Restore three guarded seals in Oakhaven's dungeon.",
        "Return to Elowen in Oakhaven's Tavern.",
        {"Repair burial seal", "Repair burial seal", "Repair burial seal"},
        "Dungeon", {2, 3, 4}, 40, 300
    },
    {
        "Recover the Treasure Map", "Dain",
        "Recover the three Goblin Map Bearer fragments.",
        "Return to Dain in Rosemoor's Adventurer's Guild.",
        {"Defeat Archer Map Bearer", "Defeat Bomber Map Bearer",
            "Defeat Shaman Map Bearer"},
        "Mountains", {1, 2, 5}, 60, 400
    },
    {
        "The Lost Wardens", "Alder",
        "Defeat the captors and rescue three lost wardens.",
        "Return to Alder in Stillbury's Inn.",
        {"Rescue forest warden", "Rescue forest warden",
            "Rescue forest warden"},
        "Forest", {1, 2, 5}, 70, 500
    },
    {
        "Relight the Drowned Beacons", "Mara",
        "Lower the tides and relight three guarded beacons.",
        "Return to Mara in Oakhaven's Tavern.",
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
        "on Dragonspine's fifth stage. Return to Oakhaven's Tavern.",
        {"Recover the golden goblet", "", ""},
        "Dragonspine", {5, 0, 0}, 0, 600, "Potion of Strength"
    },
    {
        "The Lost Magic Lamp", "Zara",
        "Recover a magic lamp from Sunscar Wastes",
        "and return it to the Guild in Rosemoor.",
        {"Recover the magic lamp", "", ""},
        "Sunscar Wastes", {DESERT_LAMP_LEVEL, 0, 0}, 80, 600
    },
    {
        "Reclaim the Emberforge", "Steward Hadrin",
        "Recover the stolen mechanism, defeat the forge's",
        "defenders, and repair the abandoned furnace.",
        {"Recover the forge mechanism", "Restore the Emberforge", ""},
        "Ashen Hollow", {EMBERFORGE_MECHANISM_LEVEL, EMBERFORGE_FURNACE_LEVEL, 0},
        EMBERFORGE_REWARD_GOLD, EMBERFORGE_REWARD_SCORE
    },
    {
        "The Silent Expedition", "Quartermaster Brenna",
        "Recover the journal and rescue Surveyor Fen.",
        "Return to Brenna in Stillbury's Inn.",
        {"Recover the expedition journal", "Rescue Surveyor Fen", ""},
        "Frostfell Wastes", {FROSTFELL_JOURNAL_LEVEL, FROSTFELL_SURVIVOR_LEVEL, 0},
        FROSTFELL_REWARD_GOLD, FROSTFELL_REWARD_SCORE
    },
    {
        "The Broken Resonance", "Surveyor Orin",
        "Read the inscriptions and restore three resonators.",
        "Return to Orin in Rosemoor's Adventurer's Guild.",
        {"Restore Root Resonator", "Restore Tide Resonator", "Restore Crown Resonator"},
        "Glassdeep Caverns", {2, 3, 4}, GLASSDEEP_REWARD_GOLD, GLASSDEEP_REWARD_SCORE
    },
    {
        "The Stolen Moonseed", "Botanist Liora",
        "Recover the Moonseed, gather moonwater, and plant",
        "the ancient circle. Return to Stillbury's Inn.",
        {"Recover the Moonseed", "Gather moonwater", "Restore the planting circle"},
        "Moonveil Gardens", {2, 3, 4}, MOONVEIL_REWARD_GOLD, MOONVEIL_REWARD_SCORE
    },
    {
        "Rest for the Forgotten", "Brother Oswin",
        "Silence three memorials and recover the royal ledger.",
        "Return to Oswin in Ridgeshire's Town Hall.",
        {"Silence Soldiers' memorial", "Silence Watchers' memorial", "Silence Choir memorial", "Recover the burial ledger"},
        "Royal Catacombs", {2, 3, 4, 5}, CATACOMBS_REWARD_GOLD, CATACOMBS_REWARD_SCORE
    },
    {
        "Guide Tomas Home", "Tomas",
        "Escape the royal jail through the hidden tunnel.",
        "Guide your fellow prisoner to central Ridgeshire.",
        {"Find the secret tunnel", "Escort Tomas to Ridgeshire"},
        "Escape Tunnel", {1, 1, 0, 0}, ESCAPE_REWARD_GOLD, ESCAPE_REWARD_SCORE
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
    if (quest == 8) {
        return g->sunscar_lamp_quest_state;
    }
    if (quest == 9) {
        return g->emberforge_quest_state;
    }
    if (quest == 10) {
        return g->frostfell_quest_state;
    }
    if (quest == 11) {
        return g->glassdeep_quest_state;
    }
    if (quest == 12) {
        return g->moonveil_quest_state;
    }
    if (quest == 13) {
        return g->catacombs_quest_state;
    }
    return g->jail_quest_state == 3 ? 3 : g->jail_quest_state == 2 ? 1 : 0;
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
    if (quest == 8) {
        return g->sunscar_lamp_quest_state >= 2;
    }
    if (quest == 9) {
        return g->emberforge_progress;
    }
    if (quest == 10) {
        return g->frostfell_quest_progress;
    }
    if (quest == 11) {
        return g->glassdeep_quest_progress;
    }
    if (quest == 12) {
        return g->moonveil_quest_progress;
    }
    if (quest == 13) {
        return g->catacombs_quest_progress;
    }
    return g->jail_quest_state == 3 ? 3 : g->location == LOCATION_ESCAPE_TUNNEL ? 1 : 0;
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
    for (int quest = 0; quest < 15; quest++) {
        if (quest_in_tab(quest_state(g, quest), tab)) {
            count++;
        }
    }
    return count;
}

int quest_journal_get_boss(const GameState *g, int index, BossJournalEntry *entry) {
    static const char *names[JOURNAL_BOSS_COUNT] = {
        "Lich King", "Necromancer", "Goblin King", "Drowned Queen",
        "Fallen Sun Guardian", "Swamp Demon", "Red Dragon", "Polar Kraken", "Desert Pharaoh", "Thorn Regent", "Cinder Lord", "Prism Sovereign", "Minotaur", "Grave Marshal", "Castellan", "Royal Arcanist", "Lord Veyr"
    };
    static const char *areas[JOURNAL_BOSS_COUNT] = {
        "Dungeon", "Forest", "Goblin Mountains", "Sunken Coast",
        "Ruined Temple", "Blackwater Swamp", "Dragonspine", "Frostfell Wastes", "Sunscar Wastes", "Moonveil Gardens", "Ashen Hollow", "Glassdeep Caverns", "Labyrinth", "Royal Catacombs", "Castle: Iron Keep", "Castle: Crown Chapel", "Castle: Throne"
    };
    static const Location regions[JOURNAL_BOSS_COUNT] = {
        LOCATION_DUNGEON, LOCATION_FOREST, LOCATION_MOUNTAINS, LOCATION_COAST,
        LOCATION_TEMPLE, LOCATION_SWAMP, LOCATION_DRAGONSPINE, LOCATION_FROSTFELL,
        LOCATION_DESERT, LOCATION_MOONVEIL, LOCATION_ASHEN, LOCATION_GLASSDEEP, LOCATION_LABYRINTH, LOCATION_CATACOMBS, LOCATION_CASTLE_INTERIOR, LOCATION_CASTLE_INTERIOR, LOCATION_CASTLE_INTERIOR
    };
    if (index < 0 || index >= JOURNAL_BOSS_COUNT) {
        return 0;
    }
    entry->name = names[index];
    entry->area = areas[index];
    entry->defeated = index == 14 ? !!(g->castle_minibosses & 1) : index == 15 ? !!(g->castle_minibosses & 2) :
        (g->defeated_bosses & (1 << regions[index])) != 0;
    return 1;
}

int quest_journal_get_entry(const GameState *g, QuestJournalTab tab, int index, QuestJournalEntry *entry) {
    if (tab == QUEST_TAB_BOSSES) {
        return 0;
    }
    int visible_index = 0;
    for (int quest = 0; quest < 15; quest++) {
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
        entry->objective_count = definition->stages[3] ? 4 : definition->stages[2] ? 3 : definition->stages[1] ? 2 : 1;
        entry->reward_gold = definition->reward_gold;
        entry->reward_score = definition->reward_score;
        entry->reward_item = definition->reward_item;
        entry->state = state;
        for (int objective = 0; objective < 4; objective++) {
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
