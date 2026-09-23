#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "screens/quest_journal.h"
#include "renderer/quest_journal_renderer.h"
#include "screens/shop.h"
#include "renderer/shop_renderer.h"
#include "screens/harbor.h"
#include "renderer/harbor_renderer.h"
#include "renderer/game_renderer.h"
#include "renderer/sprites.h"
#include "renderer/info_panel.h"
#include "renderer/message_bar.h"
#include "audio/music.h"
#include "audio/sfx.h"
#include "renderer/help_renderer.h"
#include "screens/help.h"
#include "systems/highscore.h"
#include "renderer/halloffame_renderer.h"
#include "screens/class_select.h"
#include "renderer/class_select_renderer.h"

#define WINDOW_TITLE "Castle of No Return"
#define WINDOW_W     1280
#define WINDOW_H     720
#define ENTRY_GATE_CLOSE_MS 240u
#define ENTRY_GATE_HOLD_MS 100u
#define ENTRY_GATE_OPEN_MS 260u
#define ENTRY_GATE_TOTAL_MS (ENTRY_GATE_CLOSE_MS + ENTRY_GATE_HOLD_MS + ENTRY_GATE_OPEN_MS)
#define IDLE_EVENT_WAIT_MS 100

typedef struct {
    int active;
    int switched;
    Uint32 started_at;
    Action pending_action;
    TownExitStyle style;
} TownEntryTransition;

static int town_entry_style(const GameState *g, Action action, TownExitStyle *style) {
    if (g->location != LOCATION_TOWN || action.type != ACTION_MOVE ||
        action.target_x < 0 || action.target_x >= MAP_W ||
        action.target_y < 0 || action.target_y >= MAP_H ||
        g->map.tiles[action.target_y][action.target_x] != TILE_TOWN_EXIT) {
        return 0;
    }
    if (action.target_y == 0) {
        *style = TOWN_EXIT_DUNGEON;
        return 1;
    }
    if (action.target_x == 0) {
        *style = TOWN_EXIT_FOREST;
        return 1;
    }
    if (action.target_x == TOWN_W - 1) {
        *style = TOWN_EXIT_MOUNTAINS;
        return 1;
    }
    if (action.target_y == TOWN_H - 1) {
        *style = TOWN_EXIT_COAST;
        return 1;
    }
    return 0;
}

static int entry_gate_width(Uint32 elapsed, int screen_width) {
    int max_width = (screen_width + 1) / 2;
    if (elapsed < ENTRY_GATE_CLOSE_MS) {
        return (int)((Uint64)max_width * elapsed / ENTRY_GATE_CLOSE_MS);
    }
    if (elapsed < ENTRY_GATE_CLOSE_MS + ENTRY_GATE_HOLD_MS) {
        return max_width;
    }
    if (elapsed < ENTRY_GATE_TOTAL_MS) {
        return (int)((Uint64)max_width *
            (ENTRY_GATE_TOTAL_MS - elapsed) / ENTRY_GATE_OPEN_MS);
    }
    return 0;
}

#ifdef DEBUG
typedef struct {
    int weapon_set;
    int weapon_none;
    Item weapon;
    int gold_set;
    int gold;
    int scrolls_set;
    int scrolls[3];
    int scroll_count;
} DebugConfig;

static int debug_weapon(const char *name, Item *weapon, int *none) {
    *none = 0;
    if (strcmp(name, "rusty-sword") == 0) {
        *weapon = item_make_rusty_sword();
    } else if (strcmp(name, "short-sword") == 0) {
        *weapon = item_make_short_sword();
    } else if (strcmp(name, "long-sword") == 0) {
        *weapon = item_make_long_sword();
    } else if (strcmp(name, "battle-axe") == 0) {
        *weapon = item_make_battle_axe();
    } else if (strcmp(name, "staff") == 0) {
        *weapon = item_make_staff();
    } else if (strcmp(name, "bow") == 0) {
        *weapon = item_make_bow();
    } else if (strcmp(name, "none") == 0) {
        *none = 1;
    } else {
        return 0;
    }
    return 1;
}

static int debug_scroll_id(const char *name, size_t len) {
    if (len == 11 && strncmp(name, "magic-arrow", len) == 0) {
        return SPELL_MAGIC_ARROW;
    }
    if (len == 8 && strncmp(name, "fireball", len) == 0) {
        return SPELL_FIREBALL;
    }
    if (len == 4 && strncmp(name, "heal", len) == 0) {
        return SPELL_HEAL;
    }
    return -1;
}

static int debug_parse_scrolls(DebugConfig *config, const char *list) {
    config->scroll_count = 0;
    if (strcmp(list, "none") == 0) {
        return 1;
    }
    const char *start = list;
    while (*start != '\0') {
        const char *end = strchr(start, ',');
        size_t len = end ? (size_t)(end - start) : strlen(start);
        int id = debug_scroll_id(start, len);
        if (id < 0 || config->scroll_count >= 3) {
            return 0;
        }
        config->scrolls[config->scroll_count++] = id;
        if (!end) {
            break;
        }
        start = end + 1;
    }
    return 1;
}

static int debug_parse_args(DebugConfig *config, int argc, char **argv) {
    *config = (DebugConfig){0};
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--weapon") == 0 && i + 1 < argc) {
            config->weapon_set = 1;
            if (!debug_weapon(argv[++i], &config->weapon,
                    &config->weapon_none)) {
                fprintf(stderr, "Unknown debug weapon: %s\n", argv[i]);
                return 0;
            }
        } else if (strcmp(argv[i], "--gold") == 0 && i + 1 < argc) {
            char *end = NULL;
            long gold = strtol(argv[++i], &end, 10);
            if (*argv[i] == '\0' || *end != '\0' || gold < 0 || gold > 999999) {
                fprintf(stderr, "Invalid debug gold amount: %s\n", argv[i]);
                return 0;
            }
            config->gold_set = 1;
            config->gold = (int)gold;
        } else if (strcmp(argv[i], "--scrolls") == 0 && i + 1 < argc) {
            config->scrolls_set = 1;
            if (!debug_parse_scrolls(config, argv[++i])) {
                fprintf(stderr, "Invalid debug scroll list: %s\n", argv[i]);
                return 0;
            }
        } else {
            fprintf(stderr, "Unknown or incomplete debug option: %s\n", argv[i]);
            return 0;
        }
    }
    return 1;
}

static Item debug_scroll(int id) {
    if (id == SPELL_FIREBALL) {
        return item_make_scroll_fireball();
    }
    if (id == SPELL_HEAL) {
        return item_make_scroll_heal();
    }
    return item_make_scroll_magic_arrow();
}

static void debug_apply_loadout(GameState *game, const DebugConfig *config) {
    if (config->weapon_set || config->scrolls_set) {
        int write = 0;
        for (int i = 0; i < game->inventory_count; i++) {
            Item *item = &game->inventory[i];
            if (config->weapon_set && item->type == ITEM_WEAPON) {
                continue;
            }
            if (config->scrolls_set && item->type == ITEM_SCROLL) {
                continue;
            }
            game->inventory[write++] = *item;
        }
        game->inventory_count = write;
        game->equipped_main_hand = -1;
        game->equipped_off_hand = -1;
    }
    if (config->weapon_set && !config->weapon_none &&
        game->inventory_count < MAX_INVENTORY) {
        game->inventory[game->inventory_count++] = config->weapon;
    }
    if (config->scrolls_set) {
        for (int i = 0; i < config->scroll_count &&
            game->inventory_count < MAX_INVENTORY; i++) {
            game->inventory[game->inventory_count++] =
                debug_scroll(config->scrolls[i]);
        }
    }
    if (config->gold_set) {
        game->gold = config->gold;
    }
}
#endif

static void enter_playing(Renderer *renderer, Viewport *viewport, GameState *game) {
    int vp_tiles_x = (renderer->screen_w - INFO_PANEL_W) / TILE_SIZE;
    viewport_init(viewport, vp_tiles_x, renderer->tiles_y, MAP_W, MAP_H);
    viewport_center_on(viewport, game->player.x, game->player.y);
}

static int open_shop_on_move(const GameState *game, const Action *action, ShopScreen *shop, GameScreen *screen) {
    if (action->type != ACTION_MOVE || game->location != LOCATION_TOWN ||
        action->target_x < 0 || action->target_x >= MAP_W ||
        action->target_y < 0 || action->target_y >= MAP_H) {
        return 0;
    }
    TileType tile = game->map.tiles[action->target_y][action->target_x];
    if (tile == TILE_BLACKSMITH_DOOR) {
        shop_init(shop, SHOP_TYPE_BLACKSMITH, game->defeated_bosses);
    } else if (tile == TILE_ALCHEMIST_DOOR) {
        shop_init(shop, SHOP_TYPE_ALCHEMIST, game->defeated_bosses);
    } else if (tile == TILE_HEALER_DOOR) {
        shop_init(shop, SHOP_TYPE_HEALER, game->defeated_bosses);
    } else {
        return 0;
    }
    *screen = SCREEN_SHOP;
    return 1;
}

static void handle_harbor_result(HarborResult result, GameState *game, GameScreen *screen, Renderer *renderer, Viewport *viewport) {
    if (result == HARBOR_CLOSED) {
        *screen = SCREEN_PLAYING;
        push_message(game, game->location == LOCATION_ISLAND
            ? "You remain on the island." : "You return to town.");
    } else if (result == HARBOR_MAP_REQUIRED) {
        push_message(game, "The captain needs a sea chart. Speak with Rowan.");
    } else if (result == HARBOR_BOARD) {
        game_enter_island(game);
        enter_playing(renderer, viewport, game);
        *screen = SCREEN_PLAYING;
    } else if (result == HARBOR_SAIL_TOWN) {
        game_leave_island(game);
        enter_playing(renderer, viewport, game);
        *screen = SCREEN_PLAYING;
    }
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
        case LANDING_TOGGLE_MUSIC:
            music_toggle();
            break;
        case LANDING_TOGGLE_SFX:
            sfx_toggle();
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

static void handle_slot_result(SlotResult result, SlotSelect *slots, int saving, GameScreen *screen, GameState *game, Renderer *renderer, Viewport *viewport, LandingScreen *landing) {
    int slot = slots->selected + 1;

    if (result == SLOT_CANCELLED) {
        *screen = SCREEN_LANDING;
        return;
    }
    if (result == SLOT_SAVE_CONFIRMED && saving) {
        if (save_game(game, slot)) {
            *screen = SCREEN_LANDING;
        }
        return;
    }
    if (result == SLOT_SELECTED) {
        if (saving) {
            slot_select_begin_save(slots, save_exists(slot));
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

int main(int argc, char **argv) {
#ifdef DEBUG
    DebugConfig debug_config;
    if (!debug_parse_args(&debug_config, argc, argv)) {
        fprintf(stderr, "Usage: %s [--weapon NAME] [--gold N] "
            "[--scrolls LIST]\n", argv[0]);
        return 2;
    }
#else
    (void)argc;
    (void)argv;
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

#ifdef __APPLE__
    // Prefer OpenGL for stable native fullscreen presentation on macOS.
    const char *render_driver = SDL_getenv(SDL_HINT_RENDER_DRIVER);
    if (!render_driver || !*render_driver) {
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    }
    const char *render_batching = SDL_getenv(SDL_HINT_RENDER_BATCHING);
    if (!render_batching || !*render_batching) {
        SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    }
#endif

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
    SDL_SetWindowMinimumSize(window, 600, 240);

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
    sfx_init();

    GameState game = {0};
    game_init(&game);
    // Presentation-only snapshot; persistent results remain in game.
    static GameState spell_view;
    int spell_animating = 0;
    EnemyProjectiles enemy_shots = {0};
    Uint32 enemy_shots_started_at = 0;
    TownEntryTransition entry_gate = {0};

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
    QuestJournalScreen quest_journal_screen;
    quest_journal_init(&quest_journal_screen);
    ClassSelectScreen class_select_screen;
    class_select_init(&class_select_screen);
    ShopScreen shop_screen;
    HarborScreen harbor_screen;
    harbor_init(&harbor_screen);
    HighScoreTable highscore_table;
    highscore_load(&highscore_table);

    int slot_is_save = 0;
    GameScreen screen = SCREEN_LANDING;

    int running = 1;
    int needs_redraw = 1;
    SDL_Event event;

    while (running) {
        int animating = spell_animating || entry_gate.active || enemy_shots.count > 0 ||
            (screen == SCREEN_PLAYING && game.trail_frames > 0);
        int ambient_animating = screen == SCREEN_PLAYING &&
            (game.location == LOCATION_DUNGEON ||
            game.location == LOCATION_FOREST ||
            game.location == LOCATION_MOUNTAINS ||
            game.location == LOCATION_COAST ||
            game.location == LOCATION_ISLAND ||
            game.location == LOCATION_TEMPLE);
        int has_event = SDL_PollEvent(&event);
        // Static scenes need no new present until input, exposure, or cursor blink.
        if (!has_event && !needs_redraw && !animating) {
            if (screen == SCREEN_NAME_ENTRY) {
                Uint32 elapsed = SDL_GetTicks() - name_entry.cursor_last_blink;
                int timeout = elapsed >= 500 ? 0 : (int)(500 - elapsed);
                has_event = SDL_WaitEventTimeout(&event, timeout);
            } else if (ambient_animating) {
                has_event = SDL_WaitEventTimeout(&event,
                    (int)AMBIENT_FRAME_MS);
                if (!has_event) {
                    needs_redraw = 1;
                }
            } else {
                // SDL delivers signal-generated quit events at the next poll.
                has_event = SDL_WaitEventTimeout(&event, IDLE_EVENT_WAIT_MS);
            }
        }
        while (has_event) {
            switch (event.type) {

                // ── Quit ──────────────────────────────────────────────────
                case SDL_QUIT:
                    running = 0;
                    break;

                // ── Window resize ─────────────────────────────────────────
                case SDL_WINDOWEVENT:
                    needs_redraw = 1;
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                        event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                        event.window.event == SDL_WINDOWEVENT_DISPLAY_CHANGED) {
                        int window_w;
                        int window_h;
                        SDL_GetWindowSize(window, &window_w, &window_h);
                        if (window_w > INFO_PANEL_W && window_h > MESSAGE_BAR_H) {
                            renderer_on_resize(&renderer, window_w, window_h);
                            viewport_on_resize(&viewport,
                                (renderer.screen_w - INFO_PANEL_W) / TILE_SIZE,
                                renderer.tiles_y);
                            viewport_center_on(&viewport,
                                game.player.x, game.player.y);
                        }
                    }
                    break;

                // ── Keyboard input ────────────────────────────────────────
                case SDL_KEYDOWN: {
                    needs_redraw = 1;
                    if (spell_animating || entry_gate.active || enemy_shots.count > 0) {
                        break;
                    }
                    int sc = event.key.keysym.scancode;

                    // Game over screen
                    if (screen == SCREEN_GAME_OVER) {
                        if (sc == SDL_SCANCODE_RETURN) {
                            if (highscore_qualifies(&highscore_table, game.score)) {
                                highscore_insert(&highscore_table,
                                    game.player.name, game.score, game.level);
                                highscore_save(&highscore_table);
                            }
                            screen = SCREEN_HALL_OF_FAME;
                        }
                        break;
                    }

                    // Hall of fame screen
                    if (screen == SCREEN_HALL_OF_FAME) {
                        if (sc == SDL_SCANCODE_RETURN || sc == SDL_SCANCODE_ESCAPE) {
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
                            class_select_init(&class_select_screen);
                            screen = SCREEN_CLASS_SELECT;
                        }
                        if (result == NAME_ENTRY_CANCELLED)
                            screen = SCREEN_LANDING;
                        break;
                    }

                    // Class select screen
                    if (screen == SCREEN_CLASS_SELECT) {
                        ClassSelectResult result = class_select_handle_key(
                            &class_select_screen, sc);
                        if (result == CLASS_SELECT_CONFIRMED) {
                            game.player.player_class = class_select_screen.selected;
                            game_init(&game);
                            SDL_strlcpy(game.player.name, name_entry.name,
                                sizeof(game.player.name));
                            #ifdef DEBUG
                            debug_apply_loadout(&game, &debug_config);
                            #endif
                            enter_playing(&renderer, &viewport, &game);
                            screen = SCREEN_PLAYING;
                        }
                        if (result == CLASS_SELECT_CANCELLED) {
                            screen = SCREEN_NAME_ENTRY;
                        }
                        break;
                    }

                    // Save / load slot screen
                    if (screen == SCREEN_SAVE_SLOT || screen == SCREEN_LOAD_SLOT) {
                        SlotResult result = slot_select_handle_key(&slot_select, sc);
                        handle_slot_result(result, &slot_select, slot_is_save,
                            &screen, &game, &renderer, &viewport, &landing);
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
                        } else if (result == INVENTORY_EQUIP_OFF_HAND) {
                            Action a = {ACTION_EQUIP_OFF_HAND,
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

                    if (screen == SCREEN_QUEST_JOURNAL) {
                        int quest_count = quest_journal_count(&game,
                            quest_journal_screen.tab);
                        QuestJournalResult result = quest_journal_handle_key(
                            &quest_journal_screen, sc, quest_count);
                        if (result == QUEST_JOURNAL_CLOSED) {
                            screen = SCREEN_PLAYING;
                        }
                        break;
                    }

                    // Shop screen
                    if (screen == SCREEN_SHOP) {
                        ShopResult result = shop_handle_key(&shop_screen, sc);
                        if (shop_screen.mode == 1 && shop_screen.selected >= game.inventory_count) {
                            shop_screen.selected = game.inventory_count > 0 ? game.inventory_count - 1 : 0;
                        }
                        if (result == SHOP_CLOSED) {
                            screen = SCREEN_PLAYING;
                        } else if (result == SHOP_HEAL) {
                            game_visit_healer(&game);
                        } else if (result == SHOP_BUY) {
                            Item *item = &shop_screen.items[shop_screen.selected];
                            int price = shop_buy_price(item);
                            if (game.gold < price) {
                                push_message(&game, "Not enough gold!");
                            } else if (game.inventory_count >= MAX_INVENTORY) {
                                push_message(&game, "Inventory full!");
                            } else {
                                game.gold -= price;
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
                            Item item = game.inventory[idx];
                            if (item.type == ITEM_TREASURE_MAP) {
                                push_message(&game, "The treasure map is not for sale.");
                                break;
                            }
                            if (!shop_accepts_item(shop_screen.type, &item)) {
                                push_message(&game, shop_screen.type == SHOP_TYPE_ALCHEMIST
                                    ? "Take weapons and equipment to the blacksmith."
                                    : "Take potions, scrolls and tomes to the alchemist.");
                                break;
                            }
                            int sell_price = shop_sell_price(&item);
                            char msg[32];
                            SDL_snprintf(msg, sizeof(msg), "Sold %s for %d gold",
                                item.name, sell_price);
                            game.gold += sell_price;
                            game_remove_inventory_item(&game, idx);
                            if (shop_screen.selected >= game.inventory_count)
                                shop_screen.selected = game.inventory_count - 1;
                            if (shop_screen.selected < 0)
                                shop_screen.selected = 0;
                            push_message(&game, msg);
                        }
                        break;
                    }

                    // Harbor screen
                    if (screen == SCREEN_HARBOR) {
                        HarborResult result = harbor_handle_key(&harbor_screen,
                            sc, game_has_treasure_map(&game),
                            game.location == LOCATION_ISLAND);
                        handle_harbor_result(result, &game, &screen,
                            &renderer, &viewport);
                        break;
                    }

                    // Help screen
                    if (screen == SCREEN_HELP) {
                        HelpResult result = help_handle_key(sc);
                        if (result == HELP_CLOSED) {
                            screen = SCREEN_PLAYING;
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
                                a = (Action){ACTION_MOVE, game.player.x - 1, game.player.y};
                                break;
                            case SDL_SCANCODE_A: {
                                TileType tile = game.map.tiles[game.player.y]
                                    [game.player.x];
                                if (game_has_regional_interaction(&game) ||
                                    tile == TILE_COAST_TIDE_CONTROL ||
                                    tile == TILE_COAST_BEACON_UNLIT ||
                                    tile == TILE_BROKEN_BURIAL_SEAL ||
                                    tile == TILE_CRYPT_CACHE ||
                                    tile == TILE_DUNGEON_SWITCH_OFF ||
                                    tile == TILE_DUNGEON_SWITCH_ON) {
                                    a = (Action){ACTION_INTERACT, 0, 0};
                                } else {
                                    a = (Action){ACTION_MOVE,
                                        game.player.x - 1, game.player.y};
                                }
                                break;
                            }
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
                            case SDL_SCANCODE_Q:
                                quest_journal_init(&quest_journal_screen);
                                screen = SCREEN_QUEST_JOURNAL;
                                break;
                            case SDL_SCANCODE_C:
                                a = (Action){ACTION_CAST_SPELL, 0, 0};
                                break;
                            case SDL_SCANCODE_F:
                                a = (Action){ACTION_RANGED_ATTACK, 0, 0};
                                break;
                            case SDL_SCANCODE_H:
                                screen = SCREEN_HELP;
                                break;
                            case SDL_SCANCODE_T: {
                                int px = game.player.x;
                                int py = game.player.y;
                                int found = 0;
                                for (int dy = -1; dy <= 1 && !found; dy++) {
                                    for (int dx = -1; dx <= 1 && !found; dx++) {
                                        int tx = px + dx;
                                        int ty = py + dy;
                                        if (tx < 0 || tx >= MAP_W ||
                                            ty < 0 || ty >= MAP_H) {
                                            continue;
                                        }
                                        if (game.map.tiles[ty][tx] ==
                                            TILE_NPC_ELOWEN) {
                                            game_talk_to_elowen(&game);
                                            found = 1;
                                        } else if (game.map.tiles[ty][tx] ==
                                            TILE_NPC_DAIN) {
                                            game_talk_to_dain(&game);
                                            found = 1;
                                        } else if (game.map.tiles[ty][tx] ==
                                            TILE_NPC_ALDER) {
                                            game_talk_to_alder(&game);
                                            found = 1;
                                        } else if (game.map.tiles[ty][tx] ==
                                            TILE_NPC_MARA) {
                                            game_talk_to_mara(&game);
                                            found = 1;
                                        } else if (game.map.tiles[ty][tx] ==
                                            TILE_NPC_CAIN) {
                                            game_talk_to_cain(&game);
                                            found = 1;
                                        } else if (game.map.tiles[ty][tx] ==
                                            TILE_NPC_ROWAN) {
                                            game_talk_to_rowan(&game);
                                            found = 1;
                                        } else if (game.map.tiles[ty][tx] ==
                                            TILE_NPC_ISLAND_CAPTAIN) {
                                            harbor_init(&harbor_screen);
                                            screen = SCREEN_HARBOR;
                                            push_message(&game,
                                                "Captain Rowan offers passage back to town.");
                                            found = 1;
                                        } else if (game.map.tiles[ty][tx] ==
                                            TILE_FOREST_WARDEN) {
                                            game_rescue_forest_warden(&game,
                                                tx, ty);
                                            found = 1;
                                        }
                                    }
                                }
                                if (!found) {
                                    push_message(&game,
                                        "No one to speak with nearby.");
                                }
                                break;
                            }
                            default: break;
                        }
                        if (open_shop_on_move(&game, &a, &shop_screen,
                            &screen)) {
                            a.type = ACTION_NONE;
                        }
                        if (event.key.repeat && a.type == ACTION_MOVE &&
                            a.target_x >= 0 && a.target_x < MAP_W &&
                            a.target_y >= 0 && a.target_y < MAP_H) {
                            TileType target = game.map.tiles[a.target_y][a.target_x];
                            if (target == TILE_TOWN_EXIT ||
                                target == TILE_FOREST_ENTRANCE ||
                                target == TILE_FOREST_EXIT ||
                                target == TILE_MOUNTAIN_ENTRANCE ||
                                target == TILE_MOUNTAIN_EXIT ||
                                target == TILE_COAST_ENTRANCE ||
                                target == TILE_COAST_EXIT ||
                                target == TILE_TAVERN_DOOR ||
                                target == TILE_TAVERN_EXIT ||
                                target == TILE_RETURN_EXIT) {
                                a.type = ACTION_NONE;
                            }
                        }
                        TownExitStyle entry_style;
                        if (town_entry_style(&game, a, &entry_style)) {
                            entry_gate.active = 1;
                            entry_gate.switched = 0;
                            entry_gate.started_at = SDL_GetTicks();
                            entry_gate.pending_action = a;
                            entry_gate.style = entry_style;
                            a.type = ACTION_NONE;
                        }
                        if (a.type != ACTION_NONE) {
                            int was_at_harbor_entrance = game.location == LOCATION_TOWN &&
                                game.player.x == TOWN_HARBOR_ENTRANCE_X &&
                                game.player.y == TOWN_HARBOR_ENTRANCE_Y;
                            if (a.type == ACTION_CAST_SPELL) {
                                spell_view = game;
                            }
                            action_resolve_player(&game, a);
                            if (a.type == ACTION_MOVE &&
                                !was_at_harbor_entrance &&
                                game.location == LOCATION_TOWN &&
                                game_harbor_unlocked(&game) &&
                                game.player.x == TOWN_HARBOR_ENTRANCE_X &&
                                game.player.y == TOWN_HARBOR_ENTRANCE_Y) {
                                harbor_init(&harbor_screen);
                                screen = SCREEN_HARBOR;
                                push_message(&game, "You enter the harbor dock.");
                            }
                            if (a.type == ACTION_CAST_SPELL &&
                                game.player.mp < spell_view.player.mp &&
                                game.trail_count > 0 && game.trail_frames > 0 &&
                                (game.trail_effect == TRAIL_EFFECT_MAGIC_ARROW ||
                                game.trail_effect == TRAIL_EFFECT_FIREBALL)) {
                                memcpy(spell_view.trail, game.trail,
                                    sizeof(game.trail));
                                spell_view.trail_count = game.trail_count;
                                spell_view.trail_frames = game.trail_frames;
                                spell_view.trail_effect = game.trail_effect;
                                spell_view.trail_started_at = game.trail_started_at;
                                spell_view.player.mp = game.player.mp;
                                spell_animating = 1;
                            } else {
                                action_resolve_enemies_with_projectiles(&game, &enemy_shots);
                                enemy_shots_started_at = SDL_GetTicks();
                            }
                            if (game.player.hp <= 0 && enemy_shots.count == 0) {
                                screen = SCREEN_GAME_OVER;
                            }
                            viewport_center_on(&viewport,
                                game.player.x, game.player.y);
                        }
                    }
                    break;
                }

                // ── Mouse input ───────────────────────────────────────────
                case SDL_MOUSEBUTTONDOWN: {
                    needs_redraw = 1;
                    if (entry_gate.active || spell_animating || enemy_shots.count > 0) {
                        break;
                    }
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
                    else if ((screen == SCREEN_SAVE_SLOT || screen == SCREEN_LOAD_SLOT) &&
                        !slot_select.confirming_save) {
                        int base_y = renderer.screen_h / 2 - 20;
                        for (int i = 0; i < 3; i++) {
                            int item_y     = base_y + i * 40;
                            int item_y_end = item_y + 24;
                            if (event.button.y >= item_y &&
                                event.button.y <= item_y_end) {
                                slot_select.selected = i;
                                handle_slot_result(SLOT_SELECTED, &slot_select,
                                    slot_is_save, &screen, &game, &renderer,
                                    &viewport, &landing);
                                break;
                            }
                        }
                    }
                    // Inventory screen clicks
                    if (screen == SCREEN_INVENTORY &&
                        event.button.button == SDL_BUTTON_LEFT) {
                        int base_y = 130;
                        for (int i = 0; i < game.inventory_count; i++) {
                            int item_y = base_y + i * 36;
                            int item_y_end = item_y + 24;
                            if (event.button.y >= item_y &&
                                event.button.y <= item_y_end) {
                                inventory_screen.selected = i;
                            }
                        }
                        // Check hint bar button regions
                        int hint_y = (renderer.tiles_y - 2) * TILE_SIZE;
                        int cx = renderer.screen_w / 2;
                        // U - Use
                        if (event.button.y >= hint_y &&
                            event.button.y <= hint_y + 24 &&
                            event.button.x >= cx - 245 &&
                            event.button.x <= cx - 190) {
                            Action a = {ACTION_USE_ITEM, inventory_screen.selected, 0};
                            action_resolve_player(&game, a);
                            if (game.inventory_count == 0) {
                                screen = SCREEN_PLAYING;
                            }
                        }
                        // E - Equip
                        if (event.button.y >= hint_y &&
                            event.button.y <= hint_y + 24 &&
                            event.button.x >= cx - 180 &&
                            event.button.x <= cx - 100) {
                            Action a = {ACTION_EQUIP_ITEM, inventory_screen.selected, 0};
                            action_resolve_player(&game, a);
                        }
                        // O - Equip off-hand
                        if (event.button.y >= hint_y &&
                            event.button.y <= hint_y + 24 &&
                            event.button.x >= cx - 95 &&
                            event.button.x <= cx + 20) {
                            Action a = {ACTION_EQUIP_OFF_HAND, inventory_screen.selected, 0};
                            action_resolve_player(&game, a);
                        }
                        // D - Drop
                        if (event.button.y >= hint_y &&
                            event.button.y <= hint_y + 24 &&
                            event.button.x >= cx + 25 &&
                            event.button.x <= cx + 90) {
                            Action a = {ACTION_DROP_ITEM, inventory_screen.selected, 0};
                            action_resolve_player(&game, a);
                            if (inventory_screen.selected >= game.inventory_count) {
                                inventory_screen.selected = game.inventory_count - 1;
                            }
                            if (inventory_screen.selected < 0) {
                                inventory_screen.selected = 0;
                            }
                        }
                    }
                    // Spellbook screen clicks
                    if (screen == SCREEN_SPELLBOOK &&
                        event.button.button == SDL_BUTTON_LEFT) {
                        int base_y = 130;
                        for (int i = 0; i < game.player.known_spell_count; i++) {
                            int item_y = base_y + i * 40;
                            int item_y_end = item_y + 30;
                            if (event.button.y >= item_y &&
                                event.button.y <= item_y_end) {
                                spellbook_screen.selected = i;
                                game.player.equipped_spell = i;
                            }
                        }
                    }
                    // Shop screen clicks
                    if (screen == SCREEN_SHOP && event.button.button == SDL_BUTTON_LEFT) {
                        if (shop_screen.type == SHOP_TYPE_HEALER) {
                            SDL_Point point = {event.button.x, event.button.y};
                            SDL_Rect heal = shop_healer_button_rect(&renderer, 0);
                            SDL_Rect leave = shop_healer_button_rect(&renderer, 1);
                            if (SDL_PointInRect(&point, &heal)) {
                                shop_screen.selected = 0;
                                game_visit_healer(&game);
                            } else if (SDL_PointInRect(&point, &leave)) {
                                shop_screen.selected = 1;
                                screen = SCREEN_PLAYING;
                            }
                            break;
                        }
                        int cx = renderer.screen_w / 2;
                        // Tab switching
                        if (event.button.y >= 108 && event.button.y <= 132) {
                            if (event.button.x >= cx - 60 && event.button.x <= cx) {
                                shop_screen.mode = 0;
                                shop_screen.selected = 0;
                            }
                            if (event.button.x >= cx && event.button.x <= cx + 60) {
                                shop_screen.mode = 1;
                                shop_screen.selected = 0;
                            }
                        }
                        // Item selection and purchase
                        int base_y = 140;
                        int count = shop_screen.mode == 0
                            ? shop_screen.item_count
                            : game.inventory_count;
                        for (int i = 0; i < count; i++) {
                            int item_y = base_y + i * 36;
                            int item_y_end = item_y + 28;
                            if (event.button.y >= item_y &&
                                event.button.y <= item_y_end) {
                                if (shop_screen.selected == i) {
                                    // Second click on same item — buy or sell
                                    if (shop_screen.mode == 0) {
                                        Item *item = &shop_screen.items[i];
                                        int price = shop_buy_price(item);
                                        if (game.gold >= price &&
                                            game.inventory_count < MAX_INVENTORY) {
                                            game.gold -= price;
                                            game.inventory[game.inventory_count++] = *item;
                                            char msg[32];
                                            SDL_snprintf(msg, sizeof(msg), "Bought %s", item->name);
                                            push_message(&game, msg);
                                        } else if (game.gold < price) {
                                            push_message(&game, "Not enough gold!");
                                        } else {
                                            push_message(&game, "Inventory full!");
                                        }
                                    } else {
                                        if (game.inventory_count == 0) { break; }
                                        Item item = game.inventory[i];
                                        if (item.type == ITEM_TREASURE_MAP) {
                                            push_message(&game, "The treasure map is not for sale.");
                                            break;
                                        }
                                        if (!shop_accepts_item(shop_screen.type, &item)) {
                                            push_message(&game, shop_screen.type == SHOP_TYPE_ALCHEMIST
                                                ? "Take weapons and equipment to the blacksmith."
                                                : "Take potions, scrolls and tomes to the alchemist.");
                                            break;
                                        }
                                        int sell_price = shop_sell_price(&item);
                                        char msg[32];
                                        SDL_snprintf(msg, sizeof(msg), "Sold %s for %d gold", item.name, sell_price);
                                        game.gold += sell_price;
                                        game_remove_inventory_item(&game, i);
                                        if (shop_screen.selected >= game.inventory_count) {
                                            shop_screen.selected = game.inventory_count - 1;
                                        }
                                        if (shop_screen.selected < 0) {
                                            shop_screen.selected = 0;
                                        }
                                        push_message(&game, msg);
                                    }
                                } else {
                                    // First click — just select
                                    shop_screen.selected = i;
                                }
                                break;
                            }
                        }
                    }
                    // Harbor screen clicks
                    if (screen == SCREEN_HARBOR && event.button.button == SDL_BUTTON_LEFT) {
                        SDL_Point point = {event.button.x, event.button.y};
                        for (int option = 0; option < 2; option++) {
                            SDL_Rect button = harbor_button_rect(&renderer, option);
                            if (SDL_PointInRect(&point, &button)) {
                                harbor_screen.selected = option;
                                HarborResult result = harbor_activate(&harbor_screen,
                                    game_has_treasure_map(&game),
                                    game.location == LOCATION_ISLAND);
                                handle_harbor_result(result, &game, &screen,
                                    &renderer, &viewport);
                                break;
                            }
                        }
                        break;
                    }
                    // Help screen clicks
                    if (screen == SCREEN_HELP && event.button.button == SDL_BUTTON_LEFT) {
                        screen = SCREEN_PLAYING;
                    }
                    // Hall of fame clicks
                    if (screen == SCREEN_HALL_OF_FAME &&
                        event.button.button == SDL_BUTTON_LEFT) {
                        landing_init(&landing);
                        screen = SCREEN_LANDING;
                    }
                    break;
                }

                default:
                    break;
            }
            has_event = SDL_PollEvent(&event);
        }

        if (!running) {
            break;
        }
        if (animating) {
            needs_redraw = 1;
        }

        // ── Per-frame updates ─────────────────────────────────────────────
        if (spell_animating) {
            Uint32 duration = game.trail_effect == TRAIL_EFFECT_FIREBALL
                ? SPELL_FIREBALL_MS : SPELL_ARROW_MS;
            if (SDL_GetTicks() - game.trail_started_at >= duration) {
                spell_animating = 0;
                game.trail_frames = 0;
                action_resolve_enemies_with_projectiles(&game, &enemy_shots);
                enemy_shots_started_at = SDL_GetTicks();
                if (game.player.hp <= 0 && enemy_shots.count == 0) {
                    screen = SCREEN_GAME_OVER;
                }
            }
        }
        if (entry_gate.active) {
            Uint32 elapsed = SDL_GetTicks() - entry_gate.started_at;
            if (!entry_gate.switched && elapsed >= ENTRY_GATE_CLOSE_MS) {
                action_resolve_player(&game, entry_gate.pending_action);
                action_resolve_enemies_with_projectiles(&game, &enemy_shots);
                enemy_shots_started_at = SDL_GetTicks();
                viewport_center_on(&viewport, game.player.x, game.player.y);
                entry_gate.switched = 1;
                if (game.player.hp <= 0 && enemy_shots.count == 0) {
                    screen = SCREEN_GAME_OVER;
                }
            }
            if (elapsed >= ENTRY_GATE_TOTAL_MS) {
                entry_gate.active = 0;
            }
        }
        if (enemy_shots.count > 0 &&
            SDL_GetTicks() - enemy_shots_started_at >= ENEMY_PROJECTILE_TOTAL_MS) {
            enemy_shots.count = 0;
            needs_redraw = 1;
            if (game.player.hp <= 0) {
                screen = SCREEN_GAME_OVER;
            }
        }
        if (screen == SCREEN_NAME_ENTRY) {
            int cursor_visible = name_entry.cursor_visible;
            name_entry_update(&name_entry);
            if (name_entry.cursor_visible != cursor_visible) {
                needs_redraw = 1;
            }
        }

        // Update music based on screen and location
        int is_town = game.location == LOCATION_TOWN ||
            game.location == LOCATION_TAVERN ||
            game.location == LOCATION_ISLAND;
        music_update(screen, is_town);

        if (!needs_redraw) {
            continue;
        }

        // ── Rendering ─────────────────────────────────────────────────────
        renderer_begin_frame(&renderer);

        if (screen == SCREEN_LANDING) {
            landing_draw(&renderer, &landing);
        } else if (screen == SCREEN_NAME_ENTRY) {
            name_entry_draw(&renderer, &name_entry);
        } else if (screen == SCREEN_CLASS_SELECT) {
            class_select_draw(&renderer, &class_select_screen);
        } else if (screen == SCREEN_SAVE_SLOT || screen == SCREEN_LOAD_SLOT) {
            slot_draw(&renderer, &slot_select, slot_is_save);
        } else if (screen == SCREEN_INVENTORY) {
            inventory_draw(&renderer, &game, &inventory_screen);
        } else if (screen == SCREEN_SPELLBOOK) {
            spellbook_draw(&renderer, &game, &spellbook_screen);
        } else if (screen == SCREEN_QUEST_JOURNAL) {
            quest_journal_draw(&renderer, &game, &quest_journal_screen);
        } else if (screen == SCREEN_SHOP) {
            shop_draw(&renderer, &game, &shop_screen);
        } else if (screen == SCREEN_HARBOR) {
            harbor_draw(&renderer, &game, &harbor_screen);
        } else if (screen == SCREEN_PLAYING) {
            GameState *view = spell_animating &&
                SDL_GetTicks() - game.trail_started_at < SPELL_TRAVEL_MS
                ? &spell_view : &game;
            game_draw(&renderer, view, &viewport);
            game_draw_enemy_projectiles(&renderer, &enemy_shots, &viewport,
                SDL_GetTicks() - enemy_shots_started_at);
        } else if (screen == SCREEN_GAME_OVER) {
            game_over_draw(&renderer, &game);
        } else if (screen == SCREEN_HELP) {
            help_draw(&renderer);
        } else if (screen == SCREEN_HALL_OF_FAME) {
            halloffame_draw(&renderer, &highscore_table);
        }

        if (entry_gate.active) {
            Uint32 elapsed = SDL_GetTicks() - entry_gate.started_at;
            int covered_width = entry_gate_width(elapsed, renderer.screen_w);
            if (entry_gate.style == TOWN_EXIT_FOREST) {
                draw_forest_transition(&renderer, covered_width);
            } else if (entry_gate.style == TOWN_EXIT_MOUNTAINS) {
                draw_mountain_transition(&renderer, covered_width);
            } else if (entry_gate.style == TOWN_EXIT_COAST) {
                draw_coast_transition(&renderer, covered_width);
            } else {
                draw_dungeon_transition(&renderer, covered_width);
            }
        }

        renderer_end_frame(&renderer);
        needs_redraw = 0;
    }

    // ── Cleanup ───────────────────────────────────────────────────────────
    sfx_free();
    music_free();
    renderer_free(&renderer);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
