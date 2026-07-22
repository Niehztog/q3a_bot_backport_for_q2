/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../game_q3/q_shared.h"
#include "../bspc/l_log.h"
#include "../bspc/l_qfiles.h"
#include "../botlib/l_memory.h"
#include "../botlib/l_script.h"
#include "../botlib/l_precomp.h"
#include "../botlib/l_struct.h"
#include "../botlib/aasfile.h"
#include "../game_q3/botlib.h"
#include "../game_q3/be_aas.h"
#include "../botlib/be_aas_def.h"
#include "../botlib/be_aas_bsp.h"
#include "../qcommon_q3/cm_public.h"

//#define BSPC

extern botlib_import_t botimport;
extern	qboolean capsule_collision;

botlib_import_t botimport;
clipHandle_t worldmodel;

void Error (char *error, ...);

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_Error(char *fmt, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	Error(text);
} //end of the function AAS_Error
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int Sys_MilliSeconds(void)
{
	return clock() * 1000 / CLOCKS_PER_SEC;
} //end of the function Sys_MilliSeconds
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DebugLine(vec3_t start, vec3_t end, int color)
{
} //end of the function AAS_DebugLine
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ClearShownDebugLines(void)
{
} //end of the function AAS_ClearShownDebugLines
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *BotImport_BSPEntityData(void)
{
	return CM_EntityString();
} //end of the function AAS_GetEntityData
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotImport_Trace(bsp_trace_t *bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int passent, int contentmask)
{
	trace_t result;

	CM_BoxTrace(&result, start, end, mins, maxs, worldmodel, contentmask, capsule_collision);

	bsptrace->allsolid = result.allsolid;
	bsptrace->contents = result.contents;
	VectorCopy(result.endpos, bsptrace->endpos);
	bsptrace->ent = result.entityNum;
	bsptrace->fraction = result.fraction;
	bsptrace->exp_dist = 0;
	bsptrace->plane.dist = result.plane.dist;
	VectorCopy(result.plane.normal, bsptrace->plane.normal);
	bsptrace->plane.signbits = result.plane.signbits;
	bsptrace->plane.type = result.plane.type;
	bsptrace->sidenum = 0;
	bsptrace->startsolid = result.startsolid;
	bsptrace->surface.flags = result.surfaceFlags;
} //end of the function BotImport_Trace
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int BotImport_PointContents(vec3_t p)
{
	return CM_PointContents(p, worldmodel);
} //end of the function BotImport_PointContents
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void *BotImport_GetMemory(int size)
{
	return GetMemory(size);
} //end of the function BotImport_GetMemory
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void BotImport_Print(int type, char *fmt, ...)
{
	va_list argptr;
	char buf[1024];

	va_start(argptr, fmt);
	vsprintf(buf, fmt, argptr);
	printf(buf);
	if (buf[0] != '\r') Log_Write(buf);
	va_end(argptr);
} //end of the function BotImport_Print
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void BotImport_BSPModelMinsMaxsOrigin(int modelnum, vec3_t angles, vec3_t outmins, vec3_t outmaxs, vec3_t origin)
{
	clipHandle_t h;
	vec3_t mins, maxs;
	float max;
	int	i;

	h = CM_InlineModel(modelnum);
	CM_ModelBounds(h, mins, maxs);
	//if the model is rotated
	if ((angles[0] || angles[1] || angles[2]))
	{	// expand for rotation

		max = RadiusFromBounds(mins, maxs);
		for (i = 0; i < 3; i++)
		{
			mins[i] = (mins[i] + maxs[i]) * 0.5 - max;
			maxs[i] = (mins[i] + maxs[i]) * 0.5 + max;
		} //end for
	} //end if
	if (outmins) VectorCopy(mins, outmins);
	if (outmaxs) VectorCopy(maxs, outmaxs);
	if (origin) VectorClear(origin);
} //end of the function BotImport_BSPModelMinsMaxsOrigin
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void Com_DPrintf(char *fmt, ...)
{
	va_list argptr;
	char buf[1024];

	va_start(argptr, fmt);
	vsprintf(buf, fmt, argptr);
	printf(buf);
	if (buf[0] != '\r') Log_Write(buf);
	va_end(argptr);
} //end of the function Com_DPrintf
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
int COM_Compress( char *data_p ) {
	return strlen(data_p);
}
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void Com_Memset (void* dest, const int val, const size_t count) {
	memset(dest, val, count);
}
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void Com_Memcpy (void* dest, const void* src, const size_t count) {
	memcpy(dest, src, count);
}
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_InitBotImport(void)
{
	botimport.BSPEntityData = BotImport_BSPEntityData;
	botimport.GetMemory = BotImport_GetMemory;
	botimport.FreeMemory = FreeMemory;
	botimport.Trace = BotImport_Trace;
	botimport.PointContents = BotImport_PointContents;
	botimport.Print = BotImport_Print;
	botimport.BSPModelMinsMaxsOrigin = BotImport_BSPModelMinsMaxsOrigin;
} //end of the function AAS_InitBotImport
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_CalcReachAndClusters(struct quakefile_s *qf)
{
	float time;

	Log_Print("loading collision map...\n");
	//
	if (!qf->pakfile[0]) strcpy(qf->pakfile, qf->filename);
	//load the map
	CM_LoadMap((char *) qf, false, &aasworld.bspchecksum);
	//get a handle to the world model
	worldmodel = CM_InlineModel(0);		// 0 = world, 1 + are bmodels
	//initialize bot import structure
	AAS_InitBotImport();
	//load the BSP entity string
	AAS_LoadBSPFile();
	//init physics settings
	AAS_InitSettings();
	//initialize AAS link heap
	AAS_InitAASLinkHeap();
	//initialize the AAS linked entities for the new map
	AAS_InitAASLinkedEntities();
	//reset all reachabilities and clusters
	aasworld.reachabilitysize = 0;
	aasworld.numclusters = 0;
	//set all view portals as cluster portals in case we re-calculate the reachabilities and clusters (with -reach)
	AAS_SetViewPortalsAsClusterPortals();
	//calculate reachabilities
	AAS_InitReachability();
	time = 0;
	while(AAS_ContinueInitReachability(time)) time++;
	//calculate clusters
	AAS_InitClustering();
} //end of the function AAS_CalcReachAndClusters

//===========================================================================
// Q2 version: populate cm.cmodels from Q2 dmodels[] (already loaded by
// Q2_LoadBSPFile) so that CM_InlineModel/CM_ModelBounds work for
// AAS_Reachability_Elevator.  Skips CM_LoadMap which expects Q3 BSP format.
//===========================================================================
/* Bridge function in l_bsp_q2.c — passes the Q2 BSP globals (with
 * their native Q2 types) to Q2_CM_LoadFromQ2BSP (in cm_load.c) as
 * void* parameters to avoid Q2/Q3 type name conflicts. */
void Q2_CM_LoadCollisionFromBSPGlobals(void);

//===========================================================================
// Q2 version: func_plat (and Rogue's func_plat2) is no AAS geometry (as in
// id's bspc), so a plat's shaft is a hole down to the floor the plat sinks
// into, and the upper floor
// gets walk-off-ledge and jump reachabilities into it.  They land on the plat
// or on whoever waits on it; stacked players block the rising plat, which
// reverses and crushes the one at the bottom (q2ctf5).  Gladiator's AAS
// priced these drops 3000, as falls that hurt.
//===========================================================================
static void Q2_AAS_PlatShaftDrops(void)
{
	int ent, modelnum, i, n;
	char classname[MAX_EPAIRKEY], model[MAX_EPAIRKEY];
	vec3_t mins, maxs, origin, angles = {0, 0, 0};
	aas_reachability_t *reach;

	n = 0;
	for (ent = AAS_NextBSPEntity(0); ent; ent = AAS_NextBSPEntity(ent))
	{
		if (!AAS_ValueForBSPEpairKey(ent, "classname", classname, MAX_EPAIRKEY)) continue;
		if (strcmp(classname, "func_plat") && strcmp(classname, "func_plat2")) continue;
		if (!AAS_ValueForBSPEpairKey(ent, "model", model, MAX_EPAIRKEY)) continue;
		modelnum = atoi(model+1);
		if (modelnum <= 0) continue;
		//mins and maxs of the plat in its top position
		AAS_BSPModelMinsMaxsOrigin(modelnum, angles, mins, maxs, origin);
		for (i = 1; i < aasworld.reachabilitysize; i++)
		{
			reach = &aasworld.reachability[i];
			if ((reach->traveltype & TRAVELTYPE_MASK) != TRAVEL_WALKOFFLEDGE &&
				(reach->traveltype & TRAVELTYPE_MASK) != TRAVEL_JUMP) continue;
			//from the plat's top level or higher down into the shaft
			if (reach->start[2] < maxs[2] || reach->end[2] >= maxs[2]) continue;
			if (reach->end[0] < mins[0] - 16 || reach->end[0] > maxs[0] + 16) continue;
			if (reach->end[1] < mins[1] - 16 || reach->end[1] > maxs[1] + 16) continue;
			reach->traveltime += 3000;
			n++;
		} //end for
	} //end for
	Log_Print("%6d reachabilities into plat shafts\n", n);
} //end of the function Q2_AAS_PlatShaftDrops

//===========================================================================
// Q2 version: a func_rotating is no AAS geometry either, so the space it
// sweeps is open floor to the AAS.  q2dm5's crusher (dmg 20000) turns over
// the floor of its room, and the gallery above has walk-off-ledge
// reachabilities down into it: the bots land on the arm or in its path.
// Priced like the plat shaft drops.
//===========================================================================
static int Q2_InRotatingSweep(vec3_t point, vec3_t origin, int axis, float radius, float lo, float hi)
{
	int i;
	float dist, bmin, bmax;
	vec3_t d;

	VectorSubtract(point, origin, d);
	d[axis] = 0;
	dist = 0;
	for (i = 0; i < 3; i++) dist += d[i] * d[i];
	if (dist >= (radius + 16) * (radius + 16)) return false;
	//the player's bounding box along the axis
	bmin = point[axis] + (axis == 2 ? -24 : -16);
	bmax = point[axis] + (axis == 2 ? 32 : 16);
	return bmax > lo && bmin < hi;
} //end of the function Q2_InRotatingSweep

static void Q2_AAS_RotatingDrops(void)
{
	int ent, modelnum, spawnflags, axis, i, j, n;
	char classname[MAX_EPAIRKEY], model[MAX_EPAIRKEY];
	vec3_t mins, maxs, origin, corner, angles = {0, 0, 0};
	float radius, r;
	aas_reachability_t *reach;

	n = 0;
	for (ent = AAS_NextBSPEntity(0); ent; ent = AAS_NextBSPEntity(ent))
	{
		if (!AAS_ValueForBSPEpairKey(ent, "classname", classname, MAX_EPAIRKEY)) continue;
		if (strcmp(classname, "func_rotating")) continue;
		if (!AAS_ValueForBSPEpairKey(ent, "model", model, MAX_EPAIRKEY)) continue;
		modelnum = atoi(model+1);
		if (modelnum <= 0) continue;
		//the model is built around its rotation point, the "origin" key
		AAS_BSPModelMinsMaxsOrigin(modelnum, angles, mins, maxs, NULL);
		VectorClear(origin);
		AAS_VectorForBSPEpairKey(ent, "origin", origin);
		spawnflags = 0;
		AAS_IntForBSPEpairKey(ent, "spawnflags", &spawnflags);
		//the axis as Q2's SP_func_rotating picks it
		if (spawnflags & 4) axis = 0;
		else if (spawnflags & 8) axis = 1;
		else axis = 2;
		radius = 0;
		for (j = 0; j < 8; j++)
		{
			corner[0] = (j & 1) ? maxs[0] : mins[0];
			corner[1] = (j & 2) ? maxs[1] : mins[1];
			corner[2] = (j & 4) ? maxs[2] : mins[2];
			corner[axis] = 0;
			r = VectorLength(corner);
			if (r > radius) radius = r;
		} //end for
		for (i = 1; i < aasworld.reachabilitysize; i++)
		{
			reach = &aasworld.reachability[i];
			if ((reach->traveltype & TRAVELTYPE_MASK) != TRAVEL_WALKOFFLEDGE &&
				(reach->traveltype & TRAVELTYPE_MASK) != TRAVEL_JUMP) continue;
			if (!Q2_InRotatingSweep(reach->end, origin, axis, radius,
						origin[axis] + mins[axis], origin[axis] + maxs[axis])) continue;
			if (Q2_InRotatingSweep(reach->start, origin, axis, radius,
						origin[axis] + mins[axis], origin[axis] + maxs[axis])) continue;
			reach->traveltime += 3000;
			n++;
		} //end for
	} //end for
	Log_Print("%6d reachabilities into func_rotating sweeps\n", n);
} //end of the function Q2_AAS_RotatingDrops

//===========================================================================
// Q2 version: BotTravel_Jump takes its run-up inside the jump's start area
// only.  With less than a movement frame of room (30 units at 300 ups) the
// bot jumps from a stand: the jump frame has air acceleration only (Q2
// PM_Accelerate with 1, 30 ups per frame), which carries it about 60 units
// on level ground.  Jumps it cannot make that way are priced like the plat
// shaft drops: on q2dm6 the ones off the ledge by the Railgun ended in the
// lava 31 times out of 34.
//===========================================================================
static int Q2_StandingJumpReaches(vec3_t start, vec3_t end)
{
	int i;
	float dist, dz, x, z, vx, vz, nx, nz, f;
	vec3_t dir;

	VectorSubtract(end, start, dir);
	dz = dir[2];
	dir[2] = 0;
	dist = VectorLength(dir);
	x = z = vx = 0;
	vz = aassettings.phys_jumpvel;
	for (i = 0; i < 40; i++)
	{
		vx += 0.1 * aassettings.phys_maxwalkvelocity;
		if (vx > aassettings.phys_maxwalkvelocity) vx = aassettings.phys_maxwalkvelocity;
		vz -= 0.1 * aassettings.phys_gravity;
		nx = x + 0.1 * vx;
		nz = z + 0.1 * vz;
		if (nx >= dist)
		{
			f = (dist - x) / (nx - x);
			return z + f * (nz - z) >= dz;
		} //end if
		if (vz < 0 && nz < dz) return false;
		x = nx;
		z = nz;
	} //end for
	return false;
} //end of the function Q2_StandingJumpReaches

static void Q2_AAS_StandingJumps(void)
{
	int areanum, i, n, room;
	vec3_t hordir, p;
	aas_reachability_t *reach;
	aas_areasettings_t *settings;

	n = 0;
	for (areanum = 1; areanum < aasworld.numareas; areanum++)
	{
		settings = &aasworld.areasettings[areanum];
		for (i = 0; i < settings->numreachableareas; i++)
		{
			reach = &aasworld.reachability[settings->firstreachablearea + i];
			if ((reach->traveltype & TRAVELTYPE_MASK) != TRAVEL_JUMP) continue;
			//BotTravel_Jump's run-up, away from the jump inside the start area
			VectorSubtract(reach->start, reach->end, hordir);
			hordir[2] = 0;
			VectorNormalize(hordir);
			for (room = 0; room < 80; room += 10)
			{
				VectorMA(reach->start, room + 10, hordir, p);
				p[2] += 1;
				if (AAS_PointAreaNum(p) != areanum) break;
			} //end for
			if (room >= 30) continue;
			if (Q2_StandingJumpReaches(reach->start, reach->end)) continue;
			reach->traveltime += 3000;
			n++;
		} //end for
	} //end for
	Log_Print("%6d jumps without a run-up\n", n);
} //end of the function Q2_AAS_StandingJumps

void Q2_AAS_CalcReachAndClusters(void)
{
	float time;

	Log_Print("Q2: loading full collision model from Q2 BSP data...\n");
	Q2_CM_LoadCollisionFromBSPGlobals();

	worldmodel = 0; // world = model 0
	//initialize bot import structure
	AAS_InitBotImport();
	//load the BSP entity string
	AAS_LoadBSPFile();

	/* Set Q2 physics values BEFORE AAS_InitSettings() reads them.
	 * Without this, BSPC uses Q3 defaults which mismatch Q2 physics
	 * (wrong step height, jump velocity, air control, swim speed, etc.)
	 * causing reachabilities that the bot can't actually execute.
	 * Values from yquake2/src/common/pmove.c. */
	LibVarSet("phys_maxvelocity",       "300");
	LibVarSet("phys_maxwalkvelocity",   "300");
	LibVarSet("phys_maxcrouchvelocity", "100");
	LibVarSet("phys_maxswimvelocity",   "400");
	LibVarSet("phys_maxstep",           "18");
	LibVarSet("phys_maxbarrier",        "50");
	LibVarSet("phys_watergravity",      "100");
	LibVarSet("phys_airaccelerate",     "0");
	LibVarSet("phys_swimaccelerate",    "10");
	/* Gladiator's jump start cost (its be_aas_reach.c: 600 + distance time;
	 * Q3's rs_startjump is 300): fewer routes over the gaps the bots fall
	 * into (q2dm3, q2dm6).  Q2's air control is Q3's (PM_Accelerate with 1
	 * in the air), no reason of its own. */
	LibVarSet("rs_startjump",           "600");

	//init physics settings (reads the LibVars set above)
	AAS_InitSettings();
	//initialize AAS link heap
	AAS_InitAASLinkHeap();
	//initialize the AAS linked entities for the new map
	AAS_InitAASLinkedEntities();
	//reset all reachabilities and clusters
	aasworld.reachabilitysize = 0;
	aasworld.numclusters = 0;
	//set all view portals as cluster portals
	AAS_SetViewPortalsAsClusterPortals();
	//calculate reachabilities
	AAS_InitReachability();
	time = 0;
	while(AAS_ContinueInitReachability(time)) time++;
	Q2_AAS_PlatShaftDrops();
	Q2_AAS_RotatingDrops();
	Q2_AAS_StandingJumps();
	//calculate clusters
	AAS_InitClustering();
} //end of the function Q2_AAS_CalcReachAndClusters
