#pragma once

#include "Core/Memory/AddressResolver.hpp"

namespace Core
{
class RawBase
{
public:
    constexpr RawBase() = default;

    inline static uintptr_t GetImageBase()
    {
        static const auto base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
        return base;
    }
};

template<auto A, typename T>
class RawFunc {};

template<auto A, typename R, typename... Args>
class RawFunc<A, R (*)(Args...)> : public RawBase
{
public:
    using Type = R (*)(Args...);
    using Callable = Type;

    static constexpr auto target = A;

    constexpr RawFunc() = default;

    operator bool() const noexcept
    {
        return GetPtr() != nullptr;
    }

    [[nodiscard]] inline operator Callable() const
    {
        return GetPtr();
    }

    [[nodiscard]] inline Callable operator~() const
    {
        return GetOriginalPtr();
    }

    [[nodiscard]] inline Callable GetPtr() const noexcept
    {
        return reinterpret_cast<Callable>(GetAddress());
    }

    [[nodiscard]] inline Callable GetOriginalPtr() const noexcept
    {
        return reinterpret_cast<Callable>(GetOriginalAddress());
    }

    inline static uintptr_t GetAddress() noexcept
    {
        if (!original)
        {
            ResetAddress();
        }

        return address;
    }

    inline static uintptr_t GetOriginalAddress() noexcept
    {
        if (!original)
        {
            ResetAddress();
        }

        return original;
    }

    inline static void ResetAddress() noexcept
    {
        static_assert(std::is_integral_v<decltype(target)>, "Unsupported address type");

        if constexpr (sizeof(target) == sizeof(uintptr_t))
        {
            original = target ? target + GetImageBase() : 0;
        }
        else
        {
            original = AddressResolver::GetDefault().ResolveAddress(target);
        }

        address = original;
    }

    inline static void SetAddress(uintptr_t aAddress) noexcept
    {
        if (!original)
        {
            ResetAddress();
        }

        address = aAddress;
    }

    inline static auto Invoke(Args... aArgs)
    {
        return reinterpret_cast<Callable>(GetAddress())(std::forward<Args>(aArgs)...);
    }

    inline static auto InvokeOriginal(Args... aArgs)
    {
        return reinterpret_cast<Callable>(GetOriginalAddress())(std::forward<Args>(aArgs)...);
    }

private:
    inline static uintptr_t address = 0;
    inline static uintptr_t original = 0;
};

template<auto A, typename C, typename R, typename... Args>
class RawFunc<A, R (C::*)(Args...)> : public RawFunc<A, R (*)(C*, Args...)>
{
public:
    using Base = RawFunc<A, R (*)(C*, Args...)>;
    using Base::Base;
};

template<uintptr_t A, typename T>
class RawVFunc {};

template<uintptr_t A, typename C, typename R, typename... Args>
class RawVFunc<A, R (*)(C*, Args...)> : public RawBase
{
public:
    using Type = R (*)(C*, Args...);
    using Callable = Type;

    static constexpr uintptr_t offset = A;

    constexpr RawVFunc() = default;

    auto operator()(C* aContext, Args... aArgs) const
    {
        auto vft = *reinterpret_cast<uintptr_t*>(aContext);
        auto callable = *reinterpret_cast<Callable*>(vft + offset);
        return callable(aContext, std::forward<Args>(aArgs)...);
    }
};

template<auto A, typename C, typename R, typename... Args>
class RawVFunc<A, R (C::*)(Args...)> : public RawVFunc<A, R (*)(C*, Args...)>
{
public:
    using Base = RawVFunc<A, R (*)(C*, Args...)>;
    using Base::Base;
};

template<auto A, typename T>
class RawPtr : public RawBase
{
public:
    using Type = std::conditional_t<std::is_void_v<std::remove_pointer_t<T>>, void*, std::remove_pointer_t<T>>;

    static constexpr auto target = A;
    static constexpr bool indirect = std::is_pointer_v<T> && !std::is_void_v<std::remove_pointer_t<T>>;

    constexpr RawPtr() = default;

    operator bool() const noexcept
    {
        return GetPtr() != nullptr;
    }

    [[nodiscard]] inline operator Type&() const
    {
        if constexpr (indirect)
        {
            return **GetPtr();
        }
        else
        {
            return *GetPtr();
        }
    }

    [[nodiscard]] inline operator Type*() const
    {
        if constexpr (indirect)
        {
            return *GetPtr();
        }
        else
        {
            return GetPtr();
        }
    }

    [[nodiscard]] inline Type* operator->() const
    {
        if constexpr (indirect)
        {
            return *GetPtr();
        }
        else
        {
            return GetPtr();
        }
    }

    [[nodiscard]] inline T& operator[](size_t aIndex) const
    {
        return GetPtr()[aIndex];
    }

    RawPtr& operator=(T&& aRhs) const noexcept
    {
        *GetPtr() = aRhs;
        return *this;
    }

    [[nodiscard]] inline T* GetPtr() const noexcept
    {
        return reinterpret_cast<T*>(GetAddress());
    }

    [[nodiscard]] inline T& GetRef() const noexcept
    {
        return *reinterpret_cast<T*>(GetAddress());
    }

    inline static uintptr_t GetAddress() noexcept
    {
        static_assert(std::is_integral_v<decltype(target)>, "Unsupported address type");

        if constexpr (sizeof(target) == sizeof(uintptr_t))
        {
            static uintptr_t addr = target ? target + GetImageBase() : 0;
            return addr;
        }
        else
        {
            static uintptr_t addr = AddressResolver::GetDefault().ResolveAddress(target);
            return addr;
        }
    }

    inline static T* Get()
    {
        return reinterpret_cast<T*>(GetAddress());
    }

    inline static T& Ref()
    {
        return *reinterpret_cast<T*>(GetAddress());
    }
};

template<uintptr_t A, typename T>
class OffsetPtr
{
public:
    using Type = T;
    using ValueType = std::conditional_t<std::is_void_v<std::remove_pointer_t<T>>, void*, std::remove_pointer_t<T>>;

    static constexpr uintptr_t offset = A;
    static constexpr bool indirect = std::is_pointer_v<T> && !std::is_void_v<std::remove_pointer_t<T>>;

    constexpr OffsetPtr(uintptr_t aBase)
        : address(aBase + offset)
    {
    }

    constexpr OffsetPtr(const void* aBase)
        : address(reinterpret_cast<uintptr_t>(aBase) + offset)
    {
    }

    [[nodiscard]] inline operator bool() const
    {
        if constexpr (std::is_same_v<ValueType, bool>)
        {
            return GetValuePtr() && *GetValuePtr();
        }
        else
        {
            return GetValuePtr();
        }
    }

    [[nodiscard]] inline operator ValueType&() const
    {
        return *GetValuePtr();
    }

    [[nodiscard]] inline operator ValueType*() const
    {
        return GetValuePtr();
    }

    [[nodiscard]] inline ValueType* operator->() const
    {
        return GetValuePtr();
    }

    OffsetPtr& operator=(Type&& aRhs) const noexcept
    {
        *GetPtr() = aRhs;
        return *this;
    }

    OffsetPtr& operator=(const Type& aRhs) const noexcept
    {
        *GetPtr() = aRhs;
        return *this;
    }

    [[nodiscard]] inline Type* GetPtr() const noexcept
    {
        return reinterpret_cast<Type*>(GetAddress());
    }

    [[nodiscard]] inline ValueType* GetValuePtr() const noexcept
    {
        if constexpr (indirect)
        {
            return *GetPtr();
        }
        else
        {
            return GetPtr();
        }
    }

    [[nodiscard]] inline uintptr_t GetAddress() const noexcept
    {
        return address;
    }

    inline static Type* Ptr(const void* aBase)
    {
        return OffsetPtr(aBase).GetPtr();
    }

    inline static Type& Ref(const void* aBase)
    {
        return *OffsetPtr(aBase).GetPtr();
    }

    inline static uintptr_t Addr(const void* aBase)
    {
        return reinterpret_cast<uintptr_t>(OffsetPtr(aBase).GetPtr());
    }

    inline static void Set(const void* aBase, const Type& aValue)
    {
        *OffsetPtr(aBase).GetPtr() = aValue;
    }

    inline static ValueType* ValuePtr(const void* aBase)
    {
        return OffsetPtr(aBase).GetValuePtr();
    }

    inline static ValueType& ValueRef(const void* aBase)
    {
        return *OffsetPtr(aBase).GetValuePtr();
    }

    inline static uintptr_t ValueAddr(const void* aBase)
    {
        return reinterpret_cast<uintptr_t>(OffsetPtr(aBase).GetValuePtr());
    }

    inline static void SetValue(const void* aBase, const ValueType& aValue)
    {
        *OffsetPtr(aBase).GetValuePtr() = aValue;
    }

    uintptr_t address;
};
}
