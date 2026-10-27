#include "test_utils.h"
#include "game/game.h"
#include "game/castle.h"
#include "game/giants_crown.h"
#include "game/crownroad_north.h"
#include "game/ironfang.h"
#include "game/stormwatch.h"
#include "game/mangrove.h"
#include "game/elderwood.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define ESCAPE_SLOT 99291

static int portals(const Map *map) {
    int count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            count += map->tiles[y][x] == TILE_PORTAL;
        }
    }
    return count;
}

static void enter(GameState *g, Location area) {
    switch (area) {
        case LOCATION_DUNGEON: game_enter_dungeon(g); break;
        case LOCATION_FOREST: game_enter_forest(g); break;
        case LOCATION_MOUNTAINS: game_enter_mountains(g); break;
        case LOCATION_COAST: game_enter_coast(g); break;
        case LOCATION_SWAMP: game_enter_swamp(g); break;
        case LOCATION_FROSTFELL: game_enter_frostfell(g); break;
        case LOCATION_DRAGONSPINE: game_enter_dragonspine(g); break;
        case LOCATION_DESERT: game_enter_desert(g); break;
        case LOCATION_ASHEN: game_enter_ashen(g); break;
        case LOCATION_GLASSDEEP: game_enter_glassdeep(g); break;
        case LOCATION_CATACOMBS: game_enter_catacombs(g); break;
        case LOCATION_TEMPLE: game_enter_temple(g); break;
        case LOCATION_LABYRINTH: game_enter_labyrinth(g); break;
        case LOCATION_ELDERWOOD: elderwood_enter(g); break;
        case LOCATION_GIANTS_CROWN: giants_enter(g, 0); break;
        case LOCATION_CROWNROAD_NORTH: northroad_enter(g, 1, 0); break;
        case LOCATION_IRONFANG: ironfang_enter(g, 0); break;
        case LOCATION_STORMWATCH: stormwatch_enter(g, 0); break;
        case LOCATION_MANGROVES: mangrove_enter(g, 0); break;
        case LOCATION_ISLAND: game_enter_island(g); break;
        default: break;
    }
}

static int old_save(int slot) {
    char path[80];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", slot);
    FILE *file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = calloc((size_t)size + 1, 1);
    fread(text, 1, (size_t)size, file);
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 209);
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(path, "wb");
    int ok = file && text && fputs(text, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(text);
    return ok;
}

void test_town_escapes(void) {
    ASSERT("one-way escape save slot is unused", !save_exists(ESCAPE_SLOT));
    if (save_exists(ESCAPE_SLOT)) {
        return;
    }
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("one-way escape fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    const Location towns[] = {LOCATION_TOWN, LOCATION_TOWN2, LOCATION_TOWN3, LOCATION_TOWN4, LOCATION_DAWNMERE};
    const Location areas[] = {LOCATION_DUNGEON, LOCATION_FOREST, LOCATION_MOUNTAINS, LOCATION_COAST, LOCATION_SWAMP,
        LOCATION_FROSTFELL, LOCATION_DRAGONSPINE, LOCATION_DESERT, LOCATION_ASHEN, LOCATION_GLASSDEEP,
        LOCATION_CATACOMBS, LOCATION_TEMPLE, LOCATION_LABYRINTH, LOCATION_ELDERWOOD, LOCATION_GIANTS_CROWN,
        LOCATION_CROWNROAD_NORTH, LOCATION_IRONFANG, LOCATION_STORMWATCH, LOCATION_MANGROVES, LOCATION_ISLAND};
    for (unsigned int a = 0; a < sizeof(areas) / sizeof(areas[0]); a++) {
        for (int t = 0; t < 5; t++) {
            game_init(g);
            game_enter_town_gate(g, towns[t], 0, 12);
            enter(g, areas[a]);
            ASSERT("escape fixture enters the requested area", g->location == areas[a]);
            if (areas[a] != LOCATION_ISLAND && areas[a] != LOCATION_CROWNROAD_NORTH) {
                game_descend(g);
            }
            int stage = g->level;
            int inventory = g->inventory_count;
            g->gold = 123;
            g->player.hp = 40;
            g->player.mp = 0;
            g->player.known_spells[0] = spell_make_return_to_town();
            g->player.known_spell_count = 1;
            g->player.equipped_spell = 0;
            action_resolve_player(g, (Action){ACTION_CAST_SPELL, 0, 0});
            ASSERT("every area escapes to each of the five last-visited towns at zero MP", g->location == towns[t] && g->last_town_visited == towns[t] && g->player.mp == 0);
            ASSERT("escape leaves no return portal and does not heal or pay rewards", !g->portal_active && !portals(&g->map) &&
                g->gold == 123 && g->player.hp == 40 && g->inventory_count == inventory);
            ASSERT("escape lands on walkable town ground", map_is_walkable(&g->map, g->player.x, g->player.y));
            if (t == 4) {
                ASSERT("escaped area progress saves without an active return anchor", save_game(g, ESCAPE_SLOT) && load_game(loaded, ESCAPE_SLOT) &&
                    loaded->location == towns[t] && !loaded->portal_active && !portals(&loaded->map) && loaded->gold == 123 && loaded->portal_level == stage);
            }
        }
    }

    // A legacy town portal, including a portal beneath dropped equipment, is
    // removed without replacing the town map or its items and discoveries.
    for (int t = 0; t < 5; t++) {
        game_init(g);
        game_enter_town_gate(g, towns[t], 0, 12);
        TileType floor = g->map.tiles[12][21];
        TileType item_floor = g->map.tiles[13][21];
        g->map.tiles[12][21] = TILE_PORTAL;
        g->map.tiles[13][21] = TILE_ITEM;
        g->floor_item_count = 1;
        g->floor_items[0] = (FloorItem){.active = 1, .x = 21, .y = 13, .underlying_tile = TILE_PORTAL, .item = item_make_greatsword()};
        g->floor_items[0].item.sharpened = 1;
        g->portal_active = 1;
        g->gold = 432;
        map_mark_explored(&g->map, 21, 12);
        ASSERT("legacy portal fixture migrates", save_game(g, ESCAPE_SLOT) && old_save(ESCAPE_SLOT) && load_game(loaded, ESCAPE_SLOT));
        ASSERT("migration restores native ground and preserves loose equipment and exploration", !loaded->portal_active && !portals(&loaded->map) &&
            loaded->map.tiles[12][21] == floor && loaded->map.tiles[13][21] == TILE_ITEM && loaded->floor_items[0].underlying_tile == item_floor &&
            loaded->floor_items[0].item.sharpened && loaded->gold == 432 && map_is_explored(&loaded->map, 21, 12));
        ASSERT("retired portals stay closed after another save/load", save_game(loaded, ESCAPE_SLOT) && load_game(g, ESCAPE_SLOT) && !g->portal_active && !portals(&g->map));
    }
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    castle_enter(g, 1);
    g->player.known_spells[0] = spell_make_return_to_town();
    g->player.known_spell_count = 1;
    g->player.equipped_spell = 0;
    action_resolve_player(g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("castle escape keeps its confirmation and special destination", g->castle_prompt == 2 && g->location == LOCATION_CASTLE_INTERIOR);
    castle_prompt_key(g, SDL_SCANCODE_ESCAPE, 0);
    ASSERT("canceling the castle escape leaves the player in the castle", g->location == LOCATION_CASTLE_INTERIOR);
    action_resolve_player(g, (Action){ACTION_CAST_SPELL, 0, 0});
    castle_prompt_key(g, SDL_SCANCODE_RETURN, 0);
    ASSERT("castle exception escapes beside the castle entrance without changing the last town", g->location == LOCATION_CASTLE && g->player.x == CROWNROAD_X && g->player.y == 11 &&
        !g->portal_active && g->last_town_visited == LOCATION_DAWNMERE && map_is_walkable(&g->map, g->player.x, g->player.y));
    ASSERT("castle escape and its regrouped defenders save without a return portal", save_game(g, ESCAPE_SLOT) && load_game(loaded, ESCAPE_SLOT) &&
        loaded->location == LOCATION_CASTLE && !loaded->portal_active && loaded->castle_cache[0].valid);

    game_init(g);
    game_enter_dungeon(g);
    g->last_town_visited = LOCATION_CASTLE;
    game_open_town_portal(g);
    ASSERT("invalid legacy last-town values safely fall back to Oakhaven", g->location == LOCATION_TOWN && !g->portal_active);
    char path[80];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", ESCAPE_SLOT);
    remove(path);
    free(g);
    free(loaded);
}
