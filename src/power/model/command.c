#include "command.h"
#include <string.h>

/* Mapping table: command name -> Command enum */
typedef struct {
    const char *name;
    Command cmd;
} CommandEntry;

static const CommandEntry command_map[] = {
    /* Primary commands */
    {"suspend",   CMD_SUSPEND},
    {"sleep",     CMD_SUSPEND},
    {"sp",        CMD_SUSPEND},
    {"poweroff",  CMD_POWEROFF},
    {"off",       CMD_POWEROFF},
    {"shutdown",  CMD_POWEROFF},
    {"sd",        CMD_POWEROFF},
    {"po",        CMD_POWEROFF},
    {"reboot",    CMD_REBOOT},
    {"restart",   CMD_REBOOT},
    {"rb",        CMD_REBOOT},
    {"lock",      CMD_LOCK},
    {"logout",    CMD_LOGOUT},
    {"out",       CMD_LOGOUT},
    {"log",       CMD_LOGOUT},
    {"exit",      CMD_LOGOUT},
    {"help",      CMD_HELP},
    {"-h",        CMD_HELP},
    {"--help",    CMD_HELP}
};
static const size_t map_size = sizeof(command_map) / sizeof(command_map[0]);

Command parse_command(const char *str) {
    if (str == NULL) {
        return CMD_UNKNOWN;
    }

    for (size_t i = 0; i < map_size; i++) {
        if (strcmp(str, command_map[i].name) == 0) {
            return command_map[i].cmd;
        }
    }
    return CMD_UNKNOWN;
}

const char *command_name(Command cmd) {
    switch (cmd) {
        case CMD_SUSPEND:   return "suspend";
        case CMD_POWEROFF:  return "poweroff";
        case CMD_REBOOT:    return "reboot";
        case CMD_LOCK:      return "lock";
        case CMD_LOGOUT:    return "logout";
        case CMD_HELP:      return "help";
        default:            return "unknown";
    }
}
