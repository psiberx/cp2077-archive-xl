#pragma once

namespace Raw::PersistencySystem
{
constexpr auto SetPersistentStateData = Core::RawFunc<
    /* addr = */ Red::AddressLib::PersistencySystem_SetPersistentStateData,
    /* type = */ void (*)(uint64_t a1, Red::DataBuffer& aData, uint64_t a3, uint32_t a4)>();
}
