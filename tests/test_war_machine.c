#include "expedition_fixture.h"
#include "test_utils.h"
#include "game/war_machine.h"
#include "game/ironfang.h"
#include "game/combat_feedback.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define WAR_SLOT 99287

static int route(GameState *g, int tx, int ty, int walk) {
    int previous[IRON_H][IRON_W];
    int queue[IRON_H * IRON_W];
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    memset(previous, -1, sizeof(previous));
    int head = 0;
    int tail = 1;
    int start = g->player.y * IRON_W + g->player.x;
    queue[0] = start;
    previous[g->player.y][g->player.x] = start;
    while (head < tail && previous[ty][tx] < 0) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % IRON_W + dx[i];
            int y = cell / IRON_W + dy[i];
            if (x < 0 || y < 0 || x >= IRON_W || y >= IRON_H || previous[y][x] >= 0 || !map_is_walkable(&g->map, x, y)) {
                continue;
            }
            previous[y][x] = cell;
            queue[tail++] = y * IRON_W + x;
        }
    }
    if (previous[ty][tx] < 0) {
        return 0;
    }
    int count = 0;
    for (int cell = ty * IRON_W + tx; cell != start; cell = previous[cell / IRON_W][cell % IRON_W]) {
        queue[count++] = cell;
    }
    if (walk) {
        for (int i = count - 1; i >= 0; i--) {
            action_resolve_player(g, (Action){ACTION_MOVE, queue[i] % IRON_W, queue[i] / IRON_W});
            if (g->player.x != queue[i] % IRON_W || g->player.y != queue[i] / IRON_W) {
                return 0;
            }
        }
    }
    return 1;
}

static void clear_ordinary(GameState *g) {
    for (int n = 0; n < g->enemy_count; n++) {
        if (!g->enemies[n].is_boss) {
            g->enemies[n].active = 0;
        }
    }
}

static void veyra(GameState *g) {
    game_enter_town_gate(g, LOCATION_TOWN4, 20, TOWN_H - 1);
    game_enter_town_hall(g);
    g->player.x = HALL_VEYRA_X;
    g->player.y = HALL_VEYRA_Y + 1;
    game_talk_to_veyra(g);
}

static int reach_trestle(GameState *g) {
    clear_ordinary(g);
    if (g->map.tiles[25][40] == TILE_IRON_BROKEN) {
        if (!route(g, 39, 25, 1) || !ironfang_interact(g)) {
            return 0;
        }
    }
    if (g->ironfang[g->level - 1].route) {
        if (!route(g, 29, 25, 1) || !ironfang_interact(g)) {
            return 0;
        }
    }
    if (!route(g, 9, 25, 1) || !ironfang_interact(g) || !g->ironfang[g->level - 1].aboard) {
        return 0;
    }
    for (int n = 0; n < IRON_W && g->player.x < 58; n++) {
        int before = g->player.x;
        action_resolve_player(g, (Action){ACTION_MOVE, before + 1, 25});
        if (g->player.x == before) {
            return 0;
        }
    }
    return g->player.x == 58 && g->player.y == 25;
}

static int rewrite(const char *field, double value, int old) {
    char path[80];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", WAR_SLOT);
    FILE *file = fopen(path, "rb");
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
    fread(data, 1, size, file);
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    if (old) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 206);
    test_mark_legacy_portal(root);
        cJSON_DeleteItemFromObject(root, "war_machine_quest_state");
        cJSON_DeleteItemFromObject(root, "war_machine_progress");
    } else {
        cJSON_ReplaceItemInObject(root, field, cJSON_CreateNumber(value));
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(path, "wb");
    int ok = file && data && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

void test_war_machine(void) {
    ASSERT("war machine fixture slot is unused", !save_exists(WAR_SLOT));
    if (save_exists(WAR_SLOT)) {
        return;
    }
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    Map *before = malloc(sizeof(*before));
    ASSERT("allocate war machine fixtures", g && loaded && before);
    if (!g || !loaded || !before) {
        free(g);
        free(loaded);
        free(before);
        return;
    }
    game_init(g);
    ASSERT("Ironfang quest cannot be assigned remotely", !war_machine_talk(g));
    veyra(g);
    ASSERT("Veyra keeps her original Watchfires offer", game_quest_offer_active(g) && !g->watchfire_quest_state && war_machine_dialogue(g));
    ASSERT("topic selection ignores held keys", war_machine_handle_key(g, SDL_SCANCODE_2, 1) && strcmp(g->dialogue_speaker, "Marshal Veyra") == 0);
    war_machine_handle_key(g, SDL_SCANCODE_2, 0);
    ASSERT("Veyra offers Ironfang independently with complete directions", game_quest_offer_active(g) && strcmp(g->dialogue_speaker, WAR_SPEAKER) == 0 &&
        strlen(g->dialogue_text) < MAX_DIALOGUE_LEN - 1 && strstr(g->dialogue_text, "160 gold."));
    ASSERT("an unanswered Ironfang offer survives saving", save_game(g, WAR_SLOT) && load_game(loaded, WAR_SLOT) && game_quest_offer_active(loaded) && war_machine_dialogue(loaded));
    game_handle_quest_offer_key(g, SDL_SCANCODE_N, 0);
    ASSERT("declining does not accept either of Veyra's quests", !g->watchfire_quest_state && !g->war_machine_quest_state);
    game_talk_to_veyra(g);
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    war_machine_handle_key(g, SDL_SCANCODE_2, 0);
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("both quests can be active together", g->watchfire_quest_state == 1 && g->war_machine_quest_state == 1);
    g->dialogue_active = 0;
    ironfang_enter(g, 0);
    ironfang_change_level(g, 1);
    clear_ordinary(g);
    ASSERT("the northwest side office can be explored on foot", route(g, WAR_ORDERS_X, WAR_ORDERS_Y, 1));
    g->inventory_count = MAX_INVENTORY;
    ASSERT("reading the shipment orders needs no pack slot", ironfang_interact(g) && g->war_machine_progress == WAR_ORDERS && g->inventory_count == MAX_INVENTORY);
    g->inventory_count = 0;
    int x;
    int y;
    ASSERT("completed orders leave the objective marker", !war_machine_marker(g, &x, &y));
    ASSERT("orders save independently of Watchfires", save_game(g, WAR_SLOT) && load_game(loaded, WAR_SLOT) &&
        loaded->war_machine_progress == WAR_ORDERS && loaded->watchfire_quest_state == 1 && !loaded->watchfire_quest_progress);
    ironfang_change_level(g, 1);
    ironfang_change_level(g, 1);
    clear_ordinary(g);
    ASSERT("the weapons platform cannot be reached on foot", !route(g, WAR_WINCH_X, WAR_WINCH_Y, 0));
    ASSERT("the weapons platform has its own minimap landmark", war_machine_marker(g, &x, &y) && x == WAR_WINCH_X && y == WAR_WINCH_Y);
    g->ironfang[3].route = 1;
    ASSERT("repair, east turnout and cart ride reach the flooded trestle", reach_trestle(g) && g->map.tiles[25][40] == TILE_IRON_RAIL && !g->ironfang[3].route);
    ASSERT("the quest winch cannot be operated from aboard", !war_machine_interact(g) && g->war_machine_progress == WAR_ORDERS);
    ASSERT("mid-crossing save retains passenger and quest state", save_game(g, WAR_SLOT) && load_game(loaded, WAR_SLOT) &&
        loaded->ironfang[3].aboard && loaded->ironfang[3].cart_x == 58 && loaded->player.x == 58 && loaded->war_machine_progress == WAR_ORDERS);
    action_resolve_player(g, (Action){ACTION_MOVE, 59, 25});
    ironfang_interact(g);
    ASSERT("the cart unloads onto the supply platform", !g->ironfang[3].aboard && g->player.x == 60 && g->player.y == 24);
    ASSERT("the ordinary salvage cache remains separately accessible", route(g, 59, 23, 1) && ironfang_interact(g) && g->map.tiles[23][60] == TILE_IRON_EMPTY && g->war_machine_progress == WAR_ORDERS);
    route(g, 61, 25, 1);
    *before = g->map;
    int cart_x = g->ironfang[3].cart_x;
    int cart_y = g->ironfang[3].cart_y;
    int feedback = combat_feedback_count();
    ASSERT("releasing the winch sabotages the shipment after the cache was claimed", ironfang_interact(g) && g->war_machine_progress == (WAR_ORDERS | WAR_SABOTAGE));
    ASSERT("sabotage emits visible falling cargo feedback even when the queue is full",
        combat_feedback_count() == (feedback < MAX_COMBAT_FEEDBACK ? feedback + 1 : feedback) &&
        combat_feedback_get(combat_feedback_count() - 1)->kind == FEEDBACK_WEAPONS_DUMP);
    ASSERT("sabotage preserves every terrain tile and the cart's return route", memcmp(before->tiles, g->map.tiles, sizeof(before->tiles)) == 0 &&
        g->ironfang[3].cart_x == cart_x && g->ironfang[3].cart_y == cart_y && !g->ironfang[3].route);
    feedback = combat_feedback_count();
    ASSERT("repeated sabotage cannot recreate weapons or feedback", !war_machine_interact(g) && combat_feedback_count() == feedback && g->war_machine_progress == 3);
    game_open_town_portal(g);
    ASSERT("escape cache keeps the sabotaged platform and cart", !g->portal_active && g->portal_location == LOCATION_IRONFANG && g->portal_level == 4 && g->war_machine_progress == 3);
    ASSERT("a platform escape records survives a save", save_game(g, WAR_SLOT) && load_game(loaded, WAR_SLOT) && !loaded->portal_active && loaded->war_machine_progress == 3);
    test_restore_cached_floor(g);
    ASSERT("archived floor reload preserves claimed salvage and the cart", g->location == LOCATION_IRONFANG && g->level == 4 &&
        g->map.tiles[23][60] == TILE_IRON_EMPTY && g->ironfang[3].cart_x == 60 && g->player.x == 61);
    ironfang_interact(g);
    for (int n = 0; n < 3; n++) {
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x - 1, 25});
    }
    ironfang_interact(g);
    ASSERT("the return journey remains playable after destroying the weapons", !g->ironfang[3].aboard && g->player.x == 54 && map_is_walkable(&g->map, g->player.x, g->player.y));
    ironfang_change_level(g, 1);
    clear_ordinary(g);
    g->player.x = WAR_SEAL_X;
    g->player.y = WAR_SEAL_Y + 1;
    war_machine_interact(g);
    ASSERT("the living Mine Boss prevents command seal recovery", !(g->war_machine_progress & WAR_SEAL) && !g->ironfang_defeated);
    g->player.x = 51;
    g->player.y = 12;
    g->player.attack = 999;
    action_resolve_player(g, (Action){ACTION_MOVE, 51, 11});
    ASSERT("ordinary combat records the existing Mine Boss victory", g->ironfang_defeated && !g->enemies[0].active && !(g->war_machine_progress & WAR_SEAL));
    g->player.x = WAR_SEAL_X;
    g->player.y = WAR_SEAL_Y + 1;
    g->inventory_count = MAX_INVENTORY;
    war_machine_interact(g);
    ASSERT("the command seal readies the quest outside a full pack", g->war_machine_progress == 7 && g->war_machine_quest_state == 2 && g->inventory_count == MAX_INVENTORY);
    g->inventory_count = 0;
    QuestJournalEntry entry;
    int found = 0;
    for (int n = 0; n < quest_journal_count(g, QUEST_TAB_ACTIVE); n++) {
        if (quest_journal_get_entry(g, QUEST_TAB_ACTIVE, n, &entry) && strcmp(entry.title, "Break the War Machine") == 0) {
            found = entry.objective_count == 3 && entry.stages[0] == 2 && entry.stages[1] == 4 && entry.stages[2] == 5 &&
                entry.objective_complete[0] && entry.objective_complete[1] && entry.objective_complete[2] && entry.reward_gold == 160 && entry.reward_score == 1000;
        }
    }
    ASSERT("the journal records all three mine objectives and the approved bounty", found);
    veyra(g);
    int gold = g->gold;
    int score = g->score;
    war_machine_handle_key(g, SDL_SCANCODE_2, 0);
    war_machine_handle_key(g, SDL_SCANCODE_2, 0);
    ASSERT("Veyra pays the Ironfang bounty once without finishing Watchfires", g->war_machine_quest_state == 3 && g->watchfire_quest_state == 1 &&
        g->gold == gold + WAR_REWARD_GOLD && g->score == score + WAR_REWARD_SCORE);
    ASSERT("completed quest and topic selection survive saving", save_game(g, WAR_SLOT) && load_game(loaded, WAR_SLOT) &&
        loaded->war_machine_quest_state == 3 && loaded->war_machine_progress == 7 && war_machine_dialogue(loaded));
    g->watchfire_quest_state = 2;
    g->watchfire_quest_progress = 7;
    gold = g->gold;
    war_machine_handle_key(g, SDL_SCANCODE_1, 0);
    ASSERT("switching back preserves Watchfires' independent payout", g->watchfire_quest_state == 3 && g->gold == gold + WATCHFIRE_REWARD_GOLD && g->war_machine_quest_state == 3);
    ironfang_enter(g, 0);
    ironfang_change_level(g, 1);
    ironfang_change_level(g, 1);
    ironfang_change_level(g, 1);
    ASSERT("fresh expeditions keep sabotage, claimed cache and rail repair", g->war_machine_progress == 7 && g->map.tiles[23][60] == TILE_IRON_EMPTY &&
        g->map.tiles[25][40] == TILE_IRON_RAIL && g->ironfang[3].cart_x == 10 && g->enemy_count >= 8);
    ASSERT("a rewarded quest is rejected if its objective data is missing", save_game(g, WAR_SLOT) && rewrite("war_machine_progress", 3, 0) && !load_game(loaded, WAR_SLOT));
    ASSERT("fractional quest states are rejected", save_game(g, WAR_SLOT) && rewrite("war_machine_quest_state", 1.5, 0) && !load_game(loaded, WAR_SLOT));
    ASSERT("invalid objective masks are rejected", save_game(g, WAR_SLOT) && rewrite("war_machine_progress", 8, 0) && !load_game(loaded, WAR_SLOT));
    ASSERT("command seal proof requires a recorded boss victory", save_game(g, WAR_SLOT) && rewrite("ironfang_defeated", 0, 0) && !load_game(loaded, WAR_SLOT));
    ASSERT("a v206 expedition upgrades without changing its quest coverage", save_game(g, WAR_SLOT) && rewrite(NULL, 0, 1) && load_game(loaded, WAR_SLOT) &&
        !loaded->war_machine_quest_state && !loaded->war_machine_progress && loaded->ironfang_defeated && loaded->watchfire_quest_state == 3 &&
        loaded->map.tiles[23][60] == TILE_IRON_EMPTY && loaded->map.tiles[25][40] == TILE_IRON_RAIL);
    ironfang_change_level(loaded, 1);
    loaded->player.x = WAR_SEAL_X;
    loaded->player.y = WAR_SEAL_Y + 1;
    war_machine_interact(loaded);
    ASSERT("an existing Mine Boss victory leaves its seal recoverable", loaded->war_machine_progress == WAR_SEAL && !loaded->war_machine_quest_state);
    // Pre-acceptance discoveries retain credit without granting a remote payout.
    ironfang_change_level(loaded, 0);
    ASSERT("unaccepted quests still allow the cart sabotage discovery", reach_trestle(loaded));
    action_resolve_player(loaded, (Action){ACTION_MOVE, 59, 25});
    ironfang_interact(loaded);
    war_machine_interact(loaded);
    ironfang_change_level(loaded, 0);
    ironfang_change_level(loaded, 0);
    clear_ordinary(loaded);
    route(loaded, WAR_ORDERS_X, WAR_ORDERS_Y, 1);
    war_machine_interact(loaded);
    ASSERT("all three discoveries can be made before accepting", !loaded->war_machine_quest_state && loaded->war_machine_progress == 7);
    veyra(loaded);
    war_machine_handle_key(loaded, SDL_SCANCODE_2, 0);
    gold = loaded->gold;
    game_handle_quest_offer_key(loaded, SDL_SCANCODE_Y, 0);
    ASSERT("accepting after discoveries retains them and waits for turn-in", loaded->war_machine_quest_state == 2 && loaded->war_machine_progress == 7 && loaded->gold == gold);
    // New overlays preserve old passenger saves, dropped gear and other quests.
    clear_ordinary(g);
    ASSERT("repaired infrastructure supports another cart crossing", reach_trestle(g));
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = WAR_WINCH_X, .y = WAR_WINCH_Y, .underlying_tile = TILE_IRON_FLOOR, .item = item_make_staff()};
    g->map.tiles[WAR_WINCH_Y][WAR_WINCH_X] = TILE_ITEM;
    g->gold = 4321;
    g->spice_skiff_quest_state = 3;
    g->spice_skiff_progress = 3;
    g->enemies[0].hp = 7;
    ASSERT("older passenger saves preserve cart state, wounds, gear, gold and previous quests", save_game(g, WAR_SLOT) && rewrite(NULL, 0, 1) && load_game(loaded, WAR_SLOT) &&
        loaded->ironfang[3].aboard && loaded->player.x == 58 && loaded->ironfang[3].cart_x == 58 && loaded->enemies[0].hp == 7 &&
        loaded->floor_items[0].active && loaded->map.tiles[WAR_WINCH_Y][WAR_WINCH_X] == TILE_ITEM && loaded->gold == 4321 &&
        loaded->spice_skiff_quest_state == 3 && !loaded->war_machine_progress);
    action_resolve_player(loaded, (Action){ACTION_MOVE, 59, 25});
    ironfang_interact(loaded);
    ASSERT("old dropped gear never hides or gets erased by the quest winch", war_machine_interact(loaded) && loaded->war_machine_progress == WAR_SABOTAGE &&
        loaded->floor_items[0].active && loaded->floor_items[0].underlying_tile == TILE_IRON_FLOOR && loaded->map.tiles[WAR_WINCH_Y][WAR_WINCH_X] == TILE_ITEM);
    remove("saves/savegame_99287.json");
    free(before);
    free(loaded);
    free(g);
}
