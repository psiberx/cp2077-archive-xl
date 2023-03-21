#pragma once

#include "Common.hpp"
#include "Registrar.hpp"
#include "Resolving.hpp"
#include "Macros/Definition.hpp"

namespace Red
{
template<typename>
struct RTTITypeBuilder;

template<auto>
struct RTTIScopeBuilder;

namespace Detail
{
template<typename TSpec, typename TDescriptor>
concept HasRegisterHandler = requires(TDescriptor* d)
{
    { TSpec::Register(d) };
};

template<typename TSpec, typename TDescriptor>
concept HasDescribeHandler = requires(TDescriptor* d)
{
    { TSpec::Describe(d) };
};

template<typename TSpec>
concept HasMinValueGetter = requires
{
    { TSpec::Min() };
};

template<typename TSpec>
concept HasMaxValueGetter = requires
{
    { TSpec::Max() };
};

template<typename TSpec>
consteval auto GetMinValue()
{
    if constexpr (HasMinValueGetter<TSpec>)
    {
        return TSpec::Min();
    }
    else
    {
        return -1;
    }
}

template<typename TSpec>
consteval auto GetMaxValue()
{
    if constexpr (HasMinValueGetter<TSpec>)
    {
        return TSpec::Max();
    }
    else
    {
        return 128;
    }
}

template<typename C, typename T, std::size_t I>
inline void ExtractArg(CStackFrame* aFrame, T* aArg)
{
    if constexpr (I == 0 && !std::is_void_v<C> && !std::is_base_of_v<IScriptable, C> && std::is_same_v<T, C*>)
    {
        ScriptRef<C> context{};
        GetParameter(aFrame, &context);
        *aArg = context.ref;
    }
    else
    {
        GetParameter(aFrame, aArg);
    }
}

template<typename C, typename... Args, std::size_t... I>
inline void ExtractArgs(CStackFrame* aFrame, std::tuple<Args...>& aArgs, std::index_sequence<I...>)
{
    (ExtractArg<C, Args, I>(aFrame, &std::get<I>(aArgs)), ...);
}

template<typename C, typename... Args>
inline void ExtractArgs(CStackFrame* aFrame, std::tuple<Args...>& aArgs)
{
    ExtractArgs<C>(aFrame, aArgs, std::make_index_sequence<sizeof...(Args)>());
    aFrame->code++;
}

template<typename R>
inline void AssignReturnValue(R* aRetValue, CBaseRTTIType* aRetValueType,
                              R* aRetBuffer, CBaseRTTIType* aRetBufferType)
{
    if (aRetBuffer)
    {
        if (aRetBufferType)
        {
            aRetBufferType->Assign(aRetBuffer, aRetValue);
        }
        else
        {
            aRetValueType->Assign(aRetBuffer, aRetValue);
        }
    }
}

template<auto AFunction>
requires IsFunctionPtr<decltype(AFunction)>
inline ScriptingFunction_t<void*> MakeNativeFunction()
{
    using F = decltype(AFunction);
    using C = typename FunctionPtr<F>::context_type;
    using R = typename FunctionPtr<F>::return_type;
    using Args = std::conditional_t<
        std::is_void_v<C> || std::is_base_of_v<IScriptable, C>,
        typename FunctionPtr<F>::arguments_type,
        typename FunctionPtr<F>::extended_arguments_type>;

    static const auto s_retType = !std::is_void_v<R> ? ResolveType<R>() : nullptr;

    auto f = [](IScriptable* aContext, CStackFrame* aFrame, R* aRet, CBaseRTTIType* aRetType) -> void
    {
        Args args;
        ExtractArgs<C>(aFrame, args);

        if constexpr (std::is_void_v<R>)
        {
            if constexpr (std::is_void_v<C> || !std::is_base_of_v<IScriptable, C>)
            {
                std::apply(AFunction, args);
            }
            else
            {
                std::apply(AFunction, std::tuple_cat(std::make_tuple(reinterpret_cast<C*>(aContext)), args));
            }
        }
        else
        {
            if constexpr (std::is_void_v<C> || !std::is_base_of_v<IScriptable, C>)
            {
                R ret = std::apply(AFunction, args);
                AssignReturnValue(&ret, s_retType, aRet, aRetType);
            }
            else
            {
                R ret = std::apply(AFunction, std::tuple_cat(std::make_tuple(reinterpret_cast<C*>(aContext)), args));
                AssignReturnValue(&ret, s_retType, aRet, aRetType);
            }
        }
    };

    return reinterpret_cast<ScriptingFunction_t<void*>>(+f);
}

template<typename R, typename... Args>
void DescribeNativeFunction(CBaseFunction* aFunc, R(*)(Args...))
{
    (aFunc->AddParam(ResolveTypeName<Args>(), "arg", false, Detail::IsOptional<Args>), ...);

    if constexpr (!std::is_void_v<R>)
        aFunc->SetReturnType(ResolveTypeName<R>());
}

template<typename C, typename R, typename... Args>
void DescribeNativeFunction(CBaseFunction* aFunc, R (C::*)(Args...))
{
    if constexpr (!IsScripable<C>)
        aFunc->AddParam(ResolveTypeName<ScriptRef<C>>(), "self");

    (aFunc->AddParam(ResolveTypeName<Args>(), "arg", false, Detail::IsOptional<Args>), ...);

    if constexpr (!std::is_void_v<R>)
        aFunc->SetReturnType(ResolveTypeName<R>());
}

template<typename C, typename R, typename... Args>
void DescribeNativeFunction(CBaseFunction* aFunc, R (C::*aHandler)(Args...) const)
{
    DescribeNativeFunction(aFunc, reinterpret_cast<R (C::*)(Args...)>(aHandler));
}

template<typename F>
concept IsNativeFunctionPtr = Detail::IsFunctionPtr<F>
    && Detail::FunctionPtr<F>::arity == 4
    && std::is_void_v<typename Detail::FunctionPtr<F>::return_type>
    && std::is_void_v<typename Detail::FunctionPtr<F>::context_type>
    && std::is_pointer_v<typename Detail::FunctionPtr<F>::template argument_type<0>>
    && Detail::IsScripable<std::remove_pointer_t<typename Detail::FunctionPtr<F>::template argument_type<0>>>
    && std::is_same_v<typename Detail::FunctionPtr<F>::template argument_type<1>, CStackFrame*>
    && std::is_pointer_v<typename Detail::FunctionPtr<F>::template argument_type<2>>
    && std::is_pointer_v<typename Detail::FunctionPtr<F>::template argument_type<3>>
    && Detail::IsTypeOrVoid<std::remove_pointer_t<typename Detail::FunctionPtr<F>::template argument_type<3>>>;
}

template<typename C, typename R, typename RT>
requires Detail::IsScripable<C> && Detail::IsTypeOrVoid<RT>
using NativeFunctionPtr = void (*)(C*, CStackFrame*, R*, RT*);

template<typename TClass>
class ClassDescriptor : public CClass
{
public:
    ClassDescriptor() : CClass({}, sizeof(TClass), {})
    {
        static_assert(std::is_class_v<TClass>, "TClass must be a struct or class");
        static_assert(std::is_default_constructible_v<TClass>, "TClass must be default-constructible");
        static_assert(std::is_destructible_v<TClass>, "TClass must be destructible");
    }

    void MarkAbstract()
    {
        flags.isAbstract = true;
    }

    template<typename TParent>
    void SetParent()
    {
        parent = GetClass<TParent>();
    }

    template<CName AParent>
    void SetParent()
    {
        parent = GetClass<AParent>();
    }

    void SetAlias(const char* aAlias)
    {
        SetAlias(CNamePool::Add(aAlias));
    }

    void SetAlias(CName aAlias)
    {
        if (name != aAlias)
        {
            auto rtti = CRTTISystem::Get();
            if (rtti->scriptToNative.Get(aAlias) == nullptr)
            {
                rtti->scriptToNative.Insert(aAlias, name);
                rtti->nativeToScript.Insert(name, aAlias);
            }
        }
    }

    template<class TContext, typename TRet, typename TRetType>
    CClassFunction* AddFunction(NativeFunctionPtr<TContext, TRet, TRetType> aFunc, const char* aName,
                                CBaseFunction::Flags aFlags = {})
    {
        auto* ptr = reinterpret_cast<ScriptingFunction_t<TRet*>>(aFunc);

        CClassFunction* func;
        if constexpr (std::is_same_v<TContext, IScriptable>)
        {
            func = CClassStaticFunction::Create(this, aName, aName, ptr, aFlags);
        }
        else
        {
            func = CClassFunction::Create(this, aName, aName, ptr, aFlags);
        }

        RegisterFunction(func);

        return func;
    }

    template<auto AFunction>
    requires Detail::IsFunctionPtr<decltype(AFunction)>
    CClassFunction* AddFunction(const char* aName, CBaseFunction::Flags aFlags = {})
    {
        using Function = decltype(AFunction);
        using Context = typename Detail::FunctionPtr<Function>::context_type;

        if constexpr (Detail::IsNativeFunctionPtr<Function>)
        {
            return AddFunction(AFunction, aName, aFlags);
        }
        else
        {
            auto* ptr = Detail::MakeNativeFunction<AFunction>();

            CClassFunction* func;
            if constexpr (Detail::IsScripable<Context>)
            {
                static_assert(std::is_base_of_v<Context, TClass> || std::is_base_of_v<TClass, Context>);

                func = CClassFunction::Create(this, aName, aName, ptr, aFlags);
            }
            else
            {
                func = CClassStaticFunction::Create(this, aName, aName, ptr, aFlags);
            }

            Detail::DescribeNativeFunction(func, AFunction);
            RegisterFunction(func);

            return func;
        }
    }

    template<auto AProperty>
    requires Detail::IsPropertyPtr<decltype(AProperty)>
    CProperty* AddProperty(const char* aName, CProperty::Flags aFlags = {})
    {
        using Property = decltype(AProperty);
        using Context = typename Detail::PropertyPtr<Property>::context_type;
        using Value = typename Detail::PropertyPtr<Property>::value_type;

        static_assert(std::is_base_of_v<Context, TClass>);

        const auto offset = reinterpret_cast<size_t>(&(reinterpret_cast<Context const volatile*>(0)->*AProperty));

        auto* type = ResolveType<Value>();
        auto* prop = CProperty::Create(type, aName, nullptr, static_cast<uint32_t>(offset), nullptr, aFlags);

        props.PushBack(prop);

        return prop;
    }
};

template<typename TClass>
class ClassDescriptorImpl : public ClassDescriptor<TClass>
{
    const bool IsEqual(const ScriptInstance aLhs, const ScriptInstance aRhs, uint32_t a3) final // 48
    {
        using func_t = bool (*)(CClass*, const ScriptInstance, const ScriptInstance, uint32_t);
        RelocFunc<func_t> func(RED4ext::Addresses::TTypedClass_IsEqual);
        return func(this, aLhs, aRhs, a3);
    }

    void Assign(ScriptInstance aLhs, const ScriptInstance aRhs) const final // 50
    {
        if constexpr (std::is_copy_constructible_v<TClass>)
        {
            new (aLhs) TClass(*static_cast<TClass*>(aRhs));
        }
    }

    [[nodiscard]] Memory::IAllocator* GetAllocator() const final // B8
    {
        if constexpr (Detail::HasStaticAllocator<TClass>)
        {
            return Detail::ResolveAllocatorType<TClass>::Get();
        }
        else
        {
            return Memory::RTTIAllocator::Get();
        }
    }

    void ConstructCls(ScriptInstance aMemory) const final // D8
    {
        new (aMemory) TClass();
    }

    void DestructCls(ScriptInstance aMemory) const final // E0
    {
        static_cast<TClass*>(aMemory)->~TClass();
    }

    [[nodiscard]] void* AllocMemory() const final // E8
    {
        auto classAlignment = CClass::GetAlignment();
        auto alignedSize = AlignUp(CClass::GetSize(), classAlignment);

        auto allocator = GetAllocator();
        auto allocResult = allocator->AllocAligned(alignedSize, classAlignment);

        std::memset(allocResult.memory, 0, allocResult.size);
        return allocResult.memory;
    }
};

template<typename TClass>
class ClassBuilder
{
public:
    using Descriptor = ClassDescriptorImpl<TClass>;
    using Specialization = RTTITypeBuilder<TClass>;

protected:
    static void OnRegister()
    {
        constexpr auto name = GetTypeNameStr<TClass>();

        auto* type = new Descriptor();
        type->name = CNamePool::Add(name.data());

        if constexpr (Detail::HasRegisterHandler<Specialization, Descriptor>)
        {
            Specialization::Register(type);
        }

        auto* rtti = CRTTISystem::Get();
        rtti->RegisterType(type);
    }

    static void OnDescribe()
    {
        constexpr auto name = GetTypeName<TClass>();

        auto* rtti = CRTTISystem::Get();
        auto* type = reinterpret_cast<Descriptor*>(rtti->GetClass(name));

        if (!type)
            return;

        if constexpr (Detail::IsScripable<TClass>)
        {
            if (!type->parent)
            {
                type->parent = GetClass<IScriptable>();
            }
        }

        if constexpr (Detail::HasDescribeHandler<Specialization, Descriptor>)
        {
            Specialization::Describe(type);
        }

        type->flags.isNative = true;
    }

    inline static RTTIRegistrar s_registrar{&OnRegister, &OnDescribe}; // NOLINT(cert-err58-cpp)
};

template<typename TExpansion, typename TClass>
class ClassExpansion
{
public:
    using Descriptor = ClassDescriptor<TClass>;
    using Specialization = RTTITypeBuilder<TExpansion>;

protected:
    static_assert(std::is_base_of_v<TClass, TExpansion>, "TExpansion must inherit TClass");
    static_assert(sizeof(TClass) == sizeof(TExpansion), "TExpansion must not add new members");

    static void OnDescribe()
    {
        constexpr auto name = GetTypeName<TClass>();

        auto* rtti = CRTTISystem::Get();
        auto* type = reinterpret_cast<Descriptor*>(rtti->GetClass(name));

        if (!type)
            return;

        if constexpr (Detail::HasDescribeHandler<Specialization, Descriptor>)
        {
            Specialization::Describe(type);
        }
    }

    inline static RTTIRegistrar s_registrar{nullptr, &OnDescribe}; // NOLINT(cert-err58-cpp)
};

template<typename TEnum>
class EnumDescriptor : public CEnum
{
public:
    using Value = std::underlying_type_t<TEnum>;
    using Limits = std::numeric_limits<Value>;

    static_assert(std::is_enum_v<TEnum>, "TEnum must be enum");

    EnumDescriptor() : CEnum({}, sizeof(TEnum), {})
    {
    }

    void AddOption(int64_t aValue, const char* aName)
    {
        if (aValue < Limits::min())
            return;

        if (aValue > Limits::max())
            return;

        for (uint32_t i = 0; i != valueList.size; ++i)
        {
            if (aValue == valueList.entries[i])
                return;
        }

        valueList.PushBack(aValue);
        hashList.PushBack(CNamePool::Add(aName));
    }

    template<typename E>
    void AddOption(const E aValue, const char* aName)
    {
        AddOption(static_cast<int64_t>(aValue), aName);
    }

    template<typename E = TEnum, bool AFlags = false, Value AMin = -1, Value AMax = 128>
    void AddOptions()
    {
        constexpr auto min = std::max(AMin, AFlags ? 0 : Limits::min());
        constexpr auto max = std::min(AMax, AFlags ? Limits::digits - 1 : Limits::max());
        constexpr auto range = max - min + 1;

        static_assert(range > 0, "Invalid range");
        static_assert(range < std::numeric_limits<std::uint16_t>::max(), "Invalid range");

        constexpr auto values = nameof::detail::values<E, AFlags, min>(std::make_index_sequence<range>{});

        AddOptions(this, values, std::make_index_sequence<values.size()>{});
    }

protected:
    template<typename E, std::size_t N, std::size_t... I>
    inline static void AddOptions(EnumDescriptor* aEnum, const std::array<E, N>& aValues, std::index_sequence<I...>)
    {
        (aEnum->AddOption(aValues[I], nameof::nameof_enum(aValues[I]).data()), ...);
    }
};

template<typename TEnum, bool AFlags = false>
class EnumBuilder
{
public:
    using Descriptor = EnumDescriptor<TEnum>;
    using Specialization = RTTITypeBuilder<TEnum>;

protected:
    static void OnRegister()
    {
        constexpr auto name = GetTypeNameStr<TEnum>();

        auto* type = new Descriptor();
        type->name = CNamePool::Add(name.data());

        if constexpr (Detail::HasRegisterHandler<Specialization, Descriptor>)
        {
            Specialization::Register(type);
        }

        auto* rtti = CRTTISystem::Get();
        rtti->RegisterType(type);
    }

    static void OnDescribe()
    {
        constexpr auto name = GetTypeName<TEnum>();

        auto* rtti = CRTTISystem::Get();
        auto* type = reinterpret_cast<Descriptor*>(rtti->GetEnum(name));

        constexpr auto min = Detail::GetMinValue<Specialization>();
        constexpr auto max = Detail::GetMaxValue<Specialization>();

        type->template AddOptions<TEnum, AFlags, min, max>();

        if constexpr (Detail::HasDescribeHandler<Specialization, Descriptor>)
        {
            Specialization::Describe(type);
        }
    }

    inline static RTTIRegistrar s_registrar{&OnRegister, &OnDescribe}; // NOLINT(cert-err58-cpp)
};

template<typename TExpansion, typename TEnum, bool AFlags = false>
class EnumExpansion
{
public:
    using Descriptor = EnumDescriptor<TEnum>;
    using Specialization = RTTITypeBuilder<TExpansion>;

protected:
    static void OnDescribe()
    {
        constexpr auto name = GetTypeName<TEnum>();

        auto* rtti = CRTTISystem::Get();
        auto* type = reinterpret_cast<Descriptor*>(rtti->GetEnum(name));

        if (!type)
            return;

        constexpr auto min = Detail::GetMinValue<Specialization>();
        constexpr auto max = Detail::GetMaxValue<Specialization>();

        type->template AddOptions<TExpansion, AFlags, min, max>();

        if constexpr (Detail::HasDescribeHandler<Specialization, Descriptor>)
        {
            Specialization::Describe(type);
        }
    }

    inline static RTTIRegistrar s_registrar{nullptr, &OnDescribe}; // NOLINT(cert-err58-cpp)
};

class GlobalDescriptor : public CRTTISystem
{
public:
    template<class TContext, typename TRet, typename TRetType>
    CGlobalFunction* AddFunction(NativeFunctionPtr<TContext, TRet, TRetType> aFunc, const char* aName,
                                 CBaseFunction::Flags aFlags = {})
    {
        auto* ptr = reinterpret_cast<ScriptingFunction_t<TRet*>>(aFunc);
        auto* func = CGlobalFunction::Create(aName, aName, ptr);

        func->flags = aFlags;
        func->flags.isNative = true;
        func->flags.isStatic = true;

        RegisterFunction(func);

        return func;
    }

    template<auto AFunction>
    requires Detail::IsFunctionPtr<decltype(AFunction)>
    CGlobalFunction* AddFunction(const char* aName, CBaseFunction::Flags aFlags = {})
    {
        using Function = decltype(AFunction);

        if constexpr (Detail::IsNativeFunctionPtr<Function>)
        {
            return AddFunction(AFunction, aName, aFlags);
        }
        else
        {
            auto* ptr = Detail::MakeNativeFunction<AFunction>();
            auto* func = CGlobalFunction::Create(aName, aName, ptr);

            func->flags = aFlags;
            func->flags.isNative = true;
            func->flags.isStatic = true;

            Detail::DescribeNativeFunction(func, AFunction);
            RegisterFunction(func);

            return func;
        }
    }
};

template<auto AScope>
class GlobalBuilder
{
public:
    using Descriptor = GlobalDescriptor;
    using Specialization = RTTIScopeBuilder<AScope>;

protected:
    static void OnRegister()
    {
        if constexpr (Detail::HasRegisterHandler<Specialization, Descriptor>)
        {
            auto* rtti = reinterpret_cast<Descriptor*>(CRTTISystem::Get());
            Specialization::Register(rtti);
        }
    }

    static void OnDescribe()
    {
        if constexpr (Detail::HasDescribeHandler<Specialization, Descriptor>)
        {
            auto* rtti = reinterpret_cast<Descriptor*>(CRTTISystem::Get());
            Specialization::Describe(rtti);
        }
    }

    inline static RTTIRegistrar s_registrar{&OnRegister, &OnDescribe}; // NOLINT(cert-err58-cpp)
};
}
