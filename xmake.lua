set_xmakever("2.5.9")

set_project("ArchiveXL")

set_arch("x64")
set_languages("cxx20", "cxx2a")

add_requires("fmt", "hopscotch-map", "minhook", "spdlog", "tiltedcore", "yaml-cpp")

set_optimize("none")
add_cxxflags("/MP /GR- /EHsc")

if is_mode("debug") then
    set_symbols("debug")
    add_cxxflags("/Od /Ob0 /Zi /RTC1")
elseif is_mode("release") then
    set_symbols("hidden")
    set_strip("all")
    add_cxxflags("/O2 /Ob2 /fp:fast")
elseif is_mode("releasedbg") then
    set_symbols("debug")
    set_strip("all")
    add_cxxflags("/O2 /Ob1 /fp:fast /Zi")
end

if is_mode("debug") then
    set_runtimes("MDd")
else
    set_runtimes("MD")
end

add_defines("RED4EXT_STATIC_LIB", "YAML_CPP_STATIC_DEFINE")

target("ArchiveXL")
    set_default(true)
    set_kind("shared")
    set_filename("ArchiveXL.dll")
    set_pcxxheader("src/stdafx.hpp")
    add_files("src/**.cpp", "lib/**.cpp")
    add_headerfiles("src/**.hpp", "lib/**.hpp", "build/Project.hpp")
    add_includedirs("src/", "lib/", "build/")
    add_deps("RED4ext.SDK", "semver", "wil")
    add_packages("fmt", "hopscotch-map", "minhook", "spdlog", "tiltedcore", "yaml-cpp")
    add_syslinks("Version")
    add_defines("WINVER=0x0601", "WIN32_LEAN_AND_MEAN", "NOMINMAX")
    set_configdir("build")
    add_configfiles("src/Project.hpp.in")

target("RED4ext.SDK")
    set_default(false)
    set_kind("static")
    set_group("vendor")
    add_files("vendor/RED4ext.SDK/src/**.cpp")
    add_headerfiles("vendor/RED4ext.SDK/include/**.hpp")
    add_includedirs("vendor/RED4ext.SDK/include/", { public = true })

target("semver")
    set_default(false)
    set_kind("static")
    set_group("vendor")
    add_headerfiles("vendor/semver/include/**.hpp")
    add_includedirs("vendor/semver/include/", { public = true })

target("wil")
    set_default(false)
    set_kind("static")
    set_group("vendor")
    add_headerfiles("vendor/wil/include/**.h")
    add_includedirs("vendor/wil/include/", { public = true })

add_rules("plugin.vsxmake.autoupdate")
