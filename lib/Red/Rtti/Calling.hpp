#pragma once

#include "Resolving.hpp"

namespace Red
{
template<typename... Args>
inline bool CallVirtual(IScriptable* aContext, CClass* aType, CName aFunc, Args&&... aArgs)
{
    const auto func = aType->GetFunction(aFunc);

    if (!func)
        return false;

    const auto combinedArgCount = func->params.size + (func->returnType ? 1 : 0);

    if (combinedArgCount != sizeof...(Args))
        return false;

    CStack stack(aContext);
    StackArgs_t args;

    if (combinedArgCount > 0)
    {
        ((args.emplace_back(nullptr, &aArgs)), ...);

        if (func->returnType)
        {
            stack.result = args.data();
            stack.result->type = func->returnType->type;

            if (func->params.size)
            {
                stack.args = args.data() + 1;
            }
        }
        else
        {
            stack.args = args.data();
        }

        if (func->params.size)
        {
            stack.argsCount = func->params.size;

            for (uint32_t i = 0; i < func->params.size; ++i)
            {
                stack.args[i].type = func->params[i]->type;
            }
        }
    }

    return func->Execute(&stack);
}

template<typename... Args>
inline bool CallVirtual(IScriptable* aContext, CName aFunc, Args&&... aArgs)
{
    return CallVirtual(aContext, aContext->GetType(), aFunc, std::forward<Args>(aArgs)...);
}
}
