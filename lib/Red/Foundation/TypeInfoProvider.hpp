#pragma once

#include "Core/Foundation/Feature.hpp"

namespace Red
{
class TypeInfoProvider : public Core::Feature
{
    void OnBootstrap() override;
};
}
