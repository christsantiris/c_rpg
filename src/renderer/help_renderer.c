#include "help_renderer.h"
#include "controls_renderer.h"
#include "sprites.h"
#include <stdio.h>

#define HELP_DESC_OFFSET 136

static void draw_help_row(Renderer *r, const char *key, const char *desc, int x, int y, SDL_Color color) {
    renderer_draw_text(r, key, x, y, color, r->font_tiny);
    renderer_draw_text(r, desc, x + HELP_DESC_OFFSET, y, color, r->font_tiny);
}

// Gameplay keys come from this character's bindings; view keys are fixed.
static const char *bound(const GameState *g, ControlAction action) {
    return controls_key_name(g->key_bindings[action]);
}

void help_draw(Renderer *r, const GameState *g) {
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
    SDL_Color dim = {120, 120, 120, 255};
    SDL_Color hint = {50, 70, 50, 255};

    int cx = r->screen_w / 2;
    int col1 = cx - 280;
    int col2 = cx + 20;
    int y = 40;
    int lh = 24;

    renderer_draw_text(r, "CONTROLS", cx - 50, y, gold, r->font_large);
    y += lh + 10;

    renderer_draw_text(r, "MOVEMENT",       col1, y, gold,  r->font_small);
    renderer_draw_text(r, "COMBAT",         col2, y, gold,  r->font_small);
    y += lh;
    draw_help_row(r, "Bump enemy", "Melee attack", col2, y, white);
    draw_help_row(r, bound(g, CONTROL_CAST_SPELL), "Cast spell", col2, y + lh, white);
    draw_help_row(r, bound(g, CONTROL_RANGED_ATTACK), "Fire ranged", col2, y + 2 * lh, white);
    // One move per row so long key names such as "Keypad 8" stay readable.
    draw_help_row(r, bound(g, CONTROL_MOVE_UP), "Move up", col1, y, white);
    y += lh;
    draw_help_row(r, bound(g, CONTROL_MOVE_DOWN), "Move down", col1, y, white);
    y += lh;
    draw_help_row(r, bound(g, CONTROL_MOVE_LEFT), "Move left", col1, y, white);
    y += lh;
    draw_help_row(r, bound(g, CONTROL_MOVE_RIGHT), "Move right", col1, y, white);
    y += lh;
    draw_help_row(r, "ARROWS", "Also move", col1, y, white);
    y += lh;
    draw_help_row(r, bound(g, CONTROL_DESCEND), "Stairs / exit", col1, y, white);
    y += lh;
    draw_help_row(r, bound(g, CONTROL_ASCEND), "Ascend stairs", col1, y, white);
    y += lh + 10;

    renderer_draw_text(r, "INVENTORY",      col1, y, gold,  r->font_small);
    renderer_draw_text(r, "WORLD",          col2, y, gold,  r->font_small);
    y += lh;
    draw_help_row(r, bound(g, CONTROL_INVENTORY), "Open inventory", col1, y, white);
    draw_help_row(r, "Bump shop door", "Enter shop", col2, y, white);
    y += lh;
    draw_help_row(r, bound(g, CONTROL_PICK_UP), "Pick up item", col1, y, white);
    draw_help_row(r, "ESC", "Main menu", col2, y, white);
    y += lh;
    draw_help_row(r, "U", "Use item", col1, y, white);
    draw_help_row(r, bound(g, CONTROL_TALK), "Talk to NPC", col2, y, white);
    y += lh;
    draw_help_row(r, "E/ENTER", "Equip item", col1, y, white);
    draw_help_row(r, bound(g, CONTROL_MOVE_LEFT), "Interact", col2, y, white);
    y += lh;
    draw_help_row(r, "D", "Drop item", col1, y, white);
    draw_help_row(r, bound(g, CONTROL_QUEST_JOURNAL), "Quests / bosses", col2, y, white);
    y += lh + 10;

    renderer_draw_text(r, "MAGIC",          col1, y, gold,  r->font_small);
    y += lh;
    draw_help_row(r, bound(g, CONTROL_SPELLBOOK), "Open spellbook", col1, y, white);
    y += lh;
    draw_help_row(r, bound(g, CONTROL_CAST_SPELL), "Cast equipped spell", col1, y, white);
    y += lh + 10;

    renderer_draw_text(r, "Press ? or ESC to close", cx - 120, (r->tiles_y - 2) * TILE_SIZE, hint, r->font_small);
}
