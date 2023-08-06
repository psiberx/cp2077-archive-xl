#pragma once

#include "Core/Container/Registry.hpp"
#include "Core/Foundation/Feature.hpp"
#include "Core/Stl.hpp"

namespace Core
{
class Application : public Registry<Feature>
{
public:
    template<class T, typename... Args>
    Feature::Defer<T> Register(Args&&... aArgs)
    {
        return Registry<Feature>::Register<T>(std::forward<Args>(aArgs)...);
    }

    void Bootstrap();
    void Shutdown();

protected:
    void OnRegistered(const Core::SharedPtr<Feature>& aFeature) override;

    virtual void OnStarting() {};
    virtual void OnStarted() {};
    virtual void OnStopping() {};
    virtual void OnStopped() {};

private:
    bool m_booted = false;
};
}
