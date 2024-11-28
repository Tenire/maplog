#include "logger.h"

int main() {
    // 直接初始化日志系统
    if (!maplog::Logger::instance().init(
        "logs",                  // 日志目录
        "basic_example",         // 文件前缀
        maplog::LogLevel::DEBUG, // 日志级别
        true,                    // 控制台输出
        1024 * 1024,            // 文件大小限制：1MB
        5                        // 最大文件数
    )) {
        return 1;
    }

    // 演示不同级别的日志
    LOG_DEBUG("This is a debug message");
    LOG_INFO("Application started successfully");
    LOG_WARN("Resource usage is high");
    LOG_ERROR("Failed to connect to database");
    LOG_FATAL("System is shutting down");

    // 演示流式语法
    LOG(maplog::LogLevel::INFO) << "Current memory usage: " << 1024 << "MB";

    // 演示条件日志
    for (int i = 0; i < 3; ++i) {
        LOG_IF(maplog::LogLevel::WARN, i > 1) 
            << "Counter exceeded threshold: " << i;
    }

    return 0;
} 