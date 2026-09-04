#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/game/item.h"

void test_classes(void) {
    printf("Class tests:\n");

    // --- Warrior ---
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);

    ASSERT("warrior: player_class preserved",  g.player.player_class == CLASS_WARRIOR);
    ASSERT("warrior: max_hp is 150",           g.player.max_hp == 150);
    ASSERT("warrior: hp starts full",          g.player.hp == 150);
    ASSERT("warrior: max_mp is 20",            g.player.max_mp == 20);
    ASSERT("warrior: mp starts full",          g.player.mp == 20);
    ASSERT("warrior: attack is 14",            g.player.attack == 14);
    ASSERT("warrior: defense is 6",            g.player.defense == 6);
    ASSERT("warrior: starts with 2 items",      g.inventory_count == 2);
    ASSERT("warrior: starts with rusty sword", g.inventory[0].type == ITEM_WEAPON);
    ASSERT("warrior: starts with return scroll",
        g.inventory[1].spell_id == SPELL_RETURN_TO_TOWN);

    // --- Mage ---
    g.player.player_class = CLASS_MAGE;
    game_init(&g);

    ASSERT("mage: player_class preserved",         g.player.player_class == CLASS_MAGE);
    ASSERT("mage: max_hp is 70",                   g.player.max_hp == 70);
    ASSERT("mage: hp starts full",                 g.player.hp == 70);
    ASSERT("mage: max_mp is 100",                  g.player.max_mp == 100);
    ASSERT("mage: mp starts full",                 g.player.mp == 100);
    ASSERT("mage: attack is 4",                    g.player.attack == 4);
    ASSERT("mage: defense is 2",                   g.player.defense == 2);
    ASSERT("mage: starts with 3 items",            g.inventory_count == 3);
    ASSERT("mage: first item is staff",            g.inventory[0].type == ITEM_WEAPON);
    ASSERT("mage: second item is scroll",          g.inventory[1].type == ITEM_SCROLL);
    ASSERT("mage: starts with return scroll",
        g.inventory[2].spell_id == SPELL_RETURN_TO_TOWN);

    // --- Rogue ---
    g.player.player_class = CLASS_ROGUE;
    game_init(&g);

    ASSERT("rogue: player_class preserved", g.player.player_class == CLASS_ROGUE);
    ASSERT("rogue: max_hp is 100",          g.player.max_hp == 100);
    ASSERT("rogue: hp starts full",         g.player.hp == 100);
    ASSERT("rogue: max_mp is 40",           g.player.max_mp == 40);
    ASSERT("rogue: mp starts full",         g.player.mp == 40);
    ASSERT("rogue: attack is 10",           g.player.attack == 10);
    ASSERT("rogue: defense is 4",           g.player.defense == 4);
    ASSERT("rogue: starts with 2 items",     g.inventory_count == 2);
    ASSERT("rogue: starts with bow",        g.inventory[0].is_ranged == 1);
    ASSERT("rogue: starts with return scroll",
        g.inventory[1].spell_id == SPELL_RETURN_TO_TOWN);
}
