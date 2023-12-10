#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/WorldStreaming/Unit.hpp"
#include "Red/StreamingSector.hpp"
#include "Red/StreamingWorld.hpp"

namespace App
{
using StreamingBlockRef = Red::ResourceReference<Red::world::StreamingBlock>;

class WorldStreamingModule : public ConfigurableUnitModule<WorldStreamingUnit>
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
    static bool PatchSector(Red::world::StreamingSector* aSector, const WorldSectorMod& aSectorMod);

    inline static Core::Map<Red::ResourcePath, Core::Vector<WorldSectorMod>> s_sectors;
    inline static std::shared_mutex s_sectorsLock;
};
}
