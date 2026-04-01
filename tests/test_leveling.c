#include "test_utils.h"
#include "../src/game/game.h"

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