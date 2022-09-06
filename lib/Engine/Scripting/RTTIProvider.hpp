#pragma once

#include "Core/Foundation/Feature.hpp"

namespace Engine
{
class RTTIProvider : public Core::Feature
{
    void OnBootstrap() override;
};
}
