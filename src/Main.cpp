#include "stdafx.hpp"
#include "Loader.hpp"

BOOL APIENTRY DllMain(HMODULE aModule, DWORD aReason, LPVOID aReserved)
{
    switch (aReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(aModule);
            AXL::Loader::Construct();
            break;
        }
        case DLL_PROCESS_DETACH:
        {
            AXL::Loader::Destruct();
            break;
        }
    }

    return TRUE;
}
