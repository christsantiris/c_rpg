#include "eastern_arc.h"
#include "aldric.h"
#include "mangrove.h"
#include "town_life.h"
#include "town_road.h"
#include "stormbell.h"
#include <stdlib.h>
#include <string.h>

int eastern_town_combat(const GameState *g) {
    return (g->location == LOCATION_DAWNMERE && !g->dawnmere_liberated) ||
        (g->location == LOCATION_TOWN2 && g->royal_reported_actions);
}

void eastern_init(GameState *g) {
    memset(&g->dawnmere_cache, 0, sizeof(g->dawnmere_cache));
    memset(g->dawnmere_loot, 0, sizeof(g->dawnmere_loot));
    g->dawnmere_loot_count = 0;
    g->dawnmere_liberated = 0;
    g->dawnmere_alerted = 0;
    g->dawnmere_progress = 0;
    g->dawnmere_quest_state = 0;
    g->saltwind_quest_state = 0;
    g->royal_reported_actions = 0;
    g->act4_arrived = 0;
    g->stillbury_challenge = 0;
    g->stillbury_prompt = 0;
    g->stillbury_guards_initialized = 0;
    memset(g->stillbury_guards, 0, sizeof(g->stillbury_guards));
    memset(&g->river_boat, 0, sizeof(g->river_boat));
}

void eastern_store(GameState *g) {
    if (g->location == LOCATION_DAWNMERE) {
        LevelCache *cache = &g->dawnmere_cache;
        cache->valid = 1;
        cache->map = g->map;
        cache->enemy_count = g->enemy_count;
        memcpy(cache->enemies, g->enemies, sizeof(Enemy) * g->enemy_count);
        cache->level_cleared = g->dawnmere_liberated;
        g->dawnmere_loot_count = g->floor_item_count;
        memcpy(g->dawnmere_loot, g->floor_items, sizeof(FloorItem) * g->floor_item_count);
    }
    if (g->location == LOCATION_TOWN2 && g->stillbury_guards_initialized && g->enemy_count == 3) {
        memcpy(g->stillbury_guards, g->enemies, sizeof(g->stillbury_guards));
    }
}

static void restore_town_tile(GameState *g, int x, int y, TileType tile) {
    g->map.tiles[y][x] = tile;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (!item->active || item->x != x || item->y != y) {
            continue;
        }
        if (tile == TILE_WALL) {
            item->x = x == 0 ? 1 : x == TOWN_W - 1 ? TOWN_W - 2 : x;
            item->y = y == 0 ? 1 : y == TOWN_H - 1 ? TOWN_H - 2 : y;
            item->underlying_tile = g->location == LOCATION_DAWNMERE ? map_dawnmere_ground(item->x, item->y) : TILE_TOWN_PATH;
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        } else {
            item->underlying_tile = tile;
            if (!map_is_travel_tile(tile)) {
                g->map.tiles[y][x] = TILE_ITEM;
            }
        }
    }
}

void eastern_refresh(GameState *g) {
    if (g->location == LOCATION_ALEHOUSE && !g->dawnmere_liberated) {
        g->map.tiles[DAWNMERE_SEREN_Y][DAWNMERE_SEREN_X] = TILE_NPC_SEREN;
    }
    if (g->location == LOCATION_TOWN2 || g->location == LOCATION_DAWNMERE) {
        for (int y = 0; y < TOWN_H; y++) {
            restore_town_tile(g, TOWN_W - 1, y, TILE_WALL);
            if (g->location == LOCATION_TOWN2) {
                restore_town_tile(g, 0, y, TILE_WALL);
            }
        }
        if (g->location == LOCATION_TOWN2) {
            for (int x = 0; x < TOWN_W; x++) {
                restore_town_tile(g, x, 0, TILE_WALL);
                restore_town_tile(g, x, TOWN_H - 1, TILE_WALL);
            }
            restore_town_tile(g, 20, TOWN_H - 1, TILE_TOWN_EXIT);
        }
    }
    if (g->location == LOCATION_DAWNMERE) {
        restore_town_tile(g, 20, TOWN_H - 1, TILE_WALL);
        restore_town_tile(g, 20, 0, TILE_TOWN_EXIT);
        if (!g->dawnmere_liberated && g->map.tiles[SEREN_Y][SEREN_X] == TILE_NPC_SEREN) {
            g->map.tiles[SEREN_Y][SEREN_X] = map_dawnmere_ground(SEREN_X, SEREN_Y);
        }
        if (g->dawnmere_liberated) {
            g->map.tiles[SEREN_Y][SEREN_X] = TILE_NPC_SEREN;
        }
    }
}

// Each squad holds three guards and two archers; Rusk leads the waterfront command.
static void spawn_soldier(GameState *g, Enemy *e, int i) {
    static const int posts[DAWNMERE_GARRISON][2] = {
        {9, 12}, {10, 13}, {12, 11}, {10, 11}, {13, 13},
        {20, 12}, {20, 14}, {23, 12}, {18, 13}, {23, 14},
        {34, 3}, {32, 4}, {37, 3}, {35, 5}, {38, 4}
    };
    int archer = i % DAWNMERE_SQUAD == 2 || i % DAWNMERE_SQUAD == 4;
    game_spawn_enemy(g, e, archer ? ENEMY_ROAD_ARCHER : ENEMY_ROYAL_GUARD, posts[i][0], posts[i][1]);
    e->boss_aux_x = 0;
    e->boss_aux_y = i / DAWNMERE_SQUAD + 1;
    e->max_hp = e->hp = i == DAWNMERE_RUSK ? 150 : archer ? 50 : 80;
    e->attack = i == DAWNMERE_RUSK ? 25 : archer ? 20 : 22;
    e->defense = i == DAWNMERE_RUSK ? 8 : archer ? 3 : 6;
    e->experience = i == DAWNMERE_RUSK ? 240 : 85;
    if (i == DAWNMERE_RUSK) {
        snprintf(e->name, sizeof(e->name), "Captain Rusk");
    }
}

static int post_free(const GameState *g, const Map *map, const Enemy *enemies, int index, int live) {
    const Enemy *e = &enemies[index];
    if (!map_is_walkable(map, e->x, e->y) || (live && e->x == g->player.x && e->y == g->player.y)) {
        return 0;
    }
    for (int i = 0; i < DAWNMERE_GARRISON; i++) {
        if (i != index && enemies[i].active && enemies[i].x == e->x && enemies[i].y == e->y) {
            return 0;
        }
    }
    return 1;
}

// Saves from the nine-soldier garrison keep every survivor and casualty in its squad.
static void reinforce_roster(GameState *g, const Map *map, Enemy *enemies, int *count, int live) {
    if (*count != 9) {
        return;
    }
    Enemy old[9];
    memcpy(old, enemies, sizeof(old));
    for (int i = 0; i < DAWNMERE_GARRISON; i++) {
        int squad = i / DAWNMERE_SQUAD;
        int slot = i % DAWNMERE_SQUAD;
        if (slot < 3) {
            enemies[i] = old[squad * 3 + slot];
            continue;
        }
        spawn_soldier(g, &enemies[i], i);
        enemies[i].active = 0;
    }
    for (int i = 0; i < DAWNMERE_GARRISON; i++) {
        int squad = i / DAWNMERE_SQUAD;
        Enemy *e = &enemies[i];
        if (i % DAWNMERE_SQUAD < 3 || !(old[squad * 3].active || old[squad * 3 + 1].active || old[squad * 3 + 2].active)) {
            continue;
        }
        // Reinforcements join a squad still holding its position, beside any occupied post.
        e->boss_aux_x = old[squad * 3].boss_aux_x;
        int x = e->x;
        int y = e->y;
        for (int d = 0; d < 9 && !e->active; d++) {
            e->x = x + (d + 4) % 9 % 3 - 1;
            e->y = y + (d + 4) % 9 / 3 - 1;
            e->active = post_free(g, map, enemies, i, live);
        }
    }
    *count = DAWNMERE_GARRISON;
}

void eastern_reinforce(GameState *g) {
    if (g->location == LOCATION_DAWNMERE) {
        reinforce_roster(g, &g->map, g->enemies, &g->enemy_count, 1);
        eastern_store(g);
    } else if (g->dawnmere_cache.valid) {
        reinforce_roster(g, &g->dawnmere_cache.map, g->dawnmere_cache.enemies, &g->dawnmere_cache.enemy_count, 0);
    }
}

void eastern_enter_town(GameState *g) {
    if (g->location == LOCATION_DAWNMERE) {
        if (g->dawnmere_cache.valid) {
            g->map = g->dawnmere_cache.map;
            g->enemy_count = g->dawnmere_cache.enemy_count;
            memcpy(g->enemies, g->dawnmere_cache.enemies, sizeof(Enemy) * g->enemy_count);
            g->floor_item_count = g->dawnmere_loot_count;
            memcpy(g->floor_items, g->dawnmere_loot, sizeof(FloorItem) * g->floor_item_count);
        } else if (!g->dawnmere_liberated) {
            g->enemy_count = DAWNMERE_GARRISON;
            for (int i = 0; i < DAWNMERE_GARRISON; i++) {
                Enemy *e = &g->enemies[i];
                spawn_soldier(g, e, i);
                if (e->x == g->player.x && e->y == g->player.y) {
                    // Old town saves may place the player at a new sentry position.
                    e->y += 2;
                }
            }
        }
        if (g->dawnmere_liberated) {
            g->last_town_visited = LOCATION_DAWNMERE;
        }
        eastern_refresh(g);
        eastern_store(g);
        push_message(g, g->dawnmere_liberated ? "Dawnmere is free. Seren waits by the square; Saltwind lies north." :
            "Royal occupation: squads attack anyone who comes near. Seren shelters in the Alehouse; the west road is your retreat.");
        if (!g->dawnmere_liberated && !g->dawnmere_alerted && g->player.x == 1) {
            // The checkpoint orders each arrival at the west gate to leave before any fighting starts.
            g->dialogue_active = 1;
            g->dialogue_x = g->enemies[0].x;
            g->dialogue_y = g->enemies[0].y;
            snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Royal Checkpoint");
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "Halt! By order of the King, Dawnmere is under martial law for refusing his levies. Its people are rebels, and so is any outsider in its streets. Leave town now, or we attack.");
        }
    }
    if (g->location == LOCATION_TOWN2) {
        eastern_refresh(g);
        g->act4_arrived = 1;
        if (!g->royal_reported_actions) {
            g->last_town_visited = LOCATION_TOWN2;
        }
        if (g->stillbury_challenge == 2 && g->stillbury_guards_initialized) {
            g->enemy_count = 3;
            memcpy(g->enemies, g->stillbury_guards, sizeof(g->stillbury_guards));
        }
        push_message(g, g->royal_reported_actions ? "Act 4: Stillbury. The Crown knows your actions. Its checkpoint will demand surrender." :
            "Act 4: Stillbury. Loyalist banners hang above a welcoming customs gate.");
    }
}

void eastern_update(GameState *g) {
    if (g->location != LOCATION_DAWNMERE || g->dawnmere_liberated || g->enemy_count != DAWNMERE_GARRISON) {
        return;
    }
    static const char *const squads[3] = {"the checkpoint squad", "the market squad", "Captain Rusk's command"};
    int engaged[3] = {0};
    int struck[3] = {0};
    int near[3] = {0};
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        int squad = i / DAWNMERE_SQUAD;
        engaged[squad] |= e->boss_aux_x;
        struck[squad] |= !e->active || e->hp < e->max_hp;
        near[squad] |= e->active && abs(e->x - g->player.x) + abs(e->y - g->player.y) <= 3;
    }
    // Tell the player what provoked each squad the moment it joins the fight.
    for (int squad = 0; squad < 3; squad++) {
        if (!engaged[squad] && (struck[squad] || near[squad])) {
            char message[MAX_MESSAGE_LEN];
            snprintf(message, sizeof(message), struck[squad] ? "You attacked %s. Its soldiers fight back!" :
                "You came too close to %s. Its soldiers attack!", squads[squad]);
            push_message(g, message);
        }
        engaged[squad] |= struck[squad] || near[squad];
    }
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].boss_aux_x = engaged[i / DAWNMERE_SQUAD];
        if (engaged[i / DAWNMERE_SQUAD]) {
            g->dawnmere_alerted = 1;
        }
    }
    int cleared[3] = {1, 1, !g->enemies[DAWNMERE_RUSK].active};
    for (int i = 0; i < DAWNMERE_GARRISON; i++) {
        if (!g->enemies[i].active || g->enemies[i].hp < g->enemies[i].max_hp) {
            g->dawnmere_alerted = 1;
        }
        if (i < DAWNMERE_RUSK && g->enemies[i].active) {
            cleared[i / DAWNMERE_SQUAD] = 0;
        }
    }
    for (int group = 0; group < 3; group++) {
        if (cleared[group]) {
            g->dawnmere_progress |= 1 << group;
        }
    }
    if (g->dawnmere_progress != 7) {
        return;
    }
    g->dawnmere_liberated = 1;
    g->dawnmere_quest_state = 3;
    g->royal_reported_actions |= 1;
    g->last_town_visited = LOCATION_DAWNMERE;
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
    g->gold += 100;
    player_gain_xp(g, 500);
    eastern_refresh(g);
    eastern_store(g);
    push_message(g, "Dawnmere is liberated! +100 gold, +500 XP. Rusk's orders name Stillbury; speak to Seren about Saltwind.");
}

int eastern_move(GameState *g, int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
        return 0;
    }
    if (g->location == LOCATION_DAWNMERE && !g->dawnmere_liberated) {
        if (x == 20 && y == 0) {
            push_message(g, "The garrison holds the northern waterfront. Leave by the west gate and walk around the town to reach Saltwind.");
            return 1;
        }
        if (g->map.tiles[y][x] == TILE_LOCAL_DOOR) {
            const TownBuilding *b = town_life_building(LOCATION_DAWNMERE);
            for (; b; b = town_life_next_building(LOCATION_DAWNMERE, b)) {
                if (x == b->x + b->w / 2 && y == b->y + b->h - 1 && b->interior != LOCATION_ALEHOUSE) {
                    push_message(g, "Shuttered by royal order. Seren shelters in the Alehouse.");
                    return 1;
                }
            }
        }
        if (x >= 7 && x <= 13 && y >= 10 && y <= 14) {
            if (!g->enemies[0].boss_aux_x) {
                push_message(g, "You crossed the royal checkpoint. The checkpoint squad attacks!");
            }
            g->dawnmere_alerted = 1;
            for (int i = 0; i < DAWNMERE_SQUAD; i++) {
                g->enemies[i].boss_aux_x = 1;
            }
        }
    }
    if (g->location == LOCATION_TOWN2 && g->royal_reported_actions && !g->stillbury_challenge && x >= 17 && x <= 23 && y >= 17 && y <= 19) {
        g->stillbury_prompt = 1;
        g->dialogue_active = 1;
        snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Royal Checkpoint");
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "You are wanted for acts against the Crown. Surrender your passage. Y: submit and return to the dock. N: resist. Esc: withdraw.");
        g->dialogue_x = 20;
        g->dialogue_y = 17;
        return 1;
    }
    return 0;
}

int eastern_talk(GameState *g) {
    int x = g->location == LOCATION_ALEHOUSE ? DAWNMERE_SEREN_X : SEREN_X;
    int y = g->location == LOCATION_ALEHOUSE ? DAWNMERE_SEREN_Y : SEREN_Y;
    if ((g->location != LOCATION_DAWNMERE && g->location != LOCATION_ALEHOUSE) ||
        abs(g->player.x - x) > 1 || abs(g->player.y - y) > 1 ||
        g->map.tiles[y][x] != TILE_NPC_SEREN) {
        return 0;
    }
    g->dialogue_active = 1;
    g->dialogue_x = x;
    g->dialogue_y = y;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Watchkeeper Seren");
    if (!g->dawnmere_liberated && g->dawnmere_quest_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "Checkpoint: %s. Market: %s. Captain Rusk: %s. Break these three positions to free Dawnmere and reopen its northern waterfront. The forest road remains your retreat.",
            g->dawnmere_progress & 1 ? "cleared" : "held", g->dawnmere_progress & 2 ? "cleared" : "held", g->dawnmere_progress & 4 ? "defeated" : "in command");
        return 1;
    }
    const char *text = !g->dawnmere_liberated ?
        "Dawnmere refused the Crown's levies. Rusk took our square and waterfront. Break the checkpoint, the market squad and his command. Stillbury will call you a rebel. Will you free our town?" :
        g->saltwind_quest_state == 3 ? "You found passage to Stillbury. Its authorities know what you did here, but Dawnmere remembers its liberator. Our shops and retreat road remain open." :
        "Dawnmere is free, but the royal river blockade remains. Saltwind lies north: follow its skiff landings, defeat Sszareth and reclaim the final launch. Paddle across to Stillbury's dock. Will you find passage?";
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "%s", text);
    return 1;
}

int eastern_challenge_active(const GameState *g) {
    return g->location == LOCATION_DAWNMERE && !g->dawnmere_liberated && !g->dawnmere_alerted && g->dialogue_active &&
        strcmp(g->dialogue_speaker, "Royal Checkpoint") == 0;
}

int eastern_handle_key(GameState *g, int key, int repeat) {
    if (eastern_challenge_active(g)) {
        if (repeat) {
            return 1;
        }
        if (key == SDL_SCANCODE_N) {
            g->dialogue_active = 0;
            g->dawnmere_alerted = 1;
            for (int i = 0; i < DAWNMERE_SQUAD; i++) {
                g->enemies[i].boss_aux_x = 1;
            }
            push_message(g, "You force your way into Dawnmere. The checkpoint squad attacks!");
        } else if (key == SDL_SCANCODE_Y || key == SDL_SCANCODE_ESCAPE) {
            g->dialogue_active = 0;
            town_road_enter(g, LOCATION_DAWNMERE, TOWN_SIDE_WEST, 1);
        }
        return 1;
    }
    if (!g->stillbury_prompt) {
        return 0;
    }
    if (repeat) {
        return 1;
    }
    if (key == SDL_SCANCODE_Y || key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_N) {
        g->stillbury_prompt = 0;
        g->dialogue_active = 0;
        if (key == SDL_SCANCODE_N) {
            g->stillbury_challenge = 2;
            g->stillbury_guards_initialized = 1;
            g->enemy_count = 3;
            for (int i = 0; i < 3; i++) {
                game_spawn_enemy(g, &g->enemies[i], i == 2 ? ENEMY_ROAD_ARCHER : ENEMY_ROYAL_GUARD, 19 + i, 16 - (i == 2));
                g->enemies[i].boss_aux_y = 1;
            }
            eastern_store(g);
            push_message(g, "The checkpoint turns hostile. The dock approach remains your retreat.");
        } else {
            g->player.x = 20;
            g->player.y = 22;
            push_message(g, "You withdraw toward the dock. Stillbury remains suspicious.");
        }
    }
    return 1;
}

void eastern_river_map(Map *map) {
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = x < MANGROVE_W && y < MANGROVE_H ? TILE_MANGROVE_WATER : TILE_WALL;
        }
    }
    for (int y = 0; y < MANGROVE_H; y++) {
        for (int x = 0; x < MANGROVE_W; x++) {
            if (y <= RIVER_NORTH_Y || y >= RIVER_SOUTH_Y) {
                map->tiles[y][x] = x > 0 && x < MANGROVE_W - 1 ? TILE_TOWN_FLOOR : TILE_MANGROVE_ROOT;
            }
        }
    }
    for (int y = 0; y < MANGROVE_H; y++) {
        if (y < RIVER_NORTH_Y || y > RIVER_SOUTH_Y) {
            map->tiles[y][y < RIVER_NORTH_Y ? RIVER_NORTH_X : RIVER_SOUTH_X] = TILE_TOWN_PATH;
        }
    }
    map->tiles[RIVER_NORTH_Y][RIVER_NORTH_X] = TILE_MANGROVE_LANDING;
    map->tiles[RIVER_SOUTH_Y][RIVER_SOUTH_X] = TILE_MANGROVE_LANDING;
    map->tiles[0][RIVER_NORTH_X] = TILE_MANGROVE_EXIT;
    map->tiles[MANGROVE_H - 1][RIVER_SOUTH_X] = TILE_MANGROVE_RETURN;
    map->stairs_up_x = RIVER_SOUTH_X;
    map->stairs_up_y = MANGROVE_H - 1;
    map->stairs_down_x = RIVER_NORTH_X;
    map->stairs_down_y = 0;
}

void eastern_river_enter(GameState *g, int from_town) {
    mangrove_store(g);
    eastern_store(g);
    g->location = LOCATION_RIVER_CROSSING;
    g->level = 1;
    eastern_river_map(&g->map);
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    if (!g->river_boat.initialized) {
        g->river_boat = (MangroveState){.initialized = 1, .stage = 1, .boat_x = RIVER_SOUTH_X, .boat_y = RIVER_SOUTH_Y - 1, .boat_dx = 0, .boat_dy = -1};
    }
    // A return from town reaches the shore where the player last docked.
    g->player.x = from_town ? RIVER_NORTH_X : RIVER_SOUTH_X;
    g->player.y = from_town ? 1 : MANGROVE_H - 2;
    g->river_boat.aboard = 0;
    if ((from_town && g->river_boat.boat_y > RIVER_NORTH_Y + 1) ||
        (!from_town && g->river_boat.boat_y < RIVER_SOUTH_Y - 1)) {
        g->river_boat.boat_x = from_town ? RIVER_NORTH_X : RIVER_SOUTH_X;
        g->river_boat.boat_y = from_town ? RIVER_NORTH_Y + 1 : RIVER_SOUTH_Y - 1;
    }
    for (int y = 0; y < MANGROVE_H; y++) {
        for (int x = 0; x < MANGROVE_W; x++) {
            int bit = y * MANGROVE_W + x;
            if (g->river_boat.explored[bit / 8] & (1u << (bit % 8))) {
                map_mark_explored(&g->map, x, y);
            }
        }
    }
    g->floor_item_count = g->river_boat.loot_count;
    memcpy(g->floor_items, g->river_boat.loot, sizeof(g->floor_items));
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            item->underlying_tile = g->map.tiles[item->y][item->x];
            for (int n = 0; n < i; n++) {
                const FloorItem *earlier = &g->floor_items[n];
                if (earlier->active && earlier->x == item->x && earlier->y == item->y) {
                    item->underlying_tile = earlier->underlying_tile;
                    break;
                }
            }
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
    g->player.last_dx = 0;
    g->player.last_dy = from_town ? 1 : -1;
    push_message(g, "River passage: Saltwind south, Stillbury north. Board and dock with your Interact key.");
}

int eastern_river_travel(GameState *g, int x, int y) {
    if (g->location != LOCATION_RIVER_CROSSING || g->river_boat.aboard) {
        return 0;
    }
    if (x == RIVER_NORTH_X && y == 0) {
        town_road_enter(g, LOCATION_TOWN2, TOWN_SIDE_SOUTH, 0);
        return 1;
    }
    if (x == RIVER_SOUTH_X && y == MANGROVE_H - 1) {
        mangrove_store(g);
        mangrove_return_from_river(g);
        return 1;
    }
    return 0;
}

void eastern_river_docked(GameState *g) {
    if (g->location == LOCATION_RIVER_CROSSING && g->player.y == RIVER_NORTH_Y && g->saltwind_quest_state != 3) {
        g->saltwind_quest_state = 3;
        push_message(g, "Passage completed: Stillbury dock. Walk north and enter town to begin Act 4.");
    }
}

static int royal_roster_valid(const Map *map, const Enemy *enemies, int count) {
    for (int i = 0; i < count; i++) {
        const Enemy *e = &enemies[i];
        if ((e->type != ENEMY_ROYAL_GUARD && e->type != ENEMY_ROAD_ARCHER) ||
            e->x < 0 || e->x >= TOWN_W || e->y < 0 || e->y >= TOWN_H ||
            (e->active && (!map_is_walkable(map, e->x, e->y) || e->hp <= 0 || e->hp > e->max_hp))) {
            return 0;
        }
    }
    return 1;
}

void eastern_migrate(GameState *g) {
    if ((g->location == LOCATION_TOWN2 || g->location == LOCATION_DAWNMERE) &&
        (g->player.x == 0 || g->player.x == TOWN_W - 1 || g->player.y == 0 || g->player.y == TOWN_H - 1)) {
        g->player.x = g->player.x == 0 ? 1 : g->player.x == TOWN_W - 1 ? TOWN_W - 2 : g->player.x;
        g->player.y = g->player.y == 0 ? 1 : g->player.y == TOWN_H - 1 ? TOWN_H - 2 : g->player.y;
    }
    if (g->location == LOCATION_DAWNMERE) {
        // Preserve the active town's exploration and loot while introducing its garrison.
        eastern_enter_town(g);
    }
    if (g->location == LOCATION_ALEHOUSE) {
        eastern_refresh(g);
    }
    if (g->location == LOCATION_TOWN2) {
        eastern_refresh(g);
        eastern_enter_town(g);
    }
    if (g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_DAWNMERE) {
        // The former southern waterfront now departs toward the river to the north.
        town_road_migrate(g);
    }
    if (g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN2) {
        town_road_enter(g, LOCATION_TOWN2, TOWN_SIDE_SOUTH, 1);
    }
}

int eastern_valid(const GameState *g) {
    if (g->dawnmere_cache.enemy_count < 0 || g->dawnmere_cache.enemy_count > DAWNMERE_GARRISON ||
        (g->stillbury_guards_initialized && g->stillbury_challenge != 2) ||
        (!g->stillbury_guards_initialized && g->stillbury_challenge == 2)) {
        return 0;
    }
    if (g->stillbury_guards_initialized) {
        Map town;
        int x;
        int y;
        map_generate_town2(&town, &x, &y);
        if (!royal_roster_valid(&town, g->stillbury_guards, 3)) {
            return 0;
        }
    }
    const MangroveState *river = &g->river_boat;
    if (river->initialized < 0 || river->initialized > 1 || river->aboard < 0 || river->aboard > 1 ||
        river->enemy_count || river->searched || river->loot_count < 0 || river->loot_count > MAX_FLOOR_ITEMS) {
        return 0;
    }
    if (river->initialized) {
        Map terrain;
        eastern_river_map(&terrain);
        if (river->stage != 1 || river->boat_x < 1 || river->boat_x >= MANGROVE_W - 1 ||
            river->boat_y <= RIVER_NORTH_Y || river->boat_y >= RIVER_SOUTH_Y ||
            abs(river->boat_dx) + abs(river->boat_dy) != 1) {
            return 0;
        }
        for (int i = 0; i < river->loot_count; i++) {
            const FloorItem *item = &river->loot[i];
            if (item->active && (item->x < 1 || item->x >= MANGROVE_W - 1 || item->y < 0 || item->y >= MANGROVE_H ||
                !map_is_walkable(&terrain, item->x, item->y))) {
                return 0;
            }
        }
    } else if (river->aboard || river->loot_count) {
        return 0;
    }
    if (g->dawnmere_cache.valid && !royal_roster_valid(&g->dawnmere_cache.map, g->dawnmere_cache.enemies, g->dawnmere_cache.enemy_count)) {
        return 0;
    }
    if (g->location == LOCATION_RIVER_CROSSING && (!river->initialized || g->level != 1 ||
        g->player.x < 1 || g->player.x >= MANGROVE_W - 1 || g->player.y < 0 || g->player.y >= MANGROVE_H ||
        (river->aboard ? g->player.x != river->boat_x || g->player.y != river->boat_y : !map_is_walkable(&g->map, g->player.x, g->player.y)))) {
        return 0;
    }
    return g->dawnmere_liberated >= 0 && g->dawnmere_liberated <= 1 &&
        g->dawnmere_alerted >= 0 && g->dawnmere_alerted <= 1 && g->dawnmere_progress >= 0 && g->dawnmere_progress <= 7 &&
        (g->dawnmere_quest_state == 0 || g->dawnmere_quest_state == 1 || g->dawnmere_quest_state == 3) &&
        (g->saltwind_quest_state == 0 || g->saltwind_quest_state == 1 || g->saltwind_quest_state == 3) &&
        g->royal_reported_actions >= 0 && g->royal_reported_actions <= 3 &&
        g->act4_arrived >= 0 && g->act4_arrived <= 1 &&
        (g->stillbury_challenge == 0 || g->stillbury_challenge == 2) &&
        g->stillbury_prompt >= 0 && g->stillbury_prompt <= 1 &&
        g->stillbury_guards_initialized >= 0 && g->stillbury_guards_initialized <= 1 &&
        g->dawnmere_cache.valid >= 0 && g->dawnmere_cache.valid <= 1 &&
        g->dawnmere_loot_count >= 0 && g->dawnmere_loot_count <= MAX_FLOOR_ITEMS &&
        g->dawnmere_cache.enemy_count >= 0 && g->dawnmere_cache.enemy_count <= DAWNMERE_GARRISON;
}
