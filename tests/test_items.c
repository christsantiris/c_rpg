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

    Item magic_long_sword = item_make_magic_long_sword();
    ASSERT("magic long sword is a rare one-handed Warrior sword",
        magic_long_sword.weapon_family == WEAPON_FAMILY_SWORD &&
        magic_long_sword.weapon_hands == WEAPON_HANDS_ONE &&
        magic_long_sword.rarity == ITEM_RARITY_RARE &&
        magic_long_sword.class_mask == ITEM_CLASS_WARRIOR);
    ASSERT("magic long sword combines attack with critical chance",
        magic_long_sword.attack_bonus == 10 &&
        magic_long_sword.critical_chance_bonus == 20);
    ASSERT("magic long sword has a premium price and distinct visual",
        magic_long_sword.value == 450 &&
        magic_long_sword.visual_id == ITEM_VISUAL_MAGIC_LONG_SWORD);

    Item bow = item_make_bow();
    ASSERT("bow attack bonus is balanced", bow.attack_bonus == 3);
    ASSERT("bow range is six tiles",        bow.range == 6);
    ASSERT("bow is a two-handed Rogue weapon",
        bow.weapon_family == WEAPON_FAMILY_BOW &&
        bow.weapon_hands == WEAPON_HANDS_TWO &&
        bow.class_mask == ITEM_CLASS_ROGUE);

    Item longbow = item_make_longbow();
    ASSERT("longbow is an uncommon two-handed Rogue bow",
        longbow.weapon_family == WEAPON_FAMILY_BOW &&
        longbow.weapon_hands == WEAPON_HANDS_TWO &&
        longbow.rarity == ITEM_RARITY_UNCOMMON &&
        longbow.class_mask == ITEM_CLASS_ROGUE);
    ASSERT("longbow improves the basic bow's power and reach",
        longbow.attack_bonus == 7 && longbow.range == 9 &&
        longbow.attack_bonus > bow.attack_bonus &&
        longbow.range > bow.range);
    ASSERT("longbow has an advanced price and distinct visual",
        longbow.value == 200 && longbow.value > bow.value &&
        longbow.visual_id == ITEM_VISUAL_LONGBOW);

    Item magic_longbow = item_make_magic_longbow();
    ASSERT("magic longbow is the rare two-handed Rogue bow",
        magic_longbow.weapon_family == WEAPON_FAMILY_BOW &&
        magic_longbow.weapon_hands == WEAPON_HANDS_TWO &&
        magic_longbow.rarity == ITEM_RARITY_RARE &&
        magic_longbow.class_mask == ITEM_CLASS_ROGUE);
    ASSERT("magic longbow is the Rogue's strongest ranged weapon",
        magic_longbow.attack_bonus == 13 && magic_longbow.range == 12 &&
        magic_longbow.attack_bonus > longbow.attack_bonus &&
        magic_longbow.range > longbow.range &&
        magic_longbow.pierces_targets);
    ASSERT("magic longbow has an endgame price and distinct visual",
        magic_longbow.value == 900 &&
        magic_longbow.visual_id == ITEM_VISUAL_MAGIC_LONGBOW);

    Item dagger = item_make_dagger();
    ASSERT("dagger is a one-handed Rogue weapon",
        dagger.weapon_family == WEAPON_FAMILY_DAGGER &&
        dagger.weapon_hands == WEAPON_HANDS_ONE &&
        dagger.class_mask == ITEM_CLASS_ROGUE);
    ASSERT("dagger trades attack for melee critical chance",
        dagger.attack_bonus == 2 && dagger.critical_chance_bonus == 25);
    ASSERT("dagger has a unique visual and specialist price",
        dagger.visual_id == ITEM_VISUAL_DAGGER && dagger.value == 80);

    Item magic_dagger = item_make_magic_dagger();
    ASSERT("magic dagger is a rare one-handed Rogue weapon",
        magic_dagger.weapon_family == WEAPON_FAMILY_DAGGER &&
        magic_dagger.weapon_hands == WEAPON_HANDS_ONE &&
        magic_dagger.rarity == ITEM_RARITY_RARE &&
        magic_dagger.class_mask == ITEM_CLASS_ROGUE);
    ASSERT("magic dagger improves attack and critical chance",
        magic_dagger.attack_bonus == 6 &&
        magic_dagger.critical_chance_bonus == 40 &&
        magic_dagger.attack_bonus > dagger.attack_bonus &&
        magic_dagger.critical_chance_bonus > dagger.critical_chance_bonus);
    ASSERT("magic dagger has a premium price and distinct visual",
        magic_dagger.value == 400 &&
        magic_dagger.visual_id == ITEM_VISUAL_MAGIC_DAGGER);

    Item axe = item_make_battle_axe();
    ASSERT("battle axe is a two-handed Warrior weapon",
        axe.weapon_family == WEAPON_FAMILY_AXE &&
        axe.weapon_hands == WEAPON_HANDS_TWO &&
        axe.class_mask == ITEM_CLASS_WARRIOR);

    Item magic_axe = item_make_magic_battle_axe();
    ASSERT("magic battle axe is a rare two-handed Warrior axe",
        magic_axe.weapon_family == WEAPON_FAMILY_AXE &&
        magic_axe.weapon_hands == WEAPON_HANDS_TWO &&
        magic_axe.rarity == ITEM_RARITY_RARE &&
        magic_axe.class_mask == ITEM_CLASS_WARRIOR);
    ASSERT("magic battle axe specializes against enemy armor",
        magic_axe.attack_bonus == 13 &&
        magic_axe.armor_penetration_percent == 50);
    ASSERT("magic battle axe has a premium price and distinct visual",
        magic_axe.value == 650 &&
        magic_axe.visual_id == ITEM_VISUAL_MAGIC_BATTLE_AXE);

    Item greatsword = item_make_greatsword();
    ASSERT("greatsword is a two-handed uncommon Warrior sword",
        greatsword.weapon_family == WEAPON_FAMILY_SWORD &&
        greatsword.weapon_hands == WEAPON_HANDS_TWO &&
        greatsword.rarity == ITEM_RARITY_UNCOMMON &&
        greatsword.class_mask == ITEM_CLASS_WARRIOR);
    ASSERT("greatsword trades a high price for attack and cleave",
        greatsword.attack_bonus == 12 &&
        greatsword.cleave_percent == 50 &&
        greatsword.value == 350);
    ASSERT("greatsword has a distinct visual identity",
        greatsword.visual_id == ITEM_VISUAL_GREATSWORD);

    Item magic_greatsword = item_make_magic_greatsword();
    ASSERT("magic greatsword is the rare two-handed Warrior sword",
        magic_greatsword.weapon_family == WEAPON_FAMILY_SWORD &&
        magic_greatsword.weapon_hands == WEAPON_HANDS_TWO &&
        magic_greatsword.rarity == ITEM_RARITY_RARE &&
        magic_greatsword.class_mask == ITEM_CLASS_WARRIOR);
    ASSERT("magic greatsword is stronger than the mundane greatsword",
        magic_greatsword.attack_bonus == 18 &&
        magic_greatsword.cleave_percent == 75 &&
        magic_greatsword.attack_bonus > greatsword.attack_bonus &&
        magic_greatsword.cleave_percent > greatsword.cleave_percent);
    ASSERT("magic greatsword has the highest price and its own visual",
        magic_greatsword.value == 800 &&
        magic_greatsword.value > greatsword.value &&
        magic_greatsword.visual_id == ITEM_VISUAL_MAGIC_GREATSWORD);

    Item staff = item_make_staff();
    ASSERT("staff is a two-handed Mage weapon",
        staff.weapon_family == WEAPON_FAMILY_STAFF &&
        staff.weapon_hands == WEAPON_HANDS_TWO &&
        staff.class_mask == ITEM_CLASS_MAGE);

    Item magic_staff = item_make_magic_staff();
    ASSERT("magic staff is a rare two-handed Mage weapon",
        magic_staff.weapon_family == WEAPON_FAMILY_STAFF &&
        magic_staff.weapon_hands == WEAPON_HANDS_TWO &&
        magic_staff.rarity == ITEM_RARITY_RARE &&
        magic_staff.class_mask == ITEM_CLASS_MAGE);
    ASSERT("magic staff favors spell power over melee power",
        magic_staff.attack_bonus == 9 &&
        magic_staff.spell_power_bonus == 8);
    ASSERT("magic staff has an endgame price and distinct visual",
        magic_staff.value == 850 &&
        magic_staff.visual_id == ITEM_VISUAL_MAGIC_STAFF);

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

    // --- Dagger critical strike ---
    GameState rogue = {0};
    rogue.player.player_class = CLASS_ROGUE;
    game_init(&rogue);
    Item guaranteed_critical = dagger;
    guaranteed_critical.critical_chance_bonus = 100;
    int dagger_index = rogue.inventory_count;
    rogue.inventory[rogue.inventory_count++] = guaranteed_critical;
    Action equip_dagger = {ACTION_EQUIP_ITEM, dagger_index, 0};
    action_resolve_player(&rogue, equip_dagger);
    rogue.player.x = 20;
    rogue.player.y = 12;
    rogue.enemy_count = 1;
    rogue.enemies[0] = (Enemy){0};
    rogue.enemies[0].active = 1;
    rogue.enemies[0].x = 21;
    rogue.enemies[0].y = 12;
    rogue.enemies[0].hp = 100;
    rogue.enemies[0].max_hp = 100;
    strncpy(rogue.enemies[0].name, "Target",
        sizeof(rogue.enemies[0].name) - 1);
    Action dagger_strike = {ACTION_MOVE, 21, 12};
    action_resolve_player(&rogue, dagger_strike);
    ASSERT("dagger critical deals one-and-a-half melee damage",
        rogue.enemies[0].hp == 82);

    // --- Greatsword cleave ---
    game_init(&g);
    int greatsword_index = g.inventory_count;
    g.inventory[g.inventory_count++] = greatsword;
    Action equip_greatsword = {ACTION_EQUIP_ITEM, greatsword_index, 0};
    action_resolve_player(&g, equip_greatsword);
    g.player.x = 20;
    g.player.y = 12;
    g.enemy_count = 3;
    for (int i = 0; i < g.enemy_count; i++) {
        g.enemies[i] = (Enemy){0};
        g.enemies[i].active = 1;
        g.enemies[i].hp = 100;
        g.enemies[i].max_hp = 100;
        strncpy(g.enemies[i].name, "Target",
            sizeof(g.enemies[i].name) - 1);
    }
    g.enemies[0].x = 21;
    g.enemies[0].y = 12;
    g.enemies[1].x = 21;
    g.enemies[1].y = 13;
    g.enemies[2].x = 23;
    g.enemies[2].y = 12;
    Action greatsword_strike = {ACTION_MOVE, 21, 12};
    action_resolve_player(&g, greatsword_strike);
    ASSERT("greatsword deals its full damage to the primary target",
        g.enemies[0].hp == 74);
    ASSERT("greatsword cleaves enemies adjacent to the primary target",
        g.enemies[1].hp == 87);
    ASSERT("greatsword does not cleave enemies beyond the target",
        g.enemies[2].hp == 100);

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

    // --- Magic Longbow piercing ---
    game_init(&g);
    int magic_longbow_index = g.inventory_count;
    g.inventory[g.inventory_count++] = magic_longbow;
    Action equip_magic_longbow = {ACTION_EQUIP_ITEM,
        magic_longbow_index, 0};
    action_resolve_player(&g, equip_magic_longbow);
    g.player.x = 20;
    g.player.y = 12;
    g.player.last_dx = 1;
    g.player.last_dy = 0;
    g.enemy_count = 2;
    for (int i = 0; i < g.enemy_count; i++) {
        g.enemies[i] = (Enemy){0};
        g.enemies[i].active = 1;
        g.enemies[i].hp = 100;
        g.enemies[i].max_hp = 100;
        g.enemies[i].x = 22 + i * 2;
        g.enemies[i].y = 12;
        strncpy(g.enemies[i].name, "Target",
            sizeof(g.enemies[i].name) - 1);
    }
    action_resolve_player(&g, shoot);
    ASSERT("magic longbow arrow damages the first target",
        g.enemies[0].hp < 100);
    ASSERT("magic longbow arrow pierces into the second target",
        g.enemies[1].hp < 100);

    // --- Magic Staff spell power ---
    g.player.player_class = CLASS_MAGE;
    game_init(&g);
    int magic_staff_index = g.inventory_count;
    g.inventory[g.inventory_count++] = magic_staff;
    Action equip_magic_staff = {ACTION_EQUIP_ITEM, magic_staff_index, 0};
    action_resolve_player(&g, equip_magic_staff);
    g.player.x = 20;
    g.player.y = 12;
    g.player.last_dx = 1;
    g.player.last_dy = 0;
    g.player.known_spell_count = 1;
    g.player.known_spells[0] = spell_make_magic_arrow();
    g.player.equipped_spell = 0;
    g.enemy_count = 1;
    g.enemies[0] = (Enemy){0};
    g.enemies[0].active = 1;
    g.enemies[0].x = 22;
    g.enemies[0].y = 12;
    g.enemies[0].hp = 100;
    g.enemies[0].max_hp = 100;
    strncpy(g.enemies[0].name, "Target",
        sizeof(g.enemies[0].name) - 1);
    Action cast = {ACTION_CAST_SPELL, 0, 0};
    action_resolve_player(&g, cast);
    ASSERT("magic staff adds spell power to ranged spell damage",
        g.enemies[0].hp == 75);
    g.player.known_spells[0] = spell_make_heal();
    g.player.hp = 1;
    action_resolve_player(&g, cast);
    ASSERT("magic staff adds spell power to healing",
        g.player.hp == 51);

    // --- Magic Battle Axe armor penetration ---
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    int magic_axe_index = g.inventory_count;
    g.inventory[g.inventory_count++] = magic_axe;
    Action equip_magic_axe = {ACTION_EQUIP_ITEM, magic_axe_index, 0};
    action_resolve_player(&g, equip_magic_axe);
    g.player.x = 20;
    g.player.y = 12;
    g.enemy_count = 1;
    g.enemies[0] = (Enemy){0};
    g.enemies[0].active = 1;
    g.enemies[0].x = 21;
    g.enemies[0].y = 12;
    g.enemies[0].hp = 100;
    g.enemies[0].max_hp = 100;
    g.enemies[0].defense = 10;
    strncpy(g.enemies[0].name, "Armored Target",
        sizeof(g.enemies[0].name) - 1);
    Action axe_strike = {ACTION_MOVE, 21, 12};
    action_resolve_player(&g, axe_strike);
    ASSERT("magic battle axe ignores half of enemy defense",
        g.enemies[0].hp == 78);
}
