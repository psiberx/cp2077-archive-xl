#pragma once

#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class Extension
    : public Core::HookingAgent
    , public Core::LoggingAgent
{
public:
    Extension() = default;
    virtual ~Extension() = default;

    virtual std::string_view GetName() = 0;
    virtual bool Load() { return true; }
    virtual bool Unload() { return true; }
    virtual void PostLoad() {}
};

class ConfigurableExtension : public Extension
{
public:
    virtual bool AddConfig(const std::string& aConfigName, const YAML::Node& aConfigNode) = 0;
    virtual void ResetConfigs() {}
    virtual void Configure() {}
    virtual void Reload() {}
};

struct ExtensionConfig
{
    virtual bool IsDefined() = 0;
    virtual void LoadYAML(const YAML::Node& aNode) = 0;

    [[nodiscard]] bool HasIssues() const
    {
        return !issues.empty();
    }

    std::string name;
    Core::Vector<std::string> issues;
};

template<class C>
requires std::is_base_of_v<ExtensionConfig, C>
class ConfigurableExtensionImpl : public ConfigurableExtension
{
public:
    bool AddConfig(const std::string& aName, const YAML::Node& aNode) override
    {
        C config;
        config.name = aName;
        config.LoadYAML(aNode);

        if (config.HasIssues())
        {
            for (const auto& issue : config.issues)
            {
                LogError(issue.data());
                // LogError("{}: {}", aName, issue);
                // LogError("|{}| {}", GetName(), issue);
            }
        }

        if (config.IsDefined())
        {
            m_configs.emplace_back(std::move(config));
        }

        return !config.HasIssues();
    }

    void ResetConfigs() override
    {
        m_configs.clear();
    }

protected:
    Core::Vector<C> m_configs;
};
}
