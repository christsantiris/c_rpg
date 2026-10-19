#include "regional_terrain_renderer.h"
#include "../game/regional_terrain.h"
#include "../game/landscapes.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include "controls_renderer.h"
#include <stdlib.h>
#include <stdio.h>

static void draw_landscape_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile) {
    int desert = tile == TILE_DESERT_OASIS || tile == TILE_DESERT_OASIS_DRY;
    int dry = tile == TILE_DESERT_OASIS_DRY;
    SDL_Color accent = desert ? (SDL_Color){97, 206, 196, 255} : tile == TILE_MOONVEIL_WAYSTONE_A ?
        (SDL_Color){180, 177, 255, 255} : (SDL_Color){246, 182, 220, 255};
    if (desert) {
        draw_desert_floor(r, sx, sy, x, y);
        draw_world_prop(r, sx, sy, dry ? PROP_INTAKE : PROP_SPRING, dry ? (SDL_Color){167, 139, 108, 255} : accent);
    } else {
        draw_moonveil_circle(r, sx, sy, x, y);
        draw_world_prop(r, sx, sy, PROP_ARCH, accent);
    }
    const char *label = desert ? (dry ? "DRY" : "WELL") : tile == TILE_MOONVEIL_WAYSTONE_A ? "I" : "II";
    renderer_draw_text(r, label, sx * TILE_SIZE + (desert ? 1 : 8), sy * TILE_SIZE - 10, accent, r->font_tiny);
    if (abs(g->player.x - x) + abs(g->player.y - y) <= 1) {
        char hint[64];
        snprintf(hint, sizeof(hint), "%s: %s", controls_key_name(g->key_bindings[CONTROL_MOVE_LEFT]), desert ? (dry ? "WELL EMPTY" : "DRINK WATER") : "USE MATCHING STONE");
        renderer_draw_text(r, hint, sx * TILE_SIZE - 50, sy * TILE_SIZE - 24, accent, r->font_tiny);
    }
}

void draw_regional_terrain_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile) {
    if (landscape_tile(tile)) {
        draw_landscape_tile(r, g, sx, sy, x, y, tile);
        return;
    }
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    SDL_Color blue = {122, 220, 227, 255};
    SDL_Color amber = {255, 191, 101, 255};
    if (tile <= TILE_ASHEN_VALVE || tile == TILE_ASHEN_BASALT_CROSSING || tile == TILE_ASHEN_VALVE_DIVERTED) {
        draw_ashen_ruin(r, sx, sy, x, y);
        SDL_Color green = {139, 210, 153, 255};
        if (tile == TILE_ASHEN_VALVE || tile == TILE_ASHEN_VALVE_DIVERTED) {
            int diverted = tile == TILE_ASHEN_VALVE_DIVERTED;
            draw_world_prop(r, sx, sy, PROP_LEVER, diverted ? green : blue);
            if (diverted) {
                renderer_draw_text(r, "ON", px + 5, py + 4, green, r->font_tiny);
            }
        } else if (tile == TILE_ASHEN_BASALT_CROSSING) {
            SDL_SetRenderDrawColor(r->sdl, green.r, green.g, green.b, 255);
            SDL_RenderDrawLine(r->sdl, px + 8, py + 12, px + 11, py + 15);
            SDL_RenderDrawLine(r->sdl, px + 11, py + 15, px + 17, py + 8);
        } else {
            int hot = tile >= TILE_ASHEN_ACTIVE_2;
            int warning = tile == TILE_ASHEN_WARN_2 || tile == TILE_ASHEN_WARN_1;
            SDL_Color color = hot ? (SDL_Color){255, 103, 49, 255} : warning ? amber : blue;
            SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, 255);
            SDL_Rect outline = {px + 2, py + 2, 20, 20};
            SDL_RenderDrawRect(r->sdl, &outline);
            SDL_RenderDrawLine(r->sdl, px + 3, py + 17, px + 10, py + 14);
            SDL_RenderDrawLine(r->sdl, px + 10, py + 14, px + 20, py + 19);
            if (hot) {
                draw_world_prop(r, sx, sy, PROP_LOW_FLAME_1, color);
            }
            char text[8];
            int turns = hot ? TILE_ASHEN_ACTIVE_1 - tile + 1 : warning ? TILE_ASHEN_WARN_1 - tile + 1 : TILE_ASHEN_COLD_1 - tile + 1;
            snprintf(text, sizeof(text), "%c%d", hot ? 'H' : warning ? '!' : 'C', turns);
            renderer_draw_text(r, text, px + 4, py + 4, color, r->font_tiny);
        }
    } else {
        draw_glassdeep_ruin(r, sx, sy, x, y);
        if (tile == TILE_GLASS_SEAL_CLOSED || tile == TILE_GLASS_SEAL_OPEN) {
            draw_world_prop(r, sx, sy, tile == TILE_GLASS_SEAL_CLOSED ? PROP_GATE : PROP_GATE_OPEN, blue);
        } else if (tile == TILE_GLASS_SOURCE || tile == TILE_GLASS_RECEIVER) {
            draw_world_prop(r, sx, sy, tile == TILE_GLASS_SOURCE ? PROP_CRYSTAL : PROP_INTAKE, blue);
            renderer_draw_text(r, tile == TILE_GLASS_SOURCE ? "L" : "R", px + 8, py + 3, (SDL_Color){236, 237, 255, 255}, r->font_tiny);
        } else {
            draw_world_prop(r, sx, sy, PROP_CRYSTAL, (SDL_Color){170, 146, 231, 255});
            static const int dx[4] = {0, 1, 0, -1};
            static const int dy[4] = {-1, 0, 1, 0};
            int direction = tile - TILE_GLASS_FACE_N;
            int tip_x = px + 12 + dx[direction] * 9;
            int tip_y = py + 12 + dy[direction] * 9;
            SDL_SetRenderDrawColor(r->sdl, 235, 246, 255, 255);
            SDL_RenderDrawLine(r->sdl, px + 12, py + 12, tip_x, tip_y);
            SDL_RenderDrawLine(r->sdl, tip_x, tip_y, tip_x - dx[direction] * 4 - dy[direction] * 3, tip_y - dy[direction] * 4 + dx[direction] * 3);
            SDL_RenderDrawLine(r->sdl, tip_x, tip_y, tip_x - dx[direction] * 4 + dy[direction] * 3, tip_y - dy[direction] * 4 - dx[direction] * 3);
        }
    }
    if ((tile == TILE_ASHEN_VALVE || (tile >= TILE_GLASS_FACE_N && tile <= TILE_GLASS_FACE_W)) &&
        abs(g->player.x - x) + abs(g->player.y - y) <= 1) {
        char hint[64];
        snprintf(hint, sizeof(hint), "%s: %s", controls_key_name(g->key_bindings[CONTROL_MOVE_LEFT]), tile == TILE_ASHEN_VALVE ? "DIVERT LAVA" : "TURN LIGHT");
        renderer_draw_text(r, hint, px - 40, py - 16, blue, r->font_tiny);
    }
}

void draw_regional_terrain_light(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_GLASSDEEP) {
        return;
    }
    for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
        for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
            if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
                continue;
            }
            TileType tile = g->map.tiles[y][x];
            if (tile >= TILE_GLASS_FACE_N && tile <= TILE_GLASS_SEAL_OPEN) {
                continue;
            }
            int beam = regional_terrain_light(g, x, y);
            if (!beam) {
                continue;
            }
            int px = viewport_to_screen_x(v, x) * TILE_SIZE;
            int py = viewport_to_screen_y(v, y) * TILE_SIZE;
            SDL_SetRenderDrawColor(r->sdl, 47, 118, 154, 255);
            SDL_Rect glow = beam == 1 ? (SDL_Rect){px, py + 9, TILE_SIZE, 5} : (SDL_Rect){px + 9, py, 5, TILE_SIZE};
            SDL_RenderFillRect(r->sdl, &glow);
            SDL_SetRenderDrawColor(r->sdl, 167, 239, 250, 255);
            if (beam == 1) {
                SDL_RenderDrawLine(r->sdl, px, py + 11, px + TILE_SIZE - 1, py + 11);
            } else {
                SDL_RenderDrawLine(r->sdl, px + 11, py, px + 11, py + TILE_SIZE - 1);
            }
        }
    }
}

void draw_regional_terrain_status(Renderer *r, const GameState *g) {
    if (g->location != LOCATION_ASHEN && g->location != LOCATION_GLASSDEEP &&
        g->location != LOCATION_FROSTFELL && g->location != LOCATION_DESERT && g->location != LOCATION_MOONVEIL &&
        g->location != LOCATION_SWAMP && g->location != LOCATION_DRAGONSPINE) {
        return;
    }
    if (g->location == LOCATION_SWAMP ? g->level == SWAMP_BOSS_LEVEL : g->level == 5) {
        return;
    }
    int width = r->screen_w - INFO_PANEL_W;
    int x = width - 320;
    int y = width >= 440 ? 8 : 76;
    if (width < 328 || r->tiles_y * TILE_SIZE < y + 80) {
        return;
    }
    SDL_Rect panel = {x, y, 312, 72};
    SDL_SetRenderDrawColor(r->sdl, 22, 23, 35, 255);
    SDL_RenderFillRect(r->sdl, &panel);
    SDL_SetRenderDrawColor(r->sdl, 105, 125, 162, 255);
    SDL_RenderDrawRect(r->sdl, &panel);
    const char *title = "VOLCANIC CROSSINGS";
    const char *line1 = "C: COOL   !: WARNING   H: HOT";
    const char *line2 = "VALVES MAKE LASTING PATHS";
    if (g->location == LOCATION_GLASSDEEP) {
        title = "CRYSTAL GALLERIES";
        line1 = "TURN FACE: AIM NORTH AT R";
        line2 = "SIDE GALLERIES BYPASS THE SEAL";
    } else if (g->location == LOCATION_FROSTFELL) {
        title = "GLACIER RIDGES";
        line1 = "SLICK ICE SLIDES TO SNOW";
        line2 = "BRITTLE SHORTCUTS BREAK BEHIND YOU";
    } else if (g->location == LOCATION_DESERT) {
        title = "SANDSTONE MESA FIELDS";
        line1 = "DENSE PATROLS; ROCKS BLOCK FIRE";
        line2 = "WELLS: 25% HP / MP, ONCE PER TRIP";
    } else if (g->location == LOCATION_SWAMP) {
        title = "BLACKWATER ISLANDS";
        line1 = "REEDS HIDE FROM DISTANT FOES";
        line2 = "NEARBY / WOUNDED FOES STILL FOLLOW";
    } else if (g->location == LOCATION_DRAGONSPINE) {
        title = "RAVINES AND CLIFF LEDGES";
        line1 = "PUSH BOULDERS FOR MOVABLE COVER";
        line2 = "KEEP MULTIPLE ROUTES OPEN";
    } else if (g->location == LOCATION_MOONVEIL) {
        title = "MOONLIT GARDEN LOOPS";
        line1 = "WAYSTONES: I TO I, II TO II";
        line2 = "USE INTERACT TO CROSS THE GARDEN";
    }
    SDL_Color text = {217, 224, 237, 255};
    renderer_draw_text(r, title, x + 10, y + 9, (SDL_Color){142, 215, 231, 255}, r->font_small);
    renderer_draw_text(r, line1, x + 10, y + 32, text, r->font_tiny);
    renderer_draw_text(r, line2, x + 10, y + 50, text, r->font_tiny);
}
