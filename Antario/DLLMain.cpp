#include <thread>
#include "Hooks.h"
#include "Utils\Utils.h"
#include "Utils\GlobalVars.h"

DWORD WINAPI OnDllAttach( PVOID moduleBase )
{
    while ( !GetModuleHandle( "serverbrowser.dll" ) )
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

    Hooks::Init( );

    while ( !GetAsyncKeyState( VK_DELETE ) )
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

    Hooks::Restore( );

    FreeLibraryAndExitThread( static_cast< HMODULE >( moduleBase ), 0 );
}

BOOL WINAPI DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
    if ( dwReason == DLL_PROCESS_ATTACH )
    {
        DisableThreadLibraryCalls( hModule );

        const auto handle = CreateThread( NULL, NULL, OnDllAttach, hModule, NULL, NULL );
        if ( handle )
            CloseHandle( handle );
    }

    return TRUE;
}

