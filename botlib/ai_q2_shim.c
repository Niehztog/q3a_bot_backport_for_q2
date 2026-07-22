/*
 * ai_q2_shim.c -- Q2 compatibility implementations for the real Q3 bot AI
 * source (game_q3/ai_main.c, ai_dmnet.c, ai_dmq3.c, ai_team.c, ai_chat.c),
 * which now compile directly into botlib.so instead of being hand-
 * reinvented in be_interface_q2.c.
 *
 * Three tiers of trap_ / BotAI_ / helper implementations, per the plan:
 *
 *   Tier A  -- #define trap_X X straight to an already-compiled real
 *              botlib function (see ai_q2_compat.h; nothing to do here).
 *   Tier A' -- #define trap_X(...) botimport.X(...) (also in
 *              ai_q2_compat.h).
 *   Tier B  -- short new bodies for things with no existing counterpart.
 *              That's what this file is.
 *
 * Frozen-ABI constraint: nothing in this file may add a new exported or
 * imported function to q2_bot_export_t/q2_bot_import_t
 * (botlib/be_interface_q2.c) or bot_export_t/bot_import_t
 * (game_q2/botlib.h). Every trap_* here resolves either to an
 * already-compiled real botlib function, to the already-populated
 * `botimport` table, or to a self-contained local implementation.
 */

#include "ai_q2_compat.h"
#include "../game_q3/ai_main.h"
#include "../game_q3/ai_dmq3.h"
#include "../game_q3/ai_cmd.h"		/* BotMatchMessage's real prototype */
#include "../game_q3/ai_vcmd.h"		/* BotVoiceChat_Defend's real prototype */

/* be_interface.c's real implementations behind botlib_export_t's
 * BotLibSetup/BotLibShutdown/BotLibVarSet/BotLibStartFrame/BotLibLoadMap/
 * BotLibUpdateEntity. Called directly here since ai_q2_shim.c links into
 * the same botlib.so and these already have external linkage -- they're
 * normally only reached indirectly, through the botlib_export_t table
 * that GetBotLibAPI() hands back to be_interface_q2.c's GetBotAPI(). */
int Export_BotLibSetup(void);
int Export_BotLibShutdown(void);
int Export_BotLibVarSet(char *var_name, char *value);
int Export_BotLibStartFrame(float time);
int Export_BotLibLoadMap(const char *mapname);
int Export_BotLibUpdateEntity(int ent, bot_entitystate_t *state);

/* Per-client bot state array, defined in game_q3/ai_main.c. No header
 * declares it (ai_main.h only declares BotResetState/NumBots/
 * BotEntityInfo/BotTeamLeader) -- extern-declare it directly here, same
 * as be_interface_q2.c already does. Needed by BotAI_GetClientState
 * below. */
extern bot_state_t *botstates[MAX_CLIENTS];

/* ================================================================== */
/*  Globals declared in ai_q2_compat.h                                 */
/* ================================================================== */

/* Q3-shaped entity/level scaffolding -- see the long comment in
 * ai_q2_compat.h. Always zeroed; nothing populates these in Phase 0. */
gentity_t          g_entities_compat[MAX_GENTITIES];
q2_level_compat_t  level;

/* ai_cmd.c's notleader[] (referenced extern by ai_cmd.h, read by
 * ai_team.c's FindHumanTeamLeader) -- ai_cmd.c itself is out of scope
 * for this port (see report), so the backing storage lives here instead. */
int notleader[MAX_CLIENTS];

/* NOTE: gametype/maxclients/vec3_origin are already defined for real --
 * gametype/maxclients as plain globals in game_q3/ai_dmq3.c, vec3_origin
 * in game_q2/q_shared.c (already linked into botlib.so) -- so they are
 * deliberately NOT redefined here (would be duplicate-symbol link errors). */

/* ================================================================== */
/*  Cvars, routed through botlib's own LibVar mechanism (l_libvar.c) --  */
/*  exactly how bot_developer already works. No new ABI surface: these  */
/*  become compiled-in LibVar defaults, tunable only via the existing    */
/*  BotLibVarSet export. trap_Cvar_Register's vmCvar_t->handle is (ab)used */
/*  as an index into a small local name table so trap_Cvar_Update can    */
/*  find its way back to the right LibVar -- exactly the role handle    */
/*  plays in the real engine-side cvar system this is standing in for.  */
/* ================================================================== */

#define MAX_Q2_SHIM_CVARS	64
static char q2_shim_cvar_names[MAX_Q2_SHIM_CVARS][64];
static int  q2_shim_num_cvars = 0;

static void Q2Shim_RefreshCvar(vmCvar_t *cv, const char *var_name)
{
	const char *s;

	cv->value = LibVarGetValue((char *)var_name);
	cv->integer = (int)cv->value;
	s = LibVarGetString((char *)var_name);
	Q_strncpyz(cv->string, s ? s : "", sizeof(cv->string));
}

void trap_Cvar_Register(vmCvar_t *cv, char *var_name, char *value, int flags)
{
	(void)flags;

	LibVar(var_name, value);

	if (!cv) return;

	if (q2_shim_num_cvars < MAX_Q2_SHIM_CVARS) {
		Q_strncpyz(q2_shim_cvar_names[q2_shim_num_cvars], var_name,
		           sizeof(q2_shim_cvar_names[0]));
		cv->handle = q2_shim_num_cvars;
		q2_shim_num_cvars++;
	} else {
		cv->handle = -1;
	}
	cv->modificationCount = 0;
	Q2Shim_RefreshCvar(cv, var_name);
}

void trap_Cvar_Update(vmCvar_t *cv)
{
	if (!cv || cv->handle < 0 || cv->handle >= q2_shim_num_cvars) return;
	Q2Shim_RefreshCvar(cv, q2_shim_cvar_names[cv->handle]);
}

void trap_Cvar_Set(const char *var_name, const char *value)
{
	LibVarSet((char *)var_name, (char *)value);
}

int trap_Cvar_VariableIntegerValue(const char *var_name)
{
	return (int)LibVarGetValue((char *)var_name);
}

void trap_Cvar_VariableStringBuffer(const char *var_name, char *buf, int bufsize)
{
	const char *s = LibVarGetString((char *)var_name);
	Q_strncpyz(buf, s ? s : "", bufsize);
}

/* ================================================================== */
/*  botlib_export_t entry points, wrapped 1:1 (real names are          */
/*  Export_BotLib*, not literally "BotLib*").                          */
/* ================================================================== */

int trap_BotLibSetup(void)
{
	return Export_BotLibSetup();
}

int trap_BotLibShutdown(void)
{
	return Export_BotLibShutdown();
}

int trap_BotLibVarSet(char *var_name, char *value)
{
	return Export_BotLibVarSet(var_name, value);
}

int trap_BotLibStartFrame(float time)
{
	return Export_BotLibStartFrame(time);
}

int trap_BotLibLoadMap(const char *mapname)
{
	return Export_BotLibLoadMap(mapname);
}

int trap_BotLibUpdateEntity(int ent, bot_entitystate_t *state)
{
	return Export_BotLibUpdateEntity(ent, state);
}

/* ================================================================== */
/*  Engine glue with no Q2 botlib.so-side data source yet.             */
/*  Per the plan: stub sensibly (empty string / 0), do NOT invent new  */
/*  ABI surface to fetch the real answer -- that's a Phase 1+ problem, */
/*  not Phase 0 scaffolding. Concrete Phase 1+ mechanisms noted below.  */
/* ================================================================== */

/* Mirrors real Q3 be_ai_chat.c's private bot_chatstate_t layout just
 * enough to read the cached name back out -- gender/client/name are its
 * first three real fields, in this exact order (int, int, char[32], no
 * padding surprises), so this prefix-only shadow is safe without
 * touching be_ai_chat.c's own, intentionally-private struct. */
typedef struct { int gender; int client; char name[32]; } q2_chatstate_peek_t;
extern q2_chatstate_peek_t *BotChatStateFromHandle(int handle);

/*
 * trap_GetConfigstring -- ClientName/EasyClientName/ClientSkin/
 * BotIsObserver/BotSameTeam/TeamPlayIsOn's helpers (all real, in
 * ai_dmq3.c) and BotTeamplayReport/BotUpdateInfoConfigStrings (ai_main.c)
 * all read player name/team/model through this.
 *
 * botlib.so already caches a real per-bot netname -- BotSetChatName()
 * (be_ai_chat.c) stores it in each bot's own bot_chatstate_t.name[32],
 * written both at bot setup (the bot's character name) and on every
 * BotClientSettings call (the real in-game netname). For CS_PLAYERS+n,
 * synthesize a minimal "n\\<name>\\t\\<team>" info string from that
 * cache plus bs->q2_realctfteam. Only works when client n is itself an
 * active bot (a human slot has no bs/chatstate at all) -- for a human
 * client this still degrades to an empty string, exactly as before. */
void trap_GetConfigstring(int num, char *buf, int size)
{
	int client;
	bot_state_t *bs;
	q2_chatstate_peek_t *cs;

	if (size > 0) buf[0] = '\0';
	if (size <= 0) return;

	client = num - CS_PLAYERS;
	if (client < 0 || client >= MAX_CLIENTS) return;

	bs = botstates[client];
	if (!bs || !bs->inuse || bs->cs <= 0) return;

	cs = BotChatStateFromHandle(bs->cs);
	if (!cs || !cs->name[0]) return;

	snprintf(buf, size, "\\n\\%s\\t\\%d", cs->name, bs->q2_realctfteam);
}

void trap_SetConfigstring(int num, const char *string)
{
	/* Q2 has no configstring system to write to; BotSetInfoConfigString
	 * (ai_main.c, CS_BOTINFO+client "status line") has no reader on the
	 * Q2 side either, so this is a true no-op, not a partial one. */
	(void)num; (void)string;
}

/*
 * trap_GetServerinfo -- only consumer is BotMapTitle() (ai_chat.c),
 * wanting "mapname". Mirrors game_q2/bl_chat.c's now-deleted
 * trap_GetServerinfo (which read Q2's real level.mapname from game.so
 * side) via be_interface_q2.c's q2_cached_mapname, set once per map load
 * from Q2BotLoadMap's own mapname argument.
 */
extern char q2_cached_mapname[];

void trap_GetServerinfo(char *buf, int size)
{
	if (size <= 0) return;
	snprintf(buf, size, "\\mapname\\%s", q2_cached_mapname);
}

void trap_GetUserinfo(int num, char *buf, int size)
{
	(void)num;
	if (size > 0) buf[0] = '\0';
}

void trap_SetUserinfo(int num, const char *buf)
{
	(void)num; (void)buf;
}

void trap_SendConsoleCommand(int exec_when, char *text)
{
	/* Only reachable from BotInterbreeding() (ai_main.c), itself only
	 * ever called from the now-deleted BotAIStartFrame -- unreachable
	 * dead code kept link-safe. */
	(void)exec_when; (void)text;
}

/*
 * Q2's bot chat-message queue (populated by an entirely separate,
 * already-working path -- BotConsoleMessage, called from game_q2/
 * bl_chat.c today) and Q3's snapshot-entity concept don't exist in this
 * ABI. "Nothing pending" / "no more entities" are the correct answers,
 * not placeholders standing in for missing data.
 */
int trap_BotGetServerCommand(int clientNum, char *message, int size)
{
	(void)clientNum;
	if (size > 0) message[0] = '\0';
	return 0;
}

int trap_BotGetSnapshotEntity(int clientNum, int sequence)
{
	(void)clientNum;
	(void)sequence;
	return -1;
}

/* ================================================================== */
/*  Helpers with no existing real implementation anywhere in this repo. */
/* ================================================================== */

float AngleMod(float a)
{
	return (360.0 / 65536) * ((int)(a * (65536 / 360.0)) & 65535);
}

void vectoangles(const vec3_t value1, vec3_t angles)
{
	float forward, yaw, pitch;

	if (value1[1] == 0 && value1[0] == 0) {
		yaw = 0;
		pitch = (value1[2] > 0) ? 90 : 270;
	} else {
		if (value1[0]) {
			yaw = atan2(value1[1], value1[0]) * 180 / M_PI;
		} else if (value1[1] > 0) {
			yaw = 90;
		} else {
			yaw = 270;
		}
		if (yaw < 0) yaw += 360;

		forward = sqrt(value1[0] * value1[0] + value1[1] * value1[1]);
		pitch = atan2(value1[2], forward) * 180 / M_PI;
		if (pitch < 0) pitch += 360;
	}
	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

char *Q_CleanStr(char *string)
{
	char *d, *s;
	int c;

	s = string;
	d = string;
	while ((c = *s) != 0) {
		if (Q_IsColorString(s)) {
			s++;
		} else if (c >= 0x20 && c <= 0x7E) {
			*d++ = c;
		}
		s++;
	}
	*d = '\0';
	return string;
}

void ClientUserinfoChanged(int clientNum)
{
	/* Q2 already re-derives model/skin/etc. from userinfo through its own
	 * p_client.c path on the game.so side; botlib.so never has a real
	 * userinfo string to re-derive anything from in the first place
	 * (trap_GetUserinfo/trap_SetUserinfo are both no-ops above), so
	 * there's nothing for this to do here. */
	(void)clientNum;
}

int G_ModelIndex(char *name)
{
	/* Only reachable from BotSetEntityNumForGoalWithModel, deleted in
	 * Phase 0 as MISSIONPACK-only dead code -- kept as a link-safe stub
	 * in case anything is ever re-added that calls it. */
	(void)name;
	return 0;
}

void *G_Alloc(int size)
{
	void *p = malloc((size_t)size);
	if (p) memset(p, 0, (size_t)size);
	return p;
}

void G_CheckBotSpawn(void)
{
	/* Only reachable from BotAIStartFrame, deleted in Phase 0 -- Q2's own
	 * frame loop (game_q2/g_main.c) already drives bot spawning through
	 * its own, unrelated path. */
}

/*
 * ExitLevel -- forward-declared locally in game_q3/ai_main.c (line 94,
 * `void ExitLevel( void );`, no header), real Q3 body lives in
 * game/g_main.c (tournament-restart / force-reconnect logic) -- a
 * game.so-shaped concern with no Q2 equivalent, and this port's ai_main.c
 * now lives in botlib.so, not game.so. Only reachable from
 * BotInterbreeding() (ai_main.c), itself only ever called from the
 * now-deleted BotAIStartFrame -- unreachable dead code.
 *
 * Discovered the hard way: leaving this merely declared-but-undefined
 * compiles fine (nothing in this repo's build treats an unresolved
 * symbol in a .so as a link error), but real dlopen() on this platform
 * resolves every symbol eagerly and refuses to load a .so with ANY
 * unresolved non-weak symbol at all, dead code or not -- botlib.so
 * failed to load ("undefined symbol: ExitLevel") until this stub was
 * added. A trivial, never-actually-reached stub is all that's needed.
 */
void ExitLevel(void)
{
}

/* ================================================================== */
/*  Q3 order-parsing (ai_cmd.c) and voice-chat (ai_vcmd.c) subsystems   */
/*  are out of scope for this port (see the plan's Phase 3 notes), but  */
/*  ai_dmq3.c/ai_team.c each call one of their entry points directly,   */
/*  outside any MISSIONPACK guard, so they need real linkable symbols.  */
/* ================================================================== */

/*
 * BotMatchMessage -- called unconditionally from ai_dmq3.c's
 * BotCheckConsoleMessages. Q2 has no existing "typed order" input path,
 * so returning false is a graceful no-op, not a regression (matches the
 * plan's own Phase 3 recommendation; needed now because ai_dmq3.c
 * references it directly, not only ai_cmd.c/ai_vcmd.c).
 */
int BotMatchMessage(bot_state_t *bs, char *message)
{
	(void)bs;
	(void)message;
	return 0;
}

/*
 * BotVoiceChat_Defend -- ai_team.c's FindHumanTeamLeader calls this
 * directly (not inside any #ifdef MISSIONPACK block, unlike every other
 * voice-chat call site in these 5 files) when handing defense duty to a
 * newly-found human team leader. Its real body lives in ai_vcmd.c, which
 * this port does not compile (voice chat has no Q2 equivalent -- see
 * trap_EA_Command's existing "vtaunt" no-op). No-op stub.
 */
void BotVoiceChat_Defend(bot_state_t *bs, int client, int mode)
{
	(void)bs;
	(void)client;
	(void)mode;
}

/* ================================================================== */
/*  BotAI_ / helper functions, adapted from game_q2/bl_chat.c's Q2-side */
/*  implementations for botlib.so.                                     */
/*                                                                       */
/*  NOT here (deliberately): EasyClientName/ClientName/ClientSkin/       */
/*  TeamPlayIsOn/BotSameTeam/BotIsObserver/BotIsDead/BotIntermission/    */
/*  BotInLavaOrSlime/EntityIsDead/EntityIsInvisible/EntityIsShooting/    */
/*  BotEntityVisible/BotSynonymContext/BotEntityInfo -- ai_dmq3.c (and   */
/*  ai_main.c for BotEntityInfo) already define every one of these for   */
/*  real, routed entirely through trap_GetConfigstring/                 */
/*  trap_AAS_PointContents/trap_AAS_EntityInfo/BotAI_Trace, with zero    */
/*  g_entities/level dependency. ai_chat.c gets them for free via        */
/*  ordinary cross-TU linkage now that both live in botlib.so together.  */
/* ================================================================== */

void QDECL BotAI_Print(int type, char *fmt, ...)
{
	char str[2048];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(str, fmt, ap);
	va_end(ap);

	botimport.Print(type, "%s", str);
}

void BotAI_Trace(bsp_trace_t *bsptrace, vec3_t start, vec3_t mins, vec3_t maxs,
                  vec3_t end, int passent, int contentmask)
{
	/* botimport.Trace already produces a real bsp_trace_t directly --
	 * unlike Q3's game-side trap_Trace (which returns an engine trace_t
	 * that then has to be field-copied into bsp_trace_t), no translation
	 * step is needed here.
	 *
	 * Q2 port fix: every real call site passes a Q3-style client number
	 * (bs->client/bs->entitynum, almost always "myself" for self-exclusion)
	 * as passent, not a real Q2 entity number -- translate it the same way
	 * BotEntityInfo does (see ai_q2_compat.h's Q2_ClientNumToEntityNum).
	 * The two call sites that already have a genuinely-native passent
	 * (ai_dmq3.c's BotAimAtEnemy, which used to pass entinfo.number) were
	 * changed to pass the equivalent client number instead, so nothing
	 * needs to bypass this translation. */
	botimport.Trace(bsptrace, start, mins, maxs, end, Q2_ClientNumToEntityNum(passent), contentmask);
}

int BotAI_GetClientState(int clientNum, playerState_t *state)
{
	/* botlib.so has no per-client playerState_t feed in the frozen ABI --
	 * Q2BotUpdateClient's q2_bot_updateclient_t (be_interface_q2.c) is a
	 * reduced subset, not a real playerState_t, and there is no path to
	 * fetch one for an arbitrary Q2 client on demand.
	 *
	 * BUG FIX (root-caused via a live per-frame origin trace, see report):
	 * this used to unconditionally memset(state,0,...) here regardless of
	 * clientNum -- a Phase 0 placeholder that was meant to stay harmless
	 * only "since none of [the callers] are wired to run yet" (original
	 * comment). That stopped being true the moment Q2BotAI started
	 * calling the real BotAI() (game_q3/ai_main.c), which calls this as
	 * its very first act on every AI frame: `BotAI_GetClientState(client,
	 * &bs->cur_ps)`. Q2BotUpdateClient (be_interface_q2.c) had already
	 * written the bot's real origin into that exact same bs->cur_ps
	 * moments earlier this same frame -- so every bot's position was
	 * being zeroed straight back out before BotDeathmatchAI ever ran,
	 * every single frame, forever. Traced instrumentation confirmed the
	 * value was correct at every hop up to and including the top of
	 * BotAI(), then (0,0,0) immediately after this call returned.
	 *
	 * Fix: botstates[clientNum] (ai_main.c) already holds each active
	 * bot's own cur_ps, refreshed once per server frame by
	 * Q2BotUpdateClient -- serve that instead of a blind zero whenever
	 * the requested client is a live bot. For the "self" call in BotAI()
	 * above, state IS &botstates[clientNum]->cur_ps, so this is a safe
	 * self-copy that leaves the just-populated value intact. For queries
	 * about a DIFFERENT client who happens to also be a bot (ai_chat.c's
	 * ranking checks, ai_dmq3.c's EntityIsDead, ai_team.c's
	 * BotClientTravelTimeToGoal), it now returns that bot's own
	 * last-known state instead of always-absent zero -- strictly better,
	 * never worse. Real (human) clients and never-set-up slots have no
	 * bot_state_t at all, so they still correctly report "not found"
	 * exactly as before. */
	bot_state_t *bs;

	if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
		memset(state, 0, sizeof(*state));
		return 0;
	}

	bs = botstates[clientNum];
	if (!bs || !bs->inuse) {
		memset(state, 0, sizeof(*state));
		return 0;
	}

	if (state != &bs->cur_ps)
		memcpy(state, &bs->cur_ps, sizeof(*state));
	return 1;
}

int BotAI_GetEntityState(int entityNum, entityState_t *state)
{
	(void)entityNum;
	memset(state, 0, sizeof(*state));
	return 0;
}

int BotAI_GetSnapshotEntity(int clientNum, int sequence, entityState_t *state)
{
	(void)clientNum;
	(void)sequence;
	memset(state, 0, sizeof(*state));
	return -1;
}

void QDECL BotAI_BotInitialChat(bot_state_t *bs, char *type, ...)
{
	int i, mcontext;
	va_list ap;
	char *p;
	char *vars[MAX_MATCHVARIABLES];

	memset(vars, 0, sizeof(vars));
	va_start(ap, type);
	p = va_arg(ap, char *);
	for (i = 0; i < MAX_MATCHVARIABLES; i++) {
		if (!p) break;
		vars[i] = p;
		p = va_arg(ap, char *);
	}
	va_end(ap);

	mcontext = BotSynonymContext(bs);

	trap_BotInitialChat(bs->cs, type, mcontext,
	                    vars[0], vars[1], vars[2], vars[3],
	                    vars[4], vars[5], vars[6], vars[7]);
}
