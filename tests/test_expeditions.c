#include "expedition_fixture.h"
#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/game/castle.h"
#include "../src/systems/save_load.h"
#include <string.h>

static GameState game;
static GameState loaded;

typedef struct {
    void (*enter)(GameState *);
    int depth;
    Location location;
} Expedition;

static LevelCache *expedition_cache(GameState *g) {
    switch (g->location) {
        case LOCATION_FOREST: return g->forest_cache;
        case LOCATION_MOUNTAINS: return g->mountain_cache;
        case LOCATION_COAST: return g->coast_cache;
        case LOCATION_SWAMP: return g->swamp_cache;
        case LOCATION_FROSTFELL: return g->frostfell_cache;
        case LOCATION_DESERT: return g->desert_cache;
        case LOCATION_ASHEN: return g->ashen_cache;
        case LOCATION_GLASSDEEP: return g->glassdeep_cache;
        case LOCATION_CATACOMBS: return g->catacombs_cache;
        case LOCATION_DRAGONSPINE: return g->dragonspine_cache;
        case LOCATION_TEMPLE: return g->temple_cache;
        case LOCATION_LABYRINTH: return g->labyrinth_cache;
        default: return g->level_cache;
    }
}

static void next_floor(GameState *g) {
    if (g->location == LOCATION_LABYRINTH) {
        game_change_labyrinth_floor(g, 1, 0);
    } else {
        game_descend(g);
    }
}

static void clear_regular_enemies(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (!g->enemies[i].is_boss) {
            g->enemies[i].active = 0;
            g->enemies[i].hp = 0;
            g->enemies[i].frozen_turns = 3;
        } else {
            g->enemies[i].hp = 37;
        }
    }
    g->level_cleared = 1;
}

static int active_regular_enemies(const GameState *g) {
    int count = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && !g->enemies[i].is_boss) {
            count++;
        }
    }
    return count;
}

static void test_region_expedition(Expedition area) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    area.enter(&game);
    for (int floor = 1; floor <= area.depth; floor++) {
        clear_regular_enemies(&game);
        if (floor < area.depth) {
            next_floor(&game);
        }
    }
    if (area.location == LOCATION_LABYRINTH) {
        game_leave_labyrinth(&game);
    } else {
        game_open_town_portal(&game);
    }
    area.enter(&game);
    LevelCache *cache = expedition_cache(&game);
    int replayable = 1;
    int boss_preserved = 1;
    int healthy = 1;
    for (int floor = 1; floor <= area.depth; floor++) {
        replayable &= active_regular_enemies(&game) > 0 && !game.level_cleared;
        for (int i = 0; i < game.enemy_count; i++) {
            Enemy *enemy = &game.enemies[i];
            if (enemy->is_boss) {
                boss_preserved &= enemy->active && enemy->hp == 37;
            } else if (enemy->active) {
                healthy &= enemy->hp == enemy->max_hp && !enemy->frozen_turns;
            }
        }
        if (floor < area.depth) {
            next_floor(&game);
        }
    }
    char message[128];
    snprintf(message, sizeof(message), "region %d repopulates every floor on a new expedition", area.location);
    ASSERT(message, replayable);
    ASSERT("replaying restores regular enemies to full health without old debuffs", healthy);
    ASSERT("replaying preserves wounded bosses on every floor", boss_preserved);

    clear_regular_enemies(&game);
    if (area.location == LOCATION_LABYRINTH) {
        game_leave_labyrinth(&game);
    } else {
        game_return_to_town(&game);
    }
    game.defeated_bosses |= 1 << area.location;
    for (int floor = 0; floor < area.depth; floor++) {
        for (int i = 0; i < cache[floor].enemy_count; i++) {
            if (cache[floor].enemies[i].is_boss) {
                cache[floor].enemies[i].active = 0;
            }
        }
    }
    area.enter(&game);
    int bosses_stay_dead = 1;
    for (int floor = 1; floor <= area.depth; floor++) {
        for (int i = 0; i < game.enemy_count; i++) {
            bosses_stay_dead &= !game.enemies[i].is_boss || !game.enemies[i].active;
        }
        if (floor < area.depth) {
            next_floor(&game);
        }
    }
    ASSERT("new expeditions never resurrect defeated bosses", bosses_stay_dead);
}

static void test_dungeon_portal_and_save(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game_enter_dungeon(&game);
    for (int floor = 1; floor < DUNGEON_DEPTH; floor++) {
        clear_regular_enemies(&game);
        game_descend(&game);
    }
    clear_regular_enemies(&game);
    game_open_town_portal(&game);
    test_restore_cached_floor(&game);
    ASSERT("direct archived floor reload continues the cleared expedition", !active_regular_enemies(&game));
    game_open_town_portal(&game);
    game.elowen_quest_state = 2;
    game.elowen_seals_restored = 7;
    ASSERT("town save with a cleared dungeon and active portal writes", save_game(&game, 99161));
    ASSERT("existing cached expedition loads", load_game(&loaded, 99161));
    game_enter_dungeon(&loaded);
    int replayable = 1;
    for (int floor = 1; floor <= DUNGEON_DEPTH; floor++) {
        replayable &= active_regular_enemies(&loaded) > 0;
        if (floor < DUNGEON_DEPTH) {
            game_descend(&loaded);
        }
    }
    ASSERT("walking back in after loading repopulates all five dungeon floors", replayable);
    ASSERT("replaying preserves completed seals and quest completion", loaded.elowen_quest_state == 2 && loaded.elowen_seals_restored == 7);
    game_return_to_town(&loaded);
    test_restore_cached_floor(&loaded);
    ASSERT("the original boss-floor portal remains usable after walking in", loaded.location == LOCATION_DUNGEON && loaded.level == DUNGEON_DEPTH);
    ASSERT("replayed enemy state saves and loads", save_game(&loaded, 99161) && load_game(&game, 99161) && active_regular_enemies(&game) > 0);
    remove("saves/savegame_99161.json");

    clear_regular_enemies(&game);
    game_ascend(&game);
    clear_regular_enemies(&game);
    game_descend(&game);
    ASSERT("backtracking within one expedition never repopulates a cleared floor", !active_regular_enemies(&game));
}

static void test_quest_guards_and_spawn_positions(void) {
    memset(&game, 0, sizeof(game));
    game_init(&game);
    game_enter_glassdeep(&game);
    game_descend(&game);
    game.glassdeep_quest_state = 1;
    game_refresh_quest_encounters(&game);
    int guard = -1;
    int regular = -1;
    for (int i = 0; i < game.enemy_count; i++) {
        if (strcmp(game.enemies[i].name, "Quarry Guardian") == 0) {
            guard = i;
        } else if (!game.enemies[i].is_boss) {
            regular = i;
        }
    }
    ASSERT("replay fixture includes both quest guards and regular enemies", guard >= 0 && regular >= 0);
    if (guard < 0 || regular < 0) {
        return;
    }
    clear_regular_enemies(&game);
    game.enemies[regular].x = game.enemies[0].x;
    game.enemies[regular].y = game.enemies[0].y;
    game_open_town_portal(&game);
    game_enter_glassdeep(&game);
    game_descend(&game);
    ASSERT("new expeditions leave defeated named quest guards dead", !game.enemies[guard].active);
    int valid_positions = 1;
    for (int i = 0; i < game.enemy_count; i++) {
        Enemy *enemy = &game.enemies[i];
        if (!enemy->active) {
            continue;
        }
        valid_positions &= map_is_walkable(&game.map, enemy->x, enemy->y);
        for (int j = 0; j < i; j++) {
            valid_positions &= !game.enemies[j].active || game.enemies[j].x != enemy->x || game.enemies[j].y != enemy->y;
        }
    }
    ASSERT("respawning resolves overlapping corpses into distinct walkable positions", valid_positions);
}

static void test_road_expeditions(void) {
    const Location roads[2] = {LOCATION_CROWNROAD, LOCATION_KING_ROAD_WEST};
    for (int i = 0; i < 2; i++) {
        memset(&game, 0, sizeof(game));
        game_init(&game);
        game_enter_king_road(&game, roads[i], 0);
        clear_regular_enemies(&game);
        game_leave_crownroad(&game, LOCATION_CASTLE);
        game_enter_king_road(&game, roads[i], 1);
        ASSERT("both Crown Roads repopulate on a new crossing", active_regular_enemies(&game) == CROWNROAD_ENEMIES && !game.level_cleared);
    }
}

void test_expeditions(void) {
    printf("New expedition replay tests:\n");
    const Expedition areas[] = {
        {game_enter_dungeon, DUNGEON_DEPTH, LOCATION_DUNGEON},
        {game_enter_forest, FOREST_DEPTH, LOCATION_FOREST},
        {game_enter_mountains, MOUNTAIN_DEPTH, LOCATION_MOUNTAINS},
        {game_enter_coast, COAST_DEPTH, LOCATION_COAST},
        {game_enter_swamp, SWAMP_DEPTH, LOCATION_SWAMP},
        {game_enter_frostfell, FROSTFELL_DEPTH, LOCATION_FROSTFELL},
        {game_enter_desert, DESERT_DEPTH, LOCATION_DESERT},
        {game_enter_ashen, ASHEN_DEPTH, LOCATION_ASHEN},
        {game_enter_glassdeep, GLASSDEEP_DEPTH, LOCATION_GLASSDEEP},
        {game_enter_catacombs, CATACOMBS_DEPTH, LOCATION_CATACOMBS},
        {game_enter_dragonspine, DRAGONSPINE_DEPTH, LOCATION_DRAGONSPINE},
        {game_enter_temple, TEMPLE_DEPTH, LOCATION_TEMPLE},
        {game_enter_labyrinth, LABYRINTH_DEPTH, LOCATION_LABYRINTH}
    };
    for (int i = 0; i < (int)(sizeof(areas) / sizeof(areas[0])); i++) {
        test_region_expedition(areas[i]);
    }
    test_dungeon_portal_and_save();
    test_quest_guards_and_spawn_positions();
    test_road_expeditions();
}
