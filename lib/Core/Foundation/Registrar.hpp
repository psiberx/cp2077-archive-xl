#pragma once

#include "Core/Stl.hpp"

namespace Core
{
template<class R>
class Registrar
{
public:
    template<class T, typename... Args>
    requires std::is_base_of_v<R, T>
    void Register(Args&&... aArgs)
    {
        Register(Core::MakeUnique<T>(std::forward<Args>(aArgs)...));
    }

    template<class T>
    requires std::is_base_of_v<R, T>
    void Register(struct T::Options&& aOptions)
    {
        Register(Core::MakeUnique<T>(std::forward<struct T::Options>(aOptions)));
    }

    void Register(Core::UniquePtr<R> aRegistrable)
    {
        const auto& registered = m_registered.emplace_back(std::move(aRegistrable));
        registered->SetParentRegistrar(*this);
        registered->OnRegister();

        OnRegistered(registered);
    }

protected:
    using TraverseFunc = void (*)(const Core::UniquePtr<R>&);

    void ForEach(TraverseFunc aTraverse) const
    {
        std::for_each(m_registered.begin(), m_registered.end(), aTraverse);
    }

    const Core::Vector<Core::UniquePtr<R>>& GetRegistered() const
    {
        return m_registered;
    }

    virtual void OnRegistered(const Core::UniquePtr<R>& aRegistrable) {}

private:
    Core::Vector<Core::UniquePtr<R>> m_registered;
};
}
