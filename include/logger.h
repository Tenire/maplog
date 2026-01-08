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

    void log(LogLevel level, const std::string& message);
    void stop();

    /**
     * @brief Force flush all pending log messages to disk
     * Blocks until all queued messages are written
     */
    void flush();

    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

    void setFileLevel(LogLevel level)
    {
        file_min_level_ = level;
    }
    void setConsoleLevel(LogLevel level)
    {
        console_min_level_ = level;
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
    };

    void loggerThread();
    std::string getCurrentTime();
    std::string getCurrentDate();
    std::string getLevelString(LogLevel level);
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
    size_t max_file_size_ = 10 * 1024 * 1024;
    int max_files_ = 30;
    std::string file_prefix_;

    static constexpr size_t MAX_QUEUE_SIZE = 10000;
};

/**
 * @brief Stream-style log helper that collects all << output and writes on destruction
 */
class LogStream
{
public:
    LogStream(LogLevel level) : level_(level), active_(true) {}

    // Move constructor - transfer ownership
    LogStream(LogStream&& other) noexcept : level_(other.level_), ss_(std::move(other.ss_)), active_(other.active_)
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
            Logger::instance().log(level_, ss_.str());
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
    std::stringstream ss_;
    bool active_;
};

} // namespace maplog

#define LOG_DEBUG(msg) maplog::Logger::instance().debug(msg)
#define LOG_INFO(msg)  maplog::Logger::instance().info(msg)
#define LOG_WARN(msg)  maplog::Logger::instance().warn(msg)
#define LOG_ERROR(msg) maplog::Logger::instance().error(msg)
#define LOG_FATAL(msg) maplog::Logger::instance().fatal(msg)

#define LOG(level)               maplog::LogStream(level)
#define LOG_IF(level, condition) (condition ? maplog::LogStream(level) : maplog::LogStream(maplog::LogLevel::DEBUG))
