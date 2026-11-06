#include "test_utils.h"
#include "game/eastern_arc.h"
#include "game/mangrove.h"
#include "game/town_road.h"
#include "game/town_life.h"
#include "game/stormbell.h"
#include "game/aldric.h"
#include "game/enemy_tactics.h"
#include "game/royal_evidence.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define EASTERN_SLOT 99256

static int journey_to(GameState *g, int tx, int ty) {
    static int previous[MAP_W * MAP_H];
    static int queue[MAP_W * MAP_H];
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    for (int i = 0; i < MAP_W * MAP_H; i++) {
        previous[i] = -1;
    }
    int start = g->player.y * MAP_W + g->player.x;
    int target = ty * MAP_W + tx;
    int head = 0;
    int tail = 1;
    queue[0] = start;
    previous[start] = start;
    int afloat = mangrove_aboard(g);
    while (head < tail && previous[target] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + dx[side];
            int y = cell / MAP_W + dy[side];
            if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
                continue;
            }
            int next = y * MAP_W + x;
            TileType tile = g->map.tiles[y][x];
            int passable = afloat ? tile == TILE_MANGROVE_WATER && x > 0 && x < MANGROVE_W - 1 && y > 0 && y < MANGROVE_H - 1 :
                (map_is_walkable(&g->map, x, y) || (g->location == LOCATION_MANGROVE_PATH && next == target)) && (!map_is_travel_tile(tile) || next == target);
            if (previous[next] >= 0 || !passable) {
                continue;
            }
            previous[next] = cell;
            queue[tail++] = next;
        }
    }
    if (previous[target] < 0) {
        return 0;
    }
    int length = 0;
    for (int cell = target; cell != start; cell = previous[cell]) {
        queue[length++] = cell;
    }
    Location location = g->location;
    int level = g->level;
    for (int i = length - 1; i >= 0; i--) {
        action_resolve_player(g, (Action){ACTION_MOVE, queue[i] % MAP_W, queue[i] / MAP_W});
        if (g->location != location || g->level != level) {
            return i == 0;
        }
        if (g->player.x != queue[i] % MAP_W || g->player.y != queue[i] / MAP_W) {
            return 0;
        }
    }
    return 1;
}

static int recent_message(const GameState *g, const char *text) {
    for (int i = g->message_count - 1; i >= 0 && i >= g->message_count - 4; i--) {
        if (strstr(g->messages[i], text)) {
            return 1;
        }
    }
    return 0;
}

static void defeat_regulars(GameState *g) {
    // Leave the generated terrain and encounter positions intact; simulate cleared foes.
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
}

static int boat_to(GameState *g, int x, int y) {
    const int steps[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    for (int i = 0; i < 4; i++) {
        int px = x + steps[i][0];
        int py = y + steps[i][1];
        if (g->map.tiles[py][px] == TILE_MANGROVE_WATER && journey_to(g, px, py)) {
            return mangrove_interact(g) && !mangrove_aboard(g) && g->player.x == x && g->player.y == y;
        }
    }
    return 0;
}

static int sail_stage(GameState *g, int forward) {
    int lx;
    int ly;
    mangrove_landing(g->level, forward ? 0 : 3, &lx, &ly);
    if (!journey_to(g, lx, ly) || !mangrove_interact(g) || !mangrove_aboard(g)) {
        return 0;
    }
    mangrove_landing(g->level, forward ? 3 : 0, &lx, &ly);
    return boat_to(g, lx, ly);
}

static void occupation(GameState *g, GameState *loaded) {
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    ASSERT("occupied Dawnmere has three persistent royal squads and retains Oakhaven as its safe anchor", g->enemy_count == DAWNMERE_GARRISON && !g->dawnmere_liberated && g->last_town_visited == LOCATION_TOWN);
    ASSERT("the west gate checkpoint orders the player to leave before any fighting", eastern_challenge_active(g) && strstr(g->dialogue_text, "martial law") && strstr(g->dialogue_text, "Leave town now") && g->dialogue_x == g->enemies[0].x && g->dialogue_y == g->enemies[0].y);
    for (int i = 0; i < g->enemy_count; i++) {
        int slot = i % DAWNMERE_SQUAD;
        int archer = slot >= 2 && slot % 2 == 0;
        ASSERT("royal soldiers spawn on usable streets outside NPCs and buildings", map_is_walkable(&g->map, g->enemies[i].x, g->enemies[i].y) && g->enemies[i].boss_aux_y == i / DAWNMERE_SQUAD + 1);
        ASSERT("each squad fields four guards and three archers", g->enemies[i].type == (archer ? ENEMY_ROAD_ARCHER : ENEMY_ROYAL_GUARD));
        for (int n = 0; n < i; n++) {
            ASSERT("no two soldiers share a post", g->enemies[n].x != g->enemies[i].x || g->enemies[n].y != g->enemies[i].y);
        }
    }
    ASSERT("Captain Rusk leads the waterfront command", strcmp(g->enemies[DAWNMERE_RUSK].name, "Captain Rusk") == 0 && g->enemies[DAWNMERE_RUSK].max_hp == 150);
    ASSERT("forcing a way in starts only the checkpoint squad's attack", eastern_handle_key(g, SDL_SCANCODE_N, 0) && !eastern_challenge_active(g) && g->location == LOCATION_DAWNMERE && g->dawnmere_alerted &&
        g->enemies[DAWNMERE_SQUAD - 1].boss_aux_x && !g->enemies[DAWNMERE_SQUAD].boss_aux_x && recent_message(g, "force your way into Dawnmere"));
    ASSERT("Dawnmere has the northern waterfront and eastern Stormwatch gates, with no southern branch", g->map.tiles[0][20] == TILE_TOWN_EXIT && g->map.tiles[24][20] == TILE_WALL && g->map.tiles[12][TOWN_W - 1] == TILE_TOWN_EXIT);
    int before = g->player.x;
    g->player.x = 20;
    g->player.y = 1;
    action_resolve_player(g, (Action){ACTION_MOVE, 20, 0});
    ASSERT("the occupied waterfront blocks travel and points to the road around town", g->location == LOCATION_DAWNMERE && g->player.y == 1 && strstr(g->messages[g->message_count - 1], "walk around the town"));
    g->player.x = before;
    g->player.y = 12;
    ASSERT("the sheltered Alehouse can be approached by a side lane", journey_to(g, 1, 22) && journey_to(g, 28, 22) && journey_to(g, 28, 11));
    action_resolve_player(g, (Action){ACTION_MOVE, 28, 10});
    ASSERT("the occupation contact is sheltered inside the Alehouse", g->location == LOCATION_ALEHOUSE && g->map.tiles[DAWNMERE_SEREN_Y][DAWNMERE_SEREN_X] == TILE_NPC_SEREN);
    ASSERT("Seren is reachable and offers liberation in a normal dialogue", journey_to(g, DAWNMERE_SEREN_X, DAWNMERE_SEREN_Y + 1) && eastern_talk(g) && game_quest_offer_active(g));
    g->alder_quest_state = 1;
    g->dain_quest_state = 1;
    g->mara_quest_state = 1;
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("the liberation quest respects the existing three-active limit", !g->dawnmere_quest_state && game_quest_offer_at_limit(g));
    g->mara_quest_state = 2;
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("accepting liberation uses one free active slot", g->dawnmere_quest_state == 1 && game_active_quest_count(g) == 3);
    ASSERT("leaving the shelter restores the same garrison", journey_to(g, g->map.stairs_down_x, g->map.stairs_down_y) && g->location == LOCATION_DAWNMERE && g->enemy_count == DAWNMERE_GARRISON);
    g->enemies[DAWNMERE_SQUAD + 1].hp -= 11;
    g->enemies[0].active = 0;
    eastern_update(g);
    ASSERT("a struck squad says why it joins the fight", recent_message(g, "You attacked the market squad") && g->enemies[2 * DAWNMERE_SQUAD - 1].boss_aux_x && !g->enemies[DAWNMERE_RUSK].boss_aux_x);
    ASSERT("wounds and casualties survive entering an interior and saving there", save_game(g, EASTERN_SLOT) && load_game(loaded, EASTERN_SLOT) && loaded->enemies[DAWNMERE_SQUAD + 1].hp == g->enemies[DAWNMERE_SQUAD + 1].hp && !loaded->enemies[0].active && loaded->dawnmere_alerted);
    g->player.level = 10;
    g->player.experience_next = 1000;
    int gold = g->gold;
    int xp = g->player.experience;
    for (int i = 0; i < DAWNMERE_SQUAD; i++) {
        g->enemies[i].active = 0;
    }
    eastern_update(g);
    eastern_update(g);
    ASSERT("a cleared checkpoint is reported once in the message bar", g->dawnmere_progress == 1 && strcmp(g->messages[g->message_count - 1], "Checkpoint cleared.") == 0 && strcmp(g->messages[g->message_count - 2], "Checkpoint cleared.") != 0);
    for (int i = 0; i <= DAWNMERE_RUSK; i++) {
        g->enemies[i].active = 0;
    }
    eastern_update(g);
    ASSERT("Rusk's command fights on after he falls instead of withdrawing at once", !g->dawnmere_liberated && g->dawnmere_progress == 3 && g->enemies[DAWNMERE_GARRISON - 1].active);
    for (int i = DAWNMERE_RUSK; i < DAWNMERE_GARRISON; i++) {
        g->enemies[i].active = 0;
    }
    eastern_update(g);
    ASSERT("checkpoint, market and the whole command complete liberation", g->dawnmere_liberated && g->dawnmere_progress == 7 && g->dawnmere_quest_state == 3 && g->last_town_visited == LOCATION_DAWNMERE &&
        recent_message(g, "Rusk's command is broken.") && recent_message(g, "Dawnmere is liberated!"));
    eastern_update(g);
    ASSERT("liberation awards gold and XP only once and frees the active quest slot", g->gold == gold + 100 && g->player.experience == xp + 500 && game_active_quest_count(g) == 2);
    ASSERT("Rusk's royal order enters Evidence without revealing the Royal Arcanist", royal_evidence_pending(g) == ROYAL_EVIDENCE_CONFISCATION && strstr(royal_evidence_document(4)->text, "Stillbury") && !strstr(royal_evidence_document(4)->text, "Arcanist"));
    ASSERT("liberated Seren offers the chained marsh journey", journey_to(g, SEREN_X, SEREN_Y + 1) && eastern_talk(g) && game_quest_offer_active(g));
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("the follow-up becomes the next active eastern objective", g->saltwind_quest_state == 1 && game_active_quest_count(g) == 3);
    ASSERT("liberation, royal recognition, evidence and both quests round-trip", save_game(g, EASTERN_SLOT) && load_game(loaded, EASTERN_SLOT) && loaded->dawnmere_liberated && loaded->royal_reported_actions == 1 && loaded->dawnmere_quest_state == 3 && loaded->saltwind_quest_state == 1);
    g->dialogue_active = 0;
    ASSERT("the liberated Alehouse can be entered from the square", journey_to(g, 28, 11) && journey_to(g, 28, 10) && g->location == LOCATION_ALEHOUSE);
    ASSERT("the reopened Alehouse host talks instead of selling provisions", journey_to(g, RESIDENT_X, RESIDENT_Y + 1) && !town_life_near_shopkeeper(g) && town_life_talk(g, 0));
    ASSERT("the host explains that the King closed the regular crossing and Saltwind is the way over", strstr(g->dialogue_text, "closed by order of the King") && strstr(g->dialogue_text, "Saltwind Marsh"));
}

static void marsh_journey(GameState *g, GameState *loaded, int seed) {
    game_init(g);
    srand(seed);
    g->dawnmere_liberated = 1;
    g->dawnmere_quest_state = 3;
    g->dawnmere_progress = 7;
    g->royal_reported_actions = 1;
    g->player.hp = 1000000;
    g->aldric_party_state = ALDRIC_FOLLOWING;
    g->aldric_hp = 37;
    game_enter_town_gate(g, LOCATION_DAWNMERE, 20, 0);
    action_resolve_player(g, (Action){ACTION_MOVE, 20, 0});
    int x;
    int y;
    town_road_exit(TOWN_SIDE_NORTH, &x, &y);
    ASSERT("Dawnmere departs north to its river-facing exterior and then the marsh approach", g->location == LOCATION_TOWN_ROAD && journey_to(g, x, y) && g->location == LOCATION_MANGROVE_PATH && g->player.y == MANGROVE_PATH_H - 2 && g->player.last_dy == -1);
    ASSERT("the approach reaches its real entrance facing north", journey_to(g, MANGROVE_GATE_X, MANGROVE_GATE_Y) && g->location == LOCATION_MANGROVES && g->level == 1 && g->player.y == MANGROVE_H - 2 && g->player.last_dy == -1);
    for (int stage = 1; stage <= MANGROVE_DEPTH; stage++) {
        defeat_regulars(g);
        ASSERT("each generated marsh stage connects the real entry, boat and onward shore", sail_stage(g, 1));
        int ex = g->map.stairs_down_x;
        int ey = g->map.stairs_down_y;
        if (stage == MANGROVE_DEPTH) {
            ASSERT("the final launch stays blocked until Sszareth is defeated", !journey_to(g, ex, ey) && g->location == LOCATION_MANGROVES);
            g->mangrove_defeated = 1;
        }
        ASSERT("walking across the actual departure tile advances to the next stage or river", journey_to(g, ex, ey) && (stage == 5 ? g->location == LOCATION_RIVER_CROSSING : g->location == LOCATION_MANGROVES && g->level == stage + 1));
        ASSERT("marsh boundaries connect to the opposite edge in the same direction", ex == 63 ? g->player.x == 1 : ey == 39 ? g->player.y == 1 : ey == 0 ? g->player.y == 38 : 0);
    }
    ASSERT("river arrival is on the south bank with a real skiff", journey_to(g, RIVER_SOUTH_X, RIVER_SOUTH_Y) && mangrove_interact(g) && mangrove_aboard(g));
    ASSERT("party boarding preserves living companion health", g->aldric_party_state == ALDRIC_FOLLOWING && g->aldric_hp > 0);
    ASSERT("river navigation supports saving mid-crossing", journey_to(g, 30, 20) && save_game(g, EASTERN_SLOT) && load_game(loaded, EASTERN_SLOT) && mangrove_aboard(loaded) && loaded->player.x == 30 && loaded->river_boat.boat_y == 20);
    ASSERT("the loaded skiff reaches Stillbury's landing with no teleport menu", boat_to(loaded, RIVER_NORTH_X, RIVER_NORTH_Y) && loaded->saltwind_quest_state == 3 && !loaded->act4_arrived);
    ASSERT("dock discovery and the completed journey persist", save_game(loaded, EASTERN_SLOT) && load_game(g, EASTERN_SLOT) && g->saltwind_quest_state == 3 && !g->act4_arrived);
    ASSERT("leaving the north bank reaches Stillbury's opposite-boundary approach", journey_to(g, RIVER_NORTH_X, 0) && g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN2 && g->player.y == TOWN_ROAD_H - 2 && !g->act4_arrived);
    town_road_entrance(TOWN_SIDE_SOUTH, &x, &y);
    ASSERT("walking into Stillbury starts Act 4 and retains a safe liberated Dawnmere anchor", journey_to(g, x, y) && g->location == LOCATION_TOWN2 && g->act4_arrived && g->last_town_visited == LOCATION_DAWNMERE && g->player.last_dy == -1);
    ASSERT("Stillbury has river, bridge, Blackwater and Elderwood entrances", g->map.tiles[0][20] == TILE_TOWN_EXIT && g->map.tiles[12][0] == TILE_TOWN_EXIT && g->map.tiles[12][TOWN_W - 1] == TILE_TOWN_EXIT);
    ASSERT("known anti-Crown actions trigger a checkpoint choice inside town", journey_to(g, 20, 20) && eastern_move(g, 20, 19) && g->stillbury_prompt);
    eastern_handle_key(g, SDL_SCANCODE_N, 0);
    g->enemies[0].hp -= 9;
    ASSERT("resisting activates a persistent royal guard squad", g->stillbury_challenge == 2 && g->enemy_count == 3 && save_game(g, EASTERN_SLOT) && load_game(loaded, EASTERN_SLOT) && loaded->enemies[0].hp == g->enemies[0].hp);
    ASSERT("the hostile checkpoint leaves the dock retreat available", journey_to(g, 20, TOWN_H - 1) && g->location == LOCATION_TOWN_ROAD);
    town_road_exit(TOWN_SIDE_SOUTH, &x, &y);
    ASSERT("Stillbury's exterior returns to the river's north bank facing south", journey_to(g, x, y) && g->location == LOCATION_RIVER_CROSSING && g->player.y == 1 && g->player.last_dy == 1);
    ASSERT("the same skiff supports the return crossing", journey_to(g, RIVER_NORTH_X, RIVER_NORTH_Y) && mangrove_interact(g) && mangrove_aboard(g) && boat_to(g, RIVER_SOUTH_X, RIVER_SOUTH_Y));
    ASSERT("the south bank returns to the preserved final marsh stage", journey_to(g, RIVER_SOUTH_X, MANGROVE_H - 1) && g->location == LOCATION_MANGROVES && g->level == 5 && g->player.y == 1 && g->player.last_dy == 1);
    for (int stage = MANGROVE_DEPTH; stage >= 1; stage--) {
        ASSERT("every cached marsh stage retains a usable boat retreat", sail_stage(g, 0));
        int ex = g->map.stairs_up_x;
        int ey = g->map.stairs_up_y;
        ASSERT("retreat crosses the real boundary back through earlier stages", journey_to(g, ex, ey) && (stage == 1 ? g->location == LOCATION_MANGROVE_PATH : g->location == LOCATION_MANGROVES && g->level == stage - 1));
    }
    ASSERT("the southern approach returns to Dawnmere's north exterior", journey_to(g, MANGROVE_GATE_X, MANGROVE_PATH_H - 1) && g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_DAWNMERE);
    town_road_entrance(TOWN_SIDE_NORTH, &x, &y);
    ASSERT("the whole return journey re-enters Dawnmere through its north gate", journey_to(g, x, y) && g->location == LOCATION_DAWNMERE && g->player.y == 1 && g->dawnmere_liberated);
}

static int rewrite_eastern_save(int mode) {
    FILE *file = fopen("saves/savegame_99256.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *data = calloc((size_t)length + 1, 1);
    fread(data, 1, (size_t)length, file);
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    if (mode == 0) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 235);
        cJSON_DeleteItemFromObject(root, "eastern_arc");
        cJSON *player = cJSON_GetObjectItem(root, "player");
        cJSON_SetNumberValue(cJSON_GetObjectItem(player, "x"), 14);
        cJSON_SetNumberValue(cJSON_GetObjectItem(player, "y"), 1);
        cJSON *loot = cJSON_GetArrayItem(cJSON_GetObjectItem(root, "floor_items"), 0);
        cJSON_SetNumberValue(cJSON_GetObjectItem(loot, "x"), 14);
        cJSON_SetNumberValue(cJSON_GetObjectItem(loot, "y"), 1);
    } else {
        cJSON *arc = cJSON_GetObjectItem(root, "eastern_arc");
        if (mode == 1) {
            cJSON_SetNumberValue(cJSON_GetObjectItem(arc, "dawnmere_progress"), 8);
        } else if (mode == 2) {
            cJSON_DeleteItemFromObject(arc, "river_boat");
        } else if (mode == 3) {
            cJSON_SetNumberValue(cJSON_GetObjectItem(arc, "saltwind_quest_state"), 1.5);
        } else if (mode == 5) {
            cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 236);
        } else if (mode == 6) {
            cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 237);
        } else {
            cJSON *boat = cJSON_GetObjectItem(arc, "river_boat");
            cJSON_SetNumberValue(cJSON_GetObjectItem(boat, "boat_x"), -1);
        }
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99256.json", "wb");
    int ok = file && data && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static void eastern_persistence(GameState *g, GameState *loaded) {
    game_init(g);
    mangrove_enter(g, 0);
    g->player.hp = 37;
    g->gold = 413;
    g->spice_skiff_progress = 1;
    g->alder_quest_state = 1;
    g->enemies[0].hp -= 3;
    int wound = g->enemies[0].hp;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 14, .y = 7, .underlying_tile = g->map.tiles[7][14], .item = item_make_ring_of_vigor()};
    g->map.tiles[7][14] = TILE_ITEM;
    map_mark_explored(&g->map, 14, 8);
    ASSERT("legacy marsh saves repair the retired northern jetty without losing progress", save_game(g, EASTERN_SLOT) && rewrite_eastern_save(0) && load_game(loaded, EASTERN_SLOT) && loaded->location == LOCATION_MANGROVES && loaded->map.stairs_up_y == 39 && map_is_walkable(&loaded->map, loaded->player.x, loaded->player.y));
    ASSERT("river-side repairs preserve wounds, exploration, loot, money and quests", loaded->player.hp == 37 && loaded->gold == 413 && loaded->enemies[0].hp == wound && loaded->spice_skiff_progress == 1 && loaded->alder_quest_state == 1 && loaded->floor_items[0].active && loaded->floor_items[0].item.type == ITEM_ACCESSORY && map_is_walkable(&loaded->map, loaded->floor_items[0].x, loaded->floor_items[0].y) && map_is_explored(&loaded->map, 14, 8));
    ASSERT("repaired active and cached marsh records remain stable on another save", save_game(loaded, EASTERN_SLOT) && load_game(g, EASTERN_SLOT) && g->mangrove_stages[0].enemies[0].hp == wound);
    game_init(g);
    eastern_river_enter(g, 0);
    for (int mode = 1; mode <= 4; mode++) {
        ASSERT("malformed eastern state is rejected rather than silently resetting", save_game(g, EASTERN_SLOT) && rewrite_eastern_save(mode) && !load_game(loaded, EASTERN_SLOT));
    }
    game_init(g);
    g->aldric_party_state = ALDRIC_DEAD;
    g->aldric_recruited = 1;
    g->aldric_hp = 0;
    eastern_river_enter(g, 0);
    journey_to(g, RIVER_SOUTH_X, RIVER_SOUTH_Y);
    mangrove_interact(g);
    ASSERT("boarding and docking never resurrect a fallen passenger", boat_to(g, RIVER_NORTH_X, RIVER_NORTH_Y) && g->aldric_party_state == ALDRIC_DEAD && !g->aldric_hp);
    game_init(g);
    game_enter_town_gate(g, LOCATION_TOWN2, 20, 24);
    ASSERT("unreported characters receive a peaceful Stillbury welcome", !g->stillbury_prompt && !g->stillbury_challenge && g->last_town_visited == LOCATION_TOWN2 && !eastern_move(g, 20, 19));
    g->royal_reported_actions = 2;
    ASSERT("future public western actions share the checkpoint consequence", eastern_move(g, 20, 19) && g->stillbury_prompt);
    ASSERT("an unanswered checkpoint survives save/load", save_game(g, EASTERN_SLOT) && load_game(loaded, EASTERN_SLOT) && loaded->stillbury_prompt && loaded->dialogue_active);
    eastern_handle_key(loaded, SDL_SCANCODE_Y, 0);
    ASSERT("submission turns the player back toward the dock without inventing Act 4 imprisonment", !loaded->stillbury_prompt && !loaded->stillbury_challenge && loaded->player.y == 22 && !loaded->enemy_count);
    loaded->last_town_visited = LOCATION_TOWN2;
    game_open_town_portal(loaded);
    ASSERT("wanted Stillbury cannot become an unsafe Return to Town destination", loaded->location == LOCATION_TOWN && loaded->last_town_visited == LOCATION_TOWN);
}

static void liberation_without_offer(GameState *g) {
    game_init(g);
    g->player.attack = 1000;
    g->player.defense = 1000;
    g->player.hp = g->player.max_hp = 100000;
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    for (int i = 0; i < DAWNMERE_GARRISON; i++) {
        Enemy *enemy = &g->enemies[i];
        int tries = 0;
        while (enemy->active && tries++ < 40) {
            g->player.x = enemy->x - 1;
            g->player.y = enemy->y;
            action_resolve_player(g, (Action){ACTION_MOVE, enemy->x, enemy->y});
        }
    }
    ASSERT("actual royal troop combat liberates Dawnmere without accepting a quest", g->dawnmere_liberated && g->dawnmere_quest_state == 3 && g->royal_reported_actions == 1 && !game_active_quest_count(g));
    QuestJournalEntry entry;
    ASSERT("liberation objectives immediately appear under Completed", quest_journal_get_entry(g, QUEST_TAB_COMPLETED, 0, &entry) && strcmp(entry.title, "Dawnmere Under Occupation") == 0);
}

static void checkpoint_choice(GameState *g, GameState *loaded) {
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    int x = g->player.x;
    int y = g->player.y;
    ASSERT("the challenge waits for an answer instead of letting the player walk past", eastern_challenge_active(g) && eastern_handle_key(g, SDL_SCANCODE_D, 0) && eastern_challenge_active(g) && g->player.x == x && g->player.y == y);
    ASSERT("an unanswered challenge survives save/load", save_game(g, EASTERN_SLOT) && load_game(loaded, EASTERN_SLOT) && eastern_challenge_active(loaded));
    ASSERT("leaving town returns to the road outside the west gate without a fight", eastern_handle_key(g, SDL_SCANCODE_Y, 0) && g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_DAWNMERE &&
        !g->dawnmere_alerted && !g->dialogue_active && recent_message(g, "Saltwind north, Stormwatch Headland east"));
    town_road_entrance(TOWN_SIDE_WEST, &x, &y);
    ASSERT("every west gate arrival repeats the challenge", journey_to(g, x, y) && g->location == LOCATION_DAWNMERE && eastern_challenge_active(g));
    ASSERT("Esc also leaves town", eastern_handle_key(g, SDL_SCANCODE_ESCAPE, 0) && g->location == LOCATION_TOWN_ROAD && !g->dawnmere_alerted);
    town_road_enter(g, LOCATION_DAWNMERE, TOWN_SIDE_NORTH, 0);
    town_road_entrance(TOWN_SIDE_NORTH, &x, &y);
    ASSERT("the held waterfront gate also stays closed from outside, leaving the challenged west gate as the way in", !journey_to(g, x, y) && g->location == LOCATION_TOWN_ROAD && recent_message(g, "garrison holds this gate"));
}

static void garrison_warnings(GameState *g) {
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    int engaged = 0;
    while (!engaged && g->player.x < 8) {
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + 1, 12});
        engaged = g->enemies[0].boss_aux_x;
    }
    ASSERT("walking up to the checkpoint closes the challenge and says why its squad attacks", engaged && !g->dialogue_active && recent_message(g, "You came too close to the checkpoint squad"));
    ASSERT("only the provoked squad joins the fight", !g->enemies[DAWNMERE_SQUAD].boss_aux_x && !g->enemies[DAWNMERE_RUSK].boss_aux_x);
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    g->player.x = 6;
    g->player.y = 10;
    action_resolve_player(g, (Action){ACTION_MOVE, 7, 10});
    ASSERT("crossing the checkpoint line names the reason for the attack", g->enemies[0].boss_aux_x && g->dawnmere_alerted && recent_message(g, "You crossed the royal checkpoint"));
}

static void garrison_reinforcement(GameState *g, GameState *loaded) {
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    Enemy posts[DAWNMERE_GARRISON];
    memcpy(posts, g->enemies, sizeof(posts));
    // Rebuild the former three-soldier squads, which shared their first three posts.
    for (int i = 0; i < 9; i++) {
        g->enemies[i] = posts[i / 3 * DAWNMERE_SQUAD + i % 3];
    }
    g->enemy_count = 9;
    g->enemies[0].hp -= 9;
    g->enemies[1].active = 0;
    for (int i = 3; i < 6; i++) {
        g->enemies[i].active = 0;
    }
    g->dawnmere_alerted = 1;
    g->player.x = posts[3].x;
    g->player.y = posts[3].y;
    ASSERT("a nine-soldier occupation save gains the larger garrison", save_game(g, EASTERN_SLOT) && rewrite_eastern_save(5) && load_game(loaded, EASTERN_SLOT) && loaded->enemy_count == DAWNMERE_GARRISON && loaded->dawnmere_cache.enemy_count == DAWNMERE_GARRISON);
    ASSERT("existing wounds and casualties stay with their squads", loaded->enemies[0].hp == posts[0].max_hp - 9 && !loaded->enemies[1].active && loaded->enemies[2].active && strcmp(loaded->enemies[DAWNMERE_RUSK].name, "Captain Rusk") == 0);
    ASSERT("a broken squad is not refilled", !loaded->enemies[DAWNMERE_SQUAD].active && !loaded->enemies[DAWNMERE_SQUAD + 3].active && !loaded->enemies[DAWNMERE_SQUAD + 4].active);
    Enemy *blocked = &loaded->enemies[3];
    ASSERT("reinforcements join holding squads beside an occupied post", blocked->active && map_is_walkable(&loaded->map, blocked->x, blocked->y) && (blocked->x != loaded->player.x || blocked->y != loaded->player.y) &&
        abs(blocked->x - posts[3].x) <= 1 && abs(blocked->y - posts[3].y) <= 1 && loaded->enemies[4].active && loaded->enemies[DAWNMERE_RUSK + 4].active);
    ASSERT("the reinforced garrison round-trips", save_game(loaded, EASTERN_SLOT) && load_game(g, EASTERN_SLOT) && g->enemy_count == DAWNMERE_GARRISON && g->enemies[3].x == blocked->x && g->enemies[3].y == blocked->y);
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    memcpy(posts, g->enemies, sizeof(posts));
    // Rebuild the former five-soldier squads, which shared their first five posts.
    for (int i = 0; i < 15; i++) {
        g->enemies[i] = posts[i / 5 * DAWNMERE_SQUAD + i % 5];
    }
    g->enemy_count = 15;
    for (int i = 0; i < 5; i++) {
        g->enemies[i].active = 0;
    }
    g->enemies[6].hp -= 7;
    g->dawnmere_alerted = 1;
    ASSERT("a fifteen-soldier occupation save gains the seven-soldier squads", save_game(g, EASTERN_SLOT) && rewrite_eastern_save(6) && load_game(loaded, EASTERN_SLOT) && loaded->enemy_count == DAWNMERE_GARRISON && loaded->dawnmere_cache.enemy_count == DAWNMERE_GARRISON);
    ASSERT("wounds stay with their squads and only holding squads gain a guard and an archer", loaded->enemies[DAWNMERE_SQUAD + 1].hp == posts[DAWNMERE_SQUAD + 1].max_hp - 7 && !loaded->enemies[5].active && !loaded->enemies[6].active &&
        loaded->enemies[DAWNMERE_SQUAD + 5].type == ENEMY_ROYAL_GUARD && loaded->enemies[DAWNMERE_SQUAD + 6].type == ENEMY_ROAD_ARCHER && loaded->enemies[DAWNMERE_SQUAD + 6].active && loaded->enemies[DAWNMERE_RUSK + 5].active);
}

static void dawnmere_bypass(GameState *g) {
    game_init(g);
    town_road_enter(g, LOCATION_DAWNMERE, TOWN_SIDE_WEST, 0);
    ASSERT("arriving outside occupied Dawnmere offers both roads around town", recent_message(g, "Saltwind north, Stormwatch Headland east"));
    int x;
    int y;
    town_road_exit(TOWN_SIDE_NORTH, &x, &y);
    ASSERT("the forest road reaches the marsh approach around occupied Dawnmere without a fight", journey_to(g, x, y) && g->location == LOCATION_MANGROVE_PATH && !g->dawnmere_liberated && !g->dawnmere_alerted && !g->royal_reported_actions);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    ASSERT("the occupied Alehouse can be reached without alerting the garrison", journey_to(g, 1, 22) && journey_to(g, 28, 22) && journey_to(g, 28, 11) && journey_to(g, 28, 10) && g->location == LOCATION_ALEHOUSE && !g->dawnmere_alerted);
    ASSERT("the occupied Alehouse host points around the walls to Saltwind", journey_to(g, RESIDENT_X, RESIDENT_Y + 1) && town_life_talk(g, 0) && strstr(g->dialogue_text, "closed by order of the King") && strstr(g->dialogue_text, "walk around our walls"));
}

static void royal_tactics(GameState *g) {
    const int step[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int side = 0; side < 4; side++) {
        game_init(g);
        game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
        for (int y = 1; y < TOWN_H - 1; y++) {
            for (int x = 1; x < TOWN_W - 1; x++) {
                g->map.tiles[y][x] = TILE_TOWN_PATH;
            }
        }
        g->player.x = 20;
        g->player.y = 12;
        g->enemy_count = 3;
        int fx = step[side][0];
        int fy = step[side][1];
        for (int i = 0; i < 3; i++) {
            g->enemies[i].type = i == 2 ? ENEMY_ROAD_ARCHER : ENEMY_ROYAL_GUARD;
            g->enemies[i].x = 20 + fx * 5 - fy * (i - 1);
            g->enemies[i].y = 12 + fy * 5 + fx * (i - 1);
            g->enemies[i].boss_aux_y = 1;
        }
        int hp = g->player.hp;
        int moved = 0;
        for (int turn = 0; turn < 12; turn++) {
            for (int i = 0; i < 3; i++) {
                moved |= enemy_tactics_move(g, i) == 1;
            }
        }
        ASSERT("royal squads form melee shoulders with an archer behind from every direction", moved && g->player.hp == hp && abs(g->enemies[2].x - 20) + abs(g->enemies[2].y - 12) >= 3 && (g->enemies[0].x != g->enemies[1].x || g->enemies[0].y != g->enemies[1].y));
    }
}

// Firing line scenarios use the first five slots: three guards and two archers.
static void place_squad(GameState *g, const int posts[5][2]) {
    g->enemy_count = 5;
    for (int i = 0; i < 5; i++) {
        g->enemies[i].active = 1;
        g->enemies[i].hp = g->enemies[i].max_hp;
        g->enemies[i].x = posts[i][0];
        g->enemies[i].y = posts[i][1];
        g->enemies[i].boss_aux_x = 1;
    }
}

static void royal_firing_line(GameState *g) {
    // Slots 2 and 4 are archers, placed nearer the player than the rear guard.
    const int advance[5][2] = {{26, 11}, {26, 13}, {24, 12}, {30, 12}, {25, 13}};
    const int contact[5][2] = {{21, 12}, {21, 11}, {24, 12}, {23, 15}, {26, 13}};
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    g->dialogue_active = 0;
    for (int y = 1; y < TOWN_H - 1; y++) {
        for (int x = 1; x < TOWN_W - 1; x++) {
            g->map.tiles[y][x] = TILE_TOWN_PATH;
        }
    }
    g->player.x = 20;
    g->player.y = 12;
    g->player.hp = g->player.max_hp = 100000;
    g->dawnmere_alerted = 1;
    place_squad(g, advance);
    action_resolve_enemies(g);
    int advanced = 1;
    for (int i = 0; i < 5; i++) {
        if (g->enemies[i].type == ENEMY_ROYAL_GUARD) {
            advanced &= g->enemies[i].x != advance[i][0] || g->enemies[i].y != advance[i][1];
        }
    }
    ASSERT("archers hold a firing line and leave every pursuit slot to the squad's guards", advanced && g->enemies[3].x < 30);
    place_squad(g, contact);
    action_resolve_enemies(g);
    ASSERT("a guard blocking the archers' lane steps to a shoulder instead of striking", g->enemies[0].x == 21 && g->enemies[0].y == 13 && g->enemies[1].x == 21 && g->enemies[1].y == 11);
    ASSERT("the center lane stays open for the archer behind the guards", g->enemies[2].y == 12 && g->enemies[2].x >= 22);
}

void test_eastern_arc(void) {
    printf("\nAct 3 eastern arc:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    occupation(g, loaded);
    liberation_without_offer(g);
    checkpoint_choice(g, loaded);
    garrison_warnings(g);
    garrison_reinforcement(g, loaded);
    dawnmere_bypass(g);
    royal_tactics(g);
    royal_firing_line(g);
    eastern_persistence(g, loaded);
    for (int seed = 0; seed < 4; seed++) {
        marsh_journey(g, loaded, 9020 + seed);
    }
    remove("saves/savegame_99256.json");
    free(loaded);
    free(g);
}
