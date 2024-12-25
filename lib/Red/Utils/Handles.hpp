#pragma once

namespace Red
{
template<typename T, typename U = T>
inline WeakHandle<U>& AsWeakHandle(T* aInstance)
{
    return *reinterpret_cast<WeakHandle<U>*>(&aInstance->ref);
}

template<typename T, typename U = T>
inline Handle<U> AsHandle(T* aInstance)
{
    return AsWeakHandle(aInstance);
}

template<typename T, typename U = T>
inline WeakHandle<U> ToWeakHandle(T* aInstance)
{
    if (!aInstance)
        return {};

    return AsWeakHandle(aInstance);
}

template<typename T, typename U = T>
inline WeakHandle<U> ToWeakHandle(const Handle<T>& aInstance)
{
    return aInstance;
}

template<typename T>
inline Handle<T> ToHandle(T* aInstance)
{
    if (!aInstance)
        return {};

    if (aInstance->ref.instance)
    {
        return reinterpret_cast<WeakHandle<T>*>(&aInstance->ref)->Lock();
    }
    else
    {
        return Handle<T>(aInstance);
    }
}

template<typename T, typename U>
inline Handle<T> ToHandle(U* aInstance)
{
    if (!aInstance)
        return {};

    auto instance = reinterpret_cast<ISerializable*>(aInstance);
    if (instance->ref.instance)
    {
        return reinterpret_cast<WeakHandle<T>*>(&instance->ref)->Lock();
    }
    else
    {
        return Handle<T>(reinterpret_cast<T*>(instance));
    }
}

template<typename T = ISerializable>
inline Handle<T> MakeScriptedHandle(CClass* aType)
{
    return Handle<T>(reinterpret_cast<T*>(aType->CreateInstance(true)));
}

template<typename T = ISerializable>
inline Handle<T> MakeScriptedHandle(CName aTypeName)
{
    auto classType = GetClass(aTypeName);

    if (classType == nullptr)
    {
        return {};
    }
    return Handle<T>(reinterpret_cast<T*>(classType->CreateInstance(true)));
}
}
