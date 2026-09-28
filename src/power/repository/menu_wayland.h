#ifndef POWER_MENU_WAYLAND_H
#define POWER_MENU_WAYLAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../model/config.h"
#include "../model/status.h"
#include <stddef.h>

/**
 * @brief Show the built-in Wayland menu and capture the chosen option.
 *
 * Draws the menu on a full-screen wlr-layer-shell overlay (Cairo + Pango
 * into a wl_shm buffer) so it is centred by the compositor's chosen output,
 * takes exclusive keyboard focus, and treats a click outside the menu as
 * cancel. Colours (`#rgb` / `#rrggbb`) and font come from power.conf, then
 * built-in defaults. Compiled only with `make MENU=wayland` or `MENU=both`.
 *
 * @param options Labels to display, one per row.
 * @param count Number of entries in @p options.
 * @param config Configuration for font and colours (may be NULL).
 * @param selection Buffer to receive the chosen label; left empty if the
 *        user cancelled.
 * @param selection_size Size of @p selection in bytes.
 * @return POWER_STATUS_OK if the menu ran (@p selection may be empty on
 *         cancel), POWER_STATUS_ERR_NO_LAUNCHER if it cannot run here (no
 *         Wayland display, no layer-shell, no keyboard, font or colour
 *         problem) so the caller can fall back to an external launcher, or
 *         POWER_STATUS_ERR_NULL_POINTER on invalid arguments.
 */
PowerStatus menu_wayland_run(const char *const options[], int count,
                             const PowerConfig *config, char *selection,
                             size_t selection_size);

#ifdef __cplusplus
}
#endif

#endif /* POWER_MENU_WAYLAND_H */
