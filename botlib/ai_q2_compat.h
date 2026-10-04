/*
 * ai_q2_compat.h -- Q2 compatibility header for the real Q3 bot AI source
 *
 * Included (as the FIRST include) by the 5 ported Q3 game-AI files that now
 * compile into botlib.so instead of being hand-reinvented in
 * be_interface_q2.c:
 *
 *   game_q3/ai_main.c   game_q3/ai_dmnet.c   game_q3/ai_dmq3.c
 *   game_q3/ai_team.c   game_q3/ai_chat.c
 *
 * This header supersedes game_q2/ai_chat_q2.h (which only ever had to cover
 * ai_chat.c, compiled alone into game.so). ai_chat.c now moves into
 * botlib.so alongside the other 4 files because it is called directly
 * (plain C calls, not through any exported table) by ai_dmnet.c/ai_dmq3.c --
 * see the plan's "Forced consequence" section.
 *
 * Design: reuse the REAL Q3 struct/constant headers wherever they already
 * exist and are already part of this project's botlib.so build (they are --
 * be_interface_q2.c/be_interface.c/be_aas_main.c/be_ai_*.c already include
 * this exact combination of game_q3 and botlib-internal headers today), rather
 * than hand-redefining reduced copies of Q3 structs as game_q2/ai_chat_q2.h
 * did before botlib.so had these in scope. Only what is GENUINELY missing
 * from this repo's snapshot of the Q3 source (bg_public.h was never carried
 * over; g_local.h cannot be, since it is game.so-shaped) is defined here.
 */

#ifndef AI_Q2_COMPAT_H
#define AI_Q2_COMPAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

/* ================================================================== */
/*  Real Q3 / botlib headers -- identical combination to the one      */
/*  already used by be_interface_q2.c / be_interface.c / be_ai_*.c.   */
/*  This is what gives us the REAL vec3_t, qboolean, playerState_t,   */
/*  usercmd_t, entityState_t, bsp_trace_t, bot_input_t, aas_entityinfo_t, */
/*  bot_goal_t, bot_moveresult_t, weaponinfo_t, bot_match_t, and the   */
/*  real AAS_, EA_, Bot* and Characteristic_* function prototypes -- with */
/*  zero redefinition risk, since this exact set already compiles     */
/*  cleanly together in this codebase.                                */
/* ================================================================== */

#include "../game_q3/q_shared.h"
#include "l_memory.h"
#include "l_libvar.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "aasfile.h"
#include "../game_q3/botlib.h"
#include "../game_q3/be_aas.h"
#include "be_aas_funcs.h"
#include "be_aas_def.h"
#include "be_interface.h"

#include "../game_q3/be_ea.h"
#include "be_ai_weight.h"
#include "../game_q3/be_ai_goal.h"
#include "../game_q3/be_ai_move.h"
#include "../game_q3/be_ai_weap.h"
#include "../game_q3/be_ai_chat.h"
#include "../game_q3/be_ai_char.h"
#include "../game_q3/be_ai_gen.h"

/* ================================================================== */
/*  Missing "bg_public.h"-style engine constants.                     */
/*  Real Q3 keeps these in game/bg_public.h, shared by cgame/game/ui.  */
/*  That file was never part of this repo's game_q3/ snapshot (only   */
/*  q_shared.h was carried over), so every constant below is          */
/*  genuinely absent, not a duplicate of something already in scope.  */
/*  Most values here only need to be distinct. The exceptions are the */
/*  ones real data flows into: PM_ and PMF_ (cur_ps, translated by    */
/*  Q2BotUpdateClient), EF_ and PW_ (aas_entityinfo_t flags and        */
/*  powerups, written by Q2BotUpdateEntity) and WP_ (Q2 weapon config  */
/*  numbers, see that block) -- those must keep their current values.  */
/* ================================================================== */

/* player movement types (real Q3 pmtype_t) */
#define PM_NORMAL		0
#define PM_NOCLIP		1
#define PM_SPECTATOR	2
#define PM_DEAD			3
#define PM_FREEZE		4
#define PM_INTERMISSION	5

/* playerState_t.stats[]/persistant[] indices actually referenced */
#define STAT_HEALTH			0
#define STAT_HOLDABLE_ITEM	1
#define STAT_WEAPONS		2
#define STAT_ARMOR			3

#define PERS_SCORE			0
#define PERS_HITS			1

/* entityState_t.eFlags bits actually referenced */
#define EF_DEAD				0x0001
#define EF_TELEPORT_BIT		0x0004
#define EF_FIRING			0x0010
#define EF_TALK				0x0020
#define EF_KAMIKAZE			0x2000

/* entityState_t.eType (real Q3 entityType_t) -- ET_EVENTS is the
 * sentinel: eType >= ET_EVENTS encodes a freestanding EV_* event. */
typedef enum {
	ET_GENERAL,
	ET_PLAYER,
	ET_ITEM,
	ET_MISSILE,
	ET_MOVER,
	ET_BEAM,
	ET_PORTAL,
	ET_SPEAKER,
	ET_PUSH_TRIGGER,
	ET_TELEPORT_TRIGGER,
	ET_INVISIBLE,
	ET_GRAPPLE,
	ET_TEAM,
	ET_EVENTS
} q2_entityType_t;

/* entity_event_t (real Q3) -- only need distinct values + the real
 * relative ordering around EV_EVENT_BITS/ET_EVENTS math. */
typedef enum {
	EV_NONE = 0,
	EV_FOOTSTEP,
	EV_FOOTSTEP_METAL,
	EV_FOOTSPLASH,
	EV_FOOTWADE,
	EV_SWIM,
	EV_STEP_4,
	EV_STEP_8,
	EV_STEP_12,
	EV_STEP_16,
	EV_FALL_SHORT,
	EV_FALL_MEDIUM,
	EV_FALL_FAR,
	EV_JUMP_PAD,
	EV_JUMP,
	EV_WATER_TOUCH,
	EV_WATER_LEAVE,
	EV_WATER_UNDER,
	EV_WATER_CLEAR,
	EV_ITEM_PICKUP,
	EV_GLOBAL_ITEM_PICKUP,
	EV_NOAMMO,
	EV_CHANGE_WEAPON,
	EV_FIRE_WEAPON,
	EV_USE_ITEM0,
	EV_USE_ITEM1,
	EV_USE_ITEM2,
	EV_USE_ITEM3,
	EV_USE_ITEM4,
	EV_USE_ITEM5,
	EV_USE_ITEM6,
	EV_USE_ITEM7,
	EV_USE_ITEM8,
	EV_USE_ITEM9,
	EV_USE_ITEM10,
	EV_USE_ITEM11,
	EV_USE_ITEM12,
	EV_USE_ITEM13,
	EV_USE_ITEM14,
	EV_USE_ITEM15,
	EV_ITEM_RESPAWN,
	EV_ITEM_POP,
	EV_PLAYER_TELEPORT_IN,
	EV_PLAYER_TELEPORT_OUT,
	EV_GRENADE_BOUNCE,
	EV_GENERAL_SOUND,
	EV_GLOBAL_SOUND,
	EV_GLOBAL_TEAM_SOUND,
	EV_BULLET_HIT_FLESH,
	EV_BULLET_HIT_WALL,
	EV_MISSILE_HIT,
	EV_MISSILE_MISS,
	EV_MISSILE_MISS_METAL,
	EV_RAILTRAIL,
	EV_SHOTGUN,
	EV_BULLET,
	EV_PAIN,
	EV_DEATH1,
	EV_DEATH2,
	EV_DEATH3,
	EV_OBITUARY,
	EV_POWERUP_QUAD,
	EV_POWERUP_BATTLESUIT,
	EV_POWERUP_REGEN,
	EV_GIB_PLAYER,
	EV_SCOREPLUM,
	EV_TAUNT
} q2_entity_event_t;
#define EV_EVENT_BIT1	0x00000100
#define EV_EVENT_BIT2	0x00000200
#define EV_EVENT_BITS	(EV_EVENT_BIT1|EV_EVENT_BIT2)

/* EV_GLOBAL_TEAM_SOUND eventParm values (real Q3 gameTeamSound_t subset) */
#define GTS_RED_CAPTURE			0
#define GTS_BLUE_CAPTURE		1
#define GTS_RED_RETURN			2
#define GTS_BLUE_RETURN			3
#define GTS_RED_TAKEN			4
#define GTS_BLUE_TAKEN			5

/* weapon_t subset actually referenced. Unlike the other constants in this
 * block these values are NOT arbitrary: the ported code compares them with
 * bs->weaponnum and weaponinfo_t.number, which come from the Q2 weapon
 * config (assets/botfiles/weapons.c: 1 Blaster, 2 Shotgun, 3 Super Shotgun,
 * 4 Machinegun, 5 Chaingun, 6 Grenades, 7 Grenade Launcher, 8 Rocket
 * Launcher, 9 HyperBlaster, 10 Railgun, 11 BFG10K, 18 Chainfist). Each Q3
 * weapon maps to the Q2 weapon that plays its role: the Chainfist is the
 * only melee weapon (Gauntlet), the Chaingun is the continuous hitscan
 * weapon (Lightning Gun), the HyperBlaster the rapid projectile weapon
 * (Plasma Gun). With Q3's own numbers the Blaster read as the Gauntlet
 * (BotCheckAttack then only fired within 60 units) and every per-weapon aim
 * characteristic was looked up for the wrong weapon. Weapons with no Q2
 * counterpart get numbers weapons.c does not use. */
#define WP_NONE				0
#define WP_GAUNTLET			18
#define WP_MACHINEGUN		4
#define WP_SHOTGUN			2
#define WP_GRENADE_LAUNCHER	7
#define WP_ROCKET_LAUNCHER	8
#define WP_LIGHTNING		5
#define WP_RAILGUN			10
#define WP_PLASMAGUN		9
#define WP_BFG				11
#define WP_GRAPPLING_HOOK	97
#define WP_NAILGUN			98
#define WP_CHAINGUN			99

/* powerup_t subset actually referenced */
#define PW_NONE			0
#define PW_QUAD			1
#define PW_BATTLESUIT	2
#define PW_HASTE		3
#define PW_INVIS		4
#define PW_REGEN		5
#define PW_FLIGHT		6
#define PW_REDFLAG		7
#define PW_BLUEFLAG		8
#define PW_NEUTRALFLAG	9

/* configstring bases -- trap_GetConfigstring/trap_SetConfigstring are
 * short-circuited in ai_q2_shim.c (see there), so the values only need to
 * keep the ranges apart. CS_SOUNDS is NOT defined here: the real one
 * (botlib/be_aas_def.h, already included above) would conflict -- reuse it
 * as-is. The player range follows the sound range, as in Q3's layout:
 * BotCheckEvents asks for CS_SOUNDS + a sound index (ai_q2_shim.c serves
 * the powerup respawn sound). */
#define CS_PLAYERS		(CS_SOUNDS+MAX_SOUNDS)
#define CS_BOTINFO		(CS_PLAYERS+MAX_CLIENTS)

/* how long Q3 keeps an event on an entity (bg_public.h); ai_q2_shim.c keeps
 * its temporary event entities that long */
#define EVENT_VALID_MSEC	300

/* missing size constants (real Q3 q_shared.h has these; this repo's
 * trimmed copy does not) */
#ifndef MAX_NETNAME
#define MAX_NETNAME		36
#endif

/* ================================================================== */
/*  bot_settings_t -- genuinely never defined anywhere in this repo.  */
/*  Real Q3's code/game/ai_main.h defines it; this repo's copy of     */
/*  ai_main.h (game_q3/ai_main.h) only ever *uses* bot_settings_t     */
/*  (BotAISetupClient/BotResetState), it never got its definition     */
/*  carried over. Fields are exactly what ai_main.c/ai_dmq3.c read:   */
/*  settings->characterfile, settings->skill, bs->settings.team.      */
/* ================================================================== */
typedef struct bot_settings_s {
	char	characterfile[MAX_QPATH];
	float	skill;
	char	team[MAX_QPATH];
} bot_settings_t;

/* ================================================================== */
/*  MOD_* -- Q2's real means-of-death values (mirrored, not derived,  */
/*  from game_q2/g_local.h -- botlib.so cannot include that game.so-  */
/*  shaped header, so the numeric values are copied here verbatim;    */
/*  they must keep matching g_local.h since Q2BotNotifyDeath/Kill's   */
/*  `mod` parameter, ultimately read by ai_chat.c as bs->botdeathtype/ */
/*  enemydeathtype, originates from Q2's real means_of_death values). */
/*  Extends game_q2/ai_chat_q2.h's block, which only had to cover     */
/*  ai_chat.c: ai_dmq3.c/ai_chat.c together also need MOD_GRAPPLE,    */
/*  MOD_CHAINGUN and the full Q2 numeric set below.                  */
/* ================================================================== */
#define MOD_UNKNOWN			0
#define MOD_BLASTER			1
#define MOD_SHOTGUN			2
#define MOD_SSHOTGUN		3
#define MOD_MACHINEGUN		4
#define MOD_CHAINGUN		5
#define MOD_GRENADE			6
#define MOD_G_SPLASH		7
#define MOD_ROCKET			8
#define MOD_R_SPLASH		9
#define MOD_HYPERBLASTER	10
#define MOD_RAILGUN			11
#define MOD_BFG_LASER		12
#define MOD_BFG_BLAST		13
#define MOD_BFG_EFFECT		14
#define MOD_HANDGRENADE		15
#define MOD_HG_SPLASH		16
#define MOD_WATER			17
#define MOD_SLIME			18
#define MOD_LAVA			19
#define MOD_CRUSH			20
#define MOD_TELEFRAG		21
#define MOD_FALLING			22
#define MOD_SUICIDE			23
#define MOD_HELD_GRENADE	24
#define MOD_TARGET_LASER	30
#define MOD_TRIGGER_HURT	31
#define MOD_GRAPPLE			34
/* The Reckoning */
#define MOD_RIPPER			35
#define MOD_PHALANX			36
#define MOD_TRAP			40
/* Ground Zero */
#define MOD_CHAINFIST		41
#define MOD_DISINTEGRATOR	42
#define MOD_ETF_RIFLE		43
#define MOD_HEATBEAM		45
#define MOD_TESLA			46
#define MOD_PROX			47
#define MOD_NUKE			48
#define MOD_VENGEANCE_SPHERE	49
#define MOD_HUNTER_SPHERE	50
#define MOD_DEFENDER_SPHERE	51
#define MOD_TRACKER			52
#define MOD_DOPPLE_EXPLODE	54
#define MOD_DOPPLE_VENGEANCE	55
#define MOD_DOPPLE_HUNTER	56

/* Q3-only MODs that don't exist in Q2 -- negative sentinels so they
 * never match any real Q2 mod; ai_chat.c's switch/if chains fall
 * through to the generic chat paths for these. */
#define MOD_GAUNTLET        (-100)
#define MOD_LIGHTNING       (-101)
#define MOD_PLASMA          (-102)
#define MOD_PLASMA_SPLASH   (-103)
#define MOD_NAIL            (-104)
#define MOD_PROXIMITY_MINE  (-106)
#define MOD_KAMIKAZE        (-107)
#define MOD_JUICED          (-108)

/* Q3 BFG/BFG_SPLASH == Q2 BFG_BLAST/BFG_EFFECT; Q3's GRENADE_SPLASH/
 * ROCKET_SPLASH == Q2's G_SPLASH/R_SPLASH. */
#define MOD_BFG             MOD_BFG_BLAST
#define MOD_BFG_SPLASH      MOD_BFG_EFFECT
#define MOD_GRENADE_SPLASH  MOD_G_SPLASH
#define MOD_ROCKET_SPLASH   MOD_R_SPLASH

/* ================================================================== */
/*  Team / gametype constants (bs->settings.team, gametype global).   */
/*  Purely internal bookkeeping for the ported AI logic -- these do   */
/*  NOT need to match any real Q2 CTF_TEAM1/2 numbering, since         */
/*  BotSameTeam()/bl_chat.c-equivalent code compares Q2's own real     */
/*  team fields directly and only exposes true/false across this      */
/*  boundary (see ai_q2_shim.c's BotSameTeam usage note). */
/* ================================================================== */
#define TEAM_FREE			0
#define TEAM_RED			1
#define TEAM_BLUE			2
#define TEAM_SPECTATOR		3

#define GT_FFA				0
#define GT_TOURNAMENT		1
#define GT_TEAM				3
#define GT_CTF				4
#define GT_1FCTF			5
#define GT_OBELISK			6
#define GT_HARVESTER		7

/* ================================================================== */
/*  Q3-only inventory slots with no Q2 equivalent -- mapped to a safe  */
/*  unused slot (matches game_q2/ai_chat_q2.h's original approach).   */
/*  Everything else comes from the REAL assets/botfiles/inv.h, which  */
/*  each of the 5 .c files #include directly (needs -Iassets/botfiles, */
/*  added to the botlib Makefile recipe).                             */
/* ================================================================== */
#define INVENTORY_HASTE           199
#define INVENTORY_INVISIBILITY    199
#define INVENTORY_REGEN           199
#define INVENTORY_FLIGHT          199
#define INVENTORY_GAUNTLET        198
#define INVENTORY_GRAPPLINGHOOK   197
#define INVENTORY_TELEPORTER      196
#define INVENTORY_MEDKIT          195
#define INVENTORY_NAILGUN         194
#define INVENTORY_NAILS           193
#define INVENTORY_BELT            190

/* Q3 weapons whose role a Q2 weapon plays (same pairing as the WP_* block
 * above), so the Q3 weapon rules -- BotAggression, BotWantsToCamp,
 * BotHasPersistantPowerupAndWeapon, BotSelectActivateWeapon -- see the Q2
 * weapon instead of a dead slot. The macros expand at the point of use,
 * after each .c file has included inv.h.
 *
 * INVENTORY_PROXLAUNCHER needs no alias: inv.h defines it for Ground Zero's
 * Prox Launcher, which plays the part of Team Arena's, so Q3's mines are
 * Ground Zero's Prox.
 *
 * INVENTORY_BFGAMMO is a slot of its own: Q3 counts BFG ammo in shots and
 * its rules test "> 7", ">= 10" and "> 0", while Q2's BFG draws 50 cells
 * per shot. Q2BotUpdateClient writes the cell count there once it reaches
 * 50 and 0 below that, so every one of those tests reads "can fire the BFG
 * at least once". Slot 202 is free: inv.h uses 200/201 for derived enemy
 * values and Q2 has far fewer than 200 items. */
#define INVENTORY_PLASMAGUN       INVENTORY_HYPERBLASTER
#define INVENTORY_LIGHTNING       INVENTORY_CHAINGUN
#define INVENTORY_LIGHTNINGAMMO   INVENTORY_BULLETS
#define INVENTORY_MINES           INVENTORY_PROX
#define INVENTORY_BFGAMMO         202

/* stats[STAT_HOLDABLE_ITEM] model-index sentinels (BotUpdateInventory) --
 * Q2 has no personal teleporter/medkit holdable, so stats[STAT_HOLDABLE_ITEM]
 * (always 0 in Phase 0) will never equal these; exact values don't matter. */
#define MODELINDEX_TELEPORTER     9001
#define MODELINDEX_MEDKIT         9002

/* weaponstate_t (real Q3 values) */
#define WEAPON_READY		0
#define WEAPON_RAISING		1
#define WEAPON_DROPPING		2
#define WEAPON_FIRING		3

/* playerState_t.pm_flags bits actually referenced (real Q3 values) */
#define PMF_DUCKED				1
#define PMF_TIME_KNOCKBACK		64
#define PMF_TIME_WATERJUMP		256

#define MASK_SOLID	(CONTENTS_SOLID)
#define MASK_WATER	(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)

/* BotSetTeamStatus's local teamtask values (ai_dmq3.c) -- only ever
 * formatted into a string via BotSetUserInfo, itself a no-op in Q2
 * (trap_SetUserinfo has nowhere real to write to), so exact values are
 * inconsequential; just need to be distinct. */
#define TEAMTASK_PATROL		1
#define TEAMTASK_ESCORT		2
#define TEAMTASK_FOLLOW		3
#define TEAMTASK_DEFENSE	4
#define TEAMTASK_OFFENSE	5
#define TEAMTASK_RETRIEVE	6
#define TEAMTASK_CAMP		7

/* Weapon-select return values (BotSelectActivateWeapon, BotClearPath,
 * ai_dmnet.c). They end up in bs->weaponnum / moveresult->weapon and from
 * there in EA_SelectWeapon, i.e. they are weapon config numbers like WP_*
 * (Q3's own inv.h defines them equal to the WP_* values) -- not inventory
 * slots, which is what they used to be here (the Machinegun then selected
 * weapon 10, the Railgun). */
#define WEAPONINDEX_MACHINEGUN			WP_MACHINEGUN
#define WEAPONINDEX_SHOTGUN				WP_SHOTGUN
#define WEAPONINDEX_RAILGUN				WP_RAILGUN
#define WEAPONINDEX_ROCKET_LAUNCHER		WP_ROCKET_LAUNCHER
#define WEAPONINDEX_BFG					WP_BFG
#define WEAPONINDEX_PLASMAGUN			WP_PLASMAGUN
#define WEAPONINDEX_LIGHTNING			WP_LIGHTNING

/* real Q3 q_shared.h has this; this repo's trimmed copy does not */
#define MASK_SHOT	(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE)

/* ================================================================== */
/*  VOICECHAT_* -- real Q3 gets these from MISSIONPACK's ui/menudef.h, */
/*  which lives outside this repo entirely ("../../ui/menudef.h" as   */
/*  included by ai_dmq3.c/ai_team.c resolves nowhere here). Q2 has no  */
/*  voice-chat concept at all (trap_EA_Command() already no-ops        */
/*  "vtaunt" et al in the existing chat shim) -- these only need to   */
/*  be distinct strings so BotVoiceChat/BotVoiceChatOnly (ai_team.c,  */
/*  self-contained, routes through EA_Command/trap_BotEnterChat) and   */
/*  their callers compile. */
/* ================================================================== */
#define VOICECHAT_YES				"vc_yes"
#define VOICECHAT_NO				"vc_no"
#define VOICECHAT_FOLLOWME			"vc_followme"
#define VOICECHAT_STARTLEADER		"vc_startleader"
#define VOICECHAT_GETFLAG			"vc_getflag"
#define VOICECHAT_OFFENSE			"vc_offense"
#define VOICECHAT_DEFEND			"vc_defend"
#define VOICECHAT_RETURNFLAG		"vc_returnflag"
#define VOICECHAT_INPOSITION		"vc_inposition"
#define VOICECHAT_ONDEFENSE			"vc_ondefense"
#define VOICECHAT_ONOFFENSE			"vc_onoffense"
#define VOICECHAT_ONFOLLOW			"vc_onfollow"
#define VOICECHAT_ONGETFLAG			"vc_ongetflag"
#define VOICECHAT_ONRETURNFLAG		"vc_onreturnflag"
#define VOICECHAT_WANTONDEFENSE		"vc_wantondefense"
#define VOICECHAT_WANTONOFFENSE		"vc_wantonoffense"
#define VOICECHAT_FOLLOWFLAGCARRIER	"vc_followflagcarrier"
#define VOICECHAT_IHAVEFLAG			"vc_ihaveflag"
#define VOICECHAT_CAMP				"vc_camp"
#define VOICECHAT_DEFENDFLAG		"vc_defendflag"
#define VOICECHAT_ONPATROL			"vc_onpatrol"
#define VOICECHAT_PATROL			"vc_patrol"
#define VOICECHAT_STOPLEADER		"vc_stopleader"
#define VOICECHAT_TAUNT				"vc_taunt"
#define VOICECHAT_WHOISLEADER		"vc_whoisleader"

/* ================================================================== */
/*  Q3-shaped entity state.                                            */
/*                                                                      */
/*  Real Q3 game code reaches per-entity data through g_entities[]     */
/*  directly. botlib.so has no access to Q2's edict array (that is the  */
/*  game<->botlib ABI boundary this port works around), so             */
/*  ai_q2_shim.c keeps this Q3-shaped copy, filled from what the Q2    */
/*  game does send (see "Q3-shaped game state" there). Readers:        */
/*    - BotAI_GetEntityState/BotAI_GetSnapshotEntity (ai_main.c) and   */
/*      through them BotCheckSnapshot/BotCheckEvents/                  */
/*      BotCheckForGrenades (ai_dmq3.c): inuse, r.linked, r.svFlags,   */
/*      s, eventTime.                                                   */
/*    - FindHumanTeamLeader (ai_team.c): inuse and SVF_BOT of clients. */
/*    - the hit chats (ai_chat.c): client->lasthurt_client/_mod, which  */
/*      every client entity has (Q2Shim_ClientHurt).                    */
/*  level.intermissiontime stays 0: BotIntermission falls back to the  */
/*  PM_FREEZE player state, which Q2's intermission sets.              */
/* ================================================================== */
#define SVF_NOCLIENT	0x00000001
#define SVF_BOT			0x00000008
#define SVF_BROADCAST	0x00000020

typedef struct q2_gclient_compat_s {
	int lasthurt_client;
	int lasthurt_mod;
} q2_gclient_compat_t;

typedef struct q2_gentity_compat_s {
	qboolean	inuse;
	entityState_t	s;
	struct {
		qboolean	linked;
		int		contents;
		int		svFlags;
		vec3_t	currentOrigin;
		vec3_t	mins, maxs;
	} r;
	int		eventTime;
	q2_gclient_compat_t *client;
	void	*activator;
} gentity_t;

extern gentity_t g_entities_compat[MAX_GENTITIES];
#define g_entities g_entities_compat

typedef struct q2_level_compat_s {
	int		intermissiontime;
	int		num_entities;
} q2_level_compat_t;
extern q2_level_compat_t level;

/* the number of client slots (ai_dmq3.c, from sv_maxclients) */
extern int maxclients;

/* ================================================================== */
/*  Tier A: direct aliases to already-compiled real botlib functions. */
/*  Every one of these is verified (grep'd against botlib's .c/.h) to */
/*  exist with a matching signature -- see the Phase 0 report.        */
/* ================================================================== */

/* AAS */
#define trap_AAS_AlternativeRouteGoals		AAS_AlternativeRouteGoals
#define trap_AAS_AreaInfo					AAS_AreaInfo
#define trap_AAS_AreaReachability			AAS_AreaReachability
#define trap_AAS_AreaTravelTimeToGoalArea	AAS_AreaTravelTimeToGoalArea
#define trap_AAS_BBoxAreas					AAS_BBoxAreas
#define trap_AAS_EnableRoutingArea			AAS_EnableRoutingArea
#define trap_AAS_EntityInfo					AAS_EntityInfo
#define trap_AAS_FloatForBSPEpairKey			AAS_FloatForBSPEpairKey
#define trap_AAS_Initialized					AAS_Initialized
#define trap_AAS_IntForBSPEpairKey			AAS_IntForBSPEpairKey
#define trap_AAS_NextBSPEntity				AAS_NextBSPEntity
#define trap_AAS_NextEntity					AAS_NextEntity
#define trap_AAS_PointAreaNum				AAS_PointAreaNum
#define trap_AAS_PointContents				AAS_PointContents
#define trap_AAS_PredictClientMovement		AAS_PredictClientMovement
#define trap_AAS_PredictRoute				AAS_PredictRoute
#define trap_AAS_PresenceTypeBoundingBox		AAS_PresenceTypeBoundingBox
#define trap_AAS_Swimming					AAS_Swimming
#define trap_AAS_Time						AAS_Time
#define trap_AAS_TraceAreas					AAS_TraceAreas
#define trap_AAS_ValueForBSPEpairKey			AAS_ValueForBSPEpairKey
#define trap_AAS_VectorForBSPEpairKey		AAS_VectorForBSPEpairKey

/* EA */
#define trap_EA_Action			EA_Action
#define trap_EA_Attack			EA_Attack
#define trap_EA_Command			EA_Command
#define trap_EA_Crouch			EA_Crouch
#define trap_EA_Gesture			EA_Gesture
#define trap_EA_GetInput			EA_GetInput
#define trap_EA_ResetInput		EA_ResetInput
#define trap_EA_Respawn			EA_Respawn
#define trap_EA_Say				EA_Say
#define trap_EA_SayTeam			EA_SayTeam
#define trap_EA_SelectWeapon		EA_SelectWeapon
#define trap_EA_Talk				EA_Talk
#define trap_EA_Use				EA_Use
#define trap_EA_View				EA_View

/* Chat. BotFindMatch and BotReplaceSynonyms go through ai_q2_shim.c, which
 * keeps each answer for the bots that read a chat line after the first. */
int  Q2Shim_BotFindMatch(char *str, bot_match_t *match, unsigned long int context);
void Q2Shim_BotReplaceSynonyms(char *string, unsigned long int context);
#define trap_BotAllocChatState		BotAllocChatState
#define trap_BotChatLength			BotChatLength
#define trap_BotEnterChat			BotEnterChat
#define trap_BotFindMatch			Q2Shim_BotFindMatch
#define trap_BotFreeChatState		BotFreeChatState
#define trap_BotGetChatMessage		BotGetChatMessage
#define trap_BotInitialChat			BotInitialChat
#define trap_BotMatchVariable		BotMatchVariable
#define trap_BotNextConsoleMessage	BotNextConsoleMessage
#define trap_BotNumConsoleMessages	BotNumConsoleMessages
#define trap_BotNumInitialChats		BotNumInitialChats
#define trap_BotQueueConsoleMessage	BotQueueConsoleMessage
#define trap_BotRemoveConsoleMessage	BotRemoveConsoleMessage
#define trap_BotReplyChat			BotReplyChat
#define trap_BotSetChatGender		BotSetChatGender
#define trap_BotSetChatName			BotSetChatName
#define trap_BotReplaceSynonyms		Q2Shim_BotReplaceSynonyms
#define trap_BotLoadChatFile			BotLoadChatFile
#define trap_UnifyWhiteSpaces		UnifyWhiteSpaces

/* Goal */
#define trap_BotAllocGoalState			BotAllocGoalState
#define trap_BotAvoidGoalTime			BotAvoidGoalTime
#define trap_BotChooseLTGItem			BotChooseLTGItem
#define trap_BotChooseNBGItem			BotChooseNBGItem
#define trap_BotDumpAvoidGoals			BotDumpAvoidGoals
#define trap_BotDumpGoalStack			BotDumpGoalStack
#define trap_BotEmptyGoalStack			BotEmptyGoalStack
#define trap_BotFreeGoalState			BotFreeGoalState
#define trap_BotGetLevelItemGoal			BotGetLevelItemGoal
#define trap_BotGetNextCampSpotGoal		BotGetNextCampSpotGoal
#define trap_BotGetSecondGoal			BotGetSecondGoal
#define trap_BotGetTopGoal				BotGetTopGoal
#define trap_BotGoalName					BotGoalName
#define trap_BotInterbreedGoalFuzzyLogic	BotInterbreedGoalFuzzyLogic
#define trap_BotItemGoalInVisButNotVisible	BotItemGoalInVisButNotVisible
#define trap_BotLoadItemWeights			BotLoadItemWeights
#define trap_BotMutateGoalFuzzyLogic		BotMutateGoalFuzzyLogic
#define trap_BotPopGoal					BotPopGoal
#define trap_BotPushGoal					BotPushGoal
#define trap_BotRemoveFromAvoidGoals		BotRemoveFromAvoidGoals
#define trap_BotResetAvoidGoals			BotResetAvoidGoals
#define trap_BotResetGoalState			BotResetGoalState
#define trap_BotSaveGoalFuzzyLogic		BotSaveGoalFuzzyLogic
#define trap_BotSetAvoidGoalTime			BotSetAvoidGoalTime
#define trap_BotTouchingGoal				BotTouchingGoal
#define trap_BotUpdateEntityItems		BotUpdateEntityItems

/* Move */
#define trap_BotAddAvoidSpot			BotAddAvoidSpot
#define trap_BotAllocMoveState		BotAllocMoveState
#define trap_BotFreeMoveState		BotFreeMoveState
#define trap_BotInitMoveState		BotInitMoveState
#define trap_BotMoveInDirection		BotMoveInDirection
#define trap_BotMovementViewTarget	BotMovementViewTarget
#define trap_BotMoveToGoal			BotMoveToGoal
#define trap_BotPredictVisiblePosition	BotPredictVisiblePosition
#define trap_BotReachabilityArea		BotReachabilityArea
#define trap_BotResetAvoidReach		BotResetAvoidReach
#define trap_BotResetLastAvoidReach	BotResetLastAvoidReach
#define trap_BotResetMoveState		BotResetMoveState

/* Weapon */
#define trap_BotChooseBestFightWeapon	BotChooseBestFightWeapon
#define trap_BotGetWeaponInfo			BotGetWeaponInfo
#define trap_BotLoadWeaponWeights		BotLoadWeaponWeights
#define trap_BotAllocWeaponState			BotAllocWeaponState
#define trap_BotFreeWeaponState			BotFreeWeaponState
#define trap_BotResetWeaponState			BotResetWeaponState

/* Character */
#define trap_BotLoadCharacter		BotLoadCharacter
#define trap_BotFreeCharacter		BotFreeCharacter
#define trap_Characteristic_BFloat	Characteristic_BFloat
#define trap_Characteristic_BInteger	Characteristic_BInteger
#define trap_Characteristic_String	Characteristic_String

/* Genetic */
#define trap_GeneticParentsAndChildSelection	GeneticParentsAndChildSelection

/* ================================================================== */
/*  Tier A': aliases to the already-populated botimport table         */
/*  (botlib/be_interface.h; populated in be_interface_q2.c's           */
/*  GetBotAPI -> GetBotLibAPI -> be_interface.c). trap_PointContents   */
/*  keeps its 2-arg game-side call shape (point, passent) but only     */
/*  forwards the point -- botimport.PointContents has no entity-       */
/*  exclusion concept, matching how bl_chat.c's own trap_PointContents */
/*  already ignores the entnum argument today.                        */
/* ================================================================== */
#define trap_PointContents(point, passent)	botimport.PointContents(point)

/* ================================================================== */
/*  Tier B: short new bodies (implemented in ai_q2_shim.c).           */
/*  Declared here so the 5 ported files see real prototypes (not      */
/*  implicit int-returning declarations -- several of these return    */
/*  float/pointer types where that would silently corrupt values).    */
/* ================================================================== */

/* cvars -- routed through botlib's own LibVar mechanism (l_libvar.c),
 * exactly how bot_developer already works; no new ABI surface. */
void	trap_Cvar_Register(vmCvar_t *cv, char *var_name, char *value, int flags);
void	trap_Cvar_Update(vmCvar_t *cv);
void	trap_Cvar_Set(const char *var_name, const char *value);
int		trap_Cvar_VariableIntegerValue(const char *var_name);
void	trap_Cvar_VariableStringBuffer(const char *var_name, char *buf, int bufsize);

/* botlib_export_t entry points, wrapped 1:1 because their real names
 * (be_interface.c) are Export_BotLib* -- not literally "BotLib*". */
int		trap_BotLibSetup(void);
int		trap_BotLibShutdown(void);
int		trap_BotLibVarSet(char *var_name, char *value);
int		trap_BotLibStartFrame(float time);
int		trap_BotLibLoadMap(const char *mapname);
int		trap_BotLibUpdateEntity(int ent, bot_entitystate_t *state);

/* engine glue with no Q2 botlib.so-side data source yet (see
 * ai_q2_shim.c for what each one actually does and why). */
void	trap_GetConfigstring(int num, char *buf, int size);
void	trap_SetConfigstring(int num, const char *string);
void	trap_GetServerinfo(char *buf, int size);
void	trap_GetUserinfo(int num, char *buf, int size);
void	trap_SetUserinfo(int num, const char *buf);
void	trap_SendConsoleCommand(int exec_when, char *text);

/* Q2's bot chat-message queue / Q3 snapshot concept don't exist here;
 * "nothing pending" / "no more entities" are the correct answers. */
int		trap_BotGetServerCommand(int clientNum, char *message, int size);
int		trap_BotGetSnapshotEntity(int clientNum, int sequence);

/* helpers with no existing real implementation anywhere in this repo */
float	AngleMod(float a);
void	vectoangles(const vec3_t value1, vec3_t angles);
char	*Q_CleanStr(char *string);
void	ClientUserinfoChanged(int clientNum);
int		G_ModelIndex(char *name);	/* unused after Phase 0 deletions; kept for link-safety if ever re-referenced */
void	*G_Alloc(int size);
void	G_CheckBotSpawn(void);		/* dead after BotAIStartFrame deletion; see ai_q2_shim.c */

/*
 * Deliberately NOT declared here (would conflict, not just duplicate):
 * BotAI_Print/BotAI_Trace/BotAI_GetClientState/BotAI_GetEntityState/
 * BotAI_GetSnapshotEntity/BotAI_BotInitialChat/BotEntityInfo are already
 * declared by game_q3/ai_main.h (using the real bot_state_t, once it's
 * been fully defined) -- ai_main.h is included by all 5 ported files and
 * by ai_q2_shim.c alike, always after this header, so re-declaring them
 * here against a merely-forward-declared `struct bot_state_s` would
 * fight ai_main.h's own (later, now-complete) declaration and fail with
 * "conflicting types" -- confirmed the hard way (see report). Bodies
 * live in ai_q2_shim.c regardless; only the declaration moves.
 *
 * The order parsing (ai_cmd.c: BotMatchMessage, team leadership, orders,
 * CTF flag messages) and the voice chat handlers (ai_vcmd.c) are Q3's own
 * and compile into botlib.so like the other files; ai_vcmd.c's handlers
 * answer through voice chats Q2 does not have (Q3BotClientCommand_Adapter
 * in be_interface_q2.c drops those), their orders stand.
 */

#endif /* AI_Q2_COMPAT_H */
