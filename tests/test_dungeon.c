#include "test_utils.h"
#include "../src/game/map.h"
#include <stdlib.h>
#include <time.h>

void test_dungeon(void) {
    printf("Dungeon generation tests:\n");

    srand((unsigned)time(NULL));

    // Run generation multiple times to catch randomness bugs
    for (int run = 0; run < 5; run++) {
        Map m;
        map_generate(&m, 1);

        ASSERT("at least min rooms generated", m.room_count >= MIN_ROOMS);
        ASSERT("at most max rooms generated",  m.room_count <= MAX_ROOMS);

        // Stairs exist
        ASSERT("stairs up tile is set",
            m.tiles[m.stairs_up_y][m.stairs_up_x] == TILE_STAIRS_UP);
        ASSERT("stairs down tile is set",
            m.tiles[m.stairs_down_y][m.stairs_down_x] == TILE_STAIRS_DOWN);

        // Stairs are not on the same tile
        ASSERT("stairs up and down are not on same tile",
            !(m.stairs_up_x == m.stairs_down_x &&
              m.stairs_up_y == m.stairs_down_y));

        // Stairs are within map bounds
        ASSERT("stairs up within bounds",
            m.stairs_up_x >= 0 && m.stairs_up_x < MAP_W &&
            m.stairs_up_y >= 0 && m.stairs_up_y < MAP_H);
        ASSERT("stairs down within bounds",
            m.stairs_down_x >= 0 && m.stairs_down_x < MAP_W &&
            m.stairs_down_y >= 0 && m.stairs_down_y < MAP_H);
    }
}