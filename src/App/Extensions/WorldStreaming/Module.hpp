#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/WorldStreaming/Config.hpp"
#include "Red/StreamingSector.hpp"
#include "Red/StreamingWorld.hpp"

namespace App
{
using StreamingBlockRef = Red::ResourceReference<Red::world::StreamingBlock>;

class WorldStreamingModule : public ConfigurableModuleImpl<WorldStreamingConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;
    void Configure() override;

private:
    void OnWorldLoad(Red::world::StreamingWorld* aWorld, Red::BaseStream* aStream);
    static void OnSectorReady(Red::world::StreamingSector* aSector, uint64_t);
    static bool PatchSector(Red::world::StreamingSector* aSector, const WorldSectorMod& aSectorMod);

    inline static Core::Map<Red::ResourcePath, Core::Vector<WorldSectorMod>> s_sectors;
    inline static Red::SharedSpinLock s_sectorsLock;
};
}
