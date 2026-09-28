/**
 * @file test_menu_confirm.c
 * @brief Unit and property tests for the menu confirmation helpers.
 *
 * Build and run from the project root:
 *   gcc -Wall -Wextra -std=gnu99 -Isrc tests/unit/power/test_menu_confirm.c \
 *       src/power/model/menu_confirm.c -o /tmp/test_menu_confirm && /tmp/test_menu_confirm
 */

#include "power/model/menu_confirm.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_destructive_set_is_exact(void) {
    assert(menu_command_is_destructive("poweroff"));
    assert(menu_command_is_destructive("reboot"));
    assert(menu_command_is_destructive("logout"));
    assert(!menu_command_is_destructive("lock"));
    assert(!menu_command_is_destructive("suspend"));
    assert(!menu_command_is_destructive("hibernate"));
}

static void test_destructive_rejects_near_misses(void) {
    assert(!menu_command_is_destructive(NULL));
    assert(!menu_command_is_destructive(""));
    assert(!menu_command_is_destructive("Poweroff"));
    assert(!menu_command_is_destructive("poweroff "));
    assert(!menu_command_is_destructive("power"));
}

static void test_label_format(void) {
    char out[32];
    assert(menu_confirm_label("Poweroff", out, sizeof(out)) == 0);
    assert(strcmp(out, "Confirm Poweroff") == 0);
}

static void test_label_rejects_bad_input(void) {
    char out[32];
    assert(menu_confirm_label(NULL, out, sizeof(out)) == -1);
    assert(menu_confirm_label("Reboot", NULL, sizeof(out)) == -1);
    assert(menu_confirm_label("Reboot", out, 0) == -1);
}

static void test_label_never_overflows(void) {
    /* "Confirm Reboot" needs 15 bytes with the terminator. */
    char out[16];
    assert(menu_confirm_label("Reboot", out, 15) == 0);
    assert(menu_confirm_label("Reboot", out, 14) == -1);
    assert(out[0] == '\0');
}

static void test_label_property_fits_or_fails_cleanly(void) {
    srand(117);
    for (int i = 0; i < 100000; i++) {
        char action[40];
        size_t len = (size_t)(rand() % 39);
        for (size_t j = 0; j < len; j++) {
            action[j] = (char)('A' + rand() % 26);
        }
        action[len] = '\0';

        char out[24];
        size_t size = (size_t)(1 + rand() % 23);
        int rc = menu_confirm_label(action, out, size);
        if (rc == 0) {
            assert(strlen(out) < size);
            assert(strncmp(out, "Confirm ", 8) == 0);
            assert(strcmp(out + 8, action) == 0);
        } else {
            assert(strlen(action) + 8 >= size);
        }
    }
}

int main(void) {
    test_destructive_set_is_exact();
    test_destructive_rejects_near_misses();
    test_label_format();
    test_label_rejects_bad_input();
    test_label_never_overflows();
    test_label_property_fits_or_fails_cleanly();
    printf("menu_confirm: all tests passed\n");
    return 0;
}
