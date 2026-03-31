#ifndef TEST_UTILS_HEADER_H
#define TEST_UTILS_HEADER_H

#include <stdio.h>

extern int tests_run;
extern int tests_passed;

#define ASSERT(msg, condition) do {         \
    tests_run++;                            \
    if (condition) {                        \
        tests_passed++;                     \
        printf("  PASS: %s\n", msg);        \
    } else {                                \
        printf("  FAIL: %s\n", msg);        \
    }                                       \
} while(0)

#define REPORT() do {                                           \
    printf("\n%d/%d tests passed\n", tests_passed, tests_run); \
    return tests_passed == tests_run ? 0 : 1;                  \
} while(0)

#endif