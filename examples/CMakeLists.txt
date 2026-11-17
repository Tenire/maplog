add_executable(basic_example basic_usage.cpp)
target_link_libraries(basic_example PRIVATE maplog) 

add_executable(chain_example chain_init.cpp)
target_link_libraries(chain_example PRIVATE maplog)

add_executable(zero_config_example zero_config.cpp)
target_link_libraries(zero_config_example PRIVATE maplog)

add_executable(multi_channel_example multi_channel.cpp)
target_link_libraries(multi_channel_example PRIVATE maplog)