#ifndef POWER_LOGGER_H
#define POWER_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

/**
 * @brief Log levels (increasing severity).
 */
typedef enum {
    LOG_LEVEL_DEBUG,   /**< Detailed debugging info */
    LOG_LEVEL_INFO,    /**< General operational messages */
    LOG_LEVEL_WARN,    /**< Non‑critical issues */
    LOG_LEVEL_ERROR,   /**< Errors that can be recovered from */
    LOG_LEVEL_CRITICAL /**< Fatal errors, program may exit */
} LogLevel;

/**
 * @brief Set the minimum log level (messages below this are suppressed).
 *
 * @param level Minimum LogLevel to output.
 */
void logger_set_level(LogLevel level);

/**
 * @brief Log a formatted message at the given level.
 *
 * Ownership: does not take ownership of @p fmt or varargs.
 * Thread‑safety: not guaranteed; caller should serialise if needed.
 *
 * @param level LogLevel for this message.
 * @param fmt   Printf‑style format string.
 * @param ...   Variable arguments for format.
 */
void log_message(LogLevel level, const char *fmt, ...);

/**
 * @brief Convenience macros for each level (automatically passes __FILE__ and __LINE__).
 */
#define LOG_DEBUG(fmt, ...)   log_message(LOG_LEVEL_DEBUG,   "[DEBUG] " fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)    log_message(LOG_LEVEL_INFO,    "[INFO] "  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)    log_message(LOG_LEVEL_WARN,    "[WARN] "  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   log_message(LOG_LEVEL_ERROR,   "[ERROR] " fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) log_message(LOG_LEVEL_CRITICAL, "[CRITICAL] " fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* POWER_LOGGER_H */
