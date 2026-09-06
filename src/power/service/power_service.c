#include "power_service.h"
#include "../model/command.h"
#include "../model/status.h"
#include "../repository/system_repo.h"
#include "../../shared/logging/logger.h"
#include <stdio.h>
#include <string.h>

static void print_usage(const char *progname) {
    printf("Usage: %s [command]\n", progname);
    printf("Commands:\n");
    printf("  suspend   - Suspend to RAM (sleep)\n");
    printf("  poweroff  - Power off the system\n");
    printf("  reboot    - Reboot the system\n");
    printf("  lock      - Lock the screen\n");
    printf("  logout    - Log out of the current session\n");
    printf("  help      - Show this help message\n");
    printf("\nShortcuts:\n");
    printf("  sp, sleep    - Same as suspend\n");
    printf("  off, shutdown, sd - Same as poweroff\n");
    printf("  restart, rb  - Same as reboot\n");
    printf("  out, exit, log - Same as logout\n");
}

PowerStatus power_service_execute(const char *cmd) {
    if (cmd == NULL) {
        LOG_ERROR("Command is NULL");
        return POWER_STATUS_ERR_NULL_POINTER;
    }

    Command command = parse_command(cmd);

    if (command == CMD_UNKNOWN) {
        LOG_ERROR("Unknown command: %s", cmd);
        return POWER_STATUS_ERR_INVALID_COMMAND;
    }

    LOG_INFO("Executing command: %s", command_name(command));

    switch (command) {
        case CMD_SUSPEND:
            return system_suspend();

        case CMD_POWEROFF:
            return system_poweroff();

        case CMD_REBOOT:
            return system_reboot();

        case CMD_LOCK:
            return system_lock();

        case CMD_LOGOUT:
            return system_logout();

        case CMD_HELP:
            print_usage("power");
            return POWER_STATUS_OK;

        default:
            LOG_ERROR("Unhandled command: %s", command_name(command));
            return POWER_STATUS_ERR_UNKNOWN;
    }
}
