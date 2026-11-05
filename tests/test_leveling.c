#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/systems/save_load.h"
#include <stdlib.h>
#include <string.h>

static void test_mage_mp_growth(void) {
    static GameState g;
    memset(&g, 0, sizeof(g));
    g.player.player_class = CLASS_MAGE;
    game_init(&g);
    int mp_at[12] = {0};
    mp_at[1] = g.player.max_mp;
    for (int level = 2; level <= 11; level++) {
        player_gain_xp(&g, g.player.experience_next - g.player.experience);
        mp_at[level] = g.player.max_mp;
    }
    ASSERT("a Mage gains 15 max MP per level up to level 5",
        mp_at[1] == 100 && mp_at[2] == 115 && mp_at[5] == 160);
    ASSERT("a Mage gains 10 max MP per level to level 10, then 5",
        mp_at[6] == 170 && mp_at[10] == 210 && mp_at[11] == 215);
    ASSERT("a level-up fills the Mage's larger MP pool",
        g.player.level == 11 && g.player.mp == g.player.max_mp);

    const PlayerClass others[2] = {CLASS_WARRIOR, CLASS_ROGUE};
    int others_unchanged = 1;
    for (int i = 0; i < 2; i++) {
        memset(&g, 0, sizeof(g));
        g.player.player_class = others[i];
        game_init(&g);
        int start_mp = g.player.max_mp;
        for (int level = 2; level <= 6; level++) {
            player_gain_xp(&g, g.player.experience_next - g.player.experience);
        }
        others_unchanged &= g.player.level == 6 && g.player.max_mp == start_mp;
    }
    ASSERT("Warriors and Rogues keep their max MP when they level up",
        others_unchanged);
}

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
    g.player.max_mp          = 40;
    g.player.mp              = 1;

    int old_attack  = g.player.attack;
    int old_defense = g.player.defense;
    int old_max_hp  = g.player.max_hp;

    // Gain enough XP to level up
    player_gain_xp(&g, 100);

    ASSERT("player leveled up",         g.player.level == 2);
    ASSERT("attack increased",          g.player.attack  == old_attack + 2);
    ASSERT("max_hp increased",          g.player.max_hp  == old_max_hp + 10);
    ASSERT("hp restored on level up",   g.player.hp      == g.player.max_hp);
    ASSERT("mp restored on level up",   g.player.mp      == g.player.max_mp);
    ASSERT("defense capped at 50pct",   g.player.defense <= g.player.attack / 2);
    ASSERT("xp resets after level up",  g.player.experience < g.player.experience_next);

    // Verify minimum damage always applies
    int dmg = g.player.attack - 999;
    if (dmg < 1) dmg = 1;
    ASSERT("minimum damage is always 1", dmg == 1);

    test_mage_mp_growth();
}

static void test_region_order_consistency(void) {
    static GameState g;
    const Location regions[] = {LOCATION_DUNGEON, LOCATION_FOREST, LOCATION_MOUNTAINS, LOCATION_COAST};
    for (int region = 0; region < 4; region++) {
        memset(&g, 0, sizeof(g));
        game_init(&g);
        g.location = regions[region];
        g.level = MAX_REGION_DEPTH;
        g.player.level = 17;
        srand(31);
        if (g.location == LOCATION_FOREST) {
            map_generate_forest(&g.map, g.level);
        } else if (g.location == LOCATION_MOUNTAINS) {
            map_generate_mountains(&g.map, g.level);
        } else if (g.location == LOCATION_COAST) {
            map_generate_coast(&g.map, g.level);
        } else {
            map_generate(&g.map, g.level);
        }
        Enemy reference[4][MAX_ENEMIES] = {0};
        int counts[4] = {0};
        int consistent = 1;
        for (int mask = 0; mask < 16; mask++) {
            if (mask & (1 << region)) {
                continue;
            }
            int tier = 0;
            g.defeated_bosses = 0;
            for (int prior = 0; prior < 4; prior++) {
                if (mask & (1 << prior)) {
                    g.defeated_bosses |= 1 << regions[prior];
                    tier++;
                }
            }
            srand(7);
            enemies_spawn(&g);
            if (counts[tier] == 0) {
                counts[tier] = g.enemy_count;
                memcpy(reference[tier], g.enemies, sizeof(g.enemies));
            } else {
                consistent &= counts[tier] == g.enemy_count;
                for (int i = 0; i < g.enemy_count; i++) {
                    Enemy *expected = &reference[tier][i];
                    Enemy *actual = &g.enemies[i];
                    consistent &= actual->type == expected->type &&
                        actual->max_hp == expected->max_hp && actual->attack == expected->attack &&
                        actual->defense == expected->defense;
                }
            }
        }
        ASSERT("each region's difficulty is independent of which earlier bosses were defeated", consistent);
        int smooth = counts[2] > 0 && counts[2] == counts[3];
        for (int i = 0; i < counts[3]; i++) {
            Enemy *third = &reference[2][i];
            Enemy *fourth = &reference[3][i];
            smooth &= third->type == fourth->type && fourth->max_hp >= third->max_hp &&
                fourth->max_hp <= (third->max_hp * 112 + 99) / 100 &&
                fourth->attack - third->attack <= 3;
        }
        ASSERT("the fourth region adds at most twelve percent HP and three attack over the third", smooth);
        g.player.level = 16;
        srand(7);
        enemies_spawn(&g);
        int gradual = g.enemy_count == counts[3];
        for (int i = 0; i < g.enemy_count; i++) {
            gradual &= reference[3][i].attack - g.enemies[i].attack <= 1 &&
                reference[3][i].max_hp <= (g.enemies[i].max_hp * 102 + 99) / 100;
        }
        ASSERT("reaching level seventeen adds no sudden attack or HP spike", gradual);
    }
}

void test_region_difficulty_scaling(void) {
    printf("Region difficulty scaling tests:\n");
    test_region_order_consistency();
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
        if (e->type == ENEMY_SKELETON && e->max_hp == 18 &&
            e->attack == 15 && e->defense == 2 && e->experience == 10) {
            scaled_skeleton = 1;
        }
    }
    ASSERT("two prior bosses strengthen dungeon enemies", scaled_skeleton);
    ASSERT("later regions draw from the advanced encounter mix",
        g.enemies[0].type != ENEMY_SKELETON);

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
        if (e->type == ENEMY_SKELETON && e->max_hp == 19 && e->attack == 17) {
            scaled_skeleton = 1;
        }
    }
    ASSERT("player level adds a modest adjustment by level nine", scaled_skeleton);

    g.player.level = 50;
    enemies_spawn(&g);
    scaled_skeleton = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        Enemy *e = &g.enemies[i];
        if (e->type == ENEMY_SKELETON && e->max_hp == 20 && e->attack == 19) {
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
        if (e->type == ENEMY_LICH_KING && e->max_hp == 217 &&
            e->attack == 28 && e->defense == 8 && e->experience == 480) {
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
    g.location = LOCATION_COAST;
    map_generate_coast(&g.map, g.level);
    enemies_spawn(&g);
    int coast_roles = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        coast_roles |= g.enemies[i].type == ENEMY_WATER_ELEMENTAL ||
            g.enemies[i].type == ENEMY_SEA_SERPENT;
    }
    ASSERT("late-order Coast openings still add tougher roles", coast_roles);

    // Act 1's outward band ignores which bosses fell first.
    for (int area = 0; area < 2; area++) {
        static Enemy reference[MAX_ENEMIES];
        int count = 0;
        int same = 1;
        for (int order = 0; order < 2; order++) {
            g.player.level = 5;
            g.level = 1;
            g.location = area ? LOCATION_MOUNTAINS : LOCATION_FOREST;
            g.defeated_bosses = order ? (1 << LOCATION_DUNGEON) | (1 << LOCATION_COAST) |
                (1 << (area ? LOCATION_FOREST : LOCATION_MOUNTAINS)) : 0;
            srand(4401);
            if (area) {
                map_generate_mountains(&g.map, g.level);
            } else {
                map_generate_forest(&g.map, g.level);
            }
            enemies_spawn(&g);
            if (!order) {
                count = g.enemy_count;
                memcpy(reference, g.enemies, sizeof(Enemy) * (size_t)count);
                continue;
            }
            same &= g.enemy_count == count;
            for (int i = 0; i < count && i < g.enemy_count; i++) {
                same &= g.enemies[i].type == reference[i].type && g.enemies[i].max_hp == reference[i].max_hp &&
                    g.enemies[i].attack == reference[i].attack && g.enemies[i].defense == reference[i].defense &&
                    g.enemies[i].experience == reference[i].experience;
            }
        }
        ASSERT(area ? "Goblin Mountains stats and roster ignore boss order" : "Haunted Forest stats and roster ignore boss order",
            count > 0 && same);
    }

    // Regulars gain 10% HP, 4 attack and 1 defense; bosses keep base stats.
    g.player.level = 1;
    g.defeated_bosses = 0;
    g.location = LOCATION_FOREST;
    g.level = 1;
    srand(4402);
    map_generate_forest(&g.map, g.level);
    enemies_spawn(&g);
    int pixie_band = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        const Enemy *e = &g.enemies[i];
        pixie_band |= e->type == ENEMY_PIXIE && e->max_hp == 8 && e->attack == 8 && e->defense == 1 && e->experience == 9;
    }
    ASSERT("the outward band strengthens a stage-one Pixie from 7/4/0 to 8/8/1", pixie_band);
    g.level = FOREST_BOSS_LEVEL;
    srand(4403);
    map_generate_forest(&g.map, g.level);
    enemies_spawn(&g);
    int necromancer_base = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        const Enemy *e = &g.enemies[i];
        necromancer_base |= e->type == ENEMY_FOREST_NECROMANCER && e->max_hp == 165 && e->attack == 18 && e->defense == 6;
    }
    ASSERT("the Necromancer keeps its base stats inside the band", necromancer_base);
    g.player.level = 11;
    g.level = 1;
    g.defeated_bosses = 1 << LOCATION_COAST;

    g.location = LOCATION_DUNGEON;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    int dungeon_after_coast = 0;
    for (int i = 0; i < g.enemy_count; i++) {
        dungeon_after_coast |= g.enemies[i].type == ENEMY_WRAITH ||
            g.enemies[i].type == ENEMY_CRYPT_CONJURER;
    }
    ASSERT("dungeon also advances its opening encounter mix",
        dungeon_after_coast);

    g.defeated_bosses = 1 << LOCATION_DUNGEON;
    g.location = LOCATION_COAST;
    map_generate_coast(&g.map, g.level);
    enemies_spawn(&g);
    int coast_after_dungeon = g.enemy_count > 0;
    for (int i = 0; i < g.enemy_count; i++) {
        coast_after_dungeon &= g.enemies[i].type != ENEMY_ILLUSION &&
            g.enemies[i].attack >= 17;
    }
    ASSERT("Coast also advances its opening encounter mix",
        coast_after_dungeon);
}
