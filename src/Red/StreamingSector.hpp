#pragma once

#include "Red/Addresses.hpp"

namespace Red
{
struct StreamingSectorNodeBuffer
{
    uint8_t unk00[0x28];
    DynArray<Handle<world::Node>> nodes; // 0x28
};
}

namespace Raw::StreamingSector
{
using NodeBuffer = Core::OffsetPtr<0x40, Red::StreamingSectorNodeBuffer>;

constexpr auto OnReady = Core::RawFunc<
    /* addr = */ 0x14027BCDC - Red::Addresses::ImageBase,
    /* type = */ void (*)(Red::world::StreamingSector* aSector, uint64_t a2)>();
}
