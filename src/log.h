// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LOG_H
#define LOG_H

#include "linq_netw_internal.h"
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

#if LINQ_LOG_LEVEL == TRACE
#define log_trace(...) log_log(LINQ_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LINQ_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(LINQ_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LINQ_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LINQ_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LINQ_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#elif LINQ_LOG_LEVEL == DEBUG
#define log_trace(...)
#define log_debug(...) log_log(LINQ_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(LINQ_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LINQ_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LINQ_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LINQ_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#elif LINQ_LOG_LEVEL == INFO
#define log_trace(...)
#define log_debug(...)
#define log_info(...) log_log(LINQ_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LINQ_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LINQ_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LINQ_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#elif LINQ_LOG_LEVEL == WARN
#define log_trace(...)
#define log_debug(...)
#define log_info(...)
#define log_warn(...) log_log(LINQ_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LINQ_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LINQ_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#elif LINQ_LOG_LEVEL == ERROR
#define log_trace(...)
#define log_debug(...)
#define log_info(...)
#define log_warn(...)
#define log_error(...) log_log(LINQ_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LINQ_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#elif LINQ_LOG_LEVEL == FATAL
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

#define FMT_STRING                                                             \
    "\x1b[37m=>\x1b[0m %3ld %s%s\x1b[0m \x1b[35m%12s:%04d\x1b[0m "

    static const char* level_names[] = { "TRACE", "DEBUG", "INFO ",
                                         "WARN ", "ERROR", "FATAL" };
    static const char* level_colors[] = { "\x1b[94m", "\x1b[36m", "\x1b[32m",
                                          "\x1b[33m", "\x1b[31m", "\x1b[35m" };
    static void
    log_log(E_LOG_LEVEL level, const char* file, int line, const char* fmt, ...)
    {
        size_t flen = strlen(file) - 1;
        while ((!(file[flen] == '/' || file[flen] == '\\')) && flen) { flen--; }
        file = flen > 1 ? &file[flen + 1] : file;
        va_list args;
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

#ifdef __cplusplus
}
#endif
#endif
