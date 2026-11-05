#include "royal_evidence_renderer.h"
#include "game_renderer.h"

static void rectangle(Renderer *r, SDL_Rect rect, SDL_Color color) {
    SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(r->sdl, &rect);
}

static void seal(Renderer *r, int x, int y, int size) {
    int pixel = size / 16;
    SDL_Color edge = {74, 23, 31, 255};
    SDL_Color wax = {151, 42, 48, 255};
    SDL_Color light = {194, 70, 62, 255};
    SDL_Color gold = {239, 189, 87, 255};
    rectangle(r, (SDL_Rect){x + pixel * 4, y + pixel * 12, pixel * 4, pixel * 6}, edge);
    rectangle(r, (SDL_Rect){x + pixel * 9, y + pixel * 12, pixel * 3, pixel * 6}, wax);
    for (int row = 0; row < 16; row++) {
        int inset = row < 2 || row > 13 ? 4 : row < 4 || row > 11 ? 2 : 0;
        rectangle(r, (SDL_Rect){x + inset * pixel, y + row * pixel, (16 - inset * 2) * pixel, pixel}, edge);
        if (row > 1 && row < 14) {
            rectangle(r, (SDL_Rect){x + (inset + 1) * pixel, y + row * pixel, (14 - inset * 2) * pixel, pixel}, wax);
        }
    }
    rectangle(r, (SDL_Rect){x + pixel * 4, y + pixel * 2, pixel * 7, pixel}, light);
    rectangle(r, (SDL_Rect){x + pixel * 4, y + pixel * 6, pixel * 2, pixel * 5}, gold);
    rectangle(r, (SDL_Rect){x + pixel * 7, y + pixel * 4, pixel * 2, pixel * 7}, gold);
    rectangle(r, (SDL_Rect){x + pixel * 10, y + pixel * 6, pixel * 2, pixel * 5}, gold);
    rectangle(r, (SDL_Rect){x + pixel * 4, y + pixel * 9, pixel * 8, pixel * 3}, gold);
}

static int paragraph(Renderer *r, const char *text, int x, int y, int width, TTF_Font *font, SDL_Color color) {
    int char_width = 0;
    TTF_SizeText(font, "M", &char_width, NULL);
    int columns = char_width > 0 ? width / char_width : 1;
    int lines = draw_wrapped_text(r, text, x, y, columns > 0 ? columns : 1, font, color, 1);
    return lines * (TTF_FontHeight(font) + 8);
}

void royal_evidence_draw(Renderer *r, const GameState *g, const RoyalEvidence *document, SDL_Rect paper, const char *footer) {
    if (!document) {
        return;
    }
    int compact = paper.h < 300;
    int padding = compact ? 12 : 26;
    int stamp = compact ? 32 : 64;
    SDL_Color ink = {49, 36, 30, 255};
    SDL_Color muted = {105, 70, 43, 255};
    SDL_Color burgundy = {111, 31, 42, 255};
    rectangle(r, (SDL_Rect){paper.x + 5, paper.y + 6, paper.w, paper.h}, (SDL_Color){15, 12, 18, 255});
    rectangle(r, paper, (SDL_Color){113, 77, 45, 255});
    rectangle(r, (SDL_Rect){paper.x + 3, paper.y + 3, paper.w - 6, paper.h - 6}, (SDL_Color){206, 178, 121, 255});
    rectangle(r, (SDL_Rect){paper.x + 8, paper.y + 8, paper.w - 16, paper.h - 16}, (SDL_Color){238, 222, 178, 255});
    seal(r, paper.x + paper.w - padding - stamp - 4, paper.y + padding, stamp);
    SDL_Rect inside = {paper.x + 8, paper.y + 8, paper.w - 16, paper.h - 16};
    SDL_RenderSetClipRect(r->sdl, &inside);
    int x = paper.x + padding;
    int y = paper.y + padding;
    int width = paper.w - padding * 2;
    renderer_draw_text(r, document->title, x, y, ink, compact ? r->font_small : r->font_large);
    renderer_draw_text(r, document->source, x, y + (compact ? 18 : 28), muted, r->font_tiny);
    int label_width = 0;
    TTF_SizeText(r->font_tiny, "KING'S SEAL", &label_width, NULL);
    renderer_draw_text(r, "KING'S SEAL", paper.x + paper.w - padding - label_width, y + stamp + 12, burgundy, r->font_tiny);
    y += compact ? 56 : 108;
    TTF_Font *body = compact ? r->font_tiny : r->font_small;
    y += paragraph(r, document->text, x, y, width, body, ink) + (compact ? 4 : 24);
    paragraph(r, royal_evidence_conclusion(g), x, y, width, body, burgundy);
    if (footer && footer[0]) {
        renderer_draw_text(r, footer, x, paper.y + paper.h - padding - 8, muted, r->font_tiny);
    }
    SDL_RenderSetClipRect(r->sdl, NULL);
}
