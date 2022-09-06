#pragma once

#include "HookingDriver.hpp"

namespace Core::Detail
{
template<typename R, typename... Args>
using OriginalFunc = R (*)(Args...);

template<typename C, typename R, typename... Args>
using MemberCallback = R (C::*)(Args...);

/**
 * @deprecated
 */
template<typename F>
struct BaseCapture
{
    inline static bool hooked = false;
};

/**
 * @tparam F
 * @tparam C
 * @tparam R
 * @tparam Args
 * @deprecated
 */
template<typename F, typename C, typename R, typename... Args>
struct MemberCapture : BaseCapture<F>
{
    inline static OriginalFunc<R, Args...> original;
    inline static MemberCallback<C, R, Args...> callback;
    inline static C* context;

    static R Handle(Args... aArgs)
    {
        return (context->*callback)(aArgs...);
    }
};
}
