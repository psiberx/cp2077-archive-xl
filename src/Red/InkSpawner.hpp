#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::InkSpawner
{
constexpr auto SpawnFromExternal = Core::RawFunc<
    /* addr = */ Red::Addresses::InkSpawner_SpawnFromExternal,
    /* type = */ uintptr_t (*)(
        Red::Handle<Red::ink::WidgetLibraryItemInstance>& aParentItem,
        Red::Handle<Red::ink::Widget>& aParentWidget,
        Red::Handle<Red::ink::WidgetLibraryResource>& aLocalLibrary,
        Red::ResourcePath aExternalLibraryPath,
        Red::CName aExternalLibraryItemName)>();
}
