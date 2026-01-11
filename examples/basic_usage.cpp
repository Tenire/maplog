/**
 * @Author: Tenire
 * @Date: 29 Nov 2024 02:31
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

int main()
{
    maplog::Logger::instance().init("logs", "basic_example", maplog::LogLevel::DEBUG, true, maplog::LogLevel::DEBUG, 1024 * 1024, 5);

    // Different log levels
    LOG_DEBUG("This is a debug message");
    LOG_INFO("Application started successfully");
    LOG_WARN("Resource usage is high");
    LOG_ERROR("Failed to connect to database");
    LOG_FATAL("System is shutting down");

    // Stream syntax
    LOG_INFO() << "Current memory usage: " << 1024 << "MB";
    LOG_DEBUG() << "User count: " << 42;
    LOG_WARN() << "Disk space low: " << 95 << "%";

    // Alternative syntax with LOG()
    LOG(maplog::LogLevel::INFO) << "Alternative syntax with LOG()";

    // Conditional logging
    for (int i = 0; i < 3; ++i)
    {
        LOG_IF(maplog::LogLevel::WARN, i > 1) << "Counter exceeded threshold: " << i;
    }

    // Runtime level adjustment
    maplog::Logger::instance().setConsoleLevel(maplog::LogLevel::WARN);
    LOG_INFO("This INFO won't show in console, but will be in file");
    LOG_WARN("This WARN will show in both");

    maplog::Logger::instance().flush();

    return 0;
}