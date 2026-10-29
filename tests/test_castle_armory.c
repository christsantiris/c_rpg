#include "test_utils.h"
#include "game/castle.h"
#include "systems/save_load.h"
#include <string.h>
#include <stdlib.h>

#define ARMORY_SLOT 99201
static GameState *game;
static GameState *loaded;

static Enemy *find_boss(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == ENEMY_VAMPIRE && g->enemies[i].is_boss) {
            return &g->enemies[i];
        }
    }
    return NULL;
}

static Enemy *setup(void) {
    memset(game, 0, sizeof(*game));
    game->player.player_class = CLASS_WARRIOR;
    game_init(game);
    castle_enter(game, 2);
    Enemy *e = find_boss(game);
    for (int i = 0; i < game->enemy_count; i++) {
        game->enemies[i].active = &game->enemies[i] == e;
    }
    game->player.x = 45;
    game->player.y = 46;
    game->player.hp = 1000;
    game->player.max_hp = 1000;
    game->player.defense = 0;
    game->player.experience_next = 100000;
    return e;
}

static void test_armory_reward(void) {
    Item shield = item_make_castellan_bulwark();
    Item magic = item_make_magic_shield();
    ASSERT("Bulwark trades one defense for five block chance against Magic Shield", shield.defense_bonus == 8 && shield.block_chance == 30 && shield.block_reduction_percent == 50 &&
        shield.defense_bonus == magic.defense_bonus - 1 && shield.block_chance == magic.block_chance + 5);
    ASSERT("Bulwark is a rare Warrior shield with its own saved visual", shield.type == ITEM_SHIELD && shield.rarity == ITEM_RARITY_RARE &&
        item_class_allowed(&shield, CLASS_WARRIOR) && !item_class_allowed(&shield, CLASS_MAGE) && !item_class_allowed(&shield, CLASS_ROGUE) && shield.visual_id == ITEM_VISUAL_CASTELLAN_BULWARK);
    for (int method = 0; method < 5; method++) {
        Enemy *e = setup();
        e->hp = 1;
        game->player.x = e->x;
        game->player.y = e->y + 1;
        game->player.last_dx = 0;
        game->player.last_dy = -1;
        game->player.attack = 1000;
        int gold = game->gold;
        if (method == 0) {
            action_resolve_player(game, (Action){ACTION_MOVE, e->x, e->y});
        } else if (method == 1) {
            game->player.player_class = CLASS_ROGUE;
            game->player.y = e->y + 2;
            game->inventory[0] = item_make_bow();
            game->inventory_count = 1;
            game->equipped_main_hand = 0;
            game->player.arrows = MAX_ARROWS;
            action_resolve_player(game, (Action){ACTION_RANGED_ATTACK, 0, 0});
        } else {
            game->player.player_class = CLASS_MAGE;
            game->player.known_spells[0] = method == 2 ? spell_make_magic_arrow() : method == 3 ? spell_make_fireball() : spell_make_frost_bolt();
            game->player.known_spell_count = 1;
            game->player.equipped_spell = 0;
            game->player.mp = 100;
            action_resolve_player(game, (Action){ACTION_CAST_SPELL, 0, 0});
        }
        ASSERT("melee, arrows and every damage spell award Bulwark plus the existing gold/stair unlock", !e->active && game->castellan_reward_state == 1 && game->gold == gold + 100 && (game->castle_minibosses & 1) && !game->game_won);
        castle_record_death(game, e);
        ASSERT("recording a repeated death cannot duplicate rewards", game->gold == gold + 100 && game->castellan_reward_state == 1);
    }
    game->inventory_count = MAX_INVENTORY;
    game->floor_item_count = MAX_FLOOR_ITEMS;
    game->player.x = 53;
    game->player.y = 54;
    ASSERT("full inventory and floor loot leave the guaranteed shield reserved", castle_interact(game) && game->castellan_reward_state == 1 && game->inventory_count == MAX_INVENTORY);
    ASSERT("unclaimed armory reservation survives save/load with maximum loot", save_game(game, ARMORY_SLOT) && load_game(loaded, ARMORY_SLOT) && loaded->castellan_reward_state == 1 && loaded->floor_item_count == MAX_FLOOR_ITEMS);
    castle_leave(game, 0);
    castle_enter(game, 2);
    game->player.x = 53;
    game->player.y = 54;
    game->inventory_count = 0;
    ASSERT("unclaimed Bulwark survives retreat and is collectible by every class", castle_interact(game) && game->castellan_reward_state == 2 && game->inventory_count == 1 && strcmp(game->inventory[0].name, "Castellan's Bulwark") == 0);
    ASSERT("claiming the armory cannot create a second shield", !castle_interact(game) && game->inventory_count == 1);
    ASSERT("claimed shield metadata and progress survive save/load", save_game(game, ARMORY_SLOT) && load_game(loaded, ARMORY_SLOT) && loaded->castellan_reward_state == 2 && loaded->inventory[0].visual_id == shield.visual_id && loaded->inventory[0].block_chance == 30);
}

void test_castle_armory(void) {
    printf("Castle armory Vampire and preserved equipment reward\n");
    game = calloc(1, sizeof(GameState));
    loaded = calloc(1, sizeof(GameState));
    ASSERT("armory fixtures allocate", game && loaded);
    if (!game || !loaded) {
        free(game);
        free(loaded);
        return;
    }
    test_armory_reward();
    free(game);
    free(loaded);
    remove("saves/savegame_99201.json");
}
