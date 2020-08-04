// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LOG_H
#define LOG_H

#include "sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        LINQ_TRACE,
        LINQ_DEBUG,
        LINQ_INFO,
        LINQ_WARN,
        LINQ_ERROR,
        LINQ_FATAL,
        LINQ_NONE
    } E_LOG_LEVEL;

#if LINQ_LOG_LEVEL == 6
#define log_trace(...) log_log(LINQ_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LINQ_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(LINQ_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LINQ_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LINQ_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LINQ_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#elif LINQ_LOG_LEVEL == 5
#define log_trace(...)
#define log_debug(...) log_log(LINQ_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(LINQ_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LINQ_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LINQ_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LINQ_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#elif LINQ_LOG_LEVEL == 4
#define log_trace(...)
#define log_debug(...)
#define log_info(...) log_log(LINQ_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LINQ_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LINQ_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LINQ_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#elif LINQ_LOG_LEVEL == 3
#define log_trace(...)
#define log_debug(...)
#define log_info(...)
#define log_warn(...) log_log(LINQ_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LINQ_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LINQ_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#elif LINQ_LOG_LEVEL == 2
#define log_trace(...)
#define log_debug(...)
#define log_info(...)
#define log_warn(...)
#define log_error(...) log_log(LINQ_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LINQ_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#elif LINQ_LOG_LEVEL == 1
#define log_trace(...)
#define log_debug(...)
#define log_info(...)
#define log_warn(...)
#define log_error(...)
#define log_fatal(...) log_log(LINQ_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#else
#define log_trace(...)
#define log_debug(...)
#define log_info(...)
#define log_warn(...)
#define log_error(...)
#define log_fatal(...)
#endif

#define LOG_RESET "\x1b[0m"
#define LOG_BOLD "\x1b[1m"
#define LOG_BOLD_OFF "\x1b[21m"
#define LOG_BLINK "\x1b[5m"
#define LOG_BLINK_OFF "\x1b[25m"
#define LOG_UNDERLINE "\x1b[4m"
#define LOG_UNDERLINE_OFF "\x1b[24m"

#define LOG_BLACK "\x1b[30m"
#define LOG_RED "\x1b[31m"
#define LOG_GREEN "\x1b[32m"
#define LOG_YELLOW "\x1b[33m"
#define LOG_BLUE "\x1b[34m"
#define LOG_MAGENTA "\x1b[35m"
#define LOG_CYAN "\x1b[36m"
#define LOG_WHITE "\x1b[37m"
#define LOG_DEFAULT "\x1b[39m"
#define LOG_GRAY "\x1b[90m"
#define LOG_LIGHT_RED "\x1b[91m"
#define LOG_LIGHT_GREEN "\x1b[92m"
#define LOG_LIGHT_YELLOW "\x1b[93m"
#define LOG_LIGHT_BLUE "\x1b[94m"
#define LOG_LIGHT_MAGENTA "\x1b[95m"
#define LOG_LIGHT_CYAN "\x1b[96m"
#define LOG_LIGHT_WHITE "\x1b[97m"

#define LOG_BACKGROUND_BLACK "\x1b[40m"
#define LOG_BACKGROUND_RED "\x1b[41m"
#define LOG_BACKGROUND_GREEN "\x1b[42m"
#define LOG_BACKGROUND_YELLOW "\x1b[43m"
#define LOG_BACKGROUND_BLUE "\x1b[44m"
#define LOG_BACKGROUND_MAGENTA "\x1b[45m"
#define LOG_BACKGROUND_CYAN "\x1b[46m"
#define LOG_BACKGROUND_WHITE "\x1b[47m"
#define LOG_BACKGROUND_DEFAULT "\x1b[49m"
#define LOG_BACKGROUND_LIGHT_GRAY "\x1b[100m"
#define LOG_BACKGROUND_LIGHT_RED "\x1b[101m"
#define LOG_BACKGROUND_LIGHT_GREEN "\x1b[102m"
#define LOG_BACKGROUND_LIGHT_YELLOW "\x1b[103m"
#define LOG_BACKGROUND_LIGHT_BLUE "\x1b[104m"
#define LOG_BACKGROUND_LIGHT_MAGENTA "\x1b[105m"
#define LOG_BACKGROUND_LIGHT_CYAN "\x1b[106m"
#define LOG_BACKGROUND_LIGHT_WHITE "\x1b[107m"

    static FILE** logger = NULL;
    static void log_set_fd(FILE** f) { *logger = *f; }

#define FMT_STRING                                                             \
    LOG_WHITE "=> " LOG_RESET "%5d %s%s " LOG_RESET LOG_MAGENTA                \
              "%14s:%04d " LOG_RESET

    static const char* level_names[] = { "TRACE", "DEBUG", "INFO ",
                                         "WARN ", "ERROR", "FATAL" };
    static const char* level_colors[] = {
        LOG_LIGHT_BLUE, LOG_CYAN, LOG_GREEN, LOG_YELLOW, LOG_RED, LOG_MAGENTA
    };

    static void
    log_log(E_LOG_LEVEL level, const char* file, int line, const char* fmt, ...)
    {
        size_t flen = strlen(file) - 1;
        while ((!(file[flen] == '/' || file[flen] == '\\')) && flen) { flen--; }
        file = flen > 1 ? &file[flen + 1] : file;
        va_list args;

        if (logger) {
            fprintf(
                *logger,
                FMT_STRING,
                sys_tick(),
                level_colors[level],
                level_names[level],
                file,
                line);

            va_start(args, fmt);
            vfprintf(*logger, fmt, args);
            va_end(args);
            fprintf(*logger, "\n");
            fflush(*logger);
        } else {
            fprintf(
                stdout,
                FMT_STRING,
                sys_tick(),
                level_colors[level],
                level_names[level],
                file,
                line);

            va_start(args, fmt);
            vfprintf(stdout, fmt, args);
            va_end(args);
            fprintf(stdout, "\n");
            fflush(stdout);
        }
    }

#ifdef __cplusplus
}
#endif
#endif
