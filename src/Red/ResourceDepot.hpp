#pragma once

namespace Red
{
// The game inserted two archive scopes before the mod scope in 2.5,
// RED4ext::ArchiveScope::Mod still has the old value (4).
constexpr auto ModArchiveScope = static_cast<ArchiveScope>(6);
}

namespace Raw::ResourceDepot
{
constexpr auto InitializeArchives = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceDepot_InitializeArchives,
    /* type = */ void (*)(Red::ResourceDepot* aDepot)>{};

constexpr auto LoadArchives = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceDepot_LoadArchives,
    /* type = */ void (*)(Red::ResourceDepot* aDepot,
                          Red::ArchiveGroup& aGroup,
                          const Red::DynArray<Red::CString>& aArchivePaths,
                          Red::DynArray<Red::ResourcePath>& aLoadedResourcePaths,
                          uint32_t aMemoryResident,
                          uint8_t aMode)>{}; // 2.5: base archives pass 2, mods pass 0

constexpr auto RequestResource = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceDepot_RequestResource,
    /* type = */ uintptr_t* (*)(Red::ResourceDepot* aDepot,
                                const uintptr_t* aOutResourceHandle,
                                Red::ResourcePath aPath,
                                const int32_t* aArchiveHandle)>{};

constexpr auto CheckResource = Core::RawFunc<
    /* addr = */ Red::AddressLib::ResourceDepot_CheckResource,
    /* type = */ bool (*)(Red::ResourceDepot* aDepot, Red::ResourcePath aPath)>{};
}
