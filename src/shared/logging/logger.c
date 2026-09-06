#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static LogLevel current_level = LOG_LEVEL_INFO;

void logger_set_level(LogLevel level) {
    current_level = level;
}

void log_message(LogLevel level, const char *fmt, ...) {
    if (level < current_level) {
        return;
    }

    /* Get current time */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    /* Level names */
    const char *level_names[] = {
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR",
        "CRITICAL"
    };
    const char *level_name = (level >= 0 && level <= LOG_LEVEL_CRITICAL)
                             ? level_names[level]
                             : "UNKNOWN";

    /* Print timestamp and level */
    fprintf(stderr, "[%s] [%s] ", time_buf, level_name);

    /* Print the formatted message */
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    /* Newline */
    fprintf(stderr, "\n");
    fflush(stderr);
}
