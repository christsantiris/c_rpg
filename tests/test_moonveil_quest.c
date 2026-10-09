#include "test_utils.h"
#include "game/game.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define MOONSEED_SLOT 99144
#define MOONSEED_SAVE "saves/savegame_99144.json"
static GameState game;
static GameState loaded;

static void start(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    srand(9144);
}

static void accept(void) {
    game_enter_town3(&game);
    game.player.x = LIORA_TOWN_X;
    game.player.y = LIORA_TOWN_Y + 1;
    game_talk_to_liora(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
}

static void stage(int level) {
    while (game.level < level) {
        game_descend(&game);
    }
    while (game.level > level) {
        game_ascend(&game);
    }
}

static int find_objective(TileType tile, int *x, int *y) {
    for (int ty = 0; ty < MAP_H; ty++) {
        for (int tx = 0; tx < MAP_W; tx++) {
            if (game.map.tiles[ty][tx] == tile) {
                *x = tx;
                *y = ty;
                return 1;
            }
        }
    }
    return 0;
}

static int approach(TileType tile) {
    int x = -1;
    int y = -1;
    if (!find_objective(tile, &x, &y)) {
        ASSERT("quest stage contains the expected objective", 0);
        return 0;
    }
    game.player.x = x;
    game.player.y = y;
    return 1;
}

static int guards(const GameState *g, const char *name, int alive) {
    int count = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if ((!alive || g->enemies[i].active) && strcmp(g->enemies[i].name, name) == 0) {
            count++;
        }
    }
    return count;
}

static void clear_defenders(void) {
    for (int i = 0; i < game.enemy_count; i++) {
        game.enemies[i].active = 0;
        game.enemies[i].hp = 0;
    }
}

static int bloom_tiles(const Map *map) {
    int count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            count += map->tiles[y][x] == TILE_MOONVEIL_BLOSSOMS;
        }
    }
    return count;
}

static void test_seed_first(void) {
    start();
    game_enter_town3(&game);
    ASSERT("Liora has a blocked NPC tile and a walkable approach in Rosemoor's center", game.map.tiles[LIORA_TOWN_Y][LIORA_TOWN_X] == TILE_NPC_LIORA &&
        !map_is_walkable(&game.map, LIORA_TOWN_X, LIORA_TOWN_Y) && map_is_walkable(&game.map, LIORA_TOWN_X, LIORA_TOWN_Y + 1));
    game_talk_to_liora(&game);
    ASSERT("Liora cannot assign remotely", !game.moonveil_quest_state);
    game.player.x = LIORA_TOWN_X;
    game.player.y = LIORA_TOWN_Y + 1;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Action does not replace talking to Liora", !game.moonveil_quest_state);
    game_talk_to_liora(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("Liora explains Rosemoor's west-gate route and town-center return", game.moonveil_quest_state == 1 &&
        strstr(game.dialogue_text, "through Rosemoor's west gate") && strstr(game.dialogue_text, "Rosemoor's town center") &&
        !game.alder_quest_state && !game.mara_quest_state && !game.frostfell_quest_state);
    ASSERT("town-center acceptance survives saving", save_game(&game, MOONSEED_SLOT) && load_game(&loaded, MOONSEED_SLOT) &&
        loaded.moonveil_quest_state == 1 && loaded.map.tiles[LIORA_TOWN_Y][LIORA_TOWN_X] == TILE_NPC_LIORA);
    game_enter_moonveil(&game);
    int x = -1;
    int y = -1;
    ASSERT("stage one has no Moonseed quest objects", !find_objective(TILE_MOONVEIL_SEED_POD, &x, &y) && !game_has_moonveil_interaction(&game));
    stage(2);
    ASSERT("seed pod has one Fey Trickster and two Giant Moth defenders", guards(&game, "Moonseed Guardian", 1) == 3 &&
        game.enemies[game.enemy_count - 3].type == ENEMY_FEY_TRICKSTER && game.enemies[game.enemy_count - 2].type == ENEMY_GIANT_MOTH &&
        game.enemies[game.enemy_count - 1].type == ENEMY_GIANT_MOTH);
    clear_defenders();
    if (!approach(TILE_MOONVEIL_SEED_POD)) {
        return;
    }
    int inventory = game.inventory_count;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Action collects the seed and removes its pod without using inventory", game.moonveil_quest_progress == 1 &&
        game.inventory_count == inventory && game.map.tiles[game.player.y][game.player.x] == TILE_MOONVEIL_FLOOR);
    stage(3);
    ASSERT("spring has one Carnivorous Flower defender", guards(&game, "Moonwater Guardian", 1) == 1 &&
        game.enemies[game.enemy_count - 1].type == ENEMY_LIVING_FLOWER);
    clear_defenders();
    if (!approach(TILE_MOONVEIL_SPRING)) {
        return;
    }
    game_interact_moonveil(&game);
    game_interact_moonveil(&game);
    ASSERT("moonwater is collected once without removing the spring", game.moonveil_quest_progress == 3 && game.inventory_count == inventory &&
        game.map.tiles[game.player.y][game.player.x] == TILE_MOONVEIL_SPRING);
    stage(4);
    ASSERT("planting circle has two Thorn Guardian defenders", guards(&game, "Mooncircle Guardian", 1) == 2 &&
        game.enemies[game.enemy_count - 1].type == ENEMY_THORN_GUARDIAN && game.enemies[game.enemy_count - 2].type == ENEMY_THORN_GUARDIAN);
    if (!approach(TILE_MOONVEIL_PLANTING_CIRCLE)) {
        return;
    }
    game_interact_moonveil(&game);
    ASSERT("resources alone cannot bypass the circle's living defenders", game.moonveil_quest_progress == 3 && game.moonveil_quest_state == 1);
    clear_defenders();
    int bx = game.player.x;
    int by = game.player.y;
    // Add an adjacent tangle to verify that restoration actually clears it.
    game.map.tiles[by][bx + 1] = TILE_MOONVEIL_WALL;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("planting blooms the Moonflower and restores the clearing", game.moonveil_quest_state == 2 && game.moonveil_quest_progress == 7 &&
        game.map.tiles[by][bx] == TILE_MOONVEIL_MOONFLOWER && game.map.tiles[by][bx + 1] == TILE_MOONVEIL_BLOSSOMS && bloom_tiles(&game.map) > 0);
    ASSERT("restoration requires no Thorn Regent victory or inventory slot", !(game.defeated_bosses & (1 << LOCATION_MOONVEIL)) && game.inventory_count == inventory);
    QuestJournalEntry entry;
    ASSERT("journal lists all three objectives, stages and rewards", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "The Stolen Moonseed") == 0 && strcmp(entry.giver, "Botanist Liora") == 0 && entry.objective_count == 3 &&
        entry.stages[0] == 2 && entry.stages[1] == 3 && entry.stages[2] == 4 && entry.objective_complete[0] &&
        entry.objective_complete[1] && entry.objective_complete[2] && entry.reward_gold == 90 && entry.reward_score == 700);
    int blooms = bloom_tiles(&game.map);
    ASSERT("ready quest and blooming terrain survive saving", save_game(&game, MOONSEED_SLOT) && load_game(&loaded, MOONSEED_SLOT) &&
        loaded.moonveil_quest_state == 2 && loaded.moonveil_quest_progress == 7 && loaded.moonveil_quest_encounters == 7 &&
        loaded.map.tiles[by][bx] == TILE_MOONVEIL_MOONFLOWER && bloom_tiles(&loaded.map) == blooms);
    stage(3);
    stage(4);
    ASSERT("backtracking keeps the Moonflower and defeated guardians", game.map.tiles[by][bx] == TILE_MOONVEIL_MOONFLOWER &&
        bloom_tiles(&game.map) == blooms && !guards(&game, "Mooncircle Guardian", 1));
    game.player.x = bx;
    game.player.y = by;
    game_open_town_portal(&game);
    ASSERT("portal over the Moonflower preserves the restored clearing", save_game(&game, MOONSEED_SLOT) && load_game(&loaded, MOONSEED_SLOT) &&
        loaded.portal_origin_tile == TILE_MOONVEIL_MOONFLOWER && loaded.moonveil_cache[3].map.tiles[by][bx] == TILE_MOONVEIL_MOONFLOWER);
    game = loaded;
    game_use_town_portal(&game);
    ASSERT("return portal restores the Moonflower and blooming terrain", game.location == LOCATION_MOONVEIL && game.level == 4 &&
        game.map.tiles[by][bx] == TILE_MOONVEIL_MOONFLOWER && bloom_tiles(&game.map) == blooms);
    game_return_to_town(&game);
    game_enter_town3(&game);
    game.player.x = LIORA_TOWN_X;
    game.player.y = LIORA_TOWN_Y + 1;
    int gold = game.gold;
    int score = game.score;
    game_talk_to_liora(&game);
    ASSERT("Liora awards 90 gold and 700 score once", game.moonveil_quest_state == 3 && game.gold == gold + 90 && game.score == score + 700);
    game_talk_to_liora(&game);
    ASSERT("repeat conversation cannot duplicate the reward", game.gold == gold + 90 && game.score == score + 700);
    ASSERT("completed journal retains all restored objectives", quest_journal_get_entry(&game, QUEST_TAB_COMPLETED, 0, &entry) &&
        entry.state == 3 && entry.objective_complete[0] && entry.objective_complete[1] && entry.objective_complete[2]);
    ASSERT("completed quest and reward survive saving in Rosemoor", save_game(&game, MOONSEED_SLOT) && load_game(&loaded, MOONSEED_SLOT) &&
        loaded.moonveil_quest_state == 3 && loaded.gold == gold + 90 && loaded.score == score + 700);
    game_enter_moonveil(&game);
    stage(4);
    ASSERT("later visits retain the Moonflower and clearing", game.map.tiles[by][bx] == TILE_MOONVEIL_MOONFLOWER && bloom_tiles(&game.map) == blooms);
    game_return_to_town(&game);
    memset(game.moonveil_cache, 0, sizeof(game.moonveil_cache));
    game_enter_moonveil(&game);
    stage(2);
    ASSERT("a regenerated seed stage cannot supply another Moonseed", !find_objective(TILE_MOONVEIL_SEED_POD, &x, &y) && !guards(&game, "Moonseed Guardian", 0));
    stage(4);
    ASSERT("a regenerated final quest stage restores the flower without new quest guards", find_objective(TILE_MOONVEIL_MOONFLOWER, &x, &y) &&
        bloom_tiles(&game.map) > 0 && !guards(&game, "Mooncircle Guardian", 0) && game.moonveil_quest_progress == 7);
}

static void test_water_first_and_guards(void) {
    start();
    accept();
    game_enter_moonveil(&game);
    stage(3);
    if (!approach(TILE_MOONVEIL_SPRING)) {
        return;
    }
    int x = game.player.x;
    int y = game.player.y;
    game_interact_moonveil(&game);
    ASSERT("living spring defenders prevent gathering water", !game.moonveil_quest_progress);
    game.enemies[game.enemy_count - 1].hp = 7;
    game_refresh_quest_encounters(&game);
    ASSERT("refresh retains guardian health without duplication", guards(&game, "Moonwater Guardian", 0) == 1 &&
        game.enemies[game.enemy_count - 1].hp == 7);
    ASSERT("damaged spring encounter saves and loads", save_game(&game, MOONSEED_SLOT) && load_game(&loaded, MOONSEED_SLOT));
    ASSERT("load retains damaged guardians without duplication", guards(&loaded, "Moonwater Guardian", 0) == 1 &&
        loaded.enemies[loaded.enemy_count - 1].hp == 7);
    for (int i = 0; i < game.enemy_count; i++) {
        if (strcmp(game.enemies[i].name, "Moonwater Guardian") == 0) {
            game.enemies[i].x = 1;
            game.enemies[i].y = 1;
        } else {
            game.enemies[i].active = 0;
        }
    }
    game_interact_moonveil(&game);
    ASSERT("luring the flower away cannot bypass the spring encounter", !game.moonveil_quest_progress);
    clear_defenders();
    game.floor_item_count = 2;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = x, .y = y,
            .item = item_make_health_potion(), .underlying_tile = TILE_MOONVEIL_SPRING};
    }
    game.map.tiles[y][x] = TILE_ITEM;
    game_interact_moonveil(&game);
    ASSERT("water can be gathered first under overlapping loot without destroying items", game.moonveil_quest_progress == 2 &&
        game.floor_items[0].active && game.floor_items[1].active && game.map.tiles[y][x] == TILE_ITEM &&
        game.floor_items[0].underlying_tile == TILE_MOONVEIL_SPRING);
    game.floor_item_count = 0;
    game.map.tiles[y][x] = TILE_MOONVEIL_SPRING;
    stage(4);
    clear_defenders();
    if (!approach(TILE_MOONVEIL_PLANTING_CIRCLE)) {
        return;
    }
    game_interact_moonveil(&game);
    ASSERT("moonwater alone cannot restore the circle", game.moonveil_quest_progress == 2 && game.moonveil_quest_state == 1 && !bloom_tiles(&game.map));
    stage(2);
    clear_defenders();
    if (!approach(TILE_MOONVEIL_SEED_POD)) {
        return;
    }
    game.inventory_count = MAX_INVENTORY;
    game_interact_moonveil(&game);
    ASSERT("the Moonseed can be collected after water with a full inventory", game.moonveil_quest_progress == 3 && game.inventory_count == MAX_INVENTORY);
    stage(4);
    if (!approach(TILE_MOONVEIL_PLANTING_CIRCLE)) {
        return;
    }
    x = game.player.x;
    y = game.player.y;
    game.floor_item_count = 2;
    for (int i = 0; i < 2; i++) {
        game.floor_items[i] = (FloorItem){.active = 1, .x = x, .y = y,
            .item = item_make_health_potion(), .underlying_tile = TILE_MOONVEIL_PLANTING_CIRCLE};
    }
    game.map.tiles[y][x] = TILE_ITEM;
    game_interact_moonveil(&game);
    ASSERT("planting beneath loot updates every underlay to the Moonflower", game.moonveil_quest_state == 2 && game.moonveil_quest_progress == 7 &&
        game.floor_items[0].active && game.floor_items[1].active && game.map.tiles[y][x] == TILE_ITEM &&
        game.floor_items[0].underlying_tile == TILE_MOONVEIL_MOONFLOWER && game.floor_items[1].underlying_tile == TILE_MOONVEIL_MOONFLOWER);
}

static void test_existing_progress(void) {
    start();
    game_enter_moonveil(&game);
    stage(2);
    game.enemies[0].hp = 5;
    int count = game.enemy_count;
    map_mark_explored(&game.map, game.map.stairs_up_x, game.map.stairs_up_y);
    game_open_town_portal(&game);
    accept();
    ASSERT("accepting Liora's quest retains explored maps, enemies, and an existing garden portal", game.moonveil_cache[1].valid && game.portal_active &&
        game.moonveil_cache[1].enemy_count == count && game.moonveil_cache[1].enemies[0].hp == 5 &&
        map_is_explored(&game.moonveil_cache[1].map, game.moonveil_cache[1].map.stairs_up_x, game.moonveil_cache[1].map.stairs_up_y));
    game_use_town_portal(&game);
    int x = -1;
    int y = -1;
    ASSERT("returning to an explored stage adds its quest encounter without resetting old enemies", game.location == LOCATION_MOONVEIL && game.level == 2 &&
        game.enemies[0].hp == 5 && find_objective(TILE_MOONVEIL_SEED_POD, &x, &y) && guards(&game, "Moonseed Guardian", 1) == 3);
    clear_defenders();
    if (!approach(TILE_MOONVEIL_SEED_POD)) {
        return;
    }
    game_interact_moonveil(&game);
    ASSERT("partial seed progress and dead guardians persist in saves", save_game(&game, MOONSEED_SLOT) && load_game(&loaded, MOONSEED_SLOT) &&
        loaded.moonveil_quest_progress == 1 && loaded.moonveil_quest_state == 1 && !guards(&loaded, "Moonseed Guardian", 1));
    game.moonveil_quest_state = 3;
    game.moonveil_quest_progress = 7;
    game_init(&game);
    ASSERT("a new game resets Liora's quest and garden caches", !game.moonveil_quest_state && !game.moonveil_quest_progress &&
        !game.moonveil_quest_encounters && !game.moonveil_cache[1].valid);
}

static void test_quest_access(void) {
    static unsigned char seen[MAP_H][MAP_W];
    static int queue[MAP_H * MAP_W];
    int reachable = 1;
    int springs = 1;
    int safe = 1;
    for (int seed = 0; seed < 32 && reachable && springs && safe; seed++) {
        start();
        srand((unsigned int)seed);
        game.moonveil_quest_state = 1;
        game_enter_moonveil(&game);
        for (int level = 2; level <= 4; level++) {
            game_descend(&game);
            TileType tile = level == 2 ? TILE_MOONVEIL_SEED_POD : level == 3 ? TILE_MOONVEIL_SPRING : TILE_MOONVEIL_PLANTING_CIRCLE;
            int x = -1;
            int y = -1;
            if (!find_objective(tile, &x, &y)) {
                reachable = 0;
                break;
            }
            memset(seen, 0, sizeof(seen));
            int head = 0;
            int tail = 0;
            queue[tail++] = game.map.stairs_up_y * MAP_W + game.map.stairs_up_x;
            seen[game.map.stairs_up_y][game.map.stairs_up_x] = 1;
            const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
            while (head < tail) {
                int cell = queue[head++];
                for (int i = 0; i < 4; i++) {
                    int tx = cell % MAP_W + offsets[i][0];
                    int ty = cell / MAP_W + offsets[i][1];
                    if (tx < 0 || tx >= MAP_W || ty < 0 || ty >= MAP_H || seen[ty][tx] || !map_is_walkable(&game.map, tx, ty)) {
                        continue;
                    }
                    seen[ty][tx] = 1;
                    queue[tail++] = ty * MAP_W + tx;
                }
            }
            reachable &= seen[y][x] && seen[game.map.stairs_down_y][game.map.stairs_down_x];
            if (level == 3) {
                springs &= game.map.tiles[y - 1][x] == TILE_MOONVEIL_POOL || game.map.tiles[y + 1][x] == TILE_MOONVEIL_POOL ||
                    game.map.tiles[y][x - 1] == TILE_MOONVEIL_POOL || game.map.tiles[y][x + 1] == TILE_MOONVEIL_POOL;
            }
            for (int i = 0; i < game.enemy_count; i++) {
                Enemy *enemy = &game.enemies[i];
                if (enemy->active && strstr(enemy->name, "Guardian")) {
                    safe &= seen[enemy->y][enemy->x] && (enemy->x != game.player.x || enemy->y != game.player.y) &&
                        (enemy->x != x || enemy->y != y);
                }
            }
        }
    }
    ASSERT("quest objectives and exits remain reachable across 32 garden expeditions", reachable);
    ASSERT("moonwater is gathered from a walkable bank beside an impassable pool", springs);
    ASSERT("quest defenders occupy reachable ground away from the player and objectives", safe);
}

static void test_legacy_save(void) {
    start();
    game_enter_moonveil(&game);
    stage(2);
    game.enemies[0].hp = 5;
    game_open_town_portal(&game);
    game.location = LOCATION_TOWN2;
    game_enter_inn(&game);
    game.gold = 731;
    game.player.hp = 63;
    game.defeated_bosses |= 1 << LOCATION_MOONVEIL;
    game.max_moonveil_level_reached = 5;
    game.player.x = LIORA_INN_X;
    game.player.y = LIORA_INN_Y;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = LIORA_INN_X, .y = LIORA_INN_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_short_sword()};
    game.map.tiles[LIORA_INN_Y][LIORA_INN_X] = TILE_ITEM;
    ASSERT("legacy Inn fixture saves", save_game(&game, MOONSEED_SLOT));
    FILE *file = fopen(MOONSEED_SAVE, "rb");
    if (!file) {
        return;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc((size_t)size + 1);
    fread(buffer, 1, (size_t)size, file);
    buffer[size] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (!root) {
        return;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 94);
    cJSON_DeleteItemFromObject(root, "moonveil_quest_state");
    cJSON_DeleteItemFromObject(root, "moonveil_quest_progress");
    cJSON_DeleteItemFromObject(root, "moonveil_quest_encounters");
    char *json = cJSON_Print(root);
    file = fopen(MOONSEED_SAVE, "wb");
    fputs(json, file);
    fclose(file);
    free(json);
    cJSON_Delete(root);
    ASSERT("version 94 initializes the quest without losing character, portal, boss, or cached enemy progress", load_game(&loaded, MOONSEED_SLOT) &&
        !loaded.moonveil_quest_state && !loaded.moonveil_quest_progress && !loaded.moonveil_quest_encounters &&
        loaded.gold == 731 && loaded.player.hp == 63 && loaded.portal_active && loaded.max_moonveil_level_reached == 5 &&
        (loaded.defeated_bosses & (1 << LOCATION_MOONVEIL)) && loaded.moonveil_cache[1].valid && loaded.moonveil_cache[1].enemies[0].hp == 5);
    ASSERT("legacy Inn gains Zara and safely moves overlapping player and loot beside her", loaded.map.tiles[ZARA_INN_Y][ZARA_INN_X] == TILE_NPC_GUILD_SEEKER &&
        loaded.player.y == LIORA_INN_Y + 1 && loaded.floor_items[0].active && loaded.floor_items[0].y == LIORA_INN_Y + 1 &&
        loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR && loaded.map.tiles[LIORA_INN_Y + 1][LIORA_INN_X] == TILE_ITEM);
    game = loaded;
    game_enter_town3(&game);
    game.player.x = LIORA_TOWN_X;
    game.player.y = LIORA_TOWN_Y + 1;
    game_talk_to_liora(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("Liora offers the quest after a previous Thorn Regent victory", game.moonveil_quest_state == 1);
    ASSERT("migrated saves rewrite and reload successfully", save_game(&game, MOONSEED_SLOT) && load_game(&loaded, MOONSEED_SLOT));
    int x;
    int y;
    map_generate_inn(&loaded.map, &x, &y);
    ASSERT("Stillbury's Inn houses Zara", loaded.map.tiles[ZARA_INN_Y][ZARA_INN_X] == TILE_NPC_GUILD_SEEKER);
    map_generate_town3(&loaded.map, &x, &y);
    ASSERT("Rosemoor's center houses Liora", loaded.map.tiles[LIORA_TOWN_Y][LIORA_TOWN_X] == TILE_NPC_LIORA);
    map_generate_tavern(&loaded.map, &x, &y);
    ASSERT("Oakhaven's Tavern no longer houses Liora", loaded.map.tiles[LIORA_INN_Y][LIORA_INN_X] == TILE_TAVERN_FLOOR);
    map_generate_guild(&loaded.map, &x, &y);
    ASSERT("Rosemoor's Guild does not gain Liora", loaded.map.tiles[LIORA_INN_Y][LIORA_INN_X] != TILE_NPC_LIORA);
}

void test_moonveil_quest(void) {
    printf("Moonveil Gardens: The Stolen Moonseed tests:\n");
    test_seed_first();
    test_water_first_and_guards();
    test_existing_progress();
    test_quest_access();
    test_legacy_save();
    remove(MOONSEED_SAVE);
}
