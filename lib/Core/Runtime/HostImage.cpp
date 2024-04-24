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

    TryResolveVersion(filePath);
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

const Core::FileVer& Core::HostImage::GetFileVer() const
{
    return m_fileVer;
}

const Core::SemvVer& Core::HostImage::GetProductVer() const
{
    return m_productVer;
}

bool Core::HostImage::TryResolveVersion(const std::wstring& filePath)
{
    auto size = GetFileVersionInfoSizeW(filePath.c_str(), nullptr);
    if (!size)
        return false;

    std::unique_ptr<uint8_t[]> data(new (std::nothrow) uint8_t[size]());
    if (!data)
        return false;

    if (!GetFileVersionInfoW(filePath.c_str(), 0, size, data.get()))
        return false;

    VS_FIXEDFILEINFO* fileInfo = nullptr;
    UINT fileInfoBytes;

    if (!VerQueryValueW(data.get(), L"\\", reinterpret_cast<LPVOID*>(&fileInfo), &fileInfoBytes))
        return false;

    constexpr auto signature = 0xFEEF04BD;
    if (fileInfo->dwSignature != signature)
        return false;

    m_fileVer.major = (fileInfo->dwFileVersionMS >> 16) & 0xFF;
    m_fileVer.minor = fileInfo->dwFileVersionMS & 0xFFFF;
    m_fileVer.build = (fileInfo->dwFileVersionLS >> 16) & 0xFFFF;
    m_fileVer.revision = fileInfo->dwFileVersionLS & 0xFFFF;

    m_productVer.major = (fileInfo->dwProductVersionMS >> 16) & 0xFF;
    m_productVer.minor = fileInfo->dwProductVersionMS & 0xFFFF;
    m_productVer.patch = (fileInfo->dwProductVersionLS >> 16) & 0xFFFF;

    return true;
}
