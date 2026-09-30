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

/* Q3-shaped entity state, filled from the Q2 game's updates -- see
 * "Q3-shaped game state" below and ai_q2_compat.h. level stays zeroed. */
gentity_t          g_entities_compat[MAX_GENTITIES];
q2_level_compat_t  level;

/* the temporary event entities of the game state below, and the sound index
 * of Q3's powerup respawn sound, the one sound BotCheckEvents looks up
 * (trap_GetConfigstring) that this port produces */
#define Q2SHIM_TEMPENTITIES		32
#define Q2SHIM_FIRSTTEMP			(ENTITYNUM_MAX_NORMAL - Q2SHIM_TEMPENTITIES)
#define Q2SHIM_SOUND_POWERUPRESPAWN	(MAX_SOUNDS - 1)

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

/* Handles are 1-based: a vmCvar_t nobody registered yet (zeroed, handle 0)
 * must not read another cvar's slot -- Q2BotStartFrame updates the AI cvars
 * every frame, also before the first map load registers them. Registering a
 * name again (BotSetupDeathmatchAI does, on every map) reuses its slot. */
void trap_Cvar_Register(vmCvar_t *cv, char *var_name, char *value, int flags)
{
	int i;

	(void)flags;

	LibVar(var_name, value);

	if (!cv) return;

	for (i = 0; i < q2_shim_num_cvars; i++) {
		if (!strcmp(q2_shim_cvar_names[i], var_name))
			break;
	}
	if (i == q2_shim_num_cvars && q2_shim_num_cvars < MAX_Q2_SHIM_CVARS) {
		Q_strncpyz(q2_shim_cvar_names[i], var_name, sizeof(q2_shim_cvar_names[0]));
		q2_shim_num_cvars++;
	}
	cv->handle = (i < q2_shim_num_cvars) ? i + 1 : 0;
	cv->modificationCount = 0;
	Q2Shim_RefreshCvar(cv, var_name);
}

void trap_Cvar_Update(vmCvar_t *cv)
{
	if (!cv || cv->handle <= 0 || cv->handle > q2_shim_num_cvars) return;
	Q2Shim_RefreshCvar(cv, q2_shim_cvar_names[cv->handle - 1]);
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
 * trap_GetConfigstring -- ClientName/EasyClientName/ClientSkin/BotTeam/
 * BotIsObserver (ai_dmq3.c) and BotTeamplayReport/BotUpdateInfoConfigStrings
 * (ai_main.c) read player names and teams through this.
 *
 * botlib.so already caches a real per-bot netname -- BotSetChatName()
 * (be_ai_chat.c) stores it in each bot's own bot_chatstate_t.name[32],
 * written both at bot setup (the bot's character name) and on every
 * BotClientSettings call (the real in-game netname). For CS_PLAYERS+n,
 * synthesize a minimal "n\\<name>\\t\\<team>" info string from that
 * cache and the bot's CTF team (Q2_ClientTeam, be_interface_q2.c). A human
 * has no bs/chatstate: its name is the one the game last sent through
 * BotClientSettings (Q2_ClientNetname, be_interface_q2.c), as long as it is
 * in the game -- a spectator or a client that has left has no entity, and
 * like an empty Q3 configstring it then reads as no player at all. */
extern const char *Q2_ClientNetname(int client);
extern int Q2_ClientTeam(int client);

void trap_GetConfigstring(int num, char *buf, int size)
{
	int client;
	bot_state_t *bs;
	q2_chatstate_peek_t *cs;
	aas_entityinfo_t entinfo;

	if (size > 0) buf[0] = '\0';
	if (size <= 0) return;

	/* sounds: BotCheckEvents compares the names of global sounds; the only
	 * one this port produces is the powerup respawn (Q2Shim_UpdateEntity) */
	if (num >= CS_SOUNDS && num < CS_SOUNDS + MAX_SOUNDS) {
		if (num - CS_SOUNDS == Q2SHIM_SOUND_POWERUPRESPAWN)
			Q_strncpyz(buf, "sound/items/poweruprespawn.wav", size);
		return;
	}

	client = num - CS_PLAYERS;
	if (client < 0 || client >= MAX_CLIENTS) return;

	bs = botstates[client];
	if (!bs || !bs->inuse || bs->cs <= 0) {
		if (client >= maxclients || !Q2_ClientNetname(client)[0])
			return;
		AAS_EntityInfo(client, &entinfo);
		if (!entinfo.valid)
			return;
		snprintf(buf, size, "\\n\\%s\\t\\%d", Q2_ClientNetname(client), TEAM_FREE);
		return;
	}

	/* a bot without a name yet still has its team */
	cs = BotChatStateFromHandle(bs->cs);
	snprintf(buf, size, "\\n\\%s\\t\\%d", cs ? cs->name : "", Q2_ClientTeam(client));
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

/*
 * trap_SetUserinfo -- the botlib cannot write a Q2 client's userinfo. The
 * one key the game needs is the "sex" BotDeathmatchAI stores when a bot is
 * set up (ai_dmq3.c): Q2's obituaries read it as the "gender" key
 * (p_client.c IsFemale/IsNeutral), and without it every bot "blew itself
 * up". Gladiator's botlib sent the character's gender as the bot's
 * "gender" client command on its first frame (be_ai2_dmq2.c
 * BotDeathmatchAI), which game_q2/bl_cmd.c BotCmd puts into the userinfo;
 * send the same command. The other key, "teamtask", has no Q2 counterpart.
 */
void trap_SetUserinfo(int num, const char *buf)
{
	const char *sex = Info_ValueForKey(buf, "sex");

	if (*sex)
		trap_EA_Command(num, va("gender %s", sex));
}

void trap_SendConsoleCommand(int exec_when, char *text)
{
	/* Only reachable from BotInterbreeding() (ai_main.c), itself only
	 * ever called from the now-deleted BotAIStartFrame -- unreachable
	 * dead code kept link-safe. */
	(void)exec_when; (void)text;
}

/*
 * Q3 hands chat and prints to a bot as server commands ("chat", "tchat",
 * "print"), which BotAI (ai_main.c) queues into the bot's console. In this
 * port the game queues them into the console directly (BotConsoleMessage,
 * fed by game_q2/bl_redirgi.c; Q2BotConsoleMessage in be_interface_q2.c
 * brings chat into Q3's format), so no server command is ever pending.
 */
int trap_BotGetServerCommand(int clientNum, char *message, int size)
{
	(void)clientNum;
	if (size > 0) message[0] = '\0';
	return 0;
}

/* ================================================================== */
/*  Q3-shaped game state.                                              */
/*                                                                      */
/*  Q3's BotCheckSnapshot (ai_dmq3.c) walks the entities of the bot's   */
/*  last snapshot (trap_BotGetSnapshotEntity) and reads their states     */
/*  from g_entities[] (BotAI_GetEntityState, ai_main.c): BotCheckEvents  */
/*  keeps the obituary bookkeeping and reacts to teleports and to the    */
/*  powerup respawn sound, BotCheckForGrenades avoids grenades. Q2 sends */
/*  none of this in that shape, so be_interface_q2.c feeds g_entities[]  */
/*  here from what it does send: every entity update                     */
/*  (Q2Shim_UpdateEntity), every death (Q2Shim_Obituary) and a bot's own  */
/*  teleports and respawns (Q2Shim_TeleportIn). FindHumanTeamLeader       */
/*  (ai_team.c) reads the clients' entries as well.                      */
/*                                                                        */
/*  Numbers are Q3's: a client's entity is its client number (Q2 edict   */
/*  client+1), any other entity keeps its Q2 edict number (always above  */
/*  maxclients) -- the numbers the whole botlib uses (be_interface_q2.c  */
/*  Q2_EdictToEntity).                                                    */
/*  The temporary event entities Q3 spawns (G_TempEntity) take the        */
/*  numbers below ENTITYNUM_MAX_NORMAL and live EVENT_VALID_MSEC; a real  */
/*  Q2 entity numbered that high is left out.                            */
/* ================================================================== */

/* Q2 entity events (game_q2/q_shared.h entity_event_t) and effects bits */
#define Q2EV_ITEM_RESPAWN		1
#define Q2EV_PLAYER_TELEPORT	6
#define Q2EF_GRENADE			0x00000020

static float	q2shim_tempexpire[Q2SHIM_TEMPENTITIES];
static int		q2shim_nexttemp;
static int		q2shim_eventtime;	/* stands in for level.time in eventTime */
static float	q2shim_lastframetime;

static int		q2shim_snapshot[MAX_GENTITIES];
static int		q2shim_snapshotcount;
static int		q2shim_snapshotclient = -1;

/* The powerups whose respawn Q3 announces to everyone with
 * "sound/items/poweruprespawn.wav" (items of type IT_POWERUP there): the Q2
 * and mission pack ones in their role. BotGoForPowerups (ai_dmq3.c) stops
 * avoiding the same items. */
static char *q2shim_powerups[] = {
	"Quad Damage", "Invulnerability", "DualFire Damage", "Double Damage", NULL
};

/* Every client entity's gclient, as in Q3, for what the hit chats (ai_chat.c)
 * read there: who hurt the client last and how (Q2Shim_ClientHurt). */
static q2_gclient_compat_t q2shim_clients[MAX_CLIENTS];

void Q2Shim_Reset(void)
{
	int i;

	Com_Memset(g_entities_compat, 0, sizeof(g_entities_compat));
	Com_Memset(q2shim_clients, 0, sizeof(q2shim_clients));
	for (i = 0; i < MAX_CLIENTS; i++)
		g_entities_compat[i].client = &q2shim_clients[i];
	Com_Memset(q2shim_tempexpire, 0, sizeof(q2shim_tempexpire));
	q2shim_lastframetime = 0;
	q2shim_snapshotclient = -1;
}

/* What the game told of the last damage a client took (be_interface_q2.c
 * Q2BotUpdateClient; bots only, a human's stays zero). */
void Q2Shim_ClientHurt(int client, int attacker, int mod)
{
	if (client < 0 || client >= MAX_CLIENTS)
		return;
	q2shim_clients[client].lasthurt_client = attacker;
	q2shim_clients[client].lasthurt_mod = mod;
}

/* A bot that moves to another client slot (BotMoveClient) takes it along. */
void Q2Shim_MoveClient(int oldclnum, int newclnum)
{
	if (oldclnum < 0 || oldclnum >= MAX_CLIENTS || newclnum < 0 || newclnum >= MAX_CLIENTS)
		return;
	q2shim_clients[newclnum] = q2shim_clients[oldclnum];
	Com_Memset(&q2shim_clients[oldclnum], 0, sizeof(q2shim_clients[oldclnum]));
}

/* Called once per server frame before the game sends its entity updates. */
void Q2Shim_StartFrame(void)
{
	float now = AAS_Time();
	int i;

	/* a new level starts its clock at zero */
	if (now < q2shim_lastframetime)
		Q2Shim_Reset();
	q2shim_lastframetime = now;
	for (i = 0; i < Q2SHIM_FIRSTTEMP; i++) {
		g_entities_compat[i].inuse = false;
		g_entities_compat[i].r.linked = false;
	}
	for (i = 0; i < Q2SHIM_TEMPENTITIES; i++) {
		if (q2shim_tempexpire[i] <= now) {
			g_entities_compat[Q2SHIM_FIRSTTEMP + i].inuse = false;
			g_entities_compat[Q2SHIM_FIRSTTEMP + i].r.linked = false;
		}
	}
}

static gentity_t *Q2Shim_TempEvent(int event, vec3_t origin, int broadcast)
{
	int slot = q2shim_nexttemp;
	gentity_t *ent = &g_entities_compat[Q2SHIM_FIRSTTEMP + slot];

	q2shim_nexttemp = (slot + 1) % Q2SHIM_TEMPENTITIES;
	Com_Memset(ent, 0, sizeof(*ent));
	ent->inuse = true;
	ent->r.linked = true;
	ent->r.svFlags = broadcast ? SVF_BROADCAST : 0;
	ent->s.number = Q2SHIM_FIRSTTEMP + slot;
	ent->s.eType = ET_EVENTS + event;
	if (origin) {
		/* G_TempEntity's G_SetOrigin: s.origin stays zero, as in Q3. The
		 * EV_PLAYER_TELEPORT_IN handler (BotCheckEvents) reads s.origin, so
		 * BotFindEnemy's teleport grace (ignore players near the last
		 * teleport-in spot for 3 s) only ever holds at the map origin -- in
		 * Q3 too. Given the spot, bots held fire at every respawn and
		 * teleporter exit and played no better for it. */
		VectorCopy(origin, ent->r.currentOrigin);
		VectorCopy(origin, ent->s.pos.trBase);
	}
	ent->eventTime = ++q2shim_eventtime;
	q2shim_tempexpire[slot] = AAS_Time() + EVENT_VALID_MSEC * 0.001f;
	return ent;
}

/* Q3's G_Obituary: target and attacker are client numbers, attacker
 * ENTITYNUM_WORLD when no client killed. Broadcast, like Q3's. */
void Q2Shim_Obituary(int target, int attacker, int mod)
{
	gentity_t *ent = Q2Shim_TempEvent(EV_OBITUARY, NULL, true);

	ent->s.otherEntityNum = target;
	ent->s.otherEntityNum2 = attacker;
	ent->s.eventParm = mod;
}

/* Q3's respawn() and TeleportPlayer: a player appears at origin. */
void Q2Shim_TeleportIn(int client, vec3_t origin)
{
	gentity_t *ent = Q2Shim_TempEvent(EV_PLAYER_TELEPORT_IN, origin, false);

	ent->s.clientNum = client;
}

static int Q2Shim_IsPowerup(int entnum)
{
	bot_goal_t goal;
	int i, index;

	for (i = 0; q2shim_powerups[i]; i++) {
		for (index = BotGetLevelItemGoal(-1, q2shim_powerups[i], &goal); index >= 0;
			 index = BotGetLevelItemGoal(index, q2shim_powerups[i], &goal)) {
			if (goal.entitynum == entnum)
				return true;
		}
	}
	return false;
}

/* One entity of this frame, as the game sent it (Q2BotUpdateEntity). type,
 * eflags and powerups are the Q3 values the adapter derived for AAS. */
void Q2Shim_UpdateEntity(int q2ent, int type, int eflags, int powerups,
		vec3_t origin, vec3_t mins, vec3_t maxs, int modelindex,
		int effects, int q2event)
{
	bot_state_t *bs = NULL;
	gentity_t *ent;
	int num, client = -1;

	/* the world is ENTITYNUM_WORLD, never an entity of a snapshot */
	if (q2ent <= 0) return;
	if (q2ent <= maxclients) {
		client = q2ent - 1;
		num = client;
		bs = botstates[client];
		if (bs && !bs->inuse) bs = NULL;
	}
	else {
		num = q2ent;
	}
	if (num >= Q2SHIM_FIRSTTEMP) return;
	ent = &g_entities_compat[num];
	ent->inuse = true;
	ent->r.linked = true;
	ent->r.svFlags = bs ? SVF_BOT : 0;
	VectorCopy(origin, ent->r.currentOrigin);
	VectorCopy(mins, ent->r.mins);
	VectorCopy(maxs, ent->r.maxs);
	ent->s.number = num;
	ent->s.eType = type;
	ent->s.eFlags = eflags;
	ent->s.powerups = powerups;
	VectorCopy(origin, ent->s.pos.trBase);
	VectorCopy(origin, ent->s.origin);
	ent->s.modelindex = modelindex;
	ent->s.clientNum = (client >= 0) ? client : 0;
	/* Q2 marks both grenade kinds, launched and thrown, with EF_GRENADE */
	ent->s.weapon = (type == ET_MISSILE && (effects & Q2EF_GRENADE)) ? WP_GRENADE_LAUNCHER : 0;
	ent->s.event = 0;
	ent->s.eventParm = 0;
	switch (q2event) {
	case Q2EV_PLAYER_TELEPORT:
		/* the teleport splash on a player that respawned or teleported, a
		 * temporary entity in Q3 as well; the one on the source pad is Q3's
		 * EV_PLAYER_TELEPORT_OUT, which bots ignore. A bot's own comes from
		 * its player state instead (see Q2BotUpdateClient): the game runs it
		 * after this update. */
		if (client >= 0 && !bs)
			Q2Shim_TeleportIn(client, origin);
		break;
	case Q2EV_ITEM_RESPAWN:
		if (type == ET_ITEM) {
			ent->s.event = EV_ITEM_RESPAWN;
			if (Q2Shim_IsPowerup(q2ent)) {
				gentity_t *te = Q2Shim_TempEvent(EV_GLOBAL_SOUND, NULL, true);
				te->s.eventParm = Q2SHIM_SOUND_POWERUPRESPAWN;
			}
		}
		break;
	}
	if (ent->s.event)
		ent->eventTime = ++q2shim_eventtime;
}

/* Q3's snapshot of a client: the entities in the PVS of its view, and the
 * broadcast ones, never its own. BotCheckSnapshot asks for sequence 0 first. */
static void Q2Shim_BuildSnapshot(int clientNum)
{
	bot_state_t *bs = NULL;
	gentity_t *ent;
	vec3_t eye;
	int i;

	q2shim_snapshotcount = 0;
	q2shim_snapshotclient = clientNum;
	if (clientNum >= 0 && clientNum < MAX_CLIENTS)
		bs = botstates[clientNum];
	if (!bs || !bs->inuse)
		return;
	VectorCopy(bs->cur_ps.origin, eye);
	eye[2] += bs->cur_ps.viewheight;
	for (i = 0; i < MAX_GENTITIES; i++) {
		ent = &g_entities_compat[i];
		if (!ent->inuse || !ent->r.linked) continue;
		if (ent->r.svFlags & SVF_NOCLIENT) continue;
		if (i == clientNum) continue;
		if (!(ent->r.svFlags & SVF_BROADCAST) && !botimport.inPVS(eye, ent->r.currentOrigin))
			continue;
		q2shim_snapshot[q2shim_snapshotcount++] = i;
	}
}

int trap_BotGetSnapshotEntity(int clientNum, int sequence)
{
	if (sequence == 0 || clientNum != q2shim_snapshotclient)
		Q2Shim_BuildSnapshot(clientNum);
	if (sequence < 0 || sequence >= q2shim_snapshotcount)
		return -1;
	return q2shim_snapshot[sequence];
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
	angles[0] = -pitch;
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
	 * step is needed here; the adapter's trace takes and gives Q3 entity
	 * numbers (be_interface_q2.c Q3Trace_Adapter). */
	botimport.Trace(bsptrace, start, mins, maxs, end, passent, contentmask);
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
	 * never worse.
	 *
	 * Human clients have no bot_state_t. For them, the two fields the ported
	 * code reads about OTHER clients -- origin (ai_team.c's
	 * BotClientTravelTimeToGoal) and pm_type (ai_dmq3.c's EntityIsDead) --
	 * are filled in from the client's own AAS entity, which game_q2/g_main.c
	 * updates every frame; Q2BotUpdateEntity (be_interface_q2.c) marks a dead
	 * player's entity EF_DEAD. Without this, a dead human always read as
	 * PM_NORMAL, so bots kept fighting the corpse until it respawned. An
	 * empty slot (no valid entity) still reports "not found". */
	bot_state_t *bs;
	aas_entityinfo_t entinfo;

	if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
		memset(state, 0, sizeof(*state));
		return 0;
	}

	bs = botstates[clientNum];
	if (bs && bs->inuse) {
		if (state != &bs->cur_ps)
			memcpy(state, &bs->cur_ps, sizeof(*state));
		return 1;
	}

	memset(state, 0, sizeof(*state));
	if (clientNum >= maxclients)
		return 0;
	AAS_EntityInfo(clientNum, &entinfo);
	if (!entinfo.valid)
		return 0;
	VectorCopy(entinfo.origin, state->origin);
	state->pm_type = (entinfo.flags & EF_DEAD) ? PM_DEAD : PM_NORMAL;
	return 1;
}

/* BotAI_GetEntityState, BotAI_GetSnapshotEntity and BotAI_BotInitialChat are
 * id's, in game_q3/ai_main.c; the game state they read is above. */
