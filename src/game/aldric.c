#include "aldric.h"
#include "combat_feedback.h"
#include "elowen.h"
#include "town_road.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int lich_defeated(const GameState *g) {
    return (g->defeated_bosses & (1 << LOCATION_DUNGEON)) != 0;
}

int aldric_at(const GameState *g, int x, int y) {
    return g->aldric_party_state == ALDRIC_FOLLOWING && x == g->aldric_x && y == g->aldric_y;
}

int aldric_beside(const GameState *g, int x, int y) {
    int dx = abs(g->aldric_x - x);
    int dy = abs(g->aldric_y - y);
    return g->aldric_party_state == ALDRIC_FOLLOWING && dx <= 1 && dy <= 1 && dx + dy > 0;
}

static int near_player(const GameState *g, int x, int y) {
    return abs(x - g->player.x) <= 2 && abs(y - g->player.y) <= 2;
}

static int follow_space(const GameState *g, int x, int y) {
    if (!map_is_walkable(&g->map, x, y) || (x == g->player.x && y == g->player.y) || elowen_at(g, x, y)) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 0;
        }
    }
    return 1;
}

static int seat_free(const GameState *g, int x, int y) {
    return x >= 0 && x < MAP_W && y >= 0 && y < MAP_H && g->map.tiles[y][x] == TILE_TAVERN_FLOOR &&
        (x != g->player.x || y != g->player.y) && !elowen_at(g, x, y);
}

// After the Lich King falls, Aldric leaves the square and waits in the Inn.
void aldric_place(GameState *g) {
    if (!lich_defeated(g)) {
        return;
    }
    if (g->location == LOCATION_TAVERN && g->aldric_party_state == ALDRIC_DEAD) {
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (g->map.tiles[y][x] == TILE_NPC_ALDRIC) {
                    g->map.tiles[y][x] = TILE_TAVERN_FLOOR;
                }
            }
        }
        return;
    }
    if (g->location == LOCATION_TOWN && g->map.tiles[TOWN_ALDRIC_Y][TOWN_ALDRIC_X] == TILE_NPC_ALDRIC) {
        g->map.tiles[TOWN_ALDRIC_Y][TOWN_ALDRIC_X] = TILE_TOWN_PATH;
    } else if (g->location == LOCATION_TAVERN && g->aldric_party_state != ALDRIC_FOLLOWING &&
        seat_free(g, ALDRIC_INN_X, ALDRIC_INN_Y)) {
        g->map.tiles[ALDRIC_INN_Y][ALDRIC_INN_X] = TILE_NPC_ALDRIC;
    }
}

// Aldric walks behind Elowen while she is escorted, otherwise behind the player.
// He may stand two squares from the player while fighting.
void aldric_follow(GameState *g, int lead_old_x, int lead_old_y) {
    if (g->aldric_party_state != ALDRIC_FOLLOWING) {
        return;
    }
    if (town_road_index(g->location) >= 0) {
        g->aldric_hp = ALDRIC_MAX_HP;
    }
    int escort = g->elowen_quest_state == ELOWEN_RESCUED &&
        (g->elowen_x != g->player.x || g->elowen_y != g->player.y);
    int lead_x = escort ? g->elowen_x : g->player.x;
    int lead_y = escort ? g->elowen_y : g->player.y;
    if (abs(lead_old_x - lead_x) + abs(lead_old_y - lead_y) == 1 && follow_space(g, lead_old_x, lead_old_y)) {
        g->aldric_x = lead_old_x;
        g->aldric_y = lead_old_y;
    } else if (!near_player(g, g->aldric_x, g->aldric_y) || !follow_space(g, g->aldric_x, g->aldric_y)) {
        const int dx[] = {0, -1, 1, 0};
        const int dy[] = {1, 0, 0, -1};
        const int around[2][2] = {{lead_x, lead_y}, {g->player.x, g->player.y}};
        // A one-tile landing can temporarily share the player's square.
        g->aldric_x = g->player.x;
        g->aldric_y = g->player.y;
        for (int i = 0; i < 8; i++) {
            int x = around[i / 4][0] + dx[i % 4];
            int y = around[i / 4][1] + dy[i % 4];
            if (follow_space(g, x, y)) {
                g->aldric_x = x;
                g->aldric_y = y;
                break;
            }
        }
    }
    if (g->dialogue_active && strcmp(g->dialogue_speaker, ALDRIC_SPEAKER) == 0) {
        g->dialogue_x = g->aldric_x;
        g->dialogue_y = g->aldric_y;
    }
}

// He strikes a foe beside him, or closes on one threatening the player.
void aldric_turn(GameState *g) {
    if (g->aldric_party_state != ALDRIC_FOLLOWING || g->player.hp <= 0 || g->aldric_x < 0) {
        return;
    }
    Enemy *target = NULL;
    Enemy *threat = NULL;
    int threat_distance = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (!e->active) {
            continue;
        }
        int distance = abs(e->x - g->aldric_x) + abs(e->y - g->aldric_y);
        if (distance == 1 && (!target || e->hp < target->hp)) {
            target = e;
        }
        if (abs(e->x - g->player.x) <= 3 && abs(e->y - g->player.y) <= 3 && (!threat || distance < threat_distance)) {
            threat = e;
            threat_distance = distance;
        }
    }
    if (target) {
        action_aldric_strike(g, target);
        return;
    }
    if (!threat) {
        return;
    }
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    for (int i = 0; i < 4; i++) {
        int x = g->aldric_x + dx[i];
        int y = g->aldric_y + dy[i];
        if (abs(threat->x - x) + abs(threat->y - y) < threat_distance && near_player(g, x, y) && follow_space(g, x, y)) {
            g->aldric_x = x;
            g->aldric_y = y;
            return;
        }
    }
}

void aldric_take_hit(GameState *g, const Enemy *e) {
    if (g->aldric_party_state != ALDRIC_FOLLOWING) {
        return;
    }
    int damage = e->attack - ALDRIC_DEFENSE;
    if (damage < 1) {
        damage = 1;
    }
    g->aldric_hp -= damage;
    combat_feedback_add(g, FEEDBACK_ALLY_DAMAGE, FEEDBACK_NOW, g->aldric_x, g->aldric_y, damage);
    char msg[MAX_MESSAGE_LEN];
    if (g->aldric_hp > 0) {
        snprintf(msg, sizeof(msg), "%s hits Aldric: %d dmg", e->name, damage);
        push_message(g, msg);
        return;
    }
    g->aldric_party_state = ALDRIC_DEAD;
    g->aldric_recruited = 1;
    g->aldric_hp = 0;
    g->aldric_x = -1;
    g->aldric_y = -1;
    snprintf(msg, sizeof(msg), "Aldric has died. Seek Soulstone Shrine north of Oakhaven for resurrection.");
    push_message_kind(g, msg, MESSAGE_DAMAGE_TAKEN);
}

// Shared revival placement; the shrine validates the ritual and pays its cost.
int aldric_revive(GameState *g, int hp) {
    if (g->aldric_party_state != ALDRIC_DEAD || hp <= 0) {
        push_message(g, "No fallen party member needs resurrection.");
        return 0;
    }
    const int dx[] = {0, -1, 1, 0};
    const int dy[] = {1, 0, 0, -1};
    for (int side = 0; side < 4; side++) {
        int x = g->player.x + dx[side];
        int y = g->player.y + dy[side];
        if (follow_space(g, x, y) && !map_is_travel_tile(g->map.tiles[y][x])) {
            g->aldric_party_state = ALDRIC_FOLLOWING;
            g->aldric_recruited = 1;
            g->aldric_hp = hp > ALDRIC_MAX_HP ? ALDRIC_MAX_HP : hp;
            g->aldric_x = x;
            g->aldric_y = y;
            combat_feedback_add(g, FEEDBACK_HEAL, FEEDBACK_NOW, x, y, g->aldric_hp);
            push_message(g, "Aldric returns to life and rejoins your party.");
            return 1;
        }
    }
    push_message(g, "Make room beside you for Aldric before performing the ritual.");
    return 0;
}

void aldric_talk(GameState *g, int x, int y) {
    if (g->aldric_party_state == ALDRIC_DEAD) {
        push_message(g, "Aldric is dead. His spirit awaits resurrection at Soulstone Shrine.");
        return;
    }
    const char *text = "Clear the dungeon before venturing farther afield. Revisit cleared areas to train, and prepare supplies before a long journey.";
    if (g->aldric_party_state == ALDRIC_FOLLOWING) {
        text = g->location == LOCATION_TAVERN ?
            "Shall I wait here at the Inn? You can find me here whenever you need my sword again." :
            "I'm with you. If you would rather travel alone, tell me at Oakhaven's Inn and I will wait there.";
    } else if (lich_defeated(g)) {
        text = "The Lich King is gone and Elowen is free, but evil still stalks the roads beyond Oakhaven. Alder and Dain have work for a fighter, and my short sword is still sharp. Let me join you.";
    }
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, ALDRIC_SPEAKER);
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "%s", text);
    g->dialogue_x = x;
    g->dialogue_y = y;
}

int aldric_prompt_active(const GameState *g) {
    return g->aldric_party_state != ALDRIC_DEAD && g->dialogue_active && g->location == LOCATION_TAVERN && lich_defeated(g) &&
        strcmp(g->dialogue_speaker, ALDRIC_SPEAKER) == 0;
}

static void join(GameState *g) {
    int x = g->dialogue_x;
    int y = g->dialogue_y;
    if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H && g->map.tiles[y][x] == TILE_NPC_ALDRIC) {
        g->map.tiles[y][x] = TILE_TAVERN_FLOOR;
    }
    g->aldric_party_state = ALDRIC_FOLLOWING;
    g->aldric_recruited = 1;
    g->aldric_hp = ALDRIC_MAX_HP;
    g->aldric_x = x;
    g->aldric_y = y;
    aldric_follow(g, -1, -1);
    push_message(g, "Veteran Aldric joins your party.");
}

// He takes his seat, or stays beside the player when someone stands on it.
static void wait_at_inn(GameState *g) {
    int x = ALDRIC_INN_X;
    int y = ALDRIC_INN_Y;
    if (!seat_free(g, x, y)) {
        x = g->aldric_x;
        y = g->aldric_y;
    }
    if (!seat_free(g, x, y)) {
        push_message(g, "Make room beside you, and Aldric will wait here.");
        return;
    }
    g->map.tiles[y][x] = TILE_NPC_ALDRIC;
    g->aldric_party_state = ALDRIC_WAITING;
    g->aldric_x = -1;
    g->aldric_y = -1;
    push_message(g, "Veteran Aldric waits at Oakhaven's Inn.");
}

int aldric_handle_key(GameState *g, int key, int repeat) {
    if (!aldric_prompt_active(g)) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    int following = g->aldric_party_state == ALDRIC_FOLLOWING;
    if (key == SDL_SCANCODE_N || key == SDL_SCANCODE_ESCAPE) {
        g->dialogue_active = 0;
        push_message(g, following ? "Aldric stays in your party." : "Aldric will wait at the Inn if you change your mind.");
        return 1;
    }
    if (key != SDL_SCANCODE_Y) {
        return 1;
    }
    g->dialogue_active = 0;
    if (following) {
        wait_at_inn(g);
    } else {
        join(g);
    }
    return 1;
}
