#pragma once

#include "Core/Foundation/Application.hpp"

namespace App
{
class Application : public Core::Application
{
public:
    explicit Application(HMODULE aHandle, const RED4ext::Sdk* aSdk = nullptr);
};
}
