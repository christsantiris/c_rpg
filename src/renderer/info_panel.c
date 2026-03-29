#include "info_panel.h"

void info_panel_draw(Renderer *r, const GameState *g) {
    int px = r->screen_w - INFO_PANEL_W;

    // Separator line
    SDL_SetRenderDrawColor(r->sdl, 58, 58, 106, 255);
    SDL_RenderDrawLine(r->sdl, px, 0, px, r->screen_h);

    // Panel background
    SDL_Rect bg = { px + 1, 0, INFO_PANEL_W - 1, r->screen_h };
    SDL_SetRenderDrawColor(r->sdl, 13, 13, 26, 255);
    SDL_RenderFillRect(r->sdl, &bg);

    SDL_Color label = { 74,  74, 122, 255};
    SDL_Color value = {200, 200, 232, 255};
    SDL_Color hint  = { 50,  50,  80, 255};

    int x  = px + 10;
    int y  = 16;
    int lh = 28;

    renderer_draw_text(r, "CHARACTER", x, y, label, r->font_small);
    y += lh;
    renderer_draw_text(r, g->player.name, x, y, value, r->font_small);
    y += lh * 2;

    renderer_draw_text(r, "DUNGEON", x, y, label, r->font_small);
    y += lh;
    char lvl[16];
    SDL_snprintf(lvl, sizeof(lvl), "LEVEL %d", g->level);
    renderer_draw_text(r, lvl, x, y, value, r->font_small);
    y += lh * 2;

    renderer_draw_text(r, "HP", x, y, label, r->font_small);
    y += lh;
    renderer_draw_text(r, "-- / --", x, y, value, r->font_small);
    y += lh * 2;

    renderer_draw_text(r, "MP", x, y, label, r->font_small);
    y += lh;
    renderer_draw_text(r, "-- / --", x, y, value, r->font_small);

    int hy = r->screen_h - 120;
    renderer_draw_text(r, "WASD  MOVE",    x, hy,        hint, r->font_small);
    renderer_draw_text(r, ".     DESCEND", x, hy + lh,   hint, r->font_small);
    renderer_draw_text(r, ",     ASCEND",  x, hy + lh*2, hint, r->font_small);
    renderer_draw_text(r, "ESC   MENU",    x, hy + lh*3, hint, r->font_small);
}