#include "game_renderer.h"
#include "sprites.h"
#include "info_panel.h"
#include "message_bar.h"
#include "minimap_renderer.h"
#include "renderer.h"
#include <string.h>
#include <stdlib.h>

static void draw_dialogue_text(Renderer *r, const char *text, int x, int y, int max_chars, SDL_Color color) {
    char line[64];
    int line_len = 0;
    const char *cursor = text;

    while (*cursor) {
        while (*cursor == ' ') {
            cursor++;
        }
        const char *word = cursor;
        int word_len = 0;
        while (cursor[word_len] && cursor[word_len] != ' ') {
            word_len++;
        }
        if (word_len == 0) {
            break;
        }
        if (line_len > 0 && line_len + word_len + 1 > max_chars) {
            line[line_len] = '\0';
            renderer_draw_text(r, line, x, y, color, r->font_tiny);
            y += 16;
            line_len = 0;
        }
        if (line_len > 0) {
            line[line_len++] = ' ';
        }
        if (word_len > (int)sizeof(line) - line_len - 1) {
            word_len = (int)sizeof(line) - line_len - 1;
        }
        memcpy(line + line_len, word, word_len);
        line_len += word_len;
        cursor += word_len;
    }
    if (line_len > 0) {
        line[line_len] = '\0';
        renderer_draw_text(r, line, x, y, color, r->font_tiny);
    }
}

static void draw_dialogue_bubble(Renderer *r, const GameState *g, const Viewport *v) {
    if (!g->dialogue_active ||
        (g->location != LOCATION_TAVERN &&
        g->location != LOCATION_TOWN &&
        g->location != LOCATION_FOREST)) {
        return;
    }
    int npc_x = g->dialogue_x;
    int npc_y = g->dialogue_y;
    if (!viewport_is_visible(v, npc_x, npc_y)) {
        return;
    }

    int viewport_w = r->screen_w - INFO_PANEL_W;
    if (g->location == LOCATION_TOWN) {
        viewport_w = TOWN_W * TILE_SIZE;
    } else if (g->location == LOCATION_TAVERN) {
        viewport_w = TAVERN_W * TILE_SIZE;
    }
    int bubble_w = viewport_w < 460 ? viewport_w - 16 : 440;
    int bubble_h = 98;
    int npc_screen_x = viewport_to_screen_x(v, npc_x) * TILE_SIZE +
        TILE_SIZE / 2;
    int npc_screen_y = viewport_to_screen_y(v, npc_y) * TILE_SIZE;
    int bubble_x = npc_screen_x - bubble_w / 2;
    int bubble_y = npc_screen_y - bubble_h - 22;
    if (bubble_x < 8) {
        bubble_x = 8;
    }
    if (bubble_x + bubble_w > viewport_w - 8) {
        bubble_x = viewport_w - bubble_w - 8;
    }
    if (bubble_y < 8) {
        bubble_y = 8;
    }

    SDL_Rect border = {bubble_x, bubble_y, bubble_w, bubble_h};
    SDL_Rect panel = {bubble_x + 3, bubble_y + 3, bubble_w - 6,
        bubble_h - 6};
    SDL_SetRenderDrawColor(r->sdl, 25, 20, 27, 255);
    SDL_RenderFillRect(r->sdl, &border);
    SDL_SetRenderDrawColor(r->sdl, 236, 224, 190, 255);
    SDL_RenderFillRect(r->sdl, &panel);

    int tail_x = npc_screen_x;
    if (tail_x < bubble_x + 18) {
        tail_x = bubble_x + 18;
    }
    if (tail_x > bubble_x + bubble_w - 18) {
        tail_x = bubble_x + bubble_w - 18;
    }
    for (int row = 0; row < 15; row++) {
        int half_width = (15 - row) / 2;
        SDL_Rect tail = {tail_x - half_width, bubble_y + bubble_h + row,
            half_width * 2 + 1, 1};
        SDL_SetRenderDrawColor(r->sdl, 25, 20, 27, 255);
        SDL_RenderFillRect(r->sdl, &tail);
    }
    for (int row = 0; row < 11; row++) {
        int half_width = (11 - row) / 2;
        SDL_Rect tail = {tail_x - half_width, bubble_y + bubble_h + row,
            half_width * 2 + 1, 1};
        SDL_SetRenderDrawColor(r->sdl, 236, 224, 190, 255);
        SDL_RenderFillRect(r->sdl, &tail);
    }

    renderer_draw_text(r, g->dialogue_speaker, bubble_x + 14,
        bubble_y + 12, (SDL_Color){71, 82, 138, 255}, r->font_small);
    draw_dialogue_text(r, g->dialogue_text, bubble_x + 14, bubble_y + 36,
        (bubble_w - 28) / 8, (SDL_Color){42, 32, 30, 255});
}

static void draw_weapon_arrow_at(Renderer *r, int cx, int cy, int dx, int dy, int impact) {
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

void game_draw_enemy_projectiles(Renderer *r, const EnemyProjectiles *shots, const Viewport *v, Uint32 elapsed) {
    if (elapsed >= ENEMY_PROJECTILE_TOTAL_MS || shots->count == 0) {
        return;
    }
    SDL_Rect old_clip;
    SDL_bool clipped = SDL_RenderIsClipEnabled(r->sdl);
    SDL_RenderGetClipRect(r->sdl, &old_clip);
    SDL_BlendMode old_blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &old_blend);
    SDL_Rect play_area = {0, 0, r->screen_w - INFO_PANEL_W, r->tiles_y * TILE_SIZE};
    SDL_RenderSetClipRect(r->sdl, &play_area);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    int impact = elapsed >= ENEMY_PROJECTILE_TRAVEL_MS;
    float progress = impact ? 1.0f : (float)elapsed / ENEMY_PROJECTILE_TRAVEL_MS;
    for (int i = 0; i < shots->count; i++) {
        const EnemyProjectile *shot = &shots->shots[i];
        int dx = shot->target_x - shot->start_x;
        int dy = shot->target_y - shot->start_y;
        int length = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
        if (length == 0) {
            continue;
        }
        float vx = (float)dx / length;
        float vy = (float)dy / length;
        int cx = (int)((shot->start_x - v->cam_x + dx * progress) * TILE_SIZE) + TILE_SIZE / 2;
        int cy = (int)((shot->start_y - v->cam_y + dy * progress) * TILE_SIZE) + TILE_SIZE / 2;
        SDL_Color color = {184, 100, 255, 255};
        if (shot->type == ENEMY_FOREST_NECROMANCER) {
            color = (SDL_Color){100, 240, 130, 255};
        } else if (shot->type == ENEMY_WATER_ELEMENTAL || shot->type == ENEMY_DROWNED_QUEEN) {
            color = (SDL_Color){80, 210, 255, 255};
        } else if (shot->type == ENEMY_SIREN) {
            color = (SDL_Color){240, 130, 220, 255};
        } else if (shot->type == ENEMY_GOBLIN_BOMBER) {
            color = (SDL_Color){255, 160, 55, 255};
        } else if (shot->type == ENEMY_SUN_PRIEST ||
            shot->type == ENEMY_FALLEN_SUN_GUARDIAN) {
            color = (SDL_Color){255, 188, 45, 255};
        } else if (shot->type == ENEMY_SERPENT_SPIRIT ||
            shot->type == ENEMY_MOONBOUND_SENTINEL) {
            color = (SDL_Color){75, 224, 232, 255};
        }
        if (impact) {
            int radius = 4 + (int)(elapsed - ENEMY_PROJECTILE_TRAVEL_MS) / 15;
            SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, 220);
            SDL_RenderDrawLine(r->sdl, cx - radius, cy, cx + radius, cy);
            SDL_RenderDrawLine(r->sdl, cx, cy - radius, cx, cy + radius);
            SDL_Rect burst = {cx - radius / 2, cy - radius / 2, radius, radius};
            SDL_RenderDrawRect(r->sdl, &burst);
        } else if (shot->type == ENEMY_GOBLIN_ARCHER ||
            shot->type == ENEMY_DARK_ELF ||
            shot->type == ENEMY_BLOWDART_HUNTER) {
            draw_weapon_arrow_at(r, cx, cy, (dx > 0) - (dx < 0), (dy > 0) - (dy < 0), 0);
        } else if (shot->type == ENEMY_GOBLIN_BOMBER) {
            SDL_Rect bomb = {cx - 5, cy - 5, 10, 10};
            SDL_SetRenderDrawColor(r->sdl, 36, 32, 40, 255);
            SDL_RenderFillRect(r->sdl, &bomb);
            SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, 255);
            SDL_RenderDrawRect(r->sdl, &bomb);
            SDL_RenderDrawLine(r->sdl, cx, cy - 5, cx + 4, cy - 9);
            SDL_RenderDrawPoint(r->sdl, cx + 5, cy - 10);
        } else if (shot->type == ENEMY_MOUNTAIN_GOBLIN_KING) {
            SDL_SetRenderDrawColor(r->sdl, 160, 105, 55, 255);
            SDL_RenderDrawLine(r->sdl, cx - (int)(vx * 8), cy - (int)(vy * 8), cx + (int)(vx * 5), cy + (int)(vy * 5));
            SDL_Rect blade = {cx + (int)(vx * 4) - 5, cy + (int)(vy * 4) - 5, 10, 10};
            SDL_SetRenderDrawColor(r->sdl, 210, 220, 225, 255);
            SDL_RenderFillRect(r->sdl, &blade);
        } else {
            SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, 150);
            for (int offset = -2; offset <= 2; offset++) {
                SDL_RenderDrawLine(r->sdl, cx - (int)(vx * 14) - (int)(vy * offset),
                    cy - (int)(vy * 14) + (int)(vx * offset), cx, cy);
            }
            SDL_Rect core = {cx - 3, cy - 3, 6, 6};
            SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, 255);
            SDL_RenderFillRect(r->sdl, &core);
            SDL_SetRenderDrawColor(r->sdl, 240, 250, 255, 255);
            SDL_RenderDrawPoint(r->sdl, cx, cy);
        }
    }
    SDL_RenderSetClipRect(r->sdl, clipped ? &old_clip : NULL);
    SDL_SetRenderDrawBlendMode(r->sdl, old_blend);
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

static TileType floor_item_underlay(const GameState *g, int x, int y) {
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            return (TileType)item->underlying_tile;
        }
    }
    if (g->location == LOCATION_FOREST) {
        return TILE_FOREST_FLOOR;
    }
    if (g->location == LOCATION_MOUNTAINS) {
        return TILE_MOUNTAIN_FLOOR;
    }
    if (g->location == LOCATION_COAST) {
        return TILE_COAST_FLOOR;
    }
    return TILE_FLOOR;
}

static int dungeon_is_floor(const GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        tile = floor_item_underlay(g, x, y);
    }
    return tile == TILE_FLOOR || tile == TILE_STAIRS_UP ||
        tile == TILE_STAIRS_DOWN || tile == TILE_RETURN_EXIT ||
        tile == TILE_DUNGEON_KEY || tile == TILE_CRYPT_KEY ||
        tile == TILE_CRYPT_CACHE || tile == TILE_DUNGEON_SWITCH_OFF ||
        tile == TILE_DUNGEON_SWITCH_ON || tile == TILE_PORTAL ||
        tile == TILE_BROKEN_BURIAL_SEAL ||
        tile == TILE_RESTORED_BURIAL_SEAL ||
        tile == TILE_TRAP_HIDDEN || tile == TILE_TRAP_REVEALED ||
        tile == TILE_TRAP_SPIKE || tile == TILE_TRAP_FIRE ||
        tile == TILE_TRAP_POISON;
}

static int dungeon_wall_has_torch(const GameState *g, int x, int y) {
    if (y + 1 >= MAP_H || !dungeon_is_floor(g, x, y + 1)) {
        return 0;
    }
    unsigned int seed = (unsigned int)x * 2246822519u ^
        (unsigned int)y * 3266489917u;
    return seed % 11u == 0u;
}

static int forest_is_tree(TileType tile) {
    return tile == TILE_FOREST_WALL || tile == TILE_FOREST_HIDDEN_TRAIL;
}

static int forest_is_floor(const GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        tile = floor_item_underlay(g, x, y);
    }
    return tile == TILE_FOREST_FLOOR || tile == TILE_FOREST_LANDMARK ||
        tile == TILE_FOREST_FALSE_MARKER || tile == TILE_FOREST_WARDEN ||
        tile == TILE_TRAP_HIDDEN || tile == TILE_TRAP_REVEALED ||
        tile == TILE_TRAP_SPIKE || tile == TILE_TRAP_FIRE ||
        tile == TILE_TRAP_POISON;
}

static TileType coast_visible_tile(const GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        tile = floor_item_underlay(g, x, y);
    }
    if (tile == TILE_TRAP_HIDDEN || tile == TILE_TRAP_REVEALED ||
        tile == TILE_TRAP_SPIKE || tile == TILE_TRAP_FIRE ||
        tile == TILE_TRAP_POISON) {
        return map_coast_trap_underlay(&g->map, x, y);
    }
    return tile;
}

static int coast_is_water_surface(TileType tile) {
    return tile == TILE_COAST_SHALLOW_WATER ||
        tile == TILE_COAST_DEEP_WATER ||
        tile == TILE_COAST_CHANNEL_WATER ||
        tile == TILE_COAST_BEACON_UNLIT ||
        tile == TILE_COAST_BEACON_LIT;
}

static int mountain_crossing_neighbor(const Map *m, int x, int y, int chasm) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
        return 0;
    }
    TileType tile = m->tiles[y][x];
    if (chasm) {
        return tile == TILE_MOUNTAIN_BRIDGE ||
            tile == TILE_MOUNTAIN_WEAK_BRIDGE;
    }
    return tile == TILE_MOUNTAIN_CHASM || map_is_walkable(m, x, y);
}

static unsigned int mountain_crossing_neighbors(const Map *m, int x, int y, int chasm) {
    unsigned int edges = 0;
    if (mountain_crossing_neighbor(m, x, y - 1, chasm)) {
        edges |= MOUNTAIN_EDGE_NORTH;
    }
    if (mountain_crossing_neighbor(m, x + 1, y, chasm)) {
        edges |= MOUNTAIN_EDGE_EAST;
    }
    if (mountain_crossing_neighbor(m, x, y + 1, chasm)) {
        edges |= MOUNTAIN_EDGE_SOUTH;
    }
    if (mountain_crossing_neighbor(m, x - 1, y, chasm)) {
        edges |= MOUNTAIN_EDGE_WEST;
    }
    return edges;
}

static void draw_coast_trap_underlay(Renderer *r, const GameState *g, int map_x, int map_y, int screen_x, int screen_y) {
    TileType tile = map_coast_trap_underlay(&g->map, map_x, map_y);
    if (tile == TILE_COAST_SHALLOW_WATER) {
        draw_coast_shallow_water(r, screen_x, screen_y, map_x, map_y);
    } else if (tile == TILE_COAST_DEEP_WATER ||
        tile == TILE_COAST_DRAINED_WATER) {
        draw_coast_channel(r, screen_x, screen_y, map_x, map_y, 0,
            tile == TILE_COAST_DEEP_WATER);
    } else {
        draw_coast_floor(r, screen_x, screen_y, map_x, map_y);
    }
}

static void draw_floor_item_with_underlay(Renderer *r, const GameState *g, int map_x, int map_y, int screen_x, int screen_y) {
    TileType underlay = floor_item_underlay(g, map_x, map_y);
    if (underlay == TILE_TRAP_HIDDEN && g->location == LOCATION_FOREST) {
        draw_forest_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_TRAP_HIDDEN &&
        g->location == LOCATION_MOUNTAINS) {
        draw_mountain_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_TRAP_HIDDEN &&
        g->location == LOCATION_COAST) {
        draw_coast_trap_underlay(r, g, map_x, map_y, screen_x, screen_y);
    } else if (underlay == TILE_FOREST_FLOOR) {
        draw_forest_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_MOUNTAIN_BRIDGE) {
        draw_mountain_bridge(r, screen_x, screen_y,
            mountain_crossing_neighbors(&g->map, map_x, map_y, 0));
    } else if (underlay == TILE_MOUNTAIN_CAVE_FLOOR) {
        draw_mountain_cave_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_MOUNTAIN_FORTRESS_FLOOR) {
        draw_mountain_fortress_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_MOUNTAIN_FLOOR) {
        draw_mountain_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_COAST_SHALLOW_WATER) {
        draw_coast_shallow_water(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_COAST_DRAINED_WATER) {
        draw_coast_channel(r, screen_x, screen_y, map_x, map_y, 0, 0);
    } else if (underlay == TILE_COAST_FLOOR) {
        draw_coast_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_TOWN_FLOOR) {
        draw_town_floor(r, screen_x, screen_y);
    } else if (underlay == TILE_TOWN_PATH) {
        draw_town_path(r, screen_x, screen_y);
    } else if (underlay == TILE_TAVERN_FLOOR) {
        draw_tavern_floor(r, screen_x, screen_y);
    } else if (underlay == TILE_ISLAND_WATER ||
        underlay == TILE_ISLAND_DOCK) {
        draw_island_water(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_ISLAND_SAND) {
        draw_island_sand(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_ISLAND_GRASS) {
        draw_island_grass(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_ISLAND_PATH) {
        draw_island_path(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_TEMPLE_FLOOR ||
        g->location == LOCATION_TEMPLE) {
        draw_temple_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_DUNGEON) {
        draw_dungeon_floor(r, screen_x, screen_y, map_x, map_y);
    } else {
        draw_floor(r, screen_x, screen_y);
    }
    draw_floor_item(r, screen_x, screen_y);
}

static void draw_trap_underlay(Renderer *r, const GameState *g, int map_x, int map_y, int screen_x, int screen_y) {
    int cave_neighbors = 0;
    int fortress_neighbors = 0;
    int bridge_neighbors = 0;
    for (int y = map_y - 1; y <= map_y + 1; y++) {
        for (int x = map_x - 1; x <= map_x + 1; x++) {
            if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
                continue;
            }
            TileType tile = g->map.tiles[y][x];
            cave_neighbors += tile == TILE_MOUNTAIN_CAVE_FLOOR;
            fortress_neighbors += tile == TILE_MOUNTAIN_FORTRESS_FLOOR;
            bridge_neighbors += tile == TILE_MOUNTAIN_BRIDGE;
        }
    }
    if (g->location == LOCATION_FOREST) {
        draw_forest_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_COAST) {
        draw_coast_trap_underlay(r, g, map_x, map_y, screen_x, screen_y);
    } else if (g->location == LOCATION_MOUNTAINS &&
        cave_neighbors >= fortress_neighbors &&
        cave_neighbors >= bridge_neighbors && cave_neighbors > 0) {
        draw_mountain_cave_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_MOUNTAINS &&
        fortress_neighbors >= bridge_neighbors && fortress_neighbors > 0) {
        draw_mountain_fortress_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_MOUNTAINS && bridge_neighbors > 0) {
        draw_mountain_bridge(r, screen_x, screen_y,
            mountain_crossing_neighbors(&g->map, map_x, map_y, 0));
    } else if (g->location == LOCATION_MOUNTAINS) {
        draw_mountain_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_DUNGEON) {
        draw_dungeon_floor(r, screen_x, screen_y, map_x, map_y);
    } else {
        draw_floor(r, screen_x, screen_y);
    }
}

void game_draw(Renderer *r, GameState *g, Viewport *v) {
    Viewport town_view;
    int town_scaled = g->location == LOCATION_TOWN;
    int tavern_scaled = g->location == LOCATION_TAVERN;
    int island_scaled = g->location == LOCATION_ISLAND;
    int labyrinth_scaled = g->location == LOCATION_LABYRINTH;
    if (town_scaled || island_scaled || labyrinth_scaled) {
        // Keep the entire fixed town or island map inside the play area.
        int map_w = town_scaled ? TOWN_W :
            (island_scaled ? ISLAND_W : LABYRINTH_W);
        int map_h = town_scaled ? TOWN_H :
            (island_scaled ? ISLAND_H : LABYRINTH_H);
        viewport_init(&town_view, map_w, map_h, map_w, map_h);
        v = &town_view;
        int play_w = r->screen_w - INFO_PANEL_W;
        int play_h = r->tiles_y * TILE_SIZE;
        if (play_w < 1) {
            play_w = 1;
        }
        if (play_h < 1) {
            play_h = 1;
        }
        SDL_RenderSetScale(r->sdl,
            (float)play_w / (map_w * TILE_SIZE),
            (float)play_h / (map_h * TILE_SIZE));
    } else if (tavern_scaled) {
        // Fit the entire room without stretching sprites or showing unused map tiles.
        viewport_init(&town_view, TAVERN_W, TAVERN_H, MAP_W, MAP_H);
        town_view.cam_x = TAVERN_X;
        town_view.cam_y = TAVERN_Y;
        v = &town_view;
        int play_w = r->screen_w - INFO_PANEL_W;
        int play_h = r->tiles_y * TILE_SIZE;
        if (play_w < 1) {
            play_w = 1;
        }
        if (play_h < 1) {
            play_h = 1;
        }
        float scale_x = (float)play_w / (TAVERN_W * TILE_SIZE);
        float scale_y = (float)play_h / (TAVERN_H * TILE_SIZE);
        float scale = scale_x < scale_y ? scale_x : scale_y;
        int room_w = (int)(TAVERN_W * TILE_SIZE * scale);
        int room_h = (int)(TAVERN_H * TILE_SIZE * scale);
        SDL_Rect room_view = {(play_w - room_w) / 2, (play_h - room_h) / 2,
            room_w, room_h};
        SDL_RenderSetViewport(r->sdl, &room_view);
        SDL_RenderSetScale(r->sdl, scale, scale);
    }
    int landmark_x = -1;
    int landmark_y = -1;
    if (g->location == LOCATION_FOREST && g->map.room_count > 1) {
        int room = g->level == FOREST_DEPTH ? g->map.room_count - 2 :
            g->map.room_count - 1;
        map_room_center(&g->map.rooms[room], &landmark_x, &landmark_y);
    }

    // Draw map tiles
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (!viewport_is_visible(v, x, y)) continue;
            map_mark_explored(&g->map, x, y);
            int sx = viewport_to_screen_x(v, x);
            int sy = viewport_to_screen_y(v, y);
            switch (g->map.tiles[y][x]) {
                case TILE_FLOOR:
                    if (g->location == LOCATION_DUNGEON) {
                        draw_dungeon_floor(r, sx, sy, x, y);
                    } else {
                        draw_floor(r, sx, sy);
                    }
                    break;
                case TILE_WALL: {
                    if (g->location == LOCATION_DUNGEON) {
                        draw_dungeon_wall(r, sx, sy, x, y);
                        const int dx[4] = {0, 1, 0, -1};
                        const int dy[4] = {-1, 0, 1, 0};
                        const unsigned int side[4] = {
                            DUNGEON_EDGE_NORTH, DUNGEON_EDGE_EAST,
                            DUNGEON_EDGE_SOUTH, DUNGEON_EDGE_WEST
                        };
                        for (int i = 0; i < 4; i++) {
                            int nx = x + dx[i];
                            int ny = y + dy[i];
                            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H) {
                                continue;
                            }
                            TileType neighbor = g->map.tiles[ny][nx];
                            if (neighbor == TILE_LOCKED_DOOR || neighbor == TILE_CRYPT_DOOR) {
                                draw_dungeon_door_support(r, sx, sy, side[i],
                                    neighbor == TILE_CRYPT_DOOR);
                            }
                        }
                        if (dungeon_wall_has_torch(g, x, y)) {
                            draw_dungeon_torch(r, sx, sy, x, y);
                        }
                    } else {
                        draw_wall(r, sx, sy);
                    }
                    break;
                }
                case TILE_FOREST_WALL:
                    draw_forest_wall(r, sx, sy, x, y); break;
                case TILE_FOREST_HIDDEN_TRAIL:
                    draw_forest_wall(r, sx, sy, x, y); break;
                case TILE_FOREST_FLOOR:
                    draw_forest_floor(r, sx, sy, x, y); break;
                case TILE_FOREST_ENTRANCE:
                    draw_forest_edge(r, sx, sy, x, y, 0); break;
                case TILE_FOREST_EXIT:
                    draw_forest_edge(r, sx, sy, x, y, 1); break;
                case TILE_FOREST_LANDMARK:
                    draw_forest_landmark(r, sx, sy, x, y); break;
                case TILE_FOREST_FALSE_MARKER:
                    draw_forest_false_marker(r, sx, sy, x, y); break;
                case TILE_MOUNTAIN_FLOOR:
                    draw_mountain_floor(r, sx, sy, x, y); break;
                case TILE_MOUNTAIN_WALL:
                    draw_mountain_wall(r, sx, sy, x, y);
                    break;
                case TILE_MOUNTAIN_ENTRANCE:
                    draw_mountain_edge(r, sx, sy, x, y, 0); break;
                case TILE_MOUNTAIN_EXIT:
                    draw_mountain_edge(r, sx, sy, x, y, 1); break;
                case TILE_MOUNTAIN_HIDDEN_CAVE:
                    draw_mountain_wall(r, sx, sy, x, y); break;
                case TILE_MOUNTAIN_ROCKFALL:
                    draw_mountain_rockfall(r, sx, sy, x, y); break;
                case TILE_MOUNTAIN_CHASM:
                    draw_mountain_chasm(r, sx, sy,
                        mountain_crossing_neighbors(&g->map, x, y, 1)); break;
                case TILE_MOUNTAIN_GATE:
                    draw_mountain_gate(r, sx, sy, x, y); break;
                case TILE_MOUNTAIN_CACHE:
                    draw_crypt_cache(r, sx, sy); break;
                case TILE_MOUNTAIN_WEAK_BRIDGE:
                case TILE_MOUNTAIN_BRIDGE:
                    draw_mountain_bridge(r, sx, sy,
                        mountain_crossing_neighbors(&g->map, x, y, 0)); break;
                case TILE_MOUNTAIN_CAVE_FLOOR:
                    draw_mountain_cave_floor(r, sx, sy, x, y); break;
                case TILE_MOUNTAIN_FORTRESS_FLOOR:
                    draw_mountain_fortress_floor(r, sx, sy, x, y); break;
                case TILE_COAST_FLOOR:
                    draw_coast_floor(r, sx, sy, x, y);
                    if (x + 1 < MAP_W &&
                        g->map.tiles[y][x + 1] == TILE_COAST_SLUICE_CONTROL) {
                        draw_coast_sluice_conduit(r, sx, sy);
                    }
                    break;
                case TILE_COAST_WALL:
                    draw_coast_wall(r, sx, sy, x, y);
                    if (x + 2 < MAP_W &&
                        g->map.tiles[y][x + 2] == TILE_COAST_SLUICE_CONTROL) {
                        draw_coast_sluice_intake(r, sx, sy);
                    }
                    break;
                case TILE_COAST_ENTRANCE:
                    draw_coast_edge(r, sx, sy, x, y, 0); break;
                case TILE_COAST_EXIT: {
                    int open = g->map.tiles[g->map.stairs_down_y]
                        [g->map.stairs_down_x] != TILE_COAST_DEEP_WATER;
                    draw_coast_edge(r, sx, sy, x, y, open ? 1 : 2);
                    break;
                }
                case TILE_COAST_SHALLOW_WATER:
                    draw_coast_shallow_water(r, sx, sy, x, y); break;
                case TILE_COAST_DRAINED_WATER:
                    draw_coast_channel(r, sx, sy, x, y, 0, 0); break;
                case TILE_COAST_DEEP_WATER:
                    draw_coast_channel(r, sx, sy, x, y, 0, 1); break;
                case TILE_COAST_CHANNEL_DRY:
                    draw_coast_channel(r, sx, sy, x, y, 1, 0); break;
                case TILE_COAST_CHANNEL_WATER:
                    draw_coast_channel(r, sx, sy, x, y, 1, 1); break;
                case TILE_COAST_SLUICE_CONTROL:
                    draw_coast_sluice(r, sx, sy, x, y); break;
                case TILE_COAST_CACHE:
                    draw_coast_cache(r, sx, sy, x, y); break;
                case TILE_COAST_TIDE_CONTROL:
                    draw_coast_tide_control(r, sx, sy, x, y); break;
                case TILE_COAST_BEACON_UNLIT:
                    draw_coast_beacon(r, sx, sy, x, y, 0); break;
                case TILE_COAST_BEACON_LIT:
                    draw_coast_beacon(r, sx, sy, x, y, 1); break;
                case TILE_STAIRS_UP: draw_stairs_up(r, sx, sy); break;
                case TILE_STAIRS_DOWN: draw_stairs_down(r, sx, sy); break;
                case TILE_RETURN_EXIT: draw_return_exit(r, sx, sy); break;
                case TILE_LOCKED_DOOR: draw_locked_door(r, sx, sy); break;
                case TILE_DUNGEON_KEY: draw_dungeon_key(r, sx, sy); break;
                case TILE_CRYPT_DOOR: draw_crypt_door(r, sx, sy); break;
                case TILE_CRYPT_KEY: draw_crypt_key(r, sx, sy); break;
                case TILE_CRYPT_CACHE: draw_crypt_cache(r, sx, sy); break;
                case TILE_DUNGEON_GATE: draw_dungeon_gate(r, sx, sy); break;
                case TILE_DUNGEON_SWITCH_OFF:
                    draw_dungeon_switch(r, sx, sy, 0); break;
                case TILE_DUNGEON_SWITCH_ON:
                    draw_dungeon_switch(r, sx, sy, 1); break;
                case TILE_PORTAL: draw_portal(r, sx, sy); break;
                case TILE_BROKEN_BURIAL_SEAL:
                    draw_broken_burial_seal(r, sx, sy); break;
                case TILE_RESTORED_BURIAL_SEAL:
                    draw_restored_burial_seal(r, sx, sy); break;
                case TILE_TOWN_FLOOR: draw_town_floor(r, sx, sy); break;
                case TILE_TOWN_PATH: draw_town_path(r, sx, sy); break;
                case TILE_LABYRINTH_ENTRANCE:
                    draw_town_floor(r, sx, sy); break;
                case TILE_BLACKSMITH_DOOR:
                case TILE_ALCHEMIST_DOOR:
                case TILE_HEALER_DOOR:
                case TILE_WITCH_DOOR:
                case TILE_TAVERN_DOOR:
                    draw_town_path(r, sx, sy); break;
                case TILE_TAVERN_FLOOR: draw_tavern_floor(r, sx, sy); break;
                case TILE_TAVERN_WALL: draw_tavern_wall(r, sx, sy); break;
                case TILE_TAVERN_EXIT: draw_tavern_exit(r, sx, sy); break;
                case TILE_TAVERN_TABLE: draw_tavern_table(r, sx, sy); break;
                case TILE_NPC_ELOWEN: draw_elowen(r, sx, sy); break;
                case TILE_NPC_DAIN: draw_dain(r, sx, sy); break;
                case TILE_NPC_ALDER: draw_alder(r, sx, sy); break;
                case TILE_NPC_MARA: draw_mara(r, sx, sy); break;
                case TILE_NPC_ROOK: draw_rook(r, sx, sy); break;
                case TILE_NPC_CAIN: draw_cain(r, sx, sy); break;
                case TILE_NPC_ROWAN: draw_rowan(r, sx, sy); break;
                case TILE_FOREST_WARDEN:
                    draw_forest_warden(r, sx, sy, x, y); break;
                case TILE_ISLAND_WATER:
                    draw_island_water(r, sx, sy, x, y); break;
                case TILE_ISLAND_SAND:
                    draw_island_sand(r, sx, sy, x, y); break;
                case TILE_ISLAND_GRASS:
                    draw_island_grass(r, sx, sy, x, y); break;
                case TILE_ISLAND_JUNGLE:
                    draw_island_jungle(r, sx, sy, x, y); break;
                case TILE_ISLAND_PATH:
                    draw_island_path(r, sx, sy, x, y); break;
                case TILE_ISLAND_DOCK:
                case TILE_ISLAND_SHIP:
                case TILE_NPC_ISLAND_CAPTAIN:
                    draw_island_water(r, sx, sy, x, y); break;
                case TILE_NPC_ISLAND_NAHLA:
                    draw_island_path(r, sx, sy, x, y);
                    draw_nahla(r, sx, sy); break;
                case TILE_ISLAND_CAMP:
                case TILE_ISLAND_LAGOON:
                    draw_island_grass(r, sx, sy, x, y); break;
                case TILE_ISLAND_MARKER:
                case TILE_ISLAND_STATUE:
                case TILE_ISLAND_TEMPLE_GATE:
                    draw_island_path(r, sx, sy, x, y); break;
                case TILE_TEMPLE_FLOOR:
                    draw_temple_floor(r, sx, sy, x, y); break;
                case TILE_TEMPLE_WALL:
                    draw_temple_wall(r, sx, sy, x, y); break;
                case TILE_TEMPLE_ENTRANCE:
                    draw_temple_entrance(r, sx, sy); break;
                case TILE_TEMPLE_ALTAR:
                    draw_temple_altar(r, sx, sy, g->temple_alignment); break;
                case TILE_TEMPLE_MOON_DOOR_CLOSED:
                    draw_temple_moon_door(r, sx, sy, 0); break;
                case TILE_TEMPLE_MOON_DOOR_OPEN:
                    draw_temple_moon_door(r, sx, sy, 1); break;
                case TILE_TEMPLE_SOLAR_TRAP:
                    draw_temple_solar_trap(r, sx, sy,
                        !g->temple_alignment); break;
                case TILE_TEMPLE_DORMANT_SENTINEL:
                    draw_temple_dormant_sentinel(r, sx, sy); break;
                case TILE_TEMPLE_VAULT_DOOR:
                    draw_temple_vault_door(r, sx, sy); break;
                case TILE_TEMPLE_TREASURE:
                    draw_temple_treasure(r, sx, sy); break;
                case TILE_TEMPLE_WATER:
                    draw_temple_water(r, sx, sy, x, y); break;
                case TILE_TEMPLE_RUBBLE:
                    draw_temple_rubble(r, sx, sy); break;
                case TILE_LABYRINTH_FLOOR:
                    draw_labyrinth_floor(r, sx, sy, x, y); break;
                case TILE_LABYRINTH_WALL:
                    draw_labyrinth_wall(r, sx, sy, x, y); break;
                case TILE_LABYRINTH_EXIT:
                    draw_labyrinth_exit(r, sx, sy); break;
                case TILE_LABYRINTH_SWITCH_OFF:
                    draw_labyrinth_switch(r, sx, sy, 0); break;
                case TILE_LABYRINTH_SWITCH_ON:
                    draw_labyrinth_switch(r, sx, sy, 1); break;
                case TILE_LABYRINTH_GATE:
                    draw_labyrinth_gate(r, sx, sy); break;
                case TILE_LABYRINTH_RELIC:
                    draw_labyrinth_relic(r, sx, sy); break;
                case TILE_TOWN_EXIT: draw_town_path(r, sx, sy); break;
                case TILE_SHOP_BLACKSMITH:
                case TILE_SHOP_ALCHEMIST:
                case TILE_HEALER:
                case TILE_WITCH:
                case TILE_WATCHTOWER:
                case TILE_TAVERN: draw_town_floor(r, sx, sy); break;
                case TILE_ITEM:
                    draw_floor_item_with_underlay(r, g, x, y, sx, sy); break;
                case TILE_TRAP_HIDDEN:
                    draw_trap_underlay(r, g, x, y, sx, sy);
                    break;
                case TILE_TRAP_REVEALED:
                    draw_trap_underlay(r, g, x, y, sx, sy);
                    draw_trap_warning(r, sx, sy);
                    break;
                case TILE_TRAP_SPIKE:
                    draw_trap_underlay(r, g, x, y, sx, sy);
                    draw_trap_spike(r, sx, sy);
                    break;
                case TILE_TRAP_FIRE:
                    draw_trap_underlay(r, g, x, y, sx, sy);
                    draw_trap_fire(r, sx, sy);
                    break;
                case TILE_TRAP_POISON:
                    draw_trap_underlay(r, g, x, y, sx, sy);
                    draw_trap_poison(r, sx, sy);
                    break;
                default: draw_floor(r, sx, sy); break;
            }
            if (g->location == LOCATION_DUNGEON && dungeon_is_floor(g, x, y)) {
                unsigned int edges = 0;
                if (y > 0 && g->map.tiles[y - 1][x] == TILE_WALL) {
                    edges |= DUNGEON_EDGE_NORTH;
                }
                if (x < MAP_W - 1 && g->map.tiles[y][x + 1] == TILE_WALL) {
                    edges |= DUNGEON_EDGE_EAST;
                }
                if (y < MAP_H - 1 && g->map.tiles[y + 1][x] == TILE_WALL) {
                    edges |= DUNGEON_EDGE_SOUTH;
                }
                if (x > 0 && g->map.tiles[y][x - 1] == TILE_WALL) {
                    edges |= DUNGEON_EDGE_WEST;
                }
                if (edges != 0) {
                    draw_dungeon_wall_edge(r, sx, sy, x, y, edges);
                }
            }
            if (g->location == LOCATION_FOREST && forest_is_floor(g, x, y)) {
                unsigned int edges = 0;
                if (y > 0 && forest_is_tree(g->map.tiles[y - 1][x])) {
                    edges |= FOREST_EDGE_NORTH;
                }
                if (x < MAP_W - 1 && forest_is_tree(g->map.tiles[y][x + 1])) {
                    edges |= FOREST_EDGE_EAST;
                }
                if (y < MAP_H - 1 && forest_is_tree(g->map.tiles[y + 1][x])) {
                    edges |= FOREST_EDGE_SOUTH;
                }
                if (x > 0 && forest_is_tree(g->map.tiles[y][x - 1])) {
                    edges |= FOREST_EDGE_WEST;
                }
                if (edges != 0) {
                    draw_forest_tree_edge(r, sx, sy, x, y, edges);
                }
            }
            if (g->location == LOCATION_FOREST &&
                x >= landmark_x - 1 && x <= landmark_x + 1 &&
                y >= landmark_y - 1 && y <= landmark_y + 1 &&
                g->map.tiles[y][x] == TILE_FOREST_FLOOR) {
                draw_forest_ruin(r, sx, sy, x - landmark_x, y - landmark_y);
            }
            if (g->location == LOCATION_COAST) {
                TileType tile = coast_visible_tile(g, x, y);
                if (coast_is_water_surface(tile)) {
                    unsigned int edges = 0;
                    if (y > 0 && !coast_is_water_surface(coast_visible_tile(g, x, y - 1))) {
                        edges |= COAST_SHORE_NORTH;
                    }
                    if (x < MAP_W - 1 && !coast_is_water_surface(coast_visible_tile(g, x + 1, y))) {
                        edges |= COAST_SHORE_EAST;
                    }
                    if (y < MAP_H - 1 && !coast_is_water_surface(coast_visible_tile(g, x, y + 1))) {
                        edges |= COAST_SHORE_SOUTH;
                    }
                    if (x > 0 && !coast_is_water_surface(coast_visible_tile(g, x - 1, y))) {
                        edges |= COAST_SHORE_WEST;
                    }
                    if (edges != 0) {
                        draw_coast_shore(r, sx, sy, x, y, edges, tile);
                    }
                }
            }
        }
    }

    // Regional mechanisms retain their terrain IDs even when holding loot.
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *item = &g->floor_items[i];
        if (!item->active || !viewport_is_visible(v, item->x, item->y)) {
            continue;
        }
        TileType tile = g->map.tiles[item->y][item->x];
        if (tile == TILE_MOUNTAIN_WEAK_BRIDGE || tile == TILE_MOUNTAIN_CACHE ||
            tile == TILE_MOUNTAIN_BRIDGE || tile == TILE_MOUNTAIN_CAVE_FLOOR ||
            tile == TILE_COAST_DRAINED_WATER || tile == TILE_COAST_CHANNEL_DRY ||
            map_is_coast_object(tile)) {
            draw_floor_item(r, viewport_to_screen_x(v, item->x),
                viewport_to_screen_y(v, item->y));
        }
    }

    // Gates and buildings span several town cells; draw them over the map.
    if (g->location == LOCATION_TOWN) {
        draw_town_gate(r,
            viewport_to_screen_x(v, 18), viewport_to_screen_y(v, 0),
            TOWN_EXIT_DUNGEON);
        draw_town_gate(r,
            viewport_to_screen_x(v, 0), viewport_to_screen_y(v, 10),
            TOWN_EXIT_FOREST);
        draw_town_gate(r,
            viewport_to_screen_x(v, TOWN_W - 3), viewport_to_screen_y(v, 10),
            TOWN_EXIT_MOUNTAINS);
        draw_town_gate(r,
            viewport_to_screen_x(v, 18),
            viewport_to_screen_y(v, TOWN_H - 2), TOWN_EXIT_COAST);
        draw_shop_blacksmith(r,
            viewport_to_screen_x(v, TOWN_BLACKSMITH_X), viewport_to_screen_y(v, TOWN_BLACKSMITH_Y));
        draw_shop_alchemist(r,
            viewport_to_screen_x(v, TOWN_ALCHEMIST_X),
            viewport_to_screen_y(v, TOWN_ALCHEMIST_Y));
        draw_witch_hut(r,
            viewport_to_screen_x(v, TOWN_WITCH_X),
            viewport_to_screen_y(v, TOWN_WITCH_Y));
        draw_tavern(r,
            viewport_to_screen_x(v, 5), viewport_to_screen_y(v, 16));
        draw_healer_house(r, viewport_to_screen_x(v, TOWN_HEALER_X),
            viewport_to_screen_y(v, TOWN_HEALER_Y));
        draw_harbor(r,
            viewport_to_screen_x(v, TOWN_HARBOR_X),
            viewport_to_screen_y(v, TOWN_HARBOR_Y));
        draw_labyrinth_entrance(r,
            viewport_to_screen_x(v, TOWN_LABYRINTH_X),
            viewport_to_screen_y(v, TOWN_LABYRINTH_Y),
            g->rook_quest_state == 1 || g->rook_quest_state == 2);
    }

    if (g->location == LOCATION_ISLAND) {
        draw_island_temple(r, viewport_to_screen_x(v, 10),
            viewport_to_screen_y(v, 0));
        draw_island_camp(r, viewport_to_screen_x(v, 4),
            viewport_to_screen_y(v, 12));
        draw_island_marker(r, viewport_to_screen_x(v, 8),
            viewport_to_screen_y(v, 7));
        draw_island_statue(r, viewport_to_screen_x(v, 27),
            viewport_to_screen_y(v, 7));
        draw_island_lagoon(r, viewport_to_screen_x(v, 26),
            viewport_to_screen_y(v, 11));
        draw_island_dock(r, viewport_to_screen_x(v, 14),
            viewport_to_screen_y(v, 18));
        draw_island_ship(r, viewport_to_screen_x(v, 25),
            viewport_to_screen_y(v, 17));
        draw_rowan(r, viewport_to_screen_x(v, ISLAND_CAPTAIN_X),
            viewport_to_screen_y(v, ISLAND_CAPTAIN_Y));
        for (int i = 0; i < g->floor_item_count; i++) {
            const FloorItem *item = &g->floor_items[i];
            if (item->active && viewport_is_visible(v, item->x, item->y)) {
                draw_floor_item(r, viewport_to_screen_x(v, item->x),
                    viewport_to_screen_y(v, item->y));
            }
        }
    }

    // Draw enemies
    if (g->location == LOCATION_DUNGEON ||
        g->location == LOCATION_FOREST ||
        g->location == LOCATION_MOUNTAINS ||
        g->location == LOCATION_COAST ||
        g->location == LOCATION_TEMPLE) {
        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *e = &g->enemies[i];
            if (!e->active) continue;
            if (!viewport_is_visible(v, e->x, e->y)) continue;
            int sx = viewport_to_screen_x(v, e->x);
            int sy = viewport_to_screen_y(v, e->y);
            if (e->type == ENEMY_FALLEN_SUN_GUARDIAN &&
                e->hp <= e->max_hp / 2) {
                draw_fallen_sun_guardian_broken(r, sx, sy);
            } else {
                draw_enemy(r, sx, sy, e->type);
            }
            if (e->dain_fragment) {
                int px = sx * TILE_SIZE;
                int py = sy * TILE_SIZE;
                SDL_SetRenderDrawColor(r->sdl, 242, 190, 55, 255);
                SDL_Rect marker = {px + 2, py + 2, TILE_SIZE - 4,
                    TILE_SIZE - 4};
                SDL_RenderDrawRect(r->sdl, &marker);
                SDL_RenderDrawRect(r->sdl, &marker);
            }
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
        int witch_w = 0;
        int forest_w = 0;
        int forest_h = 0;
        int dungeon_w = 0;
        int dungeon_h = 0;
        int mountains_w = 0;
        int mountains_h = 0;
        int tavern_w = 0;
        int harbor_w = 0;
        int labyrinth_w = 0;
        int coast_w = 0;
        int coast_h = 0;
        TTF_SizeText(r->font_tiny, "BLACKSMITH", &blacksmith_w, NULL);
        TTF_SizeText(r->font_tiny, "ALCHEMIST", &alchemist_w, NULL);
        TTF_SizeText(r->font_tiny, "WITCH", &witch_w, NULL);
        TTF_SizeText(r->font_tiny, "FOREST", &forest_w, &forest_h);
        TTF_SizeText(r->font_tiny, "DUNGEON", &dungeon_w, &dungeon_h);
        TTF_SizeText(r->font_tiny, "MOUNTAINS", &mountains_w, &mountains_h);
        TTF_SizeText(r->font_tiny, "TAVERN", &tavern_w, NULL);
        TTF_SizeText(r->font_tiny, "HARBOR", &harbor_w, NULL);
        TTF_SizeText(r->font_tiny, "LABYRINTH", &labyrinth_w, NULL);
        int healer_w = 0;
        TTF_SizeText(r->font_tiny, "HEALER", &healer_w, NULL);
        renderer_draw_text(r, "HEALER",
            viewport_to_screen_x(v, TOWN_HEALER_X) * TILE_SIZE +
                (TOWN_HEALER_W * TILE_SIZE - healer_w) / 2,
            viewport_to_screen_y(v, TOWN_HEALER_Y - 1) * TILE_SIZE,
            label, r->font_tiny);
        TTF_SizeText(r->font_tiny, "SUNKEN COAST", &coast_w, &coast_h);
        int bx = viewport_to_screen_x(v, TOWN_BLACKSMITH_X) * TILE_SIZE
            + (5 * TILE_SIZE - blacksmith_w) / 2;
        int by = viewport_to_screen_y(v, TOWN_BLACKSMITH_Y - 1) * TILE_SIZE;
        int ax = viewport_to_screen_x(v, TOWN_ALCHEMIST_X) * TILE_SIZE
            + (5 * TILE_SIZE - alchemist_w) / 2;
        int ay = viewport_to_screen_y(v, TOWN_ALCHEMIST_Y - 1) * TILE_SIZE;
        int witch_x = viewport_to_screen_x(v, TOWN_WITCH_X) * TILE_SIZE
            + (TOWN_WITCH_W * TILE_SIZE - witch_w) / 2;
        int witch_y = viewport_to_screen_y(v, TOWN_WITCH_Y - 1) * TILE_SIZE;
        int tavern_x = viewport_to_screen_x(v, 5) * TILE_SIZE
            + (7 * TILE_SIZE - tavern_w) / 2;
        int tavern_y = viewport_to_screen_y(v, 15) * TILE_SIZE;
        int harbor_x = viewport_to_screen_x(v, TOWN_HARBOR_X) * TILE_SIZE
            + (TOWN_HARBOR_W * TILE_SIZE - harbor_w) / 2;
        int harbor_y = viewport_to_screen_y(v, TOWN_HARBOR_Y - 1) * TILE_SIZE;
        if (bx > 0 && by > 0) {
            renderer_draw_text(r, "BLACKSMITH", bx, by, label, r->font_tiny);
        }
        if (ax > 0 && ay > 0) {
            renderer_draw_text(r, "ALCHEMIST", ax, ay, label, r->font_tiny);
        }
        if (witch_x > 0 && witch_y > 0) {
            renderer_draw_text(r, "WITCH", witch_x, witch_y, label,
                r->font_tiny);
        }
        if (tavern_x > 0 && tavern_y > 0) {
            renderer_draw_text(r, "TAVERN", tavern_x, tavern_y, label,
                r->font_tiny);
        }
        if (harbor_x > 0 && harbor_y > 0) {
            renderer_draw_text(r, "HARBOR", harbor_x, harbor_y,
                label, r->font_tiny);
        }
        renderer_draw_text(r, "LABYRINTH",
            viewport_to_screen_x(v, TOWN_LABYRINTH_X) * TILE_SIZE +
                (TILE_SIZE - labyrinth_w) / 2,
            viewport_to_screen_y(v, TOWN_LABYRINTH_Y - 3) * TILE_SIZE,
            g->rook_quest_state == 1 || g->rook_quest_state == 2 ?
                label : (SDL_Color){105, 105, 90, 255}, r->font_tiny);
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

    if (g->location == LOCATION_TAVERN) {
        SDL_Color name = {182, 214, 232, 255};
        int name_w = 0;
        TTF_SizeText(r->font_tiny, "ELOWEN", &name_w, NULL);
        int name_x = viewport_to_screen_x(v, 10) * TILE_SIZE +
            (TILE_SIZE - name_w) / 2;
        int name_y = viewport_to_screen_y(v, 6) * TILE_SIZE;
        renderer_draw_text(r, "ELOWEN", name_x, name_y, name, r->font_tiny);
        TTF_SizeText(r->font_tiny, "DAIN", &name_w, NULL);
        name_x = viewport_to_screen_x(v, 18) * TILE_SIZE +
            (TILE_SIZE - name_w) / 2;
        renderer_draw_text(r, "DAIN", name_x, name_y,
            (SDL_Color){218, 164, 84, 255}, r->font_tiny);
        TTF_SizeText(r->font_tiny, "ALDER", &name_w, NULL);
        name_x = viewport_to_screen_x(v, 28) * TILE_SIZE +
            (TILE_SIZE - name_w) / 2;
        renderer_draw_text(r, "ALDER", name_x, name_y,
            (SDL_Color){126, 190, 112, 255}, r->font_tiny);
        TTF_SizeText(r->font_tiny, "MARA", &name_w, NULL);
        name_x = viewport_to_screen_x(v, 31) * TILE_SIZE +
            (TILE_SIZE - name_w) / 2;
        name_y = viewport_to_screen_y(v, 17) * TILE_SIZE;
        renderer_draw_text(r, "MARA", name_x, name_y,
            (SDL_Color){75, 196, 201, 255}, r->font_tiny);
    }

    if (g->location == LOCATION_ISLAND) {
        SDL_Color label = {235, 201, 92, 255};
        int width = 0;
        TTF_SizeText(r->font_tiny, "RUINED TEMPLE", &width, NULL);
        renderer_draw_text(r, "RUINED TEMPLE",
            viewport_to_screen_x(v, 20) * TILE_SIZE - width / 2,
            viewport_to_screen_y(v, 1) * TILE_SIZE, label, r->font_tiny);
        TTF_SizeText(r->font_tiny, "CAPTAIN ROWAN", &width, NULL);
        renderer_draw_text(r, "CAPTAIN ROWAN",
            viewport_to_screen_x(v, ISLAND_CAPTAIN_X) * TILE_SIZE +
                (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, ISLAND_CAPTAIN_Y - 1) * TILE_SIZE,
            (SDL_Color){182, 214, 232, 255}, r->font_tiny);
    }

    // Draw spell/projectile trail
    if (g->trail_frames > 0) {
        int timed_fireball = g->trail_effect == TRAIL_EFFECT_FIREBALL &&
            g->trail_count > 0;
        if (timed_fireball) {
            Uint32 elapsed = SDL_GetTicks() - g->trail_started_at;
            if (elapsed < SPELL_TRAVEL_MS) {
                int lead = (int)(elapsed * g->trail_count / SPELL_TRAVEL_MS);
                if (lead >= g->trail_count) {
                    lead = g->trail_count - 1;
                }
                TrailTile *t = &g->trail[lead];
                if (viewport_is_visible(v, t->x, t->y)) {
                    draw_fireball(r, viewport_to_screen_x(v, t->x),
                        viewport_to_screen_y(v, t->y), g->player.last_dx,
                        g->player.last_dy, (int)(elapsed / 60));
                }
            } else if (elapsed < SPELL_FIREBALL_MS) {
                TrailTile *t = &g->trail[g->trail_count - 1];
                if (viewport_is_visible(v, t->x, t->y)) {
                    draw_fireball_impact(r, viewport_to_screen_x(v, t->x),
                        viewport_to_screen_y(v, t->y), elapsed - SPELL_TRAVEL_MS);
                }
            } else {
                g->trail_frames = 0;
            }
        } else if (g->trail_effect == TRAIL_EFFECT_MAGIC_ARROW &&
            g->trail_count > 0) {
            Uint32 elapsed = SDL_GetTicks() - g->trail_started_at;
            if (elapsed < SPELL_ARROW_MS) {
                int impact = elapsed >= SPELL_TRAVEL_MS;
                int lead = impact ? g->trail_count - 1
                    : (int)(elapsed * g->trail_count / SPELL_TRAVEL_MS);
                TrailTile *t = &g->trail[lead];
                if (t->active && viewport_is_visible(v, t->x, t->y)) {
                    draw_magic_arrow(r, viewport_to_screen_x(v, t->x),
                        viewport_to_screen_y(v, t->y), g->player.last_dx,
                        g->player.last_dy, impact && t->is_impact,
                        (int)(elapsed / 60));
                }
            } else {
                g->trail_frames = 0;
            }
        } else if (g->trail_effect == TRAIL_EFFECT_WEAPON_ARROW) {
            Uint32 elapsed = SDL_GetTicks() - g->trail_started_at;
            if (elapsed < SPELL_ARROW_MS && g->trail_count > 0) {
                int impact = elapsed >= SPELL_TRAVEL_MS;
                float progress = impact ? 1.0f
                    : (float)elapsed / SPELL_TRAVEL_MS;
                TrailTile *target = &g->trail[g->trail_count - 1];
                int start_x = viewport_to_screen_x(v, g->player.x) *
                    TILE_SIZE + TILE_SIZE / 2;
                int start_y = viewport_to_screen_y(v, g->player.y) *
                    TILE_SIZE + TILE_SIZE / 2;
                int target_x = viewport_to_screen_x(v, target->x) *
                    TILE_SIZE + TILE_SIZE / 2;
                int target_y = viewport_to_screen_y(v, target->y) *
                    TILE_SIZE + TILE_SIZE / 2;
                int arrow_x = start_x + (int)((target_x - start_x) * progress);
                int arrow_y = start_y + (int)((target_y - start_y) * progress);
                draw_weapon_arrow_at(r, arrow_x, arrow_y,
                    g->player.last_dx, g->player.last_dy,
                    impact && target->is_impact);
            } else {
                g->trail_frames = 0;
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
        if (!timed_fireball &&
            g->trail_effect != TRAIL_EFFECT_MAGIC_ARROW &&
            g->trail_effect != TRAIL_EFFECT_WEAPON_ARROW) {
            g->trail_frames--;
        }
    }

    if (g->dialogue_active && g->location == LOCATION_FOREST &&
        strcmp(g->dialogue_speaker, "Forest Warden") == 0 &&
        viewport_is_visible(v, g->dialogue_x, g->dialogue_y)) {
        draw_forest_warden(r,
            viewport_to_screen_x(v, g->dialogue_x),
            viewport_to_screen_y(v, g->dialogue_y),
            g->dialogue_x, g->dialogue_y);
    }

    // Draw player
    const Item *equipped_weapon = NULL;
    if (g->equipped_main_hand >= 0 &&
        g->equipped_main_hand < g->inventory_count) {
        equipped_weapon = &g->inventory[g->equipped_main_hand];
    }
    const Item *off_hand_weapon = NULL;
    if (g->equipped_off_hand >= 0 &&
        g->equipped_off_hand < g->inventory_count) {
        off_hand_weapon = &g->inventory[g->equipped_off_hand];
    }
    const Item *equipped_armor = NULL;
    if (g->equipped_armor >= 0 &&
        g->equipped_armor < g->inventory_count) {
        equipped_armor = &g->inventory[g->equipped_armor];
    }
    draw_player(r,
        viewport_to_screen_x(v, g->player.x),
        viewport_to_screen_y(v, g->player.y),
        g->player.player_class, equipped_weapon, off_hand_weapon,
        equipped_armor,
        g->player.last_dx, g->player.last_dy);

    draw_dialogue_bubble(r, g, v);

    if (town_scaled || tavern_scaled || island_scaled || labyrinth_scaled) {
        SDL_RenderSetScale(r->sdl, 1.0f, 1.0f);
    }
    if (tavern_scaled) {
        SDL_RenderSetViewport(r->sdl, NULL);
    }

    // Draw info panel
    info_panel_draw(r, g);

    // Draw message bar
    message_bar_draw(r, g);

    // Draw minimap overlay in top-left corner of the viewport
    minimap_draw(r, g);
}
