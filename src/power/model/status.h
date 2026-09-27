#ifndef POWER_STATUS_H
#define POWER_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Status codes returned by all power operations.
 */
typedef enum {
    POWER_STATUS_OK = 0,                 /**< Operation completed successfully */
    POWER_STATUS_ERR_NULL_POINTER,       /**< A required pointer was NULL */
    POWER_STATUS_ERR_INVALID_COMMAND,    /**< Command not recognised */
    POWER_STATUS_ERR_SYSTEM,             /**< System call failed (see errno) */
    POWER_STATUS_ERR_NO_LOCKER,          /**< No screen locker found */
    POWER_STATUS_ERR_NO_LOGOUT_METHOD,   /**< No logout method found */
    POWER_STATUS_ERR_NO_LAUNCHER,        /**< No menu launcher (dmenu/rofi/etc.) found */
    POWER_STATUS_ERR_PERMISSION_DENIED,  /**< Insufficient privileges */
    POWER_STATUS_ERR_IO,                 /**< I/O error (e.g., /sys/power/state) */
    POWER_STATUS_ERR_UNKNOWN             /**< Catch‑all for unexpected failures */
} PowerStatus;

/**
 * @brief Get a human‑readable description of a PowerStatus.
 *
 * @param status PowerStatus value.
 * @return Pointer to a static string. Do NOT free or modify.
 *         Returns "Unknown status" for unrecognised values.
 */
const char *power_status_to_string(PowerStatus status);

#ifdef __cplusplus
}
#endif

#endif /* POWER_STATUS_H */
