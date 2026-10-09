#include "castle.h"
#include "combat_feedback.h"
#include <stdlib.h>
#include <string.h>

static const int dxs[4] = {0, 1, 0, -1};
static const int dys[4] = {-1, 0, 1, 0};
static int line_clear(const Map *m, int x, int y, int tx, int ty);

static TileType tile_at(const GameState *g, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
        return TILE_CASTLE_WALL;
    }
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            const FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == x && item->y == y) {
                return item->underlying_tile;
            }
        }
    }
    return tile;
}

static TileType underfoot(const GameState *g) {
    return tile_at(g, g->player.x, g->player.y);
}

static void set_tile(GameState *g, int x, int y, TileType tile) {
    if (g->map.tiles[y][x] != TILE_ITEM) {
        g->map.tiles[y][x] = tile;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = tile;
        }
    }
}

const char *castle_floor_name(int level) {
    static const char *names[CASTLE_DEPTH] = {"Gatehouse", "Iron Keep", "Forsaken Court", "Crown Chapel", "Royal Archives", "Throne of No Return"};
    return level >= 1 && level <= CASTLE_DEPTH ? names[level - 1] : "Castle";
}

static void path(Map *m, int x, int y, int tx, int ty) {
    while (x != tx || y != ty) {
        for (int oy = -1; oy <= 1; oy++) {
            for (int ox = -1; ox <= 1; ox++) {
                m->tiles[y + oy][x + ox] = TILE_CASTLE_CARPET;
            }
        }
        if (x != tx) {
            x += tx > x ? 1 : -1;
        } else {
            y += ty > y ? 1 : -1;
        }
    }
}

static TileType encounter_tile(int level, int x, int y, TileType tile) {
    if (tile != TILE_CASTLE_FLOOR && tile != TILE_CASTLE_CARPET) {
        return tile;
    }
    if (level == 1 || level == 6) {
        if (y == 21 && x >= 14 && x <= 16) {
            return TILE_CASTLE_GATE_OPEN;
        }
        if (x == 10 && y == 15) {
            return TILE_CASTLE_LEVER;
        }
    }
    if ((level == 4 || level == 6) && y == 54 && (x == 37 || x == 53)) {
        return TILE_CASTLE_WARD;
    }
    if ((level == 3 && x == 53 && y == 35) || (level == 6 && x == 53 && y == 46)) {
        return TILE_CASTLE_MUSTER;
    }
    if (level == 6 && y == 51 && x >= 41 && x <= 49) {
        return TILE_CASTLE_FIRE_RUNE;
    }
    if (level == 5) {
        for (int i = 1; i < 5; i++) {
            int rx = 4 + (i % 2) * 30;
            int ry = 4 + (i / 2) * 20;
            if (y == ry + 10 && x >= rx + 7 && x <= rx + 15) {
                return TILE_CASTLE_FIRE_RUNE;
            }
            if (tile == TILE_CASTLE_FLOOR && (x == rx + 6 || x == rx + 16) &&
                y >= ry + 2 && y <= ry + 11 && (y < ry + 6 || y == ry + 9 || y == ry + 11)) {
                return TILE_CASTLE_BOOKCASE;
            }
        }
    }
    return tile;
}

void castle_generate(Map *m, int level) {
    memset(m, 0, sizeof(*m));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_CASTLE_WALL;
        }
    }
    m->room_count = 6;
    for (int i = 0; i < 6; i++) {
        Room *r = &m->rooms[i];
        *r = (Room){4 + (i % 2) * 30, 4 + (i / 2) * 20, 22, 14};
        for (int y = r->y; y < r->y + r->h; y++) {
            for (int x = r->x; x < r->x + r->w; x++) {
                m->tiles[y][x] = x >= r->x + 9 && x <= r->x + 12 ? TILE_CASTLE_CARPET : TILE_CASTLE_FLOOR;
            }
        }
    }
    for (int i = 0; i < 6; i++) {
        int x;
        int y;
        map_room_center(&m->rooms[i], &x, &y);
        if (i % 2 == 0) {
            path(m, x, y, x + 30, y);
        }
        if (i < 4) {
            path(m, x, y, x, y + 20);
        }
    }
    for (int i = 0; i < 6; i++) {
        Room *r = &m->rooms[i];
        m->tiles[r->y + 1][r->x + 2] = TILE_CASTLE_BANNER;
        m->tiles[r->y + 1][r->x + r->w - 3] = TILE_CASTLE_BANNER;
        m->tiles[r->y + 4][r->x + 4] = TILE_CASTLE_PILLAR;
        m->tiles[r->y + 4][r->x + r->w - 5] = TILE_CASTLE_PILLAR;
        if (level == 3 || level == 5) {
            m->tiles[r->y + 8][r->x + 4] = TILE_CASTLE_PILLAR;
        }
        if (level == 3 || level == 4) {
            for (int y = r->y + 6; y <= r->y + 8; y++) {
                if (m->tiles[y][r->x + 5] != TILE_CASTLE_CARPET) {
                    m->tiles[y][r->x + 5] = TILE_CASTLE_TABLE;
                }
                if (m->tiles[y][r->x + r->w - 6] != TILE_CASTLE_CARPET) {
                    m->tiles[y][r->x + r->w - 6] = TILE_CASTLE_TABLE;
                }
            }
        } else if (level == 5) {
            for (int x = r->x + 5; x < r->x + r->w - 5; x += 3) {
                m->tiles[r->y + 1][x] = TILE_CASTLE_BOOKCASE;
            }
        }
    }
    map_room_center(&m->rooms[0], &m->stairs_up_x, &m->stairs_up_y);
    map_room_center(&m->rooms[5], &m->stairs_down_x, &m->stairs_down_y);
    m->stairs_down_y += 4;
    // The onward stair command climbs; the back command returns to the previous floor.
    m->tiles[m->stairs_up_y][m->stairs_up_x] = TILE_STAIRS_DOWN;
    m->tiles[m->stairs_down_y][m->stairs_down_x] = level < CASTLE_DEPTH ? TILE_STAIRS_UP : TILE_CASTLE_CARPET;
    if (level == CASTLE_DEPTH) {
        m->tiles[45][45] = TILE_CASTLE_THRONE;
    }
    m->tiles[10][29] = TILE_CASTLE_GATE;
    m->tiles[11][29] = TILE_CASTLE_GATE;
    m->tiles[12][29] = TILE_CASTLE_GATE;
    m->tiles[13][24] = TILE_CASTLE_LEVER;
    if (level == 4 || level == 6) {
        for (int y = 47; y <= 49; y++) {
            m->tiles[y][40] = TILE_CASTLE_GATE_OPEN;
            m->tiles[y][50] = TILE_CASTLE_GATE;
        }
    }
    if (level >= 2 && level <= 5) {
        m->burial_trap_count = 1;
        m->burial_traps[0] = (BurialTrap){17, 32, 0, 0, 0};
        m->tiles[32][17] = TILE_CASTLE_TRAP_HIDDEN;
    }
    for (int y = 0; y < CASTLE_H; y++) {
        for (int x = 0; x < CASTLE_W; x++) {
            m->tiles[y][x] = encounter_tile(level, x, y, m->tiles[y][x]);
        }
    }
}

static TileType repaired_tile(const Map *m, int level, int x, int y, TileType tile) {
    if (tile == TILE_CASTLE_PASSAGE) {
        return TILE_CASTLE_FLOOR;
    }
    if ((level == 3 || level == 4) && tile == TILE_CASTLE_TABLE) {
        for (int i = 0; i + 1 < m->room_count; i += 2) {
            int cx;
            int cy;
            int tx;
            int ty;
            map_room_center(&m->rooms[i], &cx, &cy);
            map_room_center(&m->rooms[i + 1], &tx, &ty);
            if (x >= cx - 1 && x <= tx && y >= cy - 1 && y <= cy + 1) {
                return TILE_CASTLE_CARPET;
            }
        }
    }
    return tile;
}

static void repair_layout(Map *m, int level, FloorItem *loot, int count) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = repaired_tile(m, level, x, y, m->tiles[y][x]);
        }
    }
    for (int i = 0; i < count; i++) {
        FloorItem *item = &loot[i];
        item->underlying_tile = repaired_tile(m, level, item->x, item->y, item->underlying_tile);
    }
}

void castle_migrate_layout(GameState *g) {
    if (g->location == LOCATION_CASTLE_INTERIOR) {
        repair_layout(&g->map, g->level, g->floor_items, g->floor_item_count);
    }
    for (int i = 0; i < CASTLE_DEPTH; i++) {
        LevelCache *cache = &g->castle_cache[i];
        if (cache->valid) {
            repair_layout(&cache->map, i + 1, g->castle_loot[i], g->castle_loot_count[i]);
        }
    }
    if (g->portal_location == LOCATION_CASTLE_INTERIOR && g->portal_origin_tile == TILE_CASTLE_PASSAGE) {
        g->portal_origin_tile = TILE_CASTLE_FLOOR;
    }
}

static int occupied(const GameState *g, int x, int y) {
    if (!map_is_walkable(&g->map, x, y) || (g->player.x == x && g->player.y == y)) {
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 1;
        }
    }
    return 0;
}

static Enemy *add_enemy(GameState *g, EnemyType type, int x, int y) {
    if (g->enemy_count >= MAX_ENEMIES || occupied(g, x, y)) {
        return NULL;
    }
    int n = type - ENEMY_OATHBOUND_SOLDIER;
    static const char *names[8] = {"Oathbound Soldier", "Iron Warden", "Royal Marksman", "Court Hexer", "Bell Herald", "Castellan", "Royal Arcanist", "Lord Veyr"};
    static const int hp[8] = {95, 150, 75, 90, 65, 420, 470, 950};
    static const int attack[8] = {24, 28, 26, 27, 18, 34, 36, 42};
    static const int defense[8] = {7, 11, 5, 6, 4, 12, 9, 14};
    Enemy *e = &g->enemies[g->enemy_count++];
    *e = (Enemy){.active = 1, .type = type, .x = x, .y = y, .max_hp = hp[n], .hp = hp[n],
        .attack = attack[n] + g->level / 2, .defense = defense[n], .experience = n >= 5 ? 1000 : 100,
        .is_boss = n >= 5, .attack_target_x = -1, .attack_target_y = -1, .facing_dy = -1};
    snprintf(e->name, sizeof(e->name), "%s", names[n]);
    g->level_cleared = 0;
    return e;
}

static void spawn_detail(GameState *g) {
    if (g->level == 1 || g->level == 6) {
        add_enemy(g, ENEMY_IRON_WARDEN, 31, 11);
        add_enemy(g, ENEMY_ROYAL_MARKSMAN, 33, 11);
        add_enemy(g, ENEMY_OATHBOUND_SOLDIER, 14, 25);
        add_enemy(g, ENEMY_ROYAL_MARKSMAN, 17, 25);
    }
    if (g->level == 2 && !(g->castle_minibosses & 1)) {
        add_enemy(g, ENEMY_IRON_WARDEN, 42, 48);
        add_enemy(g, ENEMY_ROYAL_MARKSMAN, 48, 50);
    }
    if (g->level == 3) {
        add_enemy(g, ENEMY_IRON_WARDEN, 48, 34);
        add_enemy(g, ENEMY_ROYAL_MARKSMAN, 52, 28);
    }
    if ((g->level == 4 && !(g->castle_minibosses & 2)) || g->level == 6) {
        add_enemy(g, ENEMY_IRON_WARDEN, 39, 54);
        add_enemy(g, ENEMY_ROYAL_MARKSMAN, 51, 54);
    }
    if (g->level == 5) {
        for (int i = 1; i < 5; i++) {
            const Room *room = &g->map.rooms[i];
            add_enemy(g, ENEMY_OATHBOUND_SOLDIER, room->x + 3, room->y + 10);
        }
    }
    if (g->level == 6) {
        add_enemy(g, ENEMY_BELL_HERALD, 53, 48);
    }
}

void castle_spawn(GameState *g) {
    g->enemy_count = 0;
    for (int i = 1; i < 5; i++) {
        int x;
        int y;
        map_room_center(&g->map.rooms[i], &x, &y);
        add_enemy(g, i % 2 == 0 ? ENEMY_IRON_WARDEN : ENEMY_OATHBOUND_SOLDIER, x, y - 1);
        add_enemy(g, g->level >= 3 && i % 2 == 0 ? ENEMY_COURT_HEXER : ENEMY_ROYAL_MARKSMAN, x + 2, y + 1);
    }
    if (g->level >= 3) {
        add_enemy(g, ENEMY_BELL_HERALD, 49, 32);
    }
    spawn_detail(g);
    if (g->level == 2 && !(g->castle_minibosses & 1)) {
        add_enemy(g, ENEMY_CASTELLAN, 45, 49);
    } else if (g->level == 4 && !(g->castle_minibosses & 2)) {
        add_enemy(g, ENEMY_ROYAL_ARCANIST, 45, 49);
    } else if (g->level == 6 && !g->game_won) {
        add_enemy(g, ENEMY_LORD_VEYR, 45, 49);
    }
    g->level_cleared = 0;
}

static void migrate_encounter(GameState *g) {
    for (int y = 0; y < CASTLE_H; y++) {
        for (int x = 0; x < CASTLE_W; x++) {
            TileType old = tile_at(g, x, y);
            TileType tile = encounter_tile(g->level, x, y, old);
            // Keep old occupants and loot in place when installing solid shelves.
            if (tile != old && (tile != TILE_CASTLE_BOOKCASE || (!occupied(g, x, y) && g->map.tiles[y][x] != TILE_ITEM))) {
                set_tile(g, x, y, tile);
            }
        }
    }
    if (!g->level_cleared && !g->game_won) {
        spawn_detail(g);
    }
}

int castle_migrate_encounters(GameState *g) {
    GameState *floor = calloc(1, sizeof(*floor));
    if (!floor) {
        return 0;
    }
    for (int i = 0; i < CASTLE_DEPTH; i++) {
        LevelCache *cache = &g->castle_cache[i];
        if (!cache->valid) {
            continue;
        }
        floor->map = cache->map;
        memcpy(floor->enemies, cache->enemies, sizeof(floor->enemies));
        floor->enemy_count = cache->enemy_count;
        memcpy(floor->floor_items, g->castle_loot[i], sizeof(floor->floor_items));
        floor->floor_item_count = g->castle_loot_count[i];
        floor->level = i + 1;
        floor->level_cleared = cache->level_cleared;
        floor->castle_minibosses = g->castle_minibosses;
        floor->game_won = g->game_won;
        floor->player.x = -1;
        floor->player.y = -1;
        if (g->location == LOCATION_CASTLE_INTERIOR && g->level == i + 1) {
            floor->player.x = g->player.x;
            floor->player.y = g->player.y;
        }
        migrate_encounter(floor);
        cache->map = floor->map;
        memcpy(cache->enemies, floor->enemies, sizeof(cache->enemies));
        cache->enemy_count = floor->enemy_count;
        memcpy(g->castle_loot[i], floor->floor_items, sizeof(floor->floor_items));
    }
    free(floor);
    if (g->location == LOCATION_CASTLE_INTERIOR) {
        migrate_encounter(g);
    }
    return 1;
}

void castle_store(GameState *g) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level < 1 || g->level > CASTLE_DEPTH) {
        return;
    }
    int i = g->level - 1;
    LevelCache *c = &g->castle_cache[i];
    c->map = g->map;
    memcpy(c->enemies, g->enemies, sizeof(g->enemies));
    c->enemy_count = g->enemy_count;
    c->level_cleared = g->level_cleared;
    c->valid = 1;
    memcpy(g->castle_loot[i], g->floor_items, sizeof(g->floor_items));
    g->castle_loot_count[i] = g->floor_item_count;
}

static void arrive(GameState *g, int x, int y) {
    g->player.x = x;
    g->player.y = y;
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (!e->active) {
            continue;
        }
        if (e->x == x && e->y == y) {
            int moved = 0;
            for (int radius = 1; radius <= 6 && !moved; radius++) {
                for (int ty = y - radius; ty <= y + radius && !moved; ty++) {
                    for (int tx = x - radius; tx <= x + radius; tx++) {
                        if (!occupied(g, tx, ty)) {
                            e->x = tx;
                            e->y = ty;
                            moved = 1;
                            break;
                        }
                    }
                }
            }
        }
        if (castle_attack_marks(&g->map, e, x, y)) {
            e->attack_target_x = -1;
            e->attack_target_y = -1;
            e->move_timer = 2;
        }
    }
}

void castle_enter(GameState *g, int level) {
    if (level < 1 || level > CASTLE_DEPTH || g->game_won) {
        return;
    }
    castle_store(g);
    g->location = LOCATION_CASTLE_INTERIOR;
    g->level = level;
    g->player.x = 15;
    g->player.y = 11;
    LevelCache *c = &g->castle_cache[level - 1];
    if (c->valid) {
        g->map = c->map;
        memcpy(g->enemies, c->enemies, sizeof(g->enemies));
        g->enemy_count = c->enemy_count;
        g->level_cleared = c->level_cleared;
    } else {
        castle_generate(&g->map, level);
        castle_spawn(g);
    }
    memcpy(g->floor_items, g->castle_loot[level - 1], sizeof(g->floor_items));
    g->floor_item_count = g->castle_loot_count[level - 1];
    arrive(g, 15, 11);
    g->dialogue_active = 0;
    g->castle_prompt = 0;
    char message[MAX_MESSAGE_LEN];
    static const char *hints[CASTLE_DEPTH] = {
        "Two approaches: use their levers to control shooting lanes.",
        "Bait the Castellan's charge into his guard detail.",
        "Stop the Herald before a patrol arrives at the marked doorway.",
        "Interact beside the two glowing pedestals to weaken the Arcanist.",
        "Patrols circle the shelves. Fire runes flash twice before burning.",
        "Break the wards, silence the Herald, and watch the throne's fire lane."
    };
    snprintf(message, sizeof(message), "%s: %s", castle_floor_name(level), hints[level - 1]);
    push_message(g, message);
}

void castle_leave(GameState *g, int town) {
    castle_store(g);
    int saved_level = g->level;
    int px = g->player.x;
    int py = g->player.y;
    g->player.x = 15;
    g->player.y = 11;
    // Reset defenders in stable maps; revealed traps, unique progress and loot persist.
    for (int i = 0; i < CASTLE_DEPTH; i++) {
        LevelCache *c = &g->castle_cache[i];
        if (!c->valid) {
            continue;
        }
        g->level = i + 1;
        g->map = c->map;
        int calls[MAX_ENEMIES];
        int heralds = 0;
        for (int j = 0; j < c->enemy_count; j++) {
            if (c->enemies[j].type == ENEMY_BELL_HERALD) {
                calls[heralds++] = c->enemies[j].revived;
            }
        }
        castle_spawn(g);
        int herald = 0;
        for (int j = 0; j < g->enemy_count; j++) {
            if (g->enemies[j].type == ENEMY_BELL_HERALD) {
                if (herald < heralds) {
                    g->enemies[j].revived = calls[herald];
                }
                herald++;
            }
        }
        memcpy(c->enemies, g->enemies, sizeof(g->enemies));
        c->enemy_count = g->enemy_count;
        c->level_cleared = 0;
        memcpy(g->floor_items, g->castle_loot[i], sizeof(g->floor_items));
        g->floor_item_count = g->castle_loot_count[i];
        for (int y = 10; y <= 12; y++) {
            set_tile(g, 29, y, TILE_CASTLE_GATE);
        }
        if (i == 0 || i == 5) {
            for (int x = 14; x <= 16; x++) {
                set_tile(g, x, 21, TILE_CASTLE_GATE_OPEN);
            }
        }
        if (i == 3 || i == 5) {
            for (int y = 47; y <= 49; y++) {
                set_tile(g, 40, y, TILE_CASTLE_GATE_OPEN);
                set_tile(g, 50, y, TILE_CASTLE_GATE);
            }
            if (i == 5 || !(g->castle_minibosses & 2)) {
                set_tile(g, 37, 54, TILE_CASTLE_WARD);
                set_tile(g, 53, 54, TILE_CASTLE_WARD);
            }
        }
        c->map = g->map;
        memcpy(g->castle_loot[i], g->floor_items, sizeof(g->floor_items));
        g->castle_fire_phase[i] = 0;
    }
    g->level = saved_level;
    g->player.x = px;
    g->player.y = py;
    g->portal_active = 0;
    g->castle_prompt = 0;
    g->dialogue_active = 0;
    g->location = town ? LOCATION_TOWN3 : LOCATION_CASTLE;
    if (town) {
        map_generate_town3(&g->map, &g->player.x, &g->player.y);
        map_set_rosemoor_swamp_road(&g->map, g->defeated_bosses & (1 << LOCATION_SWAMP));
        g->player.x = TOWN_W - 2;
        g->player.y = TOWN3_KING_GATE_Y;
    } else {
        map_generate_castle(&g->map, &g->player.x, &g->player.y);
        g->player.x = CROWNROAD_X;
        g->player.y = 11;
    }
    g->level = 1;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->player.poison_turns = 0;
    g->player.frozen_turns = 0;
    push_message(g, town ? "You escape to Rosemoor. The castle severs your portal and regroups." : "You leave the castle. Its surviving defenders regroup.");
}

int castle_travel(GameState *g, int forward) {
    if (g->location != LOCATION_CASTLE_INTERIOR) {
        return 0;
    }
    if (forward) {
        if (g->level == 2 && !(g->castle_minibosses & 1)) {
            push_message(g, "The Castellan guards the next floor.");
        } else if (g->level == 4 && !(g->castle_minibosses & 2)) {
            push_message(g, "The Royal Arcanist guards the next floor.");
        } else if (g->level < CASTLE_DEPTH) {
            castle_enter(g, g->level + 1);
        }
    } else if (g->level == 1) {
        castle_leave(g, 0);
    } else {
        castle_enter(g, g->level - 1);
        arrive(g, g->map.stairs_down_x, g->map.stairs_down_y);
    }
    return 1;
}

void castle_request(GameState *g, int escape) {
    g->castle_prompt = escape ? 2 : 1;
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Castle ward");
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "%s", escape ? "Escape to Rosemoor? No return portal. Surviving defenders regroup; defeated minibosses stay dead. Enter confirms; Esc cancels." : "Return portals are severed here. Retreat regroups surviving defenders. Enter enters; Esc cancels.");
}

int castle_prompt_key(GameState *g, int key, int repeat) {
    if (!g->castle_prompt) {
        return 0;
    }
    if (!repeat && (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER || key == SDL_SCANCODE_ESCAPE)) {
        int prompt = g->castle_prompt;
        g->castle_prompt = 0;
        g->dialogue_active = 0;
        if (key != SDL_SCANCODE_ESCAPE) {
            if (prompt == 2) {
                castle_leave(g, 1);
            } else {
                // Abandon any portal from another adventure before entering.
                game_hide_portal_destination(g);
                g->portal_active = 0;
                castle_enter(g, 1);
            }
        }
    }
    return 1;
}

static int interaction_at(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_CASTLE_INTERIOR) {
        return 0;
    }
    for (int i = -1; i < 4; i++) {
        *x = g->player.x + (i < 0 ? 0 : dxs[i]);
        *y = g->player.y + (i < 0 ? 0 : dys[i]);
        TileType tile = tile_at(g, *x, *y);
        if (tile == TILE_CASTLE_LEVER || tile == TILE_CASTLE_WARD) {
            return 1;
        }
    }
    return 0;
}

int castle_has_interaction(const GameState *g) {
    int x;
    int y;
    return interaction_at(g, &x, &y);
}

int castle_interact(GameState *g) {
    int x;
    int y;
    if (!interaction_at(g, &x, &y)) {
        return 0;
    }
    if (tile_at(g, x, y) == TILE_CASTLE_WARD) {
        set_tile(g, x, y, TILE_CASTLE_WARD_SPENT);
        push_message(g, "The pedestal goes dark. Royal ward protection weakens!");
        return 1;
    }
    int vertical = x == 10 && y == 15;
    int open = tile_at(g, vertical ? 15 : 29, vertical ? 21 : 11) == TILE_CASTLE_GATE;
    for (int i = -1; i <= 1; i++) {
        int gx = vertical ? 15 + i : 29;
        int gy = vertical ? 21 : 11 + i;
        if (open || !occupied(g, gx, gy)) {
            set_tile(g, gx, gy, open ? TILE_CASTLE_GATE_OPEN : TILE_CASTLE_GATE);
        }
    }
    push_message(g, "The lever moves its portcullis, changing cover along this approach.");
    return 1;
}

int castle_step(GameState *g) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level <= 1 || g->level > 5) {
        return 0;
    }
    for (int i = 0; i < g->map.burial_trap_count; i++) {
        BurialTrap *t = &g->map.burial_traps[i];
        if (g->player.x != t->x || g->player.y != t->y) {
            continue;
        }
        int first = !t->spent;
        t->spent = 1;
        if (g->map.tiles[t->y][t->x] != TILE_ITEM) {
            g->map.tiles[t->y][t->x] = TILE_CASTLE_TRAP_OPEN;
        }
        for (int j = 0; j < g->floor_item_count; j++) {
            if (g->floor_items[j].x == t->x && g->floor_items[j].y == t->y) {
                g->floor_items[j].underlying_tile = TILE_CASTLE_TRAP_OPEN;
            }
        }
        castle_enter(g, g->level - 1);
        arrive(g, 8, 15);
        if (first) {
            add_enemy(g, ENEMY_OATHBOUND_SOLDIER, 18, 15);
            add_enemy(g, ENEMY_ROYAL_MARKSMAN, 21, 15);
        }
        push_message(g, "A trapdoor drops you one floor! Guards rally; the hole stays visible.");
        return 1;
    }
    return 0;
}

int castle_attack_marks(const Map *m, const Enemy *e, int x, int y) {
    if (!e->active || e->attack_target_x < 0 || e->attack_target_y < 0 || !map_is_walkable(m, x, y)) {
        return 0;
    }
    if (e->type == ENEMY_BELL_HERALD && !e->revived) {
        return 0;
    }
    if ((e->type == ENEMY_ROYAL_MARKSMAN || e->type == ENEMY_COURT_HEXER || e->type == ENEMY_ROYAL_ARCANIST ||
        (e->type == ENEMY_LORD_VEYR && e->attack_phase >= 2)) && !line_clear(m, e->x, e->y, e->attack_target_x, e->attack_target_y)) {
        return 0;
    }
    if (e->type == ENEMY_CASTELLAN) {
        if (e->attack_phase == 2) {
            int dx = e->attack_target_x - e->x;
            int dy = e->attack_target_y - e->y;
            return dx != 0 ? x == e->x + dx && abs(y - e->y) <= 1 : y == e->y + dy && abs(x - e->x) <= 1;
        }
        int dx = e->attack_target_x - e->x;
        int dy = e->attack_target_y - e->y;
        int sx = (dx > 0) - (dx < 0);
        int sy = (dy > 0) - (dy < 0);
        int length = abs(dx) + abs(dy);
        for (int i = 1; i <= length; i++) {
            int tx = e->x + sx * i;
            int ty = e->y + sy * i;
            if (!map_is_walkable(m, tx, ty)) {
                break;
            }
            if (x == tx && y == ty) {
                return 1;
            }
        }
        return 0;
    }
    int radius = e->type == ENEMY_LORD_VEYR ? (e->attack_phase == 3 ? 2 : e->attack_phase == 2 ? 1 : 0) : e->type == ENEMY_ROYAL_ARCANIST ? 1 : 0;
    return abs(x - e->attack_target_x) + abs(y - e->attack_target_y) <= radius;
}

static int distances[MAP_H][MAP_W];
static int queue[MAP_W * MAP_H];

static void build_distances(const GameState *g) {
    memset(distances, -1, sizeof(distances));
    int head = 0;
    int tail = 1;
    queue[0] = g->player.y * MAP_W + g->player.x;
    distances[g->player.y][g->player.x] = 0;
    while (head < tail) {
        int x = queue[head] % MAP_W;
        int y = queue[head++] / MAP_W;
        for (int d = 0; d < 4; d++) {
            int tx = x + dxs[d];
            int ty = y + dys[d];
            if (tx <= 0 || ty <= 0 || tx >= CASTLE_W - 1 || ty >= CASTLE_H - 1 || distances[ty][tx] >= 0 ||
                !map_is_walkable(&g->map, tx, ty) || g->map.tiles[ty][tx] == TILE_CASTLE_TRAP_OPEN || g->map.tiles[ty][tx] == TILE_CASTLE_TRAP_HIDDEN) {
                continue;
            }
            distances[ty][tx] = distances[y][x] + 1;
            queue[tail++] = ty * MAP_W + tx;
        }
    }
}

static void advance(GameState *g, Enemy *e) {
    int best = distances[e->y][e->x];
    for (int d = 0; d < 4; d++) {
        int x = e->x + dxs[d];
        int y = e->y + dys[d];
        if (x < 0 || y < 0 || x >= CASTLE_W || y >= CASTLE_H || distances[y][x] < 0 ||
            (best >= 0 && distances[y][x] >= best) || occupied(g, x, y)) {
            continue;
        }
        e->facing_dx = dxs[d];
        e->facing_dy = dys[d];
        e->x = x;
        e->y = y;
        break;
    }
}

static int line_clear(const Map *m, int x, int y, int tx, int ty) {
    int dx = abs(tx - x);
    int dy = -abs(ty - y);
    int sx = tx > x ? 1 : -1;
    int sy = ty > y ? 1 : -1;
    int error = dx + dy;
    while (x != tx || y != ty) {
        int twice = 2 * error;
        if (twice >= dy) {
            error += dy;
            x += sx;
        }
        if (twice <= dx) {
            error += dx;
            y += sy;
        }
        if (!map_is_walkable(m, x, y)) {
            return 0;
        }
    }
    return 1;
}

static void patrol(GameState *g, Enemy *e) {
    if (g->level != 5 || e->type != ENEMY_OATHBOUND_SOLDIER) {
        return;
    }
    for (int i = 1; i < 5; i++) {
        const Room *room = &g->map.rooms[i];
        if (e->x < room->x || e->x >= room->x + room->w || e->y < room->y || e->y >= room->y + room->h) {
            continue;
        }
        for (int turn = 0; turn < 4; turn++) {
            int x = e->x + e->facing_dx;
            int y = e->y + e->facing_dy;
            if (x > room->x && x < room->x + room->w - 1 && y > room->y && y < room->y + room->h - 1 && !occupied(g, x, y)) {
                e->x = x;
                e->y = y;
                return;
            }
            int dx = e->facing_dx;
            e->facing_dx = -e->facing_dy;
            e->facing_dy = dx;
        }
        return;
    }
}

static int active_wards(const GameState *g) {
    return (tile_at(g, 37, 54) == TILE_CASTLE_WARD) + (tile_at(g, 53, 54) == TILE_CASTLE_WARD);
}

int castle_fire_warning(const GameState *g, int x, int y) {
    if (g->location != LOCATION_CASTLE_INTERIOR || (g->level != 5 && g->level != 6) || g->game_won) {
        return 0;
    }
    int phase = g->castle_fire_phase[g->level - 1];
    return (phase == 1 || phase == 2) && tile_at(g, x, y) == TILE_CASTLE_FIRE_RUNE;
}

static int tick_fire(GameState *g) {
    if (g->level != 5 && g->level != 6) {
        return 0;
    }
    int *phase = &g->castle_fire_phase[g->level - 1];
    *phase = (*phase + 1) % 4;
    if ((*phase == 1 || *phase == 2) && (g->level == 5 || (g->player.x >= 34 && g->player.y >= 44))) {
        push_message(g, *phase == 1 ? "Fire runes glow! Their lanes ignite in two turns." : "Fire lanes ignite next turn! Step off the glowing runes.");
    }
    if (*phase == 3 && underfoot(g) == TILE_CASTLE_FIRE_RUNE) {
        int damage = 24 - g->player.defense / 2;
        push_message(g, "Royal fire erupts beneath you!");
        return damage > 6 ? damage : 6;
    }
    return 0;
}

static void call_patrol(GameState *g, Enemy *e) {
    int court = g->level == 3;
    int throne = g->level == 6 && e->y >= 44;
    if (court || throne) {
        int y = court ? 35 : 46;
        const EnemyType types[3] = {ENEMY_OATHBOUND_SOLDIER, ENEMY_IRON_WARDEN, ENEMY_ROYAL_MARKSMAN};
        for (int i = 0; i < 3; i++) {
            // Doorway and adjacent arrival tiles; newcomers act next turn.
            add_enemy(g, types[i], i == 1 ? 52 : 53, i == 2 ? y - 1 : y);
        }
        push_message(g, "The bell calls a patrol through the marked doorway!");
    } else {
        for (int d = 0; d < 4; d++) {
            if (add_enemy(g, ENEMY_OATHBOUND_SOLDIER, e->x + dxs[d], e->y + dys[d])) {
                break;
            }
        }
        push_message(g, "The Herald's bell calls one royal guard!");
    }
    e->revived = 1;
}

int castle_barrier_warning(const GameState *g, int x, int y) {
    if ((x != 40 && x != 50) || y < 47 || y > 49) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (e->active && e->attack_target_x >= 0 && (e->type == ENEMY_ROYAL_ARCANIST ||
            (e->type == ENEMY_LORD_VEYR && e->attack_phase >= 2))) {
            return 1;
        }
    }
    return 0;
}

static void change_wards(GameState *g) {
    for (int y = 47; y <= 49; y++) {
        for (int x = 40; x <= 50; x += 10) {
            TileType tile = tile_at(g, x, y);
            if (tile == TILE_CASTLE_GATE) {
                set_tile(g, x, y, TILE_CASTLE_GATE_OPEN);
            } else if (tile == TILE_CASTLE_GATE_OPEN && !occupied(g, x, y)) {
                set_tile(g, x, y, TILE_CASTLE_GATE);
            }
        }
    }
    push_message(g, "Royal commands shift the warned barriers. Clear routes remain open.");
}

int castle_tick(GameState *g) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->castle_prompt || g->game_won) {
        return 0;
    }
    if (castle_step(g)) {
        return 0;
    }
    build_distances(g);
    int damage = 0;
    int count = g->enemy_count;
    for (int i = 0; i < count; i++) {
        Enemy *e = &g->enemies[i];
        if (!e->active) {
            continue;
        }
        if (e->frozen_turns > 0) {
            e->frozen_turns--;
            continue;
        }
        int distance = abs(g->player.x - e->x) + abs(g->player.y - e->y);
        // Room-by-room encounters; bosses never pursue outside their arena.
        if ((e->is_boss && (g->player.x < 34 || g->player.x >= 56 || g->player.y < 44 || g->player.y >= 58)) || (!e->is_boss && distances[e->y][e->x] > 12)) {
            e->attack_target_x = -1;
            e->move_timer = 0;
            patrol(g, e);
            continue;
        }
        if (e->move_timer >= 3) {
            e->move_timer = e->move_timer == 3 ? 1 : e->move_timer - 1;
            continue;
        }
        if (e->move_timer == 1) {
            if (e->type == ENEMY_BELL_HERALD && !e->revived) {
                call_patrol(g, e);
            } else {
                if (castle_attack_marks(&g->map, e, g->player.x, g->player.y)) {
                    int hit = e->attack - g->player.defense;
                    if (e->type == ENEMY_ROYAL_ARCANIST || e->type == ENEMY_LORD_VEYR) {
                        hit += active_wards(g) * 4;
                    }
                    for (int j = 0; j < count; j++) {
                        const Enemy *hexer = &g->enemies[j];
                        if (hexer->active && hexer->type == ENEMY_COURT_HEXER && hexer != e &&
                            abs(hexer->x - e->x) + abs(hexer->y - e->y) <= 4) {
                            hit += 5;
                            break;
                        }
                    }
                    damage += hit > 3 ? hit : 3;
                }
                if (e->type == ENEMY_CASTELLAN && e->attack_phase == 1) {
                    int sx = (e->attack_target_x > e->x) - (e->attack_target_x < e->x);
                    int sy = (e->attack_target_y > e->y) - (e->attack_target_y < e->y);
                    for (int step = 0; step < 5; step++) {
                        if (occupied(g, e->x + sx, e->y + sy)) {
                            if (!map_is_walkable(&g->map, e->x + sx, e->y + sy)) {
                                e->frozen_turns = 1;
                                push_message(g, "The Castellan crashes into stone and is staggered!");
                            }
                            for (int j = 0; j < count; j++) {
                                Enemy *guard = &g->enemies[j];
                                if (guard->active && !guard->is_boss && guard->x == e->x + sx && guard->y == e->y + sy) {
                                    guard->frozen_turns = 2;
                                    guard->move_timer = 2;
                                    guard->attack_target_x = -1;
                                    guard->attack_target_y = -1;
                                    push_message(g, "The Castellan crashes into his guard and breaks its defense!");
                                }
                            }
                            break;
                        }
                        e->x += sx;
                        e->y += sy;
                    }
                }
                push_message(g, "The royal attack strikes the marked stones!");
                if (e->type == ENEMY_ROYAL_ARCANIST || (e->type == ENEMY_LORD_VEYR && e->attack_phase >= 2)) {
                    change_wards(g);
                }
            }
            e->attack_target_x = -1;
            e->attack_target_y = -1;
            e->move_timer = 2;
            continue;
        }
        if (e->move_timer == 2) {
            e->move_timer = 0;
            continue;
        }
        if (e->type == ENEMY_BELL_HERALD && !e->revived && distance <= 8) {
            e->move_timer = 1;
            push_message(g, "The Bell Herald raises its bell! Kill or freeze it to interrupt.");
            continue;
        }
        int ranged = e->type == ENEMY_ROYAL_MARKSMAN || e->type == ENEMY_COURT_HEXER || e->type == ENEMY_ROYAL_ARCANIST ||
            (e->type == ENEMY_LORD_VEYR && e->hp * 3 <= e->max_hp * 2);
        if ((ranged && distance <= 8 && line_clear(&g->map, e->x, e->y, g->player.x, g->player.y)) ||
            (!ranged && distance <= 2) || (e->type == ENEMY_CASTELLAN && distance <= 6)) {
            e->attack_target_x = g->player.x;
            e->attack_target_y = g->player.y;
            e->attack_phase = e->type == ENEMY_LORD_VEYR ? (e->hp * 3 > e->max_hp * 2 ? 1 : e->hp * 3 > e->max_hp ? 2 : 3) : 0;
            if (e->type == ENEMY_LORD_VEYR && !e->revived) {
                e->revived = 1;
                add_enemy(g, ENEMY_IRON_WARDEN, 42, 46);
                add_enemy(g, ENEMY_ROYAL_MARKSMAN, 48, 46);
                push_message(g, "Veyr commands his last two royal guards to defend the throne!");
            }
            if (e->type == ENEMY_CASTELLAN) {
                int sx = (g->player.x > e->x) - (g->player.x < e->x);
                int sy = (g->player.y > e->y) - (g->player.y < e->y);
                if (abs(g->player.x - e->x) >= abs(g->player.y - e->y)) {
                    sy = 0;
                } else {
                    sx = 0;
                }
                e->attack_phase = distance <= 2 ? 2 : 1;
                e->attack_target_x = e->x + sx * (e->attack_phase == 2 ? 1 : 5);
                e->attack_target_y = e->y + sy * (e->attack_phase == 2 ? 1 : 5);
            }
            e->move_timer = e->type == ENEMY_ROYAL_ARCANIST || (e->type == ENEMY_LORD_VEYR && e->attack_phase == 2) ? 3 :
                e->type == ENEMY_LORD_VEYR && e->attack_phase == 3 ? 4 : 1;
            int fx = g->player.x - e->x;
            int fy = g->player.y - e->y;
            e->facing_dx = abs(fx) >= abs(fy) ? (fx > 0 ? 1 : -1) : 0;
            e->facing_dy = e->facing_dx == 0 ? (fy > 0 ? 1 : -1) : 0;
            char message[MAX_MESSAGE_LEN];
            snprintf(message, sizeof(message), "%s prepares an attack. Leave the marked stones!", e->name);
            push_message(g, message);
        } else if (distances[e->y][e->x] >= 0) {
            advance(g, e);
        }
    }
    return damage + tick_fire(g);
}

int castle_enemy_damage(const GameState *g, const Enemy *e, int damage) {
    if (g->location != LOCATION_CASTLE_INTERIOR) {
        return damage;
    }
    if ((e->type == ENEMY_IRON_WARDEN || e->type == ENEMY_CASTELLAN || (e->type == ENEMY_LORD_VEYR && e->hp * 3 > e->max_hp * 2)) && e->move_timer != 2 && !e->frozen_turns) {
        int front = (g->player.x - e->x) * e->facing_dx + (g->player.y - e->y) * e->facing_dy > 0;
        if (front) {
            damage /= 2;
        }
    } else if (!e->is_boss) {
        for (int i = 0; i < g->enemy_count; i++) {
            const Enemy *guard = &g->enemies[i];
            if (!guard->active || guard->type != ENEMY_IRON_WARDEN || guard->move_timer == 2 || guard->frozen_turns ||
                abs(guard->x - e->x) + abs(guard->y - e->y) > 4) {
                continue;
            }
            int front = (g->player.x - guard->x) * guard->facing_dx + (g->player.y - guard->y) * guard->facing_dy;
            int behind = (e->x - guard->x) * guard->facing_dx + (e->y - guard->y) * guard->facing_dy;
            if (front > 0 && behind < 0) {
                damage /= 2;
                break;
            }
        }
    }
    if (e->type == ENEMY_ROYAL_ARCANIST || e->type == ENEMY_LORD_VEYR) {
        damage = damage * (100 - active_wards(g) * 20) / 100;
    }
    return damage > 0 ? damage : 1;
}

void castle_record_death(GameState *g, Enemy *e) {
    if (e->type == ENEMY_CASTELLAN || e->type == ENEMY_ROYAL_ARCANIST) {
        g->castle_minibosses |= e->type == ENEMY_CASTELLAN ? 1 : 2;
        g->gold += 100;
        push_message(g, "Miniboss defeated! The way to the next castle floor is open.");
    } else if (e->type == ENEMY_LORD_VEYR) {
        g->game_won = 1;
        g->defeated_bosses |= 1 << LOCATION_CASTLE_INTERIOR;
        g->score += 10000;
        g->portal_active = 0;
        push_message(g, "Veyr falls. The broken oath is undone. You have won!");
    } else if (e->experience > 0) {
        g->gold += 5;
    }
}
