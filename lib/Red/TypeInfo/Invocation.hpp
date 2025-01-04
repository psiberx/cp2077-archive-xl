#pragma once

#include "Resolving.hpp"

namespace Red
{
namespace Detail
{
constexpr bool IsFakeStatic(CName aTypeName)
{
    constexpr auto TDBIDHelper = Red::CName("gamedataTDBIDHelper");
    return aTypeName == TDBIDHelper;
}

using CallFunction_t = bool (*)(IFunction*, IScriptable* aContext, CStackFrame* aFrame, void* aRet, void* aRetType);

#ifdef RED4EXT_SDK_0_5_0
inline UniversalRelocFunc<CallFunction_t> CallFunctionWithFrame(RED4ext::Detail::AddressHashes::CBaseFunction_InternalExecute);
#else
inline RelocFunc<CallFunction_t> CallFunctionWithFrame(RED4ext::Addresses::CBaseFunction_InternalExecute);
#endif

inline bool CallFunctionWithStack(Red::CStackFrame* aFrame, CBaseFunction* aFunc, CStack& aStack)
{
    constexpr auto NopOp = 0;
    constexpr auto ParamOp = 27;
    constexpr auto ParamEndOp = 38;
    constexpr auto MaxCodeSize = 264;
    constexpr auto PointerSize = sizeof(void*);

    char code[MaxCodeSize];
    CStackFrame frame(nullptr, code);

    for (uint32_t i = 0; i < aFunc->params.size; ++i)
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

    const auto combinedArgCount = aFunc->params.size + (aFunc->returnType ? 1 : 0);

    if (combinedArgCount != sizeof...(Args))
        return false;

    if (!aFunc->flags.isStatic)
    {
        const auto& func = reinterpret_cast<CClassFunction*>(aFunc);

        if (!IsFakeStatic(func->parent->name))
        {
            if (!aContext || !aContext->GetType()->IsA(func->parent))
                return false;
        }
        else
        {
            static char s_dummyContext[sizeof(IScriptable)]{};
            aContext = reinterpret_cast<IScriptable*>(&s_dummyContext);
        }
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

            if (aFunc->params.size)
            {
                stack.args = args.data() + 1;
            }
        }
        else
        {
            stack.args = args.data();
        }

        if (aFunc->params.size)
        {
            stack.argsCount = aFunc->params.size;

            for (uint32_t i = 0; i < aFunc->params.size; ++i)
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
}

inline CBaseFunction* GetFunction(CClass* aType, CName aName, bool aMember = true, bool aStatic = true)
{
    if (aType)
    {
        if (aMember || Detail::IsFakeStatic(aType->name))
        {
            for (auto func : aType->funcs)
            {
                if (func->shortName == aName || func->fullName == aName)
                {
                    return func;
                }
            }
        }

        if (aStatic)
        {
            for (auto func : aType->staticFuncs)
            {
                if (func->shortName == aName || func->fullName == aName)
                {
                    return func;
                }
            }
        }

        if (aType->parent)
        {
            return GetFunction(aType->parent, aName, aMember, aStatic);
        }
    }

    return nullptr;
}

inline CBaseFunction* GetFunction(CName aType, CName aName, bool aMember = true, bool aStatic = true)
{
    return GetFunction(GetClass(aType), aName, aMember, aStatic);
}

inline CBaseFunction* GetFunction(CName aName)
{
    return CRTTISystem::Get()->GetFunction(aName);
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
