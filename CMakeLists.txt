cmake_minimum_required(VERSION 3.10)
project(MapLog VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 配置选项
option(MAPLOG_BUILD_EXAMPLES "Build example programs" ON)

# 主库目标
add_library(maplog
    src/logger.cpp
)

# 设置包含目录
target_include_directories(maplog
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 示例程序
if(MAPLOG_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# 安装配置
include(GNUInstallDirs)
install(TARGETS maplog
    EXPORT maplog-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/maplog
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 导出配置
install(EXPORT maplog-targets
    FILE MapLogTargets.cmake
    NAMESPACE MapLog::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MapLog
)

# 生成配置文件
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "MapLogConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/MapLogConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MapLogConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MapLog
)

# 安装配置文件
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MapLogConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MapLogConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MapLog
) 