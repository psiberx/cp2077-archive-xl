#pragma once

#include "Module.hpp"
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
            m_configurable.emplace_back(module);

        m_modules.emplace_back(module);
    }

    void Configure();
    void Load();
    void Unload();

private:
    bool ConfigDirExists();
    bool ReadConfig(const std::filesystem::path& aPath);

    Core::Vector<Core::SharedPtr<Module>> m_modules;
    Core::Vector<Core::SharedPtr<ConfigurableModule>> m_configurable;
    std::filesystem::path m_configDir;
    std::wstring m_configExt;
    bool m_loaded;
};
}
