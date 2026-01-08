# MapLog

一个轻量级的 C++ 日志库，支持多种日志级别、文件轮转和自定义格式。

## 特性

- 支持多个日志级别（DEBUG, INFO, WARN, ERROR, FATAL）
- **支持文件和控制台独立配置输出级别**
- 支持日志文件轮转（大小限制和数量限制）
- **支持 `flush()` 强制刷新**，确保程序退出前日志不丢失
- 线程安全（使用 `std::atomic` 保护关键状态）
- 支持流式语法和格式化语法
- 支持条件日志
- **支持运行时动态调整控制台/文件级别**

## 使用方法

### 基本使用

```cpp
#include "logger.h"

int main() {
    // 初始化日志系统
    maplog::Logger::instance().init(
        "logs",                         // 日志目录
        "myapp",                        // 文件前缀
        maplog::LogLevel::DEBUG,        // 文件最低级别
        true,                           // 启用控制台输出
        maplog::LogLevel::INFO,         // 控制台最低级别
        10 * 1024 * 1024,               // 单文件最大 10MB
        30                              // 最多保留 30 个文件
    );

    // 使用宏记录日志
    LOG_DEBUG("Debug message");
    LOG_INFO("Info message");
    LOG_WARN("Warning message");
    LOG_ERROR("Error message");

    // 动态调整控制台级别
    maplog::Logger::instance().setConsoleLevel(maplog::LogLevel::WARN);

    // 程序退出前强制刷新
    maplog::Logger::instance().flush();

    return 0;
}
```

### API 说明

| 方法                     | 说明                                    |
| ------------------------ | --------------------------------------- |
| `init(...)`              | 初始化日志系统，支持配置文件/控制台级别 |
| `flush()`                | 强制刷新队列中所有待写日志到磁盘        |
| `setFileLevel(level)`    | 运行时调整文件输出最低级别              |
| `setConsoleLevel(level)` | 运行时调整控制台输出最低级别            |
| `stop()`                 | 停止日志线程                            |
