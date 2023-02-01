#pragma once

#include "ModuleBase.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class ModuleLoader : public Core::LoggingAgent
{
public:
    ModuleLoader(std::filesystem::path aConfigDir, std::wstring aConfigExt);

    template<class T, typename... Args>
    requires std::is_base_of_v<Module, T>
    void Add(Args&&... aArgs)
    {
        auto module = Core::MakeShared<T>(std::forward<Args>(aArgs)...);

        if constexpr (std::is_base_of_v<ConfigurableModule, T>)
            m_configurables.emplace_back(module);

        m_modules.emplace_back(module);
    }

    void Configure();
    void Load();
    void PostLoad();
    void Unload();
    void Reload();

private:
    bool ExtraConfigDirExists();
    bool ReadConfig(const std::filesystem::path& aPath, const std::filesystem::path& aDir);

    Core::Vector<Core::SharedPtr<Module>> m_modules;
    Core::Vector<Core::SharedPtr<ConfigurableModule>> m_configurables;
    std::filesystem::path m_extraConfigDir;
    std::wstring m_customConfigExt;
    bool m_loaded;
};
}
