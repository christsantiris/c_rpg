#include "game_over_renderer.h"
#include "sprites.h"

void game_over_draw(Renderer *r, const GameState *g) {
    int full_tiles_x = r->screen_w / TILE_SIZE;

    for (int y = 0; y < r->tiles_y; y++)
        for (int x = 0; x < full_tiles_x; x++)
            draw_floor(r, x, y);

    for (int x = 0; x < full_tiles_x; x++) {
        draw_wall(r, x, 0);
        draw_wall(r, x, r->tiles_y - 1);
    }
    for (int y = 0; y < r->tiles_y; y++) {
        draw_wall(r, 0, y);
        draw_wall(r, full_tiles_x - 1, y);
    }

    SDL_Color red   = {200,  50,  50, 255};
    SDL_Color gold  = {220, 180,  60, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color hint  = { 50,  70,  50, 255};

    int cx = r->screen_w / 2;
    int cy = r->screen_h / 2;

    renderer_draw_text(r, "GAME OVER", cx - 80, cy - 80, red,   r->font_large);

    char name_str[32];
    SDL_snprintf(name_str, sizeof(name_str), "%s", g->player.name);
    renderer_draw_text(r, name_str, cx - 80, cy - 20, gold, r->font_small);

    char level_str[32];
    SDL_snprintf(level_str, sizeof(level_str), "FELL ON DUNGEON LEVEL %d", g->level);
    renderer_draw_text(r, level_str, cx - 120, cy + 10, white, r->font_small);

    renderer_draw_text(r, "PRESS ENTER TO CONTINUE", cx - 130, cy + 60, hint, r->font_small);
}