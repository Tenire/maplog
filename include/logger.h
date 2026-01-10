/**
 * @Author: Tenire
 * @Date: 29 Nov 2024 01:40
 * @Email: i@tenire.com
 * @Description:
 *                 _/_/_/_/_/                    _/
 *                    _/      _/_/    _/_/_/        _/  _/_/    _/_/
 *                   _/    _/_/_/_/  _/    _/  _/  _/_/      _/_/_/_/
 *                  _/    _/        _/    _/  _/  _/        _/
 *                 _/      _/_/_/  _/    _/  _/  _/          _/_/_/
 *
 */

#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <ctime>
#include <sstream>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>

namespace maplog
{

enum class LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

/**
 * @brief Source code location information
 */
struct SourceLocation
{
    const char* file = nullptr;
    int line = 0;
    const char* func = nullptr;

    static SourceLocation current(const char* file = __builtin_FILE(), int line = __builtin_LINE(), const char* func = __builtin_FUNCTION())
    {
        return {file, line, func};
    }
};

class Logger
{
public:
    static Logger& instance();

    /**
     * @brief Initialize the logger
     * @param log_dir Directory for log files
     * @param file_prefix Prefix for log file names (default: "maplog")
     * @param file_level Minimum level for file output (default: INFO)
     * @param console_output Whether to output to console (default: true)
     * @param console_level Minimum level for console output (default: WARN)
     * @param max_size Maximum size per log file in bytes (default: 10MB)
     * @param max_files Maximum number of log files to keep (default: 30)
     */
    bool init(
        const std::string& log_dir, const std::string& file_prefix = "maplog", LogLevel file_level = LogLevel::INFO, bool console_output = true,
        LogLevel console_level = LogLevel::WARN, size_t max_size = 10 * 1024 * 1024, int max_files = 30);

    /**
     * @brief Initialize the logger using current configuration
     * This is the terminal operation for the chainable configuration
     * @example Logger::instance().setLogDir("logs").setFileLevel(LogLevel::DEBUG).init();
     */
    bool init();

    void log(LogLevel level, const std::string& message, const SourceLocation& loc = {});
    void stop();

    /**
     * @brief Force flush all pending log messages to disk
     * Blocks until all queued messages are written
     */
    void flush();

    void debug(const std::string& message, const SourceLocation& loc = {});
    void info(const std::string& message, const SourceLocation& loc = {});
    void warn(const std::string& message, const SourceLocation& loc = {});
    void error(const std::string& message, const SourceLocation& loc = {});
    void fatal(const std::string& message, const SourceLocation& loc = {});

    // ========== Chainable Configuration Methods ==========
    // All setters return Logger& to support fluent interface.
    // Call init() as the terminal operation after configuration.

    Logger& setLogDir(const std::string& log_dir)
    {
        log_dir_ = log_dir;
        return *this;
    }
    Logger& setFilePrefix(const std::string& file_prefix)
    {
        file_prefix_ = file_prefix;
        return *this;
    }
    Logger& setFileLevel(LogLevel level)
    {
        file_min_level_ = level;
        return *this;
    }
    Logger& setConsoleOutput(bool enable)
    {
        console_output_ = enable;
        return *this;
    }
    Logger& setConsoleLevel(LogLevel level)
    {
        console_min_level_ = level;
        return *this;
    }
    Logger& setColorOutput(bool enabled)
    {
        color_output_ = enabled;
        return *this;
    }
    Logger& setShowSourceLocation(bool enabled)
    {
        show_source_location_ = enabled;
        return *this;
    }
    Logger& setMaxFileSize(size_t max_size)
    {
        max_file_size_ = max_size;
        return *this;
    }
    Logger& setMaxFiles(int max_files)
    {
        max_files_ = max_files;
        return *this;
    }

    std::string getCurrentLogFile() const
    {
        return current_log_file_;
    }

private:
    Logger();
    ~Logger();

    struct LogMessage
    {
        LogLevel level;
        std::string message;
        std::string timestamp;
        std::string file;
        int line;
        std::string func;
    };

    void loggerThread();
    std::string getCurrentTime();
    std::string getCurrentDate();
    std::string getLevelString(LogLevel level);
    std::string getLevelColor(LogLevel level);
    std::string extractFilename(const std::string& path);
    void checkRotate();
    void openLogFile();
    void rotateLog();
    void cleanOldLogs();

    std::string log_dir_;
    std::string current_log_file_;
    std::ofstream log_file_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::condition_variable flush_cv_;
    std::queue<LogMessage> message_queue_;
    LogLevel file_min_level_ = LogLevel::INFO;
    LogLevel console_min_level_ = LogLevel::WARN;
    std::string current_date_;
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    std::atomic<bool> flush_requested_{false};
    std::thread logger_thread_;
    bool console_output_ = true;
    bool color_output_ = true;
    bool show_source_location_ = true;
    size_t max_file_size_ = 10 * 1024 * 1024;
    int max_files_ = 30;
    std::string file_prefix_;
    std::once_flag init_flag_;

    static constexpr size_t MAX_QUEUE_SIZE = 10000;
};

/**
 * @brief Stream-style log helper that collects all << output and writes on destruction
 */
class LogStream
{
public:
    LogStream(LogLevel level, SourceLocation loc = SourceLocation::current()) : level_(level), loc_(loc), active_(true) {}

    // Move constructor - transfer ownership
    LogStream(LogStream&& other) noexcept : level_(other.level_), loc_(other.loc_), ss_(std::move(other.ss_)), active_(other.active_)
    {
        other.active_ = false;
    }

    // Disable copy
    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

    ~LogStream()
    {
        if (active_ && !ss_.str().empty())
        {
            Logger::instance().log(level_, ss_.str(), loc_);
        }
    }

    template <typename T>
    LogStream& operator<<(const T& value)
    {
        ss_ << value;
        return *this;
    }

private:
    LogLevel level_;
    SourceLocation loc_;
    std::stringstream ss_;
    bool active_;
};

} // namespace maplog

// Source location helper
#define _MAPLOG_LOC                                                                                                                                            \
    maplog::SourceLocation                                                                                                                                     \
    {                                                                                                                                                          \
        __FILE__, __LINE__, __func__                                                                                                                           \
    }

// Helper macros for overloading based on argument count
#define _MAPLOG_GET_MACRO(_0, _1, NAME, ...) NAME
#define _MAPLOG_STREAM(level)                maplog::LogStream(level, _MAPLOG_LOC)
#define _MAPLOG_DIRECT(level, msg)           maplog::Logger::instance().log(level, msg, _MAPLOG_LOC)

// LOG_DEBUG: supports both LOG_DEBUG("msg") and LOG_DEBUG() << "msg"
#define _LOG_DEBUG_0()    _MAPLOG_STREAM(maplog::LogLevel::DEBUG)
#define _LOG_DEBUG_1(msg) _MAPLOG_DIRECT(maplog::LogLevel::DEBUG, msg)
#define LOG_DEBUG(...)    _MAPLOG_GET_MACRO(_0 __VA_OPT__(, ) __VA_ARGS__, _LOG_DEBUG_1, _LOG_DEBUG_0)(__VA_ARGS__)

// LOG_INFO: supports both LOG_INFO("msg") and LOG_INFO() << "msg"
#define _LOG_INFO_0()    _MAPLOG_STREAM(maplog::LogLevel::INFO)
#define _LOG_INFO_1(msg) _MAPLOG_DIRECT(maplog::LogLevel::INFO, msg)
#define LOG_INFO(...)    _MAPLOG_GET_MACRO(_0 __VA_OPT__(, ) __VA_ARGS__, _LOG_INFO_1, _LOG_INFO_0)(__VA_ARGS__)

// LOG_WARN: supports both LOG_WARN("msg") and LOG_WARN() << "msg"
#define _LOG_WARN_0()    _MAPLOG_STREAM(maplog::LogLevel::WARN)
#define _LOG_WARN_1(msg) _MAPLOG_DIRECT(maplog::LogLevel::WARN, msg)
#define LOG_WARN(...)    _MAPLOG_GET_MACRO(_0 __VA_OPT__(, ) __VA_ARGS__, _LOG_WARN_1, _LOG_WARN_0)(__VA_ARGS__)

// LOG_ERROR: supports both LOG_ERROR("msg") and LOG_ERROR() << "msg"
#define _LOG_ERROR_0()    _MAPLOG_STREAM(maplog::LogLevel::ERROR)
#define _LOG_ERROR_1(msg) _MAPLOG_DIRECT(maplog::LogLevel::ERROR, msg)
#define LOG_ERROR(...)    _MAPLOG_GET_MACRO(_0 __VA_OPT__(, ) __VA_ARGS__, _LOG_ERROR_1, _LOG_ERROR_0)(__VA_ARGS__)

// LOG_FATAL: supports both LOG_FATAL("msg") and LOG_FATAL() << "msg"
#define _LOG_FATAL_0()    _MAPLOG_STREAM(maplog::LogLevel::FATAL)
#define _LOG_FATAL_1(msg) _MAPLOG_DIRECT(maplog::LogLevel::FATAL, msg)
#define LOG_FATAL(...)    _MAPLOG_GET_MACRO(_0 __VA_OPT__(, ) __VA_ARGS__, _LOG_FATAL_1, _LOG_FATAL_0)(__VA_ARGS__)

#define LOG(level)               maplog::LogStream(level, _MAPLOG_LOC)
#define LOG_IF(level, condition) (condition ? maplog::LogStream(level, _MAPLOG_LOC) : maplog::LogStream(maplog::LogLevel::DEBUG, _MAPLOG_LOC))
