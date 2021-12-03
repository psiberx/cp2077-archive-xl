set_xmakever("2.5.9")

set_languages("cxx20")
set_arch("x64")

add_requires("minhook", "spdlog")
-- "tiltedcore"

add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

if is_mode("debug") then
    set_optimize("none")
elseif is_mode("release") then
    set_optimize("fastest")
end

add_cxflags("/MP")
add_defines("UNICODE", "RED4EXT_STATIC_LIB", "YAML_CPP_DLL", "YAML_CPP_STATIC_DEFINE")

target("ArchiveXL")
    set_kind("shared")
    set_filename("ArchiveXL.asi")
    set_pcxxheader("src/stdafx.hpp")
    add_files("src/**.cpp")
    add_headerfiles("src/**.hpp")
    add_includedirs("src/")
    add_deps("RED4ext.SDK", "yaml-cpp")
    add_packages("minhook", "spdlog")
    add_syslinks("Version")
    add_defines("WINVER=0x0601", "WIN32_LEAN_AND_MEAN", "NOMINMAX")

target("RED4ext.SDK")
    set_kind("static")
    set_group("vendor")
    add_files("vendor/RED4ext.SDK/src/**.cpp")
    add_headerfiles("vendor/RED4ext.SDK/include/**.hpp")
    add_includedirs("vendor/RED4ext.SDK/include/", { public = true })

target("yaml-cpp")
    set_kind("static")
    set_group("vendor")
    add_files("vendor/yaml-cpp/src/**.cpp")
    add_headerfiles("vendor/yaml-cpp/include/**.hpp")
    add_includedirs("vendor/yaml-cpp/include/", { public = true })
