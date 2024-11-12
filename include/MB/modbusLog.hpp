#pragma once

#if defined(_MSC_VER)
#include <sal.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

enum mblog_level_t {
    MBLOG_LVL_TRACE,
    MBLOG_LVL_DEBUG,
    MBLOG_LVL_INFO,
    MBLOG_LVL_WARNING,
    MBLOG_LVL_ERROR,
    MBLOG_LVL_FATAL
};


#ifndef MBLOG_LEVEL_DEFAULT 
#define MBLOG_LEVEL_DEFAULT MBLOG_LVL_INFO
#endif

/**
 * @brief 初始化日地心系统
 * 
 * @param log_file 日志文件路径，如果为NULL，则使用标准输出
 * @param level 默认日志级别
 */
void mblog_init(const char *log_file, enum mblog_level_t level);

#if defined(_MSC_VER)
void mblog_printf(enum mblog_level_t level, const char *tag, _In_z_ _Printf_format_string_ const char* format, ...);
#else
void mblog_printf(enum mblog_level_t level, const char *tag, const char* format, ...)  __attribute__((format(printf, 3, 4)));
#endif

#if MBLOG_LEVEL_DEFAULT <= MBLOG_LVL_TRACE
#define LOG_TRACE(tag, format, ...) mblog_printf(MBLOG_LVL_TRACE, tag, format, ##__VA_ARGS__)
#else
#define LOG_TRACE(tag, format, ...)
#endif

#if MBLOG_LEVEL_DEFAULT <= MBLOG_LVL_DEBUG
#define LOG_DEBUG(tag, format, ...) mblog_printf(MBLOG_LVL_DEBUG, tag, format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(tag, format, ...)
#endif

#if MBLOG_LEVEL_DEFAULT <= MBLOG_LVL_INFO
#define LOG_INFO(tag, format, ...)  mblog_printf(MBLOG_LVL_INFO, tag, format, ##__VA_ARGS__)
#else
#define LOG_INFO(tag, format, ...)
#endif

#if MBLOG_LEVEL_DEFAULT <= MBLOG_LVL_WARNING
#define LOG_WARN(tag, format, ...)  mblog_printf(MBLOG_LVL_WARNING, tag, format, ##__VA_ARGS__)
#else
#define LOG_WARN(tag, format, ...)
#endif

#if MBLOG_LEVEL_DEFAULT <= MBLOG_LVL_ERROR
#define LOG_ERROR(tag, format, ...) mblog_printf(MBLOG_LVL_ERROR, tag, format, ##__VA_ARGS__)
#else
#define LOG_ERROR(tag, format, ...)
#endif

#if MBLOG_LEVEL_DEFAULT <= MBLOG_LVL_FATAL
#define LOG_FATAL(tag, format, ...) mblog_printf(MBLOG_LVL_FATAL, tag, format, ##__VA_ARGS__)
#else
#define LOG_FATAL(tag, format, ...)
#endif


#ifdef __cplusplus
}
#endif