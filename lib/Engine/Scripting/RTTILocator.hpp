#pragma once

#include <RED4ext/CName.hpp>
#include <RED4ext/RTTISystem.hpp>

namespace Engine
{
struct RTTILocator
{
public:
    RTTILocator(RED4ext::CName aName);

    operator const RED4ext::CBaseRTTIType*();
    operator const RED4ext::CClass*();
    operator bool();

private:
    void Resolve();

    RED4ext::CName m_name;
    RED4ext::CBaseRTTIType* m_type;
};
}
