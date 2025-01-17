#include "Extension.hpp"
#include "Core/Facades/Container.hpp"
#include "Red/FactoryIndex.hpp"

namespace
{
constexpr auto ExtensionName = "FactoryIndex";
constexpr auto LastFactory = Red::ResourcePath(R"(base\gameplay\factories\vehicles\vehicles.csv)");
}

std::string_view App::FactoryIndexExtension::GetName()
{
    return ExtensionName;
}

bool App::FactoryIndexExtension::Load()
{
    HookAfter<Raw::FactoryIndex::LoadFactoryAsync>(&OnLoadFactoryAsync).OrThrow();

    s_resourcePathRegistry = Core::Resolve<ResourcePathRegistry>();

    return true;
}

bool App::FactoryIndexExtension::Unload()
{
    Unhook<Raw::FactoryIndex::LoadFactoryAsync>();

    return true;
}

void App::FactoryIndexExtension::Configure()
{
    s_factories.clear();

    auto depot = Red::ResourceDepot::Get();

    Core::Set<Red::ResourcePath> invalidPaths;

    for (const auto& unit : m_configs)
    {
        for (const auto& factoryPathStr : unit.factories)
        {
            auto factoryPath = Red::ResourcePath(factoryPathStr.c_str());

            if (!depot->ResourceExists(factoryPath))
            {
                if (!invalidPaths.contains(factoryPath))
                {
                    LogError("[{}] Factory \"{}\" doesn't exist. Skipped.", ExtensionName, factoryPathStr);
                    invalidPaths.insert(factoryPath);
                }
                continue;
            }

            s_factories.insert_or_assign(factoryPath, factoryPathStr);
        }
    }

    for (const auto& [knownPath, knownPathStr] : s_factories)
    {
        s_resourcePathRegistry->RegisterPath(knownPath, knownPathStr);
    }

    m_configs.clear();
}

void App::FactoryIndexExtension::OnLoadFactoryAsync(uintptr_t aIndex, Red::ResourcePath aPath, uintptr_t aContext)
{
    if (aPath != LastFactory)
        return;

    LogInfo("[{}] Initializing factory index...", ExtensionName);

    if (!s_factories.empty())
    {
        for (const auto& [factoryPath, factoryPathStr] : s_factories)
        {
            LogInfo("[{}] Loading factory \"{}\"...", ExtensionName, factoryPathStr);

            Raw::FactoryIndex::LoadFactoryAsync(aIndex, factoryPath, aContext);
        }

        LogInfo("[{}] All factories loaded.", ExtensionName);
    }
    else
    {
        LogInfo("[{}] No factories to load.", ExtensionName);
    }
}
