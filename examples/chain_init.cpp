#include "logger.h"

// 链式配置示例：只设置你关心的选项
int main()
{
    // 链式调用，只配置需要的选项
    maplog::Logger::instance()
        .setLogDir("custom_logs")
        .setFilePrefix("myapp")
        .setConsoleLevel(maplog::LogLevel::DEBUG)
        .setColorOutput(true)
        .init();

    LOG_DEBUG("This debug shows in console now");
    LOG_INFO("Application started");
    LOG_WARN("Warning message");

    maplog::Logger::instance().flush();
    return 0;
}
