#pragma once

#include "Core/Container/Registry.hpp"

namespace Core
{
class Application;

class Feature : public RegistryProxy<Feature>
{
public:
    Feature() = default;
    virtual ~Feature() = default;

    Feature(Feature&& aOther) = delete;
    Feature(const Feature& aOther) = delete;

protected:
    friend class Application;
    friend class Registry<Feature>;

    virtual void OnRegister() {};
    virtual void OnInitialize() {};
    virtual void OnBootstrap() {};
    virtual void OnShutdown() {};

    template<typename T>
    requires std::is_base_of_v<Feature, T>
    class Defer
    {
    public:
        inline Defer(T& aTarget)
            : m_instance(aTarget)
        {
            ++m_instance.m_deferChain;
        }

        inline Defer(T* aTarget)
            : m_instance(*aTarget)
        {
            ++m_instance.m_deferChain;
        }

        inline ~Defer()
        {
            if (--m_instance.m_deferChain == 0)
            {
                static_cast<Feature&>(m_instance).OnInitialize();
            }
        }

        [[nodiscard]] inline T* operator->() const
        {
            return &m_instance;
        }

    private:
        T& m_instance;
    };

private:
    uint8_t m_deferChain = 0;
};
}
