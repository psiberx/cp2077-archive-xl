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
    union
    {
        struct
        {
            uint64_t nodeIndex : 19;           // 70
            uint64_t unk70_19 : 29;            // 70
            uint64_t unk70_48 : 16;            // 70
            float secondaryRefPointDistance;   // 78
            float streamingDistance;           // 7C
            uint32_t instanceIndex;            // 80
            worldStreamingDataGroup dataGroup; // 84
            uint8_t unk85[0x90 - 0x85];        // 85
        };
        struct
        {
            float secondaryRefPointDistance; // 70
            float streamingDistance;         // 74
            uint16_t nodeIndex;              // 78
            uint16_t unk7A;                  // 7A
            uint16_t unk7C;                  // 7C
            uint64_t unk80;                  // 80
            uint64_t unk88;                  // 88
        } v62;
    };
};
RED4EXT_ASSERT_SIZE(CompiledNodeInstanceSetupInfo, 0x90);
RED4EXT_ASSERT_OFFSET(CompiledNodeInstanceSetupInfo, node, 0x50);
RED4EXT_ASSERT_OFFSET(CompiledNodeInstanceSetupInfo, globalNodeID, 0x58);
RED4EXT_ASSERT_OFFSET(CompiledNodeInstanceSetupInfo, proxyNodeID, 0x60);
RED4EXT_ASSERT_OFFSET(CompiledNodeInstanceSetupInfo, secondaryRefPointDistance, 0x78);
RED4EXT_ASSERT_OFFSET(CompiledNodeInstanceSetupInfo, streamingDistance, 0x7C);
RED4EXT_ASSERT_OFFSET(CompiledNodeInstanceSetupInfo, instanceIndex, 0x80);

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

    [[nodiscard]] CompiledNodeInstanceSetupInfo* Get(int64_t aIndex) const
    {
        return begin() + aIndex;
    }

    uint32_t GetCount()
    {
        return buffer.size / sizeof(CompiledNodeInstanceSetupInfo);
    }
};
RED4EXT_ASSERT_SIZE(CompiledNodeInstanceSetupInfoBuffer, 0x28);

struct StreamingSectorNodeBuffer
{
    CompiledNodeInstanceSetupInfoBuffer nodeSetups; // 00
    uint32_t version;                               // 28
    DynArray<Handle<worldNode>> nodes;              // 30
    DynArray<NodeRef> nodeRefs;                     // 40
};
RED4EXT_ASSERT_OFFSET(StreamingSectorNodeBuffer, nodeSetups, 0);
RED4EXT_ASSERT_OFFSET(StreamingSectorNodeBuffer, version, 0x28);
RED4EXT_ASSERT_OFFSET(StreamingSectorNodeBuffer, nodes, 0x30);
RED4EXT_ASSERT_OFFSET(StreamingSectorNodeBuffer, nodeRefs, 0x40);

struct CollisionActor
{
    WorldTransform transform;   // 00
    uint16_t shapeStartIndex;   // 20
    uint16_t numShapes;         // 22
    int16_t scaleIndex;         // 24
    uint8_t unk26[0x30 - 0x26]; // 26
};
RED4EXT_ASSERT_SIZE(CollisionActor, 0x30);

struct CollisionShape
{
    physicsGeometryKey geometryKey; // 00
    uint16_t unk10;                 // 10
    uint16_t unk12;                 // 12
    int16_t positionIndex;          // 14
    int16_t rotationIndex;          // 16
    uint8_t presetIndex;            // 18
    uint8_t unk19[0x20 - 0x19];     // 19
};
RED4EXT_ASSERT_SIZE(CollisionShape, 0x20);

struct TransformBuffer : DataBuffer
{
    [[nodiscard]] inline Transform* begin() const
    {
        return reinterpret_cast<Transform*>(buffer.data);
    }

    [[nodiscard]] inline Transform* end() const
    {
        return reinterpret_cast<Transform*>(reinterpret_cast<uintptr_t>(buffer.data) + buffer.size);
    }

    [[nodiscard]] Transform* Get(int64_t aIndex) const
    {
        return begin() + aIndex;
    }

    uint32_t GetCount()
    {
        return buffer.size / sizeof(Transform);
    }
};
RED4EXT_ASSERT_SIZE(TransformBuffer, 0x28);

struct WorldTransformBuffer : DataBuffer
{
    [[nodiscard]] inline worldNodeTransform* begin() const
    {
        return reinterpret_cast<worldNodeTransform*>(buffer.data);
    }

    [[nodiscard]] inline worldNodeTransform* end() const
    {
        return reinterpret_cast<worldNodeTransform*>(reinterpret_cast<uintptr_t>(buffer.data) + buffer.size);
    }

    [[nodiscard]] worldNodeTransform* Get(int64_t aIndex) const
    {
        return begin() + aIndex;
    }

    uint32_t GetCount()
    {
        return buffer.size / sizeof(worldNodeTransform);
    }
};
RED4EXT_ASSERT_SIZE(WorldTransformBuffer, 0x28);

struct RenderProxyTransform
{
    WorldTransform transform;
    Vector3 scale;
};

struct RenderProxyTransformData : DataBuffer
{
    [[nodiscard]] inline RenderProxyTransform* begin() const
    {
        return reinterpret_cast<RenderProxyTransform*>(buffer.data);
    }

    [[nodiscard]] inline RenderProxyTransform* end() const
    {
        return reinterpret_cast<RenderProxyTransform*>(reinterpret_cast<uintptr_t>(buffer.data) + buffer.size);
    }

    [[nodiscard]] RenderProxyTransform* Get(int64_t aIndex) const
    {
        return begin() + aIndex;
    }

    uint32_t GetCount()
    {
        return buffer.size / sizeof(RenderProxyTransform);
    }
};
RED4EXT_ASSERT_SIZE(RenderProxyTransformData, 0x28);

struct InstancedMeshNodeCookedData
{
    Box boundingBox;
    ResourceAsyncReference<CMesh> mesh;
    CName meshAppearance;
    float forcedAutoHideDistance;
    uint32_t lodLevelScales;
    shadowsShadowCastingMode castShadows;
    shadowsShadowCastingMode castLocalShadows;
    RenderSceneLayerMask renderSceneLayerMask;
    visWorldOccluderType occluderType;
    uint8_t occluderAutoHideDistanceScale;
    bool removeFromRainMap;
    uint8_t unk38[0x2];
};
RED4EXT_ASSERT_SIZE(InstancedMeshNodeCookedData, 0x40);

struct InstancedMeshNodeCookedRange
{
    uint16_t start;
    uint16_t count;
};
RED4EXT_ASSERT_SIZE(InstancedMeshNodeCookedRange, 0x4);

struct worldInstancedMeshNodeEx : world::CookedNode
{
    DataBuffer cookedWorldTransformsBuffer;                // 38
    DynArray<RenderProxyTransform> transforms;             // 60
    DynArray<InstancedMeshNodeCookedData> cookedInstances; // 60
    DynArray<InstancedMeshNodeCookedRange> cookedRanges;   // 60
    uint8_t version;                                       // 90
    uint8_t unk91[0x98 - 0x91];                            // 91
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
using Shapes = Core::OffsetPtr<0x48, Red::Range<Red::CollisionShape>>;
using Presets = Core::OffsetPtr<0x98, Red::Range<Red::CName>>;
}
