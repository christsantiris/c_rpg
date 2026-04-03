#include "game_renderer.h"
#include "sprites.h"
#include "info_panel.h"
#include "message_bar.h"

void game_draw(Renderer *r, GameState *g, Viewport *v) {
    // Draw map tiles
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (!viewport_is_visible(v, x, y)) continue;
            int sx = viewport_to_screen_x(v, x);
            int sy = viewport_to_screen_y(v, y);
            switch (g->map.tiles[y][x]) {
                case TILE_WALL:            draw_wall(r, sx, sy);            break;
                case TILE_STAIRS_UP:       draw_stairs_up(r, sx, sy);       break;
                case TILE_STAIRS_DOWN:     draw_stairs_down(r, sx, sy);     break;
                case TILE_TOWN_FLOOR:      draw_town_floor(r, sx, sy);      break;
                case TILE_TOWN_PATH:       draw_town_path(r, sx, sy);       break;
                case TILE_TOWN_EXIT:       draw_town_exit(r, sx, sy);       break;
                case TILE_SHOP_BLACKSMITH: draw_shop_blacksmith(r, sx, sy); break;
                case TILE_SHOP_ALCHEMIST:  draw_shop_alchemist(r, sx, sy);  break;
                case TILE_ITEM:            draw_floor_item(r, sx, sy);      break;
                default:                   draw_floor(r, sx, sy);           break;
            }
        }
    }

    // Draw enemies
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *e = &g->enemies[i];
        if (!e->active) continue;
        if (!viewport_is_visible(v, e->x, e->y)) continue;
        int sx = viewport_to_screen_x(v, e->x);
        int sy = viewport_to_screen_y(v, e->y);
        draw_enemy(r, sx, sy, e->type);
    }

    // Draw spell/projectile trail
    if (g->trail_frames > 0) {
        for (int i = 0; i < g->trail_count; i++) {
            TrailTile *t = &g->trail[i];
            if (!t->active) continue;
            if (!viewport_is_visible(v, t->x, t->y)) continue;
            int sx = viewport_to_screen_x(v, t->x);
            int sy = viewport_to_screen_y(v, t->y);
            SDL_Rect tr = {
                sx * TILE_SIZE, sy * TILE_SIZE,
                TILE_SIZE, TILE_SIZE
            };
            Uint8 alpha = t->is_impact ? 200 : 120;
            SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r->sdl, t->r, t->g, t->b, alpha);
            SDL_RenderFillRect(r->sdl, &tr);
            if (t->is_impact) {
                SDL_SetRenderDrawColor(r->sdl, t->r, t->g, t->b, 255);
                SDL_RenderDrawLine(r->sdl,
                    sx * TILE_SIZE + TILE_SIZE / 2,
                    sy * TILE_SIZE + 4,
                    sx * TILE_SIZE + TILE_SIZE / 2,
                    sy * TILE_SIZE + TILE_SIZE - 4);
                SDL_RenderDrawLine(r->sdl,
                    sx * TILE_SIZE + 4,
                    sy * TILE_SIZE + TILE_SIZE / 2,
                    sx * TILE_SIZE + TILE_SIZE - 4,
                    sy * TILE_SIZE + TILE_SIZE / 2);
            }
        }
        g->trail_frames--;
    }

    // Draw player
    draw_player(r,
        viewport_to_screen_x(v, g->player.x),
        viewport_to_screen_y(v, g->player.y));

    // Draw info panel
    info_panel_draw(r, g);

    // Draw message bar
    message_bar_draw(r, g);
}