#include "Module.hpp"

namespace
{
constexpr auto ModuleName = "FactoryIndex";
constexpr auto LastFactory = RED4ext::ResourcePath(R"(base\gameplay\factories\vehicles\vehicles.csv)");
}

std::string_view App::FactoryIndexModule::GetName()
{
    return ModuleName;
}

bool App::FactoryIndexModule::Load()
{
    if (!Hook<Raw::LoadFactoryAsync>(&FactoryIndexModule::OnLoadFactoryAsync, &FactoryIndexModule::LoadFactoryAsync))
        throw std::runtime_error("Failed to hook [FactoryIndex::LoadFactoryAsync].");

    return true;
}

bool App::FactoryIndexModule::Unload()
{
    Unhook<Raw::LoadFactoryAsync>();

    return true;
}

void App::FactoryIndexModule::OnLoadFactoryAsync(uintptr_t aIndex, RED4ext::ResourcePath aPath, uintptr_t aContext)
{
    LoadFactoryAsync(aIndex, aPath, aContext);

    if (aPath == LastFactory)
    {
        LogInfo("|{}| The factory index is initializing...", ModuleName);

        if (!m_units.empty())
        {
            for (const auto& unit : m_units)
            {
                LogInfo("|{}| Processing [{}]...", ModuleName, unit.name);

                for (const auto& path : unit.factories)
                {
                    LogInfo("|{}| Adding factory [{}]...", ModuleName, path);

                    LoadFactoryAsync(aIndex, path.c_str(), aContext);

                    // TODO: Check if factory resource exists...
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
