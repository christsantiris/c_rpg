#include "garran.h"
#include "aldric.h"
#include "approach.h"
#include "combat_feedback.h"
#include "elowen.h"
#include "eastern_arc.h"
#include "mangrove.h"
#include "storm_mage.h"
#include "town_road.h"
#include "western_arc.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

void garran_init(GameState *g) {
    g->garran = (CaravanRogue){.state = GARRAN_WAITING, .x = -1, .y = -1, .hp = GARRAN_MAX_HP};
}

void garran_sync(GameState *g) {
    if (g->garran.state == GARRAN_FOLLOWING && (g->location == LOCATION_TOWN2 || g->location == LOCATION_INN ||
        (g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN2))) {
        g->garran.home = GARRAN_STILLBURY;
    }
}

int garran_position(const GameState *g, int *x, int *y) {
    const CaravanRogue *rogue = &g->garran;
    if (rogue->state == GARRAN_FOLLOWING && !mangrove_aboard(g)) {
        *x = rogue->x;
        *y = rogue->y;
        return *x >= 0 && *y >= 0;
    }
    if (rogue->state != GARRAN_WAITING) {
        return 0;
    }
    if (rogue->home == GARRAN_CAMP && approach_sunscar_road(g)) {
        *x = CAMP_GARRAN_X;
        *y = CAMP_GARRAN_Y;
        return g->map.tiles[*y][*x] == TILE_NPC_GARRAN;
    }
    if (rogue->home == GARRAN_STILLBURY && g->location == LOCATION_INN) {
        *x = GARRAN_INN_X;
        *y = GARRAN_INN_Y;
        return 1;
    }
    return 0;
}

int garran_at(const GameState *g, int x, int y) {
    int gx;
    int gy;
    return garran_position(g, &gx, &gy) && x == gx && y == gy;
}

int garran_beside(const GameState *g, int x, int y) {
    int dx = abs(g->garran.x - x);
    int dy = abs(g->garran.y - y);
    return g->garran.state == GARRAN_FOLLOWING && !mangrove_aboard(g) && dx <= 1 && dy <= 1 && dx + dy > 0;
}

static int open_space(const GameState *g, int x, int y) {
    if (!map_is_walkable(&g->map, x, y) || map_is_travel_tile(g->map.tiles[y][x]) ||
        (x == g->player.x && y == g->player.y) || aldric_at(g, x, y) || storm_mage_at(g, x, y) || elowen_at(g, x, y)) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 0;
        }
    }
    return 1;
}

void garran_follow(GameState *g, int old_x, int old_y) {
    CaravanRogue *rogue = &g->garran;
    garran_sync(g);
    if (rogue->state != GARRAN_FOLLOWING) {
        return;
    }
    if (town_road_index(g->location) >= 0 && !eastern_town_combat(g)) {
        rogue->hp = GARRAN_MAX_HP;
    }
    if (mangrove_aboard(g)) {
        rogue->x = g->player.x;
        rogue->y = g->player.y;
        return;
    }
    int lead_x = g->player.x;
    int lead_y = g->player.y;
    if (g->storm_mage.state == STORM_MAGE_FOLLOWING) {
        lead_x = g->storm_mage.x;
        lead_y = g->storm_mage.y;
    } else if (g->aldric_party_state == ALDRIC_FOLLOWING) {
        lead_x = g->aldric_x;
        lead_y = g->aldric_y;
    }
    if (abs(old_x - lead_x) + abs(old_y - lead_y) == 1 && open_space(g, old_x, old_y)) {
        rogue->x = old_x;
        rogue->y = old_y;
    } else if (abs(rogue->x - g->player.x) > 4 || abs(rogue->y - g->player.y) > 4 || !open_space(g, rogue->x, rogue->y)) {
        rogue->x = g->player.x;
        rogue->y = g->player.y;
        // Match the existing companions' fallback on a one-tile landing.
        int placed = 0;
        for (int radius = 1; radius <= 4 && !placed; radius++) {
            for (int y = g->player.y - radius; y <= g->player.y + radius && !placed; y++) {
                for (int x = g->player.x - radius; x <= g->player.x + radius; x++) {
                    if (abs(x - g->player.x) + abs(y - g->player.y) == radius && open_space(g, x, y)) {
                        rogue->x = x;
                        rogue->y = y;
                        placed = 1;
                        break;
                    }
                }
            }
        }
    }
    if (garran_dialogue(g)) {
        g->dialogue_x = rogue->x;
        g->dialogue_y = rogue->y;
    }
}

static int firing_lane(const GameState *g, int x, int y, const Enemy *target) {
    int distance = abs(target->x - x) + abs(target->y - y);
    if ((x != target->x && y != target->y) || distance < 2 || distance > GARRAN_RANGE) {
        return 0;
    }
    int dx = (target->x > x) - (target->x < x);
    int dy = (target->y > y) - (target->y < y);
    for (int step = 1; step <= distance; step++) {
        int px = x + dx * step;
        int py = y + dy * step;
        if (!map_is_walkable(&g->map, px, py) || (px == g->player.x && py == g->player.y) ||
            aldric_at(g, px, py) || storm_mage_at(g, px, py) || elowen_at(g, px, py)) {
            return 0;
        }
        for (int i = 0; i < g->enemy_count; i++) {
            const Enemy *other = &g->enemies[i];
            if (other != target && other->active && other->x == px && other->y == py) {
                return 0;
            }
        }
    }
    return 1;
}

void garran_turn(GameState *g) {
    CaravanRogue *rogue = &g->garran;
    if (rogue->state != GARRAN_FOLLOWING || rogue->x < 0 || g->player.hp <= 0 || mangrove_aboard(g)) {
        return;
    }
    Enemy *threat = NULL;
    Enemy *shot = NULL;
    int nearest = MAP_W + MAP_H;
    int shot_distance = nearest;
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (!storm_mage_hostile(g, e) || abs(e->x - g->player.x) > 5 || abs(e->y - g->player.y) > 5) {
            continue;
        }
        int distance = abs(e->x - rogue->x) + abs(e->y - rogue->y);
        if (garran_beside(g, e->x, e->y)) {
            action_garran_attack(g, e, 0);
            return;
        }
        if (distance < shot_distance && firing_lane(g, rogue->x, rogue->y, e)) {
            shot = e;
            shot_distance = distance;
        }
        if (distance < nearest) {
            threat = e;
            nearest = distance;
        }
    }
    if (shot) {
        action_garran_attack(g, shot, 1);
        return;
    }
    if (!threat) {
        return;
    }
    const int steps[4][2] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};
    int best = -nearest;
    int bx = rogue->x;
    int by = rogue->y;
    for (int i = 0; i < 4; i++) {
        int x = rogue->x + steps[i][0];
        int y = rogue->y + steps[i][1];
        if (!open_space(g, x, y) || abs(x - g->player.x) > 4 || abs(y - g->player.y) > 4) {
            continue;
        }
        int distance = abs(threat->x - x) + abs(threat->y - y);
        int score = -distance + (firing_lane(g, x, y, threat) ? 20 : 0) + (distance >= 2 ? 3 : 0);
        if (score > best) {
            best = score;
            bx = x;
            by = y;
        }
    }
    rogue->x = bx;
    rogue->y = by;
}

void garran_take_hit(GameState *g, const Enemy *e) {
    CaravanRogue *rogue = &g->garran;
    if (rogue->state != GARRAN_FOLLOWING) {
        return;
    }
    int damage = e->attack - GARRAN_DEFENSE;
    if (damage < 1) {
        damage = 1;
    }
    rogue->hp -= damage;
    combat_feedback_add(g, FEEDBACK_ALLY_DAMAGE, FEEDBACK_NOW, rogue->x, rogue->y, damage);
    char message[MAX_MESSAGE_LEN];
    if (rogue->hp <= 0) {
        rogue->hp = 0;
        rogue->state = GARRAN_DEAD;
        rogue->x = -1;
        rogue->y = -1;
        snprintf(message, sizeof(message), "Garran has died. Seek a Soulstone Shrine north of Oakhaven or west of Stillbury.");
    } else {
        snprintf(message, sizeof(message), "%s hits Garran: %d dmg", e->name, damage);
    }
    push_message_kind(g, message, MESSAGE_DAMAGE_TAKEN);
}

int garran_revive(GameState *g, int hp) {
    if (g->garran.state != GARRAN_DEAD || hp <= 0) {
        return 0;
    }
    const int steps[4][2] = {{0, 1}, {-1, 0}, {1, 0}, {0, -1}};
    for (int i = 0; i < 4; i++) {
        int x = g->player.x + steps[i][0];
        int y = g->player.y + steps[i][1];
        if (open_space(g, x, y)) {
            g->garran.state = GARRAN_FOLLOWING;
            g->garran.hp = hp > GARRAN_MAX_HP ? GARRAN_MAX_HP : hp;
            g->garran.x = x;
            g->garran.y = y;
            combat_feedback_add(g, FEEDBACK_HEAL, FEEDBACK_NOW, x, y, g->garran.hp);
            push_message(g, "Garran returns to life and rejoins your party.");
            return 1;
        }
    }
    push_message(g, "Make room beside you for Garran before performing the ritual.");
    return 0;
}

static void say(GameState *g, const char *text, int prompt) {
    g->garran.prompt = prompt;
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", GARRAN_SPEAKER);
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s", text);
}

int garran_talk(GameState *g, int x, int y) {
    int reach = g->garran.state == GARRAN_FOLLOWING ? 4 : 1;
    if (!garran_at(g, x, y) || abs(x - g->player.x) > reach || abs(y - g->player.y) > reach) {
        return 0;
    }
    garran_sync(g);
    g->dialogue_x = x;
    g->dialogue_y = y;
    say(g, g->garran.state == GARRAN_FOLLOWING ?
        "My bow is yours. We can discuss the Jarl's blockade, or you can ask me to wait at the caravan camp or Stillbury's Tavern." :
        g->garran.recruited ? "Ready to travel again? My bow and dagger are at your service." :
        "I've escorted caravans through these wastes for years. Now creatures have driven us back to camp, and the Jarl blocks the northern pass. If you're heading out there, you could use another bow. I am a Rogue; I can travel with you.", GARRAN_MENU);
    return 1;
}

int garran_dialogue(const GameState *g) {
    return g->dialogue_active && strcmp(g->dialogue_speaker, GARRAN_SPEAKER) == 0;
}

static int can_wait(const GameState *g) {
    return approach_sunscar_road(g) || g->location == LOCATION_INN;
}

static int waiting_space(const GameState *g, int x, int y) {
    if (!map_is_walkable(&g->map, x, y) || map_is_travel_tile(g->map.tiles[y][x]) ||
        g->map.tiles[y][x] == TILE_ITEM || g->map.tiles[y][x] == TILE_PORTAL || (x == g->player.x && y == g->player.y)) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 0;
        }
    }
    return 1;
}

const char *garran_hint(const GameState *g) {
    if (g->garran.prompt == GARRAN_JOIN) {
        return "Join your party? Y: Yes   N / Esc: No";
    }
    if (g->garran.prompt == GARRAN_WAIT) {
        return "Wait here? Y: Yes   N / Esc: No";
    }
    if (g->garran.prompt == GARRAN_QUEST) {
        return game_quest_offer_active(g) ? "Accept quest? Y: Yes   N / Esc: No" : "Enter / Esc: Close";
    }
    return g->garran.state == GARRAN_FOLLOWING ?
        "1: The Jarl's blockade\n2: Ask Garran to wait\nEnter / Esc: Close" :
        "1: The Jarl's blockade\n2: Join my party\nEnter / Esc: Close";
}

int garran_handle_key(GameState *g, int key, int repeat) {
    if (!garran_dialogue(g)) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (g->garran.prompt == GARRAN_MENU) {
        if (key == SDL_SCANCODE_1) {
            say(g, g->jarl_quest_state == 1 ?
                "Follow the road north to Giant's Peaks. Defeat the Last Jarl in the fifth stage to open passage to Stillbury. His defeat completes the task; no return to camp is required." : g->jarl_quest_state == 3 ?
                "The Jarl's blockade is broken. Follow the road to Giant's Bridge, then cross to Stillbury. Your work here is complete." :
                "The Last Jarl blocks the road to Stillbury under a royal charter. Follow the desert road north to Giant's Peaks. Will you defeat him on stage 5 and open the pass? His defeat completes this task immediately. Travelers from Stillbury face his blockade on the far side too.", GARRAN_QUEST);
        } else if (key == SDL_SCANCODE_2) {
            if (g->garran.state != GARRAN_FOLLOWING) {
                say(g, "I'll bring my shortbow, dagger and my own arrows. Shall I join your party? You decide whether to challenge the royal convoy or pass it by.", GARRAN_JOIN);
            } else if (can_wait(g)) {
                say(g, g->location == LOCATION_INN ? "Shall I wait here at Stillbury's Tavern?" : "Shall I wait here at the caravan camp?", GARRAN_WAIT);
            } else {
                say(g, "Ask me to wait when we reach the caravan camp or Stillbury's Tavern. For now, I'll stay with you.", GARRAN_MENU);
            }
        } else if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_N) {
            g->dialogue_active = 0;
        }
        return 1;
    }
    if (g->garran.prompt == GARRAN_QUEST) {
        if (game_handle_quest_offer_key(g, key, 0)) {
            return 1;
        }
        if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_N) {
            g->dialogue_active = 0;
        }
        return 1;
    }
    if (key != SDL_SCANCODE_Y && key != SDL_SCANCODE_N && key != SDL_SCANCODE_ESCAPE && key != SDL_SCANCODE_RETURN) {
        return 1;
    }
    g->dialogue_active = 0;
    if (key != SDL_SCANCODE_Y) {
        return 1;
    }
    if (g->garran.prompt == GARRAN_JOIN && g->garran.state == GARRAN_WAITING) {
        g->garran.state = GARRAN_FOLLOWING;
        g->garran.recruited = 1;
        western_prepare(g);
        garran_follow(g, -1, -1);
        push_message(g, "Rogue Garran joins your party with his shortbow and dagger.");
    } else if (g->garran.prompt == GARRAN_WAIT && g->garran.state == GARRAN_FOLLOWING && can_wait(g)) {
        int x = g->location == LOCATION_INN ? GARRAN_INN_X : CAMP_GARRAN_X;
        int y = g->location == LOCATION_INN ? GARRAN_INN_Y : CAMP_GARRAN_Y;
        if (!waiting_space(g, x, y)) {
            push_message(g, "Make room at Garran's waiting place first.");
            return 1;
        }
        g->garran.home = g->location == LOCATION_INN ? GARRAN_STILLBURY : GARRAN_CAMP;
        g->garran.state = GARRAN_WAITING;
        g->garran.x = -1;
        g->garran.y = -1;
        western_prepare(g);
        // Reserve his waiting place before letting another follower step aside.
        if (aldric_at(g, x, y)) {
            aldric_follow(g, -1, -1);
        }
        if (storm_mage_at(g, x, y)) {
            storm_mage_follow(g, -1, -1);
        }
        if (elowen_at(g, x, y)) {
            elowen_follow(g, -1, -1);
        }
        push_message(g, g->garran.home == GARRAN_STILLBURY ? "Garran waits at Stillbury's Tavern." : "Garran waits at the caravan camp.");
    }
    return 1;
}

int garran_valid(const GameState *g) {
    const CaravanRogue *rogue = &g->garran;
    return rogue->state >= GARRAN_WAITING && rogue->state <= GARRAN_DEAD &&
        rogue->recruited >= 0 && rogue->recruited <= 1 && rogue->home >= GARRAN_CAMP && rogue->home <= GARRAN_STILLBURY &&
        rogue->prompt >= GARRAN_MENU && rogue->prompt <= GARRAN_QUEST && rogue->hp >= 0 && rogue->hp <= GARRAN_MAX_HP &&
        (rogue->state != GARRAN_DEAD || (rogue->recruited && rogue->hp == 0)) &&
        (rogue->state != GARRAN_FOLLOWING || (rogue->recruited && rogue->hp > 0 && rogue->x >= 0 && rogue->x < MAP_W && rogue->y >= 0 && rogue->y < MAP_H));
}
