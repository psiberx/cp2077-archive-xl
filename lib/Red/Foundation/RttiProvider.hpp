#pragma once

#include "Core/Foundation/Feature.hpp"

namespace Red
{
class RttiProvider : public Core::Feature
{
    void OnBootstrap() override;
};
}
