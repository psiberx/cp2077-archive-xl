#pragma once

#include "Common.hpp"
#include "Construction.hpp"
#include "Resolving.hpp"

namespace Red
{
template<typename T, auto V = 0>
struct Optional;

template<typename T, auto ADefault>
requires (!std::is_void_v<T>)
struct Optional<T, ADefault>
{
    Optional() = default;
    
    Optional(T&& aValue)
        : value(aValue)
    {
    }

    template<typename U, typename = std::enable_if_t<!std::is_same_v<U, T>>>
    [[nodiscard]] inline explicit operator U()
    {
        return static_cast<U>(value);
    }

    inline operator T&()
    {
        return value;
    }

    [[nodiscard]] inline T* operator->()
    {
        return &value;
    }

    template<typename U>
    Optional& operator=(const U& aValue) noexcept
    {
        value = aValue;
        return *this;
    }

    template<typename U>
    Optional& operator=(U&& aValue) noexcept
    {
        value = aValue;
        return *this;
    }

    [[nodiscard]] bool IsEmpty()
    {
        return !value;
    }

    [[nodiscard]] bool IsDefault() const
    {
        return value == ADefault;
    }

    T value{ADefault};
};

template<typename T>
struct Optional<T, 0>
{
    template<typename U, typename = std::enable_if_t<!std::is_same_v<U, T>>>
    [[nodiscard]] inline explicit operator U()
    {
        return static_cast<U>(value);
    }

    inline operator T&()
    {
        return value;
    }

    [[nodiscard]] inline T* operator->()
    {
        return &value;
    }

    template<typename U>
    Optional& operator=(const U& aValue) noexcept
    {
        value = aValue;
        return *this;
    }

    template<typename U>
    Optional& operator=(U&& aValue) noexcept
    {
        value = aValue;
        return *this;
    }

    [[nodiscard]] bool IsEmpty()
    {
        return !value;
    }

    T value{};
};

template<typename T, typename U = std::remove_cvref_t<T>>
concept IsOptional = Detail::Specialization<U>::value
    && std::is_same_v<U, Optional<typename Detail::Specialization<U>::argument_type,
                                                   Detail::Specialization<U>::argument_value>>;

template<typename T>
struct ScriptRef
{
    explicit ScriptRef(bool aAllocate = false)
        : unk00(nullptr)
        , managed(false)
        , type(nullptr)
        , ref(nullptr)
    {
        if (managed)
        {
            AllocateValue();
        }
    }

    ~ScriptRef()
    {
        FreeValue();
    }

    inline explicit operator bool()
    {
        return ref;
    }

    [[nodiscard]] inline T& operator*()
    {
        return *ref;
    }

    [[nodiscard]] inline T* operator->()
    {
        return ref;
    }

    inline auto operator[](size_t aIndex)
    {
        return (*ref)[aIndex];
    }

    inline ScriptRef& operator=(const T& aValue) noexcept
    {
        if (ref)
        {
            *ref = aValue;
        }

        return *this;
    }

    inline ScriptRef& operator=(T&& aValue) noexcept
    {
        if (ref)
        {
            *ref = aValue;
        }

        return *this;
    }

    inline void AllocateValue()
    {
        if (!managed)
        {
            managed = true;
            type = GetType<T>();
            ref = Construct<T>();
        }
    }

    inline void FreeValue()
    {
        if (managed)
        {
            managed = false;
            Destruct(ref);
        }
    }

    void* unk00;         // 00 - VFT
    bool managed;        // 08 - Acrually a pointer, but never used
    CBaseRTTIType* type; // 10
    T* ref;              // 18
    CName name;          // 20
};

template<typename T, typename U = std::remove_cvref_t<T>>
concept IsScriptRef = Detail::Specialization<U>::value
    && std::is_same_v<U, ScriptRef<typename Detail::Specialization<U>::argument_type>>;

RTTI_MAP_TYPE_PROXY(Optional);
RTTI_MAP_TYPE_PREFIX(ScriptRef, "script_ref:");
}
