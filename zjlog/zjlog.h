#pragma once

#if defined(_MSC_VER)
#include <sal.h>
#endif

#if defined(_MSC_VER) && defined(SHARED_LIBRARY)
#if ZJLOG_EXPORTS
#define ZJLOG_API __declspec(dllexport)
#else
#define ZJLOG_API __declspec(dllimport)
#endif
#else
#define ZJLOG_API
#endif



#ifdef __cplusplus
extern "C" {
#endif

enum zjlog_level_t {
    ZJLOG_LVL_TRACE,
    ZJLOG_LVL_DEBUG,
    ZJLOG_LVL_INFO,
    ZJLOG_LVL_WARNING,
    ZJLOG_LVL_ERROR,
    ZJLOG_LVL_FATAL
};

/**
 * @brief 初始化日地心系统
 * 
 * @param log_file 日志文件路径，如果为NULL，则使用标准输出
 * @param level 默认日志级别
 */
void ZJLOG_API zjlog_init(const char *log_file, enum zjlog_level_t level);

#if defined(_MSC_VER)
ZJLOG_API void zjlog_printf(enum zjlog_level_t level, const char *tag, _In_z_ _Printf_format_string_ const char* format, ...);
#else
ZJLOG_API void zjlog_printf(enum zjlog_level_t level, const char *tag, const char* format, ...)  __attribute__((format(printf, 3, 4)));
#endif

#define LOG_TRACE(tag, format, ...) zjlog_printf(ZJLOG_LVL_TRACE, tag, format, ##__VA_ARGS__)
#define LOG_DEBUG(tag, format, ...) zjlog_printf(ZJLOG_LVL_DEBUG, tag, format, ##__VA_ARGS__)
#define LOG_INFO(tag, format, ...)  zjlog_printf(ZJLOG_LVL_INFO, tag, format, ##__VA_ARGS__)
#define LOG_WARN(tag, format, ...)  zjlog_printf(ZJLOG_LVL_WARNING, tag, format, ##__VA_ARGS__)
#define LOG_ERROR(tag, format, ...) zjlog_printf(ZJLOG_LVL_ERROR, tag, format, ##__VA_ARGS__)
#define LOG_FATAL(tag, format, ...) zjlog_printf(ZJLOG_LVL_FATAL, tag, format, ##__VA_ARGS__)


#ifdef __cplusplus
}
#endif