#pragma once

#include "Registrar.hpp"

#include <cassert>

namespace Core
{
template<class R>
class RegistrarProxy
{
protected:
    template<class T, typename... Args>
    requires std::is_base_of_v<R, T>
    inline void Register(Args&&... aArgs)
    {
        assert(m_parent);
        m_parent->template Register<T>(std::forward<Args>(aArgs)...);
    }

    template<class T>
    requires std::is_base_of_v<R, T>
    inline void Register(struct T::Options&& aOptions)
    {
        assert(m_parent);
        m_parent->template Register<T>(std::forward<struct T::Options>(aOptions));
    }

    inline void Register(Core::UniquePtr<R> aFeature)
    {
        assert(m_parent);
        m_parent->Register(std::forward(aFeature));
    }

private:
    friend class Registrar<R>;

    void SetParentRegistrar(Registrar<R>& aParent)
    {
        m_parent = &aParent;
    }

    Registrar<R>* m_parent{ nullptr };
};
}
