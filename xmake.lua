add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build"})

set_project("maplog")
set_version("0.1.0")
set_languages("c++17")

-- 定义选项：是否构建示例
-- 技巧：如果是作为主项目打开，默认为 true；如果被别人 add_deps 引用，默认为 false
option("build_examples")
    set_default(os.scriptdir() == os.projectdir())
    set_showmenu(true)
    set_description("Build example programs")

-- 定义核心库 Target
target("maplog")
    set_kind("static")
    add_files("src/*.cpp")
    
    -- 【关键】public = true
    -- 这意味着：谁依赖了 maplog，谁就自动拥有了 include 这个搜索路径
    add_includedirs("include", {public = true}) 

-- 根据选项决定是否进入 examples 目录
if has_config("build_examples") then
    includes("examples")
end 