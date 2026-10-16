#pragma once

#include <filesystem>
#include <vector>

#if defined(_WIN32)
#include <libloaderapi.h>
#include <minwindef.h>
#else
#include <dlfcn.h>
#endif

#include <RED4ext/Callback.hpp>
#include <RED4ext/Memory/Allocators.hpp>
#include <RED4ext/RTTISystem.hpp>

class ArchiveXL
{
public:
    inline static bool RegisterArchive(std::filesystem::path aPath)
    {
        std::error_code error;

        if (!std::filesystem::exists(aPath, error) || !std::filesystem::is_regular_file(aPath, error))
        {
            return false;
        }

        return RegisterPathOrQueue(std::move(aPath));
    }

    inline static bool RegisterArchive(HMODULE aHandle, std::filesystem::path aPath)
    {
        if (aPath.is_relative())
        {
            aPath = GetModulePath(aHandle) / aPath;
        }

        return RegisterArchive(std::move(aPath));
    }

    inline static bool RegisterArchives(std::filesystem::path aPath)
    {
        std::error_code error;

        if (!std::filesystem::exists(aPath, error) || !std::filesystem::is_directory(aPath, error))
        {
            return false;
        }

        return RegisterPathOrQueue(std::move(aPath));
    }

    inline static bool RegisterArchives(HMODULE aHandle, std::filesystem::path aPath)
    {
        if (aPath.is_relative())
        {
            aPath = GetModulePath(aHandle) / aPath;
        }

        return RegisterArchives(std::move(aPath));
    }

private:
    ArchiveXL() = default;

#if !defined(_WIN32)
    // Same test as the Windows branch above: the PoolEngine pool is in the vault's pool registry. Vault::Get() cannot
    // be used here, so the vault is taken from the game's PoolStorageProxy<PoolEngine>::GetAllocator(). That storage
    // is set up when the game image loads and its allocator is the DefaultAllocator, the first member of the vault.
    // The getter only reads that static and allocates nothing, so it is safe at any time.
    inline static bool IsPoolEngineRegistered()
    {
        using GetVault_t = RED4ext::Memory::Vault* (*)();

        static const auto getVault = reinterpret_cast<GetVault_t>(
            dlsym(RTLD_MAIN_ONLY, "_ZN3red6memory16PoolStorageProxyINS_10PoolEngineEE12GetAllocatorEv"));

        if (!getVault)
            return false;

        auto vault = getVault();
        return vault && vault->poolRegistry.Get(RED4ext::Memory::PoolEngine::Name);
    }

#endif
    inline static bool Initialize()
    {
        if (s_facade)
            return true;

#if defined(_WIN32)
        if (!RED4ext::Memory::Vault::Get()->poolRegistry.Get(RED4ext::Memory::PoolEngine::Name))
            return false;
#else
        if (!IsPoolEngineRegistered())
            return false;
#endif

        auto facade = RED4ext::CRTTISystem::Get()->GetClass("ArchiveXL");

        if (!facade || !facade->GetFunction("Version"))
            return false;

        s_facade = facade;
        return true;
    }

    inline static bool RegisterPathOrQueue(std::filesystem::path aPath)
    {
        if (!Initialize())
        {
            s_paths.push_back(std::move(aPath));

            if (s_paths.size() == 1)
            {
                RegisterPendingPaths();
            }

            return true;
        }

        return RegisterPath(aPath);
    }

    inline static bool RegisterPath(const std::filesystem::path& aPath)
    {
        bool success;
        RED4ext::CString path(aPath.string());

        auto rtti = RED4ext::CRTTISystem::Get();
        RED4ext::CStackType arg(rtti->GetType("String"), &path);
        RED4ext::CStackType result(rtti->GetType("Bool"), &success);
        RED4ext::CStack stack(nullptr, &arg, 1, &result);

        std::error_code error;
        RED4ext::CName method(std::filesystem::is_directory(aPath, error) ? "RegisterDir" : "RegisterArchive");

        s_facade->GetFunction(method)->Execute(&stack);

        return success;
    }

    static inline void RegisterPendingPaths()
    {
        if (Initialize())
        {
            for (const auto& path : s_paths)
            {
                RegisterPath(path);
            }

            s_paths.clear();
        }
        else
        {
            RED4ext::CRTTISystem::Get()->AddPostRegisterCallback(&RegisterPendingPaths);
        }
    }

    inline static std::filesystem::path GetModulePath(HMODULE aHandle)
    {
#if defined(_WIN32)
        wchar_t path[MAX_PATH]{0};
        auto length = GetModuleFileNameW(aHandle, path, MAX_PATH);

        return std::filesystem::path(path).parent_path();
#else
        // The plugin that includes this header: find the image that holds this function.
        (void)aHandle;
        Dl_info info{};
        if (dladdr(reinterpret_cast<const void*>(&GetModulePath), &info) && info.dli_fname)
        {
            return std::filesystem::path(info.dli_fname).parent_path();
        }

        return {};
#endif
    }

    inline static RED4ext::CClass* s_facade;
    inline static std::vector<std::filesystem::path> s_paths;
};
