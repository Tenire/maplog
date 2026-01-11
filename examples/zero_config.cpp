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
    // No initialization needed, auto-init on first use
    LOG_INFO("Hello, MapLog!");
    LOG_DEBUG("Debug info");
    LOG_WARN("Warning message");
    LOG_ERROR("Error occurred");

    LOG_INFO() << "User count: " << 42;

    return 0;
}
