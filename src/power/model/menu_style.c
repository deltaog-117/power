#include "menu_style.h"
#include <stdio.h>
#include <string.h>

#define FONT_SCRATCH_SIZE 128

/* Private function prototypes */
static int hex_digit(char c);
static int is_valid_size(const char *text);

/* Public function definitions */
int menu_style_parse_color(const char *text, menu_rgb_t *out) {
    if (text == NULL || out == NULL || text[0] != '#') {
        return -1;
    }
    size_t digits = strlen(text + 1);
    if (digits != 3 && digits != 6) {
        return -1;
    }

    int channel[3];
    size_t step = digits / 3;
    for (size_t i = 0; i < 3; i++) {
        int high = hex_digit(text[1 + i * step]);
        int low = step == 2 ? hex_digit(text[2 + i * step]) : high;
        if (high < 0 || low < 0) {
            return -1;
        }
        channel[i] = high * 16 + low;
    }

    out->red = channel[0] / 255.0;
    out->green = channel[1] / 255.0;
    out->blue = channel[2] / 255.0;
    return 0;
}

int menu_style_font_to_pango(const char *font, char *out, size_t out_size) {
    /* strtok_r skips leading separators, so a missing family (":size=12")
     * must be rejected here or "size=12" would be taken as the family. */
    if (font == NULL || out == NULL || out_size == 0 || font[0] == '\0' ||
        font[0] == ':') {
        return -1;
    }
    if (strchr(font, ':') == NULL) {
        int written = snprintf(out, out_size, "%s", font);
        return written >= 0 && (size_t)written < out_size ? 0 : -1;
    }

    char scratch[FONT_SCRATCH_SIZE];
    if (strlen(font) >= sizeof(scratch)) {
        return -1;
    }
    strcpy(scratch, font);

    char *save = NULL;
    char *family = strtok_r(scratch, ":", &save);
    if (family == NULL || family[0] == '\0') {
        return -1;
    }

    const char *size = NULL;
    int bold = 0;
    int italic = 0;
    for (char *token = strtok_r(NULL, ":", &save); token != NULL;
         token = strtok_r(NULL, ":", &save)) {
        if (strncmp(token, "size=", 5) == 0) {
            size = token + 5;
        } else if (strcmp(token, "bold") == 0 ||
                   strcmp(token, "weight=bold") == 0) {
            bold = 1;
        } else if (strcmp(token, "italic") == 0 ||
                   strcmp(token, "slant=italic") == 0) {
            italic = 1;
        }
    }
    if (size != NULL && !is_valid_size(size)) {
        return -1;
    }

    int written = snprintf(out, out_size, "%s%s%s%s%s", family,
                           bold ? " Bold" : "", italic ? " Italic" : "",
                           size != NULL ? " " : "", size != NULL ? size : "");
    return written >= 0 && (size_t)written < out_size ? 0 : -1;
}

/* Private function definitions */
static int hex_digit(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

/* Digits with at most one dot and at least one digit: anything else would
 * reach Pango as a bogus size and silently select a default. */
static int is_valid_size(const char *text) {
    int digits = 0;
    int dots = 0;
    for (const char *p = text; *p != '\0'; p++) {
        if (*p >= '0' && *p <= '9') {
            digits++;
        } else if (*p == '.') {
            dots++;
        } else {
            return 0;
        }
    }
    return digits > 0 && dots <= 1;
}
