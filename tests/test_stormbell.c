#include "test_utils.h"
#include "game/stormbell.h"
#include "game/stormwatch.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define BELL_SLOT 99286

static int guard(GameState *g, int index) {
    const char *names[] = {"Stormbell Raider I", "Stormbell Raider II", "Stormbell Stormcaller"};
    for (int n = 0; n < g->enemy_count; n++) {
        if (strcmp(g->enemies[n].name, names[index]) == 0) {
            return n;
        }
    }
    return -1;
}

static int route(const GameState *g, int tx, int ty) {
    unsigned char seen[STORM_H][STORM_W] = {{0}};
    int queue[STORM_H * STORM_W];
    int head = 0;
    int tail = 1;
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    queue[0] = g->map.stairs_up_y * STORM_W + g->map.stairs_up_x;
    seen[g->map.stairs_up_y][g->map.stairs_up_x] = 1;
    while (head < tail) {
        int cell = queue[head++];
        for (int n = 0; n < 4; n++) {
            int x = cell % STORM_W + dx[n];
            int y = cell / STORM_W + dy[n];
            if (x < 0 || y < 0 || x >= STORM_W || y >= STORM_H || seen[y][x] || !map_is_walkable(&g->map, x, y)) {
                continue;
            }
            seen[y][x] = 1;
            queue[tail++] = y * STORM_W + x;
        }
    }
    return seen[ty][tx];
}

static int rewrite(const char *field, double value, int old) {
    char path[80];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", BELL_SLOT);
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
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 205);
        const char *keys[] = {"stormbell_quest_state", "stormbell_progress", "stormbell_phase", "stormbell_guards"};
        for (int i = 0; i < 4; i++) {
            cJSON_DeleteItemFromObject(root, keys[i]);
        }
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

static void at_bell(GameState *g) {
    g->player.x = STORMBELL_X;
    g->player.y = STORMBELL_Y + 1;
    g->stormwatch[3].phase[0] = 0;
    g->stormwatch[3].phase[1] = 0;
}

void test_stormbell(void) {
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("allocate stormbell fixtures", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    ASSERT("Seren stands beside the east road without replacing its paving", g->map.tiles[SEREN_Y][SEREN_X] == TILE_NPC_SEREN &&
        !map_is_walkable(&g->map, SEREN_X, SEREN_Y) && g->map.tiles[12][SEREN_X] == TILE_TOWN_PATH);
    g->player.x = SEREN_X;
    g->player.y = SEREN_Y + 1;
    ASSERT("Seren offers the approved quest with readable complete dialogue", stormbell_talk(g) && game_quest_offer_active(g) &&
        strlen(g->dialogue_text) < MAX_DIALOGUE_LEN - 1 && strstr(g->dialogue_text, "180 gold."));
    game_handle_quest_offer_key(g, SDL_SCANCODE_N, 0);
    ASSERT("declining leaves the bell quest unaccepted", !g->stormbell_quest_state);
    stormbell_talk(g);
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    g->dialogue_active = 0;
    ASSERT("accepting adds the quest without awarding gold", g->stormbell_quest_state == 1 && !g->stormbell_progress);
    stormwatch_enter(g, 0);
    stormwatch_change_level(g, 1);
    stormwatch_change_level(g, 1);
    ASSERT("the optional northeast tower's clapper is reachable", route(g, STORMBELL_CLAPPER_X, STORMBELL_CLAPPER_Y));
    g->player.x = STORMBELL_CLAPPER_X;
    g->player.y = STORMBELL_CLAPPER_Y;
    g->inventory_count = MAX_INVENTORY;
    stormbell_interact(g);
    ASSERT("the clapper uses quest discoveries when the pack is full", g->stormbell_progress == 1 && g->inventory_count == MAX_INVENTORY);
    g->inventory_count = 0;
    ASSERT("clapper discovery saves before fitting", save_game(g, BELL_SLOT) && load_game(loaded, BELL_SLOT) && loaded->stormbell_progress == 1);
    stormwatch_change_level(g, 1);
    ASSERT("bell courtyard is reachable with the shutter raised", route(g, STORMBELL_X, STORMBELL_Y));
    g->map.tiles[24][32] = TILE_STORM_SHUTTER_DOWN;
    ASSERT("lowering the shutter retains access to the courtyard", route(g, STORMBELL_X, STORMBELL_Y));
    at_bell(g);
    stormwatch_interact(g);
    int marker_x;
    int marker_y;
    ASSERT("the bell has a discoverable minimap landmark", stormbell_marker(g, &marker_x, &marker_y) && marker_x == STORMBELL_X && marker_y == STORMBELL_Y);
    ASSERT("fitting repairs the bell without starting combat", g->stormbell_progress == 3 && !g->stormbell_phase && !g->dialogue_active);
    stormwatch_interact(g);
    ASSERT("ringing requires an explicit warning confirmation", stormbell_prompt_active(g) && strstr(g->dialogue_text, "two Harpy Raiders"));
    int prompt_timer = g->enemies[0].move_timer;
    action_resolve_enemies(g);
    ASSERT("ordinary enemies wait while the bell confirmation is open", g->enemies[0].move_timer == prompt_timer);
    ASSERT("the unanswered confirmation survives save/load", save_game(g, BELL_SLOT) && load_game(loaded, BELL_SLOT) && stormbell_prompt_active(loaded));
    stormbell_handle_key(g, SDL_SCANCODE_N, 0);
    ASSERT("declining the defense summons nothing", !g->stormbell_phase && !g->dialogue_active);
    stormwatch_interact(g);
    stormbell_handle_key(g, SDL_SCANCODE_Y, 1);
    ASSERT("a repeated confirmation key cannot start the defense", !g->stormbell_phase);
    stormbell_handle_key(g, SDL_SCANCODE_Y, 0);
    int first = guard(g, 0);
    int second = guard(g, 1);
    ASSERT("only two Raiders answer the first ring", g->stormbell_phase == 1 && first >= 0 && second >= 0 && guard(g, 2) < 0);
    if (first < 0 || second < 0) {
        free(g);
        free(loaded);
        return;
    }
    ASSERT("both arriving Raiders stand on connected visible land", route(g, g->enemies[first].x, g->enemies[first].y) &&
        route(g, g->enemies[second].x, g->enemies[second].y) && g->enemies[first].revived && !g->enemies[first].experience);
    g->enemies[first].hp = 7;
    g->enemies[second].active = 0;
    g->enemies[second].hp = 0;
    int timer = g->enemies[first].move_timer;
    for (int n = 0; n < 18; n++) {
        stormwatch_advance_wind(g, 1000);
    }
    ASSERT("idle wind gives no enemy turns or extra waves", g->enemies[first].move_timer == timer && g->stormbell_phase == 1 && guard(g, 2) < 0);
    ASSERT("mid-wave save captures current wounds and defeated Raiders", save_game(g, BELL_SLOT) && load_game(loaded, BELL_SLOT) &&
        loaded->stormbell_guards[0].hp == 7 && !loaded->stormbell_guards[1].active && loaded->stormbell_phase == 1);
    stormwatch_change_level(g, 0);
    stormwatch_change_level(g, 1);
    first = guard(g, 0);
    ASSERT("backtracking preserves the finite encounter", first >= 0 && g->enemies[first].hp == 7 && !g->enemies[guard(g, 1)].active);
    game_open_town_portal(g);
    ASSERT("a town portal preserves the defense", g->portal_active && g->portal_location == LOCATION_STORMWATCH && g->stormbell_phase == 1);
    game_use_town_portal(g);
    first = guard(g, 0);
    ASSERT("portal return preserves wounded Raiders", g->location == LOCATION_STORMWATCH && g->level == 4 && first >= 0 && g->enemies[first].hp == 7);
    stormwatch_leave(g);
    stormwatch_enter(g, 0);
    stormwatch_change_level(g, 1);
    stormwatch_change_level(g, 1);
    stormwatch_change_level(g, 1);
    first = guard(g, 0);
    ASSERT("fresh expedition restores surviving quest guards without reviving defeated ones", first >= 0 && g->enemies[first].hp == 7 && guard(g, 1) < 0);
    int ordinary = 0;
    for (int n = 0; n < g->enemy_count; n++) {
        ordinary += g->enemies[n].active && strncmp(g->enemies[n].name, "Stormbell ", 10) != 0;
    }
    ASSERT("ordinary Stormwatch enemies still respawn on re-entry", ordinary > 0);
    for (int n = 0; n < g->enemy_count; n++) {
        g->enemies[n].frozen_turns = 50;
    }
    // Use the combat damage path for the last Raider, then one normal enemy phase.
    g->player.attack = 999;
    g->enemies[first].x = STORMBELL_X + 1;
    g->enemies[first].y = STORMBELL_Y;
    g->player.x = STORMBELL_X;
    g->player.y = STORMBELL_Y;
    action_resolve_player(g, (Action){ACTION_MOVE, STORMBELL_X + 1, STORMBELL_Y});
    ASSERT("killing the last Raider does not spawn an immediate extra attacker", !g->enemies[first].active && guard(g, 2) < 0);
    action_resolve_enemies(g);
    int caller = guard(g, 2);
    ASSERT("the Stormcaller arrives after the Raiders and waits for the next turn", g->stormbell_phase == 2 && caller >= 0 &&
        !g->enemies[caller].attack_phase && !g->enemies[caller].move_timer);
    ASSERT("second wave saves and loads exactly", save_game(g, BELL_SLOT) && load_game(loaded, BELL_SLOT) &&
        loaded->stormbell_phase == 2 && loaded->stormbell_guards[2].active);
    g->enemies[caller].hp = 0;
    g->enemies[caller].active = 0;
    action_resolve_enemies(g);
    ASSERT("clearing the defense readies Seren's quest independently of the Matriarch", g->stormbell_phase == 3 && g->stormbell_progress == 7 &&
        g->stormbell_quest_state == 2 && !g->stormwatch_defeated);
    int count = g->enemy_count;
    at_bell(g);
    stormbell_interact(g);
    ASSERT("a restored bell cannot create repeated enemies", g->enemy_count == count && g->stormbell_phase == 3);
    QuestJournalEntry entry;
    ASSERT("the journal records all three quest objectives and reward", quest_journal_get_entry(g, QUEST_TAB_ACTIVE, 0, &entry) &&
        entry.objective_count == 3 && entry.objective_complete[0] && entry.objective_complete[1] && entry.objective_complete[2] && entry.reward_gold == 180);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    g->player.x = SEREN_X;
    g->player.y = SEREN_Y + 1;
    int gold = g->gold;
    int score = g->score;
    stormbell_talk(g);
    stormbell_talk(g);
    ASSERT("Seren pays exactly once", g->stormbell_quest_state == 3 && g->gold == gold + 180 && g->score == score + 1200);
    ASSERT("completed defense and reward survive saves", save_game(g, BELL_SLOT) && load_game(loaded, BELL_SLOT) &&
        loaded->stormbell_quest_state == 3 && loaded->stormbell_phase == 3 && stormbell_valid(loaded));
    ASSERT("fractional quest states are rejected", save_game(g, BELL_SLOT) && rewrite("stormbell_quest_state", 1.5, 0) && !load_game(loaded, BELL_SLOT));
    ASSERT("illegal bell progress is rejected", save_game(g, BELL_SLOT) && rewrite("stormbell_progress", 8, 0) && !load_game(loaded, BELL_SLOT));
    ASSERT("inconsistent finite wave states are rejected", save_game(g, BELL_SLOT) && rewrite("stormbell_phase", 1, 0) && !load_game(loaded, BELL_SLOT));
    // Discovery and defense may precede acceptance, just like other regional quests.
    g->stormbell_quest_state = 0;
    stormbell_talk(g);
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("acceptance retains discoveries made before meeting Seren", g->stormbell_quest_state == 2 && g->stormbell_progress == 7);
    game_init(g);
    srand(49);
    stormwatch_enter(g, 0);
    for (int n = 1; n < 5; n++) {
        stormwatch_change_level(g, 1);
    }
    int boss = -1;
    for (int n = 0; n < g->enemy_count; n++) {
        if (g->enemies[n].is_boss) {
            boss = n;
        }
    }
    ASSERT("seed 49 can no longer suppress the Matriarch", boss >= 0 && g->enemies[boss].type == ENEMY_TEMPEST_MATRIARCH);
    if (boss >= 0) {
        g->enemies[boss].type = ENEMY_CLIFF_GARGOYLE;
        g->enemies[boss].is_boss = 0;
        snprintf(g->enemies[boss].name, sizeof(g->enemies[boss].name), "Cliff Gargoyle");
    }
    g->gold = 4321;
    g->enemies[0].hp = 7;
    map_mark_explored(&g->map, 43, 10);
    int px = g->player.x;
    int py = g->player.y;
    ASSERT("older affected saves load with their progress intact", save_game(g, BELL_SLOT) && rewrite(NULL, 0, 1) && load_game(loaded, BELL_SLOT) &&
        loaded->gold == 4321 && loaded->enemies[0].hp == 7 && loaded->player.x == px && loaded->player.y == py && map_is_explored(&loaded->map, 43, 10));
    int bosses = 0;
    int overlaps = 0;
    for (int n = 0; n < loaded->enemy_count; n++) {
        bosses += loaded->enemies[n].is_boss;
        for (int m = n + 1; m < loaded->enemy_count; m++) {
            overlaps += loaded->enemies[n].active && loaded->enemies[m].active && loaded->enemies[n].x == loaded->enemies[m].x && loaded->enemies[n].y == loaded->enemies[m].y;
        }
    }
    ASSERT("migration adds the missing Matriarch without displacing existing enemies", bosses == 1 && !overlaps && !loaded->stormbell_phase && !loaded->stormbell_progress);
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    g->map.tiles[SEREN_Y][SEREN_X] = TILE_TOWN_FLOOR;
    ASSERT("old Dawnmere saves gain Seren while keeping the plaza", save_game(g, BELL_SLOT) && rewrite(NULL, 0, 1) && load_game(loaded, BELL_SLOT) &&
        loaded->map.tiles[SEREN_Y][SEREN_X] == TILE_NPC_SEREN && loaded->map.tiles[12][SEREN_X] == TILE_TOWN_PATH);
    // Old cached boss repair must preserve a return portal and dropped gear.
    game_init(g);
    stormwatch_enter(g, 0);
    for (int n = 1; n < 5; n++) {
        stormwatch_change_level(g, 1);
    }
    for (int n = 0; n < g->enemy_count; n++) {
        if (g->enemies[n].is_boss) {
            g->enemies[n].active = 0;
            g->enemies[n].type = ENEMY_CLIFF_GARGOYLE;
            g->enemies[n].is_boss = 0;
        }
    }
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 43, .y = 10, .underlying_tile = TILE_STORM_STONE, .item = item_make_staff()};
    g->map.tiles[10][43] = TILE_ITEM;
    game_open_town_portal(g);
    ASSERT("cached missing-boss saves can migrate with loot and a portal", save_game(g, BELL_SLOT) && rewrite(NULL, 0, 1) && load_game(loaded, BELL_SLOT) && loaded->portal_active && loaded->stormwatch[4].loot[0].active);
    bosses = 0;
    int on_loot = 0;
    for (int n = 0; n < loaded->stormwatch[4].enemy_count; n++) {
        Enemy *e = &loaded->stormwatch[4].enemies[n];
        if (e->is_boss) {
            bosses++;
            on_loot |= (e->x == 43 && e->y == 10) || (e->x == loaded->portal_x && e->y == loaded->portal_y);
        }
    }
    ASSERT("the repaired cached Matriarch avoids loot and portal tiles", bosses == 1 && !on_loot);
    loaded->stormwatch_defeated = 1;
    int before = loaded->stormwatch[4].enemy_count;
    stormwatch_repair_boss(loaded);
    ASSERT("repair never adds another boss after victory", loaded->stormwatch[4].enemy_count == before);
    // At capacity, a finite encounter waits for room instead of softlocking.
    game_init(g);
    stormwatch_enter(g, 0);
    for (int n = 1; n < 4; n++) {
        stormwatch_change_level(g, 1);
    }
    at_bell(g);
    g->stormbell_progress = 3;
    for (int n = g->enemy_count; n < AREA_ENEMY_LIMIT; n++) {
        g->enemies[n] = g->enemies[0];
        g->enemies[n].x = 15 + n;
        g->enemies[n].y = 24;
    }
    g->enemy_count = AREA_ENEMY_LIMIT;
    stormbell_interact(g);
    stormbell_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("a full floor cannot consume or start the bell defense", !g->stormbell_phase && !g->dialogue_active && g->stormbell_progress == 3);
    g->enemies[0].active = 0;
    g->enemies[1].active = 0;
    stormbell_interact(g);
    stormbell_handle_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("freeing two slots allows both Raiders without exceeding the limit", g->stormbell_phase == 1 && guard(g, 0) >= 0 && guard(g, 1) >= 0 && g->enemy_count == AREA_ENEMY_LIMIT);
    g->enemies[guard(g, 0)].active = 0;
    g->enemies[guard(g, 1)].active = 0;
    stormbell_turn(g);
    ASSERT("the next wave reuses a defeated slot safely on a full floor", g->stormbell_phase == 2 && guard(g, 2) >= 0 && g->enemy_count == AREA_ENEMY_LIMIT);
    remove("saves/savegame_99286.json");
    free(loaded);
    free(g);
}
