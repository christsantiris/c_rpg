#include "test_utils.h"
#include "../src/renderer/viewport.h"
#include "../src/game/map.h"

void test_viewport(void) {
    printf("Viewport tests:\n");

    Viewport v;
    viewport_init(&v, 53, 30, MAP_W, MAP_H);

    // Camera centering
    viewport_center_on(&v, 100, 50);
    ASSERT("cam_x centers on player", v.cam_x == 100 - 53/2);
    ASSERT("cam_y centers on player", v.cam_y == 50  - 30/2);

    // Camera clamping - top-left
    viewport_center_on(&v, 0, 0);
    ASSERT("cam_x clamps to 0 at left edge", v.cam_x == 0);
    ASSERT("cam_y clamps to 0 at top edge",  v.cam_y == 0);

    // Camera clamping - bottom-right
    viewport_center_on(&v, MAP_W - 1, MAP_H - 1);
    ASSERT("cam_x clamps at right edge", v.cam_x == MAP_W - 53);
    ASSERT("cam_y clamps at bottom edge", v.cam_y == MAP_H - 30);

    // World to screen conversion
    viewport_center_on(&v, 100, 50);
    int sx = viewport_to_screen_x(&v, 100);
    int sy = viewport_to_screen_y(&v, 50);
    ASSERT("player world x converts to screen center x", sx == 53/2);
    ASSERT("player world y converts to screen center y", sy == 30/2);

    // Visibility
    viewport_center_on(&v, 100, 50);
    ASSERT("player tile is visible",        viewport_is_visible(&v, 100, 50) == 1);
    ASSERT("far off-screen tile not visible", viewport_is_visible(&v, 0, 0)  == 0);

    // Resize
    viewport_on_resize(&v, 80, 45);
    ASSERT("tiles_x updates on resize", v.tiles_x == 80);
    ASSERT("tiles_y updates on resize", v.tiles_y == 45);
}