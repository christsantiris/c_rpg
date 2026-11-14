#include "test_utils.h"
#include "game/rosemoor.h"
#include "game/ridgeshire.h"
#include "game/approach.h"
#include "game/town_life.h"
#include "game/town_road.h"
#include "game/containers.h"
#include "game/royal_evidence.h"
#include "screens/quest_journal.h"
#include "screens/shop.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define ROSE_SLOT 99296
#define ROUTE_CELLS (MAP_W * MAP_H)

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

static void arrive(GameState *g) {
    game_init(g);
    game_enter_town_gate(g, LOCATION_TOWN3, TOWN_W - 1, TOWN3_CROWN_GATE_Y);
}

static int enter_mira(GameState *g) {
    return route_path(g, TOWN_APOTHECARY_DOOR_X, TOWN_APOTHECARY_DOOR_Y) && g->location == LOCATION_ROSEMOOR_APOTHECARY &&
        route_path(g, RESIDENT_X, RESIDENT_Y + 1) && rosemoor_talk(g);
}

static int enter_guild(GameState *g) {
    return route_path(g, TOWN_GUILD_DOOR_X, TOWN_GUILD_DOOR_Y) && g->location == LOCATION_GUILD;
}

static int talk_to(GameState *g, int x, int y) {
    return route_path(g, x, y + 1) && rosemoor_talk(g);
}

static void fill_pack(GameState *g) {
    for (int i = g->inventory_count; i < MAX_INVENTORY; i++) {
        g->inventory[i] = item_make_health_potion();
    }
    g->inventory_count = MAX_INVENTORY;
}

static int change_save(int version, const char *field, double value) {
    FILE *f = fopen("saves/savegame_99296.json", "rb");
    if (!f) {
        return 0;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *data = calloc((size_t)size + 1, 1);
    int ok = data && fread(data, 1, (size_t)size, f) == (size_t)size;
    fclose(f);
    cJSON *root = ok ? cJSON_Parse(data) : NULL;
    free(data);
    if (!root) {
        return 0;
    }
    if (version) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
        cJSON_DeleteItemFromObject(root, "rosemoor");
        if (!cJSON_GetObjectItem(root, "hunt_quest_state")->valueint) {
            cJSON_SetNumberValue(cJSON_GetObjectItem(root, "hunt_quest_progress"), 0);
            cJSON_SetNumberValue(cJSON_GetObjectItem(root, "hunt_quest_encounters"), 0);
        }
    } else {
        cJSON_ReplaceItemInObject(cJSON_GetObjectItem(root, "rosemoor"), field, cJSON_CreateNumber(value));
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    f = fopen("saves/savegame_99296.json", "wb");
    ok = f && data && fputs(data, f) >= 0;
    if (f) {
        fclose(f);
    }
    free(data);
    return ok;
}

static void offers_and_trade(GameState *g, GameState *loaded) {
    arrive(g);
    ASSERT("Mira's actual door enters the Apothecary without automatically trading", enter_mira(g) && g->rosemoor.prompt == ROSE_MIRA_MENU);
    ASSERT("Talk offers trading without assigning a quest", rosemoor_open_shop(g, SDL_SCANCODE_1) && !game_quest_offer_active(g) && !game_active_quest_count(g));
    ShopScreen shop;
    shop_init(&shop, SHOP_TYPE_APOTHECARY, g->defeated_bosses, g->player.player_class);
    ASSERT("Mira retains the Apothecary's potion stock", shop.item_count == 4 && shop.items[0].type == ITEM_POTION_HEALTH && shop.items[1].type == ITEM_POTION_MANA);
    rosemoor_handle_key(g, SDL_SCANCODE_2, 0);
    ASSERT("only the Frostfell topic opens the quest offer", game_quest_offer_active(g) && !g->rosemoor.frost_quest);
    rosemoor_handle_key(g, SDL_SCANCODE_Y, 1);
    ASSERT("holding Y cannot accept the quest", !g->rosemoor.frost_quest);
    rosemoor_handle_key(g, SDL_SCANCODE_N, 0);
    ASSERT("declining leaves Mira's quest available", !g->rosemoor.frost_quest && rosemoor_talk(g));
    rosemoor_handle_key(g, SDL_SCANCODE_2, 0);
    rosemoor_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("explicit acceptance assigns The Broken Ward", g->rosemoor.frost_quest == 1 && game_active_quest_count(g) == 1);
    ASSERT("Mira's interior, dialogue and quest survive saving", save_game(g, ROSE_SLOT) && load_game(loaded, ROSE_SLOT) && loaded->location == LOCATION_ROSEMOOR_APOTHECARY && loaded->rosemoor.frost_quest == 1 && rosemoor_dialogue(loaded));
    ASSERT("the Apothecary's real exit returns to its doorstep facing south", route_path(g, 20, 22) && g->location == LOCATION_TOWN3 && g->player.x == TOWN_APOTHECARY_DOOR_X && g->player.y == TOWN_APOTHECARY_DOOR_Y + 1 && g->player.last_dy == 1);
    ASSERT("the Guild has a usable entry and Hadrin is reachable", enter_guild(g) && talk_to(g, ROSE_HADRIN_X, ROSE_HADRIN_Y) && game_quest_offer_active(g));
    rosemoor_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("Selene offers the local hunt without sending the player to old regions", talk_to(g, GUILD_SELENE_X, GUILD_SELENE_Y) && game_quest_offer_active(g) && strstr(g->dialogue_text, "Frostfell 3") && !strstr(g->dialogue_text, "Sunscar"));
    rosemoor_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("the three Rosemoor contracts fill the normal three active slots", game_active_quest_count(g) == 3 && !g->hunt_quest_state && !g->emberforge_quest_state && !g->frostfell_quest_state);
    ASSERT("all three new contracts persist independently", save_game(g, ROSE_SLOT) && load_game(loaded, ROSE_SLOT) && loaded->rosemoor.frost_quest == 1 && loaded->rosemoor.forge_quest == 1 && loaded->rosemoor.hunt_quest == 1);
    g->defeated_bosses |= 1u << LOCATION_FROSTFELL;
    rosemoor_sync(g);
    int gold = g->gold;
    ASSERT("a boss victory immediately frees its active quest slot", g->rosemoor.frost_quest == 2 && game_active_quest_count(g) == 2);
    ASSERT("the Guild retreat remains connected", route_path(g, 20, 22) && g->location == LOCATION_TOWN3);
    ASSERT("completed objectives do not force payment when opening Mira's trade menu", enter_mira(g) && g->gold == gold && rosemoor_open_shop(g, SDL_SCANCODE_1) && strstr(rosemoor_hint(g), "Claim Frostfell"));
    rosemoor_handle_key(g, SDL_SCANCODE_2, 0);
    ASSERT("Mira pays 350 gold only when the reward topic is chosen", g->gold == gold + 350 && g->rosemoor.frost_quest == 3);
    rosemoor_talk(g);
    rosemoor_handle_key(g, SDL_SCANCODE_2, 0);
    ASSERT("Mira's payment cannot repeat", g->gold == gold + 350);
    arrive(g);
    game_enter_inn(g);
    rosemoor_prepare(g);
    ASSERT("fresh games no longer offer Brenna's rescue quest", g->map.tiles[BRENNA_INN_Y][BRENNA_INN_X] != TILE_NPC_BRENNA);
}

static int kill_enemy(GameState *g, Enemy *e) {
    if (!e || !e->active) {
        return 0;
    }
    const int dirs[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    for (int i = 0; i < 4; i++) {
        if (route_path(g, e->x + dirs[i][0], e->y + dirs[i][1])) {
            e->hp = 1;
            g->player.attack = 1000;
            action_resolve_player(g, (Action){ACTION_MOVE, e->x, e->y});
            return !e->active;
        }
    }
    return 0;
}

static void hunt_pack(GameState *g, GameState *loaded) {
    int pack = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (game_hunt_enemy_index(&g->enemies[i]) == 0 && g->enemies[i].active) {
            pack++;
        }
    }
    ASSERT("Rimefang's whole pack can be discovered before accepting Selene's quest", pack == 4 && !g->hunt_quest_state && !g->rosemoor.hunt_quest);
    int killed = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->active && game_hunt_enemy_index(e) == 0) {
            ASSERT("each hunt enemy has an actual approach and dies through combat", kill_enemy(g, e));
            killed++;
            if (killed == 1) {
                ASSERT("defeating only the leader does not finish the pack hunt", g->rosemoor.hunt_quest < 2);
                int wounded = -1;
                for (int j = 0; j < g->enemy_count; j++) {
                    if (g->enemies[j].active && game_hunt_enemy_index(&g->enemies[j]) == 0) {
                        wounded = j;
                        g->enemies[j].hp -= 3;
                        break;
                    }
                }
                ASSERT("a partially defeated pack and wounds persist on save/load", wounded >= 0 && save_game(g, ROSE_SLOT) && load_game(loaded, ROSE_SLOT) && loaded->enemy_count == g->enemy_count && !loaded->enemies[i].active && loaded->enemies[wounded].hp == g->enemies[wounded].hp && loaded->rosemoor.hunt_quest < 2);
            }
        }
    }
    ASSERT("the entire pack completes its quest and reveals the binding collar", killed == 4 && g->rosemoor.hunt_quest == 2 && (royal_evidence_found(g) & ROYAL_EVIDENCE_RIMEFANG));
    ASSERT("even the optional collar shifts suspicion toward Malakar", strstr(royal_evidence_conclusion(g), "Malakar"));
    ASSERT("a completed unaccepted hunt survives saving", save_game(g, ROSE_SLOT) && load_game(loaded, ROSE_SLOT) && loaded->rosemoor.hunt_quest == 2 && !game_active_quest_count(loaded));
    ASSERT("the new encounter preserves the stage's real retreat", route_path(g, g->map.stairs_up_x, g->map.stairs_up_y) && g->level == 2);
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
    const int dirs[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    for (int i = 0; i < 4; i++) {
        int x = g->player.x + dirs[i][0];
        int y = g->player.y + dirs[i][1];
        if (map_is_walkable(&g->map, x, y) && !map_is_travel_tile(g->map.tiles[y][x])) {
            action_resolve_player(g, (Action){ACTION_MOVE, x, y});
            break;
        }
    }
    ASSERT("retreating still leaves a real route back to the hunt stage", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->level == 3);
}

static void frost_adventure(GameState *g, GameState *loaded, int choice) {
    game_enter_frostfell(g);
    for (int stage = 1; stage <= FROSTFELL_DEPTH; stage++) {
        Enemy *boss = NULL;
        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *e = &g->enemies[i];
            if (e->type == ENEMY_FROST_KRAKEN) {
                boss = e;
            } else if (game_hunt_enemy_index(e) != 0) {
                e->active = 0;
            }
        }
        if (stage == 3) {
            hunt_pack(g, loaded);
        }
        if (stage == FROSTFELL_DEPTH) {
            ASSERT("the generated Kraken can be reached and defeated without any quest", kill_enemy(g, boss));
            ASSERT("the boss completes The Broken Ward and releases its new weapon choice", g->rosemoor.frost_quest == 2 && g->rosemoor.weapon == ROSE_WEAPON_PENDING && (royal_evidence_found(g) & ROYAL_EVIDENCE_WARD_ORDERS));
            int x = -1;
            int y = -1;
            for (int ty = 0; ty < MAP_H; ty++) {
                for (int tx = 0; tx < MAP_W; tx++) {
                    if (g->map.tiles[ty][tx] == TILE_KRAKEN_RELIQUARY || g->map.tiles[ty][tx] == TILE_KRAKEN_LAKE_RELIQUARY) {
                        x = tx;
                        y = ty;
                    }
                }
            }
            ASSERT("the Kraken reliquary is visibly placed on reachable terrain", x >= 0 && route_path(g, x, y));
            action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
            action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
            ASSERT("opening and picking up the reliquary presents a class weapon choice", rosemoor_dialogue(g) && g->rosemoor.prompt == ROSE_WEAPON);
            rosemoor_handle_key(g, SDL_SCANCODE_ESCAPE, 0);
            ASSERT("deferring keeps the weapon reserved", g->rosemoor.weapon == ROSE_WEAPON_PENDING);
            action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
            int count = g->inventory_count;
            fill_pack(g);
            rosemoor_handle_key(g, SDL_SCANCODE_1 + choice, 0);
            ASSERT("a full pack preserves the unclaimed weapon", g->rosemoor.weapon == ROSE_WEAPON_PENDING && g->inventory_count == MAX_INVENTORY);
            ASSERT("pending weapon choices survive saving with an open chest", save_game(g, ROSE_SLOT) && load_game(loaded, ROSE_SLOT) && loaded->rosemoor.weapon == ROSE_WEAPON_PENDING && loaded->inventory_count == MAX_INVENTORY && containers_record(loaded, x, y));
            ASSERT("the reliquary preserves its frozen lake terrain through save/load", loaded->map.tiles[y][x] == TILE_KRAKEN_LAKE_RELIQUARY);
            g->inventory_count = count;
            rosemoor_handle_key(g, SDL_SCANCODE_1 + choice, 0);
            ASSERT("exactly one weapon is awarded", g->inventory_count == count + 1 && g->rosemoor.weapon == ROSE_BLADE + choice && !g->kraken_bow_unclaimed);
            action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
            rosemoor_handle_key(g, SDL_SCANCODE_1 + choice, 0);
            ASSERT("repeated pickup and keys cannot award a second weapon", g->inventory_count == count + 1);
            ASSERT("the selected weapon and its class restrictions persist", save_game(g, ROSE_SLOT) && load_game(loaded, ROSE_SLOT) && loaded->inventory[count].class_mask == g->inventory[count].class_mask && !strcmp(loaded->inventory[count].name, g->inventory[count].name));
        }
        ASSERT("story additions preserve every actual Frostfell onward transition", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y));
    }
    ASSERT("the final Frostfell exit returns to Rosemoor's approach", g->location == LOCATION_APPROACH && approach_route(g)->town == LOCATION_TOWN3);
}

static void ashen_adventure(GameState *g) {
    game_enter_ashen(g);
    for (int stage = 1; stage <= ASHEN_DEPTH; stage++) {
        Enemy *boss = NULL;
        for (int i = 0; i < g->enemy_count; i++) {
            if (g->enemies[i].type == ENEMY_CINDER_LORD) {
                boss = &g->enemies[i];
            } else {
                g->enemies[i].active = 0;
            }
        }
        if (stage == ASHEN_DEPTH) {
            ASSERT("the generated Cinder Lord can be fought without accepting the forge quest", kill_enemy(g, boss));
            ASSERT("Cinder victory completes the objective and records Malakar's production ledger", g->rosemoor.forge_quest == 2 && (royal_evidence_found(g) & ROYAL_EVIDENCE_FORGE_LEDGER));
            ASSERT("the original boss equipment and potion remain separate", g->cinder_lord_reward_pending == 3);
        }
        ASSERT("the forge story preserves every actual Ashen onward transition", route_path(g, g->map.stairs_down_x, g->map.stairs_down_y));
    }
}

static void rewards(GameState *g, GameState *loaded, int potion) {
    game_enter_town_gate(g, LOCATION_TOWN3, TOWN_W - 1, TOWN3_CROWN_GATE_Y);
    int gold = g->gold;
    ASSERT("Mira's reward stays in Rosemoor and requires choosing its topic", enter_mira(g) && g->gold == gold);
    rosemoor_handle_key(g, SDL_SCANCODE_2, 0);
    ASSERT("Mira pays once and explains Malakar's planned crisis", g->gold == gold + 350 && strstr(g->dialogue_text, "deliberately"));
    ASSERT("Mira's ward and the forge together identify the manufactured crisis", strstr(royal_evidence_conclusion(g), "manufactured"));
    ASSERT("all three discoveries persist as journal evidence", save_game(g, ROSE_SLOT) && load_game(loaded, ROSE_SLOT) && (royal_evidence_found(loaded) & (ROYAL_EVIDENCE_WARD_ORDERS | ROYAL_EVIDENCE_FORGE_LEDGER | ROYAL_EVIDENCE_RIMEFANG)) == (ROYAL_EVIDENCE_WARD_ORDERS | ROYAL_EVIDENCE_FORGE_LEDGER | ROYAL_EVIDENCE_RIMEFANG));
    route_path(g, 20, 22);
    ASSERT("Hadrin's local reward can be reached through the real Guild door", enter_guild(g) && route_path(g, ROSE_HADRIN_X, ROSE_HADRIN_Y + 1));
    int count = g->inventory_count;
    fill_pack(g);
    rosemoor_talk(g);
    ASSERT("Hadrin pays gold and safely holds full-pack bracers", g->gold == gold + 750 && g->rosemoor.bracers == 1 && g->rosemoor.forge_quest == 3);
    ASSERT("the reserved bracers survive saving", save_game(g, ROSE_SLOT) && load_game(loaded, ROSE_SLOT) && loaded->rosemoor.bracers == 1 && loaded->gold == g->gold);
    g->inventory_count = count;
    rosemoor_talk(g);
    rosemoor_talk(g);
    ASSERT("reserved bracers are handed over once without repeating gold", g->inventory_count == count + 1 && !strcmp(g->inventory[count].name, "Emberforge Bracers") && g->gold == gold + 750 && g->rosemoor.bracers == 2);
    ASSERT("Selene pays her local hunt and offers a potion choice", talk_to(g, GUILD_SELENE_X, GUILD_SELENE_Y) && g->gold == gold + 950 && g->rosemoor.potion == 1 && g->rosemoor.prompt == ROSE_POTION);
    count = g->inventory_count;
    fill_pack(g);
    rosemoor_handle_key(g, SDL_SCANCODE_1 + potion, 0);
    ASSERT("Selene holds a potion when the pack is full", g->rosemoor.potion == 1);
    ASSERT("the potion choice survives save/load", save_game(g, ROSE_SLOT) && load_game(loaded, ROSE_SLOT) && loaded->rosemoor.prompt == ROSE_POTION && loaded->rosemoor.potion == 1);
    g->inventory_count = count;
    rosemoor_handle_key(g, SDL_SCANCODE_1 + potion, 0);
    rosemoor_talk(g);
    rosemoor_handle_key(g, SDL_SCANCODE_1 + potion, 0);
    ASSERT("only the selected potion is awarded with no repeated gold", g->inventory_count == count + 1 && g->rosemoor.potion == 2 && g->gold == gold + 950 && g->inventory[count].type == (potion ? ITEM_POTION_INTELLIGENCE : ITEM_POTION_STRENGTH));
    ASSERT("all three objectives sit in Completed and use no active slots", !game_active_quest_count(g) && quest_journal_count(g, QUEST_TAB_COMPLETED) == 3);
}

static void legacy_saves(GameState *g, GameState *loaded) {
    arrive(g);
    g->defeated_bosses |= 1u << LOCATION_FROSTFELL;
    g->kraken_bow_unclaimed = 1;
    g->player.hp = 17;
    g->gold = 432;
    g->frostfell_quest_state = 1;
    g->emberforge_quest_state = 1;
    g->hunt_quest_state = 1;
    ASSERT("an existing Rosemoor save migrates without losing wounds, gold or contracts", save_game(g, ROSE_SLOT) && change_save(251, NULL, 0) && load_game(loaded, ROSE_SLOT) && loaded->player.hp == 17 && loaded->gold == 432 && loaded->frostfell_quest_state == 1 && loaded->emberforge_quest_state == 1 && loaded->hunt_quest_state == 1);
    ASSERT("old Kraken victories retain their legacy reward without a new choice", loaded->rosemoor.legacy_kraken && loaded->kraken_bow_unclaimed && !loaded->rosemoor.weapon && loaded->map.tiles[TOWN_APOTHECARY_DOOR_Y][TOWN_APOTHECARY_DOOR_X] == TILE_LOCAL_DOOR);
    game_enter_inn(loaded);
    rosemoor_prepare(loaded);
    ASSERT("Brenna stays available for an accepted legacy rescue contract", loaded->map.tiles[BRENNA_INN_Y][BRENNA_INN_X] == TILE_NPC_BRENNA);
    game_enter_guild(loaded);
    ASSERT("Selene advertises accepted legacy contracts beside her new quest", talk_to(loaded, GUILD_SELENE_X, GUILD_SELENE_Y) && strstr(rosemoor_hint(loaded), "Previous Great Hunts"));
    rosemoor_handle_key(loaded, SDL_SCANCODE_3, 0);
    ASSERT("the legacy topic works before accepting the new local hunt", !strcmp(loaded->dialogue_speaker, "Huntmaster Selene") && !loaded->rosemoor.hunt_quest && loaded->hunt_quest_state == 1);
    game_enter_frostfell(loaded);
    for (int stage = 1; stage < FROSTFELL_DEPTH; stage++) {
        for (int i = 0; i < loaded->enemy_count; i++) {
            loaded->enemies[i].active = 0;
        }
        ASSERT("legacy reward recovery preserves the real Frostfell journey", route_path(loaded, loaded->map.stairs_down_x, loaded->map.stairs_down_y));
    }
    int bow = -1;
    for (int i = 0; i < loaded->floor_item_count; i++) {
        if (loaded->floor_items[i].active && !strcmp(loaded->floor_items[i].item.name, "Krakenbone Bow")) {
            bow = i;
        }
    }
    ASSERT("a previous unclaimed bow is restored on returning to the lake", bow >= 0 && route_path(loaded, loaded->floor_items[bow].x, loaded->floor_items[bow].y));
    int inventory = loaded->inventory_count;
    action_resolve_player(loaded, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("legacy recovery awards the original bow and no new weapon choice", loaded->inventory_count == inventory + 1 && !strcmp(loaded->inventory[inventory].name, "Krakenbone Bow") && loaded->inventory[inventory].attack_bonus == 10 && !loaded->rosemoor.weapon && !loaded->kraken_bow_unclaimed);
    arrive(g);
    ASSERT("new save data rejects out-of-range quest state", save_game(g, ROSE_SLOT) && change_save(0, "frost_quest", 4) && !load_game(loaded, ROSE_SLOT));
    ASSERT("new save data rejects fractional weapon state", save_game(g, ROSE_SLOT) && change_save(0, "weapon", 1.5) && !load_game(loaded, ROSE_SLOT));
    ASSERT("new save data rejects a pending weapon without the boss victory", save_game(g, ROSE_SLOT) && change_save(0, "weapon", ROSE_WEAPON_PENDING) && !load_game(loaded, ROSE_SLOT));
}

void test_rosemoor(void) {
    printf("Rosemoor story arc:\n");
    ASSERT("Rosemoor fixture slot is unused", !save_exists(ROSE_SLOT));
    if (save_exists(ROSE_SLOT)) {
        return;
    }
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("Rosemoor fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    offers_and_trade(g, loaded);
    for (int seed = 0; seed < 3; seed++) {
        arrive(g);
        srand(811 + seed * 97);
        g->player.hp = g->player.max_hp = 10000;
        g->player.defense = 1000;
        if (seed % 2) {
            ashen_adventure(g);
            frost_adventure(g, loaded, seed);
        } else {
            frost_adventure(g, loaded, seed);
            ashen_adventure(g);
        }
        rewards(g, loaded, seed % 2);
    }
    legacy_saves(g, loaded);
    remove("saves/savegame_99296.json");
    free(g);
    free(loaded);
}
