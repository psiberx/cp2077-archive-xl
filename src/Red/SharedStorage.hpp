#pragma once

namespace Red
{
template<CName N, typename T, typename... Args>
inline T* AcquireSharedInstance(Args&&... args)
{
    constexpr CName SharedName = FNV1a64("!", N);

    auto rtti = Red::CRTTISystem::Get();
    if (!rtti)
         return nullptr;

    auto* mutex = const_cast<SharedSpinLock*>(reinterpret_cast<volatile SharedSpinLock*>(&rtti->typesLock));
    auto* storage = reinterpret_cast<HashMap<CName, T*>*>(&rtti->unkF8);

    std::lock_guard _(*mutex);

    auto* entry = storage->Get(SharedName);
    if (entry)
         return *entry;

    static const auto s_instance = std::make_unique<T>(std::forward<Args>(args)...);
    storage->Insert(SharedName, s_instance.get());

    return s_instance.get();
}
}
