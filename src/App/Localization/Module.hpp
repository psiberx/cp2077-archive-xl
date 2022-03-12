#pragma once

#include "Raws.hpp"
#include "Unit.hpp"
#include "App/Common/ModuleBase.hpp"

namespace App
{
class LocalizationModule : public ConfigurableUnitModule<LocalizationUnit>
{
public:
    bool Attach() override;
    bool Detach() override;
    std::string_view GetName() override;

private:
    using OnScreenEntries = RED4ext::loc::alization::PersistenceOnScreenEntries;

    uint64_t OnLoadOnScreens(RED4ext::Handle<OnScreenEntries>* aOnScreens, Engine::ResourcePath aPath);
    static bool AppendEntries(RED4ext::Handle<OnScreenEntries>& aOnScreens, const std::string& aPath);

    inline static Raw::LoadOnScreens::type LoadOnScreens;
};
}
