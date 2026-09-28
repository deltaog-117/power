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
    char *menu_launcher_order[CONFIG_MAX_ENTRIES]; /* Ordered list of menu launcher commands */
    int menu_launcher_count;
    char *menu_font;                           /* Built-in menu: fontconfig pattern (NULL = default) */
    char *menu_bg;                             /* Built-in menu colours (NULL = X resources, then default) */
    char *menu_fg;
    char *menu_sel_bg;
    char *menu_sel_fg;
    char *menu_border;
    char *default_cmd;                         /* Default command if none provided */
    int lock_before_suspend;                    /* -1 = unset (default: on), 0 = off, 1 = on */
    int lock_before_hibernate;                  /* -1 = unset (default: on), 0 = off, 1 = on */
} PowerConfig;

/**
 * @brief Load configuration from ~/.config/power.conf.
 *
 * Recognised keys: default, locker_order, logout_order, menu_launcher_order,
 * menu_font, menu_bg, menu_fg, menu_sel_bg, menu_sel_fg, menu_border,
 * lock_before_suspend, lock_before_hibernate.
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
