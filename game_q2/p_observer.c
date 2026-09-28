//===========================================================================
//
// Name:				p_observer.c
// Function:		observer mode
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1998-01-12
// Tab Size:		3
//
// Reconstructed from the 1999 Win32 gamex86.dll (the only release where this
// file was compiled in -- the public source release omitted it).  Behaviour
// matches the disassembly at addresses 0x1007a2e7..0x1007c174 in
// reference/gamex86.dll.  The client-facing observer surface
// (CheckValidCamera, ClientCycleCamera, ClientSetCamera, DoObserver,
// ClientToggleObserver / ClientToggleAutoCam / ClientToggleChaseCam /
// ClientToggleCameraFixed / ClientToggleCameraName, ClientObserverHelp,
// ClientObserverCmd) is reconstructed line-by-line from x86 disasm of
// sub_1007b56a..sub_1007c043.
//
//===========================================================================

#include "g_local.h"
#include "p_observer.h"

// Mask value at gamex86.dll+0x10092 area, embedded as immediate 0x2010003 in
// every trace call.  Standard MASK_OPAQUE = SOLID|LAVA|SLIME|WINDOW, encoded
// as CONTENTS_SOLID(1) | CONTENTS_WINDOW(2) | CONTENTS_LAVA(8) | something.
#ifndef OBSERVER_TRACE_MASK
#define OBSERVER_TRACE_MASK 0x2010003
#endif

#ifdef OBSERVER

extern cvar_t *ra;	/* rocketarena cvar; gates leaving observer mode */
extern void SelectSpawnPoint(edict_t *ent, vec3_t origin, vec3_t angles);

// --- sub_10077d50 -----------------------------------------------------------
// Step `from` toward `to` by at most `maxstep` degrees, taking the shorter
// way around the circle.  Both inputs are anglemod'd first.  Returns the
// new angle, anglemod'd.
//===========================================================================
float ChangeAngle(float from, float to, float maxstep)
{
	float diff;

	from = anglemod(from);
	to   = anglemod(to);

	if (from == to)
		return from;

	diff = to - from;
	if (to > from)
	{
		if (diff >  180.0) diff -= 360.0;
	}
	else
	{
		if (diff < -180.0) diff += 360.0;
	}

	if (diff > 0)
	{
		if (diff >  maxstep) diff =  maxstep;
	}
	else
	{
		if (diff < -maxstep) diff = -maxstep;
	}

	return anglemod(from + diff);
}

//===========================================================================
// Forward declarations (the dispatcher and DoObserver call functions
// defined further down in this file; their reconstructions live near the
// bottom, in the original source order).  Kept here so the linker
// notices the cross-references inside ClientCycleCamera/ClientSetCamera/
// ClientToggleAutoCam/ClientToggleChaseCam which all call
// ClientToggleObserver as their first action.
//===========================================================================
void ClientToggleObserver(edict_t *ent);
void SetClientView(edict_t *ent, vec3_t end, vec3_t out_angles, vec3_t cmdangles);
void ChangeCameraAnglesSmooth(float *out, vec3_t target, float frac, float maxstep);

// --- sub_10077e29 -----------------------------------------------------------
// LerpAngles helper.  Steps pitch (out[0]) and yaw (out[1]) toward target,
// roll (out[2]) snaps directly.  Pitch uses a max of 100*frac*maxstep
// degrees, yaw uses 150*frac*maxstep --- the bot can pan faster than tilt.
// Note: also normalizes target[0] from (180,360] down to (-180,0].
//===========================================================================
void ChangeCameraAnglesSmooth(float *out, vec3_t target, float frac, float maxstep)
{
	float tmp;

	if (target[0] > 180.0f)
		target[0] -= 360.0f;

	tmp    = 100.0f * frac * maxstep;
	out[0] = ChangeAngle(out[0], target[0], tmp);

	tmp    = 150.0f * frac * maxstep;
	out[1] = ChangeAngle(out[1], target[1], tmp);

	out[2] = target[2];
}

//===========================================================================
// AngleDifference                                            sub_10077eba
//
// Public per the original p_observer.h from gladq2_src.  The shipping
// gamex86.dll has exactly ONE copy of this routine in .text at
// 0x10077eba; earlier drafts of this reconstruction carried a second
// "SubAngleDifference" alias which has now been folded into this one.
//
// Returns the signed shortest-arc difference (ang1 - ang2), wrapped to
// the range [-180, 180].  Reconstructed line-by-line from the disasm.
//===========================================================================
float AngleDifference(float ang1, float ang2)
{
	float diff;

	diff = ang1 - ang2;
	if (ang1 > ang2)
	{
		if (diff >  180.0) diff -= 360.0;
	}
	else
	{
		if (diff < -180.0) diff += 360.0;
	}
	return diff;
} //end of the function AngleDifference

// --- sub_10077f15 -----------------------------------------------------------
// Apply a new origin to the observer.  Writes both pmove.origin (rounded to
// short) and s.origin (float).  Note: pmove.origin is written raw, not
// *8-scaled --- this is intentional, the observer's pmove is PM_FREEZE so
// pmove.origin is never read by the engine.
//===========================================================================
void SetClientOrigin(edict_t *ent, vec3_t origin)
{
	/* `ent->client` re-read per store, not hoisted into a local: each store
	   through the pointer may alias it, so the reference reloads `[ecx+0x54]`
	   every time.  A local forces gcc to keep it in a spill slot instead, which
	   is where our extra 4 frame bytes came from. */
	ent->client->ps.pmove.origin[0] = (short)origin[0];
	ent->client->ps.pmove.origin[1] = (short)origin[1];
	ent->client->ps.pmove.origin[2] = (short)origin[2];
	ent->s.origin[0] = origin[0];
	ent->s.origin[1] = origin[1];
	ent->s.origin[2] = origin[2];
}

// --- sub_10077f7d -----------------------------------------------------------
// Apply new view angles to the observer by locking the client's view via
// pmove.delta_angles.  delta_angles[i] = ANGLE2SHORT(new[i] - cmd[i]).
// Does NOT touch v_angle / ps.viewangles directly --- the engine will
// derive viewangles = cmd + delta on the next pmove tick.
//===========================================================================
void ClientSetViewAngles(edict_t *ent, vec3_t new_angles, vec3_t cmd_angles)
{
	ent->client->ps.pmove.delta_angles[0] = (short)((int)(anglemod(AngleDifference(new_angles[0], cmd_angles[0])) * 65536.0f / 360.0f) & 0xffff);
	ent->client->ps.pmove.delta_angles[1] = (short)((int)(anglemod(AngleDifference(new_angles[1], cmd_angles[1])) * 65536.0f / 360.0f) & 0xffff);
	ent->client->ps.pmove.delta_angles[2] = (short)((int)(anglemod(AngleDifference(new_angles[2], cmd_angles[2])) * 65536.0f / 360.0f) & 0xffff);
}

//===========================================================================
// sub_10078043 -- SetClientView
//
//   sub_10077f15(ent, cmdangles_arg);
//   sub_10077f7d(ent, end_arg, out_angles_arg);
//
// Note: arg layout from disasm is (ent, end, out_angles, cmdangles) but the
// helper SetClientOrigin's vec3 arg is the SECOND function arg (ebp+0xc).
// UpdateChaseCamera calls us with: (ent, end, &out_angles, cmdangles)
// and the disasm shows sub_10077f15(ent, ebp+0xc), i.e. with 'end'.
//===========================================================================
/* NAME ASSIGNED BY ELIMINATION, not by content -- weaker than the other 25
 * names recovered from gamei386.so in the same pass, and flagged here so a
 * later session does not read it as verified.
 *
 * xmatch.py found no shared string literal and no shared call target for this
 * pair, and the two bodies differ in size by more than half (ours 55 B, real 199 B).  What
 * it has going for it is that after the 25 content-proven renames exactly
 * three real rows and three of ours were left, and this is one of those three.
 *
 * The likeliest reason the bodies diverge: gamei386.so is the AUGUST 2 1999
 * build and gamex86.dll -- which this reconstruction is derived from -- is
 * JULY 18, and the observer path is one of the places the two builds are known
 * to differ.  So the NAME is probably right and the BODY is probably the older
 * revision.  Re-check with xmatch.py before treating the diff as a defect. */
void SetClientView(edict_t *ent, vec3_t end, vec3_t out_angles, vec3_t cmdangles)
{
	SetClientOrigin(ent, end);                          // sub_10077f15
	ClientSetViewAngles(ent, out_angles, cmdangles);        // sub_10077f7d
} //end of the function SetClientView

// --- sub_1007806c -----------------------------------------------------------
// Cam_Message: when the OBSERVER's camera-state flag 0x10
// (CAMFL_NAME == "show target name + score") is set in
// client->camera.flags (offset client+0xf98), print the target's name
// and current score to the observer via gi.centerprintf.  Used by
// Cam_EnterDeathMode and Cam_EnterFlyByMode as a HUD notify.
//
// Reconstructed line-by-line from gamex86.dll @ 0x1007806c.  Stack
// frame: char score_buf[0x80] @ [ebp-0x80], char fragword[0x80] @
// [ebp-0x100].  The CRT helpers at 0x1008721f and 0x10087010 are
// sprintf() and strcpy() respectively.
//===========================================================================
void Cam_Message(edict_t *ent, edict_t *target, char *prefix)
{
	/* Declaration order is recorded twice over: MSVC /Od lays the first-declared
	   local closest to ebp, so the original's -0x80 / -0x100 pair puts score_buf
	   first; and gcc 2.7 fills its address-taken group top-down in declaration
	   order, which gamei386.so confirms (fragword at 0x10/0x14, score_buf at
	   0x90).  Reversing these two was worth 16 insn_diffs here and 16 more in
	   Cam_EnterDeathMode, which inlines this function. */
	char score_buf[0x80];                                /* [ebp-0x80]  */
	char fragword[0x80];                                 /* [ebp-0x100] */

	if (!(ent->client->camera.flags & CAMFL_NAME))      /* +0xf98 */
		return;

	sprintf(score_buf, "%d", target->client->resp.score);   /* +0xda8 */

	if (target->client->resp.score != 1 && target->client->resp.score != -11)
		strcpy(fragword, "frags");
	else
		strcpy(fragword, "frag");

	gi.centerprintf(ent, "%s\n\n\n%s - %s %s",
	                prefix,
	                target->client->pers.netname,       /* +0x2bc */
	                score_buf,
	                fragword);
}

// --- sub_10078125 -----------------------------------------------------------
// CanSee(ent, point): true if ent->s.origin can see `point` through the
// world (water boundary handled: if start/end straddle water, re-trace from
// the water-surface hit forward with the water content bits removed).
//===========================================================================
qboolean Cam_SpotVisible(edict_t *ent, vec3_t point)
{
	/* start, end, sav_origin -- the order gamei386.so's frame records (slotmap
	   pairs ref 0x68/0x5c/0x50 to start/end/sav_origin).  Cam_EntityVisible
	   below has the same triple in the same order. */
	vec3_t start, end, sav_origin;
	int contmask;
	trace_t tr;

	VectorCopy(ent->s.origin, sav_origin);

	if (!gi.inPVS(sav_origin, point))
		return false;

	contmask = 1;
	VectorCopy(sav_origin, start);
	VectorCopy(point,      end);

	if (gi.pointcontents(point) & MASK_WATER)
		contmask |= MASK_WATER;

	if (gi.pointcontents(sav_origin) & MASK_WATER)
	{
		if (!(contmask & MASK_WATER))
		{
			VectorCopy(point,      start);
			VectorCopy(sav_origin, end);
		}
		contmask ^= MASK_WATER;
	}

	tr = gi.trace(start, NULL, NULL, end, ent, contmask);

	if (tr.contents & MASK_WATER)
	{
		/* No `!`: the original re-traces when the surface IS translucent (or when
		   there is no surface at all).  `je bd6b9` on the null test jumps INTO this
		   block and `je bd6d9` on the flags test jumps PAST it, so the condition is
		   `surface == NULL || (flags & TRANS)`.  Behavioural, not just a byte diff. */
		if (tr.surface == NULL || (tr.surface->flags & (SURF_TRANS33|SURF_TRANS66)))
		{
			contmask &= ~MASK_WATER;
			tr = gi.trace(tr.endpos, NULL, NULL, end, ent, contmask);
		}
	}

	if (tr.fraction >= 1.0f)
		return true;
	return false;
}

// --- sub_100782ad -----------------------------------------------------------
// TargetVisible(ent, target): like Cam_SpotVisible, but trace endpoint is
// target->s.origin and target itself is treated as a valid hit
// (tr.ent == target counts as visible).
//===========================================================================
qboolean Cam_EntityVisible(edict_t *ent, edict_t *target)
{
	vec3_t start, end, sav_origin;
	edict_t *passent;
	edict_t *targent;
	int contmask;
	trace_t tr;

	VectorCopy(ent->s.origin, sav_origin);

	if (!gi.inPVS(sav_origin, target->s.origin))
		return false;

	contmask = 1;
	passent = ent;
	targent = target;
	VectorCopy(sav_origin,       start);
	VectorCopy(target->s.origin, end);

	if (gi.pointcontents(target->s.origin) & MASK_WATER)
		contmask |= MASK_WATER;

	if (gi.pointcontents(sav_origin) & MASK_WATER)
	{
		if (!(contmask & MASK_WATER))
		{
			passent = target;
			targent = ent;
			VectorCopy(target->s.origin, start);
			VectorCopy(sav_origin,       end);
		}
		contmask ^= MASK_WATER;
	}

	tr = gi.trace(start, NULL, NULL, end, passent, contmask);

	if (tr.contents & MASK_WATER)
	{
		/* No `!`: the original re-traces when the surface IS translucent (or when
		   there is no surface at all).  `je bd6b9` on the null test jumps INTO this
		   block and `je bd6d9` on the flags test jumps PAST it, so the condition is
		   `surface == NULL || (flags & TRANS)`.  Behavioural, not just a byte diff. */
		if (tr.surface == NULL || (tr.surface->flags & (SURF_TRANS33|SURF_TRANS66)))
		{
			contmask &= ~MASK_WATER;
			tr = gi.trace(tr.endpos, NULL, NULL, end, passent, contmask);
		}
	}

	if (tr.fraction >= 1.0f || tr.ent == targent)
		return true;
	return false;
}

// --- sub_1007845f -----------------------------------------------------------
// NextClient(prev): return next entity after `prev` in g_edicts that has
// classname=="player" (case-insensitive), is inuse, and does NOT have the
// FL_OBSERVER (0x10000) flag.  If `prev` is NULL, start from g_edicts[1]
// (the lea +0x458 indexes prev+1 in either case).
//===========================================================================
edict_t *Cam_Cycle(edict_t *prev)
{
	int i;
	edict_t *e;

	if (prev == NULL)
		i = 0;
	else
		i = (int)(prev - g_edicts);

	while (i < game.maxclients)
	{
		/* `&g_edicts[i+1]`, not `g_edicts + i + 1`: the former scales (i+1) once
		   (`lea ebp,[eax*8+0x458]`), the latter adds g_edicts+i and then a second
		   0x458, which is what we emitted. */
		e = &g_edicts[i + 1];
		if (e->inuse)
		{
			if (!(e->flags & FL_OBSERVER))
			{
				if (stricmp(e->classname, "player") == 0)
					return e;
			}
		}
		i++;
	}
	return NULL;
}

//===========================================================================
// Autocam state handlers and Cam_UpdateView / ChangeChaseCamOffset / SetClientView
//
// These helpers live in the original DLL at the addresses noted below.
// They are now fully reconstructed below; these are the dispatch-order
// forward declarations.
//===========================================================================
void Cam_IdleThink(edict_t *ent, usercmd_t *ucmd);   /* sub_1007a2e7 */
void Cam_FixedThink(edict_t *ent, usercmd_t *ucmd);   /* sub_10079f4d */
void Cam_FlyByThink(edict_t *ent, usercmd_t *ucmd);   /* sub_10079f64 */
void Cam_FollowThink(edict_t *ent, usercmd_t *ucmd);   /* sub_1007a1d2 */
void Cam_DeathThink(edict_t *ent, usercmd_t *ucmd);   /* sub_10079c26 */
void Cam_UpdateView(edict_t *ent, float speed, usercmd_t *ucmd); /* sub_100784f9 */
void ChangeChaseCamOffset(edict_t *ent, usercmd_t *ucmd);      /* sub_1007ace3 */
void SetClientView(edict_t *ent, vec3_t end, vec3_t out_angles, vec3_t cmdangles); /* sub_10078043 */

//===========================================================================
// sub_100784f9 -- Cam_UpdateView
//
// 'speed': passed as second argument (the dispatcher passes 1.0, 0.0, 0.0,
//          100.0, 0.0 for states 0,1,2,7,3 respectively).
//
// Branches:
//   speed == 0       -> teleport: SetClientOrigin(ent, cam->dest)
//   else trace from cam->ent->origin to cam->dest; if hit, snap; else
//     compute step along the trace direction and advance.
//   Then per state (0 / 3 / 7 / other) compute target angle vector by
//   subtracting positions and feed into ChangeCameraAnglesSmooth.
//===========================================================================
void Cam_UpdateView(edict_t *ent, float speed, usercmd_t *ucmd)
{
	camera_t *cam;
	/* move, target_ang, cmdangles, then tr LAST -- the only layout that fits
	   gamei386.so's frame: move's 7/8/10 at 0x70..0x78, target_ang[0] alone at
	   0x64, cmdangles' 2/1/1 at 0x58..0x60, and the 56-byte trace filling
	   0x20..0x57 with its two references at 0x20 and 0x28. */
	vec3_t move;
	vec3_t target_ang;
	vec3_t cmdangles;
	float dist, dt, time_to_close;
	trace_t tr;

	cam = &ent->client->camera;

	// if (speed == 0) snap to dest and skip the lerp.
	if (speed == 0.0f)
	{
		SetClientOrigin(ent, cam->dest);
		goto angles_phase;
	}

	/* NULL mins/maxs -- ref pushes two immediate zeroes, not the address of
	   vec3_origin, and every other trace in this file passes NULL. */
	tr = gi.trace(ent->s.origin, NULL, NULL, cam->dest,
	              ent, OBSERVER_TRACE_MASK);
	if (tr.fraction < 1.0)
	{
		SetClientOrigin(ent, cam->dest);
		goto angles_phase;
	}

	move[0] = cam->dest[0] - ent->s.origin[0];
	move[1] = cam->dest[1] - ent->s.origin[1];
	move[2] = cam->dest[2] - ent->s.origin[2];
	dist = VectorNormalize(move);

	dt = level.time - cam->lasttime;
	if (dist < 0.0f)
		speed = -speed;
	/* sqrt, not acos: the reference expands it inline as `fdivr; fsqrt` (gcc 2.7
	   treats sqrt() as a builtin) and calls nothing.  Behavioural. */
	time_to_close = (float)sqrt((double)(dist / speed));
	if (dt < time_to_close)
	{
		// clamp move scale = (2.0 * time_to_close - dt) * speed * dt
		dist = (speed * dt) * (2.0f * time_to_close - dt);
	}
	VectorScale(move, dist, move);
	move[0] += ent->s.origin[0];
	move[1] += ent->s.origin[1];
	move[2] += ent->s.origin[2];
	SetClientOrigin(ent, move);

angles_phase:
	if (cam->state == 0)
	{
		// target_ang = vectoangles(viewtarget - dest)
		move[0] = cam->viewtarget[0] - cam->dest[0];
		move[1] = cam->viewtarget[1] - cam->dest[1];
		move[2] = cam->viewtarget[2] - cam->dest[2];
		vectoangles(move, target_ang);
		// ChangeCameraAnglesSmooth(cam->angles, target_ang, 0.2, level.time - lasttime)
		ChangeCameraAnglesSmooth(cam->angles, target_ang, level.time - cam->lasttime, 0.2f);
	}
	else if (cam->state == 3)
	{
		// Aim at where the target is aiming: gclient_t.v_angle (at +0xe8c).
		ChangeCameraAnglesSmooth(cam->angles,
		             cam->ent->client->v_angle,
		             level.time - cam->lasttime,
		             0.8f);
	}
	else if (cam->state == 7)
	{
		move[0] = cam->ent->s.origin[0] - ent->s.origin[0];
		move[1] = cam->ent->s.origin[1] - ent->s.origin[1];
		move[2] = cam->ent->s.origin[2] - ent->s.origin[2];
		vectoangles(move, target_ang);
		ChangeCameraAnglesSmooth(cam->angles, target_ang, level.time - cam->lasttime, 0.2f);
	}
	else
	{
		// Default (states 1, 2 and anything else): aim toward cam->ent's
		// origin and just normalise into cam->angles directly.
		move[0] = cam->ent->s.origin[0] - ent->s.origin[0];
		move[1] = cam->ent->s.origin[1] - ent->s.origin[1];
		move[2] = cam->ent->s.origin[2] - ent->s.origin[2];
		vectoangles(move, cam->angles);
	}

	cmdangles[PITCH] = SHORT2ANGLE(ucmd->angles[PITCH]);
	cmdangles[YAW]   = SHORT2ANGLE(ucmd->angles[YAW]);
	cmdangles[ROLL]  = SHORT2ANGLE(ucmd->angles[ROLL]);
	ClientSetViewAngles(ent, cam->angles, cmdangles);

	cam->lasttime = level.time;
} //end of the function Cam_UpdateView

// --- sub_1007886c -----------------------------------------------------------
// Cam_SetAuto: snap the autocam's dest to the observer's own
// origin, derive a viewtarget one unit forward along the current
// cam->angles, then enter idle (state 0) via Cam_UpdateView(ent, 0, ucmd).
// Used by Cam_EnterIdleMode.
//
// Reconstructed line-by-line from gamex86.dll @ 0x1007886c.  The
// compiler kept cam at [ebp-0x10] = &client->camera (client+0xf64) and
// the AngleVectors out-vector at [ebp-0xc..-4].  Field offsets used:
//   cam+0x04 = angles, cam+0x40 = dest, cam+0x4c = viewtarget
//   ent+0x04 = ent->s.origin
//===========================================================================
void Cam_SetAuto(edict_t *ent, usercmd_t *ucmd)
{
	camera_t *cam;
	vec3_t forward;                                       /* [ebp-0xc..-4] */

	cam = &ent->client->camera;                           /* [ebp-0x10]    */

	cam->dest[0] = ent->s.origin[0];
	cam->dest[1] = ent->s.origin[1];
	cam->dest[2] = ent->s.origin[2];

	AngleVectors(cam->angles, forward, NULL, NULL);

	/* origin first, then forward: ref loads the ent-relative term and adds the
	   local (`fld [edi+4]; fadd [esp+0x20]`), we had it the other way round.
	   Textual operand order is a real gcc 2.7 lever for an FP add, and this one
	   also lands in Cam_EnterIdleMode, which inlines this function. */
	cam->viewtarget[0] = ent->s.origin[0] + forward[0];
	cam->viewtarget[1] = ent->s.origin[1] + forward[1];
	cam->viewtarget[2] = ent->s.origin[2] + forward[2];

	Cam_UpdateView(ent, 0, ucmd);
}

// --- sub_100788ff -----------------------------------------------------------
// AbortAutocam: stop following any target, lock the current view in place,
// hold for 2 seconds.
//===========================================================================
void Cam_EnterIdleMode(edict_t *ent, usercmd_t *ucmd)
{
	camera_t *cam = &ent->client->camera;

	Cam_SetAuto(ent, ucmd);

	cam->state       = 0;
	cam->pause_time  = level.time + 2.0f;
	cam->search_time = level.time;
	cam->dest2[0]    = cam->viewtarget[0];
	cam->dest2[1]    = cam->viewtarget[1];
	cam->dest2[2]    = cam->viewtarget[2];
	cam->ent         = ent;
}

//===========================================================================
// NOTE: ClientSetViewAngles was declared in Mr. Elusive's original
// p_observer.h (gladq2_src, 1998-01-12) but its body was removed from
// p_observer.c before the 1999 shipping gamex86.dll was compiled --
// byte-pattern search of the full DLL finds no function matching the
// signature.  The header declaration is kept verbatim for archival
// fidelity to the 1998 source; no body is provided here because doing
// so would require synthesizing code with no disassembly origin.
// Nothing in the reconstructed codebase calls it, so the missing
// definition does not break the link.
//===========================================================================

//===========================================================================
// Faithful disassembly-derived reconstruction of the 5 autocam state
// handlers + Cam_UpdateView / ChangeChaseCamOffset / SetClientView.
//
// Translated line-by-line from gamex86.dll at:
//   sub_10078043 = SetClientView   (18 lines  -> the trivial dispatch)
//   sub_10079f4d = Cam_FixedThink   ("fixed mode" centerprint)
//   sub_1007a1d2 = Cam_FollowThink   (target-tracking)
//   sub_10079f64 = Cam_FlyByThink   (chase from a fixed distance)
//   sub_10079c26 = Cam_DeathThink   ("bodyque" / corpse chase)
//   sub_1007ace3 = ChangeChaseCamOffset      (m_pitch / chaseoffset twiddling)
//   sub_100784f9 = Cam_UpdateView            (per-state move + angle blend)
//   sub_1007a2e7 = Cam_IdleThink   (target selection)
//
// Constants come from .rdata at gamex86.dll+0x92000.  Inline literals
// are decoded as their float values for readability.
//
// The nested helper functions (sub_10077e29, sub_10077eba, sub_10077f15,
// sub_10077f7d, sub_1007845f, sub_100788ff, sub_1007897a, sub_10078125,
// sub_100782ad, sub_10078a04, sub_10078bcc, sub_10079a92, sub_10079acf,
// sub_10078c53 Cam_TryFlyByVector, sub_10078f4a Cam_GetFlyBySpot,
// sub_10085904) are all reconstructed line-by-line from the disassembly
// further down in this file.
//===========================================================================

// ---- Forward declarations for the nested helpers (defined at file bottom) --
void  SetClientOrigin   (edict_t *ent, vec3_t origin);                     /* sub_10077f15 */
void  ClientSetViewAngles   (edict_t *ent, vec3_t out_angles, vec3_t cmd);     /* sub_10077f7d */
void  ChangeCameraAnglesSmooth           (float *out, vec3_t target, float frac, float maxstep); /* sub_10077e29 */
qboolean Cam_SpotVisible      (edict_t *ent, vec3_t point);                      /* sub_10078125 */
qboolean Cam_EntityVisible    (edict_t *ent, edict_t *target);                   /* sub_100782ad */
edict_t *Cam_Cycle       (edict_t *prev);                                   /* sub_1007845f */
void  Cam_EnterIdleMode        (edict_t *ent, usercmd_t *ucmd);                   /* sub_100788ff */
void  Cam_EnterDeathMode       (edict_t *ent);                                    /* sub_1007897a */
void  Cam_GetFollowSpot     (edict_t *ent, vec3_t out);                        /* sub_10078a04 */
void  Cam_GetFollowTarget     (edict_t *ent, vec3_t out);                        /* sub_10078bcc */
void  Cam_GetFlyByTarget      (edict_t *target, vec3_t spot);                    /* sub_10079a92 */
void  Cam_EnterFlyByMode    (edict_t *ent, edict_t *target, usercmd_t *ucmd);  /* sub_10079acf */
// (sub_10085904 = q_shared.c VectorCompare, declared in q_shared.h)


// --- sub_1007897a -----------------------------------------------------------
// OnTargetDeath: announce score, then drop into bodyque-chase (state 7)
// for 2 seconds with a 5-second search timeout.
//===========================================================================
void Cam_EnterDeathMode(edict_t *ent)
{
	camera_t *cam = &ent->client->camera;

	Cam_Message(ent, cam->ent, "");

	cam->dest[0]     = ent->s.origin[0];
	cam->dest[1]     = ent->s.origin[1];
	cam->dest[2]     = ent->s.origin[2];
	cam->state       = 7;
	cam->pause_time  = level.time + 2.0f;
	cam->lastent     = NULL;
	cam->search_time = level.time + 5.0f;
}

// --- sub_10078a04 -----------------------------------------------------------
// GetTargetMuzzle: compute a "muzzle"-aligned camera offset and trace from
// the target's viewpoint along it.  Pitch is decoupled (camera stays
// horizontal); 90 units back and 16 units up.  If the trace hits a wall,
// snap to 70 units off the wall along the wall normal; otherwise, push 20
// more units backward beyond the offset point.
//===========================================================================
void Cam_GetFollowSpot(edict_t *ent, vec3_t out)
{
	camera_t *cam = &ent->client->camera;
	/* Order recovered from gamei386.so's frame (slotmap.py pairs ref
	   0x98/0x8c/0x80/0x74/0x68/0x5c/0x50 to viewfrom/endpos/up/horizfwd/ofs/
	   angles/fwd). */
	vec3_t viewfrom, endpos, up, horizfwd, ofs, angles, fwd;
	trace_t tr;

	/* copy cam->angles into local; first AngleVectors uses unmodified */
	angles[0] = cam->angles[0];
	angles[1] = cam->angles[1];
	angles[2] = cam->angles[2];

	/* 1) up from full angles */
	AngleVectors(angles, NULL, NULL, up);

	/* zero pitch, anglemod yaw, then forward from flat angles */
	/* No `+ 0.0f` -- an IDA artifact.  Ref just pushes the dword and calls:
	   `mov eax,[esp+0x70]; push eax; call anglemod`. */
	angles[1] = anglemod(angles[1]);
	angles[0] = 0.0f;
	AngleVectors(angles, fwd, NULL, NULL);

	/* build a horizontal forward, re-tilted so it's perpendicular to up */
	horizfwd[0] = fwd[0];
	horizfwd[1] = fwd[1];
	/* `-1.0`, a DOUBLE: the whole expression is then double and its store to a
	   float costs gcc 2.7 a float_truncate stack temp -- the never-referenced
	   4-byte slot at the bottom of gamei386.so's frame (0x94, ours was 0x90).
	   The x87 code is identical either way. */
	horizfwd[2] = (horizfwd[0]*up[0] + horizfwd[1]*up[1]) * -1.0 / up[2];
	VectorNormalize(horizfwd);

	/* ofs = -90 * horizfwd + (0,0,16) */
	VectorScale(horizfwd, -90.0f, ofs);
	ofs[2] += 16.0f;

	/* viewfrom = cam->ent->s.origin + cam->ent->client->ps.viewoffset */
	viewfrom[0] = cam->ent->client->ps.viewoffset[0] + cam->ent->s.origin[0];
	viewfrom[1] = cam->ent->client->ps.viewoffset[1] + cam->ent->s.origin[1];
	viewfrom[2] = cam->ent->client->ps.viewoffset[2] + cam->ent->s.origin[2];

	endpos[0] = viewfrom[0] + ofs[0];
	endpos[1] = viewfrom[1] + ofs[1];
	endpos[2] = viewfrom[2] + ofs[2];

	tr = gi.trace(viewfrom, NULL, NULL, endpos, cam->ent, OBSERVER_TRACE_MASK);

	if (tr.fraction < 1.0f && tr.ent != g_edicts)
		VectorMA(tr.endpos, 70.0f, tr.plane.normal, out);
	else
		VectorMA(tr.endpos, 20.0f, horizfwd, out);
}

// --- sub_10078bcc -----------------------------------------------------------
// GetTargetAimEnd: 2048 units along cam->angles from cam->ent->s.origin
// when the target is alive; just the corpse position when dead.
//===========================================================================
void Cam_GetFollowTarget(edict_t *ent, vec3_t out)
{
	camera_t *cam = &ent->client->camera;

	if (cam->ent->deadflag == 0)
	{
		vec3_t fwd;
		AngleVectors(cam->angles, fwd, NULL, NULL);
		VectorMA(cam->ent->s.origin, 2048.0f, fwd, out);
	}
	else
	{
		out[0] = cam->ent->s.origin[0];
		out[1] = cam->ent->s.origin[1];
		out[2] = cam->ent->s.origin[2];
	}
}

// --- sub_10078c53 -----------------------------------------------------------
// Score a candidate camera offset relative to the target (cam->ent).
//
//   * Normalises 'ofs' in place, then scales it to 700 units (the search
//     radius).  Callers that share an offset vector across multiple calls
//     will therefore see it overwritten.
//   * Traces 700 units from the target's bbox-top (origin with z = maxs[2]-8)
//     along 'ofs' with mask 0x2010003 (MASK_SHOT|MASK_OPAQUE).  If the trace
//     hits anything other than worldspawn the candidate is rejected.
//   * The candidate point is the trace endpos pulled 5 units back toward
//     the target.  If that point is inside solid (pointcontents & 1) the
//     candidate is rejected.
//   * If the target and candidate are on opposite sides of a water surface
//     (mask 0x38 = CONTENTS_LAVA|SLIME|WATER) re-trace with the water bits
//     included so the candidate is moved to the water boundary.
//   * If the resulting trace contents include water, require that we hit a
//     translucent surface (SURF_TRANS33|SURF_TRANS66 = 0x30); otherwise
//     reject.
//   * Reject anything CLOSER than 50 units to the target.
//   * Returns 1111 for any rejection (well above the 1000-init); otherwise
//     returns fabs(333 - dist), so callers, which minimise, prefer a spot
//     about 333 units from the target.
//   * Both originals test `dist < 50` (gamex86.dll: fcomp 50.0; test ah,1;
//     je accept -- gamei386.so: fld 50.0; fcomp st(1); je reject).  An
//     earlier draft had the test inverted, rejecting everything farther than
//     50 units, i.e. almost every candidate.
//   * Locals: gamei386.so's frame is viewfrom, unit5, diff, start, end,
//     mins, maxs, tr from the top down, and the second water re-trace copies
//     tr.endpos into `end` (not into a second temporary).
//===========================================================================
float Cam_TryFlyByVector(edict_t *ent, vec3_t ofs, vec3_t out_endpos)
{
	vec3_t    viewfrom, unit5, diff, start, end;
	vec3_t    mins = {-8, -8, -8}, maxs = {8, 8, 8};
	trace_t   tr;
	camera_t *cam = &ent->client->camera;
	int       cv_view, cv_end;
	float     dist;

	VectorNormalize(ofs);
	VectorScale(ofs, 5.0f,   unit5);
	VectorScale(ofs, 700.0f, ofs);

	VectorCopy(cam->ent->s.origin, viewfrom);
	viewfrom[2] = cam->ent->absmax[2] - 8.0f;

	end[0] = viewfrom[0] + ofs[0];
	end[1] = viewfrom[1] + ofs[1];
	end[2] = viewfrom[2] + ofs[2];

	/* disasm @ 0x10078d27: push 0x2010003 (= CONTENTS_SOLID|CONTENTS_WINDOW
	   |CONTENTS_PLAYERCLIP|CONTENTS_MONSTER = MASK_PLAYERSOLID).  Earlier
	   "MASK_SHOT|MASK_OPAQUE" was 0x600001B, off by ~3M bits. */
	tr = gi.trace(viewfrom, mins, maxs, end, cam->ent, MASK_PLAYERSOLID);

	if (tr.ent != g_edicts)
		return 1111.0f;

	out_endpos[0] = tr.endpos[0] - unit5[0];
	out_endpos[1] = tr.endpos[1] - unit5[1];
	out_endpos[2] = tr.endpos[2] - unit5[2];

	if (gi.pointcontents(out_endpos) & 1)
		return 1111.0f;

	cv_view = gi.pointcontents(viewfrom) & MASK_WATER;
	cv_end  = gi.pointcontents(out_endpos) & MASK_WATER;

	if (cv_view && !cv_end)
	{
		/* disasm @ 0x10078e09 / 0x10078e62: push 0x201003b
		   = MASK_PLAYERSOLID | MASK_WATER. */
		VectorCopy(tr.endpos, start);
		tr = gi.trace(start, NULL, NULL, viewfrom, cam->ent,
		              MASK_PLAYERSOLID|MASK_WATER);
	}
	else if (!cv_view && cv_end)
	{
		VectorCopy(tr.endpos, end);
		tr = gi.trace(viewfrom, NULL, NULL, end, cam->ent,
		              MASK_PLAYERSOLID|MASK_WATER);
	}

	if (tr.contents & MASK_WATER)
	{
		if (!tr.surface)
			return 1111.0f;
		if (!(tr.surface->flags & (SURF_TRANS33|SURF_TRANS66)))
			return 1111.0f;
	}

	diff[0] = cam->ent->s.origin[0] - out_endpos[0];
	diff[1] = cam->ent->s.origin[1] - out_endpos[1];
	diff[2] = cam->ent->s.origin[2] - out_endpos[2];
	dist = VectorLength(diff);
	if (dist < 50)
		return 1111;

	return fabs(333 - dist);
}

#define TRY_CANDIDATE(ofs_expr) { \
		ofs_expr; \
		score = Cam_TryFlyByVector(ent, cand_ofs, cand_endpos); \
		if (score < best_score) { \
			best_score = score; \
			VectorCopy(cand_endpos, input_angles); \
		} \
	}

qboolean Cam_GetFlyBySpot(edict_t *ent, edict_t *target, vec3_t out)
{
	/* vec3 order recovered from gamei386.so's frame: gcc 2.7 fills the
	   address-taken group top-down in declaration order, and slotmap.py pairs
	   ref 0x50/0x44/0x38/0x2c/0x20/0x14 to cand_ofs/cand_endpos/input_angles/
	   fwd/right/up -- i.e. the two candidate vectors first, then the angles and
	   the three axes in AngleVectors' own argument order. */
	vec3_t    cand_ofs;
	float     best_score;
	camera_t *cam;
	vec3_t    cand_endpos;
	float     score;
	vec3_t    input_angles;     /* doubles as input_angles slot */
	vec3_t    fwd;
	vec3_t    right;
	vec3_t    up;

	(void)target;   /* unused in the original */

	cam = &ent->client->camera;

	/* disasm @ 0x10078f5f..0x10078f8d: slots [ebp-0x30..-0x28] are zeroed
	   via a chained x[0]=x[1]=x[2]=0 (VectorClear) then [ebp-0x28] is
	   set to cam->ent->s.angles[2].  The same slot is later overwritten
	   when a candidate beats best_score, so input_angles is reused as
	   input_angles -- no separate variable. */
	input_angles[0] = input_angles[1] = input_angles[2] = 0;
	input_angles[2] = cam->ent->s.angles[2];

	AngleVectors(input_angles, fwd, right, up);
	VectorScale(fwd, 3.0f, fwd);
	best_score = 1000.0f;

	/* Tier 0: ±3fwd ±right combined with +up */
	TRY_CANDIDATE((VectorAdd     (up, fwd, cand_ofs),
	               VectorAdd     (cand_ofs, right, cand_ofs)));
	TRY_CANDIDATE((VectorSubtract(up, fwd, cand_ofs),
	               VectorAdd     (cand_ofs, right, cand_ofs)));
	TRY_CANDIDATE((VectorAdd     (up, fwd, cand_ofs),
	               VectorSubtract(cand_ofs, right, cand_ofs)));
	TRY_CANDIDATE((VectorSubtract(up, fwd, cand_ofs),
	               VectorSubtract(cand_ofs, right, cand_ofs)));

	if (best_score >= 1000.0f)
	{
		/* Tier 1: ±3fwd alone or ±right alone, combined with +up */
		TRY_CANDIDATE( VectorAdd     (up, fwd,   cand_ofs));
		TRY_CANDIDATE( VectorSubtract(up, fwd,   cand_ofs));
		TRY_CANDIDATE( VectorAdd     (up, right, cand_ofs));
		TRY_CANDIDATE( VectorSubtract(up, right, cand_ofs));
	}

	if (best_score >= 1000.0f)
	{
		/* Tier 2: ±3fwd ±right at target altitude */
		TRY_CANDIDATE( VectorAdd     (fwd,   right, cand_ofs));
		TRY_CANDIDATE( VectorSubtract(fwd,   right, cand_ofs));
		TRY_CANDIDATE( VectorSubtract(right, fwd,   cand_ofs));
		TRY_CANDIDATE((VectorClear   (cand_ofs),
		               VectorSubtract(cand_ofs, fwd,   cand_ofs),
		               VectorSubtract(cand_ofs, right, cand_ofs)));
	}

	if (best_score >= 1000.0f)
	{
		/* Tier 3: pure ±fwd or ±right (these calls mutate fwd/right to 700*unit) */
		score = Cam_TryFlyByVector(ent, fwd,   cand_endpos);
		if (score < best_score) { best_score = score; VectorCopy(cand_endpos, input_angles); }
		score = Cam_TryFlyByVector(ent, right, cand_endpos);
		if (score < best_score) { best_score = score; VectorCopy(cand_endpos, input_angles); }
		TRY_CANDIDATE((VectorClear(cand_ofs), VectorSubtract(cand_ofs, fwd,   cand_ofs)));
		TRY_CANDIDATE((VectorClear(cand_ofs), VectorSubtract(cand_ofs, right, cand_ofs)));
	}

	if (best_score >= 1000.0f)
	{
		/* Tier 4: -up combined with ±fwd ±right */
		TRY_CANDIDATE((VectorClear(cand_ofs),
		               VectorSubtract(cand_ofs, up,    cand_ofs),
		               VectorAdd     (cand_ofs, fwd,   cand_ofs),
		               VectorAdd     (cand_ofs, right, cand_ofs)));
		TRY_CANDIDATE((VectorClear(cand_ofs),
		               VectorSubtract(cand_ofs, up,    cand_ofs),
		               VectorSubtract(cand_ofs, fwd,   cand_ofs),
		               VectorAdd     (cand_ofs, right, cand_ofs)));
		TRY_CANDIDATE((VectorClear(cand_ofs),
		               VectorSubtract(cand_ofs, up,    cand_ofs),
		               VectorAdd     (cand_ofs, fwd,   cand_ofs),
		               VectorSubtract(cand_ofs, right, cand_ofs)));
		TRY_CANDIDATE((VectorClear(cand_ofs),
		               VectorSubtract(cand_ofs, up,    cand_ofs),
		               VectorSubtract(cand_ofs, fwd,   cand_ofs),
		               VectorSubtract(cand_ofs, right, cand_ofs)));
	}

	if (best_score >= 1000.0f)
	{
		/* Tier 5: -up combined with ±fwd alone or ±right alone */
		TRY_CANDIDATE((VectorClear(cand_ofs),
		               VectorSubtract(cand_ofs, up,  cand_ofs),
		               VectorAdd     (cand_ofs, fwd, cand_ofs)));
		TRY_CANDIDATE((VectorClear(cand_ofs),
		               VectorSubtract(cand_ofs, up,    cand_ofs),
		               VectorAdd     (cand_ofs, right, cand_ofs)));
		TRY_CANDIDATE((VectorClear(cand_ofs),
		               VectorSubtract(cand_ofs, up,  cand_ofs),
		               VectorSubtract(cand_ofs, fwd, cand_ofs)));
		TRY_CANDIDATE((VectorClear(cand_ofs),
		               VectorSubtract(cand_ofs, up,    cand_ofs),
		               VectorSubtract(cand_ofs, right, cand_ofs)));
	}

	if (best_score >= 1000.0f)
		return false;

	VectorCopy(input_angles, out);
	return true;
}
#undef TRY_CANDIDATE

// --- sub_10079a92 -----------------------------------------------------------
// AutocamSetSpot: copy `src->s.origin` to `spot`, then override the Z to
// (src->maxs[2] - 8) --- a point near the top of the entity's bounding box.
//===========================================================================
void Cam_GetFlyByTarget(edict_t *src, vec3_t spot)
{
	spot[0] = src->s.origin[0];
	spot[1] = src->s.origin[1];
	spot[2] = src->s.origin[2];
	spot[2] = src->maxs[2] - 8.0f;
}

void Cam_EnterFlyByMode(edict_t *ent, edict_t *target, usercmd_t *ucmd)
{
	camera_t *cam;
	vec3_t   diff;              /* declared before pos -- gamei386.so's frame */
	vec3_t   pos;

	cam = &ent->client->camera;

	/* disasm @ 0x10079aea: fadd qword [0x100921a8]=0.4 -- double-precision
	   add, so the literal must be a double (no `f` suffix) to match. */
	cam->pause_time = level.time + 0.4;

	if (cam->ent != target)
	{
		cam->ent = target;
		if (cam->lastent != cam->ent)
		{
			Cam_Message(ent, cam->ent, "looking at");
			cam->lastent = target;
		}
	}

	if (!Cam_GetFlyBySpot(ent, target, pos))
	{
		Cam_EnterIdleMode(ent, ucmd);
		cam->pause_time = level.time + 2.0f;
		return;
	}

	Cam_GetFlyByTarget(cam->ent, cam->viewtarget);

	cam->dest[0] = pos[0];
	cam->dest[1] = pos[1];
	cam->dest[2] = pos[2];
	cam->state   = 2;

	diff[0] = cam->dest[0] - cam->viewtarget[0];
	diff[1] = cam->dest[1] - cam->viewtarget[1];
	diff[2] = cam->dest[2] - cam->viewtarget[2];

	/* disasm @ 0x10079be7: fmul qword [0x100922b0]=1.5 -- double-precision
	   multiply, so the literal must be a double (no `f` suffix).
	   Also: the value is written to cam->maxflybydist BEFORE the 500.0
	   floor check (disasm @ 0x10079bed stores, then reloads at 0x10079bf6
	   and compares against [0x10092184]=500.0f).  The clamp fires when
	   the stored value is < 500, enforcing a FLOOR (not a ceiling). */
	cam->maxflybydist = VectorLength(diff) * 1.5;
	if (cam->maxflybydist < 500.0f) cam->maxflybydist = 500.0f;

	Cam_UpdateView(ent, 0, ucmd);
}

//===========================================================================
// sub_10079c26 -- Cam_DeathThink
//
// Bodyque (corpse) tracking.  When the chase target is a corpse it links
// to a chain via [+0x19c]; we follow that chain until it terminates or
// the corpse settles, then back the camera off by 59 units.
//===========================================================================
void Cam_DeathThink(edict_t *ent, usercmd_t *ucmd)
{
	camera_t *cam = &ent->client->camera;
	float dist;
	vec3_t diff;

	if (cam->search_time < level.time)
	{
		Cam_EnterIdleMode(ent, ucmd);
		return;
	}

	/* disasm @ 0x10079c63: strcmp(classname, "bodyque"); je skip-block
	   (jumps when strings EQUAL).  The chain-follow block runs only when
	   cam->ent is NOT a "bodyque" entity but is alive; the indirection
	   it walks is edict_t.goalentity (offset 0x19c), not chain (0x218). */
	if (strcmp(cam->ent->classname, "bodyque") != 0
	    && cam->ent->deadflag == 0)
	{
		if (cam->ent->goalentity != NULL)
			cam->ent = cam->ent->goalentity;
		else
		{
			Cam_EnterIdleMode(ent, ucmd);
			return;
		}
	}

	if (cam->ent->inuse && cam->ent != ent)
	{
		// distance from camera to target; if close enough, extend pause.
		diff[0] = ent->s.origin[0] - cam->dest[0];
		diff[1] = ent->s.origin[1] - cam->dest[1];
		diff[2] = ent->s.origin[2] - cam->dest[2];
		dist = VectorLength(diff);
		if (dist > 10.0f)
		{
			dist = level.time + 1.5;
			if (cam->pause_time < dist)
				cam->pause_time = dist;
		}
	}

	if (level.time > cam->pause_time)
	{
		Cam_EnterIdleMode(ent, ucmd);
		return;
	}

	if (cam->ent->inuse && cam->ent != ent)
	{
		// Latch viewtarget onto the corpse origin (with z bump 8).
		cam->viewtarget[0] = cam->ent->s.origin[0];
		cam->viewtarget[1] = cam->ent->s.origin[1];
		cam->viewtarget[2] = cam->ent->s.origin[2];
		cam->viewtarget[2] += 8.0f;

		if (!Cam_SpotVisible(ent, cam->viewtarget))
		{
			if (cam->ent->velocity[0] == 0.0f && cam->ent->velocity[1] == 0.0f)
			{
				Cam_EnterIdleMode(ent, ucmd);
				return;
			}
			Cam_EnterFlyByMode(ent, cam->ent, ucmd);
			cam->state = 7;
			cam->pause_time = level.time + 2.0f;
		}

		if (cam->ent->velocity[0] != 0.0f || cam->ent->velocity[1] != 0.0f)
			cam->pause_time = level.time + 2.0f;
	}

	// Compute viewtarget = dest + 59 * normalize(dest - viewtarget)
	diff[0] = cam->dest[0] - cam->viewtarget[0];
	diff[1] = cam->dest[1] - cam->viewtarget[1];
	diff[2] = cam->dest[2] - cam->viewtarget[2];
	if (VectorLength(diff) <= 60.0f)
		return;

	cam->dest[0] = cam->dest[0] - cam->viewtarget[0];
	cam->dest[1] = cam->dest[1] - cam->viewtarget[1];
	cam->dest[2] = cam->dest[2] - cam->viewtarget[2];
	VectorNormalize(cam->dest);
	VectorMA(cam->viewtarget, 59.0f, cam->dest, cam->dest);
} //end of the function Cam_DeathThink

//===========================================================================
// Nested helper functions, reconstructed line-by-line from the gamex86.dll
// disassembly.  All helpers (including the fourth-tier sub_10078c53
// Cam_TryFlyByVector and sub_10078f4a Cam_GetFlyBySpot) are fully
// restored below.
//===========================================================================

//===========================================================================
// sub_10079f4d -- Cam_FixedThink
//
// Just centerprints "fixed mode".
//===========================================================================
void Cam_FixedThink(edict_t *ent, usercmd_t *ucmd)
{
	gi.centerprintf(ent, "fixed mode");
} //end of the function Cam_FixedThink

//===========================================================================
// sub_10079f64 -- Cam_FlyByThink
//
// Idle-chase state.  Holds the camera at cam->dest while watching cam->ent.
// Promotes to state 3 when the target starts shooting at us / we have a
// good line of sight; otherwise re-targets when distance closes.
//===========================================================================
void Cam_FlyByThink(edict_t *ent, usercmd_t *ucmd)
{
	camera_t *cam = &ent->client->camera;
	/* aimend before dir: gamei386.so puts the three-times-referenced vector at
	   0x24 and the other at 0x30, i.e. the reverse of what we had. */
	vec3_t aimend;
	vec3_t dir;
	float dist, yaw_diff;

	if (cam->ent->deadflag != 0)
	{
		if (level.time < 31.0f)
			Cam_EnterIdleMode(ent, ucmd);
		else
			Cam_EnterDeathMode(ent);
		return;
	}

	/* disasm @ 0x10079fc5: cmp [cam->ent + 0x228 (groundentity)], 0; je skip.
	   @ 0x10079fd9: cmp groundentity, [0x100cfd14] (g_edicts = worldspawn);
	   je skip.  Upgrade to state 3 only if standing on a non-world entity. */
	if (cam->ent->groundentity != NULL && cam->ent->groundentity != g_edicts)
	{
		Cam_GetFollowSpot(ent, aimend);
		if (Cam_SpotVisible(ent, aimend))
		{
			cam->state = 3;
			cam->pause_time = level.time + 10.0f;     // 0x10092168 = 10.0
			return;
		}
	}

	dir[0] = cam->dest[0] - cam->ent->s.origin[0];
	dir[1] = cam->dest[1] - cam->ent->s.origin[1];
	dir[2] = cam->dest[2] - cam->ent->s.origin[2];
	dist = VectorLength(dir);

	/* disasm @ 0x1007a07a..0x1007a0a1: SetSpot if (visible && maxflybydist >
	   dist) OR (pause_time > level.time); SetAutocamTarget otherwise. */
	if ((Cam_EntityVisible(ent, cam->ent) && cam->maxflybydist > dist)
	    || cam->pause_time > level.time)
	{
		Cam_GetFlyByTarget(cam->ent, cam->viewtarget);
	}
	else
	{
		Cam_EnterFlyByMode(ent, cam->ent, ucmd);
	}

	dir[0] = cam->ent->s.origin[0] - ent->s.origin[0];
	dir[1] = cam->ent->s.origin[1] - ent->s.origin[1];
	dir[2] = cam->ent->s.origin[2] - ent->s.origin[2];
	dist = VectorLength(dir);

	if (dist < 170.0f)                                  // 0x100925e0 = 170
	{
		yaw_diff = (float)fabs((double)
		           (cam->ent->s.angles[YAW] - ent->s.angles[YAW]));
		if (yaw_diff > 180.0f)
			yaw_diff = 360.0f - yaw_diff;              // 0x10092150 = 360

		if (yaw_diff < 30.0f)                          // 0x1009217c = 30
		{
			Cam_GetFollowSpot(ent, aimend);
			if (Cam_SpotVisible(ent, aimend))
			{
				cam->state = 3;
				cam->pause_time = level.time + 10.0f;
				return;
			}
		}
	}

	if (cam->search_time < level.time)
		Cam_EnterIdleMode(ent, ucmd);
} //end of the function Cam_FlyByThink

// --- sub_10077eba is reconstructed as AngleDifference() at the top of this
// file (it is the public per gladq2_src/p_observer.h).  The earlier draft
// duplicated it as `SubAngleDifference`; that alias has been removed and the
// three call sites below in ChangeCameraAnglesSmooth / ClientSetViewAngles now call
// AngleDifference directly.
//===========================================================================

//===========================================================================
// sub_1007a1d2 -- Cam_FollowThink
//
// Tracking state.  If target died, abort or transfer; otherwise verify line
// of sight, refresh muzzle / aim-end points, and time out after delay.
//===========================================================================
void Cam_FollowThink(edict_t *ent, usercmd_t *ucmd)
{
	camera_t *cam = &ent->client->camera;

	if (cam->ent->deadflag != 0)
	{
		if (level.time < 31.0f)
			Cam_EnterIdleMode(ent, ucmd);
		else
			Cam_EnterDeathMode(ent);
		return;
	}

	if (Cam_EntityVisible(ent, cam->ent))
	{
		Cam_GetFollowSpot(ent, cam->dest);
		Cam_GetFollowTarget(ent, cam->viewtarget);
		// gi.pointcontents(&cam->dest) & 1 == 1  OR  pause_time < level.time
		if ((gi.pointcontents(cam->dest) & 1)
		    || cam->pause_time < level.time)
		{
			Cam_EnterFlyByMode(ent, cam->ent, ucmd);
		}
	}
	else
	{
		Cam_EnterFlyByMode(ent, cam->ent, ucmd);
	}

	if (cam->search_time < level.time)
		Cam_EnterIdleMode(ent, ucmd);
} //end of the function Cam_FollowThink

//===========================================================================
// sub_1007a2e7 -- Cam_IdleThink
//
// Target-selection pass: walks the client list and picks a new chase target
// using one of three random strategies (highest health / highest score /
// uniform-random).  Then either commits to the new target via
// Cam_EnterFlyByMode, or -- if no different target was found -- refines
// cam->viewtarget and (when search_time has elapsed) backtracks cam->dest
// along the viewtarget->dest direction by a random distance.
//
// Reconstructed line-by-line from gamex86.dll @ 0x1007a2e7 (670 disasm
// lines).  The two trace-based "find a new dest2" blocks immediately after
// the angle-vectors call are dead in the original binary (their sqrt<=60
// guard is always satisfied because the operand is anglemod-bounded to
// [0,360)) -- preserved here as faithful no-ops via the same guard.
//===========================================================================
void Cam_IdleThink(edict_t *ent, usercmd_t *ucmd)
{
	/* Group-2 order paired slot-by-slot against gamei386.so by what each slot is
	   USED for, not by guessing: ref 0x28 takes the -1.0f init (best), 0x24 the
	   anglemod argument and the two subtractions (yaw_random), 0x20 the lone
	   `mov ...,0x0` (yaw_to_target), 0x1c the early pointer stores (chosen), and
	   0x18 is the most-referenced pointer (cam).  Descending offsets are the
	   declaration order, so cam comes LAST here. */
	float     best;
	float     yaw_random;   /* also the anglemod'ed value: our `yaw_anglemod`
	                        * was a second name for this slot, and was READ
	                        * UNINITIALISED. */
	float     yaw_to_target;
	edict_t  *chosen;
	edict_t  *it;
	float     rnd;
	camera_t *cam = &ent->client->camera;
	/* angles, fwd, delta, diff -- gamei386.so's group-1 reference counts are
	   3 3 5 | 7 7 9 | 8 8 11 | 1 2 2 top-down, which pairs to exactly this
	   order; fwd and delta were the wrong way round. */
	/* angles, diff, delta, fwd -- gamei386.so's group-1 counts are
	   3 3 5 | 7 7 9 | 8 8 11 | 1 2 2 top-down, which pairs to angles / diff /
	   delta / fwd. */
	vec3_t    diff;         // [-0x90..-0x88]: scratch for VectorLength
	vec3_t    angles;       // [-0x70..-0x68]: random angles → fwd*2000 → block-1 scratch
	                        // (overwritten in block 1 with delta+cam->dest; block 2 reads
	                        // whichever value it currently holds), and the search_time
	                        // block's backtrack point: gamei386.so works that block in this
	                        // slot, not in diff's
	vec3_t    delta;        // [-0x64..-0x5c]: pos diff → vectoangles result → trace target
	                        // → trace endpoint.  Both blocks pass &delta to gi.trace.
	vec3_t    out_ang;      // vectoangles destination; declared HERE, not in the
	                        // inner block -- it is the fourth vec3 of the
	                        // reference's group-1 run (the `1 2 2` after delta's
	                        // `8 8 11`), which an inner-block local cannot be.
	vec3_t    fwd;          // [-0xc..0x0]: AngleVectors output
	trace_t   tr;

	// ent->client->camera
	chosen = ent;

	// chosen := cam->lastent if alive
	if (cam->lastent && cam->lastent->deadflag == 0)
		chosen = cam->lastent;

	best = -1.0f;
	rnd  = (float)(rand() & 0x7FFF) / 32767.0f * 5.0f;   // 0x10092164 = 5.0

	// cam->goalent shortcut
	if (cam->goalent)
	{
		if (cam->goalent->deadflag == 0)
			chosen = cam->goalent;
		else
			chosen = ent;
		goto install_target;
	}

	if (rnd < 1.0f)                                       // 0x10092178 = 1.0
	{
		// Pick best-health client.
		it = Cam_Cycle(NULL);
		while (it != NULL)
		{
			if (it != cam->lastent && it->deadflag == 0)
			{
				float h = (float)it->health;
				if (best < h)
				{
					chosen = it;
					best   = h;
				}
			}
			it = Cam_Cycle(it);
		}
		goto install_target;
	}

	if (rnd < 2.0f)                                       // 0x1009216c = 2.0
	{
		// Pick best-score client (negative score clamped to zero).
		it = Cam_Cycle(NULL);
		while (it != NULL)
		{
			if (it != cam->lastent && it->deadflag == 0)
			{
				float s = (float)it->client->resp.score;  // client + 0xda8
				if (s < 0.0f) s = 0.0f;
				if (best < s)
				{
					chosen = it;
					best   = s;
				}
			}
			it = Cam_Cycle(it);
		}
		goto install_target;
	}

	// Uniform random pick: count valid candidates, then pick the n-th.
	best = 0.0f;
	it = Cam_Cycle(NULL);
	while (it != NULL)
	{
		if (it != cam->lastent && it->deadflag == 0)
			best += 1.0f;                                 // 0x10092178 = 1.0
		it = Cam_Cycle(it);
	}
	if (best <= 0.0f)
		goto install_target;

	/* g_local.h's random(), as Mr Elusive wrote it: gcc's -ffast-math fold of
	   best * ((rand() & 0x7fff) / 32767.0f) gives the .so's exact
	   `fld <1/32767>; fmul best; and eax,0x7fff; push eax; fimul [esp]`,
	   masking the rand result only after the reciprocal multiplied best. */
	best *= random();

	it = NULL;
	for (;;)
	{
		it = Cam_Cycle(it);
		if (it == cam->lastent)         continue;
		if (it->deadflag != 0)          continue;
		best -= 1.0f;                                     // 0x10092178 = 1.0
		if (best > 0.0f)                continue;
		break;
	}
	if (it != NULL)
		chosen = it;

install_target:
	if (chosen != ent)
	{
		// Found a (different) target: commit and bail.
		Cam_EnterFlyByMode(ent, chosen, ucmd);
		cam->delay       = level.time + 10.0f;            // 0x10092168 = 10
		cam->search_time = level.time + 60.0f;            // 0x10092230 = 60
		return;
	}

	// No different target: tighten viewtarget by tracing from cam->dest
	// toward cam->viewtarget through walls/glass; the trace endpoint
	// becomes the new viewtarget.
	cam->ent = ent;
	tr = gi.trace(cam->dest, NULL, NULL,
	              cam->viewtarget, ent, OBSERVER_TRACE_MASK);
	cam->viewtarget[0] = tr.endpos[0];
	cam->viewtarget[1] = tr.endpos[1];
	cam->viewtarget[2] = tr.endpos[2];

	// Random pitch in [-20,+20] and random yaw in [0,360).
	/* No `angle_pitch` temp: the reference's group-2 block has eight slots and
	   ours had nine, the extra one referenced exactly twice -- assigned once and
	   read once, which is all this variable ever was. */
	angles[PITCH] = (float)(rand() & 0x7FFF) / 32767.0f * 40.0f - 20.0f;
	/* Chained with angles[YAW] INNERMOST: ref stores it first and keeps the
	   value on the x87 stack (`fst [esp+0xa8]`), sets angles[ROLL], and only
	   then pops into yaw_random.  Assigning yaw_random first instead makes gcc
	   reload the slot and copy it as an integer dword. */
	angles[YAW]   = (float)(rand() & 0x7FFF) / 32767.0f * 360.0f;
	angles[ROLL]  = 0.0f;
	/* Read back rather than chained: ref keeps the value on the x87 stack across
	   the integer store to angles[ROLL] and pops it here
	   (`fst [esp+0xa8]; mov [esp+0xac],0x0; fstp yaw_random`). */
	yaw_random    = angles[YAW];

	// If cam->dest already equals ent->s.origin, yaw_to_target stays 0;
	// otherwise compute yaw from cam->dest toward ent.
	if (VectorCompare(&ent->s.origin[0], cam->dest) == 0)
	{
		delta[0] = ent->s.origin[0] - cam->dest[0];
		delta[1] = ent->s.origin[1] - cam->dest[1];
		delta[2] = ent->s.origin[2] - cam->dest[2];
		vectoangles(delta, out_ang);
		delta[0] = out_ang[0];           // overwrite delta with the angles result
		delta[1] = out_ang[1];
		delta[2] = out_ang[2];
		yaw_to_target = delta[1];
	}
	else
	{
		yaw_to_target = 0.0f;
	}

	yaw_random = anglemod(yaw_random);

	// Convert the random pitch/yaw to a forward vector scaled by 2000.
	AngleVectors(angles, fwd, NULL, NULL);
	VectorScale(fwd, 2000.0f, angles);   // [-0x70..-0x68] reused as fwd*2000

	// best := |cam->dest2 - cam->dest|  (initial "best distance to beat").
	diff[0] = cam->dest2[0] - cam->dest[0];
	diff[1] = cam->dest2[1] - cam->dest[1];
	diff[2] = cam->dest2[2] - cam->dest[2];
	best = VectorLength(diff);

	/* --- First trace candidate -------------------------------------------
	   disasm @ 0x1007a771..0x1007a78f computes
	     fabs(anglemod(yaw_random - yaw_to_target)) > 60.0
	   (call 0x10087ba7 is _CIfabs -- the function clears the sign bit at
	   0x10087c3f -- *not* sqrt as earlier drafts assumed; the block is
	   therefore LIVE for most yaw deltas, not dead.  Constant at
	   0x100925e8 is 60.0 (double).  Since anglemod's output is already
	   non-negative the fabs() is identity here, but kept verbatim.

	   The store to `angles` (= [-0x70..-0x68]) is intentional: this slot
	   was fwd*2000 going in but is reused as a *cross-block scratch*
	   read by the second trace candidate as the subtraction operand.
	   The first trace itself targets &delta (still the vectoangles
	   output at this point, treated as raw coordinates) -- this matches
	   disasm @ 0x1007a7c2 (lea eax,[ebp-0x64]; push eax) exactly. */
	if (fabs((double)anglemod(yaw_random - yaw_to_target)) > 60.0)
	{
		/* cam->dest first: ref is `fld [ecx+0x40]; fadd [esp+0x7c]`. */
		angles[0] = cam->dest[0] + delta[0];
		angles[1] = cam->dest[1] + delta[1];
		angles[2] = cam->dest[2] + delta[2];
		tr = gi.trace(cam->dest, NULL, NULL,
		              delta, ent, OBSERVER_TRACE_MASK);
		delta[0] = tr.endpos[0];
		delta[1] = tr.endpos[1];
		delta[2] = tr.endpos[2];
		diff[0] = delta[0] - cam->dest[0];
		diff[1] = delta[1] - cam->dest[1];
		diff[2] = delta[2] - cam->dest[2];
		{
			float len = VectorLength(diff);
			if (best < len)
			{
				cam->dest2[0] = delta[0];
				cam->dest2[1] = delta[1];
				cam->dest2[2] = delta[2];
				best = len;
			}
		}
	}

	/* --- Second trace candidate (yaw + 180) -------------------------------
	   disasm @ 0x1007a8af..0x1007a8d0 stores delta = cam->dest - angles.
	   `angles` here holds either fwd*2000 (if block 1's guard failed) or
	   delta_old + cam->dest (if block 1 ran) -- block 1's side-effect is
	   load-bearing for this trace target. */
	/* Two statements: ref stores the sum back into yaw_random and then pushes
	   that slot (`fstp [esp+0x24]; push [esp+0x24]`), where a single nested call
	   pushes the rvalue straight off the x87 stack. */
	yaw_random = yaw_random + 180.0f;                 // 0x100922f0 = 180
	yaw_random = anglemod(yaw_random);
	if (fabs((double)anglemod(yaw_random - yaw_to_target)) > 60.0)
	{
		delta[0] = cam->dest[0] - angles[0];
		delta[1] = cam->dest[1] - angles[1];
		delta[2] = cam->dest[2] - angles[2];
		tr = gi.trace(cam->dest, NULL, NULL,
		              delta, ent, OBSERVER_TRACE_MASK);
		delta[0] = tr.endpos[0];
		delta[1] = tr.endpos[1];
		delta[2] = tr.endpos[2];
		diff[0] = delta[0] - cam->dest[0];
		diff[1] = delta[1] - cam->dest[1];
		diff[2] = delta[2] - cam->dest[2];
		{
			float len = VectorLength(diff);
			if (best < len)
			{
				cam->dest2[0] = delta[0];
				cam->dest2[1] = delta[1];
				cam->dest2[2] = delta[2];
				best = len;
			}
		}
	}

	// pause_time elapsed: bump it [3, 5] seconds and resync viewtarget.
	if (cam->pause_time < level.time)
	{
		cam->pause_time = level.time + 3.0f               // 0x10092140 = 3
		                + 2.0f * ((float)(rand() & 0x7FFF) / 32767.0f);
		cam->viewtarget[0] = cam->dest2[0];
		cam->viewtarget[1] = cam->dest2[1];
		cam->viewtarget[2] = cam->dest2[2];
	}

	// search_time elapsed: backtrack along viewtarget->dest by a random
	// distance, and trace from ent toward that new point.
	if (cam->search_time < level.time)
	{
		VectorSubtract(cam->dest, cam->viewtarget, angles);
		VectorNormalize(angles);
		VectorScale(angles, random() * 50 + 5, angles);

		angles[0] += cam->viewtarget[0];
		angles[1] += cam->viewtarget[1];
		angles[2] += cam->viewtarget[2];

		tr = gi.trace(ent->s.origin, NULL, NULL,
		              angles, ent, OBSERVER_TRACE_MASK);

		if (tr.fraction >= 1.0f)                          // 0x10092178 = 1.0
		{
			// Trace got all the way: commit the backtracked dest.
			cam->dest[0]  = angles[0];
			cam->dest[1]  = angles[1];
			cam->dest[2]  = angles[2];
			cam->dest2[0] = cam->viewtarget[0];
			cam->dest2[1] = cam->viewtarget[1];
			cam->dest2[2] = cam->viewtarget[2];
			cam->search_time = level.time + 8.0f          // 0x1009215c = 8
			                 + ((float)(rand() & 0x7FFF) / 32767.0f) * 5.0f;
		}
		else
		{
			// Trace blocked: pin dest2 to dest and retry soon.
			cam->dest2[0] = cam->dest[0];
			cam->dest2[1] = cam->dest[1];
			cam->dest2[2] = cam->dest[2];
			cam->search_time = level.time + 1.0f          // 0x10092178 = 1
			                 + ((float)(rand() & 0x7FFF) / 32767.0f);
		}
	}
} //end of the function Cam_IdleThink

//===========================================================================
// Cam_Think
//
// Per-frame state machine for autocam mode.  Reconstructed faithfully from
// gamex86.dll @ 0x1007abd5: a switch on cam->state that calls one of five
// state handlers followed by a uniform Cam_UpdateView(ent, speed, ucmd) helper.
// Unknown states reset cam->state to 0.
//===========================================================================
void Cam_Think(edict_t *ent, usercmd_t *ucmd)
{
	camera_t *cam;

	cam = &ent->client->camera;
	if (cam->state == 0)
	{
		Cam_IdleThink(ent, ucmd);
		Cam_UpdateView(ent, 1.0, ucmd);
	} //end if
	else if (cam->state == 2)
	{
		Cam_FlyByThink(ent, ucmd);
		Cam_UpdateView(ent, 0.0, ucmd);
	} //end else if
	else if (cam->state == 3)
	{
		Cam_FollowThink(ent, ucmd);
		Cam_UpdateView(ent, 0.0, ucmd);
	} //end else if
	else if (cam->state == 7)
	{
		Cam_DeathThink(ent, ucmd);
		Cam_UpdateView(ent, 100.0, ucmd);
	} //end else if
	else if (cam->state == 1)
	{
		Cam_FixedThink(ent, ucmd);
		Cam_UpdateView(ent, 0.0, ucmd);
	} //end else if
	else
	{
		cam->state = 0;
	} //end else
} //end of the function Cam_Think

//===========================================================================
// sub_1007ace3 -- ChangeChaseCamOffset
//
// The input and camera angles live in two vec3_t's, `diff` declared first:
// gamei386.so stores all three differences to contiguous slots above the
// three angles and computes the ROLL difference in full although nothing
// reads it (gcc 2.7 never drops an array store), and gcc's inliner only
// copies an AngleDifference argument into its own register when it is not a
// plain variable, which is the load-once shape the .so has.
//
// FAITHFUL 1999 BUG, do NOT "fix": the second threshold of each pair tests
// the SAME constant, `> 3` then `< 3` (and `> 10` then `< 10`), not -3 / -10,
// so the offsets move whenever the difference is not exactly 3 (or 10).  Both
// originals prove it: gamex86.dll compares against one .rdata constant twice
// (ds:0x10092140 for 3, ds:0x10092168 for 10) where the +-20 and +-48 clamps
// use two, and gamei386.so keeps the one 3.0 on the x87 stack for both tests.
//===========================================================================
void ChangeChaseCamOffset(edict_t *ent, usercmd_t *ucmd)
{
	camera_t *cam;
	cvar_t *m_pitch;
	vec3_t diff, angles;
	float m_pitch_val, inv_m_pitch;

	cam = &ent->client->camera;
	angles[PITCH] = anglemod(SHORT2ANGLE(ucmd->angles[PITCH]) + SHORT2ANGLE(ent->client->ps.pmove.delta_angles[PITCH]));
	angles[YAW] = anglemod(SHORT2ANGLE(ucmd->angles[YAW]) + SHORT2ANGLE(ent->client->ps.pmove.delta_angles[YAW]));
	angles[ROLL] = anglemod(SHORT2ANGLE(ucmd->angles[ROLL]) + SHORT2ANGLE(ent->client->ps.pmove.delta_angles[ROLL]));
	cam->angles[0] = anglemod(cam->angles[0]);
	cam->angles[1] = anglemod(cam->angles[1]);
	cam->angles[2] = anglemod(cam->angles[2]);
	diff[PITCH] = AngleDifference(angles[PITCH], cam->angles[PITCH]);
	diff[YAW]   = AngleDifference(angles[YAW],   cam->angles[YAW]);
	diff[ROLL]  = AngleDifference(angles[ROLL],  cam->angles[ROLL]);

	// Clamp diff[PITCH] into [-20, 20].
	if (diff[PITCH] >  20.0f) diff[PITCH] =  20.0f;
	else if (diff[PITCH] < -20.0f) diff[PITCH] = -20.0f;

	// inv_m_pitch = -0.022 / m_pitch  (so chaseoffset moves proportional to m_pitch)
	m_pitch = gi.cvar("m_pitch", 0, 0);
	m_pitch_val = -0.022f;
	if (m_pitch && m_pitch->value != 0.0f)
		m_pitch_val = m_pitch->value;
	inv_m_pitch = -0.022 / m_pitch_val;

	// chaseoffset[ROLL] is the "vertical" component (offset behind the player).
	// Adjust by  +/- 0.5 * diff[PITCH] * inv_m_pitch depending on attack-button
	// state (ucmd->buttons bit 0 == BUTTON_ATTACK).
	if (ucmd->buttons & 1)
	{
		if (diff[PITCH] >  3.0f)
			cam->chaseoffset[ROLL] += inv_m_pitch * diff[PITCH] * 0.5;
		else if (diff[PITCH] < 3.0f)
			cam->chaseoffset[ROLL] += inv_m_pitch * diff[PITCH] * 0.5;
	}
	else
	{
		if (diff[PITCH] >  3.0f)
			cam->chaseoffset[PITCH] -= inv_m_pitch * diff[PITCH] * 0.5;
		else if (diff[PITCH] < 3.0f)
			cam->chaseoffset[PITCH] -= inv_m_pitch * diff[PITCH] * 0.5;
	}

	// chaseoffset[YAW] += diff[YAW] * 0.2 ; wrap.  (disasm writes cam+0x2c
	// which is chaseoffset[1]=[YAW].)  Mirrors the pitch block: two separate
	// branches, > 10 and < 10 (sic, see above), each duplicating the body.
	if (diff[YAW] >  10.0f)
	{
		cam->chaseoffset[YAW] += diff[YAW] * 0.2;
		cam->chaseoffset[YAW]  = anglemod(cam->chaseoffset[YAW]);
	}
	else if (diff[YAW] < 10.0f)
	{
		cam->chaseoffset[YAW] += diff[YAW] * 0.2;
		cam->chaseoffset[YAW]  = anglemod(cam->chaseoffset[YAW]);
	}

	// chaseoffset[PITCH] clamp into [32, 100].  (disasm writes cam+0x28
	// which is chaseoffset[0]=[PITCH])
	if (cam->chaseoffset[PITCH] >  100.0f) cam->chaseoffset[PITCH] = 100.0f;
	else if (cam->chaseoffset[PITCH] < 32.0f) cam->chaseoffset[PITCH] = 32.0f;

	// chaseoffset[ROLL] clamp into [-48, 48].
	if (cam->chaseoffset[ROLL] >  48.0f) cam->chaseoffset[ROLL] =  48.0f;
	else if (cam->chaseoffset[ROLL] < -48.0f) cam->chaseoffset[ROLL] = -48.0f;
} //end of the function ChangeChaseCamOffset

//===========================================================================
// UpdateChaseCamera
//
// Per-frame chase-cam logic.  Reconstructed faithfully from gamex86.dll @
// 0x1007b05d.  Steps:
//   1. If !CAMFL_FIXED, let user input adjust the camera (ChangeChaseCamOffset).
//   2. Snap (CAMFL_NOSMOOTHING) or LerpAngles cam->ent_angles toward the
//      chase target's v_angle, then refresh cam->lasttime.
//   3. Compute a yaw-offset forward vector (with the pitch correction
//      forward[2] = -(forward[0]*up[0] + forward[1]*up[1]) / up[2]),
//      normalise it, and build a 'dir' vector of length -chaseoffset[PITCH]
//      offset by chaseoffset[ROLL] on Z.
//   4. cam->origin = target.origin + chaseoffset.
//   5. Trace from cam->origin out by 'dir' through MASK_OPAQUE, ignoring
//      the chase target, then push the endpoint 5 units further along
//      forward (so the camera does not clip into the wall it hit).
//   6. Build cmdangles from ucmd->angles via SHORT2ANGLE and hand off to
//      SetClientView which writes the final angles back; copy those
//      into cam->angles.
//===========================================================================
void UpdateChaseCamera(edict_t *ent, usercmd_t *ucmd)
{
	camera_t *cam;
	/* angles, up, horizfwd, forward, dir, end, cmdangles -- slotmap pairs ref
	   0xa0/0x94/0x88/0x7c/0x70/0x64/0x58 to exactly that sequence; dir, up and
	   forward were permuted among themselves. */
	vec3_t   angles, up, horizfwd, forward, dir, end, cmdangles;
	trace_t  tr;

	if (!(ent->client->camera.flags & CAMFL_FIXED))
		ChangeChaseCamOffset(ent, ucmd);

	cam = &ent->client->camera;
	if (cam->flags & CAMFL_NOSMOOTHING)
	{
		VectorCopy(cam->ent->client->v_angle, cam->ent_angles);
	} //end if
	else
	{
		/* disasm @ 0x1007b0e4: call ChangeCameraAnglesSmooth (sub_10077e29) with
		   args (cam->ent_angles, target->v_angle, level.time-lasttime, 1.0). */
		ChangeCameraAnglesSmooth(cam->ent_angles,
		             cam->ent->client->v_angle,
		             level.time - cam->lasttime,
		             1.0f);
	} //end else
	cam->lasttime = level.time;

	// Build working angles from ent_angles, snag the 'up' vector
	VectorCopy(cam->ent_angles, angles);
	AngleVectors(angles, NULL, NULL, up);

	// Apply yaw chase offset, kill pitch, recompute forward
	angles[YAW] = anglemod(angles[YAW] + cam->chaseoffset[YAW]);
	angles[PITCH] = 0;
	AngleVectors(angles, forward, NULL, NULL);

	// Pitch correction: build a horizontal forward, re-tilted so it's
	// perpendicular to up.  Original keeps `forward` unmodified and uses
	// an intermediate vec3.
	horizfwd[0] = forward[0];
	horizfwd[1] = forward[1];
	horizfwd[2] = (horizfwd[0] * up[0] + horizfwd[1] * up[1]) * -1.0 / up[2];
	VectorNormalize(horizfwd);

	// dir = horizfwd * (-chaseoffset[PITCH]); dir[2] += chaseoffset[ROLL]
	VectorScale(horizfwd, -cam->chaseoffset[PITCH], dir);
	dir[2] += cam->chaseoffset[ROLL];

	// cam->origin = client viewoffset + target.origin
	cam->origin[0] = cam->ent->client->ps.viewoffset[0] + cam->ent->s.origin[0];
	cam->origin[1] = cam->ent->client->ps.viewoffset[1] + cam->ent->s.origin[1];
	cam->origin[2] = cam->ent->client->ps.viewoffset[2] + cam->ent->s.origin[2];

	// end = cam->origin + dir
	end[0] = cam->origin[0] + dir[0];
	end[1] = cam->origin[1] + dir[1];
	end[2] = cam->origin[2] + dir[2];

	// Trace from cam->origin to end, ignoring the chase target itself.
	/* vec3_origin here, NOT NULL: this one trace really does pass the address of
	   the zero vector -- swapping it for NULL takes this row off MATCH.  The
	   other four traces in this file push two immediate zeroes. */
	tr = gi.trace(cam->origin, vec3_origin, vec3_origin, end, cam->ent, MASK_OPAQUE);

	// Push the endpoint 5 units further along the (re-normalised)
	// horizontal forward so the camera doesn't sit flush against the wall.
	vectoangles(horizfwd, angles);
	VectorScale(horizfwd, 5.0, horizfwd);
	end[0] = tr.endpos[0] + horizfwd[0];
	end[1] = tr.endpos[1] + horizfwd[1];
	end[2] = tr.endpos[2] + horizfwd[2];

	// Convert ucmd->angles (short) -> cmdangles (float) via SHORT2ANGLE.
	cmdangles[0] = SHORT2ANGLE(ucmd->angles[0]);
	cmdangles[1] = SHORT2ANGLE(ucmd->angles[1]);
	cmdangles[2] = SHORT2ANGLE(ucmd->angles[2]);

	// Final placement -- writes resulting angles back through 'angles'.
	SetClientView(ent, end, angles, cmdangles);

	// cam->angles = angles  (the values written by SetClientView)
	VectorCopy(angles, cam->angles);
} //end of the function UpdateChaseCamera

//===========================================================================
// UpdateEyeCamera                                        sub_1007b363
//
// Dead code in the shipping DLL -- the byte pattern of its address
// (63 b3 07 10) appears nowhere else in gamex86.dll, so this function is
// never reached.  Reconstructed here line-by-line from the disassembly
// for archival completeness; kept static so the link is silent if it
// stays unreferenced.  The shape is a "fixed chase camera" variant: it
// snaps/lerps the cam's view to the target's v_angle, then pushes the
// camera forward 15 units along the horizontal view vector before
// finalising via SetClientView.
//===========================================================================
/* NAME ASSIGNED BY ELIMINATION, not by content -- weaker than the other 25
 * names recovered from gamei386.so in the same pass, and flagged here so a
 * later session does not read it as verified.
 *
 * xmatch.py found no shared string literal and no shared call target for this
 * pair, and the two bodies differ in size by more than half (ours 436 B, real 603 B).  What
 * it has going for it is that after the 25 content-proven renames exactly
 * three real rows and three of ours were left, and this is one of those three.
 *
 * The likeliest reason the bodies diverge: gamei386.so is the AUGUST 2 1999
 * build and gamex86.dll -- which this reconstruction is derived from -- is
 * JULY 18, and the observer path is one of the places the two builds are known
 * to differ.  So the NAME is probably right and the BODY is probably the older
 * revision.  Re-check with xmatch.py before treating the diff as a defect. */
void UpdateEyeCamera(edict_t *ent, usercmd_t *ucmd)
{
	camera_t *cam;
	vec3_t forward;
	vec3_t cmdangles;

	cam = &ent->client->camera;
	if (cam->flags & CAMFL_NOSMOOTHING)
	{
		cam->ent_angles[0] = cam->ent->client->v_angle[0];
		cam->ent_angles[1] = cam->ent->client->v_angle[1];
		cam->ent_angles[2] = cam->ent->client->v_angle[2];
	} //end if
	else
	{
		ChangeCameraAnglesSmooth(cam->ent_angles, cam->ent->client->v_angle,
		             level.time - cam->lasttime, 1.0f);
	} //end else
	cam->lasttime = level.time;

	/* The target's EYE position, not cam->chaseoffset: ref loads cam->ent into
	   edx, cam->ent->client into eax and reads [eax+0x28] -- player_state_t's
	   viewoffset (pmove_state_t is 0x1c, viewangles 0x1c, viewoffset 0x28) --
	   then adds cam->ent->s.origin.  Cam_GetFollowSpot already has this idiom.
	   Behavioural: we were adding the chase-cam offset to the eye camera. */
	cam->origin[0] = cam->ent->client->ps.viewoffset[0] + cam->ent->s.origin[0];
	cam->origin[1] = cam->ent->client->ps.viewoffset[1] + cam->ent->s.origin[1];
	cam->origin[2] = cam->ent->client->ps.viewoffset[2] + cam->ent->s.origin[2];

	AngleVectors(cam->ent_angles, forward, NULL, NULL);
	forward[2] = 0;
	VectorNormalize(forward);   // length discarded
	forward[0] *= 15.0f;
	forward[1] *= 15.0f;
	// forward[2] stays 0 (multiplied by 0 in disasm via fld [ebp-8])

	cam->origin[0] += forward[0];
	cam->origin[1] += forward[1];
	cam->origin[2] += forward[2];

	// pre-copy cam->ent_angles into cam->angles (this is overwritten again
	// after SetClientView but the original does both writes verbatim)
	cam->angles[0] = cam->ent_angles[0];
	cam->angles[1] = cam->ent_angles[1];
	cam->angles[2] = cam->ent_angles[2];

	cmdangles[0] = SHORT2ANGLE(ucmd->angles[0]);
	cmdangles[1] = SHORT2ANGLE(ucmd->angles[1]);
	cmdangles[2] = SHORT2ANGLE(ucmd->angles[2]);

	SetClientView(ent, cam->origin, cam->ent_angles, cmdangles);

	cam->angles[0] = cam->ent_angles[0];
	cam->angles[1] = cam->ent_angles[1];
	cam->angles[2] = cam->ent_angles[2];
} //end of the function UpdateEyeCamera

//===========================================================================
// CheckValidCamera                                          sub_1007b56a
//
// Snapshot the observer's own viewangles+origin into cam->clientangles/
// cam->clientorigin while the camera is targeting the observer himself,
// or hand the camera back to him (calling SetClientView) when the
// current target is gone / no longer an observer.
//===========================================================================
/* NAME ASSIGNED BY ELIMINATION, not by content -- weaker than the other 25
 * names recovered from gamei386.so in the same pass, and flagged here so a
 * later session does not read it as verified.
 *
 * xmatch.py found no shared string literal and no shared call target for this
 * pair, and the two bodies differ in size by more than half (ours 163 B, real 331 B).  What
 * it has going for it is that after the 25 content-proven renames exactly
 * three real rows and three of ours were left, and this is one of those three.
 *
 * The likeliest reason the bodies diverge: gamei386.so is the AUGUST 2 1999
 * build and gamex86.dll -- which this reconstruction is derived from -- is
 * JULY 18, and the observer path is one of the places the two builds are known
 * to differ.  So the NAME is probably right and the BODY is probably the older
 * revision.  Re-check with xmatch.py before treating the diff as a defect. */
void CheckValidCamera(edict_t *ent)
{
	camera_t *cam;

	cam = &ent->client->camera;
	if (cam->ent == ent)
	{
		cam->clientangles[0] = ent->client->ps.viewangles[0];
		cam->clientangles[1] = ent->client->ps.viewangles[1];
		cam->clientangles[2] = ent->client->ps.viewangles[2];
		cam->clientorigin[0] = ent->s.origin[0];
		cam->clientorigin[1] = ent->s.origin[1];
		cam->clientorigin[2] = ent->s.origin[2];
	} //end if
	else if (!cam->ent || !cam->ent->inuse || (cam->ent->flags & FL_OBSERVER))
	{
		ent->client->camera.ent = ent;
		SetClientView(ent,
			cam->clientorigin,
			cam->clientangles,
			/* sub_10077f7d cmdangles arg -- gclient_t resp.cmd_angles at +0xdc8;
			   the most-recent received cmd-angles vector. */
			ent->client->resp.cmd_angles);
	} //end else if
	/* else: current target is a live non-observer player; keep tracking it */
} //end of the function CheckValidCamera

//===========================================================================
// ClientCycleCamera                                          sub_1007b652
//
// "cyclecam" command: walk through the clients starting at the slot
// after the current target and lock onto the first in-use, non-observer
// one.  Falls back to ClientToggleObserver / CheckValidCamera and
// refuses to do anything while CAMFL_AUTOCAM is set.
//===========================================================================
void ClientCycleCamera(edict_t *ent)
{
	edict_t *cand;
	int i;
	int idx;

	if (!(ent->flags & FL_OBSERVER))
		ClientToggleObserver(ent);

	if (ent->client->camera.flags & CAMFL_AUTOCAM)
	{
		gi.cprintf(ent, PRINT_HIGH, "cyclecam not available in autocam mode\n");
		return;
	} //end if

	CheckValidCamera(ent);

	idx = (ent->client->camera.ent - g_edicts) - 1;
	for (i = 0; i < game.maxclients; i++)
	{
		idx++;
		if (idx >= game.maxclients) idx = 0;
		cand = g_edicts + 1 + idx;
		if (cand->inuse)
		{
			if (!(cand->flags & FL_OBSERVER) || cand == ent)
			{
				ent->client->camera.ent = cand;
				break;
			}
		}
	} //end for

	if (i == game.maxclients)
		gi.cprintf(ent, PRINT_HIGH, "no valid client found to observe\n");

	if (ent->client->camera.ent == ent)
		SetClientView(ent,
			ent->client->camera.clientorigin,
			ent->client->camera.clientangles,
			ent->client->resp.cmd_angles);
} //end of the function ClientCycleCamera

//===========================================================================
// ClientSetCamera                                            sub_1007b7bd
//
// "setcam <name>" command: target an in-use client whose pers.netname
// matches the first command argument.  Behaves like ClientCycleCamera
// for the prelude (FL_OBSERVER auto-enter, CAMFL_AUTOCAM refusal,
// CheckValidCamera snapshot).
//===========================================================================
void ClientSetCamera(edict_t *ent)
{
	char *name;
	int i;
	edict_t *cand;

	if (!(ent->flags & FL_OBSERVER))
		ClientToggleObserver(ent);

	if (ent->client->camera.flags & CAMFL_AUTOCAM)
	{
		gi.cprintf(ent, PRINT_HIGH, "setcam not available in autocam mode\n");
		return;
	} //end if

	if (gi.argc() <= 1)
	{
		gi.cprintf(ent, PRINT_HIGH, "usage: setcam <client name>\n");
		return;
	} //end if

	CheckValidCamera(ent);
	name = gi.argv(1);

	for (i = 0; i < game.maxclients; i++)
	{
		cand = g_edicts + 1 + i;
		if (!cand->inuse) continue;
		/* Q_strcasecmp, not stricmp: gamei386.so calls q_shared.c's Q_strcasecmp here
		 * (through the PLT), where a stricmp would be linux-i386.mak's strcasecmp. */
		if (!Q_strcasecmp(cand->client->pers.netname, name))
		{
			ent->client->camera.ent = cand;
			break;
		} //end if
	} //end for

	if (i == game.maxclients)
		gi.cprintf(ent, PRINT_HIGH, "no valid client found with the name %s\n", name);

	if (ent->client->camera.ent == ent)
		SetClientView(ent,
			ent->client->camera.clientorigin,
			ent->client->camera.clientangles,
			ent->client->resp.cmd_angles);
} //end of the function ClientSetCamera

//===========================================================================
// DoObserver                                                 sub_1007b926
//
// Called from ClientThink per frame.  Returns 0 if the entity is not an
// observer (caller continues normal player think).  When observing, runs
// the autocam/chasecam jump-button retarget, dispatches to
// Cam_Think / UpdateChaseCamera, and either zeros the usercmd
// for a spectator frame or returns 1 with PM_SPECTATOR set when the
// camera has been handed back to the observer himself.
//===========================================================================
int DoObserver(edict_t *ent, usercmd_t *ucmd)
{
	edict_t *next;

	if (!(ent->flags & FL_OBSERVER)) return 0;
	if (!ent->client) return 0;

	if (!(ent->client->camera.flags & CAMFL_AUTOCAM))
		CheckValidCamera(ent);

	// jump button (upmove > 100) cycles the target / chase camera, with a
	// 0.5s debounce held in cam->lastcycle so a held key doesn't tear
	// through the client list in one frame (qword 0.5 at gamex86.dll
	// 0x10092138, fsub'd from level.time at sub_1007b926+0x6f)
	/* lastcycle on the LEFT: ref materialises `ent->client` before the FP
	   subtraction, which is the evaluation order for the left-hand operand. */
	if (ucmd->upmove > 100 && ent->client->camera.lastcycle < level.time - 0.5)
	{
		ent->client->camera.lastcycle = level.time;
		if (ent->client->camera.flags & CAMFL_AUTOCAM)
		{
			next = Cam_Cycle(ent->client->camera.ent);
			if (!next || next == ent)
				next = Cam_Cycle(next);
			if (next && next != ent)
				Cam_EnterFlyByMode(ent, next, ucmd);
		} //end if
		else if (ent->client->camera.flags & CAMFL_CHASECAM)
		{
			ClientCycleCamera(ent);
		} //end else if
	} //end if

	ent->client->ps.gunindex = 0;

	/* The SPECTATOR arm is the fallthrough in both originals -- ref's last test
	   is `je` into `movetype=1; pm_type=1; buttons &= ~3; return 1` -- so the
	   condition is written the other way round and the two arms are swapped. */
	if (!(ent->client->camera.flags & (CAMFL_AUTOCAM | CAMFL_CHASECAM)) ||
	    (ent->client->camera.ent == ent && (ent->client->camera.flags & CAMFL_CHASECAM)))
	{
		// plain spectator (no autocam/chasecam, or chasecam targeting self)
		ent->movetype = MOVETYPE_NOCLIP;
		ent->client->ps.pmove.pm_type = PM_SPECTATOR;
		ucmd->buttons &= ~(BUTTON_ATTACK | BUTTON_USE);
		return 1;
	} //end if
	else
	{
		// tracking some other client: drive the camera and silence input
		ent->client->ps.pmove.pm_type = PM_DEAD;
		if (ent->client->camera.flags & CAMFL_AUTOCAM)
			Cam_Think(ent, ucmd);
		else if (ent->client->camera.flags & CAMFL_CHASECAM)
			UpdateChaseCamera(ent, ucmd);
		ucmd->buttons &= ~(BUTTON_ATTACK | BUTTON_USE);
		ucmd->forwardmove = 0;
		ucmd->sidemove = 0;
		ucmd->upmove = 0;
		ent->client->ps.pmove.gravity = 0;
		return 1;
	} //end else
} //end of the function DoObserver

//===========================================================================
// ClientToggleObserver                                       sub_1007bb4c
//
// "observer" command toggle.  When leaving observer mode the original
// first checks the CTF cvar and, if enabled, opens the team/join menu
// (sub_10018fee = CTFOpenJoinMenu).  Otherwise it checks the Rocket
// Arena cvar and refuses to leave when RA is active; only the plain
// non-CTF, non-RA path directly restores the player entity.
//===========================================================================
void ClientToggleObserver(edict_t *ent)
{
	if (ent->flags & FL_OBSERVER)
	{
		// ---- leaving observer mode ----
		if (ctf->value)
		{
			CTFOpenJoinMenu(ent);
			return;
		} //end if
		if (ra->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "can't leave observer mode\n");
			return;
		} //end if
		ent->classname = "player";
		ent->flags &= ~FL_OBSERVER;
		ent->solid = SOLID_BBOX;
		ent->takedamage = DAMAGE_AIM;
		ent->svflags &= ~SVF_NOCLIENT;
		PutClientInServer(ent);
		// teleport-style spawn-in effect
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
		ent->movetype = MOVETYPE_WALK;
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent - g_edicts);
		gi.WriteByte(MZ_LOGIN);
		gi.multicast(ent->s.origin, MULTICAST_PVS);
		gi.bprintf(PRINT_HIGH, "%s left observer mode\n", ent->client->pers.netname);
	} //end if
	else
	{
		// ---- entering observer mode ----
		ent->classname = "observer";
		ent->flags |= FL_OBSERVER;
		ent->solid = SOLID_NOT;
		ent->takedamage = DAMAGE_NO;
		ent->movetype = MOVETYPE_NOCLIP;
		ent->health = 100;
		ent->client->ps.stats[STAT_HEALTH] = ent->health;
		ent->svflags |= SVF_NOCLIENT;
		/* No deadflag/viewangles fixup here: neither original has it.  The .so
		   never touches edict+0x1ec or client+0xe8c in this function, and
		   gamex86.dll's 0x1007bb4c body (161 lines, four compares) references
		   neither offset either.  It was invented -- p_observer.c has no shipped
		   source counterpart, so everything in it is reconstruction. */
		ent->client->camera.ent = ent;
		CheckValidCamera(ent);
		if (ctf->value)
			ent->client->resp.ctf_team = 0;
		gi.bprintf(PRINT_HIGH, "%s entered observer mode\n", ent->client->pers.netname);
	} //end else
} //end of the function ClientToggleObserver

//===========================================================================
// ClientToggleCameraFixed                                    sub_1007bda4
//
// "camfixed" command: flip CAMFL_FIXED and tell the player whether the
// chase-cam offset is now "fixed" or "variable".
//===========================================================================
void ClientToggleCameraFixed(edict_t *ent)
{
	ent->client->camera.flags ^= CAMFL_FIXED;
	gi.cprintf(ent, PRINT_HIGH, "camera offsets are ");
	if (ent->client->camera.flags & CAMFL_FIXED)
		gi.cprintf(ent, PRINT_HIGH, "fixed\n");
	else
		gi.cprintf(ent, PRINT_HIGH, "variable\n");
} //end of the function ClientToggleCameraFixed

// --- sub_10079acf -----------------------------------------------------------
// SetAutocamTarget: bind the camera to a new target, compute a good vantage
// point via the flyby helper Cam_GetFlyBySpot, and either commit to
// state 2 (flyby) or fall back to abort.
//===========================================================================
qboolean Cam_GetFlyBySpot(edict_t *ent, edict_t *target, vec3_t out_pos); /* sub_10078f4a */

//===========================================================================
// ClientToggleCameraName                                     sub_1007be15
//
// "camname" command: flip CAMFL_NAME (whether the tracked player's
// name is shown on the chase camera).
//===========================================================================
void ClientToggleCameraName(edict_t *ent)
{
	ent->client->camera.flags ^= CAMFL_NAME;
	gi.cprintf(ent, PRINT_HIGH, "camera player names ");
	if (ent->client->camera.flags & CAMFL_NAME)
		gi.cprintf(ent, PRINT_HIGH, "on\n");
	else
		gi.cprintf(ent, PRINT_HIGH, "off\n");
} //end of the function ClientToggleCameraName

// NOTE: sub_10085904 in the shipping DLL is the q_shared.c VectorCompare
// statically linked into the .text segment.  We do not re-define it
// locally; callers below use the shared declaration from q_shared.h.

//===========================================================================
// ClientToggleAutoCam                                        sub_1007be86
//
// "autocam" command toggle.  When enabling, snapshot the observer's
// current position into cam->dest, fire AngleVectors() through the
// observer's v_angle to seed cam->dest+0xc..0x14 (a forward vector
// scaled by 100.0 -- the BE85b constant 0x42c80000 = 100.0f), point
// the camera at the observer himself and start the autocam state
// machine on a fresh frame.
//===========================================================================
void ClientToggleAutoCam(edict_t *ent)
{
	camera_t *cam;
	vec3_t forward;

	if (!(ent->flags & FL_OBSERVER))
		ClientToggleObserver(ent);

	ent->client->camera.flags ^= CAMFL_AUTOCAM;
	gi.cprintf(ent, PRINT_HIGH, "autocam ");
	if (ent->client->camera.flags & CAMFL_AUTOCAM)
	{
		cam = &ent->client->camera;
		cam->dest[0] = ent->s.origin[0];
		cam->dest[1] = ent->s.origin[1];
		cam->dest[2] = ent->s.origin[2];
		AngleVectors(ent->client->v_angle, forward, NULL, NULL);
		VectorMA(cam->dest, 100.0, forward, cam->viewtarget);
		cam->ent = ent;
		cam->pause_time = level.time;
		cam->delay = level.time;
		gi.cprintf(ent, PRINT_HIGH, "on\n");
	} //end if
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "off\n");
	} //end else
} //end of the function ClientToggleAutoCam

// --- sub_10078f4a -----------------------------------------------------------
// CameraFindFlybyPos: 6-tier search for a vantage point around the current
// camera target (cam->ent).  Each tier tries 4 candidate offset directions
// (combinations of forward/right/up basis vectors from the camera's
// cam->angles) and keeps the best (lowest score from Cam_TryFlyByVector).  If
// any tier finds a valid candidate (best < 1000) the remaining tiers are
// skipped.  Returns 1 and writes the chosen world position to 'out' on
// success; returns 0 if no tier found a position.
//
// The 'target' arg is present in the original signature (the caller in
// Cam_EnterFlyByMode passes it after assigning cam->ent = target) but is
// never read -- the disassembly only ever dereferences ent and the cam->ent
// it points at.
//
// Side-effect to be aware of: starting at tier 3 the helper passes the
// running forward/right basis vectors *directly* to Cam_TryFlyByVector, which
// normalises and re-scales them to 700 units.  Tiers 4-5 then use those
// post-scoring values when building their additive offsets.
//===========================================================================

//===========================================================================
// ClientToggleChaseCam                                       sub_1007bfae
//
// "chasecam" command: enter observer mode if necessary, then flip
// CAMFL_CHASECAM.
//===========================================================================
void ClientToggleChaseCam(edict_t *ent)
{
	if (!(ent->flags & FL_OBSERVER))
		ClientToggleObserver(ent);

	ent->client->camera.flags ^= CAMFL_CHASECAM;
	if (ent->client->camera.flags & CAMFL_CHASECAM)
		gi.cprintf(ent, PRINT_HIGH, "chasecam on\n");
	else
		gi.cprintf(ent, PRINT_HIGH, "chasecam off\n");
} //end of the function ClientToggleChaseCam

//===========================================================================
// ClientObserverHelp                                         sub_1007c02a
//
// "observerhelp" command: dump the observer command summary at
// PRINT_HIGH.  The original is a single gi.cprintf into one big
// string at 0x100c04f0; we keep the C-string-concatenation style.
//===========================================================================
void ClientObserverHelp(edict_t *ent)
{
	gi.cprintf(ent, PRINT_HIGH,
		"observer        toggles observer mode\n"
		"autocam         toggle auto targeting camera mode\n"
		"chasecam        toggle manually positioned chase cam\n"
		"cyclecam        cycles camera to next player\n"
		"setcam <name>   set camera to player with name\n"
		"camfixed        toggle chase camera offset fixed\n"
		"camname         toggle showing name of tracked player\n"
		"observerhelp    this help message\n");
} //end of the function ClientObserverHelp

//===========================================================================
// ClientObserverCmd
//
// returns true if cmd was an observer subcommand and was handled.
//===========================================================================
qboolean ClientObserverCmd(char *cmd, edict_t *ent)
{
	/* Q_stricmp throughout: all eight compares in gamei386.so call q_shared.c's
	 * Q_stricmp, not the strcasecmp that a plain stricmp becomes on Linux. */
	if (!Q_stricmp(cmd, "observer"))          ClientToggleObserver(ent);
	else if (!Q_stricmp(cmd, "autocam"))      ClientToggleAutoCam(ent);
	else if (!Q_stricmp(cmd, "chasecam"))     ClientToggleChaseCam(ent);
	else if (!Q_stricmp(cmd, "cyclecam"))     ClientCycleCamera(ent);
	else if (!Q_stricmp(cmd, "setcam"))       ClientSetCamera(ent);
	else if (!Q_stricmp(cmd, "camfixed"))     ClientToggleCameraFixed(ent);
	else if (!Q_stricmp(cmd, "camname"))      ClientToggleCameraName(ent);
	else if (!Q_stricmp(cmd, "observerhelp")) ClientObserverHelp(ent);
	else return false;
	return true;
} //end of the function ClientObserverCmd

#endif //OBSERVER
