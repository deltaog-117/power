#include "power/service/power_service.h"
#include "power/model/status.h"
#include "power/model/version.h"
#include "power/model/config.h"
#include "power/repository/system_repo.h"
#include "shared/logging/logger.h"
#include <stdio.h>
#include <stdlib.h>
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
    printf("  help      - Show this help message\n");
    printf("\nShortcuts:\n");
    printf("  sp, sleep    - Same as suspend\n");
    printf("  hb           - Same as hibernate\n");
    printf("  off, shutdown, sd - Same as poweroff\n");
    printf("  restart, rb  - Same as reboot\n");
    printf("  out, exit, log - Same as logout\n");
    printf("\nOptions:\n");
    printf("  --version, -v  - Show version information\n");
}

static void print_version(void) {
    printf("%s\n", POWER_VERSION_STRING);
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

    /* Load configuration */
    PowerConfig config = {0};
    if (config_load(&config) == 0) {
        system_repo_set_config(&config);
    }

    const char *cmd = NULL;
    if (argc > 1) {
        cmd = argv[1];
    } else {
        /* Use default command from config if set */
        if (config.default_cmd) {
            LOG_DEBUG("No command provided; using default: %s", config.default_cmd);
            cmd = config.default_cmd;
        } else {
            print_usage(argv[0]);
            config_free(&config);
            return 0;
        }
    }

    if (strcmp(cmd, "--version") == 0 || strcmp(cmd, "-v") == 0) {
        print_version();
        config_free(&config);
        return 0;
    }

    PowerStatus status = power_service_execute(cmd);

    config_free(&config);

    if (status != POWER_STATUS_OK) {
        return 1;
    }

    return 0;
}
