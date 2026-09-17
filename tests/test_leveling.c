#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/systems/save_load.h"
#include <stdlib.h>
#include <string.h>

void test_leveling(void) {
    printf("Leveling tests:\n");

    GameState g;
    game_init(&g);
    g.player.experience      = 0;
    g.player.experience_next = 100;
    g.player.level           = 1;
    g.player.attack          = 10;
    g.player.defense         = 5;
    g.player.max_hp          = 100;
    g.player.hp              = 100;

    int old_attack  = g.player.attack;
    int old_defense = g.player.defense;
    int old_max_hp  = g.player.max_hp;

    // Gain enough XP to level up
    player_gain_xp(&g, 100);

    ASSERT("player leveled up",         g.player.level == 2);
    ASSERT("attack increased",          g.player.attack  == old_attack + 2);
    ASSERT("max_hp increased",          g.player.max_hp  == old_max_hp + 10);
    ASSERT("hp restored on level up",   g.player.hp      == g.player.max_hp);
    ASSERT("defense capped at 50pct",   g.player.defense <= g.player.attack / 2);
    ASSERT("xp resets after level up",  g.player.experience < g.player.experience_next);

    // Verify minimum damage always applies
    int dmg = g.player.attack - 999;
    if (dmg < 1) dmg = 1;
    ASSERT("minimum damage is always 1", dmg == 1);
}

void test_region_difficulty_scaling(void) {
    printf("Region difficulty scaling tests:\n");
    static GameState g;
    static GameState loaded;
    memset(&g, 0, sizeof(g));
    game_init(&g);
    g.location = LOCATION_DUNGEON;
    g.level = 1;
    srand(7);
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    ASSERT("first region keeps base skeleton stats",
        g.enemy_count > 0 && g.enemies[0].type == ENEMY_SKELETON &&
        g.enemies[0].max_hp == 10 && g.enemies[0].attack == 3 &&
        g.enemies[0].experience == 8);

    g.defeated_bosses = (1 << LOCATION_FOREST) | (1 << LOCATION_MOUNTAINS);
    enemies_spawn(&g);
    int scaled_skeleton = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        Enemy *e = &g.enemies[i];
        if (e->type == ENEMY_SKELETON && e->max_hp == 14 &&
            e->attack == 7 && e->defense == 1 && e->experience == 10) {
            scaled_skeleton = 1;
        }
    }
    ASSERT("two prior bosses strengthen dungeon enemies", scaled_skeleton);
    ASSERT("late-order dungeon starts with a tougher regional role",
        g.enemies[0].type == ENEMY_ZOMBIE);

    g.defeated_bosses = 1 << LOCATION_DUNGEON;
    enemies_spawn(&g);
    ASSERT("a region does not count its own completed boss",
        g.enemies[0].type == ENEMY_SKELETON &&
        g.enemies[0].max_hp == 10 && g.enemies[0].attack == 3);
    g.defeated_bosses = (1 << LOCATION_FOREST) | (1 << LOCATION_MOUNTAINS);

    g.player.level = 9;
    enemies_spawn(&g);
    scaled_skeleton = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        Enemy *e = &g.enemies[i];
        if (e->type == ENEMY_SKELETON && e->max_hp == 15 && e->attack == 8) {
            scaled_skeleton = 1;
        }
    }
    ASSERT("player level adds one small step after level eight", scaled_skeleton);

    g.player.level = 50;
    enemies_spawn(&g);
    scaled_skeleton = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        Enemy *e = &g.enemies[i];
        if (e->type == ENEMY_SKELETON && e->max_hp == 15 && e->attack == 9) {
            scaled_skeleton = 1;
        }
    }
    ASSERT("player level adjustment is capped", scaled_skeleton);

    g.player.level = 1;
    g.level = DUNGEON_DEPTH;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    int scaled_boss = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        Enemy *e = &g.enemies[i];
        if (e->type == ENEMY_LICH_KING && e->max_hp == 210 &&
            e->attack == 24 && e->defense == 7 && e->experience == 480) {
            scaled_boss = 1;
        }
    }
    ASSERT("bosses get their own order scaling", scaled_boss);

    const int slot = 99011;
    if (save_exists(slot)) {
        ASSERT("difficulty test save slot must be unused", 0);
        return;
    }
    int loaded_ok = save_game(&g, slot) && load_game(&loaded, slot);
    ASSERT("scaled boss stats survive save and load",
        loaded_ok && loaded.enemy_count == g.enemy_count &&
        loaded.enemies[0].max_hp == g.enemies[0].max_hp &&
        loaded.enemies[0].attack == g.enemies[0].attack);
    remove("saves/savegame_99011.json");

    g.defeated_bosses = 0;
    game_enter_dungeon(&g);
    int original_hp = g.enemies[0].max_hp;
    g.defeated_bosses = (1 << LOCATION_FOREST) | (1 << LOCATION_MOUNTAINS);
    game_descend(&g);
    game_ascend(&g);
    ASSERT("cached floor retains its original enemy stats",
        g.enemies[0].max_hp == original_hp && original_hp == 10);

    g.defeated_bosses = (1 << LOCATION_DUNGEON) | (1 << LOCATION_FOREST) |
        (1 << LOCATION_MOUNTAINS) | (1 << LOCATION_COAST);
    g.level = 1;
    g.location = LOCATION_FOREST;
    map_generate_forest(&g.map, g.level);
    enemies_spawn(&g);
    int forest_roles = g.enemies[0].type == ENEMY_GIANT_SPIDER &&
        g.enemies[1].type == ENEMY_DARK_ELF;
    g.location = LOCATION_MOUNTAINS;
    map_generate_mountains(&g.map, g.level);
    enemies_spawn(&g);
    int mountain_roles = g.enemies[0].type == ENEMY_GOBLIN_ARCHER &&
        g.enemies[1].type == ENEMY_GOBLIN_BOMBER;
    g.location = LOCATION_COAST;
    map_generate_coast(&g.map, g.level);
    enemies_spawn(&g);
    int coast_roles = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        coast_roles |= g.enemies[i].type == ENEMY_SIREN;
    }
    ASSERT("late-order opening stages add tougher roles in every region",
        forest_roles && mountain_roles && coast_roles);
}
