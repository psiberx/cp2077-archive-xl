#include "HostImage.hpp"
#include "Core/Win.hpp"

Core::HostImage::HostImage(int aExePathDepth)
{
    const auto handle = GetModuleHandleW(nullptr);

    m_base = reinterpret_cast<uintptr_t>(handle);

    std::wstring filePath;
    wil::GetModuleFileNameW(handle, filePath);

    m_exe = filePath;
    m_root = m_exe.parent_path();

    while (--aExePathDepth >= 0)
        m_root = m_root.parent_path();
}

uintptr_t Core::HostImage::GetBase() const
{
    return m_base;
}

std::filesystem::path Core::HostImage::GetPath() const
{
    return m_exe;
}

std::string Core::HostImage::GetName() const
{
    return m_exe.stem().string();
}

std::filesystem::path Core::HostImage::GetRootDir() const
{
    return m_root;
}
