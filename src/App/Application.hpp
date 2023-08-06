#pragma once

#include "Core/Foundation/Application.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class Application
    : public Core::Application
    , public Core::LoggingAgent
{
public:
    explicit Application(HMODULE aHandle, const RED4ext::Sdk* aSdk = nullptr);

protected:
    void OnStarting() override;
};
}
