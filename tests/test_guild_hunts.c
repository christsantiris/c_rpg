#include "test_utils.h"
#include "game/game.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define HUNT_SLOT 99168
#define HUNT_SAVE "saves/savegame_99168.json"
static GameState game;
static GameState loaded;
static Map original_map;

static void start(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    srand(9168);
}

static void approach_selene(void) {
    game_enter_town3(&game);
    game_enter_guild(&game);
    game.player.x = GUILD_SELENE_X;
    game.player.y = GUILD_SELENE_Y + 1;
}

static void accept(void) {
    approach_selene();
    game_talk_to_selene(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    game_leave_guild(&game);
}

static void visit(Location area) {
    if (game.location == LOCATION_FROSTFELL || game.location == LOCATION_GLASSDEEP || game.location == LOCATION_DESERT) {
        game_return_to_town(&game);
    }
    if (area == LOCATION_FROSTFELL) {
        game_enter_town3(&game);
        game_enter_frostfell(&game);
    } else {
        game_enter_town2(&game);
        if (area == LOCATION_GLASSDEEP) {
            game_enter_glassdeep(&game);
        } else {
            game_enter_desert(&game);
        }
    }
    int stage = area == LOCATION_GLASSDEEP ? 4 : 3;
    while (game.level < stage) {
        game_descend(&game);
    }
}

static int members(const GameState *g, int alive_only) {
    int count = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        if (game_hunt_enemy_index(&g->enemies[i]) >= 0 && (!alive_only || g->enemies[i].active)) {
            count++;
        }
    }
    return count;
}

static Enemy *leader(void) {
    for (int i = 0; i < game.enemy_count; i++) {
        if (game_is_hunt_leader(&game.enemies[i])) {
            return &game.enemies[i];
        }
    }
    return NULL;
}

static void kill_in_combat(Enemy *enemy) {
    if (!enemy || !enemy->active) {
        return;
    }
    game.player.attack = 10000;
    game.player.x = enemy->x;
    game.player.y = enemy->y + 1;
    enemy->hp = 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, enemy->x, enemy->y});
}

static void finish_group(void) {
    for (int i = 0; i < game.enemy_count; i++) {
        if (game_hunt_enemy_index(&game.enemies[i]) >= 0) {
            kill_in_combat(&game.enemies[i]);
        }
    }
}

static void test_flow(void) {
    start();
    game_enter_town3(&game);
    game_enter_guild(&game);
    ASSERT("Selene has a walkable approach and Orin remains in the Guild", game.map.tiles[GUILD_SELENE_Y][GUILD_SELENE_X] == TILE_NPC_SELENE &&
        !map_is_walkable(&game.map, GUILD_SELENE_X, GUILD_SELENE_Y) && map_is_walkable(&game.map, GUILD_SELENE_X, GUILD_SELENE_Y + 1) &&
        game.map.tiles[GUILD_ORIN_Y][GUILD_ORIN_X] == TILE_NPC_ORIN);
    game_talk_to_selene(&game);
    ASSERT("Selene cannot assign quests remotely", !game.hunt_quest_state && !game.dialogue_active);
    approach_selene();
    game_talk_to_selene(&game);
    ASSERT("the Guild hunt requires explicit acceptance", game_quest_offer_active(&game) && !game.hunt_quest_state);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_N, 0);
    ASSERT("declining leaves all hunts unassigned", !game.hunt_quest_state && !game.hunt_quest_encounters);
    accept();
    QuestJournalEntry entry;
    ASSERT("journal names all regions, stages, and Selene's exact return location", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "The Three Great Hunts") == 0 && entry.objective_count == 3 && entry.stages[0] == 3 && entry.stages[1] == 4 && entry.stages[2] == 3 &&
        strcmp(entry.objective_areas[0], "Frostfell Wastes") == 0 && strcmp(entry.objective_areas[1], "Glassdeep Caverns") == 0 &&
        strcmp(entry.objective_areas[2], "Sunscar Wastes") == 0 && strstr(entry.summary_line_2, "Selene in Rosemoor's Adventurer's Guild"));
    ASSERT("accepted quest saves inside the Guild", save_game(&game, HUNT_SLOT) && load_game(&loaded, HUNT_SLOT) &&
        loaded.hunt_quest_state == 1 && !loaded.hunt_quest_progress);
    visit(LOCATION_DESERT);
    Enemy *elite = leader();
    ASSERT("Sunscar three receives a named Djinn and two Mummies", elite && strcmp(elite->name, "Dunehex") == 0 && elite->type == ENEMY_DJINN &&
        !elite->is_boss && members(&game, 1) == 3 && game.enemies[game.enemy_count - 1].type == ENEMY_MUMMY);
    int original_hp = elite ? elite->max_hp : 0;
    int elite_slot = elite ? (int)(elite - game.enemies) : -1;
    if (elite) {
        int ordinary_hp = 0;
        for (int i = 0; i < game.enemy_count; i++) {
            if (game.enemies[i].type == ENEMY_DJINN && game_hunt_enemy_index(&game.enemies[i]) < 0) {
                ordinary_hp = game.enemies[i].max_hp;
                break;
            }
        }
        ASSERT("hunt leader receives a rounded 25 percent health increase", ordinary_hp && original_hp == ordinary_hp + (ordinary_hp + 3) / 4);
        elite->hp = 9;
    }
    game_refresh_quest_encounters(&game);
    ASSERT("refresh does not duplicate hunt members or reapply the health bonus", members(&game, 0) == 3 && leader() &&
        leader()->max_hp == original_hp && leader()->hp == 9);
    ASSERT("damaged hunt group survives save/load", save_game(&game, HUNT_SLOT) && load_game(&loaded, HUNT_SLOT) &&
        members(&loaded, 1) == 3 && loaded.hunt_quest_encounters == 896 && !loaded.hunt_quest_progress && elite_slot >= 0 &&
        loaded.enemies[elite_slot].hp == 9 && loaded.enemies[elite_slot].max_hp == original_hp);
    kill_in_combat(elite);
    ASSERT("killing only the leader does not complete its hunt", elite && !elite->active && members(&game, 1) == 2 && !game.hunt_quest_progress);
    game_ascend(&game);
    game_descend(&game);
    ASSERT("backtracking retains the dead leader and living companions", members(&game, 0) == 3 && members(&game, 1) == 2 && leader() && !leader()->active);
    finish_group();
    ASSERT("all Sunscar hunt members dying in combat completes only that hunt", game.hunt_quest_progress == 4 && game.hunt_quest_state == 1);
    int ordinary_alive = 0;
    for (int i = 0; i < game.enemy_count; i++) {
        ordinary_alive += game.enemies[i].active && game_hunt_enemy_index(&game.enemies[i]) < 0;
    }
    ASSERT("unrelated ordinary enemies can remain alive after a hunt completes", ordinary_alive > 0);
    game_refresh_quest_encounters(&game);
    ASSERT("completed hunts do not respawn", !members(&game, 1) && game.hunt_quest_progress == 4);
    game.glassdeep_quest_state = 1;
    visit(LOCATION_GLASSDEEP);
    ASSERT("Shardwarden and two Stalkers coexist with Orin's resonator encounter", leader() && leader()->type == ENEMY_SHARD_GOLEM &&
        members(&game, 1) == 3 && game.glassdeep_quest_encounters == 7 && game.enemies[game.enemy_count - 1].type == ENEMY_BLIND_STALKER);
    finish_group();
    ASSERT("Glassdeep hunt does not complete Orin's quest", game.hunt_quest_progress == 6 && game.glassdeep_quest_state == 1 && !game.glassdeep_quest_progress);
    visit(LOCATION_FROSTFELL);
    ASSERT("Rimefang's pack includes three Wolves and a Frost Wraith", leader() && leader()->type == ENEMY_ICE_WOLF &&
        members(&game, 1) == 4 && game.enemies[game.enemy_count - 1].type == ENEMY_FROST_WRAITH);
    finish_group();
    ASSERT("hunts complete in any order without regional boss victories", game.hunt_quest_state == 2 && game.hunt_quest_progress == 7 &&
        game.hunt_quest_encounters == 1023 && !game.defeated_bosses);
    game_open_town_portal(&game);
    approach_selene();
    int gold = game.gold;
    int score = game.score;
    game_talk_to_selene(&game);
    game_talk_to_selene(&game);
    ASSERT("Selene awards 200 gold and 1500 score exactly once", game.hunt_quest_state == 3 && game.gold == gold + 200 && game.score == score + 1500);
    ASSERT("completed journal preserves all three hunts", quest_journal_get_entry(&game, QUEST_TAB_COMPLETED, 0, &entry) &&
        entry.objective_complete[0] && entry.objective_complete[1] && entry.objective_complete[2]);
    ASSERT("claimed reward, cached hunts, and return portal survive saving", save_game(&game, HUNT_SLOT) && load_game(&loaded, HUNT_SLOT) &&
        loaded.hunt_quest_state == 3 && loaded.hunt_quest_progress == 7 && loaded.gold == game.gold && loaded.portal_active &&
        loaded.frostfell_cache[2].valid && loaded.glassdeep_cache[3].valid && loaded.desert_cache[2].valid);
    game_use_town_portal(&game);
    ASSERT("portal return keeps the final hunt defeated", game.location == LOCATION_FROSTFELL && game.level == 3 && !members(&game, 1));
    game_init(&game);
    ASSERT("new games reset the Guild hunt", !game.hunt_quest_state && !game.hunt_quest_progress && !game.hunt_quest_encounters);
}

static void test_explored_area(void) {
    start();
    visit(LOCATION_GLASSDEEP);
    ASSERT("hunts are absent before acceptance", !members(&game, 0));
    game.enemies[0].hp = 7;
    map_mark_explored(&game.map, game.player.x, game.player.y);
    original_map = game.map;
    game_open_town_portal(&game);
    accept();
    ASSERT("acceptance preserves explored maps, enemy damage, and portal", game.glassdeep_cache[3].valid && game.portal_active &&
        game.glassdeep_cache[3].enemies[0].hp == 7 && memcmp(&original_map, &game.glassdeep_cache[3].map, sizeof(Map)) == 0);
    game_use_town_portal(&game);
    ASSERT("revisiting an explored stage adds only the hunt encounter", members(&game, 1) == 3 && game.enemies[0].hp == 7);

    start();
    visit(LOCATION_FROSTFELL);
    for (int i = 0; i < game.enemy_count; i++) {
        game.enemies[i].active = 0;
        game.enemies[i].hp = 0;
    }
    game_update_level_progress(&game);
    game_open_town_portal(&game);
    accept();
    game_use_town_portal(&game);
    int active = 0;
    for (int i = 0; i < game.enemy_count; i++) {
        active += !!game.enemies[i].active;
    }
    ASSERT("a previously cleared stage gains its four hunt enemies without replenishing the ordinary population", active == 4 && members(&game, 1) == 4 && !game.level_cleared);
}

static void test_partial_encounter(void) {
    start();
    visit(LOCATION_DESERT);
    while (game.enemy_count < NON_ROAD_ENEMY_LIMIT) {
        game.enemies[game.enemy_count++] = game.enemies[0];
    }
    game.hunt_quest_state = 1;
    game_refresh_quest_encounters(&game);
    ASSERT("a full enemy array does not falsely credit an unplaced hunt", !game.hunt_quest_encounters && !game.hunt_quest_progress);
    game.enemies[0].active = 0;
    game_refresh_quest_encounters(&game);
    Enemy *elite = leader();
    ASSERT("partial spawning records the leader's placement separately", elite && members(&game, 1) == 1 && game.hunt_quest_encounters == 128);
    if (elite) {
        elite->hp = 0;
        elite->active = 0;
    }
    game_update_level_progress(&game);
    ASSERT("a dead leader cannot complete a partially placed hunt", !game.hunt_quest_progress);
    ASSERT("partial encounters survive saving and resume without reviving their leader", save_game(&game, HUNT_SLOT) && load_game(&loaded, HUNT_SLOT) &&
        loaded.hunt_quest_encounters == 384 && !loaded.hunt_quest_progress && members(&loaded, 1) == 1);
    game = loaded;
    for (int i = 0; i < game.enemy_count; i++) {
        if (game_hunt_enemy_index(&game.enemies[i]) < 0) {
            game.enemies[i].active = 0;
            game.enemies[i].hp = 0;
            break;
        }
    }
    game_refresh_quest_encounters(&game);
    ASSERT("remaining companions spawn without duplicating earlier members", game.hunt_quest_encounters == 896 && members(&game, 1) == 2 && !leader());
    finish_group();
    ASSERT("partial hunt completes after every assigned member is defeated", game.hunt_quest_progress == 4);
}

static int legacy_fixture(void) {
    if (!save_game(&game, HUNT_SLOT)) {
        return 0;
    }
    FILE *file = fopen(HUNT_SAVE, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc((size_t)size + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    size_t read = fread(buffer, 1, (size_t)size, file);
    buffer[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 122);
    cJSON_DeleteItemFromObject(root, "hunt_quest_state");
    cJSON_DeleteItemFromObject(root, "hunt_quest_progress");
    cJSON_DeleteItemFromObject(root, "hunt_quest_encounters");
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json) {
        return 0;
    }
    file = fopen(HUNT_SAVE, "wb");
    int written = file && fputs(json, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(json);
    return written;
}

static void test_migration(void) {
    start();
    visit(LOCATION_GLASSDEEP);
    game.enemies[0].hp = 7;
    game_open_town_portal(&game);
    approach_selene();
    game.player.y = GUILD_SELENE_Y;
    game.map.tiles[GUILD_SELENE_Y][GUILD_SELENE_X] = TILE_ITEM;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = GUILD_SELENE_X, .y = GUILD_SELENE_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_short_sword()};
    game.gold = 617;
    game.watchfire_quest_state = 1;
    game.watchfire_quest_progress = 2;
    game.watchfire_quest_encounters = 2;
    game.defeated_bosses |= 1 << LOCATION_DESERT;
    ASSERT("version 122 Guild saves gain an available hunt quest", legacy_fixture() && load_game(&loaded, HUNT_SLOT) &&
        !loaded.hunt_quest_state && !loaded.hunt_quest_progress && !loaded.hunt_quest_encounters);
    ASSERT("migration preserves other quests, gold, bosses, explored enemies, and portals", loaded.gold == 617 &&
        loaded.watchfire_quest_state == 1 && loaded.watchfire_quest_progress == 2 && loaded.portal_active &&
        loaded.glassdeep_cache[3].valid && loaded.glassdeep_cache[3].enemies[0].hp == 7 && (loaded.defeated_bosses & (1 << LOCATION_DESERT)));
    ASSERT("migration adds Selene and safely moves overlapping player and loot", loaded.map.tiles[GUILD_SELENE_Y][GUILD_SELENE_X] == TILE_NPC_SELENE &&
        loaded.player.y == GUILD_SELENE_Y + 1 && loaded.floor_items[0].active && loaded.floor_items[0].y == GUILD_SELENE_Y + 1);
    game.hunt_quest_state = 1;
    game.hunt_quest_progress = 8;
    ASSERT("invalid hunt progress is rejected", save_game(&game, HUNT_SLOT) && !load_game(&loaded, HUNT_SLOT));
    game.hunt_quest_progress = 0;
    game.hunt_quest_encounters = 1024;
    ASSERT("invalid hunt member masks are rejected", save_game(&game, HUNT_SLOT) && !load_game(&loaded, HUNT_SLOT));
}

static void test_access(void) {
    static unsigned char seen[MAP_H][MAP_W];
    static int queue[MAP_W * MAP_H];
    const Location areas[3] = {LOCATION_FROSTFELL, LOCATION_GLASSDEEP, LOCATION_DESERT};
    const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    int reachable = 1;
    int safe = 1;
    for (int seed = 0; seed < 32; seed++) {
        start();
        srand((unsigned int)seed);
        accept();
        for (int area = 0; area < 3; area++) {
            visit(areas[area]);
            memset(seen, 0, sizeof(seen));
            int head = 0;
            int tail = 0;
            queue[tail++] = game.player.y * MAP_W + game.player.x;
            seen[game.player.y][game.player.x] = 1;
            while (head < tail) {
                int cell = queue[head++];
                for (int side = 0; side < 4; side++) {
                    int tx = cell % MAP_W + offsets[side][0];
                    int ty = cell / MAP_W + offsets[side][1];
                    if (tx < 0 || tx >= MAP_W || ty < 0 || ty >= MAP_H || seen[ty][tx] || !map_is_walkable(&game.map, tx, ty)) {
                        continue;
                    }
                    seen[ty][tx] = 1;
                    queue[tail++] = ty * MAP_W + tx;
                }
            }
            safe &= members(&game, 1) == (area == 0 ? 4 : 3);
            for (int i = 0; i < game.enemy_count; i++) {
                const Enemy *enemy = &game.enemies[i];
                if (game_hunt_enemy_index(enemy) >= 0) {
                    reachable &= seen[enemy->y][enemy->x];
                    safe &= (enemy->x != game.player.x || enemy->y != game.player.y) &&
                        (enemy->x != game.map.stairs_down_x || enemy->y != game.map.stairs_down_y);
                }
            }
        }
    }
    ASSERT("all hunt members are reachable across 32 generated expeditions", reachable);
    ASSERT("complete hunt groups spawn away from entrances and exits", safe);
}

void test_guild_hunts(void) {
    printf("The Three Great Hunts tests:\n");
    int unused = !save_exists(HUNT_SLOT);
    ASSERT("hunt test fixture does not overwrite a player save", unused);
    if (!unused) {
        return;
    }
    test_flow();
    test_explored_area();
    test_partial_encounter();
    test_migration();
    test_access();
    remove(HUNT_SAVE);
}
