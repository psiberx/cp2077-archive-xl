#include "Extension.hpp"
#include "App/Extensions/ResourceMeta/Extension.hpp"
#include "Core/Facades/Container.hpp"
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
    HookBefore<Raw::AnimatedComponent::InitializeAnimations>(&OnInitializeAnimations).OrThrow();

    s_resourcePathRegistry = Core::Resolve<ResourcePathRegistry>();

    return true;
}

bool App::AnimationExtension::Unload()
{
    Unhook<Raw::AnimatedComponent::InitializeAnimations>();

    return true;
}

void App::AnimationExtension::Configure()
{
    s_animsByTarget.clear();

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

            for (const auto& targetPathStr : ResourceMetaExtension::ExpandList(animation.entities))
            {
                auto targetPath = Red::ResourcePath(targetPathStr.data());

                if (!depot->ResourceExists(targetPath))
                    continue;

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

                s_animsByTarget[targetHash].emplace_back(std::move(animSetupEntry));
            }
        }
    }
}

void App::AnimationExtension::OnInitializeAnimations(Red::entAnimatedComponent* aComponent)
{
    const auto& templatePath = aComponent->owner->templatePath;

    auto anims = s_animsByTarget.find(templatePath);
    if (anims == s_animsByTarget.end())
    {
        anims = s_animsByTarget.find(Red::FNV1a64(aComponent->name.ToString(), templatePath));
        if (anims == s_animsByTarget.end())
            return;
    }

    LogInfo("[{}] Initializing animations for \"{}:{}\"...",
            ExtensionName, s_resourcePathRegistry->ResolvePathOrHash(templatePath), aComponent->name.ToString());

    for (const auto& anim : anims.value())
    {
        LogInfo("[{}] Merging animations from \"{}\" with priority {}...",
                ExtensionName, s_resourcePathRegistry->ResolvePathOrHash(anim.animSet.path), anim.priority);

        aComponent->animations.gameplay.PushBack(anim);
    }

    LogInfo("[{}] All animations merged.", ExtensionName);
}
