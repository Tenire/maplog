/**
 * @Author: Tenire
 * @Date: 10 Jan 2026 19:08
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
    maplog::Logger::instance()
        .setLogDir("custom_logs")
        .setFilePrefix("myapp")
        .setFileLevel(maplog::LogLevel::DEBUG)
        .setConsoleLevel(maplog::LogLevel::DEBUG)
        .setColorOutput(true)
        .init();

    LOG_DEBUG("This debug shows in console now");
    LOG_INFO("Application started");
    LOG_WARN("Warning message");

    maplog::Logger::instance().flush();
    return 0;
}
