#include "WorldWidgetLimitPatch.hpp"

void App::WorldWidgetLimitPatch::OnBootstrap()
{
    auto& code = Core::RawPtr<Red::AddressLib::InkWorldLayer_UpdateComponents, uint8_t[0x100]>::Ref();
    uint8_t op[] = {0x83, 0xFB, 0x0A, 0x73, 0x16};
    bool success = false;

    DWORD oldProtect = 0;
    VirtualProtect(code, sizeof(code), PAGE_EXECUTE_WRITECOPY, &oldProtect);

    auto it = std::search(std::begin(code), std::end(code), std::begin(op), std::end(op));
    if (it != std::end(code))
    {
        *(it + 3) = 0x90;
        *(it + 4) = 0x90;
        success = true;
    }

    VirtualProtect(code, sizeof(code), oldProtect, nullptr);

    if (success)
    {
        LogInfo("WorldWidgetComponent limit patch is successfully applied.");
    }
    else
    {
        LogWarning("WorldWidgetComponent limit patch is not applied, expected pattern was not found.");
    }
}
