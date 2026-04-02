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
    SDL_Color dimmed = { 80,  80,  80, 255};
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
    char hp_str[16];
    SDL_snprintf(hp_str, sizeof(hp_str), "%d / %d", g->player.hp, g->player.max_hp);
    renderer_draw_text(r, hp_str, x, y, value, r->font_small);
    y += lh * 2;

    renderer_draw_text(r, "MP", x, y, label, r->font_small);
    y += lh;
    char mp_str[16];
    SDL_snprintf(mp_str, sizeof(mp_str), "%d / %d", g->player.mp, g->player.max_mp);
    renderer_draw_text(r, mp_str, x, y, value, r->font_small);

    // Handle display XP
    y += lh * 2;
    renderer_draw_text(r, "LEVEL", x, y, label, r->font_small);
    y += lh;
    char lvl_str[16];
    SDL_snprintf(lvl_str, sizeof(lvl_str), "%d", g->player.level);
    renderer_draw_text(r, lvl_str, x, y, value, r->font_small);
    y += lh * 2;
    renderer_draw_text(r, "XP", x, y, label, r->font_small);
    y += lh;
    char xp_str[16];
    SDL_snprintf(xp_str, sizeof(xp_str), "%d / %d",
        g->player.experience, g->player.experience_next);
    renderer_draw_text(r, xp_str, x, y, value, r->font_small);

    // Message log moved to bottom of map area to avoid info panel overlap
    // SDL_Color msg_color = {180, 160, 120, 255};
    // int msg_y = r->screen_h - 120 - (MAX_MESSAGES * lh) - lh;
    // for (int i = 0; i < g->message_count; i++) {
    //     renderer_draw_text(r, g->messages[i], x, msg_y + i * lh, msg_color, r->font_tiny);
    // }

    // Equipped items
    y += lh;
    renderer_draw_text(r, "WEAPON", x, y, label, r->font_small);
    y += lh;
    if (g->equipped_weapon >= 0 && g->equipped_weapon < g->inventory_count) {
        char wpn[32];
        SDL_snprintf(wpn, sizeof(wpn), "%s", g->inventory[g->equipped_weapon].name);
        renderer_draw_text(r, wpn, x, y, value, r->font_small);
    } else {
        renderer_draw_text(r, "NONE", x, y, dimmed, r->font_small);
    }
    y += lh * 2;

    renderer_draw_text(r, "ARMOR", x, y, label, r->font_small);
    y += lh;
    if (g->equipped_armor >= 0 && g->equipped_armor < g->inventory_count) {
        char arm[32];
        SDL_snprintf(arm, sizeof(arm), "%s", g->inventory[g->equipped_armor].name);
        renderer_draw_text(r, arm, x, y, value, r->font_small);
    } else {
        renderer_draw_text(r, "NONE", x, y, dimmed, r->font_small);
    }
    y += lh * 2;

    int hy = r->screen_h - 120;
    renderer_draw_text(r, "WASD  MOVE",    x, hy,        hint, r->font_small);
    renderer_draw_text(r, ".     DESCEND", x, hy + lh,   hint, r->font_small);
    renderer_draw_text(r, ",     ASCEND",  x, hy + lh*2, hint, r->font_small);
    renderer_draw_text(r, "ESC   MENU",    x, hy + lh*3, hint, r->font_small);
}