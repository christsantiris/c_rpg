#include "jail_renderer.h"
#include "sprites.h"
#include "world_props.h"
#include "npc_renderer.h"
#include "../game/jail.h"

void jail_draw_person(Renderer *r, int sx, int sy, int prisoner) {
    npc_draw(r, sx, sy, prisoner ? NPC_ART_TOMAS : NPC_ART_INFORMANT, 0);
}

void jail_draw_building(Renderer *r, const Viewport *v) {
    int x = viewport_to_screen_x(v, JAIL_X) * TILE_SIZE;
    int y = viewport_to_screen_y(v, JAIL_Y) * TILE_SIZE;
    int w = JAIL_W * TILE_SIZE;
    if (r->royal_dungeon_texture) {
        SDL_Rect destination = {x, y, w, JAIL_H * TILE_SIZE};
        SDL_RenderCopy(r->sdl, r->royal_dungeon_texture, NULL, &destination);
    }
    int width = 0;
    TTF_SizeText(r->font_tiny, "ROYAL DUNGEON", &width, NULL);
    renderer_draw_text(r, "ROYAL DUNGEON", x + (w - width) / 2, y - TILE_SIZE / 2, (SDL_Color){220, 180, 60, 255}, r->font_tiny);
}

void jail_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile) {
    if (tile == TILE_CASTLE_WALL) {
        draw_dungeon_wall(r, sx, sy, x, y);
        return;
    }
    draw_dungeon_floor(r, sx, sy, x, y);
    if (tile == TILE_JAIL_BARS) {
        draw_world_prop(r, sx, sy, PROP_GATE, (SDL_Color){151, 155, 168, 255});
    } else if (tile == TILE_JAIL_HATCH && (g->location != LOCATION_JAIL || g->jail_quest_state >= 2)) {
        draw_world_prop(r, sx, sy, PROP_HATCH, (SDL_Color){165, 135, 90, 255});
    } else if (tile == TILE_JAIL_GEAR_CHEST) {
        draw_world_prop(r, sx, sy, PROP_CHEST, g->jail_gear_state == 3 ? (SDL_Color){95, 96, 100, 255} : (SDL_Color){218, 178, 83, 255});
    } else if (tile == TILE_TUNNEL_EXIT) {
        draw_world_prop(r, sx, sy, PROP_EXIT, (SDL_Color){138, 190, 149, 255});
    } else if (tile == TILE_NPC_PRISONER) {
        jail_draw_person(r, sx, sy, 1);
    } else if (tile == TILE_NPC_ROYAL_GUARD) {
        draw_royal_guard_overlay(r, sx, sy, 0);
    } else if (g->location == LOCATION_JAIL && x == 6 && y == 6) {
        draw_world_prop(r, sx, sy, PROP_STRAW, (SDL_Color){171, 146, 87, 255});
    }
}
