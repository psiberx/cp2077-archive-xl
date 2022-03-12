#pragma once

#include <cstdint>

namespace Core
{
template<uintptr_t A, typename F>
struct RawFunc {};

template<uintptr_t A, typename R, typename... Args>
struct RawFunc<A, R (*)(Args...)>
{
    using type = R (*)(Args...);
    using return_type = R;

    static constexpr uintptr_t offset = A;

    inline operator type() const
    {
        return GetPtr();
    }

    operator bool() const noexcept
    {
        return GetPtr() != nullptr;
    }

    [[nodiscard]] inline type GetPtr() const noexcept
    {
        return reinterpret_cast<type>(RelocAddr());
    }

    inline static uintptr_t RelocAddr() noexcept
    {
        static uintptr_t addr = offset ? offset + reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)) : 0;

        return addr;
    }
};

template<uintptr_t A, typename T>
struct RawPtr
{
    using type = T;

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
        return reinterpret_cast<T*>(RelocAddr());
    }

    inline static uintptr_t RelocAddr() noexcept
    {
        static uintptr_t addr = offset ? offset + reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)) : 0;

        return addr;
    }
};
}
