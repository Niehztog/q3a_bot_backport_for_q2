/*
 * bl_chat.c -- Q2 dispatch layer for Q3's ai_chat.c
 *
 * This file provides:
 *   1. Implementations of all trap_* / BotAI_* / helper functions
 *      declared in ai_chat_q2.h (the compat shim for Q3 ai_chat.c).
 *   2. BotChat_PopulateState() -- fills a bot_state_t from Q2 entity data.
 *   3. BotChat_NotifyDeath()  -- hook called from p_client.c on deaths.
 *   4. BotChat_Frame()        -- per-frame random chat checks.
 *
 * ai_chat.c (compiled from game_q3/) calls these functions; bl_chat.c
 * routes them through the Q2 botlib export table.
 */

#include "g_local.h"
#include "bl_main.h"
#include "ai_chat_q2.h"
#include "bl_chat.h"

/* Q3 ai_chat.c functions (defined in game_q3/ai_chat.c, linked in) */
extern int BotChat_EnterGame(bot_state_t *bs);
extern int BotChat_ExitGame(bot_state_t *bs);
extern int BotChat_StartLevel(bot_state_t *bs);
extern int BotChat_EndLevel(bot_state_t *bs);
extern int BotChat_Death(bot_state_t *bs);
extern int BotChat_Kill(bot_state_t *bs);
extern int BotChat_EnemySuicide(bot_state_t *bs);
extern int BotChat_HitTalking(bot_state_t *bs);
extern int BotChat_HitNoDeath(bot_state_t *bs);
extern int BotChat_HitNoKill(bot_state_t *bs);
extern int BotChat_Random(bot_state_t *bs);

/* ================================================================== */
/*  Global variables (declared extern in ai_chat_q2.h)                */
/* ================================================================== */

vmCvar_t      bot_nochat  = {0};
vmCvar_t      bot_fastchat = {0};
int           gametype = GT_FFA;     /* updated from ctf cvar */
bot_library_t *chat_lib = NULL;      /* set before each call into ai_chat.c */

/* ================================================================== */
/*  Internal helpers                                                  */
/* ================================================================== */

/* Get the bot library pointer for a client entity. */
static bot_library_t *BotGetLib(edict_t *bot)
{
	int cl = DF_ENTCLIENT(bot);
	if (cl < 0 || cl >= game.maxclients) return NULL;
	if (!botglobals.botstates[cl].active) return NULL;
	return botglobals.botstates[cl].library;
}

/* Update vmCvar_t globals from Q2 cvar system. */
static void BotChat_UpdateCvars(void)
{
	cvar_t *cv;

	cv = gi.cvar("nochat", "0", 0);
	bot_nochat.integer = cv ? (int)cv->value : 0;

	cv = gi.cvar("fastchat", "0", 0);
	bot_fastchat.integer = cv ? (int)cv->value : 0;

	/* Detect game mode: CTF, team DM (skin/model teams), or FFA.
	 * Q2 team DM uses dmflags bits DF_MODELTEAMS (128) or DF_SKINTEAMS (64).
	 * Rocket Arena uses the "arena" cvar. */
	cv = gi.cvar("ctf", "0", 0);
	if (cv && cv->value) {
		gametype = GT_CTF;
	} else {
		cvar_t *df = gi.cvar("dmflags", "0", 0);
		cvar_t *ar = gi.cvar("arena", "0", 0);
		if ((df && ((int)df->value & (64 | 128))) /* DF_SKINTEAMS | DF_MODELTEAMS */
		    || (ar && ar->value)) {
			gametype = GT_TEAM;
		} else {
			gametype = GT_FFA;
		}
	}
}

/* ================================================================== */
/*  trap_* implementations (called by ai_chat.c)                      */
/* ================================================================== */

int trap_Cvar_VariableIntegerValue(const char *name)
{
	/* Q3 calls this "sv_maxclients"; Q2 calls it "maxclients" */
	if (strcmp(name, "sv_maxclients") == 0)
		name = "maxclients";
	cvar_t *cv = gi.cvar((char *)name, "0", 0);
	return cv ? (int)cv->value : 0;
}

void trap_GetConfigstring(int num, char *buf, int size)
{
	int client = num - CS_PLAYERS;

	if (client >= 0 && client < game.maxclients) {
		edict_t *ent = &g_edicts[client + 1];
		if (ent->inuse && ent->client)
			snprintf(buf, size, "\\n\\%s\\t\\0",
			         ent->client->pers.netname);
		else
			buf[0] = '\0';
	} else {
		buf[0] = '\0';
	}
}

void trap_GetServerinfo(char *buf, int size)
{
	snprintf(buf, size, "\\mapname\\%s", level.mapname);
}

int trap_PointContents(vec3_t point, int entnum)
{
	(void)entnum;
	return gi.pointcontents(point);
}

void trap_AAS_PresenceTypeBoundingBox(int ptype, vec3_t mins, vec3_t maxs)
{
	(void)ptype;
	/* Q2 crouch bounding box */
	mins[0] = -16; mins[1] = -16; mins[2] = -24;
	maxs[0] =  16; maxs[1] =  16; maxs[2] =   4;
}

void trap_EA_Command(int client, char *cmd)
{
	/* vtaunt / voice commands don't exist in Q2; silently ignore. */
	(void)client;
	(void)cmd;
}

float trap_Characteristic_BFloat(int ch, int idx, float min, float max)
{
	if (!chat_lib) return 0;
	return chat_lib->funcs.BotCharacterBFloat(ch, idx, min, max);
}

int trap_Characteristic_BInteger(int ch, int idx, int min, int max)
{
	if (!chat_lib) return 0;
	return chat_lib->funcs.BotCharacterBInteger(ch, idx, min, max);
}

int trap_BotNumInitialChats(int cs, char *type)
{
	if (!chat_lib) return 0;
	return chat_lib->funcs.BotNumInitialChatsFunc(cs, type);
}

void trap_BotEnterChat(int cs, int client, int sendto)
{
	if (!chat_lib) return;
	chat_lib->funcs.BotEnterChatFunc(cs, client, sendto);
}

int trap_BotChatLength(int cs)
{
	if (!chat_lib) return 0;
	return chat_lib->funcs.BotChatLengthFunc(cs);
}

/* ================================================================== */
/*  BotAI_* implementations                                           */
/* ================================================================== */

void BotAI_BotInitialChat(bot_state_t *bs, char *type, ...)
{
	int      i, mcontext;
	va_list  ap;
	char    *p;
	char    *vars[8];

	for (i = 0; i < 8; i++)
		vars[i] = NULL;

	va_start(ap, type);
	for (i = 0; i < 8; i++) {
		p = va_arg(ap, char *);
		if (!p) break;
		vars[i] = p;
	}
	va_end(ap);

	mcontext = BotSynonymContext(bs);

	if (chat_lib)
		chat_lib->funcs.BotInitialChatFunc(bs->cs, type, mcontext,
			vars[0], vars[1], vars[2], vars[3],
			vars[4], vars[5], vars[6], vars[7]);
}

void BotAI_Trace(bsp_trace_t *trace, vec3_t start, vec3_t mins,
                 vec3_t maxs, vec3_t end, int passent, int contentmask)
{
	trace_t  t;
	edict_t *passedict = NULL;

	if (passent >= 0 && passent < game.maxclients)
		passedict = &g_edicts[passent + 1];

	t = gi.trace(start, mins, maxs, end, passedict, contentmask);

	memset(trace, 0, sizeof(*trace));
	trace->allsolid   = t.allsolid;
	trace->startsolid = t.startsolid;
	trace->fraction   = t.fraction;
	VectorCopy(t.endpos, trace->endpos);
	trace->contents   = t.contents;

	/* Convert edict pointer to entity number.
	 * World entity (g_edicts[0]) or NULL both map to 0 = ENTITYNUM_WORLD. */
	if (t.ent && t.ent != g_edicts)
		trace->ent = (int)(t.ent - g_edicts);
	else
		trace->ent = ENTITYNUM_WORLD;
}

int BotAI_GetClientState(int clientNum, playerState_t *state)
{
	edict_t *ent;

	if (clientNum < 0 || clientNum >= game.maxclients)
		return false;

	ent = &g_edicts[clientNum + 1];
	if (!ent->inuse || !ent->client)
		return false;

	memset(state, 0, sizeof(*state));
	state->persistant[PERS_SCORE] = ent->client->resp.score;
	return true;
}

/* ================================================================== */
/*  Helper functions (called by ai_chat.c)                            */
/* ================================================================== */

char *EasyClientName(int client, char *name, int size)
{
	edict_t *ent;

	if (client < 0 || client >= game.maxclients) {
		name[0] = '\0';
		return name;
	}
	ent = &g_edicts[client + 1];
	if (!ent->inuse || !ent->client) {
		name[0] = '\0';
		return name;
	}
	strncpy(name, ent->client->pers.netname, size - 1);
	name[size - 1] = '\0';
	return name;
}

char *ClientName(int client, char *name, int size)
{
	return EasyClientName(client, name, size);
}

int TeamPlayIsOn(void)
{
	return (gametype >= GT_TEAM);
}

int BotSameTeam(bot_state_t *bs, int client)
{
	edict_t *bot_ent, *other_ent;

	if (!TeamPlayIsOn()) return 0;

	bot_ent = &g_edicts[bs->client + 1];
	if (!bot_ent->inuse || !bot_ent->client) return 0;

	if (client < 0 || client >= game.maxclients) return 0;
	other_ent = &g_edicts[client + 1];
	if (!other_ent->inuse || !other_ent->client) return 0;

	return (bot_ent->client->resp.ctf_team != CTF_NOTEAM &&
	        bot_ent->client->resp.ctf_team == other_ent->client->resp.ctf_team);
}

int BotIsObserver(bot_state_t *bs)
{
	(void)bs;
	return 0;
}

int BotIsDead(bot_state_t *bs)
{
	edict_t *ent = &g_edicts[bs->client + 1];
	return (!ent->inuse || ent->health <= 0 || ent->deadflag);
}

void BotEntityInfo(int entnum, aas_entityinfo_t *info)
{
	edict_t *ent;

	memset(info, 0, sizeof(*info));
	if (entnum < 0 || entnum >= game.maxclients) return;

	ent = &g_edicts[entnum + 1];
	if (!ent->inuse) return;

	info->valid  = 1;
	info->number = entnum;
	VectorCopy(ent->s.origin, info->origin);
}

int EntityIsDead(aas_entityinfo_t *info)
{
	edict_t *ent;
	if (!info->valid) return 1;
	ent = &g_edicts[info->number + 1];
	return (!ent->inuse || ent->health <= 0 || ent->deadflag);
}

int EntityIsInvisible(aas_entityinfo_t *info)
{
	(void)info;
	return 0;
}

int EntityIsShooting(aas_entityinfo_t *info)
{
	(void)info;
	return 0;
}

float BotEntityVisible(int viewer, vec3_t eye, vec3_t viewangles,
                       float fov, int ent)
{
	trace_t  tr;
	edict_t *target;
	edict_t *source;

	(void)viewangles;
	(void)fov;

	target = &g_edicts[ent + 1];
	source = &g_edicts[viewer + 1];
	if (!target->inuse || !source->inuse)
		return 0;

	tr = gi.trace(eye, NULL, NULL, target->s.origin, source, MASK_OPAQUE);
	if (tr.fraction >= 1.0f || tr.ent == target)
		return 1.0f;

	return 0;
}

int BotSynonymContext(bot_state_t *bs)
{
	(void)bs;
	return CONTEXT_NORMAL | CONTEXT_NEARBYITEM | CONTEXT_NAMES;
}

float FloatTime(void)
{
	return level.time;
}

/* ================================================================== */
/*  State population                                                  */
/* ================================================================== */

/*
 * Fill a bot_state_t from Q2 entity data and botlib handles.
 * Called before every ai_chat.c function invocation.
 */
static void BotChat_PopulateState(bot_state_t *bs, edict_t *bot,
                                  bot_library_t *lib)
{
	int cl = DF_ENTCLIENT(bot);

	memset(bs, 0, sizeof(*bs));

	bs->client    = cl;
	bs->entitynum = cl;
	bs->thinktime = FRAMETIME;

	VectorCopy(bot->s.origin, bs->origin);
	VectorCopy(bot->s.origin, bs->eye);
	bs->eye[2] += bot->viewheight;
	if (bot->client)
		VectorCopy(bot->client->v_angle, bs->viewangles);

	/* Inventory -- copy the full array so BotValidChatPosition works */
	if (bot->client)
		memcpy(bs->inventory, bot->client->pers.inventory,
		       sizeof(bs->inventory));

	/* Death/kill info from gclient_s lasthurt fields.
	 * lasthurt_client is now stored as a client number (0-based)
	 * by g_combat.c. */
	if (bot->client) {
		bs->lastkilledby  = bot->client->lasthurt_client;
		bs->botdeathtype  = bot->client->lasthurt_mod;
		bs->botsuicide    = (bs->lastkilledby == cl);
	}
	bs->lastkilledplayer = -1;  /* filled by caller when relevant */
	bs->enemydeathtype   = 0;
	bs->enemy            = lib->funcs.BotGetEnemy(cl);
	bs->ltgtype          = 0;   /* no long-term goals in Q2 */

	/* Score */
	if (bot->client)
		bs->cur_ps.persistant[PERS_SCORE] = bot->client->resp.score;

	/* Botlib handles */
	bs->character = lib->funcs.BotGetCharacter(cl);
	bs->cs        = lib->funcs.BotGetChatState(cl);

	/* Chat cooldown */
	bs->lastchat_time = lib->funcs.BotGetLastChatTime(cl);

	bs->chatto = CHAT_ALL;
}

/* ================================================================== */
/*  Dispatch hooks (public API, called from p_client.c / bl_main.c)   */
/* ================================================================== */

/*
 * BotChat_NotifyDeath -- called from player_die() in p_client.c.
 *
 * Populates bot_state_t and calls Q3's BotChat_Death / BotChat_Kill /
 * BotChat_EnemySuicide as appropriate.
 */
void BotChat_NotifyDeath(edict_t *self, edict_t *attacker, int mod)
{
	int victim_cl = DF_ENTCLIENT(self);
	int killer_cl = (attacker && attacker->client)
	                ? DF_ENTCLIENT(attacker) : -1;
	qboolean victim_is_bot = (self->flags & FL_BOT) ? true : false;
	qboolean killer_is_bot = (attacker && (attacker->flags & FL_BOT))
	                         ? true : false;
	bot_library_t *lib;
	bot_state_t    bs;

	BotChat_UpdateCvars();

	/* --- Victim is a bot: trigger death chat --- */
	if (victim_is_bot) {
		lib = BotGetLib(self);
		if (lib) {
			chat_lib = lib;

			/* Notify botlib of the death (updates internal state) */
			lib->funcs.BotNotifyDeath(victim_cl, killer_cl, mod);

			BotChat_PopulateState(&bs, self, lib);
			bs.lastkilledby = killer_cl;
			bs.botdeathtype = mod;
			bs.botsuicide   = (killer_cl == victim_cl || killer_cl < 0);

			if (BotChat_Death(&bs)) {
				trap_BotEnterChat(bs.cs, 0, bs.chatto);
				lib->funcs.BotSetLastChatTime(victim_cl, FloatTime());
			}

			chat_lib = NULL;
		}
	}

	/* --- Killer is a bot (and not self-kill): trigger kill chat --- */
	if (killer_is_bot && attacker != self) {
		lib = BotGetLib(attacker);
		if (lib) {
			chat_lib = lib;

			lib->funcs.BotNotifyKill(killer_cl, victim_cl, mod);

			BotChat_PopulateState(&bs, attacker, lib);
			bs.lastkilledplayer = victim_cl;
			bs.enemydeathtype   = mod;

			if (victim_cl == killer_cl || killer_cl < 0) {
				/* Victim suicided */
				bs.enemy = victim_cl;
				if (BotChat_EnemySuicide(&bs)) {
					trap_BotEnterChat(bs.cs, 0, bs.chatto);
					lib->funcs.BotSetLastChatTime(killer_cl,
					                              FloatTime());
				}
			} else {
				if (BotChat_Kill(&bs)) {
					trap_BotEnterChat(bs.cs, 0, bs.chatto);
					lib->funcs.BotSetLastChatTime(killer_cl,
					                              FloatTime());
				}
			}

			chat_lib = NULL;
		}
	}

	/* --- Non-bot suicide: notify observing bots --- */
	if (!victim_is_bot &&
	    (victim_cl == killer_cl || killer_cl < 0)) {
		int i;
		for (i = 0; i < game.maxclients; i++) {
			edict_t *ent = &g_edicts[i + 1];
			if (!(ent->flags & FL_BOT) || !ent->inuse) continue;
			lib = BotGetLib(ent);
			if (!lib) continue;
			if (lib->funcs.BotGetEnemy(i) == victim_cl) {
				chat_lib = lib;

				BotChat_PopulateState(&bs, ent, lib);
				bs.enemy = victim_cl;
				bs.lastkilledplayer = victim_cl;

				if (BotChat_EnemySuicide(&bs)) {
					trap_BotEnterChat(bs.cs, 0, bs.chatto);
					lib->funcs.BotSetLastChatTime(i,
					                              FloatTime());
				}

				chat_lib = NULL;
			}
		}
	}
}

static void BotChat_CheckReply(edict_t *bot, bot_state_t *bs, bot_library_t *lib);

/*
 * BotChat_Frame -- per-frame chat checks (random chat, hit reactions).
 * Called from bl_main.c for each active bot.
 */
void BotChat_Frame(edict_t *bot)
{
	bot_library_t *lib;
	bot_state_t    bs;

	if (!(bot->flags & FL_BOT)) return;
	if (!bot->inuse || !bot->client) return;
	if (bot->health <= 0 || bot->deadflag) return;

	lib = BotGetLib(bot);
	if (!lib) return;

	BotChat_UpdateCvars();
	chat_lib = lib;

	BotChat_PopulateState(&bs, bot, lib);

	/* Random chat */
	if (BotChat_Random(&bs)) {
		trap_BotEnterChat(bs.cs, 0, bs.chatto);
		lib->funcs.BotSetLastChatTime(bs.client, FloatTime());
	}

	/* HitNoDeath -- bot was hurt but is alive */
	if (bot->client->lasthurt_mod && bot->client->damage_blood > 0) {
		if (BotChat_HitNoDeath(&bs)) {
			trap_BotEnterChat(bs.cs, 0, bs.chatto);
			lib->funcs.BotSetLastChatTime(bs.client, FloatTime());
		}
	}

	/* HitNoKill -- bot hit an enemy but didn't kill them */
	if (bs.enemy >= 0 && bs.enemy < game.maxclients) {
		edict_t *enemy_ent = &g_edicts[bs.enemy + 1];
		if (enemy_ent->inuse && enemy_ent->health > 0 &&
		    enemy_ent->client && enemy_ent->client->damage_blood > 0) {
			if (BotChat_HitNoKill(&bs)) {
				trap_BotEnterChat(bs.cs, 0, bs.chatto);
				lib->funcs.BotSetLastChatTime(bs.client, FloatTime());
			}
		}
	}

	/* Reply to player chat messages in the queue */
	BotChat_CheckReply(bot, &bs, lib);

	chat_lib = NULL;
}

/*
 * BotChat_CheckReply -- check the bot's console message queue for player
 * chat and generate a reply if appropriate.  Called from BotChat_Frame.
 */
static void BotChat_CheckReply(edict_t *bot, bot_state_t *bs, bot_library_t *lib)
{
	bot_consolemessage_t m;
	int handle;
	char botname[MAX_NETNAME];
	size_t namelen;
	float chat_reply;

	if (!lib->funcs.BotNextConsoleMessageFunc) return;
	if (!lib->funcs.BotReplyChatFunc) return;
	if (!lib->funcs.BotRemoveConsoleMessageFunc) return;

	EasyClientName(bs->client, botname, sizeof(botname));
	namelen = strlen(botname);

	while ((handle = lib->funcs.BotNextConsoleMessageFunc(bs->cs, &m)) != 0) {
		if (m.type == CMS_CHAT) {
			/* Skip recently-added messages — give the bot ~1 second to "read" */
			if (m.time > FloatTime() - 1.0f) break;

			/* Skip messages from ourselves (broadcast echo) */
			if (namelen > 0 && strncmp(m.message, botname, namelen) == 0
			    && m.message[namelen] == ':') {
				lib->funcs.BotRemoveConsoleMessageFunc(bs->cs, handle);
				continue;
			}

			/* Skip messages we already replied to (formatted duplicates
			 * that contain "Name: msg" when we already saw raw "msg") */

			if (!BotValidChatPosition(bs) || bot_nochat.integer) {
				lib->funcs.BotRemoveConsoleMessageFunc(bs->cs, handle);
				continue;
			}

			chat_reply = trap_Characteristic_BFloat(bs->character,
			                                        CHARACTERISTIC_CHAT_REPLY,
			                                        0, 1);
			if (random() < chat_reply) {
				if (lib->funcs.BotReplyChatFunc(bs->cs, m.message,
				        CONTEXT_NORMAL, CONTEXT_NORMAL,
				        NULL, NULL, NULL, NULL, NULL, NULL,
				        botname, NULL)) {
					lib->funcs.BotRemoveConsoleMessageFunc(bs->cs, handle);
					trap_BotEnterChat(bs->cs, 0, CHAT_ALL);
					lib->funcs.BotSetLastChatTime(bs->client, FloatTime());
					break;
				}
			}
		}
		lib->funcs.BotRemoveConsoleMessageFunc(bs->cs, handle);
	}
}

/* ================================================================== */
/*  Enter/Exit wrappers (called from bl_main.c with edict_t *)        */
/* ================================================================== */

/*
 * BotChat_OnPlayerSay -- called from Cmd_Say_f when a non-bot player says
 * something.  Queues the message into every active bot's console message
 * queue so BotChat_CheckReply can generate a reply on the next frame.
 *
 * 'msg' should be the raw message text (without sender name prefix).
 */
void BotChat_OnPlayerSay(edict_t *sender, const char *msg)
{
	int i;
	char buf[MAX_MESSAGE_SIZE];

	if (!msg || !msg[0]) return;

	/* Truncate to fit */
	strncpy(buf, msg, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	for (i = 0; i < game.maxclients; i++) {
		edict_t       *ent = &g_edicts[i + 1];
		bot_library_t *lib;

		if (ent == sender) continue;
		if (!(ent->flags & FL_BOT) || !ent->inuse || !ent->client) continue;

		lib = BotGetLib(ent);
		if (!lib) continue;
		if (!lib->funcs.BotConsoleMessage) continue;

		lib->funcs.BotConsoleMessage(i, CMS_CHAT, buf);
	}
}

void BotChat_OnEnterGame(edict_t *bot)
{
	bot_library_t *lib;
	bot_state_t    bs;

	if (!(bot->flags & FL_BOT)) return;
	if (!bot->inuse || !bot->client) return;

	lib = BotGetLib(bot);
	if (!lib) return;

	BotChat_UpdateCvars();
	chat_lib = lib;

	BotChat_PopulateState(&bs, bot, lib);

	if (BotChat_EnterGame(&bs)) {
		trap_BotEnterChat(bs.cs, 0, bs.chatto);
		lib->funcs.BotSetLastChatTime(bs.client, FloatTime());
	}

	chat_lib = NULL;
}

void BotChat_OnExitGame(edict_t *bot)
{
	bot_library_t *lib;
	bot_state_t    bs;

	if (!(bot->flags & FL_BOT)) return;
	if (!bot->inuse || !bot->client) return;

	lib = BotGetLib(bot);
	if (!lib) return;

	BotChat_UpdateCvars();
	chat_lib = lib;

	BotChat_PopulateState(&bs, bot, lib);

	if (BotChat_ExitGame(&bs)) {
		trap_BotEnterChat(bs.cs, 0, bs.chatto);
		lib->funcs.BotSetLastChatTime(bs.client, FloatTime());
	}

	chat_lib = NULL;
}

/* ================================================================== */
/*  Level start / end                                                  */
/* ================================================================== */

/*
 * BotChat_OnStartLevel -- iterate all active bots, trigger start-level chat.
 * Called once after map load (from g_spawn.c after BotLib_BotLoadMap).
 */
void BotChat_OnStartLevel(void)
{
	int i;

	BotChat_UpdateCvars();

	for (i = 0; i < game.maxclients; i++) {
		edict_t       *ent = &g_edicts[i + 1];
		bot_library_t *lib;
		bot_state_t    bs;

		if (!(ent->flags & FL_BOT) || !ent->inuse || !ent->client)
			continue;

		lib = BotGetLib(ent);
		if (!lib) continue;

		chat_lib = lib;
		BotChat_PopulateState(&bs, ent, lib);

		if (BotChat_StartLevel(&bs)) {
			trap_BotEnterChat(bs.cs, 0, bs.chatto);
			lib->funcs.BotSetLastChatTime(bs.client, FloatTime());
		}

		chat_lib = NULL;
	}
}

/*
 * BotChat_OnEndLevel -- iterate all active bots, trigger end-level chat.
 * Called from BeginIntermission() in p_hud.c.
 */
void BotChat_OnEndLevel(void)
{
	int i;

	BotChat_UpdateCvars();

	for (i = 0; i < game.maxclients; i++) {
		edict_t       *ent = &g_edicts[i + 1];
		bot_library_t *lib;
		bot_state_t    bs;

		if (!(ent->flags & FL_BOT) || !ent->inuse || !ent->client)
			continue;

		lib = BotGetLib(ent);
		if (!lib) continue;

		chat_lib = lib;
		BotChat_PopulateState(&bs, ent, lib);

		if (BotChat_EndLevel(&bs)) {
			trap_BotEnterChat(bs.cs, 0, bs.chatto);
			lib->funcs.BotSetLastChatTime(bs.client, FloatTime());
		}

		chat_lib = NULL;
	}
}
