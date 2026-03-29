#include <SDL2/SDL.h>
#include <stdio.h>
#include "renderer/renderer.h"
#include "renderer/sprites.h"
#include "renderer/viewport.h"
#include "renderer/landing_renderer.h"
#include "game/game.h"
#include "game/map.h"
#include "screens/landing.h"
#include "screens/name_entry.h"
#include "renderer/landing_renderer.h"

#define WINDOW_TITLE "Castle of No Return"
#define WINDOW_W     1280
#define WINDOW_H     720

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    SDL_RaiseWindow(window);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *sdl_renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!sdl_renderer) {
        fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Renderer renderer;
    renderer_init(&renderer, sdl_renderer, WINDOW_W, WINDOW_H);

    GameState game;
    game_init(&game);

    Viewport viewport;
    viewport_init(&viewport, renderer.tiles_x, renderer.tiles_y, MAP_W, MAP_H);
    viewport_center_on(&viewport, game.player.x, game.player.y);

    LandingScreen landing;
    landing_init(&landing);
    NameEntry name_entry;
    name_entry_init(&name_entry);
    GameScreen screen = SCREEN_LANDING;

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        renderer_on_resize(&renderer,
                            event.window.data1, event.window.data2);
                        viewport_on_resize(&viewport,
                            renderer.tiles_x, renderer.tiles_y);
                        viewport_center_on(&viewport,
                            game.player.x, game.player.y);
                    }
                    break;
                case SDL_KEYDOWN:
                    if (screen == SCREEN_LANDING) {
                        LandingResult result = landing_handle_key(&landing, event.key.keysym.scancode);
                        if (result == LANDING_NEW_GAME) {
                            name_entry_init(&name_entry);
                            screen = SCREEN_NAME_ENTRY;
                        }
                        if (result == LANDING_CONTINUE) {
                            screen = SCREEN_PLAYING;
                        }
                        if (result == LANDING_QUIT) running = 0;
                        break;
                    }
                    if (screen == SCREEN_NAME_ENTRY) {
                        const char *keyname = SDL_GetKeyName(event.key.keysym.sym);
                        NameEntryResult result = name_entry_handle_key(
                            &name_entry, event.key.keysym.scancode, keyname);
                        if (result == NAME_ENTRY_CONFIRMED) {
                            game_init(&game);
                            viewport_init(&viewport,
                                renderer.tiles_x, renderer.tiles_y,
                                MAP_W, MAP_H);
                            viewport_center_on(&viewport,
                                game.player.x, game.player.y);
                            screen = SCREEN_PLAYING;
                        }
                        if (result == NAME_ENTRY_CANCELLED) screen = SCREEN_LANDING;
                        break;
                    }
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_ESCAPE:
                            landing.has_active_game = 1;
                            landing.selected = 1;
                            screen = SCREEN_LANDING;
                            break;
                        case SDL_SCANCODE_UP:
                        case SDL_SCANCODE_W: game_move_player(&game,  0, -1); break;
                        case SDL_SCANCODE_DOWN:
                        case SDL_SCANCODE_S: game_move_player(&game,  0,  1); break;
                        case SDL_SCANCODE_LEFT:
                        case SDL_SCANCODE_A: game_move_player(&game, -1,  0); break;
                        case SDL_SCANCODE_RIGHT:
                        case SDL_SCANCODE_D: game_move_player(&game,  1,  0); break;
                        case SDL_SCANCODE_PERIOD:
                            if (game.map.tiles[game.player.y][game.player.x]
                                == TILE_STAIRS_DOWN) {
                                game_descend(&game);
                                viewport_center_on(&viewport,
                                    game.player.x, game.player.y);
                            }
                            break;
                        case SDL_SCANCODE_COMMA:
                            if (game.map.tiles[game.player.y][game.player.x]
                                == TILE_STAIRS_UP) {
                                game_ascend(&game);
                                viewport_center_on(&viewport,
                                    game.player.x, game.player.y);
                            }
                            break;
                        default: break;
                    }
                    viewport_center_on(&viewport,
                        game.player.x, game.player.y);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (screen == SCREEN_LANDING &&
                        event.button.button == SDL_BUTTON_LEFT) {
                        LandingResult result = landing_handle_click(
                            &landing,
                            event.button.x, event.button.y,
                            renderer.screen_w, renderer.screen_h);
                        if (result == LANDING_NEW_GAME) {
                            name_entry_init(&name_entry);
                            screen = SCREEN_NAME_ENTRY;
                        }
                        if (result == LANDING_QUIT) running = 0;
                    }
                    break;
                default:
                    break;
            }
        }
        if (screen == SCREEN_NAME_ENTRY)
            name_entry_update(&name_entry);
            
        renderer_begin_frame(&renderer);

        if (screen == SCREEN_LANDING) {
            landing_draw(&renderer, &landing);
        } else if (screen == SCREEN_NAME_ENTRY) {
            name_entry_draw(&renderer, &name_entry);
        } else if (screen == SCREEN_PLAYING) {
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    if (!viewport_is_visible(&viewport, x, y)) continue;
                    int sx = viewport_to_screen_x(&viewport, x);
                    int sy = viewport_to_screen_y(&viewport, y);
                    switch (game.map.tiles[y][x]) {
                        case TILE_WALL:        draw_wall(&renderer, sx, sy);        break;
                        case TILE_STAIRS_UP:   draw_stairs_up(&renderer, sx, sy);   break;
                        case TILE_STAIRS_DOWN: draw_stairs_down(&renderer, sx, sy); break;
                        default:               draw_floor(&renderer, sx, sy);       break;
                    }
                }
            }
            draw_player(&renderer,
                viewport_to_screen_x(&viewport, game.player.x),
                viewport_to_screen_y(&viewport, game.player.y));
        }

        renderer_end_frame(&renderer);
    }

    renderer_free(&renderer);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}