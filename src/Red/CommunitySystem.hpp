#pragma once

namespace Raw::AISpotPersistentDataArray
{
constexpr auto Reserve = Core::RawFunc<
    /* addr = */ Red::AddressLib::AISpotPersistentDataArray_Reserve,
    /* type = */ void (*)(Red::SortedArray<Red::AISpotPersistentData>* aArray, uint32_t aCapacity)>();
}

namespace Raw::AIWorkspotManager
{
using Spots = Core::OffsetPtr<0x48, Red::SortedArray<Red::AISpotPersistentData>>;

constexpr auto RegisterSpots = Core::RawFunc<
    /* addr = */ Red::AddressLib::AIWorkspotManager_RegisterSpots,
    /* type = */ void (*)(Red::AIWorkspotManager* aManager, const Red::DynArray<Red::AISpotPersistentData>& aSpots)>();
}
