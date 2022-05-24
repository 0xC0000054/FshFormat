#include <Windows.h>

HANDLE hDllInstance = nullptr;
 
BOOL WINAPI DllMain(HANDLE hInstance, DWORD fdwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        hDllInstance = hInstance;
    }
    return TRUE;
}