#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/game/item.h"

void test_classes(void) {
    printf("Class tests:\n");

    // --- Warrior ---
    GameState gw;
    gw.player.player_class = CLASS_WARRIOR;
    game_init(&gw);

    ASSERT("warrior: player_class preserved",  gw.player.player_class == CLASS_WARRIOR);
    ASSERT("warrior: max_hp is 150",           gw.player.max_hp == 150);
    ASSERT("warrior: hp starts full",          gw.player.hp == 150);
    ASSERT("warrior: max_mp is 20",            gw.player.max_mp == 20);
    ASSERT("warrior: mp starts full",          gw.player.mp == 20);
    ASSERT("warrior: attack is 14",            gw.player.attack == 14);
    ASSERT("warrior: defense is 6",            gw.player.defense == 6);
    ASSERT("warrior: starts with 1 item",      gw.inventory_count == 1);
    ASSERT("warrior: starts with rusty sword", gw.inventory[0].type == ITEM_WEAPON);

    // --- Mage ---
    GameState gm;
    gm.player.player_class = CLASS_MAGE;
    game_init(&gm);

    ASSERT("mage: player_class preserved",         gm.player.player_class == CLASS_MAGE);
    ASSERT("mage: max_hp is 70",                   gm.player.max_hp == 70);
    ASSERT("mage: hp starts full",                 gm.player.hp == 70);
    ASSERT("mage: max_mp is 100",                  gm.player.max_mp == 100);
    ASSERT("mage: mp starts full",                 gm.player.mp == 100);
    ASSERT("mage: attack is 4",                    gm.player.attack == 4);
    ASSERT("mage: defense is 2",                   gm.player.defense == 2);
    ASSERT("mage: starts with 2 items",            gm.inventory_count == 2);
    ASSERT("mage: first item is staff",            gm.inventory[0].type == ITEM_WEAPON);
    ASSERT("mage: second item is scroll",          gm.inventory[1].type == ITEM_SCROLL);

    // --- Rogue ---
    GameState gr;
    gr.player.player_class = CLASS_ROGUE;
    game_init(&gr);

    ASSERT("rogue: player_class preserved", gr.player.player_class == CLASS_ROGUE);
    ASSERT("rogue: max_hp is 100",          gr.player.max_hp == 100);
    ASSERT("rogue: hp starts full",         gr.player.hp == 100);
    ASSERT("rogue: max_mp is 40",           gr.player.max_mp == 40);
    ASSERT("rogue: mp starts full",         gr.player.mp == 40);
    ASSERT("rogue: attack is 10",           gr.player.attack == 10);
    ASSERT("rogue: defense is 4",           gr.player.defense == 4);
    ASSERT("rogue: starts with 1 item",     gr.inventory_count == 1);
    ASSERT("rogue: starts with bow",        gr.inventory[0].is_ranged == 1);
}
