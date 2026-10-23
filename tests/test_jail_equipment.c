#include "test_utils.h"
#include "game/jail.h"
#include "game/actions.h"
#include "systems/save_load.h"
#include "screens/quest_journal.h"
#include <stdlib.h>
#include <string.h>

#define GEAR_SLOT 99283

static void arrest(GameState *g) {
    game_enter_town_gate(g, LOCATION_CASTLE, INFORMANT_X, INFORMANT_Y + 1);
    jail_talk_nearby(g);
}

static void escape_cell(GameState *g) {
    g->player.x = JAIL_PRISONER_X;
    g->player.y = JAIL_PRISONER_Y + 1;
    jail_talk_nearby(g);
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    g->player.x = JAIL_HATCH_X - 1;
    g->player.y = JAIL_HATCH_Y;
    action_resolve_player(g, (Action){ACTION_MOVE, JAIL_HATCH_X, JAIL_HATCH_Y});
}

static void enter_evidence(GameState *g) {
    game_enter_town_gate(g, LOCATION_CASTLE, JAIL_DOOR_X, JAIL_DOOR_Y + 1);
    action_resolve_player(g, (Action){ACTION_MOVE, JAIL_DOOR_X, JAIL_DOOR_Y});
}

static void chest(GameState *g) {
    g->player.x = JAIL_GEAR_X;
    g->player.y = JAIL_GEAR_Y + 1;
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
}

void test_jail_equipment(void) {
    printf("Confiscated equipment and Royal Guard recovery:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("equipment recovery fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    ASSERT("equipment recovery test slot is unused", !save_exists(GEAR_SLOT));
    if (save_exists(GEAR_SLOT)) {
        free(g);
        free(loaded);
        return;
    }
    g->player.player_class = CLASS_WARRIOR;
    game_init(g);
    while (g->inventory_count) {
        game_remove_inventory_item(g, g->inventory_count - 1);
    }
    int base_attack = g->player.attack;
    int base_defense = g->player.defense;
    g->inventory[0] = item_make_health_potion();
    g->inventory[1] = item_make_magic_long_sword();
    g->inventory[1].sharpened = 1;
    g->inventory[1].attack_bonus++;
    g->inventory[1].critical_chance_bonus = 13;
    g->inventory[1].gold_drop_bonus_percent = 17;
    g->inventory[2] = item_make_buckler();
    g->inventory[3] = item_make_plate_armor();
    g->inventory[3].max_hp_bonus = 12;
    g->inventory[3].max_mp_bonus = 8;
    g->inventory[4] = item_make_treasure_map();
    g->inventory[5] = item_make_shortbow();
    g->inventory[6] = item_make_mana_potion();
    g->inventory_count = 7;
    game_equip_main_hand(g, 1);
    game_equip_shield(g, 2);
    action_resolve_player(g, (Action){ACTION_EQUIP_ITEM, 3, 0});
    Item original[4] = {g->inventory[1], g->inventory[2], g->inventory[3], g->inventory[5]};
    g->gold = 321;
    arrest(g);
    ASSERT("guards confiscate equipped and spare weapons, armor and shields", g->location == LOCATION_JAIL && g->inventory_count == 3 && g->jail_gear_count == 4 && g->jail_gear_state == 1 && g->equipped_main_hand == -1 && g->equipped_off_hand == -1 && g->equipped_armor == -1);
    ASSERT("confiscation removes equipment stat bonuses and retains gold and quest items", g->player.attack == base_attack && g->player.defense == base_defense && g->gold == 321 && g->inventory[1].type == ITEM_TREASURE_MAP);
    ASSERT("the evidence store retains every item property", !memcmp(original, g->jail_gear, sizeof(original)));
    ASSERT("confiscated gear survives saving before the NPC quest is accepted", save_game(g, GEAR_SLOT) && load_game(loaded, GEAR_SLOT) && loaded->jail_gear_count == 4 && !memcmp(original, loaded->jail_gear, sizeof(original)) && loaded->equipped_main_hand == -1 && loaded->inventory_count == 3);
    *g = *loaded;
    g->debug_passages_unlocked = 1;
    g->map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] = TILE_JAIL_HATCH;
    game_refresh_debug_passages(g);
    g->player.x = JAIL_HATCH_X - 1;
    g->player.y = JAIL_HATCH_Y;
    action_resolve_player(g, (Action){ACTION_MOVE, JAIL_HATCH_X, JAIL_HATCH_Y});
    ASSERT("debug mode cannot reveal or enter the secret tunnel before accepting Tomas's quest", g->location == LOCATION_JAIL && g->map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] == TILE_CASTLE_FLOOR);
    g->debug_passages_unlocked = 0;
    escape_cell(g);
    ASSERT("accepting the quest reveals the hatch and entering begins the escort", g->location == LOCATION_ESCAPE_TUNNEL && g->jail_quest_state == 2);
    ASSERT("Tomas provides a usable escape weapon without returning confiscated gear", g->jail_supplies_taken && g->equipped_main_hand >= 0 && g->jail_gear_claimed == 0 && g->enemies[0].x >= 18);
    g->player.x = ESCAPE_TUNNEL_W - 2;
    g->player.y = 14;
    g->prisoner_x = ESCAPE_TUNNEL_W - 3;
    g->prisoner_y = 14;
    action_resolve_player(g, (Action){ACTION_MOVE, ESCAPE_TUNNEL_W - 1, 14});
    ASSERT("escaping with Tomas leaves a separate equipment recovery quest", g->location == LOCATION_TOWN4 && g->jail_quest_state == 3 && g->jail_gear_state == 1);
    enter_evidence(g);
    ASSERT("returning to the jail building enters an evidence room defended by Royal Guards", g->location == LOCATION_JAIL_ARMORY && g->enemy_count == 3 && g->enemies[0].type == ENEMY_ROYAL_GUARD && !strcmp(g->enemies[0].name, "Royal Guard"));
    int inventory = g->inventory_count;
    chest(g);
    ASSERT("the evidence chest stays locked while its Royal Guards live", g->inventory_count == inventory && !g->jail_gear_claimed);
    int hp = g->player.hp;
    g->enemies[0].x = g->player.x - 1;
    g->enemies[0].y = g->player.y;
    g->player.defense = 0;
    action_resolve_enemies(g);
    ASSERT("Royal Guards are combat enemies who can damage the player", g->player.hp < hp);
    g->enemies[0].hp = 47;
    g->player.x = 11;
    g->player.y = 16;
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, 0, 0});
    action_resolve_player(g, (Action){ACTION_MOVE, 11, 17});
    ASSERT("retreating from the jail preserves wounded guards and dropped items", g->location == LOCATION_CASTLE && g->jail_guards[0].hp == 47 && g->jail_armory_loot_count > 0);
    ASSERT("saved evidence-room progress survives outside the jail", save_game(g, GEAR_SLOT) && load_game(loaded, GEAR_SLOT) && loaded->jail_guards[0].hp == 47 && loaded->jail_armory_loot_count == g->jail_armory_loot_count);
    *g = *loaded;
    enter_evidence(g);
    ASSERT("returning does not heal or respawn guards or erase dropped loot", g->enemies[0].hp == 47 && g->floor_item_count > 0 && g->map.tiles[g->floor_items[0].y][g->floor_items[0].x] == TILE_ITEM);
    g->player.attack = 1000;
    g->player.defense = 1000;
    for (int i = 0; i < 3; i++) {
        Enemy *guard = &g->enemies[i];
        g->player.x = guard->x - 1;
        g->player.y = guard->y;
        action_resolve_player(g, (Action){ACTION_MOVE, guard->x, guard->y});
    }
    ASSERT("actual combat defeats all three Royal Guards", !g->enemies[0].active && !g->enemies[1].active && !g->enemies[2].active);
    while (g->inventory_count < MAX_INVENTORY) {
        g->inventory[g->inventory_count++] = item_make_health_potion();
    }
    chest(g);
    ASSERT("a full inventory leaves confiscated items safely in the chest", !g->jail_gear_claimed && g->jail_gear_state == 1);
    game_remove_inventory_item(g, MAX_INVENTORY - 1);
    chest(g);
    ASSERT("one free slot reclaims the original main weapon with its upgrades", g->jail_gear_claimed == (1 << g->jail_main_hand) && g->jail_gear_state == 1 && !memcmp(&g->inventory[MAX_INVENTORY - 1], &original[0], sizeof(Item)));
    ASSERT("partial recovery and defeated guards survive an active-room save", save_game(g, GEAR_SLOT) && load_game(loaded, GEAR_SLOT) && loaded->jail_gear_claimed == g->jail_gear_claimed && !loaded->enemies[0].active);
    *g = *loaded;
    for (int attempt = 0; g->jail_gear_state == 1 && attempt < 4; attempt++) {
        game_remove_inventory_item(g, 0);
        chest(g);
    }
    ASSERT("recovering all confiscated items completes the equipment quest", g->jail_gear_state == 3 && g->jail_gear_claimed == 15);
    int exact = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < g->inventory_count; j++) {
            exact += !memcmp(&original[i], &g->inventory[j], sizeof(Item));
        }
    }
    ASSERT("all original equipment returns with exact bonuses, rarity and sharpening", exact == 4);
    inventory = g->inventory_count;
    chest(g);
    ASSERT("reopening the recovered chest cannot duplicate equipment", g->inventory_count == inventory);
    int completed = 0;
    for (int i = 0; i < quest_journal_count(g, QUEST_TAB_COMPLETED); i++) {
        QuestJournalEntry entry;
        if (quest_journal_get_entry(g, QUEST_TAB_COMPLETED, i, &entry) && !strcmp(entry.title, "Recover Your Equipment")) {
            completed = entry.objective_complete[0];
        }
    }
    ASSERT("the completed journal records equipment recovery", completed);
    ASSERT("completed recovery survives reload without reopening the stash", save_game(g, GEAR_SLOT) && load_game(loaded, GEAR_SLOT) && loaded->jail_gear_state == 3 && loaded->jail_gear_claimed == 15);
    const PlayerClass classes[] = {CLASS_MAGE, CLASS_ROGUE};
    for (int i = 0; i < 2; i++) {
        g->player.player_class = classes[i];
        game_init(g);
        arrest(g);
        escape_cell(g);
        ASSERT("each class can fight through the escape after confiscation", g->location == LOCATION_ESCAPE_TUNNEL && g->equipped_main_hand >= 0 && g->inventory[g->equipped_main_hand].type == ITEM_WEAPON && (classes[i] != CLASS_ROGUE || g->player.arrows >= 10));
    }
    remove("saves/savegame_99283.json");
    free(g);
    free(loaded);
}
