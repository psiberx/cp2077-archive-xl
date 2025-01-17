#include "Extension.hpp"
#include "App/Extensions/ResourceMeta/Extension.hpp"
#include "Red/AnimatedComponent.hpp"

namespace
{
constexpr auto ExtensionName = "EntityAnimation";
}

std::string_view App::AnimationExtension::GetName()
{
    return ExtensionName;
}

bool App::AnimationExtension::Load()
{
    HookBefore<Raw::AnimatedComponent::InitializeAnimations>(&AnimationExtension::OnInitializeAnimations).OrThrow();

    return true;
}

bool App::AnimationExtension::Unload()
{
    Unhook<Raw::AnimatedComponent::InitializeAnimations>();

    return true;
}

void App::AnimationExtension::Configure()
{
    m_animsByTarget.clear();

    auto depot = Red::ResourceDepot::Get();
    Core::Set<Red::ResourcePath> invalidPaths;

    for (const auto& unit : m_configs)
    {
        for (const auto& animation : unit.animations)
        {
            auto animPath = Red::ResourcePath(animation.set.c_str());

            if (!depot->ResourceExists(animPath))
            {
                if (!invalidPaths.contains(animPath))
                {
                    LogError("[{}] Animation \"{}\" doesn't exist. Skipped.", ExtensionName, animation.set);
                    invalidPaths.insert(animPath);
                }
                continue;
            }

            m_paths[animPath] = animation.set;

            Core::Set<Red::ResourcePath> targetList;
            {
                auto originalPath = Red::ResourcePath(animation.entity.c_str());
                const auto& entityList = ResourceMetaExtension::GetResourceList(originalPath);
                if (!entityList.empty())
                {
                    targetList.insert(entityList.begin(), entityList.end());

                    for (const auto& entityPath : entityList)
                    {
                        m_paths[entityPath] = ResourceMetaExtension::GetPathString(entityPath);
                    }
                }
                else
                {
                    targetList.insert(originalPath);

                    m_paths[originalPath] = animation.entity;
                }
            }

            for (const auto& targetPath : targetList)
            {
                if (!depot->ResourceExists(targetPath))
                {
                    if (!invalidPaths.contains(targetPath))
                    {
                        // LogWarning("[{}] Entity \"{}\" doesn't exist. Skipped.", ExtensionName, m_paths[targetPath]);
                        invalidPaths.insert(targetPath);
                    }
                    continue;
                }

                Red::animAnimSetupEntry animSetupEntry;
                animSetupEntry.animSet = animPath;
                animSetupEntry.priority = animation.priority;

                for (const auto& var : animation.variables)
                {
                    animSetupEntry.variableNames.EmplaceBack(var.c_str());
                }

                auto targetHash = targetPath.hash;

                if (!animation.component.empty())
                {
                    targetHash = Red::FNV1a64(animation.component.data(), targetHash);
                }

                m_animsByTarget[targetHash].emplace_back(std::move(animSetupEntry));
            }
        }
    }
}

void App::AnimationExtension::OnInitializeAnimations(Red::entAnimatedComponent* aComponent)
{
    const auto& templatePath = aComponent->owner->templatePath;

    auto anims = m_animsByTarget.find(templatePath);
    if (anims == m_animsByTarget.end())
    {
        anims = m_animsByTarget.find(Red::FNV1a64(aComponent->name.ToString(), templatePath));
        if (anims == m_animsByTarget.end())
            return;
    }

    LogInfo("[{}] Initializing animations for \"{}:{}\"...",
            ExtensionName, m_paths[templatePath], aComponent->name.ToString());

    for (const auto& anim : anims.value())
    {
        LogInfo("[{}] Merging animations from \"{}\" with priority {}...",
                ExtensionName, m_paths[anim.animSet.path], anim.priority);

        aComponent->animations.gameplay.PushBack(anim);
    }

    LogInfo("[{}] All animations merged.", ExtensionName);
}
