#include "test_utils.h"

int tests_run    = 0;
int tests_passed = 0;

void test_movement(void);
void test_map(void);
void test_map_tiles(void);
void test_viewport(void);
void test_dungeon(void);
void test_stairs_locked(void);
void test_town_tiles(void);
void test_town_map(void);
void test_town_spawn(void);
void test_leveling(void);
void test_items(void);
void test_classes(void);
void test_level_cache_cleared(void);
void test_return_to_town(void);

int main(void) {
    // ASSERT("sanity check true",  1 == 1);
    // ASSERT("sanity check false", 1 == 0);
    printf("=== CONR Test Suite ===\n\n");
    test_movement();
    printf("\n");
    test_map();
    printf("\n");
    test_map_tiles();
    printf("\n");
    test_viewport();
    printf("\n");
    test_dungeon();
    printf("\n");
    test_stairs_locked();
    printf("\n");
    test_town_tiles();
    printf("\n");
    test_town_map();
    printf("\n");
    test_town_spawn();
    printf("\n");
    test_level_cache_cleared();
    printf("\n");
    test_return_to_town();
    printf("\n");
    test_leveling();
    printf("\n");
    test_items();
    printf("\n");
    test_classes();
    printf("\n");
    REPORT();
}