#pragma once

namespace App
{
class ComponentPrefixResolver
{
public:
    [[nodiscard]] RED4ext::CName GetPrefix(RED4ext::CName aComponentName);

    [[nodiscard]] static Core::SharedPtr<ComponentPrefixResolver>& Get();

private:
    Core::Map<RED4ext::CName, RED4ext::CName> m_resolved;
};
}
