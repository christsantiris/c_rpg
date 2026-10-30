#include "town_life_renderer.h"
#include "sprites.h"
#include "world_props.h"
#include "npc_renderer.h"

void town_life_draw_goods(Renderer *r, int tx, int ty, int style, int shelf) {
    if (style == 4 && r->alehouse_cask_texture) {
        SDL_Rect source = {shelf ? 0 : TILE_SIZE, 0, TILE_SIZE, TILE_SIZE};
        SDL_Rect dest = {tx * TILE_SIZE, ty * TILE_SIZE, TILE_SIZE, TILE_SIZE};
        SDL_RenderCopy(r->sdl, r->alehouse_cask_texture, &source, &dest);
        return;
    }
    WorldPropArt art = style == 0 ? PROP_BREAD : style == 1 ? PROP_MEAT : style == 2 ? PROP_CANDLE : PROP_SPICES;
    if (style == 4) {
        art = PROP_GOBLET;
    } else if (style == 5) {
        art = shelf ? PROP_CRYSTAL : PROP_RELIC;
    }
    draw_world_prop(r, tx, ty, art, (SDL_Color){239, 145, 42, 255});
}

void town_life_draw_building(Renderer *r, const Viewport *v, const TownBuilding *b) {
    int x = viewport_to_screen_x(v, b->x) * TILE_SIZE;
    int y = viewport_to_screen_y(v, b->y) * TILE_SIZE;
    int w = b->w * TILE_SIZE;
    int h = b->h * TILE_SIZE;
    SDL_Texture *texture = r->local_building_textures[b->style];
    if (texture) {
        SDL_Rect dest = {x, y, w, h};
        SDL_RenderCopy(r->sdl, texture, NULL, &dest);
    } else if (b->style == 2) {
        draw_town_hall(r, viewport_to_screen_x(v, b->x), viewport_to_screen_y(v, b->y));
    } else {
        draw_workshop(r, viewport_to_screen_x(v, b->x), viewport_to_screen_y(v, b->y));
    }
    int width = 0;
    TTF_SizeText(r->font_tiny, b->label, &width, NULL);
    int label_y = y - (b->style == 4 ? 6 : TILE_SIZE);
    renderer_draw_text(r, b->label, x + (w - width) / 2, label_y,
        (SDL_Color){220, 180, 60, 255}, r->font_tiny);
}

void town_life_draw_resident(Renderer *r, int tx, int ty, int style) {
    static const NpcArt residents[4] = {NPC_ART_NESSA, NPC_ART_HOLT, NPC_ART_EDRIC, NPC_ART_SURI};
    if (style < 0 || style >= 4) {
        style = style == 5 ? 3 : 0;
    }
    draw_tavern_floor(r, tx, ty);
    npc_draw(r, tx, ty, residents[style], 0);
}
