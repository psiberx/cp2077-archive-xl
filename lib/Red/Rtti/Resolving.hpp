#pragma once

#include "Common.hpp"
#include "Macros/Resolving.hpp"

namespace Red
{
template<typename T>
struct TypeNameMapping : public std::false_type {};

template<typename G>
struct TypePrefixMapping : public std::false_type {};

template<typename T>
struct RTTITypeBuilder;

namespace Detail
{
template<typename T>
concept IsTypeNameConst = std::is_convertible_v<T, const char*> || std::is_same_v<T, std::string_view>;

template<typename T>
concept HasGeneratedTypeName = requires(T*)
{
    { T::NAME } -> IsTypeNameConst;
    { T::ALIAS } -> IsTypeNameConst;
};

template<typename T>
concept HasTypeNameBuilder = requires(T*)
{
    { Red::RTTITypeBuilder<T>::Name() } -> IsTypeNameConst;
};

template<typename T>
concept HasTypeNameMapping = TypeNameMapping<T>::value;

template<typename T>
concept HasTypePrefixMapping = TypePrefixMapping<T>::value;

template<size_t N, size_t M>
consteval auto ConcatConstTypeName(const char* aPrefix, const char* aName)
{
    constexpr auto len = N + M + 1;
    std::array<char, len> result{};

    for (auto i = 0; i < N; ++i)
        result[i] = aPrefix[i];

    for (auto i = 0; i < M; ++i)
        result[i + N] = aName[i];

    return result;
}

template<size_t N>
consteval auto MakeConstTypeName(const char* aName)
{
    constexpr auto len = N + 1;
    std::array<char, len> result{};

    for (auto i = 0; i < N; ++i)
        result[i] = aName[i];

    return result;
}

constexpr auto ScopedEnumPrefix = "enum RED4ext::";
constexpr auto ScopedEnumPrefixLength = std::char_traits<char>::length(ScopedEnumPrefix);

consteval bool IsScopedEnumName(const std::string_view& aName)
{
    return aName.starts_with(ScopedEnumPrefix);
}

template<size_t N>
consteval auto ExtractScopedEnumName(const char* aName)
{
    std::array<char, N - ScopedEnumPrefixLength + 1> result{};

    for (auto i = ScopedEnumPrefixLength, j = 0ull; i < N; ++i)
    {
        if (aName[i] != ':')
        {
            result[j] = aName[i];
            ++j;
        }
    }

    return result;
}
}

template<typename T>
consteval auto GetTypePrefixStr()
{
    using U = std::remove_cvref_t<T>;

    constexpr auto prefix = TypePrefixMapping<U>::prefix;
    constexpr auto length = std::char_traits<char>::length(prefix);

    return Detail::MakeConstTypeName<length>(prefix);
}

template<template<typename> class T>
consteval auto GetTypePrefixStr()
{
    using U = T<ISerializable>;

    constexpr auto prefix = TypePrefixMapping<U>::prefix;
    constexpr auto length = std::char_traits<char>::length(prefix);

    return Detail::MakeConstTypeName<length>(prefix);
}

template<typename T>
consteval auto GetTypeNameStr()
{
    using U = std::remove_cvref_t<T>;

    if constexpr (Detail::HasTypeNameBuilder<U>)
    {
        constexpr auto name = RTTITypeBuilder<U>::Name();

        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(name)>, std::string_view>)
        {
            return Detail::MakeConstTypeName<name.size()>(name.data());
        }
        else
        {
            constexpr auto length = std::char_traits<char>::length(name);

            return Detail::MakeConstTypeName<length>(name);
        }
    }
    else if constexpr (Detail::HasTypeNameMapping<U>)
    {
        constexpr auto name = TypeNameMapping<U>::name;
        constexpr auto length = std::char_traits<char>::length(name);

        return Detail::MakeConstTypeName<length>(name);
    }
    else if constexpr (Detail::HasGeneratedTypeName<U>)
    {
        constexpr auto name = U::NAME;
        constexpr auto length = std::char_traits<char>::length(name);

        return Detail::MakeConstTypeName<length>(name);
    }
    else if constexpr (Detail::IsSpecialization<U> && Detail::HasTypePrefixMapping<U>)
    {
        constexpr auto prefix = TypePrefixMapping<U>::prefix;
        constexpr auto length = std::char_traits<char>::length(prefix);
        constexpr auto inner = GetTypeNameStr<typename Detail::Specialization<U>::argument_type>();

        return Detail::ConcatConstTypeName<length, inner.size() - 1>(prefix, inner.data());
    }
    else if constexpr (Detail::IsOptional<U>)
    {
        constexpr auto inner = GetTypeNameStr<typename Detail::Specialization<U>::argument_type>();

        return Detail::MakeConstTypeName<inner.size() - 1>(inner.data());
    }
    else
    {
        constexpr auto fullName = nameof::nameof_full_type<U>();
        constexpr auto isScopedEnum = Detail::IsScopedEnumName(fullName);

        if constexpr (isScopedEnum)
        {
            return Detail::ExtractScopedEnumName<fullName.size()>(fullName.data());
        }
        else
        {
            constexpr auto name = nameof::detail::pretty_name(fullName);

            return Detail::MakeConstTypeName<name.size()>(name.data());
        }
    }
}

template<typename T>
consteval CName GetTypeName()
{
    return GetTypeNameStr<T>().data();
}

template<CName AType>
class TypeLocator
{
public:
    static inline CBaseRTTIType* Get()
    {
        if (!s_resolved)
        {
            Resolve();
        }

        return s_type;
    }

    static inline CClass* GetClass()
    {
        if (!s_resolved)
        {
            Resolve();
        }

        if (!s_type || s_type->GetType() != ERTTIType::Class)
        {
            return nullptr;
        }

        return reinterpret_cast<CClass*>(s_type);
    }

    static inline CRTTIHandleType* GetHandle()
    {
        if (!s_resolved)
        {
            Resolve();
        }

        if (!s_type || s_type->GetType() != ERTTIType::Handle)
        {
            return nullptr;
        }

        return reinterpret_cast<CRTTIHandleType*>(s_type);
    }

    static inline CRTTIWeakHandleType* GetWeakHandle()
    {
        if (!s_resolved)
        {
            Resolve();
        }

        if (!s_type || s_type->GetType() != ERTTIType::WeakHandle)
        {
            return nullptr;
        }

        return reinterpret_cast<CRTTIWeakHandleType*>(s_type);
    }

    static inline CRTTIArrayType* GetArray()
    {
        if (!s_resolved)
        {
            Resolve();
        }

        if (!s_type || s_type->GetType() != ERTTIType::Array)
        {
            return nullptr;
        }

        return reinterpret_cast<CRTTIArrayType*>(s_type);
    }

    static inline bool IsDefined()
    {
        if (!s_resolved)
        {
            Resolve();
        }

        return s_type;
    }

    operator CBaseRTTIType*() const
    {
        return Get();
    }

    operator CClass*() const
    {
        return GetClass();
    }

    operator CRTTIHandleType*() const
    {
        return GetHandle();
    }

    operator CRTTIWeakHandleType*() const
    {
        return GetWeakHandle();
    }

    operator CRTTIArrayType*() const
    {
        return GetArray();
    }

    [[nodiscard]] inline CBaseRTTIType* operator->() const
    {
        return Get();
    }

    operator bool()
    {
        return IsDefined();
    }

private:
    static inline void Resolve()
    {
        s_type = CRTTISystem::Get()->GetType(AType);
        s_resolved = true;
    }

    static inline CBaseRTTIType* s_type;
    static inline bool s_resolved;
};

template<typename T>
class ClassLocator : public TypeLocator<GetTypeName<T>()>
{
};

template<CName AType>
CBaseRTTIType* GetType()
{
    return TypeLocator<AType>::Get();
}

template<typename TType>
CBaseRTTIType* GetType()
{
    constexpr auto name = GetTypeName<TType>();

    return TypeLocator<name>::Get();
}

CBaseRTTIType* GetType(CName aTypeName)
{
    return CRTTISystem::Get()->GetType(aTypeName);
}

template<CName AType>
CClass* GetClass()
{
    return TypeLocator<AType>::GetClass();
}

template<typename TType>
CClass* GetClass()
{
    constexpr auto name = GetTypeName<TType>();

    return TypeLocator<name>::GetClass();
}

CClass* GetClass(CName aTypeName)
{
    auto type = CRTTISystem::Get()->GetType(aTypeName);

    if (!type || type->GetType() != ERTTIType::Class)
    {
        return nullptr;
    }

    return reinterpret_cast<CClass*>(type);
}

template<typename T>
CName ResolveTypeName()
{
    constexpr auto name = GetTypeNameStr<T>();
    return CNamePool::Add(name.data());
}

template<typename T>
CBaseRTTIType* ResolveType()
{
    static const auto s_name = ResolveTypeName<T>();
    return CRTTISystem::Get()->GetType(s_name);
}

template<typename T>
CClass* ResolveClass()
{
    static const auto s_name = ResolveTypeName<T>();
    return CRTTISystem::Get()->GetClass(s_name);
}

RTTI_TYPE_NAME(int8_t, "Int8");
RTTI_TYPE_NAME(uint8_t, "Uint8");
RTTI_TYPE_NAME(int16_t, "Int16");
RTTI_TYPE_NAME(uint16_t, "Uint16");
RTTI_TYPE_NAME(int32_t, "Int32");
RTTI_TYPE_NAME(uint32_t, "Uint32");
RTTI_TYPE_NAME(int64_t, "Int64");
RTTI_TYPE_NAME(uint64_t, "Uint64");
RTTI_TYPE_NAME(float, "Float");
RTTI_TYPE_NAME(double, "Double");
RTTI_TYPE_NAME(bool, "Bool");
RTTI_TYPE_NAME(CString, "String");
RTTI_TYPE_NAME(CName, "CName");
RTTI_TYPE_NAME(TweakDBID, "TweakDBID");
RTTI_TYPE_NAME(ItemID, "gameItemID");
RTTI_TYPE_NAME(NodeRef, "NodeRef");
RTTI_TYPE_NAME(Variant, "Variant");

RTTI_TYPE_PREFIX(DynArray, "array:");
RTTI_TYPE_PREFIX(Handle, "handle:");
RTTI_TYPE_PREFIX(WeakHandle, "whandle:");
RTTI_TYPE_PREFIX(ScriptRef, "script_ref:");
RTTI_TYPE_PREFIX(ResourceReference, "rRef:");
RTTI_TYPE_PREFIX(ResourceAsyncReference, "raRef:");
}
