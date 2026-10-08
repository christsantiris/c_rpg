#include "test_utils.h"
#include "game/catacombs.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define OSWIN_SLOT 99146
#define OSWIN_SAVE "saves/savegame_99146.json"
static GameState game;
static GameState loaded;

static void start(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    srand(9146);
}

static void approach_oswin(void) {
    game_enter_town4(&game);
    game_enter_town_hall(&game);
    game.player.x = HALL_OSWIN_X;
    game.player.y = HALL_OSWIN_Y + 1;
}

static void stage(int level) {
    while (game.level < level) {
        game_descend(&game);
    }
    while (game.level > level) {
        game_ascend(&game);
    }
}

static void approach_memorial(void) {
    const Room *room = &game.map.rooms[CATACOMBS_MEMORIAL_ROOM];
    game.player.x = room->x + 2;
    game.player.y = room->y + 1;
}

static TileType memorial_tile(const Map *map) {
    const Room *room = &map->rooms[CATACOMBS_MEMORIAL_ROOM];
    return map->tiles[room->y + 1][room->x + 1];
}

static void approach_ledger(void) {
    const Room *room = &game.map.rooms[game.map.room_count - 1];
    game.player.x = room->x + 2;
    game.player.y = room->y + room->h - 2;
}

static int guards(const GameState *g, int active) {
    int count = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        count += (!active || g->enemies[i].active) && strstr(g->enemies[i].name, "Memorial Guard") != NULL;
    }
    return count;
}

static void test_quest(void) {
    start();
    game_talk_to_oswin(&game);
    ASSERT("Oswin cannot assign his quest outside Town Hall", !game.catacombs_quest_state);
    game_enter_town4(&game);
    game.player.x = TOWN4_HALL_DOOR_X;
    game.player.y = TOWN4_HALL_DOOR_Y + 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, TOWN4_HALL_DOOR_X, TOWN4_HALL_DOOR_Y});
    ASSERT("Town Hall contains Oswin with a walkable approach alongside Hadrin", game.location == LOCATION_TOWN_HALL &&
        game.map.tiles[HALL_OSWIN_Y][HALL_OSWIN_X] == TILE_NPC_OSWIN && !map_is_walkable(&game.map, HALL_OSWIN_X, HALL_OSWIN_Y) &&
        map_is_walkable(&game.map, HALL_OSWIN_X, HALL_OSWIN_Y + 1) && game.map.tiles[HALL_MARA_Y][HALL_MARA_X] == TILE_TAVERN_FLOOR &&
        game.map.tiles[HALL_STEWARD_Y][HALL_STEWARD_X] == TILE_NPC_STEWARD);
    game_talk_to_oswin(&game);
    ASSERT("Oswin cannot assign remotely inside Town Hall", !game.catacombs_quest_state);
    game.player.x = HALL_OSWIN_X;
    game.player.y = HALL_OSWIN_Y + 1;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("Action does not replace talking to Oswin", !game.catacombs_quest_state);
    game_talk_to_oswin(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("Oswin assigns Rest for the Forgotten with the Crown Road and south-gate route", game.catacombs_quest_state == 1 &&
        !game.catacombs_quest_progress && !game.catacombs_quest_encounters && !game.mara_quest_state && !game.emberforge_quest_state &&
        game.dialogue_x == HALL_OSWIN_X && game.dialogue_y == HALL_OSWIN_Y && strstr(game.dialogue_text, "Crown Road West") &&
        strstr(game.dialogue_text, "castle's south gate") && strstr(game.dialogue_text, "floor 5"));
    QuestJournalEntry entry;
    ASSERT("journal shows all four objectives and their floors", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "Rest for the Forgotten") == 0 && entry.objective_count == 4 && entry.stages[0] == 2 &&
        entry.stages[1] == 3 && entry.stages[2] == 4 && entry.stages[3] == 5 && !entry.objective_complete[3] &&
        entry.reward_gold == 150 && entry.reward_score == 1500);
    ASSERT("accepted quest and Oswin's placement survive saving", save_game(&game, OSWIN_SLOT) && load_game(&loaded, OSWIN_SLOT) &&
        loaded.catacombs_quest_state == 1 && loaded.map.tiles[HALL_OSWIN_Y][HALL_OSWIN_X] == TILE_NPC_OSWIN);
    game_leave_town_hall(&game);
    game_enter_catacombs(&game);
    ASSERT("approach floor has no memorial encounter", !guards(&game, 0) && memorial_tile(&game.map) == TILE_OSSUARY_BRAZIER);
    stage(2);
    ASSERT("Soldiers' memorial has an Ancient Skeleton and Bone Sentinel", memorial_tile(&game.map) == TILE_MEMORIAL_BRAZIER &&
        guards(&game, 1) == 2 && game.enemies[game.enemy_count - 2].type == ENEMY_ANCIENT_SKELETON &&
        game.enemies[game.enemy_count - 1].type == ENEMY_BONE_SENTINEL && game.catacombs_quest_encounters == 1);
    game_refresh_quest_encounters(&game);
    ASSERT("refresh never duplicates memorial guards", guards(&game, 0) == 2);
    approach_memorial();
    Enemy *bones = &game.enemies[game.enemy_count - 2];
    bones->active = 0;
    bones->hp = 0;
    catacombs_record_death(&game, bones);
    ASSERT("marked memorial braziers retain the skeleton resurrection mechanic", bones->revive_timer == 2 &&
        catacombs_lit_braziers(&game.map, CATACOMBS_MEMORIAL_ROOM) == 1);
    int inventory = game.inventory_count;
    game.inventory_count = MAX_INVENTORY;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    game.inventory_count = inventory;
    catacombs_tick(&game);
    ASSERT("silencing before defeating all guards needs no inventory space and cancels resurrection", game.catacombs_quest_progress == 1 &&
        memorial_tile(&game.map) == TILE_MEMORIAL_COLD && guards(&game, 1) == 1 && !bones->active && !bones->revive_timer &&
        !catacombs_lit_braziers(&game.map, CATACOMBS_MEMORIAL_ROOM));
    ASSERT("partial progress, living guards, and cold memorial survive saving", save_game(&game, OSWIN_SLOT) && load_game(&loaded, OSWIN_SLOT) &&
        loaded.catacombs_quest_progress == 1 && loaded.catacombs_quest_encounters == 1 && guards(&loaded, 1) == 1 &&
        memorial_tile(&loaded.map) == TILE_MEMORIAL_COLD && !loaded.enemies[loaded.enemy_count - 2].revive_timer);
    game = loaded;
    stage(3);
    ASSERT("Watchers' memorial has a Grave Archer and a stronger catacomb Wraith", guards(&game, 1) == 2 &&
        game.enemies[game.enemy_count - 2].type == ENEMY_GRAVE_ARCHER && game.enemies[game.enemy_count - 1].type == ENEMY_WRAITH &&
        game.enemies[game.enemy_count - 1].max_hp >= 36);
    const Room *other = &game.map.rooms[2];
    game.player.x = other->x + 2;
    game.player.y = other->y + 1;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("ordinary braziers do not count as quest memorials", game.catacombs_quest_progress == 1);
    approach_memorial();
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    stage(2);
    ASSERT("backtracking retains the cold memorial and its surviving guard", memorial_tile(&game.map) == TILE_MEMORIAL_COLD &&
        guards(&game, 0) == 2 && guards(&game, 1) == 1 && game.catacombs_quest_progress == 3);
    stage(4);
    ASSERT("Choir memorial has a Bone Cantor and Ancient Skeleton", guards(&game, 1) == 2 &&
        game.enemies[game.enemy_count - 2].type == ENEMY_BONE_CANTOR && game.enemies[game.enemy_count - 1].type == ENEMY_ANCIENT_SKELETON);
    approach_memorial();
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("three silent memorials still require the burial ledger", game.catacombs_quest_progress == 7 && game.catacombs_quest_state == 1);
    stage(5);
    approach_ledger();
    ASSERT("the sealed ledger offers an interaction beside its plinth", game_has_regional_interaction(&game));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("living Grave Marshal prevents ledger retrieval", game.catacombs_quest_progress == 7 && game.catacombs_quest_state == 1);
    Enemy *boss = &game.enemies[0];
    boss->hp = 1;
    game.player.attack = 10000;
    game.player.x = boss->x;
    game.player.y = boss->y + 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, boss->x, boss->y});
    ASSERT("Marshal victory retains the existing mantle reward without completing the quest automatically", !boss->active &&
        (game.defeated_bosses & (1 << LOCATION_CATACOMBS)) && game.catacombs_mantle_unclaimed && game.catacombs_quest_state == 1);
    approach_ledger();
    inventory = game.inventory_count;
    game.inventory_count = MAX_INVENTORY;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    game.inventory_count = inventory;
    ASSERT("ledger retrieval completes all four objectives without inventory space", game.catacombs_quest_state == 2 &&
        game.catacombs_quest_progress == 15 && !game_has_regional_interaction(&game));
    ASSERT("ready quest and all four completed journal objectives survive saving", save_game(&game, OSWIN_SLOT) && load_game(&loaded, OSWIN_SLOT) &&
        loaded.catacombs_quest_state == 2 && loaded.catacombs_quest_progress == 15 &&
        quest_journal_get_entry(&loaded, QUEST_TAB_ACTIVE, 0, &entry) && entry.objective_complete[0] && entry.objective_complete[1] &&
        entry.objective_complete[2] && entry.objective_complete[3]);
    game_open_town_portal(&game);
    ASSERT("Catacombs return spell still leads to Rosemoor with a working portal", game.location == LOCATION_TOWN3 && game.portal_active);
    approach_oswin();
    int gold = game.gold;
    int score = game.score;
    game_talk_to_oswin(&game);
    ASSERT("Oswin awards 150 gold and 1500 score once", game.catacombs_quest_state == 3 && game.gold == gold + 150 && game.score == score + 1500);
    game_talk_to_oswin(&game);
    ASSERT("repeated conversations cannot repeat the reward", game.gold == gold + 150 && game.score == score + 1500);
    ASSERT("completed journal retains the fourth objective", quest_journal_get_entry(&game, QUEST_TAB_COMPLETED, 0, &entry) &&
        entry.objective_count == 4 && entry.objective_complete[3] && !quest_journal_count(&game, QUEST_TAB_ACTIVE));
    ASSERT("completed quest persists through save/load", save_game(&game, OSWIN_SLOT) && load_game(&loaded, OSWIN_SLOT) &&
        loaded.catacombs_quest_state == 3 && loaded.catacombs_quest_progress == 15 && loaded.gold == game.gold && loaded.score == game.score);
    game_use_town_portal(&game);
    ASSERT("portal revisit does not recreate the collected ledger", game.location == LOCATION_CATACOMBS && game.level == 5 &&
        game.map.tiles[game.player.y][game.player.x - 1] == TILE_CATACOMBS_SARCOPHAGUS);
    game_init(&game);
    ASSERT("new game resets the quest and its encounters", !game.catacombs_quest_state && !game.catacombs_quest_progress &&
        !game.catacombs_quest_encounters && !game.catacombs_cache[1].valid);
}

static void test_previous_exploration(void) {
    start();
    game.defeated_bosses |= 1 << LOCATION_CATACOMBS;
    game_enter_catacombs(&game);
    for (int level = 2; level <= 4; level++) {
        stage(level);
        approach_memorial();
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    }
    ASSERT("pre-quest extinguishing leaves all quest fields untouched", !game.catacombs_quest_state && !game.catacombs_quest_progress &&
        !game.catacombs_quest_encounters && !guards(&game, 0));
    stage(5);
    game.enemies[0].hp = 9;
    game.map.burial_traps[0].spent = 1;
    game_open_town_portal(&game);
    approach_oswin();
    game_talk_to_oswin(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("accepting after exploration credits all cold memorials and preserves portal, traps, and enemies", game.catacombs_quest_state == 1 &&
        game.catacombs_quest_progress == 7 && !game.catacombs_quest_encounters && game.portal_active && game.catacombs_cache[4].valid &&
        game.catacombs_cache[4].enemies[0].hp == 9 && game.catacombs_cache[4].map.burial_traps[0].spent);
    game_use_town_portal(&game);
    approach_ledger();
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("a previous Marshal victory makes the ledger recoverable without a second boss", game.catacombs_quest_state == 2 &&
        game.catacombs_quest_progress == 15 && !guards(&game, 0));
    stage(4);
    ASSERT("credited memorials stay cold without new defenders on revisit", memorial_tile(&game.map) == TILE_MEMORIAL_COLD &&
        !guards(&game, 0) && !game.catacombs_quest_encounters);

    start();
    approach_oswin();
    game_talk_to_oswin(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    game_leave_town_hall(&game);
    game.defeated_bosses |= 1 << LOCATION_CATACOMBS;
    game_enter_catacombs(&game);
    stage(5);
    approach_ledger();
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("collecting the ledger before silencing memorials keeps the quest active", game.catacombs_quest_state == 1 &&
        game.catacombs_quest_progress == 8 && save_game(&game, OSWIN_SLOT) && load_game(&loaded, OSWIN_SLOT) &&
        loaded.catacombs_quest_progress == 8);
    for (int level = 4; level >= 2; level--) {
        stage(level);
        approach_memorial();
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    }
    ASSERT("ledger-first quest completes while silencing memorials in reverse order", game.catacombs_quest_state == 2 && game.catacombs_quest_progress == 15);

    start();
    game_enter_catacombs(&game);
    stage(2);
    int count = game.enemy_count;
    game.enemies[0].active = 0;
    game.enemies[0].hp = 0;
    game.enemies[0].revive_timer = 2;
    game_open_town_portal(&game);
    approach_oswin();
    game_talk_to_oswin(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    game_use_town_portal(&game);
    ASSERT("adding guards to an explored chamber preserves corpses with pending resurrection", game.enemy_count == count + 2 &&
        game.enemies[0].hp == 0 && !game.enemies[0].active && game.enemies[0].revive_timer == 2 && guards(&game, 1) == 2);
}

static int write_legacy_fixture(void) {
    if (!save_game(&game, OSWIN_SLOT)) {
        return 0;
    }
    FILE *file = fopen(OSWIN_SAVE, "rb");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 96);
    cJSON_DeleteItemFromObject(root, "catacombs_quest_state");
    cJSON_DeleteItemFromObject(root, "catacombs_quest_progress");
    cJSON_DeleteItemFromObject(root, "catacombs_quest_encounters");
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json) {
        return 0;
    }
    file = fopen(OSWIN_SAVE, "wb");
    if (!file) {
        free(json);
        return 0;
    }
    int written = fputs(json, file) >= 0;
    fclose(file);
    free(json);
    return written;
}

static void test_migration(void) {
    start();
    game_enter_catacombs(&game);
    stage(2);
    approach_memorial();
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    game.enemies[0].hp = 9;
    game_open_town_portal(&game);
    approach_oswin();
    game.player.x = HALL_OSWIN_X;
    game.player.y = HALL_OSWIN_Y;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = HALL_OSWIN_X, .y = HALL_OSWIN_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_short_sword()};
    game.map.tiles[HALL_OSWIN_Y][HALL_OSWIN_X] = TILE_ITEM;
    game.mara_quest_state = 1;
    game.mara_beacons_lit = 5;
    game.gold = 617;
    game.defeated_bosses |= 1 << LOCATION_CATACOMBS;
    ASSERT("version 96 Hall fixture migrates without assigning the new quest", write_legacy_fixture() && load_game(&loaded, OSWIN_SLOT) &&
        !loaded.catacombs_quest_state && !loaded.catacombs_quest_progress && !loaded.catacombs_quest_encounters);
    ASSERT("migration preserves other quests, money, portal, boss victories, and catacomb progress", loaded.mara_quest_state == 1 &&
        loaded.mara_beacons_lit == 5 && loaded.gold == 617 && loaded.portal_active && (loaded.defeated_bosses & (1 << LOCATION_CATACOMBS)) &&
        loaded.catacombs_cache[1].valid && loaded.catacombs_cache[1].enemies[0].hp == 9 &&
        memorial_tile(&loaded.catacombs_cache[1].map) == TILE_OSSUARY_COLD);
    ASSERT("legacy Hall gains Oswin and safely moves overlapping player and loot beside him", loaded.map.tiles[HALL_OSWIN_Y][HALL_OSWIN_X] == TILE_NPC_OSWIN &&
        loaded.player.x == HALL_OSWIN_X && loaded.player.y == HALL_OSWIN_Y + 1 && loaded.floor_items[0].active &&
        loaded.floor_items[0].y == HALL_OSWIN_Y + 1 && loaded.floor_items[0].underlying_tile == TILE_TAVERN_FLOOR &&
        loaded.map.tiles[HALL_OSWIN_Y + 1][HALL_OSWIN_X] == TILE_ITEM);
    game = loaded;
    game_talk_to_oswin(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("migrated cold memorial receives credit when Oswin assigns the quest", game.catacombs_quest_state == 1 && game.catacombs_quest_progress == 1);
    ASSERT("migrated quest rewrites and reloads successfully", save_game(&game, OSWIN_SLOT) && load_game(&loaded, OSWIN_SLOT) &&
        loaded.catacombs_quest_progress == 1 && loaded.catacombs_quest_state == 1);

    start();
    game_enter_catacombs(&game);
    stage(4);
    game.enemies[0].active = 0;
    game.enemies[0].hp = 0;
    game.enemies[0].revive_timer = 2;
    game.map.burial_traps[0].timer = 1;
    ASSERT("legacy active Catacombs saves retain pending resurrection and trap warnings", write_legacy_fixture() && load_game(&loaded, OSWIN_SLOT) &&
        loaded.location == LOCATION_CATACOMBS && loaded.level == 4 && !loaded.catacombs_quest_state &&
        loaded.enemies[0].revive_timer == 2 && loaded.map.burial_traps[0].timer == 1 && memorial_tile(&loaded.map) == TILE_OSSUARY_BRAZIER);
    game.catacombs_quest_state = 1;
    game.catacombs_quest_progress = 16;
    ASSERT("save loading rejects invalid quest progress", save_game(&game, OSWIN_SLOT) && !load_game(&loaded, OSWIN_SLOT));
}

static void test_access(void) {
    static unsigned char seen[MAP_H][MAP_W];
    static int queue[MAP_H * MAP_W];
    int reachable = 1;
    int safe = 1;
    for (int seed = 0; seed < 32 && reachable && safe; seed++) {
        start();
        srand((unsigned int)seed);
        game.catacombs_quest_state = 1;
        game_enter_catacombs(&game);
        for (int level = 2; level <= 5; level++) {
            stage(level);
            memset(seen, 0, sizeof(seen));
            int head = 0;
            int tail = 0;
            queue[tail++] = game.map.stairs_up_y * MAP_W + game.map.stairs_up_x;
            seen[game.map.stairs_up_y][game.map.stairs_up_x] = 1;
            const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
            while (head < tail) {
                int position = queue[head++];
                int x = position % MAP_W;
                int y = position / MAP_W;
                for (int i = 0; i < 4; i++) {
                    int tx = x + offsets[i][0];
                    int ty = y + offsets[i][1];
                    if (tx >= 0 && tx < MAP_W && ty >= 0 && ty < MAP_H && !seen[ty][tx] && map_is_walkable(&game.map, tx, ty)) {
                        seen[ty][tx] = 1;
                        queue[tail++] = ty * MAP_W + tx;
                    }
                }
            }
            if (level < 5) {
                approach_memorial();
            } else {
                approach_ledger();
            }
            reachable &= seen[game.player.y][game.player.x] && seen[game.map.stairs_down_y][game.map.stairs_down_x];
            for (int i = 0; i < game.enemy_count; i++) {
                const Enemy *e = &game.enemies[i];
                if (strstr(e->name, "Memorial Guard")) {
                    safe &= seen[e->y][e->x] && catacombs_room_at(&game.map, e->x, e->y) == CATACOMBS_MEMORIAL_ROOM &&
                        (e->x != game.map.stairs_up_x || e->y != game.map.stairs_up_y);
                }
            }
        }
    }
    ASSERT("memorial approaches, ledger, and exits remain reachable across 32 expeditions", reachable);
    ASSERT("memorial defenders occupy reachable tiles in their brazier's chamber", safe);
}

void test_catacombs_quest(void) {
    printf("Royal Catacombs: Rest for the Forgotten tests:\n");
    int unused = !save_exists(OSWIN_SLOT);
    ASSERT("Oswin's test fixture does not overwrite a player save", unused);
    if (!unused) {
        return;
    }
    test_quest();
    test_previous_exploration();
    test_migration();
    test_access();
    remove(OSWIN_SAVE);
}
