#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/Rtti/Class.hpp"

namespace App
{
class Facade : public Red::Rtti::Class<Facade>
{
public:
    static void Reload();
    static bool Require(Red::CString& aVersion);
    static Red::CString GetVersion();

private:
    friend Descriptor;
    static void OnRegister(Descriptor* aType);
    static void OnDescribe(Descriptor* aType);
};
}
