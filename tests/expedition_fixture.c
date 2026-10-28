#include "expedition_fixture.h"
#include "game/giants_crown.h"
#include "game/crownroad_north.h"
#include "game/ironfang.h"
#include "game/stormwatch.h"
#include "game/mangrove.h"
#include "game/elderwood.h"
#include <string.h>

void test_mark_legacy_portal(cJSON *root) {
    const cJSON *level = cJSON_GetObjectItem(root, "portal_level");
    if (cJSON_IsNumber(level) && level->valueint > 0) {
        // Old Return to Town created a return anchor; new escapes do not.
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "portal_active"), 1);
    }
}

void test_restore_cached_floor(GameState *g) {
    switch (g->portal_location) {
        case LOCATION_GIANTS_CROWN: giants_enter(g, 1); return;
        case LOCATION_CROWNROAD_NORTH: northroad_enter(g, 0, 1); return;
        case LOCATION_IRONFANG: ironfang_enter(g, 1); return;
        case LOCATION_STORMWATCH: stormwatch_enter(g, 1); return;
        case LOCATION_MANGROVES: mangrove_enter(g, 1); return;
        case LOCATION_ELDERWOOD: elderwood_portal(g); return;
        default: break;
    }
    LevelCache *cache = NULL;
    int depth = MAX_REGION_DEPTH;
    switch (g->portal_location) {
        case LOCATION_DUNGEON: cache = g->level_cache; depth = DUNGEON_DEPTH; break;
        case LOCATION_FOREST: cache = g->forest_cache; break;
        case LOCATION_MOUNTAINS: cache = g->mountain_cache; break;
        case LOCATION_COAST: cache = g->coast_cache; depth = COAST_DEPTH; break;
        case LOCATION_SWAMP: cache = g->swamp_cache; depth = SWAMP_DEPTH; break;
        case LOCATION_FROSTFELL: cache = g->frostfell_cache; break;
        case LOCATION_DRAGONSPINE: cache = g->dragonspine_cache; depth = DRAGONSPINE_DEPTH; break;
        case LOCATION_DESERT: cache = g->desert_cache; depth = DESERT_DEPTH; break;
        case LOCATION_ASHEN: cache = g->ashen_cache; depth = ASHEN_DEPTH; break;
        case LOCATION_GLASSDEEP: cache = g->glassdeep_cache; depth = GLASSDEEP_DEPTH; break;
        case LOCATION_CATACOMBS: cache = g->catacombs_cache; depth = CATACOMBS_DEPTH; break;
        case LOCATION_TEMPLE: cache = g->temple_cache; depth = TEMPLE_DEPTH; break;
        case LOCATION_LABYRINTH: cache = g->labyrinth_cache; depth = LABYRINTH_DEPTH; break;
        default: return;
    }
    if (g->portal_level < 1 || g->portal_level > depth || !cache[g->portal_level - 1].valid) {
        return;
    }
    g->location = g->portal_location;
    if (g->location == LOCATION_DUNGEON) {
        g->dungeon_entry_town = g->dungeon_portal_town;
    } else if (g->location == LOCATION_FOREST) {
        g->forest_entry_town = g->forest_portal_town;
    } else if (g->location == LOCATION_MOUNTAINS) {
        g->mountain_entry_town = g->mountain_portal_town;
    } else if (g->location == LOCATION_SWAMP) {
        g->swamp_entry_town = g->swamp_portal_town;
    } else if (g->location == LOCATION_CATACOMBS) {
        g->catacombs_entry_town = g->northern_portal_town;
    }
    g->level = g->portal_level;
    g->map = cache[g->level - 1].map;
    g->enemy_count = cache[g->level - 1].enemy_count;
    g->level_cleared = cache[g->level - 1].level_cleared;
    memcpy(g->enemies, cache[g->level - 1].enemies, sizeof(g->enemies));
    g->floor_item_count = 0;
    g->player.x = g->portal_x;
    g->player.y = g->portal_y;
    g->portal_active = 0;
    game_refresh_quest_encounters(g);
}
