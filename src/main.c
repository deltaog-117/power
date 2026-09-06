#include "power/service/power_service.h"
#include "power/model/status.h"
#include "shared/logging/logger.h"
#include <stdio.h>
#include <stdlib.h>
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

int main(int argc, char *argv[]) {
    const char *log_env = getenv("POWER_LOG_LEVEL");
    if (log_env) {
        if (strcmp(log_env, "DEBUG") == 0) {
            logger_set_level(LOG_LEVEL_DEBUG);
        } else if (strcmp(log_env, "INFO") == 0) {
            logger_set_level(LOG_LEVEL_INFO);
        } else if (strcmp(log_env, "WARN") == 0) {
            logger_set_level(LOG_LEVEL_WARN);
        } else if (strcmp(log_env, "ERROR") == 0) {
            logger_set_level(LOG_LEVEL_ERROR);
        } else if (strcmp(log_env, "CRITICAL") == 0) {
            logger_set_level(LOG_LEVEL_CRITICAL);
        }
    }

    const char *cmd = NULL;
    if (argc > 1) {
        cmd = argv[1];
    } else {
        print_usage(argv[0]);
        return 0;
    }

    PowerStatus status = power_service_execute(cmd);

    if (status != POWER_STATUS_OK) {
        LOG_ERROR("Command failed: %s", power_status_to_string(status));
        return 1;
    }

    return 0;
}
