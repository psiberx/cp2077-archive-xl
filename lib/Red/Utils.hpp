#pragma once

namespace Red
{
template<typename T, typename U = T>
inline WeakHandle<U>& AsWeakHandle(T* aInstance)
{
    return *reinterpret_cast<WeakHandle<U>*>(&aInstance->ref);
}

template<typename T>
inline Handle<T> AsHandle(T* aInstance)
{
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
inline Handle<T> AsHandle(U* aInstance)
{
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
}
