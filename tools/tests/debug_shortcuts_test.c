#include "game/stonewake.h"
#include "renderer/stonewake_renderer.h"
#include "renderer/sprites.h"
#include "renderer/item_icons.h"
// Exercise the application's actual argument parsing and load-screen handler.
#define main conr_app_main
#include "../../src/main.c"
#undef main
#include <assert.h>
#include <unistd.h>
#include "../../src/game/town_road.h"
#include "../../src/game/castle.h"
#include "../../src/game/elderwood.h"
#include "../../src/game/mangrove.h"
#include "../../src/game/coastal_travel.h"

static void visit_exterior_town(GameState *g) {
    assert(g->location == LOCATION_TOWN_ROAD);
    Location town = g->road_town;
    TownSide side = g->player.y < TOWN_SPRITE_Y ? TOWN_SIDE_NORTH : g->player.y >= TOWN_SPRITE_Y + TOWN_SPRITE_H ? TOWN_SIDE_SOUTH : g->player.x < TOWN_SPRITE_X ? TOWN_SIDE_WEST : TOWN_SIDE_EAST;
    int x;
    int y;
    town_road_entrance(side, &x, &y);
    for (int step = 0; g->location == LOCATION_TOWN_ROAD && step < TOWN_ROAD_W + TOWN_ROAD_H; step++) {
        int dx = (x > g->player.x) - (x < g->player.x);
        int dy = (y > g->player.y) - (y < g->player.y);
        assert(!dx || !dy);
        action_resolve_player(g, (Action){ACTION_MOVE, g->player.x + dx, g->player.y + dy});
    }
    assert(g->location == town);
}

static Uint32 rendered_pixel(SDL_Surface *surface, int x, int y) {
    return ((Uint32 *)((unsigned char *)surface->pixels + y * surface->pitch))[x];
}

static unsigned int creature_pixels(SDL_Surface *surface, int x, int y) {
    unsigned int hash = 2166136261u;
    for (int py = y - 20; py < y + TILE_SIZE; py++) {
        for (int px = x - 8; px < x + TILE_SIZE + 8; px++) {
            hash = (hash ^ rendered_pixel(surface, px, py)) * 16777619u;
        }
    }
    return hash;
}

static void test_stonewake_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    assert(r->stonewake_enemy_texture && r->stonewake_crane_texture && r->stonewake_prop_texture && r->stonewake_workshop_texture);
    const EnemyType roster[5] = {ENEMY_CRYSTAL_SPIDER, ENEMY_BLIND_STALKER, ENEMY_SHARD_GOLEM, ENEMY_IRON_SENTINEL, ENEMY_PRISM_SOVEREIGN};
    unsigned int hashes[5];
    for (int i = 0; i < 5; i++) {
        SDL_SetRenderDrawColor(r->sdl, 0, 0, 0, 255);
        SDL_RenderClear(r->sdl);
        draw_enemy(r, 10, 8, roster[i]);
        SDL_RenderPresent(r->sdl);
        hashes[i] = creature_pixels(surface, 10 * TILE_SIZE, 8 * TILE_SIZE);
        int pixels = 0;
        int top = 48;
        int bottom = -1;
        int left = 48;
        int right = -1;
        for (int y = 8 * TILE_SIZE - 24; y < 9 * TILE_SIZE; y++) {
            for (int x = 10 * TILE_SIZE - 12; x < 11 * TILE_SIZE + 12; x++) {
                Uint8 red, green, blue;
                SDL_GetRGB(rendered_pixel(surface, x, y), surface->format, &red, &green, &blue);
                if (red || green || blue) {
                    pixels++;
                    if (y < top + 8 * TILE_SIZE - 24) {
                        top = y - (8 * TILE_SIZE - 24);
                    }
                    bottom = y - (8 * TILE_SIZE - 24);
                    int col = x - (10 * TILE_SIZE - 12);
                    if (col < left) {
                        left = col;
                    }
                    if (col > right) {
                        right = col;
                    }
                }
            }
        }
        assert(pixels > 80);
        const int widths[5] = {24, 24, 32, 24, 48};
        const int heights[5] = {16, 24, 32, 24, 48};
        assert(right - left + 1 <= widths[i] && bottom - top + 1 <= heights[i]);
        assert(bottom == 47); // Soles/tail sit on the same ground line as the 24px player.
        for (int n = 0; n < i; n++) {
            assert(hashes[i] != hashes[n]);
        }
    }
    game_init(g);
    game_enter_glassdeep(g);
    g->player.x = 18;
    g->player.y = 26;
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        for (int x = 0; x < SWAMP_MAP_W; x++) {
            map_mark_explored(&g->map, x, y);
        }
    }
    Viewport view;
    viewport_init(&view, r->tiles_x, r->tiles_y, MAP_W, MAP_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(g->map.tiles[26][18] == TILE_STONEWAKE_CRANE_NEW);
    assert(stonewake_interact(g) && g->map.tiles[26][21] == TILE_STONEWAKE_BRIDGE);
    int crane_width = 0;
    int crane_height = 0;
    assert(SDL_QueryTexture(r->stonewake_crane_texture, NULL, NULL, &crane_width, &crane_height) == 0);
    assert(crane_width == 216 && crane_height == 72);
    unsigned int poses[3];
    for (int mode = 0; mode < 3; mode++) {
        assert(stonewake_crane_mode(g->map.tiles[26][18]) == (mode + 1) % 3);
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        char path[80];
        snprintf(path, sizeof(path), "/tmp/stonewake-crane-pose-%d.bmp", mode);
        assert(SDL_SaveBMP(surface, path) == 0);
        int px = viewport_to_screen_x(&view, 18) * TILE_SIZE;
        int py = viewport_to_screen_y(&view, 26) * TILE_SIZE;
        poses[mode] = 2166136261u;
        for (int y = py - 48; y < py + 24; y++) {
            for (int x = px - 18; x < px + 54; x++) {
                poses[mode] = (poses[mode] ^ rendered_pixel(surface, x, y)) * 16777619u;
            }
        }
        if (mode < 2) {
            assert(stonewake_interact(g));
        }
    }
    assert(poses[0] != poses[1] && poses[0] != poses[2] && poses[1] != poses[2]);
    assert(g->map.tiles[26][18] == TILE_STONEWAKE_CRANE_UP);

    renderer_begin_frame(r);
    game_draw(r, g, &view);
    Item hammer = item_make_stone_warhammer();
    draw_icon_item(r, 600, 200, &hammer);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/stonewake-renderer-test.bmp") == 0);
    printf("PASS: all five Stonewake sprites render at native scale, with three distinct crane poses, workshops and a distinct warhammer icon.\n");
}

static void test_saltwind_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    game_init(g);
    mangrove_enter(g, 0);
    g->player.x = 34;
    g->player.y = 11;
    g->enemy_count = 1;
    Enemy *enemy = &g->enemies[0];
    enemy->x = 43;
    enemy->y = 11;
    Viewport view;
    viewport_init(&view, r->tiles_x, r->tiles_y, MANGROVE_W, MANGROVE_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    int x = viewport_to_screen_x(&view, enemy->x) * TILE_SIZE;
    int y = viewport_to_screen_y(&view, enemy->y) * TILE_SIZE;
    for (int type = ENEMY_LIZARDFOLK_SCOUT; type <= ENEMY_SSZARETH; type++) {
        enemy->active = 0;
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        unsigned int ground = creature_pixels(surface, x, y);
        enemy->active = 1;
        enemy->type = type;
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        assert(creature_pixels(surface, x, y) != ground);
    }
    approach_enter_region(g, 3);
    assert(approach_coast_ground(COAST_ROWAN_X, COAST_ROWAN_Y) == TILE_COAST_SAND);
    viewport_init(&view, r->tiles_x, r->tiles_y, APPROACH_W, APPROACH_H);
    g->player.x = 44;
    g->player.y = 18;
    viewport_center_on(&view, g->player.x, g->player.y);
    x = viewport_to_screen_x(&view, COAST_ROWAN_X) * TILE_SIZE;
    y = viewport_to_screen_y(&view, COAST_ROWAN_Y) * TILE_SIZE;
    g->map.tiles[COAST_ROWAN_Y][COAST_ROWAN_X] = TILE_COAST_SAND;
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    Uint32 sand = rendered_pixel(surface, x, y);
    g->map.tiles[COAST_ROWAN_Y][COAST_ROWAN_X] = TILE_NPC_ROWAN;
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(rendered_pixel(surface, x, y) == sand);
    printf("PASS: all six Saltwind enemy sprites render in the adventure; Rowan preserves his beach underlay.\n");
}

int main(void) {
    char *project = getcwd(NULL, 0);
    char *assets = realpath("assets", NULL);
    char temporary[] = "/tmp/conr-debug-shortcuts-XXXXXX";
    assert(project && assets && mkdtemp(temporary));
    assert(chdir(temporary) == 0 && symlink(assets, "assets") == 0);
    SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 1280, 720, 32, SDL_PIXELFORMAT_RGBA32);
    assert(surface);
    SDL_Renderer *sdl = SDL_CreateSoftwareRenderer(surface);
    assert(sdl);
    Renderer renderer = {0};
    renderer_init(&renderer, sdl, 1280, 720);
    assert(renderer.font_large);
    assert(renderer.elderwood_entrance_texture && renderer.elderwood_enemy_texture);
    assert(renderer.mangrove_enemy_texture && renderer.mangrove_entrance_texture && renderer.mangrove_root_texture && renderer.skiff_texture);
    for (int i = 0; i < TOWN_ROAD_COUNT; i++) {
        int width;
        int height;
        assert(renderer.town_textures[i]);
        assert(SDL_QueryTexture(renderer.town_textures[i], NULL, NULL, &width, &height) == 0);
        assert(width == TOWN_SPRITE_W * TILE_SIZE && height == TOWN_SPRITE_H * TILE_SIZE);
    }
    const TownExitStyle entrances[] = {TOWN_EXIT_COAST, TOWN_EXIT_DESERT, TOWN_EXIT_GLASSDEEP, TOWN_EXIT_FROST, TOWN_EXIT_MOONVEIL, TOWN_EXIT_DRAGONSPINE, TOWN_EXIT_ASHEN, TOWN_EXIT_CATACOMBS};
    for (int i = 0; i < 8; i++) {
        int width;
        int height;
        assert(renderer.area_entrance_textures[entrances[i]]);
        assert(SDL_QueryTexture(renderer.area_entrance_textures[entrances[i]], NULL, NULL, &width, &height) == 0);
        assert(width == 168 && height == (entrances[i] == TOWN_EXIT_DRAGONSPINE ? 216 : 144));
    }
    Viewport viewport;
    SlotSelect slots = {0};
    LandingScreen landing = {0};
    GameScreen screen = SCREEN_LOAD_SLOT;
    GameState *game = calloc(1, sizeof(*game));
    assert(game);
    test_stonewake_renderer(&renderer, game, surface);
    test_saltwind_renderer(&renderer, game, surface);
    char *arguments[] = {"conr", "--unlock-shortcuts", "--gold", "99999", "--weapon", "none"};

    // Old debug saves have three shortcuts but lack the new Lich unlock.
    for (int on_path = 0; on_path < 2; on_path++) {
        game_init(game);
        game->defeated_bosses = (1 << LOCATION_FOREST) | (1 << LOCATION_MOUNTAINS) | (1 << LOCATION_SWAMP);
        if (on_path) {
            assert(approach_enter(game, TOWN_W - 1, 12));
        }
        game->gold = 393;
        game->player.hp = 73;
        game->inventory_count = 1;
        game->inventory[0] = item_make_greatsword();
        game->inventory[0].sharpened = 1;
        game->elowen_quest_state = 1;
        game->elowen_seals_restored = 3;
        int px = game->player.x;
        int py = game->player.y;
        assert(save_game(game, 1));
        assert(debug_parse_args(&debug_config, 6, arguments));
        handle_slot_result(SLOT_SELECTED, &slots, 0, &screen, game, &renderer, &viewport, &landing);
        assert(screen == SCREEN_PLAYING && landing.has_active_game);
        assert(game->gold == 393 && game->player.hp == 73 && game->player.x == px && game->player.y == py);
        assert(game->inventory_count == 1 && game->inventory[0].sharpened);
        assert(game->elowen_quest_state == 1 && game->elowen_seals_restored == 3);
        assert(!(game->defeated_bosses & (1 << LOCATION_DUNGEON)));
        assert(game->debug_passages_unlocked && game_harbor_unlocked(game) && game_can_sail_to_island(game) && game_labyrinth_is_open(game));
        if (!on_path) {
            assert(approach_enter(game, TOWN_W - 1, 12));
        }
        int x;
        int y;
        assert(approach_dungeon_town_exit(game, &x, &y));
        assert(game->map.tiles[y][x] == TILE_APPROACH_DUNGEON_TOWN);
        assert(save_game(game, 2));
        assert(game->player.y == y);
        for (int step = 0; game->player.x < x - 1 && step < APPROACH_W; step++) {
            assert(game->map.tiles[y][game->player.x + 1] == TILE_TOWN_PATH);
            action_resolve_player(game, (Action){ACTION_MOVE, game->player.x + 1, game->player.y});
        }
        action_resolve_player(game, (Action){ACTION_MOVE, x, y});
        visit_exterior_town(game);
        assert(game->location == LOCATION_DAWNMERE);
        assert(approach_enter(game, 20, 0));
        assert(approach_mountain_town_exit(game, &x, &y));
        assert(game->map.tiles[y][x] == TILE_APPROACH_DUNGEON_TOWN);
        for (int step = 0; game->location == LOCATION_APPROACH && step < APPROACH_H; step++) {
            action_resolve_player(game, (Action){ACTION_MOVE, x, game->player.y - 1});
        }
        visit_exterior_town(game);
        assert(game->location == LOCATION_TOWN4);

        // Loading without the debug option keeps a normal character locked.
        memset(&debug_config, 0, sizeof(debug_config));
        handle_slot_result(SLOT_SELECTED, &slots, 0, &screen, game, &renderer, &viewport, &landing);
        assert(!(game->defeated_bosses & (1 << LOCATION_DUNGEON)));
        if (!on_path) {
            assert(approach_enter(game, TOWN_W - 1, 12));
        }
        assert(!approach_dungeon_town_exit(game, &x, &y));
        assert(load_game(game, 2) && game->debug_passages_unlocked && approach_dungeon_town_exit(game, &x, &y));
    }
    for (int stillbury = 0; stillbury < 2; stillbury++) {
        game_init(game);
        if (stillbury) {
            game_enter_town2(game);
        }
        assert(approach_enter(game, stillbury ? TOWN_W - 1 : 0, 12));
        int x;
        int y;
        assert(!approach_forest_town_exit(game, &x, &y));
        assert(save_game(game, 1));
        assert(debug_parse_args(&debug_config, 6, arguments));
        handle_slot_result(SLOT_SELECTED, &slots, 0, &screen, game, &renderer, &viewport, &landing);
        assert(approach_forest_town_exit(game, &x, &y));
        for (int step = 0; game->location == LOCATION_APPROACH && step < APPROACH_W; step++) {
            action_resolve_player(game, (Action){ACTION_MOVE, game->player.x + (stillbury ? 1 : -1), y});
        }
        visit_exterior_town(game);
        assert(game->location == (stillbury ? LOCATION_TOWN : LOCATION_TOWN2));
    }
    for (int rosemoor = 0; rosemoor < 2; rosemoor++) {
        game_init(game);
        if (rosemoor) {
            game_enter_town3(game);
        } else {
            game_enter_town2(game);
        }
        assert(approach_enter(game, 20, rosemoor ? TOWN_H - 1 : 0));
        int x;
        int y;
        assert(!approach_swamp_town_exit(game, &x, &y));
        assert(save_game(game, 1));
        assert(debug_parse_args(&debug_config, 6, arguments));
        handle_slot_result(SLOT_SELECTED, &slots, 0, &screen, game, &renderer, &viewport, &landing);
        assert(approach_swamp_town_exit(game, &x, &y));
        for (int step = 0; game->location == LOCATION_APPROACH && step < APPROACH_H; step++) {
            action_resolve_player(game, (Action){ACTION_MOVE, x, game->player.y + (rosemoor ? 1 : -1)});
        }
        visit_exterior_town(game);
        assert(game->location == (rosemoor ? LOCATION_TOWN2 : LOCATION_TOWN3));
    }
    game_init(game);
    assert(debug_parse_args(&debug_config, 6, arguments));
    debug_apply_loadout(game, &debug_config);
    assert(game->gold == 99999 && game->inventory_count == 0);
    assert(approach_enter(game, TOWN_W - 1, 12));
    int x;
    int y;
    assert(approach_dungeon_town_exit(game, &x, &y));
    // A normal northern-road save must open both locks when loaded with the launcher flag.
    game_init(game);
    assert(approach_enter(game, 20, 0));
    assert(game->map.tiles[NORTH_FOREST_BARRIER_Y][NORTH_ROAD_X] == TILE_ELDERWOOD_WALL);
    assert(save_game(game, 1));
    handle_slot_result(SLOT_SELECTED, &slots, 0, &screen, game, &renderer, &viewport, &landing);
    assert(game->debug_passages_unlocked && !game->elderwood_defeated && !game->defeated_bosses);
    assert(game->map.tiles[NORTH_FOREST_BARRIER_Y][NORTH_ROAD_X] == TILE_TOWN_PATH);
    assert(game->map.tiles[0][NORTH_ROAD_X] == TILE_NORTH_CASTLE);
    renderer_begin_frame(&renderer);
    game_draw(&renderer, game, &viewport);
    SDL_RenderPresent(sdl);
    elderwood_enter(game);
    for (int level = 1; level < ELDERWOOD_DEPTH; level++) {
        game_descend(game);
    }
    renderer_begin_frame(&renderer);
    game_draw(&renderer, game, &viewport);
    SDL_RenderPresent(sdl);
    game_return_to_approach(game);
    assert(northern_approach(game));
    // Normal play still requires Rook's quest; the debug flag bypasses it.
    game_init(game);
    assert(approach_enter(game, 0, 12));
    game->player.x = APPROACH_LABYRINTH_X;
    game->player.y = APPROACH_LABYRINTH_Y + 1;
    action_resolve_player(game, (Action){ACTION_MOVE, APPROACH_LABYRINTH_X, APPROACH_LABYRINTH_Y});
    assert(game->location == LOCATION_APPROACH && !game_labyrinth_is_open(game));
    assert(game->player.y == APPROACH_LABYRINTH_Y + 1);
    game_leave_approach(game);
    visit_exterior_town(game);
    game_enter_tavern(game);
    game->player.x = ROOK_TAVERN_X;
    game->player.y = ROOK_TAVERN_Y + 1;
    game_talk_to_rook(game);
    game_handle_quest_offer_key(game, SDL_SCANCODE_Y, 0);
    assert(game_labyrinth_is_open(game));
    game_leave_tavern(game);
    assert(approach_enter(game, 0, 12));
    while (game->player.x > APPROACH_LABYRINTH_X) {
        action_resolve_player(game, (Action){ACTION_MOVE, game->player.x - 1, game->player.y});
    }
    action_resolve_player(game, (Action){ACTION_MOVE, APPROACH_LABYRINTH_X, APPROACH_LABYRINTH_Y + 1});
    action_resolve_player(game, (Action){ACTION_MOVE, APPROACH_LABYRINTH_X, APPROACH_LABYRINTH_Y});
    assert(game->location == LOCATION_LABYRINTH && game->level == 1 && game->enemy_count > 0);
    assert(save_game(game, 2) && load_game(game, 2));
    game_leave_labyrinth(game);
    assert(game->location == LOCATION_APPROACH && game->labyrinth_entry_town == LOCATION_TOWN);
    assert(game->player.x == APPROACH_LABYRINTH_X && game->player.y == APPROACH_LABYRINTH_Y + 1);
    game_init(game);
    debug_apply_loadout(game, &debug_config);
    assert(game->debug_passages_unlocked && game->rook_quest_state == 0);
    assert(approach_enter(game, 0, 12));
    game->player.x = APPROACH_LABYRINTH_X;
    game->player.y = APPROACH_LABYRINTH_Y + 1;
    action_resolve_player(game, (Action){ACTION_MOVE, APPROACH_LABYRINTH_X, APPROACH_LABYRINTH_Y});
    assert(game->location == LOCATION_LABYRINTH && game->rook_quest_state == 0);
    game_leave_labyrinth(game);
    // Loaded town exteriors preserve identity and equipment under the debug option.
    assert(town_road_enter(game, LOCATION_DAWNMERE, TOWN_SIDE_WEST, 0));
    game->gold = 73;
    int road_x = game->player.x;
    int road_y = game->player.y;
    assert(save_game(game, 1));
    assert(debug_parse_args(&debug_config, 6, arguments));
    handle_slot_result(SLOT_SELECTED, &slots, 0, &screen, game, &renderer, &viewport, &landing);
    assert(game->location == LOCATION_TOWN_ROAD && game->road_town == LOCATION_DAWNMERE);
    assert(game->gold == 73 && game->player.x == road_x && game->player.y == road_y);
    renderer_begin_frame(&renderer);
    game_draw(&renderer, game, &viewport);
    SDL_RenderPresent(sdl);
    visit_exterior_town(game);
    assert(game->location == LOCATION_DAWNMERE && game->player.x == 1 && game->player.y == 12);
    // Docks and sailing work immediately for new and loaded normal characters.
    for (int loaded = 0; loaded < 2; loaded++) {
        game_init(game);
        assert(!game_harbor_unlocked(game) && !game_can_sail_to_island(game) && !game_labyrinth_is_open(game));
        game->gold = 393;
        game->inventory_count = 1;
        game->inventory[0] = item_make_greatsword();
        game->inventory[0].sharpened = 1;
        if (loaded) {
            assert(save_game(game, 1));
            handle_slot_result(SLOT_SELECTED, &slots, 0, &screen, game, &renderer, &viewport, &landing);
            assert(game->gold == 393 && game->inventory_count == 1 && game->inventory[0].sharpened);
        } else {
            debug_apply_loadout(game, &debug_config);
        }
        assert(game_harbor_unlocked(game) && game_can_sail_to_island(game) && !game_has_treasure_map(game));
        assert(!game->defeated_bosses);
        assert(!game->rook_quest_state && !game->mara_quest_state && !game->temple_treasure_state && !game->island_travel_unlocked);
        approach_enter_region(game, 3);
        game->player.x = APPROACH_W / 2;
        game->player.y = 16;
        for (int x = game->player.x + 1; x < COAST_DOCK_X; x++) {
            assert(game->map.tiles[16][x] == TILE_TOWN_PATH);
            action_resolve_player(game, (Action){ACTION_MOVE, x, 16});
        }
        for (int y = 17; y <= COAST_DOCK_ENTRY_Y; y++) {
            action_resolve_player(game, (Action){ACTION_MOVE, COAST_DOCK_X - 1, y});
        }
        for (int x = COAST_DOCK_X; x <= COAST_DOCK_ENTRY_X; x++) {
            action_resolve_player(game, (Action){ACTION_MOVE, x, COAST_DOCK_ENTRY_Y});
        }
        assert(coastal_at_docks(game));
        HarborScreen harbor = {0};
        HarborResult result = harbor_activate(&harbor, game_can_sail_to_island(game), 0);
        assert(result == HARBOR_BOARD);
        handle_harbor_result(result, game, &screen, &renderer, &viewport);
        assert(game->location == LOCATION_ISLAND);
        game->player.x = ISLAND_GATE_X;
        game->player.y = ISLAND_GATE_Y + 1;
        action_resolve_player(game, (Action){ACTION_MOVE, ISLAND_GATE_X, ISLAND_GATE_Y});
        assert(game->location == LOCATION_TEMPLE && game->level == 1 && game->enemy_count > 0);
        game_leave_temple(game);
        handle_harbor_result(HARBOR_SAIL_TOWN, game, &screen, &renderer, &viewport);
        assert(coastal_at_docks(game));
        assert(save_game(game, 2));
        game->debug_passages_unlocked = 0;
        assert(load_game(game, 2) && game->debug_passages_unlocked && game_harbor_unlocked(game) && game_can_sail_to_island(game));
        assert(!game->rook_quest_state && !game->mara_quest_state && !game->temple_treasure_state);
    }
    // Locked doors and puzzle gates stay open beneath loot, in future floors too.
    const TileType locked[] = {TILE_LOCKED_DOOR, TILE_CRYPT_DOOR, TILE_MOUNTAIN_GATE, TILE_MOUNTAIN_ROCKFALL, TILE_TEMPLE_MOON_DOOR_CLOSED, TILE_TEMPLE_VAULT_DOOR, TILE_LABYRINTH_GATE, TILE_CASTLE_GATE, TILE_GLASS_SEAL_CLOSED};
    for (int i = 0; i < (int)(sizeof(locked) / sizeof(locked[0])); i++) {
        game_init(game);
        game->location = LOCATION_DUNGEON;
        game->player.x = 5;
        game->player.y = 5;
        game->map.tiles[5][5] = TILE_FLOOR;
        game->map.tiles[5][6] = locked[i];
        game_refresh_debug_passages(game);
        assert(game->map.tiles[5][6] == locked[i]);
        game->debug_passages_unlocked = 1;
        game->floor_item_count = 1;
        game->floor_items[0] = (FloorItem){.active = 1, .x = 6, .y = 5, .underlying_tile = locked[i], .item = item_make_health_potion()};
        game->map.tiles[5][6] = TILE_ITEM;
        action_resolve_player(game, (Action){ACTION_MOVE, 6, 5});
        assert(game->player.x == 6 && game->map.tiles[5][6] == TILE_ITEM && game->floor_items[0].active && game->floor_items[0].underlying_tile != locked[i]);
    }
    game_init(game);
    debug_apply_loadout(game, &debug_config);
    castle_enter(game, 2);
    game->player.x = game->map.stairs_down_x;
    game->player.y = game->map.stairs_down_y;
    action_resolve_player(game, (Action){ACTION_ASCEND, 0, 0});
    assert(game->location == LOCATION_CASTLE_INTERIOR && game->level == 3 && !game->castle_minibosses);
    castle_enter(game, 4);
    game->player.x = game->map.stairs_down_x;
    game->player.y = game->map.stairs_down_y;
    action_resolve_player(game, (Action){ACTION_ASCEND, 0, 0});
    assert(game->location == LOCATION_CASTLE_INTERIOR && game->level == 5 && !game->castle_minibosses);
    // Debug jail entry keeps the hatch hidden until Tomas's offer is accepted.
    game_init(game);
    debug_apply_loadout(game, &debug_config);
    game->location = LOCATION_CASTLE;
    map_generate_castle(&game->map, &game->player.x, &game->player.y);
    game->player.x = JAIL_DOOR_X;
    game->player.y = JAIL_DOOR_Y + 1;
    action_resolve_player(game, (Action){ACTION_MOVE, JAIL_DOOR_X, JAIL_DOOR_Y});
    assert(game->location == LOCATION_JAIL && !game->jail_quest_state);
    assert(save_game(game, 2) && load_game(game, 2) && game->debug_passages_unlocked && !game->jail_quest_state);
    game->player.x = JAIL_HATCH_X - 1;
    game->player.y = JAIL_HATCH_Y;
    action_resolve_player(game, (Action){ACTION_MOVE, JAIL_HATCH_X, JAIL_HATCH_Y});
    assert(game->location == LOCATION_JAIL && !game->jail_quest_state);
    assert(game->map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] == TILE_CASTLE_FLOOR);
    game->player.x = JAIL_PRISONER_X;
    game->player.y = JAIL_PRISONER_Y + 1;
    assert(jail_talk_nearby(game) && game_quest_offer_active(game));
    game_handle_quest_offer_key(game, SDL_SCANCODE_Y, 0);
    assert(game->jail_quest_state == 2 && game->map.tiles[JAIL_HATCH_Y][JAIL_HATCH_X] == TILE_JAIL_HATCH);
    game->player.x = JAIL_HATCH_X - 1;
    game->player.y = JAIL_HATCH_Y;
    action_resolve_player(game, (Action){ACTION_MOVE, JAIL_HATCH_X, JAIL_HATCH_Y});
    assert(game->location == LOCATION_ESCAPE_TUNNEL && game->jail_quest_state == 2);
    assert(save_game(game, 2) && load_game(game, 2) && game->debug_passages_unlocked);
    game->player.x = 1;
    game->player.y = 14;
    action_resolve_player(game, (Action){ACTION_MOVE, 0, 14});
    assert(game->location == LOCATION_JAIL);
    game->player.x = 5;
    game->player.y = 8;
    action_resolve_player(game, (Action){ACTION_MOVE, 4, 8});
    assert(game->location == LOCATION_CASTLE && game->jail_quest_state == 2);
    game->player.x = JAIL_DOOR_X;
    game->player.y = JAIL_DOOR_Y + 1;
    action_resolve_player(game, (Action){ACTION_MOVE, JAIL_DOOR_X, JAIL_DOOR_Y});
    game->player.x = JAIL_HATCH_X - 1;
    game->player.y = JAIL_HATCH_Y;
    action_resolve_player(game, (Action){ACTION_MOVE, JAIL_HATCH_X, JAIL_HATCH_Y});
    int gold = game->gold;
    game->player.x = ESCAPE_TUNNEL_W - 2;
    game->player.y = 14;
    action_resolve_player(game, (Action){ACTION_MOVE, ESCAPE_TUNNEL_W - 1, 14});
    assert(game->location == LOCATION_TOWN4 && game->jail_quest_state == 3 && game->gold == gold + ESCAPE_REWARD_GOLD);
    free(game);
    renderer_free(&renderer);
    SDL_DestroyRenderer(sdl);
    SDL_FreeSurface(surface);
    SDL_Quit();
    remove("saves/savegame_1.json");
    remove("saves/savegame_2.json");
    remove("saves/controls.json");
    rmdir("saves");
    unlink("assets");
    assert(chdir(project) == 0);
    rmdir(temporary);
    free(project);
    free(assets);
    puts("PASS: new and loaded debug characters unlock roads, docks, island sailing, labyrinth and puzzle gates; normal saves stay gated and equipment/gold/quests survive loading and save round trips.");
    return 0;
}
