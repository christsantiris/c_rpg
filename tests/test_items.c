#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/game/actions.h"
#include "../src/game/item.h"
#include <string.h>

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
    ASSERT("long sword has sword metadata",
        sword.weapon_family == WEAPON_FAMILY_SWORD &&
        sword.weapon_hands == WEAPON_HANDS_ONE);
    ASSERT("long sword is an uncommon Warrior weapon",
        sword.rarity == ITEM_RARITY_UNCOMMON &&
        sword.class_mask == ITEM_CLASS_WARRIOR);
    ASSERT("long sword has its own visual identity",
        sword.visual_id == ITEM_VISUAL_LONG_SWORD);
    ASSERT("long sword class access is enforced by metadata",
        item_class_allowed(&sword, CLASS_WARRIOR) &&
        !item_class_allowed(&sword, CLASS_MAGE) &&
        !item_class_allowed(&sword, CLASS_ROGUE));

    Item bow = item_make_bow();
    ASSERT("bow attack bonus is balanced", bow.attack_bonus == 3);
    ASSERT("bow range is six tiles",        bow.range == 6);
    ASSERT("bow is a two-handed Rogue weapon",
        bow.weapon_family == WEAPON_FAMILY_BOW &&
        bow.weapon_hands == WEAPON_HANDS_TWO &&
        bow.class_mask == ITEM_CLASS_ROGUE);

    Item axe = item_make_battle_axe();
    ASSERT("battle axe is a two-handed Warrior weapon",
        axe.weapon_family == WEAPON_FAMILY_AXE &&
        axe.weapon_hands == WEAPON_HANDS_TWO &&
        axe.class_mask == ITEM_CLASS_WARRIOR);

    Item staff = item_make_staff();
    ASSERT("staff is a two-handed Mage weapon",
        staff.weapon_family == WEAPON_FAMILY_STAFF &&
        staff.weapon_hands == WEAPON_HANDS_TWO &&
        staff.class_mask == ITEM_CLASS_MAGE);

    Item legacy_bow = {0};
    legacy_bow.type = ITEM_WEAPON;
    strncpy(legacy_bow.name, "Bow", sizeof(legacy_bow.name) - 1);
    item_apply_legacy_metadata(&legacy_bow);
    ASSERT("legacy bow receives current weapon metadata",
        legacy_bow.weapon_family == WEAPON_FAMILY_BOW &&
        legacy_bow.weapon_hands == WEAPON_HANDS_TWO &&
        legacy_bow.visual_id == ITEM_VISUAL_BOW);

    Item rusty = item_make_rusty_sword();
    Item short_sword = item_make_short_sword();
    ASSERT("stronger existing weapons cost more",
        rusty.value < short_sword.value &&
        short_sword.value < sword.value && sword.value < axe.value);

    Item armor = item_make_leather_armor();
    ASSERT("armor type correct",            armor.type          == ITEM_ARMOR);
    ASSERT("armor defense bonus set",       armor.defense_bonus == 2);

    // --- Use health potion ---
    GameState g = {0};
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
    game_init(&g);
    g.inventory[0] = sword;
    g.inventory_count = 1;
    int base_attack = g.player.attack;
    Action equip_wpn   = {ACTION_EQUIP_ITEM, 0, 0};
    action_resolve_player(&g, equip_wpn);
    ASSERT("weapon equipped in main hand", g.equipped_main_hand == 0);
    ASSERT("off hand remains empty", g.equipped_off_hand == -1);
    ASSERT("attack increased after equip",  g.player.attack == base_attack + 6);

    // --- Class-restricted weapon equip ---
    GameState mage = {0};
    mage.player.player_class = CLASS_MAGE;
    game_init(&mage);
    int mage_base_attack = mage.player.attack;
    mage.inventory[mage.inventory_count++] = sword;
    Action equip_restricted = {ACTION_EQUIP_ITEM,
        mage.inventory_count - 1, 0};
    action_resolve_player(&mage, equip_restricted);
    ASSERT("Mage cannot equip a Warrior long sword",
        mage.equipped_main_hand == -1);
    ASSERT("failed class equip does not change attack",
        mage.player.attack == mage_base_attack);

    // --- Switching one-handed weapons ---
    game_init(&g);
    base_attack = g.player.attack;
    g.inventory[g.inventory_count++] = item_make_short_sword();
    Action equip_starting_rusty = {ACTION_EQUIP_ITEM, 0, 0};
    action_resolve_player(&g, equip_starting_rusty);
    Action equip_short = {ACTION_EQUIP_ITEM, g.inventory_count - 1, 0};
    action_resolve_player(&g, equip_short);
    ASSERT("short sword replaces rusty sword in the weapon slot",
        g.equipped_main_hand == g.inventory_count - 1);
    ASSERT("switching weapons applies only the short sword bonus",
        g.player.attack == base_attack + 3);
    Action equip_rusty = {ACTION_EQUIP_ITEM, 0, 0};
    action_resolve_player(&g, equip_rusty);
    ASSERT("rusty sword replaces short sword in the weapon slot",
        g.equipped_main_hand == 0);
    ASSERT("switching back applies only the rusty sword bonus",
        g.player.attack == base_attack + 1);

    // --- Equip armor ---
    game_init(&g);
    g.inventory[0] = armor;
    g.inventory_count = 1;
    int base_defense = g.player.defense;
    Action equip_arm   = {ACTION_EQUIP_ITEM, 0, 0};
    action_resolve_player(&g, equip_arm);
    ASSERT("armor equipped",                g.equipped_armor == 0);
    ASSERT("defense increased after equip", g.player.defense == base_defense + 2);

    // --- Equipped indices survive consumption ---
    game_init(&g);
    g.inventory[0] = hp_potion;
    g.inventory[1] = item_make_rusty_sword();
    g.inventory[2] = armor;
    g.inventory_count = 3;
    g.equipped_main_hand = -1;
    g.equipped_off_hand = -1;
    g.equipped_armor = -1;
    Action equip_shift_weapon = {ACTION_EQUIP_ITEM, 1, 0};
    Action equip_shift_armor = {ACTION_EQUIP_ITEM, 2, 0};
    action_resolve_player(&g, equip_shift_weapon);
    action_resolve_player(&g, equip_shift_armor);
    g.player.hp--;
    Action consume_first = {ACTION_USE_ITEM, 0, 0};
    action_resolve_player(&g, consume_first);
    ASSERT("consuming an earlier item shifts equipped weapon index",
        g.equipped_main_hand == 0 &&
        g.inventory[g.equipped_main_hand].type == ITEM_WEAPON);
    ASSERT("consumption preserves empty off hand",
        g.equipped_off_hand == -1);
    ASSERT("consuming an earlier item shifts equipped armor index",
        g.equipped_armor == 1 &&
        g.inventory[g.equipped_armor].type == ITEM_ARMOR);

    // --- Pickup cannot occupy a stale equipment index ---
    game_init(&g);
    Action equip_initial_weapon = {ACTION_EQUIP_ITEM, 0, 0};
    action_resolve_player(&g, equip_initial_weapon);
    g.equipped_main_hand = g.inventory_count;
    g.floor_item_count = 1;
    g.floor_items[0] = (FloorItem){0};
    g.floor_items[0].active = 1;
    g.floor_items[0].x = g.player.x;
    g.floor_items[0].y = g.player.y;
    g.floor_items[0].underlying_tile = TILE_TOWN_FLOOR;
    g.floor_items[0].item = item_make_short_sword();
    g.map.tiles[g.player.y][g.player.x] = TILE_ITEM;
    Action pick_up_sword = {ACTION_PICK_UP, 0, 0};
    action_resolve_player(&g, pick_up_sword);
    ASSERT("picking up a sword does not auto-equip it",
        g.equipped_main_hand == 0 &&
        g.inventory[g.inventory_count - 1].type == ITEM_WEAPON);

    // --- Inventory full ---
    game_init(&g);
    for (int i = 0; i < MAX_INVENTORY; i++)
        g.inventory[i] = item_make_health_potion();
    g.inventory_count = MAX_INVENTORY;
    ASSERT("inventory is full",             g.inventory_count == MAX_INVENTORY);

    // --- Bow combat rules ---
    g.player.player_class = CLASS_ROGUE;
    game_init(&g);
    Action equip_bow = {ACTION_EQUIP_ITEM, 0, 0};
    action_resolve_player(&g, equip_bow);
    g.player.x = 20;
    g.player.y = 12;
    g.player.last_dx = 1;
    g.player.last_dy = 0;
    g.enemy_count = 1;
    g.enemies[0] = (Enemy){0};
    g.enemies[0].active = 1;
    g.enemies[0].x = 21;
    g.enemies[0].y = 12;
    g.enemies[0].hp = 100;
    g.enemies[0].max_hp = 100;
    Action shoot = {ACTION_RANGED_ATTACK, 0, 0};
    action_resolve_player(&g, shoot);
    ASSERT("bow cannot fire beside an enemy", g.enemies[0].hp == 100);

    int equipped_attack = g.player.attack;
    Action bump = {ACTION_MOVE, 21, 12};
    action_resolve_player(&g, bump);
    ASSERT("bow bonus does not apply in melee",
        g.enemies[0].hp == 100 -
            (equipped_attack - bow.attack_bonus));

    g.enemies[0].x = 22;
    g.enemies[0].hp = 100;
    action_resolve_player(&g, shoot);
    int ranged_damage = 100 - g.enemies[0].hp;
    ASSERT("bow shot uses normal or critical damage",
        ranged_damage == equipped_attack ||
        ranged_damage == equipped_attack * 3 / 2);
    ASSERT("arrow trail stops on its target", g.trail_count == 2);
    ASSERT("arrow target is marked as impact",
        g.trail[g.trail_count - 1].is_impact == 1);
}
