#include "game_over_renderer.h"
#include "sprites.h"

static const char *game_over_location(const GameState *g) {
    switch (g->location) {
        case LOCATION_DUNGEON: return "DUNGEON";
        case LOCATION_FOREST: return "HAUNTED FOREST";
        case LOCATION_MOUNTAINS: return "GOBLIN MOUNTAINS";
        case LOCATION_COAST: return "SUNKEN COAST";
        case LOCATION_TAVERN: return "TAVERN";
        case LOCATION_TOWN2: return "TOWN 2";
        case LOCATION_INN: return "INN";
        case LOCATION_ISLAND: return "RUINED ISLE";
        case LOCATION_TEMPLE: return "RUINED TEMPLE";
        case LOCATION_LABYRINTH: return "ROOK'S LABYRINTH";
        default: return "TOWN";
    }
}

static void draw_centered(Renderer *r, const char *text, int y, SDL_Color color, TTF_Font *font) {
    int width = 0;
    TTF_SizeText(font, text, &width, NULL);
    renderer_draw_text(r, text, (r->screen_w - width) / 2, y, color, font);
}

void game_over_draw(Renderer *r, const GameState *g) {
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

    SDL_Color red   = {200,  50,  50, 255};
    SDL_Color gold  = {220, 180,  60, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color hint  = { 50,  70,  50, 255};

    int cy = r->screen_h / 2;

    draw_centered(r, "GAME OVER", cy - 130, red, r->font_large);

    char name_str[32];
    SDL_snprintf(name_str, sizeof(name_str), "%s", g->player.name);
    draw_centered(r, name_str, cy - 70, gold, r->font_small);

    char level_str[64];
    SDL_snprintf(level_str, sizeof(level_str), "FELL IN %s - LEVEL %d",
        game_over_location(g), g->level);
    draw_centered(r, level_str, cy - 35, white, r->font_small);

    draw_centered(r, "CAUSE OF DEATH", cy + 10, red, r->font_small);
    const char *cause = g->message_count > 0
        ? g->messages[g->message_count - 1]
        : "Your injuries proved fatal.";
    draw_centered(r, cause, cy + 38, white, r->font_small);

    draw_centered(r, "PRESS ENTER FOR HALL OF FAME", cy + 95, hint,
        r->font_small);
}
