#pragma once

#include "Core/Win.hpp"

namespace Core
{
class OwnerMutex
{
public:
    explicit OwnerMutex(std::wstring_view aName);
    ~OwnerMutex();

    bool Obtain();
    bool Release();
    bool IsOwner();

private:
    std::wstring_view m_name;
    HANDLE m_mutex;
};
}
