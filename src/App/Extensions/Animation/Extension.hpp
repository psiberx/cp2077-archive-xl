#pragma once

#include "App/Extensions/ExtensionBase.hpp"
#include "App/Extensions/Animation/Config.hpp"
#include "App/Shared/ResourcePathRegistry.hpp"

namespace App
{
class AnimationExtension : public ConfigurableExtensionImpl<AnimationsConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;
    void Configure() override;

private:
    static void OnInitializeAnimations(Red::entAnimatedComponent* aComponent);

    inline static Core::Map<uint64_t, Core::Vector<Red::animAnimSetupEntry>> s_animsByTarget;
    inline static Core::SharedPtr<ResourcePathRegistry> s_resourcePathRegistry;
};
}
