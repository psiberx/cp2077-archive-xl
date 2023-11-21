#pragma once

#include "Red/Addresses.hpp"

namespace Raw::QuestPhase
{
constexpr auto SetLoadedResource = Core::RawFunc<
    /* addr = */ 0x1404D8858 - Red::Addresses::ImageBase, // FIXME
    /* type = */ void (*)(Red::questPhaseNodeDefinition* aPhaseNode,
                          Red::Handle<Red::questQuestPhaseResource>& aPhaseResource)>();
}

namespace Raw::QuestSocketDefinition
{
using OwnerNode = Core::OffsetPtr<0x48, Red::WeakHandle<Red::questNodeDefinition>>;
}
