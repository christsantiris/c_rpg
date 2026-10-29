#include "test_utils.h"
#include "game/jail.h"
#include "game/actions.h"
#include "systems/save_load.h"
#include "screens/quest_journal.h"
#include "cJSON.h"
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
    action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
}

static int rewrite_resources(int version, const char *field, int value) {
    FILE *file = fopen("saves/savegame_99283.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = calloc((size_t)size + 1, 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    fread(data, 1, (size_t)size, file);
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    if (field) {
        cJSON_ReplaceItemInObject(root, field, cJSON_CreateNumber(value));
    } else {
        cJSON_DeleteItemFromObject(root, "jail_gold");
        cJSON_DeleteItemFromObject(root, "jail_arrows");
    }
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99283.json", "wb");
    int ok = file && json && fwrite(json, 1, strlen(json), file) == strlen(json);
    if (file) {
        fclose(file);
    }
    free(json);
    return ok;
}

static void unguarded_evidence(GameState *g) {
    g->jail_quest_state = 3;
    enter_evidence(g);
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
}

static void test_currency_and_legacy_saves(GameState *g, GameState *loaded) {
    g->player.player_class = CLASS_WARRIOR;
    game_init(g);
    g->inventory_count = 0;
    g->gold = 123;
    g->score = 42;
    g->player.known_spell_count = 1;
    g->player.known_spells[0] = spell_make_magic_arrow();
    spell_upgrade(&g->player.known_spells[0]);
    g->defeated_bosses = 1 << LOCATION_DUNGEON;
    arrest(g);
    ASSERT("gold alone creates a recoverable evidence claim", g->jail_gear_count == 0 && g->jail_gear_state == 1 && g->jail_gold == 123 && g->gold == 0);
    ASSERT("arrest preserves learned spells, score and completed bosses", g->player.known_spells[0].rank == 2 && g->score == 42 && g->defeated_bosses == (1 << LOCATION_DUNGEON));
    int ok = save_game(g, GEAR_SLOT) && load_game(loaded, GEAR_SLOT);
    ASSERT("a pending gold-only claim survives reload", ok && loaded->jail_gold == 123 && loaded->jail_gear_count == 0 && loaded->jail_gear_state == 1 && loaded->gold == 0);
    ASSERT("missing confiscation fields are rejected in new saves", save_game(g, GEAR_SLOT) && rewrite_resources(225, NULL, 0) && !load_game(loaded, GEAR_SLOT));
    ASSERT("negative confiscated gold is rejected", save_game(g, GEAR_SLOT) && rewrite_resources(225, "jail_gold", -1) && !load_game(loaded, GEAR_SLOT));
    ASSERT("confiscated ammunition cannot exceed the quiver limit", save_game(g, GEAR_SLOT) && rewrite_resources(225, "jail_arrows", MAX_ARROWS + 1) && !load_game(loaded, GEAR_SLOT));
    g->player.x = JAIL_PRISONER_X;
    g->player.y = JAIL_PRISONER_Y + 1;
    jail_talk_nearby(g);
    jail_talk_nearby(g);
    ASSERT("Tomas gives one dagger even when there were no confiscated items", g->jail_supplies_taken && g->inventory_count == 0 && game_equipped_item(g, EQUIPMENT_STRONG_HAND) && !strcmp(g->inventory[EQUIPMENT_STRONG_HAND].name, "Smuggled Dagger"));
    ok = save_game(g, GEAR_SLOT) && load_game(loaded, GEAR_SLOT);
    ASSERT("the single smuggled dagger and pending gold persist in the cell", ok && loaded->jail_supplies_taken && loaded->inventory_count == 0 && loaded->jail_gold == 123 && game_equipped_item(loaded, EQUIPMENT_STRONG_HAND));
    if (ok) {
        *g = *loaded;
    }
    escape_cell(g);
    ASSERT("entering the tunnel cannot give a second dagger", g->location == LOCATION_ESCAPE_TUNNEL && g->inventory_count == 0 && g->jail_supplies_taken && g->gold == 0 && g->player.arrows == 0);
    unguarded_evidence(g);
    g->gold = 7;
    while (g->inventory_count < MAX_INVENTORY) {
        g->inventory[g->inventory_count++] = item_make_health_potion();
    }
    chest(g);
    ASSERT("gold-only evidence is recovered with a full bag without overwriting later earnings", g->gold == 130 && g->jail_gold == 0 && g->jail_gear_state == 3 && g->inventory_count == MAX_INVENTORY);
    ASSERT("completed gold-only recovery survives reload", save_game(g, GEAR_SLOT) && load_game(loaded, GEAR_SLOT) && loaded->gold == 130 && loaded->jail_gold == 0 && loaded->jail_gear_state == 3);
    chest(g);
    ASSERT("reopening a gold-only claim cannot pay twice", g->gold == 130 && g->score == 42);

    g->player.player_class = CLASS_ROGUE;
    game_init(g);
    g->inventory_count = 0;
    g->player.arrows = 73;
    arrest(g);
    ASSERT("arrows alone are confiscated and tracked for recovery", g->player.arrows == 0 && g->jail_arrows == 73 && g->jail_gear_count == 0 && g->jail_gear_state == 1);
    unguarded_evidence(g);
    g->player.arrows = 98;
    chest(g);
    ASSERT("a nearly full quiver takes only the arrows that fit", g->player.arrows == MAX_ARROWS && g->jail_arrows == 71 && g->jail_gear_state == 1);
    ok = save_game(g, GEAR_SLOT) && load_game(loaded, GEAR_SLOT);
    ASSERT("partial ammunition recovery survives reload", ok && loaded->player.arrows == MAX_ARROWS && loaded->jail_arrows == 71 && loaded->jail_gear_state == 1);
    if (ok) {
        *g = *loaded;
    }
    g->player.arrows = 0;
    chest(g);
    chest(g);
    ASSERT("remaining arrows recover exactly once after quiver space is made", g->player.arrows == 71 && g->jail_arrows == 0 && g->jail_gear_state == 3);

    g->player.player_class = CLASS_WARRIOR;
    game_init(g);
    arrest(g);
    // An arrest made under version 224 kept consumables and money on the player.
    g->gold = 333;
    g->player.arrows = 17;
    g->inventory[0] = item_make_health_potion();
    g->inventory_count = 1;
    loaded->jail_gold = 999;
    loaded->jail_arrows = 99;
    ok = save_game(g, GEAR_SLOT) && rewrite_resources(224, NULL, 0) && load_game(loaded, GEAR_SLOT);
    ASSERT("version-224 arrests keep existing possessions and default new evidence fields to zero", ok && loaded->gold == 333 && loaded->player.arrows == 17 && loaded->inventory_count == 1 && loaded->inventory[0].type == ITEM_POTION_HEALTH && loaded->jail_gold == 0 && loaded->jail_arrows == 0 && loaded->jail_gear_state == 1);
    ASSERT("an older arrest remains stable after saving in the new format", ok && save_game(loaded, GEAR_SLOT) && load_game(g, GEAR_SLOT) && g->gold == 333 && g->player.arrows == 17 && g->jail_gear_count == 1 && g->jail_gold == 0 && g->jail_arrows == 0);
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
    Item original[7] = {g->inventory[1], g->inventory[2], g->inventory[3], g->inventory[0], g->inventory[4], g->inventory[5], g->inventory[6]};
    game_equip_main_hand(g, 1);
    game_equip_shield(g, 1);
    action_resolve_player(g, (Action){ACTION_EQUIP_ITEM, 1, 0});
    g->gold = 321;
    arrest(g);
    ASSERT("guards confiscate all bag items and worn equipment", g->location == LOCATION_JAIL && g->inventory_count == 0 && g->jail_gear_count == 7 && g->jail_gear_state == 1 && g->equipped_main_hand == -1 && g->equipped_off_hand == -1 && g->equipped_armor == -1);
    ASSERT("confiscation removes equipment bonuses and stores gold and quest items", g->player.attack == base_attack && g->player.defense == base_defense && g->gold == 0 && g->jail_gold == 321 && g->jail_gear[4].type == ITEM_TREASURE_MAP);
    ASSERT("the evidence store retains every item property", !memcmp(original, g->jail_gear, sizeof(original)));
    ASSERT("confiscated gear survives saving before the NPC quest is accepted", save_game(g, GEAR_SLOT) && load_game(loaded, GEAR_SLOT) && loaded->jail_gear_count == 7 && !memcmp(original, loaded->jail_gear, sizeof(original)) && loaded->equipped_main_hand == -1 && loaded->inventory_count == 0 && loaded->gold == 0 && loaded->jail_gold == 321);
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
    const Item *smuggled = &g->inventory[g->equipped_main_hand];
    ASSERT("the smuggled escape weapon is a dagger every class can wield", !strcmp(smuggled->name, "Smuggled Dagger") && smuggled->weapon_family == WEAPON_FAMILY_DAGGER &&
        item_class_allowed(smuggled, CLASS_WARRIOR) && item_class_allowed(smuggled, CLASS_MAGE) && item_class_allowed(smuggled, CLASS_ROGUE));
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
    ASSERT("the evidence chest stays locked while its Royal Guards live", g->inventory_count == inventory && !g->jail_gear_claimed && g->jail_gold == 321);
    int hp = g->player.hp;
    g->enemies[0].x = g->player.x - 1;
    g->enemies[0].y = g->player.y;
    g->player.defense = 0;
    action_resolve_enemies(g);
    ASSERT("Royal Guards are combat enemies who can damage the player", g->player.hp < hp);
    g->enemies[0].hp = 47;
    g->player.x = 11;
    g->player.y = 16;
    g->inventory[g->inventory_count++] = item_make_health_potion();
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
    int gold_before_recovery = g->gold;
    int score_before_recovery = g->score;
    chest(g);
    ASSERT("gold returns once without bag space or another score reward", g->gold == gold_before_recovery + 321 && g->jail_gold == 0 && g->score == score_before_recovery);
    ASSERT("a full bag restores empty worn slots and leaves other gear safely in the chest", g->jail_gear_claimed == 6 && g->jail_gear_state == 1 && g->equipped_off_hand == EQUIPMENT_WEAK_HAND && g->equipped_armor == EQUIPMENT_ARMOR);
    game_remove_inventory_item(g, MAX_INVENTORY - 1);
    chest(g);
    ASSERT("one free slot reclaims the original main weapon with its upgrades", g->jail_gear_claimed == (7 << 0) && g->jail_gear_state == 1 && !memcmp(&g->inventory[MAX_INVENTORY - 1], &original[0], sizeof(Item)));
    ASSERT("partial recovery and defeated guards survive an active-room save", save_game(g, GEAR_SLOT) && load_game(loaded, GEAR_SLOT) && loaded->jail_gear_claimed == g->jail_gear_claimed && !loaded->enemies[0].active);
    *g = *loaded;
    for (int attempt = 0; g->jail_gear_state == 1 && attempt < 7; attempt++) {
        game_remove_inventory_item(g, 0);
        chest(g);
    }
    ASSERT("recovering all confiscated items completes the equipment quest", g->jail_gear_state == 3 && g->jail_gear_claimed == 127);
    int exact = 0;
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < MAX_CARRIED_ITEMS; j++) {
            if (g->inventory[j].active && !memcmp(&original[i], &g->inventory[j], sizeof(Item))) {
                exact++;
                break;
            }
        }
    }
    ASSERT("all original belongings return with exact bonuses, rarity and sharpening", exact == 7);
    inventory = g->inventory_count;
    int recovered_gold = g->gold;
    chest(g);
    ASSERT("reopening the recovered chest cannot duplicate equipment", g->inventory_count == inventory && g->gold == recovered_gold && g->jail_gold == 0);
    int completed = 0;
    for (int i = 0; i < quest_journal_count(g, QUEST_TAB_COMPLETED); i++) {
        QuestJournalEntry entry;
        if (quest_journal_get_entry(g, QUEST_TAB_COMPLETED, i, &entry) && !strcmp(entry.title, "Recover Your Equipment")) {
            completed = entry.objective_complete[0];
        }
    }
    ASSERT("the completed journal records equipment recovery", completed);
    ASSERT("completed recovery survives reload without reopening the stash", save_game(g, GEAR_SLOT) && load_game(loaded, GEAR_SLOT) && loaded->jail_gear_state == 3 && loaded->jail_gear_claimed == 127);
    const PlayerClass classes[] = {CLASS_MAGE, CLASS_ROGUE};
    for (int i = 0; i < 2; i++) {
        g->player.player_class = classes[i];
        game_init(g);
        arrest(g);
        escape_cell(g);
        ASSERT("each class can fight through the escape after confiscation", g->location == LOCATION_ESCAPE_TUNNEL && g->equipped_main_hand >= 0 && g->inventory[g->equipped_main_hand].type == ITEM_WEAPON && g->player.arrows == 0 && (classes[i] != CLASS_ROGUE || g->jail_arrows == MAX_ARROWS));
    }
    test_currency_and_legacy_saves(g, loaded);
    remove("saves/savegame_99283.json");
    free(g);
    free(loaded);
}
