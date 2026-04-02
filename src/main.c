#include <SDL2/SDL.h>
#include <stdio.h>
#include "renderer/renderer.h"
#include "renderer/sprites.h"
#include "renderer/viewport.h"
#include "renderer/landing_renderer.h"
#include "renderer/game_over_renderer.h"
#include "renderer/info_panel.h"
#include "game/game.h"
#include "game/map.h"
#include "screens/landing.h"
#include "screens/name_entry.h"
#include "systems/save_load.h"
#include "game/enemy.h"
#include "screens/slot_select.h"
#include "renderer/slot_renderer.h"
#include "screens/inventory.h"
#include "renderer/inventory_renderer.h"

#define WINDOW_TITLE "Castle of No Return"
#define WINDOW_W     1280
#define WINDOW_H     720

static void enter_playing(Renderer *renderer, Viewport *viewport, GameState *game) {
    int vp_tiles_x = (renderer->screen_w - INFO_PANEL_W) / TILE_SIZE;
    viewport_init(viewport, vp_tiles_x, renderer->tiles_y, MAP_W, MAP_H);
    viewport_center_on(viewport, game->player.x, game->player.y);
}

static void handle_landing_result(LandingResult result, LandingScreen *landing,
    GameScreen *screen, GameState *game, Renderer *renderer, Viewport *viewport,
    NameEntry *name_entry, SlotSelect *slot_select, int *slot_is_save, int *running) {

    switch (result) {
        case LANDING_NEW_GAME:
            name_entry_init(name_entry);
            *screen = SCREEN_NAME_ENTRY;
            break;
        case LANDING_CONTINUE:
            enter_playing(renderer, viewport, game);
            *screen = SCREEN_PLAYING;
            break;
        case LANDING_SAVE_GAME:
            slot_select_init(slot_select);
            *slot_is_save = 1;
            *screen = SCREEN_SAVE_SLOT;
            break;
        case LANDING_LOAD_GAME:
            slot_select_init(slot_select);
            *slot_is_save = 0;
            *screen = SCREEN_LOAD_SLOT;
            break;
        case LANDING_QUIT:
            *running = 0;
            break;
        default:
            break;
    }
}

static void handle_slot_result(SlotResult result, int slot, int slot_is_save,
    GameScreen *screen, GameState *game, Renderer *renderer, Viewport *viewport,
    LandingScreen *landing) {

    if (result == SLOT_CANCELLED) {
        *screen = SCREEN_LANDING;
        return;
    }
    if (result == SLOT_SELECTED) {
        if (slot_is_save) {
            save_game(game, slot);
            *screen = SCREEN_LANDING;
        } else {
            if (load_game(game, slot)) {
                enter_playing(renderer, viewport, game);
                landing->has_active_game = 1;
                *screen = SCREEN_PLAYING;
                SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
            }
        }
    }
}

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
    SlotSelect slot_select;
    slot_select_init(&slot_select);
    InventoryScreen inventory_screen;
    inventory_init(&inventory_screen);
    int slot_is_save = 0;
    GameScreen screen = SCREEN_LANDING;

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {

                // ── Quit ──────────────────────────────────────────────────
                case SDL_QUIT:
                    running = 0;
                    break;

                // ── Window resize ─────────────────────────────────────────
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

                // ── Keyboard input ────────────────────────────────────────
                case SDL_KEYDOWN: {
                    int sc = event.key.keysym.scancode;

                    // Game over screen
                    if (screen == SCREEN_GAME_OVER) {
                        if (sc == SDL_SCANCODE_RETURN) {
                            landing_init(&landing);
                            screen = SCREEN_LANDING;
                        }
                        break;
                    }

                    // Landing screen
                    if (screen == SCREEN_LANDING) {
                        LandingResult result = landing.confirming_new_game
                            ? landing_handle_confirm(&landing, sc)
                            : landing_handle_key(&landing, sc);
                        handle_landing_result(result, &landing, &screen, &game,
                            &renderer, &viewport, &name_entry, &slot_select,
                            &slot_is_save, &running);
                        break;
                    }

                    // Name entry screen
                    if (screen == SCREEN_NAME_ENTRY) {
                        const char *keyname = SDL_GetKeyName(event.key.keysym.sym);
                        NameEntryResult result = name_entry_handle_key(
                            &name_entry, sc, keyname);
                        if (result == NAME_ENTRY_CONFIRMED) {
                            game_init(&game);
                            SDL_strlcpy(game.player.name, name_entry.name,
                                sizeof(game.player.name));
                            enter_playing(&renderer, &viewport, &game);
                            screen = SCREEN_PLAYING;
                        }
                        if (result == NAME_ENTRY_CANCELLED)
                            screen = SCREEN_LANDING;
                        break;
                    }

                    // Save / load slot screen
                    if (screen == SCREEN_SAVE_SLOT || screen == SCREEN_LOAD_SLOT) {
                        SlotResult result = slot_select_handle_key(&slot_select, sc);
                        handle_slot_result(result, slot_select.selected + 1,
                            slot_is_save, &screen, &game, &renderer, &viewport,
                            &landing);
                        break;
                    }

                    // Inventory screen
                    if (screen == SCREEN_INVENTORY) {
                        InventoryResult result = inventory_handle_key(
                            &inventory_screen, sc, game.inventory_count);
                        if (result == INVENTORY_CLOSED) {
                            screen = SCREEN_PLAYING;
                        } else if (result == INVENTORY_USE) {
                            Action a = {ACTION_USE_ITEM,
                                inventory_screen.selected, 0};
                            action_resolve_player(&game, a);
                            if (game.inventory_count == 0)
                                screen = SCREEN_PLAYING;
                        } else if (result == INVENTORY_EQUIP) {
                            Action a = {ACTION_EQUIP_ITEM,
                                inventory_screen.selected, 0};
                            action_resolve_player(&game, a);
                        }
                        break;
                    }

                    // Playing screen
                    if (screen == SCREEN_PLAYING) {
                        Action a = {ACTION_NONE, 0, 0};
                        switch (sc) {
                            case SDL_SCANCODE_ESCAPE:
                                landing.has_active_game = 1;
                                landing.selected = 1;
                                screen = SCREEN_LANDING;
                                break;
                            case SDL_SCANCODE_UP:
                            case SDL_SCANCODE_W:
                                a = (Action){ACTION_MOVE, game.player.x, game.player.y - 1};
                                break;
                            case SDL_SCANCODE_DOWN:
                            case SDL_SCANCODE_S:
                                a = (Action){ACTION_MOVE, game.player.x, game.player.y + 1};
                                break;
                            case SDL_SCANCODE_LEFT:
                            case SDL_SCANCODE_A:
                                a = (Action){ACTION_MOVE, game.player.x - 1, game.player.y};
                                break;
                            case SDL_SCANCODE_RIGHT:
                            case SDL_SCANCODE_D:
                                a = (Action){ACTION_MOVE, game.player.x + 1, game.player.y};
                                break;
                            case SDL_SCANCODE_PERIOD:
                                a = (Action){ACTION_DESCEND, 0, 0};
                                break;
                            case SDL_SCANCODE_COMMA:
                                a = (Action){ACTION_ASCEND, 0, 0};
                                break;
                            case SDL_SCANCODE_P:
                                a = (Action){ACTION_PICK_UP, 0, 0};
                                break;
                            case SDL_SCANCODE_I:
                                inventory_init(&inventory_screen);
                                screen = SCREEN_INVENTORY;
                                break;
                            default: break;
                        }
                        if (a.type != ACTION_NONE) {
                            action_resolve_player(&game, a);
                            action_resolve_enemies(&game);
                            if (game.player.hp <= 0)
                                screen = SCREEN_GAME_OVER;
                            viewport_center_on(&viewport,
                                game.player.x, game.player.y);
                        }
                    }
                    break;
                }

                // ── Mouse input ───────────────────────────────────────────
                case SDL_MOUSEBUTTONDOWN: {
                    if (event.button.button != SDL_BUTTON_LEFT) break;

                    // Landing screen clicks
                    if (screen == SCREEN_LANDING) {
                        LandingResult result = landing_handle_click(
                            &landing,
                            event.button.x, event.button.y,
                            renderer.screen_w, renderer.screen_h);
                        handle_landing_result(result, &landing, &screen, &game,
                            &renderer, &viewport, &name_entry, &slot_select,
                            &slot_is_save, &running);
                    }

                    // Save / load slot clicks
                    if (screen == SCREEN_SAVE_SLOT || screen == SCREEN_LOAD_SLOT) {
                        int base_y = renderer.screen_h / 2 - 20;
                        for (int i = 0; i < 3; i++) {
                            int item_y     = base_y + i * 40;
                            int item_y_end = item_y + 24;
                            if (event.button.y >= item_y &&
                                event.button.y <= item_y_end) {
                                slot_select.selected = i;
                                handle_slot_result(SLOT_SELECTED, i + 1,
                                    slot_is_save, &screen, &game, &renderer,
                                    &viewport, &landing);
                                break;
                            }
                        }
                    }
                    break;
                }

                default:
                    break;
            }
        }

        // ── Per-frame updates ─────────────────────────────────────────────
        if (screen == SCREEN_NAME_ENTRY)
            name_entry_update(&name_entry);

        // ── Rendering ─────────────────────────────────────────────────────
        renderer_begin_frame(&renderer);

        if (screen == SCREEN_LANDING) {
            landing_draw(&renderer, &landing);
        } else if (screen == SCREEN_NAME_ENTRY) {
            name_entry_draw(&renderer, &name_entry);
        } else if (screen == SCREEN_SAVE_SLOT || screen == SCREEN_LOAD_SLOT) {
            slot_draw(&renderer, &slot_select, slot_is_save);
        } else if (screen == SCREEN_INVENTORY) {
            inventory_draw(&renderer, &game, &inventory_screen);
        } else if (screen == SCREEN_PLAYING) {
            // Draw map tiles
            for (int y = 0; y < MAP_H; y++) {
                for (int x = 0; x < MAP_W; x++) {
                    if (!viewport_is_visible(&viewport, x, y)) continue;
                    int sx = viewport_to_screen_x(&viewport, x);
                    int sy = viewport_to_screen_y(&viewport, y);
                    switch (game.map.tiles[y][x]) {
                        case TILE_WALL:            draw_wall(&renderer, sx, sy);            break;
                        case TILE_STAIRS_UP:       draw_stairs_up(&renderer, sx, sy);       break;
                        case TILE_STAIRS_DOWN:     draw_stairs_down(&renderer, sx, sy);     break;
                        case TILE_TOWN_FLOOR:      draw_town_floor(&renderer, sx, sy);      break;
                        case TILE_TOWN_PATH:       draw_town_path(&renderer, sx, sy);       break;
                        case TILE_TOWN_EXIT:       draw_town_exit(&renderer, sx, sy);       break;
                        case TILE_SHOP_BLACKSMITH: draw_shop_blacksmith(&renderer, sx, sy); break;
                        case TILE_SHOP_ALCHEMIST:  draw_shop_alchemist(&renderer, sx, sy);  break;
                        case TILE_ITEM:            draw_floor_item(&renderer, sx, sy);      break;
                        default:                   draw_floor(&renderer, sx, sy);           break;
                    }
                }
            }
            // Draw enemies
            for (int i = 0; i < game.enemy_count; i++) {
                Enemy *e = &game.enemies[i];
                if (!e->active) continue;
                if (!viewport_is_visible(&viewport, e->x, e->y)) continue;
                int sx = viewport_to_screen_x(&viewport, e->x);
                int sy = viewport_to_screen_y(&viewport, e->y);
                draw_enemy(&renderer, sx, sy, e->type);
            }
            // Draw player
            draw_player(&renderer,
                viewport_to_screen_x(&viewport, game.player.x),
                viewport_to_screen_y(&viewport, game.player.y));
            // Draw info panel
            info_panel_draw(&renderer, &game);
        } else if (screen == SCREEN_GAME_OVER) {
            game_over_draw(&renderer, &game);
        }

        renderer_end_frame(&renderer);
    }

    // ── Cleanup ───────────────────────────────────────────────────────────
    renderer_free(&renderer);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}