#include "test_utils.h"
#include "game/spice_skiff.h"
#include "game/mangrove.h"
#include "game/town_life.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define SPICE_SLOT 99285

static void enter_shop(GameState *g) {
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    const TownBuilding *b = town_life_building(LOCATION_DAWNMERE);
    g->player.x = b->x + b->w / 2;
    g->player.y = b->y + b->h;
    action_resolve_player(g, (Action){ACTION_MOVE, g->player.x, g->player.y - 1});
}

static void talk(GameState *g) {
    g->player.x = RESIDENT_X;
    g->player.y = RESIDENT_Y + 1;
    town_life_talk(g, 0);
}

static int route(GameState *g, int tx, int ty, int afloat, int walk) {
    int previous[MANGROVE_H][MANGROVE_W];
    int queue[MANGROVE_W * MANGROVE_H];
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    memset(previous, -1, sizeof(previous));
    int start = g->player.y * MANGROVE_W + g->player.x;
    int target = ty * MANGROVE_W + tx;
    int head = 0;
    int tail = 1;
    queue[0] = start;
    previous[g->player.y][g->player.x] = start;
    while (head < tail && previous[ty][tx] < 0) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % MANGROVE_W + dx[i];
            int y = cell / MANGROVE_W + dy[i];
            if (x < 1 || x >= MANGROVE_W - 1 || y < 1 || y >= MANGROVE_H - 1 || previous[y][x] >= 0) {
                continue;
            }
            if (afloat ? g->map.tiles[y][x] != TILE_MANGROVE_WATER : !map_is_walkable(&g->map, x, y)) {
                continue;
            }
            int occupied = 0;
            if (walk && !afloat) {
                for (int n = 0; n < g->enemy_count; n++) {
                    occupied |= g->enemies[n].active && g->enemies[n].x == x && g->enemies[n].y == y;
                }
            }
            if (occupied) {
                continue;
            }
            previous[y][x] = cell;
            queue[tail++] = y * MANGROVE_W + x;
        }
    }
    if (previous[ty][tx] < 0) {
        return 0;
    }
    int count = 0;
    for (int cell = target; cell != start; cell = previous[cell / MANGROVE_W][cell % MANGROVE_W]) {
        queue[count++] = cell;
    }
    if (walk) {
        for (int i = count - 1; i >= 0; i--) {
            int x = queue[i] % MANGROVE_W;
            int y = queue[i] / MANGROVE_W;
            action_resolve_player(g, (Action){ACTION_MOVE, x, y});
            if (g->player.x != x || g->player.y != y) {
                return 0;
            }
        }
    }
    return 1;
}

static int reach_side_island(GameState *g) {
    int x;
    int y;
    int cx;
    int cy;
    mangrove_landing(g->level, 0, &x, &y);
    if (!route(g, x, y, 0, 1)) {
        return 0;
    }
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    if (!g->mangroves.aboard) {
        return 0;
    }
    mangrove_landing(g->level, 1, &x, &y);
    mangrove_island(g->level, 1, &cx, &cy);
    int dx = (x > cx) - (x < cx);
    int dy = (y > cy) - (y < cy);
    if (!route(g, x + dx, y + dy, 1, 1)) {
        return 0;
    }
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    return !g->mangroves.aboard && g->player.x == x && g->player.y == y;
}

static void clear_cargo_island(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].x < 20) {
            g->enemies[i].active = 0;
        }
    }
}

static int edit_save(const char *field, double value, int legacy) {
    char path[80];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", SPICE_SLOT);
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
    fread(data, 1, (size_t)size, file);
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    if (legacy) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 204);
        cJSON_DeleteItemFromObject(root, "spice_skiff_quest_state");
        cJSON_DeleteItemFromObject(root, "spice_skiff_progress");
    } else {
        cJSON_ReplaceItemInObject(root, field, cJSON_CreateNumber(value));
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(path, "wb");
    int ok = data && file && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

void test_spice_skiff(void) {
    printf("The Missing Spice Skiff quest:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("spice skiff fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    ASSERT("spice quest save slot is unused", !save_exists(SPICE_SLOT));
    if (save_exists(SPICE_SLOT)) {
        free(g);
        free(loaded);
        return;
    }
    game_init(g);
    enter_shop(g);
    ASSERT("the existing merchant door opens and waits for Talk", g->location == LOCATION_DAWNMERE_MERCHANT &&
        !g->dialogue_active && !g->spice_skiff_quest_state);
    ASSERT("Corvin requires a nearby conversation", !town_life_talk(g, 0));
    talk(g);
    ASSERT("Corvin offers the approved quest with a southward marsh direction", game_quest_offer_active(g) &&
        strstr(g->dialogue_text, "south of Dawnmere") && !strcmp(g->dialogue_text, spice_skiff_offer) && !g->spice_skiff_quest_state);
    game_handle_quest_offer_key(g, SDL_SCANCODE_N, 0);
    ASSERT("declining gives no quest or cargo", !g->spice_skiff_quest_state && !g->spice_skiff_progress);
    talk(g);
    ASSERT("an unanswered merchant offer survives saving", save_game(g, SPICE_SLOT) && load_game(loaded, SPICE_SLOT) && game_quest_offer_active(loaded));
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    g->dialogue_active = 0;
    QuestJournalEntry entry;
    ASSERT("acceptance records both Saltwind stages and the bounty", g->spice_skiff_quest_state == 1 &&
        quest_journal_get_entry(g, QUEST_TAB_ACTIVE, 0, &entry) && !strcmp(entry.title, "The Missing Spice Skiff") &&
        entry.stages[0] == 2 && entry.stages[1] == 3 && entry.objective_count == 2 && entry.reward_gold == 150 && entry.reward_score == 1000);
    mangrove_enter(g, 0);
    mangrove_change_level(g, 1);
    ASSERT("stage two's wreck cannot be reached on foot from the entry island", !route(g, SPICE_WRECK_X, SPICE_WRECK_Y, 0, 0));
    ASSERT("the real skiff reaches the northern side island", reach_side_island(g));
    ASSERT("the stranded skiff has a reachable shore approach", route(g, SPICE_WRECK_X + 1, SPICE_WRECK_Y, 0, 1));
    ASSERT("the configured action key recognizes the stranded skiff", game_has_regional_interaction(g) && spice_skiff_hint(g));
    int gold = g->gold;
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("reading or rereading the manifest gives one clue and no repeat reward", g->spice_skiff_progress == SPICE_MANIFEST && g->gold == gold);
    g->enemies[0].hp = 7;
    int boat_x = g->mangroves.boat_x;
    int boat_y = g->mangroves.boat_y;
    ASSERT("manifest, moored skiff and wounded shore guards survive a save", save_game(g, SPICE_SLOT) && load_game(loaded, SPICE_SLOT) &&
        loaded->spice_skiff_progress == SPICE_MANIFEST && loaded->mangroves.boat_x == boat_x && loaded->mangroves.boat_y == boat_y &&
        !loaded->mangroves.aboard && loaded->enemies[0].hp == 7);
    mangrove_change_level(g, 1);
    ASSERT("stage three's cargo is off the main walking route", !route(g, SPICE_CASKET_X, SPICE_CASKET_Y, 0, 0));
    ASSERT("the real skiff reaches the cargo island's east-facing landing", reach_side_island(g));
    g->player.x = SPICE_CASKET_X;
    g->player.y = SPICE_CASKET_Y + 1;
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("living shore guards prevent recovering the cargo", g->spice_skiff_progress == SPICE_MANIFEST);
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].frozen_turns = 5;
    }
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("freezing the island's guards does not count as defeating them", g->spice_skiff_progress == SPICE_MANIFEST);
    clear_cargo_island(g);
    int living = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        living += g->enemies[i].active;
    }
    g->mangroves.aboard = 1;
    ASSERT("cargo cannot be recovered from aboard a skiff", !spice_skiff_interact(g) && !spice_skiff_hint(g));
    g->mangroves.aboard = 0;
    int inventory = g->inventory_count;
    g->inventory_count = MAX_INVENTORY;
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("a full pack does not block recovery and other islands' enemies do not guard this cargo", living > 0 &&
        g->inventory_count == MAX_INVENTORY && g->spice_skiff_progress == 3 && g->spice_skiff_quest_state == 2);
    g->inventory_count = inventory;
    game_open_town_portal(g);
    ASSERT("Return to Town preserves cargo, island casualties and the moored boat", g->location == LOCATION_DAWNMERE &&
        g->portal_active && g->portal_level == 3 && g->mangrove_stages[2].boat_x == g->mangroves.boat_x && g->spice_skiff_progress == 3);
    ASSERT("the recovered cargo and return portal survive saving", save_game(g, SPICE_SLOT) && load_game(loaded, SPICE_SLOT) &&
        loaded->spice_skiff_quest_state == 2 && loaded->spice_skiff_progress == 3 && loaded->portal_active);
    enter_shop(loaded);
    gold = loaded->gold;
    int score = loaded->score;
    talk(loaded);
    talk(loaded);
    ASSERT("Corvin pays exactly one bounty without requiring Sszareth", loaded->spice_skiff_quest_state == 3 &&
        loaded->gold == gold + SPICE_REWARD_GOLD && loaded->score == score + SPICE_REWARD_SCORE && !loaded->mangrove_defeated);
    ASSERT("the completed journal keeps both discoveries", quest_journal_get_entry(loaded, QUEST_TAB_COMPLETED, 0, &entry) &&
        entry.objective_complete[0] && entry.objective_complete[1]);
    town_life_leave(loaded);
    game_use_town_portal(loaded);
    ASSERT("the portal resumes the same cargo island encounter", loaded->location == LOCATION_MANGROVES && loaded->level == 3 &&
        loaded->spice_skiff_quest_state == 3 && loaded->spice_skiff_progress == 3 && !loaded->mangroves.aboard);
    mangrove_leave(loaded);
    mangrove_enter(loaded, 0);
    mangrove_change_level(loaded, 1);
    mangrove_change_level(loaded, 1);
    int x;
    int y;
    ASSERT("physical reentry refreshes ordinary guards without replenishing quest cargo", loaded->enemy_count == 9 &&
        loaded->spice_skiff_progress == 3 && !spice_skiff_marker(loaded, &x, &y));

    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    mangrove_enter(g, 0);
    mangrove_change_level(g, 1);
    mangrove_change_level(g, 1);
    clear_cargo_island(g);
    g->player.x = SPICE_CASKET_X;
    g->player.y = SPICE_CASKET_Y + 1;
    spice_skiff_interact(g);
    mangrove_change_level(g, 0);
    g->player.x = SPICE_WRECK_X + 1;
    g->player.y = SPICE_WRECK_Y;
    spice_skiff_interact(g);
    game_open_town_portal(g);
    enter_shop(g);
    talk(g);
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("discoveries made in either order before acceptance count without resetting the expedition", g->spice_skiff_quest_state == 2 &&
        g->spice_skiff_progress == 3 && g->portal_active && g->portal_level == 2);

    game_init(g);
    enter_shop(g);
    talk(g);
    snprintf(g->dialogue_text, sizeof(g->dialogue_text), "I trade in goods from Saltwind Marsh, south of Dawnmere.");
    ASSERT("older merchant dialogue migrates into the new offer without auto-accepting", save_game(g, SPICE_SLOT) && edit_save(NULL, 0, 1) &&
        load_game(loaded, SPICE_SLOT) && game_quest_offer_active(loaded) && strstr(loaded->dialogue_text, "manifest") && !loaded->spice_skiff_quest_state);
    mangrove_enter(g, 0);
    mangrove_change_level(g, 1);
    mangrove_landing(2, 0, &x, &y);
    route(g, x, y, 0, 1);
    mangrove_interact(g);
    g->gold = 4321;
    g->heartseed_quest_state = 3;
    g->heartseed_progress = 7;
    g->heartseed_bindings = 7;
    g->heartseed_guardian = 3;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = SPICE_WRECK_X, .y = SPICE_WRECK_Y,
        .underlying_tile = TILE_MANGROVE_MUD, .item = item_make_staff()};
    g->map.tiles[SPICE_WRECK_Y][SPICE_WRECK_X] = TILE_ITEM;
    g->enemies[0].hp = 7;
    int px = g->player.x;
    int py = g->player.y;
    ASSERT("an older aboard save keeps boat, wounds, dropped gear, gold and previous quests", save_game(g, SPICE_SLOT) && edit_save(NULL, 0, 1) &&
        load_game(loaded, SPICE_SLOT) && loaded->mangroves.aboard && loaded->player.x == px && loaded->player.y == py &&
        loaded->gold == 4321 && loaded->enemies[0].hp == 7 && loaded->floor_item_count == 1 && loaded->floor_items[0].active &&
        loaded->heartseed_quest_state == 3 && !loaded->spice_skiff_quest_state && !loaded->spice_skiff_progress);
    ASSERT("invalid spice masks are rejected", save_game(g, SPICE_SLOT) && edit_save("spice_skiff_progress", 4, 0) && !load_game(loaded, SPICE_SLOT));
    ASSERT("fractional spice states are rejected", save_game(g, SPICE_SLOT) && edit_save("spice_skiff_quest_state", 1.5, 0) && !load_game(loaded, SPICE_SLOT));
    ASSERT("a ready quest without its discoveries is rejected", save_game(g, SPICE_SLOT) && edit_save("spice_skiff_quest_state", 2, 0) && !load_game(loaded, SPICE_SLOT));
    remove("saves/savegame_99285.json");
    free(loaded);
    free(g);
}
