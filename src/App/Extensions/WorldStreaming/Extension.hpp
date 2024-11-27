#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/WorldStreaming/Config.hpp"
#include "Red/StreamingSector.hpp"
#include "Red/StreamingWorld.hpp"

namespace App
{
using StreamingBlockRef = Red::ResourceReference<Red::world::StreamingBlock>;

class WorldStreamingExtension : public ConfigurableExtensionImpl<WorldStreamingConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;
    void Configure() override;

private:
    void OnWorldLoad(Red::world::StreamingWorld* aWorld, Red::BaseStream* aStream);
    static void OnSectorReady(Red::world::StreamingSector* aSector, uint64_t);
    static void OnRegisterSpots(Red::AIWorkspotManager* aManager,
                                const Red::DynArray<Red::AISpotPersistentData>& aNewSpots);

    static bool PatchSector(Red::world::StreamingSector* aSector, const WorldSectorMod& aSectorMod);

    inline static Core::Map<Red::ResourcePath, Core::Vector<WorldSectorMod>> s_sectors;
    inline static Red::SharedSpinLock s_sectorsLock;
};
}
