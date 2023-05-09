#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/Animations/Unit.hpp"

namespace App
{
class AnimationsModule : public ConfigurableUnitModule<AnimationsUnit>
{
public:
    std::string_view GetName() override;
    bool Load() override;
    void Reload() override;
    bool Unload() override;

private:
    void PrepareEntries();
    void OnInitializeAnimations(Red::entAnimatedComponent* aComponent);

    Core::Map<Red::ResourcePath, Core::Vector<Red::animAnimSetupEntry>> m_animsByEntity;
    Core::Map<Red::ResourcePath, std::string> m_paths;
};
}
