#pragma once

namespace Raw::PersistencySystem
{
constexpr auto SetPersistentStateData = Core::RawFunc<
    /* addr = */ Red::AddressLib::PersistencySystem_SetPersistentStateData,
    /* type = */ void (*)(Red::gamePersistencySystem* aSystem, Red::DataBuffer& aData, uint64_t a3, uint32_t a4)>();
}
