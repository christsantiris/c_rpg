#include "castle_renderer.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include "item_icons.h"
#include "npc_renderer.h"
#include "../game/royal_arcanist.h"
#include "../game/lord_veyr.h"
#include "../game/castle_gatehouse.h"
#include "../game/malakar.h"
#include "../game/castle_court.h"
#include "../game/castle_chapel.h"

static void rect(Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_SetRenderDrawColor(r->sdl, c.r, c.g, c.b, c.a);
    SDL_Rect box = {x, y, w, h};
    SDL_RenderFillRect(r->sdl, &box);
}

static int carpet(const Map *m, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
        return 0;
    }
    TileType tile = m->tiles[y][x];
    return tile == TILE_CASTLE_CARPET || tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN ||
        tile == TILE_CASTLE_TRAP_OPEN || tile == TILE_CASTLE_GATE || tile == TILE_CASTLE_GATE_OPEN || tile == TILE_CASTLE_FIRE_RUNE;
}

void castle_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile) {
    if (tile == TILE_CASTLE_TRAP_HIDDEN) {
        tile = TILE_CASTLE_FLOOR;
    }
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    int shade = (x * 17 + y * 11) % 9;
    SDL_Color stone = {54 + shade, 49 + shade, 65 + shade, 255};
    if (tile == TILE_CASTLE_WALL) {
        stone = (SDL_Color){30 + shade, 27 + shade, 39 + shade, 255};
    }
    rect(r, px, py, 24, 24, (SDL_Color){24, 23, 32, 255});
    rect(r, px + 1, py + 1, 22, 22, stone);
    rect(r, px + 2, py + 2, 20, 1, tile == TILE_CASTLE_WALL ? (SDL_Color){47, 42, 56, 255} : (SDL_Color){79, 72, 86, 255});
    if (tile == TILE_CASTLE_FLOOR || tile == TILE_CASTLE_WARD || tile == TILE_CASTLE_WARD_SPENT || tile == TILE_CASTLE_MUSTER || tile == TILE_CASTLE_CHAMBERLAIN) {
        rect(r, px + 2, py + 11, 20, 1, (SDL_Color){37, 34, 45, 255});
        rect(r, px + 7 + (x % 2) * 5, py + 2, 1, 9, (SDL_Color){37, 34, 45, 255});
        rect(r, px + 12 - (x % 2) * 5, py + 12, 1, 10, (SDL_Color){37, 34, 45, 255});
        rect(r, px + 3 + (x * 7 + y) % 15, py + 4 + (y * 3 + x) % 15, 2, 1, (SDL_Color){89, 80, 96, 255});
    }
    if (tile == TILE_CASTLE_WALL) {
        rect(r, px, py + 11, 24, 2, (SDL_Color){22, 20, 28, 255});
        rect(r, px + 11, py, 2, 11, (SDL_Color){22, 20, 28, 255});
        rect(r, px + 4, py + 13, 2, 11, (SDL_Color){22, 20, 28, 255});
        // Stained-glass accents and gold masonry break up long walls.
        if ((x + y * 3) % 13 == 0 && y > 0 && map_is_walkable(&g->map, x, y + 1)) {
            rect(r, px + 6, py + 3, 12, 18, (SDL_Color){126, 99, 48, 255});
            rect(r, px + 8, py + 5, 8, 14, (SDL_Color){70, 43, 110, 255});
            rect(r, px + 11, py + 6, 2, 12, (SDL_Color){156, 104, 205, 255});
        } else if ((x + y) % 7 == 0 && map_is_walkable(&g->map, x, y + 1)) {
            int flicker = (SDL_GetTicks() / AMBIENT_FRAME_MS + x) % 2;
            rect(r, px + 10, py + 10, 4, 11, (SDL_Color){88, 61, 44, 255});
            rect(r, px + 8, py + 6, 8, 7, (SDL_Color){222, 103 + flicker * 20, 36, 255});
            rect(r, px + 10, py + 4 + flicker, 4, 7, (SDL_Color){255, 212, 91, 255});
        }
        return;
    }
    if (tile == TILE_CASTLE_CARPET || tile == TILE_CASTLE_FIRE_RUNE) {
        rect(r, px, py, 24, 24, (SDL_Color){89 + shade, 27, 41, 255});
        if (!carpet(&g->map, x - 1, y)) {
            rect(r, px + 1, py, 2, 24, (SDL_Color){155, 111, 54, 255});
        }
        if (!carpet(&g->map, x + 1, y)) {
            rect(r, px + 21, py, 2, 24, (SDL_Color){155, 111, 54, 255});
        }
        if (!carpet(&g->map, x, y - 1)) {
            rect(r, px, py + 1, 24, 2, (SDL_Color){155, 111, 54, 255});
        }
        if (!carpet(&g->map, x, y + 1)) {
            rect(r, px, py + 21, 24, 2, (SDL_Color){155, 111, 54, 255});
        }
        if ((x + y) % 5 == 0) {
            rect(r, px + 10, py + 10, 4, 4, (SDL_Color){128, 61, 53, 255});
        }
    }
    if (tile == TILE_CASTLE_FIRE_RUNE) {
        if (g->level == 6 && g->castle_fire_phase[5] == 3) {
            draw_world_prop(r, sx, sy, PROP_LOW_FLAME_1, (SDL_Color){255, 161, 57, 255});
        }
        SDL_SetRenderDrawColor(r->sdl, 202, 137, 68, 255);
        SDL_RenderDrawLine(r->sdl, px + 6, py + 16, px + 12, py + 6);
        SDL_RenderDrawLine(r->sdl, px + 12, py + 6, px + 18, py + 16);
        SDL_RenderDrawLine(r->sdl, px + 6, py + 16, px + 18, py + 16);
        rect(r, px + 11, py + 10, 2, 4, (SDL_Color){240, 188, 91, 255});
    } else if (tile == TILE_CASTLE_WARD || tile == TILE_CASTLE_WARD_SPENT) {
        SDL_Color orb = tile == TILE_CASTLE_WARD ? (SDL_Color){101, 218, 239, 255} : (SDL_Color){77, 78, 94, 255};
        draw_world_prop(r, sx, sy, g->level == 2 ? PROP_FURNACE : PROP_CRYSTAL, orb);
        if (g->level == 2 && tile == TILE_CASTLE_WARD) {
            draw_world_prop(r, sx, sy, PROP_GOBLET, (SDL_Color){122, 247, 176, 255});
        }
    } else if (tile == TILE_CASTLE_MUSTER) {
        draw_world_prop(r, sx, sy, PROP_MUSTER, (SDL_Color){222, 183, 104, 255});
    } else if (tile == TILE_CASTLE_TABLE) {
        WorldPropArt art = PROP_BANQUET_TABLE;
        if (g->level == 1 && x < 23 && y >= 25 && y < 38) {
            art = PROP_STRAW;
        } else if (g->level == 2) {
            art = y < 20 ? PROP_TABLE : y < 38 ? PROP_STRAW : PROP_FURNACE;
        } else if (g->level == 3 && (x >= 48 || y >= 47)) {
            art = PROP_STRAW;
        } else if (g->level == 4 && y < 40) {
            art = PROP_TABLE;
        }
        draw_world_prop(r, sx, sy, art, (SDL_Color){179, 150, 105, 255});
        if (g->level == 4 && y < 40 && (x == 29 || x == 45)) {
            draw_world_prop(r, sx, sy, PROP_CANDLE, (SDL_Color){230, 217, 163, 255});
        }
        if (g->level == 2 && y < 20) {
            draw_world_prop(r, sx, sy, y % 2 ? PROP_GOBLET : PROP_SPICES, (SDL_Color){122, 233, 183, 255});
        }
        if (g->level == 1 && y == 40) {
            draw_world_prop(r, sx, sy, x % 2 ? PROP_BREAD : PROP_MEAT, (SDL_Color){213, 170, 99, 255});
        }
    } else if (tile == TILE_CASTLE_BOOKCASE) {
        if (g->level == 3 && y == 46) {
            draw_world_prop(r, sx, sy, PROP_BAG, (SDL_Color){192, 177, 143, 255});
        } else if (g->level == 2 && x == 54 && y == 8 && (g->castle_minibosses & 1)) {
            draw_world_prop(r, sx, sy, PROP_VAULT_DOOR, (SDL_Color){155, 167, 101, 255});
        } else if (g->level == 2 && y == 29) {
            draw_world_prop(r, sx, sy, PROP_BOOKCASE, (SDL_Color){142, 176, 121, 255});
            draw_world_prop(r, sx, sy, x % 2 ? PROP_GOBLET : PROP_SPICES, (SDL_Color){122, 233, 183, 255});
        } else if (g->level == 1 && y == 45) {
            if (x < 19) {
                draw_world_prop(r, sx, sy, PROP_BAG, (SDL_Color){170, 116, 58, 255});
            } else {
                draw_world_prop(r, sx, sy, PROP_TABLE, (SDL_Color){170, 116, 58, 255});
                Item item = x % 2 ? item_make_kite_shield() : item_make_long_sword();
                draw_icon_item(r, px, py, &item);
            }
        } else {
            draw_world_prop(r, sx, sy, PROP_BOOKCASE, (SDL_Color){170, 116, 58, 255});
        }
    } else if (tile == TILE_CASTLE_THRONE) {
        draw_world_prop(r, sx, sy, PROP_THRONE, (SDL_Color){224, 177, 82, 255});
    } else if (tile == TILE_CASTLE_PILLAR) {
        WorldPropArt art = g->level == 2 ? PROP_CRYSTAL : g->level == 4 && x < 20 && y < 40 ? PROP_RELIC : PROP_PILLAR;
        draw_world_prop(r, sx, sy, art, g->level == 2 ? (SDL_Color){94, 196, 159, 255} : (SDL_Color){136, 111, 63, 255});
    } else if (tile == TILE_CASTLE_BANNER) {
        draw_world_prop(r, sx, sy, PROP_BANNER, (SDL_Color){198, 159, 75, 255});
    } else if (tile == TILE_CASTLE_CHAMBERLAIN) {
        npc_draw(r, sx, sy, NPC_ART_HADRIN, 0);
    } else if (tile == TILE_CASTLE_GATE || tile == TILE_CASTLE_GATE_OPEN) {
        draw_world_prop(r, sx, sy, tile == TILE_CASTLE_GATE ? PROP_GATE : PROP_GATE_OPEN, (SDL_Color){156, 119, 65, 255});
    } else if (tile == TILE_CASTLE_LEVER) {
        draw_world_prop(r, sx, sy, PROP_LEVER, (SDL_Color){159, 45, 55, 255});
    } else if (tile == TILE_CASTLE_TRAP_OPEN) {
        draw_world_prop(r, sx, sy, PROP_HATCH, (SDL_Color){199, 143, 60, 255});
    } else if (tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN) {
        draw_world_prop(r, sx, sy, tile == TILE_STAIRS_UP ? PROP_STAIRS_UP : PROP_STAIRS_DOWN, (SDL_Color){181, 146, 91, 255});
        renderer_draw_text(r, tile == TILE_STAIRS_UP ? "^" : "v", px + 8, py + 6, (SDL_Color){245, 206, 109, 255}, r->font_tiny);
    }
    if (g->level == 2 && x == 53 && y == 55 && g->castellan_reward_state == 1) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){218, 180, 96, 255});
        renderer_draw_text(r, "BULWARK: A", px - 16, py - 12, (SDL_Color){255, 224, 160, 255}, r->font_tiny);
    }
    if (g->level == 2 && tile == TILE_CASTLE_LEVER && y == 51) {
        renderer_draw_text(r, x == 37 ? "WEST: A" : "EAST: A", px - 10, py - 12, (SDL_Color){255, 224, 160, 255}, r->font_tiny);
    }
    if (g->level == 4 && y == 54 && (x == 37 || x == 53) && (tile == TILE_CASTLE_WARD || tile == TILE_CASTLE_WARD_SPENT)) {
        char text[16];
        snprintf(text, sizeof(text), "%s: %s", x == 37 ? "WEST" : "EAST", tile == TILE_CASTLE_WARD ? "A" : "OFF");
        renderer_draw_text(r, text, px - 16, py - 12, (SDL_Color){174, 236, 245, 255}, r->font_tiny);
    }
    if (g->level == 6 && y == 54 && (x == 37 || x == 53) && (tile == TILE_CASTLE_WARD || tile == TILE_CASTLE_WARD_SPENT)) {
        renderer_draw_text(r, tile == TILE_CASTLE_WARD ? (x == 37 ? "WEST: A" : "EAST: A") : "WARD: OFF", px - 16, py - 12, (SDL_Color){241, 216, 145, 255}, r->font_tiny);
    }
    if (g->level == 4 && x == 53 && y == 56 && g->arcanist_reward_state == 1) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){163, 140, 214, 255});
        renderer_draw_text(r, "VESTMENTS: A", px - 28, py - 12, (SDL_Color){234, 218, 255, 255}, r->font_tiny);
    }
}

static void gatehouse_label(Renderer *r, const GameState *g, const Viewport *v, const char *text, int x, int y) {
    if (!r->font_tiny || !map_is_explored(&g->map, x, y) || !viewport_is_visible(v, x, y)) {
        return;
    }
    int width = 0;
    int height = 0;
    TTF_SizeText(r->font_tiny, text, &width, &height);
    int px = viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2;
    int py = viewport_to_screen_y(v, y) * TILE_SIZE - height - 2;
    float scale_x;
    float scale_y;
    SDL_RenderGetScale(r->sdl, &scale_x, &scale_y);
    int right = (r->screen_w - INFO_PANEL_W) / scale_x - width - 3;
    int bottom = r->tiles_y * TILE_SIZE / scale_y - height - 3;
    px = px < 3 ? 3 : px > right ? right : px;
    py = py < 88 ? 88 : py > bottom ? bottom : py;
    rect(r, px - 2, py - 1, width + 4, height + 2, (SDL_Color){24, 23, 32, 255});
    renderer_draw_text(r, text, px, py, (SDL_Color){241, 216, 145, 255}, r->font_tiny);
}

void castle_draw_gatehouse(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 1) {
        return;
    }
    static const char *names[MAX_ROOMS] = {"ENTRANCE HALL", "EAST WATCH", "BARRACKS", "MESS HALL", "KITCHEN", "UPPER STAIR", "STORES", "ARMORY", "SERVICE CONTROL", "GUARD GALLERY"};
    for (int i = 0; i < g->map.room_count; i++) {
        if (i == 8) {
            continue;
        }
        const Room *room = &g->map.rooms[i];
        gatehouse_label(r, g, v, names[i], room->x + room->w / 2, room->y + 1);
    }
    gatehouse_label(r, g, v, "DOWN: CASTLE GROUNDS", g->map.stairs_up_x, g->map.stairs_up_y);
    gatehouse_label(r, g, v, "UP: alchemy WING", g->map.stairs_down_x, g->map.stairs_down_y);
    gatehouse_label(r, g, v, "SERVICE LEVER: A", GATEHOUSE_LEVER_X, GATEHOUSE_LEVER_Y);
    gatehouse_label(r, g, v, g->map.tiles[GATEHOUSE_SHORTCUT_Y][GATEHOUSE_SHORTCUT_X] == TILE_CASTLE_GATE ? "SERVICE GATE: SEALED" : "SERVICE GATE: OPEN", GATEHOUSE_SHORTCUT_X, GATEHOUSE_SHORTCUT_Y);
}

void castle_draw_alchemy(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 2) {
        return;
    }
    static const char *names[MAX_ROOMS] = {"RECEIVING HALL", "ROYAL LABORATORY", "SPECIMEN ROOM", "INFIRMARY", "REAGENT VAULT", "UPPER STAIR", "PRIVATE STUDY", "DISTILLERY", "LAB ANTECHAMBER", "EXPERIMENT ROOM"};
    for (int i = 0; i < g->map.room_count; i++) {
        if (i == 6) {
            continue;
        }
        const Room *room = &g->map.rooms[i];
        gatehouse_label(r, g, v, names[i], room->x + room->w / 2, room->y + 1);
    }
    gatehouse_label(r, g, v, "DOWN: GATEHOUSE", 15, 11);
    gatehouse_label(r, g, v, "UP: COURT AND HOUSEHOLD", 45, 55);
    gatehouse_label(r, g, v, "LAB SEAL", 35, 22);
    gatehouse_label(r, g, v, "CONDENSER: A", 38, 16);
    gatehouse_label(r, g, v, "CONDENSER: A", 51, 16);
    gatehouse_label(r, g, v, "PRIVATE LEDGER: A", 14, 46);
    gatehouse_label(r, g, v, "SEALED ROYAL PASSAGE: A", 54, 8);
}

void castle_draw_court(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 3) {
        return;
    }
    static const char *names[MAX_ROOMS] = {"RECEIVING HALL", "GREAT HALL", "WEST GALLERY", "COUNCIL CHAMBER", "GUEST CHAMBERS", "UPPER STAIR", "SERVANTS' QUARTERS", "LINEN STORE", "HOUSEHOLD OFFICE", "EAST GALLERY"};
    for (int i = 0; i < g->map.room_count; i++) {
        if (i == 3 || i == 8) {
            continue;
        }
        const Room *room = &g->map.rooms[i];
        gatehouse_label(r, g, v, names[i], room->x + room->w / 2, room->y + 1);
    }
    gatehouse_label(r, g, v, "DOWN: ALCHEMY WING", 15, 11);
    gatehouse_label(r, g, v, "UP: CROWN CHAPEL", 45, 55);
    gatehouse_label(r, g, v, "COUNCIL ORDERS: A", 29, 6);
    gatehouse_label(r, g, v, "HOUSE LEVER: A", COURT_LEVER_X, COURT_LEVER_Y);
    gatehouse_label(r, g, v, g->map.tiles[COURT_GATE_Y][COURT_GATE_X] == TILE_CASTLE_GATE ? "HOUSE GATE: SEALED" : "HOUSE GATE: OPEN", COURT_GATE_X, COURT_GATE_Y);
    for (int y = v->cam_y; y < v->cam_y + v->tiles_y && y < CASTLE_H; y++) {
        for (int x = v->cam_x; x < v->cam_x + v->tiles_x && x < CASTLE_W; x++) {
            if (g->map.tiles[y][x] == TILE_CASTLE_CHAMBERLAIN) {
                char text[48];
                snprintf(text, sizeof(text), "EDRAS: %s", SDL_GetScancodeName((SDL_Scancode)g->key_bindings[CONTROL_TALK]));
                gatehouse_label(r, g, v, text, x, y);
            }
        }
    }
}

void castle_draw_chapel(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 4) {
        return;
    }
    static const char *names[MAX_ROOMS] = {"ENTRANCE HALL", "CROWN NAVE", "RELIQUARY", "VESTRY", "RUINED CLOISTER", "WARD SANCTUARY", "SACRISTY", "PRAYER CELL", "CHOIR GALLERY", "ROYAL GALLERY"};
    for (int i = 0; i < g->map.room_count; i++) {
        if (i == 2 || i == 6) {
            continue;
        }
        const Room *room = &g->map.rooms[i];
        gatehouse_label(r, g, v, names[i], room->x + room->w / 2, room->y + 1);
    }
    gatehouse_label(r, g, v, "DOWN: COURT AND HOUSEHOLD", 15, 11);
    gatehouse_label(r, g, v, "UP: ROYAL ARCHIVES", 45, 55);
    gatehouse_label(r, g, v, "CHAPEL REGISTER: A", 12, 26);
    gatehouse_label(r, g, v, "ROYAL LEVER: A", CHAPEL_LEVER_X, CHAPEL_LEVER_Y);
    gatehouse_label(r, g, v, g->map.tiles[CHAPEL_GATE_Y][CHAPEL_GATE_X] == TILE_CASTLE_GATE ? "ROYAL PASSAGE: SEALED" : "ROYAL PASSAGE: OPEN", CHAPEL_GATE_X, CHAPEL_GATE_Y);
}

void castle_draw_malakar_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 2) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_MALAKAR || e->max_hp <= 0 || !viewport_is_visible(v, e->x, e->y)) {
            continue;
        }
        int width = r->screen_w - INFO_PANEL_W - 16;
        width = width > 380 ? 380 : width;
        int x = (r->screen_w - INFO_PANEL_W - width) / 2;
        rect(r, x, 84, width, 76, (SDL_Color){20, 39, 35, 255});
        char text[96];
        snprintf(text, sizeof(text), "MALAKAR: %d/%d HP", e->hp, e->max_hp);
        renderer_draw_text(r, text, x + 8, 90, (SDL_Color){171, 245, 195, 255}, r->font_tiny);
        rect(r, x + 8, 107, width - 16, 7, (SDL_Color){44, 66, 56, 255});
        rect(r, x + 8, 107, (width - 16) * e->hp / e->max_hp, 7, (SDL_Color){104, 200, 142, 255});
        rect(r, x + 8 + (width - 16) * 2 / 5, 105, 2, 11, (SDL_Color){245, 208, 103, 255});
        const char *attack = e->boss_attack == MALAKAR_RESTORE ? (e->attack_target_x == 38 ? "RESTORE: WEST" : "RESTORE: EAST") :
            e->boss_attack == MALAKAR_PRESSURE ? (e->boss_aux_y == 38 ? "PRESSURE: WEST" : "PRESSURE: EAST") : "BURSTING VIAL";
        if (e->attack_target_x >= 0) {
            snprintf(text, sizeof(text), "%s: %d%s", attack, e->move_timer, e->frozen_turns ? " FROZEN" : "");
        } else {
            snprintf(text, sizeof(text), "%s", e->move_timer ? "RECOVERING" : "PREPARING");
        }
        renderer_draw_text(r, text, x + 8, 120, (SDL_Color){229, 228, 178, 255}, r->font_tiny);
        renderer_draw_text(r, "FLEES AT 40% HEALTH", x + 8, 139, (SDL_Color){245, 208, 103, 255}, r->font_tiny);
    }
}

void castle_draw_enemy(Renderer *r, int sx, int sy, const Enemy *e) {
    if (e->type == ENEMY_ROYAL_GUARD || e->type == ENEMY_VAMPIRE) {
        draw_enemy(r, sx, sy, e->type);
        return;
    }
    // Native frames are bottom-centered in 32-pixel regular / 40-pixel boss cells.
    static const SDL_Rect regular[5] = {{4, 6, 24, 26}, {36, 6, 24, 26}, {68, 6, 24, 26}, {4, 38, 24, 26}, {36, 38, 24, 26}};
    static const SDL_Rect bosses[5] = {{2, 4, 36, 36}, {42, 4, 36, 36}, {82, 4, 36, 36}, {2, 44, 36, 36}, {42, 44, 36, 36}};
    int index = e->type == ENEMY_MALAKAR ? 6 : e->type - ENEMY_OATHBOUND_SOLDIER;
    SDL_Texture *texture = index < 5 ? r->castle_enemy_texture : r->castle_boss_texture;
    SDL_Rect source;
    if (index < 5) {
        source = regular[index];
    } else {
        int phase = e->attack_phase >= 3 ? 2 : e->attack_phase == 2 ? 1 : 0;
        source = bosses[index == 7 ? 2 + phase : index - 5];
    }
    SDL_Rect destination = {sx * 24 - (e->is_boss ? 6 : 0), sy * 24 - (e->is_boss ? 12 : 2), e->is_boss ? 36 : 24, e->is_boss ? 36 : 26};
    if (texture) {
        if (e->type == ENEMY_MALAKAR) {
            SDL_SetTextureColorMod(texture, 178, 255, 192);
        }
        if (e->type == ENEMY_ROYAL_ARCANIST && e->attack_phase == 2) {
            SDL_SetTextureColorMod(texture, 190, 224, 255);
        }
        SDL_RenderCopy(r->sdl, texture, &source, &destination);
        SDL_SetTextureColorMod(texture, 255, 255, 255);
    } else {
        draw_armored_guard(r, sx, sy);
    }
    if (e->type == ENEMY_MALAKAR) {
        draw_world_prop(r, sx, sy, PROP_GOBLET, (SDL_Color){110, 238, 165, 255});
    }
    if (e->type == ENEMY_IRON_WARDEN ||
        (e->type == ENEMY_LORD_VEYR && e->attack_phase <= 1)) {
        int cx = sx * 24 + 12 + e->facing_dx * 10;
        int cy = sy * 24 + 12 + e->facing_dy * 10;
        rect(r, cx - 2, cy - 2, 4, 4, e->move_timer == 2 ? (SDL_Color){94, 88, 107, 255} : (SDL_Color){242, 195, 84, 255});
    }
}

void castle_draw_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CASTLE_INTERIOR) {
        return;
    }
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
        for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
            if (castle_fire_warning(g, x, y) && map_is_explored(&g->map, x, y)) {
                SDL_Rect tile = {(x - v->cam_x) * 24 + 1, (y - v->cam_y) * 24 + 1, 22, 22};
                SDL_SetRenderDrawColor(r->sdl, 246, 135, 39, 90);
                SDL_RenderFillRect(r->sdl, &tile);
                SDL_SetRenderDrawColor(r->sdl, 255, 204, 94, 255);
                SDL_RenderDrawRect(r->sdl, &tile);
                renderer_draw_text(r, g->castle_fire_phase[g->level - 1] == 1 ? "2" : "1", tile.x + 8, tile.y + 4, (SDL_Color){255, 236, 181, 255}, r->font_tiny);
            }
            for (int i = 0; i < g->enemy_count; i++) {
                const Enemy *e = &g->enemies[i];
                int rally = lord_veyr_escort_marks(e, x, y);
                int barrier = royal_arcanist_barrier_change(e, x, y) + lord_veyr_barrier_change(e, x, y);
                int attack_marks = castle_attack_marks(&g->map, e, x, y);
                int reagent = e->active && e->type == ENEMY_MALAKAR && e->attack_target_x >= 0 && y == 16 &&
                    ((e->boss_attack == MALAKAR_RESTORE && e->attack_target_x == x) || (e->boss_attack == MALAKAR_PRESSURE && e->boss_aux_y == x));
                if (attack_marks || rally || barrier || reagent) {
                    if (!map_is_explored(&g->map, x, y)) {
                        continue;
                    }
                    SDL_Rect tile = {(x - v->cam_x) * 24 + 1, (y - v->cam_y) * 24 + 1, 22, 22};
                    SDL_SetRenderDrawColor(r->sdl, 204, 108, 239, 55);
                    SDL_RenderFillRect(r->sdl, &tile);
                    SDL_SetRenderDrawColor(r->sdl, 236, 159, 252, 255);
                    SDL_RenderDrawRect(r->sdl, &tile);
                    if (e->type == ENEMY_ROYAL_ARCANIST || e->type == ENEMY_LORD_VEYR || e->type == ENEMY_MALAKAR) {
                        char text[8];
                        int turns = e->type == ENEMY_MALAKAR ? e->move_timer : e->move_timer >= 3 ? e->move_timer - 1 : 1;
                        int west = e->type == ENEMY_LORD_VEYR ? e->boss_attack == VEYR_WEST_LANE : e->boss_attack == ARCANIST_WEST_LANE;
                        int east = e->type == ENEMY_LORD_VEYR ? e->boss_attack == VEYR_EAST_LANE : e->boss_attack == ARCANIST_EAST_LANE;
                        const char *label = reagent ? "R" : rally ? "G" : barrier && attack_marks ? "X" : barrier ? (barrier < 0 ? "C" : "O") :
                            west ? "W" : east ? "E" : e->type == ENEMY_LORD_VEYR && e->boss_attack == VEYR_FIRE ? "F" : "";
                        snprintf(text, sizeof(text), "%s%d", label, turns);
                        SDL_SetRenderDrawColor(r->sdl, barrier ? 248 : 134, barrier ? 199 : 226, barrier ? 104 : 247, 255);
                        SDL_RenderDrawRect(r->sdl, &tile);
                        int inset = *label ? 3 : 8;
                        rect(r, tile.x + inset - 1, tile.y + 3, *label ? 18 : 10, 10, (SDL_Color){32, 27, 47, 255});
                        renderer_draw_text(r, text, tile.x + inset, tile.y + 4, (SDL_Color){255, 241, 196, 255}, r->font_tiny);
                    }
                    break;
                }
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_NONE);
}

void castle_draw_arcanist_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 4) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_ROYAL_ARCANIST || e->max_hp <= 0 || !viewport_is_visible(v, e->x, e->y) || !map_is_explored(&g->map, e->x, e->y)) {
            continue;
        }
        int width = r->screen_w - INFO_PANEL_W - 16;
        if (width > 380) {
            width = 380;
        }
        if (width < 160) {
            return;
        }
        int x = (r->screen_w - INFO_PANEL_W - width) / 2;
        int top = x < SWAMP_MAP_W / 2 + 12 ? SWAMP_MAP_H / 2 + 16 : 8;
        rect(r, x, top, width, 78, (SDL_Color){27, 26, 47, 255});
        char text[80];
        if (width < 300) {
            snprintf(text, sizeof(text), "ARCANIST: PHASE %d", e->attack_phase == 2 ? 2 : 1);
        } else {
            snprintf(text, sizeof(text), "ROYAL ARCANIST: %s", e->attack_phase == 2 ? "SHATTERED LITURGY" : "WARDED SCHOLAR");
        }
        renderer_draw_text(r, text, x + 10, top + 8, (SDL_Color){220, 225, 255, 255}, r->font_tiny);
        rect(r, x + 10, top + 23, width - 20, 8, (SDL_Color){59, 43, 81, 255});
        rect(r, x + 10, top + 23, (width - 20) * e->hp / e->max_hp, 8, (SDL_Color){153, 173, 228, 255});
        if (e->attack_target_x >= 0) {
            static const char *attacks[] = {"PREPARING", "WARDED BLAST", "CHAPEL CROSS", "WEST WARD LANE", "EAST WARD LANE"};
            int attack = e->boss_attack >= ARCANIST_BLAST && e->boss_attack <= ARCANIST_EAST_LANE ? e->boss_attack : 0;
            snprintf(text, sizeof(text), "%s: %d%s", !e->boss_aux_x && !e->boss_aux_y ? "BLOCKED ATTACK" : attacks[attack], e->move_timer >= 3 ? e->move_timer - 1 : 1, e->frozen_turns > 0 ? " FROZEN" : "");
        } else if (e->move_timer == 2) {
            snprintf(text, sizeof(text), "RECOVERY: %d%s", 1 + e->revive_timer, e->frozen_turns > 0 ? " FROZEN" : "");
        } else {
            snprintf(text, sizeof(text), "PREPARING ATTACK");
        }
        renderer_draw_text(r, text, x + 10, top + 38, (SDL_Color){178, 232, 245, 255}, r->font_tiny);
        renderer_draw_text(r, width < 300 ? "W/E PEDESTALS: A" : "DISABLE THE LINKED W/E PEDESTAL: A", x + 10, top + 55, (SDL_Color){218, 201, 240, 255}, r->font_tiny);
    }
}

void castle_draw_veyr_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 6) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_LORD_VEYR || e->max_hp <= 0 || !viewport_is_visible(v, e->x, e->y) || !map_is_explored(&g->map, e->x, e->y)) {
            continue;
        }
        int width = r->screen_w - INFO_PANEL_W - 16;
        if (width > 380) {
            width = 380;
        }
        if (width < 160) {
            return;
        }
        int x = (r->screen_w - INFO_PANEL_W - width) / 2;
        int top = x < SWAMP_MAP_W / 2 + 12 ? SWAMP_MAP_H / 2 + 16 : 8;
        rect(r, x, top, width, 78, (SDL_Color){39, 25, 42, 255});
        static const char *phases[] = {"ARMORED RULER", "BROKEN THRONE", "UNBOUND CROWN"};
        int phase = e->attack_phase >= 3 ? 2 : e->attack_phase == 2 ? 1 : 0;
        char text[80];
        if (width < 300) {
            snprintf(text, sizeof(text), "VEYR: PHASE %d", phase + 1);
        } else {
            snprintf(text, sizeof(text), "LORD VEYR: %s", phases[phase]);
        }
        renderer_draw_text(r, text, x + 10, top + 8, (SDL_Color){251, 220, 156, 255}, r->font_tiny);
        rect(r, x + 10, top + 23, width - 20, 8, (SDL_Color){76, 41, 66, 255});
        rect(r, x + 10, top + 23, (width - 20) * e->hp / e->max_hp, 8, (SDL_Color){217, 153, 82, 255});
        if (e->attack_target_x >= 0) {
            static const char *attacks[] = {"PREPARING", "OATHBLADE", "THRONE ADVANCE", "ROYAL EDICT", "WEST WARD LANE", "EAST WARD LANE", "UNBOUND CROWN", "THRONE FIRE"};
            int attack = e->boss_attack >= VEYR_SWING && e->boss_attack <= VEYR_FIRE ? e->boss_attack : 0;
            snprintf(text, sizeof(text), "%s: %d%s", !e->boss_aux_x ? "BLOCKED ATTACK" : attacks[attack], e->move_timer >= 3 ? e->move_timer - 1 : 1, e->frozen_turns > 0 ? " FROZEN" : "");
        } else if (e->move_timer == 2) {
            snprintf(text, sizeof(text), "RECOVERY: %d%s", 1 + e->revive_timer, e->frozen_turns > 0 ? " FROZEN" : "");
        } else {
            snprintf(text, sizeof(text), "PREPARING ATTACK");
        }
        renderer_draw_text(r, text, x + 10, top + 38, (SDL_Color){242, 193, 222, 255}, r->font_tiny);
        renderer_draw_text(r, width < 300 ? "BOTH WARDS OFF: CROWN R2" : "BOTH WARDS OFF: CROWN RECOVERS 2 ACTIONS", x + 10, top + 55, (SDL_Color){193, 223, 222, 255}, r->font_tiny);
    }
}
