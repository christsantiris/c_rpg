#include "game/combat_feedback.h"
#include "game/heartseed.h"
#include "expedition_fixture.h"
#include "test_utils.h"
#include "northroad_checks.h"
#include "town_road_checks.h"
#include "game/elderwood.h"
#include "systems/save_load.h"
#include "screens/quest_journal.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define ELDER_SLOT 99282

// Search the real map, treating other transition markers as obstacles.
static int path(GameState *g, int tx, int ty, int walk) {
    int previous[ELDERWOOD_W * ELDERWOOD_H];
    int queue[ELDERWOOD_W * ELDERWOOD_H];
    for (int i = 0; i < ELDERWOOD_W * ELDERWOOD_H; i++) {
        previous[i] = -1;
    }
    int start = g->player.y * ELDERWOOD_W + g->player.x;
    int target = ty * ELDERWOOD_W + tx;
    previous[start] = start;
    queue[0] = start;
    int head = 0;
    int tail = 1;
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    while (head < tail && previous[target] < 0) {
        int pos = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = pos % ELDERWOOD_W + dx[side];
            int y = pos / ELDERWOOD_W + dy[side];
            if (x < 0 || x >= ELDERWOOD_W || y < 0 || y >= ELDERWOOD_H) {
                continue;
            }
            int next = y * ELDERWOOD_W + x;
            TileType tile = g->map.tiles[y][x];
            int transition = tile == TILE_NORTH_CATACOMBS || tile == TILE_NORTH_ELDERWOOD ||
                tile == TILE_NORTH_CASTLE || tile == TILE_NORTH_OAKHAVEN ||
                tile == TILE_ELDERWOOD_ENTRANCE || tile == TILE_ELDERWOOD_EXIT;
            if (previous[next] >= 0 || (next != target && (transition || !map_is_walkable(&g->map, x, y))) ||
                (next == target && !map_is_walkable(&g->map, x, y) && !transition)) {
                continue;
            }
            previous[next] = pos;
            queue[tail++] = next;
        }
    }
    if (previous[target] < 0) {
        return 0;
    }
    int count = 0;
    for (int next = target; next != start; next = previous[next]) {
        queue[count++] = next;
    }
    if (walk) {
        for (int i = count - 1; i >= 0; i--) {
            action_resolve_player(g, (Action){ACTION_MOVE, queue[i] % ELDERWOOD_W, queue[i] / ELDERWOOD_W});
        }
    }
    return 1;
}

static int legacy(int slot) {
    char filename[80];
    snprintf(filename, sizeof(filename), "saves/savegame_%d.json", slot);
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = malloc(size + 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    size_t read = fread(data, 1, size, file);
    data[read] = 0;
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 181);
    test_mark_legacy_portal(root);
    const char *fields[] = {"elderwood_cache", "elderwood_defeated", "max_elderwood_level_reached", "elderwood_entry_town", "catacombs_entry_town", "northern_portal_town"};
    for (int i = 0; i < 6; i++) {
        cJSON_DeleteItemFromObject(root, fields[i]);
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(filename, "wb");
    int ok = data && file && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static void mechanics(GameState *g) {
    game_init(g);
    elderwood_enter(g);
    g->enemy_count = 1;
    for (int y = 10; y <= 24; y++) {
        for (int x = 10; x <= 24; x++) {
            g->map.tiles[y][x] = TILE_ELDERWOOD_FLOOR;
        }
    }
    g->player.x = 17;
    g->player.y = 15;
    g->player.defense = 0;
    Enemy *enemy = &g->enemies[0];
    *enemy = (Enemy){.type = ENEMY_ELDERTHORN, .active = 1, .is_boss = 1, .x = 12, .y = 15,
        .hp = 450, .max_hp = 450, .attack = 22, .defense = 10};
    int hp = g->player.hp;
    action_resolve_enemies(g);
    ASSERT("Elderthorn telegraphs a charge without immediately dealing damage", enemy->attack_phase && enemy->boss_attack == ELDER_CHARGE && elderwood_marks(enemy, 17, 15) && g->player.hp == hp);
    action_resolve_player(g, (Action){ACTION_MOVE, 17, 16});
    g->map.tiles[15][19] = TILE_ELDERWOOD_LOG;
    action_resolve_enemies(g);
    action_resolve_enemies(g);
    ASSERT("dodging a charge into a trunk exposes the heart without hurting the player", g->player.hp == hp && enemy->defense == 0 && enemy->move_timer == -3 && enemy->x == 18);
    for (int i = 0; i < 3; i++) {
        action_resolve_enemies(g);
    }
    ASSERT("Elderthorn recovers its bark armor after the punish window", enemy->defense == 10);
    enemy->hp = 200;
    action_resolve_enemies(g);
    action_resolve_enemies(g);
    action_resolve_enemies(g);
    ASSERT("the wounded boss summons a finite pair of wolves", g->enemy_count == 3 && enemy->boss_wave_used == 1 && g->enemies[1].type == ENEMY_BLIGHTED_WOLF && g->enemies[1].experience == 0);
    enemy->type = ENEMY_TREEFOLK;
    enemy->attack_phase = 0;
    enemy->move_timer = 0;
    enemy->x = 15;
    enemy->y = 16;
    action_resolve_enemies(g);
    ASSERT("Treefolk root attacks mark a locked target before hitting", enemy->boss_attack == ELDER_ROOTS && elderwood_marks(enemy, g->player.x, g->player.y));
    g->enemy_count = 0;
    g->map.tiles[g->player.y][g->player.x + 1] = TILE_ELDERWOOD_BRAMBLE;
    ASSERT("the bound action key recognizes adjacent Elderwood brambles", game_has_regional_interaction(g));
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("interacting cuts brambles into permanent traversable ground", map_is_walkable(&g->map, g->player.x + 1, g->player.y));
    ASSERT("cutting the last adjacent bramble clears the contextual interaction", !game_has_regional_interaction(g));
    game_descend(g);
    game_ascend(g);
    ASSERT("cut brambles and exploration persist when revisiting a stage", g->map.tiles[16][18] == TILE_ELDERWOOD_FLOOR);
}

static void test_elderwood_reentry(GameState *g, GameState *loaded) {
    game_init(g);
    g->heartseed_quest_state = 1;
    elderwood_enter(g);
    for (int level = 1; level <= ELDERWOOD_DEPTH; level++) {
        for (int n = 0; n < g->enemy_count; n++) {
            Enemy *e = &g->enemies[n];
            if (heartseed_is_guardian(g, e)) {
                e->hp = 7;
            } else {
                e->active = 0;
                e->hp = 0;
            }
        }
        g->map.tiles[16][18] = TILE_ELDERWOOD_FLOOR;
        map_mark_explored(&g->map, 18, 16);
        if (level < ELDERWOOD_DEPTH) {
            elderwood_change_level(g, 1);
        }
    }
    g->elderwood_defeated = 1;
    game_open_town_portal(g);
    ASSERT("escape stores dead Elderwood enemies without refreshing them in town", g->location == LOCATION_TOWN &&
        g->elderwood_cache[0].valid && !g->elderwood_cache[0].enemies[0].active);
    ASSERT("spent Elderwood expedition survives saving before re-entry", save_game(g, ELDER_SLOT) && load_game(loaded, ELDER_SLOT));
    elderwood_enter(loaded);
    int regular = 0;
    int preserved = 1;
    int refreshed = 1;
    for (int level = 1; level <= ELDERWOOD_DEPTH; level++) {
        const ElderwoodCache *cache = &loaded->elderwood_cache[level - 1];
        preserved &= cache->tiles[16][18] == TILE_ELDERWOOD_FLOOR;
        int bit = 16 * ELDERWOOD_W + 18;
        preserved &= !!(cache->explored[bit / 8] & (1u << (bit % 8)));
        for (int n = 0; n < cache->enemy_count; n++) {
            const Enemy *e = &cache->enemies[n];
            if (e->is_boss || strncmp(e->name, "Heartseed ", 10) == 0) {
                preserved &= !e->active && e->hp == 0;
            } else if (strcmp(e->name, "Bound Treefolk") == 0) {
                preserved &= e->active && e->hp == 7;
            } else if (e->experience > 0) {
                refreshed &= e->active && e->hp == e->max_hp && !e->attack_phase && !e->frozen_turns;
                regular++;
            }
        }
    }
    ASSERT("physical re-entry repopulates regular enemies on all five cached stages", refreshed && regular > 25 && loaded->level == 1);
    ASSERT("re-entry preserves boss deaths, quest guards, guardian wounds, cut paths and exploration", preserved && loaded->elderwood_defeated);
    loaded->enemies[0].hp = 7;
    elderwood_change_level(loaded, 1);
    elderwood_change_level(loaded, 0);
    ASSERT("backtracking within an expedition retains enemy wounds", loaded->enemies[0].hp == 7);
}

static void test_treefolk_combat(GameState *g) {
    game_init(g);
    g->location = LOCATION_ELDERWOOD;
    g->level = 1;
    for (int y = 0; y < ELDERWOOD_H; y++) {
        for (int x = 0; x < ELDERWOOD_W; x++) {
            g->map.tiles[y][x] = TILE_ELDERWOOD_FLOOR;
        }
    }
    g->player.x = 20;
    g->player.y = 20;
    g->player.attack = 30;
    g->enemy_count = 1;
    Enemy *tree = &g->enemies[0];
    *tree = (Enemy){.type = ENEMY_TREEFOLK, .name = "Treefolk", .active = 1, .x = 20, .y = 28,
        .hp = 500, .max_hp = 500, .attack = 15, .defense = 8, .experience = 90};
    action_resolve_enemies(g);
    ASSERT("attackable Treefolk still pursue the player", tree->y == 27);
    g->player.x = tree->x - 1;
    g->player.y = tree->y;
    int hp = tree->hp;
    action_resolve_player(g, (Action){ACTION_MOVE, tree->x, tree->y});
    ASSERT("bumping a Treefolk's occupied tile damages its health", tree->hp < hp && g->player.x == tree->x - 1);
    g->player.x = tree->x - 4;
    g->player.last_dx = 1;
    g->player.last_dy = 0;
    g->inventory[0] = item_make_longbow();
    g->inventory_count = 1;
    g->equipped_main_hand = 0;
    g->player.arrows = 10;
    hp = tree->hp;
    for (int n = 0; n < 3 && tree->hp == hp; n++) {
        action_resolve_player(g, (Action){ACTION_RANGED_ATTACK, 0, 0});
    }
    ASSERT("Treefolk can be damaged by arrows as well as melee", tree->hp < hp);
    tree->move_timer = 0;
    tree->attack_phase = 0;
    g->player.x = tree->x;
    g->player.y = tree->y - 4;
    int player_hp = g->player.hp;
    combat_feedback_clear();
    action_resolve_enemies(g);
    int tx = tree->attack_target_x;
    int ty = tree->attack_target_y;
    g->player.x += 3;
    action_resolve_enemies(g);
    action_resolve_enemies(g);
    int burst = 0;
    for (int n = 0; n < combat_feedback_count(); n++) {
        const CombatFeedbackEvent *event = combat_feedback_get(n);
        burst |= event->kind == FEEDBACK_ROOT_BURST && event->x == tx && event->y == ty && event->amount == 1;
    }
    ASSERT("root eruptions animate the fixed warned ground even after the player dodges", burst && g->player.hp == player_hp && tree->x == tx);
}

static void test_wolf_save_migration(GameState *g, GameState *loaded) {
    game_init(g);
    elderwood_enter(g);
    const int retired_ids[] = {95, 56, 56, 56};
    const char *names[] = {"Legacy Wolf", "Legacy Wolf", "Rimefang", "Expedition Guardian I"};
    g->enemy_count = 4;
    for (int n = 0; n < 4; n++) {
        g->enemies[n] = (Enemy){.type = retired_ids[n], .active = n != 2, .hp = n == 2 ? 0 : 7,
            .max_hp = 90, .x = 12 + n, .y = 12, .attack = 16, .defense = 4, .experience = 28,
            .move_timer = 2, .attack_phase = 1};
        snprintf(g->enemies[n].name, sizeof(g->enemies[n].name), "%s", names[n]);
    }
    elderwood_store(g);
    g->frostfell_cache[0].valid = 1;
    map_generate_frostfell(&g->frostfell_cache[0].map, 1);
    g->frostfell_cache[0].enemy_count = 4;
    memcpy(g->frostfell_cache[0].enemies, g->enemies, 4 * sizeof(Enemy));
    g->gold = 327;
    g->heartseed_quest_state = 1;
    ASSERT("retired wolf actor fixtures load without changing progress", save_game(g, ELDER_SLOT) && load_game(loaded, ELDER_SLOT) &&
        loaded->gold == 327 && loaded->heartseed_quest_state == 1);
    int converted = 1;
    for (int n = 0; n < 4; n++) {
        const Enemy *lists[] = {&loaded->enemies[n], &loaded->elderwood_cache[0].enemies[n], &loaded->frostfell_cache[0].enemies[n]};
        for (int k = 0; k < 3; k++) {
            const Enemy *e = lists[k];
            converted &= e->type == ENEMY_BLIGHTED_WOLF && e->hp == (n == 2 ? 0 : 7) &&
                e->max_hp == 90 && e->active == (n != 2) && e->attack == 16 && e->move_timer == 2;
        }
    }
    ASSERT("all retired wolves migrate in active and cached areas while preserving wounds, deaths and stats", converted);
    ASSERT("wolf migration preserves named hunt and quest encounters", strcmp(loaded->enemies[2].name, "Rimefang") == 0 &&
        strcmp(loaded->enemies[3].name, "Expedition Guardian I") == 0 && strcmp(loaded->enemies[0].name, "Blighted Wolf") == 0);
    ASSERT("retiring wolf variants leaves unrelated serialized enemy IDs unchanged", ENEMY_FROST_ARCHER == 57 &&
        ENEMY_IRONHIDE_BOAR == 96 && ENEMY_ROYAL_GUARD == 100 && ENEMY_GOBLIN_MINE_BOSS == 119);
}

void test_elderwood(void) {
    printf("Elderwood and the northern castle road:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("Elderwood fixtures allocate without adding giant static states", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    ASSERT("Elderwood save slot is unused", !save_exists(ELDER_SLOT));
    if (save_exists(ELDER_SLOT)) {
        free(g);
        free(loaded);
        return;
    }
    test_elderwood_reentry(g, loaded);
    test_treefolk_combat(g);
    test_wolf_save_migration(g, loaded);
    game_init(g);
    g->player.x = 20;
    g->player.y = 1;
    action_resolve_player(g, (Action){ACTION_MOVE, 20, 0});
    finish_town_departure(g);
    ASSERT("Oakhaven's north gate reaches the safe northern road", northern_approach(g) && g->enemy_count == 0 && g->player.last_dy == -1);
    ASSERT("Elderthorn blocks both the catacombs approach and the castle", !path(g, NORTH_CAT_X, NORTH_CAT_Y, 0) && !path(g, NORTH_ROAD_X, 0, 0));
    ASSERT("the Elderwood landmark is reachable from the road", path(g, NORTH_ELDER_X, NORTH_ELDER_Y, 1));
    ASSERT("walking into the sprite begins Elderwood stage one", g->location == LOCATION_ELDERWOOD && g->level == 1 && g->enemy_count > 0 && g->elderwood_entry_town == LOCATION_TOWN);
    g->enemies[0].hp = 7;
    g->enemies[0].attack_phase = 1;
    g->enemies[0].boss_attack = ELDER_CHARGE;
    g->enemies[0].move_timer = 2;
    g->gold = 393;
    game_descend(g);
    g->enemies[0].hp = 9;
    ASSERT("active and compact cached stages save successfully", save_game(g, ELDER_SLOT) && load_game(loaded, ELDER_SLOT));
    ASSERT("save/load retains active health, cache damage and pending attacks", loaded->level == 2 && loaded->enemies[0].hp == 9 && loaded->elderwood_cache[0].enemies[0].hp == 7 && loaded->elderwood_cache[0].enemies[0].attack_phase == 1 && loaded->gold == 393);
    game_ascend(loaded);
    ASSERT("ascending restores the previous stage without regeneration", loaded->level == 1 && loaded->enemies[0].hp == 7);
    game_open_town_portal(loaded);
    ASSERT("Elderwood escapes reach the last town without a return portal", loaded->location == LOCATION_TOWN && !loaded->portal_active && loaded->map.tiles[2][21] != TILE_PORTAL);
    ASSERT("Elderwood town portals survive a save/load round trip", save_game(loaded, ELDER_SLOT) && load_game(g, ELDER_SLOT));
    test_restore_cached_floor(g);
    ASSERT("the saved archived floor fixture resumes the exact living woods stage", g->location == LOCATION_ELDERWOOD && g->level == 1 && g->enemies[0].hp == 7 && !g->portal_active);
    game_return_to_approach(g);
    ASSERT("retreating returns beside the woodland sprite without entering town", northern_approach(g) && g->player.x == NORTH_ELDER_X && g->player.y == NORTH_ELDER_Y + 1);
    elderwood_enter(g);
    for (int i = 1; i < ELDERWOOD_DEPTH; i++) {
        game_descend(g);
    }
    Enemy *boss = &g->enemies[0];
    ASSERT("the fifth stage contains Elderthorn and Treefolk are the toughest regular enemies", boss->type == ENEMY_ELDERTHORN && g->enemies[1].type == ENEMY_TREEFOLK && g->enemies[1].hp > g->enemies[2].hp);
    g->player.x = boss->x;
    g->player.y = boss->y + 1;
    g->player.attack = 10000;
    action_resolve_player(g, (Action){ACTION_MOVE, boss->x, boss->y});
    ASSERT("an actual boss kill records Elderthorn separately from the legacy boss mask", g->elderwood_defeated && !boss->active && !(g->defeated_bosses & (1 << LOCATION_CATACOMBS)));
    BossJournalEntry entry;
    ASSERT("the boss journal records Elderthorn's defeat", quest_journal_get_boss(g, 16, &entry) && entry.defeated && !strcmp(entry.name, "Elderthorn"));
    game_return_to_approach(g);
    ASSERT("defeating Elderthorn opens the road to Catacombs while Castle stays locked", path(g, NORTH_CAT_X, NORTH_CAT_Y, 0) && !path(g, NORTH_ROAD_X, 0, 0));
    ASSERT("walking into the relocated Catacombs begins its original first floor", path(g, NORTH_CAT_X, NORTH_CAT_Y, 1) && g->location == LOCATION_CATACOMBS && g->level == 1 && g->catacombs_entry_town == LOCATION_TOWN && g->map.room_count > 0);
    g->enemies[0].hp = 8;
    game_leave_catacombs(g);
    ASSERT("Catacombs retreat preserves combat and returns to the shared road", northern_approach(g) && g->catacombs_cache[0].enemies[0].hp == 8 && g->player.x == NORTH_CAT_X);
    g->defeated_bosses |= 1 << LOCATION_CATACOMBS;
    northern_refresh(g);
    ASSERT("both victories open Crown Road North before Castle Grounds", path(g, NORTH_ROAD_X, 0, 1) && g->location == LOCATION_CROWNROAD_NORTH);
    walk_crownroad_north(g, 1);
    ASSERT("crossing the northern road reaches Castle Grounds", g->location == LOCATION_CASTLE && g->player.y == TOWN_H - 2);
    action_resolve_player(g, (Action){ACTION_MOVE, CROWNROAD_X, TOWN_H - 1});
    ASSERT("Castle's south gate reaches the occupied northern road", g->location == LOCATION_CROWNROAD_NORTH && g->enemy_count == NORTH_ENEMIES && g->player.last_dy == 1);
    walk_crownroad_north(g, 0);
    ASSERT("the open road can be walked south back to Oakhaven", path(g, NORTH_ROAD_X, ELDERWOOD_H - 1, 1) && g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN);
    walk_into_arrived_town(g);
    ASSERT("entering Oakhaven from the north places the player by its northern exit", g->location == LOCATION_TOWN && g->player.y == 1);
    game_init(g);
    g->debug_passages_unlocked = 1;
    for (int i = 0; i < town_approach_count; i++) {
        if (town_approaches[i].area == LOCATION_ELDERWOOD) {
            approach_enter_region(g, i);
            break;
        }
    }
    ASSERT("debug mode bypasses both road locks without granting boss victories", path(g, NORTH_ROAD_X, 0, 0) && !g->elderwood_defeated && !g->defeated_bosses);
    ASSERT("debug road access remains unlocked after loading", save_game(g, ELDER_SLOT) && load_game(loaded, ELDER_SLOT) && path(loaded, NORTH_ROAD_X, 0, 0));
    game_init(g);
    game_enter_catacombs(g);
    g->enemies[0].hp = 6;
    g->catacombs_quest_state = 1;
    g->gold = 777;
    ASSERT("older active Catacombs saves retain map, wounded enemies and quest progress", save_game(g, ELDER_SLOT) && legacy(ELDER_SLOT) && load_game(loaded, ELDER_SLOT) && loaded->enemies[0].hp == 6 && loaded->catacombs_quest_state == 1 && loaded->gold == 777 && !loaded->elderwood_defeated && !memcmp(loaded->map.tiles, g->map.tiles, sizeof(g->map.tiles)));
    game_leave_catacombs(loaded);
    ASSERT("older Catacombs saves retreat onto the Castle end of the northern road", northern_approach(loaded) && approach_route(loaded)->town == LOCATION_CASTLE);
    game_init(g);
    g->map.tiles[0][20] = TILE_WALL;
    g->defeated_bosses |= 1 << LOCATION_CATACOMBS;
    ASSERT("legacy Oakhaven saves gain the gate and retain earned Catacombs victory", save_game(g, ELDER_SLOT) && legacy(ELDER_SLOT) && load_game(loaded, ELDER_SLOT) && loaded->map.tiles[0][20] == TILE_TOWN_EXIT && (loaded->defeated_bosses & (1 << LOCATION_CATACOMBS)) && !loaded->elderwood_defeated);
    int connected = 1;
    for (int seed = 0; seed < 12; seed++) {
        srand(seed);
        game_init(g);
        elderwood_enter(g);
        for (int level = 1; level <= ELDERWOOD_DEPTH; level++) {
            connected &= path(g, 32, 1, 0);
            if (level < ELDERWOOD_DEPTH) {
                game_descend(g);
            }
        }
    }
    ASSERT("all five stages have routes around brambles across twelve generated expeditions", connected);
    // Passing Dawnmere on its exterior must not count as visiting it.
    game_enter_town_gate(g, LOCATION_TOWN2, TOWN_W - 1, 12);
    ASSERT("entering a town updates the saved last visit", g->last_town_visited == LOCATION_TOWN2);
    town_road_enter(g, LOCATION_DAWNMERE, TOWN_SIDE_WEST, 0);
    ASSERT("bypassing a town leaves the last visit unchanged", g->last_town_visited == LOCATION_TOWN2);
    for (int i = 0; i < town_approach_count; i++) {
        if (town_approaches[i].area == LOCATION_ELDERWOOD) {
            approach_enter_region(g, i);
            break;
        }
    }
    elderwood_enter(g);
    g->enemies[0].hp = 11;
    int portal_x = g->player.x;
    int portal_y = g->player.y;
    game_open_town_portal(g);
    ASSERT("a northern-area escape reaches the last visited town across region boundaries", g->location == LOCATION_TOWN2 && g->return_portal_town == LOCATION_TOWN2 && g->map.tiles[12][21] != TILE_PORTAL);
    ASSERT("last visit and portal destination survive save/load", save_game(g, ELDER_SLOT) && load_game(loaded, ELDER_SLOT) && loaded->last_town_visited == LOCATION_TOWN2 && loaded->return_portal_town == LOCATION_TOWN2);
    action_resolve_player(loaded, (Action){ACTION_MOVE, 21, 12});
    ASSERT("walking the former Elderwood portal tile stays in Stillbury", loaded->location == LOCATION_TOWN2 && !loaded->portal_active);
    test_restore_cached_floor(loaded);
    ASSERT("the archived Elderwood encounter retains its physical entry and wounds", loaded->location == LOCATION_ELDERWOOD && loaded->elderwood_entry_town == LOCATION_TOWN && loaded->enemies[0].hp == 11 && loaded->player.x == portal_x && loaded->player.y == portal_y);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    game_enter_catacombs(g);
    game_open_town_portal(g);
    ASSERT("Catacombs portals use the last visited town too", g->location == LOCATION_DAWNMERE && g->return_portal_town == LOCATION_DAWNMERE);
    test_restore_cached_floor(g);
    ASSERT("the Catacombs portal preserves forward progression and its road exit", g->location == LOCATION_CATACOMBS && g->level == 1 && g->catacombs_entry_town == LOCATION_CASTLE);
    const Location towns[] = {LOCATION_TOWN, LOCATION_TOWN2, LOCATION_TOWN3, LOCATION_TOWN4, LOCATION_DAWNMERE};
    for (int town = 0; town < 5; town++) {
        game_init(g);
        game_enter_town_gate(g, towns[town], 20, 0);
        // The physical dungeon entrance remains Oakhaven even when another town was visited last.
        approach_enter_region(g, 1);
        game_enter_dungeon(g);
        int x = g->player.x;
        int y = g->player.y;
        game_open_town_portal(g);
        ASSERT("Return to Town supports every town independently of the region entered", g->location == towns[town] && g->last_town_visited == towns[town] && g->return_portal_town == towns[town]);
        test_restore_cached_floor(g);
        ASSERT("all five town portals restore the dungeon and preserve its physical entry town", g->location == LOCATION_DUNGEON && g->level == 1 && g->player.x == x && g->player.y == y && g->dungeon_entry_town == LOCATION_TOWN);
    }
    // Existing Rosemoor catacomb portals retain their original anchor and floor.
    game_init(g);
    game_enter_town_gate(g, LOCATION_TOWN3, 0, 12);
    game_enter_catacombs(g);
    game_descend(g);
    game_open_town_portal(g);
    g->map.tiles[12][21] = TILE_TOWN_PATH;
    g->map.tiles[TOWN3_KING_GATE_Y + 1][TOWN_W - 3] = TILE_PORTAL;
    ASSERT("legacy Catacombs return portals are retired in Rosemoor", save_game(g, ELDER_SLOT) && legacy(ELDER_SLOT) && load_game(loaded, ELDER_SLOT) && loaded->map.tiles[TOWN3_KING_GATE_Y + 1][TOWN_W - 3] != TILE_PORTAL);
    action_resolve_player(loaded, (Action){ACTION_MOVE, TOWN_W - 3, TOWN3_KING_GATE_Y + 1});
    ASSERT("a migrated Rosemoor return portal stays retired", loaded->location == LOCATION_TOWN3 && !loaded->portal_active);
    game_enter_town4(g);
    game_enter_town_hall(g);
    ASSERT("older town-interior saves recover their last visited town", save_game(g, ELDER_SLOT) && legacy(ELDER_SLOT) && load_game(loaded, ELDER_SLOT) && loaded->last_town_visited == LOCATION_TOWN4);
    mechanics(g);
    remove("saves/savegame_99282.json");
    free(loaded);
    free(g);
}
