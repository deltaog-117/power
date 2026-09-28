/**
 * @file test_menu_nav.c
 * @brief Unit and property tests for the menu navigation helpers.
 *
 * Build and run from the project root:
 *   gcc -Wall -Wextra -std=gnu99 -Isrc tests/unit/power/test_menu_nav.c \
 *       src/power/model/menu_nav.c -o /tmp/test_menu_nav && /tmp/test_menu_nav
 */

#include "power/model/menu_nav.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void test_step_wraps_both_ways(void) {
    assert(menu_nav_step(5, 6, 1) == 0);
    assert(menu_nav_step(0, 6, -1) == 5);
    assert(menu_nav_step(-1, 6, 1) == 0);
    assert(menu_nav_step(2, 6, 0) == 2);
}

static void test_step_rejects_empty_menu(void) {
    assert(menu_nav_step(0, 0, 1) == -1);
    assert(menu_nav_step(0, -3, 1) == -1);
}

static void test_digit_maps_to_existing_rows_only(void) {
    assert(menu_nav_row_from_digit('1', 6) == 0);
    assert(menu_nav_row_from_digit('6', 6) == 5);
    assert(menu_nav_row_from_digit('7', 6) == -1);
    assert(menu_nav_row_from_digit('0', 6) == -1);
    assert(menu_nav_row_from_digit('a', 6) == -1);
}

static void test_row_at_edges(void) {
    assert(menu_nav_row_at(9, 10, 20, 3) == -1);  /* above first row */
    assert(menu_nav_row_at(10, 10, 20, 3) == 0);
    assert(menu_nav_row_at(29, 10, 20, 3) == 0);
    assert(menu_nav_row_at(30, 10, 20, 3) == 1);
    assert(menu_nav_row_at(69, 10, 20, 3) == 2);
    assert(menu_nav_row_at(70, 10, 20, 3) == -1); /* below last row */
    assert(menu_nav_row_at(15, 10, 0, 3) == -1);
}

/* Property: for any valid input the result is a valid row, and stepping by a
 * whole number of laps returns to the same row. */
static void test_step_properties(void) {
    srand(117);
    for (int i = 0; i < 1000000; i++) {
        int count = 1 + rand() % 12;
        int selected = rand() % count;
        int delta = rand() % 2001 - 1000;
        int next = menu_nav_step(selected, count, delta);
        assert(next >= 0 && next < count);
        assert(menu_nav_step(selected, count, count) == selected);
        assert(menu_nav_step(next, count, -delta) == selected);
    }
}

int main(void) {
    test_step_wraps_both_ways();
    test_step_rejects_empty_menu();
    test_digit_maps_to_existing_rows_only();
    test_row_at_edges();
    test_step_properties();
    printf("menu_nav: all tests passed\n");
    return 0;
}
