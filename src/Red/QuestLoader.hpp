#pragma once

#include "Red/Addresses.hpp"

namespace Raw::QuestLoader
{
constexpr auto ProcessPhaseResource = Core::RawFunc<
    /* addr = */ Red::Addresses::QuestLoader_ProcessPhaseResource,
    /* type = */ void (*)(void* aLoader, Red::ResourcePath aPhasePath,
                          Red::Handle<Red::questQuestPhaseResource>& aPhaseResource)>();
}
