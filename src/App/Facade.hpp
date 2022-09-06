#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Engine/Scripting/RTTIClass.hpp"

namespace App
{
class Facade : public Engine::RTTIClass<Facade>
{
public:
    static void Reload();
    static RED4ext::CString GetVersion();

private:
    friend Descriptor;
    static void OnRegister(Descriptor* aType);
    static void OnDescribe(Descriptor* aType, RED4ext::CRTTISystem* aRtti);
};
}
