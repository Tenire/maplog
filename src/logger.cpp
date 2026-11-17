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
#include <memory>
#include <unordered_map>

namespace maplog
{

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

Logger& Logger::get(const std::string& name)
{
    static std::mutex registry_mutex;
    static std::unordered_map<std::string, std::unique_ptr<Logger>> registry;

    std::lock_guard<std::mutex> lock(registry_mutex);
    auto it = registry.find(name);
    if (it == registry.end())
    {
        auto logger = std::make_unique<Logger>();
        logger->file_prefix_ = name.empty() ? "maplog" : name;
        auto [inserted_it, _] = registry.emplace(name, std::move(logger));
        return *inserted_it->second;
    }
    return *it->second;
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
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (!log_dir_.empty())
        {
            if (file_prefix_.empty())
            {
                file_prefix_ = "maplog";
            }
            if (!log_file_.is_open())
            {
                std::filesystem::create_directories(log_dir_);
                current_date_ = getCurrentDate();
                openLogFile();
            }
        }

        if (!logger_thread_.joinable())
        {
            running_.store(true, std::memory_order_release);
            logger_thread_ = std::thread(&Logger::loggerThread, this);
        }
        initialized_.store(true, std::memory_order_release);
        return true;
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "Failed to initialize logger: %s\n", e.what());
        return false;
    }
}

void Logger::log(LogLevel level, const std::string& message, const SourceLocation& loc, bool raw_override)
{
    if (!initialized_.load(std::memory_order_acquire))
    {
        std::call_once(init_flag_, [this]() { init(); });
    }

    const bool to_file = !log_dir_.empty() && level >= file_min_level_;
    const bool to_console = console_output_ && level >= console_min_level_;
    if (!to_file && !to_console)
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
            .func = loc.func ? loc.func : "",
            .raw = raw_override};
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

void Logger::raw(const std::string& message, LogLevel level)
{
    log(level, message, {}, true);
}

void Logger::loggerThread()
{
    while (true)
    {
        std::vector<LogMessage> messages;
        bool should_flush = false;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(
                lock,
                [this] { return !message_queue_.empty() || !running_.load(std::memory_order_acquire) || flush_requested_.load(std::memory_order_acquire); });

            if (message_queue_.empty() && !running_.load(std::memory_order_acquire))
            {
                break;
            }
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
                if (log_file_.is_open() && msg.level >= file_min_level_)
                {
                    checkRotate();
                    if (msg.raw || raw_output_)
                    {
                        log_file_ << msg.message << '\n';
                    }
                    else
                    {
                        std::stringstream file_ss;
                        bool need_space = false;
                        if (show_timestamp_)
                        {
                            file_ss << msg.timestamp;
                            need_space = true;
                        }
                        if (show_level_)
                        {
                            if (need_space)
                            {
                                file_ss << ' ';
                            }
                            file_ss << '[' << getLevelString(msg.level) << ']';
                            need_space = true;
                        }
                        if (show_source_location_ && !msg.file.empty())
                        {
                            if (need_space)
                            {
                                file_ss << ' ';
                            }
                            file_ss << '[' << extractFilename(msg.file) << ':' << msg.line << ' ' << msg.func << ']';
                            need_space = true;
                        }
                        if (need_space)
                        {
                            file_ss << ' ';
                        }
                        file_ss << msg.message << '\n';
                        log_file_ << file_ss.str();
                    }
                }

                if (console_output_ && msg.level >= console_min_level_)
                {
                    std::stringstream console_ss;
                    if (color_output_)
                    {
                        console_ss << getLevelColor(msg.level);
                    }
                    if (msg.raw || raw_output_)
                    {
                        console_ss << msg.message;
                    }
                    else
                    {
                        bool need_space = false;
                        if (show_timestamp_)
                        {
                            console_ss << msg.timestamp;
                            need_space = true;
                        }
                        if (show_level_)
                        {
                            if (need_space)
                            {
                                console_ss << ' ';
                            }
                            console_ss << '[' << getLevelString(msg.level) << ']';
                            need_space = true;
                        }
                        if (show_source_location_ && !msg.file.empty())
                        {
                            if (need_space)
                            {
                                console_ss << ' ';
                            }
                            console_ss << '[' << extractFilename(msg.file) << ':' << msg.line << ']';
                            need_space = true;
                        }
                        if (need_space)
                        {
                            console_ss << ' ';
                        }
                        console_ss << msg.message;
                    }
                    if (color_output_)
                    {
                        console_ss << "\033[0m";
                    }
                    console_ss << '\n';
                    fprintf(stderr, "%s", console_ss.str().c_str());
                }
            }
            catch (const std::exception& e)
            {
                fprintf(stderr, "Failed to write log: %s\n", e.what());
            }
        }

        if (log_file_.is_open() && (!messages.empty() || should_flush))
        {
            log_file_.flush();
        }

        if (should_flush)
        {
            flush_requested_.store(false, std::memory_order_release);
            flush_cv_.notify_all();
        }
    }
    if (log_file_.is_open())
    {
        log_file_.flush();
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

    const auto pos = log_file_.tellp();
    if (pos != std::streampos(-1) && static_cast<size_t>(pos) >= max_file_size_)
    {
        log_file_.flush();
        rotateLog();
    }
}

void Logger::rotateLog()
{
    log_file_.close();
    std::string new_date = getCurrentDate();

    std::error_code ec;
    if (!current_log_file_.empty() && std::filesystem::exists(current_log_file_, ec) &&
        std::filesystem::file_size(current_log_file_, ec) >= max_file_size_)
    {
        int index = 1;
        std::string archived_file;
        do
        {
            archived_file = log_dir_ + "/" + file_prefix_ + "_" + current_date_ + "_" + std::to_string(index) + ".log";
            ++index;
        } while (std::filesystem::exists(archived_file, ec));

        std::filesystem::rename(current_log_file_, archived_file, ec);
        if (ec)
        {
            fprintf(stderr, "Failed to archive log file %s: %s\n", current_log_file_.c_str(), ec.message().c_str());
        }
    }

    current_date_ = new_date;
    openLogFile();
    cleanOldLogs();

    if (!log_file_.is_open())
    {
        fprintf(stderr, "Failed to rotate log file: %s\n", current_log_file_.c_str());
    }
}

void Logger::cleanOldLogs()
{
    if (max_files_ <= 0)
    {
        return;
    }
    try
    {
        const std::string expected_prefix = file_prefix_ + "_";
        struct LogFileEntry
        {
            std::filesystem::path path;
            std::filesystem::file_time_type mtime;
        };
        std::vector<LogFileEntry> log_files;

        for (const auto& entry : std::filesystem::directory_iterator(log_dir_))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }
            const std::string filename = entry.path().filename().string();
            if (entry.path().extension() == ".log" &&
                filename.compare(0, expected_prefix.size(), expected_prefix) == 0)
            {
                std::error_code ec;
                auto mtime = entry.last_write_time(ec);
                log_files.push_back({entry.path(), mtime});
            }
        }

        std::sort(log_files.begin(), log_files.end(), [](const LogFileEntry& a, const LogFileEntry& b) {
            if (a.mtime != b.mtime)
            {
                return a.mtime > b.mtime;
            }
            return a.path.filename().string() > b.path.filename().string();
        });

        for (size_t i = static_cast<size_t>(max_files_); i < log_files.size(); ++i)
        {
            std::error_code ec;
            std::filesystem::remove(log_files[i].path, ec);
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