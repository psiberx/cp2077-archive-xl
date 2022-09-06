#pragma once

#include "Core/Stl.hpp"
#include "Engine/Stl.hpp"

#include <RED4ext/TweakDB.hpp>

namespace TweakDB
{
class FlatPool
{
public:
    static constexpr int32_t InvalidOffset = -1;

    struct Stats
    {
        float initTime = 0.0; // ms
        float updateTime = 0.0; // ms
        size_t poolSize = 0; // bytes
        size_t poolValues = 0;
        size_t knownTypes = 0;
        size_t flatEntries = 0;
    };

    FlatPool();
    explicit FlatPool(RED4ext::TweakDB* aTweakDb);

    int32_t AllocateData(const RED4ext::CStackType& aData);
    int32_t AllocateValue(const RED4ext::CBaseRTTIType* aType, RED4ext::ScriptInstance aValue);
    int32_t AllocateDefault(const RED4ext::CBaseRTTIType* aType);

    RED4ext::CStackType GetData(int32_t aOffset);
    RED4ext::ScriptInstance GetValuePtr(int32_t aOffset);

    [[nodiscard]] Stats GetStats() const;

private:
    struct FlatTypeInfo
    {
        RED4ext::CBaseRTTIType* type;
        uintptr_t offset;
    };

    using FlatValueMap = Core::Map<uint64_t, int32_t>; // Hash -> Offset
    using FlatPoolMap = Core::Map<RED4ext::CName, FlatValueMap>; // TypeName -> Pool
    using FlatDefaultMap = Core::Map<RED4ext::CName, int32_t>; // TypeName -> Offset
    using FlatTypeMap = Core::Map<uintptr_t, FlatTypeInfo>; // VFT -> Type

    void Initialize();
    void UpdateStats(float updateTime = 0);
    void SyncBuffer();

    inline RED4ext::CStackType GetFlatData(int32_t aOffset);
    inline static uint64_t Hash(const RED4ext::CBaseRTTIType* aType, RED4ext::ScriptInstance aValue);

    RED4ext::TweakDB* m_tweakDb;
    uintptr_t m_bufferEnd;
    uintptr_t m_offsetEnd;
    FlatPoolMap m_pools;
    FlatDefaultMap m_defaults;
    FlatTypeMap m_vfts;
    Stats m_stats;
};
}
