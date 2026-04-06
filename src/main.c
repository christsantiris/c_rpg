#include <SDL2/SDL.h>
#include <stdio.h>
#include "renderer/renderer.h"
#include "renderer/viewport.h"
#include "renderer/landing_renderer.h"
#include "renderer/game_over_renderer.h"
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
#include "game/actions.h"
#include "screens/spellbook.h"
#include "renderer/spellbook_renderer.h"
#include "screens/shop.h"
#include "renderer/shop_renderer.h"
#include "renderer/game_renderer.h"
#include "renderer/info_panel.h"
#include "audio/music.h"

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
            // Show loading screen for one frame before blocking load
            renderer_begin_frame(renderer);
            SDL_Color white = {200, 200, 200, 255};
            SDL_Color dim   = { 80,  80,  80, 255};
            int cx = renderer->screen_w / 2;
            int cy = renderer->screen_h / 2;
            renderer_draw_text(renderer, "LOADING...", cx - 60, cy, white, renderer->font_large);
            renderer_draw_text(renderer, "PLEASE WAIT", cx - 60, cy + 40, dim, renderer->font_small);
            renderer_end_frame(renderer);

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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0) {
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
    music_init();

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
    SpellbookScreen spellbook_screen;
    spellbook_init(&spellbook_screen);
    ShopScreen shop_screen;

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
                        } else if (result == INVENTORY_DROP) {
                            Action a = {ACTION_DROP_ITEM,
                                inventory_screen.selected, 0};
                            action_resolve_player(&game, a);
                            if (inventory_screen.selected >= game.inventory_count)
                                inventory_screen.selected = game.inventory_count - 1;
                            if (inventory_screen.selected < 0)
                                inventory_screen.selected = 0;
                        }
                        break;
                    }

                    // Spellbook screen
                    if (screen == SCREEN_SPELLBOOK) {
                        SpellbookResult result = spellbook_handle_key(
                            &spellbook_screen, sc,
                            game.player.known_spell_count);
                        if (result == SPELLBOOK_CLOSED)
                            screen = SCREEN_PLAYING;
                        if (result == SPELLBOOK_EQUIP)
                            game.player.equipped_spell =
                                spellbook_screen.selected;
                        break;
                    }

                    // Shop screen
                    if (screen == SCREEN_SHOP) {
                        ShopResult result = shop_handle_key(&shop_screen, sc);
                        if (result == SHOP_CLOSED) {
                            screen = SCREEN_PLAYING;
                        } else if (result == SHOP_BUY) {
                            Item *item = &shop_screen.items[shop_screen.selected];
                            if (game.gold < item->value) {
                                push_message(&game, "Not enough gold!");
                            } else if (game.inventory_count >= MAX_INVENTORY) {
                                push_message(&game, "Inventory full!");
                            } else {
                                game.gold -= item->value;
                                game.inventory[game.inventory_count++] = *item;
                                char msg[32];
                                SDL_snprintf(msg, sizeof(msg), "Bought %s",
                                    item->name);
                                push_message(&game, msg);
                            }
                        } else if (result == SHOP_SELL) {
                            if (game.inventory_count == 0) break;
                            int idx = shop_screen.selected;
                            if (idx >= game.inventory_count) break;
                            Item *item = &game.inventory[idx];

                            // Unequip if equipped
                            if (game.equipped_weapon == idx) {
                                game.player.attack -= item->attack_bonus;
                                game.equipped_weapon = -1;
                            } else if (game.equipped_armor == idx) {
                                game.player.defense -= item->defense_bonus;
                                game.equipped_armor = -1;
                            }
                            if (game.equipped_weapon > idx) game.equipped_weapon--;
                            if (game.equipped_armor  > idx) game.equipped_armor--;

                            int sell_price = item->value / 2;
                            char msg[32];
                            SDL_snprintf(msg, sizeof(msg), "Sold %s for %d gold",
                                item->name, sell_price);
                            game.gold += sell_price;

                            for (int i = idx; i < game.inventory_count - 1; i++)
                                game.inventory[i] = game.inventory[i + 1];
                            game.inventory_count--;
                            if (shop_screen.selected >= game.inventory_count)
                                shop_screen.selected = game.inventory_count - 1;
                            if (shop_screen.selected < 0)
                                shop_screen.selected = 0;
                            push_message(&game, msg);
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
                            case SDL_SCANCODE_B:
                                spellbook_init(&spellbook_screen);
                                screen = SCREEN_SPELLBOOK;
                                break;
                            case SDL_SCANCODE_C:
                                a = (Action){ACTION_CAST_SPELL, 0, 0};
                                break;
                            case SDL_SCANCODE_F:
                                a = (Action){ACTION_RANGED_ATTACK, 0, 0};
                                break;
                            case SDL_SCANCODE_E: {
                                // Check adjacent tiles for shops
                                int px = game.player.x;
                                int py = game.player.y;
                                int found = 0;
                                for (int dy = -1; dy <= 1 && !found; dy++) {
                                    for (int dx = -1; dx <= 1 && !found; dx++) {
                                        TileType t = game.map.tiles[py+dy][px+dx];
                                        if (t == TILE_SHOP_ALCHEMIST) {
                                            shop_init(&shop_screen,
                                                SHOP_TYPE_ALCHEMIST);
                                            screen = SCREEN_SHOP;
                                            found = 1;
                                        }
                                        if (t == TILE_SHOP_BLACKSMITH) {
                                            shop_init(&shop_screen,
                                                SHOP_TYPE_BLACKSMITH);
                                            screen = SCREEN_SHOP;
                                            found = 1;
                                        }
                                    }
                                }
                                if (!found)
                                    push_message(&game, "No shop nearby");
                                break;
                            }
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
        // Update music based on screen and location
        int is_town = (game.location == LOCATION_TOWN);
        music_update(screen, is_town);

        if (screen == SCREEN_LANDING) {
            landing_draw(&renderer, &landing);
        } else if (screen == SCREEN_NAME_ENTRY) {
            name_entry_draw(&renderer, &name_entry);
        } else if (screen == SCREEN_SAVE_SLOT || screen == SCREEN_LOAD_SLOT) {
            slot_draw(&renderer, &slot_select, slot_is_save);
        } else if (screen == SCREEN_INVENTORY) {
            inventory_draw(&renderer, &game, &inventory_screen);
        } else if (screen == SCREEN_SPELLBOOK) {
            spellbook_draw(&renderer, &game, &spellbook_screen);
        } else if (screen == SCREEN_SHOP) {
            shop_draw(&renderer, &game, &shop_screen);
        } else if (screen == SCREEN_PLAYING) {
        game_draw(&renderer, &game, &viewport);
        } else if (screen == SCREEN_GAME_OVER) {
            game_over_draw(&renderer, &game);
        }

        renderer_end_frame(&renderer);
    }

    // ── Cleanup ───────────────────────────────────────────────────────────
    music_free();
    renderer_free(&renderer);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}