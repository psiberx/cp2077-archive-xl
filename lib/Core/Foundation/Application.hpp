#pragma once

#include "Core/Container/Registry.hpp"
#include "Core/Foundation/Feature.hpp"
#include "Core/Stl.hpp"

namespace Core
{
using AutoDiscoveryCallback = void(*)(Application&);

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

    static bool Discover(AutoDiscoveryCallback aCallback);

protected:
    void OnRegistered(const SharedPtr<Feature>& aFeature) override;

    virtual void OnStarting() {};
    virtual void OnStarted() {};
    virtual void OnStopping() {};
    virtual void OnStopped() {};

private:
    bool m_booted = false;

    inline static Vector<AutoDiscoveryCallback> s_discoveryCallbacks;
};
}
