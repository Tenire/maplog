target("maplog_example_basic")
    set_kind("binary")
    add_files("basic_usage.cpp")
    add_deps("maplog")

target("maplog_example_chain")
    set_kind("binary")
    add_files("chain_init.cpp")
    add_deps("maplog")

target("maplog_example_zero")
    set_kind("binary")
    add_files("zero_config.cpp")
    add_deps("maplog")
