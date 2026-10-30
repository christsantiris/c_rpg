#include "game/game.h"
#include "game/town_life.h"
#include "renderer/town_life_renderer.h"
#include "renderer/shop_renderer.h"
#include "renderer/renderer.h"
#include "renderer/game_renderer.h"
#include "renderer/info_panel.h"
#include "renderer/inventory_renderer.h"
#include "renderer/item_icons.h"
#include "renderer/sprites.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void capture(Renderer *r, SDL_Surface *surface, const char *path) {
    renderer_end_frame(r);
    assert(SDL_SaveBMP(surface, path) == 0);
}

static void downgrade_shop_save(void) {
    FILE *file = fopen("saves/savegame_99230.json", "rb");
    assert(file);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = calloc((size_t)size + 1, 1);
    assert(text && fread(text, 1, (size_t)size, file) == (size_t)size);
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    assert(root);
    free(text);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 227);
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99230.json", "wb");
    assert(file && text && fputs(text, file) >= 0);
    fclose(file);
    free(text);
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
    assert(r.local_building_textures[5]);
    renderer_begin_frame(&r);
    draw_shop_blacksmith(&r, 3, 3);
    TownBuilding display = *town_life_building(LOCATION_GOLDSMITH);
    display.x = 11;
    display.y = 3;
    Viewport comparison;
    viewport_init(&comparison, r.tiles_x, r.tiles_y, MAP_W, MAP_H);
    comparison.cam_x = 0;
    comparison.cam_y = 0;
    town_life_draw_building(&r, &comparison, &display);
    capture(&r, surface, "/tmp/equipment-goldsmith-comparison.bmp");
    assert(chdir(project) == 0);
    source = fopen("saves/savegame_1.json", "rb");
    assert(chdir(temporary) == 0);
    if (source) {
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
        const TownBuilding *goldsmith = town_life_building(LOCATION_GOLDSMITH);
        if (g->location != LOCATION_TOWN) {
            game_init(g);
        }
        assert(g->map.tiles[goldsmith->y + goldsmith->h - 1][goldsmith->x + goldsmith->w / 2] == TILE_LOCAL_DOOR);
        viewport_center_on(&view, 20, 15);
        renderer_begin_frame(&r);
        game_draw(&r, g, &view);
        capture(&r, surface, "/tmp/equipment-goldsmith-existing-town.bmp");
        remove("saves/savegame_99227.json");
    }
    town_life_generate(&g->map, LOCATION_GOLDSMITH, &g->player.x, &g->player.y);
    g->location = LOCATION_GOLDSMITH;
    viewport_center_on(&view, g->player.x, g->player.y);
    renderer_begin_frame(&r);
    game_draw(&r, g, &view);
    capture(&r, surface, "/tmp/equipment-goldsmith-interior.bmp");
    ShopScreen shop;
    shop_init(&shop, SHOP_TYPE_GOLDSMITH, 0, g->player.player_class);
    renderer_begin_frame(&r);
    shop_draw(&r, g, &shop);
    capture(&r, surface, "/tmp/equipment-goldsmith-shop.bmp");
    g->inventory_count = 2;
    g->inventory[0] = item_make_health_potion();
    g->inventory[1] = item_make_mana_potion();
    assert(game_set_quick_potion(g, 1));
    controls_assign(g->key_bindings, CONTROL_DRINK_POTION, SDL_SCANCODE_G);
    inventory_init(&inv);
    inv.selected = 1;
    renderer_begin_frame(&r);
    inventory_draw(&r, g, &inv);
    capture(&r, surface, "/tmp/equipment-quick-potion-inventory.bmp");
    renderer_begin_frame(&r);
    game_draw(&r, g, &view);
    capture(&r, surface, "/tmp/equipment-quick-potion-sidebar.bmp");
    renderer_on_resize(&r, 1280, 480);
    renderer_begin_frame(&r);
    game_draw(&r, g, &view);
    capture(&r, surface, "/tmp/equipment-quick-potion-short.bmp");
    renderer_on_resize(&r, 1280, 720);
    renderer_begin_frame(&r);
    draw_shop_blacksmith(&r, 3, 3);
    const Location workshops[] = {LOCATION_COBBLER, LOCATION_BELT_MAKER};
    for (int i = 0; i < 2; i++) {
        assert(r.local_building_textures[6 + i]);
        TownBuilding building = *town_life_building(workshops[i]);
        building.x = 11 + i * 8;
        building.y = 3;
        town_life_draw_building(&r, &comparison, &building);
    }
    for (int family = ACCESSORY_FAMILY_GAUNTLETS; family <= ACCESSORY_FAMILY_BOOTS; family++) {
        for (int tier = 1; tier <= 3; tier++) {
            Item item = item_make_accessory(family, tier);
            draw_icon_item(&r, 90 + family * 32, 200 + tier * 32, &item);
        }
    }
    capture(&r, surface, "/tmp/equipment-accessory-art.bmp");
    for (int i = 0; i < 2; i++) {
        const TownBuilding *building = town_life_building(workshops[i]);
        game_init(g);
        game_enter_town_gate(g, building->town, 20, 12);
        for (int y = building->y; y < building->y + building->h; y++) {
            for (int x = building->x; x < building->x + building->w; x++) {
                g->map.tiles[y][x] = TILE_TOWN_FLOOR;
            }
        }
        g->player.x = building->x;
        g->player.y = building->y;
        assert(save_game(g, 99230));
        downgrade_shop_save();
        assert(load_game(g, 99230));
        assert(g->player.x == building->x + building->w / 2 && g->player.y == building->y + building->h);
        viewport_center_on(&view, 20, 15);
        renderer_begin_frame(&r);
        game_draw(&r, g, &view);
        char capture_path[100];
        snprintf(capture_path, sizeof(capture_path), "/tmp/equipment-workshop-%d-town.bmp", i);
        capture(&r, surface, capture_path);
        town_life_enter(g, building->x + building->w / 2, building->y + building->h - 1);
        assert(g->location == workshops[i]);
        viewport_center_on(&view, g->player.x, g->player.y);
        renderer_begin_frame(&r);
        game_draw(&r, g, &view);
        snprintf(capture_path, sizeof(capture_path), "/tmp/equipment-workshop-%d-interior.bmp", i);
        capture(&r, surface, capture_path);
        shop_init(&shop, i == 0 ? SHOP_TYPE_COBBLER : SHOP_TYPE_BELT_MAKER,
            (1 << LOCATION_DUNGEON) | (1 << LOCATION_FOREST), g->player.player_class);
        renderer_begin_frame(&r);
        shop_draw(&r, g, &shop);
        snprintf(capture_path, sizeof(capture_path), "/tmp/equipment-workshop-%d-shop.bmp", i);
        capture(&r, surface, capture_path);
    }
    remove("saves/savegame_99230.json");
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
