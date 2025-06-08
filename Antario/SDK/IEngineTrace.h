#pragma once

#include "bspflags.h"

#define CHAR_TEX_ANTLION		'A'
#define CHAR_TEX_BLOODYFLESH	'B'
#define	CHAR_TEX_CONCRETE		'C'
#define CHAR_TEX_DIRT			'D'
#define CHAR_TEX_EGGSHELL		'E' ///< the egg sacs in the tunnels in ep2.
#define CHAR_TEX_FLESH			'F'
#define CHAR_TEX_GRATE			'G'
#define CHAR_TEX_ALIENFLESH		'H'
#define CHAR_TEX_CLIP			'I'
//#define CHAR_TEX_UNUSED		'J'
#define CHAR_TEX_SNOW			'K'
#define CHAR_TEX_PLASTIC		'L'
#define CHAR_TEX_METAL			'M'
#define CHAR_TEX_SAND			'N'
#define CHAR_TEX_FOLIAGE		'O'
#define CHAR_TEX_COMPUTER		'P'
//#define CHAR_TEX_UNUSED		'Q'
#define CHAR_TEX_REFLECTIVE		'R'
#define CHAR_TEX_SLOSH			'S'
#define CHAR_TEX_TILE			'T'
#define CHAR_TEX_CARDBOARD		'U'
#define CHAR_TEX_VENT			'V'
#define CHAR_TEX_WOOD			'W'
//#define CHAR_TEX_UNUSED		'X' ///< do not use - "fake" materials use this (ladders, wading, clips, etc)
#define CHAR_TEX_GLASS			'Y'
#define CHAR_TEX_WARPSHIELD		'Z' ///< wierd-looking jello effect for advisor shield.

#define CHAR_TEX_STEAM_PIPE		11

#define   DISPSURF_FLAG_SURFACE           (1<<0)
#define   DISPSURF_FLAG_WALKABLE          (1<<1)
#define   DISPSURF_FLAG_BUILDABLE         (1<<2)
#define   DISPSURF_FLAG_SURFPROP1         (1<<3)
#define   DISPSURF_FLAG_SURFPROP2         (1<<4)

#define   CONTENTS_EMPTY                0

#define   CONTENTS_SOLID                0x1       
#define   CONTENTS_WINDOW               0x2
#define   CONTENTS_AUX                  0x4
#define   CONTENTS_GRATE                0x8
#define   CONTENTS_SLIME                0x10
#define   CONTENTS_WATER                0x20
#define   CONTENTS_BLOCKLOS             0x40 
#define   CONTENTS_OPAQUE               0x80 
#define   LAST_VISIBLE_CONTENTS         CONTENTS_OPAQUE

#define   ALL_VISIBLE_CONTENTS            (LAST_VISIBLE_CONTENTS | (LAST_VISIBLE_CONTENTS-1))

#define   CONTENTS_TESTFOGVOLUME        0x100
#define   CONTENTS_UNUSED               0x200     
#define   CONTENTS_BLOCKLIGHT           0x400
#define   CONTENTS_TEAM1                0x800 
#define   CONTENTS_TEAM2                0x1000 
#define   CONTENTS_IGNORE_NODRAW_OPAQUE 0x2000
#define   CONTENTS_MOVEABLE             0x4000
#define   CONTENTS_AREAPORTAL           0x8000
#define   CONTENTS_PLAYERCLIP           0x10000
#define   CONTENTS_MONSTERCLIP          0x20000
#define   CONTENTS_CURRENT_0            0x40000
#define   CONTENTS_CURRENT_90           0x80000
#define   CONTENTS_CURRENT_180          0x100000
#define   CONTENTS_CURRENT_270          0x200000
#define   CONTENTS_CURRENT_UP           0x400000
#define   CONTENTS_CURRENT_DOWN         0x800000

#define   CONTENTS_ORIGIN               0x1000000 

#define   CONTENTS_MONSTER              0x2000000 
#define   CONTENTS_DEBRIS               0x4000000
#define   CONTENTS_DETAIL               0x8000000 
#define   CONTENTS_TRANSLUCENT          0x10000000
#define   CONTENTS_LADDER               0x20000000
#define   CONTENTS_HITBOX               0x40000000

#define   SURF_LIGHT                    0x0001 
#define   SURF_SKY2D                    0x0002 
#define   SURF_SKY                      0x0004 
#define   SURF_WARP                     0x0008 
#define   SURF_TRANS                    0x0010
#define   SURF_NOPORTAL                 0x0020 
#define   SURF_TRIGGER                  0x0040 
#define   SURF_NODRAW                   0x0080 

#define   SURF_HINT                     0x0100 

#define   SURF_SKIP                     0x0200   
#define   SURF_NOLIGHT                  0x0400   
#define   SURF_BUMPLIGHT                0x0800   
#define   SURF_NOSHADOWS                0x1000   
#define   SURF_NODECALS                 0x2000   
#define   SURF_NOPAINT                  SURF_NODECALS
#define   SURF_NOCHOP                   0x4000   
#define   SURF_HITBOX                   0x8000   

// -----------------------------------------------------
// spatial content masks - used for spatial queries (traceline,etc.)
// -----------------------------------------------------
#define   MASK_ALL                      (0xFFFFFFFF)
#define   MASK_SOLID                    (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define   MASK_PLAYERSOLID              (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define   MASK_NPCSOLID                 (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define   MASK_NPCFLUID                 (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define   MASK_WATER                    (CONTENTS_WATER|CONTENTS_MOVEABLE|CONTENTS_SLIME)
#define   MASK_OPAQUE                   (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_OPAQUE)
#define   MASK_OPAQUE_AND_NPCS          (MASK_OPAQUE|CONTENTS_MONSTER)
#define   MASK_BLOCKLOS                 (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_BLOCKLOS)
#define   MASK_BLOCKLOS_AND_NPCS        (MASK_BLOCKLOS|CONTENTS_MONSTER)
#define   MASK_VISIBLE                  (MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE)
#define   MASK_VISIBLE_AND_NPCS         (MASK_OPAQUE_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE)
#define   MASK_SHOT                     (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX)
#define   MASK_SHOT_BRUSHONLY           (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS)
#define   MASK_SHOT_HULL                (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE)
#define   MASK_SHOT_PORTAL              (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define   MASK_SOLID_BRUSHONLY          (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_GRATE)
#define   MASK_PLAYERSOLID_BRUSHONLY    (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_PLAYERCLIP|CONTENTS_GRATE)
#define   MASK_NPCSOLID_BRUSHONLY       (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE)
#define   MASK_NPCWORLDSTATIC           (CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE)
#define   MASK_NPCWORLDSTATIC_FLUID     (CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP)
#define   MASK_SPLITAREAPORTAL          (CONTENTS_WATER|CONTENTS_SLIME)
#define   MASK_CURRENT                  (CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)
#define   MASK_DEADSOLID                (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_GRATE)

struct C_Ray
{
	VectorAligned m_Start; // starting point, centered within the extents
	VectorAligned m_Delta; // direction + length of the ray
	VectorAligned m_StartOffset; // Add this to m_Start to get the actual ray start
	VectorAligned m_Extents; // Describes an axis aligned box extruded along a ray
	const matrix3x4_t* m_pWorldAxisTransform;
	//const matrix3x4_t *m_pWorldAxisTransform;
	bool m_IsRay; // are the extents zero?
	bool m_IsSwept; // is delta != 0?


	C_Ray(Vector _start, Vector _end)
	{
		Init(_start, _end);
	}

	C_Ray(Vector _start, Vector _end, Vector _mins, Vector _maxs)
	{
		Init(_start, _end, _mins, _maxs);
	}

	void Init(const Vector& vecStart, const Vector& vecEnd)
	{
		m_Delta = vecEnd - vecStart;

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		m_Extents.x = m_Extents.y = m_Extents.z = 0.0f;
		m_IsRay = true;

		m_StartOffset.x = m_StartOffset.y = m_StartOffset.z = 0.0f;

		m_Start = vecStart;
	}

	void Init(Vector& vecStart, Vector& vecEnd, Vector min, Vector max)
	{
		m_Delta = vecEnd - vecStart;

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		m_Extents.x = (max.x - min.x);
		m_Extents.y = (max.y - min.y);
		m_Extents.z = (max.z - min.z);
		m_IsRay = false;

		m_StartOffset.x = m_StartOffset.y = m_StartOffset.z = 0.0f;

		m_Start = vecStart + ((max + min) * 0.5f);
	}

private:
};

struct csurface_t
{
	const char* name;
	short surfaceProps;
	unsigned short flags;
};

struct cplane_t
{
	Vector normal;
	float dist;
	byte type;
	byte signbits;
	byte pad[2];
};

struct C_Trace
{
	bool DidHitWorld() const
	{
		return m_pEnt && m_pEnt->EntIndex() == 0;
	}

	bool DidHitNonWorldEntity() const
	{
		return m_pEnt != NULL && !DidHitWorld();
	}

	bool DidHit() const
	{
		return flFraction < 1.f;
	}

	Vector start;
	Vector end;
	cplane_t plane;
	float flFraction;
	int contents;
	WORD dispFlags;
	bool allsolid;
	bool startSolid;
	float fractionLeftSolid;
	csurface_t surface;
	int hitGroup;
	short physicsBone;
	C_BaseEntity* m_pEnt;
	int hitbox;
};

enum TraceType_t
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY, // NOTE: This does *not* test static props!!!
	TRACE_ENTITIES_ONLY, // NOTE: This version will *not* test static props
	TRACE_EVERYTHING_FILTER_PROPS, // NOTE: This version will pass the IHandleEntity for props through the filter, unlike all other filters
};

class ITraceFilter
{
public:
	virtual bool ShouldHitEntity(void* pEntity, int contentsMask) = 0;
	virtual TraceType_t GetTraceType() const = 0;
};

class C_TraceFilter : public ITraceFilter
{
public:

	explicit C_TraceFilter(C_BaseEntity* entity, TraceType_t tracetype = TRACE_EVERYTHING)
	{
		pSkip1 = entity;
	}

	bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return (pEntityHandle != pSkip1);
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* pSkip1;
};

class CTraceFilterOneEntity : public ITraceFilter
{
public:

	explicit CTraceFilterOneEntity(C_BaseEntity* entity, TraceType_t tracetype = TRACE_EVERYTHING)
	{
		pEntity = entity;
	}

	bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return (pEntityHandle == pEntity);
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* pEntity;
};

class CTraceFilterOneEntity2 : public ITraceFilter
{
public:

	explicit CTraceFilterOneEntity2(C_BaseEntity* entity, TraceType_t tracetype = TRACE_EVERYTHING)
	{
		pEntity = entity;
	}

	bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return (pEntityHandle == pEntity);
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_ENTITIES_ONLY;
	}

	void* pEntity;
};

class CTraceFilterSkipTwoEntities : public ITraceFilter
{
public:
	CTraceFilterSkipTwoEntities(void* pPassEnt1, void* pPassEnt2)
	{
		passentity1 = pPassEnt1;
		passentity2 = pPassEnt2;
	}

	virtual bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return !(pEntityHandle == passentity1 || pEntityHandle == passentity2);
	}

	virtual TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* passentity1;
	void* passentity2;
};

class CTraceEntity : public ITraceFilter
{
public:

	explicit CTraceEntity(C_BaseEntity* entity, TraceType_t tracetype = TRACE_EVERYTHING)
	{
		pSkip1 = entity;
	}

	bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return !(pEntityHandle == pSkip1);
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_ENTITIES_ONLY;
	}

	void* pSkip1;
};

class CTraceWorldOnly : public ITraceFilter
{
public:

	explicit CTraceWorldOnly(C_BaseEntity* entity, TraceType_t tracetype = TRACE_EVERYTHING)
	{
		pSkip1 = entity;
	}

	bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return false;
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* pSkip1;
};

class IEngineTrace
{
public:
	// Returns the contents mask + entity at a particular world-space position
	virtual int		GetPointContents(const Vector &vecAbsPosition, int contentsMask = mask_all, IHandleEntity** ppEntity = NULL) = 0;

	// Returns the contents mask of the world only @ the world-space position (static props are ignored)
	virtual int		GetPointContents_WorldOnly(const Vector &vecAbsPosition, int contentsMask = mask_all) = 0;

	// Get the point contents, but only test the specific entity. This works
	// on static props and brush models.
	//
	// If the entity isn't a static prop or a brush model, it returns CONTENTS_EMPTY and sets
	// bFailed to true if bFailed is non-null.
	virtual int		GetPointContents_Collideable(ICollideable *pCollide, const Vector &vecAbsPosition) = 0;

	// Traces a ray against a particular entity
	virtual void	ClipRayToEntity(const C_Ray &ray, unsigned int fMask, IHandleEntity *pEnt, C_Trace *pTrace) = 0;

	// Traces a ray against a particular entity
	virtual void	ClipRayToCollideable(const C_Ray &ray, unsigned int fMask, ICollideable *pCollide, C_Trace *pTrace) = 0;

	// A version that simply accepts a ray (can work as a traceline or tracehull)
	virtual void	TraceRay(const C_Ray &ray, unsigned int fMask, ITraceFilter *pTraceFilter, C_Trace *pTrace) = 0;
};

extern IEngineTrace* g_pTrace;