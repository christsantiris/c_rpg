#include "quarry_quest_renderer.h"
#include "../game/quarry_quest.h"
#include "info_panel.h"

static void label(Renderer *r, const Viewport *v, int x, int y, const char *text) {
    if (!viewport_is_visible(v, x, y)) {
        return;
    }
    int width;
    TTF_SizeText(r->font_tiny, text, &width, NULL);
    int px = viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2;
    int py = viewport_to_screen_y(v, y) * TILE_SIZE - 18;
    int right = r->screen_w - INFO_PANEL_W - width - 4;
    px = px < 4 ? 4 : px > right ? right : px;
    int bottom = r->tiles_y * TILE_SIZE - TTF_FontHeight(r->font_tiny) - 8;
    py = py < 88 ? 88 : py > bottom ? bottom : py;
    renderer_draw_text(r, text, px, py, (SDL_Color){233, 211, 155, 255}, r->font_tiny);
}

void quarry_draw_details(Renderer *r, const GameState *g, const Viewport *v) {
    if (!quarry_road(g)) {
        return;
    }
    if (r->caravan_camp_texture && g->map.tiles[QUARRY_CAMP_Y + 2][QUARRY_CAMP_X + 1] == TILE_CARAVAN_CAMP) {
        SDL_Rect camp = {viewport_to_screen_x(v, QUARRY_CAMP_X) * TILE_SIZE,
            viewport_to_screen_y(v, QUARRY_CAMP_Y) * TILE_SIZE, 312, 144};
        SDL_RenderCopy(r->sdl, r->caravan_camp_texture, NULL, &camp);
    }
    label(r, v, QUARRY_ORIN_X, QUARRY_ORIN_Y, "SURVEYOR ORIN");
    label(r, v, QUARRY_WORKER_X + 1, QUARRY_ORIN_Y + 2, g->glassdeep_quest_state >= 2 ? "STONEWAKE RESTORED" : "QUARRY WORKERS' CAMP");
}
