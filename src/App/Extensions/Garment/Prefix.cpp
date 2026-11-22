#include "Prefix.hpp"

Red::CName App::ComponentPrefixResolver::GetPrefix(Red::CName aComponentName)
{
    if (!aComponentName)
        return {};

    {
        std::shared_lock _(m_lock);

        auto it = m_resolved.find(aComponentName);
        if (it != m_resolved.end())
        {
            return it.value();
        }
    }

    const auto nameStr = aComponentName.ToString();

    size_t prefixEnd = 2;
    constexpr size_t prefixMaxLen = 6;
    while (prefixEnd < prefixMaxLen && nameStr[prefixEnd] && nameStr[prefixEnd] != '_')
    {
        ++prefixEnd;
    }

    uint64_t prefixHash = 0;
    if (prefixEnd < prefixMaxLen && nameStr[prefixEnd] == '_')
    {
        prefixHash = Red::CNamePool::Add(std::string().append(nameStr, prefixEnd + 1).c_str());
    }

    {
        std::unique_lock _(m_lock);

        m_resolved.emplace(aComponentName, prefixHash);
    }

    return prefixHash;
}

const Core::SharedPtr<App::ComponentPrefixResolver>& App::ComponentPrefixResolver::Get()
{
    static auto s_instance = Core::MakeShared<ComponentPrefixResolver>();
    return s_instance;
}
