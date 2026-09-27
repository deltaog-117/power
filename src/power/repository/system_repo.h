#ifndef SYSTEM_REPO_H
#define SYSTEM_REPO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../model/status.h"
#include "../model/config.h"
#include <stddef.h>

/**
 * @brief Set the global configuration for the repository.
 *
 * @param config Pointer to PowerConfig struct (may be NULL).
 */
void system_repo_set_config(PowerConfig *config);

/**
 * @brief Suspend the system to RAM.
 *
 * Locks the screen first (unless disabled via `lock_before_suspend = false`
 * in the config); if locking fails, suspend is refused. Then tries logind,
 * falling back to writing "mem" to /sys/power/state.
 *
 * @return POWER_STATUS_OK on success, or an error code on failure
 *         (including a failed screen lock).
 */
PowerStatus system_suspend(void);

/**
 * @brief Suspend the system to disk (hibernate).
 *
 * Locks the screen first (unless disabled via `lock_before_hibernate = false`
 * in the config); if locking fails, hibernate is refused. Then tries logind,
 * falling back to writing "disk" to /sys/power/state.
 *
 * @return POWER_STATUS_OK on success, or an error code on failure
 *         (including a failed screen lock).
 */
PowerStatus system_hibernate(void);

/**
 * @brief Power off the system.
 *
 * Tries logind first, then calls reboot(RB_POWER_OFF).
 *
 * @return POWER_STATUS_OK on success, or an error code on failure.
 */
PowerStatus system_poweroff(void);

/**
 * @brief Reboot the system.
 *
 * Tries logind first, then calls reboot(RB_AUTOBOOT).
 *
 * @return POWER_STATUS_OK on success, or an error code on failure.
 */
PowerStatus system_reboot(void);

/**
 * @brief Lock the screen.
 *
 * Uses config-specified locker order if available, otherwise uses defaults.
 *
 * @return POWER_STATUS_OK on success, POWER_STATUS_ERR_NO_LOCKER if none found,
 *         or other error code on failure.
 */
PowerStatus system_lock(void);

/**
 * @brief Log out of the current session.
 *
 * Uses config-specified logout order if available, otherwise uses defaults.
 *
 * @return POWER_STATUS_OK on success, POWER_STATUS_ERR_NO_LOGOUT_METHOD if none found,
 *         or other error code on failure.
 */
PowerStatus system_logout(void);

/**
 * @brief Pipe a list of options to an interactive launcher and capture the
 *        option the user picked.
 *
 * Tries the config-specified `menu_launcher_order` first, falling back to
 * rofi, wofi, bemenu, fuzzel, then dmenu (in that order), so a menu keeps
 * working across distros and launchers without any configuration. Each
 * candidate is checked for availability with `command -v` before it is
 * actually launched.
 *
 * @param options Array of option strings to display, one per line.
 * @param count Number of entries in @p options.
 * @param selection Buffer to receive the chosen option. Set to an empty
 *        string if the user closed the launcher without picking anything.
 * @param selection_size Size of @p selection in bytes.
 * @return POWER_STATUS_OK if a launcher ran (@p selection may still be
 *         empty on cancel), POWER_STATUS_ERR_NO_LAUNCHER if none of the
 *         configured/default launchers are installed, or
 *         POWER_STATUS_ERR_NULL_POINTER on invalid arguments.
 */
PowerStatus system_repo_run_launcher(const char *const options[], int count,
                                      char *selection, size_t selection_size);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_REPO_H */
