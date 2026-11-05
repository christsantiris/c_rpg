#include "../game/western_arc.h"
#include "quest_journal.h"
#include "../game/royal_evidence.h"
#include <SDL2/SDL.h>
#include "../game/catacombs.h"
#include "../game/jail.h"
#include "../game/elowen.h"
#include "../game/heartseed.h"
#include "../game/spice_skiff.h"
#include "../game/stormbell.h"
#include "../game/war_machine.h"
#include "../game/crown_quest.h"

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
    const char *objective_areas[4];
} QuestDefinition;

static const QuestDefinition quest_definitions[JOURNAL_QUEST_COUNT] = {
    {
        "Save Elowen", "Bram",
        "Defeat the Lich King and free Bram's daughter from Oakhaven's dungeon.",
        "Elowen follows you. Speak to Bram in Oakhaven's Inn to reunite them and claim your reward.",
        {"Defeat the Lich King and free Elowen"},
        "Dungeon", {DUNGEON_DEPTH}, ELOWEN_REWARD_GOLD, 0
    },
    {
        "Recover the Treasure Map", "Dain",
        "Recover two treasure-map halves from the Goblin Map Bearers.",
        "Return and speak to Dain in Oakhaven's Inn.",
        {"Recover first half from Archer", "Recover second half from Warchief"},
        "Goblin Mountains", {1, 2}, 60, 400
    },
    {
        "The Lost Rangers", "Alder",
        "Defeat the spider captors and free two lost rangers.",
        "Return and speak to Alder in Oakhaven's Inn.",
        {"Free first ranger from spiders", "Free second ranger from spiders"},
        "Haunted Forest", {1, 2}, 70, 500
    },
    {
        "Relight the Drowned Beacons", "Mara",
        "Lower the tides and relight three guarded beacons.",
        "Return and speak to Mara in Oakhaven's Inn.",
        {"Light drowned beacon", "Light drowned beacon",
            "Light drowned beacon"},
        "Sunken Coast", {2, 3, 4}, 80, 600
    },
    {
        "The Buried Sun", "Nahla",
        "Defeat the temple guardian and recover the buried treasure.",
        "Return and speak to Nahla by the temple on the Ruined Isle.",
        {"Recover the buried treasure", "", ""},
        "Ruined Temple", {5, 0, 0}, 150, 2500
    },
    {
        "The Ivory Rook", "Rook",
        "Find the labyrinth on the road from Oakhaven to Dawnmere; light five runes and defeat the Minotaur.",
        "Return the ivory rook to Rook in Oakhaven's Inn.",
        {"Recover the ivory rook", "", ""},
        "Rook's Labyrinth", {5, 0, 0}, ROOK_QUEST_REWARD, 500
    },
    {
        "The Dragon's Hoard", "Ilya",
        "Recover the golden goblet from Dragonspine's fifth stage.",
        "Return and speak to Ilya in Oakhaven's Inn.",
        {"Recover the golden goblet", "", ""},
        "Dragonspine", {5, 0, 0}, 0, 600, "Potion of Strength"
    },
    {
        "The King's Tribute", "Zara",
        "Recover confiscated goods and proof of the King's tribute to the Pharaoh.",
        "Return to Zara at the caravan camp outside Sunscar for your reward.",
        {"Reclaim the royal convoy's cargo", "Recover the Pharaoh's agreement"},
        "Sunscar Wastes", {2, 5}, TRIBUTE_GOLD, 0, "500 XP"
    },
    {
        "Reclaim the Emberforge", "Steward Hadrin",
        "Recover the mechanism, defeat the guards, and repair the forge.",
        "Return and speak to Steward Hadrin in Ridgeshire's Town Hall.",
        {"Recover the forge mechanism", "Restore the Emberforge", ""},
        "Ashen Hollow", {EMBERFORGE_MECHANISM_LEVEL, EMBERFORGE_FURNACE_LEVEL, 0},
        EMBERFORGE_REWARD_GOLD, EMBERFORGE_REWARD_SCORE
    },
    {
        "The Silent Expedition", "Quartermaster Brenna",
        "Recover the journal and rescue Surveyor Fen.",
        "Return and speak to Brenna in Stillbury's Tavern.",
        {"Recover the expedition journal", "Rescue Surveyor Fen", ""},
        "Frostfell Wastes", {FROSTFELL_JOURNAL_LEVEL, FROSTFELL_SURVIVOR_LEVEL, 0},
        FROSTFELL_REWARD_GOLD, FROSTFELL_REWARD_SCORE
    },
    {
        "Reclaim the Quarry", "Surveyor Orin",
        "Reclaim guarded machinery on quarry stages 2, 3 and 4.",
        "Return and speak to Orin in Rosemoor's Adventurer's Guild.",
        {"Repair West Winch", "Repair Workshop Hoist", "Repair Main Gantry"},
        "Stonewake Quarry", {2, 3, 4}, GLASSDEEP_REWARD_GOLD, GLASSDEEP_REWARD_SCORE, "Magic Stone Warhammer"
    },
    {
        "Rest for the Forgotten", "Brother Oswin",
        "Silence three memorials and recover the royal ledger.",
        "Return and speak to Brother Oswin in Ridgeshire's Town Hall.",
        {"Silence Soldiers' memorial", "Silence Watchers' memorial", "Silence Choir memorial", "Recover the burial ledger"},
        "Royal Catacombs", {2, 3, 4, 5}, CATACOMBS_REWARD_GOLD, CATACOMBS_REWARD_SCORE
    },
    {
        "Guide Tomas Home", "Tomas",
        "Escape through the hidden tunnel and escort Tomas home.",
        "Tomas rewards you on arrival in Ridgeshire's town center.",
        {"Find the secret tunnel", "Escort Tomas to Ridgeshire"},
        "Escape Tunnel", {1, 1, 0, 0}, ESCAPE_REWARD_GOLD, ESCAPE_REWARD_SCORE
    },
    {
        "Watchfires of Ridgeshire", "Marshal Veyra",
        "Clear the defenders and restore three watchfires.",
        "Return and speak to Marshal Veyra in Ridgeshire's Town Hall.",
        {"Restore southern watchfire", "Restore Ashen watchfire", "Restore Dragonspine watchfire"},
        "Regional watchfires", {WATCHFIRE_MOUNTAINS_LEVEL, WATCHFIRE_ASHEN_LEVEL, WATCHFIRE_DRAGONSPINE_LEVEL},
        WATCHFIRE_REWARD_GOLD, WATCHFIRE_REWARD_SCORE, NULL,
        {"Goblin Mountains", "Ashen Hollow", "Dragonspine"}
    },
    {
        "The Three Great Hunts", "Huntmaster Selene",
        "Defeat each named leader and its entire hunt group.",
        "Return and speak to Selene in Rosemoor's Adventurer's Guild.",
        {"Defeat Rimefang's pack", "Defeat Shardwarden's group", "Defeat Dunehex's group"},
        "Guild hunting routes", {HUNT_FROSTFELL_LEVEL, HUNT_GLASSDEEP_LEVEL, HUNT_SUNSCAR_LEVEL},
        HUNT_REWARD_GOLD, HUNT_REWARD_SCORE, NULL,
        {"Frostfell Wastes", "Stonewake Quarry", "Sunscar Wastes"}
    },
    {
        "Recover Your Equipment", "Royal Dungeon",
        "Return to the Royal Dungeon after escaping with Tomas.",
        "Defeat its Royal Guards; reclaim every item from the evidence chest.",
        {"Recover all confiscated belongings and gold"},
        "Royal Dungeon", {1, 0, 0, 0}, 0, 0, "Your belongings, arrows and gold"
    },
    {
        "The Stolen Heartseed", "Forester Tamsin",
        "Read the orders; find stage 4's eastern golden-root grove.",
        "Cut three bindings, recover the seed; return to Tamsin in Oakhaven.",
        {"Read Dark Elf orders", "Cut all three bindings", "Recover the heartseed"},
        "Elderwood Forest", {2, 4, 4, 0}, HEARTSEED_REWARD_GOLD, HEARTSEED_REWARD_SCORE
    },
    {
        "The Missing Spice Skiff", "Spice Merchant Corvin",
        "Stage 3: western island, root arch, east-facing landing.",
        "Return both to Corvin in Dawnmere's Spice Merchant shop.",
        {"Inspect the stranded skiff's manifest", "Recover the sealed spice casket"},
        "Saltwind Marsh", {2, 3, 0, 0}, SPICE_REWARD_GOLD, SPICE_REWARD_SCORE
    },
    {
        "The Silent Stormbell", "Watchkeeper Seren",
        "Recover the clapper in stage 3; fit and ring stage 4's bell.",
        "Defeat two Raiders, then the Stormcaller; return to Seren in Dawnmere.",
        {"Recover the bell clapper", "Fit the clapper and restore the bell", "Defeat the bell's harpy defenders"},
        "Stormwatch Headland", {3, 4, 4, 0}, STORMBELL_REWARD_GOLD, STORMBELL_REWARD_SCORE
    },
    {
        "Break the War Machine", "Marshal Veyra",
        "Read stage 2's orders; ride to stage 4's supply platform.",
        "Sink the weapons; take the Mine Boss's seal to Veyra in Ridgeshire.",
        {"Recover shipment orders", "Dump the suspended weapons", "Recover the Mine Boss's command seal"},
        "Ironfang Mines", {2, 4, 5, 0}, WAR_REWARD_GOLD, WAR_REWARD_SCORE
    },
    {
        "Giant's Crown", "Historian Liora",
        "Recover the crown directly from the Last Jarl in Giant's Peaks 5.",
        "Deliver it to Edda beside Giant's Bridge, outside Stillbury's checkpoint.",
        {"Recover the Last Jarl's crown"},
        "Giant's Peaks", {5}, CROWN_REWARD_GOLD, CROWN_REWARD_SCORE, "Stoneguard Shield"
    },
    {
        "Defeat the Necromancer", "Alder",
        "Defeat the Necromancer and search his correspondence for whoever protects his work.",
        "His defeat opens the forest road east to Dawnmere. Continue your adventure there.",
        {"Defeat the Necromancer"}, "Haunted Forest", {FOREST_BOSS_LEVEL}, 0, 0
    },
    {
        "Defeat the Goblin King", "Dain",
        "Defeat the Goblin King and search his records for whoever supplies his raids.",
        "His defeat opens the mountain road west to Sunscar Wastes.",
        {"Defeat the Goblin King"}, "Goblin Mountains", {MOUNTAIN_BOSS_LEVEL}, 0, 0
    },
    {
        "Dawnmere Under Occupation", "Watchkeeper Seren",
        "Free Dawnmere from the royal garrison. Seren shelters in the Alehouse.",
        "Liberation reopens its businesses and the northern Saltwind waterfront.",
        {"Break the western checkpoint", "Break the market squad", "Defeat Captain Rusk"},
        "Dawnmere", {0, 0, 0}, 100, 0, "500 XP"
    },
    {
        "Passage Through Saltwind", "Watchkeeper Seren",
        "Reach the river through Saltwind Marsh, north of Dawnmere.",
        "Board the skiff, paddle across and dock at Stillbury. Enter town to begin Act 4.",
        {"Defeat Sszareth at the final launch", "Reach Stillbury's river dock"},
        "Saltwind Marsh", {5, 0}, 0, 0
    },
    {
        "Break the Jarl's Blockade", "Caravan Scout Garran",
        "The Last Jarl holds the road to Stillbury under a royal charter.",
        "Defeat him to open Giant's Bridge. Completion is immediate; no camp return is needed.",
        {"Defeat the Last Jarl"}, "Giant's Peaks", {5}, 0, 0
    }

};

static int quest_state(const GameState *g, int quest) {
    if (quest == 25) {
        return g->jarl_quest_state;
    }
    if (quest == 23 || quest == 24) {
        return quest == 23 ? g->dawnmere_quest_state : g->saltwind_quest_state;
    }
    if (quest == 21 || quest == 22) {
        int state = quest == 21 ? g->alder_quest_state : g->dain_quest_state;
        return state == REGIONAL_CHAIN_BOSS_ACTIVE ? 1 : state == REGIONAL_CHAIN_COMPLETE ? 3 : 0;
    }
    if (quest == 20) {
        return g->crown_quest_state;
    }
    if (quest == 19) {
        return g->war_machine_quest_state;
    }
    if (quest == 18) {
        return g->stormbell_quest_state;
    }
    if (quest == 17) {
        return g->spice_skiff_quest_state;
    }
    if (quest == 16) {
        return g->heartseed_quest_state;
    }
    if (quest == 15) {
        return g->jail_gear_state;
    }
    if (quest == 14) {
        return g->hunt_quest_state;
    }
    if (quest == 13) {
        return g->watchfire_quest_state;
    }
    if (quest == 0) {
        return g->elowen_quest_state;
    }
    if (quest == 1) {
        return g->dain_quest_state > 3 ? 3 : g->dain_quest_state;
    }
    if (quest == 2) {
        return g->alder_quest_state > 3 ? 3 : g->alder_quest_state;
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
        return g->dragon_treasure_quest_state;
    }
    if (quest == 7) {
        return g->tribute_quest_state;
    }
    if (quest == 8) {
        return g->emberforge_quest_state;
    }
    if (quest == 9) {
        return g->frostfell_quest_state;
    }
    if (quest == 10) {
        return g->glassdeep_quest_state;
    }

    if (quest == 11) {
        return g->catacombs_quest_state;
    }
    return g->jail_quest_state == 3 ? 3 : g->jail_quest_state == 2 ? 1 : 0;
}

static int quest_progress(const GameState *g, int quest) {
    if (quest == 25) {
        return g->giants_crown_defeated;
    }
    if (quest == 23 || quest == 24) {
        return quest == 23 ? g->dawnmere_progress : !!g->mangrove_defeated | (g->saltwind_quest_state == 3 ? 2 : 0);
    }
    if (quest == 21 || quest == 22) {
        return (g->defeated_bosses & (1 << (quest == 21 ? LOCATION_FOREST : LOCATION_MOUNTAINS))) != 0;
    }
    if (quest == 20) {
        return !!(g->crown_quest_progress & CROWN_TAKEN);
    }
    if (quest == 19) {
        return g->war_machine_progress;
    }
    if (quest == 18) {
        return g->stormbell_progress;
    }
    if (quest == 17) {
        return g->spice_skiff_progress;
    }
    if (quest == 16) {
        return g->heartseed_progress;
    }
    if (quest == 15) {
        return g->jail_gear_state == 3;
    }
    if (quest == 14) {
        return g->hunt_quest_progress;
    }
    if (quest == 13) {
        return g->watchfire_quest_progress;
    }
    if (quest == 0) {
        return g->elowen_quest_state >= ELOWEN_RESCUED;
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
        return g->dragon_treasure_quest_state >= 2;
    }
    if (quest == 7) {
        return g->tribute_progress;
    }
    if (quest == 8) {
        return g->emberforge_progress;
    }
    if (quest == 9) {
        return g->frostfell_quest_progress;
    }
    if (quest == 10) {
        return g->glassdeep_quest_progress;
    }

    if (quest == 11) {
        return g->catacombs_quest_progress;
    }
    return g->jail_quest_state == 3 ? 3 : g->location == LOCATION_ESCAPE_TUNNEL ? 1 : 0;
}

static int quest_in_tab(int state, QuestJournalTab tab) {
    if (tab == QUEST_TAB_COMPLETED) {
        return state == 2 || state == 3;
    }
    return tab == QUEST_TAB_ACTIVE && state == 1;
}

void quest_journal_init(QuestJournalScreen *screen) {
    screen->tab = QUEST_TAB_ACTIVE;
    screen->selected = 0;
}

int quest_journal_count(const GameState *g, QuestJournalTab tab) {
    if (tab == QUEST_TAB_EVIDENCE) {
        return royal_evidence_count(g);
    }
    if (tab == QUEST_TAB_BOSSES) {
        return JOURNAL_BOSS_COUNT;
    }
    int count = 0;
    for (int quest = 0; quest < JOURNAL_QUEST_COUNT; quest++) {
        if (quest_in_tab(quest_state(g, quest), tab)) {
            count++;
        }
    }
    return count;
}

int quest_journal_get_boss(const GameState *g, int index, BossJournalEntry *entry) {
    static const char *names[JOURNAL_BOSS_COUNT] = {
        "Lich King", "Necromancer", "Goblin King", "Drowned Queen",
        "Fallen Sun Guardian", "Swamp Demon", "Red Dragon", "Frost Kraken", "Desert Pharaoh", "Cinder Lord", "Quarry Colossus", "Minotaur", "Grave Marshal", "Castellan", "Royal Arcanist", "Lord Veyr", "Elderthorn", "Sszareth, Keeper of the Roots", "Tempest Matriarch", "Goblin Mine Boss", "The Last Jarl"
    };
    static const char *areas[JOURNAL_BOSS_COUNT] = {
        "Dungeon", "Haunted Forest", "Goblin Mountains", "Sunken Coast",
        "Ruined Temple", "Blackwater Swamp", "Dragonspine", "Frostfell Wastes", "Sunscar Wastes", "Ashen Hollow", "Stonewake Quarry", "Rook's Labyrinth", "Royal Catacombs", "Castle: Iron Keep", "Castle: Crown Chapel", "Castle: Throne", "Elderwood Forest", "Saltwind Marsh", "Stormwatch Headland", "Ironfang Mines", "Giant's Peaks"
    };
    static const Location regions[JOURNAL_BOSS_COUNT] = {
        LOCATION_DUNGEON, LOCATION_FOREST, LOCATION_MOUNTAINS, LOCATION_COAST,
        LOCATION_TEMPLE, LOCATION_SWAMP, LOCATION_DRAGONSPINE, LOCATION_FROSTFELL,
        LOCATION_DESERT, LOCATION_ASHEN, LOCATION_GLASSDEEP, LOCATION_LABYRINTH, LOCATION_CATACOMBS, LOCATION_CASTLE_INTERIOR, LOCATION_CASTLE_INTERIOR, LOCATION_CASTLE_INTERIOR, LOCATION_ELDERWOOD, LOCATION_MANGROVES, LOCATION_STORMWATCH, LOCATION_IRONFANG, LOCATION_GIANTS_CROWN
    };
    if (index < 0 || index >= JOURNAL_BOSS_COUNT) {
        return 0;
    }
    entry->name = names[index];
    entry->area = areas[index];
    entry->trophy = index == 15 && g->crown_of_unbound ? "Crown of the Unbound" : NULL;
    entry->defeated = index == 20 ? g->giants_crown_defeated : index == 19 ? g->ironfang_defeated : index == 18 ? g->stormwatch_defeated : index == 17 ? g->mangrove_defeated : index == 16 ? g->elderwood_defeated : index == 13 ? !!(g->castle_minibosses & 1) : index == 14 ? !!(g->castle_minibosses & 2) :
        (g->defeated_bosses & (1 << regions[index])) != 0;
    return 1;
}

int quest_journal_get_entry(const GameState *g, QuestJournalTab tab, int index, QuestJournalEntry *entry) {
    if (tab == QUEST_TAB_BOSSES || tab == QUEST_TAB_EVIDENCE) {
        return 0;
    }
    int visible_index = 0;
    for (int quest = 0; quest < JOURNAL_QUEST_COUNT; quest++) {
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
        if (quest == 0) {
            entry->summary_line_2 = state == ELOWEN_HOME ? "Elowen is home at Oakhaven's Inn. Bram's reward has been collected." :
                state == ELOWEN_RESCUED ? definition->summary_line_2 : "Bram waits at Oakhaven's Inn. Rescue Elowen from the Lich King.";
        }
        if (state == 3 && (quest == 21 || quest == 22)) {
            entry->summary_line_1 = quest == 21 ? "The Necromancer's royal writ bears the King's seal, granting protection to his work." :
                "The Goblin King's payment records bear the King's seal, funding raids and the western-road blockade.";
            entry->summary_line_2 = quest == 21 ? "Read it in Evidence. The road east to Dawnmere is open; no Inn return is required." :
                "Read it in Evidence. The road west to Sunscar is open. Dain can introduce you to Zara; no Inn return is required.";
        }
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
            entry->objective_areas[objective] = definition->objective_areas[objective] ? definition->objective_areas[objective] : definition->area;
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
            screen->tab = (screen->tab + QUEST_TAB_COUNT - 1) % QUEST_TAB_COUNT;
            screen->selected = 0;
            break;
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_TAB:
            screen->tab = (screen->tab + 1) % QUEST_TAB_COUNT;
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
