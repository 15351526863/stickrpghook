﻿#include <thread>
#include <array>
#include "Hooks.h"
#include "Utils\Utils.h"
#include "Features\Features.h"
#include "Menu\Menu.h"
#include "SDK\Hitboxes.h"
#include "libloaderapi.h"

Misc     g_Misc;
Hooks    g_Hooks;
Event    g_Event;

void Warning(const char* msg, ...) // wintergang https://www.unknowncheats.me/forum/1923881-post1.html
{
	if (msg == nullptr)
		return;
	typedef void(__cdecl* MsgFn)(const char* msg, va_list);
	static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandle("tier0.dll"), "Warning");
	char buffer[989];
	va_list list;
	va_start(list, msg);
	vsprintf(buffer, msg, list);
	va_end(list);
	fn(buffer, list);
}

void Hooks::Init( )
{
	// Get window handle
	while ( !( g_Hooks.hCSGOWindow = FindWindowA( "Valve001", nullptr ) ) )
	{
		using namespace std::literals::chrono_literals;
		std::this_thread::sleep_for( 50ms );
	}

	interfaces::Init( );                         // Get interfaces
	g_pNetvars = std::make_unique<NetvarTree>( );// Get netvars after getting interfaces as we use them
	netvar::Setup( );

	if ( g_Hooks.hCSGOWindow )        // Hook WNDProc to capture mouse / keyboard input
		g_Hooks.pOriginalWNDProc = reinterpret_cast< WNDPROC >( SetWindowLongPtr( g_Hooks.hCSGOWindow, GWLP_WNDPROC,
			reinterpret_cast< LONG_PTR >( g_Hooks.WndProc ) ) );

	// VMTHooks
	g_Hooks.pClientHook = std::make_unique<VMTHook>( g_pClientDll );
	g_Hooks.pClientModeHook = std::make_unique<VMTHook>( g_pClientMode );
	g_Hooks.pSurfaceHook = std::make_unique<VMTHook>( g_pSurface );
	g_Hooks.pPanelHook = std::make_unique<VMTHook>( g_pPanel );
	g_Hooks.pModelHook = std::make_unique<VMTHook>( g_pModelRender );
	g_Hooks.pRenderViewHook = std::make_unique<VMTHook>( g_pRenderView );

	// Hook the table functions
	g_Hooks.pClientHook->Hook( vtable_indexes::frameStage, Hooks::FrameStageNotify );
	g_Hooks.pClientHook->Hook( vtable_indexes::createMove, Hooks::CreateMoveProxy );
	g_Hooks.pClientModeHook->Hook( vtable_indexes::view, Hooks::OverrideView );
	g_Hooks.pSurfaceHook->Hook( vtable_indexes::lockCursor, Hooks::LockCursor );
	g_Hooks.pPanelHook->Hook( vtable_indexes::paint, Hooks::PaintTraverse );
	g_Hooks.pModelHook->Hook( vtable_indexes::dme, Hooks::DrawModelExecute );
	g_Hooks.pRenderViewHook->Hook( vtable_indexes::sceneEnd, Hooks::SceneEnd );

	g_Event.Init( );

	g::CourierNew = g_pSurface->FontCreate( );
	g_pSurface->SetFontGlyphSet( g::CourierNew, "Courier New", 14, 300, 0, 0, FONTFLAG_OUTLINE );
	g::Tahoma = g_pSurface->FontCreate( );
	g_pSurface->SetFontGlyphSet( g::Tahoma, "Tahoma", 12, 700, 0, 0, FONTFLAG_DROPSHADOW );

	Warning( "cheat by: stickrpg" );

	Utils::Log( "Hooking completed!" );
}

void Hooks::Restore( )
{
	Utils::Log( "Unhooking in progress..." );
	{   // Unhook every function we hooked and restore original one
		g_Hooks.pClientHook->Unhook( vtable_indexes::frameStage );
		g_Hooks.pClientHook->Unhook( vtable_indexes::createMove );
		g_Hooks.pClientModeHook->Unhook( vtable_indexes::view );
		g_Hooks.pSurfaceHook->Unhook( vtable_indexes::lockCursor );
		g_Hooks.pPanelHook->Unhook( vtable_indexes::paint );
		g_Hooks.pModelHook->Unhook( vtable_indexes::dme );

		SetWindowLongPtr( g_Hooks.hCSGOWindow, GWLP_WNDPROC, reinterpret_cast< LONG_PTR >( g_Hooks.pOriginalWNDProc ) );

		g_pNetvars.reset( );   /* Need to reset by-hand, global pointer so doesnt go out-of-scope */
	}
	Utils::Log( "Unhooking succeded!" );
}

void Hooks::HookPlayers( )
{
	static bool Init[ 65 ];
	static bool Hooked[ 65 ];

	for ( int i = 1; i < g_pEngine->GetMaxClients( ); ++i )
	{
		C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity( i );

		if ( !pPlayerEntity
			|| !pPlayerEntity->IsAlive( )
			|| pPlayerEntity->IsDormant( ) )
		{
			if ( Hooked[ i ] )
				g_Hooks.pPlayerHook[ i ]->Unhook( vtable_indexes::extraBonePro );

			Hooked[ i ] = false;
			continue;
		}

		if ( !Init[ i ] )
		{
			g_Hooks.pPlayerHook[ i ] = std::make_unique<ShadowVTManager>( );
			Init[ i ] = true;
		}

		if ( Hooked[ i ] )
			g_Hooks.pPlayerHook[ i ]->Unhook( vtable_indexes::extraBonePro );

		if ( !Hooked[ i ] )
		{
			g_Hooks.pPlayerHook[ i ]->Setup( pPlayerEntity );
			g_Hooks.pPlayerHook[ i ]->Hook( vtable_indexes::extraBonePro, Hooks::DoExtraBonesProcessing );

			Hooked[ i ] = true;
		}
	}
}

void __stdcall Hooks::CreateMove( int sequence_number, float input_sample_frametime, bool active, bool& send_packet )
{
	static auto oCreateMove = g_Hooks.pClientHook->GetOriginal<decltype( &Hooks::CreateMoveProxy )>( vtable_indexes::createMove );
	oCreateMove( g_pClientDll, 0, sequence_number, input_sample_frametime, active );

	auto pCmd = g_pInput->GetUserCmd( sequence_number );
	auto pVerified = g_pInput->GetVerifiedCmd( sequence_number );

	if ( !pCmd || !pCmd->command_number )
		return;

	// Get globals
	g::pCmd = pCmd;
	g::pLocalEntity = g_pEntityList->GetClientEntity( g_pEngine->GetLocalPlayer( ) );
	g::bSendPacket = true;
	if ( !g::pLocalEntity )
		return;

	g::OriginalView = g::pCmd->viewangles;

	//	HookPlayers();

	g::pCmd->buttons |= IN_BULLRUSH;

	g_Misc.OnCreateMove( );
	g_Resolver.OnCreateMove( );

	engine_prediction::RunEnginePred( );
	g_AntiAim.OnCreateMove( );
	g_Aimbot.OnCreateMove( );
	g_Legitbot.OnCreateMove( );
	engine_prediction::EndEnginePred( );

	g_Misc.MovementFix( g::OriginalView );
	g_Math.Clamp( g::pCmd->viewangles );

	send_packet = g::bSendPacket;

	if ( g::bSendPacket )
		g::RealAngle = g::pCmd->viewangles;

	pVerified->m_cmd = *pCmd;
	pVerified->m_crc = pCmd->GetChecksum( );
}

__declspec( naked ) void __fastcall Hooks::CreateMoveProxy( void* _this, int, int sequence_number, float input_sample_frametime, bool active )
{
	__asm
	{
		push ebp
		mov  ebp, esp
		push ebx; not sure if we need this
		push esp
		push dword ptr[ active ]
		push dword ptr[ input_sample_frametime ]
		push dword ptr[ sequence_number ]
		call Hooks::CreateMove
		pop  ebx
		pop  ebp
		retn 0Ch
	}
}

void __fastcall Hooks::SceneEnd( void* ecx, void* edx )
{
	static auto oSceneEnd = g_Hooks.pRenderViewHook->GetOriginal<SceneEnd_t>( vtable_indexes::sceneEnd );
	oSceneEnd( ecx, edx );

	static IMaterial* Material = nullptr;
	static bool ResetMaterial = false;

	if ( !g::pLocalEntity || !g_pEngine->IsInGame( ) || !g_pEngine->IsConnected( ) || !g_Menu.Config.Chams )
	{
		Material = nullptr;
		return;
	}

	if ( Material == nullptr )
	{
		Material = g_pMaterialSys->FindMaterial( "FlatChams", "Model textures" );
		return;
	}

	if ( !g::pLocalEntity->IsAlive( ) )
	{
		if ( ResetMaterial )
		{
			Material = nullptr;
			ResetMaterial = false;
		}

		return;
	}
	else
		ResetMaterial = true;

	for ( int i = 1; i < g_pEngine->GetMaxClients( ); ++i )
	{
		C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity( i );

		if ( !pPlayerEntity
			|| !pPlayerEntity->IsAlive( )
			|| pPlayerEntity->IsDormant( )
			|| pPlayerEntity->GetTeam( ) == g::pLocalEntity->GetTeam( ) )
			continue;

		Material->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, true );
		Material->AlphaModulate( 1.f );
		Material->ColorModulate( g_Menu.Config.ChamsColor.red / 255.f, g_Menu.Config.ChamsColor.green / 255.f, g_Menu.Config.ChamsColor.blue / 255.f );
		g_pModelRender->ForcedMaterialOverride( Material );
		pPlayerEntity->DrawModel( 0x1, pPlayerEntity->GetModelInstance( ) );
		g_pModelRender->ForcedMaterialOverride( nullptr );
	}
}

void __fastcall Hooks::DoExtraBonesProcessing( void* ECX, void* EDX, void* unkn1, void* unkn2, void* unkn3, void* unkn4, CBoneBitList& unkn5, void* unkn6 )
{
	C_BaseEntity* pPlayerEntity = ( C_BaseEntity* ) ECX;

	if ( !pPlayerEntity || pPlayerEntity == nullptr )
		return;

	if ( !pPlayerEntity->IsAlive( ) || pPlayerEntity->IsDormant( ) )
		return;

	if ( !pPlayerEntity->AnimState( ) )
		return;

	auto oDoExtraBonesProcessing = g_Hooks.pPlayerHook[ pPlayerEntity->EntIndex( ) ]->GetOriginal<ExtraBoneProcess_t>( vtable_indexes::extraBonePro );

	float Backup = pPlayerEntity->AnimState( )->m_flUnknownFraction;
	pPlayerEntity->AnimState( )->m_flUnknownFraction = 0;

	oDoExtraBonesProcessing( ECX, unkn1, unkn2, unkn3, unkn4, unkn5, unkn6 );

	pPlayerEntity->AnimState( )->m_flUnknownFraction = Backup;
}

void __fastcall Hooks::DrawModelExecute( void* ecx, void* edx, IMatRenderContext* context, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix )
{
	static auto oDrawModelExecute = g_Hooks.pModelHook->GetOriginal<DrawModelExecute_t>( vtable_indexes::dme );

	const char* ModelName = g_pModelInfo->GetModelName( ( model_t* ) info.pModel );

	C_BaseEntity* pPlayerEntity = g_pEntityList->GetClientEntity( info.index );

	if ( pPlayerEntity && pPlayerEntity->IsAlive( ) && !pPlayerEntity->IsDormant( ) && g_Aimbot.Matrix[ info.index ] && strstr( ModelName, "models/player" ) )
		oDrawModelExecute( ecx, context, state, info, g_Aimbot.Matrix[ info.index ] );
	else
		oDrawModelExecute( ecx, context, state, info, matrix );
}

void __stdcall Hooks::FrameStageNotify( ClientFrameStage_t curStage )
{
	static auto oFrameStage = g_Hooks.pClientHook->GetOriginal<FrameStageNotify_t>( vtable_indexes::frameStage );

	g_Misc.ThirdPerson( curStage );

	if ( g::pLocalEntity && g_pEngine->IsInGame( ) && curStage == FRAME_RENDER_START )
	{
		auto& poseParams = g::pLocalEntity->GetPoseParams( );
		auto state = g::pLocalEntity->AnimState( );
		const auto layers = g::pLocalEntity->AnimOverlays( );
		if ( !state || !layers )
			return;

		static auto networkedYaw = state->m_flGoalFeetYaw;
		static std::array<float, PoseParam_Count> networkedPoseParams;
		static std::array<AnimationLayer, AnimLayer_Count> networkedLayers;

		static int previousTick = 0;
		if ( previousTick != g_pGlobalVars->tickcount )
		{
			previousTick = g_pGlobalVars->tickcount;

			std::copy( layers, layers + g::pLocalEntity->GetAnimLayerCount( ), networkedLayers.begin( ) );

			g::pLocalEntity->ClientAnimations( true );
			state->Update( g::pCmd->viewangles );
			g::pLocalEntity->UpdateClientAnimation( );

			g::pLocalEntity->ClientAnimations( false );

			if ( g::bSendPacket )
			{
				std::copy( poseParams.begin( ), poseParams.end( ), networkedPoseParams.begin( ) );
				networkedYaw = state->m_flGoalFeetYaw;
			}
		}

		g::pLocalEntity->SetAbsAngles( { 0.0f, networkedYaw, 0.0f } );
		std::copy( networkedLayers.begin( ), networkedLayers.end( ), layers );
		std::copy( networkedPoseParams.begin( ), networkedPoseParams.end( ), poseParams.begin( ) );

		g::pLocalEntity->InvalidateBoneCache( );
		g::pLocalEntity->SetupBones( nullptr, 128, 0x0007FF00, g_pGlobalVars->curtime );
	}

	g_Resolver.FrameStage( curStage );

	oFrameStage( curStage );
}

void __fastcall Hooks::PaintTraverse( PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce )
{
	static auto oPaintTraverse = g_Hooks.pPanelHook->GetOriginal<PaintTraverse_t>( vtable_indexes::paint );
	static unsigned int panelID, panelHudID;

	if ( !panelHudID )
		if ( !strcmp( "HudZoom", g_pPanel->GetName( vguiPanel ) ) )
		{
			panelHudID = vguiPanel;
		}

	if ( panelHudID == vguiPanel && g::pLocalEntity && g::pLocalEntity->IsAlive( ) && g_Menu.Config.NoScope )
	{
		if ( g::pLocalEntity->IsScoped( ) )
			return;
	}

	oPaintTraverse( pPanels, vguiPanel, forceRepaint, allowForce );

	if ( !panelID )
		if ( !strcmp( "MatSystemTopPanel", g_pPanel->GetName( vguiPanel ) ) )
		{
			panelID = vguiPanel;
			g_Hooks.bInitializedDrawManager = true;
		}

	if ( panelID == vguiPanel )
	{
		g_ESP.Render( );
		g_Misc.Crosshair( );
		g_Menu.Render( );
	}
};

void Event::FireGameEvent( IGameEvent* event )
{
	if ( !event )
		return;

	if ( !g::pLocalEntity )
		return;

	auto attacker = g_pEntityList->GetClientEntity( g_pEngine->GetPlayerForUserID( event->GetInt( "attacker" ) ) );
	if ( !attacker )
		return;

	if ( attacker != g::pLocalEntity )
		return;

	int index = g_pEngine->GetPlayerForUserID( event->GetInt( "userid" ) );

	PlayerInfo_t pInfo;
	g_pEngine->GetPlayerInfo( index, &pInfo );

	g::Hit[ index ] = true;

	if ( !g_Menu.Config.Ak47meme )
		g_pEngine->ExecuteClientCmd( "play physics\\metal\\paintcan_impact_hard3.wav" );
};

void __fastcall Hooks::OverrideView( void* ecx, void* edx, CViewSetup* pSetup )
{
	static auto oOverrideView = g_Hooks.pClientModeHook->GetOriginal<OverrideView_t>( vtable_indexes::view );

	if ( g_pEngine->IsConnected( ) && g_pEngine->IsInGame( ) )
	{
		if ( !g::pLocalEntity )
			return;

		if ( !g::pLocalEntity->IsAlive( ) )
			return;

		if ( g_Menu.Config.NoRecoil )
		{
			Vector viewPunch = g::pLocalEntity->GetViewPunchAngle( );
			Vector aimPunch = g::pLocalEntity->GetAimPunchAngle( );

			pSetup->angles[ 0 ] -= ( viewPunch[ 0 ] + ( aimPunch[ 0 ] * 2 * 0.4499999f ) );
			pSetup->angles[ 1 ] -= ( viewPunch[ 1 ] + ( aimPunch[ 1 ] * 2 * 0.4499999f ) );
			pSetup->angles[ 2 ] -= ( viewPunch[ 2 ] + ( aimPunch[ 2 ] * 2 * 0.4499999f ) );
		}

		if ( g_Menu.Config.Fov != 0 && !g::pLocalEntity->IsScoped( ) )
			pSetup->fov = g_Menu.Config.Fov;

		if ( g_Menu.Config.NoZoom && g::pLocalEntity->IsScoped( ) )
			pSetup->fov = ( g_Menu.Config.Fov == 0 ) ? 90 : g_Menu.Config.Fov;
	}

	oOverrideView( ecx, edx, pSetup );
}

void __fastcall Hooks::LockCursor( ISurface* thisptr, void* edx )
{
	static auto oLockCursor = g_Hooks.pSurfaceHook->GetOriginal<LockCursor_t>( vtable_indexes::lockCursor );

	if ( !g_Menu.menuOpened )
		return oLockCursor( thisptr, edx );

	g_pSurface->UnLockCursor( );
}

LRESULT Hooks::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// for now as a lambda, to be transfered somewhere
	// Thanks uc/WasserEsser for pointing out my mistake!
	// Working when you HOLD th button, not when you press it.
	const auto getButtonHeld = [ uMsg, wParam ] ( bool& bButton, int vKey )
		{
			if ( wParam != vKey ) return;

			if ( uMsg == WM_KEYDOWN )
				bButton = true;
			else if ( uMsg == WM_KEYUP )
				bButton = false;
		};

	const auto getButtonToggle = [ uMsg, wParam ] ( bool& bButton, int vKey )
		{
			if ( wParam != vKey ) return;

			if ( uMsg == WM_KEYUP )
				bButton = !bButton;
		};

	if ( g_Hooks.bInitializedDrawManager )
	{
		// our wndproc capture fn
		if ( g_Menu.menuOpened )
		{
			return true;
		}
	}

	return CallWindowProcA( g_Hooks.pOriginalWNDProc, hWnd, uMsg, wParam, lParam );
}
