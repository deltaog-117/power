/**
 * @file test_menu_style.c
 * @brief Unit and property tests for the menu colour and font helpers.
 *
 * Build and run from the project root:
 *   gcc -Wall -Wextra -std=gnu99 -D_GNU_SOURCE -Isrc \
 *       tests/unit/power/test_menu_style.c src/power/model/menu_style.c \
 *       -o /tmp/test_menu_style && /tmp/test_menu_style
 */

#include "power/model/menu_style.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int near(double a, double b) {
    return fabs(a - b) < 1e-9;
}

static void test_color_long_form(void) {
    menu_rgb_t c;
    assert(menu_style_parse_color("#1e1e1e", &c) == 0);
    assert(near(c.red, 30 / 255.0) && near(c.green, 30 / 255.0) &&
           near(c.blue, 30 / 255.0));
    assert(menu_style_parse_color("#FF0080", &c) == 0);
    assert(near(c.red, 1.0) && near(c.green, 0.0) && near(c.blue, 128 / 255.0));
}

static void test_color_short_form_doubles_each_digit(void) {
    menu_rgb_t c;
    assert(menu_style_parse_color("#f80", &c) == 0);
    assert(near(c.red, 1.0) && near(c.green, 0x88 / 255.0) && near(c.blue, 0.0));
}

static void test_color_rejects_malformed(void) {
    menu_rgb_t c = {0.25, 0.5, 0.75};
    const char *bad[] = {NULL,      "",        "#",       "#12",
                         "#1234",   "#12345",  "#1234567", "123456",
                         "#12345g", "red",     " #123456", "#12 456"};
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++) {
        assert(menu_style_parse_color(bad[i], &c) == -1);
    }
    assert(menu_style_parse_color("#123456", NULL) == -1);
    assert(near(c.red, 0.25) && near(c.green, 0.5) && near(c.blue, 0.75));
}

static void test_font_passes_pango_through(void) {
    char out[64];
    assert(menu_style_font_to_pango("Monospace 12", out, sizeof(out)) == 0);
    assert(strcmp(out, "Monospace 12") == 0);
}

static void test_font_translates_xft(void) {
    char out[64];
    assert(menu_style_font_to_pango("monospace:size=12", out, sizeof(out)) == 0);
    assert(strcmp(out, "monospace 12") == 0);
    assert(menu_style_font_to_pango("DejaVu Sans:size=10.5:bold:italic", out,
                                    sizeof(out)) == 0);
    assert(strcmp(out, "DejaVu Sans Bold Italic 10.5") == 0);
    assert(menu_style_font_to_pango("mono:weight=bold:slant=italic", out,
                                    sizeof(out)) == 0);
    assert(strcmp(out, "mono Bold Italic") == 0);
    assert(menu_style_font_to_pango("mono:antialias=true:size=9", out,
                                    sizeof(out)) == 0);
    assert(strcmp(out, "mono 9") == 0);
}

static void test_font_rejects_bad_input(void) {
    char out[64];
    assert(menu_style_font_to_pango(NULL, out, sizeof(out)) == -1);
    assert(menu_style_font_to_pango("", out, sizeof(out)) == -1);
    assert(menu_style_font_to_pango(":size=12", out, sizeof(out)) == -1);
    assert(menu_style_font_to_pango("mono:size=", out, sizeof(out)) == -1);
    assert(menu_style_font_to_pango("mono:size=1.2.3", out, sizeof(out)) == -1);
    assert(menu_style_font_to_pango("mono:size=big", out, sizeof(out)) == -1);
    assert(menu_style_font_to_pango("mono:size=12", NULL, 8) == -1);
    assert(menu_style_font_to_pango("mono:size=12", out, 0) == -1);
}

static void test_font_never_overflows_small_buffers(void) {
    char out[8];
    assert(menu_style_font_to_pango("monospace:size=12", out, sizeof(out)) == -1);
    assert(menu_style_font_to_pango("Monospace 12", out, sizeof(out)) == -1);
    assert(menu_style_font_to_pango("mono 9", out, sizeof(out)) == 0);
}

/* Property: every valid colour round-trips through its own hex text, and
 * every channel stays inside [0, 1]. */
static void test_color_roundtrip_property(void) {
    srand(117);
    for (int i = 0; i < 1000000; i++) {
        int r = rand() % 256;
        int g = rand() % 256;
        int b = rand() % 256;
        char text[8];
        snprintf(text, sizeof(text), "#%02x%02X%02x", r, g, b);

        menu_rgb_t c;
        assert(menu_style_parse_color(text, &c) == 0);
        assert(near(c.red, r / 255.0));
        assert(near(c.green, g / 255.0));
        assert(near(c.blue, b / 255.0));
        assert(c.red >= 0.0 && c.red <= 1.0);
    }
}

/* Property: arbitrary bytes never crash the parsers, and a successful font
 * translation always fits the buffer and is NUL-terminated. */
static void test_garbage_never_crashes_property(void) {
    srand(4242);
    for (int i = 0; i < 500000; i++) {
        char text[24];
        size_t len = (size_t)(rand() % (int)(sizeof(text) - 1));
        for (size_t j = 0; j < len; j++) {
            static const char alphabet[] = "#:=.abfz09 ;sizebolditalc";
            text[j] = alphabet[rand() % (int)(sizeof(alphabet) - 1)];
        }
        text[len] = '\0';

        menu_rgb_t c;
        (void)menu_style_parse_color(text, &c);

        char out[16];
        memset(out, 'x', sizeof(out));
        if (menu_style_font_to_pango(text, out, sizeof(out)) == 0) {
            assert(memchr(out, '\0', sizeof(out)) != NULL);
        }
    }
}

int main(void) {
    test_color_long_form();
    test_color_short_form_doubles_each_digit();
    test_color_rejects_malformed();
    test_font_passes_pango_through();
    test_font_translates_xft();
    test_font_rejects_bad_input();
    test_font_never_overflows_small_buffers();
    test_color_roundtrip_property();
    test_garbage_never_crashes_property();
    printf("menu_style: all tests passed\n");
    return 0;
}
