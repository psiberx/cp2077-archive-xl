#include "Prefix.hpp"

Red::CName App::ComponentPrefixResolver::GetPrefix(Red::CName aComponentName)
{
    if (!aComponentName)
        return {};

    auto it = m_resolved.find(aComponentName);

    if (it == m_resolved.end())
    {
        const auto nameStr = aComponentName.ToString();

        size_t prefixEnd = 2;
        constexpr size_t prefixMaxLen = 4;
        while (prefixEnd < prefixMaxLen && nameStr[prefixEnd] && nameStr[prefixEnd] != '_')
            ++prefixEnd;

        uint64_t prefixHash = 0;
        if (prefixEnd < prefixMaxLen && nameStr[prefixEnd] == '_')
            prefixHash = Red::CNamePool::Add(std::string().append(nameStr, prefixEnd + 1).c_str());

        it = m_resolved.emplace(aComponentName, prefixHash).first;
    }

    return it.value();
}

Core::SharedPtr<App::ComponentPrefixResolver>& App::ComponentPrefixResolver::Get()
{
    static auto s_instance = Core::MakeShared<ComponentPrefixResolver>();
    return s_instance;
}
