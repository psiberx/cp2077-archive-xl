#pragma once

#include "Resolving.hpp"

namespace Red
{
namespace Detail
{
constexpr auto TDBIDHelper = Red::CName("gamedataTDBIDHelper");

constexpr bool IsFakeStatic(Red::CName aTypeName)
{
    return aTypeName == TDBIDHelper;
}

inline CBaseFunction* GetFunction(CClass* aType, CName aName)
{
    if (aType)
    {
        for (auto func : aType->funcs)
        {
            if (func->shortName == aName || func->fullName == aName)
            {
                return func;
            }
        }

        if (aType->parent)
        {
            return GetFunction(aType->parent, aName);
        }
    }

    return nullptr;
}

inline CBaseFunction* GetStaticFunction(CClass* aType, CName aName)
{
    if (aType)
    {
        for (auto func : aType->staticFuncs)
        {
            if (func->shortName == aName || func->fullName == aName)
            {
                return func;
            }
        }

        if (IsFakeStatic(aType->name))
        {
            return GetFunction(aType, aName);
        }

        if (aType->parent)
        {
            return GetStaticFunction(aType->parent, aName);
        }
    }

    return nullptr;
}

inline CBaseFunction* GetStaticFunction(CName aType, CName aName)
{
    return GetStaticFunction(GetClass(aType), aName);
}

inline CBaseFunction* GetGlobalFunction(CName aName)
{
    return CRTTISystem::Get()->GetFunction(aName);
}

template<typename... Args>
inline bool CallFunction(CBaseFunction* aFunc, IScriptable* aContext, Args&&... aArgs)
{
    if (!aFunc)
        return false;

    const auto combinedArgCount = aFunc->params.size + (aFunc->returnType ? 1 : 0);

    if (combinedArgCount != sizeof...(Args))
        return false;

    if (!aFunc->flags.isStatic)
    {
        const auto& func = reinterpret_cast<Red::CClassFunction*>(aFunc);

        if (!IsFakeStatic(func->parent->name))
        {
            if (!aContext || !aContext->GetType()->IsA(func->parent))
                return false;
        }
        else
        {
            static char s_dummyContext[sizeof(Red::IScriptable)]{};
            aContext = reinterpret_cast<IScriptable*>(&s_dummyContext);
        }
    }

    CStack stack(aContext);
    StackArgs_t args;

    if (combinedArgCount > 0)
    {
        ((args.emplace_back(Red::ResolveType<Args>(), const_cast<std::remove_cvref_t<Args>*>(&aArgs))), ...);

        if (aFunc->returnType)
        {
            stack.result = args.data();

            if (!Red::IsCompatible(stack.result->type, aFunc->returnType->type))
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
                if (!Red::IsCompatible(aFunc->params[i]->type, stack.args[i].type, stack.args[i].value))
                {
                    return false;
                }
            }
        }
    }

    return aFunc->Execute(&stack);
}
}

template<typename... Args>
inline bool CallVirtual(IScriptable* aContext, CClass* aType, CName aFunc, Args&&... aArgs)
{
    return Detail::CallFunction(Detail::GetFunction(aType, aFunc), aContext, std::forward<Args>(aArgs)...);
}

template<typename... Args>
inline bool CallVirtual(IScriptable* aContext, CName aFunc, Args&&... aArgs)
{
    return CallVirtual(aContext, aContext->GetType(), aFunc, std::forward<Args>(aArgs)...);
}

template<typename... Args>
inline bool CallStatic(CClass* aType, CName aFunc, Args&&... aArgs)
{
    return Detail::CallFunction(Detail::GetStaticFunction(aType, aFunc), nullptr, std::forward<Args>(aArgs)...);
}

template<typename... Args>
inline bool CallStatic(CName aType, CName aFunc, Args&&... aArgs)
{
    return Detail::CallFunction(Detail::GetStaticFunction(aType, aFunc), nullptr, std::forward<Args>(aArgs)...);
}

template<typename... Args>
inline bool CallGlobal(CName aFunc, Args&&... aArgs)
{
    return Detail::CallFunction(Detail::GetGlobalFunction(aFunc), nullptr, std::forward<Args>(aArgs)...);
}
}
