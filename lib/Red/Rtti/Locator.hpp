#pragma once

#include "Detail.hpp"

#include <RED4ext/CName.hpp>
#include <RED4ext/RTTISystem.hpp>

namespace Red::Rtti
{
template<RED4ext::CName ATypeName>
class TypeLocator
{
public:
    operator RED4ext::CBaseRTTIType*() const
    {
        if (!s_resolved)
        {
            Resolve();
        }

        return s_type;
    }

    operator RED4ext::CClass*() const
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

    operator RED4ext::CRTTIHandleType*() const
    {
        if (!s_resolved)
        {
            Resolve();
        }

        if (!s_type || s_type->GetType() != RED4ext::ERTTIType::Handle)
        {
            return nullptr;
        }

        return reinterpret_cast<RED4ext::CRTTIHandleType*>(s_type);
    }

    operator RED4ext::CRTTIWeakHandleType*() const
    {
        if (!s_resolved)
        {
            Resolve();
        }

        if (!s_type || s_type->GetType() != RED4ext::ERTTIType::WeakHandle)
        {
            return nullptr;
        }

        return reinterpret_cast<RED4ext::CRTTIWeakHandleType*>(s_type);
    }

    operator bool()
    {
        return s_type;
    }

private:
    void Resolve() const
    {
        s_type = RED4ext::CRTTISystem::Get()->GetType(ATypeName);
        s_resolved = true;
    }

    static inline RED4ext::CBaseRTTIType* s_type;
    static inline bool s_resolved;
};

template<typename T>
class ClassLocator;

template<class T>
requires detail::HasGeneratedNameConst<T>
class ClassLocator<T> : public TypeLocator<T::NAME>
{
};

template<class T>
requires detail::HasGeneratedNameConst<T>
class ClassLocator<RED4ext::Handle<T>> : public TypeLocator<RED4ext::FNV1a64(T::NAME, RED4ext::FNV1a64("handle:"))>
{
};
}