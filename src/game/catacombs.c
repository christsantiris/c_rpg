#include "catacombs.h"
#include "grave_marshal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void carve_path(Map *m, int x, int y, int tx, int ty) {
    // Random elbows keep passages from lining up into a grid.
    int horizontal_first = rand() % 2;
    for (int leg = 0; leg < 2; leg++) {
        if ((leg == 0) == horizontal_first) {
            while (x != tx) {
                m->tiles[y][x] = TILE_CATACOMBS_FLOOR;
                x += tx > x ? 1 : -1;
            }
        } else {
            while (y != ty) {
                m->tiles[y][x] = TILE_CATACOMBS_FLOOR;
                y += ty > y ? 1 : -1;
            }
        }
    }
    m->tiles[y][x] = TILE_CATACOMBS_FLOOR;
}

static int room_distance(const Room *a, const Room *b) {
    int ax, ay, bx, by;
    map_room_center(a, &ax, &ay);
    map_room_center(b, &bx, &by);
    return abs(ax - bx) + abs(ay - by);
}

static int place_rooms(Map *m) {
    // Burial chambers scatter through the crypt; the final tomb lies far from the stairs down.
    m->room_count = 0;
    for (int attempt = 0; attempt < 2000 && m->room_count < 8; attempt++) {
        Room room = {0, 0, 14 + rand() % 5, 10 + rand() % 3};
        room.x = 2 + rand() % (CATACOMBS_W - room.w - 3);
        room.y = 2 + rand() % (CATACOMBS_H - room.h - 3);
        int clear = 1;
        for (int i = 0; i < m->room_count && clear; i++) {
            const Room *other = &m->rooms[i];
            clear = room.x + room.w + 3 < other->x || other->x + other->w + 3 < room.x ||
                room.y + room.h + 3 < other->y || other->y + other->h + 3 < room.y;
        }
        if (clear && m->room_count == 7 && room_distance(&room, &m->rooms[0]) < 45) {
            clear = 0;
        }
        if (clear) {
            m->rooms[m->room_count++] = room;
        }
    }
    return m->room_count == 8;
}

static int spur_open(const Map *m, int x, int y, int from_x, int from_y) {
    // A dead end may only bore through solid rock, never into another space.
    if (x < 2 || y < 2 || x >= CATACOMBS_W - 2 || y >= CATACOMBS_H - 2 || m->tiles[y][x] != TILE_CATACOMBS_WALL) {
        return 0;
    }
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    for (int d = 0; d < 4; d++) {
        int nx = x + dx[d];
        int ny = y + dy[d];
        if (!(nx == from_x && ny == from_y) && m->tiles[ny][nx] != TILE_CATACOMBS_WALL) {
            return 0;
        }
    }
    return 1;
}

static void carve_dead_ends(Map *m) {
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    static int corridor[CATACOMBS_W * CATACOMBS_H];
    int count = 0;
    for (int y = 2; y < CATACOMBS_H - 2; y++) {
        for (int x = 2; x < CATACOMBS_W - 2; x++) {
            if (m->tiles[y][x] == TILE_CATACOMBS_FLOOR && catacombs_room_at(m, x, y) < 0) {
                corridor[count++] = y * CATACOMBS_W + x;
            }
        }
    }
    int wanted = 4 + rand() % 3;
    for (int attempt = 0; attempt < 300 && wanted > 0 && count > 0; attempt++) {
        int origin = corridor[rand() % count];
        int px = origin % CATACOMBS_W;
        int py = origin / CATACOMBS_W;
        // Bore a blind passage with one bend through solid rock, stopping where it would break through.
        int path_x[16];
        int path_y[16];
        int length = 0;
        int d = rand() % 4;
        int first = 3 + rand() % 4;
        int total = first + rand() % 5;
        for (int step = 0; step < total; step++) {
            if (step == first) {
                d = (d + (rand() % 2 ? 1 : 3)) % 4;
            }
            int nx = px + dx[d];
            int ny = py + dy[d];
            int ok = spur_open(m, nx, ny, px, py);
            for (int i = 0; i + 1 < length && ok; i++) {
                ok = abs(path_x[i] - nx) + abs(path_y[i] - ny) > 1;
            }
            if (!ok) {
                break;
            }
            path_x[length] = nx;
            path_y[length] = ny;
            length++;
            px = nx;
            py = ny;
        }
        if (length < 3) {
            continue;
        }
        for (int i = 0; i < length; i++) {
            m->tiles[path_y[i]][path_x[i]] = TILE_CATACOMBS_FLOOR;
        }
        wanted--;
    }
}

static int floor_connected(const Map *m) {
    // Every floor tile, both stairways and every chamber must join the stairs up.
    static unsigned char seen[CATACOMBS_H][CATACOMBS_W];
    static int queue[CATACOMBS_W * CATACOMBS_H];
    memset(seen, 0, sizeof(seen));
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    int head = 0;
    int tail = 0;
    seen[m->stairs_up_y][m->stairs_up_x] = 1;
    queue[tail++] = m->stairs_up_y * CATACOMBS_W + m->stairs_up_x;
    while (head < tail) {
        int x = queue[head] % CATACOMBS_W;
        int y = queue[head++] / CATACOMBS_W;
        for (int d = 0; d < 4; d++) {
            int nx = x + dx[d];
            int ny = y + dy[d];
            if (nx >= 0 && ny >= 0 && nx < CATACOMBS_W && ny < CATACOMBS_H && !seen[ny][nx] && map_is_walkable(m, nx, ny)) {
                seen[ny][nx] = 1;
                queue[tail++] = ny * CATACOMBS_W + nx;
            }
        }
    }
    for (int y = 0; y < CATACOMBS_H; y++) {
        for (int x = 0; x < CATACOMBS_W; x++) {
            if (map_is_walkable(m, x, y) && !seen[y][x]) {
                return 0;
            }
        }
    }
    return 1;
}

static void generate_layout(Map *m, int level) {
    memset(m, 0, sizeof(*m));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_CATACOMBS_WALL;
        }
    }
    if (!place_rooms(m)) {
        return;
    }
    for (int i = 0; i < m->room_count; i++) {
        Room *room = &m->rooms[i];
        for (int y = room->y; y < room->y + room->h; y++) {
            for (int x = room->x; x < room->x + room->w; x++) {
                m->tiles[y][x] = TILE_CATACOMBS_FLOOR;
            }
        }
    }
    // Each chamber joins its nearest earlier chamber, so outer chambers become dead ends;
    // one extra passage keeps a single loop through the crypt.
    for (int i = 1; i < m->room_count; i++) {
        int nearest = 0;
        for (int j = 1; j < i; j++) {
            if (room_distance(&m->rooms[i], &m->rooms[j]) < room_distance(&m->rooms[i], &m->rooms[nearest])) {
                nearest = j;
            }
        }
        int x, y, tx, ty;
        map_room_center(&m->rooms[i], &x, &y);
        map_room_center(&m->rooms[nearest], &tx, &ty);
        carve_path(m, x, y, tx, ty);
    }
    int a = rand() % m->room_count;
    int b = (a + 2 + rand() % (m->room_count - 3)) % m->room_count;
    int x, y, tx, ty;
    map_room_center(&m->rooms[a], &x, &y);
    map_room_center(&m->rooms[b], &tx, &ty);
    carve_path(m, x, y, tx, ty);
    carve_dead_ends(m);
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

void map_generate_catacombs(Map *m, int level) {
    do {
        generate_layout(m, level);
    } while (m->room_count != 8 || !floor_connected(m));
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

static int keeps_unique_state(const GameState *g, int level) {
    // A memorial silenced before Oswin's quest earns credit from the saved floor, and a
    // wounded Grave Marshal keeps his fight; both floors stay as saved.
    const LevelCache *cache = &g->catacombs_cache[level - 1];
    if (!g->catacombs_quest_state && level >= 2 && level <= 4 && memorial_is_cold(&cache->map)) {
        return 1;
    }
    for (int i = 0; i < cache->enemy_count; i++) {
        const Enemy *e = &cache->enemies[i];
        if (e->active && e->type == ENEMY_GRAVE_MARSHAL && e->hp < e->max_hp) {
            return 1;
        }
    }
    return 0;
}

void catacombs_regenerate_saved_floors(GameState *g) {
    // Floors saved on the old grid regenerate on the next visit. Memorials, the ledger,
    // the Grave Marshal and his reward are restored from state kept outside the map.
    for (int level = 1; level <= CATACOMBS_DEPTH; level++) {
        int here = g->location == LOCATION_CATACOMBS && g->level == level;
        if (!here && g->catacombs_cache[level - 1].valid && !keeps_unique_state(g, level)) {
            g->catacombs_cache[level - 1].valid = 0;
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
            "Take Crown Road West from Ridgeshire, then the castle's south gate. Enter the roadside crypt. Press A at memorials on floors 2-4; take the Grave Marshal's ledger on floor 5.");
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
    grave_marshal_interrupt(g, x, y);
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

int catacombs_conjurer_raise(GameState *g, const Enemy *caster) {
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
            push_message(g, "The Crypt Conjurer calls to rattling bones. Silence the brazier!");
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
    return e->boss_attack != MARSHAL_GUARDS && grave_marshal_attack_marks(m, e, x, y);
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
    grave_marshal_restore_reward(g);
}
