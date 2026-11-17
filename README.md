# MapLog

一个轻量级的 C++ 日志库，支持多种日志级别、文件轮转和源码位置自动捕获。

## 特性

- 支持多个日志级别（DEBUG, INFO, WARN, ERROR, FATAL）
- 支持文件和控制台独立配置输出级别
- 支持日志文件轮转（按大小和日期自动轮转，可限制文件数量）
- 支持 `flush()` 强制刷新，确保程序退出前日志不丢失
- 线程安全（使用 `std::atomic` 和异步写入线程）
- 支持流式语法 (`<<`) 和直接调用语法
- 支持条件日志
- 支持运行时动态调整控制台/文件级别
- **自动捕获源码位置**（文件名、行号、函数名）
- **控制台彩色输出**（按日志级别显示不同颜色）
- **支持链式配置（Fluent API）**
- **零配置启动** — 无需初始化即可直接使用
- **多实例与命名通道** — 支持 `Logger::get("name")` / `Logger::channel("name")` 及独立 `Logger` 实例
- **按通道格式化与原始输出模式** — 支持独立控制时间戳、级别标签、源码位置及 `raw` 原始输出

## 使用方法

### 基本使用

```cpp
#include "logger.h"

int main() {
    // 链式配置，只设置你关心的选项，其他使用默认值
    maplog::Logger::instance()
        .setLogDir("logs")           // 日志目录，不设置则仅输出到控制台
        .setFilePrefix("myapp")      // 文件前缀，默认 "maplog"
        .setFileLevel(maplog::LogLevel::DEBUG)     // 文件级别，默认 INFO
        .setConsoleLevel(maplog::LogLevel::DEBUG)  // 控制台级别，默认 WARN
        .init();  // 终结操作

    // 使用宏记录日志
    LOG_DEBUG("Debug message");
    LOG_INFO("Info message");
    LOG_WARN("Warning message");
    LOG_ERROR("Error message");

    // 流式语法
    LOG_INFO() << "User count: " << 42;

    // 条件日志
    LOG_IF(maplog::LogLevel::WARN, condition) << "Conditional message";

    // 动态调整控制台级别
    maplog::Logger::instance().setConsoleLevel(maplog::LogLevel::WARN);

    // 程序退出前强制刷新
    maplog::Logger::instance().flush();

    return 0;
}
```

### 零配置启动

```cpp
#include "logger.h"

int main() {
    // 无需任何初始化，首次调用时自动初始化（仅控制台输出）
    LOG_INFO("Hello, MapLog!");
    LOG_DEBUG() << "Debug info: " << 123;
    return 0;
}
```

### 多实例与命名通道

适用于需要将业务日志与 HTTP 访问日志、审计日志等分离的场景：

```cpp
#include "logger.h"

int main() {
    // 1. 命名通道（线程安全注册表）
    auto& access_log = maplog::Logger::get("access") // 或 Logger::channel("access")
                           .setLogDir("logs")
                           .setFilePrefix("access")
                           .setConsoleOutput(false)
                           .setShowSourceLocation(false);
    access_log.init();

    // 通道宏（支持直接传参和流式语法）
    LOG_CHANNEL_INFO("access", "GET /api/users 200 3ms");
    LOG_CHANNEL_INFO("access") << "GET /api/orders/" << 42 << " 200 5ms";

    // 原始输出（省略时间戳、级别与源码位置）
    access_log.raw("127.0.0.1 - - \"GET /health 200\" 0.5ms");
    LOG_RAW(access_log, "127.0.0.1 - - \"POST /login 200\" 12ms");

    // 2. 独立 Logger 实例（开启全局原始输出模式）
    maplog::Logger audit_logger;
    audit_logger
        .setLogDir("logs")
        .setFilePrefix("audit")
        .setRawOutput(true)
        .init();

    LOG_TO(audit_logger, maplog::LogLevel::INFO) << "{\"event\":\"login\",\"uid\":1001}";
    LOG_TO_IF(audit_logger, maplog::LogLevel::WARN, true) << "{\"event\":\"rate_limit\"}";

    access_log.flush();
    audit_logger.flush();
    return 0;
}
```

### API 说明

| 方法                     | 说明                                   |
| ------------------------ | -------------------------------------- |
| `init()`                 | 使用当前配置初始化日志系统（终结操作） |
| `init(dir, prefix, ...)` | 传统初始化方式，一次性传入所有参数     |
| `flush()`                | 强制刷新队列中所有待写日志到磁盘       |
| `stop()`                 | 停止日志线程                           |
| `Logger::get(name)`      | 获取或创建命名 `Logger` 实例（`Logger::channel(name)` 为别名） |
| `raw(msg, level)`        | 以原始模式写入单条日志（省略时间戳、级别与源码位置） |

**链式配置方法（返回 `Logger&`，支持连续调用）：**

| 方法                          | 说明                       | 默认值         |
| ----------------------------- | -------------------------- | -------------- |
| `setLogDir(dir)`              | 设置日志目录               | 无（仅控制台） |
| `setFilePrefix(prefix)`       | 设置日志文件前缀           | `"maplog"`     |
| `setFileLevel(level)`         | 设置文件最低输出级别       | `INFO`         |
| `setConsoleOutput(bool)`      | 是否启用控制台输出         | `true`         |
| `setConsoleLevel(level)`      | 设置控制台最低输出级别     | `WARN`         |
| `setColorOutput(bool)`        | 是否启用彩色输出           | `true`         |
| `setShowSourceLocation(bool)` | 是否显示源码位置           | `true`         |
| `setShowTimestamp(bool)`      | 是否显示时间戳             | `true`         |
| `setShowLevel(bool)`          | 是否显示日志级别标签       | `true`         |
| `setRawOutput(bool)`          | 是否启用原始输出模式       | `false`        |
| `setMaxFileSize(size)`        | 设置单个日志文件最大大小   | `10MB`         |
| `setMaxFiles(count)`          | 设置最多保留的日志文件数量 | `30`           |

**通道与目标实例宏：**

| 宏 | 说明 |
| --- | --- |
| `LOG_TO(logger, level)` | 向指定 `Logger` 实例写入流式日志 |
| `LOG_TO_IF(logger, level, cond)` | 条件满足时向指定 `Logger` 实例写入流式日志 |
| `LOG_RAW(logger, msg)` | 向指定 `Logger` 实例写入单条原始日志 |
| `LOG_CHANNEL(name, level)` | 向命名通道写入流式日志 |
| `LOG_CHANNEL_DEBUG/INFO/WARN/ERROR/FATAL(name, ...)` | 向命名通道写入对应级别日志（支持流式与直接传参） |

## 构建

### 直接使用源文件

最简单的方式，直接将源文件加入你的项目：

```
将 include/logger.h 和 src/logger.cpp 复制到你的项目中
```

### 使用 xmake

```bash
# 构建库和示例
xmake

# 仅构建库
xmake build maplog

# 运行示例
xmake run maplog_example_basic
```

**作为子模块引用**：

```lua
-- 在你的 xmake.lua 中
includes("path/to/MapLog")

target("your_app")
    set_kind("binary")
    add_deps("maplog")
    add_files("src/*.cpp")
```

### 使用 CMake

```bash
# 构建
mkdir build && cd build
cmake ..
make

# 安装（可选）
sudo make install
```

**作为子模块引用**：

```cmake
# 在你的 CMakeLists.txt 中
add_subdirectory(path/to/MapLog)

target_link_libraries(your_app PRIVATE maplog)
```

## 许可证

本项目采用 [GPLv3](LICENSE) 许可证。
