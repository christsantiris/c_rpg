#include "approach_checks.h"
#include "test_utils.h"
#include "shortcut_checks.h"
#include "game/game.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

#define SHORTCUT_TEST_SLOT 99125
static GameState game;
static GameState loaded;
static const Location regions[3] = {LOCATION_FOREST, LOCATION_SWAMP, LOCATION_MOUNTAINS};
static const EnemyType bosses[3] = {ENEMY_FOREST_NECROMANCER, ENEMY_SWAMP_DEMON, ENEMY_MOUNTAIN_GOBLIN_KING};
static const TileType markers[3] = {TILE_FOREST_SHORTCUT, TILE_SWAMP_SHORTCUT, TILE_MOUNTAIN_SHORTCUT};
static const TileType floors[3] = {TILE_FOREST_FLOOR, TILE_SWAMP_FLOOR, TILE_MOUNTAIN_FORTRESS_FLOOR};

static int find_shortcut(const Map *map, TileType marker, int *x, int *y) {
    int count = 0;
    for (int row = 0; row < MAP_H; row++) {
        for (int column = 0; column < MAP_W; column++) {
            if (map->tiles[row][column] == marker) {
                *x = column;
                *y = row;
                count++;
            }
        }
    }
    return count;
}

static void shortcut_victory(int region, int attack) {
    game.player.player_class = CLASS_MAGE;
    game_init(&game);
    srand(631 + region);
    game.location = regions[region];
    game.level = 5;
    if (region == 0) {
        map_generate_forest(&game.map, FOREST_BOSS_LEVEL);
    } else if (region == 1) {
        map_generate_swamp(&game.map, SWAMP_BOSS_LEVEL);
    } else {
        map_generate_mountains(&game.map, MOUNTAIN_BOSS_LEVEL);
    }
    // Fight far from the original boss room, as when the Demon pursues the player.
    for (int y = 24; y <= 32; y++) {
        for (int x = 26; x <= 36; x++) {
            game.map.tiles[y][x] = floors[region];
        }
    }
    game.player.x = attack == 0 ? 29 : 28;
    game.player.y = 28;
    game.player.last_dx = 1;
    game.player.last_dy = 0;
    game.player.mp = 100;
    game.enemy_count = 2;
    game.enemies[0] = (Enemy){.type = bosses[region], .active = 1, .is_boss = 1, .x = 30, .y = 28, .hp = 1, .max_hp = 100};
    game.enemies[1] = (Enemy){.type = ENEMY_GOBLIN_SCOUT, .active = 1, .x = 35, .y = 30, .hp = 100, .max_hp = 100};
    game.inventory[0] = attack == 2 ? item_make_bow() : item_make_staff();
    game.player.arrows = MAX_ARROWS;
    game.inventory_count = 1;
    game.equipped_main_hand = 0;
    game.player.known_spells[0] = spell_make_magic_arrow();
    game.player.known_spell_count = 1;
    game.player.equipped_spell = 0;
    Action action = attack == 0 ? (Action){ACTION_MOVE, 30, 28} :
        (Action){attack == 1 ? ACTION_CAST_SPELL : ACTION_RANGED_ATTACK, 0, 0};
    action_resolve_player(&game, action);
    ASSERT("melee, spells and arrows record regional victory without shortcut prompts", !game.enemies[0].active && (game.defeated_bosses & (1 << regions[region])) && !game_shortcut_prompt_active(&game) && game.enemies[1].active);
    int x;
    int y;
    ASSERT("regional victories never create shortcut tiles", find_shortcut(&game.map, markers[region], &x, &y) == 0);
    ASSERT("victory and surviving enemies survive saving", save_game(&game, SHORTCUT_TEST_SLOT) && load_game(&loaded, SHORTCUT_TEST_SLOT) && !loaded.enemies[0].active && loaded.enemies[1].active && !game_shortcut_prompt_active(&loaded));
    game_request_shortcut(&loaded);
    game_enter_shortcut_boss(&loaded);
    ASSERT("retired shortcut APIs cannot teleport the player", loaded.location == regions[region] && loaded.level == 5 && !game_shortcut_prompt_active(&loaded));
    remove("saves/savegame_99125.json");
}

void test_regional_shortcuts(void) {
    printf("Retired regional shortcut tests:\n");
    ASSERT("regional shortcut save slot is unused", !save_exists(SHORTCUT_TEST_SLOT));
    for (int region = 0; region < 3; region++) {
        for (int attack = 0; attack < 3; attack++) {
            shortcut_victory(region, attack);
        }
    }
}
