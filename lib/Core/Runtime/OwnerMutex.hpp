#pragma once

#include "Core/Win.hpp"

namespace Core
{
class OwnerMutex
{
public:
    explicit OwnerMutex(std::string_view aName);
    explicit OwnerMutex(std::wstring_view aName);
    ~OwnerMutex();

    bool Obtain();
    bool Release();
    bool IsOwner();

private:
    std::string_view m_aname;
    std::wstring_view m_wname;
    HANDLE m_mutex;
};
}
