#include "Module.hpp"
#include "Red/TweakDB.hpp"

namespace
{
constexpr auto ModuleName = "PuppetState";

constexpr auto BodyTypeSuffixID = Red::TweakDBID("itemsFactoryAppearanceSuffix.BodyType");
constexpr auto SuffixListID = Red::TweakDBID("itemsFactoryAppearanceSuffix.ItemsFactoryAppearanceSuffixOrderDefault");
}

std::string_view App::PuppetStateModule::GetName()
{
    return ModuleName;
}

bool App::PuppetStateModule::Load()
{
    if (!HookAfter<Raw::LoadTweakDB>(&OnLoadTweakDB))
        throw std::runtime_error("Failed to hook [LoadTweakDB].");

    FillBodyTypes();

    return true;
}

void App::PuppetStateModule::Reload()
{
    FillBodyTypes();
}

bool App::PuppetStateModule::Unload()
{
    Unhook<Raw::LoadTweakDB>();

    return true;
}

void App::PuppetStateModule::FillBodyTypes()
{
    s_bodyTypes.clear();

    for (const auto& unit : m_units)
    {
        for (const auto& bodyType : unit.bodyTypes)
        {
            s_bodyTypes.emplace(bodyType.c_str());
        }
    }
}

void App::PuppetStateModule::OnLoadTweakDB()
{
    CreateSuffixRecord(BodyTypeSuffixID, Red::GetTypeName<PuppetStateSystem>(), "GetBodyTypeSuffix");
    ActivateSuffixRecords({BodyTypeSuffixID});
}

void App::PuppetStateModule::CreateSuffixRecord(Red::TweakDBID aSuffixID, Red::CName aSystemName,
                                                Red::CName aFunctionName)
{
    if (!Red::RecordExists(aSuffixID))
    {
        Red::CreateFlat(aSuffixID, ".instantSwitch", false);
        Red::CreateFlat(aSuffixID, ".scriptedSystem", aSystemName);
        Red::CreateFlat(aSuffixID, ".scriptedFunction", aFunctionName);
        Red::CreateRecord(aSuffixID, "ItemsFactoryAppearanceSuffixBase");
    }
}

void App::PuppetStateModule::ActivateSuffixRecords(const Core::Vector<Red::TweakDBID>& aSuffixIDs)
{
    Red::AppendToFlat(SuffixListID, ".appearanceSuffixes", aSuffixIDs);
    Red::UpdateRecord(SuffixListID);
}
