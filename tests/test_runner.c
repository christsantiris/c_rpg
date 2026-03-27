#include "test_utils.h"

void test_movement(void);
void test_map(void);

int main(void) {
    printf("=== CONR Test Suite ===\n\n");
    test_movement();
    printf("\n");
    test_map();
    printf("\n");
    REPORT();
}