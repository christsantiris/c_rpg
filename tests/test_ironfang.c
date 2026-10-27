#include "expedition_fixture.h"
#include "test_utils.h"
#include "game/ironfang.h"
#include "game/approach.h"
#include "game/town_road.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define IRON_SLOT 99299

static int reach(const Map *map, unsigned char seen[IRON_H][IRON_W], int tools) {
    memset(seen, 0, IRON_W * IRON_H);
    int queue[IRON_W * IRON_H];
    int head = 0;
    int tail = 1;
    queue[0] = (IRON_H - 1) * IRON_W + 9;
    seen[IRON_H - 1][9] = 1;
    const int dx[] = {1,0,-1,0};
    const int dy[] = {0,1,0,-1};
    while (head < tail) {
        int cell = queue[head++];
        for (int n = 0; n < 4; n++) {
            int x = cell % IRON_W + dx[n];
            int y = cell / IRON_W + dy[n];
            if (x < 0 || x >= IRON_W || y < 0 || y >= IRON_H || seen[y][x]) {
                continue;
            }
            TileType tile = map->tiles[y][x];
            if (map_is_walkable(map, x, y) || (tools && (tile == TILE_IRON_TRESTLE || tile == TILE_IRON_BROKEN || tile == TILE_IRON_CRACK))) {
                seen[y][x] = 1;
                queue[tail++] = y * IRON_W + x;
            }
        }
    }
    return seen[0][54];
}

static int usable(const GameState *g) {
    unsigned char foot[IRON_H][IRON_W];
    unsigned char tools[IRON_H][IRON_W];
    if (!reach(&g->map, foot, 0) || !reach(&g->map, tools, 1)) {
        return 0;
    }
    for (int n = 0; n < g->enemy_count; n++) {
        const Enemy *enemy = &g->enemies[n];
        if (!foot[enemy->y][enemy->x] || !enemy->active) {
            return 0;
        }
        for (int e = n + 1; e < g->enemy_count; e++) {
            if (g->enemies[e].x == enemy->x && g->enemies[e].y == enemy->y) {
                return 0;
            }
        }
    }
    const int dx[] = {1,0,-1,0};
    const int dy[] = {0,1,0,-1};
    for (int y = 1; y < IRON_H - 1; y++) {
        for (int x = 1; x < IRON_W - 1; x++) {
            TileType tile = g->map.tiles[y][x];
            if (map_is_walkable(&g->map, x, y) && !tools[y][x]) {
                return 0;
            }
            if (tile == TILE_IRON_CACHE || tile == TILE_IRON_LAMP || tile == TILE_IRON_LAMP_LIT || tile == TILE_IRON_WINCH || tile == TILE_IRON_SWITCH) {
                int adjacent = 0;
                for (int n = 0; n < 4; n++) {
                    adjacent |= tools[y + dy[n]][x + dx[n]];
                }
                if (!adjacent) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

static void move(GameState *g, int dx, int dy) {
    action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
}

static void legacy_save(int version) {
    char path[80];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", IRON_SLOT);
    FILE *file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = calloc(size + 1, 1);
    fread(text, 1, size, file);
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    free(text);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    test_mark_legacy_portal(root);
    cJSON_DeleteItemFromObject(root, "ironfang");
    cJSON_DeleteItemFromObject(root, "ironfang_defeated");
    text = cJSON_PrintUnformatted(root);
    file = fopen(path, "wb");
    fwrite(text, 1, strlen(text), file);
    fclose(file);
    free(text);
    cJSON_Delete(root);
}

void test_ironfang(void) {
    printf("Ironfang Mines\n");
    ASSERT("Ironfang fixture slot is unused", !save_exists(IRON_SLOT));
    if (save_exists(IRON_SLOT)) {
        return;
    }
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("Ironfang fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    for (int seed = 0; seed < 32; seed++) {
        game_init(g);
        srand(seed);
        ironfang_enter(g, 0);
        int good = 1;
        for (int level = 1; level <= IRON_DEPTH; level++) {
            if (level > 1) {
                ironfang_change_level(g, 1);
            }
            good &= g->level == level && g->enemy_count >= 8 && usable(g) && ironfang_valid(g);
            if (level == IRON_DEPTH) {
                good &= g->enemies[0].type == ENEMY_GOBLIN_MINE_BOSS && g->enemies[0].is_boss;
            }
        }
        ASSERT("five stages have connected exits, usable side workings, reachable enemies, and a reserved boss", good);
    }
    game_init(g);
    TownApproach route;
    ASSERT("Ridgeshire east connects to Ironfang without moving any surviving region", town_road_connection(LOCATION_TOWN4, TOWN_SIDE_EAST, &route) && route.area == LOCATION_IRONFANG && LOCATION_STORMWATCH == 49 && ENEMY_TEMPEST_MATRIARCH == 118);
    approach_enter_region(g, 19);
    action_resolve_player(g, (Action){ACTION_MOVE, g->map.stairs_down_x, g->map.stairs_down_y});
    ASSERT("walking into the mine landmark starts stage one with a clear boundary entrance", g->location == LOCATION_IRONFANG && g->level == 1 && g->map.tiles[IRON_H - 1][9] == TILE_IRON_ENTRANCE && g->player.y == IRON_H - 2);
    g->enemy_count = 0;
    g->player.x = 31;
    g->player.y = 25;
    ASSERT("the final warning-free lamp interaction is exposed to bound A", game_has_regional_interaction(g) && ironfang_interact(g) && g->map.tiles[24][31] == TILE_IRON_LAMP_LIT && map_is_explored(&g->map, 36, 24));
    g->player.x = 39;
    g->player.y = 25;
    ASSERT("a broken rail blocks movement until repaired", !map_is_walkable(&g->map, 40, 25) && ironfang_interact(g) && g->map.tiles[25][40] == TILE_IRON_RAIL);
    g->player.x = 9;
    g->player.y = 25;
    ASSERT("boarding moves the player onto the actual cart", ironfang_interact(g) && g->ironfang[0].aboard && g->player.x == 10);
    for (int n = 0; n < 20 && g->player.x < 30; n++) {
        move(g, 1, 0);
    }
    ASSERT("the cart follows continuous track to its junction", g->player.x == 30 && g->player.y == 25 && g->ironfang[0].cart_x == 30);
    move(g, 0, -1);
    ASSERT("the default east setting physically blocks the north turnout", g->player.x == 30 && g->player.y == 25);
    ironfang_interact(g);
    ASSERT("the cart can disembark by the junction control", !g->ironfang[0].aboard && g->player.x == 30 && g->player.y == 24);
    ironfang_interact(g);
    ASSERT("operating the switch changes the usable branch", g->ironfang[0].route == 1);
    g->player.x = 30;
    g->player.y = 26;
    ironfang_interact(g);
    move(g, 0, -1);
    ASSERT("the selected north branch carries the cart into the gallery", g->ironfang[0].aboard && g->player.y == 23);
    ASSERT("a parked cart and its passenger survive save/load", save_game(g, IRON_SLOT) && load_game(loaded, IRON_SLOT) && loaded->ironfang[0].aboard && loaded->player.y == 23 && loaded->ironfang[0].cart_y == 23 && loaded->map.tiles[25][40] == TILE_IRON_RAIL);
    *g = *loaded;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 34, .y = 25, .underlying_tile = TILE_IRON_RAIL, .item = item_make_health_potion()};
    ASSERT("floor loot preserves the rail underlay through a current-stage save", save_game(g, IRON_SLOT) && load_game(loaded, IRON_SLOT) && loaded->floor_item_count == 1 && loaded->floor_items[0].active && loaded->floor_items[0].underlying_tile == TILE_IRON_RAIL && loaded->map.tiles[25][34] == TILE_IRON_RAIL);
    g->ironfang[0].route = 0;
    g->ironfang[0].cart_x = g->player.x = 54;
    g->ironfang[0].cart_y = g->player.y = 25;
    for (int n = 0; n < 6; n++) {
        move(g, 1, 0);
    }
    ASSERT("the salvage spur requires cart travel over water", g->player.x == 60 && g->ironfang[0].cart_x == 60 && !map_is_walkable(&g->map, 57, 25));
    ironfang_interact(g);
    int gold = g->gold;
    ironfang_interact(g);
    action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("disembarking exposes a real optional treasure chamber", !g->ironfang[0].aboard && g->map.tiles[23][60] == TILE_IRON_EMPTY && g->gold > gold);
    g->player.x = 9;
    g->player.y = 8;
    ASSERT("excavation opens an optional sealed pocket", !map_is_walkable(&g->map, 8, 8) && ironfang_interact(g) && map_is_walkable(&g->map, 8, 8));
    g->last_town_visited = LOCATION_TOWN2;
    g->player.x = 18;
    g->player.y = 25;
    g->gold = 431;
    game_open_town_portal(g);
    ASSERT("Return to Town respects the last town visited and preserves the cart", g->location == LOCATION_TOWN2 && g->portal_location == LOCATION_IRONFANG && !g->portal_active && g->ironfang[0].cart_x == 60);
    test_restore_cached_floor(g);
    ASSERT("the archived floor fixture restores the same expedition, loot, lamp, and position", g->location == LOCATION_IRONFANG && g->player.x == 18 && !g->portal_active && g->gold == 431 && g->map.tiles[23][60] == TILE_IRON_EMPTY && g->map.tiles[24][31] == TILE_IRON_LAMP_LIT);
    ironfang_change_level(g, 1);
    g->enemies[0].hp = 3;
    ironfang_change_level(g, 0);
    ironfang_change_level(g, 1);
    ASSERT("backtracking retains wounded enemies instead of respawning them", g->enemies[0].hp == 3);
    ironfang_leave(g);
    ironfang_enter(g, 0);
    ironfang_change_level(g, 1);
    ASSERT("a new expedition restores enemies and recovers a cart abandoned across water without losing repairs", g->enemies[0].hp == g->enemies[0].max_hp && g->enemies[0].hp > 3 && g->ironfang[0].cart_x == 10 && g->ironfang[0].tiles[24][31] == TILE_IRON_LAMP_LIT && g->ironfang[0].tiles[25][40] == TILE_IRON_RAIL);
    while (g->level < IRON_DEPTH) {
        ironfang_change_level(g, 1);
    }
    Enemy *boss = &g->enemies[0];
    g->enemy_count = 1;
    g->player.x = 46;
    g->player.y = 13;
    int damage = 0;
    ironfang_tick(g, boss, &damage);
    int tx = boss->attack_target_x;
    int ty = boss->attack_target_y;
    ASSERT("the boss locks a two-action rockfall footprint", boss->boss_attack == IRON_ROCKFALL && boss->move_timer == 2 && ironfang_attack_marks(g, boss, tx, ty));
    g->player.x -= 2;
    ironfang_tick(g, boss, &damage);
    ironfang_tick(g, boss, &damage);
    ASSERT("ordinary movement escapes the declared rockfall and exposes a recovery opening", !damage && boss->move_timer < 0 && boss->defense == 2);
    *loaded = *g;
    loaded->enemies[0].hp = loaded->enemies[0].max_hp / 2;
    loaded->enemies[0].move_timer = 0;
    for (int n = 0; n < 10; n++) {
        ironfang_tick(loaded, &loaded->enemies[0], &damage);
    }
    ASSERT("half-health reinforcements are one finite pair using existing Hobgoblin sprites", loaded->enemy_count == 3 && loaded->enemies[0].boss_wave_used && loaded->enemies[1].type == ENEMY_HOBGOBLIN_GUARD && loaded->enemies[2].type == ENEMY_HOBGOBLIN_GUARD && loaded->enemies[1].experience == 0);
    g->player.x = 41;
    g->player.y = 16;
    int before = boss->hp;
    ASSERT("the ore winch exposes its real interaction", ironfang_interact(g) && g->ironfang[4].ore_fuse == 3);
    action_resolve_enemies(g);
    ASSERT("releasing ore leaves two full response actions", g->ironfang[4].ore_fuse == 2 && boss->hp == before && ironfang_ore_marks(g, 51, 11));
    ASSERT("a pending ore-cart warning survives saving on the boss floor", save_game(g, IRON_SLOT) && load_game(loaded, IRON_SLOT) && loaded->ironfang[4].ore_fuse == 2 && loaded->enemies[0].hp == before);
    action_resolve_enemies(loaded);
    action_resolve_enemies(loaded);
    ASSERT("the ore cart deals meaningful damage and interrupts the boss", loaded->enemies[0].hp < before && loaded->enemies[0].move_timer < 0 && !loaded->enemies[0].attack_phase);
    g->player.x = 54;
    g->player.y = 0;
    ironfang_travel(g, 54, 0);
    ASSERT("the optional Mine Boss never locks final travel or grants credit for leaving", g->location == LOCATION_APPROACH && !g->ironfang_defeated);
    *g = *loaded;
    boss = &g->enemies[0];
    g->player.x = boss->x - 1;
    g->player.y = boss->y;
    g->player.attack = 1000;
    action_resolve_player(g, (Action){ACTION_MOVE, boss->x, boss->y});
    BossJournalEntry journal;
    quest_journal_get_boss(g, 19, &journal);
    ASSERT("killing the boss grants the victory and records its approved name", g->ironfang_defeated && !boss->active && journal.defeated && strcmp(journal.name, "Goblin Mine Boss") == 0);
    g->player.x = 54;
    g->player.y = 0;
    ironfang_travel(g, 54, 0);
    ASSERT("victory returns onto the physical Ridgeshire approach", g->location == LOCATION_APPROACH && g->approach_route == 19);
    g->location = LOCATION_TOWN4;
    map_generate_town4(&g->map, &g->player.x, &g->player.y);
    g->map.tiles[12][TOWN_W - 1] = TILE_WALL;
    g->gold = 623;
    ASSERT("the legacy migration fixture saves", save_game(g, IRON_SLOT));
    legacy_save(199);
    ASSERT("version 199 saves gain the eastern gate without losing character progress", load_game(loaded, IRON_SLOT) && loaded->location == LOCATION_TOWN4 && loaded->gold == 623 && loaded->map.tiles[12][TOWN_W - 1] == TILE_TOWN_EXIT && !loaded->ironfang[0].valid);
    town_road_enter(g, LOCATION_TOWN4, TOWN_SIDE_WEST, 0);
    int road_y = TOWN_ROAD_H / 2;
    int road_x = TOWN_SPRITE_X + TOWN_SPRITE_W + 3;
    for (int x = TOWN_SPRITE_X + TOWN_SPRITE_W; x < TOWN_ROAD_W; x++) {
        g->map.tiles[road_y][x] = TILE_WALL;
    }
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = road_x, .y = road_y, .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
    g->map.tiles[road_y][road_x] = TILE_ITEM;
    ASSERT("the older exterior-road fixture saves", save_game(g, IRON_SLOT));
    legacy_save(196);
    ASSERT("older geography migrations cannot reclose the new road or destroy its floor loot", load_game(loaded, IRON_SLOT) && loaded->location == LOCATION_TOWN_ROAD && loaded->map.tiles[road_y][TOWN_ROAD_W - 1] == TILE_TOWN_ROAD_EXIT_EAST && loaded->floor_items[0].active && loaded->floor_items[0].underlying_tile == TILE_TOWN_PATH && loaded->map.tiles[road_y][road_x] == TILE_ITEM);
    loaded->ironfang_defeated = 2;
    ASSERT("invalid mine state is rejected before writing a save", !save_game(loaded, IRON_SLOT));
    char path[80];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", IRON_SLOT);
    remove(path);
    free(g);
    free(loaded);
}
