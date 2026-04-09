#include "halloffame_renderer.h"
#include "sprites.h"

void halloffame_draw(Renderer *r, const HighScoreTable *t) {
    int full_tiles_x = r->screen_w / TILE_SIZE;
    for (int y = 0; y < r->tiles_y; y++) {
        for (int x = 0; x < full_tiles_x; x++) {
            draw_floor(r, x, y);
        }
    }
    for (int x = 0; x < full_tiles_x; x++) {
        draw_wall(r, x, 0);
        draw_wall(r, x, r->tiles_y - 1);
    }
    for (int y = 0; y < r->tiles_y; y++) {
        draw_wall(r, 0, y);
        draw_wall(r, full_tiles_x - 1, y);
    }

    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color dim = {80, 80, 80, 255};
    SDL_Color hint = {50, 70, 50, 255};

    int cx = r->screen_w / 2;
    int y = 40;
    int lh = 28;

    renderer_draw_text(r, "HALL OF FAME", cx - 80, y, gold, r->font_large);
    y += lh + 10;

    renderer_draw_text(r, "RANK", cx - 220, y, gold, r->font_small);
    renderer_draw_text(r, "NAME", cx - 140, y, gold, r->font_small);
    renderer_draw_text(r, "SCORE", cx + 60, y, gold, r->font_small);
    renderer_draw_text(r, "LEVEL", cx + 160, y, gold, r->font_small);
    y += lh;

    if (t->count == 0) {
        renderer_draw_text(r, "NO SCORES YET", cx - 80, y + 40, dim, r->font_small);
    } else {
        for (int i = 0; i < t->count; i++) {
            SDL_Color color = i == 0 ? gold : white;
            char rank[4];
            char score[12];
            char level[8];
            SDL_snprintf(rank, sizeof(rank), "%d", i + 1);
            SDL_snprintf(score, sizeof(score), "%d", t->entries[i].score);
            SDL_snprintf(level, sizeof(level), "%d", t->entries[i].level);
            renderer_draw_text(r, rank,                cx - 220, y, color, r->font_small);
            renderer_draw_text(r, t->entries[i].name,  cx - 140, y, color, r->font_small);
            renderer_draw_text(r, score,               cx + 60,  y, color, r->font_small);
            renderer_draw_text(r, level,               cx + 160, y, color, r->font_small);
            y += lh;
        }
    }

    renderer_draw_text(r, "ESC / ENTER TO CONTINUE",
        cx - 130, (r->tiles_y - 2) * TILE_SIZE, hint, r->font_small);
}