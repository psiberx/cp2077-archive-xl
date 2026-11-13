#pragma once

#include "Resolving.hpp"

namespace Red
{
namespace Detail
{
constexpr bool IsFakeStatic(CName aTypeName)
{
    return aTypeName == Red::CName("gamedataTDBIDHelper");
}

using CallFunction_t = bool (*)(IFunction*, IScriptable* aContext, CStackFrame* aFrame, void* aRet, void* aRetType);

#if defined(RED4EXT_V1_SDK_VERSION_CURRENT) || defined(RED4EXT_SDK_0_5_0)
inline UniversalRelocFunc<CallFunction_t> CallFunctionWithFrame(RED4ext::Detail::AddressHashes::CBaseFunction_InternalExecute);
#else
inline RelocFunc<CallFunction_t> CallFunctionWithFrame(RED4ext::Addresses::CBaseFunction_InternalExecute);
#endif

inline bool CallFunctionWithStack(Red::CStackFrame* aFrame, CBaseFunction* aFunc, CStack& aStack)
{
    constexpr uint8_t NopOp = 0;
    constexpr uint8_t ParamOp = 27;
    constexpr uint8_t ParamEndOp = 38;
    constexpr uint32_t OpSize = sizeof(NopOp);
    constexpr uint32_t PointerSize = sizeof(void*);
    constexpr uint32_t MaxParams = 15;
    constexpr uint32_t MaxCodeSize = (MaxParams * (OpSize + PointerSize * 2)) + OpSize;

    if (aFunc->params.Size() > MaxParams)
    {
        return false;
    }

    char code[MaxCodeSize];
    CStackFrame frame(nullptr, code);

    for (uint32_t i = 0; i < aFunc->params.Size(); ++i)
    {
        const auto& param = aFunc->params[i];
        const auto& arg = aStack.args[i];

        if (param->flags.isOptional && !arg.value)
        {
            *frame.code = NopOp;
            ++frame.code;
        }
        else
        {
            *frame.code = ParamOp;
            ++frame.code;

            *reinterpret_cast<void**>(frame.code) = arg.type;
            frame.code += PointerSize;

            *reinterpret_cast<void**>(frame.code) = arg.value;
            frame.code += PointerSize;
        }
    }

    *frame.code = ParamEndOp;
    frame.code = code; // Rewind

    if (aFrame)
    {
        frame.func = aFrame->func;
    }
    else
    {
        frame.func = aFunc;
    }

    static auto* s_dummyContext = reinterpret_cast<IScriptable*>(Red::GetClass("entEntity")->CreateInstance());

    return CallFunctionWithFrame(aFunc,
                                 aStack.context18 ? aStack.context18 : s_dummyContext,
                                 &frame,
                                 aStack.result ? aStack.result->value : nullptr,
                                 aStack.result ? aStack.result->type : nullptr);
}

inline bool CallFunctionWithStack(CBaseFunction* aFunc, CStack& aStack)
{
    return CallFunctionWithStack(nullptr, aFunc, aStack);
}

template<typename... Args>
inline bool CallFunctionWithArgs(Red::CStackFrame* aFrame, CBaseFunction* aFunc, IScriptable* aContext, Args&&... aArgs)
{
    if (!aFunc)
        return false;

    const auto combinedArgCount = aFunc->params.Size() + (aFunc->returnType ? 1 : 0);

    if (combinedArgCount != sizeof...(Args))
        return false;

    if (!aFunc->flags.isStatic)
    {
        const auto& memberFunc = reinterpret_cast<CClassFunction*>(aFunc);

        if (!IsFakeStatic(memberFunc->parent->name))
        {
            if (!aContext || !aContext->GetType()->IsA(memberFunc->parent))
                return false;
        }
        // else
        // {
        //     static char s_dummyContext[sizeof(IScriptable)]{};
        //     aContext = reinterpret_cast<IScriptable*>(&s_dummyContext);
        // }
    }

    CStack stack(aContext);
    StackArgs_t args;

    if (combinedArgCount > 0)
    {
        ((args.emplace_back(ResolveType<Args>(),
                            const_cast<std::remove_cvref_t<Args>*>(std::is_null_pointer_v<Args> ? nullptr : &aArgs))),
         ...);

        if (aFunc->returnType)
        {
            stack.result = args.data();

            if (!IsCompatible(stack.result->type, aFunc->returnType->type))
            {
                return false;
            }

            if (!aFunc->params.IsEmpty())
            {
                stack.args = args.data() + 1;
            }
        }
        else
        {
            stack.args = args.data();
        }

        if (!aFunc->params.IsEmpty())
        {
            stack.argsCount = aFunc->params.Size();

            for (uint32_t i = 0; i < aFunc->params.Size(); ++i)
            {
                const auto& param = aFunc->params[i];
                const auto& arg = stack.args[i];

                if (arg.value)
                {
                    if (!IsCompatible(param->type, arg.type, arg.value))
                    {
                        return false;
                    }
                }
                else if (!param->flags.isOptional)
                {
                    return false;
                }
            }
        }
    }

    return CallFunctionWithStack(aFrame, aFunc, stack);
}

inline HashMap<uint64_t, CBaseFunction*> resolvedFunctionMap{Memory::DefaultAllocator::Get()};
inline SharedSpinLock resolvedFunctionLock;

constexpr uint64_t CombineHash(uint64_t a, uint64_t b)
{
    constexpr uint64_t magic = 0xC6A4A7935BD1E995;
    return magic * ((magic * ((magic * b) ^ ((magic * b) >> 47))) ^ a);
}

inline std::pair<CBaseFunction*, bool> GetResolvedFunction(uint64_t aScope, uint64_t aKey)
{
    if (aScope)
    {
        aKey = CombineHash(aKey, aScope);
    }

    std::shared_lock _(resolvedFunctionLock);
    const auto it = resolvedFunctionMap.Get(aKey);

    if (it != nullptr)
    {
        return {*it, true};
    }
    else
    {
        return {nullptr, false};
    }
}

inline void PutResolvedFunction(uint64_t aScope, uint64_t aKey, CBaseFunction* aFunc)
{
    if (aScope)
    {
        aKey = CombineHash(aKey, aScope);
    }

    std::unique_lock _(resolvedFunctionLock);
    resolvedFunctionMap.InsertOrAssign(aKey, aFunc);
}
}

inline CBaseFunction* GetFunction(CName aName)
{
    auto [resolvedFunc, wasResolved] = Detail::GetResolvedFunction(0, aName);

    if (!wasResolved)
    {
        resolvedFunc = CRTTISystem::Get()->GetFunction(aName);

        Detail::PutResolvedFunction(0, aName, resolvedFunc);
    }

    return resolvedFunc;
}

inline CBaseFunction* GetFunction(CClass* aType, CName aName, bool aMember = true, bool aStatic = true)
{
    if (!aType)
    {
        return nullptr;
    }

    auto [resolvedFunc, wasResolved] = Detail::GetResolvedFunction(aType->name, aName);

    if (!wasResolved)
    {
        if (aMember || Detail::IsFakeStatic(aType->name))
        {
            for (auto memberFunc : aType->funcs)
            {
                if (memberFunc->shortName == aName || memberFunc->fullName == aName)
                {
                    resolvedFunc = memberFunc;
                    break;
                }
            }
        }

        if (!resolvedFunc && aStatic)
        {
            for (auto staticFunc : aType->staticFuncs)
            {
                if (staticFunc->shortName == aName || staticFunc->fullName == aName)
                {
                    resolvedFunc = staticFunc;
                    break;
                }
            }
        }

        if (!resolvedFunc && aType->parent)
        {
            resolvedFunc = GetFunction(aType->parent, aName, aMember, aStatic);
        }

        Detail::PutResolvedFunction(aType->name, aName, resolvedFunc);
    }

    return resolvedFunc;
}

inline CBaseFunction* GetFunction(CName aType, CName aName, bool aMember = true, bool aStatic = true)
{
    return GetFunction(GetClass(aType), aName, aMember, aStatic);
}

inline CBaseFunction* GetMemberFunction(IScriptable* aContext, CName aName)
{
    return aContext ? GetFunction(aContext->GetType(), aName, true, false) : nullptr;
}

inline CBaseFunction* GetMemberFunction(CClass* aType, CName aName)
{
    return GetFunction(aType, aName, true, false);
}

inline CBaseFunction* GetMemberFunction(CName aType, CName aName)
{
    return GetFunction(GetClass(aType), aName, true, false);
}

inline CBaseFunction* GetStaticFunction(CClass* aType, CName aName)
{
    return GetFunction(aType, aName, false, true);
}

inline CBaseFunction* GetStaticFunction(CName aType, CName aName)
{
    return GetFunction(GetClass(aType), aName, false, true);
}

inline CBaseFunction* GetGlobalFunction(CName aName)
{
    return GetFunction(aName);
}

inline bool CallFunction(Red::CStackFrame* aFrame, CBaseFunction* aFunc, CStack& aStack)
{
    return Detail::CallFunctionWithStack(aFrame, aFunc, aStack);
}

inline bool CallFunction(CBaseFunction* aFunc, CStack& aStack)
{
    return Detail::CallFunctionWithStack(nullptr, aFunc, aStack);
}

template<typename... Args>
inline bool CallFunction(Red::CStackFrame* aFrame, IScriptable* aContext, CBaseFunction* aFunc, Args&&... aArgs)
{
    return Detail::CallFunctionWithArgs(aFrame, aFunc, aContext, std::forward<Args>(aArgs)...);
}

template<typename... Args>
inline bool CallFunction(IScriptable* aContext, CBaseFunction* aFunc, Args&&... aArgs)
{
    return Detail::CallFunctionWithArgs(nullptr, aFunc, aContext, std::forward<Args>(aArgs)...);
}

template<typename... Args>
inline bool CallFunction(CBaseFunction* aFunc, Args&&... aArgs)
{
    return Detail::CallFunctionWithArgs(nullptr, aFunc, nullptr, std::forward<Args>(aArgs)...);
}

template<typename... Args>
inline bool CallVirtual(IScriptable* aContext, CClass* aType, CName aFunc, Args&&... aArgs)
{
    if (!aContext)
        return false;

    return Detail::CallFunctionWithArgs(nullptr, GetMemberFunction(aType, aFunc), aContext, std::forward<Args>(aArgs)...);
}

template<typename... Args>
inline bool CallVirtual(IScriptable* aContext, CName aFunc, Args&&... aArgs)
{
    if (!aContext)
        return false;

    return Detail::CallFunctionWithArgs(nullptr, GetMemberFunction(aContext->GetType(), aFunc), aContext, std::forward<Args>(aArgs)...);
}

template<typename... Args>
inline bool CallStatic(CClass* aType, CName aFunc, Args&&... aArgs)
{
    return Detail::CallFunctionWithArgs(nullptr, GetStaticFunction(aType, aFunc), nullptr, std::forward<Args>(aArgs)...);
}

template<typename... Args>
inline bool CallStatic(CName aType, CName aFunc, Args&&... aArgs)
{
    return Detail::CallFunctionWithArgs(nullptr, GetStaticFunction(aType, aFunc), nullptr, std::forward<Args>(aArgs)...);
}

template<typename... Args>
inline bool CallGlobal(CName aFunc, Args&&... aArgs)
{
    return Detail::CallFunctionWithArgs(nullptr, GetFunction(aFunc), nullptr, std::forward<Args>(aArgs)...);
}
}
