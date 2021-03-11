// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LOG_H
#define LOG_H

#include "netw.h"

#define app_info(...) log_info("LINQD", __VA_ARGS__)
#define app_warn(...) log_warn("LINQD", __VA_ARGS__)
#define app_debug(...) log_debug("LINQD", __VA_ARGS__)
#define app_trace(...) log_trace("LINQD", __VA_ARGS__)
#define app_error(...) log_error("LINQD", __VA_ARGS__)
#define app_fatal(...) log_fatal("LINQD", __VA_ARGS__)

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

#ifdef __cplusplus
extern "C"
{
#endif

    void log_set_callback_fn(log_callback_fn fn, void* ctx);
    void log_set_fd(FILE* f);
    void log_set_color(bool c);
    void log_log(
        E_LOG_LEVEL level,
        const char* file,
        int line,
        const char* cat,
        const char* fmt,
        ...);

#ifdef __cplusplus
}
#endif
#endif
