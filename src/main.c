#include <SDL2/SDL.h>
#include <stdio.h>
#include "renderer/renderer.h"
#include "renderer/sprites.h"
#include "game/game.h"

#define WINDOW_TITLE "Castle of No Return"
#define WINDOW_W     1280
#define WINDOW_H     720

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    #ifdef DEBUG
        printf("testing debug mode\n");
    #endif

    SDL_Window *window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
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
    game_init(&game, renderer.tiles_x, renderer.tiles_y);

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_RESIZED)
                renderer_on_resize(&renderer,
                    event.window.data1, event.window.data2);
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: running = 0;        break;
                    case SDLK_UP:
                    case SDLK_w:     game_move_player(&game,  0, -1); break;
                    case SDLK_DOWN:
                    case SDLK_s:     game_move_player(&game,  0,  1); break;
                    case SDLK_LEFT:
                    case SDLK_a:     game_move_player(&game, -1,  0); break;
                    case SDLK_RIGHT:
                    case SDLK_d:     game_move_player(&game,  1,  0); break;
                    default: break;
                }
            }
        }

        renderer_begin_frame(&renderer);

        // Floor
        for (int y = 0; y < renderer.tiles_y; y++)
            for (int x = 0; x < renderer.tiles_x; x++)
                draw_floor(&renderer, x, y);

        // Wall border
        for (int x = 0; x < renderer.tiles_x; x++) {
            draw_wall(&renderer, x, 0);
            draw_wall(&renderer, x, renderer.tiles_y - 1);
        }
        for (int y = 0; y < renderer.tiles_y; y++) {
            draw_wall(&renderer, 0, y);
            draw_wall(&renderer, renderer.tiles_x - 1, y);
        }

        // Player in the center
        draw_player(&renderer, game.player.x, game.player.y);

        renderer_end_frame(&renderer);
    }

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}