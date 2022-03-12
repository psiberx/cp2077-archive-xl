#pragma once

#include "stdafx.hpp"
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

    virtual bool Attach() = 0;
    virtual bool Detach() = 0;

    virtual std::string_view GetName() = 0;
};

class ConfigurableModule : public Module
{
public:
    virtual bool Configure(const std::string& aConfigName, const YAML::Node& aConfigNode) = 0;
    virtual void Reset() = 0;
};
}
