// Exercise the application's actual argument parsing and load-screen handler.
#define main conr_app_main
#include "../../src/main.c"
#undef main
#include <assert.h>
#include <unistd.h>

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
        assert(game->defeated_bosses & (1 << LOCATION_DUNGEON));
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
        assert(game->location == LOCATION_DAWNMERE);
        assert(approach_enter(game, 20, 0));
        assert(approach_mountain_town_exit(game, &x, &y));
        assert(game->map.tiles[y][x] == TILE_APPROACH_DUNGEON_TOWN);
        for (int step = 0; game->location == LOCATION_APPROACH && step < APPROACH_H; step++) {
            action_resolve_player(game, (Action){ACTION_MOVE, x, game->player.y - 1});
        }
        assert(game->location == LOCATION_TOWN4);

        // Loading without the debug option keeps a normal character locked.
        memset(&debug_config, 0, sizeof(debug_config));
        handle_slot_result(SLOT_SELECTED, &slots, 0, &screen, game, &renderer, &viewport, &landing);
        assert(!(game->defeated_bosses & (1 << LOCATION_DUNGEON)));
        if (!on_path) {
            assert(approach_enter(game, TOWN_W - 1, 12));
        }
        assert(!approach_dungeon_town_exit(game, &x, &y));
        assert(load_game(game, 2) && approach_dungeon_town_exit(game, &x, &y));
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
    // Unlocking town roads does not bypass Rook's quest gate.
    game_init(game);
    debug_apply_loadout(game, &debug_config);
    assert(approach_enter(game, 0, 12));
    game->player.x = APPROACH_LABYRINTH_X;
    game->player.y = APPROACH_LABYRINTH_Y + 1;
    action_resolve_player(game, (Action){ACTION_MOVE, APPROACH_LABYRINTH_X, APPROACH_LABYRINTH_Y});
    assert(game->location == LOCATION_APPROACH && !game_labyrinth_is_open(game));
    assert(game->player.y == APPROACH_LABYRINTH_Y + 1);
    game_leave_approach(game);
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
    puts("PASS: new and loaded debug characters can walk to Dawnmere, Ridgeshire and both forest/swamp towns; normal saves stay gated, Rook alone unlocks the forest-road labyrinth, and loaded equipment/gold/quests are preserved.");
    return 0;
}
