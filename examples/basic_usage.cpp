#include "logger.h"

int main()
{
    // 直接初始化日志系统
    if (!maplog::Logger::instance().init(
            "logs",                  // 日志目录
            "basic_example",         // 文件前缀
            maplog::LogLevel::DEBUG, // 文件最低级别
            true,                    // 控制台输出
            maplog::LogLevel::DEBUG, // 控制台最低级别 (新增参数！)
            1024 * 1024,             // 文件大小限制：1MB
            5                        // 最大文件数
            ))
    {
        return 1;
    }

    // 演示不同级别的日志 - 直接传参用法
    LOG_DEBUG("This is a debug message");
    LOG_INFO("Application started successfully");
    LOG_WARN("Resource usage is high");
    LOG_ERROR("Failed to connect to database");
    LOG_FATAL("System is shutting down");

    // 演示流式语法 - LOG_INFO() << 用法（无参数时返回流）
    LOG_INFO() << "Current memory usage: " << 1024 << "MB";
    LOG_DEBUG() << "User count: " << 42;
    LOG_WARN() << "Disk space low: " << 95 << "%";

    // 也可以用 LOG(level) << 语法
    LOG(maplog::LogLevel::INFO) << "Alternative syntax with LOG()";

    // 演示条件日志
    for (int i = 0; i < 3; ++i)
    {
        LOG_IF(maplog::LogLevel::WARN, i > 1) << "Counter exceeded threshold: " << i;
    }

    // 演示动态调整控制台级别
    LOG_INFO("Now changing console level to WARN...");
    maplog::Logger::instance().setConsoleLevel(maplog::LogLevel::WARN);
    LOG_INFO("This INFO won't show in console, but will be in file");
    LOG_WARN("This WARN will show in both");

    // 演示 flush() - 确保所有日志在程序退出前被写入
    maplog::Logger::instance().flush();

    return 0;
}