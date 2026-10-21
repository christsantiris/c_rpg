#include "regional_terrain_renderer.h"
#include "../game/regional_terrain.h"
#include "../game/landscapes.h"
#include "../game/castle.h"
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

void draw_area_description(Renderer *r, const GameState *g, const Viewport *v) {
    const char *title = NULL;
    const char *line1 = NULL;
    const char *line2 = NULL;
    char status[64];
    char floor_name[40];
    if (g->location == LOCATION_ASHEN) {
        title = "VOLCANIC CROSSINGS";
        line1 = "C: COOL   !: WARNING   H: HOT";
        line2 = "VALVES MAKE LASTING PATHS";
        if (g->level == ASHEN_DEPTH) {
            title = "THE CINDER FORGE";
            line1 = "SLUICES DIVERT WARNED LAVA";
            line2 = "RINGS STRIKE INNER THEN OUTER";
        }
    } else if (g->location == LOCATION_GLASSDEEP) {
        title = "CRYSTAL GALLERIES";
        line1 = "TURN FACE: AIM NORTH AT R";
        line2 = "SIDE GALLERIES BYPASS THE SEAL";
        if (g->level == GLASSDEEP_DEPTH) {
            title = "THE PRISM CHAMBER";
            line1 = "ROTATE FACES BETWEEN ATTACKS";
            line2 = "WARNINGS LOCK THE BEAM DIRECTIONS";
        }
    } else if (g->location == LOCATION_FROSTFELL) {
        title = "GLACIER RIDGES";
        line1 = "SLICK ICE SLIDES TO SNOW";
        line2 = "BRITTLE SHORTCUTS BREAK BEHIND YOU";
        if (g->level == FROSTFELL_DEPTH) {
            title = "THE KRAKEN'S FROZEN LAKE";
            line1 = "ORANGE ICE WARNS OF THE NEXT HIT";
            line2 = "ATTACK WHILE TENTACLES RECOVER";
        }
    } else if (g->location == LOCATION_DESERT) {
        title = "SANDSTONE MESA FIELDS";
        line1 = "DENSE PATROLS; ROCKS BLOCK FIRE";
        line2 = "WELLS: 25% HP / MP, ONCE PER TRIP";
        if (g->level == DESERT_DEPTH) {
            title = "THE PHARAOH'S TOMB";
            line1 = "OBELISKS BLOCK CURSE BEAMS";
            line2 = "S MARKS SHELTER FROM THE STORM";
        }
    } else if (g->location == LOCATION_SWAMP) {
        title = "BLACKWATER ISLANDS";
        line1 = "REEDS HIDE FROM DISTANT FOES";
        line2 = "NEARBY / WOUNDED FOES STILL FOLLOW";
        if (g->level == SWAMP_BOSS_LEVEL) {
            title = "THE SWAMP DEMON'S LAIR";
            line1 = "VENTS DRAIN ROT AND EXPOSE THE BOSS";
            line2 = "MOVE OFF THE WARNED GROUND";
        }
    } else if (g->location == LOCATION_DRAGONSPINE) {
        title = "RAVINES AND CLIFF LEDGES";
        line1 = "PUSH BOULDERS FOR MOVABLE COVER";
        line2 = "KEEP MULTIPLE ROUTES OPEN";
        if (g->level == DRAGONSPINE_DEPTH) {
            title = "THE DRAGON'S BASALT DEN";
            line1 = "BASALT BLOCKS THE DRAGON'S FIRE";
            line2 = "WATCH FOR CRACKED AND WEAK COVER";
        }
    } else if (g->location == LOCATION_MOONVEIL) {
        title = "MOONLIT GARDEN LOOPS";
        line1 = "WAYSTONES: I TO I, II TO II";
        line2 = "USE INTERACT TO CROSS THE GARDEN";
        if (g->level == MOONVEIL_DEPTH) {
            title = "THE THORN REGENT'S GARDEN";
            line1 = "TEND FLOWER NODES TO OPEN A FLANK";
            line2 = "LEAVE WARNED ROOT AND PETAL LANES";
        }
    } else if (g->location == LOCATION_DUNGEON) {
        title = "CRYPTS OF OAKHAVEN";
        line1 = "WALLS BLOCK ARROWS AND SPELLS";
        line2 = "RESTORE BURIAL SEALS FOR ELOWEN";
        if (g->level == DUNGEON_DEPTH) {
            title = "THE LICH KING'S CRYPT";
            line1 = "SILENCE SEALS TO STOP GRAVE WAVES";
            line2 = "LEAVE WARNED LANCE AND RING TILES";
        }
    } else if (g->location == LOCATION_FOREST) {
        title = "HAUNTED FOREST TRAILS";
        line1 = "PATHS LINK THE FOREST CLEARINGS";
        line2 = "GROVE BOSS GUARDS THE TOWN SHORTCUT";
        if (g->level == FOREST_BOSS_LEVEL) {
            title = "THE NECROMANCER'S GROVE";
            line1 = "CONSECRATE GRAVES TO STOP WAVES";
            line2 = "LEAVE NUMBERED ATTACK WARNINGS";
        }
    } else if (g->location == LOCATION_MOUNTAINS) {
        title = "GOBLIN MOUNTAIN PASSES";
        line1 = "NARROW PASSES; ROCKS BLOCK FIRE";
        line2 = "WATCH FOR BRIDGES AND ROCKFALLS";
        if (g->level == 2 || g->level == 7) {
            line1 = "WEAK BRIDGES COLLAPSE BEHIND YOU";
            line2 = "INTERACT BESIDE GAPS TO REBUILD";
        } else if (g->level == MOUNTAIN_BOSS_LEVEL) {
            title = "THE GOBLIN KING'S FORTRESS";
            line1 = "PRIME POWDER KEGS TO BREAK COVER";
            line2 = "LEAVE THE WARNED AXE AND BOMB LANES";
        }
    } else if (g->location == LOCATION_COAST) {
        title = "TIDAL SANDBANKS";
        int low = 0;
        for (int y = 0; y < MAP_H && !low; y++) {
            for (int x = 0; x < MAP_W; x++) {
                low |= g->map.tiles[y][x] == TILE_COAST_DRAINED_WATER;
            }
        }
        line1 = low ? "BLUE: EXPOSED / AMBER: FLOODED" : "BLUE: FLOODED / AMBER: EXPOSED";
        line2 = "WALK ON SAND AND PALE SHALLOWS";
        if (g->level == COAST_DEPTH) {
            line1 = "DRAIN POCKETS TO OPEN DRY ROUTES";
            line2 = "LEAVE THE QUEEN'S WARNED LANES";
        }
    } else if (g->location == LOCATION_TEMPLE) {
        title = g->temple_alignment ? "MOONBOUND TEMPLE" : "SUNBOUND TEMPLE";
        line1 = game_temple_dormant_sentinels(g) > 0 ? "USE ALTARS TO AWAKEN MOON FOES" : "ALTARS SWITCH SUN / MOON ALIGNMENT";
        int remaining = game_temple_remaining_enemies(g);
        snprintf(status, sizeof(status), "%s %s: %d FOES LEFT", g->level == TEMPLE_DEPTH ? "TREASURE" : "STAIRS", remaining > 0 ? "SEALED" : "OPEN", remaining);
        line2 = status;
    } else if (g->location == LOCATION_LABYRINTH) {
        title = "ROOK'S LABYRINTH";
        line1 = "FIVE FLOORS; ONE RUNE PER FLOOR";
        line2 = "FALSE STAIRS LEAD TO DEAD ENDS";
        if (g->level == LABYRINTH_DEPTH) {
            title = "THE MINOTAUR'S VAULT";
            line1 = "LIGHT FIVE RUNES TO OPEN THE GATE";
            line2 = "BAIT CHARGES INTO STONE TO STAGGER";
        }
    } else if (g->location == LOCATION_CATACOMBS) {
        title = "ROYAL BURIAL CHAMBERS";
        line1 = "EXTINGUISH BRAZIERS TO HALT REVIVAL";
        line2 = "BURIAL PLATES WARN BEFORE STRIKING";
        if (g->level == CATACOMBS_DEPTH) {
            title = "THE GRAVE MARSHAL'S COURT";
            line1 = "BRAZIERS INTERRUPT FUNERAL CHANNELS";
            line2 = "LEAVE WARNED POLEARM AND CROSS LANES";
        }
    } else if (g->location == LOCATION_CASTLE_INTERIOR) {
        snprintf(floor_name, sizeof(floor_name), "%s", castle_floor_name(g->level));
        title = SDL_strupr(floor_name);
        line1 = "LEAVE WARNED FIRE AND ATTACK LANES";
        line2 = "USE LEVERS TO OPEN PORTCULLISES";
        if (g->level == 2) {
            line1 = "SIDE LEVERS INTERRUPT CHARGES";
            line2 = "DEFEAT THE CASTELLAN TO CLIMB";
        } else if (g->level == 4) {
            line1 = "DISABLE THE LINKED WARD PEDESTAL";
            line2 = "DEFEAT THE ARCANIST TO CLIMB";
        } else if (g->level == CASTLE_DEPTH) {
            line1 = "DISABLE BOTH WARDS TO WEAKEN VEYR";
            line2 = "WATCH THE CROWN'S WARNED LANES";
        }
    } else if (g->location == LOCATION_CROWNROAD || g->location == LOCATION_KING_ROAD_WEST) {
        title = g->location == LOCATION_CROWNROAD ? "CROWN ROAD EAST" : "CROWN ROAD WEST";
        line1 = "BANDITS AND HORSEMEN PATROL AHEAD";
        line2 = "BREAK ARCHER SIGHTLINES WITH COVER";
    } else if (g->location == LOCATION_ESCAPE_TUNNEL) {
        title = "JAIL ESCAPE TUNNEL";
        line1 = "GUIDE TOMAS THROUGH THE AMBUSHES";
        line2 = "WAIT FOR HIM BEFORE MOVING ON";
    } else if (g->location == LOCATION_FOREST_ROAD) {
        title = "FOREST ROAD";
        line1 = "WEST: STILLBURY / EAST: OAKHAVEN";
        line2 = "NORTH FORK: NECROMANCER'S GROVE";
    } else if (g->location == LOCATION_HIGH_PASS) {
        title = "THE HIGH PASS";
        line1 = "NORTH: RIDGESHIRE / SOUTH: OAKHAVEN";
        line2 = "EAST FORK: GOBLIN KING'S HALL";
    } else if (g->location == LOCATION_SWAMP_ROAD) {
        title = "SWAMP SHORTCUT";
        line1 = "NORTH: ROSEMOOR / SOUTH: STILLBURY";
        line2 = "EAST FORK: DEMON'S CHAMBER";
    } else if (g->location == LOCATION_CASTLE) {
        title = "CASTLE CROSSROADS";
        line1 = "WEST: ROSEMOOR / EAST: RIDGESHIRE";
        line2 = "NORTH: CASTLE / SOUTH: CATACOMBS";
    } else if (g->location == LOCATION_ISLAND) {
        title = "RUINED ISLE";
        line1 = "TALK TO NAHLA BESIDE THE TEMPLE";
        line2 = "THE TEMPLE GATE BEGINS THE CLIMB";
    } else {
        return;
    }

    int width = r->screen_w - INFO_PANEL_W;
    int x = width - 320;
    int y = width >= 440 ? 8 : 76;
    // Boss health and attack panels take priority over the area description.
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (e->active && e->is_boss && e->max_hp > 0 &&
            viewport_is_visible(v, e->x, e->y) && map_is_explored(&g->map, e->x, e->y)) {
            return;
        }
    }
    if (width < 328 || r->tiles_y * TILE_SIZE < y + 80) {
        return;
    }
    SDL_Rect panel = {x, y, 312, 72};
    SDL_SetRenderDrawColor(r->sdl, 22, 23, 35, 255);
    SDL_RenderFillRect(r->sdl, &panel);
    SDL_SetRenderDrawColor(r->sdl, 105, 125, 162, 255);
    SDL_RenderDrawRect(r->sdl, &panel);
    SDL_Color text = {217, 224, 237, 255};
    renderer_draw_text(r, title, x + 10, y + 9, (SDL_Color){142, 215, 231, 255}, r->font_small);
    renderer_draw_text(r, line1, x + 10, y + 32, text, r->font_tiny);
    renderer_draw_text(r, line2, x + 10, y + 50, text, r->font_tiny);
}
