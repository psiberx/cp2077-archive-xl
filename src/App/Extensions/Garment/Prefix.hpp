#pragma once

namespace App
{
class ComponentPrefixResolver
{
public:
    [[nodiscard]] Red::CName GetPrefix(Red::CName aComponentName);

    [[nodiscard]] static const Core::SharedPtr<ComponentPrefixResolver>& Get();

private:
    Red::SharedSpinLock m_lock;
    Core::Map<Red::CName, Red::CName> m_resolved;
};
}
