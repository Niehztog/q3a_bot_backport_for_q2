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

extern int  BotAISetupClient(int client, struct bot_settings_s *settings, qboolean restart);
extern int  BotAIShutdownClient(int client, qboolean restart);
extern int  BotAI(int client, float thinktime);
extern void BotUpdateInput(bot_state_t *bs, float thinktime);
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
#define Q3PMF_DUCKED            1
#define Q3PMF_TIME_KNOCKBACK    64
#define Q3PMF_TIME_WATERJUMP    256
#define Q3PERS_SCORE            0

/* Real Q3 team_t numbering (game/bg_public.h in real Q3; this project's
 * own copy lives in botlib/ai_q2_compat.h, scoped to the 5 ported ai_*.c
 * files and deliberately not included wholesale here -- see the
 * bot_settings_t comment above). Needed so Q2BotUpdateClient below can
 * write bs->q2_realctfteam (ai_main.h, Phase 2) using the exact same
 * numbering game_q3/ai_dmq3.c's BotTeam()/TEAM_RED/TEAM_BLUE/TEAM_FREE
 * checks already expect. */
#define Q3TEAM_FREE   0
#define Q3TEAM_RED    1
#define Q3TEAM_BLUE   2

/* ====================================================================
 * Q2 bot API types (from gladq2_src/botlib.h)
 *
 * Defined here to avoid pulling in game_q2/q_shared.h, which would
 * conflict with game_q3/q_shared.h.  The structs are laid out to be
 * binary-compatible with gladq2_src/botlib.h.
 * ==================================================================== */

/* Error codes */
#define Q2_BLERR_NOERROR                 0
#define Q2_BLERR_LIBRARYNOTSETUP         1
#define Q2_BLERR_LIBRARYALREADYSETUP     2
#define Q2_BLERR_INVALIDCLIENTNUMBER     3
#define Q2_BLERR_INVALIDENTITYNUMBER     4
#define Q2_BLERR_AICLIENTNOTSETUP        19
#define Q2_BLERR_AICLIENTALREADYSETUP    20
#define Q2_BLERR_AIMOVEINACTIVECLIENT    21
#define Q2_BLERR_AIMOVETOACTIVECLIENT    22
#define Q2_BLERR_AICLIENTALREADYSHUTDOWN 23
#define Q2_BLERR_AIUPDATEINACTIVECLIENT  24
#define Q2_BLERR_AICMFORINACTIVECLIENT   25
#define Q2_BLERR_SETTINGSINACTIVECLIENT  26

#define Q2_MAX_NETNAME        16
#define Q2_MAX_CLIENTSKINNAME 128
#define Q2_MAX_FILEPATH       144
#define Q2_MAX_CHARACTERNAME  144

/* Q2 action flag bit positions (differ from Q3) */
#define Q2_ACTION_ATTACK      1
#define Q2_ACTION_USE         2
#define Q2_ACTION_RESPAWN     4
#define Q2_ACTION_JUMP        8     /* same bit as MOVEUP */
#define Q2_ACTION_MOVEUP      8
#define Q2_ACTION_CROUCH      16    /* same bit as MOVEDOWN */
#define Q2_ACTION_MOVEDOWN    16
#define Q2_ACTION_MOVEFORWARD 32
#define Q2_ACTION_MOVEBACK    64
#define Q2_ACTION_MOVELEFT    128
#define Q2_ACTION_MOVERIGHT   256
#define Q2_ACTION_DELAYEDJUMP 512

#define Q2_MAX_STATS   32
#define Q2_MAX_ITEMS   256

/* Q2 pmtype (must match game_q2/q_shared.h enum order) */
typedef enum {
    Q2PM_NORMAL,
    Q2PM_SPECTATOR,
    Q2PM_DEAD,
    Q2PM_GIB,
    Q2PM_FREEZE
} q2_pmtype_t;

typedef struct q2_bot_settings_s {
    char characterfile[Q2_MAX_FILEPATH];
    char charactername[Q2_MAX_CHARACTERNAME];
    char ailibrary[Q2_MAX_FILEPATH];
} q2_bot_settings_t;

typedef struct q2_bot_clientsettings_s {
    char netname[Q2_MAX_NETNAME];
    char skin[Q2_MAX_CLIENTSKINNAME];
} q2_bot_clientsettings_t;

typedef struct q2_bot_input_s {
    float   thinktime;
    vec3_t  dir;
    float   speed;
    vec3_t  viewangles;
    int     actionflags;
} q2_bot_input_t;

typedef struct q2_bot_updateclient_s {
    q2_pmtype_t pm_type;
    vec3_t  origin;
    vec3_t  velocity;
    byte    pm_flags;
    byte    pm_time;
    float   gravity;
    vec3_t  delta_angles;
    vec3_t  viewangles;
    vec3_t  viewoffset;
    vec3_t  kick_angles;
    vec3_t  gunangles;
    vec3_t  gunoffset;
    int     gunindex;
    int     gunframe;
    float   blend[4];
    float   fov;
    int     rdflags;
    short   stats[Q2_MAX_STATS];
    int     inventory[Q2_MAX_ITEMS];
} q2_bot_updateclient_t;

typedef struct q2_bot_updateentity_s {
    vec3_t  origin;
    vec3_t  angles;
    vec3_t  old_origin;
    vec3_t  mins;
    vec3_t  maxs;
    int     solid;
    int     modelindex;
    int     modelindex2, modelindex3, modelindex4;
    int     frame;
    int     skinnum;
    int     effects;
    int     renderfx;
    int     sound;
    int     event;
} q2_bot_updateentity_t;

typedef struct q2_bot_import_s {
    void        (*BotInput)(int client, q2_bot_input_t *bi);
    void        (*BotClientCommand)(int client, char *str, ...);
    void        (*Print)(int type, char *fmt, ...);
    bsp_trace_t (*Trace)(vec3_t start, vec3_t mins, vec3_t maxs,
                          vec3_t end, int passent, int contentmask);
    int         (*PointContents)(vec3_t point);
    void       *(*GetMemory)(int size);
    void        (*FreeMemory)(void *ptr);
    int         (*DebugLineCreate)(void);
    void        (*DebugLineDelete)(int line);
    void        (*DebugLineShow)(int line, vec3_t start, vec3_t end, int color);
    /* Added for Q3 botlib PVS support — delegates to engine's PF_inPVS.
     * NULL if not provided (adapter falls back to BSP-based PVS check). */
    int         (*inPVS)(vec3_t p1, vec3_t p2);
    /* CTF/teamplay: check if two entities are on the same team.
     * Delegates to game DLL's OnSameTeam() which handles CTF teams,
     * skin-based teams, and model-based teams. */
    int         (*OnSameTeam)(int ent1, int ent2);
} q2_bot_import_t;

typedef struct q2_bot_export_s {
    char *(*BotVersion)(void);
    int  (*BotSetupLibrary)(void);
    int  (*BotShutdownLibrary)(void);
    int  (*BotLibraryInitialized)(void);
    int  (*BotLibVarSet)(char *var_name, char *value);
    int  (*BotDefine)(char *string);
    int  (*BotLoadMap)(char *mapname, int modelindexes, char *modelindex[],
                        int soundindexes, char *soundindex[],
                        int imageindexes, char *imageindex[]);
    int  (*BotSetupClient)(int client, q2_bot_settings_t *settings);
    int  (*BotShutdownClient)(int client);
    int  (*BotMoveClient)(int oldclnum, int newclnum);
    int  (*BotClientSettings)(int client, q2_bot_clientsettings_t *settings);
    int  (*BotSettings)(int client, q2_bot_settings_t *settings);
    int  (*BotStartFrame)(float time);
    int  (*BotUpdateClient)(int client, q2_bot_updateclient_t *buc);
    int  (*BotUpdateEntity)(int ent, q2_bot_updateentity_t *bue);
    int  (*BotAddSound)(vec3_t origin, int ent, int channel, int soundindex,
                         float volume, float attenuation, float timeofs);
    int  (*BotAddPointLight)(vec3_t origin, int ent, float radius,
                              float r, float g, float b, float time, float decay);
    int  (*BotAI)(int client, float thinktime);
    int  (*BotConsoleMessage)(int client, int type, char *message);
    int  (*Test)(int parm0, char *parm1, vec3_t parm2, vec3_t parm3);
    /* AAS debug visualization */
    void (*AAS_ShowAreaFunc)(int areanum);
    void (*AAS_ShowReachableAreasFunc)(int areanum);
    void (*AAS_ClearShownDebugLinesFunc)(void);
    int  (*AAS_PointAreaNumFunc)(vec3_t point);
    int  (*AAS_AreaCenterFunc)(int areanum, vec3_t center);
    /* Chat functions (Q3 botlib API exposed to game DLL) */
    void (*BotInitialChatFunc)(int chatstate, char *type, int mcontext,
             char *var0, char *var1, char *var2, char *var3,
             char *var4, char *var5, char *var6, char *var7);
    void (*BotEnterChatFunc)(int chatstate, int clientto, int sendto);
    int  (*BotNumInitialChatsFunc)(int chatstate, char *type);
    int  (*BotChatLengthFunc)(int chatstate);
    float (*BotCharacterBFloat)(int character, int index, float min, float max);
    int  (*BotCharacterBInteger)(int character, int index, int min, int max);
    /* Death/kill notification (game DLL -> botlib) */
    void (*BotNotifyDeath)(int client, int killer, int mod);
    void (*BotNotifyKill)(int client, int victim, int mod);
    /* Query per-bot handles */
    int  (*BotGetChatState)(int client);
    int  (*BotGetCharacter)(int client);
    int  (*BotGetEnemy)(int client);
    /* Chat cooldown access */
    float (*BotGetLastChatTime)(int client);
    void  (*BotSetLastChatTime)(int client, float time);
    /* Console message queue (for chat reply) */
    int  (*BotNextConsoleMessageFunc)(int chatstate, bot_consolemessage_t *cm);
    int  (*BotReplyChatFunc)(int chatstate, char *message, int mcontext, int vcontext,
             char *var0, char *var1, char *var2, char *var3,
             char *var4, char *var5, char *var6, char *var7);
    void (*BotRemoveConsoleMessageFunc)(int chatstate, int handle);
} q2_bot_export_t;

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

/* Last weapon number (Q3 WP_*-alike weaponinfo_t.number, not a Q2
 * inventory index) commanded to Q2 via a "use <name>" client command per
 * client, purely so Q2BotAI doesn't reissue an identical "use" command
 * every server frame. Adapter-side plumbing only (like q2_entvelocity[]
 * below), not AI state -- Q2's own Use_Weapon() already no-ops on an
 * unchanged weapon regardless, so this is an efficiency guard, not a
 * correctness requirement. */
static int q2_lastweaponcmd[MAX_CLIENTS];

/* Per-entity velocity cache: computed from origin deltas between frames.
 * Neither Q2's bot_updateentity_t nor Q3's bot_entitystate_t carry velocity,
 * so we derive it in the adapter.  Used by Q2BotCheckGrenades to distinguish
 * slow grenades (speed < 300, worth dodging) from fast rockets/blaster bolts
 * (speed > 650, too fast to dodge via AAS navigation). */
#define Q2_MAX_ENTITIES 1024
typedef struct {
    vec3_t prev_origin;
    vec3_t velocity;       /* computed: (origin - prev_origin) / dt */
    float  prev_time;      /* AAS_Time() of last update */
    int    effects;        /* Q2 effects flags from last update */
} q2_entity_velocity_t;
static q2_entity_velocity_t q2_entvelocity[Q2_MAX_ENTITIES];

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

/* Q3 Trace: void out-param variant; Q2 returns by value.
 * Q3 botlib internally passes passent=-1 meaning "skip no entity".
 * Q2's BotLibImport_Trace rejects passent<0, so clamp to 0 (world). */
static void Q3Trace_Adapter(bsp_trace_t *trace, vec3_t start, vec3_t mins,
                              vec3_t maxs, vec3_t end, int passent, int contentmask)
{
    if (passent < 0) passent = 0;
    *trace = q2import.Trace(start, mins, maxs, end, passent, contentmask);
    /* Q2 uses entity 0 for the world; Q3 botlib uses ENTITYNUM_WORLD (1023).
     * BotOnTopOfEntity checks trace.ent != ENTITYNUM_WORLD to detect standing
     * on a non-world entity — translate so the check works correctly. */
    if (trace->ent == 0)
        trace->ent = ENTITYNUM_WORLD;
}

/* #5 — EntityTrace: Q2 has no dedicated per-entity trace, but we can
 * approximate it using the world trace.  Q3 uses this for collision
 * detection against specific entities (movers, platforms).  Falling back
 * to the world trace is imperfect but much better than no-hit.
 * passent=0 means "don't skip any entity except world" in Q2. */
static void Q3EntityTrace_Adapter(bsp_trace_t *trace, vec3_t start, vec3_t mins,
                                   vec3_t maxs, vec3_t end,
                                   int entnum, int contentmask)
{
    *trace = q2import.Trace(start, mins, maxs, end, 0, contentmask);
    if (trace->ent == 0)
        trace->ent = ENTITYNUM_WORLD;
}

/* inPVS — delegates to engine's PF_inPVS via the game DLL import.
 * The game DLL sets q2import.inPVS = gi.inPVS in BotSetupBotLibImport.
 * This uses the engine's pre-loaded BSP visibility data with zero
 * duplication.  Mirrors Q3's botimport.inPVS → SV_inPVS chain. */
static int Q3inPVS_Adapter(vec3_t p1, vec3_t p2)
{
    if (q2import.inPVS)
        return q2import.inPVS(p1, p2);
    return 1; /* no callback → conservatively visible */
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
 * Split at the first space so that gi.argv(0) returns just the command. */
static void Q3BotClientCommand_Adapter(int client, char *command)
{
    static char cmdbuf[256];
    char *space;

    strncpy(cmdbuf, command, sizeof(cmdbuf) - 1);
    cmdbuf[sizeof(cmdbuf) - 1] = '\0';
    space = strchr(cmdbuf, ' ');
    if (space) {
        *space = '\0';
        q2import.BotClientCommand(client, cmdbuf, space + 1, NULL);
    } else {
        q2import.BotClientCommand(client, cmdbuf, NULL);
    }
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
    const char *modestr;
    FILE       *f;
    int         handle, filesize;

    switch (mode) {
    case FS_WRITE:       modestr = "wb"; break;
    case FS_APPEND:
    case FS_APPEND_SYNC: modestr = "ab"; break;
    default:             modestr = "rb"; break;
    }

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
    {
        int pak_base = 0, pak_size = -1;
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
    if (q3 & 0x0000080) q2 |= Q2_ACTION_CROUCH;
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
    return "Q3Backport-0.1";
}

static int Q2BotSetupLibrary(void)
{
    return Export_BotLibSetup();
}

static int Q2BotShutdownLibrary(void)
{
    return Export_BotLibShutdown();
}

static int Q2BotLibraryInitialized(void)
{
    return botlibglobals.botlibsetup;
}

static int Q2BotLibVarSet(char *var_name, char *value)
{
    return Export_BotLibVarSet((char *)Q2LibVarToQ3(var_name), value);
}

static int Q2BotDefine(char *string)
{
    return PC_AddGlobalDefine(string);
}

/* Shared by Q2BotStartFrame (every frame) and Q2BotLoadMap (once, right
 * before BotSetupDeathmatchAI needs a correct answer -- see there). The
 * game DLL sets "ctf"/"teamplay"/"arena" LibVars at init time; there is
 * no real Q2 concept named "g_gametype" so this derives Q3's numbering
 * from them. */
static void Q2UpdateGametypeLibVar(void)
{
    float ctf_val = LibVarGetValue("ctf");
    float tp_val  = LibVarGetValue("teamplay");
    float ar_val  = LibVarGetValue("arena");
    if (ctf_val)
        LibVarSet("g_gametype", "4"); /* GT_CTF */
    else if (tp_val || ar_val)
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
    (void)imageindexes; (void)imageindex;

    /* The game DLL calls BotLoadMap twice:
     * 1. From g_spawn.c with the real mapname (after entity spawning)
     * 2. From bl_redirgi.c with NULL mapname (when new models are precached)
     *
     * On the real call: load BSP, load AAS, mark items, link models.
     * On the NULL reload: only re-link model indices (the table may have
     * grown since the initial call). */

    if (mapname) {
        Q_strncpyz(q2_cached_mapname, mapname, sizeof(q2_cached_mapname));

        /* Clear per-entity velocity cache from previous map */
        Com_Memset(q2_entvelocity, 0, sizeof(q2_entvelocity));

        /* Read BSP entity lump from disk before calling Q3's load */
        Q2_ReadBSPEntityData(mapname);

        errnum = Export_BotLibLoadMap(mapname);
        if (errnum != BLERR_NOERROR) return errnum;

        /* Mark all items loaded from BSP as "always present" */
        BotMarkLevelItemsPresent();

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
        Q2UpdateGametypeLibVar();

        /* Real Q3's BotAISetup calls this to register bot_rocketjump/
         * bot_grapple/etc. LibVars, set the plain gametype/maxclients
         * globals every ported file outside this adapter reads directly,
         * look up CTF flag goals when gametype is GT_CTF, and call
         * BotInitWaypoints(). trap_Cvar_Register (ai_q2_shim.c) is
         * idempotent (LibVar() no-ops if already registered), so calling
         * this again on every map change/restart is safe. */
        BotSetupDeathmatchAI();

        /* Mirrors real Q3's BotAILoadMap: reset any already-active bots'
         * state on a (re)load so stale goals/nodes from the previous map
         * don't carry over. */
        {
            int i;
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (botstates[i] && botstates[i]->inuse) {
                    BotResetState(botstates[i]);
                    botstates[i]->setupcount = 4;
                }
            }
        }
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
    return Q2_BLERR_NOERROR;
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

    return Q2_BLERR_NOERROR;
}

static int Q2BotClientSettings(int client, q2_bot_clientsettings_t *settings)
{
    bot_state_t *bs;

    if (client < 0 || client >= MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
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
    q2_redflagstatus  = Q2AI_FlagAwayFromBase(&ctf_redflag);
    q2_blueflagstatus = Q2AI_FlagAwayFromBase(&ctf_blueflag);
}

/* Phase 2 (Problem 2 fix): exposes the already-existing, already-wired
 * q2_bot_import_t.OnSameTeam callback (frozen ABI, unused elsewhere in
 * this adapter until now) to game_q3/ai_dmq3.c's BotSameTeam(), which
 * needs a pairwise "are these two clients on the same real team" answer
 * and can't reach the file-local `q2import` struct directly. Client
 * numbers in (0-indexed, matching bs->client/ai_dmq3.c's own convention)
 * -> Q2 g_edicts[] numbers out (1-indexed), exactly like
 * Q2BotUpdateClient's client+1 elsewhere in this file. */
int Q2_ClientsOnSameTeam(int client1, int client2)
{
    if (!q2import.OnSameTeam) return 0;
    return q2import.OnSameTeam(client1 + 1, client2 + 1);
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

    ret = Export_BotLibStartFrame(time);

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

    /* Update dynamic item entities at 0.3s intervals (Q3 ai_main.c:1471) */
    if (AAS_Time() - q2_entityitems_time >= 0.3f) {
        BotUpdateEntityItems();
        q2_entityitems_time = AAS_Time();
    }
    /* Update gametype for CTF/team detection.  The game DLL sets
     * "ctf", "teamplay", and "arena" LibVars at init time. */
    Q2UpdateGametypeLibVar();
    return ret;
}

static int Q2BotUpdateClient(int client, q2_bot_updateclient_t *buc)
{
    bot_entitystate_t state;
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
    for (i = 0; i < 3; i++)
        bs->cur_ps.delta_angles[i] = ANGLE2SHORT(buc->delta_angles[i]);

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
    bs->cur_ps.pm_time = buc->pm_time;
    /* Q2 doesn't send a groundEntityNum; infer on-ground from
     * PMF_ON_GROUND (bit 4), matching Q2BotUpdateEntity's identical
     * convention below for other entities. */
    bs->cur_ps.groundEntityNum = (buc->pm_flags & 4) ? 0 : ENTITYNUM_NONE;
    bs->cur_ps.persistant[Q3PERS_SCORE] = buc->stats[14]; /* Q2 STAT_FRAGS */

    /* --- Populate bs->inventory[] straight from Q2's real per-client
     * inventory + health/armor stats. See BotUpdateInventory's
     * replacement (game_q3/ai_dmq3.c) for why a direct copy is correct:
     * Q2's item indices already match assets/botfiles/inv.h's
     * INVENTORY_* slots (both MAX_ITEMS/Q2_MAX_ITEMS are 256). --- */
    Com_Memcpy(bs->inventory, buc->inventory, sizeof(bs->inventory));
    bs->inventory[28] = buc->stats[1]; /* INVENTORY_HEALTH (inv.h); Q2 STAT_HEALTH=1 */
    bs->inventory[29] = buc->stats[5]; /* INVENTORY_ARMOR (ai_q2_compat.h); Q2 STAT_ARMOR=5 */

    /* --- Phase 2 (Problem 2 fix): derive this bot's REAL CTF team.
     * ai_dmq3.c's BotTeam() can no longer read it via trap_GetConfigstring
     * (permanent stub -- see ai_q2_shim.c and the report), so it's sourced
     * here instead from the ordinary stats[] array, already part of the
     * frozen ABI: game_q2/g_ctf.c's SetCTFStats() (run every server frame
     * for every client, bots included, via p_hud.c's G_SetStats()) sets
     *   stats[22] (STAT_CTF_JOINED_TEAM1_PIC) nonzero <=> resp.ctf_team==CTF_TEAM1 (Red)
     *   stats[23] (STAT_CTF_JOINED_TEAM2_PIC) nonzero <=> resp.ctf_team==CTF_TEAM2 (Blue)
     * Confirmed correctly labeled, not just internally consistent:
     * game_q2/g_items.c's item_flag_team1/item_flag_team2 pickup names are
     * literally "Red Flag"/"Blue Flag", matching ctf_redflag/EF_FLAG1 and
     * ctf_blueflag/EF_FLAG2 respectively. --- */
    if (buc->stats[22])      bs->q2_realctfteam = Q3TEAM_RED;
    else if (buc->stats[23]) bs->q2_realctfteam = Q3TEAM_BLUE;
    else                     bs->q2_realctfteam = Q3TEAM_FREE;

    /* Same trick, for BotChat_HitTalking/HitNoDeath/HitNoKill (ai_chat.c):
     * real lasthurt_client/mod (game_q2/g_combat.c), piggybacked through
     * the otherwise-unused stats[28]/[29] by game_q2/bl_main.c. See
     * ai_main.h's q2_reallasthurt_client/_mod comment. */
    bs->q2_reallasthurt_client = buc->stats[28];
    bs->q2_reallasthurt_mod    = buc->stats[29];

    /* Translate to Q3 entity state for AAS entity tracking */
    Com_Memset(&state, 0, sizeof(state));
    state.type = 1;  /* ET_PLAYER */
    VectorCopy(buc->origin,     state.origin);
    VectorCopy(buc->viewangles, state.angles);
    VectorCopy(buc->origin,     state.old_origin);
    /* Standard Q2 player bounding box */
    state.mins[0] = -16.0f; state.mins[1] = -16.0f;
    state.mins[2] = (buc->pm_flags & 4) ? -16.0f : -24.0f; /* ducked? */
    state.maxs[0] =  16.0f; state.maxs[1] =  16.0f;
    state.maxs[2] = (buc->pm_flags & 4) ?   4.0f :  32.0f;
    state.solid  = (buc->pm_type == Q2PM_NORMAL) ? 1 : 0;
    /* #10 — Weapon state: set from gunindex so EntityIsShooting can work */
    state.weapon = buc->gunindex;
    /* #16 — Ground entity: Q2 doesn't have groundEntityNum in bot update,
     * but we can infer it from pm_flags & PMF_ON_GROUND (bit 4).
     * Set to ENTITYNUM_NONE if not on ground, 0 (world) if on ground. */
    state.groundent = (buc->pm_flags & 4) ? 0 : ENTITYNUM_NONE;

    /* In Q2, client numbers are 0-indexed but entity numbers are 1-indexed.
     * The game DLL calls BotUpdateClient(client) and BotUpdateEntity(client+1)
     * for the same bot, so we must store bot state at entity slot client+1
     * to match what BotUpdateEntity writes and what the combat scan reads. */
    return Export_BotLibUpdateEntity(client + 1, &state);
}

static int Q2BotUpdateEntity(int ent, q2_bot_updateentity_t *bue)
{
    bot_entitystate_t state;

    if (!ValidEntityNumber(ent, "BotUpdateEntity"))
        return Q2_BLERR_INVALIDENTITYNUMBER;

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
    /* #11 — Powerup bits: Q2 uses EF_* in the effects field.
     * Map Q2 effects to Q3 powerups bitmask for EntityCarriesFlag etc.
     * Q2: EF_QUAD=0x00000080, EF_PENT=0x00000100 (invulnerability) */
    state.powerups = 0;
    if (bue->effects & 0x00000080) state.powerups |= (1 << 3); /* PW_QUAD=3 */
    if (bue->effects & 0x00000100) state.powerups |= (1 << 4); /* PW_BATTLESUIT=4 */
    /* CTF flag carrier powerups — map Q2 effects to Q3 PW_ bits for
     * EntityCarriesFlag() and BotTeamFlagCarrierVisible() */
    if (bue->effects & Q2_EF_FLAG1_CARRIER) state.powerups |= (1 << 8); /* PW_REDFLAG */
    if (bue->effects & Q2_EF_FLAG2_CARRIER) state.powerups |= (1 << 9); /* PW_BLUEFLAG */
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
        int maxcl = (int)LibVarGetValue("maxclients");
        if (maxcl > 0 && ent >= 1 && ent <= maxcl) {
            state.type = 1; /* ET_PLAYER */
            /* #10 — Weapon state on player entities: Q2 stores the weapon
             * model in modelindex2.  Set state.weapon so EntityIsShooting()
             * and other Q3 checks can detect the player's current weapon. */
            state.weapon = bue->modelindex2;
        } else if (ent > 0 && bue->solid == 3 /* SOLID_BSP */ && bue->modelindex > 0) {
            /* Skip entity 0 (worldspawn) — it has SOLID_BSP + modelindex
             * but is NOT a mover.  Without this check, the world entity
             * masks real func_plat/func_door entities in
             * AAS_OriginOfMoverWithModelNum lookups. */
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
    /* Compute entity velocity from origin delta between frames.
     * Stored in adapter-side cache (not in bot_entitystate_t or
     * aas_entityinfo_t, which are Q3 botlib structs we don't modify).
     * Used by Q2BotCheckGrenades to filter slow grenades from fast
     * rockets/blaster bolts. */
    if (ent >= 0 && ent < Q2_MAX_ENTITIES) {
        q2_entity_velocity_t *ev = &q2_entvelocity[ent];
        float dt = AAS_Time() - ev->prev_time;
        if (ev->prev_time > 0.0f && dt > 0.001f) {
            vec3_t delta;
            VectorSubtract(bue->origin, ev->prev_origin, delta);
            VectorScale(delta, 1.0f / dt, ev->velocity);
        } else {
            VectorClear(ev->velocity);
        }
        VectorCopy(bue->origin, ev->prev_origin);
        ev->prev_time = AAS_Time();
        ev->effects = bue->effects;
    }

    return Export_BotLibUpdateEntity(ent, &state);
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
 * Q2BotCheckGrenades — dodge grenades on the ground
 *
 * Real Q3's BotCheckSnapshot (game_q3/ai_dmq3.c, called from
 * BotDeathmatchAI) drives grenade avoidance via BotCheckForGrenades,
 * fed by a Q3 snapshot-entity loop this Q2 port has no equivalent for
 * (trap_BotGetSnapshotEntity is stubbed to "no more entities" -- see
 * botlib/ai_q2_shim.c). Per the plan, BotCheckSnapshot's replacement
 * (game_q3/ai_dmq3.c) is a no-op and this equivalent scan runs from here
 * instead, called from Q2BotAI just below BEFORE the real BotAI() (so
 * avoid-spots are fresh when that same call's BotMoveToGoal consults
 * them). Scans entities for Q2 grenades (identified by the EF_GRENADE
 * effects bit, cached per-entity in q2_entvelocity[] by
 * Q2BotUpdateEntity -- aas_entityinfo_t itself carries no effects/weapon
 * detail fine-grained enough to tell a grenade apart from a rocket or
 * blaster bolt, all three of which are classified plain ET_MISSILE).
 * Only grenades are avoided -- rockets and blaster bolts travel too fast
 * to dodge via AAS navigation and would flood the 32-slot avoid-spot
 * array.
 * ==================================================================== */
static void Q2BotCheckGrenades(bot_state_t *bs)
{
    int ent;
    aas_entityinfo_t entinfo;
    vec3_t diff;

    BotAddAvoidSpot(bs->ms, vec3_origin, 0, AVOID_CLEAR);

    for (ent = AAS_NextEntity(0); ent; ent = AAS_NextEntity(ent)) {
        if (AAS_EntityType(ent) != 3 /* ET_MISSILE */) continue;
        if (ent < 0 || ent >= Q2_MAX_ENTITIES) continue;
        if (!(q2_entvelocity[ent].effects & Q2_EF_GRENADE)) continue;
        AAS_EntityInfo(ent, &entinfo);
        if (!entinfo.valid) continue;
        /* Only nearby grenades */
        VectorSubtract(entinfo.origin, bs->cur_ps.origin, diff);
        if (VectorLength(diff) > 400.0f) continue;
        BotAddAvoidSpot(bs->ms, entinfo.origin, 160, AVOID_ALWAYS);
    }
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
static int Q2BotAI(int client, float thinktime)
{
    bot_state_t   *bs;
    bot_input_t    q3input;
    q2_bot_input_t q2input;

    if (client < 0 || client >= MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    bs = botstates[client];
    if (!bs || !bs->inuse) return Q2_BLERR_AICLIENTNOTSETUP;

    /* Hazard awareness with no Q2-native equivalent in the real Q3
     * source -- see the function comment above. Must run before BotAI()
     * so this frame's BotMoveToGoal sees fresh avoid spots. */
    Q2BotCheckGrenades(bs);

    /* Phase 2 (Problem 1 fix): copy this frame's CTF flag-status snapshot
     * (computed once for every bot in Q2AI_UpdateCTFFlagStatus(), called
     * from Q2BotStartFrame) onto this bot. Real Q3 feeds these per-bot
     * from a configstring parse; Q2 has none, so every bot just reads the
     * same per-frame snapshot instead. */
    bs->redflagstatus  = q2_redflagstatus;
    bs->blueflagstatus = q2_blueflagstatus;

    /* The real Q3 AI: state machine, combat, weapon choice, goal
     * selection, chat -- see game_q3/ai_main.c/ai_dmnet.c/ai_dmq3.c/
     * ai_team.c/ai_chat.c. */
    BotAI(client, thinktime);

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
     * to track a target or face their own movement. */
    BotUpdateInput(bs, thinktime);

    /* --- Collect EA input and translate to Q2 (unchanged in spirit
     * from the previous adapter's tail end -- this part is orthogonal to
     * where the AI decision-making comes from). BotUpdateInput above
     * already resolved the final view angles via trap_EA_View, so
     * q3input.viewangles below is already the fully-resolved answer; no
     * separate priority reconstruction is needed on this side of the
     * bridge. --- */
    Com_Memset(&q3input, 0, sizeof(q3input));
    EA_GetInput(client, thinktime, &q3input);

    Com_Memset(&q2input, 0, sizeof(q2input));
    q2input.thinktime   = q3input.thinktime;
    VectorCopy(q3input.dir, q2input.dir);
    q2input.speed       = q3input.speed;
    q2input.actionflags = Q3ActionsToQ2(q3input.actionflags);
    VectorCopy(q3input.viewangles, q2input.viewangles);

    /* Translate EA_SelectWeapon to Q2's "use <name>" client command.
     * The real BotAI() (ai_main.c) calls trap_EA_SelectWeapon(bs->client,
     * bs->weaponnum) every frame regardless of change; EA_GetInput()
     * reports the result back as q3input.weapon. Q2 has no per-frame
     * "desired weapon" usercmd field -- gate on q2_lastweaponcmd[] so an
     * unchanged weapon doesn't reissue "use" 10-20x/sec (Q2's own
     * Use_Weapon() already no-ops in that case regardless -- this is an
     * efficiency guard, not a correctness requirement). */
    if (q3input.weapon > 0 && q3input.weapon != q2_lastweaponcmd[client]) {
        weaponinfo_t wi;
        BotGetWeaponInfo(bs->ws, q3input.weapon, &wi);
        if (wi.name[0]) {
            q2import.BotClientCommand(client, "use", wi.name, NULL);
            q2_lastweaponcmd[client] = q3input.weapon;
        }
    }

    q2import.BotInput(client, &q2input);
    EA_ResetInput(client);

    return Q2_BLERR_NOERROR;
}

static int Q2BotConsoleMessage(int client, int type, char *message)
{
    bot_state_t *bs;

    if (client < 0 || client >= MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    bs = botstates[client];
    if (!bs || !bs->inuse) return Q2_BLERR_AICMFORINACTIVECLIENT;

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
static void Q2BotNotifyDeath(int client, int killer, int mod)
{
    bot_state_t *bs;
    qboolean is_suicide;
    int i;

    if (client < 0 || client >= MAX_CLIENTS) return;

    is_suicide = (client == killer || killer < 0);

    /* Victim-side bookkeeping only applies when the victim is itself an
     * active bot -- this call now also arrives for human victims (see
     * game_q2/p_client.c's BotNotifyDeathKill) purely to drive the
     * observer broadcast below. */
    bs = botstates[client];
    if (bs && bs->inuse) {
        bs->botdeathtype = mod;
        bs->lastkilledby = killer;
        bs->botsuicide   = is_suicide;
        bs->num_deaths++;
    }

    /* Tell every bot that had this (bot or human) victim tracked as its
     * current enemy, mirroring real Q3's per-observer notification and
     * game_q2/bl_chat.c's now-deleted BotChat_NotifyDeath's third branch
     * -- not just the credited killer, who in a suicide doesn't exist. */
    if (is_suicide) {
        for (i = 0; i < maxclients; i++) {
            bot_state_t *obs = botstates[i];
            if (obs && obs->inuse && obs->enemy == client)
                obs->enemysuicide = true;
        }
    }
}

static void Q2BotNotifyKill(int client, int victim, int mod)
{
    bot_state_t *bs;
    if (client < 0 || client >= MAX_CLIENTS) return;
    bs = botstates[client];
    if (!bs || !bs->inuse) return;
    bs->enemydeathtype   = mod;
    bs->lastkilledplayer = victim;
    bs->num_kills++;
    /* Suicide/observer notification is handled by Q2BotNotifyDeath above,
     * which now runs for every victim (bot or human) and broadcasts
     * bs->enemysuicide to every bot tracking that victim as its enemy --
     * this call path only ever fires for genuine, non-suicide kills. */
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
/* On 32-bit Windows the game DLL declares the function pointer as WINAPI
 * (__stdcall).  We must match that calling convention or the stack is
 * corrupted on return (ESP off by 4 → immediate crash). */
#if defined(_WIN32) && !defined(_WIN64)
__declspec(dllexport) q2_bot_export_t * __stdcall GetBotAPI(q2_bot_import_t *import)
#else
q2_bot_export_t *GetBotAPI(q2_bot_import_t *import)
#endif
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
    q3imp.PointContents         = import->PointContents;
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
     *   pm_airaccelerate = 0     (Q3: 1 — Q2 has NO air control)
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
