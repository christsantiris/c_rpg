#include "info_panel.h"
#include "item_icons.h"
#include <stdio.h>

void info_panel_draw(Renderer *r, const GameState *g) {
    int px = r->screen_w - INFO_PANEL_W;

    // Separator line
    SDL_SetRenderDrawColor(r->sdl, 58, 58, 106, 255);
    SDL_RenderDrawLine(r->sdl, px, 0, px, r->screen_h);

    // Panel background
    SDL_Rect bg = { px + 1, 0, INFO_PANEL_W - 1, r->screen_h };
    SDL_SetRenderDrawColor(r->sdl, 13, 13, 26, 255);
    SDL_RenderFillRect(r->sdl, &bg);

    SDL_Color label  = { 74,  74, 122, 255};
    SDL_Color value  = {200, 200, 232, 255};
    SDL_Color dimmed = { 80,  80,  80, 255};
    SDL_Color hint   = { 50,  50,  80, 255};

    int x  = px + 10;
    int y  = 10;
    int lh = 20;

    // Character name
    renderer_draw_text(r, "CHARACTER", x, y, label, r->font_tiny);
    y += lh;
    renderer_draw_text(r, g->player.name, x, y, value, r->font_small);
    y += lh + 6;

    // Location
    renderer_draw_text(r, "LOCATION", x, y, label, r->font_tiny);
    y += lh;
    char loc[24];
    if (g->location == LOCATION_TOWN) {
        SDL_snprintf(loc, sizeof(loc), "TOWN");
    } else {
        SDL_snprintf(loc, sizeof(loc), "DUNGEON %d", g->level);
    }
    renderer_draw_text(r, loc, x, y, value, r->font_tiny);
    y += lh + 6;

    // HP
    renderer_draw_text(r, "HP", x, y, label, r->font_tiny);
    y += lh;
    char hp_str[16];
    SDL_snprintf(hp_str, sizeof(hp_str), "%d / %d", g->player.hp, g->player.max_hp);
    renderer_draw_text(r, hp_str, x, y, value, r->font_tiny);
    y += lh + 6;

    // MP
    renderer_draw_text(r, "MP", x, y, label, r->font_tiny);
    y += lh;
    char mp_str[16];
    SDL_snprintf(mp_str, sizeof(mp_str), "%d / %d", g->player.mp, g->player.max_mp);
    renderer_draw_text(r, mp_str, x, y, value, r->font_tiny);
    y += lh + 6;

    // Level and XP
    renderer_draw_text(r, "LEVEL", x, y, label, r->font_tiny);
    y += lh;
    char lvl_str[24];
    SDL_snprintf(lvl_str, sizeof(lvl_str), "%d  (%d/%d)",
        g->player.level, g->player.experience, g->player.experience_next);
    renderer_draw_text(r, lvl_str, x, y, value, r->font_tiny);
    y += lh + 6;

    // Gold
    renderer_draw_text(r, "GOLD", x, y, label, r->font_tiny);
    y += lh;
    char gold_str[16];
    SDL_snprintf(gold_str, sizeof(gold_str), "%d", g->gold);
    SDL_Color gold_color = {220, 180, 60, 255};
    renderer_draw_text(r, "SCORE", x, y, label, r->font_tiny);
    y += lh;
    char score_str[16];
    SDL_snprintf(score_str, sizeof(score_str), "%d", g->score);
    SDL_Color score_color = {255, 255, 255, 255};
    renderer_draw_text(r, score_str, x, y, score_color, r->font_tiny);
    y += lh + 10;
    
    // Equipment grid (2x2)
    renderer_draw_text(r, "EQUIPPED", x, y, label, r->font_tiny);
    y += lh;

    int grid_x1 = x;
    int grid_x2 = x + ICON_SIZE + 6;
    int grid_y1 = y;
    int grid_y2 = y + ICON_SIZE + 6;

    // Slot 1: Weapon (top-left)
    draw_icon_empty_slot(r, grid_x1, grid_y1);
    if (g->equipped_weapon >= 0 && g->equipped_weapon < g->inventory_count)
        draw_icon_weapon(r, grid_x1, grid_y1);

    // Slot 2: Off-hand (top-right)
    draw_icon_empty_slot(r, grid_x2, grid_y1);

    // Slot 3: Armor (bottom-left)
    draw_icon_empty_slot(r, grid_x1, grid_y2);
    if (g->equipped_armor >= 0 && g->equipped_armor < g->inventory_count)
        draw_icon_armor(r, grid_x1, grid_y2);

    // Slot 4: Helmet (bottom-right)
    draw_icon_empty_slot(r, grid_x2, grid_y2);

    y = grid_y2 + ICON_SIZE + 16;

    // Key hints
    renderer_draw_text(r, "WASD  MOVE",    x, y,        hint, r->font_tiny);
    renderer_draw_text(r, ".     DESCEND", x, y + lh,   hint, r->font_tiny);
    renderer_draw_text(r, ",     ASCEND",  x, y + lh*2, hint, r->font_tiny);
    renderer_draw_text(r, "I     INV",     x, y + lh*3, hint, r->font_tiny);
    renderer_draw_text(r, "B     SPELLS",  x, y + lh*4, hint, r->font_tiny);
    renderer_draw_text(r, "C     CAST",    x, y + lh*5, hint, r->font_tiny);
    renderer_draw_text(r, "F     FIRE",    x, y + lh*6, hint, r->font_tiny);
    renderer_draw_text(r, "T     TOWN",    x, y + lh*7, hint, r->font_tiny);
    renderer_draw_text(r, "ESC   MENU",    x, y + lh*8, hint, r->font_tiny);
}