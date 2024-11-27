#pragma once

#include "ExtensionBase.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class ExtensionLoader : public Core::LoggingAgent
{
public:
    ExtensionLoader(std::filesystem::path aConfigDir, std::wstring aConfigExt);

    template<class T, typename... Args>
    requires std::is_base_of_v<Extension, T>
    void Add(Args&&... aArgs)
    {
        auto module = Core::MakeShared<T>(std::forward<Args>(aArgs)...);

        if constexpr (std::is_base_of_v<ConfigurableExtension, T>)
            m_configurables.emplace_back(module);

        m_modules.emplace_back(module);
    }

    void Configure();
    void Load();
    void PostLoad();
    void Unload();
    void Reload();

private:
    bool AddConfig(const std::filesystem::path& aPath, const std::filesystem::path& aDir, bool aSilent = false);

    Core::Vector<Core::SharedPtr<Extension>> m_modules;
    Core::Vector<Core::SharedPtr<ConfigurableExtension>> m_configurables;
    std::filesystem::path m_bundleConfigDir;
    std::wstring m_customConfigExt;
    bool m_loaded;
};
}
