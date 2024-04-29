#include "Module.hpp"
#include "Red/EntityBuilder.hpp"
#include "Red/Mesh.hpp"
#include "Red/ResourceDepot.hpp"

namespace
{
constexpr auto ModuleName = "EntityOverride";
}

std::string_view App::EntityOverrideModule::GetName()
{
    return ModuleName;
}

bool App::EntityOverrideModule::Load()
{
    if (!HookAfter<Raw::ResourceDepot::RequestResource>(&OnResourceRequest))
        throw std::runtime_error("Failed to hook [ResourceDepot::RequestResource].");

    if (!HookAfter<Raw::EntityTemplate::OnLoad>(&OnEntityTemplateLoad))
        throw std::runtime_error("Failed to hook [EntityTemplate::OnLoad].");

    if (!HookAfter<Raw::EntityBuilder::ExtractComponentsJob>(&OnEntityTemplateExtract))
        throw std::runtime_error("Failed to hook [EntityBuilder::ExtractComponentsJob].");

    if (!HookBefore<Raw::AppearanceResource::OnLoad>(&OnAppearanceResourceLoad))
        throw std::runtime_error("Failed to hook [AppearanceResource::OnLoad].");

    if (!HookAfter<Raw::CMesh::OnLoad>(&OnMeshResourceLoad))
        throw std::runtime_error("Failed to hook [EntityBuilder::ExtractComponentsJob].");

    PrepareOverrides();

    return true;
}

void App::EntityOverrideModule::Reload()
{
    PrepareOverrides();
}

bool App::EntityOverrideModule::Unload()
{
    Unhook<Raw::EntityTemplate::OnLoad>();
    Unhook<Raw::EntityBuilder::ExtractComponentsJob>();
    Unhook<Raw::AppearanceResource::OnLoad>();
    Unhook<Raw::CMesh::OnLoad>();

    return true;
}

void App::EntityOverrideModule::PrepareOverrides()
{
    s_overrides.clear();

    auto depot = Red::ResourceDepot::Get();
    Core::Set<Red::ResourcePath> invalidPaths;

    for (const auto& unit : m_units)
    {
        for (const auto& [target, overrides] : unit.overrides)
        {
            for (const auto& override : overrides)
            {
                auto overridePath = Red::ResourcePath(override.c_str());

                if (!depot->ResourceExists(overridePath))
                {
                    if (!invalidPaths.contains(overridePath))
                    {
                        LogWarning("|{}| Resource \"{}\" doesn't exist. Skipped.", ModuleName, override);
                        invalidPaths.insert(overridePath);
                    }
                    continue;
                }

                s_overrides[target].insert(overridePath);
            }
        }
    }
}

void App::EntityOverrideModule::OnResourceRequest(Red::ResourceDepot*, const uintptr_t* aOut, Red::ResourcePath aPath,
                                                  const int32_t*)
{
    if (*aOut)
    {
        const auto& overrideIt = s_overrides.find(aPath);
        if (overrideIt != s_overrides.end())
        {
            std::unique_lock _(s_tokenLock);
            for (const auto& override : overrideIt.value())
            {
                if (!s_tokens.contains(override))
                {
                    s_tokens[override] = Red::ResourceLoader::Get()->LoadAsync(override);
                }
            }
        }
    }
}

void App::EntityOverrideModule::OnEntityTemplateLoad(Red::EntityTemplate* aTemplate, void*)
{
    const auto& overrideIt = s_overrides.find(aTemplate->path);
    if (overrideIt == s_overrides.end())
        return;

    std::shared_lock _(s_tokenLock);
    for (const auto& path : overrideIt.value())
    {
        auto override = GetOverride<Red::EntityTemplate>(path);
        if (!override)
            continue;

        for (const auto& overrideAppearance : override->appearances)
        {
            auto isNewAppearance = true;

            for (auto& existingAppearance : aTemplate->appearances)
            {
                if (existingAppearance.name == overrideAppearance.name)
                {
                    existingAppearance = overrideAppearance;
                    isNewAppearance = false;
                    break;
                }
            }

            if (isNewAppearance)
            {
                aTemplate->appearances.EmplaceBack(overrideAppearance);
            }
        }

        if (override->visualTagsSchema)
        {
            if (!aTemplate->visualTagsSchema)
            {
                aTemplate->visualTagsSchema = {};
            }

            aTemplate->visualTagsSchema->visualTags.Add(override->visualTagsSchema->visualTags);
        }
    }
}

void App::EntityOverrideModule::OnEntityTemplateExtract(void** aEntityBuilder, void* a2)
{
    bool isNewEntity = Raw::EntityBuilder::Flags::Ref(*aEntityBuilder) & 1;

    if (!isNewEntity)
        return;

    const auto& entityTemplate = Raw::EntityBuilder::Template::Ref(*aEntityBuilder);

    if (!entityTemplate)
        return;

    const auto& entityTemplatePath = entityTemplate->path;

    const auto& overrideIt = s_overrides.find(entityTemplatePath);
    if (overrideIt == s_overrides.end())
        return;

    std::shared_lock _(s_tokenLock);
    for (const auto& path : overrideIt.value())
    {
        auto token = GetOverrideToken<Red::EntityTemplate>(path);

        if (!token)
            continue;

        Red::DynArray<Red::Handle<Red::IComponent>> overrideComponents;
        Raw::EntityTemplate::ExtractComponents(overrideComponents, token);

        if (overrideComponents.size > 0)
        {
            auto& entityComponents = Raw::EntityBuilder::Components ::Ref(*aEntityBuilder);
            for (auto& overrideComponent : overrideComponents)
            {
                auto isNewComponent = true;

                for (auto& entityComponent : entityComponents)
                {
                    if (entityComponent->name == overrideComponent->name &&
                        entityComponent->id.unk00 == overrideComponent->id.unk00)
                    {
                        entityComponent = overrideComponent;
                        isNewComponent = false;
                        break;
                    }
                }

                if (isNewComponent)
                {
                    entityComponents.EmplaceBack(std::move(overrideComponent));
                }
            }
        }
    }
}

void App::EntityOverrideModule::OnAppearanceResourceLoad(Red::AppearanceResource* aResource)
{
    const auto& overrideIt = s_overrides.find(aResource->path);
    if (overrideIt == s_overrides.end())
        return;

    std::shared_lock _(s_tokenLock);
    for (const auto& path : overrideIt.value())
    {
        auto override = GetOverride<Red::AppearanceResource>(path);
        if (!override)
            continue;

        for (const auto& overrideAppearance : override->appearances)
        {
            auto isNewAppearance = true;

            for (auto& existingAppearance : aResource->appearances)
            {
                if (existingAppearance->name == overrideAppearance->name)
                {
                    existingAppearance = overrideAppearance;
                    isNewAppearance = false;
                    break;
                }
            }

            if (isNewAppearance)
            {
                aResource->appearances.EmplaceBack(overrideAppearance);
            }
        }
    }
}

void App::EntityOverrideModule::OnMeshResourceLoad(Red::CMesh* aMesh, void*)
{
    const auto& overrideIt = s_overrides.find(aMesh->path);
    if (overrideIt == s_overrides.end())
        return;

    std::shared_lock _(s_tokenLock);
    for (const auto& path : overrideIt.value())
    {
        auto override = GetOverride<Red::CMesh>(path);
        if (!override)
            continue;

        for (const auto& overrideAppearance : override->appearances)
        {
            auto isNewAppearance = true;

            for (auto& existingAppearance : aMesh->appearances)
            {
                if (existingAppearance->name == overrideAppearance->name)
                {
                    existingAppearance = overrideAppearance;
                    isNewAppearance = false;
                    break;
                }
            }

            if (isNewAppearance)
            {
                aMesh->appearances.EmplaceBack(overrideAppearance);
            }
        }
    }
}

template<typename T>
Red::Handle<T> App::EntityOverrideModule::GetOverride(Red::ResourcePath aPath)
{
    auto token = GetOverrideToken<T>(aPath);

    if (!token)
        return {};

    return token->resource;
}

template<typename T>
Red::SharedPtr<Red::ResourceToken<T>> App::EntityOverrideModule::GetOverrideToken(Red::ResourcePath aPath)
{
    auto& token = s_tokens[aPath];

    if (!token->IsFinished())
    {
        Red::WaitForResource(token, std::chrono::milliseconds(2000));
    }

    if (token->IsFailed())
        return {};

    return *reinterpret_cast<Red::SharedPtr<Red::ResourceToken<T>>*>(&token);
}
