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

namespace maplog {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

class Logger {
public:
    static Logger& instance();
    
    bool init(const std::string& log_dir, 
             const std::string& file_prefix = "maplog",
             LogLevel level = LogLevel::INFO,
             bool console_output = true,
             size_t max_size = 10 * 1024 * 1024,
             int max_files = 30);
    void log(LogLevel level, const std::string& message);
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

    template<typename T>
    Logger& operator<<(const T& message) {
        std::stringstream ss;
        ss << message;
        log(current_level_, ss.str());
        return *this;
    }

    void setLevel(LogLevel level) { 
        current_level_ = level;
        min_level_ = level; 
    }
    
    std::string getCurrentLogFile() const { return current_log_file_; }

private:
    Logger() : current_level_(LogLevel::INFO) {}
    
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
    std::mutex mutex_;
    LogLevel min_level_ = LogLevel::INFO;
    LogLevel current_level_;
    std::string current_date_;
    bool initialized_ = false;
    bool console_output_ = true;
    size_t max_file_size_ = 10 * 1024 * 1024;
    int max_files_ = 30;
    std::string file_prefix_;
};

} // namespace maplog

#define LOG_DEBUG(msg) maplog::Logger::instance().debug(msg)
#define LOG_INFO(msg)  maplog::Logger::instance().info(msg)
#define LOG_WARN(msg)  maplog::Logger::instance().warn(msg)
#define LOG_ERROR(msg) maplog::Logger::instance().error(msg)
#define LOG_FATAL(msg) maplog::Logger::instance().fatal(msg)

#define LOG(level) maplog::Logger::instance().setLevel(level); maplog::Logger::instance()
#define LOG_IF(level, condition) \
    if (condition) maplog::Logger::instance().setLevel(level); \
    if (condition) maplog::Logger::instance()