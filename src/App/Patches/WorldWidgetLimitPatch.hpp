#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class WorldWidgetLimitPatch
    : public Core::Feature
    , public Core::LoggingAgent
{
protected:
    void OnBootstrap() override;
};
}
