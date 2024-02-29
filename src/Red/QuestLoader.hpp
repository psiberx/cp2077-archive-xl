#pragma once

namespace Raw::QuestLoader
{
constexpr auto ProcessPhaseResource = Core::RawFunc<
    /* addr = */ Red::AddressLib::QuestLoader_ProcessPhaseResource,
    /* type = */ void (*)(void* aLoader, Red::ResourcePath aPhasePath,
                          Red::Handle<Red::questQuestPhaseResource>& aPhaseResource)>();
}
