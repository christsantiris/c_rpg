#include "game_renderer.h"
#include "sprites.h"
#include "info_panel.h"
#include "message_bar.h"
#include "minimap_renderer.h"
#include "renderer.h"

static void draw_weapon_arrow(Renderer *r, int tile_x, int tile_y,
                              int dx, int dy, int impact) {
    int cx = tile_x * TILE_SIZE + TILE_SIZE / 2;
    int cy = tile_y * TILE_SIZE + TILE_SIZE / 2;
    int px = -dy;
    int py = dx;
    int tail_x = cx - dx * 7;
    int tail_y = cy - dy * 7;
    int tip_x = cx + dx * 7;
    int tip_y = cy + dy * 7;
    int head_x = tip_x - dx * 4;
    int head_y = tip_y - dy * 4;

    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, 18, 16, 24, 220);
    SDL_RenderDrawLine(r->sdl, tail_x + px, tail_y + py,
        tip_x + px, tip_y + py);
    SDL_RenderDrawLine(r->sdl, tail_x - px, tail_y - py,
        tip_x - px, tip_y - py);

    SDL_SetRenderDrawColor(r->sdl, 142, 92, 46, 255);
    SDL_RenderDrawLine(r->sdl, tail_x, tail_y, tip_x, tip_y);

    SDL_SetRenderDrawColor(r->sdl, 210, 216, 220, 255);
    SDL_RenderDrawLine(r->sdl, tip_x, tip_y,
        head_x + px * 3, head_y + py * 3);
    SDL_RenderDrawLine(r->sdl, tip_x, tip_y,
        head_x - px * 3, head_y - py * 3);

    SDL_SetRenderDrawColor(r->sdl, 172, 54, 42, 255);
    SDL_RenderDrawLine(r->sdl, tail_x + dx * 2, tail_y + dy * 2,
        tail_x + px * 3, tail_y + py * 3);
    SDL_RenderDrawLine(r->sdl, tail_x + dx * 2, tail_y + dy * 2,
        tail_x - px * 3, tail_y - py * 3);

    if (impact) {
        SDL_SetRenderDrawColor(r->sdl, 232, 196, 112, 210);
        SDL_RenderDrawPoint(r->sdl, tip_x + px * 3, tip_y + py * 3);
        SDL_RenderDrawPoint(r->sdl, tip_x - px * 3, tip_y - py * 3);
    }
}

static void draw_magic_arrow(Renderer *r, int tile_x, int tile_y,
                             int dx, int dy, int impact, int frame) {
    int cx = tile_x * TILE_SIZE + TILE_SIZE / 2;
    int cy = tile_y * TILE_SIZE + TILE_SIZE / 2;
    int px = -dy;
    int py = dx;
    int tail_x = cx - dx * 7;
    int tail_y = cy - dy * 7;
    int tip_x = cx + dx * 7;
    int tip_y = cy + dy * 7;
    int head_x = tip_x - dx * 4;
    int head_y = tip_y - dy * 4;

    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, 30, 92, 230, 90);
    SDL_RenderDrawLine(r->sdl, tail_x + px * 2, tail_y + py * 2,
        tip_x + px * 2, tip_y + py * 2);
    SDL_RenderDrawLine(r->sdl, tail_x - px * 2, tail_y - py * 2,
        tip_x - px * 2, tip_y - py * 2);

    SDL_SetRenderDrawColor(r->sdl, 54, 126, 255, 255);
    SDL_RenderDrawLine(r->sdl, tail_x, tail_y, tip_x, tip_y);
    SDL_SetRenderDrawColor(r->sdl, 176, 246, 255, 255);
    SDL_RenderDrawPoint(r->sdl, cx, cy);
    SDL_RenderDrawLine(r->sdl, tip_x, tip_y,
        head_x + px * 3, head_y + py * 3);
    SDL_RenderDrawLine(r->sdl, tip_x, tip_y,
        head_x - px * 3, head_y - py * 3);

    SDL_SetRenderDrawColor(r->sdl, 76, 178, 255, 210);
    SDL_RenderDrawPoint(r->sdl,
        tail_x - dx * 2 + px * (frame % 2 ? 2 : -2),
        tail_y - dy * 2 + py * (frame % 2 ? 2 : -2));
    SDL_RenderDrawPoint(r->sdl,
        tail_x - dx * 4 - px * (frame % 2 ? 1 : -1),
        tail_y - dy * 4 - py * (frame % 2 ? 1 : -1));

    if (impact) {
        SDL_SetRenderDrawColor(r->sdl, 198, 250, 255, 230);
        SDL_RenderDrawLine(r->sdl, tip_x - px * 3, tip_y - py * 3,
            tip_x + px * 3, tip_y + py * 3);
    }
}

static void draw_fireball(Renderer *r, int tile_x, int tile_y,
                          int dx, int dy, int frame) {
    int cx = tile_x * TILE_SIZE + TILE_SIZE / 2;
    int cy = tile_y * TILE_SIZE + TILE_SIZE / 2;
    int flicker = frame % 2;
    SDL_Rect glow = {cx - 7, cy - 7, 14, 14};
    SDL_Rect flame = {cx - 5, cy - 5, 10, 10};
    SDL_Rect hot = {cx - 3, cy - 3, 6, 6};
    SDL_Rect core = {cx - 1, cy - 1, 3, 3};

    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, 194, 48, 18, 70);
    SDL_RenderFillRect(r->sdl, &glow);
    SDL_SetRenderDrawColor(r->sdl, 190, 44, 16, 255);
    SDL_RenderFillRect(r->sdl, &flame);
    SDL_SetRenderDrawColor(r->sdl, 250, 116, 18, 255);
    SDL_RenderFillRect(r->sdl, &hot);
    SDL_SetRenderDrawColor(r->sdl, 255, 238, 132, 255);
    SDL_RenderFillRect(r->sdl, &core);

    SDL_SetRenderDrawColor(r->sdl, 238, 78, 14, 220);
    for (int i = 1; i <= 3; i++) {
        int size = 5 - i;
        SDL_Rect ember = {
            cx - dx * (5 + i * 3) - size / 2 + (-dy) * (flicker ? i : -i),
            cy - dy * (5 + i * 3) - size / 2 + dx * (flicker ? i : -i),
            size, size
        };
        SDL_RenderFillRect(r->sdl, &ember);
    }
}

static void draw_fireball_impact(Renderer *r, int tile_x, int tile_y,
                                 Uint32 elapsed) {
    int cx = tile_x * TILE_SIZE + TILE_SIZE / 2;
    int cy = tile_y * TILE_SIZE + TILE_SIZE / 2;
    int radius = 5 + (int)(elapsed * (TILE_SIZE + 8) / 220);
    if (radius > TILE_SIZE + 8) {
        radius = TILE_SIZE + 8;
    }
    SDL_Rect ring = {cx - radius, cy - radius, radius * 2, radius * 2};
    SDL_Rect center = {cx - 5, cy - 5, 10, 10};
    Uint8 alpha = (Uint8)(220 - elapsed * 180 / 220);

    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, 224, 58, 12, alpha);
    SDL_RenderDrawRect(r->sdl, &ring);
    ring.x += 2;
    ring.y += 2;
    ring.w -= 4;
    ring.h -= 4;
    SDL_SetRenderDrawColor(r->sdl, 255, 136, 18, alpha);
    SDL_RenderDrawRect(r->sdl, &ring);
    SDL_SetRenderDrawColor(r->sdl, 255, 224, 112, alpha);
    SDL_RenderFillRect(r->sdl, &center);

    SDL_SetRenderDrawColor(r->sdl, 250, 92, 12, alpha);
    SDL_RenderDrawPoint(r->sdl, cx + radius + 3, cy);
    SDL_RenderDrawPoint(r->sdl, cx - radius - 3, cy);
    SDL_RenderDrawPoint(r->sdl, cx, cy + radius + 3);
    SDL_RenderDrawPoint(r->sdl, cx, cy - radius - 3);
}

void game_draw(Renderer *r, GameState *g, Viewport *v) {
    // Draw map tiles
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (!viewport_is_visible(v, x, y)) continue;
            int sx = viewport_to_screen_x(v, x);
            int sy = viewport_to_screen_y(v, y);
            switch (g->map.tiles[y][x]) {
                case TILE_WALL: draw_wall(r, sx, sy); break;
                case TILE_FOREST_WALL: draw_forest_wall(r, sx, sy); break;
                case TILE_FOREST_FLOOR: draw_forest_floor(r, sx, sy); break;
                case TILE_FOREST_ENTRANCE:
                    draw_forest_edge(r, sx, sy, 0); break;
                case TILE_FOREST_EXIT:
                    draw_forest_edge(r, sx, sy, 1); break;
                case TILE_FOREST_LANDMARK:
                    draw_forest_landmark(r, sx, sy); break;
                case TILE_MOUNTAIN_FLOOR: draw_mountain_floor(r, sx, sy); break;
                case TILE_MOUNTAIN_WALL: draw_mountain_wall(r, sx, sy); break;
                case TILE_MOUNTAIN_ENTRANCE:
                    draw_mountain_edge(r, sx, sy, 0); break;
                case TILE_MOUNTAIN_EXIT:
                    draw_mountain_edge(r, sx, sy, 1); break;
                case TILE_MOUNTAIN_BRIDGE:
                    draw_mountain_bridge(r, sx, sy); break;
                case TILE_MOUNTAIN_CAVE_FLOOR:
                    draw_mountain_cave_floor(r, sx, sy); break;
                case TILE_MOUNTAIN_FORTRESS_FLOOR:
                    draw_mountain_fortress_floor(r, sx, sy); break;
                case TILE_COAST_FLOOR: draw_coast_floor(r, sx, sy); break;
                case TILE_COAST_WALL: draw_coast_wall(r, sx, sy); break;
                case TILE_COAST_ENTRANCE:
                    draw_coast_edge(r, sx, sy, 0); break;
                case TILE_COAST_EXIT:
                    draw_coast_edge(r, sx, sy, 1); break;
                case TILE_COAST_SHALLOW_WATER:
                    draw_coast_shallow_water(r, sx, sy); break;
                case TILE_COAST_DEEP_WATER:
                    draw_coast_deep_water(r, sx, sy); break;
                case TILE_COAST_TIDE_CONTROL:
                    draw_coast_tide_control(r, sx, sy); break;
                case TILE_STAIRS_UP: draw_stairs_up(r, sx, sy); break;
                case TILE_STAIRS_DOWN: draw_stairs_down(r, sx, sy); break;
                case TILE_RETURN_EXIT: draw_return_exit(r, sx, sy); break;
                case TILE_LOCKED_DOOR: draw_locked_door(r, sx, sy); break;
                case TILE_DUNGEON_KEY: draw_dungeon_key(r, sx, sy); break;
                case TILE_PORTAL: draw_portal(r, sx, sy); break;
                case TILE_TOWN_FLOOR: draw_town_floor(r, sx, sy); break;
                case TILE_TOWN_PATH: draw_town_path(r, sx, sy); break;
                case TILE_TOWN_EXIT: {
                    TownExitStyle style;
                    int segment;
                    if (y == 0) {
                        style = TOWN_EXIT_DUNGEON;
                        segment = x - 18;
                    } else if (y == TOWN_H - 1) {
                        style = TOWN_EXIT_COAST;
                        segment = x - 18;
                    } else if (x == 0) {
                        style = TOWN_EXIT_FOREST;
                        segment = y - 10;
                    } else {
                        style = TOWN_EXIT_MOUNTAINS;
                        segment = y - 10;
                    }
                    draw_town_exit(r, sx, sy, style, segment);
                    break;
                }
                case TILE_SHOP_BLACKSMITH:
                case TILE_SHOP_ALCHEMIST:
                case TILE_TAVERN: draw_town_floor(r, sx, sy); break;
                case TILE_ITEM: draw_floor_item(r, sx, sy); break;
                case TILE_TRAP_HIDDEN:
                    if (g->location == LOCATION_FOREST)
                        draw_forest_floor(r, sx, sy);
                    else if (g->location == LOCATION_MOUNTAINS)
                        draw_mountain_floor(r, sx, sy);
                    else if (g->location == LOCATION_COAST)
                        draw_coast_floor(r, sx, sy);
                    else
                        draw_floor(r, sx, sy);
                    break;
                case TILE_TRAP_SPIKE: draw_trap_spike(r, sx, sy); break;
                case TILE_TRAP_FIRE: draw_trap_fire(r, sx, sy); break;
                case TILE_TRAP_POISON: draw_trap_poison(r, sx, sy); break;
                default: draw_floor(r, sx, sy); break;
            }
        }
    }

    // Shops are coordinated 5x4 sprites; draw each once over its town cells.
    if (g->location == LOCATION_TOWN) {
        draw_shop_blacksmith(r,
            viewport_to_screen_x(v, 7), viewport_to_screen_y(v, 7));
        draw_shop_alchemist(r,
            viewport_to_screen_x(v, 28), viewport_to_screen_y(v, 7));
        draw_tavern(r,
            viewport_to_screen_x(v, 5), viewport_to_screen_y(v, 16));
    }

    // Draw enemies
    if (g->location == LOCATION_DUNGEON ||
        g->location == LOCATION_FOREST ||
        g->location == LOCATION_MOUNTAINS ||
        g->location == LOCATION_COAST) {
        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *e = &g->enemies[i];
            if (!e->active) continue;
            if (!viewport_is_visible(v, e->x, e->y)) continue;
            int sx = viewport_to_screen_x(v, e->x);
            int sy = viewport_to_screen_y(v, e->y);
            draw_enemy(r, sx, sy, e->type);
            // Draw health bar above enemy
            int bar_w = TILE_SIZE - 4;
            int bar_h = 3;
            int bar_x = sx * TILE_SIZE + 2;
            int bar_y = sy * TILE_SIZE - 5;
            int fill_w = (bar_w * e->hp) / e->max_hp;
            SDL_Rect bg = {bar_x, bar_y, bar_w, bar_h};
            SDL_Rect fill = {bar_x, bar_y, fill_w, bar_h};
            SDL_SetRenderDrawColor(r->sdl, 60, 20, 20, 255);
            SDL_RenderFillRect(r->sdl, &bg);
            SDL_SetRenderDrawColor(r->sdl, 200, 60, 60, 255);
            SDL_RenderFillRect(r->sdl, &fill);
        }
    }

    // Draw shop labels
    if (g->location == LOCATION_TOWN) {
        SDL_Color label = {220, 180, 60, 255};
        int blacksmith_w = 0;
        int alchemist_w = 0;
        int forest_w = 0;
        int forest_h = 0;
        int dungeon_w = 0;
        int dungeon_h = 0;
        int mountains_w = 0;
        int mountains_h = 0;
        int tavern_w = 0;
        int coast_w = 0;
        int coast_h = 0;
        TTF_SizeText(r->font_tiny, "BLACKSMITH", &blacksmith_w, NULL);
        TTF_SizeText(r->font_tiny, "ALCHEMIST", &alchemist_w, NULL);
        TTF_SizeText(r->font_tiny, "FOREST", &forest_w, &forest_h);
        TTF_SizeText(r->font_tiny, "DUNGEON", &dungeon_w, &dungeon_h);
        TTF_SizeText(r->font_tiny, "MOUNTAINS", &mountains_w, &mountains_h);
        TTF_SizeText(r->font_tiny, "TAVERN", &tavern_w, NULL);
        TTF_SizeText(r->font_tiny, "SUNKEN COAST", &coast_w, &coast_h);
        int bx = viewport_to_screen_x(v, 7) * TILE_SIZE
            + (5 * TILE_SIZE - blacksmith_w) / 2;
        int by = viewport_to_screen_y(v, 6)  * TILE_SIZE;
        int ax = viewport_to_screen_x(v, 28) * TILE_SIZE
            + (5 * TILE_SIZE - alchemist_w) / 2;
        int ay = viewport_to_screen_y(v, 6)  * TILE_SIZE;
        int tavern_x = viewport_to_screen_x(v, 5) * TILE_SIZE
            + (7 * TILE_SIZE - tavern_w) / 2;
        int tavern_y = viewport_to_screen_y(v, 15) * TILE_SIZE;
        if (bx > 0 && by > 0) {
            renderer_draw_text(r, "BLACKSMITH", bx, by, label, r->font_tiny);
        }
        if (ax > 0 && ay > 0) {
            renderer_draw_text(r, "ALCHEMIST", ax, ay, label, r->font_tiny);
        }
        if (tavern_x > 0 && tavern_y > 0) {
            renderer_draw_text(r, "TAVERN", tavern_x, tavern_y, label,
                r->font_tiny);
        }
        int gate_top = viewport_to_screen_y(v, 10) * TILE_SIZE;
        int forest_x = viewport_to_screen_x(v, 1) * TILE_SIZE + 8;
        int forest_y = gate_top + (5 * TILE_SIZE - forest_h) / 2;
        int dungeon_x = viewport_to_screen_x(v, 18) * TILE_SIZE
            + (5 * TILE_SIZE - dungeon_w) / 2;
        int dungeon_y = viewport_to_screen_y(v, 1) * TILE_SIZE
            + (TILE_SIZE - dungeon_h) / 2;
        int mountains_x = viewport_to_screen_x(v, TOWN_W - 1) * TILE_SIZE
            - mountains_w - 8;
        int mountains_y = gate_top + (5 * TILE_SIZE - mountains_h) / 2;
        renderer_draw_text(r, "FOREST", forest_x, forest_y,
            (SDL_Color){90, 190, 105, 255}, r->font_tiny);
        renderer_draw_text(r, "DUNGEON", dungeon_x, dungeon_y, label,
            r->font_tiny);
        renderer_draw_text(r, "MOUNTAINS", mountains_x, mountains_y,
            (SDL_Color){220, 72, 42, 255}, r->font_tiny);
        int coast_x = viewport_to_screen_x(v, 18) * TILE_SIZE
            + (5 * TILE_SIZE - coast_w) / 2;
        int coast_y = viewport_to_screen_y(v, TOWN_H - 2) * TILE_SIZE
            + (TILE_SIZE - coast_h) / 2;
        renderer_draw_text(r, "SUNKEN COAST", coast_x, coast_y,
            (SDL_Color){62, 210, 205, 255}, r->font_tiny);
    }

    // Draw spell/projectile trail
    if (g->trail_frames > 0) {
        int timed_fireball = g->trail_effect == TRAIL_EFFECT_FIREBALL &&
            g->trail_count > 0;
        if (timed_fireball) {
            Uint32 elapsed = SDL_GetTicks() - g->trail_started_at;
            if (elapsed < 240) {
                int lead = (int)(elapsed * g->trail_count / 240);
                if (lead >= g->trail_count) {
                    lead = g->trail_count - 1;
                }
                TrailTile *t = &g->trail[lead];
                if (viewport_is_visible(v, t->x, t->y)) {
                    draw_fireball(r, viewport_to_screen_x(v, t->x),
                        viewport_to_screen_y(v, t->y), g->player.last_dx,
                        g->player.last_dy, (int)(elapsed / 60));
                }
            } else if (elapsed < 460) {
                TrailTile *t = &g->trail[g->trail_count - 1];
                if (viewport_is_visible(v, t->x, t->y)) {
                    draw_fireball_impact(r, viewport_to_screen_x(v, t->x),
                        viewport_to_screen_y(v, t->y), elapsed - 240);
                }
            } else {
                g->trail_frames = 0;
            }
        } else if (g->trail_effect == TRAIL_EFFECT_WEAPON_ARROW ||
                   g->trail_effect == TRAIL_EFFECT_MAGIC_ARROW) {
            int progress = 4 - g->trail_frames;
            int lead = g->trail_count > 1
                ? progress * (g->trail_count - 1) / 3 : 0;
            TrailTile *t = &g->trail[lead];
            if (t->active && viewport_is_visible(v, t->x, t->y)) {
                int sx = viewport_to_screen_x(v, t->x);
                int sy = viewport_to_screen_y(v, t->y);
                if (g->trail_effect == TRAIL_EFFECT_MAGIC_ARROW) {
                    draw_magic_arrow(r, sx, sy,
                        g->player.last_dx, g->player.last_dy,
                        t->is_impact, progress);
                } else {
                    draw_weapon_arrow(r, sx, sy,
                        g->player.last_dx, g->player.last_dy,
                        t->is_impact);
                }
            }
        } else for (int i = 0; i < g->trail_count; i++) {
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
        if (!timed_fireball) {
            g->trail_frames--;
        }
    }

    // Draw player
    draw_player(r,
        viewport_to_screen_x(v, g->player.x),
        viewport_to_screen_y(v, g->player.y),
        g->player.player_class);

    // Draw info panel
    info_panel_draw(r, g);

    // Draw message bar
    message_bar_draw(r, g);

    // Draw minimap overlay in top-left corner of the viewport
    minimap_draw(r, g);
}
