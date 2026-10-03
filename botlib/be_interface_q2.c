/*
===========================================================================

be_interface_q2.c  —  Q2 botlib interface adapter (Phase 1)

Wraps the Q3 botlib (GetBotLibAPI / botlib_export_t) behind the Q2 bot
library API (GetBotAPI / bot_export_t) so that the Gladiator game DLL
can load and drive the Q3 botlib without modification.

Entry point exported from botlib.so:
    q2_bot_export_t *GetBotAPI(q2_bot_import_t *import)

Design overview
---------------
* All Q3 botlib source files are compiled into the same botlib.so.
  We call their internal functions directly instead of going through
  GetBotLibAPI (which we also call once to initialise botimport).

* The Q3 botlib_import_t is populated from the Q2 bot_import_t:
    - Trace     : Q2 returns by value; adapter stores it in the out-ptr
    - EntityTrace: delegates to Q2 world trace (approximate)
    - inPVS     : real PVS check from BSP visibility lump data
    - BSPEntityData: returns entity lump read from BSP file on disk
    - FS_*      : stdio backed, using basedir/gamedir LibVars
    - HunkAlloc : mapped to GetMemory

* Per-bot state (chatstate, goalstate, movestate, weaponstate) is
  allocated in BotSetupClient and freed in BotShutdownClient.

* BotAI (Phase 1): initialise move state, pick an item goal, drive
  BotMoveToGoal, retrieve EA input and forward to BotInput.

===========================================================================
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* Q3 botlib side */
#include "../game_q3/q_shared.h"
#include "l_memory.h"
#include "l_log.h"
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
#include "inv.h"                    /* INVENTORY_* slots (assets/botfiles) */
#include "match.h"                  /* obituary templates (assets/botfiles) */

/* Forward declarations for functions defined in other botlib files
 * that are not explicitly declared in the included headers. */
extern int      AAS_PointAreaNum(vec3_t point);
extern float    AAS_Time(void);
extern int      AAS_StartFrame(float time);
extern int      AAS_Initialized(void);
extern void     AAS_ShowArea(int areanum, int groundfacesonly);
extern void     AAS_ShowReachableAreas(int areanum);
extern void     AAS_ShowReachability(aas_reachability_t *reach);
extern void     AAS_ClearShownDebugLines(void);
extern int      BotExportTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3);
extern int      PC_AddGlobalDefine(char *string);
extern qboolean ValidEntityNumber(int num, char *str);
/* bspc/l_utils.c — converts a world-space direction vector to Euler angles */
extern void     Vector2Angles(vec3_t value1, vec3_t angles);

/* Export_BotLib* functions from be_interface.c */
extern int Export_BotLibSetup(void);
extern int Export_BotLibShutdown(void);
extern int Export_BotLibVarSet(char *var_name, char *value);
extern int Export_BotLibStartFrame(float time);
extern int Export_BotLibLoadMap(const char *mapname);
extern int Export_BotLibUpdateEntity(int ent, bot_entitystate_t *state);

/* ====================================================================
 * The real Q3 bot AI (game_q3/ai_main.c, ai_dmnet.c, ai_dmq3.c, ai_team.c,
 * ai_chat.c -- compiled into this same botlib.so, see
 * botlib/ai_q2_compat.h) now drives bot decision-making, replacing the
 * hand-rolled AINode-alike state machine that used to live below in this
 * file. No header declares BotAISetupClient/BotAIShutdownClient/BotAI/
 * BotSetupDeathmatchAI/botstates[] (ai_main.h only declares
 * BotResetState/NumBots/BotEntityInfo/BotTeamLeader) -- extern-declare
 * them directly, the same pattern already used above for Export_BotLib*.
 *
 * bot_settings_t is genuinely undefined in every header this project
 * carries (mirrors botlib/ai_q2_compat.h's own copy -- deliberately not
 * #included wholesale here: that header's trap_ and gentity_t compat
 * scaffolding is scoped to the 5 ported ai_*.c files, and several of its
 * own macros, e.g. CTF_RUSHBASE_TIME, would collide with this file's
 * now-deleted same-named ones). The two definitions only ever cross the
 * ai_main.c<->be_interface_q2.c boundary as a passed struct pointer,
 * exactly like q2_bot_settings_t/bot_settings_t already do throughout
 * this file, so a matching parallel definition here is safe. */
typedef struct bot_settings_s {
    char  characterfile[MAX_QPATH];
    float skill;
    char  team[MAX_QPATH];
} bot_settings_t;

#include "../game_q3/ai_main.h"

extern int  BotAISetup(int restart);
extern int  BotAILoadMap(int restart);

/* The Q3 AI's game state (botlib/ai_q2_shim.c, "Q3-shaped game state"): the
 * g_entities[] and snapshots Q3's BotCheckSnapshot/BotCheckEvents/
 * BotCheckForGrenades read, fed from this adapter. */
extern void Q2Shim_Reset(void);
extern void Q2Shim_StartFrame(void);
extern void Q2Shim_UpdateEntity(int q2ent, int type, int eflags, int powerups,
                                vec3_t origin, vec3_t mins, vec3_t maxs,
                                int modelindex, int effects, int q2event);
extern void Q2Shim_Obituary(int target, int attacker, int mod);
extern void Q2Shim_TeleportIn(int client, vec3_t origin);
extern void Q2Shim_ClientHurt(int client, int attacker, int mod);
extern void Q2Shim_MoveClient(int oldclnum, int newclnum);
extern int  BotAISetupClient(int client, struct bot_settings_s *settings, qboolean restart);
extern int  BotAIShutdownClient(int client, qboolean restart);
extern int  BotAI(int client, float thinktime);
extern void BotUpdateInput(bot_state_t *bs, float thinktime);
extern void BotChooseWeapon(bot_state_t *bs);
extern int  AINode_Battle_Fight(bot_state_t *bs);
extern void BotSetupDeathmatchAI(void);
extern bot_state_t *botstates[MAX_CLIENTS];
extern int maxclients;

/* Real Q3 playerState_t.pm_type/pm_flags/persistant values -- NOT the
 * same numbering as Q2's own pmtype_t/PMF_* (game_q2/q_shared.h) or this
 * file's own q2_pmtype_t below. These let Q2BotUpdateClient translate
 * into bs->cur_ps (the REAL playerState_t) with the exact bit/value
 * meanings game_q3/ai_dmq3.c's BotSetupForMovement/BotIsDead/
 * BotIsObserver/BotIntermission expect (confirmed against those
 * functions directly). Real Q3 keeps these in game/bg_public.h, which
 * this repo's game_q3/ snapshot never carried (mirrors
 * botlib/ai_q2_compat.h's own near-identical, separately-defined block
 * for the 5 ported files). */
#define Q3PM_NORMAL             0
#define Q3PM_SPECTATOR          2
#define Q3PM_DEAD               3
#define Q3PM_FREEZE             4
#define Q3WEAPON_READY          0
#define Q3WEAPON_DROPPING       2
#define Q3PMF_DUCKED            1
#define Q3PMF_TIME_KNOCKBACK    64
#define Q3PMF_TIME_WATERJUMP    256
#define Q3PERS_SCORE            0

/* Real Q3 entityState_t.eFlags bits and powerup_t numbers that the ported
 * files test on aas_entityinfo_t.flags/.powerups (EntityIsShooting,
 * EntityHasQuad, EntityCarriesFlag, BotAI_GetClientState's EF_DEAD read in
 * ai_q2_shim.c). Same values as botlib/ai_q2_compat.h. */
#define Q3EF_DEAD               0x0001
#define Q3EF_TELEPORT_BIT       0x0004
#define Q3EF_FIRING             0x0010
#define Q3PW_QUAD               1
#define Q3PW_REDFLAG            7
#define Q3PW_BLUEFLAG           8

/* Real Q3 team_t numbering (game/bg_public.h in real Q3; this project's
 * own copy lives in botlib/ai_q2_compat.h, scoped to the ported ai_*.c
 * files and deliberately not included wholesale here -- see the
 * bot_settings_t comment above), for the bots' CTF teams (q2_ctfteam). */
#define Q3TEAM_FREE   0
#define Q3TEAM_RED    1
#define Q3TEAM_BLUE   2

/* The Quake II bot library interface (q2_bot_import_t, q2_bot_export_t and
 * the structs they pass) is be_interface_q2.h. */
#include "be_interface_q2.h"

/* Character indices (from ioq3/code/game/chars.h) */
#define Q2CHAR_GENDER              1
#define Q2CHAR_ATTACK_SKILL        2
#define Q2CHAR_WEAPONWEIGHTS       3
#define Q2CHAR_REACTIONTIME        6
#define Q2CHAR_AIM_ACCURACY        7
#define Q2CHAR_AIM_SKILL          16
#define Q2CHAR_CHAT_FILE          21
#define Q2CHAR_CHAT_NAME          22
#define Q2CHAR_CROUCHER           36
#define Q2CHAR_JUMPER             37
#define Q2CHAR_WEAPONJUMPING      38
#define Q2CHAR_ITEMWEIGHTS        40
#define Q2CHAR_CAMPER             44
#define Q2CHAR_EASY_FRAGGER       45
#define Q2CHAR_ALERTNESS          46
#define Q2CHAR_FIRETHROTTLE       47
#define Q2CHAR_WALKER             48

/* ====================================================================
 * Per-client AI state now lives entirely in the real Q3 ai_main.c's own
 * bot_state_t *botstates[MAX_CLIENTS] (see the extern declaration above)
 * -- this file must not keep a second, parallel per-client array. The
 * hand-rolled AINode-alike state machine that used to be described here
 * (and the private q2_botclient_t struct + q2clients[] array backing it)
 * is gone; see game_q3/ai_dmnet.c/ai_dmq3.c for the real state machine.
 * ==================================================================== */
#define Q2_BOTLIB_MAX_CLIENTS 256

/* View-model updates per 10 Hz Q2 frame (see Q2BotAI): 5 steps = 50 Hz,
 * between Q3's dedicated-server (20 Hz) and listen-server (client frame
 * rate) behaviour. */
#define Q2_VIEW_SUBSTEPS 5

/* How far (degrees of yaw) a bot's view may lead or lag the direction it is
 * walking in while it has no enemy -- see Q2BotAI. */
#define Q2_ROAM_VIEW_MAXDEVIATION 45.0f

/* How often Q2BotAI repeats a "use" while a thrown weapon the AI did not
 * choose is still in hand: a switch first plays the old weapon's drop and
 * the new one's raise animation. */
#define Q2_WEAPON_RESEND 0.5f

/* How long a weapon switch the adapter commanded counts as under way (the
 * Q3 weapon state, see Q2BotUpdateClient): Q2's drop and raise animations
 * take about 0.9 s (game_q2/p_weapon.c Weapon_Generic). */
#define Q2_WEAPON_SWITCH_TIME 1.0f

/* CTF flag carrier effects (from Q2 q_shared.h) -- still used by
 * Q2BotUpdateEntity below to translate Q2 effects bits into Q3 powerup
 * bits for AAS entity tracking; unrelated to the deleted CTF goal-
 * selection logic. */
#define Q2_EF_FLAG1_CARRIER  0x00040000
#define Q2_EF_FLAG2_CARRIER  0x00080000

/* ====================================================================
 * Module globals
 * ==================================================================== */
static q2_bot_import_t q2import;         /* stored Q2 import callbacks  */
static q2_bot_export_t q2_export;        /* returned Q2 export struct   */

/* Entity numbers. The botlib and the Q3 AI number entities as Q3's game does:
 * a client's entity is its client number, the world is ENTITYNUM_WORLD. Q2
 * numbers its edicts from the world (0), then the clients (client c is edict
 * c + 1), then everything else, which keeps its number here (always above
 * maxclients; Q3's number maxclients goes unused). The adapter translates
 * where entities cross -- the entity updates and the traces -- so the AAS
 * entity table and the AI see Q3's numbers only. */
static int q2_maxclients;

static int Q2_EdictToEntity(int edict)
{
    if (edict == 0)
        return ENTITYNUM_WORLD;
    if (edict >= 1 && edict <= q2_maxclients)
        return edict - 1;
    return edict;
}

static int Q2_EntityToEdict(int entnum)
{
    if (entnum < 0)
        return 0;       /* no pass entity: Q2's traces skip the world anyway */
    if (entnum < q2_maxclients)
        return entnum + 1;
    return entnum;
}

/* Last weapon number (Q3 WP_*-alike weaponinfo_t.number, not a Q2
 * inventory index) commanded to Q2 via a "use <name>" client command per
 * client, and the AAS time it was sent; see the "use" in Q2BotAI.
 * Adapter-side plumbing only, not AI state. */
static int   q2_lastweaponcmd[MAX_CLIENTS];
static float q2_lastweapontime[MAX_CLIENTS];

/* The Q2 pm_flags of each bot's previous update, for the teleport edge in
 * Q2BotUpdateClient. */
static byte  q2_lastpmflags[MAX_CLIENTS];

/* Per bot, the view model's frame of its last update and whether its last
 * input held fire, for Q2_BotRocketJumpReady. */
static int   q2_gunframe[MAX_CLIENTS];
static int   q2_lastattack[MAX_CLIENTS];

/* Per bot, the CTF flag status (red | blue << 1) Q2BotAI last applied from
 * the flag entities; -1 = none yet. */
static int   q2_appliedflagstatus[MAX_CLIENTS];

/* Per bot, its CTF team in Q3's numbering (Q2BotUpdateClient), which
 * trap_GetConfigstring (ai_q2_shim.c) puts in the client's configstring. */
static int   q2_ctfteam[MAX_CLIENTS];

int Q2_ClientTeam(int client)
{
    if (client < 0 || client >= MAX_CLIENTS)
        return Q3TEAM_FREE;
    return q2_ctfteam[client];
}

/* The game DLL's model index table (game_q2/bl_redirgi.c modelindexes[],
 * passed to every BotLoadMap call), and a cache from a view-model index to
 * the weapon config number of the weapon using that view model
 * (-1 = not looked up yet). See Q2_WeaponForGunIndex. */
#define Q2_MAX_MODELINDEXES 256
static char **q2_modelnames;
static int    q2_nummodelnames;
static int    q2_gunweapon[Q2_MAX_MODELINDEXES];

/* The game DLL's image index table (bl_redirgi.c imageindexes[]), kept like
 * the model table. It names the HUD icons in the stats[] every
 * BotUpdateClient brings, which tell the adapter what powerup is running. */
#define Q2_MAX_IMAGEINDEXES 256
static char **q2_imagenames;
static int    q2_numimagenames;

/* Powerups Q2BotUsePowerups switches on (see there): the pickup name "use"
 * takes, the inventory slot, the STAT_TIMER_ICON image the HUD shows while
 * it runs, and for how long it runs. */
enum {
    Q2PU_QUAD, Q2PU_DUALFIRE, Q2PU_DOUBLE, Q2PU_INVULNERABILITY,
    Q2PU_DEFENDER, Q2PU_HUNTER, Q2PU_VENGEANCE, Q2PU_DOPPLEGANGER,
    Q2PU_SILENCER, Q2PU_REBREATHER, Q2PU_ENVIRO,
    Q2PU_NUM
};
static const struct {
    char       *item;
    int         slot;
    const char *icon;
    float       time;
} q2_powerups[Q2PU_NUM] = {
    { "Quad Damage",      INVENTORY_QUAD,            "p_quad",            30 },
    { "DualFire Damage",  INVENTORY_DUALFIREDAMAGE,  "p_quadfire",        30 },
    { "Double Damage",    INVENTORY_DOUBLEDAMAGE,    "p_double",          30 },
    { "Invulnerability",  INVENTORY_INVULNERABILITY, "p_invulnerability", 30 },
    { "defender sphere",  INVENTORY_DEFENDERSPHERE,  "p_defender",        30 },
    { "hunter sphere",    INVENTORY_HUNTERSPHERE,    "p_hunter",          30 },
    { "vengeance sphere", INVENTORY_VENGEANCESPHERE, "p_vengeance",       30 },
    { "Doppleganger",     INVENTORY_DOPPLEGANGER,    NULL,                 0 },
    { "Silencer",         INVENTORY_SILENCER,        NULL,                 0 },
    { "Rebreather",       INVENTORY_REBREATHER,      "p_rebreather",      30 },
    { "Environment Suit", INVENTORY_ENVIRONMENTSUIT, "p_envirosuit",      30 },
};

/* A use shows in the inventory a frame later; wait this long before trying
 * the same item again (the doppleganger also refuses without room). */
#define Q2_POWERUP_RETRY 1.0f
/* Power armor on/off can only be told from the HUD icon, which flashes
 * every 0.8 s while other armor is worn: act only after this long. */
#define Q2_POWERARMOR_WAIT 2.0f

/* Per bot: when each powerup stops running (the HUD timer, or the use when
 * a timer of higher priority hides it), when it was last used, how many the
 * game says the bot holds, and power armor state. */
typedef struct {
    float until[Q2PU_NUM];
    float used[Q2PU_NUM];
    int   held[Q2PU_NUM];
    float powerarmor_seen;  /* the "i_powershield" armor icon was up */
    float powerarmor_since; /* holds power armor since, 0 = does not */
    float powerarmor_used;
    float updated;          /* AAS time of the last Q2_TrackPowerups */
} q2_powerupstate_t;
static q2_powerupstate_t q2_powerupstate[MAX_CLIENTS];


/* BSP entity string read from disk during BotLoadMap */
#define Q2_BSP_ENTITYSTRING_MAX 0x40000
static char q2_bsp_entitystring[Q2_BSP_ENTITYSTRING_MAX];

/* stdio-backed FS file table for Q3 botlib FS_* callbacks.
 * pak-based entries store the open pak FILE* with base/size limits so reads
 * are bounded to the entry — no tmpfile or heap copy needed. */
#define MAX_Q2_FS_FILES 32
typedef struct {
    FILE *file;   /* underlying stream; NULL = slot free */
    int   base;   /* byte offset of entry start (0 for loose files) */
    int   size;   /* entry byte count (-1 = loose file, no limit) */
    int   pos;    /* current read/seek position within entry (pak only) */
} q2fsfile_t;
static q2fsfile_t fs_files[MAX_Q2_FS_FILES];

/* ====================================================================
 * Q2 BSP entity lump reader — supports loose files and PAK archives
 *
 * Q3 botlib calls BSPEntityData() to obtain the BSP entity string for
 * item/landmark discovery.  In Q2 the engine owns the BSP; we read the
 * entity lump from disk ourselves when BotLoadMap is called.
 *
 * PAK format (all little-endian):
 *   header: char[4] "PACK", int diroffset, int dirsize
 *   entry:  char[56] name, int filepos, int filelen   (dirsize/64 entries)
 * ==================================================================== */
#define Q2BSP_IDENT     (('P'<<24)+('S'<<16)+('B'<<8)+'I')
#define Q2BSP_VERSION   38
#define Q2BSP_LUMP_ENTITIES    0
#define Q2BSP_LUMP_MODELS     13

#define Q2BSP_MAX_MODELS    256

typedef struct { int fileofs, filelen; } q2bsplump_t;
typedef struct { int ident, version; q2bsplump_t lumps[19]; } q2bspheader_t;

/* Q2 BSP inline model — matches dmodel_t (48 bytes) */
typedef struct {
    float mins[3], maxs[3];
    float origin[3];
    int   headnode;
    int   firstface, numfaces;
} q2_dmodel_t;

/* Loaded BSP inline models for BSPModelMinsMaxsOrigin */
static q2_dmodel_t q2_bsp_models[Q2BSP_MAX_MODELS];
static int         q2_bsp_nummodels;

/* The map's potentially visible sets, for Q3inPVS_Adapter: the world's BSP
 * tree and the visibility lump (Q2 qfiles.h dplane_t, dnode_t, dleaf_t and
 * dvis_t with its run-length coded rows), as Gladiator's botlib read them
 * (be_aas_bspq2.c) */
#define Q2BSP_LUMP_PLANES      1
#define Q2BSP_LUMP_VISIBILITY  3
#define Q2BSP_LUMP_NODES       4
#define Q2BSP_LUMP_LEAFS       8
#define Q2BSP_MAX_CLUSTERS     65536    /* MAX_MAP_LEAFS */

typedef struct { float normal[3]; float dist; int type; } q2_dplane_t;
typedef struct {
    int planenum;
    int children[2];
    short mins[3], maxs[3];
    unsigned short firstface, numfaces;
} q2_dnode_t;
typedef struct {
    int contents;
    short cluster, area;
    short mins[3], maxs[3];
    unsigned short firstleafface, numleaffaces;
    unsigned short firstleafbrush, numleafbrushes;
} q2_dleaf_t;

static q2_dplane_t   *q2_bsp_planes;
static q2_dnode_t    *q2_bsp_nodes;
static q2_dleaf_t    *q2_bsp_leafs;
static int           *q2_bsp_vis;       /* numclusters, bitofs[numclusters][2], rows */
static int            q2_bsp_numplanes, q2_bsp_numnodes, q2_bsp_numleafs, q2_bsp_vissize;
static unsigned char  q2_bsp_pvsrow[Q2BSP_MAX_CLUSTERS / 8];
static int            q2_bsp_pvscluster = -1;   /* the cluster q2_bsp_pvsrow is */

static void Q2_FreeBSPVisibility(void)
{
    if (q2_bsp_planes) FreeMemory(q2_bsp_planes);
    if (q2_bsp_nodes) FreeMemory(q2_bsp_nodes);
    if (q2_bsp_leafs) FreeMemory(q2_bsp_leafs);
    if (q2_bsp_vis) FreeMemory(q2_bsp_vis);
    q2_bsp_planes = NULL;
    q2_bsp_nodes = NULL;
    q2_bsp_leafs = NULL;
    q2_bsp_vis = NULL;
    q2_bsp_numplanes = q2_bsp_numnodes = q2_bsp_numleafs = q2_bsp_vissize = 0;
    q2_bsp_pvscluster = -1;
}

/* One lump of the BSP in memory of its own; NULL for an empty or cut one */
static void *Q2_ReadBSPLump(FILE *f, long base_offset, q2bspheader_t *hdr,
                            int lump, int elemsize, int *count)
{
    int len = hdr->lumps[lump].filelen;
    void *data;

    *count = 0;
    if (len <= 0 || len % elemsize)
        return NULL;
    data = GetMemory(len);
    fseek(f, base_offset + hdr->lumps[lump].fileofs, SEEK_SET);
    if ((int)fread(data, 1, len, f) != len) {
        FreeMemory(data);
        return NULL;
    }
    *count = len / elemsize;
    return data;
}

static void Q2_ReadBSPVisibility(FILE *f, long base_offset, q2bspheader_t *hdr,
                                 const char *mapname)
{
    int numclusters, i;

    q2_bsp_planes = Q2_ReadBSPLump(f, base_offset, hdr, Q2BSP_LUMP_PLANES,
                                   sizeof(q2_dplane_t), &q2_bsp_numplanes);
    q2_bsp_nodes = Q2_ReadBSPLump(f, base_offset, hdr, Q2BSP_LUMP_NODES,
                                  sizeof(q2_dnode_t), &q2_bsp_numnodes);
    q2_bsp_leafs = Q2_ReadBSPLump(f, base_offset, hdr, Q2BSP_LUMP_LEAFS,
                                  sizeof(q2_dleaf_t), &q2_bsp_numleafs);
    q2_bsp_vis = Q2_ReadBSPLump(f, base_offset, hdr, Q2BSP_LUMP_VISIBILITY, 1,
                                &q2_bsp_vissize);
    if (!q2_bsp_planes || !q2_bsp_nodes || !q2_bsp_leafs) {
        botimport.Print(PRT_WARNING, "Q2Adapt: '%s' has no BSP tree\n", mapname);
        Q2_FreeBSPVisibility();
        return;
    }
    /* a map without visibility data sees everything (qvis was not run) */
    if (!q2_bsp_vis) {
        botimport.Print(PRT_MESSAGE, "Q2Adapt: '%s' has no visibility data\n", mapname);
        return;
    }
    numclusters = q2_bsp_vis[0];
    if (numclusters <= 0 || numclusters > Q2BSP_MAX_CLUSTERS ||
        q2_bsp_vissize < (int)sizeof(int) * (1 + 2 * numclusters)) {
        numclusters = 0;
    }
    for (i = 0; i < numclusters; i++) {
        if (q2_bsp_vis[1 + 2 * i] < 0 || q2_bsp_vis[1 + 2 * i] >= q2_bsp_vissize)
            numclusters = 0;
    }
    if (!numclusters) {
        botimport.Print(PRT_WARNING, "Q2Adapt: '%s' has broken visibility data\n", mapname);
        FreeMemory(q2_bsp_vis);
        q2_bsp_vis = NULL;
        q2_bsp_vissize = 0;
    }
}

/* Read the entity lump from an already-open FILE positioned at base_offset
 * (0 for a loose BSP file, or the entry's filepos within a PAK).
 * Returns 1 on success, 0 on failure. */
static int Q2_ReadBSPEntityLump(FILE *f, long base_offset, const char *mapname)
{
    q2bspheader_t hdr;
    int len;

    fseek(f, base_offset, SEEK_SET);
    if (fread(&hdr, sizeof(hdr), 1, f) != 1 ||
        hdr.ident != Q2BSP_IDENT || hdr.version != Q2BSP_VERSION)
    {
        botimport.Print(PRT_WARNING,
            "Q2Adapt: invalid BSP header for map '%s'\n", mapname);
        return 0;
    }

    /* Read entity lump */
    len = hdr.lumps[Q2BSP_LUMP_ENTITIES].filelen;
    if (len <= 0) return 0;
    if (len >= Q2_BSP_ENTITYSTRING_MAX) len = Q2_BSP_ENTITYSTRING_MAX - 1;

    fseek(f, base_offset + hdr.lumps[Q2BSP_LUMP_ENTITIES].fileofs, SEEK_SET);
    if ((int)fread(q2_bsp_entitystring, 1, len, f) != len) {
        botimport.Print(PRT_WARNING,
            "Q2Adapt: short read of BSP entity lump for '%s'\n", mapname);
    }
    q2_bsp_entitystring[len] = '\0';

    /* #1 — Read models lump for BSPModelMinsMaxsOrigin.
     * Each Q2 BSP inline model (func_plat, func_door, etc.) has
     * pre-calculated mins/maxs/origin stored in the models lump.
     * Without this, elevator/mover navigation is completely broken. */
    q2_bsp_nummodels = 0;
    len = hdr.lumps[Q2BSP_LUMP_MODELS].filelen;
    if (len > 0) {
        int count = len / (int)sizeof(q2_dmodel_t);
        if (count > Q2BSP_MAX_MODELS) count = Q2BSP_MAX_MODELS;
        fseek(f, base_offset + hdr.lumps[Q2BSP_LUMP_MODELS].fileofs, SEEK_SET);
        if ((int)fread(q2_bsp_models, sizeof(q2_dmodel_t), count, f) == count) {
            q2_bsp_nummodels = count;
            botimport.Print(PRT_MESSAGE,
                "Q2Adapt: loaded %d BSP inline models for '%s'\n",
                count, mapname);
        }
    }

    Q2_ReadBSPVisibility(f, base_offset, &hdr, mapname);
    return 1;
}

/* Search pak0..pak9 in dir for qpath (case-insensitive).
 * On success: returns the open pak FILE*, sets *out_base to the entry's byte
 * offset within that file and *out_size to the entry's byte count.
 * The caller is responsible for fclose().  Returns NULL if not found. */
static FILE *Q2_OpenPakEntry(const char *dir, const char *qpath,
                              int *out_base, int *out_size)
{
    char pakpath[512];
    int  paknum;

    for (paknum = 0; paknum <= 9; paknum++) {
        FILE *pf;
        int   magic, diroffset, dirsize, nentries, i;

        Com_sprintf(pakpath, sizeof(pakpath), "%s/pak%d.pak", dir, paknum);
        pf = fopen(pakpath, "rb");
        if (!pf) continue;

        if (fread(&magic,     4, 1, pf) != 1 ||
            fread(&diroffset, 4, 1, pf) != 1 ||
            fread(&dirsize,   4, 1, pf) != 1 ||
            magic != 0x4b434150 /* "PACK" */)
        {
            fclose(pf); continue;
        }

        nentries = dirsize / 64;
        fseek(pf, diroffset, SEEK_SET);

        for (i = 0; i < nentries; i++) {
            char entname[56];
            int  entoffset, entsize;

            if (fread(entname,   56, 1, pf) != 1 ||
                fread(&entoffset, 4, 1, pf) != 1 ||
                fread(&entsize,   4, 1, pf) != 1) break;

            if (Q_stricmp(entname, qpath) == 0) {
                *out_base = entoffset;
                *out_size = entsize;
                return pf;  /* caller must fclose */
            }
        }
        fclose(pf);
    }
    return NULL;
}

static void Q2_ReadBSPEntityData(const char *mapname)
{
    char          path[512];
    const char   *basedir = LibVarGetString("basedir");
    const char   *gamedir = LibVarGetString("gamedir");
    FILE         *f = NULL;

    q2_bsp_entitystring[0] = '\0';
    Q2_FreeBSPVisibility();

    if (!mapname || !mapname[0]) return;

    /* 1. Try loose file: basedir/gamedir/maps/mapname.bsp */
    if (gamedir[0]) {
        Com_sprintf(path, sizeof(path), "%s/%s/maps/%s.bsp",
                    basedir, gamedir, mapname);
        f = fopen(path, "rb");
        if (f) {
            Q2_ReadBSPEntityLump(f, 0, mapname);
            fclose(f); return;
        }
    }

    /* 2. Loose file: basedir/baseq2/maps/mapname.bsp */
    Com_sprintf(path, sizeof(path), "%s/baseq2/maps/%s.bsp",
                basedir, mapname);
    f = fopen(path, "rb");
    if (f) {
        Q2_ReadBSPEntityLump(f, 0, mapname);
        fclose(f); return;
    }

    /* 3. PAK files in gamedir */
    if (gamedir[0]) {
        char bspname[64]; int base, size;
        Com_sprintf(bspname, sizeof(bspname), "maps/%s.bsp", mapname);
        Com_sprintf(path, sizeof(path), "%s/%s", basedir, gamedir);
        f = Q2_OpenPakEntry(path, bspname, &base, &size);
        if (f) { Q2_ReadBSPEntityLump(f, base, mapname); fclose(f); return; }
    }

    /* 4. PAK files in baseq2 */
    {
        char bspname[64]; int base, size;
        Com_sprintf(bspname, sizeof(bspname), "maps/%s.bsp", mapname);
        Com_sprintf(path, sizeof(path), "%s/baseq2", basedir);
        f = Q2_OpenPakEntry(path, bspname, &base, &size);
        if (f) { Q2_ReadBSPEntityLump(f, base, mapname); fclose(f); return; }
    }

    botimport.Print(PRT_WARNING,
        "Q2Adapt: couldn't find BSP for map '%s' — entity data unavailable\n",
        mapname);
}

/* ====================================================================
 * Q3 import adapters
 * Called by Q3 botlib internals; translate to Q2 import equivalents.
 * ==================================================================== */

/* Q3 Trace: void out-param variant; Q2 returns by value. The pass entity
 * and the entity hit are translated between Q3's numbers and Q2's edicts
 * (Q2_EntityToEdict): the world Q2 reports as edict 0 is ENTITYNUM_WORLD. */
/* Q2 splits solid into CONTENTS_SOLID and CONTENTS_WINDOW -- glass, solid but
 * see-through -- and pairs the two in all its masks (game_q2/q_shared.h
 * MASK_SOLID, MASK_PLAYERSOLID, MASK_SHOT). Q3 has no window contents; the
 * bot AI and the botlib ask for CONTENTS_SOLID alone, and every trace went
 * through glass: bots fired into windows, their splash check missed the pane
 * in front of them, and they saw and roamed through it. Q3's glass is plain
 * solid, so a window is solid to all of them. */
#define Q2_CONTENTS_WINDOW  2

static int Q3ContentMaskToQ2(int contentmask)
{
    if (contentmask & CONTENTS_SOLID)
        contentmask |= Q2_CONTENTS_WINDOW;
    return contentmask;
}

static int Q3PointContents_Adapter(vec3_t point)
{
    int contents = q2import.PointContents(point);

    if (contents & Q2_CONTENTS_WINDOW)
        contents |= CONTENTS_SOLID;
    return contents;
}

static void Q3Trace_Adapter(bsp_trace_t *trace, vec3_t start, vec3_t mins,
                              vec3_t maxs, vec3_t end, int passent, int contentmask)
{
    *trace = q2import.Trace(start, mins, maxs, end, Q2_EntityToEdict(passent),
                            Q3ContentMaskToQ2(contentmask));
    trace->ent = Q2_EdictToEntity(trace->ent);
}

/* EntityTrace is Q3's SV_ClipToEntity: a trace against one entity alone. The
 * AAS traces ask it for every entity linked into an area they cross
 * (AAS_AreaEntityCollision), items and players included. Q2's game only
 * traces against everything, so the world trace answers for the entity when
 * the entity is what it hits first. Anything else it hits first -- the world,
 * another entity -- is not a collision with this entity: the AAS trace itself
 * stops at the world, and another entity has its own test. Reporting those
 * hits made every area with an item or a player collide with the world
 * geometry, on AAS plane 0 (AAS_AreaEntityCollision knows no plane), which
 * threw off the on-ground tests and the movement and aim predictions there.
 *
 * Before tracing, what the world trace could report as this entity at all.
 * The engine clips nothing SOLID_NOT or SOLID_TRIGGER, and a SOLID_BBOX entity
 * as a box of CONTENTS_MONSTER (CM_HeadnodeForBox), which is Q3's
 * CONTENTS_BODY; the AAS asks with CONTENTS_SOLID|CONTENTS_PLAYERCLIP
 * (be_aas_sample.c AAS_AreaEntityCollision). So of the items, players and
 * missiles AAS_UpdateEntity links into its areas none can be hit, and only a
 * brush model is worth a trace of the whole world -- Gladiator's
 * AAS_EntityCollision likewise tests nothing but SOLID_BBOX and SOLID_BSP
 * entities, in the library. Tracing them all was BotGapDistance's 13 traces
 * a frame times every entity in every area each crosses, for every walking
 * bot: with 32 bots the frames where they bunched up at items spent ~15 ms
 * in it. The solid is the one the game sent this frame. */
static void Q3EntityTrace_Adapter(bsp_trace_t *trace, vec3_t start, vec3_t mins,
                                   vec3_t maxs, vec3_t end,
                                   int entnum, int contentmask)
{
    int solid;

    if (entnum >= 0 && entnum < aasworld.maxentities && aasworld.entities) {
        solid = aasworld.entities[entnum].i.solid;
        if (solid == SOLID_NOT || solid == SOLID_TRIGGER ||
            (solid == SOLID_BBOX && !(contentmask & CONTENTS_BODY))) {
            Com_Memset(trace, 0, sizeof(*trace));
            trace->fraction = 1;
            VectorCopy(end, trace->endpos);
            trace->ent = ENTITYNUM_NONE;
            return;
        }
    }
    *trace = q2import.Trace(start, mins, maxs, end, 0, Q3ContentMaskToQ2(contentmask));
    trace->ent = Q2_EdictToEntity(trace->ent);
    if (trace->ent != entnum) {
        trace->allsolid = trace->startsolid = false;
        trace->fraction = 1;
        VectorCopy(end, trace->endpos);
        trace->contents = 0;
        trace->ent = ENTITYNUM_NONE;
    }
}

/* The leaf of the world's BSP tree a point is in (Q2's CM_PointLeafnum), -1
 * if the tree is broken */
static int Q2_BSPPointLeaf(vec3_t p)
{
    int num = q2_bsp_nummodels > 0 ? q2_bsp_models[0].headnode : 0;
    q2_dplane_t *plane;
    float d;

    while (num >= 0) {
        if (num >= q2_bsp_numnodes || q2_bsp_nodes[num].planenum < 0 ||
            q2_bsp_nodes[num].planenum >= q2_bsp_numplanes)
            return -1;
        plane = &q2_bsp_planes[q2_bsp_nodes[num].planenum];
        if (plane->type < 3)
            d = p[plane->type] - plane->dist;
        else
            d = DotProduct(plane->normal, p) - plane->dist;
        num = q2_bsp_nodes[num].children[d < 0];
    }
    num = -1 - num;
    return num < q2_bsp_numleafs ? num : -1;
}

/* The potentially visible set of a cluster, run-length decoded as by Q2's
 * CM_DecompressVis */
static void Q2_BSPClusterPVS(int cluster)
{
    const unsigned char *vis = (const unsigned char *)q2_bsp_vis;
    const unsigned char *in = vis + q2_bsp_vis[1 + 2 * cluster];
    const unsigned char *end = vis + q2_bsp_vissize;
    unsigned char *out = q2_bsp_pvsrow;
    int row = (q2_bsp_vis[0] + 7) >> 3;
    int c;

    while (out - q2_bsp_pvsrow < row && in < end) {
        if (*in) {
            *out++ = *in++;
            continue;
        }
        if (in + 1 >= end)
            break;
        c = in[1];
        in += 2;
        if ((out - q2_bsp_pvsrow) + c > row)
            c = row - (out - q2_bsp_pvsrow);
        for (; c > 0; c--)
            *out++ = 0;
    }
    memset(out, 0, row - (out - q2_bsp_pvsrow));
    q2_bsp_pvscluster = cluster;
}

/* Whether p2 is in the potentially visible set of p1, from the map as
 * Gladiator's botlib had it (be_aas_bspq2.c AAS_InPVS): a point in no
 * cluster (in solid) sees nothing and is seen by nothing, a map without
 * visibility data sees everything. Unlike the engine's PF_inPVS it does not
 * look at area portals (closed doors): the game does not tell the botlib
 * their state.
 *
 * In two halves, the cluster of a point and whether two clusters see each
 * other, for the snapshot (ai_q2_shim.c Q2Shim_BuildSnapshot): it tests every
 * entity against one bot's eye, for every bot, every frame, and walking the
 * tree for both points of each pair was 32 bots x ~300 entities x 2 descents
 * a frame -- over 40% of a 32-bot server's CPU. Q3's server
 * keeps an entity's clusters from the time it links the entity and only tests
 * a bit; the snapshot now keeps each entity's cluster from the time its
 * origin is set. Q2_CLUSTER_ANY is a point the tree cannot place (no tree, no
 * visibility lump, a broken tree), which sees everything, even a point in
 * solid; Q2_CLUSTER_OUTSIDE is a cluster past the visibility lump, which sees
 * everything but a point in solid. The order of the tests is the old single
 * function's. */
#define Q2_CLUSTER_SOLID    -1
#define Q2_CLUSTER_ANY      -2
#define Q2_CLUSTER_OUTSIDE  -3

int Q2_PointCluster(vec3_t p)
{
    int leaf, cluster;

    if (!q2_bsp_numleafs || !q2_bsp_vis)
        return Q2_CLUSTER_ANY;
    leaf = Q2_BSPPointLeaf(p);
    if (leaf < 0)
        return Q2_CLUSTER_ANY;
    cluster = q2_bsp_leafs[leaf].cluster;
    if (cluster < 0)
        return Q2_CLUSTER_SOLID;
    if (cluster >= q2_bsp_vis[0])
        return Q2_CLUSTER_OUTSIDE;
    return cluster;
}

int Q2_ClustersVisible(int cluster1, int cluster2)
{
    if (cluster1 == Q2_CLUSTER_ANY || cluster2 == Q2_CLUSTER_ANY)
        return true;
    if (cluster1 == Q2_CLUSTER_SOLID || cluster2 == Q2_CLUSTER_SOLID)
        return false;
    if (cluster1 == Q2_CLUSTER_OUTSIDE || cluster2 == Q2_CLUSTER_OUTSIDE)
        return true;
    if (cluster1 != q2_bsp_pvscluster)
        Q2_BSPClusterPVS(cluster1);
    return (q2_bsp_pvsrow[cluster2 >> 3] >> (cluster2 & 7)) & 1;
}

static int Q3inPVS_Adapter(vec3_t p1, vec3_t p2)
{
    return Q2_ClustersVisible(Q2_PointCluster(p1), Q2_PointCluster(p2));
}

static char *Q3BSPEntityData_Callback(void)
{
    return q2_bsp_entitystring;
}

/* #1 — Real BSPModelMinsMaxsOrigin: return bounding box of Q2 BSP
 * inline models (func_plat, func_door, func_train, etc.).
 * Without this, BotTravel_Elevator / BotTravel_Train cannot execute
 * and bots cannot ride elevators or navigate through doors.
 * Data loaded from the BSP models lump in Q2_ReadBSPEntityLump. */
static void Q3BSPModelMinsMaxsOrigin(int modelnum, vec3_t angles,
                                      vec3_t mins, vec3_t maxs, vec3_t origin)
{
    (void)angles; /* Q2 inline models don't rotate at load time */
    if (modelnum >= 0 && modelnum < q2_bsp_nummodels) {
        q2_dmodel_t *m = &q2_bsp_models[modelnum];
        VectorCopy(m->mins, mins);
        VectorCopy(m->maxs, maxs);
        if (origin) VectorCopy(m->origin, origin);
    } else {
        VectorClear(mins);
        VectorClear(maxs);
        if (origin) VectorClear(origin);
    }
}

/* Q3 BotClientCommand takes a plain string ("say Hello world");
 * Q2's variadic version expects separate args ("say", "Hello world", NULL).
 * Split at the first space so that gi.argv(0) returns just the command.
 *
 * Q2's ClientCommand says any command it does not know as chat, so Q3's
 * own commands must not reach it: every bot said "team" when it entered a
 * game. "team <name>" is how a Q3 bot joins bs->settings.team at setup
 * (BotDeathmatchAI) -- Q2 never provides one, the game puts bots on teams
 * itself. The voice chats have no Q2 counterpart.
 *
 * "tell <client> <text>" (BotEnterChat CHAT_TELL; EA_Tell puts a comma after
 * the number) is how Q3 bots give and answer team orders (ai_team.c,
 * ai_cmd.c). Q3's server hands it to the addressee alone, as EC "[sender"
 * EC "]" EC ": text". A bot gets exactly that in its console here, as Q3's
 * server would queue it; Q2 has no private messages for a human, so a human
 * gets it as team chat. */
#define Q2_EC "\x19"    /* Q3's EC, the name marker of chat lines */

const char *Q2_ClientNetname(int client);

static void Q2BotTell(int client, int to, const char *text)
{
    bot_state_t *bs = (to >= 0 && to < MAX_CLIENTS) ? botstates[to] : NULL;
    char msg[MAX_MESSAGE_SIZE];

    if (!text[0])
        return;
    if (bs && bs->inuse && bs->cs > 0) {
        Com_sprintf(msg, sizeof(msg), Q2_EC "[%s" Q2_EC "]" Q2_EC ": %s", Q2_ClientNetname(client), text);
        BotQueueConsoleMessage(bs->cs, CMS_CHAT, msg);
    }
    else {
        q2import.BotClientCommand(client, "say_team", (char *)text, NULL);
    }
}

static void Q3BotClientCommand_Adapter(int client, char *command)
{
    static const char *dropped[] = {
        "team", "vsay", "vsay_team", "vtell", "vosay", "vosay_team",
        "votell", "vtaunt", NULL
    };
    static char cmdbuf[256];
    char *space, *args = NULL;
    int i;

    strncpy(cmdbuf, command, sizeof(cmdbuf) - 1);
    cmdbuf[sizeof(cmdbuf) - 1] = '\0';
    space = strchr(cmdbuf, ' ');
    if (space) {
        *space = '\0';
        args = space + 1;
    }
    for (i = 0; dropped[i]; i++) {
        if (!Q_stricmp(cmdbuf, dropped[i]))
            return;
    }
    if (!Q_stricmp(cmdbuf, "tell")) {
        char *text = args;
        int to;

        if (!text)
            return;
        to = atoi(text);
        while (*text == '-' || (*text >= '0' && *text <= '9'))
            text++;
        if (*text == ',')
            text++;
        while (*text == ' ')
            text++;
        Q2BotTell(client, to, text);
        return;
    }
    if (args)
        q2import.BotClientCommand(client, cmdbuf, args, NULL);
    else
        q2import.BotClientCommand(client, cmdbuf, NULL);
}

static int Q3AvailableMemory_Stub(void)
{
    return 0x800000;  /* 8 MB placeholder */
}

static void *Q3HunkAlloc_Adapter(int size)
{
    return q2import.GetMemory(size);
}

/* ---- stdio-backed FS functions ---- */

static int Q3_FS_FOpenFile(const char *qpath, fileHandle_t *file, fsMode_t mode)
{
    char        path[512];
    const char *basedir = LibVarGetString("basedir");
    const char *gamedir = LibVarGetString("gamedir");
    const char *datadir = LibVarGetString("datadir");
    const char *modestr;
    FILE       *f = NULL;
    int         handle, filesize;
    int         pak_base = 0, pak_size = -1;

    switch (mode) {
    case FS_WRITE:       modestr = "wb"; break;
    case FS_APPEND:
    case FS_APPEND_SYNC: modestr = "ab"; break;
    default:             modestr = "rb"; break;
    }

    if (datadir[0]) {
        /* A game that names a data directory ("datadir", relative to the
         * game directory) keeps every file of this library in it: the
         * botfiles, the AAS files and the routing caches, read and written
         * alike. Nothing is looked for anywhere else -- not in baseq2, not
         * in a pak, and not in Gladiator's pak7.pak layout below -- so a
         * game that also runs Gladiator's botlib, whose files have the same
         * names (maps/<map>.aas, items.c, weapons.c, bots/...) in another
         * format, can never hand this library one of them. */
        Com_sprintf(path, sizeof(path), "%s/%s/%s/%s", basedir,
                    gamedir[0] ? gamedir : "baseq2", datadir, qpath);
        f = fopen(path, modestr);
    }
    else {
        if (gamedir[0])
            Com_sprintf(path, sizeof(path), "%s/%s/%s", basedir, gamedir, qpath);
        else
            Com_sprintf(path, sizeof(path), "%s/baseq2/%s", basedir, qpath);

        f = fopen(path, modestr);
        if (!f && gamedir[0]) {
            /* fallback: baseq2 loose file */
            Com_sprintf(path, sizeof(path), "%s/baseq2/%s", basedir, qpath);
            f = fopen(path, modestr);
        }

        /* For read-only access, also search pak archives (e.g. bots/byte_c.c
         * in pak7.pak).  The pak FILE* is stored open; reads are bounded to the
         * entry range by Q3_FS_Read/Seek — no tmpfile or heap copy needed. */
        if (!f && mode == FS_READ) {
            if (gamedir[0]) {
                Com_sprintf(path, sizeof(path), "%s/%s", basedir, gamedir);
                f = Q2_OpenPakEntry(path, qpath, &pak_base, &pak_size);
            }
            if (!f) {
                Com_sprintf(path, sizeof(path), "%s/baseq2", basedir);
                f = Q2_OpenPakEntry(path, qpath, &pak_base, &pak_size);
            }
            /* Fallback: Gladiator pak7.pak stores bot files without the
             * "botfiles/" prefix (e.g. "bots/hunk_c.c" not
             * "botfiles/bots/hunk_c.c").  Strip the prefix and retry. */
            if (!f && strncmp(qpath, "botfiles/", 9) == 0) {
                const char *stripped = qpath + 9;
                /* loose file */
                if (gamedir[0]) {
                    Com_sprintf(path, sizeof(path), "%s/%s/%s", basedir, gamedir, stripped);
                    f = fopen(path, modestr);
                }
                if (!f) {
                    Com_sprintf(path, sizeof(path), "%s/baseq2/%s", basedir, stripped);
                    f = fopen(path, modestr);
                }
                /* pak files */
                if (!f && gamedir[0]) {
                    Com_sprintf(path, sizeof(path), "%s/%s", basedir, gamedir);
                    f = Q2_OpenPakEntry(path, stripped, &pak_base, &pak_size);
                }
                if (!f) {
                    Com_sprintf(path, sizeof(path), "%s/baseq2", basedir);
                    f = Q2_OpenPakEntry(path, stripped, &pak_base, &pak_size);
                }
            }
        }
    }
    if (!f) { *file = 0; return -1; }

    for (handle = 1; handle < MAX_Q2_FS_FILES; handle++) {
        if (!fs_files[handle].file) break;
    }
    if (handle >= MAX_Q2_FS_FILES) { fclose(f); *file = 0; return -1; }

    fs_files[handle].file = f;
    fs_files[handle].base = pak_base;
    fs_files[handle].size = pak_size;
    fs_files[handle].pos  = 0;
    filesize = (pak_size >= 0) ? pak_size
                               : (fseek(f, 0, SEEK_END), (int)ftell(f));
    if (pak_size < 0) fseek(f, 0, SEEK_SET);
    *file = handle;
    return filesize;
}

static int Q3_FS_Read(void *buffer, int len, fileHandle_t h)
{
    q2fsfile_t *s;
    if (h < 1 || h >= MAX_Q2_FS_FILES || !fs_files[h].file) return 0;
    s = &fs_files[h];
    if (s->size >= 0) {
        /* pak entry: clamp to remaining bytes, seek explicitly each call */
        int remaining = s->size - s->pos;
        if (len > remaining) len = remaining;
        if (len <= 0) return 0;
        fseek(s->file, s->base + s->pos, SEEK_SET);
        len = (int)fread(buffer, 1, len, s->file);
        s->pos += len;
        return len;
    }
    return (int)fread(buffer, 1, len, s->file);
}

static int Q3_FS_Write(const void *buffer, int len, fileHandle_t h)
{
    if (h < 1 || h >= MAX_Q2_FS_FILES || !fs_files[h].file) return 0;
    return (int)fwrite(buffer, 1, len, fs_files[h].file);
}

static void Q3_FS_FCloseFile(fileHandle_t h)
{
    if (h < 1 || h >= MAX_Q2_FS_FILES || !fs_files[h].file) return;
    fclose(fs_files[h].file);
    fs_files[h].file = NULL;
}

static int Q3_FS_Seek(fileHandle_t h, long offset, int origin)
{
    q2fsfile_t *s;
    if (h < 1 || h >= MAX_Q2_FS_FILES || !fs_files[h].file) return -1;
    s = &fs_files[h];
    if (s->size >= 0) {
        /* pak entry: update virtual position, no real fseek yet */
        int newpos;
        switch (origin) {
        case FS_SEEK_CUR: newpos = s->pos + (int)offset; break;
        case FS_SEEK_END: newpos = s->size + (int)offset; break;
        default:          newpos = (int)offset; break;
        }
        if (newpos < 0) newpos = 0;
        if (newpos > s->size) newpos = s->size;
        s->pos = newpos;
        return 0;
    }
    {
        int whence;
        switch (origin) {
        case FS_SEEK_CUR: whence = SEEK_CUR; break;
        case FS_SEEK_END: whence = SEEK_END; break;
        default:          whence = SEEK_SET; break;
        }
        return fseek(s->file, offset, whence);
    }
}

static int  Q3DebugPolygonCreate_Stub(int color, int numPoints, vec3_t *points)
{
    (void)color; (void)numPoints; (void)points;
    return 0;
}
static void Q3DebugPolygonDelete_Stub(int id) { (void)id; }

/* ====================================================================
 * Q2 libvar name → Q3 phys_* remapping
 *
 * The Q2 game sets "sv_friction", "sv_gravity" etc. via BotLibVarSet.
 * Q3 botlib uses "phys_friction", "phys_gravity" etc. internally.
 * ==================================================================== */
static const char *Q2LibVarToQ3(const char *name)
{
    static const struct { const char *q2, *q3; } remap[] = {
        { "sv_friction",          "phys_friction"          },
        { "sv_stopspeed",         "phys_stopspeed"         },
        { "sv_gravity",           "phys_gravity"           },
        { "sv_waterfriction",     "phys_waterfriction"     },
        { "sv_watergravity",      "phys_watergravity"      },
        { "sv_maxvelocity",       "phys_maxvelocity"       },
        { "sv_maxwalkvelocity",   "phys_maxwalkvelocity"   },
        { "sv_maxcrouchvelocity", "phys_maxcrouchvelocity" },
        { "sv_maxswimvelocity",   "phys_maxswimvelocity"   },
        { "sv_maxstep",           "phys_maxstep"           },
        { "sv_maxbarrier",        "phys_maxbarrier"        },
        { "sv_maxsteepness",      "phys_maxsteepness"      },
        { "sv_jumpvel",           "phys_jumpvel"           },
        { "sv_maxwaterjump",      "phys_maxwaterjump"      },
        { "sv_airaccelerate",     "phys_airaccelerate"     },
        { "sv_maxacceleration",   "phys_walkaccelerate"    },
        { NULL, NULL }
    };
    int i;
    for (i = 0; remap[i].q2; i++)
        if (!strcmp(name, remap[i].q2)) return remap[i].q3;
    return name;
}

/* ====================================================================
 * Q3 → Q2 action flag bit translation
 *
 * Q3 uses a different bit layout for ACTION_* than Q2.
 * ==================================================================== */
static int Q3ActionsToQ2(int q3)
{
    int q2 = 0;
    if (q3 & 0x0000001) q2 |= Q2_ACTION_ATTACK;
    if (q3 & 0x0000002) q2 |= Q2_ACTION_USE;
    if (q3 & 0x0000008) q2 |= Q2_ACTION_RESPAWN;
    if (q3 & 0x0000010) q2 |= Q2_ACTION_JUMP;
    if (q3 & 0x0000020) q2 |= Q2_ACTION_MOVEUP;
    if (q3 & 0x0000080) q2 |= Q2_ACTION_CROUCH;
    if (q3 & 0x0000100) q2 |= Q2_ACTION_MOVEDOWN;
    if (q3 & 0x0000200) q2 |= Q2_ACTION_MOVEFORWARD;
    if (q3 & 0x0000800) q2 |= Q2_ACTION_MOVEBACK;
    if (q3 & 0x0001000) q2 |= Q2_ACTION_MOVELEFT;
    if (q3 & 0x0002000) q2 |= Q2_ACTION_MOVERIGHT;
    if (q3 & 0x0008000) q2 |= Q2_ACTION_DELAYEDJUMP;
    return q2;
}

/* ====================================================================
 * Q2 export function implementations
 * ==================================================================== */

static char *Q2BotVersion(void)
{
    return "Q3Backport-0.2";
}

/* Q3's AI cvars (game_q3/ai_dmq3.c), refreshed every frame in Q2BotStartFrame
 * as Q3's BotAIStartFrame did; trap_Cvar_Update lives in ai_q2_shim.c. */
extern vmCvar_t bot_rocketjump, bot_grapple, bot_fastchat, bot_nochat, bot_testrchat;
void trap_Cvar_Update(vmCvar_t *cv);

static int Q2BotSetupLibrary(void)
{
    int errnum, maxclients;
    char buf[16];

    /* Every per-client table of this library -- the Q3 AI's botstates[],
     * the adapter's and the shim's (ai_q2_shim.c) -- has MAX_CLIENTS (64)
     * rows, and a Quake II server numbers its clients up to its maxclients,
     * which can be 256. Refuse such a game here, where the game can still
     * say so, rather than index past those tables when the 65th client
     * connects. The game set "maxclients" in BotInitLibrary, before this. */
    maxclients = (int)LibVarGetValue("maxclients");
    if (maxclients > MAX_CLIENTS) {
        botimport.Print(PRT_ERROR, "this bot library supports at most %d "
                        "clients, and maxclients is %d\n", MAX_CLIENTS, maxclients);
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    }

    /* The entity numbers this library works with are Quake III's: clients
     * from 0, the world at ENTITYNUM_WORLD (1022), nothing at or above
     * ENTITYNUM_MAX_NORMAL otherwise (Q2BotUpdateEntity). Its entity tables
     * are sized for that space whatever the game's maxentities is -- a
     * smaller one had no slot for the world, a larger one only rows no
     * entity can reach. */
    Com_sprintf(buf, sizeof(buf), "%d", MAX_GENTITIES);
    LibVarSet("maxentities", buf);

    /* Room for 512 level items, the default of Gladiator's botlib
     * (be_ai_goal.c); Q3's is 256. rdm11 has 256 items in deathmatch, and
     * every item dropped there was "out of level items" to the bots. The
     * game sets max_levelitems when its cvar is set. */
    LibVarValue("max_levelitems", "512");
    errnum = Export_BotLibSetup();
    if (errnum != BLERR_NOERROR)
        return errnum;
    /* the chat files are loaded now: the Q3 AI's game state forgets the
     * answers it kept from any it had before (ai_q2_shim.c) */
    Q2Shim_Reset();
    /* the game set "maxclients" in BotInitLibrary, before this */
    q2_maxclients = (int)LibVarGetValue("maxclients");
    /* Q3's own AI setup (game_q3/ai_main.c): registers the ai_main.c cvars.
     * With restart set it stops before BotInitLibrary, i.e. before setting
     * the library variables and calling BotLibSetup -- the Gladiator game
     * (game_q2/bl_main.c BotInitLibrary) has done both by now. */
    BotAISetup(true);
    return BLERR_NOERROR;
}

static int Q2BotShutdownLibrary(void)
{
    /* A game unloads a library whose setup failed (Q2BotSetupLibrary's
     * refusal, say) by shutting it down: say nothing more about it. */
    if (!botlibglobals.botlibsetup)
        return Q2_BLERR_LIBRARYNOTSETUP;
    /* and with the chat files gone, so are the answers kept from them */
    Q2Shim_Reset();
    return Export_BotLibShutdown();
}

static int Q2BotLibraryInitialized(void)
{
    return botlibglobals.botlibsetup;
}

/* The Gladiator game sets the server switches under Gladiator's library
 * variable names, once, in game_q2/bl_main.c BotInitLibrary: "nochat 1" and
 * "rocketjump 1" when set, and "fastchat 0" when fastchat is set, which leaves
 * it off -- as it did for Gladiator's botlib, whose defaults (nochat 0,
 * fastchat 0, rocketjump 1) are the Q3 AI's. The ported Q3 AI reads Q3's
 * bot_* cvars (BotSetupDeathmatchAI, ai_dmq3.c). Both are kept: the botlib
 * itself still reads "nochat" (be_ai_chat.c, reply chats). "usehook" is
 * deliberately not mapped to bot_grapple: Q3's grapple travel needs a grapple
 * weapon number and hook entity this adapter does not provide
 * (weapindex_grapple 0, see GetBotAPI). */
static const char *Q2LibVarToQ3AI(const char *name)
{
    static const struct { const char *q2, *q3; } remap[] = {
        { "rocketjump", "bot_rocketjump" },
        { "nochat",     "bot_nochat"     },
        { "fastchat",   "bot_fastchat"   },
        { NULL, NULL }
    };
    int i;
    for (i = 0; remap[i].q2; i++)
        if (!strcmp(name, remap[i].q2)) return remap[i].q3;
    return NULL;
}

static int Q2BotLibVarSet(char *var_name, char *value)
{
    const char *ai_name = Q2LibVarToQ3AI(var_name);

    if (ai_name)
        Export_BotLibVarSet((char *)ai_name, value);
    return Export_BotLibVarSet((char *)Q2LibVarToQ3(var_name), value);
}

static int Q2BotDefine(char *string)
{
    return PC_AddGlobalDefine(string);
}

/* Shared by Q2BotStartFrame (every frame) and Q2BotLoadMap (once, right
 * before BotSetupDeathmatchAI needs a correct answer -- see there). There
 * is no real Q2 concept named "g_gametype", so this derives Q3's numbering
 * from the game DLL's "ctf", "teamplay" and "dmflags" LibVars. Rocket
 * Arena's "ra" is not one of them: the Gladiator game's arenas play in
 * teams by DF_SKINTEAMS or DF_MODELTEAMS like any deathmatch
 * (game_q2/g_arena.c) and are free-for-all without them, a game whose
 * arenas always play in teams sets "teamplay" (Colosseum does), and
 * Gladiator's botlib never reads "ra" either. Taken for GT_TEAM, a
 * free-for-all arena would have every bot ask all players for a team
 * leader and keep its deathmatch chats to itself. */
/* The switches of Gladiator's team rules (Q2_ClientsOnSameTeam), read with
 * the gametype below */
#define Q2_DF_SKINTEAMS     64
#define Q2_DF_MODELTEAMS    128
#define Q2_RF_SHELLS        (0x400 | 0x800 | 0x1000)  /* RF_SHELL_RED/GREEN/BLUE */
static int q2_team_shell, q2_team_ch, q2_team_teamplay, q2_team_ctf, q2_team_dmflags;

static void Q2UpdateGametypeLibVar(void)
{
    float ctf_val = LibVarGetValue("ctf");
    float tp_val  = LibVarGetValue("teamplay");
    /* Q2 deathmatch plays in teams with DF_SKINTEAMS (64) or DF_MODELTEAMS
     * (128) (game_q2/q_shared.h); the game sends "dmflags" every frame
     * (game_q2/bl_main.c BotLib_BotStartFrame). Q2_ClientsOnSameTeam, which
     * BotSameTeam asks, knows both. */
    int dmflags   = (int)LibVarGetValue("dmflags");

    q2_team_shell    = LibVarGetValue("teamplay_shell") != 0;
    q2_team_ch       = LibVarGetValue("ch") != 0;
    q2_team_teamplay = tp_val != 0;
    q2_team_ctf      = ctf_val != 0;
    q2_team_dmflags  = dmflags;
    if (ctf_val)
        LibVarSet("g_gametype", "4"); /* GT_CTF */
    else if (tp_val || (dmflags & (64 | 128)))
        LibVarSet("g_gametype", "3"); /* GT_TEAM */
    else
        LibVarSet("g_gametype", "0"); /* GT_FFA */
}

/* Cached for trap_GetServerinfo (ai_q2_shim.c), whose only consumer is
 * BotMapTitle() (ai_chat.c) wanting "mapname". Mirrors game_q2/bl_chat.c's
 * now-deleted trap_GetServerinfo, which read Q2's real level.mapname
 * directly from game.so; botlib.so has no such access, but Q2BotLoadMap
 * below already receives the real mapname as an argument every map load. */
char q2_cached_mapname[64] = "";

static int Q2BotLoadMap(char *mapname, int nummodelindexes, char *modelindex[],
                         int soundindexes, char *soundindex[],
                         int imageindexes, char *imageindex[])
{
    int errnum;
    (void)soundindexes; (void)soundindex;

    /* The game DLL calls BotLoadMap twice:
     * 1. From g_spawn.c with the real mapname (after entity spawning)
     * 2. From bl_redirgi.c with NULL mapname (when new models are precached)
     *
     * On the real call: load BSP, load AAS, mark items, link models.
     * On the NULL reload: only re-link model indices (the table may have
     * grown since the initial call). */

    if (mapname) {
        Q_strncpyz(q2_cached_mapname, mapname, sizeof(q2_cached_mapname));

        /* the Q3 AI's game state starts over with the map (ai_q2_shim.c) */
        Q2Shim_Reset();
        Com_Memset(q2_ctfteam, 0, sizeof(q2_ctfteam));

        /* Read BSP entity lump from disk before calling Q3's load */
        Q2_ReadBSPEntityData(mapname);

        errnum = Export_BotLibLoadMap(mapname);
        if (errnum != BLERR_NOERROR) return errnum;

        /* Force AAS to finish initializing before returning, instead of
         * leaving it to finish incrementally across future
         * Export_BotLibStartFrame calls (AAS_ContinueInit/
         * AAS_ContinueInitReachability, be_aas_main.c/be_aas_reach.c).
         *
         * Discovered the hard way via real in-game testing (see report):
         * game_q2 lazily dlopen()s this library and calls BotLoadMap ->
         * BotSetupClient back-to-back the first time "addbot"/a bot-queue
         * entry is processed, all within the same server frame, with zero
         * intervening Export_BotLibStartFrame calls. The real
         * BotAISetupClient (game_q3/ai_main.c) now correctly refuses to
         * set up a client while AAS_Initialized() is false -- a real
         * safety check the old hand-rolled Q2BotSetupClient never had --
         * so without this, the very first bot added after any map load
         * always fails ("AAS not initialized"), and game_q2's bl_spawn.c
         * unloads the library entirely on that failure, repeating the
         * same race on every subsequent attempt forever.
         *
         * AAS_StartFrame -> AAS_ContinueInit only needs to actually run
         * once for a bspc-precompiled .aas file like this project ships
         * (AAS_InitReachability, be_aas_reach.c, already marks
         * reachability as done from the file's own data unless
         * "forcereachability" is set) -- the loop is just a safety margin
         * in case reachability genuinely does need to compute
         * incrementally (e.g. an .aas file with no baked-in reachability
         * data), bounded so a pathological map can't hang BotLoadMap
         * forever. */
        {
            int warmup;
            for (warmup = 0; warmup < 200 && !AAS_Initialized(); warmup++) {
                AAS_StartFrame(AAS_Time() + warmup * 0.01f);
            }
            if (!AAS_Initialized()) {
                botimport.Print(PRT_WARNING,
                    "BotLoadMap: AAS did not finish initializing after %d warmup frames\n",
                    warmup);
            }
        }

        /* --- Real Q3's BotAILoadMap (game_q3/ai_main.c) equivalent ---
         * game_q2/bl_main.c's BotInitLibrary already calls
         * BotLibVarSet("maxclients", ...) before BotSetupLibrary/BotLoadMap
         * ever run, so "maxclients" is already correct by this point;
         * BotSetupDeathmatchAI (game_q3/ai_dmq3.c) reads a DIFFERENT
         * LibVar name ("sv_maxclients", the real Q3 server cvar) into its
         * own plain `maxclients` global -- mirror the value across so
         * that read resolves correctly without hand-editing ai_dmq3.c.
         * Likewise recompute "g_gametype" here (not just once per frame
         * in Q2BotStartFrame) so BotSetupDeathmatchAI's own one-time
         * `gametype = trap_Cvar_VariableIntegerValue("g_gametype")` read
         * sees this map's real value, not a stale one from a previous
         * map or the "0" seeded at library setup. */
        LibVarSet("sv_maxclients", LibVarGetString("maxclients"));
        q2_maxclients = (int)LibVarGetValue("maxclients");
        Q2UpdateGametypeLibVar();

        /* Q3's own map setup for the AI (game_q3/ai_main.c): resets the
         * bots already in the game, then BotSetupDeathmatchAI registers the
         * AI cvars, sets the gametype/maxclients globals, looks up the CTF
         * flag goals and rebuilds the waypoint list -- in that order, since
         * BotResetState frees each bot's waypoints into the list
         * BotInitWaypoints rebuilds. With restart set it leaves out loading
         * the map, done above with the Q2 specifics. */
        BotAILoadMap(true);
    }

    /* Link item model indices from the game DLL's modelindexes[] table.
     *
     * Runs on EVERY BotLoadMap call (including NULL-mapname reloads)
     * because the table grows as models are precached.  The initial
     * call from g_spawn.c has most models; the reload calls from
     * Bot_modelindex add any late-precached models.
     *
     * This enables BotUpdateEntityItems to recognise dropped weapons
     * by their runtime modelindex later. */
    if (modelindex && nummodelindexes > 0) {
        BotLinkItemModelIndicesFromTable(nummodelindexes, modelindex);
    }

    /* Keep the table for Q2_WeaponForGunIndex. It is the game's own array,
     * valid for the whole level; forget earlier lookups since entries may
     * have been added or the map changed. */
    {
        int i;
        q2_modelnames    = modelindex;
        q2_nummodelnames = (nummodelindexes < Q2_MAX_MODELINDEXES) ? nummodelindexes : Q2_MAX_MODELINDEXES;
        for (i = 0; i < Q2_MAX_MODELINDEXES; i++)
            q2_gunweapon[i] = -1;
    }
    /* The image table likewise, for Q2_TrackPowerups. */
    q2_imagenames    = imageindex;
    q2_numimagenames = (imageindexes < Q2_MAX_IMAGEINDEXES) ? imageindexes : Q2_MAX_IMAGEINDEXES;

    return BLERR_NOERROR;
}

/* Build a real Q3 bot_settings_t from the frozen q2_bot_settings_t ABI
 * struct. settings->charactername has no home in bot_settings_t (real
 * Q3 derives the bot's chat name from the character file itself via
 * ClientName()/CHARACTERISTIC_CHAT_NAME, called from BotDeathmatchAI's
 * one-time setup block, not from a passed-in settings struct) -- see the
 * report for the trap_GetConfigstring gap this exposes.
 * settings->team has no source in the frozen ABI at all; Q2's own CTF
 * team assignment (CTFAssignTeam, game_q2/p_client.c) already runs
 * before BotSetupClient (bl_spawn.c's BotLib_BotSetupClient), so leaving
 * it empty and letting the ported code's own `team ""` EA_Command no-op
 * is safe (plan's Phase 4 note). */
static void Q2BuildBotSettings(bot_settings_t *out, const q2_bot_settings_t *in)
{
    float skill = LibVarGetValue("bot_skill");
    if (skill < 1) skill = 4;   /* default: skilled but not expert */
    if (skill > 5) skill = 5;

    Com_Memset(out, 0, sizeof(*out));
    Q_strncpyz(out->characterfile, in->characterfile, sizeof(out->characterfile));
    out->skill = skill;
    out->team[0] = '\0';
}

static int Q2BotSetupClient(int client, q2_bot_settings_t *settings)
{
    bot_settings_t bs_settings;

    if (client < 0 || client >= MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;

    if (botstates[client] && botstates[client]->inuse) {
        botimport.Print(PRT_WARNING,
            "BotSetupClient: client %d already setup\n", client);
        return Q2_BLERR_AICLIENTALREADYSETUP;
    }

    Q2BuildBotSettings(&bs_settings, settings);
    q2_lastweaponcmd[client] = 0;
    q2_lastweapontime[client] = 0;
    q2_lastpmflags[client] = 0;
    q2_appliedflagstatus[client] = -1;
    Com_Memset(&q2_powerupstate[client], 0, sizeof(q2_powerupstate[client]));
    q2_ctfteam[client] = Q3TEAM_FREE;
    Q2Shim_ClientHurt(client, 0, 0);

    /* The real BotAISetupClient (game_q3/ai_main.c) loads the character
     * file, allocates goal/weapon/chat/move states, and caches the
     * personality traits itself (trap_Characteristic_BFloat calls spread
     * throughout ai_dmq3.c) -- nothing left to hand-roll here. Returns
     * true/false, not a BLERR_ code (matches q2_bot_export_t.BotSetupClient's
     * documented contract). */
    return BotAISetupClient(client, (struct bot_settings_s *)&bs_settings, false);
}

static int Q2BotShutdownClient(int client)
{
    if (client < 0 || client >= MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;

    if (!botstates[client] || !botstates[client]->inuse) {
        botimport.Print(PRT_WARNING,
            "BotShutdownClient: client %d not setup\n", client);
        return Q2_BLERR_AICLIENTALREADYSHUTDOWN;
    }

    BotAIShutdownClient(client, false);
    q2_ctfteam[client] = Q3TEAM_FREE;
    Q2Shim_ClientHurt(client, 0, 0);
    return Q2_BLERR_NOERROR;
}

/* Every client's name as the game reports it (BotClientSettings, on
 * connect and on every userinfo change) -- humans' too, which Q3 reads
 * from the CS_PLAYERS configstrings Q2 does not have: the AI names its
 * killer, its victims and its opponents in chat through ClientName (see
 * trap_GetConfigstring in ai_q2_shim.c). */
static char q2_netnames[MAX_CLIENTS][Q2_MAX_NETNAME];

/* and every client's skin, which tells its team (Q2_ClientsOnSameTeam), with
 * the shell colour and the modelindex3 of its entity for the two coloured
 * modes there */
static char q2_skins[MAX_CLIENTS][Q2_MAX_CLIENTSKINNAME];
static int  q2_clientrenderfx[MAX_CLIENTS];
static int  q2_clientmodelindex3[MAX_CLIENTS];

const char *Q2_ClientNetname(int client)
{
    if (client < 0 || client >= MAX_CLIENTS)
        return "";
    return q2_netnames[client];
}

static int Q2BotMoveClient(int oldclnum, int newclnum)
{
    bot_state_t *bs;

    if (oldclnum < 0 || oldclnum >= MAX_CLIENTS)
        return Q2_BLERR_AIMOVEINACTIVECLIENT;
    if (newclnum < 0 || newclnum >= MAX_CLIENTS)
        return Q2_BLERR_AIMOVETOACTIVECLIENT;

    bs = botstates[oldclnum];
    if (!bs || !bs->inuse) return Q2_BLERR_AIMOVEINACTIVECLIENT;

    /* Real Q3 has no equivalent "move a live bot to a different client
     * slot" concept -- botstates[] is indexed by client number for the
     * bot's whole lifetime. This ABI entry point only exists for a
     * Gladiator-bot-era mechanism (game_q2/bl_spawn.c's
     * BotMoveToFreeClientEdict). Move the allocated bot_state_t itself
     * between slots and fix up the two fields that encode the slot
     * number. */
    botstates[newclnum] = bs;
    botstates[oldclnum] = NULL;
    bs->client    = newclnum;
    bs->entitynum = newclnum;
    q2_lastweaponcmd[newclnum] = q2_lastweaponcmd[oldclnum];
    q2_lastweaponcmd[oldclnum] = 0;
    q2_lastweapontime[newclnum] = q2_lastweapontime[oldclnum];
    q2_lastweapontime[oldclnum] = 0;
    q2_lastpmflags[newclnum] = q2_lastpmflags[oldclnum];
    q2_lastpmflags[oldclnum] = 0;
    q2_appliedflagstatus[newclnum] = q2_appliedflagstatus[oldclnum];
    q2_appliedflagstatus[oldclnum] = -1;
    q2_powerupstate[newclnum] = q2_powerupstate[oldclnum];
    Com_Memset(&q2_powerupstate[oldclnum], 0, sizeof(q2_powerupstate[oldclnum]));
    q2_ctfteam[newclnum] = q2_ctfteam[oldclnum];
    q2_ctfteam[oldclnum] = Q3TEAM_FREE;
    Q2Shim_MoveClient(oldclnum, newclnum);
    /* The chat state speaks through its client number (BotEnterChat), and
     * the game refreshed the settings before the move (game_q2/bl_spawn.c). */
    Q_strncpyz(q2_netnames[newclnum], q2_netnames[oldclnum], sizeof(q2_netnames[newclnum]));
    Q_strncpyz(q2_skins[newclnum], q2_skins[oldclnum], sizeof(q2_skins[newclnum]));
    q2_clientrenderfx[newclnum] = q2_clientrenderfx[oldclnum];
    q2_clientmodelindex3[newclnum] = q2_clientmodelindex3[oldclnum];
    BotSetChatName(bs->cs, q2_netnames[newclnum], newclnum);

    return Q2_BLERR_NOERROR;
}


static int Q2BotClientSettings(int client, q2_bot_clientsettings_t *settings)
{
    bot_state_t *bs;

    if (client < 0 || client >= MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    Q_strncpyz(q2_netnames[client], settings->netname, sizeof(q2_netnames[client]));
    Q_strncpyz(q2_skins[client], settings->skin, sizeof(q2_skins[client]));
    bs = botstates[client];
    if (!bs || !bs->inuse) return Q2_BLERR_SETTINGSINACTIVECLIENT;

    BotSetChatName(bs->cs, settings->netname, client);
    return Q2_BLERR_NOERROR;
}

static int Q2BotSettings(int client, q2_bot_settings_t *settings)
{
    bot_state_t *bs;

    if (client < 0 || client >= MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    bs = botstates[client];
    if (!bs || !bs->inuse) return Q2_BLERR_SETTINGSINACTIVECLIENT;

    Q2BuildBotSettings(&bs->settings, settings);
    return Q2_BLERR_NOERROR;
}

/* ====================================================================
 * Phase 2 (Problem 1 fix): CTF flag at-base/away status.
 *
 * Real Q3 feeds bs->redflagstatus/blueflagstatus (ai_main.h's own field
 * comments literally say "0 = at base, 1 = not at base" -- a plain
 * boolean, unlike the 4-state neutralflagstatus) from a CS_FLAGSTATUS
 * configstring broadcast this Q2 port has no equivalent of
 * (trap_GetConfigstring is a permanent stub -- see ai_q2_shim.c).
 *
 * Derived here instead from data already flowing through the existing
 * per-frame entity feed: ctf_redflag.entitynum/ctf_blueflag.entitynum
 * (game_q3/ai_dmq3.c bot_goal_t globals, already populated by the
 * existing trap_BotGetLevelItemGoal(-1,"Red Flag"/"Blue Flag",...) call
 * in BotSetupDeathmatchAI) give each flag ITEM's live AAS entity number.
 *
 * Verified against the real game_q2/g_ctf.c (not guessed): a Q2 CTF flag
 * has exactly ONE on-field representation while at home -- the entity
 * spawned by CTFFlagSetup(), solid=SOLID_TRIGGER, no SVF_NOCLIENT. The
 * instant it's taken (CTFPickup_Flag): `ent->svflags |= SVF_NOCLIENT;
 * ent->solid = SOLID_NOT;` -- and it stays exactly that way, whether
 * currently carried or lying dropped elsewhere as a SEPARATE entity
 * spawned by CTFDeadDropFlag, until CTFResetFlag() restores it on
 * capture/return/auto-return. So the original flag entity's solid state
 * alone is a complete, correct 0/1 signal; no need to locate the
 * separate dropped-item entity at all.
 *
 * This is reinforced by a second, independent signal: game_q2/g_main.c's
 * per-frame entity feed loop SKIPS calling BotUpdateEntity() entirely for
 * any SVF_NOCLIENT entity (`if (!(ent->svflags & SVF_NOCLIENT))
 * BotLib_BotUpdateEntity(ent);`, g_main.c:565), and botlib/be_aas_entity.c's
 * AAS_StartFrame() invalidates every AAS entity (.valid=false) at the top
 * of each server frame, only setting it back to true for entities that
 * get a fresh update that same frame -- so a taken flag's entity also
 * reports .valid=false from the very next frame onward, for as long as
 * it's hidden.
 *
 * IMPORTANT ordering requirement: Q2AI_UpdateCTFFlagStatus() must run
 * BEFORE Export_BotLibStartFrame() each frame (see Q2BotStartFrame
 * below). That call cascades straight into AAS_StartFrame()'s invalidate
 * pass for THIS frame, and this frame's real entity updates
 * (game_q2/g_main.c's loop) don't run until AFTER BotStartFrame returns
 * (g_main.c:522 vs :560-569). Reading AAS_EntityInfo() any time after
 * that invalidate call but before this frame's updates land would see
 * EVERY entity, flags included, as freshly invalidated and not yet
 * re-validated -- permanently "gone", every single frame, regardless of
 * the truth. Running before that call instead observes the fully-settled
 * result of the PREVIOUS frame's feed: one server frame of latency,
 * imperceptible for a binary status flag.
 * ==================================================================== */
extern bot_goal_t ctf_redflag;
extern bot_goal_t ctf_blueflag;
extern int        gametype;

static int q2_redflagstatus;
static int q2_blueflagstatus;

static int Q2AI_FlagAwayFromBase(bot_goal_t *flaggoal)
{
    aas_entityinfo_t info;
    vec3_t           delta;

    if (flaggoal->entitynum <= 0) return 0;

    AAS_EntityInfo(flaggoal->entitynum, &info);
    if (!info.valid || info.solid == SOLID_NOT) return 1;

    /* Defense in depth (per the plan): if some future map/mod variant
     * keeps the flag "valid" and solid while away from base instead of
     * hiding it Q2-CTF-style, catch it via displacement from its cached
     * spawn origin too. Q2 CTF flags don't otherwise move while at rest
     * (CTFFlagSetup settles them once at load time and nothing
     * re-simulates them afterwards), so this generous threshold won't
     * false-positive on ordinary physics settling noise. */
    VectorSubtract(info.origin, flaggoal->origin, delta);
    if (VectorLength(delta) > 64.0f) return 1;

    return 0;
}

static void Q2AI_UpdateCTFFlagStatus(void)
{
    if (gametype != 4 /* GT_CTF, see Q2UpdateGametypeLibVar */ || !AAS_Initialized()) {
        q2_redflagstatus  = 0;
        q2_blueflagstatus = 0;
        return;
    }
    /* BotSetupDeathmatchAI copies the flag goals once, at map load, before
     * BotUpdateEntityItems has linked the flag level items to their
     * entities -- the copies then carry entity number 0, which
     * Q2AI_FlagAwayFromBase reads as "at base" forever. Re-fetch until the
     * link exists. */
    if (ctf_redflag.entitynum <= 0)
        BotGetLevelItemGoal(-1, "Red Flag", &ctf_redflag);
    if (ctf_blueflag.entitynum <= 0)
        BotGetLevelItemGoal(-1, "Blue Flag", &ctf_blueflag);
    q2_redflagstatus  = Q2AI_FlagAwayFromBase(&ctf_redflag);
    q2_blueflagstatus = Q2AI_FlagAwayFromBase(&ctf_blueflag);
}

/* Whether two clients play on one team (client numbers, BotSameTeam in
 * ai_dmq3.c), decided as Gladiator's botlib did (be_ai2_dmq2.c BotSameTeam)
 * from what the game sends. Q2 tells teams apart by the skins of
 * BotClientSettings: the skin after the model with DF_SKINTEAMS and in CTF,
 * whose teams wear ctf_r and ctf_b (g_ctf.c CTFAssignSkin), the model with
 * DF_MODELTEAMS, the whole skin in the game's teamplay mode. Gladiator's two
 * coloured modes go by the entities: the shell colour (teamplay_shell) or
 * modelindex3 (Colored Hitman, g_ch.c, where a player of another colour is
 * no target). */
int Q2_ClientsOnSameTeam(int client1, int client2)
{
    aas_entityinfo_t entinfo;
    const char *skin1, *skin2, *slash1, *slash2;
    int len1, len2;

    if (client1 < 0 || client1 >= q2_maxclients || client2 < 0 || client2 >= q2_maxclients)
        return false;
    AAS_EntityInfo(client2, &entinfo);
    if (!entinfo.valid)
        return false;
    if (q2_team_shell)
        return (q2_clientrenderfx[client1] & Q2_RF_SHELLS) ==
               (q2_clientrenderfx[client2] & Q2_RF_SHELLS);
    if (q2_team_ch)
        return q2_clientmodelindex3[client1] != q2_clientmodelindex3[client2];
    skin1 = q2_skins[client1];
    skin2 = q2_skins[client2];
    if (q2_team_teamplay)
        return !Q_stricmp(skin1, skin2);
    slash1 = strchr(skin1, '/');
    slash2 = strchr(skin2, '/');
    if ((q2_team_dmflags & Q2_DF_SKINTEAMS) || q2_team_ctf)
        return !Q_stricmp(slash1 ? slash1 : skin1, slash2 ? slash2 : skin2);
    if (q2_team_dmflags & Q2_DF_MODELTEAMS) {
        len1 = slash1 ? (int)(slash1 - skin1) : (int)strlen(skin1);
        len2 = slash2 ? (int)(slash2 - skin2) : (int)strlen(skin2);
        return len1 == len2 && !strncmp(skin1, skin2, len1);
    }
    return false;
}

/* #6 — BotUpdateEntityItems timing: Q3 calls this at ~0.3s intervals
 * via BotAIRegularUpdate(), not every frame.  We throttle here to match. */
static float q2_entityitems_time;

static int Q2BotStartFrame(float time)
{
    int ret;

    /* Must run BEFORE Export_BotLibStartFrame(): see the long comment on
     * Q2AI_UpdateCTFFlagStatus() above for why (that call's
     * AAS_StartFrame -> AAS_InvalidateEntities() cascade would otherwise
     * make every entity, flags included, look "gone" for the rest of this
     * function, every single frame). */
    Q2AI_UpdateCTFFlagStatus();

    /* Same constraint for the item scan: real Q3 runs it (BotAIRegularUpdate)
     * AFTER that frame's entity updates; here those only arrive after this
     * function returns (game_q2/g_main.c), so scan the previous frame's
     * settled entities instead. Called after the invalidation, it found no
     * entity at all -- no level item was ever linked to its entity, so bots
     * could not see that an item was gone, and dropped weapons never became
     * goals. The level clock restarts on a map change. */
    if (AAS_Time() < q2_entityitems_time)
        q2_entityitems_time = 0;
    if (AAS_Time() - q2_entityitems_time >= 0.3f) {
        BotUpdateEntityItems();
        q2_entityitems_time = AAS_Time();
    }

    ret = Export_BotLibStartFrame(time);

    /* The AI's switches, updated every frame as Q3's BotAIStartFrame did
     * (the game sets them in BotInitLibrary, see Q2LibVarToQ3AI). */
    trap_Cvar_Update(&bot_rocketjump);
    trap_Cvar_Update(&bot_grapple);
    trap_Cvar_Update(&bot_fastchat);
    trap_Cvar_Update(&bot_nochat);
    trap_Cvar_Update(&bot_testrchat);

    /* Real Q3's BotAIStartFrame (game_q3/ai_main.c) -- deleted in Phase 0
     * as Q3-engine-shaped dead code -- was the ONLY place floattime ever
     * got assigned (floattime = trap_AAS_Time();); every ported file's
     * FloatTime() macro (ai_main.h: #define FloatTime() floattime) reads
     * it. Without this, FloatTime() would silently return 0 forever and
     * every timer-driven decision in ai_dmnet.c/ai_dmq3.c/ai_team.c/
     * ai_chat.c would misbehave -- confirmed against ioq3's real
     * BotAIStartFrame (code/game/ai_main.c:1551) to be sure this is what
     * real Q3 does, not a guess. */
    floattime = AAS_Time();

    /* Update gametype for CTF/team detection from the game's "ctf",
     * "teamplay" and "dmflags" LibVars. */
    Q2UpdateGametypeLibVar();

    /* The Q3 AI's game state: this frame's entity updates follow
     * (game_q2/g_main.c); expired temporary events go. */
    Q2Shim_StartFrame();
    return ret;
}

/* The weapon config number (assets/botfiles/weapons.c) of the weapon a bot
 * holds, from the view model the game shows it: Q2 sets ps.gunindex to the
 * held weapon's view model index, and each weaponinfo's "model" is that
 * weapon's view model. 0 when nothing is held (dead, weapon down) or the
 * model is unknown. */
static int Q2_WeaponForGunIndex(bot_state_t *bs, int gunindex)
{
    weaponinfo_t wi;
    int w, maxweapons;

    if (gunindex <= 0 || gunindex >= q2_nummodelnames || !q2_modelnames ||
        !q2_modelnames[gunindex])
        return 0;
    if (q2_gunweapon[gunindex] >= 0)
        return q2_gunweapon[gunindex];

    q2_gunweapon[gunindex] = 0;
    /* weapon numbers are 1..max_weaponinfo-1 (be_ai_weap.c) */
    maxweapons = (int)LibVarGetValue("max_weaponinfo");
    for (w = 1; w < maxweapons; w++) {
        Com_Memset(&wi, 0, sizeof(wi));
        BotGetWeaponInfo(bs->ws, w, &wi);
        if (wi.valid && !Q_stricmp(wi.model, q2_modelnames[gunindex])) {
            q2_gunweapon[gunindex] = w;
            break;
        }
    }
    return q2_gunweapon[gunindex];
}

/* Name of a HUD image index (bl_redirgi.c imageindexes[]), or NULL. */
static const char *Q2_ImageName(int index)
{
    if (index <= 0 || index >= q2_numimagenames || !q2_imagenames)
        return NULL;
    return q2_imagenames[index];
}

/* Which powerups a bot holds and which run, for Q2BotUsePowerups -- read
 * off the HUD like Gladiator's BotUpdateInventory did: G_SetStats shows one
 * running powerup in STAT_TIMER_ICON/STAT_TIMER (the first of quad,
 * DualFire, Double Damage, invulnerability, suit, rebreather, sphere, IR
 * goggles) and "i_powershield" as STAT_ARMOR_ICON while power armor is on.
 *
 * The ported Q3 AI reads INVENTORY_QUAD (BotAggression, no rocket jumps)
 * and INVENTORY_ENVIRONMENTSUIT (BotCheckAir) as "has it running" -- Q3
 * applies powerups at pickup -- while Q2's slot counts the unused ones. A
 * running quad or suit is therefore added to its slot. */
static void Q2_TrackPowerups(bot_state_t *bs, int client, q2_bot_updateclient_t *buc)
{
    q2_powerupstate_t *pu = &q2_powerupstate[client];
    const char *icon;
    float now = AAS_Time();
    int i;

    /* AAS time starts over with every level; so does everything here */
    if (now < pu->updated)
        Com_Memset(pu, 0, sizeof(*pu));
    pu->updated = now;

    for (i = 0; i < Q2PU_NUM; i++) {
        pu->held[i] = buc->inventory[q2_powerups[i].slot];
        /* nothing runs any more for a player that died */
        if (buc->stats[1] <= 0)             /* Q2 STAT_HEALTH */
            pu->until[i] = 0;
    }
    icon = Q2_ImageName(buc->stats[9]);     /* Q2 STAT_TIMER_ICON */
    for (i = 0; icon && i < Q2PU_NUM; i++) {
        if (q2_powerups[i].icon && !Q_stricmp(icon, q2_powerups[i].icon))
            pu->until[i] = now + buc->stats[10];  /* Q2 STAT_TIMER, seconds */
    }
    icon = Q2_ImageName(buc->stats[4]);     /* Q2 STAT_ARMOR_ICON */
    if (icon && !Q_stricmp(icon, "i_powershield"))
        pu->powerarmor_seen = now;
    if (buc->inventory[INVENTORY_POWERSCREEN] > 0 ||
        buc->inventory[INVENTORY_POWERSHIELD] > 0) {
        if (!pu->powerarmor_since)
            pu->powerarmor_since = now;
    }
    else
        pu->powerarmor_since = 0;

    if (pu->until[Q2PU_QUAD] > now)
        bs->inventory[INVENTORY_QUAD]++;
    if (pu->until[Q2PU_ENVIRO] > now)
        bs->inventory[INVENTORY_ENVIRONMENTSUIT]++;
}

static int Q2BotUpdateClient(int client, q2_bot_updateclient_t *buc)
{
    bot_state_t      *bs;
    int               i;

    if (client < 0 || client >= MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    bs = botstates[client];
    if (!bs || !bs->inuse) return Q2_BLERR_AIUPDATEINACTIVECLIENT;

    /* --- Populate bs->cur_ps (the REAL Q3 playerState_t) with exactly
     * the fields the real ported code reads this frame: BotAI() (delta
     * angle math), BotSetupForMovement/BotIsDead/BotIsObserver/
     * BotIntermission (game_q3/ai_dmq3.c) -- confirmed by reading each of
     * those functions directly. Everything else in playerState_t
     * (stats[]/ammo[]/powerups[]/weapon/etc.) is intentionally left
     * zeroed: BotUpdateInventory's replacement (game_q3/ai_dmq3.c) reads
     * bs->inventory[] instead (populated below), and no other ported
     * code path was found to depend on the rest. */
    VectorCopy(buc->origin,   bs->cur_ps.origin);
    VectorCopy(buc->velocity, bs->cur_ps.velocity);
    bs->cur_ps.viewheight = (int)buc->viewoffset[2];
    /* delta_angles: game_q2/bl_main.c copies Q2's pmove.delta_angles, which
     * are SHORT-encoded angles, straight into these floats, and then clears
     * them (BotSetPMoveState) -- so a nonzero value arrives exactly once, on
     * the frame of a respawn or teleport, and means "the view was turned by
     * this much". Apply it to the bot's view once, as the Gladiator botlib's
     * BotUpdateClient did, and keep cur_ps.delta_angles at zero: the real
     * BotAI()/BotUpdateInput() (game_q3/ai_main.c) add and subtract
     * cur_ps.delta_angles around each think, which assumes Q3's persistent
     * deltas and would take a one-shot Q2 delta back out a frame later. */
    for (i = 0; i < 3; i++) {
        bs->viewangles[i] = AngleMod(bs->viewangles[i] + SHORT2ANGLE((short)buc->delta_angles[i]));
        bs->cur_ps.delta_angles[i] = 0;
    }

    switch (buc->pm_type) {
        case Q2PM_DEAD:
        case Q2PM_GIB:       bs->cur_ps.pm_type = Q3PM_DEAD;      break;
        case Q2PM_SPECTATOR: bs->cur_ps.pm_type = Q3PM_SPECTATOR; break;
        case Q2PM_FREEZE:    bs->cur_ps.pm_type = Q3PM_FREEZE;    break;
        default:              bs->cur_ps.pm_type = Q3PM_NORMAL;    break;
    }

    /* Q2's PMF_* bit positions (game_q2/q_shared.h) differ from Q3's
     * (botlib/ai_q2_compat.h) -- translate by meaning, not by raw value.
     * PMF_TIME_TELEPORT -> PMF_TIME_KNOCKBACK: real Q3's
     * BotSetupForMovement checks PMF_TIME_KNOCKBACK (not a dedicated
     * teleport flag) + pm_time>0 to detect "just displaced, don't fight
     * movement prediction" -- Q2's PMF_TIME_TELEPORT is the equivalent
     * "pm_time is non-moving time after a non-normal move" signal. */
    {
        int flags = 0;
        if (buc->pm_flags & 1)  flags |= Q3PMF_DUCKED;        /* Q2 PMF_DUCKED */
        if (buc->pm_flags & 8)  flags |= Q3PMF_TIME_WATERJUMP;/* Q2 PMF_TIME_WATERJUMP */
        if (buc->pm_flags & 32) flags |= Q3PMF_TIME_KNOCKBACK;/* Q2 PMF_TIME_TELEPORT */
        bs->cur_ps.pm_flags = flags;
    }
    /* A teleport or a respawn: Q2 holds the player in place with
     * PMF_TIME_TELEPORT (p_client.c respawn, g_misc.c teleporter_touch).
     * Q3 marks both by toggling EF_TELEPORT_BIT, which BotSetTeleportTime
     * turns into the bot's reaction pause after its own teleport, and with
     * an EV_PLAYER_TELEPORT_IN temporary entity (see Q2Shim_TempEvent for
     * why its teleport grace stays as inert as in Q3). The game runs a bot's
     * teleports after the frame's entity updates, so the event is made here
     * (a human's comes with the entity update, see Q2Shim_UpdateEntity). */
    if ((buc->pm_flags & 32) && !(q2_lastpmflags[client] & 32)) {
        bs->cur_ps.eFlags ^= Q3EF_TELEPORT_BIT;
        Q2Shim_TeleportIn(client, buc->origin);
    }
    q2_lastpmflags[client] = buc->pm_flags;
    bs->cur_ps.pm_time = buc->pm_time;
    /* Q2 doesn't send a groundEntityNum; infer on-ground from
     * PMF_ON_GROUND (bit 4). The AI only asks whether it is ENTITYNUM_NONE. */
    bs->cur_ps.groundEntityNum = (buc->pm_flags & 4) ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
    bs->cur_ps.persistant[Q3PERS_SCORE] = buc->stats[14]; /* Q2 STAT_FRAGS */

    /* --- Populate bs->inventory[] straight from Q2's real per-client
     * inventory + health/armor stats. See BotUpdateInventory's
     * replacement (game_q3/ai_dmq3.c) for why a direct copy is correct:
     * Q2's item indices already match assets/botfiles/inv.h's
     * INVENTORY_* slots (both MAX_ITEMS/Q2_MAX_ITEMS are 256). --- */
    Com_Memcpy(bs->inventory, buc->inventory, sizeof(bs->inventory));
    bs->inventory[28] = buc->stats[1]; /* INVENTORY_HEALTH (inv.h); Q2 STAT_HEALTH=1 */
    /* INVENTORY_ARMOR: the points of the body armor worn. Not STAT_ARMOR, which
     * shows the cells while power armor is on, alternating with the body
     * armor every 8 frames when both are (game_q2/p_hud.c G_SetStats). Q2
     * holds one kind of body armor at a time, in its inventory slot. */
    bs->inventory[INVENTORY_ARMOR] = buc->inventory[INVENTORY_ARMOR_BODY] +
                                     buc->inventory[INVENTORY_ARMOR_COMBAT] +
                                     buc->inventory[INVENTORY_ARMOR_JACKET];
    /* INVENTORY_BFGAMMO (ai_q2_compat.h: slot 202): the cells, but only once
     * there are enough for one BFG shot (50 in Q2). The Q3 rules test BFG
     * ammo in Q3's one-per-shot units (> 7, >= 10, > 0); on the raw cell
     * count a bot with 8 cells counted as able to fight with the BFG. */
    bs->inventory[202] = (buc->inventory[20] >= 50) ? buc->inventory[20] : 0; /* INVENTORY_CELLS=20 */
    Q2_TrackPowerups(bs, client, buc);

    /* The weapon in hand (Q3's cur_ps.weapon). Read by the activate-goal
     * code (ai_dmnet.c: only shoot a button while holding the weapon picked
     * for it -- with this always 0, bots never shot one) and BotAttackMove
     * (closing in with the melee weapon). */
    bs->cur_ps.weapon = Q2_WeaponForGunIndex(bs, buc->gunindex);
    q2_gunframe[client] = buc->gunframe;
    /* The weapon state (Q3's cur_ps.weaponstate), which Q2 does not send:
     * Q3's BotChooseWeapon keeps its choice while a weapon drops or raises,
     * and BotAimAtEnemy leaves out its exact aim prediction. A switch is
     * under way while the view model is not yet the one of the weapon Q2BotAI
     * last sent "use" for -- for Q2_WEAPON_SWITCH_TIME at most, so a weapon
     * the game does not give (no ammo) does not freeze the choice. Without
     * it the AI chose anew during Q2's long switches and could send "use"
     * back and forth where two weapons weigh about the same. */
    if (q2_lastweaponcmd[client] > 0 && bs->cur_ps.weapon != q2_lastweaponcmd[client] &&
        AAS_Time() >= q2_lastweapontime[client] &&
        AAS_Time() < q2_lastweapontime[client] + Q2_WEAPON_SWITCH_TIME)
        bs->cur_ps.weaponstate = Q3WEAPON_DROPPING;
    else
        bs->cur_ps.weaponstate = Q3WEAPON_READY;

    /* The bot's CTF team, for its configstring (BotTeam, ai_dmq3.c, reads the
     * "t" trap_GetConfigstring serves): the HUD shows the team's picture,
     * game_q2/g_ctf.c SetCTFStats -- stats[22] STAT_CTF_JOINED_TEAM1_PIC for
     * red, stats[23] STAT_CTF_JOINED_TEAM2_PIC for blue. */
    if (buc->stats[22])      q2_ctfteam[client] = Q3TEAM_RED;
    else if (buc->stats[23]) q2_ctfteam[client] = Q3TEAM_BLUE;
    else                     q2_ctfteam[client] = Q3TEAM_FREE;

    /* Who hurt the bot last and how, for the hit chats (ai_chat.c reads the
     * client's lasthurt_client/_mod): game_q2/bl_main.c sends them in the
     * unused stats[28]/[29]. */
    Q2Shim_ClientHurt(client, buc->stats[28], buc->stats[29]);

    /* The bot's own AAS entity is NOT updated here. The game
     * DLL already sends that edict through BotUpdateEntity earlier in the
     * same frame (game_q2/g_main.c's entity loop). A second
     * Export_BotLibUpdateEntity call for the same slot in the same frame made
     * AAS_UpdateEntity record update_time == 0 and lastvisorigin == origin,
     * so the real BotAimAtEnemy (game_q3/ai_dmq3.c) divided by zero when
     * aiming at another bot and snapped its view toward yaw 0; it also
     * replaced the edict's real angles, model, frame and solid with zeros. */
    return Q2_BLERR_NOERROR;
}

/* Whether a bot can rocket jump now (be_ai_move.c BotTravel_RocketJump):
 * the rocket launcher in hand and idle -- game_q2/p_weapon.c
 * Weapon_RocketLauncher raises it in gun frames 0-4, fires it in 5-12 and
 * idles in 13-50 -- and fire released in its last input. Q2 fires a held
 * button in the next server frame's weapon think, which runs before the
 * bots' input and so before the jump; a new press fires in the bot's own
 * ClientThink, after its first half frame of movement. */
int Q2_BotRocketJumpReady(int client)
{
    bot_state_t *bs;

    if (client < 0 || client >= MAX_CLIENTS)
        return false;
    bs = botstates[client];
    if (!bs || !bs->inuse)
        return false;
    if (bs->cur_ps.weapon != (int)LibVarGetValue("weapindex_rocketlauncher"))
        return false;
    if (q2_gunframe[client] < 13 || q2_gunframe[client] > 50)
        return false;
    return !q2_lastattack[client];
}

static int Q2BotUpdateEntity(int ent, q2_bot_updateentity_t *bue)
{
    bot_entitystate_t state;

    /* An edict numbered ENTITYNUM_MAX_NORMAL (1022) or above has no Quake
     * III entity number: the next two are the world and "none", and the
     * tables end after them (Q2BotSetupLibrary). A game with that many
     * edicts in use -- maxentities defaults to 1024, and can be raised --
     * leaves those out of what the bots see instead of overwriting the
     * world with them. Clients are numbered from 1 and never get here. */
    if (ent >= ENTITYNUM_MAX_NORMAL)
        return Q2_BLERR_NOERROR;
    if (!ValidEntityNumber(ent, "BotUpdateEntity"))
        return Q2_BLERR_INVALIDENTITYNUMBER;
    if (ent >= 1 && ent <= q2_maxclients) {
        q2_clientrenderfx[ent - 1] = bue->renderfx;
        q2_clientmodelindex3[ent - 1] = bue->modelindex3;
    }

    Com_Memset(&state, 0, sizeof(state));
    VectorCopy(bue->origin,     state.origin);
    VectorCopy(bue->angles,     state.angles);
    VectorCopy(bue->old_origin, state.old_origin);
    VectorCopy(bue->mins,       state.mins);
    VectorCopy(bue->maxs,       state.maxs);
    state.solid      = bue->solid;
    state.modelindex = bue->modelindex;
    state.modelindex2= bue->modelindex2;
    state.frame      = bue->frame;
    /* #13 — Event and event parameters */
    state.event      = bue->event;
    state.eventParm  = 0; /* Q2 doesn't have a separate eventParm */
    /* #11 — Powerup bits: Q2 signals powerups through EF_* in the effects
     * field (game_q2/q_shared.h: EF_QUAD 0x00008000, EF_FLAG1 0x00040000,
     * EF_FLAG2 0x00080000); map them to the real Q3 powerup_t numbers the
     * ported code tests (EntityHasQuad, EntityCarriesFlag,
     * BotTeamFlagCarrierVisible). This used to read 0x80/0x100 -- EF_BFG and
     * EF_COLOR_SHELL -- into Q3 bits 3/4 (PW_HASTE/PW_INVIS), which made
     * every player with a colour shell (power shield) "invisible" to
     * EntityIsInvisible, and put the flags one bit too high (blue carriers
     * went unrecognised). Q3 has no invulnerability powerup, and nothing in
     * the ported files would read one. */
    state.powerups = 0;
    if (bue->effects & 0x00008000) state.powerups |= (1 << Q3PW_QUAD);
    if (bue->effects & Q2_EF_FLAG1_CARRIER) state.powerups |= (1 << Q3PW_REDFLAG);
    if (bue->effects & Q2_EF_FLAG2_CARRIER) state.powerups |= (1 << Q3PW_BLUEFLAG);
    /* #12 — Animation state: Q2 uses frame directly, no legs/torso split.
     * Set both to the entity frame for basic animation awareness. */
    state.legsAnim  = bue->frame;
    state.torsoAnim = bue->frame;
    /* Classify entity type for AAS using the Q2 solid value:
     *
     *   ET_PLAYER  (1): client slots 1..maxclients.
     *
     *   ET_MOVER   (4): SOLID_BSP (3) + modelindex > 0.
     *                   func_plat, func_door, func_train, func_rotating, etc.
     *                   AAS_OriginOfMoverWithModelNum() queries these every
     *                   frame to track elevator/door positions at runtime.
     *
     *   ET_ITEM    (2): SOLID_TRIGGER (1) + modelindex > 0.
     *                   All Q2 item pickups (weapons, health, armor, ammo,
     *                   powerups) use SOLID_TRIGGER while present in the world
     *                   and SOLID_NOT while respawning — so the entity naturally
     *                   appears/disappears from BotUpdateEntityItems() as items
     *                   are picked up and respawn.  This enables dynamic item
     *                   tracking including dropped weapons from dead players.
     *
     *   ET_MISSILE (3): Identified by Q2 effects flags (EF_ROCKET,
     *                   EF_GRENADE, EF_BLASTER) which the engine sets on
     *                   actual projectile entities.  This mirrors Q3 where
     *                   the game DLL explicitly sets s.eType = ET_MISSILE.
     *                   Previously we used SOLID_BBOX + modelindex > 0 but
     *                   that misclassified any SOLID_BBOX entity (e.g.
     *                   func_object, debris, misc_explobox) as a missile,
     *                   creating permanent false avoid-spots that blocked
     *                   bot navigation.
     *                   Fallback: SOLID_BBOX + modelindex > 0 entities
     *                   WITHOUT missile effects are classified ET_GENERAL.
     *                   The CTF grapple hook also uses SOLID_BBOX — it is
     *                   caught by the effects check (EF_GIB on some mods)
     *                   or by the grapple weapindex match.
     *
     *   ET_GENERAL (0): everything else — trigger volumes, non-solid
     *                   decorative models, effects, SOLID_BBOX entities
     *                   that are not missiles (func_object, debris). */
    /* Q2 effects flags for projectile identification */
#define Q2_EF_BLASTER   0x00000008
#define Q2_EF_ROCKET    0x00000010
#define Q2_EF_GRENADE   0x00000020
#define Q2_MISSILE_EFFECTS (Q2_EF_BLASTER | Q2_EF_ROCKET | Q2_EF_GRENADE)
    {
        if (ent >= 1 && ent <= q2_maxclients) {
            state.type = 1; /* ET_PLAYER */
            /* #10 — Weapon state on player entities: Q2 stores the weapon
             * model in modelindex2. */
            state.weapon = bue->modelindex2;
            /* Q2 player entities carry no dead/firing flags, but the player
             * model's animation frame (game_q2/m_player.h) says both -- the
             * same test the Gladiator botlib used for EntityIsDead and
             * EntityIsShooting. A player drawn with anything but the player
             * model (modelindex 255) has been gibbed. EF_DEAD reaches
             * EntityIsDead through BotAI_GetClientState's pm_type for human
             * clients (ai_q2_shim.c; bots use their own cur_ps.pm_type);
             * EF_FIRING is what EntityIsShooting (game_q3/ai_dmq3.c) tests. */
            if (bue->modelindex != 255 ||
                (bue->frame >= 173 && bue->frame <= 197))   /* FRAME_crdeath1..FRAME_death308 */
                state.flags |= Q3EF_DEAD;
            else if ((bue->frame >= 46 && bue->frame <= 53) ||   /* FRAME_attack1..8 */
                     (bue->frame >= 160 && bue->frame <= 168))   /* FRAME_crattak1..9 */
                state.flags |= Q3EF_FIRING;
        } else if (ent == 0) {
            /* Entity 0 (worldspawn) has SOLID_BSP and a modelindex but is
             * NOT a mover: as one it masked the real func_plat/func_door
             * entities in AAS_OriginOfMoverWithModelNum lookups. It is
             * model *0, as in Q3: Q2's modelindex 1 for it is the number of
             * the first brush model in the AAS, and BotOnMover
             * (be_ai_move.c) took a bot on the floor near brush model *1 for
             * a bot on that plat (rdm3's lift). */
            state.modelindex = 0;
            state.type = 0; /* ET_GENERAL */
        } else if (bue->solid == 3 /* SOLID_BSP */ && bue->modelindex > 0) {
            state.type = 4; /* ET_MOVER */
            /* Q2 runtime modelindex is offset by 1 from BSP model number:
             * Q2 assigns modelindex 1 to the world (*0), so *1 gets
             * modelindex 2, *2 gets modelindex 3, etc.
             * AAS reachabilities store the BSP model number (1, 2, ...),
             * so we subtract 1 to match.  Without this, MoverDown()
             * searches for modelindex=1 but the func_plat has modelindex=2
             * and the lookup fails. */
            state.modelindex = bue->modelindex - 1;
        } else if (bue->solid == 1 /* SOLID_TRIGGER */ && bue->modelindex > 0) {
            state.type = 2; /* ET_ITEM */
        } else if (bue->effects & Q2_MISSILE_EFFECTS) {
            /* Q2 projectiles: the engine sets EF_ROCKET, EF_GRENADE, or
             * EF_BLASTER on actual missile entities for trail rendering.
             * This is the authoritative signal, like Q3's s.eType. */
            state.type   = 3; /* ET_MISSILE */
            state.weapon = bue->modelindex; /* proxy: game DLL sets weapindex_grapple
                                             * to grapple hook model index for CTF */
        } else {
            state.type = 0; /* ET_GENERAL */
        }
    }
    /* The same update for the Q3 AI's game state (ai_q2_shim.c): the
     * snapshots BotCheckSnapshot walks for events and grenades, and the
     * clients FindHumanTeamLeader looks at. */
    Q2Shim_UpdateEntity(ent, state.type, state.flags, state.powerups,
                        bue->origin, bue->mins, bue->maxs, bue->modelindex,
                        bue->effects, bue->event);

    /* the world goes where Q3 keeps it, unlinked (AAS_UpdateEntity) */
    return Export_BotLibUpdateEntity(Q2_EdictToEntity(ent), &state);
}

static int Q2BotAddSound(vec3_t origin, int ent, int channel,
                          int soundindex, float volume,
                          float attenuation, float timeofs)
{
    (void)origin; (void)ent; (void)channel; (void)soundindex;
    (void)volume; (void)attenuation; (void)timeofs;
    return Q2_BLERR_NOERROR;  /* not implemented in Q3 botlib */
}

static int Q2BotAddPointLight(vec3_t origin, int ent, float radius,
                               float r, float g, float b,
                               float time, float decay)
{
    (void)origin; (void)ent; (void)radius;
    (void)r; (void)g; (void)b; (void)time; (void)decay;
    return Q2_BLERR_NOERROR;  /* not implemented in Q3 botlib */
}

/* ====================================================================
 * Q2BotAI — per-bot AI think, once per server frame
 *
 * Much-shrunk from the previous hand-rolled AINode-alike state machine:
 * this now just bridges into the real Q3 AI (game_q3/ai_main.c's BotAI,
 * which itself calls BotDeathmatchAI -> the real ai_dmnet.c state
 * machine / ai_dmq3.c combat+goal logic / ai_chat.c chat triggers).
 * bs->cur_ps and bs->inventory[] are already populated for this frame by
 * Q2BotUpdateClient, which the game DLL's own frame loop (g_main.c)
 * guarantees runs immediately before this for the same client (see
 * report). No dead-bot special case is needed here: BotIsDead(bs) inside
 * the real ai_dmnet.c already routes into AIEnter_Respawn, which calls
 * trap_EA_Respawn(bs->client) -- translated below by the existing,
 * unchanged Q3ActionsToQ2 bit mapping into Q2_ACTION_RESPAWN exactly
 * like every other queued action.
 * ==================================================================== */
/* Sends "use" for one powerup of q2_powerups[] if the bot holds it and it
 * does not run yet. Returns whether it did. */
static int Q2_UsePowerup(int client, int i)
{
    q2_powerupstate_t *pu = &q2_powerupstate[client];
    float now = AAS_Time();

    if (pu->held[i] <= 0 || pu->until[i] > now)
        return false;
    if (now >= pu->used[i] && now < pu->used[i] + Q2_POWERUP_RETRY)
        return false;
    q2import.BotClientCommand(client, "use", q2_powerups[i].item, NULL);
    pu->used[i] = now;
    if (q2_powerups[i].time > 0)
        pu->until[i] = now + q2_powerups[i].time;
    return true;
}

/* Switch on the powerups a bot holds. Q2 keeps them in the inventory until
 * a "use" (unless dmflags has instant items), and the Q3 AI only ever uses
 * Q3's holdables. As Gladiator did (quad and invulnerability in its fight
 * node, BotBattleUseItems in every node), with the mission packs' added:
 * - in a fight -- the fight node, or any battle node with the enemy in
 *   sight within the last second, since a powerup still held at death is
 *   lost: quad, DualFire, Double Damage, invulnerability, the doppleganger,
 *   and a sphere -- one at a time, the game refuses a second;
 * - always: the silencer, the environment suit in slime or lava, the
 *   rebreather (else the suit) with the head under water, and power armor
 *   again once there are cells: Q2 switches it on at pickup in deathmatch
 *   and off when the cells run out. "use" toggles power armor, so only
 *   when the HUD has not shown it on for a while.
 * Never used: the A-M Bomb (thrown at 100 u/s, it takes the thrower with
 * it), IR goggles and the compass (nothing in them for a bot). */
static void Q2BotUsePowerups(bot_state_t *bs, int client)
{
    q2_powerupstate_t *pu = &q2_powerupstate[client];
    float now = AAS_Time();
    vec3_t feet;

    if (bs->inventory[INVENTORY_HEALTH] <= 0)
        return;

    if (bs->enemy >= 0 &&
        (bs->ainode == AINode_Battle_Fight || bs->enemyvisible_time > now - 1.0f)) {
        Q2_UsePowerup(client, Q2PU_QUAD);
        Q2_UsePowerup(client, Q2PU_DUALFIRE);
        Q2_UsePowerup(client, Q2PU_DOUBLE);
        Q2_UsePowerup(client, Q2PU_INVULNERABILITY);
        Q2_UsePowerup(client, Q2PU_DOPPLEGANGER);
        if (pu->until[Q2PU_DEFENDER] <= now && pu->until[Q2PU_HUNTER] <= now &&
            pu->until[Q2PU_VENGEANCE] <= now) {
            if (!Q2_UsePowerup(client, Q2PU_DEFENDER) &&
                !Q2_UsePowerup(client, Q2PU_HUNTER))
                Q2_UsePowerup(client, Q2PU_VENGEANCE);
        }
    }

    Q2_UsePowerup(client, Q2PU_SILENCER);

    VectorCopy(bs->origin, feet);
    feet[2] -= 23;                          /* Q2 player mins[2] is -24 */
    if (AAS_PointContents(feet) & (CONTENTS_SLIME|CONTENTS_LAVA))
        Q2_UsePowerup(client, Q2PU_ENVIRO);
    else if ((AAS_PointContents(bs->eye) & (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA)) &&
             pu->until[Q2PU_REBREATHER] <= now && pu->until[Q2PU_ENVIRO] <= now) {
        if (!Q2_UsePowerup(client, Q2PU_REBREATHER))
            Q2_UsePowerup(client, Q2PU_ENVIRO);
    }

    if (pu->powerarmor_since && now - pu->powerarmor_since > Q2_POWERARMOR_WAIT &&
        now - pu->powerarmor_seen > Q2_POWERARMOR_WAIT &&
        (now - pu->powerarmor_used > Q2_POWERARMOR_WAIT || now < pu->powerarmor_used) &&
        bs->inventory[INVENTORY_CELLS] > 0) {
        q2import.BotClientCommand(client, "use",
            bs->inventory[INVENTORY_POWERSHIELD] > 0 ? "Power Shield" : "Power Screen", NULL);
        pu->powerarmor_used = now;
    }
}

static int Q2BotAI(int client, float thinktime)
{
    bot_state_t   *bs;
    bot_input_t    q3input;
    q2_bot_input_t q2input;

    if (client < 0 || client >= MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    bs = botstates[client];
    if (!bs || !bs->inuse) return Q2_BLERR_AICLIENTNOTSETUP;

    /* The CTF flag status. Q3 learns it from the flag messages
     * (BotMatch_CTF, ai_cmd.c, which also names the carrier) and from team
     * sound events Q2 does not have, and a bot that joins while a flag is
     * away never saw that flag's message. The flags' entities
     * (Q2AI_UpdateCTFFlagStatus, called from Q2BotStartFrame) settle it
     * whenever they change -- raising flagstatuschanged like Q3's events, so
     * BotTeamAI plans the team anew -- and leave the messages alone in
     * between. */
    {
        int status = q2_redflagstatus | (q2_blueflagstatus << 1);

        if (status != q2_appliedflagstatus[client]) {
            bs->redflagstatus  = q2_redflagstatus;
            bs->blueflagstatus = q2_blueflagstatus;
            bs->flagstatuschanged = true;
            q2_appliedflagstatus[client] = status;
        }
    }

    /* The real Q3 AI: state machine, combat, weapon choice, goal
     * selection, chat -- see game_q3/ai_main.c/ai_dmnet.c/ai_dmq3.c/
     * ai_team.c/ai_chat.c. */
    BotAI(client, thinktime);

    /* Held powerups: Q2 needs a "use" for them, see the function. */
    Q2BotUsePowerups(bs, client);

    /* Roaming view, adapted to Q2's player models. With no enemy, the Q3 AI
     * aims the view at a point ~300 units ahead along the route
     * (BotMovementViewTarget) or at random spots (BotRoamGoal), which puts it
     * 30+ degrees off the walking direction in ~40% of roaming frames and
     * behind the bot in ~7%. Q3 hides that with separately animated legs
     * that always face the movement; a Q2 model is one piece that turns with
     * the view, so the bot slid sideways or backwards along its route. Keep
     * the look-ahead (the view still leads into turns) but only within
     * Q2_ROAM_VIEW_MAXDEVIATION of the direction the bot is walking this
     * frame. Only yaw is touched, so ladder and swimming views (which need
     * pitch) are unaffected; a bot that is not walking keeps looking
     * around freely. */
    if (bs->enemy < 0) {
        bot_input_t walk;

        EA_GetInput(client, thinktime, &walk);
        if (walk.speed > 0 && (walk.dir[0] != 0 || walk.dir[1] != 0)) {
            float moveyaw = atan2(walk.dir[1], walk.dir[0]) * 180 / M_PI;
            float diff    = bs->ideal_viewangles[YAW] - moveyaw;

            while (diff > 180)  diff -= 360;
            while (diff < -180) diff += 360;
            if (diff > Q2_ROAM_VIEW_MAXDEVIATION)
                bs->ideal_viewangles[YAW] = AngleMod(moveyaw + Q2_ROAM_VIEW_MAXDEVIATION);
            else if (diff < -Q2_ROAM_VIEW_MAXDEVIATION)
                bs->ideal_viewangles[YAW] = AngleMod(moveyaw - Q2_ROAM_VIEW_MAXDEVIATION);
        }
    }

    /* Real Q3 calls this as a genuinely separate step after BotAI() (see
     * ai_main.c's own BotUpdateInput and its call site in real Q3's bot
     * scheduling code) -- it's what actually turns bs->ideal_viewangles
     * (set moments ago inside BotAI()'s call to BotDeathmatchAI, e.g. via
     * BotAimAtEnemy or movement-facing logic) into a smoothed
     * bs->viewangles and submits it via trap_EA_View. Restored here after
     * being found dead code (unreferenced anywhere) -- BotUpdateInput had
     * been deleted during the original port as a duplicate of this same
     * function's own EA-input-collection tail below, which is true, but
     * that assessment missed that BotUpdateInput also carried this call,
     * which nothing else replaced. Without it bs->viewangles never
     * changes: bots navigate and occasionally attack whatever already
     * happens to be in their frozen forward cone, but never actually turn
     * to track a target or face their own movement.
     *
     * It runs Q2_VIEW_SUBSTEPS times per frame on a proportional slice of
     * thinktime. BotChangeViewAngles is not frame-rate independent: each
     * call closes a fixed fraction of the angle error (view factor) with
     * momentum, and only the per-second cap (view maxchange) scales with
     * time. Real Q3 calls BotUpdateInput every server frame (20 Hz) on a
     * dedicated server and every client frame on a listen server
     * (code/server/sv_main.c: SV_BotFrame), while BotAI thinks at 10 Hz --
     * called once per 10 Hz Q2 frame, a 90 degree roaming turn took ~3.3 s
     * to settle instead of ~0.35 s, and bots slid along their route facing
     * sideways or backwards. Only the result of the last step is sent,
     * since Q2 takes one view per bot per frame. */
    {
        int step;
        for (step = 0; step < Q2_VIEW_SUBSTEPS; step++)
            BotUpdateInput(bs, thinktime / Q2_VIEW_SUBSTEPS);
    }

    /* --- Collect EA input and translate to Q2 (unchanged in spirit
     * from the previous adapter's tail end -- this part is orthogonal to
     * where the AI decision-making comes from). BotUpdateInput above
     * already resolved the final view angles via trap_EA_View, so
     * q3input.viewangles below is already the fully-resolved answer; no
     * separate priority reconstruction is needed on this side of the
     * bridge. --- */
    Com_Memset(&q3input, 0, sizeof(q3input));
    EA_GetInput(client, thinktime, &q3input);
    q2_lastattack[client] = (q3input.actionflags & 0x0000001) != 0;  /* ACTION_ATTACK */

    Com_Memset(&q2input, 0, sizeof(q2input));
    q2input.thinktime   = q3input.thinktime;
    VectorCopy(q3input.dir, q2input.dir);
    q2input.speed       = q3input.speed;
    q2input.actionflags = Q3ActionsToQ2(q3input.actionflags);
    VectorCopy(q3input.viewangles, q2input.viewangles);

    /* Translate EA_SelectWeapon to Q2's "use <name>" client command.
     * The real BotAI() (ai_main.c) calls trap_EA_SelectWeapon(bs->client,
     * bs->weaponnum) every frame; EA_GetInput() reports it back as
     * q3input.weapon. Q3 puts it into every usercmd; Q2 has no such field,
     * so "use" goes out whenever the choice changes.
     *
     * The game also switches weapons by itself: from the Blaster to a weapon
     * the bot picks up, and away from one that ran dry. A gun in hand the AI
     * did not choose is left alone -- the AI chooses again in every fight --
     * but a thrown weapon is not: the AI fires the weapon it chose, holding
     * fire, and a hand grenade, Trap or Tesla held that long goes off in the
     * bot's hand. So with one of those (weapons.c flags WFL_FIRERELEASED) in
     * hand the AI chooses again, as it does when it reaches an item, the
     * fire button is let go, and "use" is repeated every Q2_WEAPON_RESEND
     * seconds until the switch is done. A repeat is harmless: Use_Weapon()
     * only sets the weapon to switch to, and ignores the one already in
     * use. */
    {
        int resend = 0;

        if (bs->cur_ps.weapon > 0 && bs->cur_ps.weapon != q3input.weapon) {
            weaponinfo_t held;

            BotGetWeaponInfo(bs->ws, bs->cur_ps.weapon, &held);
            if (held.flags & WFL_FIRERELEASED) {
                BotChooseWeapon(bs);
                q3input.weapon = bs->weaponnum;
                if (q3input.weapon != bs->cur_ps.weapon) {
                    q2input.actionflags &= ~Q2_ACTION_ATTACK;
                    resend = AAS_Time() >= q2_lastweapontime[client] + Q2_WEAPON_RESEND ||
                             AAS_Time() < q2_lastweapontime[client];
                }
            }
        }
        if (q3input.weapon > 0 &&
            (q3input.weapon != q2_lastweaponcmd[client] || resend)) {
            weaponinfo_t wi;
            BotGetWeaponInfo(bs->ws, q3input.weapon, &wi);
            if (wi.name[0]) {
                q2import.BotClientCommand(client, "use", wi.name, NULL);
                q2_lastweaponcmd[client] = q3input.weapon;
                q2_lastweapontime[client] = AAS_Time();
            }
        }
    }

    /* No EA_ResetInput here: the real BotAI() resets the EA input as its
     * first step. Resetting a second time wiped the ACTION_JUMPEDLASTFRAME
     * marker the first reset sets after a jump, so EA_Jump could hold jump
     * across consecutive frames and Q2's PMF_JUMP_HELD blocked the next
     * jump. */
    q2import.BotInput(client, &q2input);

    return Q2_BLERR_NOERROR;
}

/* Q3's game marks a chatting player's name with EC characters
 * (g_cmds.c G_Say): "name" EC ": text", to the team EC "(name" EC ")" EC
 * ": text", told EC "[name" EC "]" EC ": text". The bot AI's chat templates
 * (match.c: replies, team orders, leadership) match exactly that. Q2 prints
 * chat as "name: text\n" and "(name): text\n" (game_q2/g_cmds.c Cmd_Say_f);
 * with those no chat ever matched, so bots neither replied nor took orders.
 * The sender is the longest known name the line starts with, since a name
 * can prefix another. A line from nobody known stays as it is. */

static void Q2ChatToQ3(const char *in, char *out, int size)
{
    const char *name = NULL;
    int team = (in[0] == '(');
    int i, len, best = 0;

    for (i = 0; i < MAX_CLIENTS; i++) {
        len = (int)strlen(q2_netnames[i]);
        if (!len || len <= best || strncmp(in + team, q2_netnames[i], len))
            continue;
        if (team ? strncmp(in + 1 + len, "): ", 3) : strncmp(in + len, ": ", 2))
            continue;
        name = q2_netnames[i];
        best = len;
    }
    if (!name)
        Q_strncpyz(out, in, size);
    else if (team)
        Com_sprintf(out, size, Q2_EC "(%s" Q2_EC ")" Q2_EC ": %s", name, in + 1 + best + 3);
    else
        Com_sprintf(out, size, "%s" Q2_EC ": %s", name, in + best + 2);
    len = (int)strlen(out);
    if (len > 0 && out[len - 1] == '\n')
        out[len - 1] = '\0';
}

/* The client of a name as the game prints it, -1 for none. */
static int Q2ClientForName(const char *name)
{
    int i;

    if (!name[0])
        return -1;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (!strcmp(q2_netnames[i], name))
            return i;
    }
    return -1;
}

/* The obituaries. The Gladiator game tells the bot library of no death: it
 * prints each one to every client (game_q2/p_client.c ClientObituary) --
 * "victim message." for a death by the world or by one's own hand, "victim
 * message killer message2" for a kill, "victim died." otherwise -- and
 * Gladiator's botlib read them back from its bots' consoles (BotMatchMessage,
 * MSG_DEATH). The same kind of templates (MTCONTEXT_CLIENTOBITUARY, the
 * botfiles' match.c) give the victim, the killer and, as sub type, the means
 * of death, of which Q2Shim_Obituary makes the EV_OBITUARY of Q3's G_Obituary
 * that every bot's BotCheckEvents books. The game hands a print to its bots
 * one after the other (game_q2/bl_redirgi.c Bot_bprintf), so a line the
 * last one already had this frame is the same death. The match type, not
 * the KILLER variable, tells a kill: an unset variable's offset is a char,
 * which is unsigned on some compilers. */
static void Q2CheckObituary(char *message)
{
    static char lastline[MAX_MESSAGE_SIZE];
    static float lasttime = -1;
    bot_match_t match;
    char name[MAX_MESSAGE_SIZE];
    int victim, killer;

    if (strlen(message) >= sizeof(lastline))
        return;
    if (lasttime == AAS_Time() && !strcmp(lastline, message))
        return;
    Q_strncpyz(lastline, message, sizeof(lastline));
    lasttime = AAS_Time();

    if (!BotFindMatch(message, &match, MTCONTEXT_CLIENTOBITUARY))
        return;
    BotMatchVariable(&match, VICTIM, name, sizeof(name));
    victim = Q2ClientForName(name);
    if (victim < 0)
        return;
    switch (match.type) {
    case MSG_DEATH:
        BotMatchVariable(&match, KILLER, name, sizeof(name));
        killer = Q2ClientForName(name);
        if (killer < 0)
            return;
        break;
    case MSG_SELFDEATH:
        killer = victim;
        break;
    case MSG_WORLDDEATH:
        killer = ENTITYNUM_WORLD;
        break;
    default:
        return;
    }
    Q2Shim_Obituary(victim, killer, match.subtype);
}

static int Q2BotConsoleMessage(int client, int type, char *message)
{
    bot_state_t *bs;
    char q3chat[MAX_MESSAGE_SIZE];

    if (client < 0 || client >= MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    if (type == CMS_NORMAL)
        Q2CheckObituary(message);
    bs = botstates[client];
    if (!bs || !bs->inuse) return Q2_BLERR_AICMFORINACTIVECLIENT;

    if (type == CMS_CHAT) {
        Q2ChatToQ3(message, q3chat, sizeof(q3chat));
        message = q3chat;
    }
    BotQueueConsoleMessage(bs->cs, type, message);
    return Q2_BLERR_NOERROR;
}

static int Q2BotTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3)
{
    return BotExportTest(parm0, parm1, parm2, parm3);
}

static void Q2AAS_ShowArea(int areanum)
{
    AAS_ClearShownDebugLines();
    AAS_ShowArea(areanum, true);
}

/* Show ALL reachabilities from an area at once.
 * Q3's AAS_ShowReachableAreas is designed for per-frame cycling (shows
 * one at a time every 1.5s).  This version draws them all for a one-shot
 * console command. */
static void Q2AAS_ShowAllReachabilities(int areanum)
{
    extern aas_t aasworld;
    aas_areasettings_t *settings;
    int i;

    if (areanum <= 0 || areanum >= aasworld.numareas) return;

    AAS_ClearShownDebugLines();
    AAS_ShowArea(areanum, true);

    settings = &aasworld.areasettings[areanum];
    for (i = 0; i < settings->numreachableareas; i++) {
        aas_reachability_t *reach = &aasworld.reachability[settings->firstreachablearea + i];
        AAS_ShowReachability(reach);
    }

    botimport.Print(PRT_MESSAGE, "area %d: %d reachabilities\n",
                    areanum, settings->numreachableareas);
}

/* Return the center point of an AAS area for teleport/debug. */
static qboolean Q2AAS_AreaCenter(int areanum, vec3_t center)
{
    extern aas_t aasworld;
    if (areanum <= 0 || areanum >= aasworld.numareas) return false;
    VectorCopy(aasworld.areas[areanum].center, center);
    return true;
}

/* ====================================================================
 * Chat-related export functions (game DLL -> botlib)
 * ==================================================================== */
/* This port's game used to report deaths through these two entries. Deaths
 * come from the obituary prints now, as they did for Gladiator's botlib
 * (Q2CheckObituary); a game that still calls them must not count a death
 * twice. The entries stay: the export table is fixed. */
static void Q2BotNotifyDeath(int client, int killer, int mod)
{
    (void)client; (void)killer; (void)mod;
}

static void Q2BotNotifyKill(int client, int victim, int mod)
{
    (void)client; (void)victim; (void)mod;
}

static int Q2BotGetChatState(int client)
{
    if (client < 0 || client >= MAX_CLIENTS || !botstates[client]) return 0;
    return botstates[client]->cs;
}

static int Q2BotGetCharacter(int client)
{
    if (client < 0 || client >= MAX_CLIENTS || !botstates[client]) return 0;
    return botstates[client]->character;
}

static int Q2BotGetEnemy(int client)
{
    if (client < 0 || client >= MAX_CLIENTS || !botstates[client]) return -1;
    return botstates[client]->enemy;
}

static float Q2BotGetLastChatTime(int client)
{
    if (client < 0 || client >= MAX_CLIENTS || !botstates[client]) return 0;
    return botstates[client]->lastchat_time;
}

static void Q2BotSetLastChatTime(int client, float time)
{
    if (client < 0 || client >= MAX_CLIENTS || !botstates[client]) return;
    botstates[client]->lastchat_time = time;
}

static int Q2BotNextConsoleMessage(int chatstate, bot_consolemessage_t *cm)
{
    return BotNextConsoleMessage(chatstate, cm);
}

static void Q2BotRemoveConsoleMessage(int chatstate, int handle)
{
    BotRemoveConsoleMessage(chatstate, handle);
}

static int Q2BotReplyChat(int chatstate, char *message, int mcontext, int vcontext,
    char *var0, char *var1, char *var2, char *var3,
    char *var4, char *var5, char *var6, char *var7)
{
    return BotReplyChat(chatstate, message, mcontext, vcontext,
        var0, var1, var2, var3, var4, var5, var6, var7);
}

/* ====================================================================
 * GetBotAPI  —  Q2 entry point exported from botlib.so
 *
 * The Gladiator game DLL loads botlib.so and calls this function via
 * dlsym.  We:
 *   1. Store the Q2 import struct.
 *   2. Build a Q3 botlib_import_t from the Q2 callbacks.
 *   3. Call GetBotLibAPI to initialise the Q3 botlib and set botimport.
 *   4. Fill out and return the Q2 bot_export_t.
 * ==================================================================== */
/* A plain C function on every target, 32-bit Windows included -- see
 * be_interface_q2.h. This used to be __stdcall there, to match the WINAPI
 * pointer Gladiator's 1999 game source declares. That pointer was never
 * what the real gladiator.dll answered to (its GetBotAPI ends in a plain
 * `ret`), the 1999 game got away with the mismatch, and a game that calls
 * the function the way Gladiator's library is called -- __cdecl -- was off
 * by four bytes of stack on return from this one. */
Q2_BOTLIB_EXPORT q2_bot_export_t *GetBotAPI(q2_bot_import_t *import)
{
    botlib_import_t q3imp;

    if (!import) return NULL;

    q2import = *import;

    Com_Memset(fs_files,  0, sizeof(fs_files));
    q2_bsp_entitystring[0] = '\0';

    /* Build Q3 import from Q2 import */
    Com_Memset(&q3imp, 0, sizeof(q3imp));
    q3imp.Print                 = import->Print;
    q3imp.Trace                 = Q3Trace_Adapter;
    q3imp.EntityTrace           = Q3EntityTrace_Adapter;
    q3imp.PointContents         = Q3PointContents_Adapter;
    q3imp.inPVS                 = Q3inPVS_Adapter;
    q3imp.BSPEntityData         = Q3BSPEntityData_Callback;
    q3imp.BSPModelMinsMaxsOrigin = Q3BSPModelMinsMaxsOrigin;
    q3imp.BotClientCommand      = Q3BotClientCommand_Adapter;
    q3imp.GetMemory             = import->GetMemory;
    q3imp.FreeMemory            = import->FreeMemory;
    q3imp.AvailableMemory       = Q3AvailableMemory_Stub;
    q3imp.HunkAlloc             = Q3HunkAlloc_Adapter;
    q3imp.FS_FOpenFile          = Q3_FS_FOpenFile;
    q3imp.FS_Read               = Q3_FS_Read;
    q3imp.FS_Write              = Q3_FS_Write;
    q3imp.FS_FCloseFile         = Q3_FS_FCloseFile;
    q3imp.FS_Seek               = Q3_FS_Seek;
    q3imp.DebugLineCreate       = import->DebugLineCreate;
    q3imp.DebugLineDelete       = import->DebugLineDelete;
    q3imp.DebugLineShow         = import->DebugLineShow;
    q3imp.DebugPolygonCreate    = Q3DebugPolygonCreate_Stub;
    q3imp.DebugPolygonDelete    = Q3DebugPolygonDelete_Stub;

    /* Initialise Q3 botlib and set the global botimport */
    GetBotLibAPI(BOTLIB_API_VERSION, &q3imp);

    /* ---- Phase 4: Q2-correct physics defaults ----
     *
     * Q3 botlib reads these LibVars in be_aas_move.c::AAS_InitSettings().
     * The game DLL will later override gravity/friction via BotLibVarSet
     * (which routes through Q2LibVarToQ3), but we must seed the values that
     * have no Q2 sv_* counterpart before BotSetupLibrary is called.
     *
     * Q2 reference values (server defaults):
     *   sv_maxvelocity   300   (Q3 default 320)
     *   STEPSIZE         18    (Q3 default 19)
     *   crouch maxspeed  150   (Q3 default 100)
     *   water gravity    100   (Q3 default 400)
     *   barrier jump     ~50   (Q3 default 33)
     *   sv_jumpvelocity  270   (Q3 default 270 — no change needed)
     *   sv_gravity       800   (Q3 default 800 — no change needed)
     *   sv_friction      6     (Q3 default  6  — no change needed)
     */
    /* #2 — Q2 physics values (verified from yquake2/src/common/pmove.c):
     *   pm_maxspeed      = 300   (Q3: 320)
     *   pm_duckspeed     = 100   (Q3: 100 — same)
     *   pm_waterspeed    = 400   (Q3: 150 — Q2 swims much faster)
     *   pm_accelerate    = 10    (Q3: 10 — same)
     *   pm_airaccelerate = 0     (Q3: 1 — but with 0 Q2's PM_AirMove still
     *                             accelerates with 1 in the air, as Q3 does;
     *                             phys_airaccelerate 0 makes the movement
     *                             prediction assume no air control)
     *   pm_wateraccelerate = 10  (Q3: 4 — Q2 accelerates faster in water)
     *   STEPSIZE          = 18   (Q3: 19)
     *   sv_gravity        = 800  (Q3: 800 — same)
     *   sv_friction       = 6    (Q3: 6 — same)
     *   jump velocity     = 270  (Q3: 270 — same)
     *   water gravity    ~= 100  (Q3: 400 — Q2 much lower) */
    LibVarSet("phys_maxvelocity",       "300");
    LibVarSet("phys_maxwalkvelocity",   "300");
    LibVarSet("phys_maxcrouchvelocity", "100");
    LibVarSet("phys_maxswimvelocity",   "400");
    LibVarSet("phys_maxstep",           "18");
    LibVarSet("phys_maxbarrier",        "50");
    LibVarSet("phys_watergravity",      "100");
    LibVarSet("phys_airaccelerate",     "0");
    LibVarSet("phys_swimaccelerate",    "10");
    LibVarSet("phys_wateraccelerate",   "10");

    /* CTF grappling hook: GrappleState() matches ET_MISSILE entities whose
     * state.weapon == weapindex_grapple.  In our Q2 adapter, state.weapon is
     * set to the entity's modelindex (see Q2BotUpdateEntity).  The game DLL
     * must override this libvar with the actual grapple hook model index:
     *   BotLibVarSet("weapindex_grapple", "<gi.modelindex of hook model>")
     * Until then, 0 keeps grapple detection safely disabled (no Q2 entity
     * will ever have modelindex 0). */
    LibVarSet("weapindex_grapple", "0");

    /* Weapon indices for Q2: the botlib's movement code uses these to
     * select weapons for rocket jumping and BFG jumping.  Q3 defaults
     * (RL=5, BFG=9) don't match Q2's weapons.c numbering. */
    LibVarSet("weapindex_rocketlauncher", "8");  /* Q2 RL = weapon 8 */
    LibVarSet("weapindex_bfg10k", "11");         /* Q2 BFG = weapon 11 */

    /* #14 — Game type: Q3 uses g_gametype to tell botlib what mode is active.
     * 0=FFA, 3=CTF.  Default to FFA; game DLL can override via BotLibVarSet. */
    LibVarSet("g_gametype", "0");

    /* #15 — Map checksum: Q3 uses this for AAS file validation.
     * We don't have the engine's checksum, but setting it to 0 tells
     * botlib to skip the check (it only validates if non-zero). */
    LibVarSet("sv_mapChecksum", "0");

    /* #17 — Routing cache config: tune for Q2 map sizes */
    LibVarSet("max_routingcache", "8192"); /* 8MB (doubled from Q3 default 4MB) */
    LibVarSet("saveroutingcache", "0");    /* don't save to disk by default */

    /* Debug logging: set to 1 via BotLibVarSet("bot_developer","1") from
     * the game DLL or console to enable verbose bot AI messages. */
    LibVarSet("bot_developer", "0");

    /* Fill Q2 export struct */
    Com_Memset(&q2_export, 0, sizeof(q2_export));
    q2_export.BotVersion          = Q2BotVersion;
    q2_export.BotSetupLibrary     = Q2BotSetupLibrary;
    q2_export.BotShutdownLibrary  = Q2BotShutdownLibrary;
    q2_export.BotLibraryInitialized = Q2BotLibraryInitialized;
    q2_export.BotLibVarSet        = Q2BotLibVarSet;
    q2_export.BotDefine           = Q2BotDefine;
    q2_export.BotLoadMap          = Q2BotLoadMap;
    q2_export.BotSetupClient      = Q2BotSetupClient;
    q2_export.BotShutdownClient   = Q2BotShutdownClient;
    q2_export.BotMoveClient       = Q2BotMoveClient;
    q2_export.BotClientSettings   = Q2BotClientSettings;
    q2_export.BotSettings         = Q2BotSettings;
    q2_export.BotStartFrame       = Q2BotStartFrame;
    q2_export.BotUpdateClient     = Q2BotUpdateClient;
    q2_export.BotUpdateEntity     = Q2BotUpdateEntity;
    q2_export.BotAddSound         = Q2BotAddSound;
    q2_export.BotAddPointLight    = Q2BotAddPointLight;
    q2_export.BotAI               = Q2BotAI;
    q2_export.BotConsoleMessage   = Q2BotConsoleMessage;
    q2_export.Test                = Q2BotTest;
    q2_export.AAS_ShowAreaFunc              = Q2AAS_ShowArea;
    q2_export.AAS_ShowReachableAreasFunc    = Q2AAS_ShowAllReachabilities;
    q2_export.AAS_ClearShownDebugLinesFunc  = AAS_ClearShownDebugLines;
    q2_export.AAS_PointAreaNumFunc          = AAS_PointAreaNum;
    q2_export.AAS_AreaCenterFunc            = Q2AAS_AreaCenter;
    q2_export.BotInitialChatFunc   = BotInitialChat;
    q2_export.BotEnterChatFunc     = BotEnterChat;
    q2_export.BotNumInitialChatsFunc = BotNumInitialChats;
    q2_export.BotChatLengthFunc    = BotChatLength;
    q2_export.BotCharacterBFloat   = Characteristic_BFloat;
    q2_export.BotCharacterBInteger = Characteristic_BInteger;
    q2_export.BotNotifyDeath       = Q2BotNotifyDeath;
    q2_export.BotNotifyKill        = Q2BotNotifyKill;
    q2_export.BotGetChatState      = Q2BotGetChatState;
    q2_export.BotGetCharacter      = Q2BotGetCharacter;
    q2_export.BotGetEnemy          = Q2BotGetEnemy;
    q2_export.BotGetLastChatTime          = Q2BotGetLastChatTime;
    q2_export.BotSetLastChatTime          = Q2BotSetLastChatTime;
    q2_export.BotNextConsoleMessageFunc   = Q2BotNextConsoleMessage;
    q2_export.BotRemoveConsoleMessageFunc = Q2BotRemoveConsoleMessage;
    q2_export.BotReplyChatFunc            = Q2BotReplyChat;

    return &q2_export;
}
