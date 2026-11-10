#include "western_arc_renderer.h"
#include "../game/western_arc.h"
#include "../game/approach.h"
#include "../game/garran.h"
#include "world_props.h"
#include "controls_renderer.h"

static void label(Renderer *r, const Viewport *v, int x, int y, const char *text) {
    if (!viewport_is_visible(v, x, y)) {
        return;
    }
    int width = 0;
    TTF_SizeText(r->font_tiny, text, &width, NULL);
    int px = viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2;
    int py = viewport_to_screen_y(v, y) * TILE_SIZE + 25;
    int right = v->tiles_x * TILE_SIZE;
    int bottom = v->tiles_y * TILE_SIZE;
    if (px < 4) {
        px = 4;
    }
    if (px + width > right - 4) {
        px = right - width - 4;
    }
    if (py > bottom - TTF_FontHeight(r->font_tiny) - 4) {
        py = bottom - TTF_FontHeight(r->font_tiny) - 4;
    }
    if (py < 88) {
        py = 88;
    }
    renderer_draw_text(r, text, px + 1, py + 1, (SDL_Color){29, 23, 19, 255}, r->font_tiny);
    renderer_draw_text(r, text, px, py, (SDL_Color){233, 211, 155, 255}, r->font_tiny);
}

void western_draw_details(Renderer *r, const GameState *g, const Viewport *v) {
    if (approach_sunscar_road(g)) {
        if (r->caravan_camp_texture && map_is_explored(&g->map, 46, 29)) {
            SDL_Rect dest = {viewport_to_screen_x(v, 40) * TILE_SIZE, viewport_to_screen_y(v, 29) * TILE_SIZE, 312, 144};
            SDL_RenderCopy(r->sdl, r->caravan_camp_texture, NULL, &dest);
        }
        label(r, v, CAMP_ZARA_X, CAMP_ZARA_Y, "ZARA");
        if (g->garran.state == GARRAN_WAITING && garran_at(g, CAMP_GARRAN_X, CAMP_GARRAN_Y)) {
            label(r, v, CAMP_GARRAN_X, CAMP_GARRAN_Y, "GARRAN / ROGUE");
        }
        label(r, v, CAMP_LIORA_X, CAMP_LIORA_Y, "LIORA");
        label(r, v, 46, 35, "CARAVAN CAMP");
    }
    if (g->location == LOCATION_TAVERN && !g->zara_at_camp && g->map.tiles[ZARA_OAKHAVEN_Y][ZARA_OAKHAVEN_X] == TILE_NPC_GUILD_SEEKER) {
        label(r, v, ZARA_OAKHAVEN_X, ZARA_OAKHAVEN_Y, "ZARA");
    }
    if (g->location != LOCATION_DESERT || g->level != 2 || (g->tribute_progress & 1) ||
        !viewport_is_visible(v, TRIBUTE_X, TRIBUTE_Y) || !map_is_explored(&g->map, TRIBUTE_X, TRIBUTE_Y)) {
        return;
    }
    int sx = viewport_to_screen_x(v, TRIBUTE_X);
    int sy = viewport_to_screen_y(v, TRIBUTE_Y);
    draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){190, 151, 77, 255});
    label(r, v, TRIBUTE_X, TRIBUTE_Y, "ROYAL TRIBUTE");
    if (western_hint(g) && !g->dialogue_active) {
        char hint[100];
        SDL_snprintf(hint, sizeof(hint), "%s: Inspect tribute cargo", controls_key_name(g->key_bindings[CONTROL_MOVE_LEFT]));
        label(r, v, TRIBUTE_X, TRIBUTE_Y + 1, hint);
    }
}
