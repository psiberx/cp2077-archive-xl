#pragma once

namespace Red::Log
{
inline void Channel(CName aChannel, const CString& aMessage)
{
    static auto* s_rtti = CRTTISystem::Get();
    static auto* s_logFunc = s_rtti->GetFunction("LogChannel");
    static auto* s_stringType = s_rtti->GetType("String");
    static auto* s_stringRefType = s_rtti->GetType("script_ref:String");
    static auto* s_nameType = s_rtti->GetType("CName");

    ScriptRef<CString> messageRef;
    messageRef.type = s_stringType;
    messageRef.name = s_stringType->GetName();
    messageRef.ref = const_cast<CString*>(&aMessage);

    StackArgs_t args;
    args.emplace_back(s_nameType, &aChannel);
    args.emplace_back(s_stringRefType, &messageRef);

    CStack stack(nullptr, args.data(), static_cast<uint32_t>(args.size()), nullptr);

    s_logFunc->Execute(&stack);
}

inline void Channel(Red::CName aChannel, const std::string& aMessage)
{
    Channel(aChannel, Red::CString(aMessage.c_str()));
}

template<typename... Args>
constexpr void Channel(CName aChannel, std::format_string<Args...> aFormat, Args&&... aArgs)
{
    Channel(aChannel, std::format(aFormat, std::forward<Args>(aArgs)...));
}

template<typename... Args>
constexpr void Debug(std::format_string<Args...> aFormat, Args&&... aArgs)
{
    Channel("DEBUG", std::format(aFormat, std::forward<Args>(aArgs)...));
}
}
