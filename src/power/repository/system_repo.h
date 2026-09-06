#ifndef SYSTEM_REPO_H
#define SYSTEM_REPO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../model/status.h"

/**
 * @brief Suspend the system to RAM.
 *
 * Writes "mem" to /sys/power/state.
 *
 * @return POWER_STATUS_OK on success, or an error code on failure.
 */
PowerStatus system_suspend(void);

/**
 * @brief Power off the system.
 *
 * Calls reboot(RB_POWER_OFF).
 *
 * @return POWER_STATUS_OK on success, or an error code on failure.
 */
PowerStatus system_poweroff(void);

/**
 * @brief Reboot the system.
 *
 * Calls reboot(RB_AUTOBOOT).
 *
 * @return POWER_STATUS_OK on success, or an error code on failure.
 */
PowerStatus system_reboot(void);

/**
 * @brief Lock the screen.
 *
 * Tries common screen lockers in order (i3lock, gnome‑screensaver, etc.).
 *
 * @return POWER_STATUS_OK on success, POWER_STATUS_ERR_NO_LOCKER if none found,
 *         or other error code on failure.
 */
PowerStatus system_lock(void);

/**
 * @brief Log out of the current session.
 *
 * Tries common logout methods (window managers, DEs, pkill).
 *
 * @return POWER_STATUS_OK on success, POWER_STATUS_ERR_NO_LOGOUT_METHOD if none found,
 *         or other error code on failure.
 */
PowerStatus system_logout(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_REPO_H */
