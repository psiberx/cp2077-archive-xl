#pragma once

namespace Core
{
class RawBase
{
public:
    inline static uintptr_t GetImageBase()
    {
        static const uintptr_t addr = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
        return addr;
    }
};

template<uintptr_t A, typename F>
struct RawFunc {};

template<uintptr_t A, typename R, typename... Args>
struct RawFunc<A, R (*)(Args...)> : private RawBase
{
    using Callable = R (*)(Args...);
    using ReturnType = R;

    static constexpr uintptr_t offset = A;

    inline operator Callable() const
    {
        return GetPtr();
    }

    operator bool() const noexcept
    {
        return GetPtr() != nullptr;
    }

    [[nodiscard]] inline Callable GetPtr() const noexcept
    {
        return reinterpret_cast<Callable>(GetAddress());
    }

    inline static uintptr_t GetAddress() noexcept
    {
        static uintptr_t addr = offset ? offset + GetImageBase() : 0;
        return addr;
    }

    static R Invoke(Args... aArgs)
    {
        return reinterpret_cast<Callable>(GetAddress())(std::forward<Args>(aArgs)...);
    }
};

template<uintptr_t A, typename C, typename R, typename... Args>
struct RawFunc<A, R (C::*)(Args...)> : RawFunc<A, R (*)(C*, Args...)>
{
};

template<uintptr_t A, typename T>
struct RawPtr : private RawBase
{
    using Type = T;

    static constexpr uintptr_t offset = A;

    [[nodiscard]] inline T* operator->() const
    {
        return GetPtr();
    }

    [[nodiscard]] inline operator T*() const
    {
        return GetPtr();
    }

    operator bool() const noexcept
    {
        return GetPtr() != nullptr;
    }

    [[nodiscard]] inline T* GetPtr() const noexcept
    {
        return reinterpret_cast<T*>(GetAddress());
    }

    inline static uintptr_t GetAddress() noexcept
    {
        static uintptr_t addr = offset ? offset + GetImageBase() : 0;
        return addr;
    }

    static T* Get()
    {
        return reinterpret_cast<T*>(GetAddress());
    }
};
}
