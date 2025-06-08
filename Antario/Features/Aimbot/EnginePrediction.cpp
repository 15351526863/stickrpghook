#include "EnginePrediction.h"
#include "..\..\SDK\CInput.h"
#include "..\..\SDK\CEntity.h"
#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CPrediction.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include "..\..\Menu\Menu.h"

float flOldCurtime;
float flOldFrametime;
int *m_nPredictionRandomSeed;
int *m_pSetPredictionPlayer;
static bool bInit = false;

void engine_prediction::RunEnginePred( )
{
	if ( !g::pLocalEntity->IsAlive( ) || !g_pMoveHelper )
		return;

	if ( !m_nPredictionRandomSeed || !m_pSetPredictionPlayer ) {
		m_nPredictionRandomSeed = *reinterpret_cast< int** >( Utils::FindSignature( "client.dll", "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04" ) + 2 );
		m_pSetPredictionPlayer = *reinterpret_cast< int** >( Utils::FindSignature( "client.dll", "89 35 ? ? ? ? F3 0F 10 48 20" ) + 2 );
	}

	CMoveData data;
	memset( &data, 0, sizeof( CMoveData ) );
	g::pLocalEntity->SetCurrentCommand( g::pCmd );
	g_pMoveHelper->SetHost( g::pLocalEntity );

	*m_nPredictionRandomSeed = g::pCmd->random_seed;
	*m_pSetPredictionPlayer = uintptr_t( g::pLocalEntity );

	flOldCurtime = g_pGlobalVars->curtime;
	flOldFrametime = g_pGlobalVars->frametime;

	g::uRandomSeed = *m_nPredictionRandomSeed;
	g_pGlobalVars->curtime = g::pLocalEntity->GetTickBase( ) * g_pGlobalVars->intervalPerTick;
	g_pGlobalVars->frametime = g_pGlobalVars->intervalPerTick;

	g_pMovement->StartTrackPredictionErrors( g::pLocalEntity );

	g_pPrediction->SetupMove( g::pLocalEntity, g::pCmd, g_pMoveHelper, &data );
	g_pMovement->ProcessMovement( g::pLocalEntity, &data );
	g_pPrediction->FinishMove( g::pLocalEntity, g::pCmd, &data );

	if ( g::pLocalEntity->GetActiveWeapon( ) )
		g::pLocalEntity->GetActiveWeapon( )->AccuracyPenalty( );
}

void engine_prediction::EndEnginePred( )
{
	if ( !g::pLocalEntity->IsAlive( ) || !g_pMoveHelper )
		return;

	g_pMovement->FinishTrackPredictionErrors( g::pLocalEntity );
	g_pMoveHelper->SetHost( nullptr );

	if ( m_nPredictionRandomSeed )
		*m_nPredictionRandomSeed = -1;

	if ( m_pSetPredictionPlayer )
		*m_pSetPredictionPlayer = 0;

	g_pGlobalVars->curtime = flOldCurtime;
	g_pGlobalVars->frametime = flOldFrametime;

	g::pLocalEntity->SetCurrentCommand( NULL );

	// fucking replay crash
	g_pMovement->Reset( );
}