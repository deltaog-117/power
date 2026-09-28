#include "config.h"
#include "../../shared/logging/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <limits.h>

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

static int parse_bool(const char *value, int *out) {
    if (strcasecmp(value, "true") == 0 || strcasecmp(value, "yes") == 0 || strcmp(value, "1") == 0) {
        *out = 1;
        return 0;
    }
    if (strcasecmp(value, "false") == 0 || strcasecmp(value, "no") == 0 || strcmp(value, "0") == 0) {
        *out = 0;
        return 0;
    }
    return -1;
}

/* Replace a string setting, so a key repeated in the file does not leak. */
static void set_string(char **slot, const char *value) {
    free(*slot);
    *slot = strdup(value);
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
    config->menu_launcher_count = 0;
    config->default_cmd = NULL;
    config->menu_font = NULL;
    config->menu_bg = NULL;
    config->menu_fg = NULL;
    config->menu_sel_bg = NULL;
    config->menu_sel_fg = NULL;
    config->menu_border = NULL;
    config->lock_before_suspend = -1;
    config->lock_before_hibernate = -1;
    config->menu_confirm = -1;

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
        } else if (strcmp(key, "menu_launcher_order") == 0) {
            config->menu_launcher_count = parse_list(value, config->menu_launcher_order, CONFIG_MAX_ENTRIES);
            LOG_DEBUG("Config: menu_launcher_order = %d entries", config->menu_launcher_count);
        } else if (strcmp(key, "menu_font") == 0) {
            set_string(&config->menu_font, value);
        } else if (strcmp(key, "menu_bg") == 0) {
            set_string(&config->menu_bg, value);
        } else if (strcmp(key, "menu_fg") == 0) {
            set_string(&config->menu_fg, value);
        } else if (strcmp(key, "menu_sel_bg") == 0) {
            set_string(&config->menu_sel_bg, value);
        } else if (strcmp(key, "menu_sel_fg") == 0) {
            set_string(&config->menu_sel_fg, value);
        } else if (strcmp(key, "menu_border") == 0) {
            set_string(&config->menu_border, value);
        } else if (strcmp(key, "lock_before_suspend") == 0) {
            if (parse_bool(value, &config->lock_before_suspend) == 0) {
                LOG_DEBUG("Config: lock_before_suspend = %s", config->lock_before_suspend ? "true" : "false");
            } else {
                LOG_WARN("Config line %d: invalid boolean for lock_before_suspend: '%s'", line_num, value);
            }
        } else if (strcmp(key, "lock_before_hibernate") == 0) {
            if (parse_bool(value, &config->lock_before_hibernate) == 0) {
                LOG_DEBUG("Config: lock_before_hibernate = %s", config->lock_before_hibernate ? "true" : "false");
            } else {
                LOG_WARN("Config line %d: invalid boolean for lock_before_hibernate: '%s'", line_num, value);
            }
        } else if (strcmp(key, "menu_confirm") == 0) {
            if (parse_bool(value, &config->menu_confirm) == 0) {
                LOG_DEBUG("Config: menu_confirm = %s", config->menu_confirm ? "true" : "false");
            } else {
                LOG_WARN("Config line %d: invalid boolean for menu_confirm: '%s'", line_num, value);
            }
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

    for (int i = 0; i < config->menu_launcher_count; i++) {
        free(config->menu_launcher_order[i]);
    }
    config->menu_launcher_count = 0;

    free(config->menu_font);
    free(config->menu_bg);
    free(config->menu_fg);
    free(config->menu_sel_bg);
    free(config->menu_sel_fg);
    free(config->menu_border);
    config->menu_font = NULL;
    config->menu_bg = NULL;
    config->menu_fg = NULL;
    config->menu_sel_bg = NULL;
    config->menu_sel_fg = NULL;
    config->menu_border = NULL;

    free(config->default_cmd);
    config->default_cmd = NULL;
}
