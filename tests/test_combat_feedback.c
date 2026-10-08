#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/game/combat_feedback.h"
#include <string.h>

static void setup_fight(GameState *g, PlayerClass player_class) {
    g->player.player_class = player_class;
    game_init(g);
    g->location = LOCATION_FOREST;
    g->level = 3;
    g->map.room_count = 1;
    g->map.rooms[0] = (Room){10, 10, 20, 20};
    for (int y = 10; y < 30; y++) {
        for (int x = 10; x < 30; x++) {
            g->map.tiles[y][x] = TILE_FOREST_FLOOR;
        }
    }
    g->player.x = 20;
    g->player.y = 20;
    g->player.hp = 100;
    g->player.max_hp = 100;
    g->player.mp = 100;
    g->player.defense = 0;
    g->player.last_dx = 1;
    g->player.last_dy = 0;
    g->equipped_armor = -1;
    g->equipped_off_hand = -1;
    g->enemy_count = 1;
    g->enemies[0] = (Enemy){
        .type = ENEMY_SKELETON, .x = 21, .y = 20, .active = 1,
        .hp = 100, .max_hp = 100, .attack = 10, .move_timer = 1
    };
    combat_feedback_clear();
}

static void equip_spell(GameState *g, Spell spell) {
    g->player.known_spell_count = 1;
    g->player.known_spells[0] = spell;
    g->player.equipped_spell = 0;
}

static const CombatFeedbackEvent *only_event(void) {
    return combat_feedback_count() == 1 ? combat_feedback_get(0) : NULL;
}

static void test_spell_aim_after_melee(void) {
    static GameState g;
    static const int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for (int direction = 0; direction < 4; direction++) {
        setup_fight(&g, CLASS_MAGE);
        g.location = LOCATION_SWAMP;
        g.level = SWAMP_BOSS_LEVEL;
        g.inventory[0] = item_make_staff();
        g.inventory_count = 1;
        g.equipped_main_hand = 0;
        equip_spell(&g, spell_make_magic_arrow());
        int dx = directions[direction][0];
        int dy = directions[direction][1];
        g.player.last_dx = -dx;
        g.player.last_dy = -dy;
        g.enemies[0].type = ENEMY_SWAMP_DEMON;
        g.enemies[0].x = 20 + dx;
        g.enemies[0].y = 20 + dy;
        g.enemies[0].hp = 1000;
        g.enemies[0].max_hp = 1000;
        g.enemy_count = 2;
        g.enemies[1] = g.enemies[0];
        g.enemies[1].x = 20 - dx;
        g.enemies[1].y = 20 - dy;
        action_resolve_player(&g, (Action){ACTION_MOVE, 20 + dx, 20 + dy});
        ASSERT("staff melee faces the Demon without moving the player",
            g.player.x == 20 && g.player.y == 20 && g.enemies[0].hp < 1000 &&
            g.player.last_dx == dx && g.player.last_dy == dy);
        int hp = g.enemies[0].hp;
        int mp = g.player.mp;
        action_resolve_player(&g, (Action){ACTION_CAST_SPELL, 0, 0});
        ASSERT("Magic Arrow hits the melee target instead of firing behind the player",
            g.enemies[0].hp < hp && g.enemies[1].hp == 1000 && g.player.mp < mp);
        ASSERT("Magic Arrow animation points toward the adjacent melee target",
            g.trail_count == 1 && g.trail[0].x == 20 + dx &&
            g.trail[0].y == 20 + dy && g.trail[0].is_impact);
    }
}

static void test_fireball_collisions(void) {
    static GameState g;
    static const int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    static const EnemyType types[4] = {ENEMY_SKELETON, ENEMY_ASH_HOUND, ENEMY_OBSIDIAN_GUARDIAN, ENEMY_CINDER_LORD};
    Action cast = {ACTION_CAST_SPELL, 0, 0};
    for (int direction = 0; direction < 4; direction++) {
        setup_fight(&g, CLASS_MAGE);
        equip_spell(&g, spell_make_fireball());
        int dx = directions[direction][0];
        int dy = directions[direction][1];
        g.player.last_dx = dx;
        g.player.last_dy = dy;
        g.enemies[0].type = types[direction];
        g.enemies[0].x = 20 + dx;
        g.enemies[0].y = 20 + dy;
        g.enemy_count = 3;
        g.enemies[1] = g.enemies[0];
        g.enemies[1].x += dy;
        g.enemies[1].y += dx;
        g.enemies[2] = g.enemies[0];
        g.enemies[2].x = 20 + 4 * dx;
        g.enemies[2].y = 20 + 4 * dy;
        action_resolve_player(&g, cast);
        ASSERT("Fireball damages an adjacent enemy in every direction and spends mana once",
            g.enemies[0].hp < 100 && g.player.mp == 80);
        ASSERT("Fireball explodes on the melee target and damages nearby enemies rather than passing through",
            g.enemies[1].hp < 100 && g.enemies[2].hp == 100 && combat_feedback_count() == 2);
        ASSERT("Fireball's animated impact matches the adjacent blast location",
            g.trail_count == 1 && g.trail[0].x == 20 + dx &&
            g.trail[0].y == 20 + dy && g.trail[0].is_impact);
    }

    setup_fight(&g, CLASS_MAGE);
    equip_spell(&g, spell_make_fireball());
    g.enemies[0].active = 0;
    g.enemy_count = 2;
    g.enemies[1] = g.enemies[0];
    g.enemies[1].active = 1;
    g.enemies[1].x = 22;
    action_resolve_player(&g, cast);
    ASSERT("inactive enemies do not stop Fireball before its first living target",
        g.enemies[0].hp == 100 && g.enemies[1].hp < 100 &&
        g.trail_count == 2 && g.trail[1].x == 22 && g.trail[1].is_impact);

    setup_fight(&g, CLASS_MAGE);
    equip_spell(&g, spell_make_fireball());
    g.enemies[0].x = 25;
    g.map.tiles[20][22] = TILE_FOREST_WALL;
    action_resolve_player(&g, cast);
    const CombatFeedbackEvent *event = only_event();
    ASSERT("a wall stops Fireball's blast at the visible end of its path",
        g.enemies[0].hp == 100 && g.trail_count == 1 && g.trail[0].is_impact);
    ASSERT("a blocked Fireball reports its miss at the actual blast location",
        event && event->kind == FEEDBACK_MISS && event->x == 21 && event->y == 20);

    setup_fight(&g, CLASS_MAGE);
    equip_spell(&g, spell_make_fireball());
    g.enemies[0].x = 24;
    g.map.tiles[20][21] = TILE_FOREST_WALL;
    action_resolve_player(&g, cast);
    ASSERT("Fireball cannot damage a distant enemy when its first step is blocked",
        g.enemies[0].hp == 100 && g.trail_count == 0 && g.player.mp == 80);
}

void test_combat_feedback(void) {
    printf("Combat feedback tests:\n");
    test_spell_aim_after_melee();
    test_fireball_collisions();
    static GameState g;
    Action cast = {ACTION_CAST_SPELL, 0, 0};

    setup_fight(&g, CLASS_WARRIOR);
    action_resolve_player(&g, (Action){ACTION_MOVE, 21, 20});
    const CombatFeedbackEvent *e = only_event();
    ASSERT("melee hit shows the damage dealt on the enemy's tile",
        e && e->kind == FEEDBACK_ENEMY_DAMAGE && e->arrival == FEEDBACK_NOW &&
        e->x == 21 && e->y == 20 && e->amount == 100 - g.enemies[0].hp);
    ASSERT("feedback records the area and stage it happened in",
        e && e->location == LOCATION_FOREST && e->level == 3);
    ASSERT("an ordinary hit message keeps the plain colour",
        g.message_kinds[g.message_count - 1] == MESSAGE_NORMAL);

    combat_feedback_clear();
    action_resolve_enemies(&g);
    e = only_event();
    ASSERT("enemy melee shows the damage taken on the player's tile",
        e && e->kind == FEEDBACK_PLAYER_DAMAGE && e->arrival == FEEDBACK_NOW &&
        e->x == 20 && e->y == 20 && e->amount == 10 && g.player.hp == 90);
    ASSERT("damage taken messages are marked red",
        g.message_kinds[g.message_count - 1] == MESSAGE_DAMAGE_TAKEN);

    setup_fight(&g, CLASS_WARRIOR);
    g.enemies[0].type = ENEMY_GOBLIN_ARCHER;
    g.enemies[0].x = 14;
    action_resolve_enemies(&g);
    e = only_event();
    ASSERT("an enemy's ranged hit appears when its projectile lands",
        e && e->kind == FEEDBACK_PLAYER_DAMAGE &&
        e->arrival == FEEDBACK_AFTER_ENEMY_SHOT && e->amount == 10 &&
        g.player.hp == 90);

    setup_fight(&g, CLASS_MAGE);
    g.enemies[0].type = ENEMY_WRAITH;
    g.player.mp = 10;
    action_resolve_enemies(&g);
    ASSERT("Wraith hits show the HP lost and the MP drained",
        combat_feedback_count() == 2 &&
        combat_feedback_get(0)->kind == FEEDBACK_PLAYER_DAMAGE &&
        combat_feedback_get(0)->amount == 100 - g.player.hp &&
        combat_feedback_get(1)->kind == FEEDBACK_MANA_LOSS &&
        combat_feedback_get(1)->amount == 3 && g.player.mp == 7);

    setup_fight(&g, CLASS_ROGUE);
    g.inventory[0] = item_make_bow();
    g.inventory_count = 1;
    g.equipped_main_hand = 0;
    g.enemies[0].x = 23;
    action_resolve_player(&g, (Action){ACTION_RANGED_ATTACK, 0, 0});
    e = only_event();
    ASSERT("a bow hit appears when the arrow lands",
        e && (e->kind == FEEDBACK_ENEMY_DAMAGE ||
        e->kind == FEEDBACK_ENEMY_CRITICAL) &&
        e->arrival == FEEDBACK_AFTER_PLAYER_SHOT && e->x == 23 &&
        e->y == 20 && e->amount == 100 - g.enemies[0].hp && e->amount > 0);

    // Bows crit 15% of the time for half again the damage: 10 becomes 15.
    g.player.attack = 10;
    int crits = 0;
    int normal_hits = 0;
    int kinds_match = 1;
    for (int i = 0; i < 200; i++) {
        g.enemies[0].hp = 1000;
        g.player.arrows = MAX_ARROWS;
        combat_feedback_clear();
        action_resolve_player(&g, (Action){ACTION_RANGED_ATTACK, 0, 0});
        e = only_event();
        int lost = 1000 - g.enemies[0].hp;
        kinds_match &= e && e->amount == lost &&
            (lost == 15 ? e->kind == FEEDBACK_ENEMY_CRITICAL
            : lost == 10 && e->kind == FEEDBACK_ENEMY_DAMAGE);
        crits += lost == 15;
        normal_hits += lost == 10;
    }
    ASSERT("bow crits are marked critical and ordinary arrows are not",
        kinds_match && crits > 0 && normal_hits > 0);

    setup_fight(&g, CLASS_WARRIOR);
    g.inventory[0] = item_make_dagger();
    g.inventory[0].critical_chance_bonus = 100;
    g.inventory_count = 1;
    g.equipped_main_hand = 0;
    g.player.attack = 10;
    action_resolve_player(&g, (Action){ACTION_MOVE, 21, 20});
    e = only_event();
    ASSERT("a melee crit is marked critical with its boosted damage",
        e && e->kind == FEEDBACK_ENEMY_CRITICAL && e->arrival == FEEDBACK_NOW &&
        e->x == 21 && e->amount == 15 && g.enemies[0].hp == 85);
    ASSERT("critical hit messages are marked gold",
        g.message_kinds[g.message_count - 1] == MESSAGE_CRITICAL);

    // Shields halve the damage of a blocked hit: 10 becomes 5.
    setup_fight(&g, CLASS_WARRIOR);
    g.inventory[0] = item_make_buckler();
    g.inventory[0].block_chance = 100;
    g.inventory_count = 1;
    g.equipped_main_hand = -1;
    g.equipped_off_hand = 0;
    action_resolve_enemies(&g);
    ASSERT("a block shows the reduced damage with BLOCK above it",
        combat_feedback_count() == 2 &&
        combat_feedback_get(0)->kind == FEEDBACK_PLAYER_DAMAGE &&
        combat_feedback_get(0)->amount == 5 &&
        combat_feedback_get(1)->kind == FEEDBACK_BLOCK &&
        combat_feedback_get(1)->amount == 5 &&
        combat_feedback_get(1)->x == 20 && g.player.hp == 95);
    ASSERT("the block message says how much damage the shield stopped",
        g.message_count >= 2 &&
        strcmp(g.messages[g.message_count - 2], "Blocked 5 of 10 damage!") == 0);
    ASSERT("block messages are marked blue and the damage after them red",
        g.message_kinds[g.message_count - 2] == MESSAGE_DEFENDED &&
        g.message_kinds[g.message_count - 1] == MESSAGE_DAMAGE_TAKEN);

    setup_fight(&g, CLASS_ROGUE);
    g.inventory[0] = item_make_leather_armor();
    g.inventory[0].evasion_chance = 100;
    g.inventory_count = 1;
    g.equipped_main_hand = -1;
    g.equipped_armor = 0;
    action_resolve_enemies(&g);
    e = only_event();
    ASSERT("a dodge shows DODGE on the player and no damage",
        e && e->kind == FEEDBACK_DODGE && e->arrival == FEEDBACK_NOW &&
        e->x == 20 && e->y == 20 && g.player.hp == 100);
    ASSERT("dodge messages are marked blue",
        strcmp(g.messages[g.message_count - 1], "Dodged!") == 0 &&
        g.message_kinds[g.message_count - 1] == MESSAGE_DEFENDED);
    g.enemies[0].type = ENEMY_GOBLIN_ARCHER;
    g.enemies[0].x = 14;
    g.enemies[0].move_timer = 1;
    combat_feedback_clear();
    action_resolve_enemies(&g);
    e = only_event();
    ASSERT("a dodged arrow shows DODGE when the arrow arrives",
        e && e->kind == FEEDBACK_DODGE &&
        e->arrival == FEEDBACK_AFTER_ENEMY_SHOT && g.player.hp == 100);

    setup_fight(&g, CLASS_ROGUE);
    g.inventory[0] = item_make_bow();
    g.inventory_count = 1;
    g.equipped_main_hand = 0;
    g.enemy_count = 0;
    action_resolve_player(&g, (Action){ACTION_RANGED_ATTACK, 0, 0});
    e = only_event();
    ASSERT("a missed arrow shows MISS where it lands",
        e && e->kind == FEEDBACK_MISS &&
        e->arrival == FEEDBACK_AFTER_PLAYER_SHOT && e->x == 26 && e->y == 20);

    setup_fight(&g, CLASS_MAGE);
    equip_spell(&g, spell_make_magic_arrow());
    g.enemy_count = 0;
    action_resolve_player(&g, cast);
    e = only_event();
    ASSERT("a missed Magic Arrow shows MISS where it fades",
        e && e->kind == FEEDBACK_MISS &&
        e->arrival == FEEDBACK_AFTER_PLAYER_SHOT && e->x == 26 && e->y == 20);
    equip_spell(&g, spell_make_fireball());
    combat_feedback_clear();
    action_resolve_player(&g, cast);
    e = only_event();
    ASSERT("a Fireball that catches no one shows MISS at the blast",
        e && e->kind == FEEDBACK_MISS && e->x == 24 && e->y == 20);

    setup_fight(&g, CLASS_MAGE);
    equip_spell(&g, spell_make_magic_arrow());
    g.enemies[0].x = 23;
    action_resolve_player(&g, cast);
    e = only_event();
    ASSERT("a Magic Arrow hit appears when the spell lands",
        e && e->kind == FEEDBACK_ENEMY_DAMAGE &&
        e->arrival == FEEDBACK_AFTER_PLAYER_SHOT && e->x == 23 &&
        e->amount == 100 - g.enemies[0].hp && e->amount > 0);

    setup_fight(&g, CLASS_MAGE);
    equip_spell(&g, spell_make_fireball());
    g.enemy_count = 3;
    g.enemies[0].x = 24;
    g.enemies[1] = g.enemies[0];
    g.enemies[1].x = 25;
    g.enemies[2] = g.enemies[0];
    g.enemies[2].x = 28;
    action_resolve_player(&g, cast);
    ASSERT("Fireball shows one number for each enemy in the blast",
        combat_feedback_count() == 2 &&
        combat_feedback_get(0)->x == 24 &&
        combat_feedback_get(0)->amount == 100 - g.enemies[0].hp &&
        combat_feedback_get(1)->x == 25 &&
        combat_feedback_get(1)->amount == 100 - g.enemies[1].hp &&
        combat_feedback_get(1)->arrival == FEEDBACK_AFTER_PLAYER_SHOT &&
        g.enemies[2].hp == 100);

    setup_fight(&g, CLASS_MAGE);
    equip_spell(&g, spell_make_heal());
    g.player.hp = 40;
    action_resolve_player(&g, cast);
    e = only_event();
    ASSERT("Heal shows the HP restored on the player's tile",
        e && e->kind == FEEDBACK_HEAL && e->arrival == FEEDBACK_NOW &&
        e->x == 20 && e->y == 20 && e->amount == 60 && g.player.hp == 100);

    setup_fight(&g, CLASS_WARRIOR);
    g.enemy_count = 0;
    g.player.poison_turns = 2;
    action_resolve_player(&g, (Action){ACTION_MOVE, 20, 19});
    e = only_event();
    ASSERT("poison damage shows on the tile the player moved to",
        e && e->kind == FEEDBACK_PLAYER_DAMAGE && e->x == 20 &&
        e->y == 19 && e->amount == 3 && g.player.hp == 97);
    ASSERT("poison messages are marked green",
        g.message_kinds[g.message_count - 1] == MESSAGE_POISON);

    setup_fight(&g, CLASS_WARRIOR);
    g.enemy_count = 2;
    g.enemies[0] = (Enemy){
        .type = ENEMY_GOBLIN_SHAMAN, .x = 20, .y = 16, .active = 1,
        .hp = 10, .max_hp = 10, .attack = 1, .move_timer = 2
    };
    g.enemies[1] = (Enemy){
        .type = ENEMY_SKELETON, .x = 22, .y = 16, .active = 1,
        .hp = 50, .max_hp = 100, .attack = 1
    };
    action_resolve_enemies(&g);
    e = only_event();
    ASSERT("Goblin Shaman heals show the HP restored on the healed ally",
        e && e->kind == FEEDBACK_HEAL && e->x == 22 && e->y == 16 &&
        e->amount == 6 && g.enemies[1].hp == 56);

    combat_feedback_clear();
    for (int i = 1; i <= MAX_COMBAT_FEEDBACK + 1; i++) {
        combat_feedback_add(&g, FEEDBACK_ENEMY_DAMAGE, FEEDBACK_NOW, 0, 0, i);
    }
    ASSERT("a full feedback list keeps the newest results",
        combat_feedback_count() == MAX_COMBAT_FEEDBACK &&
        combat_feedback_get(0)->amount == 2 &&
        combat_feedback_get(MAX_COMBAT_FEEDBACK - 1)->amount ==
            MAX_COMBAT_FEEDBACK + 1);
    combat_feedback_clear();
    ASSERT("clearing removes every result", combat_feedback_count() == 0);

    g.message_count = 0;
    push_message_kind(&g, "one", MESSAGE_POISON);
    push_message(&g, "two");
    push_message_kind(&g, "three", MESSAGE_CRITICAL);
    push_message_kind(&g, "four", MESSAGE_DAMAGE_TAKEN);
    ASSERT("message colours stay with their messages as old ones scroll away",
        g.message_count == MAX_MESSAGES && strcmp(g.messages[0], "two") == 0 &&
        g.message_kinds[0] == MESSAGE_NORMAL &&
        g.message_kinds[1] == MESSAGE_CRITICAL &&
        g.message_kinds[2] == MESSAGE_DAMAGE_TAKEN);
}
