#pragma once

#include <TiltedCore/StlAllocator.hpp>
#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>

namespace Core
{
// Pretty much the same as TiltedCore with StlAllocator,
// but unique ptr allows implicit casting to the base.

namespace Detail
{
template<class T>
struct UniqueDeleter
{
    constexpr UniqueDeleter() noexcept = default;

    template<class U>
    requires std::is_base_of_v<T, U>
    UniqueDeleter(const UniqueDeleter<U>& d) noexcept {}

    void operator()(std::conditional_t<std::is_array_v<T>, T, T*> aData)
    {
        TiltedPhoques::Delete<T>(aData);
    }
};
}

template<class T>
using Vector = std::vector<T, TiltedPhoques::StlAllocator<T>>;

template<class T>
using Set = tsl::hopscotch_set<T, std::hash<T>, std::equal_to<T>, TiltedPhoques::StlAllocator<T>>;

template<class T, class U>
using Map = tsl::hopscotch_map<T, U, std::hash<T>, std::equal_to<T>, TiltedPhoques::StlAllocator<std::pair<T, U>>>;

template<class T, class U>
using SortedMap = std::map<T, U, std::less<T>, TiltedPhoques::StlAllocator<std::pair<const T, U>>>;

// TODO: OrderedMap

template<class T>
using SharedPtr = std::shared_ptr<T>;

template<class T>
using WeakPtr = std::weak_ptr<T>;

template<class T>
using UniquePtr = std::unique_ptr<T, Detail::UniqueDeleter<T>>;

template<typename T>
struct ShareFromThis : public std::enable_shared_from_this<T>
{
    SharedPtr<T> ToShared()
    {
        return std::enable_shared_from_this<T>::shared_from_this();
    }

    WeakPtr<T> ToWeak()
    {
        return std::enable_shared_from_this<T>::weak_from_this();
    }
};

template<typename T, typename... Args>
auto MakeShared(Args&&... aArgs)
{
    return std::allocate_shared<T>(TiltedPhoques::StlAllocator<T>(), std::forward<Args>(aArgs)...);
}

template<typename T, typename... Args>
auto MakeUnique(Args&&... aArgs)
{
    return UniquePtr<T>(TiltedPhoques::New<T>(std::forward<Args>(aArgs)...));
}
}
