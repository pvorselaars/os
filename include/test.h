#ifndef TEST_H
#define TEST_H

/* Test counters: defined in one translation unit (tests/test_state.c) */
extern unsigned long tests;
extern unsigned long tests_passed;
extern unsigned long tests_failed;

#define TEST_CASE(name) \
    arch_debug_printf("Testing: %s\n", name);

#define TEST_ASSERT_EQUAL(expected, actual) \
    { \
        if ((expected) != (actual)) { \
            arch_debug_printf("FAIL: %s != %s\n", #expected, #actual); \
            arch_debug_printf("      0x%0lx != 0x%0lx\n", (unsigned long)(expected), (unsigned long)(actual)); \
            tests_failed++; \
        } else { \
            tests_passed++; \
        } \
        tests++; \
    } 

#define TEST_REPORT() \
    { \
        arch_debug_printf("Test report: %lu tests run, %lu passed, %lu failed\n", tests, tests_passed, tests_failed); \
    }

#endif