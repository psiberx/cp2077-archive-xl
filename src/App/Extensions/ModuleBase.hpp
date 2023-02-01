#pragma once

#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class Module
    : public Core::HookingAgent
    , public Core::LoggingAgent
{
public:
    Module() = default;
    virtual ~Module() = default;

    virtual std::string_view GetName() = 0;
    virtual bool Load() = 0;
    virtual bool Unload() = 0;
    virtual void PostLoad() {}
};

class ConfigurableModule : public Module
{
public:
    virtual bool Configure(const std::string& aConfigName, const YAML::Node& aConfigNode) = 0;
    virtual void Reset() = 0;
    virtual void Reload() {};
};

struct ConfigurableUnit
{
    explicit ConfigurableUnit(std::string aName) : name(std::move(aName)) {}

    virtual bool IsDefined() = 0;
    virtual void LoadYAML(const YAML::Node& aNode) = 0;

    [[nodiscard]] bool HasIssues() const
    {
        return !issues.empty();
    }

    const std::string name;
    Core::Vector<std::string> issues;
};

template<class U>
requires std::is_base_of_v<ConfigurableUnit, U>
class ConfigurableUnitModule : public ConfigurableModule
{
public:
    bool Configure(const std::string& aName, const YAML::Node& aNode) override
    {
        U unit(aName);
        unit.LoadYAML(aNode);

        if (unit.IsDefined())
            m_units.emplace_back(std::move(unit));

        if (unit.HasIssues())
        {
            for (const auto& issue : unit.issues)
                LogError("|{}| {}", GetName(), issue);

            return false;
        }

        return true;
    }

    void Reset() override
    {
        m_units.clear();
    }

protected:
    Core::Vector<U> m_units;
};}
