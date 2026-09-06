#ifndef SYSTEM_REPO_H
#define SYSTEM_REPO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../model/status.h"
#include "../model/config.h"

/**
 * @brief Set the global configuration for the repository.
 *
 * @param config Pointer to PowerConfig struct (may be NULL).
 */
void system_repo_set_config(PowerConfig *config);

/**
 * @brief Suspend the system to RAM.
 *
 * Tries logind first, then writes "mem" to /sys/power/state.
 *
 * @return POWER_STATUS_OK on success, or an error code on failure.
 */
PowerStatus system_suspend(void);

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

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_REPO_H */
