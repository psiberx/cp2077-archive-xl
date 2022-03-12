#include "Module.hpp"
#include "Language.hpp"

namespace
{
constexpr auto ModuleName = "Localization";
constexpr uint64_t MinCustomPrimaryID = 300000;
}

std::string_view App::LocalizationModule::GetName()
{
    return ModuleName;
}

bool App::LocalizationModule::Attach()
{
    if (!Hook<Raw::LoadOnScreens>(&LocalizationModule::OnLoadOnScreens, &LocalizationModule::LoadOnScreens))
    {
        LogError("[{}] Failed to hook localization loader.", ModuleName);
        return false;
    }

    return true;
}

bool App::LocalizationModule::Detach()
{
    if (!Unhook<Raw::LoadOnScreens>())
    {
        LogError("[{}] Failed to unhook localization loader.", ModuleName);
        return false;
    }

    return true;
}

uint64_t App::LocalizationModule::OnLoadOnScreens(RED4ext::Handle<OnScreenEntries>* aOnScreens,
                                                  Engine::ResourcePath aPath)
{
    auto result = LoadOnScreens(aOnScreens, aPath);
    auto language = Language::ResolveFromResource(aPath);

    LogInfo("[{}] The localization system is initializing for the language [{}]...", ModuleName, language.ToString());

    if (!m_units.empty())
    {
        bool successAll = true;

        for (const auto& unit : m_units)
        {
            auto paths = unit.onscreens.find(language);
            if (paths != unit.onscreens.end())
            {
                LogInfo("[{}] Processing [{}]...", ModuleName, unit.name);
            }
            else
            {
                paths = unit.onscreens.find(unit.fallback);
                LogInfo("[{}] Processing [{}] using fallback language [{}]...", ModuleName, unit.name, unit.fallback);
            }

            for (const auto& path : paths->second)
                successAll &= AppendEntries(*aOnScreens, path);
        }

        if (successAll)
            LogInfo("[{}] All localization entries merged.", ModuleName);
        else
            LogWarning("[{}] Localization entries merged with issues.", ModuleName);
    }
    else
    {
        LogInfo("[{}] No localization entries to merge.", ModuleName);
    }

    return result;
}

bool App::LocalizationModule::AppendEntries(RED4ext::Handle<OnScreenEntries>& aOnScreens, const std::string& aPath)
{
    RED4ext::Handle<OnScreenEntries> resource;
    LoadOnScreens(&resource, aPath.c_str());

    if (!resource.instance)
    {
        LogError("[{}] Resource [{}] failed to load.", ModuleName, aPath);
        return false;
    }

    LogInfo("[{}] Merging entries from [{}]...", ModuleName, aPath);

    bool success = true;
    auto& entries = resource.GetPtr()->entries;

    for (uint32_t i = 0; i < entries.size; ++i)
    {
        auto& entry = entries[i];

        if (entry.primaryKey == 0)
        {
            if (entry.secondaryKey.Length() == 0)
            {
                LogWarning("[{}] Skipping entry #{}: primary or secondary key is required.", ModuleName, i);
                success = false;
                continue;
            }

            entry.primaryKey = RED4ext::FNV1a64(entry.secondaryKey.c_str());
        }
        else if (entry.primaryKey <= MinCustomPrimaryID)
        {
            LogWarning("[{}] Skipping entry #{}: primary keys below {} are not allowed.",
                       ModuleName, i, MinCustomPrimaryID);
            success = false;
            continue;
        }

        // TODO: Check for existing keys?

        aOnScreens->entries.EmplaceBack(entry);
    }

    return success;
}
