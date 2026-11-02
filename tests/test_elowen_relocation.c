#include "test_utils.h"
#include "approach_checks.h"
#include "game/elowen.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

#define ELOWEN_SAVE_SLOT 99149
static GameState game;
static GameState loaded;

static int find_prisoner(void) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (game.map.tiles[y][x] == TILE_NPC_ELOWEN) {
                return y * MAP_W + x;
            }
        }
    }
    return -1;
}

static int boss_at(int x, int y) {
    for (int i = 0; i < game.enemy_count; i++) {
        if (game.enemies[i].active && game.enemies[i].type == ENEMY_LICH_KING &&
            game.enemies[i].x == x && game.enemies[i].y == y) {
            return 1;
        }
    }
    return 0;
}

static int walk_to(int tx, int ty) {
    static int previous[MAP_H][MAP_W];
    static int queue[MAP_W * MAP_H];
    memset(previous, -1, sizeof(previous));
    int start = game.player.y * MAP_W + game.player.x;
    int target = ty * MAP_W + tx;
    int head = 0;
    int tail = 1;
    queue[0] = start;
    previous[game.player.y][game.player.x] = start;
    const int dx[] = {1, -1, 0, 0};
    const int dy[] = {0, 0, 1, -1};
    while (head < tail && previous[ty][tx] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + dx[side];
            int y = cell / MAP_W + dy[side];
            if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || previous[y][x] >= 0 || boss_at(x, y)) {
                continue;
            }
            int door = game.dungeon_key_found && game.map.tiles[y][x] == TILE_LOCKED_DOOR;
            if (!map_is_walkable(&game.map, x, y) && !door) {
                continue;
            }
            previous[y][x] = cell;
            queue[tail++] = y * MAP_W + x;
        }
    }
    ASSERT("a continuous route reaches the rescue's actual travel or conversation tile", previous[ty][tx] >= 0);
    if (previous[ty][tx] < 0) {
        printf("Rescue route: from %d,%d to %d,%d tile %d on floor %d\n", game.player.x, game.player.y, tx, ty, game.map.tiles[ty][tx], game.level);
        return 0;
    }
    int count = 0;
    for (int cell = target; cell != start; cell = previous[cell / MAP_W][cell % MAP_W]) {
        queue[count++] = cell;
    }
    Location location = game.location;
    for (int i = count - 1; i >= 0; i--) {
        int x = queue[i] % MAP_W;
        int y = queue[i] / MAP_W;
        for (int tries = 0; tries < 40 && (game.player.x != x || game.player.y != y); tries++) {
            action_resolve_player(&game, (Action){ACTION_MOVE, x, y});
        }
        if (game.location != location || game.player.x != x || game.player.y != y) {
            ASSERT("every step of the rescue route works through actual move actions", 0);
            return 0;
        }
    }
    return 1;
}

static void test_rescue(int player_class, int accepted) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = player_class;
    game_init(&game);
    srand(4310 + player_class * 3 + accepted);
    game.player.hp = 1000000;
    game.player.max_hp = 1000000;
    game.player.attack = 10000;
    game_enter_tavern(&game);
    ASSERT("Elowen does not begin as an Inn quest giver", find_prisoner() < 0);
    if (accepted) {
        game.player.x = BRAM_INN_X;
        game.player.y = BRAM_INN_Y + 1;
        game_talk_to_bram(&game);
        game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
        ASSERT("Bram assigns Save Elowen", game.elowen_quest_state == ELOWEN_ASSIGNED && game_active_quest_count(&game) == 1);
    } else {
        game.dain_quest_state = 1;
        game.alder_quest_state = 1;
        game.mara_quest_state = 1;
    }
    game_leave_tavern(&game);
    if (!walk_to(20, TOWN_H - 2)) {
        return;
    }
    action_resolve_player(&game, (Action){ACTION_MOVE, 20, TOWN_H - 1});
    walk_town_approach(&game);
    ASSERT("the south road leads to Oakhaven's dungeon", game.location == LOCATION_DUNGEON && game.level == 1);
    for (int level = 1; level < DUNGEON_DEPTH; level++) {
        int retired_seals = 0;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                retired_seals |= game.map.tiles[y][x] == TILE_BROKEN_BURIAL_SEAL;
            }
        }
        ASSERT("early dungeon floors have neither Elowen nor the retired seal objectives", find_prisoner() < 0 && !retired_seals);
        if (!walk_to(game.map.stairs_down_x, game.map.stairs_down_y)) {
            return;
        }
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("the actual stairs advance to the next dungeon floor", game.location == LOCATION_DUNGEON && game.level == level + 1);
    }
    int captive = find_prisoner();
    ASSERT("Elowen is held in the final chamber without requiring an accepted quest", captive >= 0);
    if (captive < 0) {
        return;
    }
    ASSERT("prisoner placement and pending rescue survive saving", save_game(&game, ELOWEN_SAVE_SLOT) &&
        load_game(&loaded, ELOWEN_SAVE_SLOT) && loaded.elowen_quest_state == game.elowen_quest_state &&
        loaded.map.tiles[captive / MAP_W][captive % MAP_W] == TILE_NPC_ELOWEN);
    game = loaded;
    int key_x = -1;
    int key_y = -1;
    int boss = -1;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (game.map.tiles[y][x] == TILE_DUNGEON_KEY) {
                key_x = x;
                key_y = y;
            }
        }
    }
    ASSERT("the rescue keeps the Lich's door key reachable", key_x >= 0 && walk_to(key_x, key_y));
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("the player collects the Lich King door key", game.dungeon_key_found);
    if (!walk_to(captive % MAP_W - 1, captive / MAP_W)) {
        return;
    }
    game_talk_to_elowen(&game);
    ASSERT("the captive cannot be freed before the Lich falls", strstr(game.dialogue_text, "bound me here") && game.elowen_quest_state < ELOWEN_RESCUED);
    for (int i = 0; i < game.enemy_count; i++) {
        if (game.enemies[i].active && game.enemies[i].type == ENEMY_LICH_KING) {
            boss = i;
        }
    }
    ASSERT("the final room still contains the Lich King", boss >= 0);
    if (boss < 0) {
        return;
    }
    Enemy *lich = &game.enemies[boss];
    const int dx[] = {-1, 1, 0, 0};
    const int dy[] = {0, 0, -1, 1};
    int approach = -1;
    for (int i = 0; i < 4; i++) {
        if (map_is_walkable(&game.map, lich->x + dx[i], lich->y + dy[i])) {
            approach = i;
            break;
        }
    }
    ASSERT("the captive leaves a usable approach to the Lich", approach >= 0);
    if (approach < 0 || !walk_to(lich->x + dx[approach], lich->y + dy[approach])) {
        return;
    }
    lich->hp = 1;
    for (int tries = 0; lich->active && tries < 40; tries++) {
        if (player_class == CLASS_MAGE) {
            game.player.known_spells[0] = spell_make_fireball();
            game.player.known_spell_count = 1;
            game.player.equipped_spell = 0;
            game.player.mp = 1000;
            game.player.last_dx = lich->x - game.player.x;
            game.player.last_dy = lich->y - game.player.y;
            action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
        } else {
            action_resolve_player(&game, (Action){ACTION_MOVE, lich->x, lich->y});
        }
    }
    ASSERT("the Lich's defeat automatically frees Elowen for every class", !lich->active &&
        game.elowen_quest_state == ELOWEN_RESCUED && find_prisoner() < 0);
    QuestJournalEntry entry;
    ASSERT("rescue moves directly to Completed even with a full active journal", quest_journal_get_entry(&game, QUEST_TAB_COMPLETED, 0, &entry) &&
        strcmp(entry.title, "Save Elowen") == 0 && entry.state == 2 && entry.objective_complete[0] &&
        strstr(entry.summary_line_2, "Speak to Bram") && game_active_quest_count(&game) == (accepted ? 0 : 3));
    ASSERT("Elowen explains her father's location and reward", strcmp(game.dialogue_speaker, "Elowen") == 0 &&
        strstr(game.dialogue_text, "father Bram") && strstr(game.dialogue_text, "reward") &&
        elowen_at(&game, game.elowen_x, game.elowen_y));
    int follower_x = game.elowen_x;
    int follower_y = game.elowen_y;
    int old_x = game.player.x;
    int old_y = game.player.y;
    action_resolve_player(&game, (Action){ACTION_MOVE, follower_x, follower_y});
    ASSERT("the nonblocking escort follows into the player's previous square", game.player.x == follower_x &&
        game.player.y == follower_y && game.elowen_x == old_x && game.elowen_y == old_y);
    ASSERT("escort coordinates and completed objective survive saving", save_game(&game, ELOWEN_SAVE_SLOT) &&
        load_game(&loaded, ELOWEN_SAVE_SLOT) && loaded.elowen_quest_state == ELOWEN_RESCUED &&
        loaded.elowen_x == game.elowen_x && loaded.elowen_y == game.elowen_y);
    game = loaded;
    action_resolve_enemies(&game);
    ASSERT("remaining guards cannot attack or kill Elowen", game.elowen_quest_state == ELOWEN_RESCUED &&
        elowen_at(&game, game.elowen_x, game.elowen_y));
    if (accepted) {
        if (!walk_to(game.map.stairs_down_x, game.map.stairs_down_y)) {
            return;
        }
        action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
        walk_return_approach(&game);
    } else {
        game.player.known_spells[0] = spell_make_return_to_town();
        game.player.known_spell_count = 1;
        game.player.equipped_spell = 0;
        game.player.mp = 1000;
        action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
    }
    ASSERT("walking and the one-way portal both bring the escort to Oakhaven", game.location == LOCATION_TOWN &&
        game.elowen_quest_state == ELOWEN_RESCUED && !game.portal_active &&
        abs(game.elowen_x - game.player.x) + abs(game.elowen_y - game.player.y) == 1);
    if (!walk_to(TOWN_TAVERN_DOOR_X, TOWN_TAVERN_DOOR_Y + 1)) {
        return;
    }
    action_resolve_player(&game, (Action){ACTION_MOVE, TOWN_TAVERN_DOOR_X, TOWN_TAVERN_DOOR_Y});
    ASSERT("the escort enters the Inn without becoming a duplicate resident", game.location == LOCATION_TAVERN && find_prisoner() < 0);
    if (!walk_to(BRAM_INN_X, BRAM_INN_Y + 1)) {
        return;
    }
    game.inventory_count = MAX_INVENTORY;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        game.inventory[i] = item_make_health_potion();
    }
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = ELOWEN_TAVERN_X, .y = ELOWEN_TAVERN_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_health_potion()};
    game.map.tiles[ELOWEN_TAVERN_Y][ELOWEN_TAVERN_X] = TILE_ITEM;
    int gold = game.gold;
    game_talk_to_bram(&game);
    game_talk_to_bram(&game);
    ASSERT("Bram pays exactly 80 gold once despite a full pack or journal", game.elowen_quest_state == ELOWEN_HOME &&
        game.gold == gold + ELOWEN_REWARD_GOLD && game.elowen_x == -1 && game.elowen_y == -1);
    ASSERT("the reunion preserves items dropped at Elowen's home", game.floor_items[0].active &&
        game.floor_items[0].y == ELOWEN_TAVERN_Y + 1 && game.map.tiles[ELOWEN_TAVERN_Y + 1][ELOWEN_TAVERN_X] == TILE_ITEM);
    if (!walk_to(ELOWEN_TAVERN_X, ELOWEN_TAVERN_Y + 1)) {
        return;
    }
    game_talk_to_elowen(&game);
    ASSERT("Elowen becomes a permanent Inn resident with the first clue against the King", find_prisoner() >= 0 &&
        strstr(game.dialogue_text, "in league with the King") && !strstr(game.dialogue_text, "Malakar") &&
        !game_quest_offer_active(&game));
    ASSERT("reunion and testimony persist across save/load without repeating the reward", save_game(&game, ELOWEN_SAVE_SLOT) &&
        load_game(&loaded, ELOWEN_SAVE_SLOT) && loaded.elowen_quest_state == ELOWEN_HOME &&
        loaded.gold == gold + ELOWEN_REWARD_GOLD && strstr(loaded.dialogue_text, "in league with the King"));
    game = loaded;
    game_leave_tavern(&game);
    game_enter_tavern(&game);
    ASSERT("return visits keep Elowen at the Inn", game.map.tiles[ELOWEN_TAVERN_Y][ELOWEN_TAVERN_X] == TILE_NPC_ELOWEN);
    game_enter_dungeon(&game);
    for (int level = 1; level < DUNGEON_DEPTH; level++) {
        game_descend(&game);
    }
    ASSERT("replayed dungeons never recapture Elowen", game.elowen_quest_state == ELOWEN_HOME && find_prisoner() < 0);
}

void test_elowen_relocation(void) {
    printf("Save Elowen rescue, escort and reunion tests:\n");
    ASSERT("Elowen test save slot is unused", !save_exists(ELOWEN_SAVE_SLOT));
    for (int player_class = CLASS_WARRIOR; player_class <= CLASS_ROGUE; player_class++) {
        for (int accepted = 0; accepted <= 1; accepted++) {
            test_rescue(player_class, accepted);
        }
    }
    remove("saves/savegame_99149.json");
}
