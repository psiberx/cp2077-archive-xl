#pragma once

#include "Common.hpp"
#include "Registrar.hpp"
#include "Resolving.hpp"
#include "Macros/Definition.hpp"

namespace Red
{
template<Scope>
struct TypeInfoBuilder;

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

template<typename T>
concept HasSystemGetter = requires(T*)
{
    { T::Get() } -> std::convertible_to<Handle<T>>;
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

template<typename T>
concept IsAllocationForwarded = requires
{
    { T::ForwardAllocator };
};

template<typename T>
concept IsConstructionForwarded = requires
{
    { T::ForwardInitializer };
};

template<typename TEnum>
consteval auto GetDefaultMinValue()
{
    if constexpr (std::is_signed_v<std::underlying_type_t<TEnum>>)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

template<typename TEnum>
consteval auto GetDefaultMaxValue()
{
    return 128;
}

template<typename TSpec, typename TEnum>
consteval auto GetMinValue()
{
    if constexpr (HasMinValueGetter<TSpec>)
    {
        return TSpec::Min();
    }
    else
    {
        return GetDefaultMinValue<TEnum>();
    }
}

template<typename TSpec, typename TEnum>
consteval auto GetMaxValue()
{
    if constexpr (HasMinValueGetter<TSpec>)
    {
        return TSpec::Max();
    }
    else
    {
        return GetDefaultMaxValue<TEnum>();
    }
}

template<typename T = void>
inline void ExtractArg(CStackFrame* aFrame, T* aInstance = nullptr)
{
    if constexpr (std::is_pointer_v<T>)
    {
        aFrame->useDirectData = true;
    }

    aFrame->data = nullptr;
    aFrame->dataType = nullptr;
    aFrame->currentParam++;

    const auto opcode = *(aFrame->code++);
    OpcodeHandlers::Run(opcode, aFrame->context, aFrame, aInstance, IsScriptRef<T> ? aInstance : nullptr);

    if constexpr (std::is_pointer_v<T>)
    {
        aFrame->useDirectData = false;

        if constexpr (!std::is_void_v<T>)
        {
            if (aFrame->data && aInstance)
            {
                *aInstance = reinterpret_cast<T>(aFrame->data);
            }
        }
    }
}

template<typename C, typename T, std::size_t I>
inline void ExtractArg(CStackFrame* aFrame, T* aArg)
{
    if constexpr (I == 0 && !std::is_void_v<C> && !std::is_base_of_v<IScriptable, C> && std::is_same_v<T, C*>)
    {
        ScriptRef<C> context{};
        ExtractArg(aFrame, &context);
        *aArg = context.ref;
    }
    else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
    {
        CString* str{};
        ExtractArg(aFrame, &str);
        *aArg = {str->c_str(), str->Length()};
    }
    else if constexpr (std::is_same_v<T, CStackFrame*>)
    {
        *aArg = aFrame;
    }
    else
    {
        ExtractArg(aFrame, aArg);
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
    aFrame->currentParam = 0;
    ++aFrame->code;
}

template<typename R>
inline void AssignReturnValue(R* aRetValue, CBaseRTTIType* aRetValueType,
                              R* aRetBuffer, CBaseRTTIType* aRetBufferType)
{
    if (aRetBuffer)
    {
        if (aRetBufferType)
        {
            aRetValueType = aRetBufferType;
        }

        if constexpr (std::is_same_v<R, std::string> || std::is_same_v<R, std::string_view>)
        {
            CString str(aRetValue->data());
            aRetValueType->Assign(aRetBuffer, &str);
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

template<auto AProperty>
requires IsPropertyPtr<decltype(AProperty)>
inline ScriptingFunction_t<void*> MakeNativeGetter()
{
    using Property = decltype(AProperty);
    using Context = typename Detail::PropertyPtr<Property>::context_type;
    using Value = typename Detail::PropertyPtr<Property>::value_type;

    static const auto s_retType = ResolveType<Value>();

    auto f = [](Context* aContext, CStackFrame* aFrame, Value* aRet, CBaseRTTIType* aRetType) -> void
    {
        ++aFrame->code;
        AssignReturnValue(&(aContext->*AProperty), s_retType, aRet, aRetType);
    };

    return reinterpret_cast<ScriptingFunction_t<void*>>(+f);
}

template<typename T>
inline void DescribeParameter(CBaseFunction* aFunc)
{
    using U = std::remove_cvref_t<T>;

    if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, std::string_view>)
    {
        aFunc->AddParam(GetTypeName<CString>(), "arg");
    }
    else if constexpr (!std::is_same_v<U, CStackFrame*>)
    {
        aFunc->AddParam(ResolveTypeName<T>(), "arg", false, Detail::IsOptional<T>);
    }
}

template<typename T>
inline void DescribeReturnValue(CBaseFunction* aFunc)
{
    using U = std::remove_cvref_t<T>;

    if constexpr (!std::is_void_v<U>)
    {
        if constexpr (std::is_same_v<U, std::string> || std::is_same_v<U, std::string_view>)
        {
            aFunc->SetReturnType(GetTypeName<CString>());
        }
        else
        {
            aFunc->SetReturnType(ResolveTypeName<U>());
        }
    }
}

template<typename R, typename... Args>
inline void DescribeNativeFunction(CBaseFunction* aFunc, R(*)(Args...))
{
    (DescribeParameter<Args>(aFunc), ...);

    if constexpr (!std::is_void_v<R>)
    {
        aFunc->SetReturnType(ResolveTypeName<R>());
    }
}

template<typename C, typename R, typename... Args>
inline void DescribeNativeFunction(CBaseFunction* aFunc, R (C::*)(Args...))
{
    if constexpr (!IsScriptable<C>)
    {
        aFunc->AddParam(ResolveTypeName<ScriptRef<C>>(), "self");
    }

    (DescribeParameter<Args>(aFunc), ...);

    if constexpr (!std::is_void_v<R>)
    {
        aFunc->SetReturnType(ResolveTypeName<R>());
    }
}

template<typename C, typename R, typename... Args>
inline void DescribeNativeFunction(CBaseFunction* aFunc, R (C::*aHandler)(Args...) const)
{
    DescribeNativeFunction(aFunc, reinterpret_cast<R (C::*)(Args...)>(aHandler));
}

template<typename F>
concept IsNativeFunctionPtr = Detail::IsFunctionPtr<F>
    && Detail::FunctionPtr<F>::arity == 4
    && std::is_void_v<typename Detail::FunctionPtr<F>::return_type>
    && std::is_void_v<typename Detail::FunctionPtr<F>::context_type>
    && std::is_pointer_v<typename Detail::FunctionPtr<F>::template argument_type<0>>
    && Detail::IsScriptable<std::remove_pointer_t<typename Detail::FunctionPtr<F>::template argument_type<0>>>
    && std::is_same_v<typename Detail::FunctionPtr<F>::template argument_type<1>, CStackFrame*>
    && std::is_pointer_v<typename Detail::FunctionPtr<F>::template argument_type<2>>
    && std::is_pointer_v<typename Detail::FunctionPtr<F>::template argument_type<3>>
    && Detail::IsTypeOrVoid<std::remove_pointer_t<typename Detail::FunctionPtr<F>::template argument_type<3>>>;

inline std::string MakeScriptTypeName(CBaseRTTIType* aType)
{
    switch (aType->GetType())
    {
    case RED4ext::ERTTIType::Class:
    {
        return CRTTISystem::Get()->ConvertNativeToScriptName(aType->GetName()).ToString();
    }
    case RED4ext::ERTTIType::Handle:
    case RED4ext::ERTTIType::WeakHandle:
    {
        auto pInnerType = reinterpret_cast<RED4ext::CRTTIHandleType*>(aType)->innerType;
        return MakeScriptTypeName(pInnerType);
    }
    case RED4ext::ERTTIType::Array:
    {
        auto pInnerType = reinterpret_cast<RED4ext::CRTTIArrayType*>(aType)->innerType;
        return "array<" + MakeScriptTypeName(pInnerType) + ">";
    }
    case RED4ext::ERTTIType::ScriptReference:
    {
        auto pInnerType = reinterpret_cast<RED4ext::CRTTIScriptReferenceType*>(aType)->innerType;
        return "script_ref<" + MakeScriptTypeName(pInnerType) + ">";
    }
    default: return aType->GetName().ToString();
    }
}

inline std::string MakeScriptFunctionName(CBaseFunction* aFunc, const char* aName = nullptr)
{
    auto name = std::string(aName ? aName : aFunc->shortName.ToString()) + ';';

    for (const auto& param : aFunc->params)
    {
        name += MakeScriptTypeName(param->type);
    }

    return name;
}

inline RawBuffer MakeScriptForwardCode(CBaseFunction* aFunc)
{
    constexpr uint8_t ParamOp = 25;
    constexpr uint8_t CallStaticOp = 36;
    constexpr uint8_t ParamEndOp = 38;
    constexpr uint8_t ReturnOp = 39;
    constexpr uint32_t OpSize = sizeof(char);
    constexpr uint32_t OffsetSize = sizeof(uint16_t);
    constexpr uint32_t FlagsSize = sizeof(uint16_t);
    constexpr uint32_t PointerSize = sizeof(void*);
    constexpr uint32_t BaseCodeSize = OpSize + OffsetSize * 2 + PointerSize + FlagsSize + OpSize;
    constexpr uint16_t BaseExitOffset = BaseCodeSize - OpSize - OffsetSize;

    const uint32_t extraCodeSize = aFunc->params.size * (OpSize + PointerSize) + (aFunc->returnType ? 1 : 0);
    const uint32_t finalCodeSize = BaseCodeSize + extraCodeSize;
    const uint16_t finalExitOffset = BaseExitOffset + extraCodeSize;

    Memory::EngineAllocator allocator;
    auto buffer = allocator.Alloc(finalCodeSize);
    auto code = reinterpret_cast<uint8_t*>(buffer.memory);

    if (aFunc->returnType)
    {
        *code = ReturnOp;
        code += OpSize;
    }

    *code = CallStaticOp;
    code += OpSize;

    *reinterpret_cast<uint16_t*>(code) = finalExitOffset;
    code += OffsetSize;

    *reinterpret_cast<uint16_t*>(code) = 0;
    code += OffsetSize;

    *reinterpret_cast<void**>(code) = aFunc;
    code += PointerSize;

    *reinterpret_cast<uint16_t*>(code) = 0;
    code += FlagsSize;

    for (const auto& param : aFunc->params)
    {
        *code = ParamOp;
        code += OpSize;

        *reinterpret_cast<void**>(code) = param;
        code += PointerSize;
    }

    *code = ParamEndOp;
    code += OpSize;

    return {buffer.memory, finalCodeSize};
}

using FunctionFlagsStorage = uint32_t;
static_assert(sizeof(CBaseFunction::Flags) == sizeof(FunctionFlagsStorage));
constexpr auto FunctionSpecialFlag = 1 << (8 * sizeof(FunctionFlagsStorage) - 1);
}

inline bool IsSpecial(CBaseFunction* aFunc)
{
    return *reinterpret_cast<Detail::FunctionFlagsStorage*>(&aFunc->flags) & Detail::FunctionSpecialFlag;
}

inline void MarkSpecial(CBaseFunction* aFunc)
{
    *reinterpret_cast<Detail::FunctionFlagsStorage*>(&aFunc->flags) |= Detail::FunctionSpecialFlag;
}

template<typename C, typename R, typename RT>
requires Detail::IsScriptable<C> && Detail::IsTypeOrVoid<RT>
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

    void MarkScripted()
    {
        if constexpr (Detail::IsScriptable<TClass>)
        {
            flags.isScriptedClass = true;
        }
        else
        {
            flags.isScriptedStruct = true;
        }
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
            if constexpr (Detail::IsScriptable<Context>)
            {
                if constexpr (!std::is_same_v<TClass, Context>)
                {
                    if (std::is_base_of_v<TClass, Context>)
                    {
                        static_assert(sizeof(TClass) == sizeof(Context), "Expansion must not add new members");
                    }
                    else
                    {
                        static_assert(std::is_base_of_v<TClass, Context>, "Cannot add function from unrelated class");
                    }
                }

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

    template<auto AFunction>
    requires Detail::IsFunctionPtr<decltype(AFunction)>
    CClassFunction* AddScriptFunction(const char* aName, CBaseFunction::Flags aFlags = {})
    {
        auto nativeName = std::string("_").append(aName);
        auto nativeFunc = AddFunction<AFunction>(nativeName.c_str(), aFlags);

        Memory::RTTIFunctionAllocator allocator;
        auto scriptFunc = allocator.Alloc<CClassFunction>();
        std::memcpy(scriptFunc, nativeFunc, sizeof(CClassFunction)); // NOLINT(bugprone-undefined-memory-manipulation)

        auto fullName = Detail::MakeScriptFunctionName(scriptFunc, aName);
        scriptFunc->shortName = CNamePool::Add(aName);
        scriptFunc->fullName = CNamePool::Add(fullName.c_str());

        auto bytecode = Detail::MakeScriptForwardCode(nativeFunc);
        scriptFunc->bytecode.bytecode.buffer.data = bytecode.data;
        scriptFunc->bytecode.bytecode.buffer.size = bytecode.size;

        scriptFunc->flags.isNative = false;

        RegisterFunction(scriptFunc);
        MarkSpecial(scriptFunc);

        return scriptFunc;
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

    template<auto AProperty>
    requires Detail::IsPropertyPtr<decltype(AProperty)>
    CClassFunction* AddGetter(const char* aName)
    {
        using Property = decltype(AProperty);
        using Value = typename Detail::PropertyPtr<Property>::value_type;

        auto* ptr = Detail::MakeNativeGetter<AProperty>();
        auto* func = CClassFunction::Create(this, aName, aName, ptr);
        func->SetReturnType(ResolveTypeName<Value>());

        RegisterFunction(func);

        return func;
    }
};

template<typename TClass>
class ClassDescriptorDefaultImpl : public ClassDescriptor<TClass>
{
    const bool IsEqual(const ScriptInstance aLhs, const ScriptInstance aRhs, uint32_t a3) final // 48
    {
        if constexpr (Detail::IsConstructionForwarded<TClass>)
        {
            if (!CClass::parent->flags.isAbstract)
            {
                return CClass::parent->IsEqual(aLhs, aRhs);
            }
            return false;
        }
        else
        {
            using func_t = bool (*)(CClass*, const ScriptInstance, const ScriptInstance, uint32_t);
            RelocFunc<func_t> func(RED4ext::Addresses::TTypedClass_IsEqual);
            return func(this, aLhs, aRhs, a3);
        }
    }

    void Assign(ScriptInstance aLhs, const ScriptInstance aRhs) const final // 50
    {
        if constexpr (Detail::IsConstructionForwarded<TClass>)
        {
            if (!CClass::parent->flags.isAbstract)
            {
                CClass::parent->Assign(aLhs, aRhs);
            }
        }
        else if constexpr (std::is_copy_constructible_v<TClass>)
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
        else if constexpr (Detail::IsAllocationForwarded<TClass> || Detail::IsConstructionForwarded<TClass>)
        {
            return CClass::parent->GetAllocator();
        }
        else
        {
            return Memory::RTTIAllocator::Get();
        }
    }

    void ConstructCls(ScriptInstance aMemory) const final // D8
    {
        if constexpr (Detail::IsConstructionForwarded<TClass>)
        {
            if (!CClass::parent->flags.isAbstract)
            {
                CClass::parent->ConstructCls(aMemory);
            }
        }
        else
        {
            new (aMemory) TClass();
        }
    }

    void DestructCls(ScriptInstance aMemory) const final // E0
    {
        if constexpr (Detail::IsConstructionForwarded<TClass>)
        {
            if (!CClass::parent->flags.isAbstract)
            {
                CClass::parent->DestructCls(aMemory);
            }
        }
        else
        {
            static_cast<TClass*>(aMemory)->~TClass();
        }
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

    template<typename E = TEnum,
             Value AMin = Detail::GetDefaultMinValue<E>(),
             Value AMax = Detail::GetDefaultMaxValue<E>(),
             bool AFlags = false>
    void AddOptions()
    {
        constexpr auto min = std::max(AMin, AFlags ? static_cast<Value>(0) : Limits::min());
        constexpr auto max = std::min(AMax, AFlags ? static_cast<Value>(Limits::digits - 1) : Limits::max());
        constexpr auto range = max - min + 1;

        static_assert(range > 0, "Invalid range");
        static_assert(range < std::numeric_limits<std::uint16_t>::max(), "Invalid range");

        constexpr auto values = nameof::detail::values<E, AFlags, min>(std::make_index_sequence<range>{});

        AddOptions(this, values, std::make_index_sequence<values.size()>{});
    }

    template<typename E = TEnum,
             Value AMin = Detail::GetDefaultMinValue<E>(),
             Value AMax = Detail::GetDefaultMaxValue<E>()>
    void AddFlags()
    {
        AddOptions<E, AMin, AMax, true>();
    }

protected:
    template<typename E, std::size_t N, std::size_t... I>
    inline static void AddOptions(EnumDescriptor* aEnum, const std::array<E, N>& aValues, std::index_sequence<I...>)
    {
        (aEnum->AddOption(aValues[I], nameof::nameof_enum(aValues[I]).data()), ...);
    }
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

template<typename TClass>
requires std::is_class_v<TClass>
struct ClassDefinition
{
    using Descriptor = ClassDescriptorDefaultImpl<TClass>;
    using Specialization = TypeInfoBuilder<Scope::For<TClass>()>;

    static inline void RegisterType()
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

    static inline void DescribeType()
    {
        constexpr auto name = GetTypeName<TClass>();

        auto* rtti = CRTTISystem::Get();
        auto* type = reinterpret_cast<Descriptor*>(rtti->GetClass(name));

        if (!type)
            return;

        if constexpr (Detail::IsScriptable<TClass>)
        {
            if (!type->parent)
            {
                if constexpr (Detail::IsGameSystem<TClass>)
                {
                    type->parent = GetClass<IGameSystem>();
                }
                else if constexpr (Detail::IsScriptableSystem<TClass>)
                {
                    type->parent = GetClass<ScriptableSystem>();
                }
                else
                {
                    type->parent = GetClass<IScriptable>();
                }
            }
        }

        if constexpr (Detail::HasDescribeHandler<Specialization, Descriptor>)
        {
            Specialization::Describe(type);
        }

        type->flags.isNative = true;

        if constexpr (Detail::IsScriptableSystem<TClass>)
        {
            type->flags.isScriptedClass = true;
        }

        if constexpr (Detail::IsGameSystem<TClass>)
        {
            SystemBuilder<TClass>::RegisterGetter();
            SystemBuilder<TClass>::RegisterSystem();
        }
    }

    inline static TypeInfoRegistrar s_registrar{&RegisterType, &DescribeType}; // NOLINT(cert-err58-cpp)

    constexpr operator Scope() const noexcept
    {
        return Scope::For<TClass>();
    }
};

template<typename TSystem>
struct SystemBuilder
{
    static inline void ScriptGetter(Red::IScriptable* aContext, Red::CStackFrame* aFrame,
                                    Red::Handle<Red::IScriptable>* aRet, int64_t)
    {
        ++aFrame->code;

        if (aRet)
        {
            if constexpr (Detail::HasSystemGetter<TSystem>)
            {
                *aRet = TSystem::Get();
            }
            else
            {
                const auto framework = CGameEngine::Get()->framework;
                if (framework && framework->gameInstance)
                {
                    static const auto systemType = GetType<TSystem>();
                    const auto systemInstance = framework->gameInstance->systemMap.Get(systemType);
                    if (systemInstance)
                    {
                        *aRet = *systemInstance;
                    }
                }
            }
        }
    }

    static inline void RegisterGetter()
    {
        constexpr auto systemRefStr = GetTypeNameStr<Handle<TSystem>>();
        constexpr auto systemNameStr = GetTypeNameStr<TSystem>();
        constexpr auto getterNameStr = Detail::ConcatConstStr<3, systemNameStr.size() - 1>("Get", systemNameStr.data());

        auto gameType = GetClass<ScriptGameInstance>();
        auto getterFunc = CClassStaticFunction::Create(gameType, getterNameStr.data(), getterNameStr.data(), &ScriptGetter);
        getterFunc->SetReturnType(CNamePool::Add(systemRefStr.data()));

        gameType->staticFuncs.PushBack(getterFunc);
    }

    static inline void RegisterSystem()
    {
        auto engine = CGameEngine::Get();

        if (!engine || !engine->framework)
        {
            TypeInfoRegistrar::AddDescribeCallback(&RegisterSystem);
            return;
        }

        auto gameInstance = engine->framework->gameInstance;
        auto systemType = GetClass<TSystem>();

        if (gameInstance->systemMap.Get(systemType))
            return;

        auto systemInstance = BuildSystem();

        if (!systemInstance)
            return;

        JobHandle job{};
        systemInstance->gameInstance = gameInstance;
        systemInstance->OnInitialize(job);

        gameInstance->systemInstances.PushBack(systemInstance);
        gameInstance->systemMap.Insert(systemType, systemInstance);
    }

    static inline Handle<TSystem> BuildSystem()
    {
        if constexpr (Detail::HasSystemGetter<TSystem>)
        {
            return TSystem::Get();
        }
        else
        {
            return MakeHandle<TSystem>();
        }
    }
};

template<typename TClass, Scope AScope>
struct ClassExpansion
{
    using Descriptor = ClassDescriptor<TClass>;
    using Specialization = TypeInfoBuilder<AScope>;

    static inline void DescribeType()
    {
        if constexpr (Detail::HasDescribeHandler<Specialization, Descriptor>)
        {
            constexpr auto name = GetTypeName<TClass>();

            auto* rtti = CRTTISystem::Get();
            auto* type = reinterpret_cast<Descriptor*>(rtti->GetClass(name));

            if (!type)
                return;

            Specialization::Describe(type);
        }
    }

    inline static TypeInfoRegistrar s_registrar{nullptr, &DescribeType}; // NOLINT(cert-err58-cpp)

    constexpr operator Scope() const noexcept
    {
        return AScope;
    }
};

template<typename TEnum, bool AFlags = false>
struct EnumDefinition
{
    using Descriptor = EnumDescriptor<TEnum>;
    using Specialization = TypeInfoBuilder<Scope::For<TEnum>()>;

    static inline void RegisterType()
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

    static inline void DescribeType()
    {
        constexpr auto name = GetTypeName<TEnum>();

        auto* rtti = CRTTISystem::Get();
        auto* type = reinterpret_cast<Descriptor*>(rtti->GetEnum(name));

        constexpr auto min = Detail::GetMinValue<Specialization, TEnum>();
        constexpr auto max = Detail::GetMaxValue<Specialization, TEnum>();

        type->template AddOptions<TEnum, min, max, AFlags>();

        if constexpr (Detail::HasDescribeHandler<Specialization, Descriptor>)
        {
            Specialization::Describe(type);
        }
    }

    inline static TypeInfoRegistrar s_registrar{&RegisterType, &DescribeType}; // NOLINT(cert-err58-cpp)

    constexpr operator Scope() const noexcept
    {
        return Scope::For<TEnum>();
    }
};

template<typename TEnum>
struct FlagsDefinition : EnumDefinition<TEnum, true> {};

template<typename TEnum, Scope AScope>
struct EnumExpansion
{
    using Descriptor = EnumDescriptor<TEnum>;
    using Specialization = TypeInfoBuilder<AScope>;

    static inline void DescribeType()
    {
        if constexpr (Detail::HasDescribeHandler<Specialization, Descriptor>)
        {
            constexpr auto name = GetTypeName<TEnum>();

            auto* rtti = CRTTISystem::Get();
            auto* type = reinterpret_cast<Descriptor*>(rtti->GetEnum(name));

            if (!type)
                return;

            Specialization::Describe(type);
        }
    }

    inline static TypeInfoRegistrar s_registrar{nullptr, &DescribeType}; // NOLINT(cert-err58-cpp)

    constexpr operator Scope() const noexcept
    {
        return AScope;
    }
};

template<Scope AScope>
struct GlobalDefinition
{
    using Descriptor = GlobalDescriptor;
    using Specialization = TypeInfoBuilder<AScope>;

    static inline void Register()
    {
        if constexpr (Detail::HasRegisterHandler<Specialization, Descriptor>)
        {
            auto* rtti = reinterpret_cast<Descriptor*>(CRTTISystem::Get());
            Specialization::Register(rtti);
        }
    }

    static inline void Describe()
    {
        if constexpr (Detail::HasDescribeHandler<Specialization, Descriptor>)
        {
            auto* rtti = reinterpret_cast<Descriptor*>(CRTTISystem::Get());
            Specialization::Describe(rtti);
        }
    }

    inline static TypeInfoRegistrar s_registrar{&Register, &Describe}; // NOLINT(cert-err58-cpp)

    constexpr operator Scope() const noexcept
    {
        return AScope;
    }
};
}
