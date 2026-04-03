/*
 * ai_chat_q2.h -- Q2 compatibility header for Q3's ai_chat.c
 *
 * This header is included by game_q3/ai_chat.c (after g_local.h) and
 * provides all the types, constants, and function declarations that
 * ai_chat.c expects from Q3's headers.  The implementations of the
 * functions live in game_q2/bl_chat.c.
 */

#ifndef AI_CHAT_Q2_H
#define AI_CHAT_Q2_H

#include <stdarg.h>

/* ------------------------------------------------------------------ */
/*  Avoid pulling in Q3 headers -- everything ai_chat.c needs is here */
/* ------------------------------------------------------------------ */

/* ================================================================== */
/*  Q3 types                                                          */
/* ================================================================== */

typedef struct {
	int integer;
} vmCvar_t;

/*
 * Minimal playerState_t with just the fields ai_chat.c reads.
 * Q2 already has player_state_t (different layout), so this name
 * is safe -- Q2 never uses "playerState_t".
 */
typedef struct {
	int persistant[16];       /* PERS_SCORE is index 0 */
} playerState_t;

/*
 * bot_state_t -- the per-bot data structure that ai_chat.c operates on.
 * bl_chat.c populates one of these from Q2 entity data before calling
 * any BotChat_* function.
 */
typedef struct bot_state_s {
	int             client;           /* 0-based client number */
	int             entitynum;        /* same as client in our mapping */
	playerState_t   cur_ps;
	float           thinktime;
	vec3_t          origin;
	vec3_t          eye;
	vec3_t          viewangles;
	int             inventory[MAX_ITEMS];
	int             lastkilledby;     /* client num of last killer */
	int             lastkilledplayer; /* client num of last victim */
	int             botdeathtype;     /* MOD of own death */
	int             enemydeathtype;   /* MOD used to kill enemy */
	int             botsuicide;       /* true if suicide */
	int             enemy;            /* current enemy client num (-1 = none) */
	int             chatto;           /* CHAT_ALL or CHAT_TEAM */
	float           lastchat_time;
	int             ltgtype;          /* long-term goal type */
	int             character;        /* botlib character handle */
	int             cs;               /* botlib chat-state handle */
} bot_state_t;

/* ================================================================== */
/*  aas_entityinfo_t (from Q3 be_aas.h)                               */
/* ================================================================== */

typedef struct aas_entityinfo_s {
	int     valid;
	int     type;
	int     flags;
	float   ltime;
	float   update_time;
	int     number;
	vec3_t  origin;
	vec3_t  angles;
	vec3_t  old_origin;
	vec3_t  lastvisorigin;
	vec3_t  mins;
	vec3_t  maxs;
	int     groundent;
	int     solid;
	int     modelindex;
	int     modelindex2;
	int     frame;
	int     event;
	int     weapon;
	int     legsAnim;
	int     torsoAnim;
} aas_entityinfo_t;

/* bsp_trace_t is already defined in game_q2/botlib.h (included via g_local.h) */

/* ================================================================== */
/*  Constants                                                         */
/* ================================================================== */

/* -- MOD mapping: Q3 names to Q2 values ----------------------------- */
/* Q2 already has: MOD_SHOTGUN, MOD_MACHINEGUN, MOD_GRENADE, MOD_ROCKET,
 * MOD_RAILGUN, MOD_WATER, MOD_SLIME, MOD_LAVA, MOD_FALLING, MOD_CRUSH,
 * MOD_SUICIDE, MOD_TARGET_LASER, MOD_TRIGGER_HURT, MOD_UNKNOWN,
 * MOD_TELEFRAG, MOD_CHAINGUN (different weapon but same concept)
 */

/* Q3-only MODs that don't exist in Q2 -- use negative sentinel values
 * so they never match any actual Q2 mod.  ai_chat.c compares against
 * these in switch/if chains; unmatched values fall through to the
 * "generic" chat paths. */
#define MOD_GAUNTLET        (-100)
#define MOD_LIGHTNING       (-101)
#define MOD_PLASMA          (-102)
#define MOD_PLASMA_SPLASH   (-103)
#define MOD_NAIL            (-104)
#define MOD_CHAINGUN_Q3     (-105)
#define MOD_PROXIMITY_MINE  (-106)
#define MOD_KAMIKAZE        (-107)
#define MOD_JUICED          (-108)

/* Q3 BFG == Q2 BFG_BLAST; Q3 BFG_SPLASH == Q2 BFG_EFFECT */
#define MOD_BFG             MOD_BFG_BLAST
#define MOD_BFG_SPLASH      MOD_BFG_EFFECT

/* Q3's GRENADE_SPLASH / ROCKET_SPLASH map to Q2's G_SPLASH / R_SPLASH */
#define MOD_GRENADE_SPLASH  MOD_G_SPLASH
#define MOD_ROCKET_SPLASH   MOD_R_SPLASH

/* -- Game types ------------------------------------------------------ */
#define GT_TOURNAMENT       1

/* -- Chat destinations ----------------------------------------------- */
#define CHAT_ALL            0
#define CHAT_TEAM           1

/* -- Entity numbers -------------------------------------------------- */
#define ENTITYNUM_NONE      (-1)
#define ENTITYNUM_WORLD     0

/* -- Presence types -------------------------------------------------- */
#define PRESENCE_CROUCH     2

/* -- Player-state indices -------------------------------------------- */
#define PERS_SCORE          0

/* -- Config-string base (arbitrary, used only in trap_GetConfigstring) */
#define CS_PLAYERS          544

/* -- Team constants -------------------------------------------------- */
#define TEAM_FREE           0
#define TEAM_RED            1
#define TEAM_BLUE           2
#define TEAM_SPECTATOR      3

/* -- Game type constants (Q3 values, subset used by ai_chat.c) ------- */
#define GT_FFA              0
#define GT_TOURNAMENT       1
#define GT_TEAM             3
#define GT_CTF              4

/* -- Long-term goal types (values don't matter for Q2 DM) ------------ */
#define LTG_TEAMHELP        1
#define LTG_TEAMACCOMPANY   2
#define LTG_RUSHBASE        3

/* -- Inventory: Q3 powerup indices mapped to Q2 inv.h slots ---------- */
#define INVENTORY_QUAD_PU         23   /* matches Q2 inv.h */
#define INVENTORY_HASTE           199  /* no Q2 equivalent; safe unused slot */
#define INVENTORY_INVISIBILITY    199
#define INVENTORY_REGEN           199
#define INVENTORY_FLIGHT          199

/* Resolve the name collision: Q3 ai_chat.c uses INVENTORY_QUAD.
 * Q2 inv.h also defines INVENTORY_QUAD (= 23).  If inv.h is included
 * first, we're fine.  If not, define it here. */
#ifndef INVENTORY_QUAD
#define INVENTORY_QUAD      23
#endif

/* -- Synonym context flags ------------------------------------------- */
#define CONTEXT_NORMAL      1
#define CONTEXT_NEARBYITEM  2
#define CONTEXT_NAMES       1024

/* -- Characteristic indices from botfiles/chars.h -------------------- */
#define CHARACTERISTIC_GENDER                1
#define CHARACTERISTIC_CHAT_FILE             21
#define CHARACTERISTIC_CHAT_NAME             22
#define CHARACTERISTIC_CHAT_CPM              23
#define CHARACTERISTIC_CHAT_INSULT           24
#define CHARACTERISTIC_CHAT_MISC             25
#define CHARACTERISTIC_CHAT_STARTENDLEVEL    26
#define CHARACTERISTIC_CHAT_ENTEREXITGAME    27
#define CHARACTERISTIC_CHAT_KILL             28
#define CHARACTERISTIC_CHAT_DEATH            29
#define CHARACTERISTIC_CHAT_ENEMYSUICIDE     30
#define CHARACTERISTIC_CHAT_HITTALKING       31
#define CHARACTERISTIC_CHAT_HITNODEATH       32
#define CHARACTERISTIC_CHAT_HITNOKILL        33
#define CHARACTERISTIC_CHAT_RANDOM           34
#define CHARACTERISTIC_CHAT_REPLY            35

/* -- Misc ------------------------------------------------------------ */
/* MAX_CLIENTS, MAX_ITEMS, MAX_INFO_STRING are already defined in Q2's q_shared.h.
 * Q2 uses MAX_INFO_STRING=512 (smaller than Q3's 1024), which is fine
 * since our trap_GetConfigstring produces short strings. */

/* MASK_WATER, MASK_SOLID, MASK_OPAQUE, CONTENTS_LAVA, CONTENTS_SLIME,
 * CONTENTS_SOLID are all defined in Q2's q_shared.h */

/* ================================================================== */
/*  g_entities mapping                                                */
/* ================================================================== */

/*
 * In Q3, g_entities[clientNum] is the gentity for that client.
 * In Q2, g_edicts[clientNum + 1] is the edict (entity 0 = world).
 * This macro bridges the two.
 */
#define g_entities (g_edicts + 1)

/* ================================================================== */
/*  Global variables (defined in bl_chat.c)                           */
/* ================================================================== */

extern vmCvar_t bot_nochat;
extern vmCvar_t bot_fastchat;
extern int      gametype;

/* ================================================================== */
/*  trap_* functions (implemented in bl_chat.c)                       */
/* ================================================================== */

int   trap_Cvar_VariableIntegerValue(const char *name);
void  trap_GetConfigstring(int num, char *buf, int size);
void  trap_GetServerinfo(char *buf, int size);
int   trap_PointContents(vec3_t point, int entnum);
void  trap_AAS_PresenceTypeBoundingBox(int ptype, vec3_t mins, vec3_t maxs);
void  trap_EA_Command(int client, char *cmd);
float trap_Characteristic_BFloat(int ch, int idx, float min, float max);
int   trap_Characteristic_BInteger(int ch, int idx, int min, int max);
int   trap_BotNumInitialChats(int cs, char *type);
void  trap_BotEnterChat(int cs, int client, int sendto);
int   trap_BotChatLength(int cs);

/* ================================================================== */
/*  BotAI_* functions (implemented in bl_chat.c)                      */
/* ================================================================== */

void  BotAI_BotInitialChat(bot_state_t *bs, char *type, ...);
void  BotAI_Trace(bsp_trace_t *trace, vec3_t start, vec3_t mins,
                  vec3_t maxs, vec3_t end, int passent, int contentmask);
int   BotAI_GetClientState(int clientNum, playerState_t *state);

/* ================================================================== */
/*  Helper functions (implemented in bl_chat.c)                       */
/* ================================================================== */

char *EasyClientName(int client, char *name, int size);
char *ClientName(int client, char *name, int size);

int   TeamPlayIsOn(void);
int   BotSameTeam(bot_state_t *bs, int client);
int   BotIsObserver(bot_state_t *bs);
int   BotIsDead(bot_state_t *bs);

void  BotEntityInfo(int entnum, aas_entityinfo_t *info);
int   BotValidChatPosition(bot_state_t *bs);
int   EntityIsDead(aas_entityinfo_t *info);
int   EntityIsInvisible(aas_entityinfo_t *info);
int   EntityIsShooting(aas_entityinfo_t *info);

float BotEntityVisible(int viewer, vec3_t eye, vec3_t viewangles,
                       float fov, int ent);
int   BotSynonymContext(bot_state_t *bs);
float FloatTime(void);

/* Info_ValueForKey is declared in Q2's q_shared.h */

#endif /* AI_CHAT_Q2_H */
