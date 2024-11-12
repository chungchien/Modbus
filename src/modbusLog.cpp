#include "modbusLog.hpp"
#include <ctime>
#include <cstdarg>
#include <chrono>
#include <iostream>
#include <fstream>


static const char *kLevelStrings[] = {"Trace",   "Debug", "Info",
                                      "Warning", "Error", "Fatal"};
static const char *kLevelColors[] = {"\033[90m", "\033[37m", "\033[32m",
                                     "\033[33m", "\033[31m", "\033[35m"};


static bool s_is_initialized = false;
static std::shared_ptr<std::ostream> s_log_stream;
static mblog_level_t s_log_level = MBLOG_LVL_INFO;

void mblog_init(const char *log_file, enum mblog_level_t level)
{
    if (s_is_initialized) {
        LOG_WARN("zjlog", "zjlog already initialized");
        return;
    }
    s_log_level = level;
    if (log_file) {
        s_log_stream = std::shared_ptr<std::ostream>(new std::ofstream(log_file, std::ios::out | std::ios::app));
    } else {
        s_log_stream = std::shared_ptr<std::ostream>(&std::cout, [](std::ostream *) {});
    }

    s_is_initialized = true;
}

void mblog_printf(mblog_level_t level, const char *tag, const char* format, ...)
{
    if (level < s_log_level) {
        return;
    }
    if (!s_is_initialized) {
        mblog_init(NULL, MBLOG_LVL_INFO);
    }

    static char message[4096];
    va_list args;
    va_start(args, format);
    vsprintf_s(message, format, args);
    va_end(args);

    char datetime_str[64];
    auto tm = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::strftime(datetime_str, sizeof(datetime_str), "%F %D", std::localtime(&tm));

    (*s_log_stream) << kLevelColors[level] << kLevelStrings[level] << "["
        << datetime_str << "][" << tag  << "]: " << message << "\033[0m" << std::endl;

}
