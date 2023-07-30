#include "Module.hpp"
#include "Red/AppearanceChanger.hpp"
#include "Red/FactoryIndex.hpp"

namespace
{
constexpr auto ModuleName = "AppearanceSwap";

constexpr auto DefaultAppearanceName = Red::CName("default");
constexpr auto RandomAppearanceName = Red::CName("random");
constexpr auto EmptyAppearanceName = Red::CName("empty_appearance_default");
constexpr auto EmptyAppearancePath = Red::ResourcePath(R"(base\characters\appearances\player\items\empty_appearance.app)");

constexpr auto AutoHideAppearanceTags = {
    std::pair(Red::CName("hide_for_FPP"), "&FPP"),
    std::pair(Red::CName("hide_for_Male"), "&Male"),
    std::pair(Red::CName("hide_for_Female"), "&Female"),
};

constexpr auto MagicAppearanceMarker = '!';

constexpr auto EntityOffset = 0xD0; // todo: use RawPtrs
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

    if (!HookAfter<Raw::AppearanceResource::FindAppearance>(&OnFindDefinition))
        throw std::runtime_error("Failed to hook [AppearanceResource::FindAppearance].");

    if (!Hook<Raw::AppearanceChangeRequest::LoadEntityTemplate>(&OnLoadEntityTemplate))
        throw std::runtime_error("Failed to hook [AppearanceChangeRequest::LoadEntityTemplate].");

    s_emptyAppearance = Core::MakeUnique<Red::TemplateAppearance>();
    s_emptyAppearance->name = EmptyAppearanceName;
    s_emptyAppearance->appearanceName = DefaultAppearanceName;
    s_emptyAppearance->appearanceResource = EmptyAppearancePath;

    return true;
}

bool App::AppearanceSwapModule::Unload()
{
    Unhook<Raw::EntityTemplate::FindAppearance>();
    Unhook<Raw::AppearanceResource::FindAppearance>();
    Unhook<Raw::AppearanceChangeRequest::LoadEntityTemplate>();

    return true;
}

Red::TemplateAppearance* App::AppearanceSwapModule::OnFindAppearance(Red::EntityTemplate* aTemplate,
                                                                     Red::CName aAppearanceName)
{
    if (aAppearanceName == EmptyAppearanceName)
        return s_emptyAppearance.get();

    auto appearance = Raw::EntityTemplate::FindAppearance(aTemplate, aAppearanceName);

    if (appearance)
        return appearance;

    if (!aAppearanceName || aAppearanceName == DefaultAppearanceName || aAppearanceName == RandomAppearanceName)
        return nullptr;

    const std::string_view appearanceNameStr = aAppearanceName.ToString();

    const auto magicMarkerPos = appearanceNameStr.find(MagicAppearanceMarker);
    if (magicMarkerPos != std::string_view::npos)
    {
        const auto baseAppearanceName = Red::FNV1a64(reinterpret_cast<const uint8_t*>(appearanceNameStr.data()),
                                                     magicMarkerPos);
        const auto baseAppearance = Raw::EntityTemplate::FindAppearance(aTemplate, baseAppearanceName);
        if (baseAppearance)
        {
            aTemplate->appearances.EmplaceBack(*baseAppearance);
            appearance = aTemplate->appearances.End() - 1;
            appearance->name = aAppearanceName;
            appearance->appearanceName = aAppearanceName;
            return appearance;
        }
    }

    if (aTemplate->visualTagsSchema)
    {
        const auto& visualTags = aTemplate->visualTagsSchema->visualTags;
        if (!visualTags.IsEmpty())
        {
            for (const auto& tag : AutoHideAppearanceTags)
            {
                if (visualTags.Contains(tag.first) && appearanceNameStr.find(tag.second) != std::string_view::npos)
                {
                    aTemplate->appearances.EmplaceBack(*s_emptyAppearance);
                    appearance = aTemplate->appearances.End() - 1;
                    appearance->name = aAppearanceName;
                    return appearance;
                }
            }
        }
    }

    return nullptr;
}

void App::AppearanceSwapModule::OnFindDefinition(Red::AppearanceResource* aResource,
                                                 Red::Handle<Red::AppearanceDefinition>* aDefinition,
                                                 Red::CName aAppearanceName, uint32_t a4, uint8_t a5)
{
    if (!*aDefinition)
    {
        const std::string_view appearanceNameStr = aAppearanceName.ToString();
        const auto magicMarkerPos = appearanceNameStr.find(MagicAppearanceMarker);
        if (magicMarkerPos != std::string_view::npos)
        {
            const auto baseAppearanceName = Red::FNV1a64(reinterpret_cast<const uint8_t*>(appearanceNameStr.data()),
                                                         magicMarkerPos);

            Red::Handle<Red::AppearanceDefinition> baseAppearance;
            Raw::AppearanceResource::FindAppearance(aResource, &baseAppearance, baseAppearanceName, 0, 0);

            if (baseAppearance)
            {
                constexpr auto DefinitionDataOffset = sizeof(Red::ISerializable);
                constexpr auto DefinitionDataSize = sizeof(Red::AppearanceDefinition) - DefinitionDataOffset;

                auto definition = Red::MakeHandle<Red::AppearanceDefinition>();
                for (const auto prop : Red::GetClass<Red::AppearanceDefinition>()->props)
                {
                    prop->SetValue(definition.instance, prop->GetValuePtr<void>(baseAppearance.instance));
                }

                definition->name = aAppearanceName;

                {
                    std::unique_lock _(*Raw::AppearanceResource::Mutex(aResource));
                    aResource->appearances.PushBack(definition);
                }

                *aDefinition = std::move(definition);
            }
        }
    }
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
