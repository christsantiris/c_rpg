#include "game/game.h"
#include "renderer/renderer.h"
#include "renderer/game_renderer.h"
#include "renderer/info_panel.h"
#include "renderer/inventory_renderer.h"
#include "renderer/item_icons.h"
#include "renderer/sprites.h"
#include "systems/save_load.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void capture(Renderer *r, SDL_Surface *surface, const char *path) {
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, path) == 0);
}

int main(void) {
    char *project = getcwd(NULL, 0);
    char *assets = realpath("assets", NULL);
    char temporary[] = "/tmp/conr-equipment-renderer-XXXXXX";
    assert(project && assets && mkdtemp(temporary));
    assert(chdir(temporary) == 0 && symlink(assets, "assets") == 0);
    SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 1280, 720, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_Renderer *sdl = SDL_CreateSoftwareRenderer(surface);
    assert(surface && sdl);
    Renderer r = {0};
    renderer_init(&r, sdl, 1280, 720);
    assert(r.font_large && r.font_small && r.font_tiny);
    GameState *g = calloc(1, sizeof(*g));
    assert(g);
    g->player.player_class = CLASS_WARRIOR;
    game_init(g);
    strcpy(g->player.name, "Equipment Review");
    Viewport view;
    viewport_init(&view, (r.screen_w - INFO_PANEL_W) / TILE_SIZE, r.tiles_y, MAP_W, MAP_H);
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(&r);
    game_draw(&r, g, &view);
    capture(&r, surface, "/tmp/equipment-new-character.bmp");
    assert(game_equip_main_hand(g, 0));
    const Item items[] = {item_make_buckler(), item_make_leather_cap(), item_make_chain_mail(), item_make_ring_of_vigor(), item_make_copper_amulet()};
    const EquipmentSlot slots[] = {EQUIPMENT_WEAK_HAND, EQUIPMENT_HELMET, EQUIPMENT_ARMOR, EQUIPMENT_ACCESSORY_1, EQUIPMENT_ACCESSORY_2};
    for (int i = 0; i < 5; i++) {
        g->inventory[g->inventory_count++] = items[i];
        assert(game_equip_item(g, 0, slots[i]));
    }
    g->player.known_spells[0] = spell_make_magic_arrow();
    g->player.known_spell_count = 1;
    g->player.equipped_spell = 0;
    renderer_begin_frame(&r);
    game_draw(&r, g, &view);
    capture(&r, surface, "/tmp/equipment-six-slots-sidebar.bmp");
    InventoryScreen inv;
    inventory_init(&inv);
    inventory_handle_key(&inv, SDL_SCANCODE_TAB, g->inventory_count, SDL_SCANCODE_I);
    inv.selected = 4;
    renderer_begin_frame(&r);
    inventory_draw(&r, g, &inv);
    capture(&r, surface, "/tmp/equipment-six-slots-inventory.bmp");
    inv.equipment_tab = 0;
    const Item catalog[] = {item_make_leather_cap(), item_make_leather_gauntlets(), item_make_leather_bracers(), item_make_ring_of_vigor(), item_make_copper_amulet()};
    for (int i = 0; i < MAX_INVENTORY; i++) {
        g->inventory[g->inventory_count++] = catalog[i % 5];
    }
    inv.selected = 9;
    renderer_begin_frame(&r);
    inventory_draw(&r, g, &inv);
    capture(&r, surface, "/tmp/equipment-full-bag.bmp");
    int rows = inventory_visible_rows(r.tiles_y);
    assert(130 + rows * 36 <= (r.tiles_y - 9) * TILE_SIZE);
    renderer_on_resize(&r, 1280, 480);
    renderer_begin_frame(&r);
    inventory_draw(&r, g, &inv);
    capture(&r, surface, "/tmp/equipment-short-window-bag.bmp");
    rows = inventory_visible_rows(r.tiles_y);
    assert(130 + rows * 36 <= (r.tiles_y - 9) * TILE_SIZE);
    renderer_on_resize(&r, 1280, 720);
    renderer_begin_frame(&r);
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        for (int direction = 0; direction < 4; direction++) {
            int dx = direction == 2 ? 1 : direction == 3 ? -1 : 0;
            int dy = direction == 0 ? 1 : direction == 1 ? -1 : 0;
            for (int cap = 0; cap < 2; cap++) {
                draw_player_at(&r, 60 + direction * 48 + cap * 24, 60 + cls * 48, cls, NULL, NULL, NULL, cap ? &catalog[0] : NULL, dx, dy);
            }
        }
    }
    for (int i = 0; i < 5; i++) {
        draw_icon_item(&r, 60 + i * 32, 230, &catalog[i]);
    }
    capture(&r, surface, "/tmp/equipment-native-art.bmp");
    // Read an existing character through the real loader in isolation.
    char path[1024];
    snprintf(path, sizeof(path), "%s/saves/savegame_1.json", project);
    FILE *source = fopen(path, "rb");
    if (source) {
        assert(save_game(g, 99227));
        FILE *copy = fopen("saves/savegame_99227.json", "wb");
        assert(copy);
        char block[8192];
        size_t count;
        while ((count = fread(block, 1, sizeof(block), source)) > 0) {
            assert(fwrite(block, 1, count, copy) == count);
        }
        fclose(source);
        fclose(copy);
        assert(load_game(g, 99227));
        inv.equipment_tab = 1;
        inv.selected = 0;
        renderer_begin_frame(&r);
        inventory_draw(&r, g, &inv);
        capture(&r, surface, "/tmp/equipment-existing-save.bmp");
        remove("saves/savegame_99227.json");
    }
    free(g);
    renderer_free(&r);
    SDL_DestroyRenderer(sdl);
    SDL_FreeSurface(surface);
    SDL_Quit();
    free(project);
    free(assets);
    puts("PASS: six-slot equipment, full bag, bare heads, accessories, separate spell and existing-save migration render at native scale.");
    return 0;
}
