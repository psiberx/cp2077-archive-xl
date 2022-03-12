#pragma once

#include <TiltedCore/StlAllocator.hpp>
#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>
#include <map>
#include <memory>
#include <type_traits>
#include <vector>

namespace Core
{
// Pretty much the same as TiltedCore with StlAllocator,
// but unique ptr allows implicit casting to the base.

namespace detail
{
template<class T, class U>
concept ChildOf = std::is_base_of<U, T>::value;

template<class T>
struct UniqueDeleter
{
    constexpr UniqueDeleter() noexcept = default;

    template<ChildOf<T> U>
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

template<class T>
using SharedPtr = std::shared_ptr<T>;

template<class T>
using UniquePtr = std::unique_ptr<T, detail::UniqueDeleter<T>>;

template<typename T, typename... Args>
auto MakeShared(Args&&... args)
{
    return std::allocate_shared<T>(TiltedPhoques::StlAllocator<T>(), std::forward<Args>(args)...);
}

template<typename T, typename... Args>
auto MakeUnique(Args&&... aArgs)
{
    return UniquePtr<T>(TiltedPhoques::New<T>(std::forward<Args>(aArgs)...));
}
}
