#pragma once

namespace Red
{
// TODO: Use RED4ext::SharedPtrBase for smart pointers

using Instance = void*;

template<typename T = void>
using InstancePtr = Core::SharedPtr<T>;

template<typename T, typename... Args>
InstancePtr<T> MakeInstance(Args&&... args)
{
    return Core::MakeShared<T>(std::forward<Args>(args)...);
}

template<typename T = void>
struct Value
{
    Value(CBaseRTTIType* aType = nullptr, T* aInstance = nullptr)
        : type(aType)
        , instance(aInstance)
    {
    }

    Value(const CStackType& aData)
        : type(aData.type)
        , instance(aData.value)
    {
    }

    [[nodiscard]] inline T* operator->() const
    {
        return instance;
    }

    [[nodiscard]] inline operator T*() const
    {
        return instance;
    }

    [[nodiscard]] inline operator CStackType*() const
    {
        return reinterpret_cast<CStackType*>(this);
    }

    [[nodiscard]] inline operator const CStackType&() const
    {
        return *reinterpret_cast<const CStackType*>(this);
    }

    explicit operator bool() const noexcept
    {
        return instance != nullptr;
    }

    template<typename U>
    requires (std::is_pointer_v<U>)
    inline U As()
    {
        return *reinterpret_cast<U>(instance);
    }

    template<typename U>
    requires (!std::is_pointer_v<U>)
    inline U& As()
    {
        return *reinterpret_cast<U*>(instance);
    }

    CBaseRTTIType* type;
    T* instance;
};

template<typename T = void>
struct ManagedValue : Value<T>
{
    using Data = Value<T>;

    ManagedValue(CBaseRTTIType* aType, void* aInstance = nullptr)
        : Data(aType)
    {
        Data::instance = Data::type->GetAllocator()->AllocAligned(Data::type->GetSize(), Data::type->GetAlignment()).memory;
        if (aInstance)
        {
            Data::type->Assign(Data::instance, aInstance);
        }
        else
        {
            std::memset(Data::instance, 0, Data::type->GetSize());
            Data::type->Construct(Data::instance);
        }
    }

    template<typename... Args>
    requires (!std::is_void_v<T>)
    ManagedValue(CBaseRTTIType* aType, Args&&... aArgs)
        : Data(aType)
    {
        Data::instance = Data::type->GetAllocator()->AllocAligned(Data::type->GetSize(), Data::type->GetAlignment()).memory;
        new (Data::instance) T(std::forward<Args>(aArgs)...);
    }

    ~ManagedValue()
    {
        if (Data::type && Data::instance)
        {
            Data::type->Destruct(Data::instance);
            Data::type->GetAllocator()->Free(Data::instance);
        }
    }

    ManagedValue& operator=(void* aInstance)
    {
        Data::type->Assign(Data::instance, aInstance);
    }
};

template<typename T = void>
using ValuePtr = Core::SharedPtr<const ManagedValue<T>>;

template<typename T, typename... Args>
requires (!std::is_void_v<T>)
ValuePtr<T> MakeValue(CBaseRTTIType* aType, Args&&... aArgs)
{
    return Core::MakeShared<ManagedValue<T>>(aType, std::forward<Args>(aArgs)...);
}

template<typename T = void>
ValuePtr<T> MakeValue(CBaseRTTIType* aType, void* aInstance = nullptr)
{
    return Core::MakeShared<ManagedValue<T>>(aType, aInstance);
}

template<typename T, typename... Args>
requires (!std::is_void_v<T>)
ValuePtr<T> MakeValue(CName aTypeName, Args&&... aArgs)
{
    return Core::MakeShared<ManagedValue<T>>(CRTTISystem::Get()->GetType(aTypeName), std::forward<Args>(aArgs)...);
}

template<typename T = void>
ValuePtr<T> MakeValue(CName aTypeName, void* aInstance = nullptr)
{
    return Core::MakeShared<ManagedValue<T>>(CRTTISystem::Get()->GetType(aTypeName), aInstance);
}
}
