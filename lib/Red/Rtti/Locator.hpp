#pragma once

#include <RED4ext/CName.hpp>
#include <RED4ext/RTTISystem.hpp>

namespace Red::Rtti
{
template<RED4ext::CName ATypeName>
class TypeLocator
{
public:
    operator const RED4ext::CBaseRTTIType*()
    {
        if (!s_resolved)
        {
            Resolve();
        }

        return s_type;
    }

    operator const RED4ext::CClass*()
    {
        if (!s_resolved)
        {
            Resolve();
        }

        if (!s_type || s_type->GetType() != RED4ext::ERTTIType::Class)
        {
            return nullptr;
        }

        return reinterpret_cast<RED4ext::CClass*>(s_type);
    }

    operator bool()
    {
        return s_type;
    }

private:
    void Resolve()
    {
        s_type = RED4ext::CRTTISystem::Get()->GetType(ATypeName);
        s_resolved = true;
    }

    static inline RED4ext::CBaseRTTIType* s_type;
    static inline bool s_resolved;
};

template<class TClass>
class ClassLocator : public TypeLocator<TClass::NAME>
{
};
}