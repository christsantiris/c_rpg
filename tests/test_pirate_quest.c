#include "test_utils.h"
#include "game/pirate.h"
#include "game/town_life.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define QUEST_SLOT 99286
#define QUEST_PATH "saves/savegame_99286.json"

static int edit_save(int version, double state) {
    FILE *file = fopen(QUEST_PATH, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *json = malloc(size + 1);
    if (!json) {
        fclose(file);
        return 0;
    }
    size_t length = fread(json, 1, size, file);
    json[length] = 0;
    fclose(file);
    cJSON *root = cJSON_Parse(json);
    free(json);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    cJSON_DeleteItemFromObject(root, "pirate_quest_state");
    if (state >= 0) {
        cJSON_AddNumberToObject(root, "pirate_quest_state", state);
    }
    json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(QUEST_PATH, "wb");
    int ok = file && json && fputs(json, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(json);
    return ok;
}

static void approach_corvin(GameState *g) {
    g->player.x = RESIDENT_X;
    g->player.y = RESIDENT_Y + 1;
}

void test_pirate_quest(void) {
    printf("Dawnmere merchant treasure quest:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("merchant quest fixtures allocate without overwriting player saves", g && loaded && !save_exists(QUEST_SLOT));
    if (!g || !loaded || save_exists(QUEST_SLOT)) {
        free(g);
        free(loaded);
        return;
    }
    game_init(g);
    pirate_enter(g);
    pirate_change_level(g, 1);
    pirate_change_level(g, 1);
    unsigned int seed = g->pirate_cache[2].seed;
    g->enemies[0].hp = 7;
    g->player.x = PIRATE_QUEST_X + 1;
    g->player.y = PIRATE_QUEST_Y;
    int gold = g->gold;
    ASSERT("the silver coffer has a walkable approach and requires Corvin's map", map_is_walkable(&g->map, g->player.x, g->player.y) &&
        pirate_interact(g) && !g->pirate_quest_state && g->gold == gold && !g->pirate_cache[2].treasure_claimed);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 20, TOWN_H - 1);
    const TownBuilding *merchant = town_life_building(LOCATION_DAWNMERE);
    ASSERT("Dawnmere's merchant has a real doorway connected to the town path", merchant && merchant->interior == LOCATION_MERCHANT &&
        g->map.tiles[10][9] == TILE_LOCAL_DOOR && map_is_walkable(&g->map, 9, 11));
    g->player.x = 9;
    g->player.y = 11;
    action_resolve_player(g, (Action){ACTION_MOVE, 9, 10});
    ASSERT("walking into the merchant enters a safe shop without starting dialogue", g->location == LOCATION_MERCHANT && !g->enemy_count &&
        !g->dialogue_active && g->map.tiles[RESIDENT_Y][RESIDENT_X] == TILE_NPC_RESIDENT && !town_life_talk(g, 0));
    approach_corvin(g);
    ASSERT("talking offers the map quest and its full bounty before it is assigned", town_life_talk(g, 0) && game_quest_offer_active(g) &&
        !g->pirate_quest_state && strstr(g->dialogue_text, "100 gold") && strstr(g->dialogue_text, "Mariner's Cloak"));
    ASSERT("an unanswered merchant offer survives saving inside the shop", save_game(g, QUEST_SLOT) && load_game(loaded, QUEST_SLOT) &&
        loaded->location == LOCATION_MERCHANT && game_quest_offer_active(loaded));
    *g = *loaded;
    game_handle_quest_offer_key(g, SDL_SCANCODE_N, 0);
    ASSERT("declining the map leaves the quest unassigned", !g->dialogue_active && !g->pirate_quest_state);
    town_life_talk(g, 0);
    int inventory = g->inventory_count;
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    QuestJournalEntry entry;
    ASSERT("accepting gifts the map in the journal without consuming an inventory slot", g->pirate_quest_state == 1 &&
        g->inventory_count == inventory && quest_journal_get_entry(g, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "The Merchant's Lost Fortune") == 0 && entry.stages[0] == 3 && entry.reward_gold == 100 &&
        strcmp(entry.reward_item, "Mariner's Cloak") == 0 && !entry.objective_complete[0]);
    ASSERT("accepting Corvin's map itself preserves the cached expedition", g->pirate_cache[2].seed == seed &&
        g->pirate_cache[2].enemies[0].hp == 7);
    g->player.x = 20;
    g->player.y = 21;
    action_resolve_player(g, (Action){ACTION_MOVE, 20, 22});
    ASSERT("exiting the shop returns beside its doorway in Dawnmere", g->location == LOCATION_DAWNMERE && g->player.x == 9 &&
        g->player.y == 11 && g->last_town_visited == LOCATION_DAWNMERE);
    ASSERT("accepted map state survives a save in town", save_game(g, QUEST_SLOT) && load_game(loaded, QUEST_SLOT) && loaded->pirate_quest_state == 1);
    *g = *loaded;
    pirate_enter(g);
    pirate_change_level(g, 1);
    pirate_change_level(g, 1);
    int crew_hp = g->enemies[0].max_hp;
    ASSERT("walking back into Pirate Coast restores its crew while preserving terrain and the gifted map", g->pirate_cache[2].seed == seed &&
        g->enemies[0].hp == crew_hp && g->pirate_quest_state == 1 && g->map.tiles[PIRATE_QUEST_Y][PIRATE_QUEST_X] == TILE_PIRATE_QUEST_CHEST);
    g->inventory_count = MAX_INVENTORY;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        g->inventory[i] = item_make_health_potion();
    }
    g->player.x = PIRATE_QUEST_X + 1;
    g->player.y = PIRATE_QUEST_Y;
    ASSERT("the map coffer can be recovered with a full pack without claiming ordinary plunder", pirate_interact(g) && g->pirate_quest_state == 2 &&
        g->inventory_count == MAX_INVENTORY && g->gold == gold && !g->pirate_cache[2].treasure_claimed &&
        g->map.tiles[PIRATE_QUEST_Y][PIRATE_QUEST_X] == TILE_PIRATE_DECK);
    pirate_interact(g);
    ASSERT("the recovered coffer cannot be claimed again", g->pirate_quest_state == 2 && g->gold == gold);
    g->player.x = PIRATE_QUEST_X;
    g->player.y = PIRATE_QUEST_Y;
    action_resolve_player(g, (Action){ACTION_DROP_ITEM, 0, 0});
    ASSERT("recovered treasure and dropped loot survive an active-stage save", save_game(g, QUEST_SLOT) && load_game(loaded, QUEST_SLOT) &&
        loaded->pirate_quest_state == 2 && loaded->map.tiles[PIRATE_QUEST_Y][PIRATE_QUEST_X] == TILE_ITEM &&
        loaded->floor_items[0].underlying_tile == TILE_PIRATE_DECK);
    *g = *loaded;
    pirate_change_level(g, 1);
    pirate_change_level(g, 0);
    ASSERT("backtracking retains the recovered coffer and the loot underlay", g->pirate_quest_state == 2 &&
        g->map.tiles[PIRATE_QUEST_Y][PIRATE_QUEST_X] == TILE_ITEM && g->floor_items[0].underlying_tile == TILE_PIRATE_DECK);
    game_open_town_portal(g);
    ASSERT("return to town reaches Dawnmere with the recovered treasure", g->location == LOCATION_DAWNMERE && g->portal_active && g->pirate_quest_state == 2);
    g->player.x = 9;
    g->player.y = 11;
    action_resolve_player(g, (Action){ACTION_MOVE, 9, 10});
    approach_corvin(g);
    g->inventory[g->inventory_count++] = item_make_health_potion();
    town_life_talk(g, 0);
    ASSERT("Corvin holds both rewards while the pack is full", g->pirate_quest_state == 2 && g->gold == gold &&
        g->inventory_count == MAX_INVENTORY && strstr(g->dialogue_text, "Make room") != NULL);
    ASSERT("the pending bounty survives saving inside the shop", save_game(g, QUEST_SLOT) && load_game(loaded, QUEST_SLOT) &&
        loaded->location == LOCATION_MERCHANT && loaded->pirate_quest_state == 2 && loaded->gold == gold);
    *g = *loaded;
    g->inventory_count--;
    int score = g->score;
    town_life_talk(g, 0);
    Item *cloak = &g->inventory[g->inventory_count - 1];
    ASSERT("returning the coffer awards exactly 100 gold and the promised equipment", g->pirate_quest_state == 3 && g->gold == gold + 100 &&
        g->score == score && strcmp(cloak->name, "Mariner's Cloak") == 0 && cloak->defense_bonus == 3 && cloak->evasion_chance == 5);
    ASSERT("every class can equip the Mariner's Cloak", item_class_allowed(cloak, CLASS_WARRIOR) && item_class_allowed(cloak, CLASS_MAGE) &&
        item_class_allowed(cloak, CLASS_ROGUE));
    ASSERT("the completed quest moves to the journal's completed tab", !quest_journal_count(g, QUEST_TAB_ACTIVE) &&
        quest_journal_get_entry(g, QUEST_TAB_COMPLETED, 0, &entry) && entry.objective_complete[0] && entry.state == 3);
    ASSERT("completed bounty and equipment metadata survive save and load", save_game(g, QUEST_SLOT) && load_game(loaded, QUEST_SLOT) &&
        loaded->pirate_quest_state == 3 && loaded->gold == gold + 100 && loaded->inventory[loaded->inventory_count - 1].evasion_chance == 5 &&
        loaded->inventory[loaded->inventory_count - 1].class_mask == ITEM_CLASS_ALL);
    *g = *loaded;
    town_life_talk(g, 0);
    ASSERT("repeat conversations after reload never duplicate the bounty", g->gold == gold + 100 && g->inventory_count == MAX_INVENTORY);
    g->player.x = 20;
    g->player.y = 21;
    action_resolve_player(g, (Action){ACTION_MOVE, 20, 22});
    game_use_town_portal(g);
    ASSERT("the portal restores the expedition without respawning its quest treasure", g->location == LOCATION_PIRATE_COAST &&
        g->level == 3 && g->pirate_quest_state == 3 && g->enemies[0].hp == crew_hp && g->floor_items[0].underlying_tile == TILE_PIRATE_DECK);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 20, 12);
    g->map.tiles[10][9] = TILE_LOCAL_BUILDING;
    ASSERT("old Dawnmere saves gain the shop doorway while retaining gold and expedition progress", save_game(g, QUEST_SLOT) &&
        edit_save(185, -1) && load_game(loaded, QUEST_SLOT) && !loaded->pirate_quest_state && loaded->map.tiles[10][9] == TILE_LOCAL_DOOR &&
        loaded->gold == gold + 100 && loaded->pirate_cache[2].seed == seed && loaded->pirate_cache[2].enemies[0].hp == crew_hp);
    ASSERT("new saves reject missing quest state rather than resetting progress", save_game(g, QUEST_SLOT) && edit_save(186, -1) && !load_game(loaded, QUEST_SLOT));
    ASSERT("new saves reject invalid quest-state values", save_game(g, QUEST_SLOT) && edit_save(186, 4) && !load_game(loaded, QUEST_SLOT) &&
        save_game(g, QUEST_SLOT) && edit_save(186, 1.5) && !load_game(loaded, QUEST_SLOT));
    remove(QUEST_PATH);
    free(g);
    free(loaded);
}
