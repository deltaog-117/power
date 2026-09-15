#ifndef POWER_CONFIG_H
#define POWER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Maximum number of entries in a priority list */
#define CONFIG_MAX_ENTRIES 32

/* Configuration structure */
typedef struct {
    char *locker_order[CONFIG_MAX_ENTRIES];   /* Ordered list of locker commands */
    int locker_count;
    char *logout_order[CONFIG_MAX_ENTRIES];   /* Ordered list of logout commands */
    int logout_count;
    char *default_cmd;                         /* Default command if none provided */
    int lock_before_suspend;                    /* -1 = unset (default: on), 0 = off, 1 = on */
} PowerConfig;

/**
 * @brief Load configuration from ~/.config/power.conf.
 *
 * Recognised keys: default, locker_order, logout_order, lock_before_suspend.
 *
 * @param config Pointer to PowerConfig struct to populate.
 * @return 0 on success, -1 on error (or file not found).
 */
int config_load(PowerConfig *config);

/**
 * @brief Free any dynamically allocated memory in config.
 *
 * @param config Pointer to PowerConfig struct.
 */
void config_free(PowerConfig *config);

#ifdef __cplusplus
}
#endif

#endif /* POWER_CONFIG_H */
