#include "western_arc.h"
#include "approach.h"
#include "crown_quest.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int near(const GameState *g, int x, int y) {
    return abs(g->player.x - x) + abs(g->player.y - y) <= 1;
}

static void dialogue(GameState *g, const char *name, int x, int y, const char *text) {
    g->dialogue_active = 1;
    g->dialogue_x = x;
    g->dialogue_y = y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", name);
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s", text);
}

void western_init(GameState *g) {
    g->tribute_quest_state = 0;
    g->tribute_progress = 0;
    g->jarl_quest_state = 0;
    g->zara_at_camp = 0;
}

int western_convoy_enemy(const GameState *g, const Enemy *e) {
    return g->location == LOCATION_DESERT && g->level == 2 && e->boss_aux_y == TRIBUTE_SQUAD &&
        (e->type == ENEMY_ROYAL_GUARD || e->type == ENEMY_ROAD_ARCHER);
}

static int convoy_remaining(const GameState *g) {
    int alive = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        alive += western_convoy_enemy(g, &g->enemies[i]) && g->enemies[i].active;
    }
    return alive;
}

// Change only the camp footprint; dropped items keep their own tiles and contents.
static void ground(GameState *g, int x, int y, TileType tile) {
    if (g->map.tiles[y][x] == TILE_TOWN_PATH || g->map.tiles[y][x] == TILE_PORTAL || map_is_travel_tile(g->map.tiles[y][x])) {
        return;
    }
    if (g->map.tiles[y][x] == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == x && item->y == y) {
                if (item->underlying_tile != TILE_TOWN_PATH && !map_is_travel_tile(item->underlying_tile)) {
                    item->underlying_tile = tile;
                }
            }
        }
    } else {
        g->map.tiles[y][x] = tile;
    }
}

static void place_npc(GameState *g, int x, int y, TileType tile) {
    if ((g->player.x != x || g->player.y != y) && g->map.tiles[y][x] != TILE_ITEM && g->map.tiles[y][x] != TILE_PORTAL) {
        g->map.tiles[y][x] = tile;
    }
}

static int free_post(const GameState *g, int x, int y) {
    if (g->map.tiles[y][x] != TILE_DESERT_FLOOR || (g->player.x == x && g->player.y == y) ||
        (x == TRIBUTE_X && y == TRIBUTE_Y)) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 0;
        }
    }
    return 1;
}

static void prepare_convoy(GameState *g) {
    if (g->location != LOCATION_DESERT || g->level != 2 || (g->tribute_progress & 1)) {
        return;
    }
    int existing = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        existing += western_convoy_enemy(g, &g->enemies[i]);
    }
    // Named escorts retain wounds and deaths when ordinary desert patrols respawn.
    static const int posts[TRIBUTE_GUARDS][2] = {{-2, 0}, {-1, -1}, {0, -2}, {1, -1}, {2, 0}, {1, 2}, {-1, 2}};
    for (int n = existing; n < TRIBUTE_GUARDS && g->enemy_count < LEVEL_CACHE_ENEMY_CAPACITY; n++) {
        int bx = -1;
        int by = -1;
        int best = MAP_W + MAP_H;
        for (int y = TRIBUTE_Y - 4; y <= TRIBUTE_Y + 4; y++) {
            for (int x = TRIBUTE_X - 4; x <= TRIBUTE_X + 4; x++) {
                int distance = abs(x - TRIBUTE_X - posts[n][0]) + abs(y - TRIBUTE_Y - posts[n][1]);
                if (distance < best && free_post(g, x, y)) {
                    bx = x;
                    by = y;
                    best = distance;
                }
            }
        }
        if (bx < 0) {
            break;
        }
        Enemy *e = &g->enemies[g->enemy_count++];
        int archer = n == 2 || n == 5 || n == 6;
        game_spawn_enemy(g, e, archer ? ENEMY_ROAD_ARCHER : ENEMY_ROYAL_GUARD, bx, by);
        e->max_hp = e->hp = archer ? 55 : 90;
        e->attack = archer ? 21 : 24;
        e->defense = archer ? 3 : 7;
        e->experience = 90;
        e->boss_aux_x = !!(g->royal_reported_actions & 2);
        e->boss_aux_y = TRIBUTE_SQUAD;
        snprintf(e->name, sizeof(e->name), "%s", archer ? "Tribute Archer" : "Tribute Guard");
        g->level_cleared = 0;
    }
}

void western_prepare(GameState *g) {
    if (g->location == LOCATION_TAVERN) {
        if (g->zara_at_camp) {
            if (g->map.tiles[ZARA_OAKHAVEN_Y][ZARA_OAKHAVEN_X] == TILE_NPC_GUILD_SEEKER) {
                ground(g, ZARA_OAKHAVEN_X, ZARA_OAKHAVEN_Y, TILE_TAVERN_FLOOR);
            }
        } else if (g->defeated_bosses & (1u << LOCATION_MOUNTAINS)) {
            place_npc(g, ZARA_OAKHAVEN_X, ZARA_OAKHAVEN_Y, TILE_NPC_GUILD_SEEKER);
        }
    }
    if (approach_sunscar_road(g)) {
        g->zara_at_camp = 1;
        for (int y = 25; y <= 34; y++) {
            for (int x = 40; x <= 52; x++) {
                ground(g, x, y, TILE_TOWN_FLOOR);
            }
        }
        int occupied = g->player.x >= 40 && g->player.x <= 52 && g->player.y >= 29 && g->player.y <= 34;
        occupied |= g->aldric_x >= 40 && g->aldric_x <= 52 && g->aldric_y >= 29 && g->aldric_y <= 34;
        occupied |= g->elowen_x >= 40 && g->elowen_x <= 52 && g->elowen_y >= 29 && g->elowen_y <= 34;
        for (int i = 0; i < g->floor_item_count; i++) {
            const FloorItem *item = &g->floor_items[i];
            occupied |= item->active && item->x >= 40 && item->x <= 52 && item->y >= 29 && item->y <= 34;
        }
        if (!occupied) {
            for (int y = 30; y <= 33; y++) {
                for (int x = 40; x <= 52; x++) {
                    if (x <= 43 || x >= 49 || (x >= 45 && x <= 47 && y >= 31)) {
                        ground(g, x, y, TILE_CARAVAN_CAMP);
                    }
                }
            }
        }
        place_npc(g, CAMP_ZARA_X, CAMP_ZARA_Y, TILE_NPC_GUILD_SEEKER);
        place_npc(g, CAMP_GARRAN_X, CAMP_GARRAN_Y, TILE_NPC_GARRAN);
        place_npc(g, CAMP_LIORA_X, CAMP_LIORA_Y, TILE_NPC_LIORA);
    }
    if (approach_bridge_road(g)) {
        place_npc(g, EDDA_STILLBURY_X, EDDA_STILLBURY_Y, TILE_NPC_EDDA);
    }
    prepare_convoy(g);
}

void western_update(GameState *g) {
    if (approach_sunscar_road(g) || approach_bridge_road(g)) {
        western_prepare(g);
    }
    if (g->defeated_bosses & (1u << LOCATION_DESERT)) {
        g->tribute_progress |= 2;
    }
    if (g->tribute_quest_state == 1 && g->tribute_progress == 3) {
        g->tribute_quest_state = 2;
        push_message(g, "Completed: The King's Tribute. Zara waits at the caravan camp outside Sunscar with your reward.");
    }
    if (g->jarl_quest_state == 1 && g->giants_crown_defeated) {
        g->jarl_quest_state = 3;
        push_message(g, "Completed: Break the Jarl's Blockade. The road to Giant's Bridge is open; no return to Garran is needed.");
    }
    int struck = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (western_convoy_enemy(g, e)) {
            struck |= !e->active || e->hp < e->max_hp || e->frozen_turns > 0 || e->poison_turns > 0;
        }
    }
    if (struck) {
        if (!(g->royal_reported_actions & 2)) {
            push_message(g, "Your attack on the royal tribute convoy is reported to Stillbury. The Crown will remember.");
        }
        g->royal_reported_actions |= 2;
        for (int i = 0; i < g->enemy_count; i++) {
            if (western_convoy_enemy(g, &g->enemies[i])) {
                g->enemies[i].boss_aux_x = 1;
            }
        }
    }
}

int western_talk(GameState *g) {
    int camp = approach_sunscar_road(g);
    int inn = g->location == LOCATION_TAVERN && !g->zara_at_camp;
    int zx = camp ? CAMP_ZARA_X : ZARA_OAKHAVEN_X;
    int zy = camp ? CAMP_ZARA_Y : ZARA_OAKHAVEN_Y;
    western_update(g);
    if ((camp || inn) && g->map.tiles[zy][zx] == TILE_NPC_GUILD_SEEKER && near(g, zx, zy)) {
        if (g->tribute_quest_state == 2) {
            g->tribute_quest_state = 3;
            g->gold += TRIBUTE_GOLD;
            player_gain_xp(g, TRIBUTE_XP);
            dialogue(g, "Zara", zx, zy, "Our goods are safe! The Pharaoh's agreement bears the King's seal: his tribute is taken from his own people and routed through Stillbury customs. We will return the cargo to its owners. Take 200 gold and 500 experience.");
            push_message(g, "The King's Tribute reward: 200 gold and 500 XP.");
        } else if (g->tribute_quest_state == 3) {
            dialogue(g, "Zara", zx, zy, "The confiscated cargo is back with its owners. The Pharaoh's agreement points to Stillbury customs. Follow the desert road north to Giant's Peaks and the bridge when you are ready.");
        } else if (g->tribute_quest_state == 1) {
            dialogue(g, "Zara", zx, zy, "Reclaim the confiscated cargo from the royal convoy in Sunscar 2, and defeat the Pharaoh in Sunscar 5 to recover his agreement. Attacking the convoy will anger the Crown. Return to me at the caravan camp outside Sunscar for 200 gold and 500 experience.");
        } else {
            dialogue(g, "Zara", zx, zy, "Soldiers seized our goods as tribute for the Pharaoh. Recover the convoy cargo in Sunscar 2 and his agreement in Sunscar 5. Attacking the convoy will be reported to Stillbury; you may pass it by. Meet me at the camp outside Sunscar afterward for 200 gold and 500 experience.");
        }
        return 1;
    }
    if (camp && near(g, CAMP_GARRAN_X, CAMP_GARRAN_Y)) {
        dialogue(g, "Caravan Scout Garran", CAMP_GARRAN_X, CAMP_GARRAN_Y, g->jarl_quest_state == 1 ?
            "Follow the road north to Giant's Peaks. Defeat the Last Jarl in the fifth stage to open onward passage to Stillbury. No return to me is required." : g->jarl_quest_state == 3 ?
            "The Jarl's blockade is broken. Follow the road from Giant's Peaks to Giant's Bridge, then cross to Stillbury. Your work here is complete." :
            "The Last Jarl blocks the road to Stillbury under a royal charter. Follow the desert road north to Giant's Peaks. Will you defeat him on stage 5 and open the pass? His defeat completes this task immediately. The bridge itself is unguarded; travelers from Stillbury face his blockade on the far side too.");
        return 1;
    }
    if (camp && near(g, CAMP_LIORA_X, CAMP_LIORA_Y)) {
        crown_quest_sync(g);
        dialogue(g, "Historian Liora", CAMP_LIORA_X, CAMP_LIORA_Y, g->crown_quest_state >= 2 ?
            "You recovered the Jarl's crown. Deliver it to Antiquarian Edda beside the Stillbury end of Giant's Bridge, outside the town checkpoint. She has your reward; you need not return here." :
            "The Last Jarl wears the old giant kingdom's crown. Will you recover it when he falls and deliver it to Antiquarian Edda? She waits beside the Stillbury end of Giant's Bridge, outside the checkpoint. Edda offers 220 gold and a Stoneguard Shield. No seal or vault key is needed.");
        return 1;
    }
    return 0;
}

int western_convoy_prompt(const GameState *g) {
    return g->dialogue_active && g->location == LOCATION_DESERT && g->level == 2 &&
        !strcmp(g->dialogue_speaker, "Royal Tribute Convoy") && !(g->tribute_progress & 1) && convoy_remaining(g) > 0;
}

const char *western_hint(const GameState *g) {
    if (g->location == LOCATION_DESERT && g->level == 2 && !(g->tribute_progress & 1) && near(g, TRIBUTE_X, TRIBUTE_Y)) {
        return "A: INSPECT ROYAL TRIBUTE CARGO";
    }
    return NULL;
}

int western_interact(GameState *g) {
    if (!western_hint(g)) {
        return 0;
    }
    western_update(g);
    if (convoy_remaining(g)) {
        dialogue(g, "Royal Tribute Convoy", TRIBUTE_X, TRIBUTE_Y,
            "These confiscated goods are the King's tribute to the Pharaoh. The escort will oppose any attempt to reclaim them. Intervening will be reported to Stillbury. Reclaim the cargo, or move along?");
    } else {
        g->tribute_progress |= 1;
        push_message(g, "Confiscated tribute cargo recovered. It is kept with quest discoveries and uses no pack space.");
        western_update(g);
    }
    return 1;
}

int western_handle_key(GameState *g, int key, int repeat) {
    if (!western_convoy_prompt(g)) {
        int camp_dialogue = approach_sunscar_road(g) || approach_bridge_road(g) ||
            (g->location == LOCATION_TAVERN && !strcmp(g->dialogue_speaker, "Zara"));
        if (camp_dialogue && g->dialogue_active && !game_quest_offer_active(g) &&
            (key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_RETURN)) {
            if (!repeat) {
                g->dialogue_active = 0;
            }
            return 1;
        }
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (key == SDL_SCANCODE_N || key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_RETURN) {
        g->dialogue_active = 0;
    } else if (key == SDL_SCANCODE_Y) {
        g->dialogue_active = 0;
        g->royal_reported_actions |= 2;
        for (int i = 0; i < g->enemy_count; i++) {
            if (western_convoy_enemy(g, &g->enemies[i])) {
                g->enemies[i].boss_aux_x = 1;
            }
        }
        push_message(g, "You challenge the royal convoy. Stillbury will hear of your intervention.");
    }
    return 1;
}

void western_migrate(GameState *g) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_RETIRED_DESERT_LAMP) {
                g->map.tiles[y][x] = TILE_DESERT_FLOOR;
            }
        }
    }
    for (int n = 0; n < DESERT_DEPTH; n++) {
        if (!g->desert_cache[n].valid) {
            continue;
        }
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (g->desert_cache[n].map.tiles[y][x] == TILE_RETIRED_DESERT_LAMP) {
                    g->desert_cache[n].map.tiles[y][x] = TILE_DESERT_FLOOR;
                }
            }
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].underlying_tile == TILE_RETIRED_DESERT_LAMP) {
            g->floor_items[i].underlying_tile = TILE_DESERT_FLOOR;
        }
    }
    if (g->portal_origin_tile == TILE_RETIRED_DESERT_LAMP) {
        g->portal_origin_tile = TILE_DESERT_FLOOR;
    }
    if ((g->location == LOCATION_INN && !strcmp(g->dialogue_speaker, "Zara")) ||
        (g->location == LOCATION_GUILD && !strcmp(g->dialogue_speaker, "Antiquarian Edda"))) {
        g->dialogue_active = 0;
    }
    if (g->location == LOCATION_INN && g->map.tiles[ZARA_INN_Y][ZARA_INN_X] == TILE_NPC_GUILD_SEEKER) {
        ground(g, ZARA_INN_X, ZARA_INN_Y, TILE_TAVERN_FLOOR);
    }
    if (g->location == LOCATION_GUILD && g->map.tiles[EDDA_Y][EDDA_X] == TILE_NPC_EDDA) {
        ground(g, EDDA_X, EDDA_Y, TILE_TAVERN_FLOOR);
    }
    western_prepare(g);
    western_update(g);
}

int western_valid(const GameState *g) {
    return g->tribute_quest_state >= 0 && g->tribute_quest_state <= 3 &&
        g->tribute_progress >= 0 && g->tribute_progress <= 3 &&
        (g->tribute_quest_state < 2 || g->tribute_progress == 3) &&
        (g->jarl_quest_state == 0 || g->jarl_quest_state == 1 || g->jarl_quest_state == 3) &&
        (g->jarl_quest_state != 3 || g->giants_crown_defeated) &&
        g->zara_at_camp >= 0 && g->zara_at_camp <= 1;
}
