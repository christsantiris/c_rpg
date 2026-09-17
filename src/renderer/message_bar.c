#include "message_bar.h"
#include <string.h>

static int message_line_length(const char *message, int max_chars) {
    int length = (int)SDL_strlen(message);
    if (length <= max_chars) {
        return length;
    }
    int split = max_chars;
    while (split > 0 && message[split] != ' ') {
        split--;
    }
    int minimum_lines = (length + max_chars - 1) / max_chars;
    int remaining = length - split - 1;
    if (split > 0 &&
        1 + (remaining + max_chars - 1) / max_chars <= minimum_lines) {
        return split;
    }
    return max_chars;
}

static int message_line_count(const char *message, int max_chars) {
    int lines = 0;
    while (*message) {
        message += message_line_length(message, max_chars);
        while (*message == ' ') {
            message++;
        }
        lines++;
    }
    return lines;
}

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

    if (g->message_count == 0 || !r->font_tiny) {
        return;
    }

    SDL_Color color = {180, 160, 120, 255};
    int glyph_w = 8;
    int glyph_h;
    if (TTF_SizeText(r->font_tiny, "W", &glyph_w, &glyph_h) != 0 ||
        glyph_w < 1) {
        glyph_w = 8;
    }
    int max_chars = (r->screen_w - 20) / glyph_w;
    if (max_chars < 1) {
        max_chars = 1;
    }
    int max_lines = (r->screen_h - bar_top - 12) / 10;
    int first = g->message_count;
    int used_lines = 0;
    for (int i = g->message_count - 1; i >= 0; i--) {
        int lines = message_line_count(g->messages[i], max_chars);
        if (used_lines + lines > max_lines) {
            break;
        }
        first = i;
        used_lines += lines;
    }
    int y = r->screen_h - 6 - used_lines * 10;
    for (int i = first; i < g->message_count; i++) {
        const char *message = g->messages[i];
        while (*message) {
            int length = message_line_length(message, max_chars);
            char line[MAX_MESSAGE_LEN];
            memcpy(line, message, (size_t)length);
            line[length] = '\0';
            renderer_draw_text(r, line, 10, y, color, r->font_tiny);
            y += 10;
            message += length;
            while (*message == ' ') {
                message++;
            }
        }
    }
}
