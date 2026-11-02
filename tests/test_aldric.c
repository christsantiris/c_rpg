#include "test_utils.h"
#include "approach_checks.h"
#include "game/aldric.h"
#include "game/elowen.h"
#include "systems/save_load.h"
#include <stdlib.h>
#include <string.h>

#define ALDRIC_SAVE_SLOT 99231
static GameState *game;
static GameState *loaded;

static int count_aldric_tiles(const GameState *g) {
    int count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            count += g->map.tiles[y][x] == TILE_NPC_ALDRIC;
        }
    }
    return count;
}

// He stays on open ground near the player and never shares a square with the party.
static int follower_ok(const GameState *g) {
    int distance = abs(g->aldric_x - g->player.x) + abs(g->aldric_y - g->player.y);
    int max_distance = g->elowen_quest_state == ELOWEN_RESCUED ? 2 : 1;
    return g->aldric_party_state == ALDRIC_FOLLOWING && map_is_walkable(&g->map, g->aldric_x, g->aldric_y) &&
        distance >= 1 && distance <= max_distance && !elowen_at(g, g->aldric_x, g->aldric_y);
}

static int walk_to(GameState *g, int tx, int ty, int *followed) {
    static int previous[MAP_H][MAP_W];
    static int queue[MAP_W * MAP_H];
    memset(previous, -1, sizeof(previous));
    int start = g->player.y * MAP_W + g->player.x;
    int head = 0;
    int tail = 1;
    queue[0] = start;
    previous[g->player.y][g->player.x] = start;
    const int dx[] = {1, -1, 0, 0};
    const int dy[] = {0, 0, 1, -1};
    while (head < tail && previous[ty][tx] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + dx[side];
            int y = cell / MAP_W + dy[side];
            if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || previous[y][x] >= 0 || !map_is_walkable(&g->map, x, y)) {
                continue;
            }
            previous[y][x] = cell;
            queue[tail++] = y * MAP_W + x;
        }
    }
    if (previous[ty][tx] < 0) {
        return 0;
    }
    int count = 0;
    for (int cell = ty * MAP_W + tx; cell != start; cell = previous[cell / MAP_W][cell % MAP_W]) {
        queue[count++] = cell;
    }
    Location location = g->location;
    for (int i = count - 1; i >= 0; i--) {
        action_resolve_player(g, (Action){ACTION_MOVE, queue[i] % MAP_W, queue[i] / MAP_W});
        if (followed && g->aldric_party_state == ALDRIC_FOLLOWING && !follower_ok(g)) {
            *followed = 0;
        }
        if (g->location != location) {
            return 1;
        }
    }
    return g->player.x == tx && g->player.y == ty;
}

static void enter_inn(GameState *g) {
    walk_to(g, TOWN_TAVERN_DOOR_X, TOWN_TAVERN_DOOR_Y + 1, NULL);
    action_resolve_player(g, (Action){ACTION_MOVE, TOWN_TAVERN_DOOR_X, TOWN_TAVERN_DOOR_Y});
}

static void leave_inn(GameState *g, int *followed) {
    walk_to(g, 20, 21, followed);
    action_resolve_player(g, (Action){ACTION_MOVE, 20, 22});
}

static void test_square_before_lich(void) {
    memset(game, 0, sizeof(*game));
    game_init(game);
    ASSERT("Aldric advises from Oakhaven's square before the Lich falls",
        game->map.tiles[TOWN_ALDRIC_Y][TOWN_ALDRIC_X] == TILE_NPC_ALDRIC && game->aldric_party_state == ALDRIC_WAITING);
    aldric_talk(game, TOWN_ALDRIC_X, TOWN_ALDRIC_Y);
    ASSERT("his early advice is unchanged and offers no recruitment",
        strstr(game->dialogue_text, "Clear the dungeon") && !aldric_prompt_active(game) &&
        !aldric_handle_key(game, SDL_SCANCODE_Y, 0) && game->aldric_party_state == ALDRIC_WAITING);
    game->dialogue_active = 0;
    enter_inn(game);
    ASSERT("the Inn has no Aldric before the Lich falls", game->location == LOCATION_TAVERN && count_aldric_tiles(game) == 0);
    leave_inn(game, NULL);
    ASSERT("leaving the Inn keeps Aldric in the square", game->location == LOCATION_TOWN &&
        game->map.tiles[TOWN_ALDRIC_Y][TOWN_ALDRIC_X] == TILE_NPC_ALDRIC);
}

static void test_moves_to_inn(void) {
    memset(game, 0, sizeof(*game));
    game_init(game);
    game->defeated_bosses |= 1 << LOCATION_DUNGEON;
    enter_inn(game);
    ASSERT("after the Lich, Aldric waits at his Inn seat beside Alder and Dain",
        game->map.tiles[ALDRIC_INN_Y][ALDRIC_INN_X] == TILE_NPC_ALDRIC && count_aldric_tiles(game) == 1 &&
        game->map.tiles[ALDER_OAKHAVEN_Y][ALDER_OAKHAVEN_X] == TILE_NPC_ALDER &&
        game->map.tiles[DAIN_OAKHAVEN_Y][DAIN_OAKHAVEN_X] == TILE_NPC_DAIN);
    leave_inn(game, NULL);
    ASSERT("leaving the Inn leaves the square empty", game->location == LOCATION_TOWN &&
        game->map.tiles[TOWN_ALDRIC_Y][TOWN_ALDRIC_X] == TILE_TOWN_PATH && count_aldric_tiles(game) == 0);
    game->location = LOCATION_FOREST_ROAD;
    game_leave_forest_road(game, LOCATION_TOWN);
    ASSERT("arriving by the forest road leaves the square empty", game->location == LOCATION_TOWN && count_aldric_tiles(game) == 0);
    walk_to(game, 20, TOWN_H - 2, NULL);
    action_resolve_player(game, (Action){ACTION_MOVE, 20, TOWN_H - 1});
    walk_town_approach(game);
    game_return_to_town(game);
    ASSERT("returning from an adventure leaves the square empty", game->location == LOCATION_TOWN && count_aldric_tiles(game) == 0);
}

static void test_recruit_follow_dismiss(void) {
    memset(game, 0, sizeof(*game));
    game_init(game);
    srand(2311);
    game->defeated_bosses |= 1 << LOCATION_DUNGEON;
    game->elowen_quest_state = ELOWEN_HOME;
    enter_inn(game);
    walk_to(game, ALDRIC_INN_X, ALDRIC_INN_Y + 1, NULL);
    aldric_talk(game, ALDRIC_INN_X, ALDRIC_INN_Y);
    ASSERT("Aldric's advice changes and he offers to join", strstr(game->dialogue_text, "Let me join you") &&
        aldric_prompt_active(game));
    ASSERT("other keys leave the offer open", aldric_handle_key(game, SDL_SCANCODE_W, 0) && aldric_prompt_active(game));
    aldric_handle_key(game, SDL_SCANCODE_N, 0);
    ASSERT("declining leaves Aldric at the Inn without a penalty", !game->dialogue_active &&
        game->aldric_party_state == ALDRIC_WAITING && game->map.tiles[ALDRIC_INN_Y][ALDRIC_INN_X] == TILE_NPC_ALDRIC);
    aldric_talk(game, ALDRIC_INN_X, ALDRIC_INN_Y);
    aldric_handle_key(game, SDL_SCANCODE_Y, 0);
    ASSERT("accepting adds Aldric to the party beside the player", !game->dialogue_active && follower_ok(game) &&
        game->map.tiles[ALDRIC_INN_Y][ALDRIC_INN_X] == TILE_TAVERN_FLOOR && count_aldric_tiles(game) == 0);

    int followed = 1;
    leave_inn(game, &followed);
    ASSERT("Aldric follows out of the Inn", followed && game->location == LOCATION_TOWN && follower_ok(game));
    int old_x = game->player.x;
    int old_y = game->player.y;
    action_resolve_player(game, (Action){ACTION_MOVE, game->aldric_x, game->aldric_y});
    ASSERT("walking into Aldric swaps places", game->player.x != old_x || game->player.y != old_y);
    ASSERT("after the swap he stands beside the player", follower_ok(game));
    aldric_talk(game, game->aldric_x, game->aldric_y);
    ASSERT("outside the Inn he explains how to dismiss him, with no prompt",
        strstr(game->dialogue_text, "Oakhaven's Inn") && !aldric_prompt_active(game));
    game->dialogue_active = 0;

    ASSERT("Aldric save slot is unused", !save_exists(ALDRIC_SAVE_SLOT));
    int ok = save_game(game, ALDRIC_SAVE_SLOT) && load_game(loaded, ALDRIC_SAVE_SLOT);
    ASSERT("save/load keeps Aldric in the party at his position", ok &&
        loaded->aldric_party_state == ALDRIC_FOLLOWING && loaded->aldric_x == game->aldric_x &&
        loaded->aldric_y == game->aldric_y && follower_ok(loaded));

    walk_to(game, 20, TOWN_H - 2, &followed);
    action_resolve_player(game, (Action){ACTION_MOVE, 20, TOWN_H - 1});
    ASSERT("Aldric follows through Oakhaven's south gate", followed && game->location != LOCATION_TOWN && follower_ok(game));
    walk_town_approach(game);
    ASSERT("Aldric follows into the dungeon", game->location == LOCATION_DUNGEON && follower_ok(game));
    for (int i = 0; i < 6; i++) {
        action_resolve_enemies(game);
    }
    ASSERT("enemy turns never leave Aldric under an enemy", follower_ok(game));
    game_return_to_town(game);
    action_resolve_player(game, (Action){ACTION_WAIT, 0, 0});
    ASSERT("Aldric returns to Oakhaven with the player", game->location == LOCATION_TOWN && follower_ok(game));

    enter_inn(game);
    ASSERT("his seat stays empty while he travels with the player", game->location == LOCATION_TAVERN &&
        follower_ok(game) && count_aldric_tiles(game) == 0);
    walk_to(game, 20, 15, &followed);
    aldric_talk(game, game->aldric_x, game->aldric_y);
    ASSERT("inside the Inn he offers to wait there", strstr(game->dialogue_text, "wait here at the Inn") &&
        aldric_prompt_active(game));
    aldric_handle_key(game, SDL_SCANCODE_N, 0);
    ASSERT("declining keeps Aldric in the party", game->aldric_party_state == ALDRIC_FOLLOWING && follower_ok(game));
    aldric_talk(game, game->aldric_x, game->aldric_y);
    aldric_handle_key(game, SDL_SCANCODE_Y, 0);
    ASSERT("dismissing returns Aldric to his Inn seat", game->aldric_party_state == ALDRIC_WAITING &&
        game->aldric_x == -1 && game->map.tiles[ALDRIC_INN_Y][ALDRIC_INN_X] == TILE_NPC_ALDRIC && count_aldric_tiles(game) == 1);
    ok = save_game(game, ALDRIC_SAVE_SLOT) && load_game(loaded, ALDRIC_SAVE_SLOT);
    ASSERT("save/load keeps a dismissed Aldric at the Inn", ok && loaded->aldric_party_state == ALDRIC_WAITING &&
        loaded->map.tiles[ALDRIC_INN_Y][ALDRIC_INN_X] == TILE_NPC_ALDRIC);
    leave_inn(game, NULL);
    enter_inn(game);
    ASSERT("he is still at his seat on the next visit", count_aldric_tiles(game) == 1 &&
        game->map.tiles[ALDRIC_INN_Y][ALDRIC_INN_X] == TILE_NPC_ALDRIC);
    remove("saves/savegame_99231.json");
}

static void test_dismiss_with_seat_taken(void) {
    memset(game, 0, sizeof(*game));
    game_init(game);
    game->defeated_bosses |= 1 << LOCATION_DUNGEON;
    enter_inn(game);
    walk_to(game, ALDRIC_INN_X, ALDRIC_INN_Y + 1, NULL);
    aldric_talk(game, ALDRIC_INN_X, ALDRIC_INN_Y);
    aldric_handle_key(game, SDL_SCANCODE_Y, 0);
    walk_to(game, ALDRIC_INN_X, ALDRIC_INN_Y, NULL);
    int x = game->aldric_x;
    int y = game->aldric_y;
    aldric_talk(game, x, y);
    aldric_handle_key(game, SDL_SCANCODE_Y, 0);
    ASSERT("with the player on his seat, Aldric waits where he stands", game->aldric_party_state == ALDRIC_WAITING &&
        game->map.tiles[y][x] == TILE_NPC_ALDRIC && game->player.x == ALDRIC_INN_X && game->player.y == ALDRIC_INN_Y);
    aldric_talk(game, x, y);
    aldric_handle_key(game, SDL_SCANCODE_Y, 0);
    ASSERT("he can be recruited again from there", follower_ok(game) && count_aldric_tiles(game) == 0);
}

static void test_walks_with_elowen(void) {
    memset(game, 0, sizeof(*game));
    game_init(game);
    game->defeated_bosses |= 1 << LOCATION_DUNGEON;
    enter_inn(game);
    walk_to(game, ALDRIC_INN_X, ALDRIC_INN_Y + 1, NULL);
    aldric_talk(game, ALDRIC_INN_X, ALDRIC_INN_Y);
    aldric_handle_key(game, SDL_SCANCODE_Y, 0);
    game->elowen_quest_state = ELOWEN_RESCUED;
    game->elowen_x = -1;
    game->elowen_y = -1;
    elowen_follow(game, -1, -1);
    aldric_follow(game, -1, -1);
    int followed = follower_ok(game);
    walk_to(game, 8, 4, &followed);
    walk_to(game, 32, 20, &followed);
    leave_inn(game, &followed);
    walk_to(game, 4, 4, &followed);
    ASSERT("Aldric and Elowen never overlap each other or the player", followed && game->location == LOCATION_TOWN &&
        follower_ok(game) && (game->elowen_x != game->player.x || game->elowen_y != game->player.y));
}

void test_aldric(void) {
    printf("Aldric recruitment tests:\n");
    game = calloc(1, sizeof(*game));
    loaded = calloc(1, sizeof(*loaded));
    test_square_before_lich();
    test_moves_to_inn();
    test_recruit_follow_dismiss();
    test_dismiss_with_seat_taken();
    test_walks_with_elowen();
    free(game);
    free(loaded);
}
