#include "stdafx.hpp"
#include "Environment.hpp"

Environment::Environment()
{
    TCHAR filePath[MAX_PATH] = { 0 };
    GetModuleFileName(GetModuleHandle(nullptr), filePath, std::size(filePath));

    m_exe = filePath;
    m_root = m_exe.parent_path() // x64
            .parent_path()       // bin
            .parent_path();      // root
}

std::filesystem::path Environment::GetGameExe() const
{
    return m_exe;
}

std::filesystem::path Environment::GetRootDir() const
{
    return m_root;
}

std::filesystem::path Environment::GetArchiveModDir() const
{
    return m_root / L"archive" / L"pc" / L"mod";
}
