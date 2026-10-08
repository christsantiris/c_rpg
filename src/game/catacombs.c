#include "catacombs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void carve_path(Map *m, int x, int y, int tx, int ty) {
    while (x != tx) {
        m->tiles[y][x] = TILE_CATACOMBS_FLOOR;
        x += tx > x ? 1 : -1;
    }
    while (y != ty) {
        m->tiles[y][x] = TILE_CATACOMBS_FLOOR;
        y += ty > y ? 1 : -1;
    }
    m->tiles[y][x] = TILE_CATACOMBS_FLOOR;
}

void map_generate_catacombs(Map *m, int level) {
    memset(m, 0, sizeof(*m));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_CATACOMBS_WALL;
        }
    }
    m->room_count = 8;
    for (int i = 0; i < m->room_count; i++) {
        Room *room = &m->rooms[i];
        *room = (Room){6 + (i % 2) * 27, 5 + (i / 2) * 18, 14 + rand() % 5, 10 + rand() % 3};
        for (int y = room->y; y < room->y + room->h; y++) {
            for (int x = room->x; x < room->x + room->w; x++) {
                m->tiles[y][x] = TILE_CATACOMBS_FLOOR;
            }
        }
    }
    // Cross passages and two vertical routes form loops around burial chambers.
    for (int i = 0; i < m->room_count; i++) {
        int x;
        int y;
        map_room_center(&m->rooms[i], &x, &y);
        if (i % 2 == 0) {
            int tx;
            int ty;
            map_room_center(&m->rooms[i + 1], &tx, &ty);
            carve_path(m, x, y, tx, ty);
        }
        if (i + 2 < m->room_count) {
            int tx;
            int ty;
            map_room_center(&m->rooms[i + 2], &tx, &ty);
            carve_path(m, x, y, tx, ty);
        }
    }
    map_room_center(&m->rooms[0], &m->stairs_up_x, &m->stairs_up_y);
    Room *last = &m->rooms[m->room_count - 1];
    m->stairs_down_x = last->x + last->w / 2;
    m->stairs_down_y = last->y + last->h - 2;
    m->tiles[m->stairs_up_y][m->stairs_up_x] = TILE_STAIRS_UP;
    m->tiles[m->stairs_down_y][m->stairs_down_x] = level == CATACOMBS_DEPTH ? TILE_RETURN_EXIT : TILE_STAIRS_DOWN;
    for (int i = 1; i < m->room_count; i++) {
        Room *room = &m->rooms[i];
        m->tiles[room->y + 1][room->x + 1] = TILE_OSSUARY_BRAZIER;
        m->tiles[room->y + 1][room->x + room->w - 2] = TILE_CATACOMBS_SARCOPHAGUS;
        m->tiles[room->y + room->h - 2][room->x + 1] = TILE_CATACOMBS_SARCOPHAGUS;
        if (i == m->room_count - 1 && level == CATACOMBS_DEPTH) {
            m->tiles[room->y + 1][room->x + room->w - 2] = TILE_OSSUARY_BRAZIER;
            continue;
        }
        BurialTrap *trap = &m->burial_traps[m->burial_trap_count++];
        *trap = (BurialTrap){room->x + room->w / 2, room->y + room->h / 2 + 2, i % 2, 0, 0};
        m->tiles[trap->y][trap->x] = TILE_BURIAL_PLATE;
    }
}

int catacombs_room_at(const Map *m, int x, int y) {
    for (int i = 0; i < m->room_count; i++) {
        const Room *room = &m->rooms[i];
        if (x >= room->x && x < room->x + room->w && y >= room->y && y < room->y + room->h) {
            return i;
        }
    }
    return -1;
}

int catacombs_lit_braziers(const Map *m, int room) {
    if (room < 0 || room >= m->room_count) {
        return 0;
    }
    int count = 0;
    const Room *r = &m->rooms[room];
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            count += m->tiles[y][x] == TILE_OSSUARY_BRAZIER || m->tiles[y][x] == TILE_MEMORIAL_BRAZIER;
        }
    }
    return count;
}

static int adjacent_brazier(const GameState *g, int *tx, int *ty) {
    if (g->location != LOCATION_CATACOMBS) {
        return 0;
    }
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    for (int i = 0; i < 4; i++) {
        int x = g->player.x + dx[i];
        int y = g->player.y + dy[i];
        if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
            (g->map.tiles[y][x] == TILE_OSSUARY_BRAZIER || g->map.tiles[y][x] == TILE_MEMORIAL_BRAZIER)) {
            *tx = x;
            *ty = y;
            return 1;
        }
    }
    return 0;
}

static int adjacent_ledger(const GameState *g, int *tx, int *ty) {
    if (g->location != LOCATION_CATACOMBS || g->catacombs_quest_state != 1 || g->level != CATACOMBS_DEPTH ||
        (g->catacombs_quest_progress & CATACOMBS_LEDGER_RECOVERED)) {
        return 0;
    }
    for (int y = g->player.y - 1; y <= g->player.y + 1; y++) {
        for (int x = g->player.x - 1; x <= g->player.x + 1; x++) {
            if (abs(x - g->player.x) + abs(y - g->player.y) == 1 && x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
                g->map.tiles[y][x] == TILE_BURIAL_LEDGER) {
                *tx = x;
                *ty = y;
                return 1;
            }
        }
    }
    return 0;
}

static void quest_ready(GameState *g) {
    if (g->catacombs_quest_state == 1 && g->catacombs_quest_progress == CATACOMBS_QUEST_COMPLETE) {
        g->catacombs_quest_state = 2;
        push_message(g, "The dead rest. Return to Brother Oswin in Ridgeshire's Town Hall.");
    }
}

static int memorial_is_cold(const Map *m) {
    if (m->room_count <= CATACOMBS_MEMORIAL_ROOM) {
        return 0;
    }
    const Room *room = &m->rooms[CATACOMBS_MEMORIAL_ROOM];
    TileType tile = m->tiles[room->y + 1][room->x + 1];
    return tile == TILE_OSSUARY_COLD || tile == TILE_MEMORIAL_COLD;
}

void catacombs_refresh_quest(GameState *g) {
    if (g->location != LOCATION_CATACOMBS || !g->catacombs_quest_state || g->map.room_count <= CATACOMBS_MEMORIAL_ROOM) {
        return;
    }
    if (g->level >= 2 && g->level <= 4) {
        int bit = 1 << (g->level - 2);
        if (memorial_is_cold(&g->map)) {
            g->catacombs_quest_progress |= bit;
        }
        const Room *room = &g->map.rooms[CATACOMBS_MEMORIAL_ROOM];
        g->map.tiles[room->y + 1][room->x + 1] = g->catacombs_quest_progress & bit ? TILE_MEMORIAL_COLD : TILE_MEMORIAL_BRAZIER;
    } else if (g->level == CATACOMBS_DEPTH) {
        const Room *room = &g->map.rooms[g->map.room_count - 1];
        g->map.tiles[room->y + room->h - 2][room->x + 1] =
            g->catacombs_quest_progress & CATACOMBS_LEDGER_RECOVERED ? TILE_CATACOMBS_SARCOPHAGUS : TILE_BURIAL_LEDGER;
    }
    quest_ready(g);
}

void catacombs_accept_quest(GameState *g) {
    g->catacombs_quest_state = 1;
    for (int level = 2; level <= 4; level++) {
        const LevelCache *cache = &g->catacombs_cache[level - 1];
        if (cache->valid && memorial_is_cold(&cache->map)) {
            g->catacombs_quest_progress |= 1 << (level - 2);
        }
    }
}

void game_talk_to_oswin(GameState *g) {
    if (g->location != LOCATION_TOWN_HALL || abs(g->player.x - HALL_OSWIN_X) > 1 || abs(g->player.y - HALL_OSWIN_Y) > 1) {
        return;
    }
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Brother Oswin");
    g->dialogue_x = HALL_OSWIN_X;
    g->dialogue_y = HALL_OSWIN_Y;
    if (g->catacombs_quest_state == 0) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "The dead need names and rest. Take Crown Road West from Ridgeshire's west gate, then the castle's south gate. Press A at memorials on floors 2-4; take the Grave Marshal's ledger on floor 5.");
    } else if (g->catacombs_quest_state == 1) {
        int silenced = 0;
        for (int i = 0; i < 3; i++) {
            silenced += !!(g->catacombs_quest_progress & (1 << i));
        }
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "%d of 3 memorials are silent; the ledger is %s. Press A beside a memorial to stop resurrection, even while its guards live. Bring the royal dead's names back here.",
            silenced, g->catacombs_quest_progress & CATACOMBS_LEDGER_RECOVERED ? "recovered" : "still in the royal tomb");
    } else if (g->catacombs_quest_state == 2) {
        g->catacombs_quest_state = 3;
        g->gold += CATACOMBS_REWARD_GOLD;
        g->score += CATACOMBS_REWARD_SCORE;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "Their families can finally learn where they rest. The memorial flames are silent. Take 150 gold with the Town Hall's thanks.");
        push_message(g, "Completed: Rest for the Forgotten. 150 gold and 1500 score awarded.");
    } else {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN, "The forgotten have their names again. May their graves remain quiet.");
    }
}

int catacombs_has_interaction(const GameState *g) {
    int x;
    int y;
    return adjacent_brazier(g, &x, &y) || adjacent_ledger(g, &x, &y);
}

int catacombs_interact(GameState *g) {
    int x;
    int y;
    if (adjacent_ledger(g, &x, &y)) {
        if (!(g->defeated_bosses & (1 << LOCATION_CATACOMBS))) {
            push_message(g, "The Grave Marshal's seal binds the ledger. Defeat him first.");
            return 1;
        }
        g->catacombs_quest_progress |= CATACOMBS_LEDGER_RECOVERED;
        g->map.tiles[y][x] = TILE_CATACOMBS_SARCOPHAGUS;
        push_message(g, "Royal burial ledger recovered. The fallen have names again.");
        quest_ready(g);
        return 1;
    }
    if (!adjacent_brazier(g, &x, &y)) {
        return 0;
    }
    int memorial = g->map.tiles[y][x] == TILE_MEMORIAL_BRAZIER;
    g->map.tiles[y][x] = memorial ? TILE_MEMORIAL_COLD : TILE_OSSUARY_COLD;
    if (memorial && g->catacombs_quest_state == 1 && g->level >= 2 && g->level <= 4) {
        g->catacombs_quest_progress |= 1 << (g->level - 2);
        push_message(g, "Memorial silenced. Its bound bones cannot rise again.");
        quest_ready(g);
    } else {
        push_message(g, "The ossuary flame dies. Its bound bones fall silent.");
    }
    return 1;
}

void catacombs_record_death(GameState *g, Enemy *e) {
    if (g->location == LOCATION_CATACOMBS && e->type == ENEMY_ANCIENT_SKELETON && !e->revived &&
        catacombs_lit_braziers(&g->map, catacombs_room_at(&g->map, e->x, e->y))) {
        e->revive_timer = 2;
        g->level_cleared = 0;
        push_message(g, "Ancient bones rattle! Extinguish their brazier to prevent revival.");
    }
}

int catacombs_cantor_raise(GameState *g, const Enemy *caster) {
    int room = catacombs_room_at(&g->map, caster->x, caster->y);
    if (!catacombs_lit_braziers(&g->map, room)) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *dead = &g->enemies[i];
        if (!dead->active && dead->hp <= 0 && !dead->revived && dead->revive_timer == 0 &&
            (dead->type == ENEMY_ANCIENT_SKELETON || dead->type == ENEMY_BONE_SENTINEL) &&
            catacombs_room_at(&g->map, dead->x, dead->y) == room) {
            dead->revive_timer = 2;
            push_message(g, "The Bone Cantor calls to rattling bones. Silence the brazier!");
            return 1;
        }
    }
    return 0;
}

int catacombs_trap_marks(const Map *m, const BurialTrap *trap, int x, int y) {
    if ((trap->vertical && x != trap->x) || (!trap->vertical && y != trap->y)) {
        return 0;
    }
    int dx = (x > trap->x) - (x < trap->x);
    int dy = (y > trap->y) - (y < trap->y);
    int distance = abs(x - trap->x) + abs(y - trap->y);
    if (distance > 4) {
        return 0;
    }
    for (int step = 0; step <= distance; step++) {
        int tx = trap->x + dx * step;
        int ty = trap->y + dy * step;
        if (!map_is_walkable(m, tx, ty) || m->tiles[ty][tx] == TILE_STAIRS_UP ||
            m->tiles[ty][tx] == TILE_STAIRS_DOWN || m->tiles[ty][tx] == TILE_RETURN_EXIT || m->tiles[ty][tx] == TILE_PORTAL) {
            return 0;
        }
    }
    return 1;
}

int catacombs_tick(GameState *g) {
    if (g->location != LOCATION_CATACOMBS) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->active || e->revive_timer <= 0) {
            continue;
        }
        if (!catacombs_lit_braziers(&g->map, catacombs_room_at(&g->map, e->x, e->y))) {
            e->revive_timer = 0;
            continue;
        }
        e->revive_timer--;
        if (e->revive_timer > 0) {
            continue;
        }
        int occupied = g->player.x == e->x && g->player.y == e->y;
        for (int j = 0; j < g->enemy_count; j++) {
            occupied |= g->enemies[j].active && g->enemies[j].x == e->x && g->enemies[j].y == e->y;
        }
        if (occupied || !map_is_walkable(&g->map, e->x, e->y)) {
            e->revive_timer = 1;
            continue;
        }
        e->revived = 1;
        e->active = 1;
        e->hp = e->max_hp;
        e->experience = 0;
        // The next phase consumes this pause; newly risen enemies cannot attack immediately.
        e->frozen_turns = 1;
        push_message(g, "Bound bones reassemble. They cannot rise a second time.");
        g->level_cleared = 0;
    }
    int damage = 0;
    for (int i = 0; i < g->map.burial_trap_count; i++) {
        BurialTrap *trap = &g->map.burial_traps[i];
        if (trap->spent) {
            continue;
        }
        if (trap->timer > 0) {
            trap->timer--;
            if (trap->timer == 0) {
                trap->spent = 1;
                if (catacombs_trap_marks(&g->map, trap, g->player.x, g->player.y)) {
                    damage += 12 + g->level * 2;
                }
                push_message(g, "Burial spikes erupt along the marked stones!");
            }
        } else if (g->player.x == trap->x && g->player.y == trap->y) {
            trap->timer = 1;
            push_message(g, "A burial plate clicks. Leave the glowing spike line!");
        }
    }
    game_update_level_progress(g);
    return damage;
}

int catacombs_sweep_marks(const Map *m, const Enemy *e, int x, int y) {
    if (e->attack_target_x < 0 || e->attack_target_y < 0 || !map_is_walkable(m, x, y)) {
        return 0;
    }
    int dx = e->attack_target_x - e->x;
    int dy = e->attack_target_y - e->y;
    int rx = x - e->x;
    int ry = y - e->y;
    return dx != 0 ? rx == dx && abs(ry) <= 1 : ry == dy && abs(rx) <= 1;
}

int catacombs_enemy_damage(const GameState *g, const Enemy *e, int damage) {
    if (g->location == LOCATION_CATACOMBS && e->type == ENEMY_GRAVE_MARSHAL) {
        int flames = catacombs_lit_braziers(&g->map, g->map.room_count - 1);
        int percent = flames >= 2 ? 50 : (flames == 1 ? 75 : 100);
        damage = (damage * percent + 99) / 100;
    }
    return damage;
}

void catacombs_restore_reward(GameState *g) {
    if (g->location != LOCATION_CATACOMBS || g->level != CATACOMBS_DEPTH || !g->catacombs_mantle_unclaimed) {
        return;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].active && strcmp(g->floor_items[i].item.name, "Gravekeeper's Mantle") == 0) {
            return;
        }
    }
    if (g->floor_item_count >= MAX_FLOOR_ITEMS) {
        return;
    }
    int x;
    int y;
    map_room_center(&g->map.rooms[g->map.room_count - 1], &x, &y);
    FloorItem *fi = &g->floor_items[g->floor_item_count++];
    *fi = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_CATACOMBS_FLOOR, .item = item_make_gravekeeper_mantle()};
    g->map.tiles[y][x] = TILE_ITEM;
}
