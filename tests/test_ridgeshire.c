#include "test_utils.h"
#include "game/ridgeshire.h"
#include "game/act5_routes.h"
#include "game/containers.h"
#include "game/aldric.h"
#include "game/garran.h"
#include "game/storm_mage.h"
#include "game/town_life.h"
#include "game/town_road.h"
#include "game/ironfang.h"
#include "game/red_dragon.h"
#include "game/royal_evidence.h"
#include "game/war_machine.h"
#include "screens/quest_journal.h"
#include "screens/shop.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define RIDGE_SLOT 99295
#define ROUTE_CELLS (MAP_W * MAP_H)

static int change_save(int version, const char *field, int value) {
    FILE *file = fopen("saves/savegame_99295.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = calloc((size_t)size + 1, 1);
    int ok = data && fread(data, 1, (size_t)size, file) == (size_t)size;
    fclose(file);
    cJSON *root = ok ? cJSON_Parse(data) : NULL;
    free(data);
    if (!root) {
        return 0;
    }
    if (version) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
        cJSON_DeleteItemFromObject(root, "ridgeshire");
    } else {
        cJSON *arc = cJSON_GetObjectItem(root, "ridgeshire");
        cJSON_ReplaceItemInObject(arc, field, cJSON_CreateNumber(value));
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99295.json", "wb");
    ok = data && file && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static int route_path(GameState *g, int tx, int ty) {
    int *previous = malloc(sizeof(int) * ROUTE_CELLS);
    int *queue = malloc(sizeof(int) * ROUTE_CELLS);
    if (!previous || !queue) {
        free(previous);
        free(queue);
        return 0;
    }
    for (int i = 0; i < ROUTE_CELLS; i++) {
        previous[i] = -1;
    }
    int start = g->player.y * MAP_W + g->player.x;
    int target = ty * MAP_W + tx;
    if (start == target) {
        return 1;
    }
    int head = 0;
    int tail = 1;
    previous[start] = start;
    queue[0] = start;
    const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail && previous[target] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + steps[side][0];
            int y = cell / MAP_W + steps[side][1];
            if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
                continue;
            }
            int next = y * MAP_W + x;
            TileType tile = g->map.tiles[y][x];
            int transition = (map_is_travel_tile(tile) && tile != TILE_ROAD_GATE_OPEN) || tile == TILE_SWAMP_ENTRANCE || tile == TILE_SWAMP_EXIT || tile == TILE_COAST_ENTRANCE || tile == TILE_COAST_EXIT || tile == TILE_ASHEN_ENTRANCE || tile == TILE_ASHEN_EXIT || tile == TILE_TOWN_EXIT || tile == TILE_FROST_ENTRANCE || tile == TILE_FROST_EXIT || tile == TILE_DRAGON_ENTRANCE || tile == TILE_DRAGON_EXIT || tile == TILE_IRON_ENTRANCE || tile == TILE_IRON_EXIT;
            int occupied = 0;
            for (int i = 0; i < g->enemy_count; i++) {
                occupied |= g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y;
            }
            int entrance = tile == TILE_NORTH_SWAMP || tile == TILE_NORTH_CATACOMBS;
            if (previous[next] >= 0 || occupied || tile == TILE_FROST_ICE || (!map_is_walkable(&g->map, x, y) && !(entrance && next == target)) || (transition && next != target)) {
                continue;
            }
            previous[next] = cell;
            queue[tail++] = next;
        }
    }
    int ok = previous[target] >= 0;
    int count = 0;
    if (ok) {
        for (int cell = target; cell != start; cell = previous[cell]) {
            queue[count++] = cell;
        }
        Location location = g->location;
        int level = g->level;
        int route = g->approach_route;
        for (int i = count - 1; i >= 0; i--) {
            int x = queue[i] % MAP_W;
            int y = queue[i] / MAP_W;
            action_resolve_player(g, (Action){ACTION_MOVE, x, y});
            if (location == LOCATION_ESCAPE_TUNNEL && g->location == location) {
                action_resolve_enemies(g);
            }
            if (g->location != location || g->level != level || g->approach_route != route) {
                ok = i == 0;
                break;
            }
            if (g->player.x != x || g->player.y != y) {
                ok = 0;
                break;
            }
        }
    }
    free(previous);
    free(queue);
    return ok;
}

static void clear_enemies(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
}

static void arrive(GameState *g) {
    game_init(g);
    game_enter_town_gate(g, LOCATION_TOWN4, 0, TOWN4_CROWN_GATE_Y);
}

static int enter_building(GameState *g, Location interior) {
    const TownBuilding *b = town_life_building(interior);
    return b && route_path(g, b->x + b->w / 2, b->y + b->h - 1) && g->location == interior;
}

static int talk_edric(GameState *g) {
    return route_path(g, RESIDENT_X, RESIDENT_Y + 1) && ridgeshire_talk(g) &&
        ridgeshire_handle_key(g, SDL_SCANCODE_1, 0);
}

static void offers_and_services(GameState *g, GameState *loaded) {
    game_init(g);
    game_enter_town_gate(g, LOCATION_CASTLE, CROWNROAD_X, TOWN_H - 1);
    ASSERT("the courier is reachable without accepting a quest", route_path(g, RENN_X - 1, RENN_Y) && ridgeshire_talk(g) && strstr(g->dialogue_text, "Crown Road East") && !game_active_quest_count(g));
    arrive(g);
    game_enter_dragonspine(g);
    g->enemies[0].hp = 7;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    game_return_to_approach(g);
    game_enter_town_gate(g, LOCATION_TOWN4, 0, TOWN4_CROWN_GATE_Y);
    ASSERT("Ridgeshire's new Tavern has a real walk-in doorway", enter_building(g, LOCATION_RIDGESHIRE_TAVERN));
    ASSERT("Ilya offers her hoard quest in Ridgeshire", route_path(g, RIDGE_ILYA_X, RIDGE_ILYA_Y + 1) && ridgeshire_talk(g) && game_quest_offer_active(g));
    ridgeshire_handle_key(g, SDL_SCANCODE_Y, 1);
    ASSERT("holding Y does not accept Ilya's offer", !g->dragon_treasure_quest_state);
    ridgeshire_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("the local hoard quest takes one active slot", g->dragon_treasure_quest_state == 1 && game_active_quest_count(g) == 1);
    ASSERT("accepting Ilya's quest preserves the existing Dragonspine expedition", g->dragonspine_cache[0].valid && g->dragonspine_cache[0].enemies[0].hp == 7);
    ASSERT("the new Tavern interior and quest offer survive saving", save_game(g, RIDGE_SLOT) && load_game(loaded, RIDGE_SLOT) && loaded->location == LOCATION_RIDGESHIRE_TAVERN && loaded->dragon_treasure_quest_state == 1);
    ASSERT("the Tavern's actual retreat leads back into town", route_path(g, 20, 22) && g->location == LOCATION_TOWN4);
    ASSERT("Edric's lodge has a working doorway", enter_building(g, LOCATION_MONASTERY) && talk_edric(g) && game_quest_offer_active(g));
    ridgeshire_handle_key(g, SDL_SCANCODE_N, 0);
    ASSERT("declining the binding quest leaves it available", !g->ridgeshire.dragon_quest && talk_edric(g));
    ridgeshire_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("the binding quest is independent of the hoard quest", g->ridgeshire.dragon_quest == 1 && g->dragon_treasure_quest_state == 1 && game_active_quest_count(g) == 2);
    ASSERT("the lodge's actual retreat returns to Ridgeshire", route_path(g, 20, 22) && g->location == LOCATION_TOWN4);
    ASSERT("the Town Hall doorway remains usable", route_path(g, TOWN4_HALL_DOOR_X, TOWN4_HALL_DOOR_Y) && g->location == LOCATION_TOWN_HALL);
    ASSERT("fresh characters are not offered the old Emberforge contract", g->map.tiles[HALL_STEWARD_Y][HALL_STEWARD_X] == TILE_TAVERN_FLOOR);
    ASSERT("Veyra offers Ironfang without the old Watchfires offer", route_path(g, HALL_VEYRA_X, HALL_VEYRA_Y + 1) && ridgeshire_talk(g) && game_quest_offer_active(g) && strstr(g->dialogue_text, "400 gold"));
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("the three local quests fill exactly the normal quest limit", game_active_quest_count(g) == 3 && !g->watchfire_quest_state && !g->catacombs_quest_state && !g->emberforge_quest_state);
    ASSERT("local quest states persist independently", save_game(g, RIDGE_SLOT) && load_game(loaded, RIDGE_SLOT) && loaded->ridgeshire.dragon_quest == 1 && loaded->war_machine_quest_state == 1 && loaded->dragon_treasure_quest_state == 1);
    ShopScreen shop;
    shop_init(&shop, SHOP_TYPE_RUNEWRIGHT, 15, CLASS_MAGE);
    int potions = 0;
    int spells = 0;
    for (int i = 0; i < shop.item_count; i++) {
        potions += shop.items[i].type == ITEM_POTION_HEALTH || shop.items[i].type == ITEM_POTION_MANA;
        spells += shop.items[i].type == ITEM_SCROLL || shop.items[i].type == ITEM_SPELL_TOME;
    }
    ASSERT("the runewright sells health and mana supplies plus spells", potions == 2 && spells >= 5);
    shop_init(&shop, SHOP_TYPE_FLETCHER, 15, CLASS_ROGUE);
    ASSERT("the existing Fletcher now provides bows as well as arrows", shop.item_count >= 4 && shop.items[0].type == ITEM_ARROWS && shop.items[1].weapon_family == WEAPON_FAMILY_BOW);
    arrive(g);
    enter_building(g, LOCATION_RIDGESHIRE_TAVERN);
    route_path(g, RESIDENT_X, RESIDENT_Y + 1);
    g->royal_reported_actions = 3;
    g->gold = 200;
    g->player.hp = 1;
    g->aldric_recruited = 1;
    g->aldric_party_state = ALDRIC_DEAD;
    g->aldric_hp = 0;
    g->storm_mage.recruited = 1;
    g->storm_mage.state = STORM_MAGE_FOLLOWING;
    g->storm_mage.hp = 1;
    g->storm_mage.mp = 0;
    g->garran.state = GARRAN_WAITING;
    g->garran.hp = 7;
    ridgeshire_talk(g);
    ASSERT("the Innkeeper acknowledges Crown reports without a new arrest", strstr(g->dialogue_text, "name reached") && g->royal_reported_actions == 3);
    ridgeshire_handle_key(g, SDL_SCANCODE_1, 0);
    ridgeshire_handle_key(g, SDL_SCANCODE_N, 0);
    ASSERT("declining rest spends no gold", g->gold == 200 && g->player.hp == 1);
    ridgeshire_talk(g);
    ridgeshire_handle_key(g, SDL_SCANCODE_1, 0);
    ridgeshire_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("paid rest heals living travelers without resurrecting Aldric", g->gold == 100 && g->player.hp == g->player.max_hp && g->aldric_party_state == ALDRIC_DEAD && !g->aldric_hp && g->storm_mage.hp == 70 && g->storm_mage.mp == 100);
    ASSERT("rest does not heal a companion waiting elsewhere", g->garran.hp == 7);
}

static void rail_sabotage(GameState *g) {
    ASSERT("the broken eastern rail has an accessible repair position", route_path(g, 39, 25));
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("the actual interaction repairs the eastern rail", g->map.tiles[25][40] == TILE_IRON_RAIL);
    IronfangStage *state = &g->ironfang[3];
    ASSERT("the actual minecart is reachable", route_path(g, state->cart_x, state->cart_y));
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("the player boards before crossing the flooded trestle", state->aboard);
    for (int i = 0; i < 40 && state->aboard && state->cart_x < 60; i++) {
        action_resolve_player(g, (Action){ACTION_MOVE, state->cart_x + 1, state->cart_y});
    }
    ASSERT("the cart follows real rails onto the isolated loading platform", state->cart_x == 60 && state->cart_y == 25 && state->aboard);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("the isolated platform allows safe disembarkation", !state->aboard && g->player.x >= 60);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("releasing the loading winch sabotages the weapons shipment", g->war_machine_progress & WAR_SABOTAGE);
    ASSERT("the cart's return landing remains reachable", route_path(g, state->cart_x, state->cart_y));
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    for (int i = 0; i < 40 && state->aboard && state->cart_x > 10; i++) {
        action_resolve_player(g, (Action){ACTION_MOVE, state->cart_x - 1, state->cart_y});
    }
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("sabotage preserves the real return journey across the trestle", state->cart_x == 10 && !state->aboard);
}

static void ironfang_adventure(GameState *g) {
    ironfang_enter(g, 0);
    for (int stage = 1; stage <= IRON_DEPTH; stage++) {
        for (int i = 0; i < g->enemy_count; i++) {
            if (g->enemies[i].type != ENEMY_GOBLIN_MINE_BOSS) {
                g->enemies[i].active = 0;
            }
        }
        if (stage == 2) {
            ASSERT("Malakar's actual side-office desk is reachable", route_path(g, WAR_ORDERS_X, WAR_ORDERS_Y + 1));
            action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
            ASSERT("reading the orders reveals evidence before accepting the quest", (g->war_machine_progress & WAR_ORDERS) && (royal_evidence_found(g) & ROYAL_EVIDENCE_REQUISITIONS));
        }
        if (stage == 4) {
            rail_sabotage(g);
        }
        if (stage == 5) {
            Enemy *boss = NULL;
            for (int i = 0; i < g->enemy_count; i++) {
                if (g->enemies[i].type == ENEMY_GOBLIN_MINE_BOSS) {
                    boss = &g->enemies[i];
                }
            }
            ASSERT("the generated Mine Boss can be approached", boss && route_path(g, boss->x, boss->y + 1));
            if (boss) {
                boss->hp = 1;
                g->player.attack = 1000;
                action_resolve_player(g, (Action){ACTION_MOVE, boss->x, boss->y});
            }
            ASSERT("a real boss kill records Ironfang's victory", g->ironfang_defeated);
            ASSERT("the command coffer is reachable after victory", route_path(g, WAR_SEAL_X, WAR_SEAL_Y + 1));
            action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
            ASSERT("all three discoveries complete Ironfang without requiring prior acceptance", g->war_machine_quest_state == 2 && g->war_machine_progress == 7);
        }
        ASSERT("quest interactions preserve every actual onward transition", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y));
    }
}

static void dragon_adventure(GameState *g, GameState *loaded, int armor) {
    game_enter_dragonspine(g);
    for (int stage = 1; stage <= DRAGONSPINE_DEPTH; stage++) {
        for (int i = 0; i < g->enemy_count; i++) {
            if (g->enemies[i].type != ENEMY_RED_DRAGON) {
                g->enemies[i].active = 0;
            }
        }
        if (stage < DRAGONSPINE_DEPTH) {
            ASSERT("the dragon investigation leaves every onward exit usable", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y));
            continue;
        }
        Enemy *boss = NULL;
        for (int i = 0; i < g->enemy_count; i++) {
            if (g->enemies[i].type == ENEMY_RED_DRAGON) {
                boss = &g->enemies[i];
            }
        }
        ASSERT("the generated Red Dragon has an approachable lair", boss && route_path(g, boss->x, boss->y + 1));
        if (boss) {
            boss->hp = 1;
            g->player.attack = 1000;
            action_resolve_player(g, (Action){ACTION_MOVE, boss->x, boss->y});
        }
        ASSERT("the dragon's actual defeat completes the binding objective and reveals its shackle", g->ridgeshire.dragon_quest == 2 && (royal_evidence_found(g) & ROYAL_EVIDENCE_BINDING) && g->ridgeshire.dragon_armor == RIDGE_ARMOR_PENDING);
        int cx = -1;
        int cy = -1;
        int gx = -1;
        int gy = -1;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                TileType tile = g->map.tiles[y][x];
                if (tile == TILE_DRAGON_RELIQUARY_FLOOR || tile == TILE_DRAGON_RELIQUARY_ASH || tile == TILE_DRAGON_RELIQUARY_HOARD) {
                    cx = x;
                    cy = y;
                }
                if (tile == TILE_DRAGON_TREASURE) {
                    gx = x;
                    gy = y;
                }
            }
        }
        ASSERT("the armor strongbox can be reached without an NPC quest", cx >= 0 && route_path(g, cx, cy));
        action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
        action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
        ASSERT("the hoard offers a choice rather than giving unwanted armor", g->ridgeshire.prompt == RIDGE_ARMOR && g->ridgeshire.dragon_armor == RIDGE_ARMOR_PENDING);
        ridgeshire_handle_key(g, SDL_SCANCODE_ESCAPE, 0);
        ASSERT("declining leaves the armor choice available", g->ridgeshire.dragon_armor == RIDGE_ARMOR_PENDING);
        action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
        int inventory = g->inventory_count;
        for (int i = inventory; i < MAX_INVENTORY; i++) {
            g->inventory[i] = item_make_health_potion();
        }
        g->inventory_count = MAX_INVENTORY;
        ridgeshire_handle_key(g, SDL_SCANCODE_1 + armor, 0);
        ASSERT("a full pack preserves the player's unchosen armor reward", g->red_dragon_reward_state == 1 && g->ridgeshire.dragon_armor == RIDGE_ARMOR_PENDING && g->inventory_count == MAX_INVENTORY);
        ASSERT("an open reliquary and full-pack armor choice survive save/load", save_game(g, RIDGE_SLOT) && load_game(loaded, RIDGE_SLOT) && loaded->ridgeshire.dragon_armor == RIDGE_ARMOR_PENDING && loaded->inventory_count == MAX_INVENTORY && containers_record(loaded, cx, cy));
        ASSERT("old unclaimed dragon rewards keep their original mantle", change_save(250, NULL, 0) && load_game(loaded, RIDGE_SLOT) && loaded->ridgeshire.legacy_dragon && !loaded->ridgeshire.dragon_armor);
        loaded->inventory_count = 0;
        loaded->dialogue_active = 0;
        action_resolve_player(loaded, (Action){ACTION_PICK_UP, 0, 0});
        ASSERT("a migrated pending reliquary still awards the legacy mantle", loaded->inventory_count == 1 && !strcmp(loaded->inventory[0].name, "Dragon Scale Mantle") && loaded->red_dragon_reward_state == 2);
        g->inventory_count = inventory;
        ridgeshire_handle_key(g, SDL_SCANCODE_1 + armor, 0);
        ASSERT("one selected armor is granted exactly once", g->inventory_count == inventory + 1 && g->ridgeshire.dragon_armor == RIDGE_DRAGONPLATE + armor && g->red_dragon_reward_state == 2);
        ridgeshire_handle_key(g, SDL_SCANCODE_1 + armor, 0);
        ASSERT("repeated keys cannot claim a second armor", g->inventory_count == inventory + 1);
        Item expected = item_make_dragonforged_armor(armor);
        ASSERT("each class's selected armor retains its bonuses after save/load", save_game(g, RIDGE_SLOT) && load_game(loaded, RIDGE_SLOT) && !strcmp(loaded->inventory[inventory].name, expected.name) && loaded->inventory[inventory].class_mask == expected.class_mask && loaded->inventory[inventory].max_hp_bonus == expected.max_hp_bonus && loaded->inventory[inventory].max_mp_bonus == expected.max_mp_bonus && loaded->inventory[inventory].evasion_chance == expected.evasion_chance && loaded->inventory[inventory].spell_cost_reduction_percent == expected.spell_cost_reduction_percent);
        ASSERT("the goblet exists before accepting Ilya's optional quest", gx >= 0 && route_path(g, gx, gy));
        action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
        ASSERT("finding the goblet completes its objective without occupying a bag slot", g->dragon_treasure_quest_state == 2 && g->inventory_count == inventory + 1);
        ASSERT("the summit's actual final exit remains usable after all discoveries", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_APPROACH);
    }
}

static void legacy_and_validation(GameState *g, GameState *loaded) {
    arrive(g);
    const TownBuilding *b = town_life_building(LOCATION_RIDGESHIRE_TAVERN);
    g->player.x = b->x + 1;
    g->player.y = b->y + 1;
    g->map.tiles[g->player.y][g->player.x] = TILE_TOWN_FLOOR;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = g->player.x, .y = g->player.y, .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
    g->map.tiles[g->player.y][g->player.x] = TILE_ITEM;
    g->player.hp = 17;
    g->gold = 432;
    g->watchfire_quest_state = 1;
    g->emberforge_quest_state = 1;
    g->war_machine_quest_state = 3;
    g->war_machine_progress = 7;
    g->ironfang_defeated = 1;
    g->defeated_bosses |= 1u << LOCATION_DRAGONSPINE;
    g->red_dragon_reward_state = 2;
    ASSERT("an old Ridgeshire exterior migrates without losing wounds or loot", save_game(g, RIDGE_SLOT) && change_save(250, NULL, 0) && load_game(loaded, RIDGE_SLOT) && loaded->player.x == b->x + b->w / 2 && loaded->player.y == b->y + b->h && loaded->player.hp == 17 && loaded->gold == 432 && loaded->floor_items[0].active && loaded->floor_items[0].x == loaded->player.x && loaded->floor_items[0].y == loaded->player.y);
    ASSERT("paid legacy rewards cannot be claimed a second time", loaded->ridgeshire.legacy_dragon && !loaded->ridgeshire.dragon_armor && loaded->ridgeshire.ring_reward == 2 && loaded->war_machine_quest_state == 3);
    ASSERT("accepted legacy contracts remain active after migration", loaded->watchfire_quest_state == 1 && loaded->emberforge_quest_state == 1);
    game_enter_town_hall(loaded);
    ASSERT("the steward remains available for a previously accepted contract", loaded->map.tiles[HALL_STEWARD_Y][HALL_STEWARD_X] == TILE_NPC_STEWARD);
    arrive(g);
    ASSERT("new save data rejects an invalid armor choice", save_game(g, RIDGE_SLOT) && change_save(0, "dragon_armor", 99) && !load_game(loaded, RIDGE_SLOT));
    ASSERT("new save data rejects an out-of-range quest state", save_game(g, RIDGE_SLOT) && change_save(0, "dragon_quest", 4) && !load_game(loaded, RIDGE_SLOT));
}

static void rewards_and_proof(GameState *g, GameState *loaded) {
    game_enter_town_hall(g);
    route_path(g, HALL_OSWIN_X, HALL_OSWIN_Y + 1);
    ridgeshire_talk(g);
    ASSERT("Oswin connects independent clues to the King's crown", g->ridgeshire.crown_record && strstr(g->dialogue_text, "King is being controlled") && (royal_evidence_found(g) & ROYAL_EVIDENCE_CROWN));
    ASSERT("all evidence survives saving without inventory items", save_game(g, RIDGE_SLOT) && load_game(loaded, RIDGE_SLOT) && loaded->ridgeshire.crown_record && loaded->war_machine_progress == 7 && (royal_evidence_found(loaded) & ROYAL_EVIDENCE_BINDING));
    route_path(g, HALL_VEYRA_X, HALL_VEYRA_Y + 1);
    int gold = g->gold;
    int count = g->inventory_count;
    for (int i = count; i < MAX_INVENTORY; i++) {
        g->inventory[i] = item_make_health_potion();
    }
    g->inventory_count = MAX_INVENTORY;
    ridgeshire_talk(g);
    ASSERT("Veyra pays 400 gold once and reserves a ring when the bag is full", g->gold == gold + 400 && g->war_machine_quest_state == 3 && g->ridgeshire.ring_reward == 1);
    int pending = 0;
    QuestJournalEntry entry;
    for (int i = 0; quest_journal_get_entry(g, QUEST_TAB_COMPLETED, i, &entry); i++) {
        if (!strcmp(entry.title, "Break the War Machine")) {
            pending = entry.state == 2 && strstr(entry.summary_line_2, "Gold and score paid");
        }
    }
    ASSERT("the journal keeps a full-pack ring marked as a pending reward", pending);
    g->inventory_count = count;
    ridgeshire_talk(g);
    ASSERT("a reserved Royal Ward Ring can be collected without another payment", g->gold == gold + 400 && g->inventory_count == count + 1 && g->ridgeshire.ring_reward == 2 && !strcmp(g->inventory[count].name, "Royal Ward Ring"));
    ridgeshire_talk(g);
    ASSERT("Veyra does not duplicate an earned ring", g->inventory_count == count + 1 && g->gold == gold + 400);
    town_life_generate(&g->map, LOCATION_MONASTERY, &g->player.x, &g->player.y);
    g->location = LOCATION_MONASTERY;
    talk_edric(g);
    talk_edric(g);
    ASSERT("Edric's examination pays 350 gold exactly once", g->gold == gold + 750 && g->ridgeshire.dragon_quest == 3);
    town_life_generate(&g->map, LOCATION_RIDGESHIRE_TAVERN, &g->player.x, &g->player.y);
    g->location = LOCATION_RIDGESHIRE_TAVERN;
    route_path(g, RIDGE_ILYA_X, RIDGE_ILYA_Y + 1);
    ridgeshire_talk(g);
    ridgeshire_talk(g);
    ASSERT("Ilya pays 300 gold locally exactly once", g->gold == gold + 1050 && g->dragon_treasure_quest_state == 3);
    ASSERT("all three completed quests free their active slots", !game_active_quest_count(g) && quest_journal_count(g, QUEST_TAB_COMPLETED) == 3);
}

static void partial_evidence(GameState *g, GameState *loaded) {
    game_enter_town_hall(g);
    ASSERT("Oswin examines either first clue without requiring the other adventure", route_path(g, HALL_OSWIN_X, HALL_OSWIN_Y + 1) && ridgeshire_talk(g) && g->ridgeshire.crown_record && strstr(g->dialogue_text, "We need both"));
    ASSERT("one clue raises suspicion without announcing the final conclusion", !strstr(royal_evidence_conclusion(g), "controls the King"));
    ASSERT("the crown inventory survives saving before the second clue", save_game(g, RIDGE_SLOT) && load_game(loaded, RIDGE_SLOT) && loaded->ridgeshire.crown_record && !strstr(royal_evidence_conclusion(loaded), "controls the King"));
}

void test_ridgeshire(void) {
    printf("Ridgeshire local story arc:\n");
    ASSERT("Ridgeshire fixture slot is unused", !save_exists(RIDGE_SLOT));
    if (save_exists(RIDGE_SLOT)) {
        return;
    }
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("Ridgeshire fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    offers_and_services(g, loaded);
    for (int order = 0; order < 3; order++) {
        arrive(g);
        srand(19 + order * 97);
        g->player.hp = g->player.max_hp = 10000;
        g->player.defense = 1000;
        if (order) {
            dragon_adventure(g, loaded, order == 1 ? 2 : 1);
            partial_evidence(g, loaded);
            ironfang_adventure(g);
        } else {
            ironfang_adventure(g);
            partial_evidence(g, loaded);
            dragon_adventure(g, loaded, 0);
        }
        rewards_and_proof(g, loaded);
    }
    legacy_and_validation(g, loaded);
    remove("saves/savegame_99295.json");
    free(g);
    free(loaded);
}

