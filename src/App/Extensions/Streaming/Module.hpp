#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/Streaming/Unit.hpp"
#include "Red/StreamingWorld.hpp"

namespace App
{
using StreamingBlockRef = Red::ResourceReference<Red::world::StreamingBlock>;

class StreamingModule : public ConfigurableUnitModule<StreamingUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;

private:
    void OnWorldLoad(Red::world::StreamingWorld* aWorld, Red::BaseStream* aStream);
};
}
