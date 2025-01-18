#pragma once

#include "Common.hpp"
#include "Macros/Resolving.hpp"

namespace Red
{
template<Scope>
struct TypeInfoBuilder;

template<typename T>
struct TypeNameMapping : public std::false_type {};

template<typename G>
struct TypePrefixMapping : public std::false_type {};

template<typename G>
struct TypeProxyMapping : public std::false_type {};

namespace Detail
{
template<typename T>
concept IsTypeNameConst = std::is_same_v<std::remove_cvref_t<T>, const char*> || std::is_convertible_v<T, std::string_view>;

template<typename T>
concept HasGeneratedTypeName = requires(T*)
{
    { T::NAME } -> IsTypeNameConst;
    { T::ALIAS } -> IsTypeNameConst;
};

template<typename T>
concept HasTypeNameMapping = TypeNameMapping<T>::value;

template<typename G>
concept HasTypePrefixMapping = TypePrefixMapping<G>::value;

template<typename G>
concept HasTypeProxyMapping = TypeProxyMapping<G>::value;

template<typename T>
concept HasTypeNameBuilder = requires(T*)
{
    { TypeInfoBuilder<Scope::For<T>()>::Name() } -> IsTypeNameConst;
};

template<typename T>
consteval auto ResolveTypeNameBuilder()
{
    return TypeInfoBuilder<Scope::For<T>()>::Name();
}

template<size_t N>
consteval auto MakeConstStr(const char* aName)
{
    constexpr auto len = N + 1;
    std::array<char, len> result{};

    for (auto i = 0; i < N; ++i)
        result[i] = aName[i];

    return result;
}

template<size_t N, size_t M>
consteval auto ConcatConstStr(const char* aPrefix, const char* aName)
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
consteval auto UpFirstConstStr(const char* aName)
{
    constexpr auto len = N + 1;
    std::array<char, len> result{};

    for (auto i = 0; i < N; ++i)
        result[i] = aName[i];

    if (result[0] >= 'a' && result[0] <= 'z')
    {
        result[0] -= ('a' - 'A');
    }

    return result;
}

consteval auto RemoveMemberPrefix(std::string_view aName)
{
    if (aName.starts_with("m_"))
    {
        aName.remove_prefix(2);
    }

    return aName;
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

    return Detail::MakeConstStr<length>(prefix);
}

template<template<typename> class T>
consteval auto GetTypePrefixStr()
{
    return GetTypePrefixStr<T<ISerializable>>();
}

template<typename T>
consteval auto GetTypeNameStr()
{
    using U = std::remove_cvref_t<T>;

    if constexpr (Detail::HasTypeNameBuilder<U>)
    {
        constexpr auto name = Detail::ResolveTypeNameBuilder<U>();

        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(name)>, const char*>)
        {
            constexpr auto length = std::char_traits<char>::length(name);

            return Detail::MakeConstStr<length>(name);
        }
        else
        {
            return Detail::MakeConstStr<name.size()>(name.data());
        }
    }
    else if constexpr (Detail::HasTypeNameMapping<U>)
    {
        constexpr auto name = TypeNameMapping<U>::name;
        constexpr auto length = std::char_traits<char>::length(name);

        return Detail::MakeConstStr<length>(name);
    }
    else if constexpr (Detail::HasGeneratedTypeName<U>)
    {
        constexpr auto name = U::NAME;
        constexpr auto length = std::char_traits<char>::length(name);

        return Detail::MakeConstStr<length>(name);
    }
    else if constexpr (Detail::IsSpecialization<U> && Detail::HasTypePrefixMapping<U>)
    {
        constexpr auto prefix = TypePrefixMapping<U>::prefix;
        constexpr auto length = std::char_traits<char>::length(prefix);
        constexpr auto inner = GetTypeNameStr<typename Detail::Specialization<U>::argument_type>();

        return Detail::ConcatConstStr<length, inner.size() - 1>(prefix, inner.data());
    }
    else if constexpr (Detail::HasTypeProxyMapping<U>)
    {
        return GetTypeNameStr<typename TypeProxyMapping<U>::type>();
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

            return Detail::MakeConstStr<name.size()>(name.data());
        }
    }
}

template<typename T>
consteval CName GetTypeName()
{
    return GetTypeNameStr<T>().data();
}

template<typename T>
consteval uint64_t GetTypeHash()
{
    return FNV1a64(GetTypeNameStr<T>().data());
}

template<CName AType>
class TypeLocator
{
public:
    static inline CBaseRTTIType* Get() noexcept
    {
        if (!s_resolved)
        {
            Resolve();
        }

        return s_type;
    }

    static inline CClass* GetClass() noexcept
    {
        if (!s_resolved)
        {
            Resolve();

            if (s_type && s_type->GetType() != ERTTIType::Class)
            {
                s_type = nullptr;
            }
        }

        return reinterpret_cast<CClass*>(s_type);
    }

    static inline CRTTIHandleType* GetHandle() noexcept
    {
        if (!s_resolved)
        {
            Resolve();

            if (s_type && s_type->GetType() != ERTTIType::Handle)
            {
                s_type = nullptr;
            }
        }

        return reinterpret_cast<CRTTIHandleType*>(s_type);
    }

    static inline CRTTIWeakHandleType* GetWeakHandle() noexcept
    {
        if (!s_resolved)
        {
            Resolve();

            if (s_type && s_type->GetType() != ERTTIType::WeakHandle)
            {
                s_type = nullptr;
            }
        }

        return reinterpret_cast<CRTTIWeakHandleType*>(s_type);
    }

    static inline CRTTIArrayType* GetArray() noexcept
    {
        if (!s_resolved)
        {
            Resolve();

            if (s_type && s_type->GetType() != ERTTIType::Array)
            {
                s_type = nullptr;
            }
        }

        return reinterpret_cast<CRTTIArrayType*>(s_type);
    }

    static inline CEnum* GetEnum() noexcept
    {
        if (!s_resolved)
        {
            Resolve();

            if (s_type && s_type->GetType() != ERTTIType::Enum)
            {
                s_type = nullptr;
            }
        }

        return reinterpret_cast<CEnum*>(s_type);
    }

    static inline bool IsDefined() noexcept
    {
        if (!s_resolved)
        {
            Resolve();
        }

        return s_type;
    }

    operator CBaseRTTIType*() const noexcept
    {
        return Get();
    }

    operator CClass*() const noexcept
    {
        return GetClass();
    }

    operator CRTTIHandleType*() const noexcept
    {
        return GetHandle();
    }

    operator CRTTIWeakHandleType*() const noexcept
    {
        return GetWeakHandle();
    }

    operator CRTTIArrayType*() const noexcept
    {
        return GetArray();
    }

    [[nodiscard]] inline CBaseRTTIType* operator->() const noexcept
    {
        return Get();
    }

    operator bool() noexcept
    {
        return IsDefined();
    }

private:
    static inline void Resolve() noexcept
    {
        if (auto rtti = CRTTISystem::Get())
        {
            s_type = rtti->GetType(AType);
            s_resolved = true;
        }
    }

    static inline CBaseRTTIType* s_type;
    static inline bool s_resolved;
};

template<typename T>
class ClassLocator : public TypeLocator<GetTypeName<T>()>
{
};

template<CName AType>
inline CBaseRTTIType* GetType() noexcept
{
    return TypeLocator<AType>::Get();
}

template<typename TType>
inline CBaseRTTIType* GetType() noexcept
{
    constexpr auto name = GetTypeName<TType>();

    return TypeLocator<name>::Get();
}

inline CBaseRTTIType* GetType(CName aTypeName)
{
    return CRTTISystem::Get()->GetType(aTypeName);
}

template<CName AType>
inline CClass* GetClass() noexcept
{
    return TypeLocator<AType>::GetClass();
}

template<typename TType>
inline CClass* GetClass() noexcept
{
    constexpr auto name = GetTypeName<TType>();

    return TypeLocator<name>::GetClass();
}

inline CClass* GetClass(CName aTypeName)
{
    auto type = CRTTISystem::Get()->GetType(aTypeName);

    if (!type || type->GetType() != ERTTIType::Class)
    {
        return nullptr;
    }

    return reinterpret_cast<CClass*>(type);
}

inline CClass* GetScriptClass(CName aTypeName)
{
    auto type = GetClass(aTypeName);
    return type ? type : CRTTISystem::Get()->GetClassByScriptName(aTypeName);
}

inline CName GetScriptAlias(CName aTypeName)
{
    auto alias = CRTTISystem::Get()->nativeToScript.Get(aTypeName);
    return alias ? *alias : CName{};
}

template<CName AType>
inline CEnum* GetEnum() noexcept
{
    return TypeLocator<AType>::GetEnum();
}

template<typename TType>
inline CEnum* GetEnum() noexcept
{
    constexpr auto name = GetTypeName<TType>();

    return TypeLocator<name>::GetEnum();
}

inline CEnum* GetEnum(CName aTypeName)
{
    auto type = CRTTISystem::Get()->GetType(aTypeName);

    if (!type || type->GetType() != ERTTIType::Enum)
    {
        return nullptr;
    }

    return reinterpret_cast<CEnum*>(type);
}

template<typename T>
inline CName ResolveTypeName()
{
    constexpr auto str = GetTypeNameStr<T>();
    static const auto name = CNamePool::Add(str.data());
    return name;
}

template<typename T>
inline CBaseRTTIType* ResolveType()
{
    static const auto s_name = ResolveTypeName<T>();
    return CRTTISystem::Get()->GetType(s_name);
}

template<typename T>
inline CClass* ResolveClass()
{
    static const auto s_name = ResolveTypeName<T>();
    return CRTTISystem::Get()->GetClass(s_name);
}

inline void RegisterPendingTypes()
{
    CRTTISystem::Get()->GetType("Int32");
}

template<typename T, typename U = ISerializable>
inline T* Cast(U* aObject)
{
    return (aObject && aObject->GetType()->IsA(Red::GetClass<T>()))
               ? reinterpret_cast<T*>(aObject)
               : nullptr;
}

template<typename T, typename U = ISerializable>
inline const Handle<T>& Cast(const Handle<U>& aObject)
{
    static const Handle<T> s_null;
    return (aObject && aObject->GetType()->IsA(Red::GetClass<T>()))
               ? *reinterpret_cast<const Handle<T>*>(&aObject)
               : s_null;
}

template<typename T, typename U = ISerializable>
inline const WeakHandle<T>& CastWeak(const WeakHandle<U>& aObject)
{
    static const WeakHandle<T> s_null;
    return (aObject && aObject.instance->GetType()->IsA(Red::GetClass<T>()))
               ? *reinterpret_cast<const WeakHandle<T>*>(&aObject)
               : s_null;
}

template<typename T>
inline bool IsInstanceOf(CClass* aType)
{
    return aType->IsA(Red::GetClass<T>());
}

inline bool IsInstanceOf(CClass* aType, CName aClassName)
{
    return aType->IsA(Red::GetClass(aClassName));
}

template<typename T>
inline bool IsInstanceOf(ISerializable* aObject)
{
    return aObject->GetType()->IsA(Red::GetClass<T>());
}

inline bool IsInstanceOf(ISerializable* aObject, CName aClassName)
{
    return aObject->GetType()->IsA(Red::GetClass(aClassName));
}

template<typename T>
inline bool IsInstanceOf(const WeakHandle<ISerializable>& aObject)
{
    return aObject && aObject.instance->GetType()->IsA(Red::GetClass<T>());
}

inline bool IsCompatible(CBaseRTTIType* aLhsType, CBaseRTTIType* aRhsType, void* aRhsValue = nullptr)
{
    if (!aLhsType || !aRhsType)
        return false;

    if (aLhsType != aRhsType)
    {
        auto lhsMetaType = aLhsType->GetType();

        if ((lhsMetaType == ERTTIType::Handle || lhsMetaType == ERTTIType::WeakHandle)
            && lhsMetaType == aRhsType->GetType())
        {
            auto lhsSubType = reinterpret_cast<CClass*>(reinterpret_cast<CRTTIHandleType*>(aLhsType)->innerType);
            auto rhsInstance = aRhsValue ? reinterpret_cast<Handle<ISerializable>*>(aRhsValue)->instance : nullptr;
            auto rhsSubType = rhsInstance
                                  ? rhsInstance->GetType()
                                  : reinterpret_cast<CClass*>(reinterpret_cast<CRTTIHandleType*>(aRhsType)->innerType);

            return rhsSubType->IsA(lhsSubType);
        }

        return false;
    }

    return true;
}
}
