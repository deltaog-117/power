#include "config.h"
#include "../../shared/logging/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define CONFIG_PATH ".config/power.conf"

static char *trim_whitespace(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static int parse_list(const char *value, char *list[], int max_entries) {
    char *value_copy = strdup(value);
    if (!value_copy) return 0;

    int count = 0;
    char *token = strtok(value_copy, ",");
    while (token && count < max_entries) {
        char *trimmed = trim_whitespace(token);
        if (strlen(trimmed) > 0) {
            list[count] = strdup(trimmed);
            count++;
        }
        token = strtok(NULL, ",");
    }
    free(value_copy);
    return count;
}

int config_load(PowerConfig *config) {
    const char *home = getenv("HOME");
    if (!home) {
        LOG_WARN("HOME environment variable not set");
        return -1;
    }

    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/%s", home, CONFIG_PATH);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        LOG_DEBUG("No config file found at %s; using defaults", path);
        return -1;
    }

    LOG_INFO("Loading config from %s", path);

    char line[1024];
    int line_num = 0;
    config->locker_count = 0;
    config->logout_count = 0;
    config->default_cmd = NULL;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        char *trimmed = trim_whitespace(line);

        /* Skip empty lines and comments */
        if (trimmed[0] == '\0' || trimmed[0] == '#') continue;

        /* Find the '=' separator */
        char *eq = strchr(trimmed, '=');
        if (!eq) {
            LOG_WARN("Config line %d: missing '=' separator", line_num);
            continue;
        }

        *eq = '\0';
        char *key = trim_whitespace(trimmed);
        char *value = trim_whitespace(eq + 1);

        if (strcmp(key, "default") == 0) {
            config->default_cmd = strdup(value);
            LOG_DEBUG("Config: default = %s", value);
        } else if (strcmp(key, "locker_order") == 0) {
            config->locker_count = parse_list(value, config->locker_order, CONFIG_MAX_ENTRIES);
            LOG_DEBUG("Config: locker_order = %d entries", config->locker_count);
        } else if (strcmp(key, "logout_order") == 0) {
            config->logout_count = parse_list(value, config->logout_order, CONFIG_MAX_ENTRIES);
            LOG_DEBUG("Config: logout_order = %d entries", config->logout_count);
        } else {
            LOG_WARN("Config line %d: unknown key '%s'", line_num, key);
        }
    }

    fclose(fp);
    return 0;
}

void config_free(PowerConfig *config) {
    for (int i = 0; i < config->locker_count; i++) {
        free(config->locker_order[i]);
    }
    config->locker_count = 0;

    for (int i = 0; i < config->logout_count; i++) {
        free(config->logout_order[i]);
    }
    config->logout_count = 0;

    free(config->default_cmd);
    config->default_cmd = NULL;
}
