#ifndef POWER_MENU_X11_H
#define POWER_MENU_X11_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../model/config.h"
#include "../model/status.h"
#include <stddef.h>

/**
 * @brief Show the built-in X11 menu and capture the chosen option.
 *
 * Draws a small override-redirect window (Xlib + Xft) centred on the
 * monitor under the pointer. Colours come from power.conf, then from the
 * user's X resources (*background, *foreground, *color4), then from
 * built-in defaults, so it follows the current theme without setup.
 * Compiled only with `make MENU=builtin`.
 *
 * @param options Labels to display, one per row.
 * @param count Number of entries in @p options.
 * @param config Configuration for font and colours (may be NULL).
 * @param selection Buffer to receive the chosen label; left empty if the
 *        user cancelled.
 * @param selection_size Size of @p selection in bytes.
 * @return POWER_STATUS_OK if the menu ran (@p selection may be empty on
 *         cancel), POWER_STATUS_ERR_NO_LAUNCHER if it cannot run here (no
 *         X display, Wayland session, font or keyboard grab failed) so the
 *         caller can fall back to an external launcher, or
 *         POWER_STATUS_ERR_NULL_POINTER on invalid arguments.
 */
PowerStatus menu_x11_run(const char *const options[], int count,
                         const PowerConfig *config, char *selection,
                         size_t selection_size);

#ifdef __cplusplus
}
#endif

#endif /* POWER_MENU_X11_H */
