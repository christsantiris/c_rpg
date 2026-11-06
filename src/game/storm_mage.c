#include "storm_mage.h"
#include "aldric.h"
#include "combat_feedback.h"
#include "eastern_arc.h"
#include "elowen.h"
#include "mangrove.h"
#include "town_road.h"
#include "western_arc.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

void storm_mage_init(GameState *g) {
    g->storm_mage = (StormMage){.state = STORM_MAGE_CAPTIVE, .x = -1, .y = -1, .hp = 70, .mp = 100, .level = 1};
}

int storm_mage_max_hp(const GameState *g) {
    return 70 + (g->storm_mage.level - 1) * 10;
}

int storm_mage_max_mp(const GameState *g) {
    int level = g->storm_mage.level;
    int early = level < 5 ? level : 5;
    int middle = level < 10 ? level : 10;
    return 100 + (early - 1) * 15 + (middle > 5 ? middle - 5 : 0) * 10 + (level > 10 ? level - 10 : 0) * 5;
}

static int position(const GameState *g, int *x, int *y) {
    if (g->storm_mage.state == STORM_MAGE_FOLLOWING && !mangrove_aboard(g)) {
        *x = g->storm_mage.x;
        *y = g->storm_mage.y;
        return *x >= 0 && *y >= 0;
    }
    if (g->storm_mage.state <= STORM_MAGE_FREED && g->location == LOCATION_STORMWATCH && g->level == STORM_DEPTH) {
        *x = STORM_MAGE_X;
        *y = STORM_MAGE_Y;
        return 1;
    }
    if (g->storm_mage.state == STORM_MAGE_WAITING && g->location == LOCATION_ALEHOUSE) {
        *x = STORM_MAGE_INN_X;
        *y = STORM_MAGE_INN_Y;
        return 1;
    }
    return 0;
}

int storm_mage_at(const GameState *g, int x, int y) {
    int mx;
    int my;
    return position(g, &mx, &my) && x == mx && y == my;
}

int storm_mage_beside(const GameState *g, int x, int y) {
    int dx = abs(g->storm_mage.x - x);
    int dy = abs(g->storm_mage.y - y);
    return g->storm_mage.state == STORM_MAGE_FOLLOWING && !mangrove_aboard(g) && dx <= 1 && dy <= 1 && dx + dy > 0;
}

static int open_space(const GameState *g, int x, int y) {
    if (!map_is_walkable(&g->map, x, y) || map_is_travel_tile(g->map.tiles[y][x]) ||
        (x == g->player.x && y == g->player.y) || aldric_at(g, x, y) || elowen_at(g, x, y)) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 0;
        }
    }
    return 1;
}

void storm_mage_sync(GameState *g) {
    if (g->stormwatch_defeated && g->storm_mage.state == STORM_MAGE_CAPTIVE) {
        g->storm_mage.state = STORM_MAGE_FREED;
    }
}

void storm_mage_follow(GameState *g, int old_x, int old_y) {
    StormMage *mage = &g->storm_mage;
    if (mage->state != STORM_MAGE_FOLLOWING) {
        return;
    }
    if (town_road_index(g->location) >= 0 && !eastern_town_combat(g)) {
        mage->hp = storm_mage_max_hp(g);
        mage->mp = storm_mage_max_mp(g);
    }
    if (mangrove_aboard(g)) {
        mage->x = g->player.x;
        mage->y = g->player.y;
        return;
    }
    int lead_x = g->aldric_party_state == ALDRIC_FOLLOWING ? g->aldric_x : g->player.x;
    int lead_y = g->aldric_party_state == ALDRIC_FOLLOWING ? g->aldric_y : g->player.y;
    if (abs(old_x - lead_x) + abs(old_y - lead_y) == 1 && open_space(g, old_x, old_y)) {
        mage->x = old_x;
        mage->y = old_y;
    } else if (abs(mage->x - g->player.x) > 3 || abs(mage->y - g->player.y) > 3 || !open_space(g, mage->x, mage->y)) {
        mage->x = g->player.x;
        mage->y = g->player.y;
        // A narrow landing may temporarily share the player's tile, as Aldric does.
        int placed = 0;
        for (int radius = 1; radius <= 3 && !placed; radius++) {
            for (int y = g->player.y - radius; y <= g->player.y + radius && !placed; y++) {
                for (int x = g->player.x - radius; x <= g->player.x + radius; x++) {
                    if (abs(x - g->player.x) + abs(y - g->player.y) == radius && open_space(g, x, y)) {
                        mage->x = x;
                        mage->y = y;
                        placed = 1;
                        break;
                    }
                }
            }
        }
    }
    if (g->dialogue_active && strcmp(g->dialogue_speaker, STORM_MAGE_SPEAKER) == 0) {
        g->dialogue_x = mage->x;
        g->dialogue_y = mage->y;
    }
}

int storm_mage_hostile(const GameState *g, const Enemy *e) {
    if (!e->active || (western_convoy_enemy(g, e) && !e->boss_aux_x)) {
        return 0;
    }
    if (g->location == LOCATION_DAWNMERE && !g->dawnmere_liberated && !e->boss_aux_x) {
        return 0;
    }
    return 1;
}

static int firing_lane(const GameState *g, int x, int y, const Enemy *target) {
    Spell fireball = spell_make_fireball();
    int distance = abs(target->x - x) + abs(target->y - y);
    if ((x != target->x && y != target->y) || distance < 1 || distance > fireball.range) {
        return 0;
    }
    int dx = (target->x > x) - (target->x < x);
    int dy = (target->y > y) - (target->y < y);
    for (int step = 1; step <= distance; step++) {
        int px = x + dx * step;
        int py = y + dy * step;
        if (!map_is_walkable(&g->map, px, py) || (px == g->player.x && py == g->player.y) ||
            aldric_at(g, px, py) || elowen_at(g, px, py)) {
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

void storm_mage_turn(GameState *g) {
    StormMage *mage = &g->storm_mage;
    if (mage->state != STORM_MAGE_FOLLOWING || mage->x < 0 || g->player.hp <= 0 || mangrove_aboard(g)) {
        return;
    }
    Enemy *threat = NULL;
    int nearest = MAP_W + MAP_H;
    Spell fireball = spell_make_fireball();
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (!storm_mage_hostile(g, e) || abs(e->x - g->player.x) > 4 || abs(e->y - g->player.y) > 4) {
            continue;
        }
        int distance = abs(e->x - mage->x) + abs(e->y - mage->y);
        if (mage->mp >= fireball.mp_cost && firing_lane(g, mage->x, mage->y, e)) {
            action_storm_mage_attack(g, e, 1);
            return;
        }
        if (mage->mp < fireball.mp_cost && distance == 1) {
            action_storm_mage_attack(g, e, 0);
            return;
        }
        if (distance < nearest) {
            threat = e;
            nearest = distance;
        }
    }
    if (!threat) {
        return;
    }
    const int steps[4][2] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};
    int best = -nearest;
    int bx = mage->x;
    int by = mage->y;
    for (int i = 0; i < 4; i++) {
        int x = mage->x + steps[i][0];
        int y = mage->y + steps[i][1];
        if (!open_space(g, x, y) || abs(x - g->player.x) > 3 || abs(y - g->player.y) > 3) {
            continue;
        }
        int distance = abs(threat->x - x) + abs(threat->y - y);
        int score = -distance;
        if (mage->mp >= fireball.mp_cost) {
            score += firing_lane(g, x, y, threat) ? 10 : 0;
            score += distance >= 2 ? 3 : 0;
        }
        if (score > best) {
            best = score;
            bx = x;
            by = y;
        }
    }
    mage->x = bx;
    mage->y = by;
}

void storm_mage_gain_xp(GameState *g, int xp) {
    StormMage *mage = &g->storm_mage;
    if (mage->state != STORM_MAGE_FOLLOWING || xp <= 0) {
        return;
    }
    mage->experience += xp;
    while (mage->level < 50 && mage->experience >= mage->level * 100) {
        mage->experience -= mage->level * 100;
        mage->level++;
        mage->hp = storm_mage_max_hp(g);
        mage->mp = storm_mage_max_mp(g);
        char message[MAX_MESSAGE_LEN];
        snprintf(message, sizeof(message), "Soren reaches Mage level %d.", mage->level);
        push_message(g, message);
    }
}

void storm_mage_take_hit(GameState *g, const Enemy *e) {
    StormMage *mage = &g->storm_mage;
    if (mage->state != STORM_MAGE_FOLLOWING) {
        return;
    }
    int damage = e->attack - (mage->level + 1);
    if (damage < 1) {
        damage = 1;
    }
    mage->hp -= damage;
    combat_feedback_add(g, FEEDBACK_ALLY_DAMAGE, FEEDBACK_NOW, mage->x, mage->y, damage);
    char message[MAX_MESSAGE_LEN];
    if (mage->hp <= 0) {
        mage->hp = 0;
        mage->state = STORM_MAGE_DEAD;
        mage->x = -1;
        mage->y = -1;
        snprintf(message, sizeof(message), "Soren has died. Seek Soulstone Shrine north of Oakhaven for resurrection.");
    } else {
        snprintf(message, sizeof(message), "%s hits Soren: %d dmg", e->name, damage);
    }
    push_message_kind(g, message, MESSAGE_DAMAGE_TAKEN);
}

int storm_mage_revive(GameState *g, int hp) {
    if (g->storm_mage.state != STORM_MAGE_DEAD || hp <= 0) {
        return 0;
    }
    const int steps[4][2] = {{0, 1}, {-1, 0}, {1, 0}, {0, -1}};
    for (int i = 0; i < 4; i++) {
        int x = g->player.x + steps[i][0];
        int y = g->player.y + steps[i][1];
        if (open_space(g, x, y)) {
            g->storm_mage.state = STORM_MAGE_FOLLOWING;
            g->storm_mage.hp = hp > storm_mage_max_hp(g) ? storm_mage_max_hp(g) : hp;
            g->storm_mage.x = x;
            g->storm_mage.y = y;
            combat_feedback_add(g, FEEDBACK_HEAL, FEEDBACK_NOW, x, y, g->storm_mage.hp);
            push_message(g, "Soren returns to life and rejoins your party.");
            return 1;
        }
    }
    push_message(g, "Make room beside you for Soren before performing the ritual.");
    return 0;
}

int storm_mage_talk(GameState *g, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || abs(x - g->player.x) > 1 || abs(y - g->player.y) > 1) {
        return 0;
    }
    const char *speaker;
    const char *text;
    if (g->map.tiles[y][x] == TILE_NPC_STORM_GUIDE) {
        int oak = g->location == LOCATION_TOWN;
        speaker = oak ? "Traveler Neris" : "Fisher Tovan";
        text = oak ?
            "A powerful mage named Soren vanished at Stormwatch Headland, east of Dawnmere. They say the Tempest Matriarch holds him in her fort. If you travel that way, he may still be alive." :
            "Soren once defended this shore with fire. The Tempest Matriarch captured him at Stormwatch Headland, east of town. Defeat her in the final court to break his bonds. You can reach the headland by walking around our walls.";
        if (g->storm_mage.state == STORM_MAGE_FOLLOWING) {
            text = "You freed Soren! It is good to see him traveling in your party. The Matriarch could not extinguish his fire.";
        } else if (g->storm_mage.state == STORM_MAGE_DEAD) {
            text = "I heard Soren has fallen. Soulstone Shrine, north of Oakhaven, can resurrect a lost companion.";
        } else if (g->storm_mage.state == STORM_MAGE_WAITING) {
            text = "Soren is free and waiting at Dawnmere's Alehouse. Speak to him there if you want a mage to join your party.";
        } else if (g->stormwatch_defeated) {
            text = "The Matriarch has fallen! Soren should be free of her ward. Speak to him in the final court at Stormwatch Headland.";
        }
    } else if (storm_mage_at(g, x, y)) {
        storm_mage_sync(g);
        speaker = STORM_MAGE_SPEAKER;
        text = g->storm_mage.state == STORM_MAGE_CAPTIVE ?
            "The Matriarch's ward holds me here. Defeat her to break it. Take care of yourself first; her winds are deadly." :
            g->storm_mage.state == STORM_MAGE_FOLLOWING ?
            "My fire is yours. At Dawnmere's Alehouse, you can ask me to wait if you wish to travel without me." :
            g->storm_mage.recruited ? "Ready to travel again? My staff and Fireball are at your service." :
            "You broke her hold! Captivity drained my strength, but I still know Fireball. I must begin again as a level 1 Mage. Let me join your party and repay this debt.";
    } else {
        return 0;
    }
    g->dialogue_active = 1;
    g->dialogue_x = x;
    g->dialogue_y = y;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "%s", speaker);
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s", text);
    return 1;
}

int storm_mage_prompt_active(const GameState *g) {
    return g->dialogue_active && strcmp(g->dialogue_speaker, STORM_MAGE_SPEAKER) == 0 &&
        (g->storm_mage.state == STORM_MAGE_FREED || g->storm_mage.state == STORM_MAGE_WAITING ||
        (g->storm_mage.state == STORM_MAGE_FOLLOWING && g->location == LOCATION_ALEHOUSE));
}

int storm_mage_handle_key(GameState *g, int key, int repeat) {
    if (!storm_mage_prompt_active(g)) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (key != SDL_SCANCODE_Y && key != SDL_SCANCODE_N && key != SDL_SCANCODE_ESCAPE) {
        return 1;
    }
    int following = g->storm_mage.state == STORM_MAGE_FOLLOWING;
    g->dialogue_active = 0;
    if (key == SDL_SCANCODE_Y && !following) {
        g->storm_mage.state = STORM_MAGE_FOLLOWING;
        g->storm_mage.recruited = 1;
        storm_mage_follow(g, -1, -1);
        push_message(g, "Mage Soren joins your party. He knows Fireball.");
    } else if ((key == SDL_SCANCODE_Y && following) || g->storm_mage.state == STORM_MAGE_FREED) {
        g->storm_mage.state = STORM_MAGE_WAITING;
        g->storm_mage.x = -1;
        g->storm_mage.y = -1;
        push_message(g, "Soren will wait at Dawnmere's Alehouse. Speak to him there to recruit him.");
    }
    return 1;
}

void storm_mage_place_guide(GameState *g) {
    int x = g->location == LOCATION_TOWN ? STORM_GUIDE_OAK_X : STORM_GUIDE_DAWN_X;
    int y = g->location == LOCATION_TOWN ? STORM_GUIDE_OAK_Y : STORM_GUIDE_DAWN_Y;
    if (g->location != LOCATION_TOWN && g->location != LOCATION_ALEHOUSE) {
        return;
    }
    if (g->player.x == x && g->player.y == y) {
        g->player.y++;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->y++;
            item->underlying_tile = g->location == LOCATION_TOWN ? TILE_TOWN_PATH : TILE_TAVERN_FLOOR;
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
    g->map.tiles[y][x] = TILE_NPC_STORM_GUIDE;
}

void storm_mage_migrate(GameState *g, int version) {
    storm_mage_sync(g);
    if (version < 245) {
        storm_mage_place_guide(g);
        if (g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_DAWNMERE) {
            town_road_migrate(g);
        }
    }
}

int storm_mage_valid(const GameState *g) {
    const StormMage *mage = &g->storm_mage;
    return mage->state >= STORM_MAGE_CAPTIVE && mage->state <= STORM_MAGE_DEAD &&
        mage->recruited >= 0 && mage->recruited <= 1 && mage->level >= 1 && mage->level <= 50 &&
        mage->experience >= 0 && (mage->level == 50 || mage->experience < mage->level * 100) &&
        mage->hp >= 0 && mage->hp <= storm_mage_max_hp(g) && mage->mp >= 0 && mage->mp <= storm_mage_max_mp(g) &&
        (mage->state != STORM_MAGE_DEAD || (mage->recruited && mage->hp == 0)) &&
        (mage->state != STORM_MAGE_FOLLOWING || (mage->recruited && mage->hp > 0 && mage->x >= 0 && mage->x < MAP_W && mage->y >= 0 && mage->y < MAP_H));
}
