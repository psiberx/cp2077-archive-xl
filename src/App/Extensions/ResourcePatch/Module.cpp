#include "Module.hpp"
#include "Red/EntityBuilder.hpp"
#include "Red/Mesh.hpp"
#include "Red/ResourceDepot.hpp"

namespace
{
constexpr auto ModuleName = "ResourcePatch";
}

std::string_view App::ResourcePatchModule::GetName()
{
    return ModuleName;
}

bool App::ResourcePatchModule::Load()
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

    PreparePatches();

    return true;
}

void App::ResourcePatchModule::Reload()
{
    PreparePatches();
}

bool App::ResourcePatchModule::Unload()
{
    Unhook<Raw::EntityTemplate::OnLoad>();
    Unhook<Raw::EntityBuilder::ExtractComponentsJob>();
    Unhook<Raw::AppearanceResource::OnLoad>();
    Unhook<Raw::CMesh::OnLoad>();

    return true;
}

void App::ResourcePatchModule::PreparePatches()
{
    s_patches.clear();

    auto depot = Red::ResourceDepot::Get();
    Core::Set<Red::ResourcePath> invalidPaths;

    for (auto& unit : m_units)
    {
        for (const auto& [targetPath, patchList] : unit.patches)
        {
            for (const auto& patchPath : patchList)
            {
                if (!depot->ResourceExists(patchPath))
                {
                    if (!invalidPaths.contains(patchPath))
                    {
                        LogWarning("|{}| Resource \"{}\" doesn't exist. Skipped.", ModuleName, unit.paths[patchPath]);
                        invalidPaths.insert(patchPath);
                    }
                    continue;
                }

                s_patches[targetPath].insert(patchPath);
            }
        }
    }
}

void App::ResourcePatchModule::OnResourceRequest(Red::ResourceDepot*, const uintptr_t* aOut, Red::ResourcePath aPath,
                                                 const int32_t*)
{
    if (*aOut)
    {
        const auto& patchIt = s_patches.find(aPath);
        if (patchIt != s_patches.end())
        {
            std::unique_lock _(s_tokenLock);
            for (const auto& patchPath : patchIt.value())
            {
                if (!s_tokens.contains(patchPath))
                {
                    s_tokens[patchPath] = Red::ResourceLoader::Get()->LoadAsync(patchPath);
                }
            }
        }
    }
}

void App::ResourcePatchModule::OnEntityTemplateLoad(Red::EntityTemplate* aTemplate, void*)
{
    const auto& patchIt = s_patches.find(aTemplate->path);
    if (patchIt == s_patches.end())
        return;

    std::shared_lock _(s_tokenLock);
    for (const auto& patchPath : patchIt.value())
    {
        auto patchTemplate = GetPatchResource<Red::EntityTemplate>(patchPath);
        if (!patchTemplate)
            continue;

        for (const auto& patchAppearance : patchTemplate->appearances)
        {
            auto isNewAppearance = true;

            for (auto& existingAppearance : aTemplate->appearances)
            {
                if (existingAppearance.name == patchAppearance.name)
                {
                    existingAppearance = patchAppearance;
                    isNewAppearance = false;
                    break;
                }
            }

            if (isNewAppearance)
            {
                aTemplate->appearances.EmplaceBack(patchAppearance);
            }
        }

        if (patchTemplate->visualTagsSchema)
        {
            if (!aTemplate->visualTagsSchema)
            {
                aTemplate->visualTagsSchema = {};
            }

            aTemplate->visualTagsSchema->visualTags.Add(patchTemplate->visualTagsSchema->visualTags);
        }
    }
}

void App::ResourcePatchModule::OnEntityTemplateExtract(void** aEntityBuilder, void* a2)
{
    bool isNewEntity = Raw::EntityBuilder::Flags::Ref(*aEntityBuilder) & 1;

    if (!isNewEntity)
        return;

    const auto& entityTemplate = Raw::EntityBuilder::Template::Ref(*aEntityBuilder);

    if (!entityTemplate)
        return;

    const auto& entityTemplatePath = entityTemplate->path;

    const auto& patchIt = s_patches.find(entityTemplatePath);
    if (patchIt == s_patches.end())
        return;

    std::shared_lock _(s_tokenLock);
    for (const auto& pathPath : patchIt.value())
    {
        auto patchToken = GetPatchToken<Red::EntityTemplate>(pathPath);

        if (!patchToken)
            continue;

        Red::DynArray<Red::Handle<Red::ISerializable>> bufferObjects;

        Raw::EntityTemplate::BufferMask::Ref(patchToken->resource) = -1;
        Raw::EntityTemplate::ExtractBufferObjects(bufferObjects, patchToken);

        if (bufferObjects.size > 0)
        {
            auto& entity = Raw::EntityBuilder::Entity::Ref(*aEntityBuilder);
            auto& entityComponents = Raw::EntityBuilder::Components::Ref(*aEntityBuilder);

            for (auto& bufferObject : bufferObjects)
            {
                if (auto patchComponent = Red::Cast<Red::IComponent>(bufferObject))
                {
                    auto isNewComponent = true;

                    for (auto& entityComponent : entityComponents)
                    {
                        if (entityComponent->name == patchComponent->name &&
                            entityComponent->id.unk00 == patchComponent->id.unk00)
                        {
                            entityComponent = patchComponent;
                            isNewComponent = false;
                            break;
                        }
                    }

                    if (isNewComponent)
                    {
                        entityComponents.EmplaceBack(patchComponent);
                    }
                }
                else if (auto patchEntity = Red::Cast<Red::Entity>(bufferObject))
                {
                    entity = patchEntity;
                }
            }
        }
    }
}

void App::ResourcePatchModule::OnAppearanceResourceLoad(Red::AppearanceResource* aResource)
{
    const auto& patchIt = s_patches.find(aResource->path);
    if (patchIt == s_patches.end())
        return;

    std::shared_lock _(s_tokenLock);
    for (const auto& patchPath : patchIt.value())
    {
        auto patchResource = GetPatchResource<Red::AppearanceResource>(patchPath);
        if (!patchResource)
            continue;

        for (const auto& patchAppearance : patchResource->appearances)
        {
            auto isNewAppearance = true;

            for (auto& existingAppearance : aResource->appearances)
            {
                if (existingAppearance->name == patchAppearance->name)
                {
                    existingAppearance = patchAppearance;
                    isNewAppearance = false;
                    break;
                }
            }

            if (isNewAppearance)
            {
                aResource->appearances.EmplaceBack(patchAppearance);
            }
        }
    }
}

void App::ResourcePatchModule::OnMeshResourceLoad(Red::CMesh* aMesh, void*)
{
    const auto& patchIt = s_patches.find(aMesh->path);
    if (patchIt == s_patches.end())
        return;

    std::shared_lock _(s_tokenLock);
    for (const auto& patchPath : patchIt.value())
    {
        auto patchMesh = GetPatchResource<Red::CMesh>(patchPath);
        if (!patchMesh)
            continue;

        for (const auto& patchAppearance : patchMesh->appearances)
        {
            auto isNewAppearance = true;

            for (auto& existingAppearance : aMesh->appearances)
            {
                if (existingAppearance->name == patchAppearance->name)
                {
                    existingAppearance = patchAppearance;
                    isNewAppearance = false;
                    break;
                }
            }

            if (isNewAppearance)
            {
                aMesh->appearances.EmplaceBack(patchAppearance);
            }
        }
    }
}

template<typename T>
Red::Handle<T> App::ResourcePatchModule::GetPatchResource(Red::ResourcePath aPath)
{
    auto token = GetPatchToken<T>(aPath);

    if (!token)
        return {};

    return token->resource;
}

template<typename T>
Red::SharedPtr<Red::ResourceToken<T>> App::ResourcePatchModule::GetPatchToken(Red::ResourcePath aPath)
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
