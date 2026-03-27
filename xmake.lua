add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build"})

set_project("maplog")
set_version("1.0.0")
set_languages("c++17")

-- Option: whether to build examples
-- When used as the main project, defaults to true; when included as a dependency, defaults to false
option("build_examples")
    set_default(os.scriptdir() == os.projectdir())
    set_showmenu(true)
    set_description("Build example programs")

-- Core library target
target("maplog")
    set_kind("static")
    add_files("src/*.cpp")
    add_includedirs("include", {public = true})
    add_headerfiles("include/**.h")

-- Include examples if enabled
if has_config("build_examples") then
    includes("examples")
end