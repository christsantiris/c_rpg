#include <SDL2/SDL.h>
#include <stdio.h>
#include "renderer/renderer.h"

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

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE) running = 0;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_RESIZED)
                renderer_on_resize(&renderer,
                    event.window.data1, event.window.data2);
        }

        renderer_begin_frame(&renderer);

        // Draw checkerboard to verify tile grid
        for (int y = 0; y < renderer.tiles_y; y++) {
            for (int x = 0; x < renderer.tiles_x; x++) {
                SDL_Color color = (x + y) % 2 == 0
                    ? (SDL_Color){20, 20, 40, 255}
                    : (SDL_Color){30, 30, 55, 255};
                renderer_draw_tile_bg(&renderer, x, y, color);
            }
        }

        renderer_end_frame(&renderer);
    }

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}