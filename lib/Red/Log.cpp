#include "Log.hpp"

void Red::Log::Channel(Red::CName aChannel, const std::string& aMessage)
{
    static auto* rtti = CRTTISystem::Get();
    static auto* logFunc = rtti->GetFunction("LogChannel");
    static auto* stringType = rtti->GetType("String");
    static auto* nameType = rtti->GetType("CName");

    CString message(aMessage.c_str());
    ScriptRef<CString> messageRef;
    messageRef.innerType = stringType;
    messageRef.hash = stringType->GetName();
    messageRef.ref = &message;

    StackArgs_t args;
    args.emplace_back(nameType, &aChannel);
    args.emplace_back(stringType, &messageRef);

    CStack stack(nullptr, args.data(), static_cast<uint32_t>(args.size()), nullptr);

    logFunc->Execute(&stack);
}
