#include "game/combat_feedback.h"
#include "../../src/game/coast_exits.h"
#include "game/boss_gates.h"
#include "game/containers.h"
#include "game/giants_crown.h"
#include "game/spice_skiff.h"
#include "game/stormbell.h"
#include "game/war_machine.h"
#include "game/crown_quest.h"
#include "renderer/giants_crown_renderer.h"
#include "game/crownroad_north.h"
#include "game/ironfang.h"
#include "game/stonewake.h"
#include "game/stormwatch.h"
#include "renderer/stonewake_renderer.h"
#include "renderer/sprites.h"
#include "renderer/item_icons.h"
// Exercise the application's actual argument parsing and load-screen handler.
#define main conr_app_main
#include "../../src/main.c"
#undef main
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../../src/game/town_road.h"
#include "../../src/game/castle.h"
#include "../../src/game/elderwood.h"
#include "../../src/game/mangrove.h"
#include "../../src/game/coastal_travel.h"

static void test_boss_gate_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    assert(r->road_checkpoint_texture && r->road_bank_texture);
    const int routes[] = {1, 15, 0, 4, 2, 11, 5, 9, 16, 14};
    for (int n = 0; n < 10; n++) {
        for (int open = 0; open < 2; open++) {
            game_init(g);
            if (open) {
                g->elderwood_defeated = 1;
                g->defeated_bosses = (1 << LOCATION_DUNGEON) | (1 << LOCATION_FOREST) | (1 << LOCATION_MOUNTAINS) |
                    (1 << LOCATION_SWAMP) | (1 << LOCATION_CATACOMBS);
            }
            approach_enter_region(g, routes[n]);
            RoadCheckpoint points[2];
            int count = boss_gate_checkpoints(g, points);
            assert(count > 0);
            RoadCheckpoint point = points[routes[n] == 14 ? 1 : 0];
            g->player.x = point.x + (point.vertical ? 0 : point.x < APPROACH_W / 2 ? 2 : -2);
            g->player.y = point.y + (!point.vertical ? 0 : routes[n] == 14 ? -2 : point.y < APPROACH_H / 2 ? 2 : -2);
            assert(map_is_walkable(&g->map, g->player.x, g->player.y));
            for (int y = 0; y < APPROACH_H; y++) {
                for (int x = 0; x < APPROACH_W; x++) {
                    map_mark_explored(&g->map, x, y);
                }
            }
            Viewport view;
            viewport_init(&view, (r->screen_w - INFO_PANEL_W) / TILE_SIZE, r->tiles_y, APPROACH_W, APPROACH_H);
            viewport_center_on(&view, g->player.x, g->player.y);
            renderer_begin_frame(r);
            game_draw(r, g, &view);
            renderer_end_frame(r);
            char path[100];
            snprintf(path, sizeof(path), "/tmp/boss-gate-%d-%d.bmp", routes[n], open);
            assert(SDL_SaveBMP(surface, path) == 0);
        }
    }
    puts("PASS: boss checkpoints render closed/open at player scale from both approaches, including both northern passes.");
}

static void test_container_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    assert(r->loot_chest_texture);
    const Location areas[] = {LOCATION_DUNGEON, LOCATION_COAST, LOCATION_GIANTS_CROWN};
    const TileType floors[] = {TILE_FLOOR, TILE_COAST_FLOOR, TILE_GIANTS_GRASS};
    const TileType chests[] = {TILE_CRYPT_CACHE, TILE_COAST_CACHE, TILE_GIANTS_CACHE};
    const char *names[] = {"dungeon", "coast", "giants"};
    Viewport view;
    for (int n = 0; n < 3; n++) {
        game_init(g);
        g->location = areas[n];
        g->level = 1;
        g->enemy_count = 0;
        g->floor_item_count = 0;
        g->player.x = 13;
        g->player.y = 12;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                g->map.tiles[y][x] = floors[n];
                map_mark_explored(&g->map, x, y);
            }
        }
        g->map.tiles[12][12] = chests[n];
        viewport_init(&view, r->tiles_x, r->tiles_y, MAP_W, MAP_H);
        viewport_center_on(&view, 13, 12);
        for (int step = 0; step < 4; step++) {
            if (step == 1 || step == 2) {
                Action action = move_left_action(g, g->key_bindings[CONTROL_MOVE_LEFT]);
                assert(action.type == ACTION_INTERACT);
                action_resolve_player(g, action);
            } else if (step == 3) {
                action_resolve_player(g, (Action){ACTION_PICK_UP, 0, 0});
                assert(g->gold > 0);
            }
            assert(move_left_action(g, SDL_SCANCODE_LEFT).type == ACTION_MOVE);
            renderer_begin_frame(r);
            game_draw(r, g, &view);
            renderer_end_frame(r);
            char path[120];
            snprintf(path, sizeof(path), "/tmp/chest-%s-%d.bmp", names[n], step);
            assert(SDL_SaveBMP(surface, path) == 0);
        }
    }
    puts("PASS: A opens/closes chests, P collects loose gold, and arrows still move; native renderer captures all chest states.");
}

static void test_heartseed_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    game_init(g);
    g->player.x = TAMSIN_X + 1;
    g->player.y = TAMSIN_Y;
    Viewport view;
    int width = (r->screen_w - INFO_PANEL_W) / TILE_SIZE;
    viewport_init(&view, width, r->tiles_y, TOWN_W, TOWN_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/heartseed-tamsin-town.bmp") == 0);
    assert(heartseed_talk(g) && game_quest_offer_active(g));
    assert(game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0));
    g->dialogue_active = 0;
    elderwood_enter(g);
    game_descend(g);
    g->player.x = HEARTSEED_ORDERS_X + 1;
    g->player.y = HEARTSEED_ORDERS_Y;
    viewport_init(&view, width, r->tiles_y, ELDERWOOD_W, ELDERWOOD_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/heartseed-orders-camp.bmp") == 0);
    assert(heartseed_interact(g));
    game_descend(g);
    game_descend(g);
    g->player.x = HEARTSEED_X;
    g->player.y = HEARTSEED_Y + 2;
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/heartseed-bound-grove.bmp") == 0);
    for (int i = 0; i < 3; i++) {
        g->player.x = heartseed_stakes[i][0];
        g->player.y = heartseed_stakes[i][1] + 1;
        assert(heartseed_interact(g));
    }
    assert(g->heartseed_guardian == HEARTSEED_FREED);
    g->player.x = HEARTSEED_X;
    g->player.y = HEARTSEED_Y + 1;
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/heartseed-freed-grove.bmp") == 0);
    assert(heartseed_interact(g) && g->heartseed_quest_state == 2);
    QuestJournalScreen journal;
    quest_journal_init(&journal);
    renderer_begin_frame(r);
    quest_journal_draw(r, g, &journal);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/heartseed-journal.bmp") == 0);
    puts("PASS: Heartseed NPC, orders, root arch, bindings, guardian release and quest journal render in the native game renderer.");
}

static void test_stormbell_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    assert(r->stormbell_texture);
    int width = (r->screen_w - INFO_PANEL_W) / TILE_SIZE;
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    g->player.x = SEREN_X;
    g->player.y = SEREN_Y + 1;
    Viewport view;
    viewport_init(&view, width, r->tiles_y, TOWN_W, TOWN_H);
    viewport_center_on(&view, 21, 11);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/stormbell-seren-town.bmp") == 0);
    assert(stormbell_talk(g) && game_quest_offer_active(g));
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/stormbell-seren-offer.bmp") == 0);
    assert(game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0));
    g->dialogue_active = 0;
    stormwatch_enter(g, 0);
    stormwatch_change_level(g, 1);
    stormwatch_change_level(g, 1);
    g->player.x = STORMBELL_CLAPPER_X;
    g->player.y = STORMBELL_CLAPPER_Y + 2;
    viewport_init(&view, width, r->tiles_y, STORM_W, STORM_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/stormbell-tower-clapper.bmp") == 0);
    g->player.y--;
    assert(stormbell_interact(g));
    stormwatch_change_level(g, 1);
    g->player.x = STORMBELL_X;
    g->player.y = STORMBELL_Y + 1;
    g->stormwatch[3].phase[0] = 0;
    g->stormwatch[3].phase[1] = 0;
    assert(move_left_action(g, g->key_bindings[CONTROL_MOVE_LEFT]).type == ACTION_INTERACT);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/stormbell-broken-bell.bmp") == 0);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    assert(g->stormbell_progress == 3);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    assert(stormbell_prompt_active(g));
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/stormbell-confirmation.bmp") == 0);
    assert(stormbell_handle_key(g, SDL_SCANCODE_Y, 0));
    assert(g->stormbell_phase == 1);
    for (int y = 18; y <= 28; y++) {
        for (int x = 29; x <= 44; x++) {
            map_mark_explored(&g->map, x, y);
        }
    }
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/stormbell-raider-defense.bmp") == 0);
    QuestJournalScreen journal;
    quest_journal_init(&journal);
    renderer_begin_frame(r);
    quest_journal_draw(r, g, &journal);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/stormbell-journal.bmp") == 0);
    puts("PASS: Seren, clapper, broken and repaired bell, confirmation, finite Raiders and journal render at native scale.");
}

static void test_war_machine_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    int atlas_w;
    int atlas_h;
    assert(r->war_machine_texture);
    assert(SDL_QueryTexture(r->war_machine_texture, NULL, NULL, &atlas_w, &atlas_h) == 0 && atlas_w == 168 && atlas_h == 48);
    game_init(g);
    game_enter_town_gate(g, LOCATION_TOWN4, 20, TOWN_H - 1);
    game_enter_town_hall(g);
    g->player.x = HALL_VEYRA_X;
    g->player.y = HALL_VEYRA_Y + 1;
    Viewport view;
    int width = (r->screen_w - INFO_PANEL_W) / TILE_SIZE;
    viewport_init(&view, width, r->tiles_y, TOWN_W, TOWN_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    game_talk_to_veyra(g);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/war-machine-veyra-topics.bmp") == 0);
    assert(war_machine_handle_key(g, SDL_SCANCODE_2, 0) && game_quest_offer_active(g));
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/war-machine-veyra-offer.bmp") == 0);
    assert(game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0));
    g->dialogue_active = 0;
    ironfang_enter(g, 0);
    ironfang_change_level(g, 1);
    g->player.x = WAR_ORDERS_X;
    g->player.y = WAR_ORDERS_Y + 1;
    map_mark_explored(&g->map, WAR_ORDERS_X, WAR_ORDERS_Y);
    viewport_init(&view, width, r->tiles_y, IRON_W, IRON_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    assert(move_left_action(g, g->key_bindings[CONTROL_MOVE_LEFT]).type == ACTION_INTERACT);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/war-machine-shipment-office.bmp") == 0);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    assert(g->war_machine_progress == WAR_ORDERS);
    ironfang_change_level(g, 1);
    ironfang_change_level(g, 1);
    g->player.x = WAR_WINCH_X;
    g->player.y = WAR_WINCH_Y + 1;
    g->ironfang[3].cart_x = 60;
    g->ironfang[3].cart_y = 25;
    for (int y = 20; y <= 29; y++) {
        for (int x = 52; x < IRON_W; x++) {
            map_mark_explored(&g->map, x, y);
        }
    }
    viewport_center_on(&view, g->player.x, g->player.y);
    assert(move_left_action(g, g->key_bindings[CONTROL_MOVE_LEFT]).type == ACTION_INTERACT);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/war-machine-weapons-platform.bmp") == 0);
    int enemies = g->enemy_count;
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    assert(g->war_machine_progress == 3 && g->enemy_count == enemies);
    assert(game_combat_feedback_active(SDL_GetTicks()));
    SDL_Delay(170);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/war-machine-falling-weapons.bmp") == 0);
    SDL_Delay(360);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/war-machine-weapons-splash.bmp") == 0);
    SDL_Delay(180);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/war-machine-empty-gantry.bmp") == 0);
    ironfang_change_level(g, 1);
    g->player.x = WAR_SEAL_X;
    g->player.y = WAR_SEAL_Y + 1;
    map_mark_explored(&g->map, WAR_SEAL_X, WAR_SEAL_Y);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/war-machine-command-coffer.bmp") == 0);
    g->player.x = 51;
    g->player.y = 12;
    g->player.attack = 999;
    action_resolve_player(g, (Action){ACTION_MOVE, 51, 11});
    assert(g->ironfang_defeated);
    g->player.x = WAR_SEAL_X;
    g->player.y = WAR_SEAL_Y + 1;
    assert(war_machine_interact(g) && g->war_machine_quest_state == 2);
    QuestJournalScreen journal;
    quest_journal_init(&journal);
    renderer_begin_frame(r);
    quest_journal_draw(r, g, &journal);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/war-machine-journal.bmp") == 0);
    puts("PASS: Veyra's independent topics, Ironfang offer, shipment office, weapons gantry, falling crates, splash, command coffer and quest journal render at native scale.");
}

static void test_crown_quest_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    assert(r->crown_quest_texture);
    int w;
    int h;
    assert(SDL_QueryTexture(r->crown_quest_texture, NULL, NULL, &w, &h) == 0 && w == 144 && h == 48);
    game_init(g);
    game_enter_town3(g);
    game_enter_guild(g);
    g->player.x = EDDA_X;
    g->player.y = EDDA_Y + 1;
    Viewport view;
    int width = (r->screen_w - INFO_PANEL_W) / TILE_SIZE;
    viewport_init(&view, width, r->tiles_y, TOWN_W, TOWN_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/crown-quest-edda-guild.bmp") == 0);
    assert(crown_quest_talk(g) && game_quest_offer_active(g));
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/crown-quest-edda-offer.bmp") == 0);
    assert(game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0));
    giants_enter(g, 0);
    giants_change_level(g, 1);
    g->player.x = CROWN_INSCRIPTION_X;
    g->player.y = CROWN_INSCRIPTION_Y + 1;
    map_mark_explored(&g->map, CROWN_INSCRIPTION_X, CROWN_INSCRIPTION_Y);
    viewport_init(&view, width, r->tiles_y, GIANTS_W, GIANTS_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    assert(move_left_action(g, g->key_bindings[CONTROL_MOVE_LEFT]).type == ACTION_INTERACT);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/crown-quest-royal-inscription.bmp") == 0);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    giants_change_level(g, 1);
    g->player.x = 9;
    g->player.y = 23;
    assert(giants_interact(g) && map_is_walkable(&g->map, 8, 23));
    g->player.x = 6;
    for (int y = 20; y <= 26; y++) {
        for (int x = 2; x <= 11; x++) {
            map_mark_explored(&g->map, x, y);
        }
    }
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/crown-quest-seal-chamber.bmp") == 0);
    assert(giants_interact(g) && (g->crown_quest_progress & CROWN_SEAL));
    giants_change_level(g, 1);
    giants_change_level(g, 1);
    g->player.x = 9;
    g->player.y = 23;
    for (int y = 20; y <= 26; y++) {
        for (int x = 2; x <= 11; x++) {
            map_mark_explored(&g->map, x, y);
        }
    }
    viewport_center_on(&view, g->player.x, g->player.y);
    assert(!map_is_walkable(&g->map, 8, 23));
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/crown-quest-warded-vault.bmp") == 0);
    Enemy *boss = &g->enemies[0];
    g->player.x = boss->x - 1;
    g->player.y = boss->y;
    g->player.attack = 10000;
    action_resolve_player(g, (Action){ACTION_MOVE, boss->x, boss->y});
    assert(g->giants_crown_defeated);
    g->player.x = 9;
    g->player.y = 23;
    assert(giants_interact(g) && g->crown_vault_open);
    g->player.x = 6;
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/crown-quest-open-vault.bmp") == 0);
    assert(giants_interact(g) && g->crown_quest_state == 2);
    QuestJournalScreen journal;
    quest_journal_init(&journal);
    renderer_begin_frame(r);
    quest_journal_draw(r, g, &journal);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/crown-quest-journal.bmp") == 0);
    game_enter_town3(g);
    game_enter_guild(g);
    g->player.x = EDDA_X;
    g->player.y = EDDA_Y + 1;
    assert(crown_quest_talk(g) && g->crown_quest_state == 3 && g->crown_reward_state == 2);
    int shield = g->inventory_count - 1;
    int sword = g->inventory_count;
    g->inventory[g->inventory_count++] = item_make_long_sword();
    assert(game_equip_main_hand(g, sword) && game_equip_shield(g, shield));
    viewport_init(&view, width, r->tiles_y, TOWN_W, TOWN_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    g->dialogue_active = 0;
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/crown-quest-stoneguard-equipped.bmp") == 0);
    renderer_begin_frame(r);
    InventoryScreen inventory;
    inventory_init(&inventory);
    inventory.selected = shield;
    inventory_draw(r, g, &inventory);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/crown-quest-stoneguard-inventory.bmp") == 0);
    puts("PASS: Edda, royal inscription, seal chamber, warded/open vault, crown, journal, equipped Stoneguard and shield HP comparison render at native scale.");
}

static void test_spice_skiff_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    int width;
    int height;
    assert(r->spice_skiff_texture);
    assert(SDL_QueryTexture(r->spice_skiff_texture, NULL, NULL, &width, &height) == 0);
    assert(width == 156 && height == 72);
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    const TownBuilding *shop = town_life_building(LOCATION_DAWNMERE);
    g->player.x = shop->x + shop->w / 2;
    g->player.y = shop->y + shop->h;
    Viewport view;
    int play_width = (r->screen_w - INFO_PANEL_W) / TILE_SIZE;
    viewport_init(&view, play_width, r->tiles_y, TOWN_W, TOWN_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/spice-skiff-merchant-door.bmp") == 0);
    action_resolve_player(g, (Action){ACTION_MOVE, g->player.x, g->player.y - 1});
    assert(g->location == LOCATION_DAWNMERE_MERCHANT);
    g->player.x = RESIDENT_X;
    g->player.y = RESIDENT_Y + 1;
    assert(town_life_talk(g, 0) && game_quest_offer_active(g));
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/spice-skiff-corvin-offer.bmp") == 0);
    assert(game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0));
    g->dialogue_active = 0;
    mangrove_enter(g, 0);
    mangrove_change_level(g, 1);
    g->player.x = SPICE_WRECK_X + 1;
    g->player.y = SPICE_WRECK_Y;
    viewport_init(&view, play_width, r->tiles_y, MANGROVE_W, MANGROVE_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    assert(move_left_action(g, g->key_bindings[CONTROL_MOVE_LEFT]).type == ACTION_INTERACT);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/spice-skiff-stranded-skiff.bmp") == 0);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    assert(g->spice_skiff_progress == SPICE_MANIFEST);
    mangrove_change_level(g, 1);
    g->player.x = SPICE_CASKET_X;
    g->player.y = SPICE_CASKET_Y + 1;
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/spice-skiff-guarded-cargo.bmp") == 0);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    assert(g->spice_skiff_progress == SPICE_MANIFEST);
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].x < 20) {
            g->enemies[i].active = 0;
        }
    }
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
    assert(g->spice_skiff_quest_state == 2);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/spice-skiff-recovered-cargo.bmp") == 0);
    QuestJournalScreen journal;
    quest_journal_init(&journal);
    renderer_begin_frame(r);
    quest_journal_draw(r, g, &journal);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/spice-skiff-journal.bmp") == 0);
    puts("PASS: Corvin's quest offer, stranded skiff, root arch, guarded and recovered cargo, configured Interact key and journal render at native scale.");
}

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

// Screen pixel of a map tile under the area's zoomed camera, which shows fewer, larger tiles.
static void zoomed_screen(const GameState *g, const Viewport *view, int map_w, int map_h, int tx, int ty, int *x, int *y) {
    float zoom = game_camera_zoom(g);
    Viewport zoomed;
    viewport_init(&zoomed, (int)(view->tiles_x / zoom + 0.999f), (int)(view->tiles_y / zoom + 0.999f), map_w, map_h);
    if (zoomed.tiles_x > map_w) {
        zoomed.tiles_x = map_w;
    }
    if (zoomed.tiles_y > map_h) {
        zoomed.tiles_y = map_h;
    }
    viewport_center_on(&zoomed, g->player.x, g->player.y);
    *x = (int)(viewport_to_screen_x(&zoomed, tx) * TILE_SIZE * zoom);
    *y = (int)(viewport_to_screen_y(&zoomed, ty) * TILE_SIZE * zoom);
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


static void test_giants_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    assert(r->area_entrance_textures[TOWN_EXIT_GIANTS]);
    int width;
    int height;
    SDL_QueryTexture(r->area_entrance_textures[TOWN_EXIT_GIANTS], NULL, NULL, &width, &height);
    assert(width == 168 && height == 144);
    game_init(g);
    srand(7203);
    giants_enter(g, 0);
    SDL_RenderSetViewport(r->sdl, NULL);
    SDL_RenderSetClipRect(r->sdl, NULL);
    SDL_RenderSetScale(r->sdl, 1.0f, 1.0f);
    const EnemyType roster[] = {ENEMY_OGRE, ENEMY_CAVE_TROLL, ENEMY_HILL_GIANT, ENEMY_STONE_GIANT, ENEMY_SHARD_GOLEM, ENEMY_FROST_GIANT, ENEMY_LAST_JARL};
    for (int n = 0; n < 7; n++) {
        SDL_SetRenderDrawColor(r->sdl, 0, 0, 0, 255);
        SDL_RenderClear(r->sdl);
        draw_enemy(r, 10, 8, roster[n]);
        SDL_RenderPresent(r->sdl);
        int pixels = 0;
        for (int y = 8 * TILE_SIZE - 12; y < 9 * TILE_SIZE + 12; y++) {
            for (int x = 10 * TILE_SIZE - 12; x < 11 * TILE_SIZE + 12; x++) {
                Uint8 red;
                Uint8 green;
                Uint8 blue;
                SDL_GetRGB(rendered_pixel(surface, x, y), surface->format, &red, &green, &blue);
                if (red || green || blue) {
                    pixels++;
                    if (roster[n] != ENEMY_SHARD_GOLEM) {
                        assert(x >= 10 * TILE_SIZE && x < 11 * TILE_SIZE && y >= 8 * TILE_SIZE && y < 9 * TILE_SIZE);
                    }
                }
            }
        }
        assert(pixels > 100);
    }
    Viewport view;
    viewport_init(&view, r->tiles_x, r->tiles_y, GIANTS_W, GIANTS_H);
    for (int level = 1; level <= GIANTS_DEPTH; level++) {
        g->player.x = 32;
        g->player.y = 24;
        for (int y = 0; y < GIANTS_H; y++) {
            for (int x = 0; x < GIANTS_W; x++) {
                map_mark_explored(&g->map, x, y);
            }
        }
        viewport_center_on(&view, g->player.x, g->player.y);
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        if (level == 1) {
            assert(rendered_pixel(surface, 6, 6) != rendered_pixel(surface, 22, 28));
        }
        char path[100];
        snprintf(path, sizeof(path), "/tmp/giants-crown-stage-%d.bmp", level);
        assert(SDL_SaveBMP(surface, path) == 0);
        if (level < GIANTS_DEPTH) {
            giants_change_level(g, 1);
        }
    }
    approach_enter_region(g, GIANTS_APPROACH_ROUTE);
    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y + 1;
    viewport_init(&view, r->tiles_x, r->tiles_y, APPROACH_W, APPROACH_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/giants-crown-approach.bmp") == 0);
    g->location = LOCATION_CASTLE;
    map_generate_castle(&g->map, &g->player.x, &g->player.y);
    viewport_init(&view, TOWN_W, TOWN_H, TOWN_W, TOWN_H);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/giants-crown-castle-road.bmp") == 0);
    approach_enter_region(g, ASHEN_APPROACH_ROUTE);
    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y + 1;
    viewport_init(&view, r->tiles_x, r->tiles_y, APPROACH_W, APPROACH_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/ashen-rift-approach.bmp") == 0);
    printf("PASS: Giant's Peaks new enemies fit the native 24px scale; recurring creatures retain their original art; all stages and both entrances render.\n");
}

static void test_northroad_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    game_init(g);
    g->debug_passages_unlocked = 1;
    northroad_enter(g, 0, 0);
    assert(g->enemy_count == NORTH_ENEMIES);
    g->player.x = 32;
    g->player.y = 87;
    for (int y = 0; y < CROWN_NORTH_H; y++) {
        for (int x = 0; x < CROWN_NORTH_W; x++) {
            map_mark_explored(&g->map, x, y);
        }
    }
    Viewport view;
    viewport_init(&view, r->tiles_x, r->tiles_y, CROWN_NORTH_W, CROWN_NORTH_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/crown-north-renderer-test.bmp") == 0);
    const EnemyType roster[] = {ENEMY_GOBLIN_SCOUT, ENEMY_HOBGOBLIN_GUARD, ENEMY_GOBLIN_ARCHER,
        ENEMY_GOBLIN_WARCHIEF, ENEMY_GOBLIN_SHAMAN, ENEMY_BLIGHTED_WOLF};
    g->enemy_count = 1;
    Enemy *enemy = &g->enemies[0];
    *enemy = (Enemy){.x = 32, .y = 82, .hp = 50, .max_hp = 50};
    int x;
    int y;
    zoomed_screen(g, &view, CROWN_NORTH_W, CROWN_NORTH_H, enemy->x, enemy->y, &x, &y);
    for (int i = 0; i < 6; i++) {
        enemy->active = 0;
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        unsigned int ground = creature_pixels(surface, x, y);
        enemy->active = 1;
        enemy->type = roster[i];
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        assert(creature_pixels(surface, x, y) != ground);
    }
    printf("PASS: Crown Road North's six recurring enemy types render visibly on their road terrain at the zoomed camera scale.\n");
}

static void test_stonewake_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    assert(r->stonewake_enemy_texture && r->stonewake_crane_texture && r->stonewake_prop_texture && r->stonewake_workshop_texture);
    const EnemyType roster[5] = {ENEMY_OGRE, ENEMY_BLIND_STALKER, ENEMY_SHARD_GOLEM, ENEMY_STONE_GIANT, ENEMY_PRISM_SOVEREIGN};
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
        const int heights[5] = {24, 24, 32, 24, 48};
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
        int px;
        int py;
        zoomed_screen(g, &view, SWAMP_MAP_W, SWAMP_MAP_H, 18, 26, &px, &py);
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

static void clock_step(GameState *g, StormwatchClock *clock, Uint32 *now) {
    *now += STORM_WIND_STEP_MS;
    assert(update_stormwatch_clock(g, clock, *now, 0));
}

static void test_stormwatch_input(GameState *g) {
    game_init(g);
    stormwatch_enter(g, 0);
    g->enemy_count = 0;
    g->player.x = 48;
    g->player.y = 24;
    StormwatchClock clock = {0};
    Uint32 now = 100;
    assert(!update_stormwatch_clock(g, &clock, now, 0));
    clock_step(g, &clock, &now);
    clock_step(g, &clock, &now);
    assert(g->stormwatch[0].phase[0] == 1 && move_left_action(g, SDL_SCANCODE_A).type == ACTION_MOVE);
    // A single arrow step enters the exposed strip; no held/repeated key is needed.
    action_resolve_player(g, (Action){ACTION_MOVE, 48, 23});
    action_resolve_enemies(g);
    assert(g->player.x == 48 && g->player.y == 23 && g->stormwatch[0].phase[0] == 1);
    clock_step(g, &clock, &now);
    assert(g->stormwatch[0].phase[0] == 2);
    clock_step(g, &clock, &now);
    assert(g->stormwatch[0].phase[0] == 3);
    combat_feedback_clear();
    clock_step(g, &clock, &now);
    assert(g->player.x == 49 && g->player.y == 23);
    const CombatFeedbackEvent *event = combat_feedback_get(combat_feedback_count() - 1);
    assert(event->kind == FEEDBACK_PLAYER_PUSH);
    int x = 240;
    int y = 240;
    game_player_push_offset(g, event->created_at, &x, &y);
    assert(x == 216 && y == 240);
    x = 240;
    game_player_push_offset(g, event->created_at + PUSH_ANIMATION_MS / 2, &x, &y);
    assert(x == 228 && y == 240);
    x = 240;
    game_player_push_offset(g, event->created_at + PUSH_ANIMATION_MS, &x, &y);
    assert(x == 240 && y == 240 && g->player.x == 49);
    // Reach the next final warning without issuing any player actions.
    for (int step = 0; step < 5; step++) {
        clock_step(g, &clock, &now);
    }
    assert(move_left_action(g, SDL_SCANCODE_A).type == ACTION_INTERACT);
    assert(move_left_action(g, SDL_SCANCODE_LEFT).type == ACTION_MOVE);
    action_resolve_player(g, move_left_action(g, SDL_SCANCODE_A));
    action_resolve_enemies(g);
    assert(g->stormwatch[0].braced && g->stormwatch[0].phase[0] == 3);
    now += 999;
    assert(!update_stormwatch_clock(g, &clock, now, 0) && g->stormwatch[0].braced);
    now++;
    assert(update_stormwatch_clock(g, &clock, now, 0));
    assert(g->player.x == 49 && !g->stormwatch[0].braced);
    // Menus/focus loss discard wall-clock gaps and preserve a partial second.
    g->stormwatch[0].wind_elapsed_ms = 600;
    int phase = g->stormwatch[0].phase[0];
    now += 60000;
    assert(!update_stormwatch_clock(g, &clock, now, 1));
    assert(g->stormwatch[0].phase[0] == phase && g->stormwatch[0].wind_elapsed_ms == 600 && g->player.x == 49);
    now += 60000;
    assert(!update_stormwatch_clock(g, &clock, now, 0));
    now += 399;
    assert(!update_stormwatch_clock(g, &clock, now, 0) && g->stormwatch[0].wind_elapsed_ms == 999);
    now++;
    assert(update_stormwatch_clock(g, &clock, now, 0) && g->stormwatch[0].phase[0] == (phase + 1) % 6);
    stormwatch_change_level(g, 1);
    now += 60000;
    assert(!update_stormwatch_clock(g, &clock, now, 0) && !g->stormwatch[1].wind_elapsed_ms);
    // Use SDL's actual clock too: no key event and no enemy turn is issued here.
    stormwatch_enter(g, 0);
    g->enemy_count = 0;
    g->player.x = 48;
    g->player.y = 24;
    g->stormwatch[0].phase[0] = 1;
    clock = (StormwatchClock){0};
    Uint32 started = SDL_GetTicks();
    update_stormwatch_clock(g, &clock, started, 0);
    while (g->player.x == 48 && SDL_GetTicks() - started < 5 * STORM_WIND_STEP_MS) {
        update_stormwatch_clock(g, &clock, SDL_GetTicks(), 0);
        SDL_Delay(10);
    }
    assert(g->player.x == 49 && g->player.y == 24 && !g->enemy_count);
    printf("PASS: a single arrow step followed by idle frames triggers a visible gust; bracing survives until the gust; menus pause the exact clock without catching up.\n");
    printf("PASS: SDL wall-clock frames push a standing character without any key event or enemy turn.\n");
}

static void test_stormwatch_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    game_init(g);
    stormwatch_enter(g, 0);
    g->player.x = 24;
    g->player.y = 24;
    g->enemy_count = 1;
    Enemy *enemy = &g->enemies[0];
    *enemy = (Enemy){.x = 25, .y = 24, .hp = 40, .max_hp = 40};
    for (int y = 0; y < STORM_H; y++) {
        for (int x = 0; x < STORM_W; x++) {
            map_mark_explored(&g->map, x, y);
        }
    }
    Viewport view;
    viewport_init(&view, r->tiles_x < STORM_W ? r->tiles_x : STORM_W, r->tiles_y < STORM_H ? r->tiles_y : STORM_H, STORM_W, STORM_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    int x;
    int y;
    zoomed_screen(g, &view, STORM_W, STORM_H, enemy->x, enemy->y, &x, &y);
    for (int type = ENEMY_HARPY_RAIDER; type <= ENEMY_TEMPEST_MATRIARCH; type++) {
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
    assert(SDL_SaveBMP(surface, "/tmp/stormwatch-renderer-test.bmp") == 0);
    g->debug_passages_unlocked = 1;
    while (g->level < STORM_DEPTH) {
        stormwatch_change_level(g, 1);
    }
    assert(!g->stormwatch_defeated && g->enemies[g->enemy_count - 1].type == ENEMY_TEMPEST_MATRIARCH);
    stormwatch_change_level(g, 1);
    assert(g->location == LOCATION_APPROACH && g->approach_route == 18 && !g->stormwatch_defeated);
    printf("PASS: five Stormwatch enemies render visibly at the zoomed camera scale; debug opens final departure without defeating its boss.\n");
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
    int x;
    int y;
    zoomed_screen(g, &view, MANGROVE_W, MANGROVE_H, enemy->x, enemy->y, &x, &y);
    const EnemyType marsh_types[] = {ENEMY_LIZARDFOLK_SCOUT, ENEMY_REED_SPEARHUNTER, ENEMY_MARSH_SPITTER, ENEMY_GIANT_CRAB, ENEMY_LIZARDFOLK_SCALEGUARD, ENEMY_SSZARETH};
    for (int i = 0; i < (int)(sizeof(marsh_types) / sizeof(marsh_types[0])); i++) {
        enemy->active = 0;
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        unsigned int ground = creature_pixels(surface, x, y);
        enemy->active = 1;
        enemy->type = marsh_types[i];
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

static void test_regional_projectiles(Renderer *r, GameState *g, SDL_Surface *surface) {
    const Location areas[] = {LOCATION_ELDERWOOD, LOCATION_MANGROVES, LOCATION_STORMWATCH, LOCATION_IRONFANG,
        LOCATION_GIANTS_CROWN, LOCATION_CROWNROAD_NORTH, LOCATION_CATACOMBS, LOCATION_FROSTFELL, LOCATION_DRAGONSPINE};
    const int widths[] = {ELDERWOOD_W, MANGROVE_W, STORM_W, IRON_W, GIANTS_W, CROWN_NORTH_W, CATACOMBS_W, SWAMP_MAP_W, SWAMP_MAP_W};
    const int heights[] = {ELDERWOOD_H, MANGROVE_H, STORM_H, IRON_H, GIANTS_H, CROWN_NORTH_H, CATACOMBS_H, SWAMP_MAP_H, SWAMP_MAP_H};
    size_t bytes = (size_t)surface->pitch * surface->h;
    unsigned char *baseline = malloc(bytes);
    assert(baseline);
    for (int area = 0; area < 9; area++) {
        game_init(g);
        g->location = areas[area];
        g->level = 1;
        g->player.x = widths[area] - 4;
        g->player.y = heights[area] - 4;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                g->map.tiles[y][x] = TILE_ELDERWOOD_FLOOR;
                map_mark_explored(&g->map, x, y);
            }
        }
        g->enemy_count = 1;
        g->enemies[0] = (Enemy){.type = ENEMY_DARK_ELF_ARCHER, .name = "Dark Elf", .active = 1,
            .x = g->player.x - 5, .y = g->player.y, .hp = 30, .max_hp = 30, .attack = 10, .move_timer = 1};
        EnemyProjectiles shots = {.count = 1};
        shots.shots[0] = (EnemyProjectile){ENEMY_DARK_ELF_ARCHER, g->enemies[0].x, g->enemies[0].y, g->player.x, g->player.y};
        if (area == 0) {
            // Exercise real Elderwood firing; the other fixtures isolate each
            // renderer's camera without invoking a different area's enemy AI.
            action_resolve_enemies_with_projectiles(g, &shots);
        }
        assert(shots.count == 1 && shots.shots[0].start_x == g->enemies[0].x && shots.shots[0].target_x == g->player.x);
        Viewport base;
        viewport_init(&base, (r->screen_w - INFO_PANEL_W) / TILE_SIZE, r->tiles_y, MAP_W, MAP_H);
        viewport_center_on(&base, g->player.x, g->player.y);
        // Zoomed areas show fewer, larger tiles; screen pixels scale by the zoom.
        float zoom = game_camera_zoom(g);
        int tiles_x = (int)(base.tiles_x / zoom + 0.999f);
        int tiles_y = (int)(base.tiles_y / zoom + 0.999f);
        Viewport expected;
        viewport_init(&expected, tiles_x < widths[area] ? tiles_x : widths[area],
            tiles_y < heights[area] ? tiles_y : heights[area], widths[area], heights[area]);
        viewport_center_on(&expected, g->player.x, g->player.y);
        renderer_begin_frame(r);
        game_draw(r, g, &base);
        renderer_end_frame(r);
        memcpy(baseline, surface->pixels, bytes);
        SDL_Texture *scene = SDL_CreateTextureFromSurface(r->sdl, surface);
        assert(scene);
        SDL_SetTextureBlendMode(scene, SDL_BLENDMODE_NONE);
        for (int frame = 0; frame < 3; frame++) {
            SDL_RenderCopy(r->sdl, scene, NULL, NULL);
            Uint32 elapsed = frame * ENEMY_PROJECTILE_TRAVEL_MS / 2;
            game_draw_enemy_projectiles(r, g, &shots, &base, elapsed);
            renderer_end_frame(r);
            int cx = (int)((((2 - frame) * shots.shots[0].start_x + frame * shots.shots[0].target_x) * TILE_SIZE / 2 - expected.cam_x * TILE_SIZE + TILE_SIZE / 2) * zoom);
            int cy = (int)(((g->player.y - expected.cam_y) * TILE_SIZE + TILE_SIZE / 2) * zoom);
            int changed = 0;
            for (int y = 0; y < surface->h; y++) {
                for (int x = 0; x < surface->w; x++) {
                    Uint32 before = ((Uint32 *)(baseline + y * surface->pitch))[x];
                    if (rendered_pixel(surface, x, y) != before) {
                        if (abs(x - cx) > TILE_SIZE * zoom || abs(y - cy) > TILE_SIZE * zoom) {
                            fprintf(stderr, "Regional projectile %d frame %d: pixel %d,%d outside impact %d,%d\n", area, frame, x, y, cx, cy);
                        }
                        assert(abs(x - cx) <= TILE_SIZE * zoom && abs(y - cy) <= TILE_SIZE * zoom);
                        changed++;
                    }
                }
            }
            assert(changed > 4);
            if (area == 0) {
                char path[80];
                snprintf(path, sizeof(path), "/tmp/elderwood-arrow-%d.bmp", frame);
                assert(SDL_SaveBMP(surface, path) == 0);
            }
        }
        SDL_DestroyTexture(scene);
    }
    free(baseline);
    puts("PASS: arrows launch from their shooter and land on the player with the rendered regional camera in nine areas.");
}

static void test_elderwood_roots(Renderer *r, GameState *g, SDL_Surface *surface) {
    game_init(g);
    elderwood_enter(g);
    g->enemy_count = 2;
    g->enemies[0] = (Enemy){.type = ENEMY_TREEFOLK, .name = "Treefolk", .active = 1, .x = 32, .y = 32,
        .hp = 92, .max_hp = 92, .attack = 15, .defense = 8, .experience = 90};
    g->enemies[1] = (Enemy){.type = ENEMY_BLIGHTED_WOLF, .name = "Blighted Wolf", .active = 1, .x = 36, .y = 32,
        .hp = 26, .max_hp = 26, .frozen_turns = 10};
    g->player.x = 32;
    g->player.y = 28;
    for (int y = 24; y < 36; y++) {
        for (int x = 25; x < 40; x++) {
            g->map.tiles[y][x] = TILE_ELDERWOOD_FLOOR;
        }
    }
    Viewport view;
    viewport_init(&view, (r->screen_w - INFO_PANEL_W) / TILE_SIZE, r->tiles_y, MAP_W, MAP_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    action_resolve_enemies(g);
    for (int frame = 0; frame < 3; frame++) {
        if (frame == 1) {
            action_resolve_enemies(g);
            action_resolve_enemies(g);
            SDL_Delay(180);
        } else if (frame == 2) {
            SDL_Delay(270);
        }
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        char path[80];
        snprintf(path, sizeof(path), "/tmp/elderwood-roots-%d.bmp", frame);
        assert(SDL_SaveBMP(surface, path) == 0);
    }
    puts("PASS: Elderwood renders attackable Treefolk footing, a recurring wolf, warning roots, eruption and fading roots.");
}

static void test_coast_exit_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    for (int level = 1; level <= COAST_DEPTH; level++) {
        game_init(g);
        g->location = LOCATION_COAST;
        g->level = level;
        map_generate_coast(&g->map, level);
        g->enemy_count = 0;
        for (int forward = 0; forward < 2; forward++) {
            int x, y;
            assert(coast_exit_point(&g->map, forward, &x, &y));
            g->player.x = forward ? g->map.stairs_down_x : g->map.stairs_up_x;
            g->player.y = forward ? g->map.stairs_down_y : g->map.stairs_up_y;
            map_mark_explored(&g->map, x, y);
            Viewport view;
            viewport_init(&view, (r->screen_w - INFO_PANEL_W) / TILE_SIZE, r->tiles_y, MAP_W, MAP_H);
            viewport_center_on(&view, g->player.x, g->player.y);
            renderer_begin_frame(r);
            game_draw(r, g, &view);
            renderer_end_frame(r);
            int text_pixels = 0;
            for (int py = 64; py < view.tiles_y * TILE_SIZE; py++) {
                for (int px = 0; px < view.tiles_x * TILE_SIZE; px++) {
                    Uint8 red, green, blue;
                    SDL_GetRGB(rendered_pixel(surface, px, py), surface->format, &red, &green, &blue);
                    text_pixels += forward ? red == 115 && green == 245 && blue == 225 : red == 245 && green == 220 && blue == 155;
                }
            }
            assert(text_pixels > 80);
            Uint8 red, green, blue;
            SDL_GetRGB(rendered_pixel(surface, 6 + x / 2 - 1, 6 + y / 2 - 1), surface->format, &red, &green, &blue);
            assert(forward ? red == 115 && green == 245 && blue == 225 : red == 245 && green == 220 && blue == 155);
            char path[80];
            snprintf(path, sizeof(path), "/tmp/coast-exit-%d-%d.bmp", level, forward);
            assert(SDL_SaveBMP(surface, path) == 0);
        }
    }
    puts("PASS: all ten coast boundary transitions show destination labels and discovered minimap markers at all four edges.");
}

static void test_dawnmere_dialogue(Renderer *r, GameState *g, SDL_Surface *surface) {
    for (int state = 0; state < 5; state++) {
        game_init(g);
        game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
        assert(g->map.tiles[DAIN_TOWN_Y][DAIN_TOWN_X] == TILE_TOWN_PATH);
        if (state < 4) {
            g->player.x = SEREN_X;
            g->player.y = SEREN_Y + 1;
            g->stormbell_quest_state = state;
            assert(controls_action_for_key(g->key_bindings, SDL_SCANCODE_T) == CONTROL_TALK);
            assert(stormbell_talk(g) && g->dialogue_active);
        } else {
            const TownBuilding *b = town_life_building(LOCATION_ALEHOUSE);
            town_life_enter(g, b->x + b->w / 2, b->y + b->h - 1);
            g->player.x = DAIN_ALEHOUSE_X;
            g->player.y = DAIN_ALEHOUSE_Y + 1;
            game_talk_to_dain(g);
            assert(game_quest_offer_active(g) && strstr(g->dialogue_text, "Dawnmere"));
        }
        Viewport view;
        viewport_init(&view, r->tiles_x, r->tiles_y, TOWN_W, TOWN_H);
        viewport_center_on(&view, g->player.x, g->player.y);
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        int pixels = 0;
        for (int y = 0; y < r->tiles_y * TILE_SIZE; y++) {
            for (int x = 0; x < r->screen_w - INFO_PANEL_W; x++) {
                Uint8 red, green, blue;
                SDL_GetRGB(rendered_pixel(surface, x, y), surface->format, &red, &green, &blue);
                pixels += red == 236 && green == 224 && blue == 190;
            }
        }
        assert(pixels > 1000);
        char path[80];
        snprintf(path, sizeof(path), "/tmp/dawnmere-dialogue-%d.bmp", state);
        assert(SDL_SaveBMP(surface, path) == 0);
        if (state == 0 || state == 4) {
            assert(game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0));
            assert(!game_quest_offer_active(g) && (state == 0 ? g->stormbell_quest_state : g->dain_quest_state) == 1);
        }
    }
    puts("PASS: Seren's offer, progress, reward and completed dialogue and Dain's relocated offer visibly render in Dawnmere.");
}

static void test_one_way_escape(Renderer *r, GameState *g, SDL_Surface *surface) {
    const Location towns[] = {LOCATION_TOWN, LOCATION_TOWN2, LOCATION_TOWN3, LOCATION_TOWN4, LOCATION_DAWNMERE};
    for (int n = 0; n < 6; n++) {
        game_init(g);
        game_enter_town_gate(g, towns[n % 5], 0, 12);
        if (n == 5) {
            castle_enter(g, 1);
        } else {
            game_enter_temple(g);
        }
        g->player.known_spells[0] = spell_make_return_to_town();
        g->player.known_spell_count = 1;
        g->player.equipped_spell = 0;
        action_resolve_player(g, (Action){ACTION_CAST_SPELL, 0, 0});
        if (n == 5) {
            assert(g->castle_prompt == 2);
            castle_prompt_key(g, SDL_SCANCODE_RETURN, 0);
            assert(g->location == LOCATION_CASTLE && g->player.x == CROWNROAD_X && g->player.y == 11);
        } else {
            assert(g->location == towns[n]);
        }
        assert(!g->portal_active);
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                assert(g->map.tiles[y][x] != TILE_PORTAL);
            }
        }
        Viewport view;
        viewport_init(&view, r->tiles_x, r->tiles_y, TOWN_W, TOWN_H);
        viewport_center_on(&view, g->player.x, g->player.y);
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        char path[100];
        snprintf(path, sizeof(path), "/tmp/one-way-escape-%d.bmp", n);
        assert(SDL_SaveBMP(surface, path) == 0);
    }
    puts("PASS: temple escape renders in each last-visited town without a return portal; the castle exception renders beside its entrance in Castle Grounds.");
}

static void test_rowan_dialogue(Renderer *r, GameState *g, SDL_Surface *surface) {
    for (int mode = 0; mode < 4; mode++) {
        game_init(g);
        approach_enter_region(g, 3);
        g->player.x = COAST_ROWAN_X + 1;
        g->player.y = COAST_ROWAN_Y;
        if (mode > 0) {
            g->defeated_bosses |= 1 << LOCATION_COAST;
        }
        if (mode == 2) {
            while (g->inventory_count < MAX_INVENTORY) {
                g->inventory[g->inventory_count++] = item_make_health_potion();
            }
        }
        if (mode == 3) {
            g->island_travel_unlocked = 1;
        }
        coastal_refresh_docks(g);
        assert(coastal_approach(g) && g->map.tiles[COAST_ROWAN_Y][COAST_ROWAN_X] == TILE_NPC_ROWAN);
        assert(map_is_walkable(&g->map, g->player.x, g->player.y));
        game_talk_to_rowan(g);
        assert(g->dialogue_active && strcmp(g->dialogue_speaker, "Captain Rowan") == 0);
        assert(g->dialogue_x == COAST_ROWAN_X && g->dialogue_y == COAST_ROWAN_Y);
        if (mode == 0) {
            assert(strstr(g->dialogue_text, "Drowned Queen") && !game_has_treasure_map(g) && !game_can_sail_to_island(g));
            assert(save_game(g, 99290) && load_game(g, 99290));
            game_talk_to_rowan(g);
        } else if (mode == 1) {
            assert(game_has_treasure_map(g) && game_can_sail_to_island(g));
            int inventory = g->inventory_count;
            game_talk_to_rowan(g);
            assert(g->inventory_count == inventory);
        } else if (mode == 2) {
            assert(strstr(g->dialogue_text, "Make room") && !game_has_treasure_map(g));
        } else {
            assert(strstr(g->dialogue_text, "route is charted") && !game_has_treasure_map(g));
        }
        Viewport view;
        viewport_init(&view, r->tiles_x, r->tiles_y, APPROACH_W, APPROACH_H);
        viewport_center_on(&view, g->player.x, g->player.y);
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        int panel_pixels = 0;
        for (int y = 0; y < r->tiles_y * TILE_SIZE; y++) {
            for (int x = 0; x < r->screen_w - INFO_PANEL_W; x++) {
                Uint8 red;
                Uint8 green;
                Uint8 blue;
                SDL_GetRGB(rendered_pixel(surface, x, y), surface->format, &red, &green, &blue);
                panel_pixels += red == 236 && green == 224 && blue == 190;
            }
        }
        assert(panel_pixels > 1000);
        char path[100];
        snprintf(path, sizeof(path), "/tmp/rowan-docks-%d.bmp", mode);
        assert(SDL_SaveBMP(surface, path) == 0);
    }
    puts("PASS: Rowan's dialogue visibly renders at the relocated docks before and after the Queen, after loading, with a full pack, and after charting the route.");
}

static void test_town_gate_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    assert(r->town_gate_texture);
    int w;
    int h;
    assert(SDL_QueryTexture(r->town_gate_texture, NULL, NULL, &w, &h) == 0 && w == 768 && h == 1152);
    const Location towns[] = {LOCATION_TOWN, LOCATION_TOWN2, LOCATION_TOWN3, LOCATION_TOWN4, LOCATION_DAWNMERE, LOCATION_CASTLE};
    const char *names[] = {"oakhaven", "stillbury", "rosemoor", "ridgeshire", "dawnmere", "castle"};
    const int dx[] = {0, -1, 0, 1};
    const int dy[] = {1, 0, -1, 0};
    Map *before = malloc(sizeof(*before));
    assert(before);
    renderer_begin_frame(r);
    for (int y = 0; y < 14; y++) {
        for (int x = 0; x < 22; x++) {
            draw_town_floor(r, x, y);
            if (x == 12 || y == 6) {
                draw_town_path(r, x, y);
            }
        }
    }
    draw_shop_blacksmith(r, 5, 2);
    draw_town_gate(r, 12, 0, LOCATION_TOWN, 0, 1);
    draw_town_gate(r, 21, 6, LOCATION_TOWN, -1, 0);
    draw_town_gate(r, 12, 13, LOCATION_TOWN, 0, -1);
    draw_town_gate(r, 0, 6, LOCATION_TOWN, 1, 0);
    Item sword = item_make_long_sword();
    Item shield = item_make_tower_shield();
    draw_player_at(r, 12 * TILE_SIZE, 7 * TILE_SIZE, CLASS_WARRIOR, &sword, &shield, NULL, NULL, 0, -1);
    SDL_RenderPresent(r->sdl);
    assert(SDL_SaveBMP(surface, "/tmp/town-gates-native-scale.bmp") == 0);
    for (int n = 0; n < 6; n++) {
        // Every opening must preserve the complete 24px road tile underneath.
        for (int side = 0; side < 4; side++) {
            SDL_SetRenderDrawColor(r->sdl, 1, 233, 177, 255);
            SDL_RenderClear(r->sdl);
            draw_town_gate(r, 12, 10, towns[n], dx[side], dy[side]);
            SDL_RenderPresent(r->sdl);
            Uint32 background = rendered_pixel(surface, 0, 0);
            for (int y = 10 * TILE_SIZE; y < 11 * TILE_SIZE; y++) {
                for (int x = 12 * TILE_SIZE; x < 13 * TILE_SIZE; x++) {
                    assert(rendered_pixel(surface, x, y) == background);
                }
            }
            int changed = 0;
            for (int y = 24; y < 456; y++) {
                for (int x = 120; x < 456; x++) {
                    changed += rendered_pixel(surface, x, y) != background;
                }
            }
            assert(changed > 2000);
        }
        game_init(g);
        g->location = towns[n];
        switch (towns[n]) {
            case LOCATION_TOWN: map_generate_town(&g->map, &g->player.x, &g->player.y); break;
            case LOCATION_TOWN2: map_generate_town2(&g->map, &g->player.x, &g->player.y); break;
            case LOCATION_TOWN3: map_generate_town3(&g->map, &g->player.x, &g->player.y); break;
            case LOCATION_TOWN4: map_generate_town4(&g->map, &g->player.x, &g->player.y); break;
            case LOCATION_DAWNMERE: map_generate_dawnmere(&g->map, &g->player.x, &g->player.y); break;
            case LOCATION_CASTLE: map_generate_castle(&g->map, &g->player.x, &g->player.y); break;
            default: assert(0); break;
        }
        g->enemy_count = 0;
        g->player.x = 20;
        g->player.y = 4;
        memcpy(before, &g->map, sizeof(*before));
        Viewport view;
        viewport_init(&view, TOWN_W, TOWN_H, TOWN_W, TOWN_H);
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        assert(memcmp(before->tiles, g->map.tiles, sizeof(before->tiles)) == 0);
        char path[100];
        snprintf(path, sizeof(path), "/tmp/town-gates-%s.bmp", names[n]);
        assert(SDL_SaveBMP(surface, path) == 0);
    }
    for (int route = 13; route <= 14; route++) {
        game_init(g);
        approach_enter_region(g, route);
        Viewport view;
        viewport_init(&view, (r->screen_w - INFO_PANEL_W) / TILE_SIZE, r->tiles_y, APPROACH_W, APPROACH_H);
        viewport_center_on(&view, g->player.x, g->player.y);
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        char path[100];
        snprintf(path, sizeof(path), "/tmp/town-gates-castle-approach-%d.bmp", route);
        assert(SDL_SaveBMP(surface, path) == 0);
    }
    free(before);
    puts("PASS: all 24 town gate views preserve a full transparent road tile; five towns and Castle Grounds render upright gates without changing their maps.");
}

static void test_alehouse_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    assert(r->local_building_textures[4] && r->alehouse_cask_texture);
    int width;
    int height;
    SDL_QueryTexture(r->local_building_textures[4], NULL, NULL, &width, &height);
    assert(width == 168 && height == 144);
    SDL_QueryTexture(r->alehouse_cask_texture, NULL, NULL, &width, &height);
    assert(width == 2 * TILE_SIZE && height == TILE_SIZE);
    game_init(g);
    game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
    Viewport view;
    viewport_init(&view, TOWN_W, TOWN_H, TOWN_W, TOWN_H);
    Map *before = malloc(sizeof(*before));
    assert(before);
    *before = g->map;
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(memcmp(before->tiles, g->map.tiles, sizeof(before->tiles)) == 0);
    assert(SDL_SaveBMP(surface, "/tmp/alehouse-dawnmere.bmp") == 0);
    // A character's transparent corners must retain the plaza's stone pixels.
    int sample_x = (DAIN_TOWN_X * TILE_SIZE + 1) * (r->screen_w - INFO_PANEL_W) / (TOWN_W * TILE_SIZE);
    int sample_y = (DAIN_TOWN_Y * TILE_SIZE + 1) * r->tiles_y / TOWN_H;
    Uint32 paving = rendered_pixel(surface, sample_x, sample_y);
    const TileType visitors[] = {TILE_NPC_DAIN, TILE_NPC_SEREN, TILE_NPC_TAMSIN, TILE_NPC_ALDRIC, TILE_NPC_ROWAN, TILE_NPC_CAIN};
    for (int i = 0; i < 6; i++) {
        g->map.tiles[DAIN_TOWN_Y][DAIN_TOWN_X] = visitors[i];
        renderer_begin_frame(r);
        game_draw(r, g, &view);
        renderer_end_frame(r);
        assert(rendered_pixel(surface, sample_x, sample_y) == paving);
    }
    g->map = *before;
    const TownBuilding *b = town_life_building(LOCATION_ALEHOUSE);
    g->player.x = b->x + b->w / 2;
    g->player.y = b->y + b->h;
    action_resolve_player(g, (Action){ACTION_MOVE, g->player.x, g->player.y - 1});
    assert(g->location == LOCATION_ALEHOUSE);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/alehouse-interior.bmp") == 0);
    g->player.x = DAIN_ALEHOUSE_X;
    g->player.y = DAIN_ALEHOUSE_Y + 1;
    assert(town_life_talk(g, 0) && game_quest_offer_active(g));
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/alehouse-dain-dialogue.bmp") == 0);
    game_handle_quest_offer_key(g, SDL_SCANCODE_Y, 0);
    assert(g->dain_quest_state == 1);
    free(before);
    puts("PASS: Alehouse, cask shelves and Dain's quest dialogue render; six NPCs preserve the town's paving.");
}

static void test_ironfang_renderer(Renderer *r, GameState *g, SDL_Surface *surface) {
    assert(r->ironfang_boss_texture && r->area_entrance_textures[TOWN_EXIT_IRONFANG]);
    int width;
    int height;
    SDL_QueryTexture(r->ironfang_boss_texture, NULL, NULL, &width, &height);
    assert(width == TILE_SIZE && height == TILE_SIZE);
    SDL_QueryTexture(r->area_entrance_textures[TOWN_EXIT_IRONFANG], NULL, NULL, &width, &height);
    assert(width == 168 && height == 144);
    game_init(g);
    ironfang_enter(g, 0);
    g->enemy_count = 0;
    g->player.x = 9;
    g->player.y = 25;
    Action interact = move_left_action(g, g->key_bindings[CONTROL_MOVE_LEFT]);
    assert(interact.type == ACTION_INTERACT);
    action_resolve_player(g, interact);
    assert(g->ironfang[0].aboard && g->player.x == 10);
    Action left = move_left_action(g, SDL_SCANCODE_LEFT);
    assert(left.type == ACTION_MOVE);
    action_resolve_player(g, (Action){ACTION_MOVE, 11, 25});
    assert(g->player.x == 12 && g->ironfang[0].cart_x == 12 && g->player.last_dx == 1 && g->player.last_dy == 0);
    game_init(g);
    ironfang_enter(g, 0);
    const EnemyType roster[] = {ENEMY_GOBLIN_SCOUT, ENEMY_GOBLIN_ARCHER, ENEMY_GOBLIN_WARCHIEF, ENEMY_GIANT_SPIDER, ENEMY_CAVE_TROLL, ENEMY_HOBGOBLIN_GUARD, ENEMY_GOBLIN_SHAMAN, ENEMY_GOBLIN_MINE_BOSS};
    for (int n = 0; n < 8; n++) {
        SDL_SetRenderDrawColor(r->sdl, 0, 0, 0, 255);
        SDL_RenderClear(r->sdl);
        draw_enemy(r, 10, 8, roster[n]);
        SDL_RenderPresent(r->sdl);
        int pixels = 0;
        for (int y = 8 * TILE_SIZE; y < 9 * TILE_SIZE; y++) {
            for (int x = 10 * TILE_SIZE; x < 11 * TILE_SIZE; x++) {
                Uint8 red, green, blue;
                SDL_GetRGB(rendered_pixel(surface, x, y), surface->format, &red, &green, &blue);
                pixels += red || green || blue;
            }
        }
        assert(pixels > 65);
    }
    g->location = LOCATION_TOWN4;
    map_generate_town4(&g->map, &g->player.x, &g->player.y);
    TownExitStyle style;
    assert(town_entry_style(g, (Action){ACTION_MOVE, TOWN_W - 1, 12}, &style) && style == TOWN_EXIT_ROAD);
    g->debug_passages_unlocked = 1;
    ironfang_enter(g, 0);
    game_refresh_debug_passages(g);
    assert(g->map.tiles[25][40] == TILE_IRON_RAIL && g->map.tiles[8][8] == TILE_IRON_FLOOR && !g->ironfang_defeated);
    puts("PASS: Ironfang's eight enemy sprites render at native scale; bound Interact boards a cart, arrows drive it, Ridgeshire uses its road gate, and debug repairs optional passages.");
}

static void test_boss_camera_lean(Renderer *r, GameState *g, SDL_Surface *surface) {
    game_init(g);
    g->location = LOCATION_DUNGEON;
    g->level = 1;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            g->map.tiles[y][x] = TILE_FLOOR;
            map_mark_explored(&g->map, x, y);
        }
    }
    g->player.x = 40;
    g->player.y = 20;
    g->enemy_count = 1;
    Viewport base;
    viewport_init(&base, (r->screen_w - INFO_PANEL_W) / TILE_SIZE, r->tiles_y, MAP_W, MAP_H);
    viewport_center_on(&base, g->player.x, g->player.y);
    float zoom = game_camera_zoom(g);
    int rows = (int)(r->tiles_y / zoom + 0.999f);
    int cols = (int)(base.tiles_x / zoom + 0.999f);
    assert(rows == 18);
    size_t bytes = (size_t)surface->pitch * surface->h;
    unsigned char *first = malloc(bytes);
    assert(first);
    // Fourteen rows below is outside the centred view; only the lean can show the boss.
    for (int frame = 0; frame < 2; frame++) {
        game_spawn_enemy(g, &g->enemies[0], ENEMY_MOUNTAIN_GOBLIN_KING, g->player.x + frame, g->player.y + 14);
        assert(g->enemies[0].active && g->enemies[0].is_boss);
        renderer_begin_frame(r);
        game_draw(r, g, &base);
        renderer_end_frame(r);
        if (frame == 0) {
            memcpy(first, surface->pixels, bytes);
        }
    }
    // The lean keeps the player two rows from the top, which puts the boss on row 16.
    int tile = (int)(TILE_SIZE * zoom);
    int boss_x = cols / 2;
    int changed = 0;
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            if (((Uint32 *)(first + y * surface->pitch))[x] != rendered_pixel(surface, x, y)) {
                assert(y >= 15 * tile && y < 17 * tile && x >= (boss_x - 1) * tile && x < (boss_x + 3) * tile);
                changed++;
            }
        }
    }
    assert(changed > 0);
    free(first);
    puts("PASS: the zoomed camera leans toward a boss fourteen rows away, keeping the player two rows from the edge.");
}

static void roster_scene(Renderer *r, GameState *g, SDL_Surface *surface, const char *path) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map_mark_explored(&g->map, x, y);
        }
    }
    Viewport view;
    viewport_init(&view, r->tiles_x, r->tiles_y, MAP_W, MAP_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(r);
    game_draw(r, g, &view);
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, path) == 0);
}

static void test_approved_roster_renderer(Renderer *r, GameState *g, SDL_Surface *surface, const char *project) {
    const EnemyType types[] = {ENEMY_DARK_ELF_ARCHER, ENEMY_DARK_ELF_RANGER, ENEMY_FOREST_TROLL, ENEMY_GOBLIN_WARCHIEF, ENEMY_HOBGOBLIN_GUARD, ENEMY_GIANT_SPIDER, ENEMY_OGRE, ENEMY_STONE_GIANT, ENEMY_ROYAL_GUARD, ENEMY_VAMPIRE, ENEMY_SUN_PRIEST, ENEMY_LUNAR_EFFIGY};
    const char *labels[] = {"Dark Elf Archer", "Dark Elf Ranger", "Forest Troll", "Goblin Warchief", "Hobgoblin Guard", "Giant Spider", "Ogre", "Stone Giant", "Royal Guard", "Vampire", "Sun Priest", "Lunar Effigy"};
    renderer_begin_frame(r);
    for (int i = 0; i < 12; i++) {
        int x = 3 + (i % 3) * 10;
        int y = 3 + (i / 3) * 5;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                draw_forest_floor(r, x + dx, y + dy, x + dx, y + dy);
            }
        }
        draw_enemy(r, x, y, types[i]);
        renderer_draw_text(r, labels[i], x * TILE_SIZE - 10, (y + 2) * TILE_SIZE, (SDL_Color){239, 235, 214, 255}, r->font_tiny);
    }
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, "/tmp/approved-enemy-sprites.bmp") == 0);

    game_init(g);
    castle_enter(g, 2);
    g->player.x = 45;
    g->player.y = 47;
    roster_scene(r, g, surface, "/tmp/approved-castle-vampire.bmp");
    castle_enter(g, 3);
    g->player.x = 49;
    g->player.y = 34;
    roster_scene(r, g, surface, "/tmp/approved-castle-guard.bmp");

    game_init(g);
    game_enter_temple(g);
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
        g->enemies[i].alignment_dormant = 0;
    }
    for (int y = 0; y < TEMPLE_H; y++) {
        for (int x = 0; x < TEMPLE_W; x++) {
            if (g->map.tiles[y][x] == TILE_TEMPLE_DORMANT_SENTINEL) {
                g->map.tiles[y][x] = TILE_TEMPLE_FLOOR;
            }
        }
    }
    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y;
    action_resolve_player(g, (Action){ACTION_ASCEND, 0, 0});
    assert(g->level == 2);
    g->player.x = 53;
    g->player.y = 26;
    roster_scene(r, g, surface, "/tmp/approved-temple-sun.bmp");
    int switched = 0;
    for (int y = 0; y < TEMPLE_H && !switched; y++) {
        for (int x = 0; x < TEMPLE_W && !switched; x++) {
            if (g->map.tiles[y][x] == TILE_TEMPLE_ALTAR) {
                g->player.x = x;
                g->player.y = y;
                assert(game_interact_temple(g));
                switched = 1;
            }
        }
    }
    assert(g->temple_alignment);
    g->player.x = 40;
    g->player.y = 8;
    roster_scene(r, g, surface, "/tmp/approved-temple-moon.bmp");

    char source_path[1024];
    snprintf(source_path, sizeof(source_path), "%s/saves/savegame_1.json", project);
    FILE *source = fopen(source_path, "rb");
    if (source) {
        mkdir("saves", 0700);
        FILE *copy = fopen("saves/savegame_99224.json", "wb");
        assert(copy);
        char buffer[8192];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0) {
            assert(fwrite(buffer, 1, bytes, copy) == bytes);
        }
        fclose(copy);
        fclose(source);
        assert(load_game(g, 99224));
        if (!g->forest_cache[3].valid) {
            remove("saves/savegame_99224.json");
            memset(g, 0, sizeof(*g));
            return;
        }
        g->location = LOCATION_FOREST;
        g->level = 4;
        g->map = g->forest_cache[3].map;
        g->enemy_count = g->forest_cache[3].enemy_count;
        memcpy(g->enemies, g->forest_cache[3].enemies, g->enemy_count * sizeof(Enemy));
        for (int i = 0; i < g->enemy_count; i++) {
            if (g->enemies[i].active && g->enemies[i].type == ENEMY_DARK_ELF_RANGER) {
                g->player.x = g->enemies[i].x;
                g->player.y = g->enemies[i].y + 1;
                break;
            }
        }
        roster_scene(r, g, surface, "/tmp/approved-existing-save-forest.bmp");
        remove("saves/savegame_99224.json");
    }
    memset(g, 0, sizeof(*g));
    puts("PASS: approved enemy sprites, castle replacements, temple alignments and a migrated existing forest save render through the game renderer.");
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
    const TownExitStyle entrances[] = {TOWN_EXIT_COAST, TOWN_EXIT_DESERT, TOWN_EXIT_GLASSDEEP, TOWN_EXIT_FROST, TOWN_EXIT_DRAGONSPINE, TOWN_EXIT_ASHEN, TOWN_EXIT_CATACOMBS, TOWN_EXIT_STORMWATCH};
    for (int i = 0; i < (int)(sizeof(entrances) / sizeof(entrances[0])); i++) {
        int width;
        int height;
        assert(renderer.area_entrance_textures[entrances[i]]);
        assert(SDL_QueryTexture(renderer.area_entrance_textures[entrances[i]], NULL, NULL, &width, &height) == 0);
        assert(width == 168 && height == (entrances[i] == TOWN_EXIT_DRAGONSPINE || entrances[i] == TOWN_EXIT_ASHEN ? 216 : 144));
    }
    int mountain_w;
    int mountain_h;
    assert(SDL_QueryTexture(renderer.mountain_entrance_texture, NULL, NULL, &mountain_w, &mountain_h) == 0);
    assert(mountain_w == 168 && mountain_h == APPROACH_MOUNTAIN_ROWS * TILE_SIZE);
    Viewport viewport;
    SlotSelect slots = {0};
    LandingScreen landing = {0};
    GameScreen screen = SCREEN_LOAD_SLOT;
    GameState *game = calloc(1, sizeof(*game));
    assert(game);
    test_approved_roster_renderer(&renderer, game, surface, project);
    test_boss_gate_renderer(&renderer, game, surface);
    test_container_renderer(&renderer, game, surface);
    test_stonewake_renderer(&renderer, game, surface);
    test_saltwind_renderer(&renderer, game, surface);
    test_rowan_dialogue(&renderer, game, surface);
    test_regional_projectiles(&renderer, game, surface);
    test_elderwood_roots(&renderer, game, surface);
    test_coast_exit_renderer(&renderer, game, surface);
    test_dawnmere_dialogue(&renderer, game, surface);
    test_one_way_escape(&renderer, game, surface);
    test_stormwatch_renderer(&renderer, game, surface);
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
            assert(game->map.tiles[y][game->player.x + 1] == TILE_TOWN_PATH || game->map.tiles[y][game->player.x + 1] == TILE_ROAD_GATE_OPEN);
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
    assert(game->map.tiles[NORTH_FOREST_BARRIER_Y][NORTH_ROAD_X] == TILE_ROAD_GATE_CLOSED);
    assert(save_game(game, 1));
    handle_slot_result(SLOT_SELECTED, &slots, 0, &screen, game, &renderer, &viewport, &landing);
    assert(game->debug_passages_unlocked && !game->elderwood_defeated && !game->defeated_bosses);
    assert(game->map.tiles[NORTH_FOREST_BARRIER_Y][NORTH_ROAD_X] == TILE_ROAD_GATE_OPEN);
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
    test_stormwatch_input(game);
    test_ironfang_renderer(&renderer, game, surface);
    test_boss_camera_lean(&renderer, game, surface);
    test_northroad_renderer(&renderer, game, surface);
    test_giants_renderer(&renderer, game, surface);
    test_heartseed_renderer(&renderer, game, surface);
    test_stormbell_renderer(&renderer, game, surface);
    test_war_machine_renderer(&renderer, game, surface);
    test_crown_quest_renderer(&renderer, game, surface);
    test_spice_skiff_renderer(&renderer, game, surface);
    test_alehouse_renderer(&renderer, game, surface);
    test_town_gate_renderer(&renderer, game, surface);
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
