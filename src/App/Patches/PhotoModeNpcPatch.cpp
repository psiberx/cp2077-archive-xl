#include "PhotoModeNpcPatch.hpp"

void App::PhotoModeNpcPatch::OnBootstrap()
{
    auto& code = Core::RawPtr<Red::AddressLib::PhotoModeSystem_ProcessAttribute, uint8_t[0x8000]>::Ref();
    uint8_t op[] = {0x0F, 0x82, 0x4A, 0xFF, 0xFF, 0xFF, 0x41, 0x8D, 0x40, 0xFE, 0x83, 0xF8, 0x18};
    bool success = false;

    DWORD oldProtect = 0;
    VirtualProtect(code, sizeof(code), PAGE_EXECUTE_WRITECOPY, &oldProtect);

    auto it = std::search(std::begin(code), std::end(code), std::begin(op), std::end(op));
    if (it != std::end(code))
    {
        *(it + sizeof(op) - 1) = 0xFF;
        success = true;
    }

    VirtualProtect(code, sizeof(code), oldProtect, nullptr);

    if (success)
    {
        LogInfo("Photo Mode NPC limit patch is successfully applied.");
    }
    else
    {
        LogWarning("Photo Mode NPC limit patch is not applied, expected pattern was not found.");
    }
}
