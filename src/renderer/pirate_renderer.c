#include "pirate_renderer.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include "controls_renderer.h"

static unsigned int water_seed(int x, int y) {
    unsigned int seed = (unsigned int)x * 73856093u ^ (unsigned int)y * 19349663u;
    return seed ^ (seed >> 13);
}

static void draw_water(Renderer *r, int sx, int sy, int x, int y) {
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    SDL_Rect cell = {px, py, TILE_SIZE, TILE_SIZE};
    SDL_SetRenderDrawColor(r->sdl, 5, 35, 50, 255);
    SDL_RenderFillRect(r->sdl, &cell);
    unsigned int seed = water_seed(x, y);
    unsigned int frame = SDL_GetTicks() / 120u;
    for (int i = 0; i < 2; i++) {
        unsigned int phase = (frame + (seed >> (i * 8))) % 16u;
        int swell = phase < 8 ? phase : 15 - phase;
        int wx = px + 2 + (seed >> (i * 5) & 7u) + swell / 2;
        int wy = py + 3 + i * 11 + (seed >> 16 & 3u) + swell / 4;
        int length = 4 + (seed >> (i * 3 + 4) & 3u);
        SDL_SetRenderDrawColor(r->sdl, 26 + swell * 3, 69 + swell * 5, 86 + swell * 5, 255);
        SDL_RenderDrawLine(r->sdl, wx, wy, wx + length, wy);
        SDL_RenderDrawLine(r->sdl, wx + 2, wy - 1, wx + length - 1, wy - 1);
        if (swell >= 5) {
            SDL_SetRenderDrawColor(r->sdl, 70, 137, 151, 255);
            SDL_RenderDrawPoint(r->sdl, wx + length / 2, wy - 1);
        }
    }
}

static int sea_at(const Map *map, int x, int y) {
    return x >= 0 && x < PIRATE_W && y >= 0 && y < PIRATE_H && map->tiles[y][x] == TILE_PIRATE_SEA;
}

static int sand_at(const Map *map, int x, int y) {
    if (x < 0 || x >= PIRATE_W || y < 0 || y >= PIRATE_H) {
        return 0;
    }
    TileType tile = map->tiles[y][x];
    return tile == TILE_PIRATE_SAND || tile == TILE_PIRATE_CHEST || tile == TILE_PIRATE_CRATE;
}

// Round both beach headlands and the inside corners of water inlets at pixel scale.
static void round_shore(Renderer *r, const Map *map, int sx, int sy, int x, int y, int sea) {
    const int curve[12] = {10, 7, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0};
    const int dx[4] = {-1, 1, 1, -1};
    const int dy[4] = {-1, -1, 1, 1};
    for (int corner = 0; corner < 4; corner++) {
        int round = sea ? sand_at(map, x + dx[corner], y) && sand_at(map, x, y + dy[corner]) :
            sea_at(map, x + dx[corner], y) && sea_at(map, x, y + dy[corner]);
        if (!round) {
            continue;
        }
        int cx = sx * TILE_SIZE + (dx[corner] < 0 ? 0 : TILE_SIZE - 1);
        int cy = sy * TILE_SIZE + (dy[corner] < 0 ? 0 : TILE_SIZE - 1);
        for (int row = 0; row < 12; row++) {
            int width = curve[row];
            if (!width) {
                continue;
            }
            SDL_SetRenderDrawColor(r->sdl, sea ? 177 : 5, sea ? 158 : 35, sea ? 109 : 50, 255);
            SDL_RenderDrawLine(r->sdl, cx, cy - dy[corner] * row, cx - dx[corner] * (width - 1), cy - dy[corner] * row);
        }
    }
}

static void draw_sand(Renderer *r, int sx, int sy, int x, int y) {
    SDL_Rect cell = {sx * TILE_SIZE, sy * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    SDL_SetRenderDrawColor(r->sdl, 177, 158, 109, 255);
    SDL_RenderFillRect(r->sdl, &cell);
    unsigned int seed = water_seed(x, y);
    SDL_SetRenderDrawColor(r->sdl, 193, 174, 123, 255);
    int yy = cell.y + 4 + seed % 15;
    SDL_RenderDrawLine(r->sdl, cell.x + 3, yy, cell.x + 13, yy);
    SDL_SetRenderDrawColor(r->sdl, 156, 138, 93, 255);
    SDL_RenderDrawLine(r->sdl, cell.x + 9, yy + 2, cell.x + 18, yy + 2);
    if (seed % 11 == 0) {
        SDL_SetRenderDrawColor(r->sdl, 231, 215, 168, 255);
        SDL_Rect shell = {cell.x + 10, cell.y + 15, 4, 2};
        SDL_RenderFillRect(r->sdl, &shell);
        SDL_RenderDrawPoint(r->sdl, cell.x + 12, cell.y + 14);
    }
}

static void draw_coastal_rock(Renderer *r, int sx, int sy, int x, int y) {
    SDL_Rect cell = {sx * TILE_SIZE, sy * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    SDL_SetRenderDrawColor(r->sdl, 55, 57, 48, 255);
    SDL_RenderFillRect(r->sdl, &cell);
    unsigned int seed = water_seed(x, y);
    for (int i = 0; i < 4; i++) {
        unsigned int bits = seed >> (i * 4);
        SDL_Rect rock = {cell.x + (int)(bits % 11), cell.y + i * 6, 13, 6};
        SDL_SetRenderDrawColor(r->sdl, 85 + bits % 13, 83 + bits % 9, 67 + bits % 7, 255);
        SDL_RenderFillRect(r->sdl, &rock);
        SDL_SetRenderDrawColor(r->sdl, 120, 115, 92, 255);
        SDL_RenderDrawLine(r->sdl, rock.x + 2, rock.y, rock.x + 10, rock.y);
    }
}

static void draw_pirate_edge(Renderer *r, const Map *map, int sx, int sy, int x, int y) {
    int dx = x == 0 ? -1 : x == PIRATE_W - 1 ? 1 : 0;
    int dy = y == 0 ? -1 : y == PIRATE_H - 1 ? 1 : 0;
    int fx = x - dy;
    int fy = y + dx;
    // The flanking terrain identifies the surface even when loot covers the approach.
    int deck = fx >= 0 && fx < PIRATE_W && fy >= 0 && fy < PIRATE_H && map->tiles[fy][fx] == TILE_PIRATE_RAIL;
    draw_pirate_tile(r, map, sx, sy, x, y, deck ? TILE_PIRATE_DECK : TILE_PIRATE_SAND);
    SDL_Color ink = deck ? (SDL_Color){211, 190, 140, 255} : (SDL_Color){115, 91, 48, 255};
    SDL_Color edge = deck ? (SDL_Color){47, 39, 30, 255} : (SDL_Color){213, 187, 130, 255};
    // Worn ground chevrons point out through the existing cliff, stockade or rail opening.
    for (int n = 0; n < 2; n++) {
        int cx = sx * TILE_SIZE + 12 + dx * (n * 7 - 3);
        int cy = sy * TILE_SIZE + 12 + dy * (n * 7 - 3);
        for (int shade = 1; shade >= 0; shade--) {
            SDL_Color color = shade ? edge : ink;
            SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, color.a);
            for (int side = -1; side <= 1; side += 2) {
                SDL_RenderDrawLine(r->sdl, cx + dx * 2, cy + dy * 2 + shade,
                    cx - dx * 3 - dy * side * 4, cy - dy * 3 + dx * side * 4 + shade);
            }
        }
    }
}

void draw_pirate_tile(Renderer *r, const Map *map, int sx, int sy, int x, int y, TileType tile) {
    if (tile >= TILE_PIRATE_CARGO && tile <= TILE_PIRATE_MAST) {
        draw_pirate_tile(r, map, sx, sy, x, y, TILE_PIRATE_DECK);
        if (r->pirate_harbor_texture) {
            int index = tile == TILE_PIRATE_MAST ? x < 45 ? 0 : 1 :
                tile == TILE_PIRATE_CARGO ? x < 45 ? 2 : 3 : tile == TILE_PIRATE_WINCH ? 4 : 5;
            SDL_Rect source = {index * 96 + 36, 96, 24, 24};
            SDL_Rect dest = {sx * TILE_SIZE, sy * TILE_SIZE, 24, 24};
            if (tile == TILE_PIRATE_MAST) {
                source = (SDL_Rect){index * 96, 0, 96, 120};
                dest = (SDL_Rect){sx * TILE_SIZE - 36, sy * TILE_SIZE - 96, 96, 120};
            }
            SDL_RenderCopy(r->sdl, r->pirate_harbor_texture, &source, &dest);
        }
        return;
    }
    if (tile == TILE_PIRATE_QUEST_CHEST) {
        draw_pirate_tile(r, map, sx, sy, x, y, TILE_PIRATE_DECK);
        draw_world_prop(r, sx, sy, PROP_SILVER_CHEST, (SDL_Color){177, 205, 218, 255});
        return;
    }
    if (tile == TILE_PIRATE_SEA) {
        draw_water(r, sx, sy, x, y);
        round_shore(r, map, sx, sy, x, y, 1);
        return;
    }
    if (tile == TILE_PIRATE_CLIFF) {
        draw_coastal_rock(r, sx, sy, x, y);
        return;
    }
    if (tile == TILE_PIRATE_ENTRANCE || tile == TILE_PIRATE_EXIT) {
        draw_pirate_edge(r, map, sx, sy, x, y);
        return;
    }
    SDL_Rect cell = {sx * TILE_SIZE, sy * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    if (tile == TILE_PIRATE_DECK || tile == TILE_PIRATE_RAIL || tile == TILE_PIRATE_BULKHEAD) {
        SDL_SetRenderDrawColor(r->sdl, 99, 80, 57, 255);
        SDL_RenderFillRect(r->sdl, &cell);
        for (int n = 0; n < 4; n++) {
            int yy = cell.y + n * 6;
            SDL_SetRenderDrawColor(r->sdl, 66, 54, 40, 255);
            SDL_RenderDrawLine(r->sdl, cell.x, yy, cell.x + 23, yy);
            int seam = cell.x + (n % 2 ? 6 : 18);
            SDL_RenderDrawLine(r->sdl, seam, yy, seam, yy + 5);
            SDL_SetRenderDrawColor(r->sdl, 124, 102, 73, 255);
            SDL_RenderDrawLine(r->sdl, cell.x + 3, yy + 2, cell.x + 16, yy + 2);
            SDL_SetRenderDrawColor(r->sdl, 49, 43, 34, 255);
            SDL_RenderDrawPoint(r->sdl, seam + 2, yy + 4);
        }
        if (tile == TILE_PIRATE_RAIL) {
            SDL_SetRenderDrawColor(r->sdl, 40, 31, 24, 255);
            int left = sea_at(map, x - 1, y);
            int right = sea_at(map, x + 1, y);
            int top = sea_at(map, x, y - 1);
            SDL_Rect rail = {cell.x, cell.y + (top ? 0 : 19), 24, 5};
            if (left || right) {
                rail = (SDL_Rect){cell.x + (right ? 19 : 0), cell.y, 5, 24};
            }
            SDL_RenderFillRect(r->sdl, &rail);
            SDL_SetRenderDrawColor(r->sdl, 166, 122, 72, 255);
            if (left || right) {
                SDL_RenderDrawLine(r->sdl, rail.x + 2, rail.y, rail.x + 2, rail.y + 23);
            } else {
                SDL_RenderDrawLine(r->sdl, rail.x, rail.y + 1, rail.x + 23, rail.y + 1);
            }
        } else if (tile == TILE_PIRATE_BULKHEAD) {
            SDL_SetRenderDrawColor(r->sdl, 48, 36, 26, 255);
            SDL_RenderFillRect(r->sdl, &cell);
            for (int n = 0; n < 4; n++) {
                SDL_SetRenderDrawColor(r->sdl, 87 + n % 2 * 10, 65, 40, 255);
                SDL_Rect plank = {cell.x + n * 6 + 1, cell.y + 3, 4, 18};
                SDL_RenderFillRect(r->sdl, &plank);
            }
            SDL_SetRenderDrawColor(r->sdl, 142, 108, 63, 255);
            SDL_RenderDrawLine(r->sdl, cell.x, cell.y + 2, cell.x + 23, cell.y + 2);
            SDL_SetRenderDrawColor(r->sdl, 31, 28, 23, 255);
            SDL_RenderDrawLine(r->sdl, cell.x, cell.y + 20, cell.x + 23, cell.y + 20);
        }
        return;
    }
    draw_sand(r, sx, sy, x, y);
    if (tile == TILE_PIRATE_SAND) {
        round_shore(r, map, sx, sy, x, y, 0);
    }
    if (tile == TILE_PIRATE_PALISADE) {
        for (int n = 0; n < 4; n++) {
            int xx = cell.x + n * 6;
            SDL_SetRenderDrawColor(r->sdl, 39, 31, 25, 255);
            SDL_Rect post = {xx, cell.y + 4, 6, 20};
            SDL_RenderFillRect(r->sdl, &post);
            SDL_SetRenderDrawColor(r->sdl, 112 + n % 2 * 12, 79, 44, 255);
            post.x++; post.w = 4; post.y += 2; post.h -= 2;
            SDL_RenderFillRect(r->sdl, &post);
            SDL_RenderDrawLine(r->sdl, xx + 1, cell.y + 4, xx + 3, cell.y);
            SDL_RenderDrawLine(r->sdl, xx + 3, cell.y, xx + 4, cell.y + 4);
        }
    } else if (tile == TILE_PIRATE_CRATE) {
        SDL_Rect crate = {cell.x + 2, cell.y + 2, 20, 20};
        SDL_SetRenderDrawColor(r->sdl, 51, 37, 26, 255);
        SDL_RenderFillRect(r->sdl, &crate);
        crate.x += 2; crate.y += 2; crate.w -= 4; crate.h -= 4;
        SDL_SetRenderDrawColor(r->sdl, 153, 103, 49, 255);
        SDL_RenderFillRect(r->sdl, &crate);
        SDL_SetRenderDrawColor(r->sdl, 82, 59, 35, 255);
        SDL_RenderDrawLine(r->sdl, crate.x, crate.y, crate.x + 15, crate.y + 15);
        SDL_RenderDrawLine(r->sdl, crate.x + 15, crate.y, crate.x, crate.y + 15);
    } else if (tile == TILE_PIRATE_CHEST) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){222, 177, 69, 255});
    }
}

static void draw_harbor_status(Renderer *r, const GameState *g) {
    int width = r->screen_w - INFO_PANEL_W;
    int x = width - 294;
    unsigned int state = g->pirate_cache[2].harbor_state;
    unsigned int claimed = g->pirate_cache[2].treasure_claimed;
    int plunder = !!(claimed & 1) + !!(claimed & 2) + !!(claimed & 4);
    int cargo = !!(state & PIRATE_CARGO_KEY) + !!(state & PIRATE_STRONGBOX);
    const char *zone = "QUAYSIDE";
    if (g->player.x >= 26 && g->player.x <= 34 && g->player.y >= 5 && g->player.y <= 21) {
        zone = g->player.y <= 13 ? "THE GULL / CARGO ROOM" : "THE GULL / MAIN DECK";
    } else if (g->player.x >= 41 && g->player.x <= 49 && g->player.y >= 5 && g->player.y <= 21) {
        zone = g->player.y <= 13 ? "THE NIGHTJAR / CARGO ROOM" : "THE NIGHTJAR / MAIN DECK";
    }
    SDL_Rect panel = {x, 8, 286, 74};
    SDL_SetRenderDrawColor(r->sdl, 32, 30, 25, 255);
    SDL_RenderFillRect(r->sdl, &panel);
    SDL_SetRenderDrawColor(r->sdl, 139, 115, 70, 255);
    SDL_RenderDrawRect(r->sdl, &panel);
    renderer_draw_text(r, "SMUGGLERS' PIERS", x + 12, 17, (SDL_Color){238, 207, 139, 255}, r->font_small);
    renderer_draw_text(r, zone, x + 12, 40, (SDL_Color){231, 222, 200, 255}, r->font_tiny);
    char status[64];
    SDL_snprintf(status, sizeof(status), "PLUNDER %d/3   CARGO %d/2   %s", plunder, cargo, state & PIRATE_CARGO_KEY ? "KEY +" : "KEY -");
    renderer_draw_text(r, status, x + 12, 59, (SDL_Color){179, 179, 149, 255}, r->font_tiny);
    const char *hint = pirate_interaction_hint(g);
    if (hint) {
        char action[96];
        SDL_snprintf(action, sizeof(action), "%s: %s", controls_key_name(g->key_bindings[CONTROL_MOVE_LEFT]), hint);
        int tw = 0;
        int th = 0;
        TTF_SizeUTF8(r->font_tiny, action, &tw, &th);
        int w = tw + 28;
        int y = r->tiles_y * TILE_SIZE - 44;
        SDL_Rect prompt = {(width - w) / 2, y, w, 32};
        SDL_SetRenderDrawColor(r->sdl, 32, 30, 25, 255);
        SDL_RenderFillRect(r->sdl, &prompt);
        SDL_SetRenderDrawColor(r->sdl, 191, 160, 94, 255);
        SDL_RenderDrawRect(r->sdl, &prompt);
        renderer_draw_text(r, action, prompt.x + 14, y + (32 - th) / 2, (SDL_Color){244, 222, 165, 255}, r->font_tiny);
    }
}

void draw_pirate_status(Renderer *r, const GameState *g, const Viewport *v) {
    int width = r->screen_w - INFO_PANEL_W;
    if (g->location != LOCATION_PIRATE_COAST || width < 328) {
        return;
    }
    if (g->level == 3) {
        draw_harbor_status(r, g);
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_CAPTAIN_VOSS || !viewport_is_visible(v, e->x, e->y)) {
            continue;
        }
        int x = width - 320;
        SDL_Rect panel = {x, 8, 312, 100};
        SDL_SetRenderDrawColor(r->sdl, 26, 27, 31, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 196, 155, 72, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        renderer_draw_text(r, "CAPTAIN VOSS", x + 12, 17, (SDL_Color){240, 205, 122, 255}, r->font_small);
        SDL_Rect bar = {x + 12, 39, 288, 8};
        SDL_SetRenderDrawColor(r->sdl, 67, 40, 35, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = e->max_hp ? 288 * e->hp / e->max_hp : 0;
        SDL_SetRenderDrawColor(r->sdl, 191, 66, 47, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        char status[64];
        if (e->frozen_turns) {
            SDL_snprintf(status, sizeof(status), "FROZEN: %d TURNS", e->frozen_turns);
        } else if (e->attack_phase) {
            SDL_snprintf(status, sizeof(status), "%s: %d TURNS", e->boss_attack == PIRATE_SABER ? "SABER SWEEP" : "BLADE WAVE", e->move_timer);
        } else {
            SDL_snprintf(status, sizeof(status), "%s", e->move_timer < 0 ? "RECOVERING: STRIKE NOW" : "WATCH FOR HIS NEXT ORDER");
        }
        renderer_draw_text(r, status, x + 12, 57, (SDL_Color){236, 225, 200, 255}, r->font_tiny);
        renderer_draw_text(r, "CLEAR THE MARKED DECK BEFORE IMPACT", x + 12, 79, (SDL_Color){236, 225, 200, 255}, r->font_tiny);
        break;
    }
}

void draw_pirate_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_PIRATE_COAST) {
        return;
    }
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *enemy = &g->enemies[i];
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                if (pirate_marks(g, enemy, x, y)) {
                    SDL_Rect rect = {viewport_to_screen_x(v, x) * TILE_SIZE, viewport_to_screen_y(v, y) * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    SDL_SetRenderDrawColor(r->sdl, 238, 78, 34, 110);
                    SDL_RenderFillRect(r->sdl, &rect);
                    SDL_SetRenderDrawColor(r->sdl, 255, 204, 90, 240);
                    SDL_RenderDrawRect(r->sdl, &rect);
                    char count[8];
                    SDL_snprintf(count, sizeof(count), "%d", enemy->move_timer);
                    renderer_draw_text(r, count, rect.x + 8, rect.y + 5, (SDL_Color){255, 239, 172, 255}, r->font_tiny);
                }
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_NONE);
}
