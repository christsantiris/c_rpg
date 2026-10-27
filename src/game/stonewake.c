#include "containers.h"
#include "stonewake.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int stonewake_map(const Map *map) {
    return map->tiles[0][0] == TILE_STONEWAKE_BOUNDARY || map->tiles[0][0] == TILE_STONEWAKE_LEGACY_BOUNDARY;
}

int stonewake_crane_tile(TileType tile) {
    return tile == TILE_STONEWAKE_CRANE_NEW || tile == TILE_STONEWAKE_CRANE_UP ||
        tile == TILE_STONEWAKE_CRANE_DOWN || tile == TILE_STONEWAKE_CRANE_SALVAGE;
}

int stonewake_crane_mode(TileType tile) {
    return tile == TILE_STONEWAKE_CRANE_DOWN ? 1 : tile == TILE_STONEWAKE_CRANE_SALVAGE ? 2 : 0;
}

const char *stonewake_stage_name(int level) {
    static const char *names[5] = {"Outer Workings", "Excavation Pits", "Buried Workshops", "Abandoned Galleries", "Colossus Yard"};
    return names[level >= 1 && level <= 5 ? level - 1 : 0];
}

static void rect(Map *map, int x, int y, int w, int h, TileType tile) {
    for (int ty = y; ty < y + h; ty++) {
        for (int tx = x; tx < x + w; tx++) {
            if (tx > 0 && ty > 0 && tx < SWAMP_MAP_W - 1 && ty < SWAMP_MAP_H - 1) {
                map->tiles[ty][tx] = tile;
            }
        }
    }
}

static void ramp(Map *map, int x, int y, int tx, int ty) {
    while (1) {
        rect(map, x - 1, y - 1, 3, 3, TILE_GLASSDEEP_FLOOR);
        if (x == tx && y == ty) {
            break;
        }
        if (x != tx) {
            x += (tx > x) - (tx < x);
        } else {
            y += (ty > y) - (ty < y);
        }
    }
}

static int salvage_platform(Map *map, int x, int y, TileType store) {
    if (x < 1 || y < 5 || x + 10 >= MAP_W || y + 1 >= MAP_H) {
        return 0;
    }
    // A moat separates the platform from the permanent ramps in every direction.
    for (int ty = y - 4; ty < y; ty++) {
        for (int tx = x + 3; tx <= x + 7; tx++) {
            TileType tile = map->tiles[ty][tx];
            if (tile != TILE_GLASSDEEP_POOL && tile != TILE_GLASSDEEP_WALL) {
                return 0;
            }
        }
    }
    rect(map, x + 4, y - 3, 3, 2, TILE_STONEWAKE_LEDGE);
    map->tiles[y - 3][x + 5] = store;
    map->tiles[y - 1][x + 5] = TILE_STONEWAKE_GAP;
    return 1;
}

void stonewake_generate(Map *map, int level) {
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = TILE_GLASSDEEP_WALL;
        }
    }
    // Different yards, loops and gallery proportions per stage, with seeded variation.
    static const int centers[5][9][2] = {
        {{35,7},{12,17},{35,18},{57,17},{13,35},{35,35},{57,35},{13,52},{35,53}},
        {{35,7},{14,15},{36,22},{57,14},{11,36},{36,39},{58,36},{56,53},{35,53}},
        {{35,7},{12,18},{34,17},{57,18},{14,38},{35,34},{56,39},{13,53},{35,53}},
        {{35,7},{11,15},{33,22},{58,17},{14,36},{36,40},{58,36},{56,53},{35,53}},
        {{35,7},{12,16},{35,18},{58,16},{13,34},{35,35},{58,34},{13,51},{35,52}}
    };
    int stage = level >= 1 && level <= 5 ? level - 1 : 0;
    map->room_count = 9;
    for (int i = 0; i < 9; i++) {
        int w = i == 8 ? 20 : 10 + rand() % 5;
        int h = i == 8 ? 13 : 8 + rand() % 4;
        int cx = centers[stage][i][0] + (i == 0 || i == 8 ? 0 : rand() % 3 - 1);
        int cy = centers[stage][i][1];
        map->rooms[i] = (Room){cx - w / 2, cy - h / 2, w, h};
        rect(map, cx - w / 2, cy - h / 2, w, h, TILE_GLASSDEEP_FLOOR);
        rect(map, cx - 2, cy + 1, 5, 2, TILE_GLASSDEEP_RUIN);
    }
    static const int links[][2] = {{0,1},{0,2},{0,3},{1,4},{2,5},{3,6},{4,7},{7,8},{6,8},{4,5},{5,6}};
    for (int i = 0; i < (int)(sizeof(links) / sizeof(links[0])); i++) {
        int ax, ay, bx, by;
        map_room_center(&map->rooms[links[i][0]], &ax, &ay);
        map_room_center(&map->rooms[links[i][1]], &bx, &by);
        ramp(map, ax, ay, bx, by);
    }
    // Two flooded cuts sit between the permanent ramp circuits.
    for (int n = 0; n < 2; n++) {
        int cx = n ? (level == 5 ? 44 : 47) : 23;
        int cy = n ? (level == 5 ? 51 : 45) : 26;
        rect(map, cx - 4, cy - 4, 9, 9, TILE_GLASSDEEP_POOL);
        // The ramps ring each cut, so every crane setting leaves each main yard reachable.
        ramp(map, cx - 6, cy - 6, cx + 6, cy - 6);
        ramp(map, cx - 6, cy + 6, cx + 6, cy + 6);
        ramp(map, cx - 6, cy - 6, cx - 6, cy + 6);
        ramp(map, cx + 6, cy - 6, cx + 6, cy + 6);
        // Walkable banks connect both crane ends to the permanent circuit.
        ramp(map, cx - 6, cy, cx - 6, n ? 35 : 17);
        ramp(map, cx + 6, cy, cx + 6, n ? 53 : 35);
        for (int x = cx - 4; x <= cx + 4; x++) {
            map->tiles[cy][x] = TILE_STONEWAKE_GAP;
        }
        map->tiles[cy][cx - 5] = TILE_STONEWAKE_CRANE_NEW;
        map->tiles[cy][cx + 5] = TILE_STONEWAKE_REMOTE;
        salvage_platform(map, cx - 5, cy, TILE_STONEWAKE_SUPPLIES);
    }
    for (int n = 0; n < 2; n++) {
        Room *room = &map->rooms[n ? 7 : 4];
        rect(map, room->x + 2, room->y + 1, 3, 2, TILE_STONEWAKE_WORKSHOP);
    }
    ramp(map, 35, 7, 35, 1);
    ramp(map, 35, 53, 35, SWAMP_MAP_H - 2);
    map->stairs_up_x = 35;
    map->stairs_up_y = 0;
    map->stairs_down_x = 35;
    map->stairs_down_y = SWAMP_MAP_H - 1;
    map->tiles[0][35] = TILE_GLASSDEEP_ENTRANCE;
    map->tiles[SWAMP_MAP_H - 1][35] = TILE_GLASSDEEP_EXIT;
    map->tiles[0][0] = TILE_STONEWAKE_BOUNDARY;
}

static TileType ground(const GameState *g, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
        return TILE_GLASSDEEP_WALL;
    }
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_PORTAL && g->portal_active && g->portal_location == g->location && g->portal_level == g->level && g->portal_x == x && g->portal_y == y) {
        return g->portal_origin_tile;
    }
    for (int i = 0; tile == TILE_ITEM && i < g->floor_item_count; i++) {
        if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
            return g->floor_items[i].underlying_tile;
        }
    }
    return tile;
}

int stonewake_has_salvage(const GameState *g, int x, int y) {
    return ground(g, x + 4, y - 2) == TILE_STONEWAKE_LEDGE;
}

static void set_ground(GameState *g, int x, int y, TileType tile) {
    int portal = g->portal_active && g->portal_location == g->location && g->portal_level == g->level && g->portal_x == x && g->portal_y == y;
    if (portal) {
        g->portal_origin_tile = tile;
    }
    if (g->map.tiles[y][x] != TILE_ITEM && !(portal && g->map.tiles[y][x] == TILE_PORTAL)) {
        g->map.tiles[y][x] = tile;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
            g->floor_items[i].underlying_tile = tile;
        }
    }
}

static int occupied(const GameState *g, int x, int y) {
    if (g->player.x == x && g->player.y == y) {
        return 1;
    }
    if (g->portal_active && g->portal_location == g->location && g->portal_level == g->level && g->portal_x == x && g->portal_y == y) {
        return 1;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
            return 1;
        }
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 1;
        }
    }
    return 0;
}

static int target(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_GLASSDEEP || !stonewake_map(&g->map)) {
        return 0;
    }
    const int dx[5] = {0,0,1,0,-1};
    const int dy[5] = {0,-1,0,1,0};
    for (int i = 0; i < 5; i++) {
        *x = g->player.x + dx[i];
        *y = g->player.y + dy[i];
        TileType tile = ground(g, *x, *y);
        if (tile == TILE_STONEWAKE_REMOTE) {
            *x -= 10;
            return stonewake_crane_tile(ground(g, *x, *y));
        }
        if (stonewake_crane_tile(tile) || tile == TILE_STONEWAKE_SUPPLIES) {
            return 1;
        }
    }
    return 0;
}

int stonewake_has_interaction(const GameState *g) {
    int x, y;
    return target(g, &x, &y);
}

int stonewake_interact(GameState *g) {
    if (g->location == LOCATION_GLASSDEEP && containers_hint(g)) {
        return containers_interact(g);
    }
    int x, y;
    if (!target(g, &x, &y)) {
        return 0;
    }
    TileType tile = ground(g, x, y);
    if (tile == TILE_STONEWAKE_SUPPLIES) {
        return containers_interact(g);
    }
    int mode = stonewake_crane_mode(tile);
    int next = (mode + 1) % 3;
    int has_salvage = stonewake_has_salvage(g, x, y);
    if (!has_salvage && next == 2) {
        next = 0;
    }
    if (x + 10 >= MAP_W || y < 4) {
        return 0;
    }
    // Only retract unoccupied sections; a portal or dropped item on the island also holds its access open.
    for (int dx = 1; dx <= 9; dx++) {
        int remains = next == 1 || (next == 2 && dx <= 5);
        if (!remains && occupied(g, x + dx, y)) {
            push_message(g, "The moving gangway is occupied. Clear its actors, loot and portals first.");
            return 1;
        }
    }
    if (has_salvage && next != 2) {
        for (int ty = y - 3; ty < y; ty++) {
            for (int tx = x + 4; tx <= x + 6; tx++) {
                if (occupied(g, tx, ty)) {
                    push_message(g, "The salvage platform is occupied. Its gangway must remain down.");
                    return 1;
                }
            }
        }
    }
    for (int dx = 1; dx <= 9; dx++) {
        set_ground(g, x + dx, y, next == 1 || (next == 2 && dx <= 5) ? TILE_STONEWAKE_BRIDGE : TILE_STONEWAKE_GAP);
        map_mark_explored(&g->map, x + dx, y);
    }
    if (has_salvage) {
        set_ground(g, x + 5, y - 1, next == 2 ? TILE_STONEWAKE_BRIDGE : TILE_STONEWAKE_GAP);
        if (next == 2) {
            for (int ty = y - 3; ty < y; ty++) {
                for (int tx = x + 4; tx <= x + 6; tx++) {
                    map_mark_explored(&g->map, tx, ty);
                }
            }
        }
    }
    set_ground(g, x, y, next == 1 ? TILE_STONEWAKE_CRANE_DOWN : next == 2 ? TILE_STONEWAKE_CRANE_SALVAGE : TILE_STONEWAKE_CRANE_UP);
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->active && e->type == ENEMY_PRISM_SOVEREIGN && e->boss_aux_x <= 0 && e->move_timer > 0 && abs(e->x - x) + abs(e->y - y) <= 12) {
            e->move_timer = -2;
            e->boss_aux_x = 5;
            push_message(g, "The crane load jars the Colossus! Its joints are exposed for two actions.");
        }
    }
    push_message(g, next == 1 ? "CROSSING: the through gangway is down. The salvage spur is raised." :
        next == 2 ? "SALVAGE: the east crossing lifts; a spur lowers to the supply platform." :
        "RAISED: both gangways are lifted. Use the permanent ramp circuit.");
    return 1;
}

int stonewake_attack_marks(const GameState *g, const Enemy *e, int x, int y) {
    if (e->type != ENEMY_PRISM_SOVEREIGN || !e->active || e->move_timer <= 0 || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    int dx = x - e->attack_target_x;
    int dy = y - e->attack_target_y;
    if (e->boss_aux_y == 1) {
        return dx == 0 && dy == 0;
    }
    return e->boss_attack == 2 ? abs(dx) + abs(dy) <= 2 : abs(dx) <= 1 && abs(dy) <= 1;
}

static int warning_escape(const GameState *g, const Enemy *e, int x, int y, int turns) {
    if (!stonewake_attack_marks(g, e, x, y)) {
        return 1;
    }
    if (turns <= 0) {
        return 0;
    }
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    for (int n = 0; n < 4; n++) {
        int nx = x + dx[n];
        int ny = y + dy[n];
        if (!map_is_walkable(&g->map, nx, ny)) {
            continue;
        }
        int blocked = 0;
        for (int i = 0; i < g->enemy_count; i++) {
            blocked |= g->enemies[i].active && g->enemies[i].x == nx && g->enemies[i].y == ny;
        }
        if (!blocked && warning_escape(g, e, nx, ny, turns - 1)) {
            return 1;
        }
    }
    return 0;
}

int stonewake_colossus_tick(GameState *g, Enemy *e) {
    if (e->move_timer == 0 && abs(g->player.x - e->x) + abs(g->player.y - e->y) > 12) {
        return 0;
    }
    if (e->boss_aux_x > 0) {
        e->boss_aux_x--;
    }
    if (e->move_timer < 0) {
        e->move_timer++;
        return 0;
    }
    if (e->move_timer > 0) {
        if (e->move_timer > 1) {
            e->move_timer--;
            return 0;
        }
        int hit = stonewake_attack_marks(g, e, g->player.x, g->player.y);
        e->move_timer = -2;
        push_message(g, "The Colossus strikes the marked ground, exposing its joints for two actions.");
        int damage = e->attack - g->player.defense / 2;
        return hit ? (damage > 3 ? damage : 3) : 0;
    }
    e->attack_phase = e->hp <= e->max_hp / 2 || e->attack_phase;
    e->boss_attack = e->attack_phase && e->boss_attack == 1 ? 2 : 1;
    e->attack_target_x = g->player.x;
    e->attack_target_y = g->player.y;
    e->move_timer = e->boss_attack == 2 ? 3 : 2;
    e->boss_aux_y = 0;
    if (!warning_escape(g, e, g->player.x, g->player.y, e->move_timer)) {
        e->boss_aux_y = 1;
        if (!warning_escape(g, e, g->player.x, g->player.y, e->move_timer)) {
            e->move_timer = -1;
            return 0;
        }
    }
    push_message(g, e->boss_attack == 2 ? "Quarry shockwave in 3 actions! Leave the marked diamond." : "Colossus hammerfall in 2 actions! Leave the marked ground.");
    return 0;
}

int stonewake_claim_warhammer(GameState *g) {
    if (g->stonewake_warhammer_state != 1) {
        return 0;
    }
    if (g->inventory_count >= MAX_INVENTORY) {
        push_message(g, "Make room in your pack. Orin keeps your Magic Stone Warhammer safe.");
        return 0;
    }
    g->inventory[g->inventory_count++] = item_make_stone_warhammer();
    g->stonewake_warhammer_state = 2;
    push_message(g, "Received Magic Stone Warhammer: +8 attack, +30 maximum HP while equipped.");
    return 1;
}

static TileType migrated_tile(TileType tile) {
    if ((tile >= TILE_GLASS_FACE_N && tile <= TILE_GLASS_SEAL_OPEN) ||
        (tile >= TILE_PRISM_LEFT && tile <= TILE_PRISM_RETURN)) {
        return TILE_GLASSDEEP_RUIN;
    }
    return tile;
}

static void migrate_loot(FloorItem *items, int count) {
    for (int i = 0; i < count; i++) {
        items[i].underlying_tile = migrated_tile(items[i].underlying_tile);
    }
}

void stonewake_migrate(GameState *g) {
    // Retain the geometry of expeditions already in progress. Unvisited stages use the new generator.
    for (int n = -1; n < GLASSDEEP_DEPTH; n++) {
        Map *map = n < 0 ? &g->map : &g->glassdeep_cache[n].map;
        if ((n < 0 && g->location != LOCATION_GLASSDEEP) || (n >= 0 && !g->glassdeep_cache[n].valid) ||
            stonewake_map(map) || map->room_count < 8) {
            continue;
        }
        map->tiles[0][0] = TILE_STONEWAKE_LEGACY_BOUNDARY;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                map->tiles[y][x] = migrated_tile(map->tiles[y][x]);
            }
        }
        // Existing walking squares never close. Add optional spans only across spare rock/water.
        int cranes = 0;
        for (int y = 5; y < SWAMP_MAP_H - 5 && cranes < 2; y++) {
            for (int x = 4; x < SWAMP_MAP_W - 15 && cranes < 2; x++) {
                const Room *last = &map->rooms[map->room_count - 1];
                if ((y >= last->y && y < last->y + last->h) || map->tiles[y][x] != TILE_GLASSDEEP_FLOOR || map->tiles[y][x + 10] != TILE_GLASSDEEP_FLOOR) {
                    continue;
                }
                int clear = 1;
                for (int dx = 1; dx <= 9; dx++) {
                    TileType tile = map->tiles[y][x + dx];
                    clear &= tile == TILE_GLASSDEEP_WALL || tile == TILE_GLASSDEEP_POOL;
                }
                if (!clear) {
                    continue;
                }
                map->tiles[y][x] = TILE_STONEWAKE_CRANE_UP;
                for (int dx = 1; dx <= 9; dx++) {
                    map->tiles[y][x + dx] = TILE_STONEWAKE_GAP;
                }
                cranes++;
                y += 4;
                break;
            }
        }
        Enemy *enemies = n < 0 ? g->enemies : g->glassdeep_cache[n].enemies;
        int count = n < 0 ? g->enemy_count : g->glassdeep_cache[n].enemy_count;
        if (n < 0) {
            migrate_loot(g->floor_items, g->floor_item_count);
        }
        for (int i = 0; i < count; i++) {
            const char *name = enemies[i].type == ENEMY_CRYSTAL_SPIDER ? "Rockworm" :
                enemies[i].type == ENEMY_BLIND_STALKER ? "Quarry Construct" :
                enemies[i].type == ENEMY_SHARD_GOLEM ? "Stone Troll" :
                enemies[i].type == ENEMY_PRISM_SOVEREIGN ? "Quarry Colossus" : NULL;
            if (strcmp(enemies[i].name, "Resonator Guardian") == 0) {
                snprintf(enemies[i].name, sizeof(enemies[i].name), "Quarry Guardian");
            } else if (name && strcmp(enemies[i].name, "Quarry Guardian") != 0 && strcmp(enemies[i].name, "Shardwarden") != 0) {
                snprintf(enemies[i].name, sizeof(enemies[i].name), "%s", name);
            }
            if (enemies[i].type == ENEMY_PRISM_SOVEREIGN) {
                enemies[i].move_timer = 0;
                enemies[i].boss_attack = 0;
                enemies[i].boss_aux_x = 0;
                enemies[i].boss_aux_y = 0;
            }
        }
    }
    if (g->portal_location == LOCATION_GLASSDEEP) {
        g->portal_origin_tile = migrated_tile(g->portal_origin_tile);
    }
    if (g->dialogue_active && (strstr(g->dialogue_speaker, "Resonator") || strcmp(g->dialogue_speaker, "Surveyor Orin") == 0)) {
        g->dialogue_active = 0;
    }
}

void stonewake_begin_expedition(GameState *g) {
    for (int i = 0; i < GLASSDEEP_DEPTH; i++) {
        LevelCache *cache = &g->glassdeep_cache[i];
        if (!cache->valid || cache->map.tiles[0][0] != TILE_STONEWAKE_LEGACY_BOUNDARY) {
            continue;
        }
        // Keep an old return portal's expedition intact until that portal is used.
        if (g->portal_active && g->portal_location == LOCATION_GLASSDEEP && g->portal_level == i + 1) {
            continue;
        }
        cache->valid = 0;
        cache->level_cleared = 0;
        cache->enemy_count = 0;
    }
}

static TileType saved_ground(const GameState *g, const Map *map, int level, int x, int y) {
    if (map == &g->map) {
        return ground(g, x, y);
    }
    TileType tile = map->tiles[y][x];
    if (tile == TILE_PORTAL && g->portal_active && g->portal_location == LOCATION_GLASSDEEP && g->portal_level == level && g->portal_x == x && g->portal_y == y) {
        return g->portal_origin_tile;
    }
    return tile;
}

static void set_saved_ground(GameState *g, Map *map, int level, int x, int y, TileType tile) {
    if (g->portal_active && g->portal_location == LOCATION_GLASSDEEP && g->portal_level == level && g->portal_x == x && g->portal_y == y) {
        g->portal_origin_tile = tile;
    }
    if (map == &g->map) {
        set_ground(g, x, y, tile);
    } else if (map->tiles[y][x] == TILE_PORTAL && g->portal_active && g->portal_location == LOCATION_GLASSDEEP && g->portal_level == level && g->portal_x == x && g->portal_y == y) {
        g->portal_origin_tile = tile;
    } else {
        map->tiles[y][x] = tile;
    }
}

void stonewake_migrate_cranes(GameState *g) {
    for (int n = -1; n < GLASSDEEP_DEPTH; n++) {
        Map *map = n < 0 ? &g->map : &g->glassdeep_cache[n].map;
        int level = n < 0 ? g->level : n + 1;
        if ((n < 0 && g->location != LOCATION_GLASSDEEP) || (n >= 0 && !g->glassdeep_cache[n].valid) || !stonewake_map(map)) {
            continue;
        }
        int stores[2][2];
        int count = 0;
        // Keep each old store's claimed/unclaimed state when moving it into a flooded cut.
        for (int y = 1; y < SWAMP_MAP_H - 1 && count < 2; y++) {
            for (int x = 1; x < SWAMP_MAP_W - 1 && count < 2; x++) {
                TileType tile = saved_ground(g, map, level, x, y);
                if (tile == TILE_STONEWAKE_SUPPLIES || tile == TILE_STONEWAKE_EMPTY) {
                    stores[count][0] = x;
                    stores[count++][1] = y;
                }
            }
        }
        int moved = 0;
        for (int y = 5; y < SWAMP_MAP_H - 1; y++) {
            for (int x = 1; x < SWAMP_MAP_W - 11; x++) {
                TileType tile = saved_ground(g, map, level, x, y);
                if (!stonewake_crane_tile(tile) || map->tiles[y - 2][x + 4] == TILE_STONEWAKE_LEDGE) {
                    continue;
                }
                if (tile == TILE_STONEWAKE_CRANE_UP) {
                    set_saved_ground(g, map, level, x, y, TILE_STONEWAKE_CRANE_NEW);
                }
                TileType store = moved < count ? saved_ground(g, map, level, stores[moved][0], stores[moved][1]) : TILE_STONEWAKE_SUPPLIES;
                if (salvage_platform(map, x, y, store)) {
                    if (moved < count) {
                        set_saved_ground(g, map, level, stores[moved][0], stores[moved][1], TILE_GLASSDEEP_FLOOR);
                    }
                    moved++;
                }
                if (saved_ground(g, map, level, x + 10, y) == TILE_GLASSDEEP_RUIN || saved_ground(g, map, level, x + 10, y) == TILE_GLASSDEEP_FLOOR) {
                    set_saved_ground(g, map, level, x + 10, y, TILE_STONEWAKE_REMOTE);
                }
            }
        }
    }
}
