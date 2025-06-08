#include "Autowall.h"
#include "..\..\Utils\Utils.h"
#include "..\..\SDK\IVEngineClient.h"
#include "..\..\SDK\PlayerInfo.h"
#include "..\..\SDK\ISurfaceData.h"
#include "..\..\SDK\Hitboxes.h"
#include "..\..\SDK\bspflags.h"
#include "..\..\SDK\ICvar.h"
#include "..\..\Utils\Math.h"
#include "..\..\SDK\ClientClass.h"
#include <algorithm>

// esoterik ftw

Autowall g_Autowall;

void TraceLine(Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, C_BaseEntity *ignore, C_Trace *ptr)
{
	C_TraceFilter filter(ignore);
	g_pTrace->TraceRay(C_Ray(vecAbsStart, vecAbsEnd), mask, &filter, ptr);
}

bool VectortoVectorVisible(Vector src, Vector point)
{
	C_Trace TraceInit;
	TraceLine(src, point, mask_solid, g::pLocalEntity, &TraceInit);
	C_Trace Trace;
	TraceLine(src, point, mask_solid, TraceInit.m_pEnt, &Trace);

	if (Trace.flFraction == 1.0f || TraceInit.flFraction == 1.0f)
		return true;

	return false;
}

float GetHitgroupDamageMult(int iHitGroup)
{
	switch (iHitGroup)
	{
	case HITGROUP_HEAD:
	return 4.f;
	case HITGROUP_STOMACH:
	return 1.25f;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
	return 0.75f;
	}

	return 1.0f;
}

bool HandleBulletPenetration(WeaponInfo_t *wpn_data, FireBulletData &data, bool extracheck, Vector point);

void ScaleDamage(int hitgroup, C_BaseEntity *enemy, float weapon_armor_ratio, float &current_damage)
{
	current_damage *= GetHitgroupDamageMult(hitgroup);

	if (enemy->ArmorValue() > 0.0f && hitgroup < HITGROUP_LEFTLEG)
	{
		if (hitgroup == HITGROUP_HEAD && !enemy->HasHelmet())
			return;

		float armorscaled = (weapon_armor_ratio * 0.5f) * current_damage;
		if ((current_damage - armorscaled) * 0.5f > enemy->ArmorValue())
			armorscaled = current_damage - (enemy->ArmorValue() * 2.0f);
		current_damage = armorscaled;
	}
}

void UTIL_ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, ITraceFilter* filter, C_Trace* tr)
{
	float smallestFraction = tr->flFraction;
	constexpr float maxRange = 60.0f;

	Vector delta(vecAbsEnd - vecAbsStart);
	const float delta_length = delta.Length();
	delta.NormalizeInPlace();

	C_Ray ray(vecAbsStart, vecAbsEnd);

	for (int i = 1; i <= g_pGlobalVars->maxClients; ++i) 
	{
		auto ent = g_pEntityList->GetClientEntity(i);
		if (!ent || ent->IsDormant() || !ent->IsAlive()) 
			continue;

		if (filter && !filter->ShouldHitEntity(ent, mask)) 
			continue;

		auto collideble = ent->GetCollideable();
		auto mins = collideble->OBBMins();
		auto maxs = collideble->OBBMaxs();

		auto obb_center = (maxs + mins) * 0.5f;
		auto extend = (obb_center - vecAbsStart);
		auto rangeAlong = delta.Dot(extend);

		float range;
		if (rangeAlong >= 0.0f) 
		{
			if (rangeAlong <= delta_length)
				range = Vector(obb_center - ((delta * rangeAlong) + vecAbsStart)).Length();
			else 
				range = -(obb_center - vecAbsEnd).Length();
		}
		else 
			range = -extend.Length();		

		if (range >= 0.0f && range <= maxRange) 
		{
			C_Trace playerTrace;
			g_pTrace->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ent, &playerTrace);
			if (playerTrace.flFraction < smallestFraction) 
			{
				*tr = playerTrace;
				smallestFraction = playerTrace.flFraction;
			}
		}
	}
}

bool SimulateFireBullet(C_BaseEntity *local, C_BaseCombatWeapon *weapon, FireBulletData &data)
{
	data.penetrate_count = 4;
	data.trace_length = 0.0f;
	auto *wpn_data = weapon->GetCSWpnData();
	data.current_damage = (float)wpn_data->weapon_damage;
	while ((data.penetrate_count > 0) && (data.current_damage >= 1.0f))
	{
		data.trace_length_remaining = wpn_data->weapon_range - data.trace_length;
		Vector End_Point = data.src + data.direction * data.trace_length_remaining;
		TraceLine(data.src, End_Point, 0x4600400B, local, &data.enter_trace);
		UTIL_ClipTraceToPlayers(data.src, End_Point * 40.f, 0x4600400B, &data.filter, &data.enter_trace);
		if (data.enter_trace.flFraction == 1.0f) 
			break;
		if ((data.enter_trace.hitGroup <= 7) && (data.enter_trace.hitGroup > 0) && (local->GetTeam() != data.enter_trace.m_pEnt->GetTeam()))
		{
			data.trace_length += data.enter_trace.flFraction * data.trace_length_remaining;
			data.current_damage *= pow(wpn_data->weapon_range_mod, data.trace_length * 0.002);
			ScaleDamage(data.enter_trace.hitGroup, data.enter_trace.m_pEnt, wpn_data->weapon_armor_ratio, data.current_damage);
			return true;
		}
		if (!HandleBulletPenetration(wpn_data, data, false, Vector(0,0,0))) 
			break;
	}
	return false;
}

bool TraceToExitalt(Vector& end, C_Trace* enter_trace, Vector start, Vector dir, C_Trace* exit_trace)
{
	float distance = 0.0f;
	while (distance <= 90.0f) 
	{
		distance += 4.0f;

		end = start + dir * distance;
		auto point_contents = g_pTrace->GetPointContents(end, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL);
		if (point_contents & MASK_SHOT_HULL && !(point_contents & CONTENTS_HITBOX)) continue;

		auto new_end = end - (dir * 4.0f);
		C_Ray ray(end, new_end);
		g_pTrace->TraceRay(ray, MASK_SHOT, 0, exit_trace);
		if (exit_trace->startSolid && exit_trace->surface.flags & SURF_HITBOX) 
		{
			ray.Init(end, start);
			C_TraceFilter filter(exit_trace->m_pEnt);
			g_pTrace->TraceRay(ray, 0x600400B, &filter, exit_trace);
			if ((exit_trace->flFraction < 1.0f || exit_trace->allsolid) && !exit_trace->startSolid) 
			{
				end = exit_trace->end;
				return true;
			}

			continue;
		}

		if (!(exit_trace->flFraction < 1.0 || exit_trace->allsolid || exit_trace->startSolid) || exit_trace->startSolid) 
		{
			if (exit_trace->m_pEnt) 
			{
				if (enter_trace->m_pEnt && enter_trace->m_pEnt == g_pEntityList->GetClientEntity(0)) // CWorld
					return true;
			}

			continue;
		}

		if (exit_trace->surface.flags >> 7 & 1 && !(enter_trace->surface.flags >> 7 & 1)) 
			continue;

		if (exit_trace->plane.normal.Dot(dir) <= 1.0f) 
		{
			auto fraction = exit_trace->flFraction * 4.0f;
			end = end - (dir * fraction);

			return true;
		}
	}

	return false;
}

bool HandleBulletPenetration(WeaponInfo_t *wpn_data, FireBulletData &data, bool extracheck, Vector point)
{
	surfacedata_t *enter_surface_data = g_pSurfaceData->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;
	float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;
	data.trace_length += data.enter_trace.flFraction * data.trace_length_remaining;
	data.current_damage *= pow(wpn_data->weapon_range_mod, (data.trace_length * 0.002));
	if ((data.trace_length > 3000.f) || (enter_surf_penetration_mod < 0.1f))
		data.penetrate_count = 0;
	if (data.penetrate_count <= 0)
		return false;
	Vector dummy;
	C_Trace trace_exit;
	if (!TraceToExitalt(dummy, &data.enter_trace, data.enter_trace.end, data.direction, &trace_exit))
		return false;
	surfacedata_t *exit_surface_data = g_pSurfaceData->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;
	float exit_surf_penetration_mod = exit_surface_data->game.flPenetrationModifier;
	float final_damage_modifier = 0.16f;
	float combined_penetration_modifier = 0.0f;
	if (((data.enter_trace.contents & contents_grate) != 0) || (enter_material == 89) || (enter_material == 71)) 
	{
		combined_penetration_modifier = 3.0f;
		final_damage_modifier = 0.05f; 
	}
	else 
		combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;
	if (enter_material == exit_material)
	{
		if (exit_material == 87 || exit_material == 85)combined_penetration_modifier = 3.0f;
		else if (exit_material == 76)combined_penetration_modifier = 2.0f;
	}
	float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
	float v35 = (data.current_damage * final_damage_modifier) + v34 * 3.0f * fmaxf(0.0f, (3.0f / wpn_data->weapon_penetration) * 1.25f);
	float thickness = VectorLength(trace_exit.end - data.enter_trace.end);
	if (extracheck)
		if (!VectortoVectorVisible(trace_exit.end, point))
			return false;
	thickness *= thickness;
	thickness *= v34;
	thickness /= 24.0f;
	float lost_damage = fmaxf(0.0f, v35 + thickness);
	if (lost_damage > data.current_damage)
		return false;
	if (lost_damage >= 0.0f)
		data.current_damage -= lost_damage;
	if (data.current_damage < 1.0f)
		return false;
	data.src = trace_exit.end;
	data.penetrate_count--;

	return true;
}

float Autowall::Damage(const Vector &point)
{
	auto data = FireBulletData(g::pLocalEntity->GetEyePosition(), g::pLocalEntity);

	Vector angles;
	angles = g_Math.CalcAngle(data.src, point);
	g_Math.AngleVectors(angles, &data.direction);
	VectorNormalize(data.direction);

	if (SimulateFireBullet(g::pLocalEntity, g::pLocalEntity->GetActiveWeapon(), data))
		return data.current_damage;

	return 0.f;
}

bool Autowall::CanHitFloatingPoint(const Vector &point, const Vector &source) // ez
{
	if (!g::pLocalEntity)
		return false;

	FireBulletData data = FireBulletData(source, g::pLocalEntity);

	Vector angles = g_Math.CalcAngle(data.src, point);
	g_Math.AngleVectors(angles, &data.direction);
	VectorNormalize(data.direction);

	C_BaseCombatWeapon *pWeapon = (C_BaseCombatWeapon*)g::pLocalEntity->GetActiveWeapon();

	if (!pWeapon)
		return false;

	data.penetrate_count = 1;
	data.trace_length = 0.0f;

	WeaponInfo_t *weaponData = pWeapon->GetCSWpnData();

	if (!weaponData)
		return false;

	data.current_damage = (float)weaponData->weapon_damage;
	data.trace_length_remaining = weaponData->weapon_range - data.trace_length;
	Vector end = data.src + (data.direction * data.trace_length_remaining);
	TraceLine(data.src, end, mask_shot | contents_hitbox, g::pLocalEntity, &data.enter_trace);

	if (VectortoVectorVisible(data.src, point) || HandleBulletPenetration(weaponData, data, true, point))
		return true;

	return false;
}