#include "game.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "../game/actions.h"

static void spawn_temple_enemies(GameState *g);

static int region_order_tier(const GameState *g) {
    const Location regions[4] = {
        LOCATION_DUNGEON, LOCATION_FOREST, LOCATION_MOUNTAINS, LOCATION_COAST
    };
    int tier = 0;
    for (int i = 0; i < 4; i++) {
        if (regions[i] != g->location &&
            (g->defeated_bosses & (1 << regions[i]))) {
            tier++;
        }
    }
    return tier > 3 ? 3 : tier;
}

static void scale_spawned_enemy(const GameState *g, Enemy *e) {
    static const int regular_hp[4] = {100, 145, 180, 200};
    static const int regular_attack[4] = {0, 7, 12, 15};
    static const int boss_hp[4] = {100, 130, 155, 170};
    static const int boss_attack[4] = {0, 6, 10, 13};
    // Temple base stats already assume the four regional bosses are defeated.
    int tier = g->location == LOCATION_TEMPLE ? 0 : region_order_tier(g);
    int level_progress = g->player.level - 1;
    if (level_progress < 0) {
        level_progress = 0;
    }
    if (level_progress > 16) {
        level_progress = 16;
    }
    // Later victories add smaller increments; leveling never adds a two-point attack jump.
    int hp_percent = (e->is_boss ? boss_hp[tier] : regular_hp[tier]) +
        20 * level_progress / 16;
    if (e->type != ENEMY_ILLUSION) {
        e->max_hp = (e->max_hp * hp_percent + 50) / 100;
        e->hp = e->max_hp;
    }
    e->attack += (e->is_boss ? boss_attack[tier] : regular_attack[tier]) +
        level_progress / 4;
    e->defense += tier;
    e->experience = (e->experience * (100 + 10 * tier) + 50) / 100;
}

static void spawn_enemy(GameState *g, Enemy *e, EnemyType type, int x, int y) {
    e->active  = 1;
    e->type    = type;
    e->x       = x;
    e->y       = y;
    e->is_boss = 0;
    e->dain_fragment = 0;
    e->frozen_turns = 0;
    e->move_timer = 0;
    switch (type) {
        case ENEMY_SKELETON:
            strncpy(e->name, "Skeleton", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 10; e->hp = 10;
            e->attack = 3;  e->defense = 0;
            e->experience = 8;
            break;
        case ENEMY_GOBLIN:
            strncpy(e->name, "Goblin", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 15; e->hp = 15;
            e->attack = 4;  e->defense = 1;
            e->experience = 10;
            break;
        case ENEMY_ZOMBIE:
            strncpy(e->name, "Zombie", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 22; e->hp = 22;
            e->attack = 6;  e->defense = 1;
            e->experience = 14;
            e->move_timer = 0;
            break;
        case ENEMY_CRYPT_BAT:
            strncpy(e->name, "Crypt Bat", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 7; e->hp = 7;
            e->attack = 4; e->defense = 0;
            e->experience = 10;
            break;
        case ENEMY_WRAITH:
            strncpy(e->name, "Wraith", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 18; e->hp = 18;
            e->attack = 7; e->defense = 2;
            e->experience = 22;
            break;
        case ENEMY_CRYPT_CONJURER:
            strncpy(e->name, "Crypt Conjurer", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 24; e->hp = 24;
            e->attack = 7; e->defense = 1;
            e->experience = 35;
            break;
        case ENEMY_PIXIE:
            strncpy(e->name, "Pixie", sizeof(e->name) - 1);
            e->max_hp = 7; e->hp = 7; e->attack = 4; e->defense = 0;
            e->experience = 9;
            break;
        case ENEMY_BLIGHTED_WOLF:
            strncpy(e->name, "Blighted Wolf", sizeof(e->name) - 1);
            e->max_hp = 14; e->hp = 14; e->attack = 6; e->defense = 1;
            e->experience = 14;
            break;
        case ENEMY_GIANT_SPIDER:
            strncpy(e->name, "Giant Spider", sizeof(e->name) - 1);
            e->max_hp = 18; e->hp = 18; e->attack = 7; e->defense = 2;
            e->experience = 20;
            break;
        case ENEMY_DARK_ELF:
            strncpy(e->name, "Dark Elf", sizeof(e->name) - 1);
            e->max_hp = 22; e->hp = 22; e->attack = 9; e->defense = 3;
            e->experience = 28;
            break;
        case ENEMY_GIANT_WURM:
            strncpy(e->name, "Giant Wurm", sizeof(e->name) - 1);
            e->max_hp = 38; e->hp = 38; e->attack = 12; e->defense = 5;
            e->experience = 42;
            break;
        case ENEMY_FOREST_TROLL:
            strncpy(e->name, "Forest Troll", sizeof(e->name) - 1);
            e->max_hp = 48; e->hp = 48; e->attack = 13; e->defense = 5;
            e->experience = 48;
            break;
        case ENEMY_FOREST_NECROMANCER:
            strncpy(e->name, "Necromancer", sizeof(e->name) - 1);
            e->max_hp = 165; e->hp = 165; e->attack = 18; e->defense = 6;
            e->experience = 450;
            e->is_boss = 1;
            break;
        case ENEMY_GOBLIN_SCOUT:
            strncpy(e->name, "Goblin Scout", 15);
            e->max_hp=12; e->hp=12; e->attack=5; e->defense=1; e->experience=12;
            break;
        case ENEMY_GOBLIN_ARCHER:
            strncpy(e->name, "Goblin Archer", 15);
            e->max_hp=16; e->hp=16; e->attack=7; e->defense=1; e->experience=19;
            break;
        case ENEMY_GOBLIN_BOMBER:
            strncpy(e->name, "Goblin Bomber", 15);
            e->max_hp=18; e->hp=18; e->attack=9; e->defense=1; e->experience=24;
            break;
        case ENEMY_TUNNEL_SPIDER:
            strncpy(e->name, "Tunnel Spider", 15);
            e->max_hp=20; e->hp=20; e->attack=8; e->defense=3; e->experience=25;
            break;
        case ENEMY_CAVE_TROLL:
            strncpy(e->name, "Cave Troll", 15);
            e->max_hp=52; e->hp=52; e->attack=14; e->defense=5; e->experience=52;
            break;
        case ENEMY_HOBGOBLIN_GUARD:
            strncpy(e->name, "Hobgoblin Guard", 15);
            e->max_hp=36; e->hp=36; e->attack=11; e->defense=7; e->experience=44;
            break;
        case ENEMY_GOBLIN_SHAMAN:
            strncpy(e->name, "Goblin Shaman", 15);
            e->max_hp=26; e->hp=26; e->attack=10; e->defense=2; e->experience=46;
            break;
        case ENEMY_MOUNTAIN_GOBLIN_KING:
            strncpy(e->name, "Goblin King", 15);
            e->max_hp=180; e->hp=180; e->attack=20; e->defense=8; e->experience=500;
            e->is_boss=1;
            break;
        case ENEMY_ILLUSION:
            strncpy(e->name, "Illusion", 15);
            e->max_hp = 1; e->hp = 1; e->attack = 7; e->defense = 0;
            e->experience = 12;
            break;
        case ENEMY_MERFOLK:
            strncpy(e->name, "Merfolk", 15);
            e->max_hp = 20; e->hp = 20; e->attack = 8; e->defense = 2;
            e->experience = 22;
            break;
        case ENEMY_SIREN:
            strncpy(e->name, "Siren", 15);
            e->max_hp = 18; e->hp = 18; e->attack = 9; e->defense = 1;
            e->experience = 30;
            break;
        case ENEMY_GIANT_CRAB:
            strncpy(e->name, "Giant Crab", 15);
            e->max_hp = 42; e->hp = 42; e->attack = 11; e->defense = 6;
            e->experience = 38;
            break;
        case ENEMY_ANIMATED_STATUE:
            strncpy(e->name, "Animated Statue", 15);
            e->max_hp = 58; e->hp = 58; e->attack = 14; e->defense = 8;
            e->experience = 56;
            break;
        case ENEMY_WATER_ELEMENTAL:
            strncpy(e->name, "Water Elemental", 15);
            e->max_hp = 34; e->hp = 34; e->attack = 13; e->defense = 3;
            e->experience = 48;
            break;
        case ENEMY_SEA_SERPENT:
            strncpy(e->name, "Sea Serpent", 15);
            e->max_hp = 40; e->hp = 40; e->attack = 15; e->defense = 4;
            e->experience = 62;
            break;
        case ENEMY_DROWNED_QUEEN:
            strncpy(e->name, "Drowned Queen", 15);
            e->max_hp = 220; e->hp = 220; e->attack = 22; e->defense = 8;
            e->experience = 650;
            e->is_boss = 1;
            break;
        case ENEMY_RELIC_SCARABS:
            strncpy(e->name, "Relic Scarabs", 15);
            e->max_hp = 32; e->hp = 32; e->attack = 13; e->defense = 3;
            e->experience = 55;
            break;
        case ENEMY_TEMPLE_STALKER:
            strncpy(e->name, "Temple Stalker", 15);
            e->max_hp = 48; e->hp = 48; e->attack = 17; e->defense = 5;
            e->experience = 85;
            break;
        case ENEMY_BLOWDART_HUNTER:
            strncpy(e->name, "Dart Hunter", 15);
            e->max_hp = 38; e->hp = 38; e->attack = 16; e->defense = 3;
            e->experience = 80;
            break;
        case ENEMY_VINEBOUND_GUARDIAN:
            strncpy(e->name, "Vine Guardian", 15);
            e->max_hp = 82; e->hp = 82; e->attack = 19; e->defense = 9;
            e->experience = 115;
            break;
        case ENEMY_SUN_PRIEST:
            strncpy(e->name, "Sun Priest", 15);
            e->max_hp = 52; e->hp = 52; e->attack = 18; e->defense = 4;
            e->experience = 105;
            break;
        case ENEMY_SERPENT_SPIRIT:
            strncpy(e->name, "Serpent Spirit", 15);
            e->max_hp = 58; e->hp = 58; e->attack = 19; e->defense = 5;
            e->experience = 115;
            break;
        case ENEMY_TREASURE_WRAITH:
            strncpy(e->name, "Treasure Wraith", 15);
            e->max_hp = 62; e->hp = 62; e->attack = 20; e->defense = 6;
            e->experience = 130;
            break;
        case ENEMY_LUNAR_EFFIGY:
            strncpy(e->name, "Lunar Effigy", 15);
            e->max_hp = 70; e->hp = 70; e->attack = 19; e->defense = 8;
            e->experience = 135;
            break;
        case ENEMY_MOONBOUND_SENTINEL:
            strncpy(e->name, "Moon Sentinel", 15);
            e->max_hp = 95; e->hp = 95; e->attack = 22; e->defense = 10;
            e->experience = 180;
            break;
        case ENEMY_FALLEN_SUN_GUARDIAN:
            strncpy(e->name, "Sun Guardian", 15);
            e->max_hp = 340; e->hp = 340; e->attack = 27; e->defense = 12;
            e->experience = 1000;
            e->is_boss = 1;
            break;
        case ENEMY_ORC:
            strncpy(e->name, "Orc", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 25; e->hp = 25;
            e->attack = 7;  e->defense = 2;
            e->experience = 20;
            break;
        case ENEMY_TROLL:
            strncpy(e->name, "Troll", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 40; e->hp = 40;
            e->attack = 10; e->defense = 4;
            e->experience = 30;
            break;
        case ENEMY_GIANT:
            strncpy(e->name, "Giant", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 60; e->hp = 60;
            e->attack = 14; e->defense = 6;
            e->experience = 50;
            break;
        case ENEMY_GOBLIN_KING:
            strncpy(e->name, "Goblin King", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 100; e->hp = 100;
            e->attack = 15;  e->defense = 5;
            e->experience = 200;
            e->is_boss = 1;
            break;
        case ENEMY_LICH_KING:
            strncpy(e->name, "Lich King", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 140; e->hp = 140;
            e->attack = 18;  e->defense = 6;
            e->experience = 400;
            e->is_boss = 1;
            break;
        case ENEMY_DEMON_LORD:
            strncpy(e->name, "Demon Lord", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 350; e->hp = 350;
            e->attack = 38;  e->defense = 15;
            e->experience = 700;
            e->is_boss = 1;
            break;
        case ENEMY_RED_DRAGON:
            strncpy(e->name, "Red Dragon", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 500; e->hp = 500;
            e->attack = 55;  e->defense = 22;
            e->experience = 1200;
            e->is_boss = 1;
            break;
        case ENEMY_TARRASQUE:
            strncpy(e->name, "Tarrasque", sizeof(e->name) - 1);
            e->name[sizeof(e->name) - 1] = '\0';
            e->max_hp = 800; e->hp = 800;
            e->attack = 80;  e->defense = 35;
            e->experience = 2000;
            e->is_boss = 1;
            break;
    }
    e->name[sizeof(e->name) - 1] = '\0';
    scale_spawned_enemy(g, e);
}

static int boss_for_level(const GameState *g, EnemyType *type) {
    int boss_level = DUNGEON_DEPTH;
    if (g->location == LOCATION_FOREST) {
        boss_level = FOREST_DEPTH;
    } else if (g->location == LOCATION_MOUNTAINS) {
        boss_level = MOUNTAIN_DEPTH;
    } else if (g->location == LOCATION_COAST) {
        boss_level = COAST_DEPTH;
    }
    if (g->level != boss_level) {
        return 0;
    }
    if (g->defeated_bosses & (1 << g->location)) {
        return 0;
    }
    if (g->location == LOCATION_FOREST) {
        *type = ENEMY_FOREST_NECROMANCER;
    } else if (g->location == LOCATION_MOUNTAINS) {
        *type = ENEMY_MOUNTAIN_GOBLIN_KING;
    } else if (g->location == LOCATION_COAST) {
        *type = ENEMY_DROWNED_QUEEN;
    } else {
        *type = ENEMY_LICH_KING;
    }
    return 1;
}

static int enemy_terrain_open(const GameState *g, int x, int y) {
    if (!map_is_walkable(&g->map, x, y) ||
        (g->map.tiles[y][x] != TILE_FLOOR &&
        g->map.tiles[y][x] != TILE_FOREST_FLOOR &&
        g->map.tiles[y][x] != TILE_MOUNTAIN_FLOOR &&
        g->map.tiles[y][x] != TILE_MOUNTAIN_BRIDGE &&
        g->map.tiles[y][x] != TILE_MOUNTAIN_CAVE_FLOOR &&
        g->map.tiles[y][x] != TILE_MOUNTAIN_FORTRESS_FLOOR &&
        g->map.tiles[y][x] != TILE_COAST_FLOOR &&
        g->map.tiles[y][x] != TILE_COAST_SHALLOW_WATER &&
        g->map.tiles[y][x] != TILE_COAST_DRAINED_WATER &&
        g->map.tiles[y][x] != TILE_COAST_CHANNEL_DRY)) {
        return 0;
    }
    return 1;
}

void game_repair_forest_enemy_positions(Map *m, Enemy *actors, int count, int px, int py) {
    unsigned char reachable[MAP_H][MAP_W] = {{0}};
    int queue[MAP_W * MAP_H];
    int head = 0;
    int tail = 0;
    int start_x = m->stairs_up_x;
    int start_y = m->stairs_up_y;
    if (!map_is_walkable(m, start_x, start_y)) {
        return;
    }
    reachable[start_y][start_x] = 1;
    queue[tail++] = start_y * MAP_W + start_x;
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    while (head < tail) {
        int cell = queue[head++];
        int x = cell % MAP_W;
        int y = cell / MAP_W;
        for (int side = 0; side < 4; side++) {
            int nx = x + dx[side];
            int ny = y + dy[side];
            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H ||
                reachable[ny][nx] || !map_is_walkable(m, nx, ny)) {
                continue;
            }
            reachable[ny][nx] = 1;
            queue[tail++] = ny * MAP_W + nx;
        }
    }

    for (int i = 0; i < count; i++) {
        Enemy *enemy = &actors[i];
        if (!enemy->active) {
            continue;
        }
        int overlaps_earlier = 0;
        for (int other = 0; other < i; other++) {
            if (actors[other].active && actors[other].x == enemy->x &&
                actors[other].y == enemy->y) {
                overlaps_earlier = 1;
                break;
            }
        }
        if (enemy->x >= 0 && enemy->x < MAP_W && enemy->y >= 0 &&
            enemy->y < MAP_H && reachable[enemy->y][enemy->x] &&
            !(enemy->x == start_x && enemy->y == start_y) &&
            !(enemy->x == px && enemy->y == py) && !overlaps_earlier) {
            continue;
        }
        int best_distance = 2 * (MAP_W + MAP_H) + 1;
        int best_x = -1;
        int best_y = -1;
        for (int y = 1; y < MAP_H - 1; y++) {
            for (int x = 1; x < MAP_W - 1; x++) {
                if (!reachable[y][x] || m->tiles[y][x] != TILE_FOREST_FLOOR ||
                    (x == start_x && y == start_y) ||
                    (x == px && y == py)) {
                    continue;
                }
                int occupied = 0;
                for (int other = 0; other < count; other++) {
                    if (other != i && actors[other].active &&
                        actors[other].x == x && actors[other].y == y) {
                        occupied = 1;
                        break;
                    }
                }
                int distance = abs(x - enemy->x) + abs(y - enemy->y);
                for (int side = 0; side < 4; side++) {
                    TileType neighbor = m->tiles[y + dy[side]][x + dx[side]];
                    if (neighbor == TILE_FOREST_WALL ||
                        neighbor == TILE_FOREST_HIDDEN_TRAIL) {
                        distance += MAP_W + MAP_H;
                        break;
                    }
                }
                if (!occupied && distance < best_distance) {
                    best_distance = distance;
                    best_x = x;
                    best_y = y;
                }
            }
        }
        if (best_x >= 0) {
            enemy->x = best_x;
            enemy->y = best_y;
        }
    }
}

static int enemy_tile_open(const GameState *g, int x, int y) {
    // Keep keys, stairs, traps, and portals visible and unobstructed.
    if (!enemy_terrain_open(g, x, y)) {
        return 0;
    }
    if (x == g->map.stairs_up_x && y == g->map.stairs_up_y) {
        return 0;
    }
    if (g->location == LOCATION_FOREST) {
        static const int dx[4] = {0, 1, 0, -1};
        static const int dy[4] = {-1, 0, 1, 0};
        for (int side = 0; side < 4; side++) {
            int nx = x + dx[side];
            int ny = y + dy[side];
            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H) {
                return 0;
            }
            TileType neighbor = g->map.tiles[ny][nx];
            if (neighbor == TILE_FOREST_WALL ||
                neighbor == TILE_FOREST_HIDDEN_TRAIL) {
                return 0;
            }
        }
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active &&
            g->enemies[i].x == x && g->enemies[i].y == y) {
            return 0;
        }
    }
    return 1;
}

static int find_enemy_tile_in_room(GameState *g, int room_index, int *x, int *y) {
    Room *room = &g->map.rooms[room_index];
    for (int ty = room->y + 1; ty < room->y + room->h - 1; ty++) {
        for (int tx = room->x + 1; tx < room->x + room->w - 1; tx++) {
            if (enemy_tile_open(g, tx, ty)) {
                *x = tx;
                *y = ty;
                return 1;
            }
        }
    }
    return 0;
}

static int find_enemy_tile(GameState *g, int *x, int *y, int room_limit) {
    for (int attempt = 0; attempt < 100; attempt++) {
        int room_idx = rand() % room_limit;
        Room *room = &g->map.rooms[room_idx];
        if (room->w < 3 || room->h < 3) {
            continue;
        }
        int tx = room->x + 1 + rand() % (room->w - 2);
        int ty = room->y + 1 + rand() % (room->h - 2);
        if (enemy_tile_open(g, tx, ty)) {
            *x = tx;
            *y = ty;
            return 1;
        }
    }

    for (int i = 0; i < room_limit; i++) {
        Room *room = &g->map.rooms[i];
        for (int ty = room->y + 1; ty < room->y + room->h - 1; ty++) {
            for (int tx = room->x + 1; tx < room->x + room->w - 1; tx++) {
                if (enemy_tile_open(g, tx, ty)) {
                    *x = tx;
                    *y = ty;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int spawn_into_open_tile(GameState *g, EnemyType type, int room_limit) {
    if (g->enemy_count >= MAX_ENEMIES) {
        return 0;
    }
    int x;
    int y;
    if (!find_enemy_tile(g, &x, &y, room_limit)) {
        return 0;
    }
    spawn_enemy(g, &g->enemies[g->enemy_count], type, x, y);
    g->enemy_count++;
    return 1;
}

static Enemy *spawn_quest_enemy_at(GameState *g, EnemyType type, int x, int y) {
    int slot = -1;
    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].active) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        if (g->enemy_count >= MAX_ENEMIES) {
            return NULL;
        }
        slot = g->enemy_count++;
    }
    spawn_enemy(g, &g->enemies[slot], type, x, y);
    return &g->enemies[slot];
}

static Enemy *spawn_quest_enemy_open(GameState *g, EnemyType type, int room_limit) {
    int x;
    int y;
    if (!find_enemy_tile(g, &x, &y, room_limit)) {
        return NULL;
    }
    return spawn_quest_enemy_at(g, type, x, y);
}

static void spawn_quest_enemy_near(GameState *g, EnemyType type, int center_x, int center_y) {
    for (int radius = 1; radius <= 4; radius++) {
        for (int y = center_y - radius; y <= center_y + radius; y++) {
            for (int x = center_x - radius; x <= center_x + radius; x++) {
                if ((x != center_x - radius && x != center_x + radius &&
                    y != center_y - radius && y != center_y + radius) ||
                    !enemy_tile_open(g, x, y)) {
                    continue;
                }
                spawn_quest_enemy_at(g, type, x, y);
                return;
            }
        }
    }
}

static int place_dain_map_bearer(GameState *g) {
    if (g->location != LOCATION_MOUNTAINS || g->dain_quest_state != 1) {
        return 0;
    }
    EnemyType target_type;
    int target_bit;
    if (g->level == 2) {
        target_type = ENEMY_GOBLIN_ARCHER;
        target_bit = DAIN_FRAGMENT_ARCHER;
    } else if (g->level == 3) {
        target_type = ENEMY_GOBLIN_BOMBER;
        target_bit = DAIN_FRAGMENT_BOMBER;
    } else if (g->level == 5) {
        target_type = ENEMY_GOBLIN_SHAMAN;
        target_bit = DAIN_FRAGMENT_SHAMAN;
    } else {
        return 0;
    }
    if (g->dain_map_fragments & target_bit) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active &&
            g->enemies[i].dain_fragment == target_bit) {
            return 0;
        }
    }
    int room_limit = g->level == MOUNTAIN_DEPTH
        ? g->map.room_count - 1 : g->map.room_count;
    Enemy *target = spawn_quest_enemy_open(g, target_type, room_limit);
    if (!target) {
        return 0;
    }
    target->dain_fragment = target_bit;
    target->max_hp = target->max_hp * 3 / 2;
    target->hp = target->max_hp;
    target->attack += 2;
    strncpy(target->name, "Map Bearer", sizeof(target->name) - 1);
    target->name[sizeof(target->name) - 1] = '\0';
    EnemyType guard = g->level == 2 ? ENEMY_GOBLIN_SCOUT :
        (g->level == 3 ? ENEMY_TUNNEL_SPIDER : ENEMY_HOBGOBLIN_GUARD);
    spawn_quest_enemy_near(g, guard, target->x, target->y);
    spawn_quest_enemy_near(g, guard, target->x, target->y);
    return 1;
}

static int quest_group_pending(const GameState *g) {
    if (g->location == LOCATION_DUNGEON && g->elowen_quest_state == 1) {
        int bit = g->level == 2 ? 1 : (g->level == 4 ? 2 :
            (g->level == 6 ? 4 : 0));
        if (!bit) {
            return 0;
        }
        return !(g->elowen_seals_restored & bit);
    }
    if (g->location == LOCATION_FOREST && g->alder_quest_state == 1) {
        int bit = g->level == 2 ? ALDER_WARDEN_STAGE_2 :
            (g->level == 5 ? ALDER_WARDEN_STAGE_5 :
            (g->level == 7 ? ALDER_WARDEN_STAGE_7 : 0));
        if (!bit) {
            return 0;
        }
        return !(g->alder_wardens_rescued & bit);
    }
    if (g->location == LOCATION_COAST && g->mara_quest_state == 1) {
        int bit = g->level == 1 ? MARA_BEACON_STAGE_1 :
            (g->level == 3 ? MARA_BEACON_STAGE_3 :
            (g->level == 6 ? MARA_BEACON_STAGE_6 : 0));
        if (!bit) {
            return 0;
        }
        return !(g->mara_beacons_lit & bit);
    }
    return 0;
}

void enemies_spawn(GameState *g) {
    g->enemy_count = 0;
    if (g->map.room_count == 0) {
        return;
    }

    int order_tier = region_order_tier(g);
    int num_enemies = 10 + g->level;
    if (quest_group_pending(g)) {
        num_enemies -= 3;
    }
    if (num_enemies > MAX_ENEMIES) {
        num_enemies = MAX_ENEMIES;
    }

    EnemyType boss_type;
    if (boss_for_level(g, &boss_type)) {
        int boss_x = g->map.stairs_down_x + 1;
        int boss_y = g->map.stairs_down_y;
        if (g->location == LOCATION_FOREST ||
            g->location == LOCATION_MOUNTAINS ||
            g->location == LOCATION_COAST) {
            map_room_center(&g->map.rooms[g->map.room_count - 1],
                &boss_x, &boss_y);
        }
        if (!enemy_tile_open(g, boss_x, boss_y)) {
            find_enemy_tile_in_room(g, g->map.room_count - 1,
                &boss_x, &boss_y);
        }
        if (enemy_tile_open(g, boss_x, boss_y)) {
            spawn_enemy(g, &g->enemies[g->enemy_count++], boss_type,
                boss_x, boss_y);
        }
    }

    int boss_level = g->location == LOCATION_FOREST ? FOREST_DEPTH :
        (g->location == LOCATION_MOUNTAINS ? MOUNTAIN_DEPTH :
        (g->location == LOCATION_COAST ? COAST_DEPTH : DUNGEON_DEPTH));
    int regular_room_limit = g->level == boss_level
        ? g->map.room_count - 1 : g->map.room_count;
    place_dain_map_bearer(g);
    if (g->location == LOCATION_COAST) {
        for (int y = 1; y < MAP_H - 1; y++) {
            for (int x = 2; x < MAP_W - 1; x++) {
                if (g->map.tiles[y][x] == TILE_COAST_CACHE &&
                    g->enemy_count < num_enemies && enemy_tile_open(g, x - 2, y)) {
                    EnemyType guard = g->level >= 6 ? ENEMY_SEA_SERPENT :
                        (g->level >= 3 ? ENEMY_ANIMATED_STATUE : ENEMY_GIANT_CRAB);
                    spawn_enemy(g, &g->enemies[g->enemy_count++], guard, x - 2, y);
                }
            }
        }
    }
    while (g->enemy_count < num_enemies) {
        EnemyType type;
        int roll = rand() % 100;
        // Completed regions advance enemy roles without skipping map stages.
        int level = g->level + 3 * order_tier;

        if (g->location == LOCATION_COAST) {
            if (level == 1) {
                type = roll < 55 ? ENEMY_ILLUSION : ENEMY_MERFOLK;
            } else if (level == 2) {
                type = roll < 30 ? ENEMY_ILLUSION :
                    (roll < 70 ? ENEMY_MERFOLK : ENEMY_GIANT_CRAB);
            } else if (level == 3) {
                type = roll < 25 ? ENEMY_MERFOLK :
                    (roll < 55 ? ENEMY_SIREN :
                    (roll < 80 ? ENEMY_GIANT_CRAB : ENEMY_ILLUSION));
            } else if (level == 4) {
                type = roll < 25 ? ENEMY_SIREN :
                    (roll < 50 ? ENEMY_GIANT_CRAB :
                    (roll < 75 ? ENEMY_ANIMATED_STATUE : ENEMY_MERFOLK));
            } else if (level == 5) {
                type = roll < 25 ? ENEMY_GIANT_CRAB :
                    (roll < 50 ? ENEMY_ANIMATED_STATUE :
                    (roll < 75 ? ENEMY_WATER_ELEMENTAL : ENEMY_SIREN));
            } else {
                type = roll < 15 ? ENEMY_MERFOLK :
                    (roll < 30 ? ENEMY_SIREN :
                    (roll < 45 ? ENEMY_GIANT_CRAB :
                    (roll < 62 ? ENEMY_ANIMATED_STATUE :
                    (roll < 82 ? ENEMY_WATER_ELEMENTAL : ENEMY_SEA_SERPENT))));
            }
        } else if (g->location == LOCATION_MOUNTAINS) {
            if (level == 1)
                type = ENEMY_GOBLIN_SCOUT;
            else if (level == 2)
                type = roll < 55 ? ENEMY_GOBLIN_SCOUT : ENEMY_GOBLIN_ARCHER;
            else if (level == 3)
                type = roll < 25 ? ENEMY_GOBLIN_SCOUT :
                    (roll < 50 ? ENEMY_GOBLIN_ARCHER :
                    (roll < 75 ? ENEMY_GOBLIN_BOMBER : ENEMY_TUNNEL_SPIDER));
            else if (level == 4)
                type = roll < 20 ? ENEMY_GOBLIN_ARCHER :
                    (roll < 40 ? ENEMY_GOBLIN_BOMBER :
                    (roll < 60 ? ENEMY_TUNNEL_SPIDER :
                    (roll < 80 ? ENEMY_CAVE_TROLL : ENEMY_HOBGOBLIN_GUARD)));
            else
                type = roll < 15 ? ENEMY_GOBLIN_ARCHER :
                    (roll < 30 ? ENEMY_GOBLIN_BOMBER :
                    (roll < 45 ? ENEMY_TUNNEL_SPIDER :
                    (roll < 62 ? ENEMY_CAVE_TROLL :
                    (roll < 82 ? ENEMY_HOBGOBLIN_GUARD : ENEMY_GOBLIN_SHAMAN))));
        } else if (g->location == LOCATION_FOREST) {
            if (level == 1)
                type = roll < 55 ? ENEMY_PIXIE : ENEMY_BLIGHTED_WOLF;
            else if (level == 2)
                type = roll < 30 ? ENEMY_PIXIE :
                    (roll < 65 ? ENEMY_BLIGHTED_WOLF : ENEMY_GIANT_SPIDER);
            else if (level == 3)
                type = roll < 20 ? ENEMY_PIXIE :
                    (roll < 45 ? ENEMY_BLIGHTED_WOLF :
                    (roll < 70 ? ENEMY_GIANT_SPIDER : ENEMY_DARK_ELF));
            else if (level == 4)
                type = roll < 20 ? ENEMY_GIANT_SPIDER :
                    (roll < 45 ? ENEMY_DARK_ELF :
                    (roll < 70 ? ENEMY_GIANT_WURM : ENEMY_FOREST_TROLL));
            else
                type = roll < 15 ? ENEMY_PIXIE :
                    (roll < 30 ? ENEMY_BLIGHTED_WOLF :
                    (roll < 45 ? ENEMY_GIANT_SPIDER :
                    (roll < 65 ? ENEMY_DARK_ELF :
                    (roll < 82 ? ENEMY_GIANT_WURM : ENEMY_FOREST_TROLL))));
        } else if (level == 1) {
            type = ENEMY_SKELETON;
        } else if (level == 2) {
            if (roll < 50) type = ENEMY_SKELETON;
            else if (roll < 80) type = ENEMY_ZOMBIE;
            else type = ENEMY_CRYPT_BAT;
        } else if (level == 3) {
            if (roll < 30) type = ENEMY_SKELETON;
            else if (roll < 65) type = ENEMY_ZOMBIE;
            else if (roll < 85) type = ENEMY_CRYPT_BAT;
            else type = ENEMY_WRAITH;
        } else if (level == 4) {
            if (roll < 20) type = ENEMY_SKELETON;
            else if (roll < 55) type = ENEMY_ZOMBIE;
            else if (roll < 70) type = ENEMY_CRYPT_BAT;
            else if (roll < 90) type = ENEMY_WRAITH;
            else type = ENEMY_CRYPT_CONJURER;
        } else {
            if (roll < 15) type = ENEMY_SKELETON;
            else if (roll < 45) type = ENEMY_ZOMBIE;
            else if (roll < 60) type = ENEMY_CRYPT_BAT;
            else if (roll < 80) type = ENEMY_WRAITH;
            else type = ENEMY_CRYPT_CONJURER;
        }
        if (!spawn_into_open_tile(g, type, regular_room_limit)) {
            break;
        }
    }
    if (g->location == LOCATION_FOREST) {
        game_repair_forest_enemy_positions(&g->map, g->enemies,
            g->enemy_count, g->map.stairs_up_x, g->map.stairs_up_y);
    }
}

void game_init(GameState *g) {
    srand((unsigned)time(NULL));
    g->level = 1;
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        g->level_cache[i].valid = 0;
        g->forest_cache[i].valid = 0;
        g->mountain_cache[i].valid = 0;
        g->coast_cache[i].valid = 0;
    }
    g->message_count = 0;
    g->level_cleared = 0;
    g->max_level_reached = 1;
    g->max_forest_level_reached = 1;
    g->max_mountain_level_reached = 1;
    g->max_coast_level_reached = 1;
    g->max_temple_level_reached = 1;
    g->location = LOCATION_TOWN;
    int spawn_x, spawn_y;
    map_generate_town(&g->map, &spawn_x, &spawn_y);
    g->player.x = spawn_x;
    g->player.y = spawn_y;
    g->player.name[0] = '\0';
    g->player.level = 1;
    g->player.experience = 0;
    g->player.experience_next = 100;
    g->inventory_count = 0;
    g->equipped_main_hand = -1;
    g->equipped_off_hand = -1;
    g->equipped_armor = -1;
    g->gold = 0;
    g->score = 0;
    g->dungeon_key_found = 0;
    g->dungeon_crypt_keys = 0;
    g->portal_active = 0;
    g->portal_level = 0;
    g->portal_location = LOCATION_DUNGEON;
    g->portal_x = 0;
    g->portal_y = 0;
    g->portal_origin_tile = TILE_FLOOR;
    g->defeated_bosses = 0;
    g->elowen_quest_state = 0;
    g->elowen_seals_restored = 0;
    g->dain_quest_state = 0;
    g->dain_map_fragments = 0;
    g->alder_quest_state = 0;
    g->alder_wardens_rescued = 0;
    g->mara_quest_state = 0;
    g->mara_beacons_lit = 0;
    g->cain_scroll_given = 0;
    for (int i = 0; i < TEMPLE_DEPTH; i++) {
        g->temple_cache[i].valid = 0;
    }
    g->temple_alignment = 0;
    g->temple_sentinels_awakened = 0;
    g->temple_treasure_state = 0;
    g->dialogue_active = 0;
    g->dialogue_speaker[0] = '\0';
    g->dialogue_text[0] = '\0';
    g->dialogue_x = 0;
    g->dialogue_y = 0;
    g->floor_item_count = 0;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        g->inventory[i].active = 0;
    }
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        g->floor_items[i].active = 0;
    }

    g->player.known_spell_count = 0;
    g->player.equipped_spell = -1;
    g->player.last_dx = 0;
    g->player.last_dy = 0;
    g->player.poison_turns = 0;
    g->trail_count = 0;
    g->trail_frames = 0;
    g->trail_effect = TRAIL_EFFECT_GENERIC;
    g->trail_started_at = 0;

    switch (g->player.player_class) {
        case CLASS_WARRIOR:
            g->player.max_hp = 150;
            g->player.max_mp = 20;
            g->player.attack = 14;
            g->player.defense = 6;
            g->inventory[g->inventory_count++] = item_make_rusty_sword();
            break;
        case CLASS_MAGE:
            g->player.max_hp = 70;
            g->player.max_mp = 100;
            g->player.attack = 4;
            g->player.defense = 2;
            g->inventory[g->inventory_count++] = item_make_staff();
            g->inventory[g->inventory_count++] = item_make_scroll_magic_arrow();
            break;
        case CLASS_ROGUE:
            g->player.max_hp = 100;
            g->player.max_mp = 40;
            g->player.attack = 10;
            g->player.defense = 4;
            g->inventory[g->inventory_count++] = item_make_bow();
            break;
    }
    g->player.hp = g->player.max_hp;
    g->player.mp = g->player.max_mp;

    // Spawn enemies in random rooms
    enemies_spawn(g);
}

void game_move_player(GameState *g, int dx, int dy) {
    int nx = g->player.x + dx;
    int ny = g->player.y + dy;
    if (!map_is_walkable(&g->map, nx, ny)) {
        return;
    }
    g->player.x = nx;
    g->player.y = ny;
    g->dialogue_active = 0;
}

static int repaired_equipment_index(const GameState *g, int index, ItemType type) {
    if (index < 0) {
        return -1;
    }
    int start = index < g->inventory_count ? index : g->inventory_count - 1;
    for (int i = start; i >= 0; i--) {
        if (g->inventory[i].type == type) {
            return i;
        }
    }
    for (int i = start + 1; i < g->inventory_count; i++) {
        if (g->inventory[i].type == type) {
            return i;
        }
    }
    return -1;
}

static int valid_off_hand_item(const Item *item) {
    return item->type == ITEM_SHIELD ||
        (item->type == ITEM_WEAPON &&
        item->weapon_hands == WEAPON_HANDS_ONE);
}

void game_repair_equipment_indices(GameState *g) {
    if (g->equipped_main_hand >= g->inventory_count ||
        (g->equipped_main_hand >= 0 &&
        g->inventory[g->equipped_main_hand].type != ITEM_WEAPON)) {
        g->equipped_main_hand = repaired_equipment_index(g,
            g->equipped_main_hand, ITEM_WEAPON);
    }
    if (g->equipped_off_hand >= g->inventory_count ||
        (g->equipped_off_hand >= 0 &&
        !valid_off_hand_item(&g->inventory[g->equipped_off_hand]))) {
        g->equipped_off_hand = -1;
    }
    if (g->equipped_off_hand == g->equipped_main_hand) {
        g->equipped_off_hand = -1;
    }
    if (g->equipped_off_hand >= 0 &&
        (g->equipped_main_hand < 0 ||
        g->inventory[g->equipped_main_hand].weapon_hands == WEAPON_HANDS_TWO ||
        !valid_off_hand_item(&g->inventory[g->equipped_off_hand]))) {
        g->equipped_off_hand = -1;
    }
    if (g->equipped_armor >= g->inventory_count ||
        (g->equipped_armor >= 0 &&
        g->inventory[g->equipped_armor].type != ITEM_ARMOR)) {
        g->equipped_armor = repaired_equipment_index(g,
            g->equipped_armor, ITEM_ARMOR);
    }
}

int game_off_hand_attack_bonus(const Item *weapon) {
    return (weapon->attack_bonus + 1) / 2;
}

void game_unequip_main_hand(GameState *g) {
    if (g->equipped_main_hand < 0 ||
        g->equipped_main_hand >= g->inventory_count) {
        g->equipped_main_hand = -1;
        return;
    }
    Item *weapon = &g->inventory[g->equipped_main_hand];
    g->player.attack -= weapon->attack_bonus;
    g->player.max_mp -= weapon->max_mp_bonus;
    if (g->player.mp > g->player.max_mp) {
        g->player.mp = g->player.max_mp;
    }
    g->equipped_main_hand = -1;
}

void game_unequip_off_hand(GameState *g) {
    if (g->equipped_off_hand < 0 ||
        g->equipped_off_hand >= g->inventory_count) {
        g->equipped_off_hand = -1;
        return;
    }
    Item *item = &g->inventory[g->equipped_off_hand];
    if (item->type == ITEM_WEAPON) {
        g->player.attack -= game_off_hand_attack_bonus(item);
    } else if (item->type == ITEM_SHIELD) {
        g->player.defense -= item->defense_bonus;
    }
    g->equipped_off_hand = -1;
}

int game_equip_off_hand(GameState *g, int index) {
    if (index < 0 || index >= g->inventory_count ||
        g->equipped_main_hand < 0 ||
        g->equipped_main_hand >= g->inventory_count ||
        index == g->equipped_main_hand) {
        return 0;
    }
    Item *main_hand = &g->inventory[g->equipped_main_hand];
    Item *weapon = &g->inventory[index];
    if (main_hand->weapon_hands != WEAPON_HANDS_ONE ||
        weapon->type != ITEM_WEAPON ||
        weapon->weapon_hands != WEAPON_HANDS_ONE ||
        !item_class_allowed(weapon, g->player.player_class)) {
        return 0;
    }
    game_unequip_off_hand(g);
    g->equipped_off_hand = index;
    g->player.attack += game_off_hand_attack_bonus(weapon);
    return 1;
}

int game_equip_shield(GameState *g, int index) {
    if (index < 0 || index >= g->inventory_count ||
        g->equipped_main_hand < 0 ||
        g->equipped_main_hand >= g->inventory_count) {
        return 0;
    }
    Item *main_hand = &g->inventory[g->equipped_main_hand];
    Item *shield = &g->inventory[index];
    if (main_hand->weapon_hands != WEAPON_HANDS_ONE ||
        shield->type != ITEM_SHIELD ||
        !item_class_allowed(shield, g->player.player_class)) {
        return 0;
    }
    game_unequip_off_hand(g);
    g->equipped_off_hand = index;
    g->player.defense += shield->defense_bonus;
    return 1;
}

int game_equip_main_hand(GameState *g, int index) {
    if (index < 0 || index >= g->inventory_count) {
        return 0;
    }
    Item *weapon = &g->inventory[index];
    if (weapon->type != ITEM_WEAPON ||
        !item_class_allowed(weapon, g->player.player_class)) {
        return 0;
    }
    if (g->equipped_off_hand == index ||
        weapon->weapon_hands == WEAPON_HANDS_TWO) {
        game_unequip_off_hand(g);
    }
    game_unequip_main_hand(g);
    g->equipped_main_hand = index;
    g->player.attack += weapon->attack_bonus;
    g->player.max_mp += weapon->max_mp_bonus;
    return 1;
}

void game_remove_inventory_item(GameState *g, int index) {
    if (index < 0 || index >= g->inventory_count) {
        return;
    }
    Item *item = &g->inventory[index];
    int removing_main_hand = g->equipped_main_hand == index;
    int promote_off_hand = removing_main_hand &&
        g->equipped_off_hand >= 0 &&
        g->inventory[g->equipped_off_hand].type == ITEM_WEAPON;
    if (g->equipped_main_hand == index) {
        game_unequip_main_hand(g);
    }
    if (promote_off_hand) {
        int promoted = g->equipped_off_hand;
        game_unequip_off_hand(g);
        g->equipped_main_hand = promoted;
        g->player.attack += g->inventory[promoted].attack_bonus;
    } else if (removing_main_hand && g->equipped_off_hand >= 0) {
        game_unequip_off_hand(g);
    } else if (g->equipped_off_hand == index) {
        game_unequip_off_hand(g);
    }
    if (g->equipped_armor == index) {
        game_remove_armor_bonuses(g, item);
        g->equipped_armor = -1;
    }
    for (int i = index; i < g->inventory_count - 1; i++) {
        g->inventory[i] = g->inventory[i + 1];
    }
    g->inventory_count--;
    if (g->equipped_main_hand > index) {
        g->equipped_main_hand--;
    }
    if (g->equipped_off_hand > index) {
        g->equipped_off_hand--;
    }
    if (g->equipped_armor > index) {
        g->equipped_armor--;
    }
}

void game_apply_armor_bonuses(GameState *g, const Item *armor) {
    g->player.defense += armor->defense_bonus;
    g->player.max_hp += armor->max_hp_bonus;
    g->player.hp += armor->max_hp_bonus;
    g->player.max_mp += armor->max_mp_bonus;
    g->player.mp += armor->max_mp_bonus;
}

void game_remove_armor_bonuses(GameState *g, const Item *armor) {
    g->player.defense -= armor->defense_bonus;
    g->player.max_hp -= armor->max_hp_bonus;
    if (g->player.hp > g->player.max_hp) {
        g->player.hp = g->player.max_hp;
    }
    g->player.max_mp -= armor->max_mp_bonus;
    if (g->player.mp > g->player.max_mp) {
        g->player.mp = g->player.max_mp;
    }
}

static LevelCache *active_cache(GameState *g) {
    if (g->location == LOCATION_FOREST) {
        return g->forest_cache;
    }
    if (g->location == LOCATION_MOUNTAINS) {
        return g->mountain_cache;
    }
    if (g->location == LOCATION_COAST) {
        return g->coast_cache;
    }
    if (g->location == LOCATION_TEMPLE) {
        return g->temple_cache;
    }
    return g->level_cache;
}

static int *active_max_level(GameState *g) {
    if (g->location == LOCATION_FOREST) return &g->max_forest_level_reached;
    if (g->location == LOCATION_MOUNTAINS)
        return &g->max_mountain_level_reached;
    if (g->location == LOCATION_COAST) {
        return &g->max_coast_level_reached;
    }
    if (g->location == LOCATION_TEMPLE) {
        return &g->max_temple_level_reached;
    }
    return &g->max_level_reached;
}

static int active_depth(const GameState *g) {
    if (g->location == LOCATION_FOREST) {
        return FOREST_DEPTH;
    }
    if (g->location == LOCATION_MOUNTAINS) {
        return MOUNTAIN_DEPTH;
    }
    if (g->location == LOCATION_COAST) {
        return COAST_DEPTH;
    }
    if (g->location == LOCATION_TEMPLE) {
        return TEMPLE_DEPTH;
    }
    return DUNGEON_DEPTH;
}

static int place_elowen_seal(GameState *g) {
    if (g->location != LOCATION_DUNGEON || g->elowen_quest_state != 1) {
        return 0;
    }
    int seal_index = -1;
    if (g->level == 2) {
        seal_index = 0;
    } else if (g->level == 4) {
        seal_index = 1;
    } else if (g->level == 6) {
        seal_index = 2;
    }
    if (seal_index < 0 || g->map.room_count < 2) {
        return 0;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_BROKEN_BURIAL_SEAL ||
                g->map.tiles[y][x] == TILE_RESTORED_BURIAL_SEAL) {
                return 0;
            }
        }
    }
    Room *room = &g->map.rooms[g->map.room_count / 2];
    int x;
    int y;
    map_room_center(room, &x, &y);
    g->map.tiles[y][x] =
        (g->elowen_seals_restored & (1 << seal_index))
        ? TILE_RESTORED_BURIAL_SEAL : TILE_BROKEN_BURIAL_SEAL;
    return !(g->elowen_seals_restored & (1 << seal_index));
}

static void spawn_elowen_guardians(GameState *g) {
    EnemyType primary = g->level == 2 ? ENEMY_SKELETON :
        (g->level == 4 ? ENEMY_WRAITH : ENEMY_CRYPT_CONJURER);
    EnemyType support = g->level == 2 ? ENEMY_CRYPT_BAT :
        (g->level == 4 ? ENEMY_SKELETON : ENEMY_WRAITH);
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] != TILE_BROKEN_BURIAL_SEAL) {
                continue;
            }
            spawn_quest_enemy_near(g, primary, x, y);
            spawn_quest_enemy_near(g, support, x, y);
            spawn_quest_enemy_near(g, support, x, y);
            return;
        }
    }
}

static int alder_warden_bit(int level) {
    if (level == 2) {
        return ALDER_WARDEN_STAGE_2;
    } else if (level == 5) {
        return ALDER_WARDEN_STAGE_5;
    } else if (level == 7) {
        return ALDER_WARDEN_STAGE_7;
    }
    return 0;
}

static int place_alder_warden(GameState *g) {
    if (g->location != LOCATION_FOREST || g->alder_quest_state != 1) {
        return 0;
    }
    int bit = alder_warden_bit(g->level);
    if (!bit || (g->alder_wardens_rescued & bit)) {
        return 0;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_FOREST_WARDEN) {
                return 0;
            }
        }
    }
    int room_index = g->level == 2 ? 7 : (g->level == 5 ? 8 : 5);
    if (room_index >= g->map.room_count) {
        return 0;
    }
    int x;
    int y;
    Room *room = &g->map.rooms[room_index];
    map_room_center(room, &x, &y);
    if (!enemy_tile_open(g, x, y)) {
        int found = 0;
        for (int candidate_y = room->y + 1;
            candidate_y < room->y + room->h - 1 && !found; candidate_y++) {
            for (int candidate_x = room->x + 1;
                candidate_x < room->x + room->w - 1; candidate_x++) {
                if (enemy_tile_open(g, candidate_x, candidate_y)) {
                    x = candidate_x;
                    y = candidate_y;
                    found = 1;
                    break;
                }
            }
        }
        if (!found) {
            return 0;
        }
    }
    g->map.tiles[y][x] = TILE_FOREST_WARDEN;
    return 1;
}

static void spawn_alder_guardian(GameState *g) {
    if (g->location != LOCATION_FOREST) {
        return;
    }
    EnemyType primary;
    EnemyType support;
    if (g->level == 2) {
        primary = ENEMY_GIANT_SPIDER;
        support = ENEMY_BLIGHTED_WOLF;
    } else if (g->level == 5) {
        primary = ENEMY_DARK_ELF;
        support = ENEMY_PIXIE;
    } else if (g->level == 7) {
        primary = ENEMY_FOREST_TROLL;
        support = ENEMY_BLIGHTED_WOLF;
    } else {
        return;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] != TILE_FOREST_WARDEN) {
                continue;
            }
            spawn_quest_enemy_near(g, primary, x, y);
            spawn_quest_enemy_near(g, support, x, y);
            spawn_quest_enemy_near(g, support, x, y);
            return;
        }
    }
}

static int mara_beacon_bit(int level) {
    if (level == 1) {
        return MARA_BEACON_STAGE_1;
    } else if (level == 3) {
        return MARA_BEACON_STAGE_3;
    } else if (level == 6) {
        return MARA_BEACON_STAGE_6;
    }
    return 0;
}

static int place_mara_beacon(GameState *g) {
    if (g->location != LOCATION_COAST || g->mara_quest_state == 0) {
        return 0;
    }
    int bit = mara_beacon_bit(g->level);
    if (!bit) {
        return 0;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_COAST_BEACON_UNLIT ||
                g->map.tiles[y][x] == TILE_COAST_BEACON_LIT) {
                return 0;
            }
        }
    }
    int room_index = g->level == 1 ? 1 : (g->level == 3 ? 3 : 8);
    if (room_index >= g->map.room_count) {
        return 0;
    }
    int x;
    int y;
    map_room_center(&g->map.rooms[room_index], &x, &y);
    if (g->level == 6) {
        int tide_is_high = 0;
        for (int map_y = 0; map_y < MAP_H && !tide_is_high; map_y++) {
            for (int map_x = 0; map_x < MAP_W; map_x++) {
                if (g->map.tiles[map_y][map_x] == TILE_COAST_DEEP_WATER) {
                    tide_is_high = 1;
                    break;
                }
            }
        }
        for (int offset_y = -1; offset_y <= 1; offset_y++) {
            for (int offset_x = -1; offset_x <= 1; offset_x++) {
                int water_x = x + offset_x;
                int water_y = y + offset_y;
                if ((offset_x != 0 || offset_y != 0) &&
                    (g->map.tiles[water_y][water_x] == TILE_COAST_FLOOR ||
                    g->map.tiles[water_y][water_x] ==
                        TILE_COAST_SHALLOW_WATER)) {
                    g->map.tiles[water_y][water_x] = tide_is_high
                        ? TILE_COAST_DEEP_WATER : TILE_COAST_DRAINED_WATER;
                }
            }
        }
    }
    g->map.tiles[y][x] = (g->mara_beacons_lit & bit)
        ? TILE_COAST_BEACON_LIT : TILE_COAST_BEACON_UNLIT;
    return !(g->mara_beacons_lit & bit);
}

static void spawn_mara_guardian(GameState *g) {
    if (g->location != LOCATION_COAST) {
        return;
    }
    EnemyType primary;
    EnemyType support;
    if (g->level == 1) {
        primary = ENEMY_GIANT_CRAB;
        support = ENEMY_MERFOLK;
    } else if (g->level == 3) {
        primary = ENEMY_ANIMATED_STATUE;
        support = ENEMY_GIANT_CRAB;
    } else if (g->level == 6) {
        primary = ENEMY_SEA_SERPENT;
        support = ENEMY_WATER_ELEMENTAL;
    } else {
        return;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] != TILE_COAST_BEACON_UNLIT) {
                continue;
            }
            spawn_quest_enemy_near(g, primary, x, y);
            spawn_quest_enemy_near(g, support, x, y);
            spawn_quest_enemy_near(g, support, x, y);
            return;
        }
    }
}

void game_refresh_quest_encounters(GameState *g) {
    int seal_placed = place_elowen_seal(g);
    int warden_placed = place_alder_warden(g);
    int beacon_placed = place_mara_beacon(g);
    place_dain_map_bearer(g);
    if (seal_placed) {
        spawn_elowen_guardians(g);
    }
    if (warden_placed) {
        spawn_alder_guardian(g);
    }
    if (beacon_placed) {
        spawn_mara_guardian(g);
    }
}

static void generate_active_level(GameState *g) {
    if (g->location == LOCATION_FOREST) {
        map_generate_forest(&g->map, g->level);
    } else if (g->location == LOCATION_MOUNTAINS) {
        map_generate_mountains(&g->map, g->level);
    } else if (g->location == LOCATION_COAST) {
        map_generate_coast(&g->map, g->level);
    } else if (g->location == LOCATION_TEMPLE) {
        int spawn_x;
        int spawn_y;
        map_generate_temple(&g->map, g->level, &spawn_x, &spawn_y);
    } else {
        map_generate(&g->map, g->level);
    }
    g->enemy_count = 0;
    if (g->location == LOCATION_TEMPLE) {
        g->temple_alignment = 0;
        g->temple_sentinels_awakened = 0;
        spawn_temple_enemies(g);
        game_update_level_progress(g);
        return;
    }
    int seal_placed = place_elowen_seal(g);
    int warden_placed = place_alder_warden(g);
    int beacon_placed = place_mara_beacon(g);
    enemies_spawn(g);
    if (seal_placed) {
        spawn_elowen_guardians(g);
    }
    if (warden_placed) {
        spawn_alder_guardian(g);
    }
    if (beacon_placed) {
        spawn_mara_guardian(g);
    }
    game_update_level_progress(g);
}

static void sync_temple_floor_state(GameState *g) {
    if (g->location != LOCATION_TEMPLE) {
        return;
    }
    int guardian_defeated =
        g->defeated_bosses & (1 << LOCATION_TEMPLE);
    g->temple_alignment = 0;
    g->temple_sentinels_awakened = 1;
    for (int y = 0; y < TEMPLE_H; y++) {
        for (int x = 0; x < TEMPLE_W; x++) {
            if (g->map.tiles[y][x] == TILE_TEMPLE_MOON_DOOR_OPEN) {
                g->temple_alignment = 1;
            }
            if (g->map.tiles[y][x] == TILE_TEMPLE_DORMANT_SENTINEL) {
                g->temple_sentinels_awakened = 0;
            }
            if (guardian_defeated &&
                g->map.tiles[y][x] == TILE_TEMPLE_VAULT_DOOR) {
                g->map.tiles[y][x] = TILE_TEMPLE_FLOOR;
            }
            if (g->temple_treasure_state >= 2 &&
                g->map.tiles[y][x] == TILE_TEMPLE_TREASURE) {
                g->map.tiles[y][x] = TILE_TEMPLE_RUBBLE;
            }
        }
    }
}

void game_descend(GameState *g) {
    int depth = active_depth(g);
    if (g->level >= depth) return;

    LevelCache *cache = active_cache(g);
    int *max_level = active_max_level(g);

    if (g->level >= 1 && g->level <= depth) {
        cache[g->level - 1].map         = g->map;
        cache[g->level - 1].enemy_count = g->enemy_count;
        for (int i = 0; i < g->enemy_count; i++)
            cache[g->level - 1].enemies[i] = g->enemies[i];
        cache[g->level - 1].valid = 1;
        cache[g->level - 1].level_cleared = g->level_cleared;
    }

    g->level++;
    if (g->level > *max_level)
        *max_level = g->level;
    g->level_cleared = 0;
    if (g->level <= depth && cache[g->level - 1].valid) {
        g->map         = cache[g->level - 1].map;
        g->enemy_count = cache[g->level - 1].enemy_count;
        for (int i = 0; i < g->enemy_count; i++)
            g->enemies[i] = cache[g->level - 1].enemies[i];
        g->level_cleared = cache[g->level - 1].level_cleared;
        game_refresh_quest_encounters(g);
    } else {
        g->level_cleared = 0;
        generate_active_level(g);
    }
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    sync_temple_floor_state(g);
}

void game_ascend(GameState *g) {
    if (g->level <= 1) return;

    LevelCache *cache = active_cache(g);

    if (g->level <= active_depth(g)) {
        cache[g->level - 1].map           = g->map;
        cache[g->level - 1].enemy_count   = g->enemy_count;
        cache[g->level - 1].level_cleared = g->level_cleared;
        for (int i = 0; i < g->enemy_count; i++)
            cache[g->level - 1].enemies[i] = g->enemies[i];
        cache[g->level - 1].valid = 1;
    }

    g->level--;

    if (cache[g->level - 1].valid) {
        g->map         = cache[g->level - 1].map;
        g->enemy_count = cache[g->level - 1].enemy_count;
        g->level_cleared = cache[g->level - 1].level_cleared;
        for (int i = 0; i < g->enemy_count; i++)
            g->enemies[i] = cache[g->level - 1].enemies[i];
        game_refresh_quest_encounters(g);
    } else {
        g->level_cleared = 0;
    }

    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y;
    sync_temple_floor_state(g);
}

static void enter_adventure(GameState *g, Location location) {
    g->location = location;
    LevelCache *cache = active_cache(g);
    int *max_level = active_max_level(g);

    g->level = 1;
    g->level_cleared = 0;
    if (!g->portal_active || g->portal_location != location) {
        *max_level = 1;
        for (int i = 0; i < active_depth(g); i++) {
            cache[i].valid = 0;
        }
    }
    generate_active_level(g);
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
}

void game_enter_dungeon(GameState *g) {
    enter_adventure(g, LOCATION_DUNGEON);
}

void game_enter_forest(GameState *g) {
    enter_adventure(g, LOCATION_FOREST);
}

void game_enter_mountains(GameState *g) {
    enter_adventure(g, LOCATION_MOUNTAINS);
}

void game_enter_coast(GameState *g) {
    enter_adventure(g, LOCATION_COAST);
}

int game_harbor_unlocked(const GameState *g) {
    int bosses = (1 << LOCATION_DUNGEON) | (1 << LOCATION_FOREST) |
        (1 << LOCATION_MOUNTAINS) | (1 << LOCATION_COAST);
    return (g->defeated_bosses & bosses) == bosses &&
        g->elowen_quest_state == 3 && g->dain_quest_state == 3 &&
        g->alder_quest_state == 3 && g->mara_quest_state == 3;
}

int game_healer_price(const GameState *g) {
    int missing = g->player.max_hp - g->player.hp;
    return missing > 0 ? (missing + 2) / 3 : 0;
}

int game_healer_emergency_available(const GameState *g) {
    return g->player.hp > 0 &&
        g->player.hp * 4 < g->player.max_hp &&
        g->gold < game_healer_price(g);
}

void game_visit_healer(GameState *g) {
    if (g->location != LOCATION_TOWN || g->player.hp <= 0) {
        return;
    }
    int price = game_healer_price(g);
    if (price == 0) {
        push_message(g, "Lysa: You are already in good health.");
        return;
    }
    if (g->gold < price) {
        push_message(g, "Lysa: You do not have enough gold for treatment.");
        return;
    }
    g->gold -= price;
    g->player.hp = g->player.max_hp;
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message), "Lysa restores your HP to full for %d gold.", price);
    push_message(g, message);
}

void game_visit_healer_emergency(GameState *g) {
    if (g->location != LOCATION_TOWN || g->player.hp <= 0) {
        return;
    }
    if (!game_healer_emergency_available(g)) {
        push_message(g, "Lysa: Emergency care is reserved for critical need.");
        return;
    }
    g->player.hp = (g->player.max_hp + 1) / 2;
    push_message(g, "Lysa provides emergency care, restoring you to half health.");
}

int game_witch_price(const GameState *g) {
    int missing = g->player.max_mp - g->player.mp;
    return missing > 0 ? (missing + 2) / 3 : 0;
}

void game_visit_witch(GameState *g) {
    if (g->location != LOCATION_TOWN || g->player.hp <= 0) {
        return;
    }
    int price = game_witch_price(g);
    if (price == 0) {
        push_message(g, "Morwen: Your spirit is already full.");
        return;
    }
    if (g->gold < price) {
        push_message(g, "Morwen: You lack the gold for this ritual.");
        return;
    }
    g->gold -= price;
    g->player.mp = g->player.max_mp;
    char message[MAX_MESSAGE_LEN];
    snprintf(message, sizeof(message),
        "Morwen restores your MP to full for %d gold.", price);
    push_message(g, message);
}

static void place_harbor_road(GameState *g) {
    if (!game_harbor_unlocked(g)) {
        return;
    }
    // Branch below Rowan, then bend through a landing onto the dock.
    for (int x = 21; x < TOWN_HARBOR_X; x++) {
        g->map.tiles[TOWN_HARBOR_Y + 1][x] = TILE_TOWN_PATH;
    }
    for (int y = TOWN_HARBOR_Y + 1; y <= TOWN_HARBOR_ENTRANCE_Y; y++) {
        g->map.tiles[y][TOWN_HARBOR_X - 1] = TILE_TOWN_PATH;
    }
    for (int x = TOWN_HARBOR_X; x <= TOWN_HARBOR_ENTRANCE_X; x++) {
        g->map.tiles[TOWN_HARBOR_ENTRANCE_Y][x] = TILE_TOWN_PATH;
    }
}

static void place_town_portal(GameState *g) {
    if (!g->portal_active || g->location != LOCATION_TOWN) {
        return;
    }
    int x = 21;
    int y = 2;
    if (g->portal_location == LOCATION_FOREST) {
        x = 2;
        y = 13;
    } else if (g->portal_location == LOCATION_MOUNTAINS) {
        x = TOWN_W - 3;
        y = 13;
    } else if (g->portal_location == LOCATION_COAST) {
        x = 21;
        y = TOWN_H - 3;
    } else if (g->portal_location == LOCATION_TEMPLE) {
        x = TOWN_HARBOR_X - 1;
        y = TOWN_HARBOR_ENTRANCE_Y;
    }
    g->map.tiles[y][x] = TILE_PORTAL;
}

void game_enter_tavern(GameState *g) {
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_TAVERN;
    map_generate_tavern(&g->map, &spawn_x, &spawn_y);
    g->player.x = spawn_x;
    g->player.y = spawn_y;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    push_message(g, "You enter the Lantern & Cask.");
}

void game_leave_tavern(GameState *g) {
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_TOWN;
    map_generate_town(&g->map, &spawn_x, &spawn_y);
    place_harbor_road(g);
    g->player.x = 8;
    g->player.y = 21;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    place_town_portal(g);
    push_message(g, "You step back into town.");
}

void game_enter_island(GameState *g) {
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_ISLAND;
    g->level = 1;
    g->level_cleared = 0;
    map_generate_island(&g->map, &spawn_x, &spawn_y);
    g->player.x = spawn_x;
    g->player.y = spawn_y;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    push_message(g, "You make landfall on the Ruined Isle.");
    push_message(g, "The treasure map points beyond the temple gate.");
}

void game_leave_island(GameState *g) {
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_TOWN;
    map_generate_town(&g->map, &spawn_x, &spawn_y);
    place_harbor_road(g);
    g->player.x = TOWN_HARBOR_ENTRANCE_X;
    g->player.y = TOWN_HARBOR_ENTRANCE_Y;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    place_town_portal(g);
    push_message(g, "The ship returns you to town.");
}

static void cache_temple_level(GameState *g) {
    LevelCache *cache = &g->temple_cache[g->level - 1];
    cache->map = g->map;
    cache->enemy_count = g->enemy_count;
    cache->level_cleared = g->level_cleared;
    for (int i = 0; i < g->enemy_count; i++) {
        cache->enemies[i] = g->enemies[i];
    }
    cache->valid = 1;
}

static void spawn_temple_enemies(GameState *g) {
    static const int counts[TEMPLE_DEPTH] = {6, 7, 8, 7};
    static const EnemyType types[TEMPLE_DEPTH][8] = {
        {ENEMY_RELIC_SCARABS, ENEMY_TEMPLE_STALKER,
            ENEMY_BLOWDART_HUNTER, ENEMY_VINEBOUND_GUARDIAN,
            ENEMY_SUN_PRIEST, ENEMY_SERPENT_SPIRIT},
        {ENEMY_RELIC_SCARABS, ENEMY_BLOWDART_HUNTER,
            ENEMY_VINEBOUND_GUARDIAN, ENEMY_SUN_PRIEST,
            ENEMY_SERPENT_SPIRIT, ENEMY_TEMPLE_STALKER,
            ENEMY_LUNAR_EFFIGY},
        {ENEMY_TEMPLE_STALKER, ENEMY_BLOWDART_HUNTER,
            ENEMY_VINEBOUND_GUARDIAN, ENEMY_SUN_PRIEST,
            ENEMY_SERPENT_SPIRIT, ENEMY_TREASURE_WRAITH,
            ENEMY_LUNAR_EFFIGY, ENEMY_RELIC_SCARABS},
        {ENEMY_RELIC_SCARABS, ENEMY_TEMPLE_STALKER,
            ENEMY_BLOWDART_HUNTER, ENEMY_SUN_PRIEST,
            ENEMY_SERPENT_SPIRIT, ENEMY_TREASURE_WRAITH,
            ENEMY_FALLEN_SUN_GUARDIAN}
    };
    static const int positions[TEMPLE_DEPTH][8][2] = {
        {{14, 26}, {10, 29}, {19, 24}, {7, 24},
            {53, 24}, {48, 29}},
        {{10, 25}, {18, 23}, {48, 24}, {55, 27},
            {26, 17}, {39, 17}, {42, 6}},
        {{10, 25}, {18, 24}, {48, 25}, {55, 27},
            {24, 17}, {40, 17}, {12, 6}, {50, 6}},
        {{14, 26}, {10, 29}, {19, 24}, {53, 24},
            {48, 29}, {41, 19}, {32, 9}}
    };
    g->enemy_count = 0;
    int floor = g->level - 1;
    for (int i = 0; i < counts[floor]; i++) {
        EnemyType type = types[floor][i];
        if (type == ENEMY_FALLEN_SUN_GUARDIAN &&
            (g->defeated_bosses & (1 << LOCATION_TEMPLE))) {
            continue;
        }
        spawn_enemy(g, &g->enemies[g->enemy_count++], type,
            positions[floor][i][0], positions[floor][i][1]);
    }
}

void game_enter_temple(GameState *g) {
    g->location = LOCATION_TEMPLE;
    g->level = 1;
    g->floor_item_count = 0;
    g->dialogue_active = 0;
    if (g->temple_cache[0].valid) {
        g->map = g->temple_cache[0].map;
        g->enemy_count = g->temple_cache[0].enemy_count;
        g->level_cleared = g->temple_cache[0].level_cleared;
        for (int i = 0; i < g->enemy_count; i++) {
            g->enemies[i] = g->temple_cache[0].enemies[i];
        }
    } else {
        int spawn_x;
        int spawn_y;
        map_generate_temple(&g->map, 1, &spawn_x, &spawn_y);
        g->player.x = spawn_x;
        g->player.y = spawn_y;
        g->level_cleared = 0;
        g->temple_alignment = 0;
        g->temple_sentinels_awakened = 0;
        spawn_temple_enemies(g);
    }
    sync_temple_floor_state(g);
    g->player.x = TEMPLE_ENTRANCE_X;
    g->player.y = TEMPLE_ENTRANCE_Y;
    push_message(g, "You enter the Ruined Temple. The sun seal burns.");
}

void game_leave_temple(GameState *g) {
    cache_temple_level(g);
    int spawn_x;
    int spawn_y;
    g->location = LOCATION_ISLAND;
    map_generate_island(&g->map, &spawn_x, &spawn_y);
    g->player.x = ISLAND_GATE_X;
    g->player.y = ISLAND_GATE_Y + 1;
    g->enemy_count = 0;
    g->floor_item_count = 0;
    push_message(g, "You step back onto the island.");
}

static int temple_interaction_tile(TileType tile) {
    return tile == TILE_TEMPLE_ALTAR || tile == TILE_TEMPLE_TREASURE;
}

int game_has_temple_interaction(const GameState *g) {
    if (g->location != LOCATION_TEMPLE) {
        return 0;
    }
    static const int offsets[5][2] = {
        {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}
    };
    for (int i = 0; i < 5; i++) {
        int x = g->player.x + offsets[i][0];
        int y = g->player.y + offsets[i][1];
        if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
            temple_interaction_tile(g->map.tiles[y][x])) {
            return 1;
        }
    }
    return 0;
}

static void toggle_temple_alignment(GameState *g) {
    g->temple_alignment = !g->temple_alignment;
    if (g->temple_alignment) {
        g->temple_sentinels_awakened = 1;
    }
    for (int y = 0; y < TEMPLE_H; y++) {
        for (int x = 0; x < TEMPLE_W; x++) {
            TileType tile = g->map.tiles[y][x];
            if (g->temple_alignment && tile == TILE_TEMPLE_MOON_DOOR_CLOSED) {
                g->map.tiles[y][x] = TILE_TEMPLE_MOON_DOOR_OPEN;
            } else if (!g->temple_alignment &&
                tile == TILE_TEMPLE_MOON_DOOR_OPEN) {
                g->map.tiles[y][x] = TILE_TEMPLE_MOON_DOOR_CLOSED;
            } else if (g->temple_alignment &&
                tile == TILE_TEMPLE_DORMANT_SENTINEL) {
                g->map.tiles[y][x] = TILE_TEMPLE_FLOOR;
                if (g->enemy_count < MAX_ENEMIES) {
                    spawn_enemy(g, &g->enemies[g->enemy_count++],
                        ENEMY_MOONBOUND_SENTINEL, x, y);
                }
            }
        }
    }
    push_message(g, g->temple_alignment
        ? "Moon rises: lunar doors open and sentinels awaken!"
        : "Sun rises: lunar doors close and solar traps ignite!");
}

int game_interact_temple(GameState *g) {
    if (g->location != LOCATION_TEMPLE) {
        return 0;
    }
    static const int offsets[5][2] = {
        {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}
    };
    for (int i = 0; i < 5; i++) {
        int x = g->player.x + offsets[i][0];
        int y = g->player.y + offsets[i][1];
        if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
            continue;
        }
        TileType tile = g->map.tiles[y][x];
        if (tile == TILE_TEMPLE_ALTAR) {
            toggle_temple_alignment(g);
            return 1;
        }
        if (tile == TILE_TEMPLE_TREASURE) {
            if (!(g->defeated_bosses & (1 << LOCATION_TEMPLE))) {
                push_message(g, "The Fallen Sun Guardian seals the treasure vault.");
                return 1;
            }
            if (g->temple_treasure_state < 2) {
                g->temple_treasure_state = 2;
                g->map.tiles[y][x] = TILE_TEMPLE_RUBBLE;
                push_message(g, "The Buried Sun is recovered. Return it to Nahla.");
            }
            return 1;
        }
    }
    return 0;
}

void game_record_temple_enemy_defeated(GameState *g, EnemyType type) {
    if (g->location != LOCATION_TEMPLE ||
        type != ENEMY_FALLEN_SUN_GUARDIAN) {
        return;
    }
    g->defeated_bosses |= 1 << LOCATION_TEMPLE;
    for (int y = 0; y < TEMPLE_H; y++) {
        for (int x = 0; x < TEMPLE_W; x++) {
            if (g->map.tiles[y][x] == TILE_TEMPLE_VAULT_DOOR) {
                g->map.tiles[y][x] = TILE_TEMPLE_FLOOR;
            }
        }
    }
    push_message(g, "The guardian falls. The buried vault opens!");
}

void game_return_to_town(GameState *g) {
    Location returning_from = g->location;
    LevelCache *cache = active_cache(g);
    // Cache current level before leaving
    if (g->level >= 1 && g->level <= active_depth(g)) {
        cache[g->level - 1].map = g->map;
        cache[g->level - 1].enemy_count   = g->enemy_count;
        cache[g->level - 1].level_cleared = g->level_cleared;
        for (int i = 0; i < g->enemy_count; i++)
            cache[g->level - 1].enemies[i] = g->enemies[i];
        cache[g->level - 1].valid = 1;
    }

    g->location = LOCATION_TOWN;
    int spawn_x, spawn_y;
    map_generate_town(&g->map, &spawn_x, &spawn_y);
    place_harbor_road(g);
    if (returning_from == LOCATION_FOREST) {
        g->player.x = 1; g->player.y = 12;
    } else if (returning_from == LOCATION_MOUNTAINS) {
        g->player.x = TOWN_W - 2; g->player.y = 12;
    } else if (returning_from == LOCATION_COAST) {
        g->player.x = 20; g->player.y = TOWN_H - 2;
    } else if (returning_from == LOCATION_TEMPLE) {
        g->player.x = TOWN_HARBOR_X - 1;
        g->player.y = TOWN_HARBOR_ENTRANCE_Y;
    } else {
        g->player.x = 20; g->player.y = 1;
    }
    g->floor_item_count = 0;
    g->enemy_count = 0;
    place_town_portal(g);
}

void game_open_town_portal(GameState *g) {
    if (g->location != LOCATION_DUNGEON &&
        g->location != LOCATION_FOREST &&
        g->location != LOCATION_MOUNTAINS &&
        g->location != LOCATION_COAST &&
        g->location != LOCATION_TEMPLE) {
        return;
    }
    g->portal_active = 1;
    g->portal_level = g->level;
    g->portal_location = g->location;
    g->portal_x = g->player.x;
    g->portal_y = g->player.y;
    g->portal_origin_tile = g->map.tiles[g->player.y][g->player.x];
    g->map.tiles[g->player.y][g->player.x] = TILE_PORTAL;
    game_return_to_town(g);
    push_message(g, "A return portal remains open.");
}

static int portal_landing_open(const GameState *g, int x, int y) {
    if (!map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x &&
            g->enemies[i].y == y) {
            return 0;
        }
    }
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    for (int i = 0; i < 4; i++) {
        if (map_is_walkable(&g->map, x + dx[i], y + dy[i])) {
            return 1;
        }
    }
    return 0;
}

void game_use_town_portal(GameState *g) {
    if (!g->portal_active || g->portal_level < 1 ||
        g->portal_level > MAX_REGION_DEPTH) return;
    int level = g->portal_level;
    LevelCache *cache = g->level_cache;
    if (g->portal_location == LOCATION_FOREST) {
        cache = g->forest_cache;
    } else if (g->portal_location == LOCATION_MOUNTAINS) {
        cache = g->mountain_cache;
    } else if (g->portal_location == LOCATION_COAST) {
        cache = g->coast_cache;
    } else if (g->portal_location == LOCATION_TEMPLE) {
        cache = g->temple_cache;
    }
    if (!cache[level - 1].valid) return;

    g->location = g->portal_location;
    g->level = level;
    g->map = cache[level - 1].map;
    g->enemy_count = cache[level - 1].enemy_count;
    g->level_cleared = cache[level - 1].level_cleared;
    for (int i = 0; i < g->enemy_count; i++)
        g->enemies[i] = cache[level - 1].enemies[i];
    game_refresh_quest_encounters(g);
    sync_temple_floor_state(g);
    int landing_x = g->portal_x;
    int landing_y = g->portal_y;
    if (landing_x >= 0 && landing_x < MAP_W &&
        landing_y >= 0 && landing_y < MAP_H &&
        g->map.tiles[landing_y][landing_x] == TILE_PORTAL) {
        g->map.tiles[landing_y][landing_x] = g->portal_origin_tile;
    }
    if (!portal_landing_open(g, landing_x, landing_y)) {
        landing_x = g->map.stairs_up_x;
        landing_y = g->map.stairs_up_y;
        if (!portal_landing_open(g, landing_x, landing_y)) {
            g->portal_active = 0;
            game_return_to_town(g);
            push_message(g, "No safe ground beyond the portal.");
            return;
        }
        push_message(g, "The portal returns you at the entrance.");
    } else {
        push_message(g, "Returned through the portal.");
    }
    g->player.x = landing_x;
    g->player.y = landing_y;
    g->portal_active = 0;
}

void game_talk_to_cain(GameState *g) {
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Cain");
    g->dialogue_x = TOWN_CAIN_X;
    g->dialogue_y = TOWN_CAIN_Y;
    const char *warning = "Undead lurk north, beasts west, goblins east, and sea horrors south.";
    if (g->cain_scroll_given) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "%s Read your scroll to learn Return to Town. Use it when danger grows!", warning);
    } else if (g->inventory_count >= MAX_INVENTORY) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "%s Make room in your pack, then speak to me for a Return to Town scroll.", warning);
    } else {
        g->inventory[g->inventory_count++] = item_make_scroll_return_to_town();
        g->cain_scroll_given = 1;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "%s Take this Return to Town scroll for your adventure. Read it to learn a way home!", warning);
        push_message(g, "Cain gives you a Scroll of Return to Town.");
    }
}

int game_has_treasure_map(const GameState *g) {
    for (int i = 0; i < g->inventory_count; i++) {
        if (g->inventory[i].type == ITEM_TREASURE_MAP) {
            return 1;
        }
    }
    return 0;
}

static int island_interaction_tile(TileType tile) {
    return tile == TILE_ISLAND_CAMP || tile == TILE_ISLAND_MARKER ||
        tile == TILE_ISLAND_STATUE || tile == TILE_ISLAND_LAGOON ||
        tile == TILE_ISLAND_TEMPLE_GATE;
}

int game_has_island_interaction(const GameState *g) {
    if (g->location != LOCATION_ISLAND) {
        return 0;
    }
    static const int offsets[5][2] = {
        {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}
    };
    for (int i = 0; i < 5; i++) {
        int x = g->player.x + offsets[i][0];
        int y = g->player.y + offsets[i][1];
        if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H &&
            island_interaction_tile(g->map.tiles[y][x])) {
            return 1;
        }
    }
    return 0;
}

int game_interact_island(GameState *g) {
    if (g->location != LOCATION_ISLAND) {
        return 0;
    }
    static const int offsets[5][2] = {
        {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}
    };
    for (int i = 0; i < 5; i++) {
        int x = g->player.x + offsets[i][0];
        int y = g->player.y + offsets[i][1];
        if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
            continue;
        }
        TileType tile = g->map.tiles[y][x];
        if (tile == TILE_ISLAND_CAMP) {
            push_message(g, "A waterlogged journal warns that the temple guardians still stir.");
            return 1;
        }
        if (tile == TILE_ISLAND_MARKER) {
            push_message(g, "The carved sun matches the symbol on your treasure map.");
            return 1;
        }
        if (tile == TILE_ISLAND_STATUE) {
            push_message(g, "The broken guardian faces the ruined temple gate.");
            return 1;
        }
        if (tile == TILE_ISLAND_LAGOON) {
            push_message(g, "Fresh water spills from beneath the ancient stonework.");
            return 1;
        }
        if (tile == TILE_ISLAND_TEMPLE_GATE) {
            push_message(g, "The treasure trail continues inside the ruined temple.");
            return 1;
        }
    }
    return 0;
}

void game_talk_to_nahla(GameState *g) {
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Nahla");
    g->dialogue_x = ISLAND_NAHLA_X;
    g->dialogue_y = ISLAND_NAHLA_Y;
    if (g->temple_treasure_state == 0) {
        g->temple_treasure_state = 1;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "The temple is a stepped pyramid. Climb its four tiers, defeat "
            "the Fallen Sun Guardian, and recover the Buried Sun from the summit vault.");
        push_message(g, "Quest assigned: The Buried Sun.");
        return;
    }
    if (g->temple_treasure_state == 1) {
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "The Guardian waits at the pyramid summit. The sun and moon altars "
            "change which passages are safe as you climb.");
        push_message(g, "Quest active: The Buried Sun.");
        return;
    }
    if (g->temple_treasure_state == 2) {
        g->temple_treasure_state = 3;
        g->gold += 150;
        g->score += 2500;
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "You found it. The Buried Sun belongs to history again. "
            "Take this reward for surviving the pyramid.");
        push_message(g, "Quest complete: The Buried Sun. 150 gold awarded.");
        return;
    }
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
        "The summit is quiet now. The island will remember what you recovered.");
}

void game_talk_to_rowan(GameState *g) {
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, MAX_SPEAKER_LEN, "Captain Rowan");
    g->dialogue_x = TOWN_ROWAN_X;
    g->dialogue_y = TOWN_ROWAN_Y;
    if (game_harbor_unlocked(g)) {
        if (game_has_treasure_map(g)) {
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "Keep that map safe. It charts the sea route to the island "
                "and marks buried treasure beneath the ruined temple.");
        } else if (g->inventory_count >= MAX_INVENTORY) {
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "The road now reaches the harbor. I have a treasure map for you. "
                "Make room in your pack, then speak to me again.");
        } else {
            g->inventory[g->inventory_count++] = item_make_treasure_map();
            snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
                "The road now reaches the harbor. Take this treasure map: "
                "it charts a sea route to the island and marks treasure beneath its ruined temple.");
            push_message(g, "Rowan gives you an Island Treasure Map.");
        }
        return;
    }
    snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
        "Beyond these shores lies an island, reachable only by ship. "
        "Sailors whisper of a ruined temple and treasure buried beneath it. "
        "For now, the harbor is closed.");
}

static void prepare_quest_expedition(GameState *g, Location location) {
    LevelCache *cache;
    int *max_level;
    if (location == LOCATION_DUNGEON) {
        cache = g->level_cache;
        max_level = &g->max_level_reached;
    } else if (location == LOCATION_FOREST) {
        cache = g->forest_cache;
        max_level = &g->max_forest_level_reached;
    } else if (location == LOCATION_MOUNTAINS) {
        cache = g->mountain_cache;
        max_level = &g->max_mountain_level_reached;
    } else {
        cache = g->coast_cache;
        max_level = &g->max_coast_level_reached;
    }
    for (int i = 0; i < MAX_REGION_DEPTH; i++) {
        cache[i].valid = 0;
        cache[i].level_cleared = 0;
    }
    *max_level = 1;
    if (g->portal_active && g->portal_location == location) {
        g->portal_active = 0;
    }
}

void game_talk_to_elowen(GameState *g) {
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Elowen", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = 10;
    g->dialogue_y = 7;
    if (g->elowen_quest_state == 0) {
        g->elowen_quest_state = 1;
        g->elowen_seals_restored = 0;
        prepare_quest_expedition(g, LOCATION_DUNGEON);
        strncpy(g->dialogue_text,
            "The dead have gathered around shattered burial seals on dungeon floors 2, 4, and 6. Break through them and restore each seal.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Quest assigned: The Broken Seals.");
        return;
    }
    if (g->elowen_quest_state == 1) {
        int restored = 0;
        for (int i = 0; i < 3; i++) {
            if (g->elowen_seals_restored & (1 << i)) {
                restored++;
            }
        }
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "You have restored %d of 3 burial seals. Return when all three are whole.",
            restored);
        char status[MAX_MESSAGE_LEN];
        snprintf(status, sizeof(status), "Quest progress: %d/3 seals.",
            restored);
        push_message(g, status);
        return;
    }
    if (g->elowen_quest_state == 2) {
        g->elowen_quest_state = 3;
        g->gold += 40;
        g->score += 300;
        strncpy(g->dialogue_text,
            "The crypt is bound once more. Its dead may finally sleep. Take this gold with my gratitude.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Quest complete: The Broken Seals.");
        return;
    }
    strncpy(g->dialogue_text, "You have my gratitude, adventurer.",
        MAX_DIALOGUE_LEN - 1);
    g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
    push_message(g, "Elowen's quest is already complete.");
}

void game_talk_to_dain(GameState *g) {
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Dain", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = 18;
    g->dialogue_y = 7;
    if (g->dain_quest_state == 0) {
        g->dain_quest_state = 1;
        g->dain_map_fragments = 0;
        prepare_quest_expedition(g, LOCATION_MOUNTAINS);
        strncpy(g->dialogue_text,
            "Three goblin warbands carry pieces of an old dwarven map. Hunt their leaders on mountain stages 2, 3, and 5.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Assigned: Recover the Treasure Map.");
        return;
    }
    if (g->dain_quest_state == 1) {
        int defeated = 0;
        for (int bit = 0; bit < 3; bit++) {
            if (g->dain_map_fragments & (1 << bit)) {
                defeated++;
            }
        }
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "You have recovered %d of 3 map fragments. The remaining pieces are still carried through the mountains.",
            defeated);
        char status[MAX_MESSAGE_LEN];
        snprintf(status, sizeof(status),
            "Quest progress: %d/3 map fragments.", defeated);
        push_message(g, status);
        return;
    }
    if (g->dain_quest_state == 2) {
        g->dain_quest_state = 3;
        g->gold += 60;
        g->score += 400;
        strncpy(g->dialogue_text,
            "The treasure map is whole again. It reveals a dwarven hoard the goblins never learned how to find.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Completed: Recover the Treasure Map.");
        return;
    }
    strncpy(g->dialogue_text,
        "The restored map still points toward riches hidden beneath the mountains.",
        MAX_DIALOGUE_LEN - 1);
    g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
    push_message(g, "Dain's quest is already complete.");
}

void game_record_dain_kill(GameState *g, EnemyType type) {
    if (g->location != LOCATION_MOUNTAINS || g->dain_quest_state != 1) {
        return;
    }
    int target = 0;
    const char *status = NULL;
    if (type == ENEMY_GOBLIN_ARCHER) {
        target = DAIN_FRAGMENT_ARCHER;
        status = "Map fragment recovered from Archer.";
    } else if (type == ENEMY_GOBLIN_BOMBER) {
        target = DAIN_FRAGMENT_BOMBER;
        status = "Map fragment recovered from Bomber.";
    } else if (type == ENEMY_GOBLIN_SHAMAN) {
        target = DAIN_FRAGMENT_SHAMAN;
        status = "Map fragment recovered from Shaman.";
    }
    if (!target || (g->dain_map_fragments & target)) {
        return;
    }
    g->dain_map_fragments |= target;
    push_message(g, status);
    if ((g->dain_map_fragments & 7) == 7) {
        g->dain_quest_state = 2;
        push_message(g, "Treasure map complete. Return to Dain.");
    }
}

void game_talk_to_alder(GameState *g) {
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Alder", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = 28;
    g->dialogue_y = 7;
    if (g->alder_quest_state == 0) {
        g->alder_quest_state = 1;
        g->alder_wardens_rescued = 0;
        prepare_quest_expedition(g, LOCATION_FOREST);
        strncpy(g->dialogue_text,
            "Three of my wardens are trapped behind enemy hunting parties on forest stages 2, 5, and 7. Defeat their captors and bring them home.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Assigned: The Lost Wardens.");
        return;
    }
    if (g->alder_quest_state == 1) {
        int rescued = 0;
        for (int bit = 0; bit < 3; bit++) {
            if (g->alder_wardens_rescued & (1 << bit)) {
                rescued++;
            }
        }
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "You have rescued %d of my 3 wardens. Search the guarded groves on forest stages 2, 5, and 7.",
            rescued);
        char status[MAX_MESSAGE_LEN];
        snprintf(status, sizeof(status), "Quest progress: %d/3 wardens.",
            rescued);
        push_message(g, status);
        return;
    }
    if (g->alder_quest_state == 2) {
        g->alder_quest_state = 3;
        g->gold += 70;
        g->score += 500;
        strncpy(g->dialogue_text,
            "All three returned safely. The forest has taken enough from us. Accept this reward with an old ranger's gratitude.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Completed: The Lost Wardens.");
        return;
    }
    strncpy(g->dialogue_text,
        "My wardens are home, and the paths feel less lonely for it.",
        MAX_DIALOGUE_LEN - 1);
    g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
    push_message(g, "Alder's quest is already complete.");
}

void game_rescue_forest_warden(GameState *g, int x, int y) {
    if (g->location != LOCATION_FOREST || g->alder_quest_state != 1 ||
        x < 0 || x >= MAP_W || y < 0 || y >= MAP_H ||
        g->map.tiles[y][x] != TILE_FOREST_WARDEN) {
        return;
    }
    int bit = alder_warden_bit(g->level);
    if (!bit || (g->alder_wardens_rescued & bit)) {
        return;
    }
    g->alder_wardens_rescued |= bit;
    g->map.tiles[y][x] = TILE_FOREST_FLOOR;
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Forest Warden", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = x;
    g->dialogue_y = y;
    if (g->level == 2) {
        strncpy(g->dialogue_text,
            "You cut through the spider web binding me. I can follow your trail home from here.",
            MAX_DIALOGUE_LEN - 1);
    } else if (g->level == 5) {
        strncpy(g->dialogue_text,
            "The Dark Elves thought this grove would be my prison. I will make my way back to Alder.",
            MAX_DIALOGUE_LEN - 1);
    } else {
        strncpy(g->dialogue_text,
            "Those cursed roots were pulling me beneath the earth. You reached me just in time.",
            MAX_DIALOGUE_LEN - 1);
    }
    g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
    int rescued = 0;
    for (int index = 0; index < 3; index++) {
        if (g->alder_wardens_rescued & (1 << index)) {
            rescued++;
        }
    }
    char status[MAX_MESSAGE_LEN];
    snprintf(status, sizeof(status), "Warden rescued: %d/3.", rescued);
    push_message(g, status);
    if ((g->alder_wardens_rescued & 7) == 7) {
        g->alder_quest_state = 2;
        push_message(g, "All wardens rescued. Return to Alder.");
    }
}

void game_talk_to_mara(GameState *g) {
    g->dialogue_active = 1;
    strncpy(g->dialogue_speaker, "Mara", MAX_SPEAKER_LEN - 1);
    g->dialogue_speaker[MAX_SPEAKER_LEN - 1] = '\0';
    g->dialogue_x = 31;
    g->dialogue_y = 18;
    if (g->mara_quest_state == 0) {
        g->mara_quest_state = 1;
        g->mara_beacons_lit = 0;
        prepare_quest_expedition(g, LOCATION_COAST);
        strncpy(g->dialogue_text,
            "Drowned guardians surround beacons on coast stages 1, 3, and 6. Lower the tide, defeat them, and relight each flame.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Assigned: Relight the Drowned Beacons.");
        return;
    }
    if (g->mara_quest_state == 1) {
        int lit = 0;
        for (int bit = 0; bit < 3; bit++) {
            if (g->mara_beacons_lit & (1 << bit)) {
                lit++;
            }
        }
        snprintf(g->dialogue_text, MAX_DIALOGUE_LEN,
            "You have relit %d of 3 beacons. The remaining lights wait on coast stages 1, 3, and 6.",
            lit);
        char status[MAX_MESSAGE_LEN];
        snprintf(status, sizeof(status), "Quest progress: %d/3 beacons.", lit);
        push_message(g, status);
        return;
    }
    if (g->mara_quest_state == 2) {
        g->mara_quest_state = 3;
        g->gold += 80;
        g->score += 600;
        strncpy(g->dialogue_text,
            "All three flames shine across the drowned road. Sailors can find safe water again. Take this with my thanks.",
            MAX_DIALOGUE_LEN - 1);
        g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
        push_message(g, "Completed: Relight the Drowned Beacons.");
        return;
    }
    strncpy(g->dialogue_text,
        "The beacons still burn. Even this ruined coast can guide travelers home.",
        MAX_DIALOGUE_LEN - 1);
    g->dialogue_text[MAX_DIALOGUE_LEN - 1] = '\0';
    push_message(g, "Mara's quest is already complete.");
}

void game_light_coast_beacon(GameState *g, int x, int y) {
    if (g->location != LOCATION_COAST || x < 0 || x >= MAP_W || y < 0 ||
        y >= MAP_H || g->map.tiles[y][x] != TILE_COAST_BEACON_UNLIT) {
        return;
    }
    if (g->mara_quest_state != 1) {
        push_message(g, "An ancient drowned beacon stands here.");
        return;
    }
    for (int map_y = 0; map_y < MAP_H; map_y++) {
        for (int map_x = 0; map_x < MAP_W; map_x++) {
            if (g->map.tiles[map_y][map_x] == TILE_COAST_DEEP_WATER) {
                push_message(g, "Lower the tide before lighting it.");
                return;
            }
        }
    }
    int bit = mara_beacon_bit(g->level);
    if (!bit || (g->mara_beacons_lit & bit)) {
        return;
    }
    g->mara_beacons_lit |= bit;
    g->map.tiles[y][x] = TILE_COAST_BEACON_LIT;
    int lit = 0;
    for (int index = 0; index < 3; index++) {
        if (g->mara_beacons_lit & (1 << index)) {
            lit++;
        }
    }
    char status[MAX_MESSAGE_LEN];
    snprintf(status, sizeof(status), "Beacon lit: %d/3.", lit);
    push_message(g, status);
    if ((g->mara_beacons_lit & 7) == 7) {
        g->mara_quest_state = 2;
        push_message(g, "All beacons lit. Return to Mara.");
    }
}

void game_mark_level_cleared(GameState *g) {
    g->level_cleared = 1;
}

void game_update_level_progress(GameState *g) {
    if (g->defeated_bosses & (1 << g->location)) {
        if (g->location == LOCATION_FOREST && g->level == FOREST_DEPTH) {
            map_reveal_forest_exit(&g->map);
        } else if (g->location == LOCATION_DUNGEON && g->level == DUNGEON_DEPTH &&
            g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] != TILE_RETURN_EXIT) {
            g->map.tiles[g->map.stairs_down_y][g->map.stairs_down_x] = TILE_RETURN_EXIT;
            for (int i = 0; i < g->floor_item_count; i++) {
                FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == g->map.stairs_down_x &&
                    item->y == g->map.stairs_down_y) {
                    item->underlying_tile = TILE_RETURN_EXIT;
                }
            }
            push_message(g, "A passage to town opens!");
        }
    }

    int active_enemies = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active) {
            active_enemies++;
        }
    }

    if (active_enemies == 0) {
        game_mark_level_cleared(g);
    }
}

void player_gain_xp(GameState *g, int xp) {
    g->player.experience += xp;

    while (g->player.experience >= g->player.experience_next &&
           g->player.level < 50) {
        g->player.experience    -= g->player.experience_next;
        g->player.level++;
        g->player.max_hp        += 10;
        g->player.hp             = g->player.max_hp;
        g->player.mp             = g->player.max_mp;
        g->player.attack        += 2;

        // Defense grows at half rate, capped at 50% of attack
        int new_defense = g->player.defense + 1;
        int defense_cap = g->player.attack / 2;
        g->player.defense = new_defense > defense_cap ? defense_cap : new_defense;

        g->player.experience_next = g->player.level * 100;

        char msg[MAX_MESSAGE_LEN];
        snprintf(msg, sizeof(msg), "Level up! Now level %d", g->player.level);
        push_message(g, msg);
    }
}
