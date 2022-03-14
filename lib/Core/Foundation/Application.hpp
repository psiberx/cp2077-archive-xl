#pragma once

#include "Feature.hpp"
#include "Registrar.hpp"
#include "Core/Stl.hpp"

namespace Core
{
class Application : public Registrar<Feature>
{
public:
    void Bootstrap();
    void Shutdown();

protected:
    void OnRegistered(const Core::UniquePtr<Feature>& aFeature) override;

private:
    bool m_booted{ false };
};
}
