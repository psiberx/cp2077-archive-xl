#pragma once

#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class Module
    : public Core::HookingAgent
    , public Core::LoggingAgent
{
public:
    Module() = default;
    virtual ~Module() = default;

    virtual std::string_view GetName() = 0;
    virtual bool Load() = 0;
    virtual bool Unload() = 0;
};

class ConfigurableModule : public Module
{
public:
    virtual bool Configure(const std::string& aConfigName, const YAML::Node& aConfigNode) = 0;
    virtual void Reset() = 0;
};
}
