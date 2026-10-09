#include "test_utils.h"
#include "game/jail.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define JAIL_TEST_SLOT 99148
static GameState game;
static GameState loaded;

static void outside_castle(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    game.location = LOCATION_CASTLE;
    map_generate_castle(&game.map, &game.player.x, &game.player.y);
    game.player.x = INFORMANT_X;
    game.player.y = INFORMANT_Y + 1;
}

static void move(int x, int y) {
    action_resolve_player(&game, (Action){ACTION_MOVE, x, y});
}

static void accept_escape(void) {
    jail_talk_nearby(&game);
    while (game.player.x < JAIL_PRISONER_X - 1) {
        move(game.player.x + 1, game.player.y);
    }
    jail_talk_nearby(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
}

static void reach_tunnel(void) {
    while (game.player.y < JAIL_HATCH_Y) {
        move(game.player.x, game.player.y + 1);
    }
    while (game.location == LOCATION_JAIL && game.player.x < JAIL_HATCH_X) {
        move(game.player.x + 1, game.player.y);
    }
}

static cJSON *read_fixture(void) {
    FILE *file = fopen("saves/savegame_99148.json", "r");
    if (!file) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    size_t count = fread(buffer, 1, size, file);
    buffer[count] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    return root;
}

static int write_fixture(cJSON *root) {
    if (!root) {
        return 0;
    }
    char *json = cJSON_Print(root);
    FILE *file = fopen("saves/savegame_99148.json", "w");
    int result = file && json;
    if (result) {
        fputs(json, file);
    }
    if (file) {
        fclose(file);
    }
    free(json);
    cJSON_Delete(root);
    return result;
}

static int tunnel_step(void) {
    int distance[ESCAPE_TUNNEL_H][ESCAPE_TUNNEL_W];
    int queue[ESCAPE_TUNNEL_H * ESCAPE_TUNNEL_W];
    memset(distance, -1, sizeof(distance));
    int head = 0;
    int tail = 0;
    int tx = ESCAPE_TUNNEL_W - 2;
    int ty = 14;
    queue[tail++] = ty * ESCAPE_TUNNEL_W + tx;
    distance[ty][tx] = 0;
    const int dx[4] = {1, 0, -1, 0};
    const int dy[4] = {0, 1, 0, -1};
    while (head < tail) {
        int cell = queue[head++];
        int x = cell % ESCAPE_TUNNEL_W;
        int y = cell / ESCAPE_TUNNEL_W;
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (nx < 1 || nx >= ESCAPE_TUNNEL_W - 1 || ny < 1 || ny >= ESCAPE_TUNNEL_H - 1 ||
                distance[ny][nx] >= 0 || !map_is_walkable(&game.map, nx, ny)) {
                continue;
            }
            distance[ny][nx] = distance[y][x] + 1;
            queue[tail++] = ny * ESCAPE_TUNNEL_W + nx;
        }
    }
    if (game.player.x == tx && game.player.y == ty) {
        move(tx + 1, ty);
        action_resolve_enemies(&game);
        return 1;
    }
    int current = distance[game.player.y][game.player.x];
    for (int i = 0; i < 4; i++) {
        int x = game.player.x + dx[i];
        int y = game.player.y + dy[i];
        if (x >= 1 && x < ESCAPE_TUNNEL_W - 1 && y >= 1 && y < ESCAPE_TUNNEL_H - 1 &&
            distance[y][x] >= 0 && distance[y][x] < current) {
            move(x, y);
            action_resolve_enemies(&game);
            return 1;
        }
    }
    return 0;
}

void test_jail(void) {
    printf("Royal jail and prisoner escort tests:\n");
    ASSERT("jail test save slot is unused", !save_exists(JAIL_TEST_SLOT));
    outside_castle();
    ASSERT("castle grounds have a jail and ordinary townsman", game.map.tiles[JAIL_DOOR_Y][JAIL_DOOR_X] == TILE_JAIL_DOOR &&
        game.map.tiles[INFORMANT_Y][INFORMANT_X] == TILE_NPC_INFORMANT &&
        !map_is_walkable(&game.map, JAIL_X, JAIL_Y));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    move(INFORMANT_X, INFORMANT_Y);
    ASSERT("approaching, bumping, and action do not trigger the arrest", game.location == LOCATION_CASTLE && !game.jail_quest_state);
    game.player.x = JAIL_DOOR_X;
    game.player.y = JAIL_DOOR_Y + 1;
    move(JAIL_DOOR_X, JAIL_DOOR_Y);
    ASSERT("jail front door remains locked", game.location == LOCATION_CASTLE && game.player.y == JAIL_DOOR_Y + 1);
    ASSERT("the townsman cannot report a distant player", !jail_talk_nearby(&game));
    game.player.x = INFORMANT_X;
    game.player.y = INFORMANT_Y + 1;
    int inventory = game.inventory_count;
    int gold = game.gold;
    ASSERT("talking to the townsman triggers an explained arrest", jail_talk_nearby(&game) && game.location == LOCATION_JAIL &&
        game.jail_quest_state == 1 && strstr(game.dialogue_text, "informant") && strstr(game.dialogue_text, "locked"));
    ASSERT("arrest preserves equipment and money", game.inventory_count == inventory && game.gold == gold &&
        game.map.tiles[JAIL_PRISONER_Y][JAIL_PRISONER_X] == TILE_NPC_PRISONER &&
        game.map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] != TILE_JAIL_HATCH && !map_is_walkable(&game.map, 12, 13));
    ASSERT("unoffered prisoner quest is absent from the journal", !quest_journal_count(&game, QUEST_TAB_ACTIVE));
    ASSERT("saving and loading preserves the unexplained hatch and jail", save_game(&game, JAIL_TEST_SLOT) &&
        load_game(&loaded, JAIL_TEST_SLOT) && loaded.location == LOCATION_JAIL && loaded.jail_quest_state == 1 &&
        loaded.player.x == game.player.x && loaded.map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] == TILE_CASTLE_FLOOR);
    game_open_town_portal(&game);
    game_return_to_town(&game);
    ASSERT("jail portals cannot bypass the prisoner encounter", game.location == LOCATION_JAIL && !game.portal_active);
    game.portal_active = 1;
    game.portal_location = LOCATION_ASHEN;
    game.portal_level = 1;
    game_use_town_portal(&game);
    ASSERT("older return portals cannot bypass jail but remain available after escape", game.location == LOCATION_JAIL &&
        game.portal_active && game.portal_location == LOCATION_ASHEN);
    game.portal_active = 0;
    while (game.player.x < JAIL_PRISONER_X - 1) {
        move(game.player.x + 1, game.player.y);
    }
    jail_talk_nearby(&game);
    ASSERT("Tomas offers the escort without revealing the hatch", game_quest_offer_active(&game) && game.jail_quest_state == 1 &&
        game.map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] != TILE_JAIL_HATCH && !quest_journal_count(&game, QUEST_TAB_ACTIVE));
    game_handle_quest_offer_key(&game, SDL_SCANCODE_N, 0);
    ASSERT("declining Tomas leaves the escort available for later", game.jail_quest_state == 1 && !game.dialogue_active);
    jail_talk_nearby(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    ASSERT("accepting Tomas's escort reveals a hatch", game.jail_quest_state == 2 &&
        game.map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] == TILE_JAIL_HATCH && strstr(game.dialogue_text, "dangerous"));
    QuestJournalEntry entry;
    ASSERT("journal records the assigned escort and its reward", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "Guide Tomas Home") == 0 && entry.objective_count == 2 &&
        entry.reward_gold == ESCAPE_REWARD_GOLD && !entry.objective_complete[0]);
    jail_talk_nearby(&game);
    ASSERT("repeated prisoner dialogue cannot pay out early", game.gold == gold && game.jail_quest_state == 2);
    ASSERT("revealed hatch survives saving in jail", save_game(&game, JAIL_TEST_SLOT) && load_game(&loaded, JAIL_TEST_SLOT) &&
        loaded.jail_quest_state == 2 && loaded.map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] == TILE_JAIL_HATCH);
    reach_tunnel();
    ASSERT("walking onto the hatch enters one tunnel with Tomas and enemies", game.location == LOCATION_ESCAPE_TUNNEL &&
        game.level == 1 && game.enemy_count == NON_ROAD_ENEMY_LIMIT && jail_prisoner_at(&game, 1, 14) &&
        quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) && entry.objective_complete[0] && !entry.objective_complete[1]);
    int open = 1;
    int archers = 0;
    int bodyguards = 0;
    int stronger = 0;
    for (int i = 0; i < game.enemy_count; i++) {
        Enemy *enemy = &game.enemies[i];
        open &= map_is_walkable(&game.map, enemy->x, enemy->y);
        archers += enemy->type == ENEMY_ROAD_ARCHER;
        bodyguards += enemy->type == ENEMY_HOBGOBLIN_GUARD;
        stronger += enemy->x >= ESCAPE_TUNNEL_LEGACY_W && enemy->max_hp > 32 && enemy->attack > 10;
        for (int j = 0; j < i; j++) {
            open &= enemy->x != game.enemies[j].x || enemy->y != game.enemies[j].y;
        }
    }
    ASSERT("the longer tunnel adds stronger ranged and shielding enemies on distinct walkable tiles",
        ESCAPE_TUNNEL_W >= 180 && open && stronger >= 18 && archers >= 6 && bodyguards >= 3);
    ASSERT("deeper tunnel bends narrow to three tiles while leaving Tomas a walkable route",
        map_is_walkable(&game.map, 90, 7) && map_is_walkable(&game.map, 90, 8) &&
        map_is_walkable(&game.map, 90, 9) && !map_is_walkable(&game.map, 90, 6) &&
        !map_is_walkable(&game.map, 90, 10) && map_is_walkable(&game.map, 120, 20) &&
        map_is_walkable(&game.map, 160, 8));
    game.player.known_spell_count = 1;
    game.player.equipped_spell = 0;
    game.player.known_spells[0] = spell_make_return_to_town();
    int mana = game.player.mp;
    action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("town spell leaves Tomas and mana intact and explains the wards", game.location == LOCATION_ESCAPE_TUNNEL &&
        game.player.mp == mana && game.prisoner_x == 1 && strstr(game.messages[game.message_count - 1], "wards"));
    game.player.x = 1;
    move(0, 14);
    ASSERT("closing the hatch prevents returning to royal custody", game.location == LOCATION_ESCAPE_TUNNEL && game.player.x == 1);
    game.player.x = 6;
    int prisoner = game.prisoner_x;
    int facing_x = game.player.last_dx;
    int facing_y = game.player.last_dy;
    action_resolve_player(&game, (Action){ACTION_WAIT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("waiting advances Tomas without changing player position or spell facing", game.prisoner_x == prisoner + 1 &&
        game.prisoner_y == 14 && game.player.x == 6 && game.player.last_dx == facing_x && game.player.last_dy == facing_y);
    game.enemies[0].hp = 7;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 3, .y = 14, .underlying_tile = TILE_CASTLE_FLOOR, .item = item_make_health_potion()};
    game.map.tiles[14][3] = TILE_ITEM;
    ASSERT("save/load keeps escort position, tunnel layout, damage, and loot", save_game(&game, JAIL_TEST_SLOT) &&
        load_game(&loaded, JAIL_TEST_SLOT) && loaded.location == LOCATION_ESCAPE_TUNNEL && loaded.jail_quest_state == 2 &&
        loaded.prisoner_x == game.prisoner_x && loaded.prisoner_y == game.prisoner_y && loaded.enemies[0].hp == 7 &&
        loaded.floor_items[0].active && memcmp(loaded.map.tiles, game.map.tiles, sizeof(game.map.tiles)) == 0);
    // Model an in-progress short tunnel without disturbing its original actors or loot.
    game.enemy_count = 12;
    game.enemies[1].active = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = ESCAPE_TUNNEL_LEGACY_W - 1; x < MAP_W; x++) {
            game.map.tiles[y][x] = TILE_CASTLE_WALL;
        }
    }
    game.map.tiles[14][ESCAPE_TUNNEL_LEGACY_W - 1] = TILE_TUNNEL_EXIT;
    map_clear_exploration(&game.map);
    map_mark_explored(&game.map, 6, 14);
    ASSERT("short tunnel migration fixture saves", save_game(&game, JAIL_TEST_SLOT));
    cJSON *legacy = read_fixture();
    if (legacy) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(legacy, "save_version"), 117);
    }
    ASSERT("old tunnel saves gain the deeper passage without moving Tomas or respawning defeated enemies",
        write_fixture(legacy) && load_game(&loaded, JAIL_TEST_SLOT) &&
        loaded.enemy_count == NON_ROAD_ENEMY_LIMIT && loaded.player.x == game.player.x && loaded.prisoner_x == game.prisoner_x &&
        loaded.prisoner_y == game.prisoner_y && loaded.enemies[0].hp == 7 && !loaded.enemies[1].active &&
        loaded.floor_items[0].active && loaded.floor_items[0].x == 3 && loaded.map.tiles[14][3] == TILE_ITEM &&
        map_is_explored(&loaded.map, 6, 14) && !map_is_explored(&loaded.map, 90, 8) &&
        loaded.map.tiles[14][ESCAPE_TUNNEL_LEGACY_W - 1] == TILE_CASTLE_FLOOR &&
        loaded.map.tiles[14][ESCAPE_TUNNEL_W - 1] == TILE_TUNNEL_EXIT && loaded.gold == game.gold);
    game = loaded;
    game.enemies[12].active = 0;
    game.enemies[13].hp = 9;
    ASSERT("saving an extended tunnel keeps new enemy damage and defeats without rebuilding it again",
        save_game(&game, JAIL_TEST_SLOT) && load_game(&loaded, JAIL_TEST_SLOT) &&
        loaded.enemy_count == NON_ROAD_ENEMY_LIMIT && !loaded.enemies[12].active && loaded.enemies[13].hp == 9 &&
        memcmp(loaded.map.tiles, game.map.tiles, sizeof(game.map.tiles)) == 0);
    game.prisoner_x = 3;
    game.prisoner_y = 14;
    game.enemy_count = 5;
    for (int i = 0; i < game.enemy_count; i++) {
        game.enemies[i] = (Enemy){.active = 1, .type = ENEMY_GIANT_RAT, .x = 4, .y = 12 + i};
    }
    jail_follow(&game);
    ASSERT("Tomas cannot pass through a line of tunnel enemies", game.prisoner_x == 3 && game.prisoner_y == 14);
    game.enemy_count = 0;
    jail_follow(&game);
    ASSERT("clearing the dangerous route lets Tomas advance again", game.prisoner_x == 4 && game.prisoner_y == 14);
    game.player.x = ESCAPE_TUNNEL_W - 2;
    game.player.y = 14;
    move(ESCAPE_TUNNEL_W - 1, 14);
    ASSERT("the exit cannot abandon a lagging prisoner", game.location == LOCATION_ESCAPE_TUNNEL && game.jail_quest_state == 2 && game.gold == gold);
    game = loaded;
    game.player.attack = 1000;
    game.player.defense = 1000;
    game.player.hp = 10000;
    game.player.max_hp = 10000;
    int guard = 0;
    int defeated = 0;
    int previous_enemies = game.enemy_count;
    while (game.location == LOCATION_ESCAPE_TUNNEL && guard++ < 400 && tunnel_step()) {
        if (game.location == LOCATION_ESCAPE_TUNNEL) {
            int remaining = 0;
            for (int i = 0; i < game.enemy_count; i++) {
                remaining += game.enemies[i].active;
            }
            defeated += previous_enemies - remaining;
            previous_enemies = remaining;
        }
    }
    ASSERT("the full tunnel can be fought and escorted around all bends", game.location == LOCATION_TOWN4 &&
        game.player.x == 20 && game.player.y == 12 && game.jail_quest_state == 3 && jail_prisoner_at(&game, 21, 12) &&
        defeated >= 18 && guard >= 180);
    ASSERT("arrival awards the escort reward once without damaging dungeon caches", game.gold >= gold + ESCAPE_REWARD_GOLD &&
        game.score >= ESCAPE_REWARD_SCORE && !game.level_cache[0].valid && !game.enemy_count && !game.floor_item_count);
    ASSERT("completed journal records both escape objectives", quest_journal_get_entry(&game, QUEST_TAB_COMPLETED, 0, &entry) &&
        entry.objective_complete[0] && entry.objective_complete[1] && !quest_journal_count(&game, QUEST_TAB_ACTIVE));
    gold = game.gold;
    int score = game.score;
    jail_talk_nearby(&game);
    ASSERT("thanks in Ridgeshire cannot award a second reward", game.gold == gold && game.score == score);
    ASSERT("completed escort and reward persist", save_game(&game, JAIL_TEST_SLOT) && load_game(&loaded, JAIL_TEST_SLOT) &&
        loaded.jail_quest_state == 3 && loaded.gold == gold && loaded.score == score);
    game.location = LOCATION_CASTLE;
    map_generate_castle(&game.map, &game.player.x, &game.player.y);
    game.player.x = INFORMANT_X;
    game.player.y = INFORMANT_Y + 1;
    jail_talk_nearby(&game);
    ASSERT("returning to the informant cannot repeat the arrest or reward", game.location == LOCATION_CASTLE && game.jail_quest_state == 3 && game.gold == gold);

    outside_castle();
    game.alder_quest_state = 1;
    game.alder_wardens_rescued = 1;
    game.defeated_bosses = 1 << LOCATION_MOUNTAINS;
    game.player.x = JAIL_X;
    game.player.y = JAIL_Y;
    game.floor_item_count = 2;
    game.floor_items[0] = (FloorItem){.active = 1, .x = INFORMANT_X, .y = INFORMANT_Y,
        .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
    game.floor_items[1] = (FloorItem){.active = 1, .x = JAIL_X - 1, .y = JAIL_Y,
        .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_mana_potion()};
    for (int y = JAIL_Y; y < JAIL_Y + JAIL_H; y++) {
        for (int x = JAIL_X; x < JAIL_X + JAIL_W; x++) {
            game.map.tiles[y][x] = TILE_TOWN_FLOOR;
        }
    }
    game.map.tiles[INFORMANT_Y][INFORMANT_X] = TILE_ITEM;
    game.map.tiles[JAIL_Y][JAIL_X - 1] = TILE_ITEM;
    ASSERT("legacy castle fixture saves", save_game(&game, JAIL_TEST_SLOT));
    cJSON *root = read_fixture();
    if (root) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 100);
        cJSON_DeleteItemFromObject(root, "jail_quest_state");
        cJSON_DeleteItemFromObject(root, "prisoner_x");
        cJSON_DeleteItemFromObject(root, "prisoner_y");
    }
    ASSERT("version 100 saves gain the jail without resetting progress", write_fixture(root) && load_game(&loaded, JAIL_TEST_SLOT) &&
        !loaded.jail_quest_state && loaded.alder_quest_state == 1 && loaded.alder_wardens_rescued == 1 &&
        loaded.defeated_bosses == game.defeated_bosses && loaded.gold == game.gold);
    ASSERT("migration relocates overlapping players and loot and keeps the informant", loaded.player.x == JAIL_DOOR_X &&
        loaded.player.y == JAIL_DOOR_Y + 1 && loaded.floor_items[0].active && loaded.floor_items[0].x == JAIL_DOOR_X &&
        loaded.floor_items[0].y == JAIL_DOOR_Y + 1 && loaded.map.tiles[INFORMANT_Y][INFORMANT_X] == TILE_NPC_INFORMANT &&
        loaded.map.tiles[JAIL_DOOR_Y][JAIL_DOOR_X] == TILE_JAIL_DOOR);
    ASSERT("migrated path loot keeps cobblestone underneath", loaded.floor_items[1].active &&
        loaded.floor_items[1].underlying_tile == TILE_TOWN_PATH && loaded.map.tiles[JAIL_Y][JAIL_X - 1] == TILE_ITEM);
    outside_castle();
    accept_escape();
    reach_tunnel();
    ASSERT("escort corruption fixture saves", save_game(&game, JAIL_TEST_SLOT));
    root = read_fixture();
    if (root) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "prisoner_x"), ESCAPE_TUNNEL_W);
    }
    ASSERT("out-of-bounds companions are rejected before pathfinding", write_fixture(root) && !load_game(&loaded, JAIL_TEST_SLOT));
    remove("saves/savegame_99148.json");
}
