#ifndef TEST_UTILS_HEADER_H
#define TEST_UTILS_HEADER_H

#include <stdio.h>

extern int tests_run;
extern int tests_passed;

#define ANSI_GREEN "\033[32m"
#define ANSI_RED   "\033[31m"
#define ANSI_RESET "\033[0m"

#define ASSERT(msg, condition) do {                                          \
    tests_run++;                                                             \
    if (condition) {                                                         \
        tests_passed++;                                                      \
        printf(ANSI_GREEN "  PASS: %s\n" ANSI_RESET, msg);                  \
    } else {                                                                 \
        printf(ANSI_RED   "  FAIL: %s\n" ANSI_RESET, msg);                  \
    }                                                                        \
} while(0)

#define REPORT() do {                                                        \
    if (tests_passed == tests_run) {                                         \
        printf(ANSI_GREEN "\n%d/%d tests passed\n" ANSI_RESET,              \
            tests_passed, tests_run);                                        \
    } else {                                                                 \
        printf(ANSI_RED "\n%d/%d tests passed\n" ANSI_RESET,                \
            tests_passed, tests_run);                                        \
    }                                                                        \
} while(0)

#endif