#include "Module.hpp"
#include "Red/AppearanceChanger.hpp"
#include "Red/FactoryIndex.hpp"

namespace
{
constexpr auto ModuleName = "AppearanceSwap";

constexpr auto DefaultApperanceName = Red::CName("default");
constexpr auto EmptyApperanceName = Red::CName("empty_appearance_default");
constexpr auto EmptyApperancePath = Red::ResourcePath(R"(base\characters\appearances\player\items\empty_appearance.app)");

constexpr auto FPPSuffixValue = "&FPP";
constexpr auto DisableFPPTag = Red::CName("disable_FPP");

constexpr auto EntityOffset = 0xD0;
constexpr auto EntityPathOffset = 0x60;
constexpr auto EntityNameFlat = ".entityName";
constexpr auto RecordOffset = 0x100;
constexpr auto FactoryOffset = 0x120;
}

std::string_view App::AppearanceSwapModule::GetName()
{
    return ModuleName;
}

bool App::AppearanceSwapModule::Load()
{
    if (!Hook<Raw::EntityTemplate::FindAppearance>(&OnFindAppearance))
        throw std::runtime_error("Failed to hook [EntityTemplate::FindAppearance].");

    if (!Hook<Raw::AppearanceChangeRequest::LoadEntityTemplate>(&OnLoadEntityTemplate))
        throw std::runtime_error("Failed to hook [AppearanceChangeRequest::LoadEntityTemplate].");

    s_emptyApperance = Core::MakeUnique<Red::TemplateAppearance>();
    s_emptyApperance->name = EmptyApperanceName;
    s_emptyApperance->appearanceName = DefaultApperanceName;
    s_emptyApperance->appearanceResource = EmptyApperancePath;

    return true;
}

bool App::AppearanceSwapModule::Unload()
{
    Unhook<Raw::EntityTemplate::FindAppearance>();
    Unhook<Raw::AppearanceChangeRequest::LoadEntityTemplate>();

    return true;
}

Red::TemplateAppearance* App::AppearanceSwapModule::OnFindAppearance(Red::EntityTemplate* aTemplate, Red::CName aName)
{
    if (aName == EmptyApperanceName)
        return s_emptyApperance.get();

    auto appearance = Raw::EntityTemplate::FindAppearance(aTemplate, aName);

    if (!appearance && aName != DefaultApperanceName && aTemplate->visualTagsSchema)
    {
        const auto& visualTags = aTemplate->visualTagsSchema->visualTags;
        if (!visualTags.IsEmpty())
        {
            if (visualTags.Contains(DisableFPPTag))
            {
                const std::string_view nameStr = aName.ToString();
                if (nameStr.find(FPPSuffixValue) != std::string_view::npos)
                {
                    aTemplate->appearances.EmplaceBack(*s_emptyApperance);
                    appearance = aTemplate->appearances.End() - 1;
                    appearance->name = aName;
                }
            }
        }
    }

    return appearance;
}

bool App::AppearanceSwapModule::OnLoadEntityTemplate(uintptr_t aRequest)
{
    Red::ResourcePath originalPath;

    auto targetEntity = *reinterpret_cast<uintptr_t*>(aRequest + EntityOffset);
    auto visualRecord = *reinterpret_cast<Red::gamedataTweakDBRecord**>(aRequest + RecordOffset);
    auto entityFactory = *reinterpret_cast<uintptr_t*>(aRequest + FactoryOffset);

    if (entityFactory && targetEntity && visualRecord)
    {
        auto tweakDB = Red::TweakDB::Get();
        auto entityFlat = tweakDB->GetFlatValue({visualRecord->recordID, EntityNameFlat});
        if (entityFlat)
        {
            Red::ResourcePath overridePath;
            Raw::FactoryIndex::ResolveResource(entityFactory, overridePath, *entityFlat->GetValue<Red::CName>());

            if (overridePath)
            {
                originalPath = *reinterpret_cast<Red::ResourcePath*>(targetEntity + EntityPathOffset);
                *reinterpret_cast<Red::ResourcePath*>(targetEntity + EntityPathOffset) = overridePath;
            }
        }
    }

    bool result = Raw::AppearanceChangeRequest::LoadEntityTemplate(aRequest);

    if (originalPath)
    {
        *reinterpret_cast<Red::ResourcePath*>(targetEntity + EntityPathOffset) = originalPath;
    }

    return result;
}
