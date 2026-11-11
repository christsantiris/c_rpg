#include "test_utils.h"
#include "game/actions.h"
#include "game/approach.h"
#include "game/eastern_arc.h"
#include "game/quarry_quest.h"
#include "game/resurrection.h"
#include "game/aldric.h"
#include "game/prism_sovereign.h"
#include "game/stonewake.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define QUARRY_SLOT 99291
static GameState g;
static GameState loaded;

static void start(void) {
    memset(&g, 0, sizeof(g));
    game_init(&g);
    approach_enter_region(&g, APPROACH_QUARRY_ROAD);
}

static int travel(TileType tile) {
    return map_is_travel_tile(tile) || tile == TILE_GLASSDEEP_ENTRANCE || tile == TILE_GLASSDEEP_EXIT;
}

static int walk_to(int tx, int ty) {
    int previous[MAP_W * MAP_H];
    int queue[MAP_W * MAP_H];
    for (int i = 0; i < MAP_W * MAP_H; i++) {
        previous[i] = -1;
    }
    int start = g.player.y * MAP_W + g.player.x;
    int target = ty * MAP_W + tx;
    if (start == target && travel(g.map.tiles[ty][tx])) {
        const int inward[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
        for (int i = 0; i < 4; i++) {
            int x = tx + inward[i][0];
            int y = ty + inward[i][1];
            if (map_is_walkable(&g.map, x, y) && !travel(g.map.tiles[y][x])) {
                action_resolve_player(&g, (Action){ACTION_MOVE, x, y});
                return walk_to(tx, ty);
            }
        }
        return 0;
    }
    int head = 0;
    int tail = 1;
    previous[start] = start;
    queue[0] = start;
    const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail && previous[target] < 0) {
        int cell = queue[head++];
        for (int side = 0; side < 4; side++) {
            int x = cell % MAP_W + steps[side][0];
            int y = cell / MAP_W + steps[side][1];
            if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
                continue;
            }
            int next = y * MAP_W + x;
            if (previous[next] >= 0 || !map_is_walkable(&g.map, x, y) || (travel(g.map.tiles[y][x]) && next != target)) {
                continue;
            }
            previous[next] = cell;
            queue[tail++] = next;
        }
    }
    if (previous[target] < 0) {
        return 0;
    }
    int count = 0;
    for (int cell = target; cell != start; cell = previous[cell]) {
        queue[count++] = cell;
    }
    Location location = g.location;
    int level = g.level;
    for (int i = count - 1; i >= 0; i--) {
        int x = queue[i] % MAP_W;
        int y = queue[i] / MAP_W;
        action_resolve_player(&g, (Action){ACTION_MOVE, x, y});
        if (g.location != location || g.level != level) {
            return i == 0;
        }
        if (g.player.x != x || g.player.y != y) {
            return 0;
        }
    }
    return 1;
}

static void quiet(void) {
    for (int i = 0; i < g.enemy_count; i++) {
        g.enemies[i].active = 0;
        g.enemies[i].hp = 0;
    }
    g.dialogue_active = 0;
}

static void orin(void) {
    g.dialogue_active = 0;
    ASSERT("a real path reaches Orin beside the workers' camp", walk_to(QUARRY_ORIN_X, QUARRY_ORIN_Y + 1) && quarry_talk(&g));
}

static void completion(void) {
    start();
    orin();
    ASSERT("Orin offers an optional quest on the western road", game_quest_offer_active(&g) && !g.glassdeep_quest_state);
    game_handle_quest_offer_key(&g, SDL_SCANCODE_Y, 0);
    g.glassdeep_quest_progress = 7;
    quarry_update(&g);
    ASSERT("repairs alone do not complete the quest", g.glassdeep_quest_state == 1 && game_active_quest_count(&g) == 1);
    g.royal_reported_actions = 3;
    g.defeated_bosses |= 1u << LOCATION_GLASSDEEP;
    quarry_record_defeat(&g);
    ASSERT("boss plus repairs completes immediately and forgives Stillbury's local hostility", g.glassdeep_quest_state == 2 && !game_active_quest_count(&g) && !eastern_stillbury_hostile(&g) && g.royal_reported_actions == 3 && g.quarry_potion == QUARRY_POTION_PENDING);
    int gold = g.gold;
    int score = g.score;
    orin();
    ASSERT("returning pays currency once and opens the equipment choice", g.glassdeep_quest_state == 3 && g.gold == gold + 120 && g.score == score + 1000 && g.quarry_prompt == QUARRY_EQUIPMENT);
    quarry_handle_key(&g, SDL_SCANCODE_ESCAPE, 0);
    orin();
    ASSERT("deferring a reward never pays currency twice", g.gold == gold + 120 && g.score == score + 1000 && g.quarry_equipment == QUARRY_PENDING);
}

static void equipment(void) {
    for (int choice = 1; choice <= 3; choice++) {
        start();
        g.glassdeep_quest_progress = 7;
        g.defeated_bosses |= 1u << LOCATION_GLASSDEEP;
        quarry_update(&g);
        g.inventory_count = MAX_INVENTORY;
        for (int i = 0; i < MAX_INVENTORY; i++) {
            g.inventory[i] = item_make_health_potion();
        }
        orin();
        quarry_handle_key(&g, SDL_SCANCODE_1 + choice - 1, 0);
        ASSERT("a full pack leaves the equipment choice safely pending", g.quarry_equipment == QUARRY_PENDING && g.inventory_count == MAX_INVENTORY);
        ASSERT("pending equipment choices and dialogue round-trip through save/load", save_game(&g, QUARRY_SLOT) && load_game(&loaded, QUARRY_SLOT) && loaded.quarry_equipment == QUARRY_PENDING && loaded.quarry_prompt == QUARRY_EQUIPMENT && quarry_dialogue(&loaded));
        g = loaded;
        g.inventory_count = 0;
        quarry_handle_key(&g, SDL_SCANCODE_1 + choice - 1, 1);
        ASSERT("held number keys cannot grant a reward", !g.inventory_count);
        quarry_handle_key(&g, SDL_SCANCODE_1 + choice - 1, 0);
        ASSERT("each equipment option grants exactly one item", g.inventory_count == 1 && g.quarry_equipment == QUARRY_PENDING + choice);
        if (choice == 1) {
            ASSERT("warhammer retains its approved stats", g.inventory[0].attack_bonus == 8 && g.inventory[0].max_hp_bonus == 30);
        } else if (choice == 2) {
            ASSERT("quarry bow has the approved attack, range, HP and class", g.inventory[0].attack_bonus == 9 && g.inventory[0].range == 6 && g.inventory[0].max_hp_bonus == 20 && g.inventory[0].class_mask == ITEM_CLASS_ROGUE);
        } else {
            ASSERT("robes retain their approved defense, MP and spell discount", g.inventory[0].defense_bonus == 3 && g.inventory[0].max_mp_bonus == 35 && g.inventory[0].spell_cost_reduction_percent == 12);
        }
        ASSERT("claimed equipment and item stats survive save/load", save_game(&g, QUARRY_SLOT) && load_game(&loaded, QUARRY_SLOT) && loaded.quarry_equipment == g.quarry_equipment && loaded.inventory[0].max_hp_bonus == g.inventory[0].max_hp_bonus);
        g = loaded;
        g.inventory_count = 0;
        orin();
        quarry_handle_key(&g, SDL_SCANCODE_1, 0);
        ASSERT("selling or dropping the reward cannot grant another", !g.inventory_count && g.quarry_prompt == QUARRY_MENU);
    }
}

static int find(TileType tile, int *x, int *y) {
    for (*y = 0; *y < MAP_H; (*y)++) {
        for (*x = 0; *x < MAP_W; (*x)++) {
            TileType actual = g.map.tiles[*y][*x];
            if (actual == TILE_ITEM) {
                for (int i = 0; i < g.floor_item_count; i++) {
                    if (g.floor_items[i].active && g.floor_items[i].x == *x && g.floor_items[i].y == *y) {
                        actual = g.floor_items[i].underlying_tile;
                        break;
                    }
                }
            }
            if (actual == tile) {
                return 1;
            }
        }
    }
    return 0;
}

static void stages(void) {
    for (int seed = 11; seed <= 13; seed++) {
        start();
        srand(seed);
        ASSERT("western camp leaves the real quarry entrance usable", walk_to(g.map.stairs_down_x, g.map.stairs_down_y) && g.location == LOCATION_GLASSDEEP && g.level == 1);
        for (int stage = 1; stage <= GLASSDEEP_DEPTH; stage++) {
            quiet();
            int crane_x[2];
            int crane_y[2];
            int cranes = 0;
            for (int y = 0; y < MAP_H && cranes < 2; y++) {
                for (int x = 0; x < MAP_W && cranes < 2; x++) {
                    if (stonewake_crane_tile(g.map.tiles[y][x])) {
                        crane_x[cranes] = x;
                        crane_y[cranes++] = y;
                    }
                }
            }
            for (int i = 0; i < cranes; i++) {
                ASSERT("crane controls remain reachable with the new machinery", walk_to(crane_x[i] - 1, crane_y[i]));
                for (int mode = 0; mode < seed - 11; mode++) {
                    ASSERT("normal interaction sets the seed's raised, crossing or salvage state", stonewake_interact(&g));
                }
            }
            if (stage >= 2 && stage <= 4) {
                int x;
                int y;
                int found = find(TILE_GLASSDEEP_RESONATOR, &x, &y);
                ASSERT("machinery is present before accepting Orin's quest", !g.glassdeep_quest_state && found);
                if (!found) {
                    return;
                }
                ASSERT("a continuous route reaches the machinery", walk_to(x, y));
                action_resolve_player(&g, (Action){ACTION_INTERACT, 0, 0});
                ASSERT("repairs before accepting are recorded", g.glassdeep_quest_progress & (1 << (stage - 2)));
            }
            if (stage < GLASSDEEP_DEPTH) {
                ASSERT("the actual south boundary advances to the next quarry stage", walk_to(g.map.stairs_down_x, g.map.stairs_down_y) && g.level == stage + 1 && g.player.last_dy == 1);
                quiet();
                ASSERT("the actual north boundary retreats to the cached stage", walk_to(g.map.stairs_up_x, g.map.stairs_up_y) && g.level == stage && g.player.last_dy == -1);
                quiet();
                ASSERT("cached stages preserve their usable onward transitions", walk_to(g.map.stairs_down_x, g.map.stairs_down_y) && g.level == stage + 1 && g.player.last_dy == 1);
            }
        }
        ASSERT("all three discoveries stay recorded without requiring a quest slot", g.glassdeep_quest_progress == 7 && !game_active_quest_count(&g));
        ASSERT("the final quarry objective retains its retreat", walk_to(g.map.stairs_up_x, g.map.stairs_up_y) && g.level == GLASSDEEP_DEPTH - 1);
        quiet();
        ASSERT("the final cached stage remains reachable", walk_to(g.map.stairs_down_x, g.map.stairs_down_y) && g.level == GLASSDEEP_DEPTH);
        quiet();
        ASSERT("the actual final exit returns only to the local quarry road", walk_to(g.map.stairs_down_x, g.map.stairs_down_y) && quarry_road(&g));
    }
}

static void potions(void) {
    for (int choice = 1; choice <= 2; choice++) {
        start();
        game_enter_glassdeep(&g);
        for (int stage = 1; stage < GLASSDEEP_DEPTH; stage++) {
            game_descend(&g);
        }
        quiet();
        Enemy *boss = NULL;
        for (int i = 0; i < g.enemy_count; i++) {
            if (g.enemies[i].type == ENEMY_PRISM_SOVEREIGN) {
                boss = &g.enemies[i];
                break;
            }
        }
        ASSERT("the final quarry stage retains its boss", boss != NULL);
        if (!boss) {
            return;
        }
        boss->active = 1;
        boss->hp = 1;
        g.player.x = boss->x - 1;
        g.player.y = boss->y;
        g.player.attack = 999;
        action_resolve_player(&g, (Action){ACTION_MOVE, boss->x, boss->y});
        ASSERT("a real Colossus kill records its victory and new reward", !boss->active && (g.defeated_bosses & (1u << LOCATION_GLASSDEEP)) && g.quarry_potion == QUARRY_POTION_PENDING);
        prism_sovereign_restore_reward(&g);
        int x;
        int y;
        ASSERT("a new Colossus victory creates a choice strongbox without legacy automatic loot", !g.prism_sovereign_reward_pending && (find(TILE_PRISM_RELIQUARY, &x, &y) || find(TILE_PRISM_PAVED_RELIQUARY, &x, &y)));
        ASSERT("the strongbox can be reached through actual map tiles", walk_to(x, y));
        action_resolve_player(&g, (Action){ACTION_PICK_UP, 0, 0});
        ASSERT("the normal pickup action opens the potion choice", quarry_dialogue(&g) && g.quarry_prompt == QUARRY_POTION);
        g.inventory_count = MAX_INVENTORY;
        for (int i = 0; i < MAX_INVENTORY; i++) {
            g.inventory[i] = item_make_health_potion();
        }
        quarry_handle_key(&g, SDL_SCANCODE_1, 0);
        ASSERT("a full pack preserves the potion strongbox", g.quarry_potion == QUARRY_POTION_PENDING && g.inventory_count == MAX_INVENTORY);
        ASSERT("the unchosen potion and its dialogue survive save/load", save_game(&g, QUARRY_SLOT) && load_game(&loaded, QUARRY_SLOT) && loaded.quarry_potion == QUARRY_POTION_PENDING && loaded.quarry_prompt == QUARRY_POTION);
        g = loaded;
        g.inventory_count = 0;
        quarry_handle_key(&g, SDL_SCANCODE_1 + choice - 1, 0);
        ASSERT("the chosen potion is granted once", g.quarry_potion == QUARRY_POTION_PENDING + choice && g.inventory_count == 1 && strcmp(g.inventory[0].name, choice == 1 ? "Potion of Strength" : "Potion of Intelligence") == 0);
        g.inventory_count = 0;
        prism_sovereign_restore_reward(&g);
        ASSERT("claimed strongboxes cannot reopen after dropping the potion", !quarry_reward_interact(&g, 0) && !find(TILE_PRISM_RELIQUARY, &x, &y) && !find(TILE_PRISM_PAVED_RELIQUARY, &x, &y));
    }
}

static void shrine_and_welcome(void) {
    start();
    ASSERT("the western shrine is usable before either quest or boss", resurrection_available(&g) && !g.defeated_bosses && !g.glassdeep_quest_state && g.map.tiles[SOULSTONE_Y][SOULSTONE_X] == TILE_SOULSTONE_SHRINE && map_is_explored(&g.map, SOULSTONE_X, SOULSTONE_Y));
    ASSERT("the camp has a continuous path to the western warlock", walk_to(SOULSTONE_WARLOCK_X, SOULSTONE_WARLOCK_Y - 1) && resurrection_talk(&g));
    g.aldric_party_state = ALDRIC_DEAD;
    g.aldric_hp = 0;
    g.gold = RESURRECTION_GOLD;
    resurrection_handle_key(&g, SDL_SCANCODE_1, 0);
    resurrection_handle_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("the western warlock uses normal paid resurrection", g.aldric_party_state == ALDRIC_FOLLOWING && g.aldric_hp == ALDRIC_MAX_HP && !g.gold);
    g.player.player_class = CLASS_MAGE;
    g.player.level = RESURRECTION_LEVEL;
    g.player.max_mp = 200;
    g.player.mp = 200;
    g.player.known_spells[0] = spell_make_resurrection();
    g.player.known_spell_count = 1;
    g.player.equipped_spell = 0;
    g.aldric_party_state = ALDRIC_DEAD;
    g.aldric_hp = 0;
    ASSERT("the western ritual circle can be reached and cast at", walk_to(SOULSTONE_RITUAL_X, SOULSTONE_RITUAL_Y) && resurrection_offer_spell(&g));
    resurrection_handle_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("western Mage resurrection uses the existing MP cost and half HP", g.aldric_party_state == ALDRIC_FOLLOWING && g.aldric_hp == ALDRIC_MAX_HP / 2 && g.player.mp == 200 - RESURRECTION_MP);
    g.glassdeep_quest_progress = 7;
    g.defeated_bosses |= 1u << LOCATION_GLASSDEEP;
    g.royal_reported_actions = 3;
    quarry_update(&g);
    const int checks[4][2] = {{6, 12}, {33, 12}, {20, 5}, {20, 18}};
    for (int i = 0; i < 4; i++) {
        game_enter_town_gate(&g, LOCATION_TOWN2, 0, 12);
        eastern_move(&g, checks[i][0], checks[i][1]);
        ASSERT("every Stillbury checkpoint allows its restorer and anchors the safe town", !g.stillbury_prompt && !eastern_town_combat(&g) && g.last_town_visited == LOCATION_TOWN2 && g.royal_reported_actions == 3);
    }
    ASSERT("the restored friendly attitude survives save/load", save_game(&g, QUARRY_SLOT) && load_game(&loaded, QUARRY_SLOT) && !eastern_stillbury_hostile(&loaded) && loaded.royal_reported_actions == 3);
}

static int downgrade(void) {
    const char *path = "saves/savegame_99291.json";
    FILE *file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = calloc((size_t)size + 1, 1);
    int ok = data && fread(data, 1, (size_t)size, file) == (size_t)size;
    fclose(file);
    cJSON *root = ok ? cJSON_Parse(data) : NULL;
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 248);
    cJSON_DeleteItemFromObject(root, "quarry_equipment");
    cJSON_DeleteItemFromObject(root, "quarry_potion");
    cJSON_DeleteItemFromObject(root, "quarry_prompt");
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(path, "wb");
    ok = data && file && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static int machines(const Map *map) {
    int count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            count += map->tiles[y][x] == TILE_GLASSDEEP_RESONATOR || map->tiles[y][x] == TILE_GLASSDEEP_RESONATOR_LIT;
        }
    }
    return count;
}

static void remove_machinery(Map *map) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (map->tiles[y][x] == TILE_GLASSDEEP_RESONATOR) {
                map->tiles[y][x] = TILE_GLASSDEEP_RUIN;
            }
        }
    }
}

static void legacy(void) {
    start();
    g.glassdeep_quest_state = 2;
    g.glassdeep_quest_progress = 7;
    g.inventory_count = MAX_INVENTORY;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        g.inventory[i] = item_make_health_potion();
    }
    ASSERT("an old completed quest migrates without requiring a new boss victory", save_game(&g, QUARRY_SLOT) && downgrade() && load_game(&loaded, QUARRY_SLOT) && loaded.glassdeep_quest_state == 2 && loaded.quarry_equipment == QUARRY_LEGACY);
    g = loaded;
    int gold = g.gold;
    orin();
    ASSERT("old unpaid quests retain their original currency and reserved hammer", g.gold == gold + 120 && g.stonewake_warhammer_state == 1 && g.quarry_equipment == QUARRY_LEGACY);
    g.inventory_count = 0;
    orin();
    ASSERT("the grandfathered hammer can be claimed once after freeing the pack", g.inventory_count == 1 && g.stonewake_warhammer_state == 2 && g.inventory[0].max_hp_bonus == 30);
    orin();
    ASSERT("legacy rewards do not gain new equipment choices", g.inventory_count == 1 && g.gold == gold + 120 && g.quarry_prompt == QUARRY_MENU);

    start();
    game_enter_glassdeep(&g);
    for (int stage = 1; stage < 4; stage++) {
        game_descend(&g);
    }
    g.glassdeep_quest_encounters = 0;
    remove_machinery(&g.map);
    for (int stage = 1; stage <= 3; stage++) {
        remove_machinery(&g.glassdeep_cache[stage].map);
    }
    g.enemies[0].hp = 7;
    g.glassdeep_cache[1].enemies[0].hp = 9;
    map_mark_explored(&g.map, 35, 3);
    ASSERT("older active and cached quarry maps migrate in place", save_game(&g, QUARRY_SLOT) && downgrade() && load_game(&loaded, QUARRY_SLOT) && machines(&loaded.map) == 1 && machines(&loaded.glassdeep_cache[1].map) == 1 && machines(&loaded.glassdeep_cache[2].map) == 1);
    ASSERT("migration preserves wounds, exploration and pre-quest state", loaded.enemies[0].hp == 7 && loaded.glassdeep_cache[1].enemies[0].hp == 9 && map_is_explored(&loaded.map, 35, 3) && !loaded.glassdeep_quest_state);

    start();
    game_enter_glassdeep(&g);
    for (int stage = 1; stage < GLASSDEEP_DEPTH; stage++) {
        game_descend(&g);
    }
    quiet();
    g.glassdeep_quest_state = 1;
    g.defeated_bosses |= 1u << LOCATION_GLASSDEEP;
    g.prism_sovereign_reward_pending = PRISM_ROBES_PENDING | PRISM_POTION_PENDING;
    ASSERT("old boss loot remains grandfathered and cannot create a new potion choice", save_game(&g, QUARRY_SLOT) && downgrade() && load_game(&loaded, QUARRY_SLOT) && loaded.quarry_potion == QUARRY_POTION_LEGACY && loaded.prism_sovereign_reward_pending == (PRISM_ROBES_PENDING | PRISM_POTION_PENDING));

    start();
    g.royal_reported_actions = 3;
    game_enter_town_gate(&g, LOCATION_TOWN2, 0, 12);
    eastern_move(&g, 6, 12);
    eastern_handle_key(&g, SDL_SCANCODE_N, 0);
    g.enemies[0].hp = 7;
    g.glassdeep_quest_progress = 7;
    g.defeated_bosses |= 1u << LOCATION_GLASSDEEP;
    quarry_update(&g);
    ASSERT("restoring the quarry withdraws an existing checkpoint squad without erasing wounds", !g.enemy_count && !g.stillbury_challenge && g.stillbury_guards_initialized && g.stillbury_guards[0].hp == 7 && eastern_valid(&g));
    ASSERT("retired checkpoint guards and the restored welcome survive save/load", save_game(&g, QUARRY_SLOT) && load_game(&loaded, QUARRY_SLOT) && loaded.stillbury_guards[0].hp == 7 && !loaded.enemy_count && !eastern_stillbury_hostile(&loaded));
}

void test_quarry_quest(void) {
    printf("Stillbury quarry recovery, reward choices and western shrine:\n");
    completion();
    equipment();
    stages();
    potions();
    shrine_and_welcome();
    legacy();
    remove("saves/savegame_99291.json");
}
