#pragma once

#include "Core/Container/Container.hpp"
#include "Core/Stl.hpp"

namespace Core
{
template<class R>
class RegistryProxy;

template<class R>
class Registry
{
public:
    template<class T, typename... Args>
    requires std::is_base_of_v<R, T>
    T& Register(Args&&... aArgs)
    {
        auto registrable = Core::MakeShared<T>(std::forward<Args>(aArgs)...);
        m_registered.push_back(registrable);
        Container::Set(registrable);

        if constexpr (std::is_base_of_v<RegistryProxy<R>, T>)
        {
            registrable->SetParent(*this);
        }

        OnRegistered(registrable);

        return *registrable.get();
    }

protected:
    using TraverseFunc = void (*)(const Core::SharedPtr<R>&);

    void ForEach(TraverseFunc aTraverse) const
    {
        std::for_each(m_registered.begin(), m_registered.end(), aTraverse);
    }

    const Core::Vector<Core::SharedPtr<R>>& GetRegistered() const
    {
        return m_registered;
    }

    template<typename T>
    inline static Core::SharedPtr<T> Resolve()
    {
        return Container::Get<T>();
    }

    template<typename T>
    inline static bool Resolvable()
    {
        return Container::Get<T>();
    }

    virtual void OnRegistered(const Core::SharedPtr<R>& aRegistred) {}

private:
    Core::Vector<Core::SharedPtr<R>> m_registered;
};

template<class R>
class RegistryProxy
{
protected:
    template<class T, typename... Args>
    requires std::is_base_of_v<R, T>
    T& Register(Args&&... aArgs)
    {
        assert(m_parent);
        return m_parent->template Register<T>(std::forward<Args>(aArgs)...);
    }

    template<class T>
    requires std::is_base_of_v<R, T>
    T& Register(Core::SharedPtr<R> aFeature)
    {
        assert(m_parent);
        return m_parent->Register(std::forward(aFeature));
    }

private:
    friend class Registry<R>;

    void SetParent(Registry<R>& aParent)
    {
        m_parent = &aParent;
    }

    Registry<R>* m_parent{ nullptr };
};
}
