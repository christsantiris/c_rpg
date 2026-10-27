#include "containers_renderer.h"
#include "../game/containers.h"
#include "sprites.h"
#include "world_props.h"
#include "item_icons.h"
#include "controls_renderer.h"
#include "giants_crown_renderer.h"
#include "mangrove_renderer.h"
#include "stormwatch_renderer.h"
#include "ironfang_renderer.h"
#include "crownroad_north_renderer.h"
#include "castle_renderer.h"
#include "jail_renderer.h"
#include "info_panel.h"

static void floor_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, int kind) {
    TileType ground = containers_ground(g, x, y);
    switch (g->location) {
        case LOCATION_DUNGEON: draw_dungeon_floor(r, sx, sy, x, y); break;
        case LOCATION_FOREST: draw_forest_floor(r, sx, sy, x, y); break;
        case LOCATION_MOUNTAINS: draw_mountain_cave_floor(r, sx, sy, x, y); break;
        case LOCATION_COAST:
            if (kind == TILE_QUEEN_RELIQUARY) {
                draw_coast_channel(r, sx, sy, x, y, 0, 0);
            } else {
                draw_coast_floor(r, sx, sy, x, y);
            }
            break;
        case LOCATION_SWAMP: draw_swamp_floor(r, sx, sy, x, y); break;
        case LOCATION_DRAGONSPINE:
            draw_dragonspine_floor(r, sx, sy, x, y,
                ground == TILE_DRAGON_ASH || ground == TILE_DRAGON_RELIQUARY_ASH ? 1 :
                ground == TILE_DRAGON_HOARD || ground == TILE_DRAGON_RELIQUARY_HOARD ? 2 : 0);
            break;
        case LOCATION_LABYRINTH: draw_labyrinth_floor(r, sx, sy, x, y); break;
        case LOCATION_TEMPLE: draw_temple_floor(r, sx, sy, x, y); break;
        case LOCATION_DESERT: draw_desert_floor(r, sx, sy, x, y); break;
        case LOCATION_ASHEN: draw_ashen_floor(r, sx, sy, x, y); break;
        case LOCATION_GLASSDEEP:
            if (ground == TILE_GLASSDEEP_RUIN || ground == TILE_PRISM_PAVED_RELIQUARY) {
                draw_glassdeep_ruin(r, sx, sy, x, y);
            } else {
                draw_glassdeep_floor(r, sx, sy, x, y);
            }
            break;
        case LOCATION_CATACOMBS: draw_catacombs_tile(r, &g->map, sx, sy, x, y, TILE_CATACOMBS_FLOOR); break;
        case LOCATION_GIANTS_CROWN: giants_draw_tile(r, g, sx, sy, x, y, TILE_GIANTS_GRASS); break;
        case LOCATION_MANGROVES: mangrove_draw_tile(r, g, sx, sy, x, y, g->level == 4 ? TILE_MANGROVE_DECK : TILE_MANGROVE_MUD); break;
        case LOCATION_IRONFANG: ironfang_draw_tile(r, g, sx, sy, x, y, TILE_IRON_FLOOR); break;
        case LOCATION_STORMWATCH: stormwatch_draw_tile(r, g, sx, sy, x, y, TILE_STORM_GRASS); break;
        case LOCATION_CROWNROAD_NORTH: northroad_draw_tile(r, sx, sy, x, y, TILE_ELDERWOOD_FLOOR); break;
        case LOCATION_CASTLE_INTERIOR: castle_draw_tile(r, g, sx, sy, x, y, TILE_CASTLE_FLOOR); break;
        case LOCATION_JAIL_ARMORY: jail_draw_tile(r, g, sx, sy, x, y, TILE_CASTLE_FLOOR); break;
        default: draw_floor(r, sx, sy); break;
    }
}

void containers_draw(Renderer *r, const GameState *g, const Viewport *v) {
    if (!r->loot_chest_texture) {
        return;
    }
    for (int sy = 0; sy < v->tiles_y; sy++) {
        for (int sx = 0; sx < v->tiles_x; sx++) {
            int x = sx + v->cam_x;
            int y = sy + v->cam_y;
            int kind;
            int opened;
            if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || !map_is_explored(&g->map, x, y) ||
                g->map.tiles[y][x] == TILE_PORTAL || !containers_at(g, x, y, &kind, &opened)) {
                continue;
            }
            floor_tile(r, g, sx, sy, x, y, kind);
            int row = g->location == LOCATION_COAST || g->location == LOCATION_MANGROVES ? 1 :
                g->location == LOCATION_GIANTS_CROWN ? 2 : g->location == LOCATION_CASTLE_INTERIOR || g->location == LOCATION_JAIL_ARMORY ? 3 : 0;
            SDL_Rect source = {opened * 24, row * 24, 24, 24};
            SDL_Rect dest = {sx * TILE_SIZE, sy * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            SDL_RenderCopy(r->sdl, r->loot_chest_texture, &source, &dest);
            // Existing dropped equipment on the same tile remains visible.
            for (int i = 0; i < g->floor_item_count; i++) {
                const FloorItem *item = &g->floor_items[i];
                if (item->active && item->x == x && item->y == y) {
                    draw_icon_item(r, sx * TILE_SIZE, sy * TILE_SIZE, &item->item);
                }
            }
        }
    }
    for (int i = 0; i < g->loot_container_count; i++) {
        const LootContainer *c = &g->loot_containers[i];
        if (c->location != (int)g->location || c->level != g->level ||
            !viewport_is_visible(v, c->loot_x, c->loot_y) || !map_is_explored(&g->map, c->loot_x, c->loot_y)) {
            continue;
        }
        int sx = viewport_to_screen_x(v, c->loot_x);
        int sy = viewport_to_screen_y(v, c->loot_y);
        if (c->gold) {
            draw_floor_gold(r, sx, sy);
        } else if (c->rewards) {
            Item item = containers_reward(c, c->rewards & 1 ? 1 : 2);
            draw_icon_item(r, sx * TILE_SIZE, sy * TILE_SIZE, &item);
        } else if (c->kind == TILE_JAIL_GEAR_CHEST && g->jail_gear_state == 1) {
            draw_world_prop(r, sx, sy, PROP_MIXED_LOOT, (SDL_Color){191, 190, 178, 255});
        }
    }
}

void containers_draw_hint(Renderer *r, const GameState *g) {
    const char *hint = containers_hint(g);
    int loot = 0;
    for (int i = 0; i < g->loot_container_count; i++) {
        const LootContainer *c = &g->loot_containers[i];
        loot |= c->location == (int)g->location && c->level == g->level && c->loot_x == g->player.x && c->loot_y == g->player.y &&
            (c->gold || c->rewards || (c->kind == TILE_JAIL_GEAR_CHEST && g->jail_gear_state == 1));
    }
    if (!hint && !loot) {
        return;
    }
    char text[180];
    char interact[40];
    snprintf(interact, sizeof(interact), "%s", controls_key_name(g->key_bindings[CONTROL_MOVE_LEFT]));
    snprintf(text, sizeof(text), "%s%s%s%s%s: PICK UP LOOSE LOOT", hint ? interact : "", hint ? ": " : "",
        hint ? hint + 3 : "", hint ? " | " : "", controls_key_name(g->key_bindings[CONTROL_PICK_UP]));
    SDL_Rect backing = {4, r->tiles_y * TILE_SIZE - 21, r->screen_w - INFO_PANEL_W - 8, 18};
    SDL_SetRenderDrawColor(r->sdl, 21, 29, 31, 255);
    SDL_RenderFillRect(r->sdl, &backing);
    renderer_draw_text(r, text, 9, backing.y + 4, (SDL_Color){239, 218, 154, 255}, r->font_tiny);
}
