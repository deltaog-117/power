#ifndef POWER_COMMAND_H
#define POWER_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * @brief All supported power commands.
 */
typedef enum {
    CMD_SUSPEND,   /**< Suspend to RAM (sleep) */
    CMD_HIBERNATE, /**< Suspend to disk (hibernate) */
    CMD_POWEROFF,  /**< Power off the system */
    CMD_REBOOT,    /**< Reboot the system */
    CMD_LOCK,      /**< Lock the screen */
    CMD_LOGOUT,    /**< Log out of current session */
    CMD_HELP,      /**< Show help/usage */
    CMD_UNKNOWN    /**< Parse failure / unrecognised command */
} Command;

/**
 * @brief Parse a command string into a Command enum.
 *
 * @param str Null‑terminated command string (case‑sensitive).
 * @return Command value. Returns CMD_UNKNOWN if @p str is NULL
 *         or not recognised.
 */
Command parse_command(const char *str);

/**
 * @brief Get a human‑readable name for a command.
 *
 * @param cmd Command enum value.
 * @return Pointer to a static string. Do NOT free or modify.
 *         Returns "unknown" for CMD_UNKNOWN.
 */
const char *command_name(Command cmd);

#ifdef __cplusplus
}
#endif

#endif /* POWER_COMMAND_H */
