#include "Module.hpp"
#include "Red/FactoryIndex.hpp"

namespace
{
constexpr auto ModuleName = "FactoryIndex";
constexpr auto LastFactory = Red::ResourcePath(R"(base\gameplay\factories\vehicles\vehicles.csv)");
}

std::string_view App::FactoryIndexModule::GetName()
{
    return ModuleName;
}

bool App::FactoryIndexModule::Load()
{
    if (!HookAfter<Raw::FactoryIndex::LoadFactoryAsync>(&FactoryIndexModule::OnLoadFactoryAsync))
        throw std::runtime_error("Failed to hook [FactoryIndex::LoadFactoryAsync].");

    return true;
}

bool App::FactoryIndexModule::Unload()
{
    Unhook<Raw::FactoryIndex::LoadFactoryAsync>();

    return true;
}

void App::FactoryIndexModule::OnLoadFactoryAsync(uintptr_t aIndex, Red::ResourcePath aPath, uintptr_t aContext)
{
    if (aPath == LastFactory)
    {
        LogInfo("|{}| Initializing factory index...", ModuleName);

        if (!m_configs.empty())
        {
            for (const auto& unit : m_configs)
            {
                LogInfo("|{}| Processing \"{}\"...", ModuleName, unit.name);

                for (const auto& path : unit.factories)
                {
                    LogInfo("|{}| Adding factory \"{}\"...", ModuleName, path);

                    // TODO: Check if factory resource exists...
                    Raw::FactoryIndex::LoadFactoryAsync(aIndex, path.c_str(), aContext);
                }
            }

            LogInfo("|{}| All factories added to the index.", ModuleName);
        }
        else
        {
            LogInfo("|{}| No factories to add to the index.", ModuleName);
        }
    }
}
