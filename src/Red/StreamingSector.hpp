#pragma once

#include "Red/Common.hpp"

namespace Red
{
struct __declspec(align(0x10)) CompiledNodeInstanceSetupInfo
{
    Transform transform;               // 00
    Vector3 scale;                     // 20
    Vector3 secondaryRefPointPosition; // 2C
    Vector3 streamingRefPoint;         // 38
    Vector3 unk44;                     // 44
    worldNode* node;                   // 50
    uint64_t globalNodeID;             // 58
    uint64_t proxyNodeID;              // 60
    ResourcePath unk68;                // 68
    float secondaryRefPointDistance;   // 70
    float streamingDistance;           // 74
    uint16_t nodeIndex;                // 78
    uint16_t unk7A;                    // 7A
    uint16_t unk7C;                    // 7C
    uint16_t unk7E;                    // 7E
    uint64_t unk80;                    // 80
    uint64_t unk88;                    // 88
};
RED4EXT_ASSERT_SIZE(CompiledNodeInstanceSetupInfo, 0x90);
RED4EXT_ASSERT_OFFSET(CompiledNodeInstanceSetupInfo, node, 0x50);
RED4EXT_ASSERT_OFFSET(CompiledNodeInstanceSetupInfo, globalNodeID, 0x58);
RED4EXT_ASSERT_OFFSET(CompiledNodeInstanceSetupInfo, nodeIndex, 0x78);

struct CompiledNodeInstanceSetupInfoBuffer : DataBuffer
{
    [[nodiscard]] inline CompiledNodeInstanceSetupInfo* begin() const
    {
        return reinterpret_cast<CompiledNodeInstanceSetupInfo*>(buffer.data);
    }

    [[nodiscard]] inline CompiledNodeInstanceSetupInfo* end() const
    {
        return reinterpret_cast<CompiledNodeInstanceSetupInfo*>(reinterpret_cast<uintptr_t>(buffer.data) +
                                                                buffer.size);
    }

    [[nodiscard]] CompiledNodeInstanceSetupInfo* GetInstance(int64_t aIndex) const
    {
        return begin() + aIndex;
    }

    uint32_t GetInstanceCount()
    {
        return buffer.size / sizeof(CompiledNodeInstanceSetupInfo);
    }
};
RED4EXT_ASSERT_SIZE(CompiledNodeInstanceSetupInfoBuffer, 0x28);

struct StreamingSectorNodeBuffer
{
    CompiledNodeInstanceSetupInfoBuffer nodeSetups; // 00
    DynArray<Handle<worldNode>> nodes;              // 28
    DynArray<NodeRef> nodeRefs;                     // 38
};
RED4EXT_ASSERT_OFFSET(StreamingSectorNodeBuffer, nodes, 0x28);
RED4EXT_ASSERT_OFFSET(StreamingSectorNodeBuffer, nodeRefs, 0x38);

struct CollisionActor
{
    WorldTransform transform;
    uint64_t unk20;
};
}

namespace Raw::StreamingSector
{
using NodeBuffer = Core::OffsetPtr<0x40, Red::StreamingSectorNodeBuffer>;

constexpr auto PostLoad = Core::RawFunc<
    /* addr = */ Red::AddressLib::StreamingSector_PostLoad,
    /* type = */ void (*)(Red::worldStreamingSector* aSector, uint64_t a2)>();
}

namespace Raw::CollisionNode
{
using Actors = Core::OffsetPtr<0x38, Red::Range<Red::CollisionActor>>;
}
