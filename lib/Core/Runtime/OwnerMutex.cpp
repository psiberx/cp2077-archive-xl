#include "OwnerMutex.hpp"

Core::OwnerMutex::OwnerMutex(std::wstring_view aName)
    : m_name(aName)
    , m_mutex(nullptr)
{
}

Core::OwnerMutex::~OwnerMutex()
{
    Release();
}

bool Core::OwnerMutex::Obtain()
{
    const auto mutex = CreateMutexW(NULL, TRUE, m_name.data());

    if (!mutex)
        return false;

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        ReleaseMutex(mutex);
        return false;
    }

    m_mutex = mutex;

    return true;
}

bool Core::OwnerMutex::Release()
{
    if (!m_mutex)
        return false;

    ReleaseMutex(m_mutex);
    m_mutex = nullptr;

    return true;
}

bool Core::OwnerMutex::IsOwner()
{
    return m_mutex;
}
