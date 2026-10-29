#include "expedition_fixture.h"
#include "test_utils.h"
#include "game/stonewake.h"
#include "game/jail.h"
#include "game/regional_terrain.h"
#include "systems/save_load.h"
#include "screens/quest_journal.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define QUARRY_SLOT 99304
#define QUARRY_SAVE "saves/savegame_99304.json"

static int find(const Map *m, TileType tile, int *x, int *y) {
    for (int ty = 1; ty < SWAMP_MAP_H - 1; ty++) {
        for (int tx = 1; tx < SWAMP_MAP_W - 1; tx++) {
            if (m->tiles[ty][tx] == tile) {
                *x = tx;
                *y = ty;
                return 1;
            }
        }
    }
    return 0;
}

static void flood(const Map *m, unsigned char seen[MAP_H][MAP_W]) {
    int queue[MAP_H * MAP_W];
    int head = 0;
    int tail = 0;
    memset(seen, 0, MAP_H * MAP_W);
    queue[tail++] = m->stairs_up_y * MAP_W + m->stairs_up_x;
    seen[m->stairs_up_y][m->stairs_up_x] = 1;
    const int dx[4] = {0,1,0,-1};
    const int dy[4] = {-1,0,1,0};
    while (head < tail) {
        int cell = queue[head++];
        for (int n = 0; n < 4; n++) {
            int x = cell % MAP_W + dx[n];
            int y = cell / MAP_W + dy[n];
            if (map_is_walkable(m, x, y) && !seen[y][x]) {
                seen[y][x] = 1;
                queue[tail++] = y * MAP_W + x;
            }
        }
    }
}

static int connected(const Map *m) {
    unsigned char seen[MAP_H][MAP_W];
    flood(m, seen);
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (map_is_walkable(m, x, y) && m->tiles[y][x] != TILE_STONEWAKE_LEDGE && !seen[y][x]) {
                return 0;
            }
        }
    }
    return seen[m->stairs_down_y][m->stairs_down_x];
}

static void test_layouts(GameState *g) {
    int valid = 1;
    int population = 1;
    int salvage = 1;
    int sentinel = 0;
    game_init(g);
    for (int seed = 0; seed < 100 && valid && salvage; seed++) {
        for (int stage = 1; stage <= 5; stage++) {
            srand((unsigned int)seed);
            stonewake_generate(&g->map, stage);
            g->location = LOCATION_GLASSDEEP;
            g->level = stage;
            g->enemy_count = 0;
            g->floor_item_count = 0;
            int controls[2][2];
            int count = 0;
            for (int y = 1; y < SWAMP_MAP_H; y++) {
                for (int x = 1; x < SWAMP_MAP_W; x++) {
                    if (g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_NEW && count < 2) {
                        controls[count][0] = x;
                        controls[count++][1] = y;
                    }
                }
            }
            valid &= count == 2 && stonewake_map(&g->map);
            // All nine pairs of raised/crossing/salvage settings keep the adventure route open.
            for (int a = 0; a < 3 && count == 2; a++) {
                for (int b = 0; b < 3; b++) {
                    valid &= connected(&g->map);
                    unsigned char seen[MAP_H][MAP_W];
                    flood(&g->map, seen);
                    for (int n = 0; n < 2; n++) {
                        int x = controls[n][0];
                        int y = controls[n][1];
                        int mode = stonewake_crane_mode(g->map.tiles[y][x]);
                        salvage &= seen[y - 2][x + 5] == (mode == 2);
                    }
                    g->player.x = controls[1][0];
                    g->player.y = controls[1][1];
                    valid &= stonewake_interact(g);
                }
                g->player.x = controls[0][0];
                g->player.y = controls[0][1];
                valid &= stonewake_interact(g);
            }
            g->player.x = g->map.stairs_up_x;
            g->player.y = g->map.stairs_up_y;
            enemies_spawn(g);
            unsigned char seen[MAP_H][MAP_W];
            flood(&g->map, seen);
            for (int i = 0; i < g->enemy_count; i++) {
                Enemy *e = &g->enemies[i];
                population &= seen[e->y][e->x] && e->hp > 0 && e->name[0];
                if (e->type == ENEMY_STONE_GIANT) {
                    population &= stage >= 4 && strcmp(e->name, "Stone Giant") == 0;
                    sentinel++;
                }
            }
        }
    }
    ASSERT("500 layouts keep the main route connected in all nine crane configurations", valid);
    ASSERT("salvage platforms are accessible only with the salvage spur lowered across all 500 layouts", salvage);
    ASSERT("ordinary enemies spawn on reachable main ground; Stone Giants start on stage four", population && sentinel > 0);
}

static void test_cranes(GameState *g, GameState *loaded) {
    game_init(g);
    game_enter_glassdeep(g);
    g->enemy_count = 0;
    g->floor_item_count = 0;
    int x = -1;
    int y = -1;
    ASSERT("a new quarry crane can be found", find(&g->map, TILE_STONEWAKE_CRANE_NEW, &x, &y));
    if (x < 0) {
        return;
    }
    g->player.x = x;
    g->player.y = y;
    ASSERT("raised gangways block walking across the cut", !map_is_walkable(&g->map, x + 1, y));
    ASSERT("lowering the crossing creates a real through route while its salvage spur stays raised", stonewake_interact(g) && g->map.tiles[y][x + 8] == TILE_STONEWAKE_BRIDGE && !map_is_walkable(&g->map, x + 5, y - 1));
    g->enemy_count = 1;
    g->enemies[0] = (Enemy){.active = 1, .type = ENEMY_OGRE, .x = x + 7, .y = y, .hp = 7, .max_hp = 30};
    stonewake_interact(g);
    ASSERT("the east crossing cannot retract under an enemy", g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_DOWN && g->enemies[0].hp == 7);
    g->enemies[0].active = 0;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = x + 7, .y = y, .underlying_tile = TILE_STONEWAKE_BRIDGE, .item = item_make_health_potion()};
    g->map.tiles[y][x + 7] = TILE_ITEM;
    stonewake_interact(g);
    ASSERT("changing routes cannot discard loot on a retracting section", g->floor_items[0].active && g->map.tiles[y][x + 7] == TILE_ITEM && g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_DOWN);
    g->floor_items[0].active = 0;
    g->map.tiles[y][x + 7] = TILE_STONEWAKE_BRIDGE;
    g->portal_active = 1;
    g->portal_location = LOCATION_GLASSDEEP;
    g->portal_level = 1;
    g->portal_x = x + 7;
    g->portal_y = y;
    g->portal_origin_tile = TILE_STONEWAKE_BRIDGE;
    stonewake_interact(g);
    ASSERT("changing routes cannot close a return portal's crossing", g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_DOWN);
    g->portal_active = 0;
    g->floor_item_count = 0;
    g->player.x = x + 10;
    stonewake_interact(g);
    ASSERT("the far-bank lever deploys salvage access while lifting the east crossing", g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_SALVAGE && !map_is_walkable(&g->map, x + 8, y) && map_is_walkable(&g->map, x + 5, y - 1) && connected(&g->map));
    g->player.x = x + 1;
    stonewake_interact(g);
    ASSERT("a player cannot lift the salvage gangway underneath themselves", g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_SALVAGE);
    g->player.x = x;
    g->portal_active = 1;
    g->portal_x = x + 4;
    g->portal_y = y - 2;
    g->portal_origin_tile = TILE_STONEWAKE_LEDGE;
    stonewake_interact(g);
    ASSERT("a return portal on the island holds its access open", g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_SALVAGE);
    g->portal_active = 0;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = x + 4, .y = y - 2, .underlying_tile = TILE_STONEWAKE_LEDGE, .item = item_make_health_potion()};
    stonewake_interact(g);
    ASSERT("loot on the island holds its access open", g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_SALVAGE);
    g->floor_item_count = 0;
    stonewake_interact(g);
    ASSERT("the unoccupied gangways can be fully raised", g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_UP && !map_is_walkable(&g->map, x + 4, y) && !map_is_walkable(&g->map, x + 5, y - 1));
    g->portal_active = 1;
    g->portal_x = x;
    g->portal_y = y;
    g->portal_origin_tile = TILE_STONEWAKE_CRANE_UP;
    stonewake_interact(g);
    ASSERT("controls at a hidden return-portal anchor keep its ground state synchronized", g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_DOWN && g->portal_origin_tile == TILE_STONEWAKE_CRANE_DOWN);
    g->map.tiles[y][x] = TILE_PORTAL;
    stonewake_interact(g);
    ASSERT("controls beneath a visible return portal preserve the portal while updating its ground", g->map.tiles[y][x] == TILE_PORTAL && g->portal_origin_tile == TILE_STONEWAKE_CRANE_SALVAGE);
    stonewake_interact(g);
    g->portal_active = 0;
    g->map.tiles[y][x] = TILE_STONEWAKE_CRANE_UP;
    g->floor_item_count = 2;
    for (int n = 0; n < 2; n++) {
        g->floor_items[n] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_STONEWAKE_CRANE_UP, .item = item_make_health_potion()};
    }
    g->map.tiles[y][x] = TILE_ITEM;
    stonewake_interact(g);
    ASSERT("controls beneath overlapping loot preserve both items and update their underlays", g->map.tiles[y][x] == TILE_ITEM && g->floor_items[0].underlying_tile == TILE_STONEWAKE_CRANE_DOWN && g->floor_items[1].underlying_tile == TILE_STONEWAKE_CRANE_DOWN);
    stonewake_interact(g);
    ASSERT("salvage alignment, wounds and loot round trip through saving", save_game(g, QUARRY_SLOT) && load_game(loaded, QUARRY_SLOT) && loaded->map.tiles[y - 1][x + 5] == TILE_STONEWAKE_BRIDGE && loaded->map.tiles[y][x + 8] == TILE_STONEWAKE_GAP && loaded->floor_items[1].underlying_tile == TILE_STONEWAKE_CRANE_SALVAGE && loaded->enemies[0].hp == 7);
    g->floor_item_count = 0;
    g->map.tiles[y][x] = TILE_STONEWAKE_CRANE_SALVAGE;
    g->player.x = x + 5;
    g->player.y = y - 2;
    int gold = g->gold;
    stonewake_interact(g);
    stonewake_interact(g);
    action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("the isolated supply store pays once after reaching it by the salvage spur", g->gold == gold + 25 && g->map.tiles[y - 3][x + 5] == TILE_STONEWAKE_EMPTY);
}

static void test_debug_crane_actions(GameState *g, GameState *loaded) {
    game_init(g);
    g->debug_passages_unlocked = 1;
    game_enter_glassdeep(g);
    g->enemy_count = 0;
    int x = -1;
    int y = -1;
    find(&g->map, TILE_STONEWAKE_CRANE_NEW, &x, &y);
    if (x < 0) {
        find(&g->map, TILE_STONEWAKE_CRANE_DOWN, &x, &y);
    }
    ASSERT("debug action fixture has a crane", x >= 0);
    if (x < 0) {
        return;
    }
    g->player.x = x;
    g->player.y = y;
    game_refresh_debug_passages(g);
    ASSERT("debug unlock opens a new crossing once", g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_DOWN);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("the real interaction action changes debug cranes to the salvage route", g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_SALVAGE);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_player(g, (Action){ACTION_MOVE, x + 1, y});
    ASSERT("a following movement action respects a player-raised bridge even with debug unlock enabled", g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_UP && g->player.x == x && g->player.y == y && !map_is_walkable(&g->map, x + 1, y));
    ASSERT("a player-raised debug bridge stays raised after save/load", save_game(g, QUARRY_SLOT) && load_game(loaded, QUARRY_SLOT) && loaded->debug_passages_unlocked && loaded->map.tiles[y][x] == TILE_STONEWAKE_CRANE_UP && !map_is_walkable(&loaded->map, x + 1, y));
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_player(g, (Action){ACTION_MOVE, x + 1, y});
    ASSERT("the real movement action crosses the same tile after lowering the gangway", g->player.x == x + 1 && g->map.tiles[y][x + 1] == TILE_STONEWAKE_BRIDGE);
}

static void test_salvage_replay(GameState *g) {
    game_init(g);
    game_enter_glassdeep(g);
    int x = -1;
    int y = -1;
    find(&g->map, TILE_STONEWAKE_CRANE_NEW, &x, &y);
    ASSERT("salvage replay fixture has a crane", x >= 0);
    if (x < 0) {
        return;
    }
    g->enemy_count = 1;
    g->enemies[0] = (Enemy){.active = 0, .type = ENEMY_OGRE, .x = x + 5, .y = y - 3, .hp = 0, .max_hp = 30, .experience = 40};
    snprintf(g->enemies[0].name, sizeof(g->enemies[0].name), "Ogre");
    g->map.tiles[y - 3][x + 5] = TILE_STONEWAKE_EMPTY;
    g->map.tiles[y][x] = TILE_STONEWAKE_CRANE_UP;
    game_return_to_town(g);
    game_enter_glassdeep(g);
    unsigned char seen[MAP_H][MAP_W];
    flood(&g->map, seen);
    int reachable = 1;
    int living = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (e->active) {
            living++;
            reachable &= seen[e->y][e->x];
        }
    }
    ASSERT("replay cannot respawn a dead enemy on an isolated emptied salvage store", living > 0 && reachable && g->map.tiles[y - 3][x + 5] == TILE_STONEWAKE_EMPTY);
}

static void test_hammer(GameState *g, GameState *loaded) {
    for (int player_class = 0; player_class <= 2; player_class++) {
        game_init(g);
        g->player.player_class = player_class;
        g->inventory_count = 2;
        g->inventory[0] = item_make_stone_warhammer();
        g->inventory[1] = item_make_stone_warhammer();
        g->equipped_main_hand = -1;
        int hp = g->player.hp;
        int max_hp = g->player.max_hp;
        int attack = g->player.attack;
        ASSERT("every class can equip the two-handed stone warhammer without receiving free healing", game_equip_main_hand(g, 0) && g->player.max_hp == max_hp + 30 && g->player.hp == hp && g->player.attack == attack + 8 && !game_equip_off_hand(g, 1));
        g->player.hp = g->player.max_hp;
        game_equip_main_hand(g, 0);
        game_equip_main_hand(g, 0);
        ASSERT("reselecting or swapping equivalent hammers neither removes health nor stacks their maximum HP", g->player.hp == max_hp + 30 && g->player.max_hp == max_hp + 30 && g->player.attack == attack + 8);
        ASSERT("an equipped hammer and its HP bonus survive save/load without applying the bonus twice", save_game(g, QUARRY_SLOT) && load_game(loaded, QUARRY_SLOT) && loaded->player.max_hp == max_hp + 30 && loaded->inventory[EQUIPMENT_STRONG_HAND].weapon_family == WEAPON_FAMILY_HAMMER && loaded->inventory[EQUIPMENT_STRONG_HAND].max_hp_bonus == 30);
        game_unequip_main_hand(g);
        ASSERT("unequipping clamps current HP and removes the temporary bonus exactly once", g->player.hp == max_hp && g->player.max_hp == max_hp && g->player.attack == attack);
        g->player.hp = 20;
        for (int n = 0; n < 5; n++) {
            game_equip_main_hand(g, 0);
            game_unequip_main_hand(g);
        }
        ASSERT("repeated weapon swaps cannot heal a wounded player", g->player.hp == 20 && g->player.max_hp == max_hp);
    }
    game_equip_main_hand(g, 0);
    int base_max_hp = g->player.max_hp - 30;
    game_enter_town_gate(g, LOCATION_CASTLE, INFORMANT_X, INFORMANT_Y + 1);
    jail_talk_nearby(g);
    ASSERT("Royal Guard confiscation removes the hammer's HP bonus and retains both hammers as evidence", g->location == LOCATION_JAIL && g->player.max_hp == base_max_hp && g->jail_gear_count == 2 && g->jail_gear[0].max_hp_bonus == 30);
    game_init(g);
    game_enter_town3(g);
    game_enter_guild(g);
    g->player.x = GUILD_ORIN_X;
    g->player.y = GUILD_ORIN_Y + 1;
    g->glassdeep_quest_state = 2;
    g->glassdeep_quest_progress = 7;
    g->glassdeep_quest_encounters = 7;
    for (int n = 0; n < MAX_INVENTORY; n++) {
        g->inventory[n] = item_make_health_potion();
    }
    g->inventory_count = MAX_INVENTORY;
    int gold = g->gold;
    game_talk_to_orin(g);
    game_talk_to_orin(g);
    ASSERT("a full pack reserves the hammer with Orin and awards quest currency only once", g->stonewake_warhammer_state == 1 && g->glassdeep_quest_state == 3 && g->gold == gold + 120 && strstr(g->dialogue_text, "Make room"));
    ASSERT("the reserved equipment reward survives saving in the Guild", save_game(g, QUARRY_SLOT) && load_game(loaded, QUARRY_SLOT) && loaded->stonewake_warhammer_state == 1);
    g->inventory_count--;
    game_talk_to_orin(g);
    game_talk_to_orin(g);
    ASSERT("making one slot claims exactly one warhammer on a later conversation", g->stonewake_warhammer_state == 2 && g->inventory_count == MAX_INVENTORY && g->inventory[MAX_INVENTORY - 1].weapon_family == WEAPON_FAMILY_HAMMER && g->gold == gold + 120);
}

static void test_colossus(GameState *g, GameState *loaded) {
    game_init(g);
    g->location = LOCATION_GLASSDEEP;
    g->level = 5;
    stonewake_generate(&g->map, 5);
    enemies_spawn(g);
    g->enemy_count = 1;
    Enemy *e = &g->enemies[0];
    g->player.x = e->x + 3;
    g->player.y = e->y;
    int hp = e->hp;
    ASSERT("the Colossus declares a fixed two-action hammerfall before causing damage", stonewake_colossus_tick(g, e) == 0 && e->move_timer == 2 && stonewake_attack_marks(g, e, g->player.x, g->player.y));
    int tx = e->attack_target_x;
    g->player.x -= 2;
    ASSERT("warnings stay on their declared squares when the player moves", e->attack_target_x == tx && !stonewake_attack_marks(g, e, g->player.x, g->player.y));
    ASSERT("the first response action does not resolve the hammerfall", !stonewake_colossus_tick(g, e) && e->move_timer == 1);
    ASSERT("leaving the warning avoids damage and opens a two-action recovery window", !stonewake_colossus_tick(g, e) && e->move_timer == -2 && e->hp == hp);
    e->move_timer = 0;
    e->hp = e->max_hp / 2;
    e->boss_attack = 1;
    stonewake_colossus_tick(g, e);
    ASSERT("the fractured phase adds a three-action shockwave with a fixed diamond footprint", e->attack_phase && e->boss_attack == 2 && e->move_timer == 3 && stonewake_attack_marks(g, e, e->attack_target_x, e->attack_target_y));
    ASSERT("a saved Colossus warning preserves target, timing and wounds", save_game(g, QUARRY_SLOT) && load_game(loaded, QUARRY_SLOT) && loaded->enemies[0].hp == e->hp && loaded->enemies[0].move_timer == 3 && loaded->enemies[0].attack_target_x == e->attack_target_x);
    int x = 39;
    int y = 51;
    g->player.x = x;
    g->player.y = y;
    e->boss_aux_x = 0;
    ASSERT("the arena crane can interrupt a windup without damaging or deleting the boss", stonewake_interact(g) && e->move_timer == -2 && e->boss_aux_x == 5 && e->hp == e->max_hp / 2);
    e->move_timer = 2;
    stonewake_interact(g);
    ASSERT("the interrupt cooldown prevents repeated crane toggles from locking the boss indefinitely", e->move_timer == 2 && e->boss_aux_x == 5);
    g->player.x = e->x + 3;
    g->player.y = e->y;
    e->attack_target_x = g->player.x;
    e->attack_target_y = g->player.y;
    e->boss_aux_y = 0;
    e->boss_attack = 1;
    e->move_timer = 1;
    e->frozen_turns = 2;
    int player_hp = g->player.hp;
    action_resolve_enemies(g);
    ASSERT("freezing the Colossus pauses its declared attack and causes no damage", e->move_timer == 1 && e->frozen_turns == 1 && g->player.hp == player_hp);
    e->frozen_turns = 0;
    int damage = stonewake_colossus_tick(g, e);
    ASSERT("remaining on the marked tile resolves real damage, followed by recovery", damage > 0 && e->move_timer == -2);
    // A completely surrounded player must not receive an unavoidable area warning.
    e->move_timer = 0;
    e->attack_phase = 0;
    e->hp = e->max_hp;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx || dy) {
                g->map.tiles[g->player.y + dy][g->player.x + dx] = TILE_GLASSDEEP_WALL;
            }
        }
    }
    stonewake_colossus_tick(g, e);
    ASSERT("the boss withholds attacks when there is no legal escape from the warning", e->move_timer < 0);

}

static cJSON *read_quarry_save(void) {
    FILE *file = fopen(QUARRY_SAVE, "rb");
    if (!file) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc((size_t)size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    size_t length = fread(buffer, 1, (size_t)size, file);
    buffer[length] = 0;
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    return root;
}

static int write_quarry_save(cJSON *root) {
    char *text = cJSON_Print(root);
    FILE *file = fopen(QUARRY_SAVE, "wb");
    if (!text || !file) {
        free(text);
        if (file) {
            fclose(file);
        }
        return 0;
    }
    fputs(text, file);
    fclose(file);
    free(text);
    return 1;
}

static void test_migration(GameState *g, GameState *loaded) {
    game_init(g);
    g->location = LOCATION_GLASSDEEP;
    g->level = 5;
    stonewake_generate(&g->map, 5);
    g->map.tiles[0][0] = TILE_GLASSDEEP_WALL;
    g->player.x = 35;
    g->player.y = 52;
    g->player.hp = 41;
    g->gold = 764;
    g->glassdeep_quest_state = 3;
    g->glassdeep_quest_progress = 7;
    g->glassdeep_quest_encounters = 7;
    g->defeated_bosses |= 1 << LOCATION_GLASSDEEP;
    g->max_glassdeep_level_reached = 5;
    g->enemy_count = 2;
    g->enemies[0] = (Enemy){.active = 1, .is_boss = 1, .type = ENEMY_PRISM_SOVEREIGN, .x = 35, .y = 54, .hp = 37, .max_hp = 275, .move_timer = 2, .boss_attack = 1};
    snprintf(g->enemies[0].name, sizeof(g->enemies[0].name), "Prism Sovereign");
    g->enemies[1] = (Enemy){.active = 0, .type = ENEMY_BLIND_STALKER, .x = 34, .y = 52, .hp = 0, .max_hp = 38};
    snprintf(g->enemies[1].name, sizeof(g->enemies[1].name), "Resonator Guardian");
    map_mark_explored(&g->map, 35, 52);
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 36, .y = 52, .underlying_tile = TILE_GLASS_FACE_N, .item = item_make_health_potion()};
    g->map.tiles[52][36] = TILE_ITEM;
    g->portal_active = 1;
    g->portal_location = LOCATION_GLASSDEEP;
    g->portal_level = 5;
    g->portal_x = 35;
    g->portal_y = 51;
    g->portal_origin_tile = TILE_GLASS_FACE_E;
    g->glassdeep_cache[4].valid = 1;
    g->glassdeep_cache[4].map = g->map;
    g->glassdeep_cache[4].enemy_count = 2;
    g->glassdeep_cache[4].enemies[0] = g->enemies[0];
    g->glassdeep_cache[4].enemies[1] = g->enemies[1];
    g->dialogue_active = 1;
    snprintf(g->dialogue_speaker, sizeof(g->dialogue_speaker), "Crown Resonator");
    ASSERT("legacy quarry fixture saves before migration", save_game(g, QUARRY_SLOT));
    cJSON *root = read_quarry_save();
    ASSERT("legacy save JSON is readable", root != NULL);
    if (!root) {
        return;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 193);
    test_mark_legacy_portal(root);
    cJSON_DeleteItemFromObject(root, "stonewake_warhammer_state");
    ASSERT("a version 193 Glassdeep expedition loads as Stonewake", write_quarry_save(root) && load_game(loaded, QUARRY_SLOT) && stonewake_map(&loaded->map) && stonewake_map(&loaded->glassdeep_cache[4].map));
    ASSERT("migration preserves player health, gold, stage progress, quest completion and boss victories", loaded->player.hp == 41 && loaded->gold == 764 && loaded->level == 5 && loaded->max_glassdeep_level_reached == 5 && loaded->glassdeep_quest_state == 3 && loaded->glassdeep_quest_progress == 7 && loaded->defeated_bosses == g->defeated_bosses);
    ASSERT("migration preserves active/cached wounds and deaths and cancels retired beam warnings", loaded->enemies[0].hp == 37 && !loaded->enemies[1].active && !loaded->enemies[0].move_timer && loaded->glassdeep_cache[4].enemies[0].hp == 37 && !loaded->glassdeep_cache[4].enemies[1].active && !strcmp(loaded->enemies[0].name, "Quarry Colossus") && !strcmp(loaded->enemies[1].name, "Quarry Guardian"));
    ASSERT("migration preserves loot, exploration and portals while replacing their retired optical underlays", loaded->floor_items[0].active && loaded->map.tiles[52][36] == TILE_ITEM && loaded->floor_items[0].underlying_tile == TILE_GLASSDEEP_RUIN && !loaded->portal_active && loaded->portal_origin_tile == TILE_GLASSDEEP_RUIN && map_is_explored(&loaded->map, 35, 52) && !loaded->dialogue_active);
    ASSERT("previously completed quests reserve their new equipment reward without duplicating currency", loaded->stonewake_warhammer_state == 1 && loaded->gold == 764);
    stonewake_migrate(loaded);
    ASSERT("migration can repeat without changing rewards or wounds", loaded->stonewake_warhammer_state == 1 && loaded->enemies[0].hp == 37);
    stonewake_begin_expedition(loaded);
    ASSERT("the next fresh expedition upgrades legacy floors without resetting quests, rewards or victories", !loaded->glassdeep_cache[4].valid && loaded->glassdeep_quest_progress == 7 && loaded->stonewake_warhammer_state == 1 && loaded->defeated_bosses == g->defeated_bosses);
    game_enter_glassdeep(loaded);
    ASSERT("a fresh legacy revisit actually generates the full quarry while retaining recorded completion", loaded->level == 1 && loaded->map.tiles[0][0] == TILE_STONEWAKE_BOUNDARY && loaded->glassdeep_quest_progress == 7 && loaded->max_glassdeep_level_reached == 5);

    ASSERT("migrated progress can be resaved and reloaded as version 194", save_game(loaded, QUARRY_SLOT) && load_game(g, QUARRY_SLOT) && g->stonewake_warhammer_state == 1 && g->glassdeep_quest_progress == 7);
    cJSON_Delete(root);
    root = read_quarry_save();
    if (!root) {
        return;
    }
    cJSON_DeleteItemFromObject(root, "stonewake_warhammer_state");
    ASSERT("version 194 rejects a missing persistent reward state", write_quarry_save(root) && !load_game(g, QUARRY_SLOT));
    cJSON_AddNumberToObject(root, "stonewake_warhammer_state", 1.5);
    ASSERT("reward state validation rejects fractional values", write_quarry_save(root) && !load_game(g, QUARRY_SLOT));
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "stonewake_warhammer_state"), 3);
    ASSERT("reward state validation rejects unknown states", write_quarry_save(root) && !load_game(g, QUARRY_SLOT));
    cJSON_Delete(root);
}

static void test_crane_migration(GameState *g, GameState *loaded) {
    game_init(g);
    game_enter_glassdeep(g);
    g->enemy_count = 1;
    g->enemies[0].hp = 9;
    int x = -1;
    int y = -1;
    find(&g->map, TILE_STONEWAKE_CRANE_NEW, &x, &y);
    ASSERT("version 194 fixture has a crane", x >= 0);
    if (x < 0) {
        return;
    }
    // Recreate the old flooded cut and its two shore stores.
    for (int cy = 1; cy < SWAMP_MAP_H; cy++) {
        for (int cx = 1; cx < SWAMP_MAP_W; cx++) {
            TileType tile = g->map.tiles[cy][cx];
            if (tile == TILE_STONEWAKE_LEDGE || tile == TILE_STONEWAKE_SUPPLIES) {
                g->map.tiles[cy][cx] = TILE_GLASSDEEP_POOL;
            } else if (tile == TILE_STONEWAKE_REMOTE) {
                g->map.tiles[cy][cx] = TILE_GLASSDEEP_RUIN;
            } else if (tile == TILE_STONEWAKE_CRANE_NEW) {
                g->map.tiles[cy - 1][cx + 5] = TILE_GLASSDEEP_POOL;
                g->map.tiles[cy][cx] = TILE_STONEWAKE_CRANE_UP;
            }
        }
    }
    Room *room = &g->map.rooms[1];
    int sx = room->x + 2;
    int sy = room->y + 2;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = sx, .y = sy, .underlying_tile = TILE_STONEWAKE_EMPTY, .item = item_make_health_potion()};
    g->map.tiles[sy][sx] = TILE_ITEM;
    room = &g->map.rooms[6];
    g->map.tiles[room->y + 2][room->x + 2] = TILE_STONEWAKE_SUPPLIES;
    g->player.x = x;
    g->player.y = y;
    g->portal_active = 1;
    g->portal_level = 1;
    g->portal_location = LOCATION_GLASSDEEP;
    g->portal_x = x;
    g->portal_y = y;
    g->portal_origin_tile = TILE_STONEWAKE_CRANE_UP;
    g->map.tiles[y][x] = TILE_PORTAL;
    g->glassdeep_cache[0].valid = 1;
    g->glassdeep_cache[0].map = g->map;
    g->glassdeep_cache[0].enemy_count = g->enemy_count;
    g->glassdeep_cache[0].enemies[0] = g->enemies[0];
    int gold = g->gold;
    ASSERT("pre-rework crane fixture saves", save_game(g, QUARRY_SLOT));
    cJSON *root = read_quarry_save();
    if (!root) {
        ASSERT("version 194 fixture is readable", 0);
        return;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 194);
    test_mark_legacy_portal(root);
    ASSERT("version 194 saves gain salvage platforms in the active and cached floor", write_quarry_save(root) && load_game(loaded, QUARRY_SLOT) && stonewake_has_salvage(loaded, x, y) && loaded->glassdeep_cache[0].map.tiles[y - 2][x + 4] == TILE_STONEWAKE_LEDGE);
    ASSERT("moving the store retains its claimed state and preserves overlapping floor loot", loaded->map.tiles[y - 3][x + 5] == TILE_STONEWAKE_EMPTY && loaded->floor_items[0].active && loaded->map.tiles[sy][sx] == TILE_ITEM && loaded->floor_items[0].underlying_tile == TILE_GLASSDEEP_FLOOR);
    ASSERT("crane migration keeps portal anchors, player position, money and enemy wounds", !loaded->portal_active && loaded->portal_origin_tile == TILE_STONEWAKE_CRANE_NEW && loaded->map.tiles[y][x] == TILE_STONEWAKE_CRANE_NEW && loaded->player.x == x && loaded->player.y == y && loaded->enemies[0].hp == 9 && loaded->gold == gold);
    stonewake_migrate_cranes(loaded);
    ASSERT("repeating crane migration does not refill a claimed salvage store", loaded->map.tiles[y - 3][x + 5] == TILE_STONEWAKE_EMPTY && loaded->gold == gold);
    ASSERT("the migrated version 195 floor round trips without losing its portal or claimed store", save_game(loaded, QUARRY_SLOT) && load_game(g, QUARRY_SLOT) && g->portal_origin_tile == TILE_STONEWAKE_CRANE_NEW && g->map.tiles[y - 3][x + 5] == TILE_STONEWAKE_EMPTY);
    cJSON_Delete(root);
}

static void test_replay(GameState *g, GameState *loaded) {
    game_init(g);
    game_enter_town_gate(g, LOCATION_TOWN2, SOUTH_AREA_GATE_X, TOWN_H - 1);
    game_enter_glassdeep(g);
    int x = -1;
    int y = -1;
    find(&g->map, TILE_STONEWAKE_CRANE_NEW, &x, &y);
    g->player.x = x;
    g->player.y = y;
    stonewake_interact(g);
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
    game_return_to_town(g);
    game_enter_glassdeep(g);
    int alive = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        alive += g->enemies[i].active;
    }
    ASSERT("a fresh physical expedition repopulates regular enemies while retaining operated cranes", alive > 0 && g->map.tiles[y][x] == TILE_STONEWAKE_CRANE_DOWN);
    g->player.x = x + 3;
    g->player.y = y;
    int count = g->enemy_count;
    g->enemies[0].hp = 7;
    g->enemies[1].active = 0;
    game_open_town_portal(g);
    ASSERT("Return to Town uses the actual last visited Stillbury town", g->location == LOCATION_TOWN2);
    ASSERT("portal expedition saves while the player is in town", save_game(g, QUARRY_SLOT) && load_game(loaded, QUARRY_SLOT));
    test_restore_cached_floor(loaded);
    ASSERT("loading the archived floor fixture retains bridge geometry, enemies and their wounds/deaths", loaded->location == LOCATION_GLASSDEEP && loaded->level == 1 && loaded->player.x == x + 3 && loaded->player.y == y && loaded->map.tiles[y][x + 3] == TILE_STONEWAKE_BRIDGE && loaded->enemy_count == count && loaded->enemies[0].hp == 7 && !loaded->enemies[1].active);
}

void test_stonewake(void) {
    printf("Stonewake Quarry machinery, progression and equipment:\n");
    if (save_exists(QUARRY_SLOT)) {
        ASSERT("Stonewake save slot must be unused", 0);
        return;
    }
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("Stonewake fixtures allocate on the heap", g && loaded);
    if (g && loaded) {
        test_layouts(g);
        test_cranes(g, loaded);
        test_debug_crane_actions(g, loaded);
        test_salvage_replay(g);
        test_hammer(g, loaded);
        test_colossus(g, loaded);
        test_replay(g, loaded);
        test_migration(g, loaded);
        test_crane_migration(g, loaded);
    }
    free(g);
    free(loaded);
    remove(QUARRY_SAVE);
}
