#pragma once

#include "App/Extensions/ExtensionBase.hpp"

namespace App
{
struct ResourceFix
{
    [[nodiscard]] inline bool DefinesNameMappings() const
    {
        return !names.empty();
    }

    [[nodiscard]] inline Red::CName GetMappedName(Red::CName aName) const
    {
        const auto& it = names.find(aName);
        return it != names.end() ? it.value() : aName;
    }

    [[nodiscard]] inline bool DefinesPathMappings() const
    {
        return !paths.empty();
    }

    [[nodiscard]] inline Red::ResourcePath GetMappedPath(Red::ResourcePath aPath) const
    {
        const auto& it = paths.find(aPath);
        return it != paths.end() ? it.value() : aPath;
    }

    [[nodiscard]] inline bool DefinesContext() const
    {
        return !context.empty();
    }

    [[nodiscard]] inline const Core::Map<Red::CName, std::string>& GetContext() const
    {
        return context;
    }

    void Merge(const ResourceFix& aOther)
    {
        for (const auto& [oldName, newName] : aOther.names)
        {
            names[oldName] = newName;
        }

        for (const auto& [oldPath, newPath] : aOther.paths)
        {
            paths[oldPath] = newPath;
        }

        for (const auto& [param, value] : aOther.context)
        {
            context[param] = value;
        }
    }

    Core::Map<Red::CName, Red::CName> names;
    Core::Map<Red::ResourcePath, Red::ResourcePath> paths;
    Core::Map<Red::CName, std::string> context;
};

struct ResourceMetaConfig : ExtensionConfig
{
    using ExtensionConfig::ExtensionConfig;

    bool IsDefined() override;
    void LoadYAML(const YAML::Node& aNode) override;
    void LoadScopes(const YAML::Node& aNode);
    void LoadFixes(const YAML::Node& aNode);

    Core::Map<Red::ResourcePath, Core::Set<Red::ResourcePath>> scopes;
    Core::Map<Red::ResourcePath, ResourceFix> fixes;
    Core::Map<Red::ResourcePath, std::string> paths;
};
}
