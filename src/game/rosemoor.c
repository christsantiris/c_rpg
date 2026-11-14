#include "rosemoor.h"
#include "containers.h"
#include "aldric.h"
#include "garran.h"
#include "ridgeshire.h"
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

static void say(GameState *g, const char *who, const char *text, int x, int y, int prompt) {
    g->rosemoor.prompt = prompt;
    g->dialogue_active = 1;
    g->dialogue_x = x;
    g->dialogue_y = y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", who);
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s", text);
}

void rosemoor_sync(GameState *g) {
    if ((g->defeated_bosses & (1u << LOCATION_FROSTFELL)) && g->rosemoor.frost_quest < 2) {
        g->rosemoor.frost_quest = 2;
        push_message(g, "Completed: The Broken Ward. The Kraken releases the stolen wardstone; ancient protections recover. Visit Mira in Rosemoor.");
    }
    if ((g->defeated_bosses & (1u << LOCATION_ASHEN)) && g->rosemoor.forge_quest < 2) {
        g->rosemoor.forge_quest = 2;
        push_message(g, "Completed: The Arcanist's Forge. Binding production ends; Malakar's ledger is kept as evidence. Visit Hadrin in Rosemoor.");
    }
    if ((g->hunt_quest_progress & 1) && g->rosemoor.hunt_quest < 2) {
        g->rosemoor.hunt_quest = 2;
        push_message(g, "Completed: Hunt Rimefang. The entire pack is defeated. Claim Selene's reward in Rosemoor's Guild.");
    }
}

static void prepare_reliquary(GameState *g) {
    if (g->location != LOCATION_FROSTFELL || g->level != FROSTFELL_DEPTH || g->rosemoor.weapon != ROSE_WEAPON_PENDING || !g->map.room_count) {
        return;
    }
    for (int i = 0; i < g->loot_container_count; i++) {
        if (g->loot_containers[i].kind == TILE_KRAKEN_RELIQUARY) {
            return;
        }
    }
    const Room *room = &g->map.rooms[g->map.room_count - 1];
    int best = MAP_W + MAP_H;
    int rx = -1;
    int ry = -1;
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            TileType tile = g->map.tiles[y][x];
            if (tile == TILE_KRAKEN_RELIQUARY || tile == TILE_KRAKEN_LAKE_RELIQUARY) {
                return;
            }
            int occupied = 0;
            for (int i = 0; i < g->enemy_count; i++) {
                occupied |= g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y;
            }
            int distance = abs(x - room->x - room->w / 2) + abs(y - room->y - room->h / 2);
            if ((tile == TILE_FROST_FLOOR || tile == TILE_FROST_LAKE) && !occupied && distance < best) {
                rx = x;
                ry = y;
                best = distance;
            }
        }
    }
    if (rx >= 0) {
        g->map.tiles[ry][rx] = g->map.tiles[ry][rx] == TILE_FROST_LAKE ? TILE_KRAKEN_LAKE_RELIQUARY : TILE_KRAKEN_RELIQUARY;
        push_message(g, "A Krakenbone weapon choice waits in the reliquary. Interact to open; Pick Up to choose.");
    }
}

void rosemoor_prepare(GameState *g) {
    if (g->location == LOCATION_TOWN3 && g->map.tiles[TOWN_APOTHECARY_DOOR_Y][TOWN_APOTHECARY_DOOR_X] == TILE_ALCHEMIST_DOOR) {
        g->map.tiles[TOWN_APOTHECARY_DOOR_Y][TOWN_APOTHECARY_DOOR_X] = TILE_LOCAL_DOOR;
    }
    if (g->location == LOCATION_GUILD && g->map.tiles[ROSE_HADRIN_Y][ROSE_HADRIN_X] == TILE_TAVERN_FLOOR && !near(g, ROSE_HADRIN_X, ROSE_HADRIN_Y) &&
        !aldric_at(g, ROSE_HADRIN_X, ROSE_HADRIN_Y) && !storm_mage_at(g, ROSE_HADRIN_X, ROSE_HADRIN_Y) && !garran_at(g, ROSE_HADRIN_X, ROSE_HADRIN_Y)) {
        g->map.tiles[ROSE_HADRIN_Y][ROSE_HADRIN_X] = TILE_NPC_STEWARD;
    }
    if (g->location == LOCATION_INN && !g->frostfell_quest_state && g->map.tiles[BRENNA_INN_Y][BRENNA_INN_X] == TILE_NPC_BRENNA) {
        g->map.tiles[BRENNA_INN_Y][BRENNA_INN_X] = TILE_TAVERN_FLOOR;
    }
    prepare_reliquary(g);
}

static void mira_quest(GameState *g) {
    int state = g->rosemoor.frost_quest;
    if (state == 2) {
        g->rosemoor.frost_quest = 3;
        g->gold += 350;
        push_message(g, "Mira pays 350 gold for The Broken Ward.");
    }
    const char *text = state == 0 ?
        "Malakar ordered Frostfell's wardstones removed. Creatures escaped, yet he forbade repairs. Defeat the Frost Kraken on stage 5: it swallowed the last wardstone. Recover it and its orders for 350 gold. Will you investigate?" : state == 1 ?
        "Leave Rosemoor by the north gate. Defeat the Frost Kraken on Frostfell 5. Its stolen wardstone and the wardkeepers' orders are recovered automatically. Return here for 350 gold." :
        "Malakar deliberately dismantled the wards and suppressed the warnings. The restored stone protects the northern settlements again. Your 350 gold is paid. Hadrin can compare the orders with the Emberforge ledger; Edric and Oswin investigate the King's crown in Ridgeshire.";
    if (state >= 2 && g->ridgeshire.crown_record && (g->defeated_bosses & (1u << LOCATION_DRAGONSPINE)) && (g->war_machine_progress & WAR_ORDERS)) {
        text = "Ridgeshire's evidence shows Malakar controls the King. These orders show he also engineered the influx of creatures, then suppressed warnings. The King became his shield. Your 350 gold is paid; take this evidence when you confront the Arcanist.";
    }
    say(g, ROSE_FROST_SPEAKER, text, RESIDENT_X, RESIDENT_Y, ROSE_INFO);
}

static void forge_quest(GameState *g) {
    int state = g->rosemoor.forge_quest;
    if (state == 2) {
        g->rosemoor.forge_quest = 3;
        g->rosemoor.bracers = 1;
        g->gold += 400;
        push_message(g, "Hadrin pays 400 gold for The Arcanist's Forge.");
    }
    if (g->rosemoor.bracers == 1) {
        if (g->inventory_count < MAX_INVENTORY) {
            g->inventory[g->inventory_count++] = item_make_emberforge_bracers();
            g->rosemoor.bracers = 2;
            push_message(g, "Emberforge Bracers received: +2 defense, +30 maximum HP and MP. All classes.");
        } else {
            push_message(g, "Hadrin keeps your Emberforge Bracers safe. Make room and speak to him again.");
        }
    }
    say(g, ROSE_FORGE_SPEAKER, state == 0 ?
        "I kept the Emberforge until Malakar seized it and dismissed the royal inspectors. The Cinder Lord now makes binding devices there. Defeat him on Ashen Hollow 5 and recover the ledger. Return for 400 gold and Emberforge Bracers. Will you help?" : state == 1 ?
        "Take Rosemoor's west gate to Ashen Hollow. Defeat the Cinder Lord on stage 5; his production ledger is recovered automatically. His death ends the binding production. Return here for 400 gold and Emberforge Bracers." :
        "The ledger links Malakar's bindings to his creature allies and the King's ceremonial crown. He manufactured the crisis while hiding behind royal commands. Production has ended. Your gold is paid; I hold your bracers if your pack was full.", ROSE_HADRIN_X, ROSE_HADRIN_Y, ROSE_INFO);
}

static void hunt_quest(GameState *g) {
    int state = g->rosemoor.hunt_quest;
    if (state == 2) {
        g->rosemoor.hunt_quest = 3;
        g->rosemoor.potion = 1;
        g->gold += 200;
        push_message(g, "Selene pays 200 gold. Choose one Strength or Intelligence potion.");
    }
    if (g->rosemoor.potion == 1) {
        say(g, ROSE_HUNT_SPEAKER, "The expedition road is safer. Your 200 gold is paid. Choose ONE potion: Strength permanently adds 1 base attack; Intelligence permanently adds 1 maximum MP. A full pack leaves your choice safe with me.", GUILD_SELENE_X, GUILD_SELENE_Y, ROSE_POTION);
        return;
    }
    say(g, ROSE_HUNT_SPEAKER, state == 0 ?
        "Rimefang's pack attacks our northern expeditions. Hunt the leader and its entire pack on Frostfell 3. No Kraken fight is required. Return for 200 gold and a choice of Strength or Intelligence potion. Mira studies the broken wards; Hadrin knows the Emberforge. Will you hunt?" : state == 1 ?
        "Rimefang and every member of its pack must fall on Frostfell 3. Luring them away does not count as defeating them. Return here to claim 200 gold and one potion. The regional boss is optional." :
        "The pack is gone and your reward is collected. Malakar's binding collar explains its unusual coordination. Mira and Hadrin can help uncover why creatures descended on the kingdom.", GUILD_SELENE_X, GUILD_SELENE_Y, ROSE_INFO);
}

int rosemoor_talk(GameState *g) {
    rosemoor_sync(g);
    if (g->location == LOCATION_ROSEMOOR_APOTHECARY && near(g, RESIDENT_X, RESIDENT_Y)) {
        say(g, "Apothecary Mira", "Welcome to Rosemoor's Apothecary. I trade potions and study the corruption troubling Frostfell. We can trade, or discuss the broken wards. Selene and Hadrin offer other work at the Guild.", RESIDENT_X, RESIDENT_Y, ROSE_MIRA_MENU);
        return 1;
    }
    if (g->location == LOCATION_GUILD && near(g, ROSE_HADRIN_X, ROSE_HADRIN_Y) && g->map.tiles[ROSE_HADRIN_Y][ROSE_HADRIN_X] == TILE_NPC_STEWARD) {
        forge_quest(g);
        return 1;
    }
    if (g->location == LOCATION_GUILD && near(g, GUILD_SELENE_X, GUILD_SELENE_Y)) {
        hunt_quest(g);
        return 1;
    }
    if (g->location == LOCATION_SPICE_SHOP && near(g, RESIDENT_X, RESIDENT_Y)) {
        const char *text = g->royal_reported_actions ?
            "Word reached us that you opposed the Crown. You have my welcome, though royal officers may disagree. Our caravans were attacked while Malakar's marked shipments passed untouched. Mira studies Frostfell; Hadrin and Selene are at the Guild." :
            "Our caravans were attacked while Malakar's marked shipments passed untouched. They called the shortages inevitable. Mira studies Frostfell's broken wards; Hadrin and Selene at the Guild can guide you to Ashen Hollow and a local hunt.";
        if (g->rosemoor.frost_quest >= 2 && g->rosemoor.forge_quest >= 2) {
            text = "Trade can recover now that the wards are restored and the binding forge is stopped. Your evidence shows Malakar manufactured this crisis. Rosemoor welcomes you; royal officers elsewhere still keep their own records.";
        }
        say(g, "Spice Merchant Suri", text, RESIDENT_X, RESIDENT_Y, ROSE_INFO);
        return 1;
    }
    return 0;
}

int rosemoor_dialogue(const GameState *g) {
    if (!g->dialogue_active) {
        return 0;
    }
    const char *names[] = {"Apothecary Mira", ROSE_FROST_SPEAKER, ROSE_FORGE_SPEAKER, ROSE_HUNT_SPEAKER, "Spice Merchant Suri", "Krakenbone Weapons"};
    for (int i = 0; i < 6; i++) {
        if (!strcmp(g->dialogue_speaker, names[i])) {
            return 1;
        }
    }
    return 0;
}

const char *rosemoor_hint(const GameState *g) {
    int legacy_hunt = !strcmp(g->dialogue_speaker, ROSE_HUNT_SPEAKER) && g->hunt_quest_state;
    if (legacy_hunt && game_quest_offer_active(g)) {
        return game_quest_offer_at_limit(g) ? "Quest limit reached; finish active quests.\n3: Previous Great Hunts contract\nN / Esc: Close" :
            "Accept quest? Y: Yes   N: No\n3: Previous Great Hunts contract";
    }
    if (g->rosemoor.prompt == ROSE_MIRA_MENU) {
        return g->rosemoor.frost_quest == 2 ? "1: Buy / sell\n2: Claim Frostfell reward\nEnter / Esc: Close" : g->rosemoor.frost_quest == 1 ? "1: Buy / sell\n2: Discuss The Broken Ward\nEnter / Esc: Close" : "1: Buy / sell\n2: Ask about Frostfell\nEnter / Esc: Close";
    }
    if (g->rosemoor.prompt == ROSE_POTION) {
        return legacy_hunt ? "1: Strength potion\n2: Intelligence potion\n3: Previous Great Hunts contract\nEsc: Choose later" : "1: Strength potion\n2: Intelligence potion\nEsc: Choose later";
    }
    if (g->rosemoor.prompt == ROSE_WEAPON) {
        return "1: Krakenbone Blade (Warrior)\n2: Krakenbone Warbow (Rogue)\n3: Rimestaff (Mage)\nEsc: Choose later";
    }
    if (legacy_hunt) {
        return "3: Previous Great Hunts contract\nEnter / Esc: Close";
    }
    return "Enter / Esc: Close";
}

int rosemoor_open_shop(const GameState *g, int key) {
    return rosemoor_dialogue(g) && g->rosemoor.prompt == ROSE_MIRA_MENU && key == SDL_SCANCODE_1;
}

void rosemoor_weapon_offer(GameState *g) {
    say(g, "Krakenbone Weapons", "Choose ONE weapon: Krakenbone Blade (+11 attack, +25 max HP, 20% critical, one hand); Krakenbone Warbow (+14 attack, range 7, piercing); Rimestaff (+10 attack, +10 spell power, +45 max MP, 10% cheaper spells). A full pack preserves your choice.", g->player.x, g->player.y, ROSE_WEAPON);
}

int rosemoor_handle_key(GameState *g, int key, int repeat) {
    if (!rosemoor_dialogue(g)) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (!strcmp(g->dialogue_speaker, ROSE_HUNT_SPEAKER) && key == SDL_SCANCODE_3 && g->hunt_quest_state) {
        game_talk_to_selene(g);
        return 1;
    }
    if (game_quest_offer_active(g)) {
        return game_handle_quest_offer_key(g, key, 0);
    }
    if (g->rosemoor.prompt == ROSE_MIRA_MENU && key == SDL_SCANCODE_2) {
        mira_quest(g);
    } else if (g->rosemoor.prompt == ROSE_POTION && (key == SDL_SCANCODE_1 || key == SDL_SCANCODE_2) && g->rosemoor.potion == 1) {
        if (g->inventory_count >= MAX_INVENTORY) {
            push_message(g, "Make room first; Selene keeps your potion choice safe.");
            return 1;
        }
        g->inventory[g->inventory_count++] = key == SDL_SCANCODE_1 ? item_make_strength_potion() : item_make_intelligence_potion();
        g->rosemoor.potion = 2;
        g->dialogue_active = 0;
        push_message(g, "Selene's potion reward collected.");
    } else if (g->rosemoor.prompt == ROSE_WEAPON && key >= SDL_SCANCODE_1 && key <= SDL_SCANCODE_3 && g->rosemoor.weapon == ROSE_WEAPON_PENDING) {
        if (g->inventory_count >= MAX_INVENTORY) {
            push_message(g, "Make room first; the Krakenbone weapon choice remains safe.");
            return 1;
        }
        int choice = key - SDL_SCANCODE_1;
        Item weapon = item_make_kraken_weapon(choice);
        g->inventory[g->inventory_count++] = weapon;
        g->score += weapon.value;
        g->rosemoor.weapon = ROSE_BLADE + choice;
        g->kraken_bow_unclaimed = 0;
        for (int i = 0; i < g->loot_container_count; i++) {
            if (g->loot_containers[i].kind == TILE_KRAKEN_RELIQUARY) {
                g->loot_containers[i].rewards = 0;
            }
        }
        g->dialogue_active = 0;
        push_message(g, "Krakenbone weapon claimed. The other choices are released.");
    } else if (key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_N) {
        g->dialogue_active = 0;
    }
    return 1;
}

void rosemoor_kraken_reward(GameState *g) {
    if (!g->rosemoor.weapon && !g->rosemoor.legacy_kraken) {
        g->rosemoor.weapon = ROSE_WEAPON_PENDING;
        g->kraken_bow_unclaimed = 1;
    }
    rosemoor_sync(g);
    prepare_reliquary(g);
}

void rosemoor_migrate(GameState *g, int version) {
    if (version < 252) {
        g->rosemoor = (RosemoorArc){0};
        g->rosemoor.legacy_kraken = !!(g->defeated_bosses & (1u << LOCATION_FROSTFELL)) || g->kraken_bow_unclaimed;
        if (g->hunt_quest_state == 3) {
            g->rosemoor.hunt_quest = 3;
            g->rosemoor.potion = 2;
        }
    }
    rosemoor_sync(g);
    rosemoor_prepare(g);
}

int rosemoor_valid(const GameState *g) {
    const RosemoorArc *r = &g->rosemoor;
    return r->frost_quest >= 0 && r->frost_quest <= 3 && r->forge_quest >= 0 && r->forge_quest <= 3 &&
        r->hunt_quest >= 0 && r->hunt_quest <= 3 && r->bracers >= 0 && r->bracers <= 2 && r->potion >= 0 && r->potion <= 2 &&
        r->weapon >= ROSE_WEAPON_NONE && r->weapon <= ROSE_STAFF && r->prompt >= ROSE_INFO && r->prompt <= ROSE_WEAPON &&
        r->legacy_kraken >= 0 && r->legacy_kraken <= 1 &&
        (!r->bracers || r->forge_quest == 3) && (!r->potion || r->hunt_quest == 3) &&
        (!r->weapon || (!r->legacy_kraken && (g->defeated_bosses & (1u << LOCATION_FROSTFELL)))) &&
        (r->weapon != ROSE_WEAPON_PENDING || g->kraken_bow_unclaimed) &&
        (r->weapon < ROSE_BLADE || !g->kraken_bow_unclaimed) &&
        (r->frost_quest < 2 || (g->defeated_bosses & (1u << LOCATION_FROSTFELL))) &&
        (r->forge_quest < 2 || (g->defeated_bosses & (1u << LOCATION_ASHEN))) &&
        (r->hunt_quest < 2 || (g->hunt_quest_progress & 1));
}
