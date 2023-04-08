#pragma once

#include "Core/Foundation/Feature.hpp"

namespace Support
{
class RedLibProvider : public Core::Feature
{
    void OnBootstrap() override;
};
}
