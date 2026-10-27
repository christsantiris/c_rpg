#include "../game/boss_gates.h"
#include "../game/approach.h"
#include "minimap_renderer.h"
#include "../game/jail.h"
#include "../game/spice_skiff.h"
#include "../game/stormbell.h"
#include "../game/war_machine.h"
#include "../game/crown_quest.h"

// Minimap renders as a corner overlay on the game viewport.
// Each tile is 1x1px, scaled 1:2 (100x50px total).
// A semi-transparent dark background sits behind it for readability.
#define MINIMAP_SCALE 2
#define MINIMAP_PAD   6

void minimap_draw(Renderer *r, const GameState *g) {
    if (g->location != LOCATION_APPROACH && g->location != LOCATION_GIANTS_CROWN && g->location != LOCATION_CROWNROAD_NORTH && g->location != LOCATION_IRONFANG && g->location != LOCATION_STORMWATCH && g->location != LOCATION_MANGROVES && g->location != LOCATION_ESCAPE_TUNNEL && g->location != LOCATION_DUNGEON &&
        g->location != LOCATION_FOREST &&
        g->location != LOCATION_MOUNTAINS &&
        g->location != LOCATION_DRAGONSPINE &&
        g->location != LOCATION_HIGH_PASS &&
        g->location != LOCATION_COAST &&
        g->location != LOCATION_SWAMP &&
        g->location != LOCATION_FROSTFELL &&
        g->location != LOCATION_DESERT &&
        g->location != LOCATION_ASHEN &&
        g->location != LOCATION_GLASSDEEP &&
        g->location != LOCATION_ELDERWOOD && g->location != LOCATION_CATACOMBS &&
        g->location != LOCATION_CASTLE_INTERIOR &&
        g->location != LOCATION_TEMPLE &&
        g->location != LOCATION_LABYRINTH) {
        return;
    }

    int ox = MINIMAP_PAD;
    int oy = MINIMAP_PAD;
    int map_w = MAP_W;
    int map_h = MAP_H;
    if (g->location == LOCATION_APPROACH) {
        map_w = APPROACH_W;
        map_h = APPROACH_H;
    }
    if (g->location == LOCATION_MANGROVES) {
        map_w = MANGROVE_W;
        map_h = MANGROVE_H;
    }
    if (g->location == LOCATION_ESCAPE_TUNNEL) {
        map_w = ESCAPE_TUNNEL_W;
        map_h = ESCAPE_TUNNEL_H;
    }
    if (g->location == LOCATION_SWAMP ||
        g->location == LOCATION_DRAGONSPINE ||
        g->location == LOCATION_DESERT ||
        g->location == LOCATION_ASHEN ||
        g->location == LOCATION_GLASSDEEP ||
        g->location == LOCATION_FROSTFELL) {
        map_w = SWAMP_MAP_W;
        map_h = SWAMP_MAP_H;
    }

    if (g->location == LOCATION_CROWNROAD_NORTH) {
        map_w = CROWN_NORTH_W;
        map_h = CROWN_NORTH_H;
    }
    if (g->location == LOCATION_GIANTS_CROWN) {
        map_w = GIANTS_W;
        map_h = GIANTS_H;
    }
    if (g->location == LOCATION_IRONFANG) {
        map_w = IRON_W;
        map_h = IRON_H;
    }
    if (g->location == LOCATION_STORMWATCH) {
        map_w = STORM_W;
        map_h = STORM_H;
    }
    if (g->location == LOCATION_ELDERWOOD) {
        map_w = ELDERWOOD_W;
        map_h = ELDERWOOD_H;
    }
    if (g->location == LOCATION_CATACOMBS) {
        map_w = CATACOMBS_W;
        map_h = CATACOMBS_H;
    }
    if (g->location == LOCATION_CASTLE_INTERIOR) {
        map_w = CASTLE_W;
        map_h = CASTLE_H;
    }

    // Dark semi-transparent background
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_Rect bg = { ox - 2, oy - 2,
        map_w / MINIMAP_SCALE + 4, map_h / MINIMAP_SCALE + 4 };
    SDL_SetRenderDrawColor(r->sdl, 0, 0, 0, 180);
    SDL_RenderFillRect(r->sdl, &bg);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_NONE);

    // Tiles — sample the full 2x2 block per pixel so 1-tile-wide
    // hallways are never missed due to stride skipping.
    for (int ty = 0; ty < map_h; ty += MINIMAP_SCALE) {
        for (int tx = 0; tx < map_w; tx += MINIMAP_SCALE) {
            int draw_x = ox + tx / MINIMAP_SCALE;
            int draw_y = oy + ty / MINIMAP_SCALE;
            int has_stair = 0;
            int has_floor = 0;
            int has_water = 0;

            for (int dy = 0; dy < MINIMAP_SCALE; dy++) {
                for (int dx = 0; dx < MINIMAP_SCALE; dx++) {
                    int sx = tx + dx;
                    int sy = ty + dy;
                    if (sx >= map_w || sy >= map_h) {
                        continue;
                    }
                    if (!map_is_explored(&g->map, sx, sy)) {
                        continue;
                    }
                    TileType tile = g->map.tiles[sy][sx];
                    if (tile == TILE_IRON_WATER || tile == TILE_IRON_TRESTLE || tile == TILE_STORM_WATER || tile == TILE_MANGROVE_WATER || tile == TILE_GLASSDEEP_POOL || tile == TILE_STONEWAKE_GAP) {
                        has_water = 1;
                    }
                    if (tile == TILE_CROWN_EXIT || tile == TILE_CROWN_SUPPLIES || tile == TILE_IRON_ENTRANCE || tile == TILE_IRON_EXIT || tile == TILE_IRON_LAMP_LIT || tile == TILE_IRON_SWITCH || tile == TILE_IRON_CACHE || tile == TILE_STORM_EXIT || tile == TILE_STORM_ENTRANCE || tile == TILE_STORM_SUPPLIES || tile == TILE_STORM_CRANK ||
                        tile == TILE_MANGROVE_EXIT || tile == TILE_MANGROVE_LANDING || tile == TILE_MANGROVE_RETURN || tile == TILE_MANGROVE_CACHE ||
                        tile == TILE_CASTLE_TRAP_OPEN || tile == TILE_CASTLE_WARD || tile == TILE_CASTLE_WARD_SPENT || tile == TILE_CASTLE_MUSTER ||
                        tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN ||
                        tile == TILE_ELDERWOOD_ENTRANCE || tile == TILE_ELDERWOOD_EXIT ||
                        tile == TILE_RETURN_EXIT || tile == TILE_DUNGEON_KEY ||
                        tile == TILE_DUNGEON_STAIRS_SEALED || tile == TILE_DUNGEON_STAIRS_RETURN ||
                        tile == TILE_CRYPT_KEY || tile == TILE_CRYPT_CACHE ||
                        tile == TILE_GIANTS_ENTRANCE || tile == TILE_GIANTS_EXIT || tile == TILE_GIANTS_CACHE || tile == TILE_PORTAL || tile == TILE_FOREST_ENTRANCE ||
                        tile == TILE_FOREST_EXIT || tile == TILE_FOREST_SHORTCUT || tile == TILE_MOUNTAIN_ENTRANCE ||
                        tile == TILE_MOUNTAIN_EXIT || tile == TILE_MOUNTAIN_SHORTCUT || tile == TILE_COAST_ENTRANCE ||
                        tile == TILE_COAST_EXIT || tile == TILE_TAVERN_EXIT ||
                        tile == TILE_SWAMP_ENTRANCE ||
                        tile == TILE_SWAMP_EXIT ||
                        tile == TILE_SWAMP_SHORTCUT ||
                        tile == TILE_FROST_ENTRANCE ||
                        tile == TILE_FROST_EXIT ||
                        tile == TILE_DESERT_ENTRANCE ||
                        tile == TILE_DESERT_EXIT ||
                        tile == TILE_ASHEN_ENTRANCE ||
                        tile == TILE_ASHEN_EXIT ||
                        tile == TILE_GLASSDEEP_ENTRANCE ||
                        tile == TILE_GLASSDEEP_EXIT ||
                        tile == TILE_SWAMP_DAUGHTER ||
                        tile == TILE_DRAGON_ENTRANCE ||
                        tile == TILE_DRAGON_EXIT ||
                        tile == TILE_DRAGON_TREASURE ||
                        tile == TILE_HIGH_PASS_ENTRANCE ||
                        tile == TILE_HIGH_PASS_EXIT ||
                        tile == TILE_TUNNEL_EXIT || tile == TILE_JAIL_HATCH ||
                        tile == TILE_NPC_TAMSIN || tile == TILE_HEARTSEED_ORDERS ||
                        tile == TILE_HEARTSEED_STAKE || tile == TILE_HEARTSEED_ALTAR ||
                        tile == TILE_NPC_ELOWEN || tile == TILE_NPC_DAIN ||
                        tile == TILE_NPC_ALDER || tile == TILE_NPC_MARA ||
                        tile == TILE_NPC_VEYRA || tile == TILE_WATCHFIRE_COLD || tile == TILE_WATCHFIRE_LIT ||
                        tile == TILE_NPC_ROOK || tile == TILE_NPC_OSWIN || tile == TILE_BURIAL_LEDGER ||
     tile == TILE_NPC_SELENE || tile == TILE_NPC_ORIN || tile == TILE_GLASSDEEP_RESONATOR || tile == TILE_GLASSDEEP_RESONATOR_LIT ||
                        tile == TILE_NPC_BRENNA || tile == TILE_FROST_JOURNAL || tile == TILE_NPC_FROST_SURVIVOR ||
                        tile == TILE_NPC_INNKEEPER ||
                        tile == TILE_NPC_HEALER || tile == TILE_NPC_WITCH ||
                        tile == TILE_NPC_CAIN ||
                        tile == TILE_NPC_BRAM ||
                        tile == TILE_NPC_ALDRIC ||
                        tile == TILE_NPC_ROWAN ||
                        tile == TILE_NPC_DRAGON_SEEKER ||
                        tile == TILE_NPC_GUILD_SEEKER || tile == TILE_DESERT_LAMP ||
                        tile == TILE_EMBERFORGE_MECHANISM || tile == TILE_EMBERFORGE_COLD || tile == TILE_EMBERFORGE_LIT ||
                        tile == TILE_COAST_BEACON_UNLIT ||
                        tile == TILE_COAST_BEACON_LIT ||
                        tile == TILE_FOREST_WARDEN ||
                        tile == TILE_TEMPLE_ENTRANCE ||
                        tile == TILE_TEMPLE_ALTAR ||
                        tile == TILE_TEMPLE_TREASURE ||
                        tile == TILE_LABYRINTH_EXIT ||
                        tile == TILE_LABYRINTH_SWITCH_OFF ||
                        tile == TILE_LABYRINTH_SWITCH_ON ||
                        tile == TILE_LABYRINTH_RELIC) {
                        has_stair = 1;
                    } else if (tile == TILE_COAST_TIDE_CONTROL ||
                        tile == TILE_COAST_SLUICE_CONTROL || tile == TILE_COAST_CACHE) {
                        has_stair = 1;
                    } else if (tile == TILE_MOUNTAIN_ROCKFALL ||
                        tile == TILE_MOUNTAIN_GATE || tile == TILE_MOUNTAIN_CACHE) {
                        has_stair = 1;
                    } else if (tile == TILE_DESERT_OASIS || tile == TILE_DESERT_OASIS_DRY ||
                    tile == TILE_ASHEN_VALVE || tile == TILE_ASHEN_VALVE_DIVERTED || (tile >= TILE_GLASS_FACE_N && tile <= TILE_GLASS_SEAL_OPEN)) {
                        has_stair = 1;
                    } else if (tile == TILE_STONEWAKE_CRANE_NEW || tile == TILE_STONEWAKE_CRANE_UP ||
                        tile == TILE_STONEWAKE_CRANE_DOWN || tile == TILE_STONEWAKE_CRANE_SALVAGE ||
                        tile == TILE_STONEWAKE_REMOTE || tile == TILE_STONEWAKE_SUPPLIES) {
                        has_stair = 1;
                    } else if (tile == TILE_FOREST_LANDMARK) {
                        has_stair = 1;
                    } else if (tile == TILE_BROKEN_BURIAL_SEAL) {
                        has_stair = 1;
                    } else if (tile == TILE_OSSUARY_BRAZIER || tile == TILE_OSSUARY_COLD ||
                        tile == TILE_MEMORIAL_BRAZIER || tile == TILE_MEMORIAL_COLD) {
                        has_stair = 1;
                    } else if (tile != TILE_GIANTS_WALL && tile != TILE_GIANTS_CRACK && tile != TILE_GIANTS_COLUMN &&
                        tile != TILE_STORM_WATER && tile != TILE_STORM_WALL && tile != TILE_STORM_SHUTTER_UP && tile != TILE_MANGROVE_WATER && tile != TILE_MANGROVE_ROOT && tile != TILE_CASTLE_WALL && tile != TILE_CASTLE_PILLAR &&
                        tile != TILE_CASTLE_BANNER && tile != TILE_CASTLE_GATE && tile != TILE_CASTLE_TABLE &&
                        tile != TILE_CASTLE_BOOKCASE && tile != TILE_CASTLE_THRONE && tile != TILE_CATACOMBS_WALL &&
                        tile != TILE_CATACOMBS_SARCOPHAGUS && tile != TILE_WALL &&
                        tile != TILE_ELDERWOOD_WALL && tile != TILE_ELDERWOOD_LOG && tile != TILE_ELDERWOOD_BRAMBLE &&
                        tile != TILE_FOREST_WALL &&
                        tile != TILE_MOUNTAIN_WALL &&
                        tile != TILE_MOUNTAIN_CHASM &&
                        tile != TILE_MOUNTAIN_HIDDEN_CAVE &&
                        tile != TILE_COAST_WALL &&
                        tile != TILE_COAST_SEA &&
                        tile != TILE_SWAMP_WALL && tile != TILE_SWAMP_WATER &&
                        tile != TILE_DRAGON_RAVINE && tile != TILE_DRAGON_BOULDER && tile != TILE_DRAGON_ASH_BOULDER &&
                        tile != TILE_DESERT_WALL && tile != TILE_DESERT_MESA &&
                        tile != TILE_PHARAOH_OBELISK &&
                        tile != TILE_ASHEN_WALL &&
                        tile != TILE_ASHEN_LAVA &&
                        tile != TILE_GLASSDEEP_WALL &&
                        tile != TILE_GLASSDEEP_POOL &&
                        tile != TILE_STONEWAKE_BOUNDARY && tile != TILE_STONEWAKE_LEGACY_BOUNDARY && tile != TILE_STONEWAKE_GAP &&
                        tile != TILE_STONEWAKE_WORKSHOP &&
                        tile != TILE_GLASS_SEAL_CLOSED &&
                        tile != TILE_FROST_WALL &&
                        tile != TILE_FROST_LAKE_HOLE &&
                        tile != TILE_FROST_BROKEN_ICE &&
                        tile != TILE_DRAGON_WALL &&
                        tile != TILE_COAST_DEEP_WATER &&
                        tile != TILE_COAST_CHANNEL_WATER &&
                        tile != TILE_TAVERN_WALL &&
                        tile != TILE_TAVERN_TABLE &&
                        tile != TILE_LOCKED_DOOR &&
                        tile != TILE_CRYPT_DOOR &&
                        tile != TILE_FOREST_HIDDEN_TRAIL &&
                        tile != TILE_TEMPLE_WALL &&
                        tile != TILE_TEMPLE_MOON_DOOR_CLOSED &&
                        tile != TILE_TEMPLE_DORMANT_SENTINEL &&
                        tile != TILE_TEMPLE_VAULT_DOOR &&
                        tile != TILE_LABYRINTH_WALL &&
                        tile != TILE_LABYRINTH_GATE) {
                        has_floor = 1;
                    }
                }
            }

            if (has_stair) {
                SDL_SetRenderDrawColor(r->sdl, 220, 180, 60, 255);
                SDL_RenderDrawPoint(r->sdl, draw_x, draw_y);
            } else if (has_floor) {
                if (g->location == LOCATION_GIANTS_CROWN) {
                    SDL_SetRenderDrawColor(r->sdl, 153, 169, 124, 255);
                } else if (g->location == LOCATION_MANGROVES) {
                    SDL_SetRenderDrawColor(r->sdl, 126, 135, 77, 255);
                } else if (g->location == LOCATION_FOREST) {
                    SDL_SetRenderDrawColor(r->sdl, 45, 95, 55, 255);
                } else if (g->location == LOCATION_MOUNTAINS ||
                    g->location == LOCATION_HIGH_PASS) {
                    SDL_SetRenderDrawColor(r->sdl, 125, 42, 32, 255);
                } else if (g->location == LOCATION_DRAGONSPINE) {
                    SDL_SetRenderDrawColor(r->sdl, 150, 185, 211, 255);
                } else if (g->location == LOCATION_FROSTFELL) {
                    SDL_SetRenderDrawColor(r->sdl, 205, 222, 236, 255);
                } else if (g->location == LOCATION_DESERT) {
                    SDL_SetRenderDrawColor(r->sdl, 218, 181, 110, 255);
                } else if (g->location == LOCATION_ASHEN) {
                    SDL_SetRenderDrawColor(r->sdl, 176, 100, 62, 255);
                } else if (g->location == LOCATION_CATACOMBS) {
                    SDL_SetRenderDrawColor(r->sdl, 166, 158, 134, 255);
                } else if (g->location == LOCATION_GLASSDEEP) {
                    SDL_SetRenderDrawColor(r->sdl, 175, 167, 128, 255);
                } else if (g->location == LOCATION_COAST) {
                    SDL_SetRenderDrawColor(r->sdl, 35, 125, 145, 255);
                } else if (g->location == LOCATION_TEMPLE) {
                    SDL_SetRenderDrawColor(r->sdl, 65, 105, 75, 255);
                } else if (g->location == LOCATION_TAVERN) {
                    SDL_SetRenderDrawColor(r->sdl, 135, 78, 39, 255);
                } else if (g->location == LOCATION_LABYRINTH) {
                    SDL_SetRenderDrawColor(r->sdl, 72, 92, 70, 255);
                } else {
                    SDL_SetRenderDrawColor(r->sdl, 145, 140, 195, 255);
                }
                SDL_RenderDrawPoint(r->sdl, draw_x, draw_y);
            } else if (has_water) {
                SDL_SetRenderDrawColor(r->sdl, 24, 70, 81, 255);
                SDL_RenderDrawPoint(r->sdl, draw_x, draw_y);
            }
        }
    }

    if (g->location == LOCATION_MANGROVES && g->mangroves.initialized &&
        map_is_explored(&g->map, g->mangroves.boat_x, g->mangroves.boat_y)) {
        SDL_SetRenderDrawColor(r->sdl, 210, 155, 79, 255);
        SDL_RenderDrawPoint(r->sdl, ox + g->mangroves.boat_x / MINIMAP_SCALE, oy + g->mangroves.boat_y / MINIMAP_SCALE);
    }
    int quest_x;
    int quest_y;
    if ((spice_skiff_marker(g, &quest_x, &quest_y) || stormbell_marker(g, &quest_x, &quest_y) || war_machine_marker(g, &quest_x, &quest_y) || crown_quest_marker(g, &quest_x, &quest_y)) && map_is_explored(&g->map, quest_x, quest_y)) {
        SDL_SetRenderDrawColor(r->sdl, 240, 200, 100, 255);
        SDL_RenderDrawPoint(r->sdl, ox + quest_x / MINIMAP_SCALE, oy + quest_y / MINIMAP_SCALE);
    }
    RoadCheckpoint checkpoints[2];
    int checkpoint_count = boss_gate_checkpoints(g, checkpoints);
    for (int n = 0; n < checkpoint_count; n++) {
        RoadCheckpoint point = checkpoints[n];
        if (!map_is_explored(&g->map, point.x, point.y)) {
            continue;
        }
        int x = ox + point.x / MINIMAP_SCALE;
        int y = oy + point.y / MINIMAP_SCALE;
        if (boss_gate_open(g, point.area)) {
            SDL_SetRenderDrawColor(r->sdl, 143, 202, 138, 255);
            SDL_RenderDrawLine(r->sdl, x - 2, y, x - 1, y + 1);
            SDL_RenderDrawLine(r->sdl, x - 1, y + 1, x + 2, y - 2);
        } else {
            SDL_SetRenderDrawColor(r->sdl, 225, 169, 99, 255);
            SDL_Rect lock = {x - 2, y - 1, 5, 4};
            SDL_RenderDrawRect(r->sdl, &lock);
            SDL_RenderDrawLine(r->sdl, x - 1, y - 2, x + 1, y - 2);
        }
    }

    // Player dot
    SDL_SetRenderDrawColor(r->sdl, 80, 200, 80, 255);
    SDL_RenderDrawPoint(r->sdl,
        ox + g->player.x / MINIMAP_SCALE,
        oy + g->player.y / MINIMAP_SCALE);
}
