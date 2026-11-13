#include "ridgeshire.h"
#include "aldric.h"
#include "garran.h"
#include "red_dragon.h"
#include "royal_evidence.h"
#include "storm_mage.h"
#include "town_life.h"
#include "war_machine.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

static int near(const GameState *g, int x, int y) {
    return abs(g->player.x - x) <= 1 && abs(g->player.y - y) <= 1;
}

static void say(GameState *g, const char *speaker, const char *text, int x, int y, int prompt) {
    g->ridgeshire.prompt = prompt;
    g->dialogue_active = 1;
    g->dialogue_x = x;
    g->dialogue_y = y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", speaker);
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s", text);
}

void ridgeshire_sync(GameState *g) {
    if ((g->defeated_bosses & (1u << LOCATION_DRAGONSPINE)) && g->ridgeshire.dragon_quest < 2) {
        g->ridgeshire.dragon_quest = 2;
        push_message(g, "Completed: The Dragon's Binding. Its runic shackle is kept with your evidence. Visit Edric in Ridgeshire.");
    }
    if (g->war_machine_quest_state >= 2 && !g->ridgeshire.ring_reward) {
        g->ridgeshire.ring_reward = 1;
    }
}

void ridgeshire_prepare(GameState *g) {
    if (g->location == LOCATION_CASTLE) {
        TileType tile = g->map.tiles[RENN_Y][RENN_X];
        if ((tile == TILE_TOWN_FLOOR || tile == TILE_TOWN_PATH) && !near(g, RENN_X, RENN_Y) &&
            !aldric_at(g, RENN_X, RENN_Y) && !storm_mage_at(g, RENN_X, RENN_Y) && !garran_at(g, RENN_X, RENN_Y)) {
            g->map.tiles[RENN_Y][RENN_X] = TILE_NPC_STORM_GUIDE;
        }
    }
    if (g->location == LOCATION_TOWN_HALL && !g->emberforge_quest_state && g->map.tiles[HALL_STEWARD_Y][HALL_STEWARD_X] == TILE_NPC_STEWARD) {
        g->map.tiles[HALL_STEWARD_Y][HALL_STEWARD_X] = TILE_TAVERN_FLOOR;
    }
}

void ridgeshire_claim_ring(GameState *g) {
    if (g->ridgeshire.ring_reward != 1 || g->war_machine_quest_state != 3) {
        return;
    }
    if (g->inventory_count >= MAX_INVENTORY) {
        push_message(g, "Veyra keeps your Royal Ward Ring safe. Make room and speak to her again.");
        return;
    }
    g->inventory[g->inventory_count++] = item_make_royal_ward_ring();
    g->ridgeshire.ring_reward = 2;
    push_message(g, "Royal Ward Ring received: +2 defense, +25 maximum HP and MP. All classes can equip it.");
}

static void dragon_offer(GameState *g) {
    ridgeshire_sync(g);
    int state = g->ridgeshire.dragon_quest;
    if (state == 2) {
        g->ridgeshire.dragon_quest = 3;
        g->gold += 350;
        push_message(g, "Edric rewards The Dragon's Binding: 350 gold.");
    }
    const char *text = state == 0 ?
        "Dragonspine's attacks bear signs of binding magic. Defeat the Red Dragon on stage 5 and bring its runic shackle for examination. I will pay 350 gold. The dragon's hoard also holds a choice of armor. Will you investigate?" : state == 1 ?
        "Take Ridgeshire's north gate to Dragonspine. Defeat the Red Dragon on stage 5; its shackle travels with your evidence, outside your pack. Return here for 350 gold." :
        "The shackle suppresses another creature's will. Its runes match Malakar's binding apparatus. I have paid your 350 gold; show Archivist Oswin at the Town Hall. We must discover whether this magic was used on the King.";
    say(g, RIDGE_DRAGON_SPEAKER, text, RESIDENT_X, RESIDENT_Y, RIDGE_MENU);
}

static void hoard_offer(GameState *g) {
    if (g->dragon_treasure_quest_state == 2) {
        g->dragon_treasure_quest_state = 3;
        g->gold += 300;
        push_message(g, "Ilya pays 300 gold for the golden goblet. The Dragon's Hoard is complete.");
        say(g, RIDGE_HOARD_SPEAKER, "The goblet is genuine! Here are your 300 gold. Thank you for bringing it to me here in Ridgeshire.", RIDGE_ILYA_X, RIDGE_ILYA_Y, RIDGE_MENU);
    } else {
        say(g, RIDGE_HOARD_SPEAKER, g->dragon_treasure_quest_state == 3 ?
            "Your recovered goblet has pride of place in my collection. Your reward is already paid." : g->dragon_treasure_quest_state == 1 ?
            "Find the golden goblet in the Red Dragon's lair on Dragonspine 5. Return here to collect 300 gold. Edric studies its binding magic; I am only interested in the goblet." :
            "I collect ancient treasures. Find the golden goblet in the Red Dragon's lair on Dragonspine 5 and bring it here. I pay 300 gold. It is separate from Edric's investigation and the dragon's armor. Will you look for it?", RIDGE_ILYA_X, RIDGE_ILYA_Y, RIDGE_MENU);
    }
}

int ridgeshire_talk(GameState *g) {
    ridgeshire_sync(g);
    if (g->location == LOCATION_CASTLE && near(g, RENN_X, RENN_Y) && g->map.tiles[RENN_Y][RENN_X] == TILE_NPC_STORM_GUIDE) {
        say(g, "Courier Renn", "Every report goes through Malakar's office now. I was refused an audience with the King. Our iron shipments vanish, yet goblins carry newly forged weapons. Marshal Veyra investigates from Ridgeshire. Follow Crown Road East; its workshop and runewright can also prepare you for the castle.", RENN_X, RENN_Y, RIDGE_MENU);
        return 1;
    }
    if (g->location == LOCATION_TOWN_HALL && near(g, HALL_VEYRA_X, HALL_VEYRA_Y)) {
        return war_machine_talk(g);
    }
    if (g->location == LOCATION_TOWN_HALL && near(g, HALL_OSWIN_X, HALL_OSWIN_Y)) {
        int clues = !!(g->war_machine_progress & WAR_ORDERS) + !!(g->defeated_bosses & (1u << LOCATION_DRAGONSPINE));
        if (clues) {
            g->ridgeshire.crown_record = 1;
        }
        say(g, "Archivist Oswin", clues == 2 ?
            "Malakar commissioned these same runes for the King's ceremonial crown. Ironfang's requisitions describe the apparatus; the dragon's shackle proves what it does. Together with this inventory, the evidence shows the King is being controlled. Veyra must hear this." : clues ?
            "This royal inventory records Malakar commissioning binding runes for the King's ceremonial crown. Compare it with Ironfang's requisitions and the Red Dragon's shackle. We need both to establish what he has done." :
            "The King once received reports in person. Now Malakar's office intercepts them. Bring Ironfang's requisitions or the dragon's runic shackle, and I can compare them with the royal inventories. Veyra and Edric can guide you. I also retain records of your earlier contracts.", HALL_OSWIN_X, HALL_OSWIN_Y, RIDGE_MENU);
        return 1;
    }
    if (g->location == LOCATION_MONASTERY && near(g, RESIDENT_X, RESIDENT_Y)) {
        say(g, "Runewright Edric", g->ridgeshire.dragon_quest >= 2 ?
            "Welcome back. The dragon's shackle reveals how binding magic can suppress a will. Let us discuss your findings, or browse spells and health and mana potions. Oswin keeps the royal inventories in the Town Hall." :
            "Welcome to my lodge. Dragonspine's violence bears signs of unnatural magic. I can explain that investigation, or sell spells and health and mana potions. Veyra is investigating the iron shipments; Oswin keeps the royal records in the Town Hall.", RESIDENT_X, RESIDENT_Y, RIDGE_EDRIC_MENU);
        return 1;
    }
    if (g->location == LOCATION_RIDGESHIRE_TAVERN && near(g, RIDGE_ILYA_X, RIDGE_ILYA_Y)) {
        hoard_offer(g);
        return 1;
    }
    if (g->location == LOCATION_RIDGESHIRE_TAVERN && near(g, RESIDENT_X, RESIDENT_Y)) {
        say(g, "Ridgeshire Innkeeper", g->royal_reported_actions ?
            "Your name reached us ahead of you. Veyra judges what you do for this town. I offer a bed and supplies, not an arrest. Ilya collects treasures by the west tables. Rest for 100 gold restores you and your living companions; fallen friends still need resurrection." :
            "Welcome to Ridgeshire. Veyra handles the mine trouble; Edric studies the dragon. Ilya collects treasures by the west tables. Rest for 100 gold restores you and your living companions. Fallen friends still need resurrection.", RESIDENT_X, RESIDENT_Y, RIDGE_INN_MENU);
        return 1;
    }
    return 0;
}

int ridgeshire_dialogue(const GameState *g) {
    if (!g->dialogue_active) {
        return 0;
    }
    const char *names[] = {"Courier Renn", "Archivist Oswin", "Runewright Edric", RIDGE_DRAGON_SPEAKER, RIDGE_HOARD_SPEAKER, "Ridgeshire Innkeeper", "Dragon's Hoard Armor"};
    for (int i = 0; i < 7; i++) {
        if (!strcmp(g->dialogue_speaker, names[i])) {
            return 1;
        }
    }
    return 0;
}

const char *ridgeshire_hint(const GameState *g) {
    if (g->ridgeshire.prompt == RIDGE_EDRIC_MENU) {
        return "1: The Dragon's Binding\n2: Browse spells and potions\nEnter / Esc: Close";
    }
    if (g->ridgeshire.prompt == RIDGE_INN_MENU) {
        return "1: Rest for 100 gold\nEnter / Esc: Close";
    }
    if (g->ridgeshire.prompt == RIDGE_REST) {
        return "Rest for 100 gold? Y: Yes   N / Esc: No";
    }
    if (g->ridgeshire.prompt == RIDGE_ARMOR) {
        return "1: Dragonplate (Warrior)\n2: Dragonscale Leathers (Rogue)\n3: Emberweave Robes (Mage)\nEsc: Choose later";
    }
    return !strcmp(g->dialogue_speaker, "Archivist Oswin") && g->catacombs_quest_state ? "3: Previous Catacombs contract\nEnter / Esc: Close" : "Enter / Esc: Close";
}

void ridgeshire_armor_offer(GameState *g) {
    say(g, "Dragon's Hoard Armor", "Choose ONE dragonforged armor reward. Dragonplate: +12 defense, +60 max HP. Dragonscale Leathers: +8 defense, +25 max HP, 15% evasion. Emberweave Robes: +5 defense, +70 max MP, 20% cheaper spells. A full pack leaves your choice safe here.", g->player.x, g->player.y, RIDGE_ARMOR);
}

int ridgeshire_handle_key(GameState *g, int key, int repeat) {
    if (!ridgeshire_dialogue(g)) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (game_quest_offer_active(g)) {
        return game_handle_quest_offer_key(g, key, 0);
    }
    if (g->ridgeshire.prompt == RIDGE_EDRIC_MENU && key == SDL_SCANCODE_1) {
        dragon_offer(g);
    } else if (g->ridgeshire.prompt == RIDGE_INN_MENU && key == SDL_SCANCODE_1) {
        say(g, "Ridgeshire Innkeeper", "Rest restores HP and MP for you and your living companions. Fallen companions remain dead. Pay 100 gold?", RESIDENT_X, RESIDENT_Y, RIDGE_REST);
    } else if (g->ridgeshire.prompt == RIDGE_REST && key == SDL_SCANCODE_Y) {
        if (g->gold < RIDGE_REST_GOLD) {
            push_message(g, "You need 100 gold to rest here.");
            return 1;
        }
        g->gold -= RIDGE_REST_GOLD;
        g->player.hp = g->player.max_hp;
        g->player.mp = g->player.max_mp;
        g->player.poison_turns = 0;
        if (g->aldric_party_state == ALDRIC_FOLLOWING) {
            g->aldric_hp = ALDRIC_MAX_HP;
        }
        if (g->storm_mage.state == STORM_MAGE_FOLLOWING) {
            g->storm_mage.hp = storm_mage_max_hp(g);
            g->storm_mage.mp = storm_mage_max_mp(g);
        }
        if (g->garran.state == GARRAN_FOLLOWING) {
            g->garran.hp = GARRAN_MAX_HP;
        }
        g->dialogue_active = 0;
        push_message(g, "You and your living companions rest. Paid 100 gold.");
    } else if (g->ridgeshire.prompt == RIDGE_ARMOR && key >= SDL_SCANCODE_1 && key <= SDL_SCANCODE_3) {
        if (g->ridgeshire.dragon_armor != RIDGE_ARMOR_PENDING || g->red_dragon_reward_state != 1) {
            g->dialogue_active = 0;
            return 1;
        }
        if (g->inventory_count >= MAX_INVENTORY) {
            push_message(g, "Your armor choice remains safe in the dragon's hoard. Make room first.");
            return 1;
        }
        int choice = key - SDL_SCANCODE_1;
        Item armor = item_make_dragonforged_armor(choice);
        g->inventory[g->inventory_count++] = armor;
        g->score += armor.value;
        g->ridgeshire.dragon_armor = RIDGE_DRAGONPLATE + choice;
        g->red_dragon_reward_state = 2;
        for (int i = 0; i < g->loot_container_count; i++) {
            if (g->loot_containers[i].kind == TILE_DRAGON_RELIQUARY_FLOOR) {
                g->loot_containers[i].rewards = 0;
            }
        }
        red_dragon_restore_reward(g);
        g->dialogue_active = 0;
        push_message(g, "Dragonforged armor claimed. The other choices are released; this reward is awarded once.");
    } else if (!strcmp(g->dialogue_speaker, "Archivist Oswin") && key == SDL_SCANCODE_3 && g->catacombs_quest_state) {
        game_talk_to_oswin(g);
    } else if (key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_N) {
        g->dialogue_active = 0;
    }
    return 1;
}

void ridgeshire_migrate(GameState *g, int version) {
    if (version < 251) {
        g->ridgeshire = (RidgeshireArc){0};
        g->ridgeshire.legacy_dragon = !!(g->defeated_bosses & (1u << LOCATION_DRAGONSPINE)) || g->red_dragon_reward_state != 0;
        g->ridgeshire.ring_reward = g->war_machine_quest_state == 3 ? 2 : 0;
        if (g->location == LOCATION_TOWN4) {
            town_life_migrate(g);
        }
        if (g->dialogue_active && !strcmp(g->dialogue_speaker, "Ilya") && g->location == LOCATION_TAVERN) {
            g->dialogue_active = 0;
        }
    }
    ridgeshire_sync(g);
    ridgeshire_prepare(g);
    if (g->aldric_party_state == ALDRIC_FOLLOWING && !map_is_walkable(&g->map, g->aldric_x, g->aldric_y)) {
        aldric_follow(g, -1, -1);
    }
    if (g->storm_mage.state == STORM_MAGE_FOLLOWING && !map_is_walkable(&g->map, g->storm_mage.x, g->storm_mage.y)) {
        storm_mage_follow(g, -1, -1);
    }
    if (g->garran.state == GARRAN_FOLLOWING && !map_is_walkable(&g->map, g->garran.x, g->garran.y)) {
        garran_follow(g, -1, -1);
    }
}

int ridgeshire_valid(const GameState *g) {
    const RidgeshireArc *arc = &g->ridgeshire;
    return arc->dragon_quest >= 0 && arc->dragon_quest <= 3 && arc->ring_reward >= 0 && arc->ring_reward <= 2 &&
        arc->dragon_armor >= RIDGE_ARMOR_NONE && arc->dragon_armor <= RIDGE_EMBERWEAVE &&
        arc->crown_record >= 0 && arc->crown_record <= 1 && arc->prompt >= RIDGE_MENU && arc->prompt <= RIDGE_ARMOR &&
        arc->legacy_dragon >= 0 && arc->legacy_dragon <= 1 &&
        (arc->dragon_quest < 2 || (g->defeated_bosses & (1u << LOCATION_DRAGONSPINE)));
}
