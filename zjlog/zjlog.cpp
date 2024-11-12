#include "zjlog.h"
#include <iostream>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/manipulators.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/expressions/formatters/format.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes.hpp>


namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

src::severity_logger_mt< zjlog_level_t > lg;

static const char *kLevelStrings[] = {"Trace",   "Debug", "Info",
                                      "Warning", "Error", "Fatal"};
static const char *kLevelColors[] = {"\033[90m", "\033[37m", "\033[32m",
                                     "\033[33m", "\033[31m", "\033[35m"};

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", zjlog_level_t)


void file_log_formatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    auto level = logging::extract<zjlog_level_t>("Severity", rec.attribute_values()).get();
    auto lineid = logging::extract<unsigned int>("LineID", rec.attribute_values()).get();
    auto tag = logging::extract<std::string>("Tag", rec.attribute_values()).get();
    auto datetime = logging::extract<boost::posix_time::ptime>("TimeStamp", rec.attribute_values()).get();

    char datetime_str[64];
    auto tm = to_tm(datetime);
    strftime(datetime_str, sizeof(datetime_str), "%Y-%m-%d %H:%M:%S", &tm);
    strm <<  kLevelStrings[level] << "[" << lineid << "]["
        << datetime_str << "][" << tag  << "]: " << rec[expr::smessage];
}

void console_log_formatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    auto level = logging::extract<zjlog_level_t>("Severity", rec.attribute_values()).get();
    auto lineid = logging::extract<unsigned int>("LineID", rec.attribute_values()).get();
    auto tag = logging::extract<std::string>("Tag", rec.attribute_values()).get();
    auto datetime = logging::extract<boost::posix_time::ptime>("TimeStamp", rec.attribute_values()).get();

    char datetime_str[64];
    auto tm = to_tm(datetime);
    strftime(datetime_str, sizeof(datetime_str), "%Y-%m-%d %H:%M:%S", &tm);
    strm << kLevelColors[level] << kLevelStrings[level] << "[" << lineid << "]["
        << datetime_str << "][" << tag  << "]: " << rec[expr::smessage] << "\033[0m";
}


static bool s_is_initialized = false;

void zjlog_init(const char *log_file, enum zjlog_level_t level)
{
    if (s_is_initialized) {
        LOG_WARN("zjlog", "zjlog already initialized");
        return;
    }

    if (log_file) {
        logging::add_file_log(
            keywords::file_name = log_file,
            keywords::format = file_log_formatter
        );
    } else {
        logging::add_console_log(
            std::clog,
            keywords::format = console_log_formatter
        );
    }

    logging::core::get()->set_filter(severity >= level);
    logging::add_common_attributes();

    s_is_initialized = true;
}

void zjlog_printf(zjlog_level_t level, const char *tag, const char* format, ...)
{
    if (!s_is_initialized) {
        zjlog_init(NULL, ZJLOG_LVL_INFO);
    }

    static char buffer[4096];
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, format, args);
    va_end(args);

    BOOST_LOG_SEV(lg, level) << logging::add_value("Tag", tag) << buffer;
    //BOOST_LOG(lg) << "Hello world!";

}
