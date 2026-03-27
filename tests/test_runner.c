#include "test_utils.h"

int tests_run    = 0;
int tests_passed = 0;

void test_movement(void);
void test_map(void);
void test_map_tiles(void);
void test_viewport(void);

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
    REPORT();
}