#include "test_utils.h"
#include "game/castle.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define CASTLE_TEST_SLOT 99136
static GameState game;
static GameState loaded;
static unsigned char visited[MAP_H][MAP_W];
static int queue[MAP_W * MAP_H];

static void start(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    game.player.hp = 1000;
    game.player.max_hp = 1000;
    castle_enter(&game, 1);
}

static Enemy *boss(void) {
    for (int i = 0; i < game.enemy_count; i++) {
        if (game.enemies[i].is_boss && game.enemies[i].active) {
            return &game.enemies[i];
        }
    }
    return NULL;
}

static void kill_boss(void) {
    Enemy *e = boss();
    if (!e) {
        return;
    }
    game.player.attack = 10000;
    game.player.experience_next = 100000;
    game.player.x = e->x;
    game.player.y = e->y + 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
}

static void test_castle_layout(void) {
    start();
    for (int floor = 1; floor <= 6; floor++) {
        castle_enter(&game, floor);
        memset(visited, 0, sizeof(visited));
        int head = 0;
        int tail = 1;
        queue[0] = game.player.y * MAP_W + game.player.x;
        visited[game.player.y][game.player.x] = 1;
        const int dx[4] = {0, 1, 0, -1};
        const int dy[4] = {-1, 0, 1, 0};
        while (head < tail) {
            int x = queue[head] % MAP_W;
            int y = queue[head++] / MAP_W;
            for (int d = 0; d < 4; d++) {
                int tx = x + dx[d];
                int ty = y + dy[d];
                if (map_is_walkable(&game.map, tx, ty) && !visited[ty][tx]) {
                    visited[ty][tx] = 1;
                    queue[tail++] = ty * MAP_W + tx;
                }
            }
        }
        int reachable = visited[game.map.stairs_down_y][game.map.stairs_down_x];
        int distinct = 1;
        int bosses = 0;
        for (int i = 0; i < game.enemy_count; i++) {
            Enemy *e = &game.enemies[i];
            reachable &= visited[e->y][e->x];
            bosses += e->is_boss;
            for (int j = 0; j < i; j++) {
                distinct &= e->x != game.enemies[j].x || e->y != game.enemies[j].y;
            }
        }
        ASSERT("castle guards and progression reachable around closed gates", reachable && distinct);
        ASSERT("castle minibosses occur only on 2/4; final boss on 6", bosses == (floor == 2 || floor == 4 || floor == 6));
        ASSERT("castle entrance is protected from traps", game.map.tiles[game.player.y][game.player.x] == TILE_STAIRS_DOWN);
    }
}

static void test_castle_progress(void) {
    start();
    castle_enter(&game, 2);
    castle_travel(&game, 1);
    ASSERT("living Castellan blocks floor 3", game.level == 2);
    kill_boss();
    ASSERT("melee victory records miniboss without ending campaign", game.castle_minibosses == 1 && !game.game_won && !(game.defeated_bosses & (1 << LOCATION_CASTLE_INTERIOR)));
    ASSERT("Castellan victory reveals persistent passage", game.map.tiles[52][48] == TILE_CASTLE_PASSAGE);
    castle_travel(&game, 1);
    ASSERT("Castellan victory opens floor 3", game.level == 3);
    castle_enter(&game, 4);
    castle_travel(&game, 1);
    ASSERT("living Arcanist blocks floor 5", game.level == 4);
    kill_boss();
    castle_travel(&game, 1);
    ASSERT("Arcanist victory opens floor 5", game.level == 5 && game.castle_minibosses == 3);
    castle_travel(&game, 1);
    ASSERT("missing seals block only throne entrance", game.level == 5 && game.dialogue_active);
    game.castle_seals = 15;
    castle_travel(&game, 1);
    ASSERT("four seals open floor 6", game.level == 6 && boss() && boss()->type == ENEMY_LORD_VEYR);
    kill_boss();
    ASSERT("final boss ends the game with victory", game.game_won && (game.defeated_bosses & (1 << LOCATION_CASTLE_INTERIOR)));
    int hp = game.player.hp;
    action_resolve_enemies(&game);
    ASSERT("no enemy phase follows campaign victory", game.player.hp == hp);
    int x = game.player.x;
    action_resolve_player(&game, (Action){ACTION_MOVE, x + 1, game.player.y});
    ASSERT("completed campaign cannot continue moving", game.player.x == x);
}

static void test_castle_fall_and_retreat(void) {
    start();
    castle_enter(&game, 2);
    kill_boss();
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 17, .y = 32, .underlying_tile = TILE_CASTLE_TRAP_HIDDEN, .item = item_make_magic_shield()};
    game.map.tiles[32][17] = TILE_ITEM;
    game.player.x = 17;
    game.player.y = 32;
    int count = game.castle_cache[0].enemy_count;
    game.castle_cache[0].level_cleared = 1;
    ASSERT("trapdoor drops exactly one floor", castle_step(&game) && game.level == 1);
    ASSERT("fall landing safe and reinforcement patrol created", game.player.x == 8 && game.player.y == 15 && game.enemy_count == count + 2 && !game.level_cleared);
    ASSERT("fall reveals hole beneath loot permanently", game.castle_cache[1].map.burial_traps[0].spent && game.castle_loot[1][0].underlying_tile == TILE_CASTLE_TRAP_OPEN);
    castle_enter(&game, 2);
    ASSERT("defeated miniboss and dropped loot survive fall", !boss() && game.floor_item_count == 1 && game.floor_items[0].item.type == ITEM_SHIELD);
    game.player.x = 17;
    game.player.y = 32;
    castle_step(&game);
    ASSERT("repeated fall cannot summon another patrol", game.enemy_count == count + 2);
    castle_enter(&game, 4);
    Enemy *e = boss();
    e->hp = 1;
    game.enemies[0].active = 0;
    game.castle_seals = 5;
    game.defeated_bosses |= 1 << LOCATION_SWAMP;
    game.portal_active = 1;
    game_open_town_portal(&game);
    ASSERT("castle escape requires confirmation", game.castle_prompt == 2 && game.location == LOCATION_CASTLE_INTERIOR);
    castle_prompt_key(&game, SDL_SCANCODE_ESCAPE, 0);
    ASSERT("cancel escape preserves current attempt", game.location == LOCATION_CASTLE_INTERIOR && boss()->hp == 1);
    game_open_town_portal(&game);
    castle_prompt_key(&game, SDL_SCANCODE_RETURN, 0);
    ASSERT("confirmed escape reaches town without return portal", game.location == LOCATION_TOWN3 && !game.portal_active);
    ASSERT("castle escape preserves Rosemoor's unlocked swamp shortcut", game.map.tiles[TOWN_H - 1][ROSEMOOR_SWAMP_ROAD_X] == TILE_TOWN_EXIT);
    ASSERT("retreat retains seals and miniboss victory", game.castle_seals == 5 && game.castle_minibosses == 1);
    castle_enter(&game, 4);
    ASSERT("retreat restores surviving boss and ordinary guards", boss()->hp == boss()->max_hp && game.enemies[0].active);
    castle_enter(&game, 2);
    ASSERT("retreat preserves known trapdoor and unique loot", game.map.burial_traps[0].spent && game.floor_item_count == 1 && !boss());
    castle_leave(&game, 0);
    game.player.x = 17;
    game.player.y = 12;
    ASSERT("earned passage is reachable from grounds", castle_interact(&game) && game.level == 2 && game.player.x == 48 && game.player.y == 52);
}

static void test_castle_combat(void) {
    start();
    castle_enter(&game, 2);
    Enemy *e = boss();
    game.player.x = e->x;
    game.player.y = e->y - 3;
    int hp = game.player.hp;
    action_resolve_enemies(&game);
    ASSERT("Castellan warns before damaging player", e->move_timer == 1 && game.player.hp == hp);
    ASSERT("charge warning marks fixed lane", castle_attack_marks(&game.map, e, game.player.x, game.player.y));
    game.player.x++;
    action_resolve_enemies(&game);
    ASSERT("side step avoids charge and exposes recovery", game.player.hp == hp && e->move_timer == 2);
    e->move_timer = 0;
    e->facing_dx = 0;
    e->facing_dy = -1;
    game.player.x = e->x;
    game.player.y = e->y - 1;
    ASSERT("frontal shield reduces damage", castle_enemy_damage(&game, e, 20) == 10);
    game.player.y = e->y + 1;
    ASSERT("flank bypasses shield", castle_enemy_damage(&game, e, 20) == 20);
    e->move_timer = 2;
    game.player.y = e->y - 1;
    ASSERT("recovery permits full damage", castle_enemy_damage(&game, e, 20) == 20);
    castle_enter(&game, 4);
    e = boss();
    game.player.x = 45;
    game.player.y = 53;
    action_resolve_enemies(&game);
    ASSERT("Arcanist announces changing wards", castle_barrier_warning(&game, 40, 48));
    TileType gate = game.map.tiles[48][40];
    e->frozen_turns = 1;
    action_resolve_enemies(&game);
    ASSERT("freeze pauses boss attack and barrier change", game.map.tiles[48][40] == gate && e->move_timer == 3);
    game.player.x++;
    action_resolve_enemies(&game);
    game.player.x++;
    action_resolve_enemies(&game);
    ASSERT("warned wards change after player's dodge turn", game.map.tiles[48][40] != gate);
}

static void test_castle_seals(void) {
    const Location regions[4] = {LOCATION_DUNGEON, LOCATION_GLASSDEEP, LOCATION_MOONVEIL, LOCATION_ASHEN};
    for (int i = 0; i < 4; i++) {
        start();
        game.location = regions[i];
        game.level = 3;
        if (i == 0) {
            map_generate(&game.map, 3);
        } else if (i == 1) {
            map_generate_glassdeep(&game.map, 3);
        } else if (i == 2) {
            map_generate_moonveil(&game.map, 3);
        } else {
            map_generate_ashen(&game.map, 3);
        }
        castle_refresh_seal(&game);
        int found = 0;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                if (game.map.tiles[y][x] == TILE_CASTLE_SEAL) {
                    game.player.x = x;
                    game.player.y = y;
                    found++;
                }
            }
        }
        ASSERT("one guaranteed seal appears on each town's regional floor 3", found == 1);
        ASSERT("seal interaction records correct permanent bit", castle_interact(&game) && game.castle_seals == (1 << i));
        castle_refresh_seal(&game);
        ASSERT("collected seal does not respawn", game.map.tiles[game.player.y][game.player.x] != TILE_CASTLE_SEAL);
    }
}

static void test_castle_saves(void) {
    start();
    castle_enter(&game, 2);
    kill_boss();
    game.map.burial_traps[0].spent = 1;
    game.map.tiles[32][17] = TILE_CASTLE_TRAP_OPEN;
    game.castle_seals = 9;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 8, .y = 15, .underlying_tile = TILE_CASTLE_FLOOR, .item = item_make_magic_shield()};
    game.map.tiles[15][8] = TILE_ITEM;
    castle_enter(&game, 4);
    Enemy *e = boss();
    e->hp = 123;
    e->move_timer = 1;
    e->attack_target_x = 45;
    e->attack_target_y = 53;
    e->facing_dx = 1;
    e->facing_dy = 0;
    castle_request(&game, 1);
    ASSERT("castle saves successfully", save_game(&game, CASTLE_TEST_SLOT));
    ASSERT("castle loads successfully", load_game(&loaded, CASTLE_TEST_SLOT));
    ASSERT("save restores castle and permanent progress", loaded.location == LOCATION_CASTLE_INTERIOR && loaded.level == 4 && loaded.castle_seals == 9 && loaded.castle_minibosses == 1);
    ASSERT("save restores warning and enemy facing", loaded.enemies[loaded.enemy_count - 1].hp == 123 && loaded.enemies[loaded.enemy_count - 1].attack_target_y == 53 && loaded.enemies[loaded.enemy_count - 1].facing_dx == 1);
    ASSERT("save restores pending escape confirmation", loaded.castle_prompt == 2 && loaded.dialogue_active);
    castle_prompt_key(&loaded, SDL_SCANCODE_ESCAPE, 0);
    castle_enter(&loaded, 2);
    ASSERT("cached floor restores revealed trap and full item metadata", loaded.map.tiles[32][17] == TILE_CASTLE_TRAP_OPEN && loaded.floor_item_count == 1 && loaded.floor_items[0].item.block_chance == game.castle_loot[1][0].item.block_chance);
    game.game_won = 1;
    ASSERT("campaign victory survives save/load", save_game(&game, CASTLE_TEST_SLOT) && load_game(&loaded, CASTLE_TEST_SLOT) && loaded.game_won);
    remove("saves/savegame_99136.json");
}

static void test_castle_class_victory(void) {
    for (int player_class = CLASS_MAGE; player_class <= CLASS_ROGUE; player_class++) {
        start();
        castle_enter(&game, 6);
        Enemy *e = boss();
        e->hp = 1;
        game.player.player_class = player_class;
        game.player.x = e->x;
        game.player.y = e->y + 1;
        game.player.last_dx = 0;
        game.player.last_dy = -1;
        game.player.attack = 1000;
        game.player.experience_next = 100000;
        if (player_class == CLASS_MAGE) {
            game.player.known_spells[0] = spell_make_magic_arrow();
            game.player.known_spells[0].damage = 1000;
            game.player.known_spell_count = 1;
            game.player.equipped_spell = 0;
            game.player.mp = 100;
            action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
        } else {
            game.player.y = e->y + 2;
            game.inventory[0] = item_make_bow();
            game.inventory_count = 1;
            game.equipped_main_hand = 0;
            action_resolve_player(&game, (Action){ACTION_RANGED_ATTACK, 0, 0});
        }
        ASSERT("magic and bow kills trigger campaign victory", game.game_won);
    }
}

static void test_castle_wide_warning(void) {
    start();
    castle_enter(&game, 6);
    Enemy *e = boss();
    e->hp = e->max_hp / 4;
    game.player.x = 45;
    game.player.y = 53;
    int hp = game.player.hp;
    action_resolve_enemies(&game);
    ASSERT("Unbound Crown warns for three movement turns", e->move_timer == 4 && e->attack_phase == 3);
    for (int i = 0; i < 3; i++) {
        game.player.x++;
        action_resolve_enemies(&game);
    }
    ASSERT("ordinary walking escapes widest boss attack", game.player.hp == hp && e->move_timer == 2);
    ASSERT("Veyr summons a capped escort once", e->revived && game.enemy_count == 12);
    game.player.x = 45;
    game.player.y = 53;
    e->hp = e->max_hp / 2;
    e->move_timer = 0;
    action_resolve_enemies(&game);
    e->hp = e->max_hp / 4;
    ASSERT("health phase changes cannot widen pending warning", e->attack_phase == 2 && !castle_attack_marks(&game.map, e, 45, 55));
    start();
    game.player.x = game.map.stairs_down_x;
    game.player.y = game.map.stairs_down_y;
    action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
    ASSERT("ordinary onward command climbs castle stairs", game.level == 2);
    action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
    ASSERT("ordinary back command descends castle stairs", game.level == 1);
}

static void test_castle_mechanisms(void) {
    start();
    game.inventory[0] = item_make_health_potion();
    game.inventory_count = 1;
    game.equipped_main_hand = -1;
    action_resolve_player(&game, (Action){ACTION_DROP_ITEM, 0, 0});
    ASSERT("loot cannot hide castle stairs", game.map.tiles[game.player.y][game.player.x] == TILE_STAIRS_DOWN);
    action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
    ASSERT("stairs remain usable with dropped loot", game.location == LOCATION_CASTLE);
    castle_enter(&game, 2);
    game.player.x = 48;
    game.player.y = 52;
    game.inventory[0] = item_make_health_potion();
    game.inventory_count = 1;
    action_resolve_player(&game, (Action){ACTION_DROP_ITEM, 0, 0});
    kill_boss();
    ASSERT("miniboss passage reveals beneath existing loot", game.map.tiles[52][48] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_CASTLE_PASSAGE);
    game.player.x = 48;
    game.player.y = 52;
    ASSERT("loot cannot prevent passage interaction", castle_has_interaction(&game));
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("picking up loot restores earned passage", game.map.tiles[52][48] == TILE_CASTLE_PASSAGE);

    start();
    game.enemy_count = 1;
    Enemy *e = &game.enemies[0];
    e->type = ENEMY_ROYAL_MARKSMAN;
    e->x = 25;
    e->y = 11;
    e->attack_target_x = 31;
    e->attack_target_y = 11;
    e->move_timer = 1;
    game.map.tiles[11][29] = TILE_CASTLE_GATE_OPEN;
    ASSERT("ranged warning follows open shooting lane", castle_attack_marks(&game.map, e, 31, 11));
    game.map.tiles[11][29] = TILE_CASTLE_GATE;
    ASSERT("closing portcullis blocks a pending ranged attack", !castle_attack_marks(&game.map, e, 31, 11));
    game.player.x = 20;
    game.player.y = 11;
    e->x = 22;
    e->attack_target_x = -1;
    e->move_timer = 0;
    game.map.tiles[11][21] = TILE_CASTLE_PILLAR;
    castle_tick(&game);
    ASSERT("ranged guards reposition around nearby cover instead of attacking through it", e->move_timer == 0 && e->attack_target_x < 0 && (e->x != 22 || e->y != 11));

    start();
    castle_enter(&game, 3);
    game.enemies[0] = game.enemies[8];
    game.enemy_count = 1;
    game.player.x = 49;
    game.player.y = 33;
    castle_tick(&game);
    castle_tick(&game);
    ASSERT("Herald calls exactly one reinforcement", game.enemy_count == 2 && game.enemies[0].revived);
    for (int i = 0; i < 15; i++) {
        castle_tick(&game);
    }
    ASSERT("Herald melee attacks cannot repeat reinforcement call", game.enemy_count == 2);
    castle_leave(&game, 0);
    castle_enter(&game, 3);
    ASSERT("retreat preserves Herald's spent reinforcement allowance", game.enemies[8].revived);
}

static void test_castle_legacy_save(void) {
    start();
    game.location = LOCATION_DUNGEON;
    game.level = 3;
    map_generate(&game.map, 3);
    game.enemy_count = 0;
    game.floor_item_count = 0;
    game.gold = 321;
    game.score = 9876;
    strcpy(game.player.name, "Legacy Hero");
    ASSERT("legacy fixture saves", save_game(&game, CASTLE_TEST_SLOT));
    FILE *f = fopen("saves/savegame_99136.json", "rb");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *buffer = malloc((size_t)size + 1);
    fread(buffer, 1, (size_t)size, f);
    buffer[size] = '\0';
    fclose(f);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    cJSON_DeleteItemFromObject(root, "castle");
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 85);
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    f = fopen("saves/savegame_99136.json", "wb");
    fputs(json, f);
    fclose(f);
    free(json);
    ASSERT("version 85 save migrates without castle fields", load_game(&loaded, CASTLE_TEST_SLOT));
    ASSERT("migration preserves character and progress", strcmp(loaded.player.name, "Legacy Hero") == 0 && loaded.gold == 321 && loaded.score == 9876 && loaded.level == 3);
    ASSERT("migration initializes fresh castle progress", !loaded.castle_seals && !loaded.castle_minibosses && !loaded.game_won && !loaded.castle_cache[0].valid);
    int seals = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            seals += loaded.map.tiles[y][x] == TILE_CASTLE_SEAL;
        }
    }
    ASSERT("old explored floor receives its collectible without regeneration", seals == 1 && loaded.map.stairs_up_x == game.map.stairs_up_x && loaded.map.stairs_down_y == game.map.stairs_down_y);
    int enemies = loaded.enemy_count;
    game_refresh_quest_encounters(&loaded);
    ASSERT("seal migration cannot duplicate guardian on refresh", loaded.enemy_count == enemies && enemies == 1);
    ASSERT("migrated save rewrites and reloads", save_game(&loaded, CASTLE_TEST_SLOT) && load_game(&game, CASTLE_TEST_SLOT));
    remove("saves/savegame_99136.json");
}

void test_castle(void) {
    printf("Castle of No Return\n");
    test_castle_layout();
    test_castle_progress();
    test_castle_fall_and_retreat();
    test_castle_combat();
    test_castle_seals();
    test_castle_saves();
    test_castle_class_victory();
    test_castle_wide_warning();
    test_castle_mechanisms();
    test_castle_legacy_save();
}
