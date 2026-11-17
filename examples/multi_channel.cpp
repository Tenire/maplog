#include "logger.h"

int main()
{
    // 1. Default business logger (singleton)
    maplog::Logger::instance()
        .setLogDir("logs")
        .setFilePrefix("app")
        .setFileLevel(maplog::LogLevel::DEBUG)
        .setConsoleLevel(maplog::LogLevel::INFO)
        .init();

    LOG_INFO("Application started");
    LOG_DEBUG() << "Business logger initialized";

    // 2. Named channel logger (e.g. HTTP access log for framework integration)
    auto& access_log = maplog::Logger::get("access")
                           .setLogDir("logs")
                           .setFilePrefix("access")
                           .setFileLevel(maplog::LogLevel::INFO)
                           .setConsoleOutput(false)
                           .setShowSourceLocation(false);
    access_log.init();

    access_log.info("GET /api/health 200 1ms");
    access_log.raw("127.0.0.1 - - \"GET /api/raw 200\" 0.5ms");
    LOG_RAW(access_log, "127.0.0.1 - - \"POST /api/login 200\" 12ms");
    LOG_CHANNEL_INFO("access", "GET /api/users 200 3ms");
    LOG_CHANNEL_INFO("access") << "GET /api/orders/" << 42 << " 200 5ms";

    // 3. Direct stack-allocated logger with raw output mode enabled
    maplog::Logger audit_logger;
    audit_logger
        .setLogDir("logs")
        .setFilePrefix("audit")
        .setFileLevel(maplog::LogLevel::INFO)
        .setConsoleOutput(false)
        .setRawOutput(true)
        .init();

    LOG_TO(audit_logger, maplog::LogLevel::INFO) << "{\"event\":\"user_login\",\"uid\":1001}";

    // 4. Conditional logging (false branch short-circuits without emitting)
    LOG_IF(maplog::LogLevel::WARN, false) << "Should not be logged";
    LOG_IF(maplog::LogLevel::WARN, true) << "Threshold exceeded";

    // 5. Flush all loggers before exit
    maplog::Logger::instance().flush();
    access_log.flush();
    audit_logger.flush();

    return 0;
}
