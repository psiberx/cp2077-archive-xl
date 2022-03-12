#pragma once

#include "Module.hpp"
#include "Unit.hpp"

namespace App
{
template<class U>
requires std::is_base_of_v<Unit, U>
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
                LogError("[{}] {}", GetName(), issue);

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
};
}
