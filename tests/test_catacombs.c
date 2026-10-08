#include "test_utils.h"
#include "game/catacombs.h"
#include "systems/save_load.h"
#include "screens/quest_journal.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define CATACOMBS_TEST_SLOT 99134
static GameState game;
static GameState loaded;
static unsigned char visited[MAP_H][MAP_W];
static int queue[MAP_W * MAP_H];

static void start_catacombs(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_MAGE;
    game_init(&game);
    game_enter_catacombs(&game);
}

static void arena(void) {
    start_catacombs();
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            game.map.tiles[y][x] = TILE_CATACOMBS_WALL;
        }
    }
    game.map.room_count = 1;
    game.map.rooms[0] = (Room){10, 10, 12, 12};
    for (int y = 10; y < 22; y++) {
        for (int x = 10; x < 22; x++) {
            game.map.tiles[y][x] = TILE_CATACOMBS_FLOOR;
        }
    }
    game.map.tiles[11][11] = TILE_OSSUARY_BRAZIER;
    game.map.burial_trap_count = 0;
    game.enemy_count = 0;
    game.floor_item_count = 0;
    game.player.x = 14;
    game.player.y = 15;
    game.player.attack = 1000;
    game.player.hp = 1000;
    game.player.max_hp = 1000;
    game.player.defense = 0;
    game.player.experience = 0;
    game.player.experience_next = 10000;
    game.inventory_count = 0;
    game.equipped_main_hand = -1;
    game.equipped_off_hand = -1;
    game.equipped_armor = -1;
}

static Enemy *skeleton(EnemyType type) {
    Enemy *e = &game.enemies[game.enemy_count++];
    *e = (Enemy){.active = 1, .x = 14, .y = 14, .type = type, .hp = 1, .max_hp = 48, .experience = 45, .attack_target_x = -1, .attack_target_y = -1};
    snprintf(e->name, sizeof(e->name), "%s", type == ENEMY_ANCIENT_SKELETON ? "Ancient Skeleton" : "Bone Sentinel");
    return e;
}

static void test_layouts(void) {
    int connected = 1;
    int safe = 1;
    int roster = 1;
    for (int seed = 0; seed < 12; seed++) {
        start_catacombs();
        srand((unsigned)seed);
        for (int level = 1; level <= CATACOMBS_DEPTH; level++) {
            game.level = level;
            map_generate_catacombs(&game.map, level);
            enemies_spawn(&game);
            memset(visited, 0, sizeof(visited));
            int head = 0;
            int tail = 1;
            queue[0] = game.map.stairs_up_y * MAP_W + game.map.stairs_up_x;
            visited[game.map.stairs_up_y][game.map.stairs_up_x] = 1;
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
            connected &= visited[game.map.stairs_down_y][game.map.stairs_down_x];
            int bosses = 0;
            int ancient = 0;
            int cantors = 0;
            for (int i = 0; i < game.enemy_count; i++) {
                Enemy *e = &game.enemies[i];
                connected &= visited[e->y][e->x];
                bosses += e->is_boss;
                ancient += e->type == ENEMY_ANCIENT_SKELETON;
                cantors += e->type == ENEMY_BONE_CANTOR;
                if (e->type == ENEMY_ANCIENT_SKELETON) {
                    roster &= e->max_hp > 10 && e->attack > 3 && e->defense > 0;
                }
                if (level == CATACOMBS_DEPTH && !e->is_boss) {
                    roster &= catacombs_room_at(&game.map, e->x, e->y) != game.map.room_count - 1;
                }
            }
            roster &= ancient > 0 && bosses == (level == CATACOMBS_DEPTH) && game.enemy_count <= AREA_ENEMY_LIMIT;
            roster &= (cantors > 0) == (level >= 3);
            for (int i = 0; i < game.map.burial_trap_count; i++) {
                BurialTrap *trap = &game.map.burial_traps[i];
                safe &= !catacombs_trap_marks(&game.map, trap, game.map.stairs_up_x, game.map.stairs_up_y);
                safe &= !catacombs_trap_marks(&game.map, trap, game.map.stairs_down_x, game.map.stairs_down_y);
            }
            safe &= catacombs_lit_braziers(&game.map, game.map.room_count - 1) == (level == CATACOMBS_DEPTH ? 2 : 1);
        }
    }
    ASSERT("all seeded catacomb floors connect both stairs and every enemy", connected);
    ASSERT("Ancient Skeletons are tougher, Cantors begin on floor three, and only floor five has a boss", roster);
    ASSERT("burial lines protect stairs and the final tomb contains two braziers", safe);
}

static void test_travel(void) {
    start_catacombs();
    game.location = LOCATION_CASTLE;
    map_generate_castle(&game.map, &game.player.x, &game.player.y);
    game.player.x = CROWNROAD_X;
    game.player.y = TOWN_H - 2;
    action_resolve_player(&game, (Action){ACTION_MOVE, CROWNROAD_X, TOWN_H - 1});
    ASSERT("the castle south road enters the Royal Catacombs", game.location == LOCATION_CATACOMBS && game.level == 1 && game.player.x == game.map.stairs_up_x && game.player.y == game.map.stairs_up_y);
    int bx = game.map.rooms[1].x + 1;
    int by = game.map.rooms[1].y + 1;
    game.map.tiles[by][bx] = TILE_OSSUARY_COLD;
    game.enemies[0].hp = 7;
    game.player.x = game.map.stairs_down_x;
    game.player.y = game.map.stairs_down_y;
    action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
    ASSERT("stairs descend while enemies remain alive", game.level == 2 && game.catacombs_cache[0].valid && !game.catacombs_cache[0].level_cleared);
    game.player.x = game.map.stairs_up_x;
    game.player.y = game.map.stairs_up_y;
    action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
    ASSERT("ascending restores enemy health and extinguished braziers", game.level == 1 && game.enemies[0].hp == 7 && game.map.tiles[by][bx] == TILE_OSSUARY_COLD);
    game.player.x = game.map.stairs_up_x;
    game.player.y = game.map.stairs_up_y;
    action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
    ASSERT("ordinary retreat emerges at the castle south gate", game.location == LOCATION_CASTLE && game.player.x == CROWNROAD_X && game.player.y == TOWN_H - 2);
    game_enter_catacombs(&game);
    ASSERT("re-entry preserves the same expedition", game.enemies[0].hp == 7 && game.map.tiles[by][bx] == TILE_OSSUARY_COLD);
    game_descend(&game);
    int x = game.player.x + 1;
    int y = game.player.y;
    game.player.x = x;
    game_open_town_portal(&game);
    ASSERT("Return to Town opens a catacomb portal beside Rosemoor's Crown Road gate", game.location == LOCATION_TOWN3 && game.portal_active && game.portal_location == LOCATION_CATACOMBS && game.map.tiles[TOWN3_KING_GATE_Y + 1][TOWN_W - 3] == TILE_PORTAL);
    int saved = save_game(&game, CATACOMBS_TEST_SLOT) && load_game(&loaded, CATACOMBS_TEST_SLOT);
    ASSERT("a Rosemoor portal survives saving and loading", saved && loaded.portal_active && loaded.catacombs_cache[1].valid);
    game = loaded;
    game.player.x = TOWN_W - 3;
    game.player.y = TOWN3_KING_GATE_Y;
    action_resolve_player(&game, (Action){ACTION_MOVE, TOWN_W - 3, TOWN3_KING_GATE_Y + 1});
    ASSERT("the saved portal restores the exact catacomb floor and position", game.location == LOCATION_CATACOMBS && game.level == 2 && game.player.x == x && game.player.y == y && !game.portal_active);
}

static void test_revival(void) {
    arena();
    Enemy *e = skeleton(ENEMY_ANCIENT_SKELETON);
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    ASSERT("Ancient Skeleton death awards experience and starts a visible resurrection warning", !e->active && e->revive_timer == 2 && game.player.experience == 45);
    action_resolve_enemies(&game);
    ASSERT("skeletons remain dead throughout their first warning phase", !e->active && e->revive_timer == 1 && !game.level_cleared);
    int saved = save_game(&game, CATACOMBS_TEST_SLOT) && load_game(&loaded, CATACOMBS_TEST_SLOT);
    ASSERT("a pending skeleton revival survives save/load", saved && loaded.enemies[0].revive_timer == 1 && !loaded.enemies[0].active);
    game = loaded;
    e = &game.enemies[0];
    int hp = game.player.hp;
    action_resolve_enemies(&game);
    ASSERT("skeletons revive once with full health and cannot attack on that phase", e->active && e->revived && e->hp == e->max_hp && e->experience == 0 && game.player.hp == hp);
    int xp = game.player.experience;
    int gold = game.gold;
    int items = game.floor_item_count;
    int score = game.score;
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("the second death grants no experience, gold, score, loot, or further revival", !e->active && e->revive_timer == 0 && game.player.experience == xp && game.gold == gold && game.floor_item_count == items && game.score == score);
    ASSERT("a spent revival survives save/load", save_game(&game, CATACOMBS_TEST_SLOT) && load_game(&loaded, CATACOMBS_TEST_SLOT) && loaded.enemies[0].revived && loaded.enemies[0].experience == 0);
    arena();
    e = skeleton(ENEMY_ANCIENT_SKELETON);
    action_resolve_player(&game, (Action){ACTION_MOVE, e->x, e->y});
    action_resolve_enemies(&game);
    game.player.x = 11;
    game.player.y = 12;
    ASSERT("an adjacent brazier exposes the interaction control", game_has_regional_interaction(&game));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_enemies(&game);
    ASSERT("extinguishing the brazier cancels a warned revival", !e->active && e->revive_timer == 0 && game.map.tiles[11][11] == TILE_OSSUARY_COLD);
    ASSERT("a cold brazier does not offer repeated interactions", !game_has_regional_interaction(&game));
    arena();
    e = skeleton(ENEMY_BONE_SENTINEL);
    e->active = 0;
    e->hp = 0;
    Enemy caster = {.x = 16, .y = 16, .type = ENEMY_BONE_CANTOR};
    ASSERT("a Cantor can warn a dead guard in its own lit chamber", catacombs_cantor_raise(&game, &caster) && e->revive_timer == 2);
    e->revived = 1;
    e->revive_timer = 0;
    ASSERT("a Cantor cannot grant a second resurrection", !catacombs_cantor_raise(&game, &caster));
    e->revived = 0;
    caster.x = 5;
    ASSERT("Cantors cannot resurrect guards in other chambers", !catacombs_cantor_raise(&game, &caster));
    caster.x = 16;
    game.map.tiles[11][11] = TILE_OSSUARY_COLD;
    ASSERT("extinguished braziers disable Cantor resurrection", !catacombs_cantor_raise(&game, &caster));
    arena();
    e = skeleton(ENEMY_ANCIENT_SKELETON);
    e->active = 0;
    e->revive_timer = 1;
    game.player.x = e->x;
    game.player.y = e->y;
    action_resolve_enemies(&game);
    ASSERT("a skeleton never respawns on the player", !e->active && e->revive_timer == 1);
    game.player.x++;
    action_resolve_enemies(&game);
    ASSERT("a blocked revival resumes when its corpse tile becomes free", e->active && e->revived);
}

static void test_traps(void) {
    arena();
    game.map.burial_trap_count = 1;
    BurialTrap *trap = &game.map.burial_traps[0];
    *trap = (BurialTrap){14, 15, 0, 0, 0};
    game.map.tiles[15][14] = TILE_BURIAL_PLATE;
    int hp = game.player.hp;
    action_resolve_enemies(&game);
    ASSERT("stepping on a burial plate warns without immediate damage", trap->timer == 1 && game.player.hp == hp);
    game.inventory[0] = item_make_health_potion();
    game.inventory_count = 1;
    action_resolve_player(&game, (Action){ACTION_DROP_ITEM, 0, 0});
    ASSERT("dropped items preserve visible burial plates and pending warnings", game.map.tiles[15][14] == TILE_BURIAL_PLATE && trap->timer == 1 && game.floor_items[0].underlying_tile == TILE_BURIAL_PLATE);
    action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("picking up a dropped item preserves the burial mechanism", game.map.tiles[15][14] == TILE_BURIAL_PLATE && trap->timer == 1 && game.inventory_count == 1);

    ASSERT("spike warnings stay on their marked line and stop at walls", catacombs_trap_marks(&game.map, trap, 17, 15) && !catacombs_trap_marks(&game.map, trap, 14, 16));
    game.map.tiles[15][16] = TILE_CATACOMBS_WALL;
    ASSERT("walls interrupt burial spike lines", !catacombs_trap_marks(&game.map, trap, 17, 15));
    game.map.tiles[15][16] = TILE_CATACOMBS_FLOOR;
    int saved = save_game(&game, CATACOMBS_TEST_SLOT) && load_game(&loaded, CATACOMBS_TEST_SLOT);
    ASSERT("armed burial traps survive save/load", saved && loaded.map.burial_trap_count == 1 && loaded.map.burial_traps[0].timer == 1);
    game = loaded;
    trap = &game.map.burial_traps[0];
    action_resolve_enemies(&game);
    ASSERT("remaining in the line takes damage on the following turn", game.player.hp < hp && trap->spent && trap->timer == 0);
    hp = game.player.hp;
    action_resolve_enemies(&game);
    ASSERT("spent burial traps cannot repeatedly damage the player", game.player.hp == hp);
    *trap = (BurialTrap){14, 15, 0, 0, 0};
    action_resolve_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_MOVE, 14, 16});
    action_resolve_enemies(&game);
    ASSERT("moving perpendicular to the warning avoids burial spikes", game.player.hp == hp && trap->spent);
    arena();
    game.map.burial_trap_count = 1;
    game.map.burial_traps[0] = (BurialTrap){14, 15, 0, 1, 0};
    hp = game.player.hp;
    game_open_town_portal(&game);
    game_use_town_portal(&game);
    action_resolve_enemies(&game);
    ASSERT("a saved trap warning cannot strike immediately on portal arrival", game.location == LOCATION_CATACOMBS && game.player.x == 14 && game.player.y == 15 && game.player.hp == hp && game.map.burial_traps[0].spent);

}

static void test_boss(void) {
    arena();
    Enemy *boss = skeleton(ENEMY_GRAVE_MARSHAL);
    boss->is_boss = 1;
    boss->hp = boss->max_hp = 300;
    boss->attack = 26;
    game.map.tiles[11][20] = TILE_OSSUARY_BRAZIER;
    ASSERT("two braziers halve damage without making the Marshal invulnerable", catacombs_enemy_damage(&game, boss, 100) == 50 && catacombs_enemy_damage(&game, boss, 1) == 1);
    game.map.tiles[11][11] = TILE_OSSUARY_COLD;
    ASSERT("one brazier reduces incoming damage by a quarter", catacombs_enemy_damage(&game, boss, 100) == 75);
    game.map.tiles[11][20] = TILE_OSSUARY_COLD;
    ASSERT("both extinguished braziers remove the Marshal's protection", catacombs_enemy_damage(&game, boss, 100) == 100);
    int hp = game.player.hp;
    action_resolve_enemies(&game);
    ASSERT("the Marshal marks a three-tile sweep before attacking", game.player.hp == hp && boss->attack_target_y == 15 && catacombs_sweep_marks(&game.map, boss, 13, 15) && catacombs_sweep_marks(&game.map, boss, 15, 15));
    int saved = save_game(&game, CATACOMBS_TEST_SLOT) && load_game(&loaded, CATACOMBS_TEST_SLOT);
    ASSERT("a Marshal windup and extinguished braziers survive save/load", saved && loaded.enemies[0].attack_target_y == 15 && loaded.map.tiles[11][11] == TILE_OSSUARY_COLD);
    game = loaded;
    boss = &game.enemies[0];
    action_resolve_player(&game, (Action){ACTION_MOVE, 14, 16});
    action_resolve_enemies(&game);
    ASSERT("the sweep strikes saved tiles rather than following the player", game.player.hp == hp && boss->attack_target_x == -1);
    int x = boss->x;
    int y = boss->y;
    action_resolve_enemies(&game);
    ASSERT("the Marshal pauses for a recovery turn after sweeping", boss->x == x && boss->y == y && game.player.hp == hp);
    game.player.y = boss->y + 1;
    action_resolve_enemies(&game);
    boss->frozen_turns = 2;
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("freezing the Marshal pauses its pending sweep without damage", game.player.hp == hp && boss->attack_target_y == game.player.y && boss->frozen_turns == 0);
    action_resolve_enemies(&game);
    ASSERT("standing in the sweep after its warning takes damage", game.player.hp < hp && boss->attack_target_x == -1);

    for (int method = 0; method < 4; method++) {
        start_catacombs();
        while (game.level < CATACOMBS_DEPTH) {
            game_descend(&game);
        }
        boss = &game.enemies[0];
        ASSERT("the Grave Marshal occupies the final royal tomb", boss->type == ENEMY_GRAVE_MARSHAL && boss->is_boss);
        game.player.x = game.map.stairs_down_x;
        game.player.y = game.map.stairs_down_y;
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("the living Marshal blocks the final return passage", game.location == LOCATION_CATACOMBS && game.level == CATACOMBS_DEPTH);
        boss->hp = 1;
        game.player.attack = 10000;
        game.player.mp = 1000;
        game.player.x = boss->x;
        game.player.y = boss->y + (method == 3 ? 2 : 1);
        game.player.last_dx = 0;
        game.player.last_dy = -1;
        if (method == 0) {
            action_resolve_player(&game, (Action){ACTION_MOVE, boss->x, boss->y});
        } else if (method == 3) {
            game.inventory[game.inventory_count] = item_make_magic_longbow();
            game.player.arrows = MAX_ARROWS;
            game.equipped_main_hand = game.inventory_count++;
            action_resolve_player(&game, (Action){ACTION_RANGED_ATTACK, 0, 0});
        } else {
            game.player.known_spells[0] = method == 1 ? spell_make_magic_arrow() : spell_make_fireball();
            game.player.known_spell_count = 1;
            game.player.equipped_spell = 0;
            action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
        }
        int rewards = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            rewards += strcmp(game.floor_items[i].item.name, "Gravekeeper's Mantle") == 0;
        }
        BossJournalEntry entry;
        ASSERT("melee, spells, and bows grant the mantle and record the Marshal's defeat", !boss->active && rewards == 1 && game.catacombs_mantle_unclaimed && quest_journal_get_boss(&game, 13, &entry) && entry.defeated);
        game.player.x = game.map.stairs_down_x;
        game.player.y = game.map.stairs_down_y;
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("boss victory returns to the castle with other enemies still alive", game.location == LOCATION_CASTLE && game.catacombs_cache[4].enemy_count > 1);
        game_enter_catacombs(&game);
        while (game.level < CATACOMBS_DEPTH) {
            game_descend(&game);
        }
        catacombs_restore_reward(&game);
        rewards = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            rewards += strcmp(game.floor_items[i].item.name, "Gravekeeper's Mantle") == 0;
        }
        ASSERT("an unclaimed mantle persists across retreat without duplicate drops", rewards == 1 && !game.enemies[0].active);
    }
    Item mantle = item_make_gravekeeper_mantle();
    ASSERT("the mantle gives defense and maximum health to every class", mantle.defense_bonus == 6 && mantle.max_hp_bonus == 20 && item_class_allowed(&mantle, CLASS_MAGE) && item_class_allowed(&mantle, CLASS_WARRIOR) && item_class_allowed(&mantle, CLASS_ROGUE));
    arena();
    int hp_before = game.player.max_hp;
    int def_before = game.player.defense;
    game.inventory[0] = mantle;
    game.inventory_count = 1;
    action_resolve_player(&game, (Action){ACTION_EQUIP_ITEM, 0, 0});
    ASSERT("equipping the mantle applies its health and defense bonuses", game.equipped_armor == 0 && game.player.max_hp == hp_before + 20 && game.player.defense == def_before + 6);
    ASSERT("equipped mantle bonuses survive save/load", save_game(&game, CATACOMBS_TEST_SLOT) && load_game(&loaded, CATACOMBS_TEST_SLOT) && loaded.inventory[0].max_hp_bonus == 20 && loaded.player.max_hp == game.player.max_hp && loaded.player.defense == game.player.defense);

}

static void test_migration(void) {
    start_catacombs();
    game_leave_catacombs(&game);
    game.player.hp = 57;
    game.gold = 111;
    game.score = 4567;
    game.defeated_bosses = 1 << LOCATION_FOREST;
    game.map.tiles[TOWN_H - 1][CROWNROAD_X] = TILE_WALL;
    int saved = save_game(&game, CATACOMBS_TEST_SLOT);
    FILE *file = fopen("saves/savegame_99134.json", "rb");
    if (!saved || !file) {
        ASSERT("legacy catacomb migration fixture can be written", 0);
        return;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = malloc((size_t)size + 1);
    size_t read = fread(text, 1, (size_t)size, file);
    text[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    free(text);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 82);
    cJSON_DeleteItemFromObject(root, "catacombs_cache");
    cJSON_DeleteItemFromObject(root, "max_catacombs_level_reached");
    cJSON_DeleteItemFromObject(root, "catacombs_mantle_unclaimed");
    char *json = cJSON_PrintUnformatted(root);
    file = fopen("saves/savegame_99134.json", "wb");
    if (file) {
        fwrite(json, 1, strlen(json), file);
        fclose(file);
    }
    free(json);
    cJSON_Delete(root);
    int ok = load_game(&loaded, CATACOMBS_TEST_SLOT);
    ASSERT("version 82 saves load and open the castle's south gate", ok && loaded.location == LOCATION_CASTLE && loaded.map.tiles[TOWN_H - 1][CROWNROAD_X] == TILE_TOWN_EXIT && loaded.max_catacombs_level_reached == 1);
    ASSERT("migration preserves character, money, score, boss victories, and previous regional caches", ok && loaded.player.hp == 57 && loaded.gold == 111 && loaded.score == 4567 && loaded.defeated_bosses == (1 << LOCATION_FOREST) && loaded.inventory_count == game.inventory_count && loaded.level_cache[0].valid == game.level_cache[0].valid);
    int empty = 1;
    for (int i = 0; i < CATACOMBS_DEPTH; i++) {
        empty &= !loaded.catacombs_cache[i].valid;
    }
    ASSERT("migration adds an untouched catacomb cache and no unearned mantle", empty && !loaded.catacombs_mantle_unclaimed);
    ASSERT("migrated saves rewrite and reload successfully", save_game(&loaded, CATACOMBS_TEST_SLOT) && load_game(&game, CATACOMBS_TEST_SLOT));
}

void test_catacombs(void) {
    printf("Royal Catacombs:\n");
    int unused = !save_exists(CATACOMBS_TEST_SLOT);
    ASSERT("the catacomb test slot does not overwrite a save", unused);
    if (!unused) {
        return;
    }
    test_layouts();
    test_travel();
    test_revival();
    test_traps();
    test_boss();
    test_migration();
    remove("saves/savegame_99134.json");
}
