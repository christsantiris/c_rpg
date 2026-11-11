#include "quarry_quest.h"
#include "approach.h"
#include "eastern_arc.h"
#include "aldric.h"
#include "elowen.h"
#include "prism_sovereign.h"
#include "resurrection.h"
#include "stonewake.h"
#include "storm_mage.h"
#include "garran.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

int quarry_road(const GameState *g) {
    return g->location == LOCATION_APPROACH && g->approach_route == APPROACH_QUARRY_ROAD;
}

static int occupied(const GameState *g, int x, int y) {
    if ((g->player.x == x && g->player.y == y) || aldric_at(g, x, y) || elowen_at(g, x, y) || storm_mage_at(g, x, y) || garran_at(g, x, y)) {
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 1;
        }
    }
    return 0;
}

static void paving(GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_PORTAL || tile == TILE_ITEM || map_is_travel_tile(tile)) {
        return;
    }
    g->map.tiles[y][x] = TILE_TOWN_PATH;
}

void quarry_prepare(GameState *g) {
    if (!quarry_road(g)) {
        return;
    }
    for (int y = SOULSTONE_RITUAL_Y; y <= QUARRY_ORIN_Y; y++) {
        paving(g, APPROACH_W / 2, y);
    }
    for (int x = QUARRY_WORKER_X; x <= APPROACH_W / 2; x++) {
        paving(g, x, QUARRY_ORIN_Y);
    }
    int clear = 1;
    for (int y = QUARRY_CAMP_Y + 1; y <= QUARRY_CAMP_Y + 4; y++) {
        for (int x = QUARRY_CAMP_X; x < QUARRY_CAMP_X + 13; x++) {
            TileType tile = g->map.tiles[y][x];
            clear &= !occupied(g, x, y) && tile != TILE_ITEM && tile != TILE_PORTAL && !map_is_travel_tile(tile);
        }
    }
    if (clear) {
        for (int y = QUARRY_CAMP_Y + 1; y <= QUARRY_CAMP_Y + 4; y++) {
            for (int x = QUARRY_CAMP_X; x < QUARRY_CAMP_X + 13; x++) {
                if (x <= QUARRY_CAMP_X + 3 || x >= QUARRY_CAMP_X + 9 || (x >= QUARRY_CAMP_X + 5 && x <= QUARRY_CAMP_X + 7 && y >= QUARRY_CAMP_Y + 3)) {
                    g->map.tiles[y][x] = TILE_CARAVAN_CAMP;
                }
            }
        }
    }
    const int posts[] = {QUARRY_ORIN_X, QUARRY_WORKER_X, QUARRY_WORKER_X + 2};
    for (int i = 0; i < 3; i++) {
        int x = posts[i];
        TileType tile = g->map.tiles[QUARRY_ORIN_Y][x];
        if (!occupied(g, x, QUARRY_ORIN_Y) && tile != TILE_ITEM && tile != TILE_PORTAL) {
            g->map.tiles[QUARRY_ORIN_Y][x] = i ? TILE_NPC_FROST_SURVIVOR : TILE_NPC_ORIN;
        }
        map_mark_explored(&g->map, x, QUARRY_ORIN_Y);
    }
    resurrection_place(g);
}

void quarry_update(GameState *g) {
    if (g->glassdeep_quest_state < 2 && g->glassdeep_quest_progress == 7 && (g->defeated_bosses & (1u << LOCATION_GLASSDEEP))) {
        g->glassdeep_quest_state = 2;
        push_message(g, "Completed: Reclaim the Quarry. Stillbury welcomes its restorer. Orin waits at the western workers' camp.");
    }
    if (g->glassdeep_quest_state >= 2) {
        if (g->quarry_equipment == QUARRY_NONE) {
            g->quarry_equipment = QUARRY_PENDING;
        }
        if (g->location == LOCATION_TOWN2 && g->stillbury_challenge == 2) {
            eastern_store(g);
            g->enemy_count = 0;
        }
        if (g->stillbury_prompt && strcmp(g->dialogue_speaker, "Royal Checkpoint") == 0) {
            g->dialogue_active = 0;
        }
        g->stillbury_prompt = 0;
        g->stillbury_challenge = 0;
    }
    quarry_prepare(g);
}

static void say(GameState *g, const char *speaker, const char *text, int prompt, int x, int y) {
    g->quarry_prompt = prompt;
    g->dialogue_active = 1;
    g->dialogue_x = x;
    g->dialogue_y = y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", speaker);
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s", text);
}

int quarry_dialogue(const GameState *g) {
    return g->dialogue_active && (strcmp(g->dialogue_speaker, "Surveyor Orin") == 0 || strcmp(g->dialogue_speaker, "Quarry Strongbox") == 0 || strcmp(g->dialogue_speaker, "Quarry Worker") == 0);
}

int quarry_talk(GameState *g) {
    if (!quarry_road(g)) {
        return 0;
    }
    if (abs(g->player.x - QUARRY_ORIN_X) > 1 || abs(g->player.y - QUARRY_ORIN_Y) > 1) {
        if (g->player.x >= QUARRY_WORKER_X - 1 && g->player.x <= QUARRY_WORKER_X + 3 && abs(g->player.y - QUARRY_ORIN_Y) <= 1) {
            say(g, "Quarry Worker", g->glassdeep_quest_state >= 2 ? "The Colossus is gone and the machinery works! We're returning to Stonewake. Stillbury will have stone for its walls and roads again. Thank you." :
                "Stonewake's guardians drove us out. We shelter here beside the shrine. Speak to Surveyor Orin; he can tell you how to restore the quarry.", QUARRY_MENU, QUARRY_WORKER_X, QUARRY_ORIN_Y);
            return 1;
        }
        return 0;
    }
    quarry_update(g);
    if (g->glassdeep_quest_state == 0) {
        say(g, "Surveyor Orin", "We abandoned Stonewake when its guardians turned on us. Stillbury needs stone for its walls, roads and river landing. Repair the West Winch on stage 2, Workshop Hoist on 3, and Main Gantry on 4, then defeat the Quarry Colossus on 5. Help us return to work for 120 gold and a choice of equipment?", QUARRY_MENU, QUARRY_ORIN_X, QUARRY_ORIN_Y);
    } else if (g->glassdeep_quest_state == 1) {
        char text[MAX_DIALOGUE_LEN];
        snprintf(text, sizeof(text), "West Winch: %s. Workshop Hoist: %s. Main Gantry: %s. Colossus: %s. Repairs alone cannot make the quarry safe. Restore all three and defeat the Colossus, then return here. The shrine can revive fallen companions before you set out.",
            g->glassdeep_quest_progress & 1 ? "repaired" : "needed", g->glassdeep_quest_progress & 2 ? "repaired" : "needed", g->glassdeep_quest_progress & 4 ? "repaired" : "needed", g->defeated_bosses & (1u << LOCATION_GLASSDEEP) ? "defeated" : "alive");
        say(g, "Surveyor Orin", text, QUARRY_MENU, QUARRY_ORIN_X, QUARRY_ORIN_Y);
    } else {
        if (g->glassdeep_quest_state == 2) {
            g->glassdeep_quest_state = 3;
            g->gold += GLASSDEEP_REWARD_GOLD;
            g->score += GLASSDEEP_REWARD_SCORE;
            if (g->quarry_equipment == QUARRY_LEGACY && !g->stonewake_warhammer_state) {
                g->stonewake_warhammer_state = 1;
            }
            push_message(g, "Orin pays 120 gold and 1,000 score. Stillbury's workers can return to Stonewake.");
        }
        if (g->quarry_equipment == QUARRY_LEGACY) {
            stonewake_claim_warhammer(g);
        }
        say(g, "Surveyor Orin", g->quarry_equipment == QUARRY_PENDING ?
            "Stonewake works again, and Stillbury welcomes you! Choose ONE reward:\n1 Magic Stone Warhammer: +8 attack, +30 maximum HP; two handed, all classes.\n2 Quarryman's Bow: +9 attack, range 6, +20 maximum HP; Rogue.\n3 Prismweave Robes: +3 defense, +35 maximum MP, 12% cheaper spells; Mage.\nEsc: Choose later." :
            g->stonewake_warhammer_state == 1 ? "Your previously earned Magic Stone Warhammer is safe with me. Make room and return. Stillbury thanks you for restoring its quarry." :
            "The workers are back at Stonewake. Stillbury has stone for its walls and roads again. Your reward is already collected, and you will always find a welcome here.",
            g->quarry_equipment == QUARRY_PENDING ? QUARRY_EQUIPMENT : QUARRY_MENU, QUARRY_ORIN_X, QUARRY_ORIN_Y);
    }
    return 1;
}

int quarry_reward_interact(GameState *g, int pickup) {
    if (g->location != LOCATION_GLASSDEEP || g->level != GLASSDEEP_DEPTH || g->quarry_potion != QUARRY_POTION_PENDING) {
        return 0;
    }
    const int steps[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < (pickup ? 1 : 5); i++) {
        int x = g->player.x + steps[i][0];
        int y = g->player.y + steps[i][1];
        if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
            continue;
        }
        TileType tile = g->map.tiles[y][x];
        for (int n = 0; n < g->floor_item_count; n++) {
            if (g->floor_items[n].active && g->floor_items[n].x == x && g->floor_items[n].y == y) {
                tile = g->floor_items[n].underlying_tile;
                break;
            }
        }
        if (tile == TILE_PRISM_RELIQUARY || tile == TILE_PRISM_PAVED_RELIQUARY) {
            say(g, "Quarry Strongbox", "Choose ONE potion:\n1 Potion of Strength\n2 Potion of Intelligence\nEsc: Leave it safely in the strongbox.", QUARRY_POTION, x, y);
            return 1;
        }
    }
    return 0;
}

int quarry_handle_key(GameState *g, int key, int repeat) {
    if (!quarry_dialogue(g) || g->quarry_prompt == QUARRY_MENU) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_N) {
        g->dialogue_active = 0;
        g->quarry_prompt = QUARRY_MENU;
        return 1;
    }
    int choice = key == SDL_SCANCODE_1 ? 1 : key == SDL_SCANCODE_2 ? 2 : key == SDL_SCANCODE_3 ? 3 : 0;
    int equipment = g->quarry_prompt == QUARRY_EQUIPMENT;
    if (!choice || (!equipment && choice > 2)) {
        return 1;
    }
    if ((equipment && (g->quarry_equipment != QUARRY_PENDING || g->glassdeep_quest_state != 3)) ||
        (!equipment && g->quarry_potion != QUARRY_POTION_PENDING)) {
        g->dialogue_active = 0;
        g->quarry_prompt = QUARRY_MENU;
        return 1;
    }
    if (g->inventory_count >= MAX_INVENTORY) {
        push_message(g, "Make room in your pack. Your unchosen reward stays safe; Esc closes this offer.");
        return 1;
    }
    Item item = equipment ? choice == 1 ? item_make_stone_warhammer() : choice == 2 ? item_make_quarry_bow() : item_make_prismweave_robes() :
        choice == 1 ? item_make_strength_potion() : item_make_intelligence_potion();
    g->inventory[g->inventory_count++] = item;
    if (equipment) {
        g->quarry_equipment = QUARRY_PENDING + choice;
        if (choice == 1) {
            g->stonewake_warhammer_state = 2;
        }
    } else {
        g->quarry_potion = QUARRY_POTION_PENDING + choice;
        g->score += item.value;
        prism_sovereign_restore_reward(g);
    }
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Received %s. This reward can be claimed only once.", item.name);
    push_message(g, message);
    g->dialogue_active = 0;
    g->quarry_prompt = QUARRY_MENU;
    return 1;
}

void quarry_record_defeat(GameState *g) {
    if (g->quarry_potion == QUARRY_POTION_NONE) {
        g->quarry_potion = QUARRY_POTION_PENDING;
    }
    quarry_update(g);
}

int quarry_valid(const GameState *g) {
    return g->quarry_equipment >= QUARRY_NONE && g->quarry_equipment <= QUARRY_LEGACY &&
        g->quarry_potion >= QUARRY_POTION_NONE && g->quarry_potion <= QUARRY_POTION_LEGACY &&
        g->quarry_prompt >= QUARRY_MENU && g->quarry_prompt <= QUARRY_POTION;
}

int quarry_migrate(GameState *g, int version) {
    if (version < 249) {
        g->quarry_equipment = g->glassdeep_quest_state >= 2 || g->stonewake_warhammer_state ? QUARRY_LEGACY : QUARRY_NONE;
        g->quarry_potion = g->defeated_bosses & (1u << LOCATION_GLASSDEEP) ? QUARRY_POTION_LEGACY : QUARRY_POTION_NONE;
        g->quarry_prompt = QUARRY_MENU;
        if (g->location == LOCATION_GUILD) {
            if (g->map.tiles[GUILD_ORIN_Y][GUILD_ORIN_X] == TILE_NPC_ORIN) {
                g->map.tiles[GUILD_ORIN_Y][GUILD_ORIN_X] = TILE_TAVERN_FLOOR;
            }
            if (strcmp(g->dialogue_speaker, "Surveyor Orin") == 0) {
                g->dialogue_active = 0;
            }
        }
        GameState *stage = calloc(1, sizeof(*stage));
        if (!stage) {
            return 0;
        }
        stage->player = g->player;
        stage->defeated_bosses = g->defeated_bosses;
        stage->location = LOCATION_GLASSDEEP;
        stage->glassdeep_quest_state = g->glassdeep_quest_state;
        stage->glassdeep_quest_progress = g->glassdeep_quest_progress;
        stage->glassdeep_quest_encounters = g->glassdeep_quest_encounters;
        for (int i = 1; i <= 3; i++) {
            LevelCache *cache = &g->glassdeep_cache[i];
            if (!cache->valid || (g->location == LOCATION_GLASSDEEP && g->level == i + 1)) {
                continue;
            }
            stage->level = i + 1;
            stage->map = cache->map;
            stage->enemy_count = cache->enemy_count;
            memcpy(stage->enemies, cache->enemies, sizeof(cache->enemies));
            game_refresh_quarry_stage(stage);
            cache->map = stage->map;
            cache->enemy_count = stage->enemy_count;
            memcpy(cache->enemies, stage->enemies, sizeof(cache->enemies));
        }
        g->glassdeep_quest_encounters = stage->glassdeep_quest_encounters;
        free(stage);
        if (g->location == LOCATION_GLASSDEEP) {
            game_refresh_quarry_stage(g);
            LevelCache *active = &g->glassdeep_cache[g->level - 1];
            if (active->valid) {
                active->map = g->map;
                active->enemy_count = g->enemy_count;
                memcpy(active->enemies, g->enemies, sizeof(active->enemies));
                active->level_cleared = g->level_cleared;
            }
        }
    }
    quarry_update(g);
    return quarry_valid(g);
}
