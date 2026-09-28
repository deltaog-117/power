#ifndef POWER_MENU_CONFIRM_H
#define POWER_MENU_CONFIRM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Label of the row that backs out of a confirmation. */
#define MENU_CANCEL_LABEL "Cancel"

/**
 * @brief Tell whether a menu command needs a second confirmation.
 *
 * Destructive means the action ends the session or the machine's uptime
 * (poweroff, reboot, logout). Lock, suspend and hibernate are recoverable
 * and stay one step.
 *
 * @param command Canonical command name (e.g. "poweroff"); may be NULL.
 * @return 1 if the action needs confirmation, 0 otherwise.
 */
int menu_command_is_destructive(const char *command);

/**
 * @brief Build the "Confirm <label>" row text for a confirmation menu.
 *
 * @param action_label Menu label of the picked action (e.g. "Poweroff").
 * @param out Destination buffer.
 * @param out_size Size of @p out in bytes.
 * @return 0 on success, -1 if an argument is invalid or the text does not fit.
 */
int menu_confirm_label(const char *action_label, char *out, size_t out_size);

#ifdef __cplusplus
}
#endif

#endif /* POWER_MENU_CONFIRM_H */
