#ifndef POWER_MENU_STYLE_H
#define POWER_MENU_STYLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* A colour with each channel already in [0.0, 1.0], ready for Cairo. */
typedef struct {
    double red;
    double green;
    double blue;
} menu_rgb_t;

/**
 * @brief Parse a `#rgb` or `#rrggbb` colour (hex digits in either case).
 *
 * X colour names are not supported: a Wayland client has no X server to
 * resolve them, and guessing a palette would hide typos.
 *
 * @param text Colour text; may be NULL.
 * @param out Receives the colour; left untouched on failure.
 * @return 0 on success, -1 if @p text is NULL or not a valid colour.
 */
int menu_style_parse_color(const char *text, menu_rgb_t *out);

/**
 * @brief Convert a `menu_font` value into a Pango font description string.
 *
 * `menu_font` is documented in Xft/fontconfig form (`family:size=12`), which
 * Pango cannot read. A value without `:` is already a Pango description
 * (`Family 12`) and is copied unchanged; otherwise `size=`, `bold`, `italic`
 * (and `weight=bold`, `slant=italic`) are translated and other attributes
 * are dropped.
 *
 * @param font Font text; may be NULL.
 * @param out Destination buffer.
 * @param out_size Size of @p out in bytes.
 * @return 0 on success, -1 if @p font is empty or malformed, an argument is
 *         invalid, or the result does not fit.
 */
int menu_style_font_to_pango(const char *font, char *out, size_t out_size);

#ifdef __cplusplus
}
#endif

#endif /* POWER_MENU_STYLE_H */
