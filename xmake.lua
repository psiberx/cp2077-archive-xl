set_xmakever("2.5.9")

set_project("ArchiveXL")
set_version("1.28.6", {build = "%y%m%d%H%M"})

set_arch("x64")
set_languages("cxx20")
add_cxxflags("/MP /GR- /EHsc /Zi")

if is_mode("debug") then
    set_optimize("none")
    set_symbols("debug")
    set_strip("none")
    add_cxxflags("/Od /Ob0 /RTC1")
elseif is_mode("release") then
    set_optimize("fastest")
    set_symbols("debug")
    set_strip("all")
    add_cxxflags("/Ob2 /Gy")
    add_ldflags("/DEBUG:FULL")
    add_ldflags("/INCREMENTAL:NO")
    add_ldflags("/OPT:REF")
    add_ldflags("/OPT:ICF")
elseif is_mode("releasedbg") then
    set_optimize("fastest")
    set_symbols("debug")
    set_strip("all")
    add_cxxflags("/Ob1")
end

if is_mode("debug") then
    set_runtimes("MDd")
else
    set_runtimes("MD")
end

add_requires("hopscotch-map", "minhook", "spdlog", "tiltedcore", "yaml-cpp")
add_requireconfs("tiltedcore.mimalloc", {version = "2.x", override = true}) -- mimalloc 3.x dropped mi_malloc_size used by TiltedCore

target("ArchiveXL")
    set_default(true)
    set_kind("shared")
    set_filename("ArchiveXL.dll")
    set_pcxxheader("src/pch.hpp")
    add_files("src/**.cpp", "src/**.rc", "src/**.asm", "lib/**.cpp")
    add_headerfiles("src/**.hpp", "lib/**.hpp")
    add_includedirs("src/", "lib/")
    add_deps("RED4ext.SDK", "nameof", "semver", "wil")
    add_packages("hopscotch-map", "minhook", "spdlog", "tiltedcore", "yaml-cpp")
    add_syslinks("Version", "User32")
    add_defines("WINVER=0x0601", "WIN32_LEAN_AND_MEAN", "NOMINMAX")
    set_configdir("src")
    add_configfiles("config/Project.hpp.in", {prefixdir = "App"})
    add_configfiles("config/Version.rc.in", {prefixdir = "App"})
    set_configvar("AUTHOR", "psiberx")
    set_configvar("NAME", "ArchiveXL")

target("RED4ext.SDK")
    set_default(false)
    set_kind("headeronly")
    set_group("vendor")
    add_headerfiles("vendor/RED4ext.SDK/include/**.hpp")
    add_includedirs("vendor/RED4ext.SDK/include/", { public = true })

target("nameof")
    set_default(false)
    set_kind("headeronly")
    set_group("vendor")
    add_headerfiles("vendor/nameof/include/**.hpp")
    add_includedirs("vendor/nameof/include/", { public = true })

target("semver")
    set_default(false)
    set_kind("headeronly")
    set_group("vendor")
    add_headerfiles("vendor/semver/include/**.hpp")
    add_includedirs("vendor/semver/include/", { public = true })

target("wil")
    set_default(false)
    set_kind("headeronly")
    set_group("vendor")
    add_headerfiles("vendor/wil/include/**.h")
    add_includedirs("vendor/wil/include/", { public = true })

add_rules("plugin.vsxmake.autoupdate")
