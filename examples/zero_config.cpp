#include "logger.h"

// 零配置示例：不需要任何初始化，直接使用
int main()
{
    // 直接用！首次调用时自动初始化
    LOG_INFO("Hello, MapLog!");
    LOG_DEBUG("Debug info");
    LOG_WARN("Warning message");
    LOG_ERROR("Error occurred");

    // 流式语法
    LOG_INFO() << "User count: " << 42;

    return 0;
}
