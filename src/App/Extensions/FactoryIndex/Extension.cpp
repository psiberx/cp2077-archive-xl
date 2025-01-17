#include "Extension.hpp"
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
    HookAfter<Raw::FactoryIndex::LoadFactoryAsync>(&FactoryIndexExtension::OnLoadFactoryAsync).OrThrow();

    return true;
}

bool App::FactoryIndexExtension::Unload()
{
    Unhook<Raw::FactoryIndex::LoadFactoryAsync>();

    return true;
}

void App::FactoryIndexExtension::OnLoadFactoryAsync(uintptr_t aIndex, Red::ResourcePath aPath, uintptr_t aContext)
{
    if (aPath == LastFactory)
    {
        LogInfo("[{}] Initializing factory index...", ExtensionName);

        if (!m_configs.empty())
        {
            for (const auto& unit : m_configs)
            {
                // LogInfo("[{}] Processing \"{}\"...", ExtensionName, unit.name);

                for (const auto& path : unit.factories)
                {
                    LogInfo("[{}] Adding factory \"{}\"...", ExtensionName, path);

                    // TODO: Check if factory resource exists...
                    Raw::FactoryIndex::LoadFactoryAsync(aIndex, path.c_str(), aContext);
                }
            }

            LogInfo("[{}] All factories added to the index.", ExtensionName);
        }
        else
        {
            LogInfo("[{}] No factories to add to the index.", ExtensionName);
        }
    }
}
