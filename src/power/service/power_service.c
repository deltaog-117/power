#include "power_service.h"
#include "../model/command.h"
#include "../model/menu_confirm.h"
#include "../model/status.h"
#include "../repository/system_repo.h"
#include "../../shared/logging/logger.h"
#include <stdio.h>
#include <string.h>

static void print_usage(const char *progname) {
    printf("Usage: %s [command]\n", progname);
    printf("Commands:\n");
    printf("  suspend   - Suspend to RAM (sleep)\n");
    printf("  hibernate - Suspend to disk (hibernate)\n");
    printf("  poweroff  - Power off the system\n");
    printf("  reboot    - Reboot the system\n");
    printf("  lock      - Lock the screen\n");
    printf("  logout    - Log out of the current session\n");
    printf("  menu      - Show an interactive power menu\n");
    printf("  help      - Show this help message\n");
    printf("\nShortcuts:\n");
    printf("  sp, sleep    - Same as suspend\n");
    printf("  hb           - Same as hibernate\n");
    printf("  off, shutdown, sd - Same as poweroff\n");
    printf("  restart, rb  - Same as reboot\n");
    printf("  out, exit, log - Same as logout\n");
}

/* Menu labels shown to the user, paired with the command name each one
 * dispatches to (reusing power_service_execute so behaviour, including
 * lock-before-sleep and logging, stays identical to running it directly). */
static const char *const menu_labels[] = {
    "Lock", "Suspend", "Hibernate", "Poweroff", "Reboot", "Logout"
};
static const char *const menu_commands[] = {
    "lock", "suspend", "hibernate", "poweroff", "reboot", "logout"
};
static const size_t menu_count = sizeof(menu_labels) / sizeof(menu_labels[0]);

/* Second menu pass for destructive actions. Row 1 is the confirmation so a
 * plain Enter accepts it; anything else (Cancel, Esc, a closed launcher)
 * backs out without acting. */
static int confirm_action(const char *action_label) {
    char confirm_row[64];
    if (menu_confirm_label(action_label, confirm_row, sizeof(confirm_row)) != 0) {
        return 0;
    }

    const char *const rows[] = { confirm_row, MENU_CANCEL_LABEL };
    char answer[64];
    PowerStatus status = system_repo_run_launcher(rows, 2, answer, sizeof(answer));
    return status == POWER_STATUS_OK && strcmp(answer, confirm_row) == 0;
}

static PowerStatus run_menu(void) {
    char selection[64];
    PowerStatus status = system_repo_run_launcher(menu_labels, (int)menu_count,
                                                   selection, sizeof(selection));
    if (status != POWER_STATUS_OK) {
        return status;
    }

    if (selection[0] == '\0') {
        LOG_INFO("Menu cancelled; no action taken");
        return POWER_STATUS_OK;
    }

    for (size_t i = 0; i < menu_count; i++) {
        if (strcmp(selection, menu_labels[i]) == 0) {
            LOG_INFO("Menu selection: %s", menu_labels[i]);
            if (menu_command_is_destructive(menu_commands[i]) &&
                system_repo_menu_confirm_enabled() &&
                !confirm_action(menu_labels[i])) {
                LOG_INFO("%s not confirmed; no action taken", menu_labels[i]);
                return POWER_STATUS_OK;
            }
            return power_service_execute(menu_commands[i]);
        }
    }

    LOG_ERROR("Unrecognised menu selection: %s", selection);
    return POWER_STATUS_ERR_INVALID_COMMAND;
}

PowerStatus power_service_execute(const char *cmd) {
    if (cmd == NULL) {
        LOG_ERROR("Command is NULL");
        return POWER_STATUS_ERR_NULL_POINTER;
    }

    Command command = parse_command(cmd);

    if (command == CMD_UNKNOWN) {
        LOG_ERROR("Unknown command: %s", cmd);
        LOG_ERROR("Suggestion: Use 'help' to see available commands.");
        return POWER_STATUS_ERR_INVALID_COMMAND;
    }

    LOG_INFO("Executing command: %s", command_name(command));

    switch (command) {
        case CMD_SUSPEND:
            return system_suspend();

        case CMD_HIBERNATE:
            return system_hibernate();

        case CMD_POWEROFF:
            return system_poweroff();

        case CMD_REBOOT:
            return system_reboot();

        case CMD_LOCK:
            return system_lock();

        case CMD_LOGOUT:
            return system_logout();

        case CMD_MENU:
            return run_menu();

        case CMD_HELP:
            print_usage("power");
            return POWER_STATUS_OK;

        default:
            LOG_ERROR("Unhandled command: %s", command_name(command));
            return POWER_STATUS_ERR_UNKNOWN;
    }
}
