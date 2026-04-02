#include "message_bar.h"

void message_bar_draw(Renderer *r, const GameState *g) {
    // Background bar
    int bar_top = r->tiles_y * TILE_SIZE;
    SDL_Rect bar = { 0, bar_top,
                     r->screen_w, r->screen_h - bar_top };
    SDL_SetRenderDrawColor(r->sdl, 10, 10, 20, 255);
    SDL_RenderFillRect(r->sdl, &bar);

    // Top border line
    SDL_SetRenderDrawColor(r->sdl, 58, 58, 106, 255);
    SDL_RenderDrawLine(r->sdl, 0, bar_top, r->screen_w, bar_top);

    if (g->message_count == 0) return;

    SDL_Color color = {180, 160, 120, 255};
    int x = 10;
    int y = r->screen_h - MESSAGE_BAR_H + 8;

    // Show messages separated by spaces
    for (int i = 0; i < g->message_count; i++) {
        renderer_draw_text(r, g->messages[i], x, y, color, r->font_tiny);
        x += (int)SDL_strlen(g->messages[i]) * 8 + 20;
    }
}