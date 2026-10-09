#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/game/actions.h"
#include "../src/systems/save_load.h"
#include "../src/screens/quest_journal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GameState frost_game;
static GameState frost_loaded;
static Map frost_layout;
static unsigned char visited[MAP_H][MAP_W];
static int queue[MAP_W * MAP_H];

#define AVOID_SLICK_ICE 1
#define AVOID_THIN_ICE 2

// Every snow, lake and exit tile must be reachable from the entrance, plus any
// slick or thin ice the flags allow. A flagged tile is treated as impassable,
// proving slick patches and thin-ice shortcuts are never the only way through.
static int frost_layout_connected(const Map *map, int avoid) {
    memset(visited, 0, sizeof(visited));
    int head = 0;
    int tail = 0;
    queue[tail++] = map->stairs_up_y * MAP_W + map->stairs_up_x;
    visited[map->stairs_up_y][map->stairs_up_x] = 1;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    while (head < tail) {
        int tile = queue[head++];
        int x = tile % MAP_W;
        int y = tile / MAP_W;
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (nx >= 0 && nx < MAP_W && ny >= 0 && ny < MAP_H && !visited[ny][nx] &&
                map_is_walkable(map, nx, ny) &&
                !((avoid & AVOID_SLICK_ICE) && map->tiles[ny][nx] == TILE_FROST_ICE) &&
                !((avoid & AVOID_THIN_ICE) && map->tiles[ny][nx] == TILE_FROST_THIN_ICE)) {
                visited[ny][nx] = 1;
                queue[tail++] = ny * MAP_W + nx;
            }
        }
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = map->tiles[y][x];
            if ((tile == TILE_FROST_FLOOR || tile == TILE_FROST_LAKE ||
                tile == TILE_FROST_EXIT ||
                (tile == TILE_FROST_ICE && !(avoid & AVOID_SLICK_ICE)) ||
                (tile == TILE_FROST_THIN_ICE && !(avoid & AVOID_THIN_ICE))) &&
                !visited[y][x]) {
                return 0;
            }
        }
    }
    return 1;
}

static int in_room(const Room *r, int x, int y) {
    return x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h;
}

// Counts connected runs of a tile, or returns -1 if any run's size is outside
// min_size..max_size, or need_5x3 is set and a run is not a solid 5x3
// rectangle (the shape of every slick-ice patch).
static int count_tile_runs(const Map *map, TileType kind, int need_5x3, int min_size, int max_size) {
    memset(visited, 0, sizeof(visited));
    int runs = 0;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (map->tiles[y][x] != kind || visited[y][x]) {
                continue;
            }
            runs++;
            int head = 0;
            int tail = 0;
            int min_x = x;
            int max_x = x;
            int min_y = y;
            int max_y = y;
            queue[tail++] = y * MAP_W + x;
            visited[y][x] = 1;
            while (head < tail) {
                int tile = queue[head++];
                int tx = tile % MAP_W;
                int ty = tile / MAP_W;
                min_x = tx < min_x ? tx : min_x;
                max_x = tx > max_x ? tx : max_x;
                min_y = ty < min_y ? ty : min_y;
                max_y = ty > max_y ? ty : max_y;
                for (int i = 0; i < 4; i++) {
                    int nx = tx + dx[i];
                    int ny = ty + dy[i];
                    if (nx >= 0 && nx < MAP_W && ny >= 0 && ny < MAP_H &&
                        !visited[ny][nx] && map->tiles[ny][nx] == kind) {
                        visited[ny][nx] = 1;
                        queue[tail++] = ny * MAP_W + nx;
                    }
                }
            }
            if (tail < min_size || tail > max_size || (need_5x3 &&
                (tail != 15 || max_x - min_x + 1 != 5 || max_y - min_y + 1 != 3))) {
                return -1;
            }
        }
    }
    return runs;
}

// Every thin-ice tile sits in a clean one-wide tunnel: walkable ground only
// straight ahead and behind, wall on both sides, and never inside or beside
// the stage-5 lake.
static int thin_ice_tunnels_clean(const Map *map, int level) {
    for (int y = 1; y < MAP_H - 1; y++) {
        for (int x = 1; x < MAP_W - 1; x++) {
            if (map->tiles[y][x] != TILE_FROST_THIN_ICE) {
                continue;
            }
            int north = map_is_walkable(map, x, y - 1);
            int south = map_is_walkable(map, x, y + 1);
            int west = map_is_walkable(map, x - 1, y);
            int east = map_is_walkable(map, x + 1, y);
            int vertical = north && south && !west && !east &&
                map->tiles[y][x - 1] == TILE_FROST_WALL && map->tiles[y][x + 1] == TILE_FROST_WALL;
            int horizontal = west && east && !north && !south &&
                map->tiles[y - 1][x] == TILE_FROST_WALL && map->tiles[y + 1][x] == TILE_FROST_WALL;
            if (!vertical && !horizontal) {
                return 0;
            }
            if (level == FROSTFELL_DEPTH && in_room(&map->rooms[map->room_count - 1], x, y)) {
                return 0;
            }
            for (int ny = y - 1; ny <= y + 1; ny++) {
                for (int nx = x - 1; nx <= x + 1; nx++) {
                    if (map->tiles[ny][nx] == TILE_FROST_LAKE ||
                        map->tiles[ny][nx] == TILE_FROST_LAKE_HOLE) {
                        return 0;
                    }
                }
            }
        }
    }
    return 1;
}

// Every patch sits in a middle clearing (never the first or last) with snow
// all round it, so a slide in any direction always ends on snow.
static int ice_patches_ringed(const Map *map) {
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    for (int y = 1; y < MAP_H - 1; y++) {
        for (int x = 1; x < MAP_W - 1; x++) {
            if (map->tiles[y][x] != TILE_FROST_ICE) {
                continue;
            }
            int in_middle_clearing = 0;
            for (int i = 1; i < map->room_count - 1; i++) {
                in_middle_clearing |= in_room(&map->rooms[i], x, y);
            }
            if (!in_middle_clearing || in_room(&map->rooms[0], x, y) ||
                in_room(&map->rooms[map->room_count - 1], x, y)) {
                return 0;
            }
            for (int ny = y - 1; ny <= y + 1; ny++) {
                for (int nx = x - 1; nx <= x + 1; nx++) {
                    if (map->tiles[ny][nx] != TILE_FROST_ICE &&
                        map->tiles[ny][nx] != TILE_FROST_FLOOR) {
                        return 0;
                    }
                }
            }
            for (int i = 0; i < 4; i++) {
                int sx = x;
                int sy = y;
                while (sx > 0 && sx < MAP_W - 1 && sy > 0 && sy < MAP_H - 1 &&
                    map->tiles[sy][sx] == TILE_FROST_ICE) {
                    sx += dx[i];
                    sy += dy[i];
                }
                if (map->tiles[sy][sx] != TILE_FROST_FLOOR) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

static int frost_layout_snowed_over(const Map *map) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = map->tiles[y][x];
            if (tile == TILE_SWAMP_FLOOR || tile == TILE_SWAMP_WALL ||
                tile == TILE_SWAMP_ENTRANCE || tile == TILE_SWAMP_EXIT) {
                return 0;
            }
        }
    }
    return 1;
}

static void walk_onto(GameState *g, int x, int y) {
    action_resolve_player(g, (Action){ACTION_MOVE, x, y});
}

static int frost_enemy_type(EnemyType type) {
    return type == ENEMY_ICE_WOLF || type == ENEMY_FROST_ARCHER ||
        type == ENEMY_YETI || type == ENEMY_FROST_WRAITH ||
        type == ENEMY_ICE_GOLEM || type == ENEMY_ICE_GIANT;
}

static int steps_from(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    return dx > dy ? dx : dy;
}

static int find_kraken(const GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].type == ENEMY_POLAR_KRAKEN) {
            return i;
        }
    }
    return -1;
}

// The first stage each creature may appear on.
static int first_stage(EnemyType type) {
    switch (type) {
        case ENEMY_YETI: return 2;
        case ENEMY_FROST_WRAITH: return 3;
        case ENEMY_ICE_GOLEM: return 4;
        case ENEMY_ICE_GIANT: return 5;
        case ENEMY_POLAR_KRAKEN: return 5;
        default: return 1;
    }
}

static void test_frostfell_rosters(void) {
    int only_frost = 1;
    int stages_respected = 1;
    int counts_match = 1;
    int on_snow = 1;
    int one_kraken_on_the_lake = 1;
    int seen_on_stage_5[ENEMY_ICE_GIANT + 1] = {0};
    for (int level = 1; level <= FROSTFELL_DEPTH; level++) {
        for (int seed = 1; seed <= 25; seed++) {
            memset(&frost_game, 0, sizeof(frost_game));
            game_init(&frost_game);
            srand((unsigned int)(seed * 17 + level));
            frost_game.location = LOCATION_FROSTFELL;
            frost_game.level = level;
            map_generate_frostfell(&frost_game.map, level);
            enemies_spawn(&frost_game);
            counts_match &= frost_game.enemy_count == 10 + level;
            int krakens = 0;
            for (int i = 0; i < frost_game.enemy_count; i++) {
                const Enemy *e = &frost_game.enemies[i];
                if (e->type == ENEMY_POLAR_KRAKEN) {
                    krakens++;
                    one_kraken_on_the_lake &= e->is_boss &&
                        frost_game.map.tiles[e->y][e->x] == TILE_FROST_LAKE;
                    continue;
                }
                only_frost &= frost_enemy_type(e->type);
                stages_respected &= first_stage(e->type) <= level;
                on_snow &= frost_game.map.tiles[e->y][e->x] == TILE_FROST_FLOOR;
                if (level == FROSTFELL_DEPTH && frost_enemy_type(e->type)) {
                    seen_on_stage_5[e->type] = 1;
                }
            }
            one_kraken_on_the_lake &= krakens == (level == FROSTFELL_DEPTH);
        }
    }
    int all_kinds_on_stage_5 = seen_on_stage_5[ENEMY_ICE_WOLF] &&
        seen_on_stage_5[ENEMY_FROST_ARCHER] && seen_on_stage_5[ENEMY_YETI] &&
        seen_on_stage_5[ENEMY_FROST_WRAITH] && seen_on_stage_5[ENEMY_ICE_GOLEM] &&
        seen_on_stage_5[ENEMY_ICE_GIANT];
    ASSERT("Frostfell spawns only its six frost creatures, 10 plus the stage in number",
        only_frost && counts_match);
    ASSERT("each stage adds one heavier creature and stage 5 fields all six",
        stages_respected && all_kinds_on_stage_5);
    ASSERT("frost creatures spawn on open snow", on_snow);
    ASSERT("the Polar Kraken waits alone on the stage 5 lake", one_kraken_on_the_lake);
}

static int lake_hole_count(const Map *map, const Room *lake) {
    int holes = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (map->tiles[y][x] != TILE_FROST_LAKE_HOLE) {
                continue;
            }
            if (x < lake->x || x >= lake->x + lake->w ||
                y < lake->y || y >= lake->y + lake->h || map_is_walkable(map, x, y)) {
                return -1;
            }
            holes++;
        }
    }
    return holes;
}

static int beside_hole(const GameState *g, int x, int y) {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (g->map.tiles[y + dy][x + dx] == TILE_FROST_LAKE_HOLE) {
                return 1;
            }
        }
    }
    return 0;
}

// Finds open lake ice at least two tiles from the Kraken, beside a hole or not.
static int find_lake_spot(const GameState *g, const Enemy *kraken, int want_hole, int *out_x, int *out_y) {
    for (int y = 1; y < SWAMP_MAP_H - 1; y++) {
        for (int x = 1; x < SWAMP_MAP_W - 1; x++) {
            if (g->map.tiles[y][x] == TILE_FROST_LAKE &&
                steps_from(x, y, kraken->x, kraken->y) >= 2 &&
                beside_hole(g, x, y) == want_hole) {
                *out_x = x;
                *out_y = y;
                return 1;
            }
        }
    }
    return 0;
}

static void test_polar_kraken(void) {
    int lakes_ok = 1;
    for (int seed = 1; seed <= 20; seed++) {
        srand((unsigned int)(seed * 13));
        map_generate_frostfell(&frost_layout, FROSTFELL_DEPTH);
        const Room *lake = &frost_layout.rooms[frost_layout.room_count - 1];
        lakes_ok &= lake_hole_count(&frost_layout, lake) >= 4 &&
            frost_layout_connected(&frost_layout, 0);
    }
    ASSERT("stage 5 ends in a frozen lake with at least four ice holes",
        lakes_ok);

    memset(&frost_game, 0, sizeof(frost_game));
    frost_game.player.player_class = CLASS_ROGUE;
    game_init(&frost_game);
    srand(808);
    frost_game.location = LOCATION_FROSTFELL;
    frost_game.level = FROSTFELL_DEPTH;
    map_generate_frostfell(&frost_game.map, FROSTFELL_DEPTH);
    enemies_spawn(&frost_game);
    int k = find_kraken(&frost_game);
    for (int i = 0; i < frost_game.enemy_count; i++) {
        if (i != k) {
            frost_game.enemies[i].active = 0;
        }
    }
    Enemy *kraken = &frost_game.enemies[k];
    frost_game.player.hp = 10000;
    frost_game.player.max_hp = 10000;
    frost_game.player.defense = 0;
    frost_game.equipped_off_hand = -1;
    frost_game.player.x = frost_game.map.stairs_up_x;
    frost_game.player.y = frost_game.map.stairs_up_y;
    action_resolve_enemies(&frost_game);
    ASSERT("the Kraken stays still until the player steps onto its lake",
        k >= 0 && kraken->move_timer == 0 && frost_game.player.hp == 10000);

    int hole_x = 0;
    int hole_y = 0;
    int clear_x = 0;
    int clear_y = 0;
    int spots = find_lake_spot(&frost_game, kraken, 1, &hole_x, &hole_y) &&
        find_lake_spot(&frost_game, kraken, 0, &clear_x, &clear_y);
    int kraken_x = kraken->x;
    int kraken_y = kraken->y;
    frost_game.player.x = hole_x;
    frost_game.player.y = hole_y;
    action_resolve_enemies(&frost_game);
    int warned = frost_game.player.hp == 10000 &&
        strcmp(frost_game.messages[frost_game.message_count - 1],
            "Tentacles rise! Move off the marked tile and away from holes!") == 0;
    action_resolve_enemies(&frost_game);
    int struck = frost_game.player.hp < 10000 &&
        strstr(frost_game.messages[frost_game.message_count - 1], "Kraken tentacle") != NULL;
    ASSERT("the Kraken warns for a turn, then its tentacles hit a player beside a hole",
        spots && warned && struck);

    frost_game.player.hp = 10000;
    frost_game.player.x = clear_x;
    frost_game.player.y = clear_y;
    action_resolve_enemies(&frost_game);
    action_resolve_enemies(&frost_game);
    ASSERT("standing still away from the holes is struck without moving the Kraken",
        frost_game.player.hp < 10000 && kraken->x == kraken_x && kraken->y == kraken_y);

    frost_game.player.known_spell_count = 1;
    frost_game.player.known_spells[0] = spell_make_frost_bolt();
    frost_game.player.equipped_spell = 0;
    frost_game.player.mp = 100;
    frost_game.player.x = kraken->x + 2;
    frost_game.player.y = kraken->y;
    frost_game.player.last_dx = -1;
    frost_game.player.last_dy = 0;
    int hp_before = kraken->hp;
    action_resolve_player(&frost_game, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("Frost Bolt hurts the Polar Kraken but cannot freeze it",
        kraken->hp < hp_before && kraken->frozen_turns == 0);

    frost_game.player.x = frost_game.map.stairs_down_x + 1;
    frost_game.player.y = frost_game.map.stairs_down_y;
    walk_onto(&frost_game, frost_game.map.stairs_down_x, frost_game.map.stairs_down_y);
    int blocked = frost_game.location == LOCATION_FROSTFELL &&
        strcmp(frost_game.messages[frost_game.message_count - 1],
            "The Polar Kraken bars the way west!") == 0;

    frost_game.player.x = kraken->x + 1;
    frost_game.player.y = kraken->y;
    kraken->hp = 1;
    walk_onto(&frost_game, kraken->x, kraken->y);
    int bow_dropped = 0;
    for (int i = 0; i < frost_game.floor_item_count; i++) {
        bow_dropped |= frost_game.floor_items[i].active &&
            strcmp(frost_game.floor_items[i].item.name, "Krakenbone Bow") == 0;
    }
    BossJournalEntry entry;
    int journal_ok = quest_journal_get_boss(&frost_game, 7, &entry) &&
        strcmp(entry.name, "Polar Kraken") == 0 && entry.defeated;
    ASSERT("the Kraken bars the exit, then drops the Krakenbone Bow and is marked defeated",
        blocked && !kraken->active && bow_dropped && journal_ok &&
        (frost_game.defeated_bosses & (1 << LOCATION_FROSTFELL)));

    frost_game.player.x = frost_game.map.stairs_down_x + 1;
    frost_game.player.y = frost_game.map.stairs_down_y;
    walk_onto(&frost_game, frost_game.map.stairs_down_x, frost_game.map.stairs_down_y);
    int left = frost_game.location == LOCATION_TOWN3;
    frost_game.location = LOCATION_FROSTFELL;
    frost_game.level = FROSTFELL_DEPTH;
    map_generate_frostfell(&frost_game.map, FROSTFELL_DEPTH);
    enemies_spawn(&frost_game);
    ASSERT("with the Kraken dead the exit opens and it never returns",
        left && find_kraken(&frost_game) < 0);
}

// An open snowfield with the player at (20, 20) and one frost creature.
static void setup_snowfield(GameState *g, EnemyType type, int x, int y) {
    memset(g, 0, sizeof(*g));
    g->player.player_class = CLASS_MAGE;
    game_init(g);
    g->location = LOCATION_FROSTFELL;
    g->level = 3;
    g->map.room_count = 1;
    g->map.rooms[0] = (Room){8, 8, 26, 26};
    for (int ty = 8; ty < 34; ty++) {
        for (int tx = 8; tx < 34; tx++) {
            g->map.tiles[ty][tx] = TILE_FROST_FLOOR;
        }
    }
    g->player.x = 20;
    g->player.y = 20;
    g->player.hp = 10000;
    g->player.max_hp = 10000;
    g->player.mp = 100;
    g->player.defense = 0;
    g->player.last_dx = 1;
    g->player.last_dy = 0;
    g->equipped_armor = -1;
    g->equipped_off_hand = -1;
    g->enemy_count = 1;
    g->enemies[0] = (Enemy){
        .type = type, .x = x, .y = y, .active = 1,
        .hp = 500, .max_hp = 500, .attack = 12
    };
}

static void test_frostfell_behaviours(void) {
    setup_snowfield(&frost_game, ENEMY_YETI, 30, 30);
    frost_game.player.frozen_turns = 1;
    action_resolve_player(&frost_game, (Action){ACTION_MOVE, 21, 20});
    int lost_turn = frost_game.player.x == 20 && frost_game.player.frozen_turns == 0 &&
        strcmp(frost_game.messages[frost_game.message_count - 1],
            "You are frozen solid and lose a turn!") == 0;
    action_resolve_player(&frost_game, (Action){ACTION_MOVE, 21, 20});
    ASSERT("a frozen player loses one turn, then moves again",
        lost_turn && frost_game.player.x == 21);

    frost_game.player.frozen_turns = 1;
    frost_game.player.hp = 50;
    frost_game.inventory[0] = item_make_health_potion();
    frost_game.inventory_count = 1;
    action_resolve_player(&frost_game, (Action){ACTION_USE_ITEM, 0, 0});
    ASSERT("drinking a potion while frozen restores health and spends the frozen turn",
        frost_game.player.hp == frost_game.player.max_hp &&
        frost_game.player.frozen_turns == 0 && frost_game.player.freeze_recovery == 1);

    setup_snowfield(&frost_game, ENEMY_FROST_WRAITH, 21, 20);
    srand(99);
    int freezes = 0;
    int frozen_for_one_turn = 1;
    for (int turn = 0; turn < 400; turn++) {
        frost_game.player.frozen_turns = 0;
        frost_game.player.hp = 10000;
        action_resolve_enemies(&frost_game);
        if (frost_game.player.frozen_turns > 0) {
            freezes++;
            frozen_for_one_turn &= frost_game.player.frozen_turns == 1;
        }
    }
    ASSERT("about one Frost Wraith hit in four freezes the player for a turn",
        freezes >= 60 && freezes <= 140 && frozen_for_one_turn);

    setup_snowfield(&frost_game, ENEMY_ICE_GOLEM, 23, 20);
    frost_game.player.known_spell_count = 1;
    frost_game.player.known_spells[0] = spell_make_frost_bolt();
    frost_game.player.equipped_spell = 0;
    action_resolve_player(&frost_game, (Action){ACTION_CAST_SPELL, 0, 0});
    int golem_hurt = frost_game.enemies[0].hp < 500;
    int golem_unfrozen = frost_game.enemies[0].frozen_turns == 0;
    frost_game.enemies[0].type = ENEMY_YETI;
    frost_game.player.mp = 100;
    action_resolve_player(&frost_game, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("Ice Golems take Frost Bolt damage but never freeze",
        golem_hurt && golem_unfrozen && frost_game.enemies[0].frozen_turns == 2);

    setup_snowfield(&frost_game, ENEMY_FROST_ARCHER, 24, 20);
    EnemyProjectiles shots = {0};
    int arrows = 0;
    for (int turn = 0; turn < 2; turn++) {
        action_resolve_enemies_with_projectiles(&frost_game, &shots);
        arrows += shots.count;
    }
    ASSERT("Frost Archers shoot down a clear line instead of closing in",
        arrows == 1 && frost_game.enemies[0].x == 24 &&
        frost_game.player.hp < 10000 &&
        strstr(frost_game.messages[frost_game.message_count - 1], "Frost arrow") != NULL);

    // Six tiles away is inside the distance at which enemies notice the player.
    setup_snowfield(&frost_game, ENEMY_ICE_WOLF, 26, 20);
    action_resolve_enemies(&frost_game);
    int wolf_steps = steps_from(26, 20, frost_game.enemies[0].x, frost_game.enemies[0].y);
    setup_snowfield(&frost_game, ENEMY_ICE_GOLEM, 26, 20);
    action_resolve_enemies(&frost_game);
    action_resolve_enemies(&frost_game);
    int golem_steps = steps_from(26, 20, frost_game.enemies[0].x, frost_game.enemies[0].y);
    ASSERT("Ice Wolves cover two tiles a turn while Ice Golems take two turns per tile",
        wolf_steps == 2 && golem_steps == 1);
}

static void test_freeze_recovery(void) {
    setup_snowfield(&frost_game, ENEMY_FROST_WRAITH, 21, 20);
    for (int i = 1; i < 3; i++) {
        frost_game.enemies[i] = frost_game.enemies[0];
        frost_game.enemies[i].x = 19 + i;
        frost_game.enemies[i].y = 19;
    }
    frost_game.enemy_count = 3;
    frost_game.player.frozen_turns = 1;
    action_resolve_player(&frost_game, (Action){ACTION_MOVE, 20, 21});
    int saved = save_game(&frost_game, 99018) && load_game(&frost_loaded, 99018);
    ASSERT("thaw recovery survives saving between the player and enemy turns",
        saved && frost_loaded.player.frozen_turns == 0 && frost_loaded.player.freeze_recovery == 1);
    remove("saves/savegame_99018.json");
    frost_game = frost_loaded;
    action_resolve_enemies(&frost_game);
    walk_onto(&frost_game, 20, 21);
    ASSERT("all three wraiths leave a thawed player able to move next turn",
        frost_game.player.x == 20 && frost_game.player.y == 21 &&
        frost_game.player.freeze_recovery == 0 && frost_game.player.frozen_turns == 0);

    frost_game.player.y = 20;
    int previous_frozen = 0;
    int consecutive = 0;
    int lost = 0;
    srand(999);
    for (int turn = 0; turn < 500; turn++) {
        frost_game.player.hp = 10000;
        int frozen = frost_game.player.frozen_turns;
        consecutive |= frozen && previous_frozen;
        lost += frozen;
        previous_frozen = frozen;
        action_resolve_player(&frost_game, (Action){ACTION_PICK_UP, 0, 0});
        action_resolve_enemies(&frost_game);
    }
    ASSERT("wraiths can freeze again but never cost consecutive player turns",
        lost > 0 && !consecutive);
}

static int setup_kraken_encounter(int seed) {
    memset(&frost_game, 0, sizeof(frost_game));
    frost_game.player.player_class = CLASS_ROGUE;
    game_init(&frost_game);
    player_gain_xp(&frost_game, 2800);
    frost_game.defeated_bosses = 1 << LOCATION_FOREST;
    frost_game.location = LOCATION_FROSTFELL;
    frost_game.level = FROSTFELL_DEPTH;
    frost_game.max_frostfell_level_reached = FROSTFELL_DEPTH;
    srand((unsigned int)seed);
    map_generate_frostfell(&frost_game.map, frost_game.level);
    enemies_spawn(&frost_game);
    int k = find_kraken(&frost_game);
    for (int i = 0; i < frost_game.enemy_count; i++) {
        frost_game.enemies[i].active = i == k;
    }
    frost_game.inventory[0] = item_make_longbow();
    action_resolve_player(&frost_game, (Action){ACTION_EQUIP_ITEM, 0, 0});
    return k;
}

static int aim_from_clear_ice(GameState *g, const Enemy *e) {
    const int dx[8] = {0, 1, 0, -1, 1, 1, -1, -1};
    const int dy[8] = {-1, 0, 1, 0, -1, 1, -1, 1};
    for (int d = 0; d < 8; d++) {
        for (int step = 1; step <= 9; step++) {
            int x = e->x + dx[d] * step;
            int y = e->y + dy[d] * step;
            if (!map_is_walkable(&g->map, x, y)) {
                break;
            }
            if (step >= 2 && !beside_hole(g, x, y)) {
                g->player.x = x;
                g->player.y = y;
                g->player.last_dx = -dx[d];
                g->player.last_dy = -dy[d];
                return 1;
            }
        }
    }
    return 0;
}

static void test_kraken_targeted_strike(void) {
    int punished = 1;
    for (int seed = 1; seed <= 100; seed++) {
        int k = setup_kraken_encounter(seed);
        if (k < 0 || !aim_from_clear_ice(&frost_game, &frost_game.enemies[k])) {
            punished = 0;
            continue;
        }
        Enemy *e = &frost_game.enemies[k];
        int hp = frost_game.player.hp;
        action_resolve_player(&frost_game, (Action){ACTION_RANGED_ATTACK, 0, 0});
        action_resolve_enemies(&frost_game);
        punished &= e->attack_target_x == frost_game.player.x &&
            e->attack_target_y == frost_game.player.y && frost_game.player.hp == hp;
        action_resolve_player(&frost_game, (Action){ACTION_RANGED_ATTACK, 0, 0});
        action_resolve_enemies(&frost_game);
        punished &= frost_game.player.hp < hp && e->attack_target_x == -1;
    }
    ASSERT("stationary bow shots on clear ice draw a warned strike on 100 maps", punished);

    int k = setup_kraken_encounter(808);
    Enemy *e = &frost_game.enemies[k];
    aim_from_clear_ice(&frost_game, e);
    action_resolve_player(&frost_game, (Action){ACTION_RANGED_ATTACK, 0, 0});
    action_resolve_enemies(&frost_game);
    int x = frost_game.player.x;
    int y = frost_game.player.y;
    int saved = save_game(&frost_game, 99019) && load_game(&frost_loaded, 99019);
    ASSERT("a pending Kraken strike keeps its marked tile after save and load",
        saved && frost_loaded.enemies[k].attack_target_x == x &&
        frost_loaded.enemies[k].attack_target_y == y && frost_loaded.enemies[k].move_timer % 2 == 1);
    remove("saves/savegame_99019.json");
    frost_game = frost_loaded;
    e = &frost_game.enemies[k];
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    int moved = 0;
    for (int d = 0; d < 4; d++) {
        int nx = x + dx[d];
        int ny = y + dy[d];
        if (map_is_walkable(&frost_game.map, nx, ny) &&
            !beside_hole(&frost_game, nx, ny) && steps_from(nx, ny, e->x, e->y) > 1) {
            walk_onto(&frost_game, nx, ny);
            moved = 1;
            break;
        }
    }
    int hp = frost_game.player.hp;
    action_resolve_enemies(&frost_game);
    ASSERT("one step off the marked tile and clear of holes dodges the strike",
        moved && frost_game.player.hp == hp && e->attack_target_x == -1);

    action_resolve_enemies(&frost_game);
    game_open_town_portal(&frost_game);
    game_use_town_portal(&frost_game);
    e = &frost_game.enemies[k];
    hp = frost_game.player.hp;
    action_resolve_enemies(&frost_game);
    ASSERT("a portal trip cannot erase the Kraken's pending strike",
        frost_game.player.hp < hp && e->attack_target_x == -1);

    frost_game.player.x = frost_game.map.stairs_up_x;
    frost_game.player.y = frost_game.map.stairs_up_y;
    action_resolve_enemies(&frost_game);
    ASSERT("retreating across the map cancels the Kraken's targeting",
        e->move_timer == 0 && e->attack_target_x == -1 && e->attack_target_y == -1);
}

static int kraken_bows_on_floor(const GameState *g) {
    int count = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        count += g->floor_items[i].active &&
            strcmp(g->floor_items[i].item.name, "Krakenbone Bow") == 0;
    }
    return count;
}

static void test_kraken_reward_persistence(void) {
    int k = setup_kraken_encounter(808);
    Enemy *e = &frost_game.enemies[k];
    aim_from_clear_ice(&frost_game, e);
    e->hp = 1;
    action_resolve_player(&frost_game, (Action){ACTION_RANGED_ATTACK, 0, 0});
    int dropped = frost_game.kraken_bow_unclaimed && kraken_bows_on_floor(&frost_game) == 1;
    game_open_town_portal(&frost_game);
    int saved = save_game(&frost_game, 99020) && load_game(&frost_loaded, 99020);
    frost_game = frost_loaded;
    game_use_town_portal(&frost_game);
    game_refresh_quest_encounters(&frost_game);
    game_refresh_quest_encounters(&frost_game);
    ASSERT("the unclaimed bow survives a town save and portal return exactly once",
        dropped && saved && frost_game.kraken_bow_unclaimed &&
        kraken_bows_on_floor(&frost_game) == 1 && find_kraken(&frost_game) < 0);

    game_ascend(&frost_game);
    game_descend(&frost_game);
    ASSERT("backtracking to stage four preserves the bow on stage five",
        frost_game.level == FROSTFELL_DEPTH && kraken_bows_on_floor(&frost_game) == 1);

    game_return_to_town(&frost_game);
    game_enter_frostfell(&frost_game);
    while (frost_game.level < FROSTFELL_DEPTH) {
        game_descend(&frost_game);
    }
    ASSERT("the unclaimed bow returns on a fresh Frostfell map without respawning the boss",
        kraken_bows_on_floor(&frost_game) == 1 && find_kraken(&frost_game) < 0);
    int x;
    int y;
    map_room_center(&frost_game.map.rooms[frost_game.map.room_count - 1], &x, &y);
    frost_game.player.x = x;
    frost_game.player.y = y;
    int inventory_count = frost_game.inventory_count;
    frost_game.inventory_count = MAX_INVENTORY;
    action_resolve_player(&frost_game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("a full inventory leaves the bow unclaimed",
        frost_game.kraken_bow_unclaimed && kraken_bows_on_floor(&frost_game) == 1);
    frost_game.inventory_count = inventory_count;
    action_resolve_player(&frost_game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("collecting the bow clears its claim and restores the lake tile",
        !frost_game.kraken_bow_unclaimed && kraken_bows_on_floor(&frost_game) == 0 &&
        frost_game.map.tiles[y][x] == TILE_FROST_LAKE);
    game_open_town_portal(&frost_game);
    saved = save_game(&frost_game, 99020) && load_game(&frost_loaded, 99020);
    frost_game = frost_loaded;
    game_use_town_portal(&frost_game);
    int in_pack = 0;
    int metadata_ok = 0;
    for (int i = 0; i < frost_game.inventory_count; i++) {
        const Item *item = &frost_game.inventory[i];
        if (strcmp(item->name, "Krakenbone Bow") == 0) {
            in_pack++;
            metadata_ok = item->attack_bonus == 10 && item->range == 10 &&
                item->weapon_family == WEAPON_FAMILY_BOW && item->class_mask == ITEM_CLASS_ROGUE;
        }
    }
    ASSERT("saving and revisiting after pickup cannot duplicate the bow",
        saved && in_pack == 1 && metadata_ok && !frost_game.kraken_bow_unclaimed &&
        kraken_bows_on_floor(&frost_game) == 0);
    remove("saves/savegame_99020.json");
}

// Paints slick ice from x0 to x1 along row y of the test snowfield.
static void paint_ice(GameState *g, int x0, int x1, int y) {
    for (int x = x0; x <= x1; x++) {
        g->map.tiles[y][x] = TILE_FROST_ICE;
    }
}

static int message_count_of(const GameState *g, const char *text) {
    int count = 0;
    for (int i = 0; i < g->message_count; i++) {
        count += strcmp(g->messages[i], text) == 0;
    }
    return count;
}

static void test_slick_ice(void) {
    // A live Yeti waits off the path; it must not get a turn mid-slide.
    setup_snowfield(&frost_game, ENEMY_YETI, 25, 26);
    paint_ice(&frost_game, 21, 24, 20);
    frost_game.player.poison_turns = 2;
    walk_onto(&frost_game, 21, 20);
    ASSERT("stepping onto slick ice slides the player to the snow beyond in one turn",
        frost_game.player.x == 25 && frost_game.player.y == 20 &&
        frost_game.player.poison_turns == 1 && frost_game.player.hp == 10000 - 3 &&
        frost_game.enemies[0].x == 25 && frost_game.enemies[0].y == 26 &&
        message_count_of(&frost_game, "You slide across the ice.") == 1);
    ASSERT("a slide records its path from the first ice tile to where it stops",
        frost_game.trail_effect == TRAIL_EFFECT_ICE_SLIDE && frost_game.trail_frames > 0 &&
        frost_game.trail_count == 5 && frost_game.trail[0].x == 21 &&
        frost_game.trail[4].x == 25 && frost_game.trail[4].y == 20);

    setup_snowfield(&frost_game, ENEMY_YETI, 23, 20);
    frost_game.enemies[0].active = 0;
    paint_ice(&frost_game, 21, 24, 20);
    walk_onto(&frost_game, 21, 20);
    ASSERT("a fallen creature lying on the ice does not stop a slide",
        frost_game.player.x == 25);

    setup_snowfield(&frost_game, ENEMY_YETI, 22, 20);
    paint_ice(&frost_game, 21, 22, 20);
    walk_onto(&frost_game, 21, 20);
    ASSERT("a step onto ice that cannot slide stays put and says nothing about sliding",
        frost_game.player.x == 21 && frost_game.trail_effect != TRAIL_EFFECT_ICE_SLIDE &&
        message_count_of(&frost_game, "You slide across the ice.") == 0);

    setup_snowfield(&frost_game, ENEMY_YETI, 30, 30);
    frost_game.enemies[0].active = 0;
    paint_ice(&frost_game, 21, 24, 20);
    frost_game.map.tiles[20][25] = TILE_FROST_WALL;
    walk_onto(&frost_game, 21, 20);
    int stopped_on_ice = frost_game.player.x == 24;
    walk_onto(&frost_game, 23, 20);
    ASSERT("a wall stops the slide on the ice, and the next step slides back to snow",
        stopped_on_ice && frost_game.player.x == 20 && frost_game.player.y == 20);

    setup_snowfield(&frost_game, ENEMY_YETI, 24, 20);
    paint_ice(&frost_game, 21, 24, 20);
    walk_onto(&frost_game, 21, 20);
    ASSERT("a creature on the ice stops the slide beside it without an attack",
        frost_game.player.x == 23 && frost_game.enemies[0].hp == 500);

    setup_snowfield(&frost_game, ENEMY_YETI, 30, 30);
    frost_game.enemies[0].active = 0;
    paint_ice(&frost_game, 21, 24, 20);
    frost_game.map.tiles[20][25] = TILE_FROST_EXIT;
    frost_game.map.tiles[21][24] = TILE_FROST_ICE;
    frost_game.map.tiles[22][24] = TILE_ITEM;
    frost_game.floor_items[0] = (FloorItem){
        .active = 1, .x = 24, .y = 22, .underlying_tile = TILE_FROST_FLOOR,
        .item = item_make_health_potion()
    };
    frost_game.floor_item_count = 1;
    walk_onto(&frost_game, 21, 20);
    int stopped_before_exit = frost_game.player.x == 24 && frost_game.level == 3 &&
        frost_game.location == LOCATION_FROSTFELL;
    walk_onto(&frost_game, 24, 21);
    ASSERT("slides stop short of exits and items instead of using them",
        stopped_before_exit && frost_game.player.x == 24 && frost_game.player.y == 21 &&
        frost_game.floor_items[0].active && frost_game.inventory_count < MAX_INVENTORY);

    setup_snowfield(&frost_game, ENEMY_YETI, 30, 30);
    frost_game.enemies[0].active = 0;
    paint_ice(&frost_game, 21, 24, 20);
    frost_game.player.x = 22;
    frost_game.player.frozen_turns = 1;
    walk_onto(&frost_game, 23, 20);
    ASSERT("a frozen player standing on ice neither steps nor slides",
        frost_game.player.x == 22 && frost_game.player.frozen_turns == 0 &&
        message_count_of(&frost_game, "You slide across the ice.") == 0);

    setup_snowfield(&frost_game, ENEMY_YETI, 26, 20);
    paint_ice(&frost_game, 21, 25, 20);
    action_resolve_enemies(&frost_game);
    ASSERT("creatures keep their footing on slick ice",
        frost_game.enemies[0].x == 25 && frost_game.enemies[0].y == 20);
}

// A wall band across rows 22-24 of the test snowfield, crossed by a three-tile
// thin-ice shortcut at x=20. The player starts on the snow just above it.
static void build_thin_ice_tunnel(GameState *g) {
    for (int x = 8; x < 34; x++) {
        for (int y = 22; y <= 24; y++) {
            g->map.tiles[y][x] = TILE_FROST_WALL;
        }
    }
    for (int y = 22; y <= 24; y++) {
        g->map.tiles[y][20] = TILE_FROST_THIN_ICE;
    }
    g->player.y = 21;
}

static int tunnel_tiles_are(const GameState *g, TileType tile) {
    return g->map.tiles[22][20] == tile && g->map.tiles[23][20] == tile &&
        g->map.tiles[24][20] == tile;
}

static void name_creature(Enemy *e, const char *name, int experience) {
    snprintf(e->name, sizeof(e->name), "%s", name);
    e->experience = experience;
}

static void test_thin_ice(void) {
    setup_snowfield(&frost_game, ENEMY_YETI, 30, 30);
    frost_game.enemies[0].active = 0;
    build_thin_ice_tunnel(&frost_game);
    walk_onto(&frost_game, 20, 22);
    int creaked = message_count_of(&frost_game,
        "The thin ice creaks. It will give way once you step off.") == 1;
    walk_onto(&frost_game, 20, 23);
    walk_onto(&frost_game, 20, 24);
    ASSERT("thin ice creaks once when stepped on and holds while walked along",
        creaked && frost_game.player.y == 24 &&
        tunnel_tiles_are(&frost_game, TILE_FROST_THIN_ICE) &&
        message_count_of(&frost_game,
            "The thin ice creaks. It will give way once you step off.") == 1);

    walk_onto(&frost_game, 20, 25);
    int collapsed = frost_game.player.y == 25 &&
        tunnel_tiles_are(&frost_game, TILE_FROST_BROKEN_ICE) &&
        !map_is_walkable(&frost_game.map, 20, 24) &&
        map_is_explored(&frost_game.map, 20, 22) && map_is_explored(&frost_game.map, 20, 23) &&
        map_is_explored(&frost_game.map, 20, 24) &&
        message_count_of(&frost_game, "The thin ice collapses behind you!") == 1;
    walk_onto(&frost_game, 20, 24);
    ASSERT("stepping off collapses the whole shortcut into water that cannot be crossed back",
        collapsed && frost_game.player.y == 25);

    setup_snowfield(&frost_game, ENEMY_YETI, 30, 30);
    frost_game.enemies[0].active = 0;
    build_thin_ice_tunnel(&frost_game);
    walk_onto(&frost_game, 20, 22);
    walk_onto(&frost_game, 20, 21);
    ASSERT("backing straight off still collapses the whole shortcut, never just part of it",
        frost_game.player.y == 21 && tunnel_tiles_are(&frost_game, TILE_FROST_BROKEN_ICE));

    setup_snowfield(&frost_game, ENEMY_YETI, 20, 24);
    name_creature(&frost_game.enemies[0], "Yeti", 70);
    build_thin_ice_tunnel(&frost_game);
    int gold_before = frost_game.gold;
    int score_before = frost_game.score;
    walk_onto(&frost_game, 20, 22);
    walk_onto(&frost_game, 20, 21);
    ASSERT("a creature on a collapsing shortcut falls through as a normal kill",
        !frost_game.enemies[0].active && frost_game.player.experience == 70 &&
        frost_game.score - score_before == frost_game.gold - gold_before &&
        frost_game.level_cleared == 1 &&
        message_count_of(&frost_game, "The thin ice collapses, drowning the Yeti!") == 1);

    // Over many drownings some coins are rolled; each goes straight to the
    // player and none is left on the water.
    setup_snowfield(&frost_game, ENEMY_YETI, 20, 24);
    srand(1);
    gold_before = frost_game.gold;
    score_before = frost_game.score;
    for (int i = 0; i < 100; i++) {
        frost_game.enemies[0].active = 1;
        frost_game.enemies[0].x = 20;
        frost_game.enemies[0].y = 24;
        build_thin_ice_tunnel(&frost_game);
        walk_onto(&frost_game, 20, 22);
        walk_onto(&frost_game, 20, 21);
    }
    ASSERT("drowned creatures pay their gold straight to the player",
        frost_game.gold > gold_before && frost_game.floor_item_count == 0 &&
        frost_game.score - score_before == frost_game.gold - gold_before);

    setup_snowfield(&frost_game, ENEMY_YETI, 20, 24);
    name_creature(&frost_game.enemies[0], "Yeti", 70);
    frost_game.enemies[0].is_boss = 1;
    build_thin_ice_tunnel(&frost_game);
    walk_onto(&frost_game, 20, 22);
    walk_onto(&frost_game, 20, 21);
    ASSERT("bosses never fall through a collapsing shortcut",
        frost_game.enemies[0].active && frost_game.player.experience == 0 &&
        message_count_of(&frost_game, "The thin ice collapses behind you!") == 1);

    setup_snowfield(&frost_game, ENEMY_YETI, 20, 23);
    name_creature(&frost_game.enemies[0], "Yeti", 70);
    frost_game.enemies[0].active = 0;
    build_thin_ice_tunnel(&frost_game);
    walk_onto(&frost_game, 20, 22);
    walk_onto(&frost_game, 20, 21);
    ASSERT("a creature already dead on the shortcut is not killed again",
        frost_game.player.experience == 0 &&
        message_count_of(&frost_game, "The thin ice collapses behind you!") == 1);

    setup_snowfield(&frost_game, ENEMY_YETI, 26, 23);
    name_creature(&frost_game.enemies[0], "Yeti", 70);
    build_thin_ice_tunnel(&frost_game);
    for (int y = 22; y <= 24; y++) {
        frost_game.map.tiles[y][26] = TILE_FROST_THIN_ICE;
    }
    walk_onto(&frost_game, 20, 22);
    walk_onto(&frost_game, 20, 21);
    ASSERT("a collapse takes only its own shortcut and the creatures on it",
        tunnel_tiles_are(&frost_game, TILE_FROST_BROKEN_ICE) &&
        frost_game.map.tiles[22][26] == TILE_FROST_THIN_ICE &&
        frost_game.map.tiles[23][26] == TILE_FROST_THIN_ICE &&
        frost_game.map.tiles[24][26] == TILE_FROST_THIN_ICE &&
        frost_game.enemies[0].active && frost_game.player.experience == 0);

    setup_snowfield(&frost_game, ENEMY_YETI, 20, 24);
    name_creature(&frost_game.enemies[0], "Yeti", 30);
    frost_game.enemies[1] = frost_game.enemies[0];
    frost_game.enemies[1].y = 23;
    frost_game.enemies[1].experience = 40;
    frost_game.enemy_count = 2;
    build_thin_ice_tunnel(&frost_game);
    walk_onto(&frost_game, 20, 22);
    frost_game.message_count = 0;
    walk_onto(&frost_game, 20, 21);
    int only_one_collapse_line = 1;
    for (int i = 0; i < frost_game.message_count; i++) {
        only_one_collapse_line &=
            strcmp(frost_game.messages[i], "The thin ice collapses, drowning 2 creatures!") == 0 ||
            strncmp(frost_game.messages[i], "Found ", 6) == 0;
    }
    ASSERT("several creatures falling through share one message and both count",
        !frost_game.enemies[0].active && !frost_game.enemies[1].active &&
        frost_game.player.experience == 70 && only_one_collapse_line &&
        message_count_of(&frost_game, "The thin ice collapses, drowning 2 creatures!") == 1);

    setup_snowfield(&frost_game, ENEMY_YETI, 20, 26);
    build_thin_ice_tunnel(&frost_game);
    frost_game.player.y = 19;
    for (int turn = 0; turn < 6; turn++) {
        action_resolve_enemies(&frost_game);
    }
    ASSERT("creatures cross thin ice and step off it without breaking it",
        frost_game.enemies[0].y == 20 && tunnel_tiles_are(&frost_game, TILE_FROST_THIN_ICE));

    setup_snowfield(&frost_game, ENEMY_YETI, 30, 30);
    frost_game.enemies[0].active = 0;
    build_thin_ice_tunnel(&frost_game);
    frost_game.player.y = 22;
    frost_game.inventory[0] = item_make_health_potion();
    frost_game.inventory_count = 1;
    action_resolve_player(&frost_game, (Action){ACTION_DROP_ITEM, 0, 0});
    int refused = frost_game.inventory_count == 1 && frost_game.floor_item_count == 0 &&
        frost_game.map.tiles[22][20] == TILE_FROST_THIN_ICE &&
        message_count_of(&frost_game,
            "Anything dropped here would sink when the ice gives way.") == 1;
    frost_game.player.y = 21;
    action_resolve_player(&frost_game, (Action){ACTION_DROP_ITEM, 0, 0});
    ASSERT("nothing can be dropped on thin ice, though snow beside it takes a drop",
        refused && frost_game.inventory_count == 0 && frost_game.floor_item_count == 1);

    // Teleport counts like a step: landing on the ice creaks, leaving it
    // collapses the shortcut, and jumping clean over it leaves it whole.
    setup_snowfield(&frost_game, ENEMY_YETI, 30, 30);
    frost_game.enemies[0].active = 0;
    build_thin_ice_tunnel(&frost_game);
    frost_game.player.known_spell_count = 1;
    frost_game.player.known_spells[0] = spell_make_teleport();
    frost_game.player.known_spells[0].range = 2;
    frost_game.player.equipped_spell = 0;
    frost_game.player.last_dx = 0;
    frost_game.player.last_dy = 1;
    action_resolve_player(&frost_game, (Action){ACTION_CAST_SPELL, 0, 0});
    int landed = frost_game.player.y == 23 && tunnel_tiles_are(&frost_game, TILE_FROST_THIN_ICE) &&
        message_count_of(&frost_game,
            "The thin ice creaks. It will give way once you step off.") == 1;
    action_resolve_player(&frost_game, (Action){ACTION_CAST_SPELL, 0, 0});
    int left = frost_game.player.y == 25 && tunnel_tiles_are(&frost_game, TILE_FROST_BROKEN_ICE);
    build_thin_ice_tunnel(&frost_game);
    frost_game.player.known_spells[0].range = 4;
    frost_game.player.mp = 100;
    action_resolve_player(&frost_game, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("teleporting onto thin ice creaks, off it collapses it, and over it leaves it whole",
        landed && left && frost_game.player.y == 25 &&
        tunnel_tiles_are(&frost_game, TILE_FROST_THIN_ICE));
}

void test_frostfell(void) {
    printf("Frostfell Wastes tests:\n");
    test_frostfell_rosters();
    test_frostfell_behaviours();
    test_freeze_recovery();
    test_kraken_targeted_strike();
    test_kraken_reward_persistence();
    test_slick_ice();
    test_thin_ice();
    test_polar_kraken();

    int layouts_ok = 1;
    int patch_counts_ok = 1;
    int patches_safe = 1;
    int rand_untouched = 1;
    int tunnel_counts_ok = 1;
    int stage_tunnels[FROSTFELL_DEPTH + 1] = {0};
    int tunnels_clean = 1;
    int tunnels_optional = 1;
    for (int level = 1; level <= FROSTFELL_DEPTH; level++) {
        for (int seed = 1; seed <= 20; seed++) {
            srand((unsigned int)(seed * 31 + level));
            map_generate_swamp(&frost_layout, level);
            int swamp_next = rand();
            srand((unsigned int)(seed * 31 + level));
            map_generate_frostfell(&frost_layout, level);
            rand_untouched &= rand() == swamp_next;
            patch_counts_ok &= count_tile_runs(&frost_layout, TILE_FROST_ICE, 1, 15, 15) == level - 1;
            patches_safe &= ice_patches_ringed(&frost_layout) &&
                frost_layout_connected(&frost_layout, AVOID_SLICK_ICE);
            int tunnels = count_tile_runs(&frost_layout, TILE_FROST_THIN_ICE, 0, 2, 14);
            tunnel_counts_ok &= tunnels >= 0 &&
                (level < 3 ? tunnels == 0 : tunnels <= level - 2);
            stage_tunnels[level] += tunnels > 0 ? tunnels : 0;
            tunnels_clean &= thin_ice_tunnels_clean(&frost_layout, level);
            tunnels_optional &= frost_layout_connected(&frost_layout, AVOID_THIN_ICE);
            layouts_ok &= frost_layout.stairs_up_x == SWAMP_MAP_W - 1 &&
                frost_layout.stairs_down_x == 0 &&
                frost_layout.tiles[frost_layout.stairs_up_y][SWAMP_MAP_W - 1] ==
                    TILE_FROST_ENTRANCE &&
                frost_layout.tiles[frost_layout.stairs_down_y][0] == TILE_FROST_EXIT &&
                frost_layout_snowed_over(&frost_layout) &&
                frost_layout_connected(&frost_layout, 0);
        }
    }
    ASSERT("Frostfell stages run east to west on connected snowfields",
        layouts_ok);
    ASSERT("stage 1 has no slick ice and each later stage adds one 5x3 patch",
        patch_counts_ok);
    ASSERT("slick patches sit in middle clearings, ringed by snow, never blocking a route",
        patches_safe);
    ASSERT("snowing over and icing a stage draws no extra random numbers",
        rand_untouched);
    // The floors leave room for a different libc rand(); these seeds give 20, 37
    // and 47 shortcuts on stages 3, 4 and 5.
    ASSERT("stages 1-2 have no thin ice and stage N has up to N-2 shortcuts of 2-14 tiles",
        tunnel_counts_ok && stage_tunnels[3] >= 18 && stage_tunnels[4] >= 32 &&
        stage_tunnels[5] >= 40);
    ASSERT("thin-ice shortcuts are walled one-wide tunnels clear of the lake",
        tunnels_clean);
    ASSERT("every stage stays connected after all its shortcuts collapse",
        tunnels_optional);
    ASSERT("new frost tiles keep the saved tile ids before them",
        TILE_FROST_FLOOR == 135 && TILE_FROST_LAKE_HOLE == 140 && TILE_FROST_ICE == 141 &&
        TILE_FROST_THIN_ICE == 142 && TILE_FROST_BROKEN_ICE == 143);

    memset(&frost_game, 0, sizeof(frost_game));
    frost_game.player.player_class = CLASS_WARRIOR;
    game_init(&frost_game);
    srand(4242);
    game_enter_town2(&frost_game);
    int west_gate_open = 1;
    for (int y = 10; y <= 14; y++) {
        west_gate_open &= frost_game.map.tiles[y][0] == TILE_TOWN_EXIT;
    }
    frost_game.player.x = 1;
    frost_game.player.y = 12;
    walk_onto(&frost_game, 0, 12);
    ASSERT("Stillbury's west exit leads to Sunscar instead of Frostfell",
        west_gate_open && frost_game.location == LOCATION_DESERT);
    game_enter_town3(&frost_game);
    int north_gate = 1;
    for (int x = 18; x <= 22; x++) {
        north_gate &= frost_game.map.tiles[0][x] == TILE_TOWN_EXIT;
    }
    frost_game.player.x = 20;
    frost_game.player.y = 1;
    walk_onto(&frost_game, 20, 0);
    ASSERT("Rosemoor's north gate opens onto the first Frostfell stage",
        north_gate && frost_game.location == LOCATION_FROSTFELL &&
        frost_game.level == 1 &&
        frost_game.player.x == frost_game.map.stairs_up_x &&
        frost_game.player.y == frost_game.map.stairs_up_y);

    for (int stage = 1; stage < FROSTFELL_DEPTH; stage++) {
        frost_game.player.x = frost_game.map.stairs_down_x + 1;
        frost_game.player.y = frost_game.map.stairs_down_y;
        walk_onto(&frost_game, frost_game.map.stairs_down_x, frost_game.map.stairs_down_y);
    }
    int deepest = frost_game.level == FROSTFELL_DEPTH &&
        frost_game.max_frostfell_level_reached == FROSTFELL_DEPTH;
    frost_game.player.x = frost_game.map.stairs_up_x - 1;
    frost_game.player.y = frost_game.map.stairs_up_y;
    walk_onto(&frost_game, frost_game.map.stairs_up_x, frost_game.map.stairs_up_y);
    ASSERT("the western exits descend to stage 5 and the east entrance climbs back",
        deepest && frost_game.level == FROSTFELL_DEPTH - 1 &&
        frost_game.frostfell_cache[FROSTFELL_DEPTH - 1].valid &&
        frost_game.player.x == frost_game.map.stairs_down_x);

    // Clear the stage so no creature stands on the portal's landing tile.
    for (int i = 0; i < frost_game.enemy_count; i++) {
        frost_game.enemies[i].active = 0;
    }
    frost_game.player.x = frost_game.map.stairs_down_x + 1;
    frost_game.player.y = frost_game.map.stairs_down_y;
    int portal_x = frost_game.player.x;
    int portal_y = frost_game.player.y;
    game_open_town_portal(&frost_game);
    int in_town = frost_game.location == LOCATION_TOWN3 &&
        frost_game.player.x == 20 && frost_game.player.y == 1 &&
        frost_game.map.tiles[2][20] == TILE_PORTAL;
    game_use_town_portal(&frost_game);
    ASSERT("a return portal from Frostfell opens in Rosemoor and leads back",
        in_town && frost_game.location == LOCATION_FROSTFELL &&
        frost_game.level == FROSTFELL_DEPTH - 1 &&
        frost_game.player.x == portal_x && frost_game.player.y == portal_y);

    const int slot = 99017;
    frost_game.player.frozen_turns = 1;
    int loaded_ok = save_game(&frost_game, slot) && load_game(&frost_loaded, slot);
    frost_game.player.frozen_turns = 0;
    ASSERT("Frostfell progress, stage caches and a freeze survive save and load",
        loaded_ok && frost_loaded.location == LOCATION_FROSTFELL &&
        frost_loaded.level == FROSTFELL_DEPTH - 1 &&
        frost_loaded.max_frostfell_level_reached == FROSTFELL_DEPTH &&
        frost_loaded.player.frozen_turns == 1 &&
        frost_loaded.frostfell_cache[FROSTFELL_DEPTH - 1].valid &&
        memcmp(frost_loaded.frostfell_cache[FROSTFELL_DEPTH - 1].map.tiles,
            frost_game.frostfell_cache[FROSTFELL_DEPTH - 1].map.tiles,
            sizeof(frost_game.map.tiles)) == 0);
    remove("saves/savegame_99017.json");

    for (int stage = frost_game.level; stage < FROSTFELL_DEPTH; stage++) {
        frost_game.player.x = frost_game.map.stairs_down_x + 1;
        frost_game.player.y = frost_game.map.stairs_down_y;
        walk_onto(&frost_game, frost_game.map.stairs_down_x, frost_game.map.stairs_down_y);
    }
    int kraken_index = find_kraken(&frost_game);
    if (kraken_index >= 0) {
        frost_game.enemies[kraken_index].active = 0;
    }
    frost_game.player.x = frost_game.map.stairs_down_x + 1;
    frost_game.player.y = frost_game.map.stairs_down_y;
    walk_onto(&frost_game, frost_game.map.stairs_down_x, frost_game.map.stairs_down_y);
    ASSERT("the final western exit returns to Rosemoor's north gate",
        frost_game.location == LOCATION_TOWN3 &&
        frost_game.player.x == 20 && frost_game.player.y == 1);
}
