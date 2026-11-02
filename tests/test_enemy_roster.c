#include "test_utils.h"
#include "game/game.h"
#include "game/castle.h"
#include "game/elderwood.h"
#include "game/ironfang.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define ROSTER_SLOT 99223

static void legacy_enemy_json(cJSON *node) {
    if (cJSON_IsObject(node)) {
        cJSON_DeleteItemFromObject(node, "alignment_dormant");
        cJSON *type = cJSON_GetObjectItem(node, "type");
        if (cJSON_IsNumber(type) && cJSON_GetObjectItem(node, "move_timer")) {
            if (type->valueint == 6 || type->valueint == 92) {
                cJSON_DeleteItemFromObject(node, "is_boss");
            }
            if (type->valueint == ENEMY_GOBLIN_WARCHIEF) {
                cJSON_ReplaceItemInObject(node, "type", cJSON_CreateNumber(23));
                cJSON_ReplaceItemInObject(node, "name", cJSON_CreateString("Goblin Bomber"));
            } else if (type->valueint == ENEMY_GIANT_SPIDER) {
                cJSON_ReplaceItemInObject(node, "type", cJSON_CreateNumber(24));
                cJSON_ReplaceItemInObject(node, "name", cJSON_CreateString("Tunnel Spider"));
            }
        }
    }
    for (cJSON *child = node->child; child; child = child->next) {
        legacy_enemy_json(child);
    }
}

static int write_legacy_roster(void) {
    FILE *file = fopen("saves/savegame_99223.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = malloc((size_t)size + 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    size_t read = fread(data, 1, (size_t)size, file);
    fclose(file);
    data[read] = '\0';
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_ReplaceItemInObject(root, "save_version", cJSON_CreateNumber(221));
    legacy_enemy_json(root);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99223.json", "wb");
    int ok = file && json && fwrite(json, 1, strlen(json), file) == strlen(json);
    if (file) {
        fclose(file);
    }
    free(json);
    return ok;
}

static void test_location_migrations(GameState *g, GameState *loaded) {
    game_init(g);
    game_enter_crownroad(g, LOCATION_CROWNROAD, 0);
    g->enemy_count = 2;
    for (int i = 0; i < 2; i++) {
        g->enemies[i].type = ENEMY_HOBGOBLIN_GUARD;
        snprintf(g->enemies[i].name, sizeof(g->enemies[i].name), "Hobgoblin Guard");
        g->enemies[i].active = !i;
        g->enemies[i].hp = i ? 0 : 7;
    }
    g->crownroad_cache.valid = 1;
    g->crownroad_cache.enemy_count = 2;
    memcpy(g->crownroad_cache.enemies, g->enemies, 2 * sizeof(Enemy));
    g->crownroad_west_cache = g->crownroad_cache;
    ASSERT("version-221 roads migrate live and both cached guard rosters", save_game(g, ROSTER_SLOT) && write_legacy_roster() && load_game(loaded, ROSTER_SLOT) && loaded->enemies[0].type == ENEMY_ROYAL_GUARD && loaded->crownroad_cache.enemies[0].type == ENEMY_ROYAL_GUARD && loaded->crownroad_west_cache.enemies[0].type == ENEMY_ROYAL_GUARD);
    ASSERT("road guard migration keeps wounds and defeated patrols", loaded->enemies[0].hp == 7 && !loaded->enemies[1].active && !loaded->crownroad_cache.enemies[1].active && !loaded->crownroad_west_cache.enemies[1].active);

    game_init(g);
    ironfang_enter(g, 0);
    Enemy first = g->enemies[0];
    Enemy second = g->enemies[1];
    game_spawn_enemy(g, &g->enemies[0], ENEMY_GOBLIN_WARCHIEF, first.x, first.y);
    game_spawn_enemy(g, &g->enemies[1], ENEMY_GIANT_SPIDER, second.x, second.y);
    g->enemies[0].hp = 7;
    ironfang_store(g);
    ASSERT("version-221 Ironfang migrates retired IDs before validating its allowed roster", save_game(g, ROSTER_SLOT) && write_legacy_roster() && load_game(loaded, ROSTER_SLOT) && ironfang_valid(loaded) && loaded->enemies[0].type == ENEMY_GOBLIN_WARCHIEF && loaded->enemies[1].type == ENEMY_GIANT_SPIDER && loaded->ironfang[0].enemies[0].type == ENEMY_GOBLIN_WARCHIEF && loaded->ironfang[0].enemies[1].type == ENEMY_GIANT_SPIDER && loaded->enemies[0].hp == 7);
}

static Enemy *find_actor(GameState *g, EnemyType type) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == type) {
            return &g->enemies[i];
        }
    }
    return NULL;
}

static void switch_alignment(GameState *g) {
    for (int y = 0; y < TEMPLE_H; y++) {
        for (int x = 0; x < TEMPLE_W; x++) {
            if (g->map.tiles[y][x] == TILE_TEMPLE_ALTAR) {
                g->player.x = x;
                g->player.y = y;
                action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
                return;
            }
        }
    }
}

static void test_alignment_lifetimes(GameState *g, GameState *loaded) {
    game_init(g);
    game_enter_temple(g);
    Enemy *priest = find_actor(g, ENEMY_SUN_PRIEST);
    Enemy *effigy = &g->enemies[g->enemy_count++];
    game_spawn_enemy(g, effigy, ENEMY_LUNAR_EFFIGY, 14, 26);
    game_temple_sync_enemies(g);
    ASSERT("Sun starts with a visible priest and dormant lunar effigy", priest && priest->active && !effigy->active && effigy->alignment_dormant);
    priest->hp = 17;
    priest->poison_turns = 2;
    effigy->hp = 23;
    int remaining = game_temple_remaining_enemies(g);
    switch_alignment(g);
    ASSERT("altar interaction swaps alignment-specific actors without bypassing the seal", g->temple_alignment && !priest->active && priest->alignment_dormant && effigy->active && !effigy->alignment_dormant && game_temple_remaining_enemies(g) == remaining);
    ASSERT("alignment swaps preserve wounds and status effects", priest->hp == 17 && priest->poison_turns == 2 && effigy->hp == 23);
    ASSERT("phase dormancy round trips through save/load", save_game(g, ROSTER_SLOT) && load_game(loaded, ROSTER_SLOT) && find_actor(loaded, ENEMY_SUN_PRIEST)->alignment_dormant && !find_actor(loaded, ENEMY_SUN_PRIEST)->active && find_actor(loaded, ENEMY_LUNAR_EFFIGY)->active && game_temple_remaining_enemies(loaded) == remaining);
    switch_alignment(loaded);
    priest = find_actor(loaded, ENEMY_SUN_PRIEST);
    effigy = find_actor(loaded, ENEMY_LUNAR_EFFIGY);
    ASSERT("returning to Sun restores the same wounded priest and hides the effigy", !loaded->temple_alignment && priest->active && priest->hp == 17 && !effigy->active && effigy->alignment_dormant && effigy->hp == 23);
    priest->hp = 0;
    priest->active = 0;
    switch_alignment(loaded);
    switch_alignment(loaded);
    ASSERT("cycling alignments never resurrects a defeated priest", !priest->active && !priest->alignment_dormant && priest->hp == 0);
    game_leave_temple(loaded);
    ASSERT("cached temple floors save dormant actors", save_game(loaded, ROSTER_SLOT) && load_game(g, ROSTER_SLOT) && g->temple_cache[0].valid);
    game_enter_temple(g);
    ASSERT("new expedition reentry still applies the saved alignment", find_actor(g, ENEMY_SUN_PRIEST)->active && !find_actor(g, ENEMY_LUNAR_EFFIGY)->active && find_actor(g, ENEMY_LUNAR_EFFIGY)->alignment_dormant);
    effigy = find_actor(g, ENEMY_LUNAR_EFFIGY);
    g->temple_alignment = 1;
    g->player.x = effigy->x;
    g->player.y = effigy->y;
    game_temple_sync_enemies(g);
    ASSERT("awakening cannot place an enemy on top of the player", effigy->active && (effigy->x != g->player.x || effigy->y != g->player.y) && map_is_walkable(&g->map, effigy->x, effigy->y));
}

static void test_retired_actor_migration(GameState *g, GameState *loaded) {
    const int old_ids[] = {17, 18, 23, 24, 43, 78, 91, 92, 96, 113, 6};
    const EnemyType new_ids[] = {ENEMY_DARK_ELF_ARCHER, ENEMY_DARK_ELF_RANGER, ENEMY_GOBLIN_WARCHIEF, ENEMY_GIANT_SPIDER, ENEMY_WRAITH, ENEMY_OGRE, ENEMY_ROYAL_GUARD, ENEMY_VAMPIRE, ENEMY_DARK_ELF_RANGER, ENEMY_STONE_GIANT, ENEMY_LICH_KING};
    const char *names[] = {"Dark Elf", "Giant Wurm", "Goblin Bomber", "Tunnel Spider", "Treasure Wraith", "Rockworm", "Bell Herald", "Castellan", "Ironhide Boar", "Iron Sentinel", "Demon Lord"};
    game_init(g);
    g->location = LOCATION_FOREST;
    g->level = 1;
    map_generate_forest(&g->map, 1);
    g->player.x = g->map.stairs_up_x;
    g->player.y = g->map.stairs_up_y;
    g->enemy_count = 11;
    for (int i = 0; i < 11; i++) {
        g->enemies[i] = (Enemy){.type = old_ids[i], .active = i != 4, .hp = i == 4 ? 0 : 7, .max_hp = 90, .x = 14 + i, .y = 12, .attack = 16, .defense = 4, .experience = 28, .poison_turns = 2, .is_boss = i == 7 || i == 10, .attack_target_x = -1, .attack_target_y = -1};
        snprintf(g->enemies[i].name, sizeof(g->enemies[i].name), "%s", names[i]);
    }
    g->forest_cache[0].valid = 1;
    g->forest_cache[0].map = g->map;
    g->forest_cache[0].enemy_count = 11;
    memcpy(g->forest_cache[0].enemies, g->enemies, 11 * sizeof(Enemy));
    g->gold = 327;
    g->heartseed_quest_state = 1;
    g->defeated_bosses = 1 << LOCATION_DUNGEON;
    ASSERT("historical actor fixture saves and loads", save_game(g, ROSTER_SLOT) && write_legacy_roster() && load_game(loaded, ROSTER_SLOT));
    int preserved = 1;
    for (int i = 0; i < 11; i++) {
        const Enemy *lists[] = {&loaded->enemies[i], &loaded->forest_cache[0].enemies[i]};
        for (int n = 0; n < 2; n++) {
            const Enemy *e = lists[n];
            preserved &= e->type == new_ids[i] && e->hp == (i == 4 ? 0 : 7) && e->max_hp == 90 && e->active == (i != 4) && e->attack == 16 && e->defense == 4 && e->poison_turns == 2;
        }
    }
    ASSERT("all retired actors migrate in live and cached floors without healing or respawning", preserved);
    ASSERT("roster migration preserves currency, quests and boss victories", loaded->gold == 327 && loaded->heartseed_quest_state == 1 && loaded->defeated_bosses == (1 << LOCATION_DUNGEON));
    ASSERT("legacy bosses without an explicit boss field retain their progression roles", loaded->enemies[10].is_boss && loaded->enemies[7].is_boss && strcmp(loaded->enemies[7].name, "Vampire") == 0);
    snprintf(g->enemies[3].name, sizeof(g->enemies[3].name), "Map Bearer");
    g->enemies[3].dain_fragment = DAIN_FRAGMENT_WARCHIEF;
    ASSERT("named quest actors keep their identity after consolidation", save_game(g, ROSTER_SLOT) && load_game(loaded, ROSTER_SLOT) && loaded->enemies[3].type == ENEMY_GIANT_SPIDER && strcmp(loaded->enemies[3].name, "Map Bearer") == 0 && loaded->enemies[3].dain_fragment == DAIN_FRAGMENT_WARCHIEF);
}

static void test_opening_rosters(GameState *g) {
    const EnemyType forest[] = {ENEMY_PIXIE, ENEMY_BLIGHTED_WOLF, ENEMY_GIANT_SPIDER, ENEMY_DARK_ELF_ARCHER, ENEMY_DARK_ELF_RANGER, ENEMY_FOREST_TROLL, ENEMY_DARK_ELF_GUARD};
    const EnemyType mountains[] = {ENEMY_GOBLIN_SCOUT, ENEMY_GOBLIN_ARCHER, ENEMY_GIANT_SPIDER, ENEMY_GOBLIN_WARCHIEF, ENEMY_GOBLIN_SHAMAN, ENEMY_CAVE_TROLL, ENEMY_HOBGOBLIN_GUARD};
    unsigned int seen[2] = {0};
    int valid = 1;
    int gentle_opening = 1;
    for (int area = 0; area < 2; area++) {
        const EnemyType *roster = area ? mountains : forest;
        for (int order = 0; order < 2; order++) {
            for (int seed = 1; seed <= 8; seed++) {
                game_init(g);
                srand((unsigned int)(5300 + seed));
                g->location = area ? LOCATION_MOUNTAINS : LOCATION_FOREST;
                g->defeated_bosses = order ? 1 << LOCATION_DUNGEON : 0;
                for (int stage = 1; stage <= 5; stage++) {
                    g->level = stage;
                    if (area) {
                        map_generate_mountains(&g->map, stage);
                    } else {
                        map_generate_forest(&g->map, stage);
                    }
                    enemies_spawn(g);
                    for (int i = 0; i < g->enemy_count; i++) {
                        const Enemy *enemy = &g->enemies[i];
                        if (enemy->is_boss) {
                            valid &= enemy->type == (area ? ENEMY_MOUNTAIN_GOBLIN_KING : ENEMY_FOREST_NECROMANCER);
                            continue;
                        }
                        int found = 0;
                        for (int type = 0; type < 7; type++) {
                            if (enemy->type == roster[type]) {
                                found = 1;
                                seen[area] |= 1u << type;
                            }
                        }
                        valid &= found;
                        if (!area && !order && stage < 3 && enemy->type == ENEMY_DARK_ELF_GUARD) {
                            gentle_opening = 0;
                        }
                    }
                }
            }
        }
    }
    ASSERT("all five forest and mountain stages use only the approved rosters across seeds and boss progress", valid);
    ASSERT("every approved regular enemy appears in its intended area", seen[0] == 127 && seen[1] == 127);
    ASSERT("unscaled forest opening stages introduce lighter enemies before Dark Elf Guards", gentle_opening);
}

static void test_dark_elf_guard(GameState *g, GameState *loaded) {
    game_init(g);
    game_enter_forest(g);
    Enemy hobgoblin;
    Room *room = &g->map.rooms[0];
    game_spawn_enemy(g, &g->enemies[0], ENEMY_DARK_ELF_GUARD, room->x + room->w / 2, room->y + room->h / 2);
    game_spawn_enemy(g, &hobgoblin, ENEMY_HOBGOBLIN_GUARD, 0, 0);
    Enemy *guard = &g->enemies[0];
    ASSERT("Dark Elf Guard matches Hobgoblin Guard combat strength and experience", guard->max_hp == hobgoblin.max_hp && guard->attack == hobgoblin.attack && guard->defense == hobgoblin.defense && guard->experience == hobgoblin.experience && !guard->is_boss);
    ASSERT("Dark Elf Guard has its own identity", guard->type == ENEMY_DARK_ELF_GUARD && strcmp(guard->name, "Dark Elf Guard") == 0);
    g->enemy_count = 1;
    guard->hp = 11;
    guard->poison_turns = 2;
    guard->frozen_turns = 1;
    g->forest_cache[0].valid = 1;
    g->forest_cache[0].map = g->map;
    g->forest_cache[0].enemy_count = 1;
    g->forest_cache[0].enemies[0] = *guard;
    int saved = save_game(g, ROSTER_SLOT) && load_game(loaded, ROSTER_SLOT);
    ASSERT("Dark Elf Guard saves in active and cached forest stages", saved && loaded->enemies[0].type == ENEMY_DARK_ELF_GUARD && loaded->forest_cache[0].enemies[0].type == ENEMY_DARK_ELF_GUARD);
    ASSERT("guard wounds and status effects survive active and cached save/load", saved && loaded->enemies[0].hp == 11 && loaded->enemies[0].poison_turns == 2 && loaded->enemies[0].frozen_turns == 1 && loaded->forest_cache[0].enemies[0].hp == 11 && loaded->forest_cache[0].enemies[0].poison_turns == 2 && loaded->forest_cache[0].enemies[0].frozen_turns == 1);

    game_init(g);
    g->location = LOCATION_FOREST;
    g->level = 1;
    for (int y = 10; y < 30; y++) {
        for (int x = 10; x < 30; x++) {
            g->map.tiles[y][x] = TILE_FOREST_FLOOR;
        }
    }
    g->player.x = 20;
    g->player.y = 20;
    g->player.hp = 1000;
    g->player.max_hp = 1000;
    g->player.defense = 0;
    g->equipped_armor = -1;
    g->equipped_off_hand = -1;
    g->enemy_count = 4;
    game_spawn_enemy(g, &g->enemies[0], ENEMY_DARK_ELF_GUARD, 15, 20);
    game_spawn_enemy(g, &g->enemies[1], ENEMY_DARK_ELF_ARCHER, 14, 20);
    game_spawn_enemy(g, &g->enemies[2], ENEMY_PIXIE, 18, 20);
    game_spawn_enemy(g, &g->enemies[3], ENEMY_BLIGHTED_WOLF, 17, 21);
    EnemyProjectiles shots = {0};
    action_resolve_enemies_with_projectiles(g, &shots);
    guard = &g->enemies[0];
    ASSERT("Dark Elf Guard advances to protect an archer ahead of nearer skirmishers", abs(guard->x - g->player.x) + abs(guard->y - g->player.y) < 5);
    g->enemy_count = 1;
    guard->x = 19;
    guard->y = 20;
    int hp = g->player.hp;
    action_resolve_enemies_with_projectiles(g, &shots);
    ASSERT("Dark Elf Guard attacks in melee without firing arrows", g->player.hp < hp && shots.count == 0);

    guard->hp = 1;
    g->player.player_class = CLASS_MAGE;
    g->player.known_spells[0] = spell_make_magic_arrow();
    g->player.known_spell_count = 1;
    g->player.equipped_spell = 0;
    g->player.mp = 100;
    g->player.last_dx = -1;
    g->player.last_dy = 0;
    int score = g->score;
    int experience = g->player.experience;
    action_resolve_player(g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("defeating a Dark Elf Guard awards normal guard score and experience", !guard->active && g->score == score + 70 && g->player.experience == experience + 44);
    Enemy *revenant = game_raise_grove_servant(g, 19, 19, 0);
    ASSERT("the Necromancer can raise a fallen Dark Elf Guard without creating another reward", revenant == guard && revenant->type == ENEMY_DARK_ELF_GUARD && revenant->hp == revenant->max_hp / 2 && revenant->revived && revenant->experience == 0);
}

void test_enemy_roster(void) {
    printf("Approved enemy roster and alignment lifetimes\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("roster fixtures allocate without large static states", g && loaded);
    if (g && loaded) {
        test_opening_rosters(g);
        test_dark_elf_guard(g, loaded);
        test_alignment_lifetimes(g, loaded);
        test_retired_actor_migration(g, loaded);
        test_location_migrations(g, loaded);
    }
    free(g);
    free(loaded);
    remove("saves/savegame_99223.json");
}
