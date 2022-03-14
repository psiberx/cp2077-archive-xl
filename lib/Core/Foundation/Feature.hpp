#pragma once

#include "Registrar.hpp"
#include "RegistrarProxy.hpp"

namespace Core
{
class Application;

class Feature : public RegistrarProxy<Feature>
{
public:
    Feature() = default;
    virtual ~Feature() = default;

    Feature(Feature&& aOther) = delete;
    Feature(const Feature& aOther) = delete;

protected:
    virtual void OnRegister() {};
    virtual void OnBootstrap() {};
    virtual void OnShutdown() {};

    friend class Application;
    friend class Registrar<Feature>;
};
}
