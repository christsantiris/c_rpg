#include "minimap_renderer.h"

// Minimap renders as a corner overlay on the game viewport.
// Each tile is 1x1px, scaled 1:2 (100x50px total).
// A semi-transparent dark background sits behind it for readability.
#define MINIMAP_SCALE 2
#define MINIMAP_W     (MAP_W / MINIMAP_SCALE)
#define MINIMAP_H     (MAP_H / MINIMAP_SCALE)
#define MINIMAP_PAD   6

void minimap_draw(Renderer *r, const GameState *g) {
    if (g->location != LOCATION_DUNGEON) {
        return;
    }

    int ox = MINIMAP_PAD;
    int oy = MINIMAP_PAD;

    // Dark semi-transparent background
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_Rect bg = { ox - 2, oy - 2, MINIMAP_W + 4, MINIMAP_H + 4 };
    SDL_SetRenderDrawColor(r->sdl, 0, 0, 0, 180);
    SDL_RenderFillRect(r->sdl, &bg);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_NONE);

    // Tiles — sample the full 2x2 block per pixel so 1-tile-wide
    // hallways are never missed due to stride skipping.
    for (int ty = 0; ty < MAP_H; ty += MINIMAP_SCALE) {
        for (int tx = 0; tx < MAP_W; tx += MINIMAP_SCALE) {
            int draw_x = ox + tx / MINIMAP_SCALE;
            int draw_y = oy + ty / MINIMAP_SCALE;
            int has_stair = 0;
            int has_floor = 0;

            for (int dy = 0; dy < MINIMAP_SCALE; dy++) {
                for (int dx = 0; dx < MINIMAP_SCALE; dx++) {
                    int sx = tx + dx;
                    int sy = ty + dy;
                    if (sx >= MAP_W || sy >= MAP_H) {
                        continue;
                    }
                    TileType tile = g->map.tiles[sy][sx];
                    if (tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN) {
                        has_stair = 1;
                    } else if (tile != TILE_WALL) {
                        has_floor = 1;
                    }
                }
            }

            if (has_stair) {
                SDL_SetRenderDrawColor(r->sdl, 220, 180, 60, 255);
                SDL_RenderDrawPoint(r->sdl, draw_x, draw_y);
            } else if (has_floor) {
                SDL_SetRenderDrawColor(r->sdl, 70, 70, 100, 255);
                SDL_RenderDrawPoint(r->sdl, draw_x, draw_y);
            }
        }
    }

    // Player dot
    SDL_SetRenderDrawColor(r->sdl, 80, 200, 80, 255);
    SDL_RenderDrawPoint(r->sdl,
        ox + g->player.x / MINIMAP_SCALE,
        oy + g->player.y / MINIMAP_SCALE);
}
