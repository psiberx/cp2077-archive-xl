#pragma once

namespace App
{
struct Unit
{
    explicit Unit(std::string aName) : name(std::move(aName)) {}

    virtual bool IsDefined() = 0;
    virtual void LoadYAML(const YAML::Node& aNode) = 0;

    [[nodiscard]] bool HasIssues() const
    {
        return !issues.empty();
    }

    const std::string name;
    Core::Vector<std::string> issues;
};
}
