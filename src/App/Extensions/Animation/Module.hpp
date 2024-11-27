#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/Animation/Config.hpp"

namespace App
{
class AnimationModule : public ConfigurableModuleImpl<AnimationsConfig>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    bool Unload() override;
    void Configure() override;

private:
    void OnInitializeAnimations(Red::entAnimatedComponent* aComponent);

    Core::Map<uint64_t, Core::Vector<Red::animAnimSetupEntry>> m_animsByTarget;
    Core::Map<Red::ResourcePath, std::string> m_paths;
};
}
