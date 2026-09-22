#include "harbor_renderer.h"
#include "info_panel.h"
#include "message_bar.h"

static void draw_centered_text(Renderer *r, const char *text, int center_x, int y, SDL_Color color, TTF_Font *font) {
    if (!font) {
        return;
    }
    int width = 0;
    TTF_SizeText(font, text, &width, NULL);
    renderer_draw_text(r, text, center_x - width / 2, y, color, font);
}

static void draw_harbor_background(Renderer *r, int play_w, int play_h) {
    SDL_Rect sky = {0, 0, play_w, play_h * 2 / 3};
    SDL_SetRenderDrawColor(r->sdl, 31, 58, 72, 255);
    SDL_RenderFillRect(r->sdl, &sky);

    SDL_Rect horizon = {0, play_h * 2 / 3, play_w, play_h / 3};
    SDL_SetRenderDrawColor(r->sdl, 8, 54, 70, 255);
    SDL_RenderFillRect(r->sdl, &horizon);

    for (int y = play_h * 2 / 3; y < play_h; y += 18) {
        Uint8 blue = (Uint8)(76 + (y / 18) % 3 * 10);
        SDL_SetRenderDrawColor(r->sdl, 12, 71, blue, 255);
        SDL_RenderDrawLine(r->sdl, 0, y, play_w, y);
    }

    SDL_SetRenderDrawColor(r->sdl, 224, 211, 164, 180);
    SDL_RenderDrawLine(r->sdl, 0, play_h * 2 / 3, play_w, play_h * 2 / 3);
}

SDL_Rect harbor_button_rect(const Renderer *r, int option) {
    int play_w = r->screen_w - INFO_PANEL_W;
    int play_h = r->tiles_y * TILE_SIZE;
    int button_w = play_w - 32;
    if (button_w > 520) {
        button_w = 520;
    }
    SDL_Rect button = {(play_w - button_w) / 2,
        play_h - 126 + option * 54, button_w, 42};
    return button;
}

static void draw_harbor_option(Renderer *r, const HarborScreen *s, int option, int enabled, const char *label) {
    SDL_Rect button = harbor_button_rect(r, option);
    int selected = s->selected == option;
    SDL_Color border = selected ? (SDL_Color){220, 180, 60, 255}
        : (SDL_Color){84, 104, 108, 255};
    SDL_Color text = enabled ? (SDL_Color){218, 226, 216, 255}
        : (SDL_Color){126, 126, 126, 255};
    if (selected && enabled) {
        text = (SDL_Color){245, 220, 142, 255};
    }

    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, 9, 18, 24, 225);
    SDL_RenderFillRect(r->sdl, &button);
    SDL_SetRenderDrawColor(r->sdl, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(r->sdl, &button);
    if (selected) {
        SDL_Rect inner = {button.x + 3, button.y + 3, button.w - 6, button.h - 6};
        SDL_RenderDrawRect(r->sdl, &inner);
    }
    renderer_draw_text(r, selected ? ">" : " ", button.x + 18,
        button.y + 14, border, r->font_small);
    renderer_draw_text(r, label, button.x + 46, button.y + 14,
        text, r->font_small);
}

void harbor_draw(Renderer *r, const GameState *g, const HarborScreen *s) {
    int play_w = r->screen_w - INFO_PANEL_W;
    int play_h = r->tiles_y * TILE_SIZE;
    int has_map = game_has_treasure_map(g);
    draw_harbor_background(r, play_w, play_h);

    if (r->harbor_texture) {
        SDL_Rect source = {150, 174, 1129, 827};
        int art_h = play_h - 24;
        int art_w = art_h * source.w / source.h;
        if (art_w > play_w - 32) {
            art_w = play_w - 32;
            art_h = art_w * source.h / source.w;
        }
        SDL_Rect destination = {(play_w - art_w) / 2,
            play_h - art_h, art_w, art_h};
        SDL_RenderCopy(r->sdl, r->harbor_texture, &source, &destination);
    }

    SDL_Rect heading = {24, 18, play_w - 48, 82};
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, 7, 14, 19, 210);
    SDL_RenderFillRect(r->sdl, &heading);
    SDL_SetRenderDrawColor(r->sdl, 91, 128, 132, 255);
    SDL_RenderDrawRect(r->sdl, &heading);

    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color ready = {132, 204, 160, 255};
    SDL_Color locked = {220, 118, 92, 255};
    draw_centered_text(r, "HARBOR DOCK", play_w / 2, 34, gold, r->font_large);
    draw_centered_text(r, has_map
        ? "THE TREASURE MAP CHARTS A COURSE TO THE RUINED ISLE."
        : "A SEA CHART IS REQUIRED. SPEAK WITH CAPTAIN ROWAN.",
        play_w / 2, 70, has_map ? ready : locked, r->font_tiny);

    draw_harbor_option(r, s, 0, has_map, has_map
        ? "BOARD SHIP - SAIL TO THE RUINED ISLE"
        : "BOARD SHIP - TREASURE MAP REQUIRED");
    draw_harbor_option(r, s, 1, 1, "RETURN TO TOWN");

    info_panel_draw_harbor(r, g);
    message_bar_draw(r, g);
}
