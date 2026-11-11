#pragma once

#include "Red/InkSpawner.hpp"

namespace Raw::InkWidgetLibrary
{
constexpr auto AsyncSpawnFromExternal = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWidgetLibrary_AsyncSpawnFromExternal,
    /* type = */ bool (*)(Red::inkWidgetLibraryResource* aLibrary,
                          Red::InkSpawningInfo& aSpawningInfo,
                          Red::ResourcePath aExternalPath,
                          Red::CName aItemName,
                          bool aFlag)>();

constexpr auto AsyncSpawnFromLocal = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWidgetLibrary_AsyncSpawnFromLocal,
    /* type = */ bool (*)(Red::inkWidgetLibraryResource* aLibrary,
                          Red::InkSpawningInfo& aSpawningInfo,
                          Red::CName aItemName,
                          bool aFlag)>();

constexpr auto SpawnFromExternal = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWidgetLibrary_SpawnFromExternal,
    /* type = */ void* (*)(Red::inkWidgetLibraryResource* aLibrary,
                           Red::Handle<Red::inkWidgetLibraryItemInstance>& aOut,
                           Red::ResourcePath aExternalPath,
                           Red::CName aItemName)>();

constexpr auto SpawnFromLocal = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWidgetLibrary_SpawnFromLocal,
    /* type = */ void* (*)(Red::inkWidgetLibraryResource* aLibrary,
                           Red::Handle<Red::inkWidgetLibraryItemInstance>& aOut,
                           Red::CName aItemName)>();

constexpr auto FindExternalLibrary = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWidgetLibrary_FindExternalLibrary,
    /* type = */ void* (*)(Red::inkWidgetLibraryResource* aLibrary,
                           Red::Handle<Red::inkWidgetLibraryResource>& aOut,
                           Red::ResourcePath aExternalPath)>();
}
