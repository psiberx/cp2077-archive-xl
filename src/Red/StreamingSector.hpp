#pragma once

#include "Red/Addresses.hpp"
#include "Red/Common.hpp"

namespace Red
{
struct StreamingSectorNodeBuffer
{
    uint8_t unk00[0x28];
    DynArray<Handle<world::Node>> nodes; // 0x28
};

struct CollisionActor
{
    WorldTransform transform;
    uint64_t unk20;
};
}

namespace Raw::StreamingSector
{
using NodeBuffer = Core::OffsetPtr<0x40, Red::StreamingSectorNodeBuffer>;

constexpr auto OnReady = Core::RawFunc<
    /* addr = */ Red::Addresses::StreamingSector_OnReady,
    /* type = */ void (*)(Red::world::StreamingSector* aSector, uint64_t a2)>();
}

namespace Raw::CollisionNode
{
using Actors = Core::OffsetPtr<0x38, Red::SomeIterator<Red::CollisionActor>>;
}
