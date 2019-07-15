#ifndef NUKEBAR_UTILS_H
#define NUKEBAR_UTILS_H

#include <stdbool.h>

#define LOG_ERROR_LABEL "ERROR"
#define LOG_WARNING_LABEL "WARNING"
#define LOG_DEBUG_LABEL "DEBUG"
#define LOG_INFO_LABEL "INFO"
#define LOG_TRACING_LABEL "TRACING"
#define LOG_TRACING2_LABEL "TRACING"

enum log_levels
{
    log_none     = 0U,
    log_error    = (1U << 0U),
    log_warning  = (1U << 1U),
    log_info     = (1U << 2U),
    log_debug    = (1U << 3U),
    log_tracing  = (1U << 4U),
    log_tracing2 = (1U << 5U),
};

static bool level_is(unsigned incoming, enum log_levels level)
{
    return ((incoming & level) == level);
}

static const char *get_log_level (enum log_levels l)
{
    if (level_is(l, log_tracing2)) { return LOG_TRACING2_LABEL; }
    if (level_is(l, log_tracing)) { return LOG_TRACING_LABEL; }
    if (level_is(l, log_debug)) { return LOG_DEBUG_LABEL; }
    if (level_is(l, log_info)) { return LOG_INFO_LABEL; }
    if (level_is(l, log_warning)) { return LOG_WARNING_LABEL; }
    if (level_is(l, log_error)) { return LOG_ERROR_LABEL; }
    return LOG_TRACING_LABEL;
}

#define _error(...) _logd(log_error, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define _warn(...) _logd(log_warning, __FILE__, __func__, __LINE__,  __VA_ARGS__)
#define _info(...) _logd(log_info, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define _debug(...) _logd(log_debug, __FILE__, __func__, __LINE__, __VA_ARGS__)
#ifdef DEBUG
#define _trace(...) _logd(log_tracing, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define _trace2(...) _logd(log_tracing2, __FILE__, __func__, __LINE__, __VA_ARGS__)
#else
#define _trace(...)
#define _trace2(...)
#endif

int _logd(enum log_levels level, const char *file, const char *function, const int line, const char *format, ...)
{
    extern enum log_levels _log_level;
    if (!level_is(_log_level, level)) { return 0; }

    va_list args;
    va_start(args, format);

    const char *label = get_log_level(level);

    char suffix[1024] = {0};
    snprintf(suffix, 1024, " in %s:%d::%s()\n", file, line, function);

    size_t s_len = strlen(suffix);
    size_t f_len = strlen(format);

    char log_format[1024] = {0};
    snprintf(log_format, 1024, "%-7s ", label);

    strncat(log_format, format, f_len + 1);
    strncat(log_format, suffix, s_len + 1);

    int result = vfprintf(stderr, log_format, args);
    va_end(args);

    return result;
}

#endif // NUKEBAR_UTILS_H
