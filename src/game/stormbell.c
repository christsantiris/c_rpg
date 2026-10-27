#include "stormbell.h"
#include "stormwatch.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *names[] = {"Stormbell Raider I", "Stormbell Raider II", "Stormbell Stormcaller"};
static const int arrivals[3][2] = {{39, 24}, {37, 25}, {40, 23}};

static int near(const GameState *g, int x, int y) {
    return abs(g->player.x - x) + abs(g->player.y - y) <= 1;
}

static int index_of(const Enemy *e) {
    for (int i = 0; i < 3; i++) {
        if (strcmp(e->name, names[i]) == 0) {
            return i;
        }
    }
    return -1;
}

void stormbell_accept(GameState *g) {
    if (g->stormbell_quest_state == 1 && g->stormbell_progress == 7) {
        g->stormbell_quest_state = 2;
        push_message(g, "The stormbell rings safely again. Return to Watchkeeper Seren beside Dawnmere's east road.");
    }
}

int stormbell_talk(GameState *g) {
    if (g->location != LOCATION_DAWNMERE || !near(g, SEREN_X, SEREN_Y)) {
        return 0;
    }
    g->dialogue_active = 1;
    g->dialogue_x = SEREN_X;
    g->dialogue_y = SEREN_Y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Watchkeeper Seren");
    if (!g->stormbell_quest_state) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Restore Stormwatch's warning bell. Find its clapper in stage 3's northeast tower; fit and ring it by stage 4's shutter. Defend it from harpies, then return here for 180 gold.");
    } else if (g->stormbell_quest_state == 1) {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Clapper: %s. Bell: %s. Courtyard: %s. Find the clapper in Stormwatch 3's northeast tower; restore the bell beside stage 4's shutter. Return here after the defense.",
            g->stormbell_progress & 1 ? "found" : "missing", g->stormbell_progress & 2 ? "repaired" : "broken",
            g->stormbell_phase == 3 ? "secure" : g->stormbell_phase ? "contested" : "silent");
    } else if (g->stormbell_quest_state == 2) {
        g->stormbell_quest_state = 3;
        g->gold += STORMBELL_REWARD_GOLD;
        g->score += STORMBELL_REWARD_SCORE;
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "The stormbell carries across the headland again! Ships can hear the danger before reaching the rocks. Take 180 gold with Dawnmere's thanks.");
        push_message(g, "Completed: The Silent Stormbell. +180 gold, +1200 score.");
    } else {
        snprintf(g->dialogue_text, sizeof(g->dialogue_text), "The stormbell still warns passing ships. You gave our watch its voice again.");
    }
    return 1;
}

const char *stormbell_hint(const GameState *g) {
    if (g->location != LOCATION_STORMWATCH) {
        return NULL;
    }
    if (g->level == 3 && !(g->stormbell_progress & STORMBELL_CLAPPER) && near(g, STORMBELL_CLAPPER_X, STORMBELL_CLAPPER_Y)) {
        return "RECOVER BELL CLAPPER";
    }
    if (g->level == 4 && near(g, STORMBELL_X, STORMBELL_Y)) {
        if (!(g->stormbell_progress & STORMBELL_FITTED)) {
            return g->stormbell_progress & STORMBELL_CLAPPER ? "FIT BELL CLAPPER" : "INSPECT BROKEN STORMBELL";
        }
        return g->stormbell_phase == 3 ? "STORMBELL RESTORED" : g->stormbell_phase ? "DEFEND THE STORMBELL" : "RING STORMBELL (CONFIRM)";
    }
    return NULL;
}

int stormbell_marker(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_STORMWATCH) {
        return 0;
    }
    if (g->level == 3 && !(g->stormbell_progress & STORMBELL_CLAPPER)) {
        *x = STORMBELL_CLAPPER_X;
        *y = STORMBELL_CLAPPER_Y;
        return 1;
    }
    if (g->level == 4 && g->stormbell_phase != 3) {
        *x = STORMBELL_X;
        *y = STORMBELL_Y;
        return 1;
    }
    return 0;
}

int stormbell_interact(GameState *g) {
    if (!stormbell_hint(g)) {
        return 0;
    }
    if (g->level == 3) {
        g->stormbell_progress |= STORMBELL_CLAPPER;
        push_message(g, "You recover the bronze bell clapper. Fit it to the bell beside Stormwatch 4's shutter. It needs no pack slot.");
    } else if (!(g->stormbell_progress & STORMBELL_CLAPPER)) {
        push_message(g, "The stormbell has no clapper. Search the northeast tower in Stormwatch 3.");
    } else if (!(g->stormbell_progress & STORMBELL_FITTED)) {
        g->stormbell_progress |= STORMBELL_FITTED;
        push_message(g, "You fit the clapper. Prepare before ringing: two Harpy Raiders will answer, then a Harpy Stormcaller.");
    } else if (!g->stormbell_phase) {
        g->dialogue_active = 1;
        g->dialogue_x = STORMBELL_X;
        g->dialogue_y = STORMBELL_Y;
        snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Ring the stormbell?");
        snprintf(g->dialogue_text, sizeof(g->dialogue_text),
            "Ringing draws two Harpy Raiders, followed by one Harpy Stormcaller. Prepare your spells and use the shutter, shelter and bracing. You may retreat; there is no time limit.");
    } else {
        push_message(g, g->stormbell_phase == 3 ? "The restored stormbell warns ships. Its defenders will not return." : "Harpies still answer the bell. Defeat its remaining defenders; ringing again will summon no more.");
    }
    return 1;
}

int stormbell_prompt_active(const GameState *g) {
    return g->dialogue_active && g->location == LOCATION_STORMWATCH && g->level == 4 &&
        !g->stormbell_phase && (g->stormbell_progress & STORMBELL_FITTED) &&
        strcmp(g->dialogue_speaker, "Ring the stormbell?") == 0;
}

void stormbell_capture(const GameState *g, Enemy guards[3]) {
    memcpy(guards, g->stormbell_guards, sizeof(Enemy) * 3);
    const StormwatchStage *stage = &g->stormwatch[3];
    int live = g->location == LOCATION_STORMWATCH && g->level == 4;
    const Enemy *enemies = live ? g->enemies : stage->enemies;
    int count = live ? g->enemy_count : stage->valid ? stage->enemy_count : 0;
    for (int i = 0; i < count; i++) {
        int index = index_of(&enemies[i]);
        if (index >= 0) {
            guards[index] = enemies[i];
        }
    }
}

// Cardinal flood fill keeps every arrival on the player's connected land, even
// when the shutter is raised. Re-entry retains wounds and warning turns.
static Enemy *place_guard(GameState *g, int i, int returning) {
    unsigned char seen[STORM_H][STORM_W] = {{0}};
    int queue[STORM_H * STORM_W];
    int head = 0;
    int tail = 1;
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    queue[0] = STORMBELL_Y * STORM_W + STORMBELL_X;
    seen[STORMBELL_Y][STORMBELL_X] = 1;
    while (head < tail) {
        int cell = queue[head++];
        for (int n = 0; n < 4; n++) {
            int x = cell % STORM_W + dx[n];
            int y = cell / STORM_W + dy[n];
            if (x < 0 || x >= STORM_W || y < 0 || y >= STORM_H || seen[y][x] || !map_is_walkable(&g->map, x, y)) {
                continue;
            }
            seen[y][x] = 1;
            queue[tail++] = y * STORM_W + x;
        }
    }
    int tx = returning ? g->stormbell_guards[i].x : arrivals[i][0];
    int ty = returning ? g->stormbell_guards[i].y : arrivals[i][1];
    for (int radius = 0; radius < STORM_W + STORM_H; radius++) {
        for (int n = 0; n < tail; n++) {
            int x = queue[n] % STORM_W;
            int y = queue[n] / STORM_W;
            if (abs(x - tx) + abs(y - ty) != radius || (x == g->player.x && y == g->player.y) ||
                (x == STORMBELL_X && y == STORMBELL_Y) || g->map.tiles[y][x] == TILE_ITEM) {
                continue;
            }
            Enemy *e = stormwatch_spawn(g, i == 2 ? ENEMY_HARPY_STORMCALLER : ENEMY_HARPY_RAIDER, x, y, 1);
            if (e) {
                if (returning) {
                    *e = g->stormbell_guards[i];
                    e->x = x;
                    e->y = y;
                } else {
                    snprintf(e->name, sizeof(e->name), "%s", names[i]);
                }
                g->stormbell_guards[i] = *e;
                g->level_cleared = 0;
                return e;
            }
        }
    }
    return NULL;
}

void stormbell_restore(GameState *g) {
    if (g->location != LOCATION_STORMWATCH || g->level != 4 || !g->stormbell_phase || g->stormbell_phase == 3) {
        return;
    }
    int count = g->stormbell_phase == 1 ? 2 : 3;
    for (int i = 0; i < count; i++) {
        if (!g->stormbell_guards[i].active) {
            continue;
        }
        int present = 0;
        for (int n = 0; n < g->enemy_count; n++) {
            present |= index_of(&g->enemies[n]) == i;
        }
        if (!present) {
            place_guard(g, i, 1);
        }
    }
}

int stormbell_handle_key(GameState *g, int key, int repeat) {
    if (!stormbell_prompt_active(g)) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (key == SDL_SCANCODE_N || key == SDL_SCANCODE_ESCAPE) {
        g->dialogue_active = 0;
        return 1;
    }
    if (key != SDL_SCANCODE_Y) {
        return 1;
    }
    // Check room for both before starting; full old floors can be cleared first.
    int slots = AREA_ENEMY_LIMIT - g->enemy_count;
    for (int n = 0; n < g->enemy_count; n++) {
        slots += !g->enemies[n].active;
    }
    g->dialogue_active = 0;
    if (slots < 2) {
        push_message(g, "Clear space among the courtyard's defenders before ringing the bell.");
        return 1;
    }
    g->stormbell_phase = 1;
    place_guard(g, 0, 0);
    place_guard(g, 1, 0);
    push_message(g, "The stormbell sounds! Two Harpy Raiders descend. The Stormcaller follows when both fall.");
    return 1;
}

void stormbell_turn(GameState *g) {
    if (g->location != LOCATION_STORMWATCH || g->level != 4 || !g->stormbell_phase || g->stormbell_phase == 3) {
        return;
    }
    Enemy guards[3];
    stormbell_capture(g, guards);
    memcpy(g->stormbell_guards, guards, sizeof(guards));
    if (g->stormbell_guards[0].active || g->stormbell_guards[1].active) {
        return;
    }
    if (g->stormbell_phase == 1) {
        if (place_guard(g, 2, 0)) {
            g->stormbell_phase = 2;
            push_message(g, "The Raiders fall. A Harpy Stormcaller descends: watch its lightning warning!");
        }
    } else if (!g->stormbell_guards[2].active) {
        g->stormbell_phase = 3;
        g->stormbell_progress |= STORMBELL_SECURE;
        push_message(g, "The courtyard is secure. The stormbell warns passing ships again.");
        stormbell_accept(g);
    }
}

void stormbell_migrate(GameState *g) {
    if (g->location != LOCATION_DAWNMERE || g->map.tiles[SEREN_Y][SEREN_X] == TILE_NPC_SEREN) {
        return;
    }
    // Leave an occupied quest site for the next town entry rather than moving
    // the player, a dropped item, or a return portal out from under an old save.
    if ((g->player.x == SEREN_X && g->player.y == SEREN_Y) || g->map.tiles[SEREN_Y][SEREN_X] != TILE_TOWN_FLOOR) {
        return;
    }
    g->map.tiles[SEREN_Y][SEREN_X] = TILE_NPC_SEREN;
}

int stormbell_valid(const GameState *g) {
    int state = g->stormbell_quest_state;
    int progress = g->stormbell_progress;
    int phase = g->stormbell_phase;
    if (state < 0 || state > 3 || progress < 0 || progress > 7 || phase < 0 || phase > 3 ||
        ((progress & STORMBELL_FITTED) && !(progress & STORMBELL_CLAPPER)) ||
        (phase && !(progress & STORMBELL_FITTED)) || !!(progress & STORMBELL_SECURE) != (phase == 3) ||
        (state == 1 && progress == 7) || (state >= 2 && progress != 7)) {
        return 0;
    }
    int count = phase == 1 ? 2 : phase ? 3 : 0;
    for (int i = 0; i < count; i++) {
        const Enemy *e = &g->stormbell_guards[i];
        if (index_of(e) != i || e->type != (i == 2 ? ENEMY_HARPY_STORMCALLER : ENEMY_HARPY_RAIDER) ||
            e->active < 0 || e->active > 1 || e->x < 0 || e->x >= STORM_W || e->y < 0 || e->y >= STORM_H ||
            e->max_hp <= 0 || e->hp > e->max_hp || (e->active && e->hp <= 0) || e->is_boss || !e->revived || e->experience) {
            return 0;
        }
    }
    return phase != 3 || (!g->stormbell_guards[0].active && !g->stormbell_guards[1].active && !g->stormbell_guards[2].active);
}
