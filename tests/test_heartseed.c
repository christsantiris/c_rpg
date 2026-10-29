#include "expedition_fixture.h"
#include "test_utils.h"
#include "game/heartseed.h"
#include "game/elderwood.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define HEARTSEED_SLOT 99284

static Enemy *guardian(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (heartseed_is_guardian(g, &g->enemies[i])) {
            return &g->enemies[i];
        }
    }
    return NULL;
}

static void stage(GameState *g, int level) {
    elderwood_enter(g);
    while (g->level < level) {
        game_descend(g);
    }
}

static void interact(GameState *g, int x, int y) {
    g->player.x = x;
    g->player.y = y;
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
}

static int reachable(const GameState *g, int tx, int ty) {
    unsigned char seen[ELDERWOOD_W * ELDERWOOD_H] = {0};
    int queue[ELDERWOOD_W * ELDERWOOD_H];
    int head = 0;
    int tail = 1;
    queue[0] = 46 * ELDERWOOD_W + 32;
    seen[queue[0]] = 1;
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    while (head < tail) {
        int current = queue[head++];
        int x = current % ELDERWOOD_W;
        int y = current / ELDERWOOD_W;
        if (x == tx && y == ty) {
            return 1;
        }
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (nx < 1 || ny < 1 || nx >= ELDERWOOD_W - 1 || ny >= ELDERWOOD_H - 1) {
                continue;
            }
            int next = ny * ELDERWOOD_W + nx;
            if (!seen[next] && map_is_walkable(&g->map, nx, ny)) {
                seen[next] = 1;
                queue[tail++] = next;
            }
        }
    }
    return 0;
}

static int rewrite_save(const char *field, int value, int legacy) {
    char path[80];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", HEARTSEED_SLOT);
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
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 203);
    test_mark_legacy_portal(root);
        const char *fields[] = {"heartseed_quest_state", "heartseed_progress", "heartseed_bindings", "heartseed_sites", "heartseed_guardian"};
        for (int i = 0; i < 5; i++) {
            cJSON_DeleteItemFromObject(root, fields[i]);
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

static void accept(GameState *g) {
    g->player.x = TAMSIN_X + 1;
    g->player.y = TAMSIN_Y;
    heartseed_talk(g);
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    g->dialogue_active = 0;
}

void test_heartseed(void) {
    printf("The Stolen Heartseed quest:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("heartseed fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    ASSERT("heartseed test save slot is unused", !save_exists(HEARTSEED_SLOT));
    if (save_exists(HEARTSEED_SLOT)) {
        free(g);
        free(loaded);
        return;
    }
    game_init(g);
    ASSERT("Tamsin stands beside the northern road without blocking it or the tavern", g->map.tiles[TAMSIN_Y][TAMSIN_X] == TILE_NPC_TAMSIN &&
        !map_is_walkable(&g->map, TAMSIN_X, TAMSIN_Y) && map_is_walkable(&g->map, 20, TAMSIN_Y) &&
        g->map.tiles[TOWN_TAVERN_DOOR_Y][TOWN_TAVERN_DOOR_X] == TILE_TAVERN_DOOR);
    ASSERT("remote talking cannot accept Tamsin's quest", !heartseed_talk(g) && !g->heartseed_quest_state);
    g->player.x = TAMSIN_X + 1;
    g->player.y = TAMSIN_Y;
    ASSERT("Tamsin offers an explicit quest choice", heartseed_talk(g) && game_quest_offer_active(g));
    ASSERT("Tamsin's briefing is not cut off", strstr(g->dialogue_text, "bring both seed and orders to me for 120 gold.") != NULL);
    game_handle_quest_offer_key(g, SDL_SCANCODE_N, 0);
    ASSERT("declining leaves the quest available", !g->heartseed_quest_state && !g->dialogue_active);
    accept(g);
    QuestJournalEntry entry;
    ASSERT("accepting adds the correct stages and rewards to the journal", g->heartseed_quest_state == 1 &&
        quest_journal_get_entry(g, QUEST_TAB_ACTIVE, 0, &entry) && !strcmp(entry.title, "The Stolen Heartseed") &&
        entry.stages[0] == 2 && entry.stages[1] == 4 && entry.stages[2] == 4 &&
        entry.reward_gold == 120 && entry.reward_score == 800);
    heartseed_talk(g);
    ASSERT("Tamsin's progress report is not cut off", strstr(g->dialogue_text, "Bring the orders and seed back here.") != NULL);
    g->dialogue_active = 0;
    stage(g, 2);
    int count = g->enemy_count;
    interact(g, HEARTSEED_ORDERS_X, HEARTSEED_ORDERS_Y + 1);
    heartseed_prepare(g);
    ASSERT("the orders remain readable and guards are placed only once", g->heartseed_progress == HEARTSEED_ORDERS &&
        count == g->enemy_count && game_has_regional_interaction(g));
    game_descend(g);
    game_descend(g);
    Enemy *bound = guardian(g);
    ASSERT("the grove has a real Treefolk guardian within the regional enemy limit", bound && bound->active && bound->hp == 113 &&
        g->enemy_count == AREA_ENEMY_LIMIT && g->heartseed_guardian == HEARTSEED_BOUND);
    interact(g, HEARTSEED_X, HEARTSEED_Y + 1);
    ASSERT("the heartseed stays bound until its stakes are cut", !(g->heartseed_progress & HEARTSEED_TAKEN));
    interact(g, heartseed_stakes[0][0], heartseed_stakes[0][1] + 1);
    ASSERT("cutting a stake preserves a distinct cut state", g->heartseed_bindings == 1 &&
        g->map.tiles[heartseed_stakes[0][1]][heartseed_stakes[0][0]] == TILE_HEARTSEED_CUT_STAKE);
    if (bound) {
        bound->attack_phase = 1;
        bound->boss_attack = ELDER_ROOTS;
        bound->attack_target_x = 53;
        bound->attack_target_y = 35;
        bound->move_timer = 2;
    }
    ASSERT("partial bindings and a pending guardian attack save successfully", save_game(g, HEARTSEED_SLOT) && load_game(loaded, HEARTSEED_SLOT));
    bound = guardian(loaded);
    ASSERT("loading restores quest clues, bindings, and the same guardian warning", loaded->heartseed_progress == HEARTSEED_ORDERS &&
        loaded->heartseed_bindings == 1 && bound && bound->attack_phase == 1 && bound->move_timer == 2);
    interact(loaded, heartseed_stakes[1][0], heartseed_stakes[1][1] + 1);
    int xp = loaded->player.experience;
    interact(loaded, heartseed_stakes[2][0], heartseed_stakes[2][1] - 1);
    ASSERT("the final stake frees the guardian and cancels its attack once", bound && !bound->active && !bound->attack_phase &&
        loaded->heartseed_guardian == HEARTSEED_FREED && loaded->heartseed_bindings == 7 && loaded->player.experience > xp);
    xp = loaded->player.experience;
    heartseed_interact(loaded);
    ASSERT("repeating the final interaction cannot award guardian XP again", loaded->player.experience == xp);
    loaded->inventory_count = MAX_INVENTORY;
    interact(loaded, HEARTSEED_X, HEARTSEED_Y + 1);
    ASSERT("a full pack cannot block the tracked heartseed or ready state", loaded->inventory_count == MAX_INVENTORY &&
        loaded->heartseed_progress == 7 && loaded->heartseed_quest_state == 2);
    game_open_town_portal(loaded);
    ASSERT("the escape cache preserves the freed encounter and cut stakes", loaded->location == LOCATION_TOWN && !loaded->portal_active &&
        loaded->elderwood_cache[3].tiles[30][49] == TILE_HEARTSEED_CUT_STAKE);
    loaded->inventory_count = 0;
    int gold = loaded->gold;
    int score = loaded->score;
    loaded->player.x = TAMSIN_X + 1;
    loaded->player.y = TAMSIN_Y;
    heartseed_talk(loaded);
    heartseed_talk(loaded);
    ASSERT("Tamsin pays exactly one bounty independently of Elderthorn", loaded->heartseed_quest_state == 3 &&
        loaded->gold == gold + HEARTSEED_REWARD_GOLD && loaded->score == score + HEARTSEED_REWARD_SCORE && !loaded->elderwood_defeated);
    ASSERT("the completed journal retains all three objectives", quest_journal_get_entry(loaded, QUEST_TAB_COMPLETED, 0, &entry) &&
        entry.objective_complete[0] && entry.objective_complete[1] && entry.objective_complete[2]);
    test_restore_cached_floor(loaded);
    bound = guardian(loaded);
    ASSERT("loading the archived floor fixture never revives the released guardian", loaded->location == LOCATION_ELDERWOOD &&
        loaded->level == 4 && loaded->heartseed_guardian == HEARTSEED_FREED && bound && !bound->active);
    ASSERT("completed quest and freed guardian survive another save", save_game(loaded, HEARTSEED_SLOT) && load_game(g, HEARTSEED_SLOT) &&
        g->heartseed_quest_state == 3 && g->heartseed_guardian == HEARTSEED_FREED && guardian(g) && !guardian(g)->active);

    game_init(g);
    stage(g, 4);
    bound = guardian(g);
    if (bound) {
        g->player.x = bound->x;
        g->player.y = bound->y + 1;
        g->player.attack = 10000;
        action_resolve_player(g, (Action){ACTION_MOVE, bound->x, bound->y});
    }
    ASSERT("killing the bound guardian records the combat alternative", bound && !bound->active && g->heartseed_guardian == HEARTSEED_SLAIN);
    xp = g->player.experience;
    for (int i = 0; i < 3; i++) {
        interact(g, heartseed_stakes[i][0], heartseed_stakes[i][1] + 1);
    }
    interact(g, HEARTSEED_X, HEARTSEED_Y + 1);
    ASSERT("the slain guardian's seed remains recoverable without paying release XP", (g->heartseed_progress & HEARTSEED_TAKEN) &&
        g->heartseed_guardian == HEARTSEED_SLAIN && g->player.experience == xp && !g->heartseed_quest_state);
    game_ascend(g);
    game_ascend(g);
    interact(g, HEARTSEED_ORDERS_X, HEARTSEED_ORDERS_Y + 1);
    game_open_town_portal(g);
    accept(g);
    ASSERT("discoveries made before acceptance immediately satisfy the quest", g->heartseed_quest_state == 2 && g->heartseed_progress == 7);

    game_init(g);
    g->player.x = TAMSIN_X;
    g->player.y = TAMSIN_Y;
    g->map.tiles[TAMSIN_Y][TAMSIN_X] = TILE_TOWN_FLOOR;
    g->gold = 4321;
    g->alder_quest_state = 3;
    g->alder_wardens_rescued = 7;
    ASSERT("a version 203 town save gains Tamsin without losing progress", save_game(g, HEARTSEED_SLOT) && rewrite_save(NULL, 0, 1) &&
        load_game(loaded, HEARTSEED_SLOT) && loaded->map.tiles[TAMSIN_Y][TAMSIN_X] == TILE_NPC_TAMSIN &&
        loaded->player.x == TAMSIN_X + 1 && loaded->gold == 4321 && loaded->alder_quest_state == 3);
    stage(loaded, 4);
    loaded->map.tiles[34][45] = TILE_ELDERWOOD_FLOOR;
    loaded->elderwood_defeated = 1;
    for (int i = 0; i < loaded->enemy_count; i++) {
        if (heartseed_is_guardian(loaded, &loaded->enemies[i]) || strncmp(loaded->enemies[i].name, "Heartseed ", 10) == 0) {
            loaded->enemies[i].active = 0;
            loaded->enemies[i].name[0] = '\0';
        }
    }
    loaded->enemies[0].hp = 7;
    ASSERT("an active older forest save gains its grove while preserving damage and cut paths", save_game(loaded, HEARTSEED_SLOT) &&
        rewrite_save(NULL, 0, 1) && load_game(g, HEARTSEED_SLOT) && g->enemies[0].hp == 7 && g->gold == 4321 &&
        g->elderwood_defeated && g->map.tiles[34][45] == TILE_ELDERWOOD_FLOOR && guardian(g) && guardian(g)->active);
    ASSERT("malformed quest masks are rejected", save_game(g, HEARTSEED_SLOT) && rewrite_save("heartseed_bindings", 8, 0) && !load_game(loaded, HEARTSEED_SLOT));
    ASSERT("impossible ready states are rejected", save_game(g, HEARTSEED_SLOT) && rewrite_save("heartseed_quest_state", 2, 0) && !load_game(loaded, HEARTSEED_SLOT));

    game_init(g);
    stage(g, 4);
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 49, .y = 30,
        .underlying_tile = TILE_HEARTSEED_STAKE, .item = item_make_staff()};
    g->map.tiles[30][49] = TILE_ITEM;
    interact(g, 49, 31);
    ASSERT("a dropped item over a binding survives cutting its underlying stake", g->floor_items[0].active &&
        g->floor_items[0].underlying_tile == TILE_HEARTSEED_CUT_STAKE && g->map.tiles[30][49] == TILE_ITEM);
    ASSERT("quest object underlays survive saving with dropped equipment", save_game(g, HEARTSEED_SLOT) && load_game(loaded, HEARTSEED_SLOT) &&
        loaded->floor_item_count == 1 && loaded->floor_items[0].underlying_tile == TILE_HEARTSEED_CUT_STAKE && loaded->heartseed_bindings == 1);
    game_init(g);
    g->map.tiles[TAMSIN_Y][TAMSIN_X] = TILE_ITEM;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = TAMSIN_X, .y = TAMSIN_Y,
        .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_staff()};
    ASSERT("migration moves old dropped gear beside Tamsin without losing it", save_game(g, HEARTSEED_SLOT) && rewrite_save(NULL, 0, 1) &&
        load_game(loaded, HEARTSEED_SLOT) && loaded->floor_items[0].active && loaded->floor_items[0].x == TAMSIN_X + 1 &&
        loaded->map.tiles[TAMSIN_Y][TAMSIN_X] == TILE_NPC_TAMSIN && loaded->map.tiles[TAMSIN_Y][TAMSIN_X + 1] == TILE_ITEM);

    int connected = 1;
    for (int seed = 0; seed < 12; seed++) {
        srand((unsigned int)seed);
        game_init(g);
        stage(g, 2);
        connected &= reachable(g, HEARTSEED_ORDERS_X, HEARTSEED_ORDERS_Y);
        game_descend(g);
        game_descend(g);
        for (int i = 0; i < 3; i++) {
            connected &= reachable(g, heartseed_stakes[i][0], heartseed_stakes[i][1]);
        }
        connected &= reachable(g, HEARTSEED_X, HEARTSEED_Y);
        for (int i = 0; i < g->enemy_count; i++) {
            connected &= reachable(g, g->enemies[i].x, g->enemies[i].y);
        }
    }
    ASSERT("orders, all stakes, seed and every grove enemy are reachable across twelve seeds", connected);
    remove("saves/savegame_99284.json");
    free(loaded);
    free(g);
}
