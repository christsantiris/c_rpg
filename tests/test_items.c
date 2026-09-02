#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/game/actions.h"
#include "../src/game/item.h"

void test_items(void) {
    printf("Item tests:\n");

    // --- Factory functions ---
    Item hp_potion = item_make_health_potion();
    ASSERT("health potion type correct",    hp_potion.type    == ITEM_POTION_HEALTH);
    ASSERT("health potion heal_hp set",     hp_potion.heal_hp == 30);
    ASSERT("health potion has value",       hp_potion.value   > 0);

    Item mp_potion = item_make_mana_potion();
    ASSERT("mana potion type correct",      mp_potion.type    == ITEM_POTION_MANA);
    ASSERT("mana potion heal_mp set",       mp_potion.heal_mp == 20);

    Item sword = item_make_long_sword();
    ASSERT("weapon type correct",           sword.type         == ITEM_WEAPON);
    ASSERT("weapon attack bonus set",       sword.attack_bonus == 6);

    Item bow = item_make_bow();
    ASSERT("bow attack bonus is balanced", bow.attack_bonus == 3);
    ASSERT("bow range is six tiles",        bow.range == 6);

    Item armor = item_make_leather_armor();
    ASSERT("armor type correct",            armor.type          == ITEM_ARMOR);
    ASSERT("armor defense bonus set",       armor.defense_bonus == 2);

    // --- Use health potion ---
    GameState g;
    game_init(&g);
    g.player.hp     = 50;
    g.player.max_hp = 100;
    g.inventory[0]  = hp_potion;
    g.inventory_count = 1;
    Action use = {ACTION_USE_ITEM, 0, 0};
    action_resolve_player(&g, use);
    ASSERT("hp restored after potion",      g.player.hp == 80);
    ASSERT("potion removed from inventory", g.inventory_count == 0);

    // --- Equip weapon ---
    GameState g2;
    game_init(&g2);
    g2.inventory[0]   = sword;
    g2.inventory_count = 1;
    int base_attack    = g2.player.attack;
    Action equip_wpn   = {ACTION_EQUIP_ITEM, 0, 0};
    action_resolve_player(&g2, equip_wpn);
    ASSERT("weapon equipped",               g2.equipped_weapon == 0);
    ASSERT("attack increased after equip",  g2.player.attack == base_attack + 6);

    // --- Equip armor ---
    GameState g3;
    game_init(&g3);
    g3.inventory[0]   = armor;
    g3.inventory_count = 1;
    int base_defense   = g3.player.defense;
    Action equip_arm   = {ACTION_EQUIP_ITEM, 0, 0};
    action_resolve_player(&g3, equip_arm);
    ASSERT("armor equipped",                g3.equipped_armor == 0);
    ASSERT("defense increased after equip", g3.player.defense == base_defense + 2);

    // --- Inventory full ---
    GameState g4;
    game_init(&g4);
    for (int i = 0; i < MAX_INVENTORY; i++)
        g4.inventory[i] = item_make_health_potion();
    g4.inventory_count = MAX_INVENTORY;
    ASSERT("inventory is full",             g4.inventory_count == MAX_INVENTORY);

    // --- Bow combat rules ---
    GameState rogue = {0};
    rogue.player.player_class = CLASS_ROGUE;
    game_init(&rogue);
    Action equip_bow = {ACTION_EQUIP_ITEM, 0, 0};
    action_resolve_player(&rogue, equip_bow);
    rogue.player.x = 10;
    rogue.player.y = 10;
    rogue.player.last_dx = 1;
    rogue.player.last_dy = 0;
    rogue.enemy_count = 1;
    rogue.enemies[0] = (Enemy){0};
    rogue.enemies[0].active = 1;
    rogue.enemies[0].x = 11;
    rogue.enemies[0].y = 10;
    rogue.enemies[0].hp = 100;
    rogue.enemies[0].max_hp = 100;
    Action shoot = {ACTION_RANGED_ATTACK, 0, 0};
    action_resolve_player(&rogue, shoot);
    ASSERT("bow cannot fire beside an enemy", rogue.enemies[0].hp == 100);

    int equipped_attack = rogue.player.attack;
    Action bump = {ACTION_MOVE, 11, 10};
    action_resolve_player(&rogue, bump);
    ASSERT("bow bonus does not apply in melee",
        rogue.enemies[0].hp == 100 -
            (equipped_attack - bow.attack_bonus));

    rogue.enemies[0].x = 12;
    rogue.enemies[0].hp = 100;
    action_resolve_player(&rogue, shoot);
    int ranged_damage = 100 - rogue.enemies[0].hp;
    ASSERT("bow shot uses normal or critical damage",
        ranged_damage == equipped_attack ||
        ranged_damage == equipped_attack * 3 / 2);
}
