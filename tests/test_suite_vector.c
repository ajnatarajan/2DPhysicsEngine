#include "vector.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>

void test_vec_zero() {
    assert(VEC_ZERO.x == 0.0);
    assert(VEC_ZERO.y == 0.0);
}

void test_vec_add() {
    assert(vec_equal(vec_add(VEC_ZERO, VEC_ZERO), VEC_ZERO));
    assert(vec_equal(vec_add(VEC_ZERO, (Vector){1, 2}), (Vector){1, 2}));
    assert(vec_equal(vec_add((Vector){1, 5}, (Vector){2, 10}), (Vector){3, 15}));
    assert(vec_equal(vec_add((Vector){-1.5, -1.5}, (Vector){2.5, 2.5}), (Vector){1, 1}));
}

void test_vec_subtract() {
    assert(vec_equal(vec_subtract(VEC_ZERO, VEC_ZERO), VEC_ZERO));
    assert(vec_equal(vec_subtract(VEC_ZERO, (Vector){1, 2}), (Vector){-1, -2}));
    assert(vec_equal(vec_subtract((Vector){3, 15}, (Vector){2, 10}), (Vector){1, 5}));
    assert(vec_equal(vec_subtract((Vector){1.5, 1.5}, (Vector){-2.5, -2.5}), (Vector){4, 4}));
    assert(vec_equal(vec_subtract((Vector){-1, -2}, (Vector){3, 4}), (Vector){-4, -6}));
}

void test_vec_negate() {
    assert(vec_equal(vec_negate(VEC_ZERO), VEC_ZERO));
    assert(vec_equal(vec_negate((Vector){-5, 6}), (Vector){5, -6}));
    assert(vec_equal(vec_negate((Vector){2, -3}), (Vector){-2, 3}));
}

void test_vec_multiply() {
    assert(vec_equal(vec_multiply(0, (Vector){5, 5}), VEC_ZERO));
    assert(vec_equal(vec_multiply(1, (Vector){5, 7}), (Vector){5, 7}));
    assert(vec_equal(vec_multiply(1, (Vector){5, 7}), (Vector){5, 7}));
    assert(vec_equal(vec_multiply(10, (Vector){2, 3}), (Vector){20, 30}));
    assert(vec_equal(vec_multiply(10, (Vector){-2, -3}), (Vector){-20, -30}));
    assert(vec_equal(vec_multiply(-3, (Vector){7, 5}), (Vector){-21, -15}));
    assert(vec_equal(vec_multiply(0.5, (Vector){-2, 3}), (Vector){-1, 1.5}));
}

void test_vec_dot() {
    assert(vec_dot(VEC_ZERO, (Vector){1, 2}) == 0);
    assert(vec_dot((Vector){1, 2}, (Vector){3, 4}) == 11);
    assert(vec_dot((Vector){-5, 3}, (Vector){2, 7}) == 11);
}

void test_vec_cross() {
    assert(vec_cross(VEC_ZERO, (Vector){1, 2}) == 0);
    assert(vec_cross((Vector){1, 2}, (Vector){3, 4}) == -2);
    assert(vec_cross((Vector){-5, 3}, (Vector){2, 7}) == -41);
}

void test_vec_rotate() {
    // No rotation
    assert(vec_isclose(vec_rotate((Vector){5, 7}, 0), (Vector){5, 7}));
    // 90-degree rotation
    assert(vec_isclose(vec_rotate((Vector){5, 7}, 0.5 * M_PI), (Vector){-7, 5}));
    // 180-degree rotation
    assert(vec_isclose(vec_rotate((Vector){5, 7}, M_PI), (Vector){-5, -7}));
    // 270-degree rotation
    assert(vec_isclose(vec_rotate((Vector){5, 7}, 1.5 * M_PI), (Vector){7, -5}));
    // 3-4-5 triangle
    assert(vec_isclose(vec_rotate((Vector){5, 0}, acos(4.0 / 5.0)), (Vector){4, 3}));
    // Rotate (0, 0)
    assert(vec_isclose(vec_rotate(VEC_ZERO, 1.0), VEC_ZERO));
}


int main(int argc, char *argv[]) {
    // Run all tests if there are no command-line arguments
    bool all_tests = argc == 1;
    // Read test name from file
    char testname[100];
    if (!all_tests) {
        read_testname(argv[1], testname, sizeof(testname));
    }

    DO_TEST(test_vec_zero)
    DO_TEST(test_vec_add)
    DO_TEST(test_vec_subtract)
    DO_TEST(test_vec_negate)
    DO_TEST(test_vec_multiply)
    DO_TEST(test_vec_dot)
    DO_TEST(test_vec_cross)
    DO_TEST(test_vec_rotate)

    puts("vector_test PASS");

    return 0;
}
