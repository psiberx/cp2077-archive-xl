#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "App/Extensions/PuppetState/System.hpp"
#include "App/Extensions/PuppetState/Unit.hpp"

namespace App
{
class PuppetStateModule : public ConfigurableUnitModule<PuppetStateUnit>
{
public:
    bool Load() override;
    void Reload() override;
    bool Unload() override;

    std::string_view GetName() override;

private:
    void FillBodyTypes();

    static void OnLoadTweakDB();
    static void CreateSuffixRecord(Red::TweakDBID aSuffixID, Red::CName aSystemName, Red::CName aFunctionName);
    static void ActivateSuffixRecords(const Core::Vector<Red::TweakDBID>& aSuffixIDs);

    inline static Core::Set<Red::CName> s_bodyTypes;
    friend struct PuppetStateSystem;
};
}
