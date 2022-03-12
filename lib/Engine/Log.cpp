#include "Log.hpp"

#include <RED4ext/CString.hpp>
#include <RED4ext/NativeTypes.hpp>
#include <RED4ext/RTTISystem.hpp>
#include <RED4ext/Scripting/Functions.hpp>
#include <RED4ext/Scripting/Stack.hpp>

void Engine::Log::Channel(RED4ext::CName aChannel, const std::string& aMessage)
{
    static auto* rtti = RED4ext::CRTTISystem::Get();
    static auto* logFunc = rtti->GetFunction("LogChannel");
    static auto* stringType = rtti->GetType("String");
    static auto* nameType = rtti->GetType("CName");

    RED4ext::CString message(aMessage.c_str());
    RED4ext::ScriptRef<RED4ext::CString> messageRef;
    messageRef.innerType = stringType;
    messageRef.hash = stringType->GetName();
    messageRef.ref = &message;

    RED4ext::StackArgs_t args;
    args.emplace_back(nameType, &aChannel);
    args.emplace_back(stringType, &messageRef);

    RED4ext::CStack stack(nullptr, args.data(), static_cast<uint32_t>(args.size()), nullptr);

    logFunc->Execute(&stack);
}
