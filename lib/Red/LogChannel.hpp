#pragma once

namespace Red::Log
{
inline void Channel(Red::CName aChannel, const std::string& aMessage)
{
    static auto* s_rtti = CRTTISystem::Get();
    static auto* s_logFunc = s_rtti->GetFunction("LogChannel");
    static auto* s_stringType = s_rtti->GetType("String");
    static auto* s_nameType = s_rtti->GetType("CName");

    CString message(aMessage.c_str());
    ScriptRef<CString> messageRef;
    messageRef.innerType = s_stringType;
    messageRef.hash = s_stringType->GetName();
    messageRef.ref = &message;

    StackArgs_t args;
    args.emplace_back(s_nameType, &aChannel);
    args.emplace_back(s_stringType, &messageRef);

    CStack stack(nullptr, args.data(), static_cast<uint32_t>(args.size()), nullptr);

    s_logFunc->Execute(&stack);
}

template<typename... Args>
constexpr void Channel(CName aChannel, std::_Fmt_string<Args...> aFormat, Args&&... aArgs)
{
    Channel(aChannel, std::format(aFormat, std::forward<Args>(aArgs)...));
}

template<typename... Args>
constexpr void Debug(std::_Fmt_string<Args...> aFormat, Args&&... aArgs)
{
    Channel("DEBUG", std::format(aFormat, std::forward<Args>(aArgs)...));
}
}
