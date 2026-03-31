#include "test_utils.h"

int tests_run    = 0;
int tests_passed = 0;

void test_movement(void);
void test_map(void);
void test_map_tiles(void);
void test_viewport(void);
void test_dungeon(void);
void test_stairs_locked(void);

int main(void) {
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
    test_stairs_locked();
    printf("\n");
    REPORT();
}