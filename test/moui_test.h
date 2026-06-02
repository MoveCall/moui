#ifndef MOUI_TEST_H
#define MOUI_TEST_H

#include <stdio.h>
#include <string.h>

static int _test_pass = 0;
static int _test_fail = 0;

#define TEST(name) static void test_##name(void)

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        printf("  FAIL: %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        _test_fail++; return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf("  FAIL: %s:%d: %s == %s (%d != %d)\n", __FILE__, __LINE__, #a, #b, (int)(a), (int)(b)); \
        _test_fail++; return; \
    } \
} while(0)

#define ASSERT_NE(a, b) do { \
    if ((a) == (b)) { \
        printf("  FAIL: %s:%d: %s != %s (both %d)\n", __FILE__, __LINE__, #a, #b, (int)(a)); \
        _test_fail++; return; \
    } \
} while(0)

#define RUN_TEST(name) do { \
    printf("  %-40s", #name); \
    int _before = _test_fail; \
    test_##name(); \
    if (_test_fail == _before) { printf("PASS\n"); _test_pass++; } \
} while(0)

#define TEST_REPORT() do { \
    printf("\n%d passed, %d failed\n", _test_pass, _test_fail); \
    return _test_fail > 0 ? 1 : 0; \
} while(0)

#endif
