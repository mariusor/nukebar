/**
 *
 * @author Marius Orcsik <marius@habarnam.ro>
 */
#ifndef NUKEBAR_CONFIGURATION_H
#define NUKEBAR_CONFIGURATION_H

#include <sys/stat.h>
#include <stdbool.h>

#include "sstrings.h"
#include "ini.h"

#define MAX_PROPERTY_LENGTH        512 //bytes
#define USER_NAME_LENGTH MAX_PROPERTY_LENGTH / 3
#define HOME_LENGTH MAX_PROPERTY_LENGTH - 30

struct env_variables {
    const char user_name[USER_NAME_LENGTH + 1];
    const char home[HOME_LENGTH + 1];
    const char xdg_config_home[MAX_PROPERTY_LENGTH + 1];
    const char xdg_data_home[MAX_PROPERTY_LENGTH + 1];
    const char xdg_cache_home[MAX_PROPERTY_LENGTH + 1];
    const char xdg_runtime_dir[MAX_PROPERTY_LENGTH + 1];
};

struct configuration {
    bool env_loaded;
    struct env_variables env;
    const char name[MAX_PROPERTY_LENGTH];
    const char file[MAX_PROPERTY_LENGTH];
    const int height;
};

#ifndef APPLICATION_NAME
#define APPLICATION_NAME            "nukebar"
#endif

#define CONFIG_FILE_NAME            "config"
#define CONFIG_DIR_NAME             ".config"
#define CACHE_DIR_NAME              ".cache"
#define DATA_DIR_NAME               ".local/share"
#define HOME_DIR                    "/home"

#define TOKENIZED_CONFIG_DIR        "%s/%s"
#define TOKENIZED_DATA_DIR          "%s/%s"
#define TOKENIZED_CACHE_DIR         "%s/%s"
#define TOKENIZED_CONFIG_PATH       "%s/%s/%s"

#define HOME_VAR_NAME               "HOME"
#define USERNAME_VAR_NAME           "USER"
#define XDG_CONFIG_HOME_VAR_NAME    "XDG_CONFIG_HOME"
#define XDG_DATA_HOME_VAR_NAME      "XDG_DATA_HOME"
#define XDG_CACHE_HOME_VAR_NAME     "XDG_CACHE_HOME"
#define XDG_RUNTIME_DIR_VAR_NAME    "XDG_RUNTIME_DIR"

#define CONFIG_VALUE_TRUE           "true"
#define CONFIG_VALUE_ONE            "1"
#define CONFIG_VALUE_FALSE          "false"
#define CONFIG_VALUE_ZERO           "0"
#define CONFIG_KEY_ENABLED          "enabled"
#define CONFIG_KEY_IGNORE           "ignore"

#define CONFIG_GROUP_BAR            "bar"
#define CONFIG_KEY_SCREEN           "screen"
#define CONFIG_VALUE_PRIMARY        "primary"
#define CONFIG_KEY_HEIGHT           "height"

static void load_environment(struct env_variables *env)
{
    if (NULL == env) { return; }
    extern char **environ;

    size_t home_var_len = strlen(HOME_VAR_NAME);
    size_t username_var_len = strlen(USERNAME_VAR_NAME);
    size_t config_home_var_len = strlen(XDG_CONFIG_HOME_VAR_NAME);
    size_t data_home_var_len = strlen(XDG_DATA_HOME_VAR_NAME);
    size_t cache_home_var_len = strlen(XDG_CACHE_HOME_VAR_NAME);
    size_t runtime_dir_var_len = strlen(XDG_RUNTIME_DIR_VAR_NAME);

    size_t home_len = 0;
    size_t username_len = 0;
    size_t config_home_len = 0;
    size_t data_home_len = 0;
    size_t cache_home_len = 0;
    size_t runtime_dir_len = 0;

    size_t i = 0;
    while(environ[i]) {
        const char *current = environ[i];
        size_t current_len = strlen(current);
        if (strncmp(current, HOME_VAR_NAME, home_var_len) == 0) {
            home_len = current_len - home_var_len;
            strncpy((char*)env->home, current + home_var_len + 1, home_len);
        }
        if (strncmp(current, USERNAME_VAR_NAME, username_var_len) == 0) {
            username_len = current_len - username_var_len;
            strncpy((char*)env->user_name, current + username_var_len + 1, username_len);
        }
        if (strncmp(current, XDG_CONFIG_HOME_VAR_NAME, config_home_var_len) == 0) {
            config_home_len = current_len - config_home_var_len;
            strncpy((char*)env->xdg_config_home, current + config_home_var_len + 1, config_home_len);
        }
        if (strncmp(current, XDG_DATA_HOME_VAR_NAME, data_home_var_len) == 0) {
            data_home_len = current_len - data_home_var_len;
            strncpy((char*)env->xdg_data_home, current + data_home_var_len + 1, data_home_len);
        }
        if (strncmp(current, XDG_CACHE_HOME_VAR_NAME, cache_home_var_len) == 0) {
            cache_home_len = current_len - cache_home_var_len;
            strncpy((char*)env->xdg_cache_home, current + cache_home_var_len + 1, cache_home_len);
        }
        if (strncmp(current, XDG_RUNTIME_DIR_VAR_NAME, runtime_dir_var_len) == 0) {
            runtime_dir_len = current_len - runtime_dir_var_len;
            strncpy((char*)env->xdg_runtime_dir, current + runtime_dir_var_len + 1, runtime_dir_len);
        }
        i++;
    }
    if (strlen(env->user_name) > 0 && strlen(env->home) == 0) {
        snprintf((char*)env->home, HOME_LENGTH, TOKENIZED_DATA_DIR, HOME_DIR, env->user_name);
    }
    if (strlen(env->home) > 0) {
        if (strlen(env->xdg_data_home) == 0) {
            snprintf((char*)&env->xdg_data_home, MAX_PROPERTY_LENGTH, TOKENIZED_DATA_DIR, env->home, DATA_DIR_NAME);
        }
        if (strlen(env->xdg_config_home) == 0) {
            snprintf((char*)&env->xdg_config_home, MAX_PROPERTY_LENGTH, TOKENIZED_CONFIG_DIR, env->home, CONFIG_DIR_NAME);
        }
        if (strlen(env->xdg_cache_home) == 0) {
            snprintf((char*)&env->xdg_cache_home, MAX_PROPERTY_LENGTH, TOKENIZED_CACHE_DIR, env->home, CACHE_DIR_NAME);
        }
    }
}

static char *get_config_path(struct configuration *config, const char *file_name)
{
    if (NULL == config) { return NULL; }

    if (NULL == file_name) {
        file_name = "config";
    }

    size_t name_len = strlen(config->name);
    size_t config_home_len = strlen(config->env.xdg_config_home);
    size_t cred_len = strlen(file_name);
    size_t path_len = name_len + config_home_len + cred_len + 2;

    char *path = calloc(1, (1 + path_len) * sizeof(char));
    if (NULL == path) { return NULL; }

    snprintf(path, path_len + 1, TOKENIZED_CONFIG_PATH, config->env.xdg_config_home, config->name, file_name);
    return path;
}

char *get_config_file(struct configuration *config)
{
    return get_config_path(config, CONFIG_FILE_NAME);
}

void load_ini_from_file(struct ini_config *ini, const char* path)
{
    if (NULL == ini) { return; }
    if (NULL == path) { return; }

    FILE *file = fopen(path, "r");
    if (NULL == file) { return; }

    long file_size;

    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);

    if (file_size <= 0) { return; }
    rewind (file);

    char *buffer = calloc(1, sizeof(char) * (file_size + 1));
    if (1 != fread(buffer, file_size, 1, file)) {
        _warn("config::error: unable to read file %s", path);
        fclose(file);
        return;
    }
    fclose(file);

    if (ini_parse(buffer, file_size, ini) < 0) {
        _error("config::error: failed to parse file %s", path);
    }
    free(buffer);
}

void load_config_from_file(struct configuration *config, const char* path)
{
    if (NULL == config) { return; }
    if (NULL == path) { return; }

    struct ini_config ini = {0};
    load_ini_from_file(&ini, path);
    int group_count = arrlen(ini.groups);
    for (int i = 0; i < group_count; i++) {
        // todo(marius): add the config load
        struct ini_group *group = ini.groups[i];
        if (strncmp(group->name->data, CONFIG_GROUP_BAR, 3) == 0) {
            //for (int i = 0; i < group->item_count; i++) {
            //}
        }
    }

    print_ini(&ini);
    ini_config_clean(&ini);
}

void load_config (struct configuration *config)
{
    char* path = get_config_file(config);
    if (NULL == path) {
        return;
    }

    load_config_from_file(config, path);
    strncpy((char*)config->file, path, MAX_PROPERTY_LENGTH);

    free(path);
}

void configuration_clean(struct configuration *config)
{
    if (NULL == config) { return; }
    _trace2("mem::free::configuration");
}

void print_configuration(struct configuration *config)
{
    printf("app::name %s\n", config->name);
    printf("app::user %s\n", config->env.user_name);
    printf("app::home_folder %s\n", config->env.home);
    printf("app::config_folder %s\n", config->env.xdg_config_home);
    printf("app::config_file %s\n", config->file);
    printf("app::data_folder %s\n", config->env.xdg_data_home);
    printf("app::cache_folder %s\n", config->env.xdg_cache_home);
    printf("app::runtime_dir %s\n", config->env.xdg_runtime_dir);
}

bool load_configuration(struct configuration *config, const char *name)
{
    if (NULL == config) { return false; }
    if (NULL != name) {
        strncpy((char*)config->name, name, MAX_PROPERTY_LENGTH - 1);
    } else {
        strncpy((char*)config->name, APPLICATION_NAME, MAX_PROPERTY_LENGTH - 1);
    }

    if (!config->env_loaded) {
        load_environment(&config->env);
        config->env_loaded = true;
    }

    load_config(config);

    return true;
}

bool folder_exists(const char *path)
{
    if (NULL == path) { return false; }

    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

#endif // NUKEBAR_CONFIGURATION_H
