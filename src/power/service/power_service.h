#ifndef POWER_SERVICE_H
#define POWER_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../model/status.h"

/**
 * @brief Execute a power command.
 *
 * Parses @p cmd, executes the corresponding operation (suspend, poweroff,
 * reboot, lock, logout, or help), logs the action, and returns a status code.
 *
 * @param cmd Command string (e.g., "suspend", "lock", "help").
 * @return PowerStatus indicating success or failure.
 */
PowerStatus power_service_execute(const char *cmd);

#ifdef __cplusplus
}
#endif

#endif /* POWER_SERVICE_H */
