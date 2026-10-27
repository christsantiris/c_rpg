#include "test_utils.h"
#include "game/crown_quest.h"
#include "game/giants_crown.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define CROWN_SLOT 99288

static int route(GameState *g, int tx, int ty, int walk) {
    int previous[GIANTS_H][GIANTS_W];
    int queue[GIANTS_H * GIANTS_W];
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    memset(previous, -1, sizeof(previous));
    int start = g->player.y * GIANTS_W + g->player.x;
    int head = 0;
    int tail = 1;
    queue[0] = start;
    previous[g->player.y][g->player.x] = start;
    while (head < tail && previous[ty][tx] < 0) {
        int cell = queue[head++];
        for (int n = 0; n < 4; n++) {
            int x = cell % GIANTS_W + dx[n];
            int y = cell / GIANTS_W + dy[n];
            if (x < 0 || y < 0 || x >= GIANTS_W || y >= GIANTS_H || previous[y][x] >= 0 || !map_is_walkable(&g->map, x, y)) {
                continue;
            }
            previous[y][x] = cell;
            queue[tail++] = y * GIANTS_W + x;
        }
    }
    if (previous[ty][tx] < 0) {
        return 0;
    }
    int count = 0;
    for (int cell = ty * GIANTS_W + tx; cell != start; cell = previous[cell / GIANTS_W][cell % GIANTS_W]) {
        queue[count++] = cell;
    }
    if (walk) {
        for (int n = count - 1; n >= 0; n--) {
            action_resolve_player(g, (Action){ACTION_MOVE, queue[n] % GIANTS_W, queue[n] / GIANTS_W});
            if (g->player.x != queue[n] % GIANTS_W || g->player.y != queue[n] / GIANTS_W) {
                return 0;
            }
        }
    }
    return 1;
}

static void clear_ordinary(GameState *g) {
    for (int n = 0; n < g->enemy_count; n++) {
        if (!g->enemies[n].is_boss) {
            g->enemies[n].active = 0;
        }
    }
}

static void edda(GameState *g) {
    game_enter_town3(g);
    game_enter_guild(g);
    g->player.x = EDDA_X;
    g->player.y = EDDA_Y + 1;
    crown_quest_talk(g);
}

static void to_stage(GameState *g, int stage) {
    while (g->level != stage) {
        // Simulate boundary travel so a preceding chamber position is not reused.
        g->player.x = 32;
        g->player.y = g->level < stage ? 0 : GIANTS_H - 1;
        giants_change_level(g, g->level < stage);
    }
    clear_ordinary(g);
}

static int item_count(const GameState *g, const char *name) {
    int count = 0;
    for (int n = 0; n < g->inventory_count; n++) {
        count += strcmp(g->inventory[n].name, name) == 0;
    }
    return count;
}

static void fill_pack(GameState *g) {
    while (g->inventory_count < MAX_INVENTORY) {
        g->inventory[g->inventory_count++] = item_make_health_potion();
    }
}

static int rewrite(const char *field, double value, int old) {
    char path[80];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", CROWN_SLOT);
    FILE *file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = calloc((size_t)size + 1, 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    fread(data, 1, size, file);
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    if (old) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 207);
        const char *fields[] = {"crown_quest_state", "crown_quest_progress", "crown_vault_open", "crown_reward_state"};
        for (int n = 0; n < 4; n++) {
            cJSON_DeleteItemFromObject(root, fields[n]);
        }
    } else {
        cJSON_ReplaceItemInObject(root, field, cJSON_CreateNumber(value));
    }
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(path, "wb");
    int ok = data && file && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

static void shield_checks(GameState *g, GameState *loaded) {
    int shield = g->inventory_count - 1;
    g->inventory[0] = item_make_long_sword();
    g->inventory[1] = item_make_magic_shield();
    g->player.player_class = CLASS_WARRIOR;
    game_equip_main_hand(g, 0);
    int base_hp = g->player.max_hp;
    int base_defense = g->player.defense;
    int hp = base_hp - 10;
    g->player.hp = hp;
    ASSERT("Stoneguard has the approved defense, block and health bonuses", g->inventory[shield].type == ITEM_SHIELD &&
        g->inventory[shield].defense_bonus == 8 && g->inventory[shield].block_chance == 20 && g->inventory[shield].max_hp_bonus == 30);
    ASSERT("equipping Stoneguard increases maximum HP without healing", game_equip_shield(g, shield) && g->player.max_hp == base_hp + 30 &&
        g->player.hp == hp && g->player.defense == base_defense + 8);
    ASSERT("re-equipping the same shield never stacks its bonuses", game_equip_shield(g, shield) && g->player.max_hp == base_hp + 30 && g->player.hp == hp && g->player.defense == base_defense + 8);
    g->player.hp = base_hp + 20;
    ASSERT("swapping shields removes health and defense exactly once", game_equip_shield(g, 1) && g->player.max_hp == base_hp && g->player.hp == base_hp && g->player.defense == base_defense + 9);
    game_equip_shield(g, shield);
    g->player.hp = base_hp + 20;
    ASSERT("an equipped Stoneguard survives saving without doubling HP", save_game(g, CROWN_SLOT) && load_game(loaded, CROWN_SLOT) &&
        loaded->equipped_off_hand == shield && loaded->player.max_hp == base_hp + 30 && loaded->player.hp == base_hp + 20 && loaded->inventory[shield].max_hp_bonus == 30);
    game_unequip_off_hand(loaded);
    ASSERT("removing a loaded shield restores base HP and clamps current HP", loaded->player.max_hp == base_hp && loaded->player.hp == base_hp && loaded->player.defense == base_defense);
    game_remove_inventory_item(g, shield);
    int count = g->inventory_count;
    crown_quest_talk(g);
    ASSERT("dropping a claimed reward removes its bonuses and never recreates it", g->player.max_hp == base_hp && g->player.defense == base_defense &&
        g->inventory_count == count && !item_count(g, "Stoneguard Shield") && g->crown_reward_state == 2);
}

void test_crown_quest(void) {
    ASSERT("Giant's Crown fixture slot is unused", !save_exists(CROWN_SLOT));
    if (save_exists(CROWN_SLOT)) {
        return;
    }
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    Map *before = malloc(sizeof(*before));
    ASSERT("allocate Giant's Crown fixtures", g && loaded && before);
    if (!g || !loaded || !before) {
        free(g);
        free(loaded);
        free(before);
        return;
    }
    game_init(g);
    ASSERT("Edda cannot assign her quest remotely", !crown_quest_talk(g));
    edda(g);
    ASSERT("Edda occupies an accessible Guild floor without replacing Orin or Selene", g->map.tiles[EDDA_Y][EDDA_X] == TILE_NPC_EDDA &&
        map_is_walkable(&g->map, EDDA_X, EDDA_Y + 1) && g->map.tiles[GUILD_ORIN_Y][GUILD_ORIN_X] == TILE_NPC_ORIN && g->map.tiles[GUILD_SELENE_Y][GUILD_SELENE_X] == TILE_NPC_SELENE);
    ASSERT("Edda offers a complete readable quest with normal acceptance", game_quest_offer_active(g) && strlen(g->dialogue_text) < MAX_DIALOGUE_LEN - 1 && strstr(g->dialogue_text, "Stoneguard Shield."));
    ASSERT("an unanswered Edda offer survives saving", save_game(g, CROWN_SLOT) && load_game(loaded, CROWN_SLOT) && game_quest_offer_active(loaded) && !loaded->crown_quest_state);
    game_handle_quest_offer_key(g, SDL_SCANCODE_N, 0);
    ASSERT("declining leaves the quest unaccepted", !g->crown_quest_state);
    crown_quest_talk(g);
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("accepting starts Giant's Crown", g->crown_quest_state == 1 && !g->crown_quest_progress);
    giants_enter(g, 0);
    to_stage(g, 2);
    ASSERT("the optional village inscription is reachable on foot", route(g, CROWN_INSCRIPTION_X, CROWN_INSCRIPTION_Y, 1));
    giants_interact(g);
    ASSERT("the inscription records the western seal chamber's directions", g->crown_quest_progress == CROWN_INSCRIPTION);
    to_stage(g, 3);
    ASSERT("the royal seal begins behind cracked masonry", !route(g, CROWN_RELIC_X, CROWN_RELIC_Y, 0) && route(g, 9, 23, 1));
    Enemy giant;
    game_spawn_enemy(g, &giant, ENEMY_STONE_GIANT, 10, 24);
    int damage = 0;
    giants_tick(g, &giant, &damage);
    ASSERT("a nearby giant warns a strike that includes the cracked doorway", giant.attack_phase && giant.move_timer == 2 && giants_attack_marks(&giant, CROWN_DOOR_X, CROWN_DOOR_Y));
    g->player.y = 25;
    giants_tick(g, &giant, &damage);
    giants_tick(g, &giant, &damage);
    ASSERT("baiting the warned strike opens the seal chamber safely", !damage && g->map.tiles[CROWN_DOOR_Y][CROWN_DOOR_X] == TILE_GIANTS_PAVING && route(g, 6, 23, 1));
    fill_pack(g);
    giants_interact(g);
    ASSERT("the royal seal uses quest discoveries outside a full pack", g->crown_quest_progress == (CROWN_INSCRIPTION | CROWN_SEAL) && g->inventory_count == MAX_INVENTORY);
    g->player.x = 5;
    g->player.y = 24;
    giants_interact(g);
    ASSERT("the ordinary coffer remains separate from the seal pedestal", g->map.tiles[23][4] == TILE_GIANTS_CACHE);
    g->player.x = 4;
    g->player.y = 24;
    giants_interact(g);
    ASSERT("the seal chamber's ordinary coffer still pays normally", g->map.tiles[23][4] == TILE_GIANTS_EMPTY && (g->crown_quest_progress & CROWN_SEAL));
    ASSERT("seal discovery and opened masonry survive a save", save_game(g, CROWN_SLOT) && load_game(loaded, CROWN_SLOT) && loaded->crown_quest_progress == 3 && loaded->map.tiles[23][8] == TILE_GIANTS_PAVING);
    to_stage(g, 5);
    ASSERT("the new royal vault is warded without removing normal paths", !route(g, CROWN_RELIC_X, CROWN_RELIC_Y, 0) && route(g, 9, 23, 1) && !map_is_walkable(&g->map, 8, 23));
    giants_interact(g);
    ASSERT("a royal seal alone cannot open the living Jarl's ward", !g->crown_vault_open && !map_is_walkable(&g->map, 8, 23));
    game_spawn_enemy(g, &giant, ENEMY_STONE_GIANT, 10, 24);
    giants_tick(g, &giant, &damage);
    g->player.y = 25;
    giants_tick(g, &giant, &damage);
    giants_tick(g, &giant, &damage);
    ASSERT("a heavy strike cannot bypass the royal ward", !map_is_walkable(&g->map, 8, 23) && !g->crown_vault_open);
    Enemy *boss = &g->enemies[0];
    g->player.x = boss->x - 1;
    g->player.y = boss->y;
    g->player.attack = 10000;
    action_resolve_player(g, (Action){ACTION_MOVE, boss->x, boss->y});
    ASSERT("ordinary combat credits the Jarl objective without granting the crown", g->giants_crown_defeated && (g->crown_quest_progress & CROWN_JARL) && !(g->crown_quest_progress & CROWN_TAKEN));
    route(g, 9, 23, 1);
    giants_interact(g);
    ASSERT("seal and victory open the vault with a clear return route", g->crown_vault_open && map_is_walkable(&g->map, 8, 23) && route(g, 6, 23, 1));
    giants_interact(g);
    ASSERT("the crown is recoverable with a full inventory", g->crown_quest_state == 2 && g->crown_quest_progress == 15 && g->inventory_count == MAX_INVENTORY);
    QuestJournalEntry entry;
    ASSERT("the journal shows all four approved objectives and rewards", quest_journal_get_entry(g, QUEST_TAB_ACTIVE, 0, &entry) &&
        strcmp(entry.title, "Giant's Crown") == 0 && entry.objective_count == 4 && entry.objective_complete[0] && entry.objective_complete[1] &&
        entry.objective_complete[2] && entry.objective_complete[3] && entry.reward_gold == 220 && entry.reward_score == 1600 && strcmp(entry.reward_item, "Stoneguard Shield") == 0);
    game_open_town_portal(g);
    ASSERT("return portal preserves the recovered crown and vault", g->portal_active && g->portal_location == LOCATION_GIANTS_CROWN && g->portal_level == 5 && g->crown_quest_progress == 15);
    ASSERT("a portal to the vault survives saving", save_game(g, CROWN_SLOT) && load_game(loaded, CROWN_SLOT) && loaded->portal_active && loaded->crown_vault_open && loaded->crown_quest_state == 2);
    game_use_town_portal(g);
    ASSERT("the portal returns inside the same open vault", g->location == LOCATION_GIANTS_CROWN && g->level == 5 && map_is_walkable(&g->map, 8, 23) && g->crown_quest_progress == 15);
    to_stage(g, 4);
    to_stage(g, 5);
    ASSERT("backtracking keeps the vault open and its crown claimed", g->crown_vault_open && g->crown_quest_progress == 15 && map_is_walkable(&g->map, 8, 23));
    giants_leave(g);
    giants_enter(g, 0);
    to_stage(g, 5);
    ASSERT("fresh expeditions preserve quest discoveries and still respawn ordinary enemies", g->enemy_count > 0 && g->crown_quest_progress == 15 && map_is_walkable(&g->map, 8, 23));
    int gold = g->gold;
    int score = g->score;
    edda(g);
    crown_quest_talk(g);
    ASSERT("Edda pays once and reserves the shield while the pack is full", g->crown_quest_state == 3 && g->crown_reward_state == 1 &&
        g->gold == gold + 220 && g->score == score + 1600 && !item_count(g, "Stoneguard Shield"));
    ASSERT("the reserved reward survives save/load", save_game(g, CROWN_SLOT) && load_game(loaded, CROWN_SLOT) && loaded->crown_reward_state == 1 && loaded->crown_quest_state == 3);
    game_remove_inventory_item(g, g->inventory_count - 1);
    crown_quest_talk(g);
    crown_quest_talk(g);
    ASSERT("making room delivers exactly one shield without another payout", g->crown_reward_state == 2 && item_count(g, "Stoneguard Shield") == 1 && g->gold == gold + 220 && g->score == score + 1600);
    shield_checks(g, loaded);
    ASSERT("fractional quest states are rejected", save_game(g, CROWN_SLOT) && rewrite("crown_quest_state", 1.5, 0) && !load_game(loaded, CROWN_SLOT));
    ASSERT("invalid crown progress is rejected", save_game(g, CROWN_SLOT) && rewrite("crown_quest_progress", 16, 0) && !load_game(loaded, CROWN_SLOT));
    ASSERT("a claimed crown cannot have a closed quest vault", save_game(g, CROWN_SLOT) && rewrite("crown_vault_open", 0, 0) && !load_game(loaded, CROWN_SLOT));
    ASSERT("rewards cannot become unearned after completion", save_game(g, CROWN_SLOT) && rewrite("crown_reward_state", 0, 0) && !load_game(loaded, CROWN_SLOT));
    ASSERT("royal proof requires the recorded Jarl victory", save_game(g, CROWN_SLOT) && rewrite("giants_crown_defeated", 0, 0) && !load_game(loaded, CROWN_SLOT));
    // Old saves keep their existing open room, player, wounds and dropped gear.
    game_init(g);
    game_enter_town3(g);
    giants_enter(g, 0);
    to_stage(g, 5);
    g->giants_crown_defeated = 1;
    g->enemies[0].active = 0;
    g->enemies[0].hp = 0;
    g->enemies[1].hp = 7;
    g->map.tiles[23][8] = TILE_GIANTS_PAVING;
    g->map.tiles[23][4] = TILE_GIANTS_EMPTY;
    g->player.x = 5;
    g->player.y = 24;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 6, .y = 24, .underlying_tile = TILE_GIANTS_PAVING, .item = item_make_staff()};
    g->map.tiles[24][6] = TILE_ITEM;
    map_mark_explored(&g->map, 5, 23);
    g->gold = 4321;
    g->spice_skiff_quest_state = 3;
    g->spice_skiff_progress = 3;
    *before = g->map;
    ASSERT("old completed regions upgrade without resetting the map or progress", save_game(g, CROWN_SLOT) && rewrite(NULL, 0, 1) && load_game(loaded, CROWN_SLOT) &&
        loaded->player.x == 5 && loaded->player.y == 24 && loaded->enemies[1].hp == 7 && loaded->floor_items[0].active && loaded->gold == 4321 &&
        loaded->spice_skiff_quest_state == 3 && loaded->giants_crown_defeated && loaded->crown_quest_progress == CROWN_JARL &&
        memcmp(before->tiles, loaded->map.tiles, sizeof(before->tiles)) == 0);
    crown_quest_interact(loaded);
    ASSERT("an old opened room cannot bypass the royal seal requirement", !(loaded->crown_quest_progress & CROWN_TAKEN));
    edda(loaded);
    game_handle_quest_offer_key(loaded, SDL_SCANCODE_Y, 0);
    ASSERT("acceptance credits a previous Jarl victory", loaded->crown_quest_state == 1 && loaded->crown_quest_progress == CROWN_JARL);
    // A portal into a legacy vault keeps its escape route and exact destination.
    game_open_town_portal(g);
    ASSERT("a legacy portal into the vault upgrades without losing its destination", save_game(g, CROWN_SLOT) && rewrite(NULL, 0, 1) && load_game(loaded, CROWN_SLOT) && loaded->portal_active && loaded->portal_x == 5 && loaded->portal_y == 24);
    game_use_town_portal(loaded);
    ASSERT("returning through an old portal never traps the player", loaded->location == LOCATION_GIANTS_CROWN && loaded->player.x == 5 && loaded->player.y == 24 && map_is_walkable(&loaded->map, 8, 23));
    // Every class can use Interact after nearby giants have been killed.
    game_init(g);
    giants_enter(g, 0);
    to_stage(g, 3);
    route(g, 9, 23, 1);
    ASSERT("Interact provides the alternate seal-chamber opening", giants_interact(g) && route(g, 6, 23, 1));
    giants_interact(g);
    ASSERT("the seal can be discovered before accepting Edda's quest", !g->crown_quest_state && (g->crown_quest_progress & CROWN_SEAL));
    to_stage(g, 2);
    route(g, CROWN_INSCRIPTION_X, CROWN_INSCRIPTION_Y, 1);
    giants_interact(g);
    to_stage(g, 5);
    boss = &g->enemies[0];
    g->player.x = boss->x - 1;
    g->player.y = boss->y;
    g->player.attack = 10000;
    action_resolve_player(g, (Action){ACTION_MOVE, boss->x, boss->y});
    route(g, 9, 23, 1);
    giants_interact(g);
    route(g, 6, 23, 1);
    giants_interact(g);
    edda(g);
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    ASSERT("all discoveries made before acceptance retain credit", g->crown_quest_state == 2 && g->crown_quest_progress == 15);
    game_init(g);
    giants_enter(g, 0);
    to_stage(g, 5);
    g->debug_passages_unlocked = 1;
    game_refresh_debug_passages(g);
    ASSERT("debug shortcuts open the royal passage without granting victory or quest artifacts", map_is_walkable(&g->map, 8, 23) && !g->giants_crown_defeated && !g->crown_quest_progress && !g->crown_vault_open);
    // The resident migration leaves old floor loot and occupied tiles untouched.
    game_init(g);
    game_enter_town3(g);
    game_enter_guild(g);
    g->map.tiles[EDDA_Y][EDDA_X] = TILE_TAVERN_FLOOR;
    ASSERT("old Guild saves gain Edda without replacing existing residents", save_game(g, CROWN_SLOT) && rewrite(NULL, 0, 1) && load_game(loaded, CROWN_SLOT) && loaded->map.tiles[EDDA_Y][EDDA_X] == TILE_NPC_EDDA && loaded->map.tiles[GUILD_ORIN_Y][GUILD_ORIN_X] == TILE_NPC_ORIN);
    g->player.x = EDDA_X;
    g->player.y = EDDA_Y;
    ASSERT("resident migration preserves a player occupying Edda's future tile", save_game(g, CROWN_SLOT) && rewrite(NULL, 0, 1) && load_game(loaded, CROWN_SLOT) && loaded->player.x == EDDA_X && loaded->player.y == EDDA_Y && loaded->map.tiles[EDDA_Y][EDDA_X] == TILE_TAVERN_FLOOR);
    remove("saves/savegame_99288.json");
    free(before);
    free(loaded);
    free(g);
}
