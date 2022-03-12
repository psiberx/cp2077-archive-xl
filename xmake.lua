set_xmakever("2.5.9")

set_project("ArchiveXL")

set_languages("cxx20", "cxx2a")
set_arch("x64")

add_requires("fmt", "hopscotch-map", "minhook", "spdlog", "tiltedcore", "yaml-cpp")

add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

if is_mode("debug") then
    set_runtimes("MDd")
else
    set_runtimes("MD")
end

if is_mode("debug") then
    set_optimize("none")
elseif is_mode("release") then
    set_optimize("fastest")
elseif is_mode("releasedbg") then
    set_optimize("fastest")
end

add_cxflags("/MP")
add_defines("RED4EXT_STATIC_LIB", "YAML_CPP_STATIC_DEFINE")

target("ArchiveXL")
    set_kind("shared")
    set_filename("ArchiveXL.dll")
    set_pcxxheader("src/stdafx.hpp")
    add_files("src/**.cpp", "lib/**.cpp")
    add_headerfiles("src/**.hpp", "lib/**.hpp")
    add_includedirs("src/", "lib/")
    add_deps("RED4ext.SDK", "wil")
    add_packages("fmt", "hopscotch-map", "minhook", "spdlog", "tiltedcore", "yaml-cpp")
    add_syslinks("Version")
    add_defines("WINVER=0x0601", "WIN32_LEAN_AND_MEAN", "NOMINMAX")

target("RED4ext.SDK")
    set_kind("static")
    set_group("vendor")
    add_files("vendor/RED4ext.SDK/src/**.cpp")
    add_headerfiles("vendor/RED4ext.SDK/include/**.hpp")
    add_includedirs("vendor/RED4ext.SDK/include/", { public = true })

target("wil")
    set_kind("static")
    set_group("vendor")
    add_headerfiles("vendor/wil/include/**.h")
    add_includedirs("vendor/wil/include/", { public = true })
