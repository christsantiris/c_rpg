#include "test_utils.h"
#include "game/game.h"
#include "systems/save_load.h"
#include "screens/quest_journal.h"
#include <string.h>

#define MINOTAUR_TEST_SLOT 99133
static GameState game;
static GameState loaded;

static int enter_vault(int runes) {
    game.player.player_class = CLASS_MAGE;
    game_init(&game);
    game.rook_quest_state = 1;
    game.rook_labyrinth_switches = runes;
    game_enter_labyrinth(&game);
    while (game.level < LABYRINTH_DEPTH) {
        game_change_labyrinth_floor(&game, 1, 0);
    }
    for (int i = 0; i < game.enemy_count; i++) {
        if (game.enemies[i].type == ENEMY_MINOTAUR) {
            return i;
        }
    }
    return -1;
}

static void test_vault_combat(void) {
    int index = enter_vault(15);
    ASSERT("the fifth labyrinth floor has a Minotaur guarding the sealed vault", index >= 0 && game.enemies[index].is_boss && game.enemies[index].x == 39 && game.map.tiles[game.map.stairs_up_y][35] == TILE_LABYRINTH_GATE);
    if (index < 0) {
        return;
    }
    for (int i = 0; i < game.enemy_count; i++) {
        if (i != index) {
            game.enemies[i].active = 0;
        }
    }
    game.player.x = 34;
    game.player.y = game.map.stairs_up_y;
    game.player.hp = 100;
    game.player.defense = 2;
    for (int turn = 0; turn < 5; turn++) {
        action_resolve_enemies(&game);
    }
    ASSERT("the closed rune gate prevents the Minotaur reaching the player", game.player.hp == 100 && game.enemies[index].x == 39);
    game.map.tiles[game.player.y][35] = TILE_LABYRINTH_FLOOR;
    game.player.x = 36;
    action_resolve_enemies(&game);
    ASSERT("opening the vault lets the Minotaur pursue along its corridor", game.enemies[index].x == 38 && game.player.hp == 100);
    action_resolve_enemies(&game);
    action_resolve_enemies(&game);
    ASSERT("the Minotaur attacks in melee using normal armor protection", game.enemies[index].x == 37 && game.player.hp == 84);
}

static void test_ranged_victories(void) {
    for (int attack = 0; attack < 3; attack++) {
        int index = enter_vault(31);
        if (index < 0) {
            ASSERT("a Minotaur is available for ranged combat", 0);
            return;
        }
        Enemy *boss = &game.enemies[index];
        boss->hp = 1;
        game.player.x = boss->x - 2;
        game.player.y = boss->y;
        game.player.last_dx = 1;
        game.player.last_dy = 0;
        game.player.mp = 100;
        game.inventory[0] = attack == 2 ? item_make_bow() : item_make_staff();
        game.player.arrows = MAX_ARROWS;
        game.inventory_count = 1;
        game.equipped_main_hand = 0;
        game.player.known_spells[0] = attack == 1 ? spell_make_fireball() : spell_make_magic_arrow();
        game.player.known_spell_count = 1;
        game.player.equipped_spell = 0;
        action_resolve_player(&game, (Action){attack == 2 ? ACTION_RANGED_ATTACK : ACTION_CAST_SPELL, 0, 0});
        int shields = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            shields += game.floor_items[i].active && game.floor_items[i].item.visual_id == ITEM_VISUAL_MAGIC_SHIELD;
        }
        BossJournalEntry entry;
        ASSERT("Magic Arrow, Fireball, and bow victories award the Minotaur's shield and journal completion", !boss->active && shields == 1 && quest_journal_get_boss(&game, 12, &entry) && entry.defeated && strcmp(entry.name, "Minotaur") == 0);
        game.player.x = 40;
        game_interact_labyrinth(&game);
        ASSERT("a ranged Minotaur victory releases Rook's ivory rook", game.rook_quest_state == 2);
    }
}

static void test_warden_save_migration(void) {
    int index = enter_vault(31);
    if (index < 0) {
        ASSERT("a legacy Warden save fixture can be created", 0);
        return;
    }
    Enemy *boss = &game.enemies[index];
    snprintf(boss->name, sizeof(boss->name), "Maze Warden");
    boss->hp = 57;
    boss->move_timer = 7;
    boss->frozen_turns = 1;
    game.gold = 37;
    game.score = 1234;
    game.player.hp = 89;
    map_mark_explored(&game.map, boss->x, boss->y);
    LevelCache *cache = &game.labyrinth_cache[LABYRINTH_DEPTH - 1];
    cache->valid = 1;
    cache->map = game.map;
    cache->enemy_count = game.enemy_count;
    memcpy(cache->enemies, game.enemies, sizeof(game.enemies));
    int ok = save_game(&game, MINOTAUR_TEST_SLOT) && load_game(&loaded, MINOTAUR_TEST_SLOT);
    ASSERT("legacy active and cached Wardens load as Minotaurs", ok && loaded.enemies[index].type == ENEMY_MINOTAUR && strcmp(loaded.enemies[index].name, "Minotaur") == 0 && loaded.labyrinth_cache[LABYRINTH_DEPTH - 1].valid && strcmp(loaded.labyrinth_cache[LABYRINTH_DEPTH - 1].enemies[index].name, "Minotaur") == 0);
    ASSERT("migration retains enemy health, combat stats, and turn state", ok && loaded.enemies[index].hp == 57 && loaded.enemies[index].max_hp == boss->max_hp && loaded.enemies[index].attack == boss->attack && loaded.enemies[index].defense == boss->defense && loaded.enemies[index].move_timer == 7 && loaded.enemies[index].frozen_turns == 1 && loaded.labyrinth_cache[LABYRINTH_DEPTH - 1].enemies[index].hp == 57);
    ASSERT("migration retains explored maps, runes, quest state, and character progress", ok && memcmp(&loaded.map, &game.map, sizeof(Map)) == 0 && memcmp(&loaded.labyrinth_cache[LABYRINTH_DEPTH - 1].map, &cache->map, sizeof(Map)) == 0 && loaded.rook_labyrinth_switches == 31 && loaded.rook_quest_state == 1 && loaded.gold == 37 && loaded.score == 1234 && loaded.player.hp == 89);
    ASSERT("migrated Minotaurs can be saved and loaded again", ok && save_game(&loaded, MINOTAUR_TEST_SLOT) && load_game(&loaded, MINOTAUR_TEST_SLOT) && loaded.enemies[index].hp == 57 && strcmp(loaded.enemies[index].name, "Minotaur") == 0);
    boss->active = 0;
    boss->hp = 0;
    cache->enemies[index] = *boss;
    game.defeated_bosses |= 1 << LOCATION_LABYRINTH;
    game.rook_quest_state = 3;
    game.rook_quest_completions = 1;
    ok = save_game(&game, MINOTAUR_TEST_SLOT) && load_game(&loaded, MINOTAUR_TEST_SLOT);
    BossJournalEntry entry;
    ASSERT("previous Warden victories retain the completed Minotaur journal and quest", ok && !loaded.enemies[index].active && !loaded.labyrinth_cache[LABYRINTH_DEPTH - 1].enemies[index].active && loaded.rook_quest_state == 3 && loaded.rook_quest_completions == 1 && quest_journal_get_boss(&loaded, 12, &entry) && entry.defeated);
    if (ok) {
        game_enter_labyrinth(&loaded);
        while (loaded.level < LABYRINTH_DEPTH) {
            game_change_labyrinth_floor(&loaded, 1, 0);
        }
    }
    int alive = 0;
    for (int i = 0; i < loaded.enemy_count; i++) {
        alive += loaded.enemies[i].active && loaded.enemies[i].type == ENEMY_MINOTAUR;
    }
    ASSERT("a completed legacy encounter does not respawn the Minotaur", ok && alive == 0);
    remove("saves/savegame_99133.json");
}

void test_minotaur(void) {
    printf("Minotaur tests:\n");
    int unused = !save_exists(MINOTAUR_TEST_SLOT);
    ASSERT("Minotaur temporary save slot is unused", unused);
    if (!unused) {
        return;
    }
    test_vault_combat();
    test_ranged_victories();
    test_warden_save_migration();
}
