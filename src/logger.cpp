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

#include "logger.h"
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <algorithm>

namespace maplog
{

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
{
    running_.store(true, std::memory_order_release);
}

Logger::~Logger()
{
    stop();
}

void Logger::stop()
{
    running_.store(false, std::memory_order_release);
    queue_cv_.notify_one();
    if (logger_thread_.joinable())
    {
        logger_thread_.join();
    }
}

bool Logger::init(
    const std::string& log_dir, const std::string& file_prefix, LogLevel file_level, bool console_output, LogLevel console_level, size_t max_size,
    int max_files)
{
    log_dir_ = log_dir;
    file_prefix_ = file_prefix;
    file_min_level_ = file_level;
    console_output_ = console_output;
    console_min_level_ = console_level;
    max_file_size_ = max_size;
    max_files_ = max_files;

    return init();
}

bool Logger::init()
{
    try
    {
        if (log_dir_.empty())
        {
            log_dir_ = "logs";
        }
        if (file_prefix_.empty())
        {
            file_prefix_ = "maplog";
        }

        std::filesystem::create_directories(log_dir_);
        current_date_ = getCurrentDate();
        openLogFile();

        if (!log_file_.is_open())
        {
            fprintf(stderr, "Failed to open log file: %s\n", current_log_file_.c_str());
            return false;
        }

        logger_thread_ = std::thread(&Logger::loggerThread, this);
        initialized_.store(true, std::memory_order_release);
        return true;
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "Failed to initialize logger: %s\n", e.what());
        return false;
    }
}

void Logger::log(LogLevel level, const std::string& message, const SourceLocation& loc)
{
    if (!initialized_.load(std::memory_order_acquire))
    {
        std::call_once(init_flag_, [this]() { init(); });
    }

    if (level < file_min_level_)
    {
        return;
    }

    try
    {
        LogMessage log_msg{
            .level = level,
            .message = message,
            .timestamp = getCurrentTime(),
            .file = loc.file ? loc.file : "",
            .line = loc.line,
            .func = loc.func ? loc.func : ""};

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (message_queue_.size() < MAX_QUEUE_SIZE)
            {
                message_queue_.push(std::move(log_msg));
            }
        }
        queue_cv_.notify_one();
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "Failed to queue log message: %s\n", e.what());
    }
}

void Logger::loggerThread()
{
    while (running_.load(std::memory_order_acquire))
    {
        std::vector<LogMessage> messages;
        bool should_flush = false;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(
                lock,
                [this] { return !message_queue_.empty() || !running_.load(std::memory_order_acquire) || flush_requested_.load(std::memory_order_acquire); });

            while (!message_queue_.empty())
            {
                messages.push_back(std::move(message_queue_.front()));
                message_queue_.pop();
            }

            if (flush_requested_.load(std::memory_order_acquire))
            {
                should_flush = true;
            }
        }

        for (const auto& msg : messages)
        {
            try
            {
                checkRotate();

                std::stringstream file_ss;
                file_ss << msg.timestamp << " [" << getLevelString(msg.level) << "]";
                if (show_source_location_ && !msg.file.empty())
                {
                    file_ss << " [" << extractFilename(msg.file) << ":" << msg.line << " " << msg.func << "]";
                }
                file_ss << " " << msg.message << std::endl;
                log_file_ << file_ss.str();

                if (console_output_ && msg.level >= console_min_level_)
                {
                    std::stringstream console_ss;
                    if (color_output_)
                    {
                        console_ss << getLevelColor(msg.level);
                    }
                    console_ss << msg.timestamp << " [" << getLevelString(msg.level) << "]";
                    if (show_source_location_ && !msg.file.empty())
                    {
                        console_ss << " [" << extractFilename(msg.file) << ":" << msg.line << "]";
                    }
                    console_ss << " " << msg.message;
                    if (color_output_)
                    {
                        console_ss << "\033[0m";
                    }
                    console_ss << std::endl;
                    fprintf(stderr, "%s", console_ss.str().c_str());
                }
            }
            catch (const std::exception& e)
            {
                fprintf(stderr, "Failed to write log: %s\n", e.what());
            }
        }

        if (!messages.empty() || should_flush)
        {
            log_file_.flush();
        }

        // Notify flush() that we're done
        if (should_flush)
        {
            flush_requested_.store(false, std::memory_order_release);
            flush_cv_.notify_all();
        }
    }
}

void Logger::flush()
{
    if (!initialized_.load(std::memory_order_acquire))
    {
        return;
    }

    // Request flush and wait for it to complete
    flush_requested_.store(true, std::memory_order_release);
    queue_cv_.notify_one();

    std::unique_lock<std::mutex> lock(queue_mutex_);
    flush_cv_.wait(lock, [this] { return !flush_requested_.load(std::memory_order_acquire); });
}

void Logger::debug(const std::string& message, const SourceLocation& loc)
{
    log(LogLevel::DEBUG, message, loc);
}
void Logger::info(const std::string& message, const SourceLocation& loc)
{
    log(LogLevel::INFO, message, loc);
}
void Logger::warn(const std::string& message, const SourceLocation& loc)
{
    log(LogLevel::WARN, message, loc);
}
void Logger::error(const std::string& message, const SourceLocation& loc)
{
    log(LogLevel::ERROR, message, loc);
}
void Logger::fatal(const std::string& message, const SourceLocation& loc)
{
    log(LogLevel::FATAL, message, loc);
}

std::string Logger::getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::getCurrentDate()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y%m%d");
    return ss.str();
}

std::string Logger::getLevelString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO ";
    case LogLevel::WARN:
        return "WARN ";
    case LogLevel::ERROR:
        return "ERROR";
    case LogLevel::FATAL:
        return "FATAL";
    default:
        return "UNKN ";
    }
}

std::string Logger::getLevelColor(LogLevel level)
{
    switch (level)
    {
    case LogLevel::DEBUG:
        return "\033[36m"; // Cyan
    case LogLevel::INFO:
        return "\033[32m"; // Green
    case LogLevel::WARN:
        return "\033[33m"; // Yellow
    case LogLevel::ERROR:
        return "\033[31m"; // Red
    case LogLevel::FATAL:
        return "\033[35;1m"; // Magenta Bold
    default:
        return "";
    }
}

std::string Logger::extractFilename(const std::string& path)
{
    size_t pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

void Logger::checkRotate()
{
    std::string current_date = getCurrentDate();
    if (current_date != current_date_)
    {
        rotateLog();
        return;
    }

    if (log_file_.tellp() >= max_file_size_)
    {
        rotateLog();
    }
}

void Logger::rotateLog()
{
    log_file_.close();
    current_date_ = getCurrentDate();
    cleanOldLogs();
    openLogFile();

    if (!log_file_.is_open())
    {
        fprintf(stderr, "Failed to rotate log file: %s\n", current_log_file_.c_str());
    }
}

void Logger::cleanOldLogs()
{
    try
    {
        std::vector<std::string> log_files;

        for (const auto& entry : std::filesystem::directory_iterator(log_dir_))
        {
            if (entry.path().extension() == ".log")
            {
                log_files.push_back(entry.path().string());
            }
        }

        std::sort(log_files.begin(), log_files.end(), std::greater<>());

        for (size_t i = max_files_; i < log_files.size(); ++i)
        {
            std::filesystem::remove(log_files[i]);
        }
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "Failed to clean old logs: %s\n", e.what());
    }
}

void Logger::openLogFile()
{
    current_log_file_ = log_dir_ + "/" + file_prefix_ + "_" + current_date_ + ".log";
    log_file_.open(current_log_file_, std::ios::app);
}

} // namespace maplog