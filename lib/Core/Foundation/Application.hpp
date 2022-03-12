#pragma once

#include "Feature.hpp"
#include "Core/Stl.hpp"

namespace Core
{
namespace detail
{
template<class T>
concept AFeature = std::is_base_of<Feature, T>::value;
}

class Application
{
public:
    void Register(Core::UniquePtr<Feature> aFeature);

    template<detail::AFeature T, typename... Args>
    void Register(Args&&... aArgs)
    {
        Register(Core::MakeUnique<T>(std::forward<Args>(aArgs)...));
    }

    template<detail::AFeature T>
    void Register(struct T::Options&& aOptions)
    {
        Register(Core::MakeUnique<T>(std::forward<struct T::Options>(aOptions)));
    }

    void Bootstrap();
    void Shutdown();

private:
    std::vector<Core::UniquePtr<Feature>> m_features;
    bool m_booted{ false };
};
}
