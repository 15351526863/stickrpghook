#pragma once
#include "../../Utils/GlobalVars.h"
#include "../../SDK/CGlobalVarsBase.h"
#include "../../SDK/IEngineTrace.h"

struct FireBulletData
{
        FireBulletData(const Vector &eyePos, C_BaseEntity* entity) : src(eyePos), filter(entity)
        {
                trace_length = 0.f;
                trace_length_remaining = 0.f;
                current_damage = 0.f;
                penetrate_count = 0;
                hit_count = 0;
                damage = 0.f;
                penetrated = false;
        }

        Vector          src;
        C_Trace         enter_trace;
        Vector          direction;
        C_TraceFilter   filter;
        float           trace_length;
        float           trace_length_remaining;
        float           current_damage;
        int             penetrate_count;
        C_Trace         trace;
        Vector          hit_points[5];
        int             hit_count;
        float           damage;
        bool            penetrated;
};

class Autowall
{
public:
	bool CanHitFloatingPoint(const Vector &point, const Vector &source);
	float Damage(const Vector &point);
private:
};
extern Autowall g_Autowall;
