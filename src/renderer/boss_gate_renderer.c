#include "boss_gate_renderer.h"
#include "sprites.h"
#include "../game/approach.h"
#include "../game/elderwood.h"
#include "info_panel.h"
#include <stdio.h>
#include <stdlib.h>

static int theme(Location area) {
    switch (area) {
        case LOCATION_DUNGEON: return 0;
        case LOCATION_FOREST: return 1;
        case LOCATION_MOUNTAINS: return 2;
        case LOCATION_SWAMP: return 3;
        case LOCATION_ELDERWOOD: return 4;
        case LOCATION_CATACOMBS: return 5;
        default: return 0;
    }
}

void boss_gate_draw_bank(Renderer *r, const GameState *g, int sx, int sy, int x, int y) {
    const TownApproach *route = approach_route(g);
    Location area = northern_approach(g) ? y > 22 ? LOCATION_ELDERWOOD : LOCATION_CATACOMBS : route ? route->area : LOCATION_FOREST;
    unsigned int seed = (unsigned int)x * 73856093u ^ (unsigned int)y * 19349663u;
    int neighbors = 0;
    if (y > 0 && g->map.tiles[y - 1][x] == TILE_ROAD_BANK) {
        neighbors |= 1;
    }
    if (x + 1 < APPROACH_W && g->map.tiles[y][x + 1] == TILE_ROAD_BANK) {
        neighbors |= 2;
    }
    if (y + 1 < APPROACH_H && g->map.tiles[y + 1][x] == TILE_ROAD_BANK) {
        neighbors |= 4;
    }
    if (x > 0 && g->map.tiles[y][x - 1] == TILE_ROAD_BANK) {
        neighbors |= 8;
    }
    SDL_Rect source = {(neighbors * 4 + (int)(seed % 4u)) * TILE_SIZE, theme(area) * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    SDL_Rect dest = {sx * TILE_SIZE, sy * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    SDL_RenderCopy(r->sdl, r->road_bank_texture, &source, &dest);
}

void boss_gate_draw_banks(Renderer *r, const GameState *g, const Viewport *v) {
    RoadCheckpoint points[2];
    int count = boss_gate_checkpoints(g, points);
    const TownApproach *route = approach_route(g);
    // Roads with scenery draw their banks as that scenery instead.
    if (!count || (route && approach_scenery(route->area) != TILE_WALL)) {
        return;
    }
    // Connected bank edges share a continuous canopy or rock face.
    for (int y = v->cam_y; y < v->cam_y + v->tiles_y && y < APPROACH_H; y++) {
        for (int x = v->cam_x; x < v->cam_x + v->tiles_x && x < APPROACH_W; x++) {
            if (g->map.tiles[y][x] == TILE_ROAD_BANK) {
                boss_gate_draw_bank(r, g, viewport_to_screen_x(v, x), viewport_to_screen_y(v, y), x, y);
            }
        }
    }
}

void boss_gate_draw(Renderer *r, const GameState *g, const Viewport *v) {
    RoadCheckpoint points[2];
    int count = boss_gate_checkpoints(g, points);
    for (int n = 0; n < count; n++) {
        RoadCheckpoint point = points[n];
        int open = boss_gate_open(g, point.area);
        int x = viewport_to_screen_x(v, point.x) * TILE_SIZE;
        int y = viewport_to_screen_y(v, point.y) * TILE_SIZE;
        SDL_Rect source = {(point.vertical * 2 + open) * 72, theme(point.area) * 96, 72, 96};
        SDL_Rect dest = {x - 24, y - 48, 72, 96};
        SDL_RenderCopy(r->sdl, r->road_checkpoint_texture, &source, &dest);
        // Shape and words carry the status as well as its color.
        SDL_Color color = open ? (SDL_Color){143, 202, 138, 255} : (SDL_Color){225, 169, 99, 255};
        if (!open) {
            SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, 255);
            SDL_RenderDrawLine(r->sdl, x + 8, y - 37, x + 16, y - 29);
            SDL_RenderDrawLine(r->sdl, x + 8, y - 29, x + 16, y - 37);
        }
        if (abs(g->player.x - point.x) + abs(g->player.y - point.y) <= 5) {
            const BossGateRule *rule = boss_gate_rule(point.area);
            char text[100];
            snprintf(text, sizeof(text), open ? "PASSAGE SECURED" : "PASSAGE BLOCKED: %s", rule->boss);
            int width;
            TTF_SizeText(r->font_tiny, text, &width, NULL);
            int left = x + 12 - width / 2;
            int maximum = r->screen_w - INFO_PANEL_W - width - 6;
            left = left < 6 ? 6 : left > maximum ? maximum : left;
            SDL_Rect backing = {left - 4, y - 69, width + 8, 20};
            SDL_SetRenderDrawColor(r->sdl, 14, 23, 20, 255);
            SDL_RenderFillRect(r->sdl, &backing);
            renderer_draw_text(r, text, left, y - 66, color, r->font_tiny);
        }
    }
}
