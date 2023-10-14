#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/Streaming/Unit.hpp"
#include "Red/StreamingSector.hpp"
#include "Red/StreamingWorld.hpp"

namespace App
{
using StreamingBlockRef = Red::ResourceReference<Red::world::StreamingBlock>;

class StreamingModule : public ConfigurableUnitModule<StreamingUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    void Reload() override;
    bool Unload() override;

private:
    void PrepareSectors();
    void OnWorldLoad(Red::world::StreamingWorld* aWorld, Red::BaseStream* aStream);
    static void OnSectorReady(Red::world::StreamingSector* aSector, uint64_t);

    inline static Core::Map<Red::ResourcePath, Core::Vector<StreamingSectorMod>> s_sectors;
};
}
