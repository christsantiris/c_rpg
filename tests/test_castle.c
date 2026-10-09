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
        int clear_corridors = 1;
        for (int i = 0; i < game.map.room_count; i++) {
            int x;
            int y;
            map_room_center(&game.map.rooms[i], &x, &y);
            reachable &= visited[y][x];
            if (i % 2 == 0) {
                for (int ty = y - 1; ty <= y + 1; ty++) {
                    for (int tx = x; tx <= x + 30; tx++) {
                        clear_corridors &= game.map.tiles[ty][tx] != TILE_CASTLE_TABLE;
                    }
                }
            }
        }
        for (int y = 0; y < CASTLE_H; y++) {
            for (int x = 0; x < CASTLE_W; x++) {
                if (map_is_walkable(&game.map, x, y)) {
                    reachable &= visited[y][x];
                }
            }
        }
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
        ASSERT("every castle room is connected and tables leave carpeted corridors clear", reachable && clear_corridors);
        if (floor == 3 || floor == 4) {
            loaded = game;
            loaded.enemy_count = 0;
            loaded.player.x = 15;
            loaded.player.y = 31;
            for (int x = 16; x <= 45; x++) {
                action_resolve_player(&loaded, (Action){ACTION_MOVE, x, 31});
            }
            ASSERT("court corridors can be walked directly without moving tables or using a lever",
                loaded.player.x == 45 && loaded.player.y == 31 && loaded.level == floor);
        }
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
    ASSERT("Castellan victory leaves ordinary floor without an exit portal", game.map.tiles[52][48] == TILE_CASTLE_FLOOR);
    castle_travel(&game, 1);
    ASSERT("Castellan victory opens floor 3", game.level == 3);
    castle_enter(&game, 4);
    castle_travel(&game, 1);
    ASSERT("living Arcanist blocks floor 5", game.level == 4);
    kill_boss();
    castle_travel(&game, 1);
    ASSERT("Arcanist victory opens floor 5", game.level == 5 && game.castle_minibosses == 3);
    castle_travel(&game, 1);
    ASSERT("throne opens without regional collectibles", game.level == 6 && boss() && boss()->type == ENEMY_LORD_VEYR);
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
    ASSERT("retreat retains miniboss victory", game.castle_minibosses == 1);
    castle_enter(&game, 4);
    ASSERT("retreat restores surviving boss and ordinary guards", boss()->hp == boss()->max_hp && game.enemies[0].active);
    castle_enter(&game, 2);
    ASSERT("retreat preserves known trapdoor and unique loot", game.map.burial_traps[0].spent && game.floor_item_count == 1 && !boss());
    castle_leave(&game, 0);
    game.player.x = 17;
    game.player.y = 12;
    ASSERT("retired shortcut cannot enter the keep from the grounds", !castle_has_interaction(&game) &&
        !castle_interact(&game) && game.location == LOCATION_CASTLE);
    game.player.x = 23;
    game.castle_minibosses |= 2;
    ASSERT("retired chapel shortcut is also unavailable", !castle_has_interaction(&game) && !castle_interact(&game));
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

static void test_castle_no_seals(void) {
    const Location regions[4] = {LOCATION_DUNGEON, LOCATION_GLASSDEEP, LOCATION_MOONVEIL, LOCATION_ASHEN};
    for (int i = 0; i < 4; i++) {
        start();
        game.location = regions[i];
        game.level = 3;
        game.enemy_count = 0;
        if (i == 0) {
            map_generate(&game.map, 3);
        } else if (i == 1) {
            map_generate_glassdeep(&game.map, 3);
        } else if (i == 2) {
            map_generate_moonveil(&game.map, 3);
        } else {
            map_generate_ashen(&game.map, 3);
        }
        game_refresh_quest_encounters(&game);
        int found = 0;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                found += game.map.tiles[y][x] == TILE_CASTLE_SEAL;
            }
        }
        ASSERT("regional floors no longer spawn castle collectibles or guardians", found == 0 && game.enemy_count == 0);
    }
}

static int write_legacy_seal_save(void) {
    FILE *file = fopen("saves/savegame_99136.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc((size_t)size + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    size_t read = fread(buffer, 1, (size_t)size, file);
    buffer[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 88);
    cJSON_AddNumberToObject(cJSON_GetObjectItem(root, "castle"), "seals", 7);
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    if (!json) {
        return 0;
    }
    file = fopen("saves/savegame_99136.json", "wb");
    if (!file) {
        free(json);
        return 0;
    }
    int written = fputs(json, file) >= 0;
    fclose(file);
    free(json);
    return written;
}

static void test_castle_removed_seal_migration(void) {
    const Location regions[4] = {LOCATION_DUNGEON, LOCATION_GLASSDEEP, LOCATION_MOONVEIL, LOCATION_ASHEN};
    const TileType floors[4] = {TILE_FLOOR, TILE_GLASSDEEP_FLOOR, TILE_MOONVEIL_FLOOR, TILE_ASHEN_FLOOR};
    for (int i = 0; i < 4; i++) {
        start();
        game.location = regions[i];
        game.level = 3;
        game.player.x = 15;
        game.player.y = 11;
        if (i == 0) {
            map_generate(&game.map, 3);
        } else if (i == 1) {
            map_generate_glassdeep(&game.map, 3);
        } else if (i == 2) {
            map_generate_moonveil(&game.map, 3);
        } else {
            map_generate_ashen(&game.map, 3);
        }
        game.gold = 321;
        game.castle_minibosses = 1;
        game.elowen_quest_state = 2;
        game.elowen_seals_restored = 7;
        game.map.tiles[11][11] = TILE_ITEM;
        game.map.tiles[11][12] = TILE_CASTLE_SEAL;
        game.map.tiles[11][14] = TILE_PORTAL;
        map_mark_explored(&game.map, 12, 11);
        game.enemy_count = 2;
        game.enemies[0] = (Enemy){.active = 1, .type = ENEMY_SKELETON, .x = 10, .y = 11, .hp = 100, .max_hp = 100};
        strcpy(game.enemies[0].name, "Royal Seal Guardian");
        game.enemies[1] = (Enemy){.active = 1, .type = ENEMY_SKELETON, .x = 13, .y = 11, .hp = 7, .max_hp = 100};
        strcpy(game.enemies[1].name, "Skeleton");
        game.floor_item_count = 1;
        game.floor_items[0] = (FloorItem){.active = 1, .x = 11, .y = 11, .underlying_tile = TILE_CASTLE_SEAL, .item = item_make_health_potion()};
        game.portal_active = 1;
        game.portal_location = regions[i];
        game.portal_level = 3;
        game.portal_x = 14;
        game.portal_y = 11;
        game.portal_origin_tile = TILE_CASTLE_SEAL;
        game.dialogue_active = 1;
        strcpy(game.dialogue_speaker, "Royal seals");
        strcpy(game.dialogue_text, "Four seals are required.");
        LevelCache *caches[] = {game.level_cache, game.glassdeep_cache, game.moonveil_cache, game.ashen_cache};
        LevelCache *cache = &caches[i][2];
        cache->valid = 1;
        cache->map = game.map;
        cache->enemy_count = 2;
        memcpy(cache->enemies, game.enemies, sizeof(cache->enemies));
        ASSERT("version 88 royal seal fixture loads", save_game(&game, CASTLE_TEST_SLOT) && write_legacy_seal_save() && load_game(&loaded, CASTLE_TEST_SLOT));
        ASSERT("retired marker becomes regional ground without losing exploration", loaded.map.tiles[11][12] == floors[i] && map_is_explored(&loaded.map, 12, 11));
        ASSERT("migration preserves loot while replacing its seal underlay", loaded.floor_items[0].active && loaded.floor_items[0].item.type == ITEM_POTION_HEALTH && loaded.floor_items[0].underlying_tile == floors[i] && loaded.map.tiles[11][11] == TILE_ITEM);
        ASSERT("migration preserves portal while replacing its seal underlay", loaded.portal_active && loaded.portal_origin_tile == floors[i] && loaded.map.tiles[11][14] == floors[i]);
        ASSERT("migration removes only the extra royal guardian", !loaded.enemies[0].active && loaded.enemies[1].active && loaded.enemies[1].hp == 7);
        LevelCache *restored[] = {loaded.level_cache, loaded.glassdeep_cache, loaded.moonveil_cache, loaded.ashen_cache};
        cache = &restored[i][2];
        ASSERT("cached royal markers and guardians are also retired", cache->valid && cache->map.tiles[11][12] == floors[i] && !cache->enemies[0].active && cache->enemies[1].hp == 7);
        ASSERT("migration preserves gold, miniboss wins and burial quest progress", loaded.gold == 321 && loaded.castle_minibosses == 1 && loaded.elowen_quest_state == 2 && loaded.elowen_seals_restored == 7);
        ASSERT("obsolete seal requirement dialogue is cleared", !loaded.dialogue_active && loaded.dialogue_speaker[0] == '\0');
        ASSERT("migrated save round-trips without seal fields", save_game(&loaded, CASTLE_TEST_SLOT) && load_game(&game, CASTLE_TEST_SLOT) && !game.enemies[0].active && game.floor_items[0].underlying_tile == floors[i]);
    }
    remove("saves/savegame_99136.json");
}

static void test_castle_saves(void) {
    start();
    castle_enter(&game, 2);
    kill_boss();
    game.map.burial_traps[0].spent = 1;
    game.map.tiles[32][17] = TILE_CASTLE_TRAP_OPEN;
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
    ASSERT("save restores castle and permanent progress", loaded.location == LOCATION_CASTLE_INTERIOR && loaded.level == 4 && loaded.castle_minibosses == 1);
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
            game.player.arrows = MAX_ARROWS;
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
    ASSERT("Veyr summons a capped escort once", e->revived && game.enemy_count == 19);
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
    ASSERT("descending cannot climb castle stairs", game.level == 1 && game.location == LOCATION_CASTLE_INTERIOR);
    action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
    ASSERT("ascending climbs castle stairs", game.level == 2 && game.location == LOCATION_CASTLE_INTERIOR);
    action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
    ASSERT("ascending cannot descend castle stairs", game.level == 2 && game.location == LOCATION_CASTLE_INTERIOR);
    action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
    ASSERT("descending returns to previous castle floor", game.level == 1 && game.location == LOCATION_CASTLE_INTERIOR);
}

static void test_castle_mechanisms(void) {
    start();
    game.inventory[0] = item_make_health_potion();
    game.inventory_count = 1;
    game.equipped_main_hand = -1;
    action_resolve_player(&game, (Action){ACTION_DROP_ITEM, 0, 0});
    ASSERT("loot cannot hide castle stairs", game.map.tiles[game.player.y][game.player.x] == TILE_STAIRS_DOWN);
    action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
    ASSERT("stairs remain usable with dropped loot", game.location == LOCATION_CASTLE);
    castle_enter(&game, 2);
    game.player.x = 48;
    game.player.y = 52;
    game.inventory[0] = item_make_health_potion();
    game.inventory_count = 1;
    action_resolve_player(&game, (Action){ACTION_DROP_ITEM, 0, 0});
    kill_boss();
    ASSERT("miniboss victory keeps ordinary floor beneath existing loot", game.map.tiles[52][48] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_CASTLE_FLOOR);
    game.player.x = 48;
    game.player.y = 52;
    ASSERT("miniboss victory cannot create an exit interaction", !castle_has_interaction(&game) && !castle_interact(&game));
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("picking up loot restores ordinary castle floor", game.map.tiles[52][48] == TILE_CASTLE_FLOOR);

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
    ASSERT("Court Herald calls one three-guard patrol", game.enemy_count == 4 && game.enemies[0].revived);
    for (int i = 0; i < 15; i++) {
        castle_tick(&game);
    }
    ASSERT("Herald melee attacks cannot repeat reinforcement call", game.enemy_count == 4);
    castle_leave(&game, 0);
    castle_enter(&game, 3);
    ASSERT("retreat preserves Herald's spent reinforcement allowance", game.enemies[8].revived);
}

static int save_legacy_layout(int version) {
    if (!save_game(&game, CASTLE_TEST_SLOT)) {
        return 0;
    }
    FILE *file = fopen("saves/savegame_99136.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc((size_t)size + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    size_t count = fread(buffer, 1, (size_t)size, file);
    buffer[count] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99136.json", "wb");
    int result = file && json;
    if (result) {
        result = fputs(json, file) >= 0;
    }
    if (file) {
        fclose(file);
    }
    free(json);
    return result;
}

static void test_castle_layout_migration(void) {
    for (int scenario = 0; scenario < 3; scenario++) {
        start();
        game.castle_minibosses = 3;
        game.gold = 321;
        for (int floor = 2; floor <= 4; floor++) {
            castle_enter(&game, floor);
            if (floor == 3 || floor == 4) {
                for (int i = 0; i < game.map.room_count; i++) {
                    Room *room = &game.map.rooms[i];
                    for (int y = room->y + 6; y <= room->y + 8; y++) {
                        game.map.tiles[y][room->x + 5] = TILE_CASTLE_TABLE;
                        game.map.tiles[y][room->x + room->w - 6] = TILE_CASTLE_TABLE;
                    }
                }
            }
            game.map.tiles[52][48] = TILE_CASTLE_PASSAGE;
            game.floor_item_count = 2;
            game.floor_items[0] = (FloorItem){.active = 1, .x = 48, .y = 52,
                .underlying_tile = TILE_CASTLE_PASSAGE, .item = item_make_magic_shield()};
            game.map.tiles[52][48] = TILE_ITEM;
            game.floor_items[1] = (FloorItem){.active = 1, .x = 20, .y = 31,
                .underlying_tile = floor >= 3 ? TILE_CASTLE_TABLE : TILE_CASTLE_CARPET, .item = item_make_health_potion()};
            game.map.tiles[31][20] = TILE_ITEM;
            game.map.tiles[52][49] = TILE_CASTLE_PASSAGE;
            game.map.tiles[32][17] = TILE_CASTLE_TRAP_OPEN;
            game.map.burial_traps[0].spent = 1;
            game.map.burial_traps[0].timer = 1;
            map_mark_explored(&game.map, 20, 31);
            game.map.tiles[11][29] = TILE_CASTLE_GATE_OPEN;
            game.enemies[0].active = 0;
            game.enemies[1].hp = 7;
            game.enemies[1].move_timer = 1;
            game.enemies[1].attack_target_x = 15;
            game.enemies[1].attack_target_y = 32;
            castle_store(&game);
        }
        if (scenario < 2) {
            castle_enter(&game, scenario == 0 ? 3 : 4);
            game.player.x = 19;
            game.player.y = 31;
        } else {
            // The player may be outside the castle when a cached floor needs repair.
            game.location = LOCATION_CASTLE;
            game.level = 1;
            map_generate_castle(&game.map, &game.player.x, &game.player.y);
            game.floor_item_count = 0;
            game.enemy_count = 0;
        }
        int saved = save_legacy_layout(118) && load_game(&loaded, CASTLE_TEST_SLOT);
        ASSERT("castle layout migration preserves character, position, victories, and rewards",
            saved && loaded.location == game.location && loaded.level == game.level &&
            loaded.player.x == game.player.x && loaded.player.y == game.player.y &&
            loaded.gold == 321 && loaded.castle_minibosses == 3);
        int repaired = saved;
        for (int i = 1; i <= 3; i++) {
            LevelCache *cache = &loaded.castle_cache[i];
            repaired &= cache->valid && cache->map.tiles[52][49] == TILE_CASTLE_FLOOR &&
                cache->map.tiles[52][48] == TILE_ITEM && loaded.castle_loot[i][0].underlying_tile == TILE_CASTLE_FLOOR &&
                loaded.castle_loot[i][0].item.block_chance == game.castle_loot[i][0].item.block_chance &&
                cache->map.tiles[31][20] == TILE_ITEM && loaded.castle_loot[i][1].underlying_tile == TILE_CASTLE_CARPET &&
                !cache->enemies[0].active && cache->enemies[1].hp == 7 && cache->enemies[1].move_timer == 1 &&
                cache->enemies[1].attack_target_y == 32 && cache->map.burial_traps[0].spent &&
                cache->map.burial_traps[0].timer && cache->map.tiles[32][17] == TILE_CASTLE_TRAP_OPEN &&
                map_is_explored(&cache->map, 20, 31) && cache->map.tiles[11][29] == TILE_CASTLE_GATE_OPEN;
            if (i >= 2) {
                repaired &= cache->map.tiles[30][20] == TILE_CASTLE_CARPET && cache->map.tiles[32][39] == TILE_CASTLE_CARPET &&
                    cache->map.tiles[10][9] == TILE_CASTLE_TABLE;
            }
        }
        ASSERT("cached castle migration removes portals and blocking tables while retaining loot, combat, gates, and traps", repaired);
        if (scenario < 2) {
            ASSERT("live court migration repairs the same corridor and portal beneath loot",
                saved && loaded.map.tiles[30][20] == TILE_CASTLE_CARPET && loaded.map.tiles[32][39] == TILE_CASTLE_CARPET &&
                loaded.floor_items[0].underlying_tile == TILE_CASTLE_FLOOR &&
                loaded.floor_items[1].underlying_tile == TILE_CASTLE_CARPET && !loaded.enemies[0].active && loaded.enemies[1].hp == 7);
            loaded.enemy_count = 0;
            action_resolve_player(&loaded, (Action){ACTION_MOVE, 20, 31});
            action_resolve_player(&loaded, (Action){ACTION_PICK_UP, 0, 0});
            ASSERT("picking up migrated corridor loot exposes walkable carpet", loaded.player.x == 20 &&
                loaded.map.tiles[31][20] == TILE_CASTLE_CARPET);
        }
        ASSERT("repaired castle layout and progress survive another save/load",
            saved && save_game(&loaded, CASTLE_TEST_SLOT) && load_game(&game, CASTLE_TEST_SLOT) &&
            game.castle_minibosses == 3 && game.castle_cache[2].map.tiles[30][20] == TILE_CASTLE_CARPET &&
            game.castle_cache[1].map.tiles[52][49] == TILE_CASTLE_FLOOR);
    }
    remove("saves/savegame_99136.json");
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
    ASSERT("migration initializes fresh castle progress", !loaded.castle_minibosses && !loaded.game_won && !loaded.castle_cache[0].valid);
    int seals = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            seals += loaded.map.tiles[y][x] == TILE_CASTLE_SEAL;
        }
    }
    ASSERT("old explored floor loads without collectibles or regeneration", seals == 0 && loaded.map.stairs_up_x == game.map.stairs_up_x && loaded.map.stairs_down_y == game.map.stairs_down_y);
    int enemies = loaded.enemy_count;
    game_refresh_quest_encounters(&loaded);
    ASSERT("legacy refresh no longer adds royal guardians", loaded.enemy_count == enemies && enemies == 0);
    ASSERT("migrated save rewrites and reloads", save_game(&loaded, CASTLE_TEST_SLOT) && load_game(&game, CASTLE_TEST_SLOT));
    remove("saves/savegame_99136.json");
}

static void test_castle_floor_tactics(void) {
    start();
    Enemy shot = game.enemies[1];
    shot.x = 15;
    shot.y = 25;
    shot.attack_target_x = 15;
    shot.attack_target_y = 17;
    ASSERT("gatehouse has two guarded approaches with independent cover", game.enemy_count == 12 &&
        game.map.tiles[11][29] == TILE_CASTLE_GATE && game.map.tiles[21][15] == TILE_CASTLE_GATE_OPEN &&
        castle_attack_marks(&game.map, &shot, 15, 17));
    game.player.x = 10;
    game.player.y = 14;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("second lever closes only its own portcullis and blocks a shooting lane",
        game.map.tiles[21][15] == TILE_CASTLE_GATE && game.map.tiles[11][29] == TILE_CASTLE_GATE &&
        !castle_attack_marks(&game.map, &shot, 15, 17));
    castle_interact(&game);
    ASSERT("second lever can reopen the alternate approach", game.map.tiles[21][15] == TILE_CASTLE_GATE_OPEN);

    castle_enter(&game, 2);
    Enemy *e = boss();
    Enemy *guard = &game.enemies[8];
    ASSERT("Castellan has a Warden and Marksman guard detail", guard->type == ENEMY_IRON_WARDEN &&
        game.enemies[9].type == ENEMY_ROYAL_MARKSMAN && game.enemy_count == 11);
    for (int i = 0; i < game.enemy_count; i++) {
        game.enemies[i].active = &game.enemies[i] == e || &game.enemies[i] == guard;
    }
    game.player.x = 46;
    game.player.y = 45;
    guard->x = 45;
    guard->y = 47;
    guard->move_timer = 2;
    e->attack_phase = 1;
    e->attack_target_x = 45;
    e->attack_target_y = 44;
    e->move_timer = 1;
    castle_tick(&game);
    ASSERT("baiting Castellan's charge into a guard breaks its defense and interrupts its attack",
        e->y == 48 && guard->frozen_turns == 2 && guard->attack_target_x == -1 &&
        castle_enemy_damage(&game, guard, 20) == 20);

    for (int player_class = CLASS_WARRIOR; player_class <= CLASS_ROGUE; player_class++) {
        start();
        castle_enter(&game, 4);
        e = boss();
        game.player.player_class = player_class;
        ASSERT("two chapel pedestals protect the Arcanist", castle_enemy_damage(&game, e, 100) == 60);
        game.player.x = 37;
        game.player.y = 53;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("every class can disable a pedestal and reduce royal protection",
            game.map.tiles[54][37] == TILE_CASTLE_WARD_SPENT && castle_enemy_damage(&game, e, 100) == 80);
        game.player.x = 53;
        game.player.y = 53;
        castle_interact(&game);
        ASSERT("disabling both pedestals removes ward protection", castle_enemy_damage(&game, e, 100) == 100);
        castle_enter(&game, 5);
        castle_enter(&game, 4);
        ASSERT("disabled wards survive floor revisits", game.map.tiles[54][37] == TILE_CASTLE_WARD_SPENT);
        castle_leave(&game, 0);
        castle_enter(&game, 4);
        ASSERT("retreat restores wards around a surviving Arcanist", game.map.tiles[54][37] == TILE_CASTLE_WARD);
    }
    e = boss();
    game.enemies[0] = *e;
    game.enemy_count = 1;
    e = &game.enemies[0];
    game.player.x = 45;
    game.player.y = 53;
    game.player.defense = 0;
    e->attack_target_x = 45;
    e->attack_target_y = 53;
    e->move_timer = 1;
    ASSERT("active pedestals also strengthen warned boss attacks", castle_tick(&game) == e->attack + 8);

    start();
    castle_enter(&game, 5);
    int px = game.enemies[0].x;
    int py = game.enemies[0].y;
    castle_tick(&game);
    ASSERT("archive guards patrol while the player is elsewhere", game.enemies[0].x != px || game.enemies[0].y != py);
    ASSERT("archive shelves create cover beside open carpeted routes", game.map.tiles[26][40] == TILE_CASTLE_BOOKCASE &&
        map_is_walkable(&game.map, 45, 31) && game.map.tiles[34][45] == TILE_CASTLE_FIRE_RUNE);
    game.enemy_count = 0;
    game.castle_fire_phase[4] = 0;
    game.player.x = 45;
    game.player.y = 34;
    game.player.defense = 0;
    ASSERT("archive fire gives a first harmless warning", castle_tick(&game) == 0 && castle_fire_warning(&game, 45, 34));
    ASSERT("archive fire gives a second harmless warning", castle_tick(&game) == 0 && game.castle_fire_phase[4] == 2);
    ASSERT("remaining in a warned lane causes fire damage", castle_tick(&game) == 24 && !castle_fire_warning(&game, 45, 34));
    game.castle_fire_phase[4] = 2;
    game.player.y = 33;
    ASSERT("one ordinary step escapes the warned fire lane", castle_tick(&game) == 0);
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 45, .y = 34,
        .underlying_tile = TILE_CASTLE_FIRE_RUNE, .item = item_make_health_potion()};
    game.map.tiles[34][45] = TILE_ITEM;
    game.player.y = 34;
    game.castle_fire_phase[4] = 2;
    ASSERT("loot cannot conceal or deactivate a warned fire lane", castle_fire_warning(&game, 45, 34) && castle_tick(&game) == 24);
    castle_request(&game, 1);
    int phase = game.castle_fire_phase[4];
    ASSERT("escape confirmation pauses floor hazards", castle_tick(&game) == 0 && game.castle_fire_phase[4] == phase);

    start();
    castle_enter(&game, 6);
    e = boss();
    ASSERT("throne combines guarded approaches, wards, a muster door and fire runes",
        game.map.tiles[21][15] == TILE_CASTLE_GATE_OPEN && game.map.tiles[54][37] == TILE_CASTLE_WARD &&
        game.map.tiles[46][53] == TILE_CASTLE_MUSTER && game.map.tiles[51][45] == TILE_CASTLE_FIRE_RUNE &&
        castle_enemy_damage(&game, e, 100) == 30);
    game.enemies[8].revived = 0;
    game.enemies[15].revived = 1;
    castle_leave(&game, 0);
    castle_enter(&game, 6);
    ASSERT("each Herald retains its own spent call through retreat", !game.enemies[8].revived && game.enemies[15].revived);
}

static void test_castle_tactics_saves(void) {
    start();
    castle_enter(&game, 4);
    game.player.x = 37;
    game.player.y = 53;
    castle_interact(&game);
    castle_enter(&game, 5);
    game.castle_fire_phase[4] = 2;
    game.enemies[0].facing_dx = 1;
    game.enemies[0].facing_dy = 0;
    castle_enter(&game, 6);
    game.castle_fire_phase[5] = 1;
    game.enemies[15].revived = 1;
    ASSERT("new castle encounters save and load", save_game(&game, CASTLE_TEST_SLOT) && load_game(&loaded, CASTLE_TEST_SLOT));
    ASSERT("save restores live and cached fire warnings, ward state, patrol facing and Herald calls",
        loaded.castle_fire_phase[4] == 2 && loaded.castle_fire_phase[5] == 1 && loaded.enemies[15].revived &&
        loaded.castle_cache[3].map.tiles[54][37] == TILE_CASTLE_WARD_SPENT && loaded.castle_cache[4].enemies[0].facing_dx == 1);
    castle_enter(&loaded, 5);
    ASSERT("revisited archive resumes the saved warning rather than restarting its timer", castle_fire_warning(&loaded, 45, 34));

    // Build a version 119 fixture with the old roster and floor decoration.
    start();
    game.castle_minibosses = 1;
    game.gold = 321;
    for (int level = 1; level <= CASTLE_DEPTH; level++) {
        castle_enter(&game, level);
        Enemy *e = boss();
        int count = 8 + (level >= 3);
        if (e) {
            game.enemies[count++] = *e;
        }
        game.enemy_count = count;
        game.enemies[0].hp = 7;
        game.enemies[1].active = 0;
        game.level_cleared = level == 1;
        for (int y = 0; y < CASTLE_H; y++) {
            for (int x = 0; x < CASTLE_W; x++) {
                TileType tile = game.map.tiles[y][x];
                if (tile >= TILE_CASTLE_WARD) {
                    game.map.tiles[y][x] = tile == TILE_CASTLE_FIRE_RUNE ? TILE_CASTLE_CARPET : TILE_CASTLE_FLOOR;
                }
                if (level == 5 && tile == TILE_CASTLE_BOOKCASE && y % 20 != 5) {
                    game.map.tiles[y][x] = TILE_CASTLE_FLOOR;
                }
            }
        }
        game.map.tiles[15][10] = TILE_CASTLE_FLOOR;
        for (int x = 14; x <= 16; x++) {
            game.map.tiles[21][x] = TILE_CASTLE_CARPET;
        }
        map_mark_explored(&game.map, 45, 31);
    }
    castle_enter(&game, 5);
    game.player.x = 40;
    game.player.y = 26;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 50, .y = 26,
        .underlying_tile = TILE_CASTLE_FLOOR, .item = item_make_magic_shield()};
    game.map.tiles[26][50] = TILE_ITEM;
    castle_store(&game);
    ASSERT("version 119 castle save migrates", save_legacy_layout(119) && load_game(&loaded, CASTLE_TEST_SLOT));
    ASSERT("migration preserves player, money, defeated guards, damage and miniboss progress",
        loaded.player.x == 40 && loaded.player.y == 26 && loaded.gold == 321 && loaded.castle_minibosses == 1 &&
        loaded.enemies[0].hp == 7 && !loaded.enemies[1].active && loaded.castle_cache[1].enemy_count == 8);
    ASSERT("migration installs distinct encounters without crushing player or loot",
        loaded.map.tiles[26][40] == TILE_CASTLE_FLOOR && loaded.map.tiles[26][50] == TILE_ITEM &&
        loaded.floor_items[0].underlying_tile == TILE_CASTLE_FLOOR && loaded.floor_items[0].item.block_chance == game.floor_items[0].item.block_chance &&
        loaded.map.tiles[34][45] == TILE_CASTLE_FIRE_RUNE && loaded.castle_cache[3].map.tiles[54][37] == TILE_CASTLE_WARD &&
        loaded.castle_cache[2].map.tiles[35][53] == TILE_CASTLE_MUSTER && loaded.castle_cache[5].enemy_count == 17 &&
        loaded.castle_cache[0].enemy_count == 8 && map_is_explored(&loaded.map, 45, 31));
    int count = loaded.enemy_count;
    ASSERT("resaving migrated castle does not add another guard detail", save_game(&loaded, CASTLE_TEST_SLOT) &&
        load_game(&game, CASTLE_TEST_SLOT) && game.enemy_count == count && game.castle_cache[5].enemy_count == 17);
    remove("saves/savegame_99136.json");
}

void test_castle(void) {
    printf("Castle of No Return\n");
    test_castle_layout();
    test_castle_progress();
    test_castle_fall_and_retreat();
    test_castle_combat();
    test_castle_no_seals();
    test_castle_removed_seal_migration();
    test_castle_saves();
    test_castle_class_victory();
    test_castle_wide_warning();
    test_castle_mechanisms();
    test_castle_layout_migration();
    test_castle_legacy_save();
    test_castle_floor_tactics();
    test_castle_tactics_saves();
}
