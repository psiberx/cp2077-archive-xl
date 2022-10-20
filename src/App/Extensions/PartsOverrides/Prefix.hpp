#pragma once

namespace App
{
class ComponentPrefixResolver
{
public:
    [[nodiscard]] Red::CName GetPrefix(Red::CName aComponentName);

    [[nodiscard]] static Core::SharedPtr<ComponentPrefixResolver>& Get();

private:
    Core::Map<Red::CName, Red::CName> m_resolved;
};
}
