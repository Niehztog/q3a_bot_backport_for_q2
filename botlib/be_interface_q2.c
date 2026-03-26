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
 * Per-client AI state
 *
 * AI state machine modelled after Q3's AINode system (ai_dmnet.c) and
 * the Gladiator bot's internal state machine.  Both systems use the same
 * states; Gladiator kept them inside botlib.dll, Q3 moved them to the
 * game DLL.  We bring them back into botlib.
 *
 *   SEEK_LTG → [enemy found + aggression >= 50] → BATTLE_FIGHT
 *            → [enemy found + aggression <  50] → BATTLE_RETREAT
 *
 *   BATTLE_FIGHT → [enemy lost]         → SEEK_LTG
 *                → [aggression drops]   → BATTLE_RETREAT
 *
 *   BATTLE_RETREAT → [enemy lost]       → SEEK_LTG
 *                  → [aggression rises] → BATTLE_FIGHT
 * ==================================================================== */
#define Q2_BOTLIB_MAX_CLIENTS 256

typedef enum {
    Q2AI_SEEK_LTG,        /* navigating to long-term goal (no combat) */
    Q2AI_SEEK_NBG,        /* diverting to nearby goal (no combat) */
    Q2AI_BATTLE_FIGHT,    /* dedicated combat; goals paused */
    Q2AI_BATTLE_RETREAT,  /* navigating to goals while defending */
    Q2AI_BATTLE_CHASE,    /* pursuing enemy's last known position */
    Q2AI_BATTLE_NBG,      /* picking up nearby item during combat */
} q2_aistate_t;

typedef struct {
    qboolean    inuse;
    int         character;
    int         chatstate;
    int         goalstate;
    int         movestate;
    int         weaponstate;
    /* Cached from last BotUpdateClient */
    vec3_t      origin;
    vec3_t      velocity;
    vec3_t      viewangles;
    vec3_t      viewoffset;
    byte        pm_flags;
    q2_pmtype_t pm_type;
    int         inventory[Q2_MAX_ITEMS];
    /* AI state machine */
    q2_aistate_t aistate;
    int         enemy;          /* entity number of current enemy (-1 = none) */
    float       enemy_time;     /* AAS_Time() when enemy was last visible */
    /* Goal tracking */
    bot_goal_t  ltg;            /* cached LTG; used as context for NBG range check */
    qboolean    hasgoal;        /* an LTG is on the goal stack */
    qboolean    hasnbg;         /* an NBG is pushed on top of the LTG */
    float       ltg_check_time; /* AAS_Time() when BotChooseLTGItem was last called */
    float       goal_set_time;  /* AAS_Time() when the current LTG was set */
    float       nbg_check_time; /* AAS_Time() when BotChooseNBGItem was last checked */
    q2_bot_settings_t settings;
    /* Weapon selection: weapon number last sent via "use" (0 = not set) */
    int         best_weapon_num;
    /* Enemy distance from last frame (written to inventory[200/201] before
     * weapon selection so default_w.c can use range-aware weights).
     * Mirrors Q3's BotUpdateBattleInventory / ENEMY_HORIZONTAL_DIST scheme. */
    int         enemy_hdist;    /* horizontal distance to nearest visible enemy */
    int         enemy_height;   /* vertical offset (enemy.z - bot.z) */
    /* Player stats cached from BotUpdateClient (Q2 stats[] array) */
    int         health;         /* stats[STAT_HEALTH=1] */
    int         armor;          /* stats[STAT_ARMOR=5] (#8) */
    int         lasthealth;     /* health from previous frame (damage detection) */
    /* Personality traits — cached from character file in Q2BotSetupClient.
     * Mirrors Q3's per-frame Characteristic_BFloat() calls but cached
     * once at setup for efficiency (traits don't change during a game). */
    float       attack_skill;       /* [0,1] strafe quality, distance management */
    float       aim_skill;          /* [0,1] trajectory prediction quality */
    float       aim_accuracy;       /* [0,1] per-weapon accuracy scatter */
    float       reactiontime;       /* [0,5] delay before engaging (seconds) */
    float       alertness;          /* [0,1] detection range scale */
    float       jumper;             /* [0,1] jump probability in combat */
    float       croucher;           /* [0,1] crouch probability in combat */
    float       firethrottle;       /* [0,1] fire-pause pattern (0=always fire) */
    float       camper;             /* [0,1] camping tendency */
    float       easy_fragger;       /* [0,1] will shoot chatting players */
    float       weaponjumping;      /* [0,1] rocket jump willingness */
    /* Combat movement (mirrors Q3's BotAttackMove in ai_dmq3.c:2635) */
    float       attackstrafe_time;  /* accumulated strafe duration */
    float       attackjump_time;    /* next allowed jump time */
    float       attackcrouch_time;  /* next allowed crouch time */
    int         flags;              /* BFL_STRAFERIGHT (1), BFL_AVOIDRIGHT (2), BFL_ATTACKED (4) */
#define BFL_STRAFERIGHT  1
#define BFL_AVOIDRIGHT   2
#define BFL_ATTACKED     4
    /* Reaction time + fire throttle state (mirrors Q3's BotCheckAttack) */
    float       enemysight_time;    /* AAS_Time() when enemy first spotted this engagement */
    float       firethrottlewait_time;  /* next time bot will consider firing again */
    float       firethrottleshoot_time; /* next time bot must re-evaluate fire/wait */
    /* Enemy velocity tracking for aim prediction */
    vec3_t      enemyorigin;        /* enemy origin cached every 0.5s for prediction */
    vec3_t      enemyvelocity;      /* enemy velocity cached every 0.5s */
    float       enemyposition_time; /* AAS_Time() when enemyorigin/velocity was last updated */
    /* Stuck detection */
    vec3_t      lastorigin;         /* origin from previous frame */
    float       notblocked_time;    /* last time NOT blocked */
    /* Hazard/environment */
    float       lastair_time;       /* last time bot was breathing air */
    /* Powerup tracking (compare with prev frame to detect pickup) */
    int         prev_quad;          /* inventory[23] from previous frame */
    int         prev_invuln;        /* inventory[24] from previous frame */
    /* Enemy chase (mirrors Q3's AINode_Battle_Chase):
     * When LOS is lost, bot navigates toward the enemy's last known
     * position for up to 10 seconds before giving up. */
    vec3_t      lastenemyorigin;    /* world pos where enemy was last seen */
    int         lastenemyareanum;   /* AAS area of that position */
    float       chase_time;         /* AAS_Time() when chase started (0 = not chasing) */
    /* Camping (mirrors Q3's BotWantsToCamp / BotGoCamp) */
    float       camp_time;          /* AAS_Time() when last camp ended (cooldown) */
    /* NBG during combat: time limit for nearby goal pickup */
    float       nbg_combat_time;    /* AAS_Time() deadline for battle NBG */
    /* Retreat nearby item check throttle */
    float       retreat_check_time; /* next time to check for nearby items */
    /* Spawn protection: recently teleported players */
    float       teleport_time;      /* AAS_Time() when bot last teleported */
    /* Random walk fallback when navigation fails persistently. */
    vec3_t      roam_dir;           /* current random walk direction */
    float       roam_dir_time;      /* AAS_Time() when roam_dir was last set */
    float       move_fail_time;     /* AAS_Time() when movedir first became zero */
} q2_botclient_t;

/* ====================================================================
 * Module globals
 * ==================================================================== */
static q2_bot_import_t q2import;         /* stored Q2 import callbacks  */
static q2_bot_export_t q2_export;        /* returned Q2 export struct   */
static q2_botclient_t  q2clients[Q2_BOTLIB_MAX_CLIENTS];

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

/* Q3 BotClientCommand takes a plain string; Q2 takes variadic */
static void Q3BotClientCommand_Adapter(int client, char *command)
{
    q2import.BotClientCommand(client, command);
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
        /* Clear per-entity velocity cache from previous map */
        Com_Memset(q2_entvelocity, 0, sizeof(q2_entvelocity));

        /* Read BSP entity lump from disk before calling Q3's load */
        Q2_ReadBSPEntityData(mapname);

        errnum = Export_BotLibLoadMap(mapname);
        if (errnum != BLERR_NOERROR) return errnum;

        /* Mark all items loaded from BSP as "always present" */
        BotMarkLevelItemsPresent();
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

static int Q2BotSetupClient(int client, q2_bot_settings_t *settings)
{
    q2_botclient_t *bc;
    char filename[Q2_MAX_FILEPATH];
    char name[Q2_MAX_CHARACTERNAME];
    char gender[16];
    int  errnum;

    if (client < 0 || client >= Q2_BOTLIB_MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;

    bc = &q2clients[client];
    if (bc->inuse) {
        botimport.Print(PRT_WARNING,
            "BotSetupClient: client %d already setup\n", client);
        return Q2_BLERR_AICLIENTALREADYSETUP;
    }

    Com_Memset(bc, 0, sizeof(*bc));
    bc->aistate         = Q2AI_SEEK_LTG;
    bc->enemy           = -1;
    bc->enemy_hdist     = 9999;   /* no enemy yet — don't bias range weights */
    bc->enemy_height    = 0;
    bc->notblocked_time = -1.0f;  /* sentinel: first frame will set to AAS_Time() */
    Com_Memcpy(&bc->settings, settings, sizeof(*settings));

    /* Load character file (determines weights, chat, skills).
     * Try the specified file first; fall back to our default character. */
    bc->character = BotLoadCharacter(settings->characterfile, 5.0f);
    if (!bc->character && settings->characterfile[0]) {
        /* specified file failed — try default */
        bc->character = BotLoadCharacter("bots/default_c.c", 5.0f);
    }
    if (!bc->character) {
        botimport.Print(PRT_WARNING,
            "BotSetupClient: no character file loaded, using hardcoded defaults\n");
    }

    /* Cache personality traits from character file.
     * Q3 reads these per-frame with trap_Characteristic_BFloat().
     * We cache them once since they don't change during a game.
     * Fallback values are for skill ~3 (moderate). */
    if (bc->character) {
        bc->attack_skill  = Characteristic_BFloat(bc->character, Q2CHAR_ATTACK_SKILL, 0, 1);
        bc->aim_skill     = Characteristic_BFloat(bc->character, Q2CHAR_AIM_SKILL, 0, 1);
        bc->aim_accuracy  = Characteristic_BFloat(bc->character, Q2CHAR_AIM_ACCURACY, 0, 1);
        bc->reactiontime  = Characteristic_BFloat(bc->character, Q2CHAR_REACTIONTIME, 0, 5);
        bc->alertness     = Characteristic_BFloat(bc->character, Q2CHAR_ALERTNESS, 0, 1);
        bc->jumper        = Characteristic_BFloat(bc->character, Q2CHAR_JUMPER, 0, 1);
        bc->croucher      = Characteristic_BFloat(bc->character, Q2CHAR_CROUCHER, 0, 1);
        bc->firethrottle  = Characteristic_BFloat(bc->character, Q2CHAR_FIRETHROTTLE, 0, 1);
        bc->camper        = Characteristic_BFloat(bc->character, Q2CHAR_CAMPER, 0, 1);
        bc->easy_fragger  = Characteristic_BFloat(bc->character, Q2CHAR_EASY_FRAGGER, 0, 1);
        bc->weaponjumping = Characteristic_BFloat(bc->character, Q2CHAR_WEAPONJUMPING, 0, 1);
    } else {
        bc->attack_skill  = 0.6f;
        bc->aim_skill     = 0.6f;
        bc->aim_accuracy  = 0.65f;
        bc->reactiontime  = 0.7f;
        bc->alertness     = 0.6f;
        bc->jumper        = 0.5f;
        bc->croucher      = 0.3f;
        bc->firethrottle  = 0.6f;
        bc->camper        = 0.1f;
        bc->easy_fragger  = 0.5f;
        bc->weaponjumping = 0.5f;
    }

    /* Goal state + item weights */
    bc->goalstate = BotAllocGoalState(client);
    if (bc->character) {
        Characteristic_String(bc->character, Q2CHAR_ITEMWEIGHTS,
                               filename, sizeof(filename));
    } else {
        Q_strncpyz(filename, "bots/default_i.c", sizeof(filename));
    }
    errnum = BotLoadItemWeights(bc->goalstate, filename);
    if (errnum != BLERR_NOERROR)
        botimport.Print(PRT_WARNING,
            "BotSetupClient: BotLoadItemWeights(%s) failed: %d\n",
            filename, errnum);

    /* Weapon state + weapon weights */
    bc->weaponstate = BotAllocWeaponState();
    if (bc->character) {
        Characteristic_String(bc->character, Q2CHAR_WEAPONWEIGHTS,
                               filename, sizeof(filename));
    } else {
        Q_strncpyz(filename, "bots/default_w.c", sizeof(filename));
    }
    errnum = BotLoadWeaponWeights(bc->weaponstate, filename);
    if (errnum != BLERR_NOERROR)
        botimport.Print(PRT_WARNING,
            "BotSetupClient: BotLoadWeaponWeights(%s) failed: %d\n",
            filename, errnum);

    /* Chat state */
    bc->chatstate = BotAllocChatState();
    if (bc->character) {
        Characteristic_String(bc->character, Q2CHAR_CHAT_FILE,
                               filename, sizeof(filename));
        Characteristic_String(bc->character, Q2CHAR_CHAT_NAME,
                               name, sizeof(name));
        errnum = BotLoadChatFile(bc->chatstate, filename, name);
        if (errnum != BLERR_NOERROR)
            botimport.Print(PRT_WARNING,
                "BotSetupClient: BotLoadChatFile(%s) failed: %d\n",
                filename, errnum);

        Characteristic_String(bc->character, Q2CHAR_GENDER, gender, sizeof(gender));
        if      (*gender == 'f' || *gender == 'F')
            BotSetChatGender(bc->chatstate, CHAT_GENDERFEMALE);
        else if (*gender == 'm' || *gender == 'M')
            BotSetChatGender(bc->chatstate, CHAT_GENDERMALE);
        else
            BotSetChatGender(bc->chatstate, CHAT_GENDERLESS);
    }
    BotSetChatName(bc->chatstate, settings->charactername, client);

    bc->movestate = BotAllocMoveState();
    bc->inuse = true;
    return 1;  /* BotSetupClient returns true/false, not BLERR_ codes (see botlib.h) */
}

static int Q2BotShutdownClient(int client)
{
    q2_botclient_t *bc;

    if (client < 0 || client >= Q2_BOTLIB_MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;

    bc = &q2clients[client];
    if (!bc->inuse) {
        botimport.Print(PRT_WARNING,
            "BotShutdownClient: client %d not setup\n", client);
        return Q2_BLERR_AICLIENTALREADYSHUTDOWN;
    }

    BotFreeMoveState(bc->movestate);
    BotFreeGoalState(bc->goalstate);
    BotFreeChatState(bc->chatstate);
    BotFreeWeaponState(bc->weaponstate);
    if (bc->character) BotFreeCharacter(bc->character);

    Com_Memset(bc, 0, sizeof(*bc));
    return Q2_BLERR_NOERROR;
}

static int Q2BotMoveClient(int oldclnum, int newclnum)
{
    q2_botclient_t *src, *dst;

    if (oldclnum < 0 || oldclnum >= Q2_BOTLIB_MAX_CLIENTS)
        return Q2_BLERR_AIMOVEINACTIVECLIENT;
    if (newclnum < 0 || newclnum >= Q2_BOTLIB_MAX_CLIENTS)
        return Q2_BLERR_AIMOVETOACTIVECLIENT;

    src = &q2clients[oldclnum];
    dst = &q2clients[newclnum];
    if (!src->inuse) return Q2_BLERR_AIMOVEINACTIVECLIENT;

    Com_Memcpy(dst, src, sizeof(*dst));
    Com_Memset(src, 0, sizeof(*src));
    return Q2_BLERR_NOERROR;
}

static int Q2BotClientSettings(int client, q2_bot_clientsettings_t *settings)
{
    q2_botclient_t *bc;

    if (client < 0 || client >= Q2_BOTLIB_MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    bc = &q2clients[client];
    if (!bc->inuse) return Q2_BLERR_SETTINGSINACTIVECLIENT;

    BotSetChatName(bc->chatstate, settings->netname, client);
    return Q2_BLERR_NOERROR;
}

static int Q2BotSettings(int client, q2_bot_settings_t *settings)
{
    q2_botclient_t *bc;

    if (client < 0 || client >= Q2_BOTLIB_MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    bc = &q2clients[client];
    if (!bc->inuse) return Q2_BLERR_SETTINGSINACTIVECLIENT;

    Com_Memcpy(&bc->settings, settings, sizeof(*settings));
    return Q2_BLERR_NOERROR;
}

/* #6 — BotUpdateEntityItems timing: Q3 calls this at ~0.3s intervals
 * via BotAIRegularUpdate(), not every frame.  We throttle here to match. */
static float q2_entityitems_time;

static int Q2BotStartFrame(float time)
{
    int ret = Export_BotLibStartFrame(time);
    /* Update dynamic item entities at 0.3s intervals (Q3 ai_main.c:1471) */
    if (AAS_Time() - q2_entityitems_time >= 0.3f) {
        BotUpdateEntityItems();
        q2_entityitems_time = AAS_Time();
    }
    return ret;
}

static int Q2BotUpdateClient(int client, q2_bot_updateclient_t *buc)
{
    bot_entitystate_t state;
    q2_botclient_t   *bc;

    if (client < 0 || client >= Q2_BOTLIB_MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    bc = &q2clients[client];
    if (!bc->inuse) return Q2_BLERR_AIUPDATEINACTIVECLIENT;

    /* Cache Q2 state for use in BotAI */
    VectorCopy(buc->origin,     bc->origin);
    VectorCopy(buc->velocity,   bc->velocity);
    VectorCopy(buc->viewangles, bc->viewangles);
    VectorCopy(buc->viewoffset, bc->viewoffset);
    bc->pm_flags = buc->pm_flags;
    bc->pm_type  = buc->pm_type;
    Com_Memcpy(bc->inventory, buc->inventory, sizeof(bc->inventory));
    bc->health = buc->stats[1]; /* STAT_HEALTH = 1 in Q2 */
    bc->armor  = buc->stats[5]; /* #8: STAT_ARMOR = 5 in Q2 */

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
     *   ET_MISSILE (3): SOLID_BBOX (2) + modelindex > 0.
     *                   Rockets, grenades, blaster bolts, and the CTF grapple
     *                   hook.  state.weapon is set to modelindex so that
     *                   GrappleState() can identify the hook when the game DLL
     *                   sets weapindex_grapple to the hook model's index via
     *                   BotLibVarSet("weapindex_grapple", "<modelindex>").
     *
     *   ET_GENERAL (0): everything else — trigger volumes, non-solid
     *                   decorative models, effects (SOLID_NOT with any
     *                   modelindex, or any other solid value). */
    {
        int maxcl = (int)LibVarGetValue("maxclients");
        if (maxcl > 0 && ent >= 1 && ent <= maxcl) {
            state.type = 1; /* ET_PLAYER */
            /* #10 — Weapon state on player entities: Q2 stores the weapon
             * model in modelindex2.  Set state.weapon so EntityIsShooting()
             * and other Q3 checks can detect the player's current weapon. */
            state.weapon = bue->modelindex2;
        } else if (bue->solid == 3 /* SOLID_BSP */ && bue->modelindex > 0) {
            state.type = 4; /* ET_MOVER */
        } else if (bue->solid == 1 /* SOLID_TRIGGER */ && bue->modelindex > 0) {
            state.type = 2; /* ET_ITEM */
        } else if (bue->solid == 2 /* SOLID_BBOX */ && bue->modelindex > 0) {
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
 * Q2BotAggression — evaluate combat readiness based on weapon loadout
 *
 * Returns 0-100.  Values below 50 mean the bot should avoid combat and
 * focus on acquiring better weapons/items.  Mirrors Q3's BotAggression()
 * (ai_dmq3.c:2197) and the Gladiator bot's combat-feasibility check.
 *
 * Q2 inventory indices (from g_items.c itemlist[]):
 *    8=shotgun  9=supershotgun 10=machinegun 11=chaingun
 *   13=grenadelauncher 14=rocketlauncher 15=hyperblaster
 *   16=railgun 17=bfg
 *   12=ammo_grenades 18=shells 19=bullets 20=cells
 *   21=rockets 22=slugs   23=item_quad
 * ==================================================================== */
static int Q2BotAggression(q2_botclient_t *bc)
{
    int *inv = bc->inventory;

    /* Very low health — avoid all combat */
    if (bc->health < 40) return 0;

    /* Quad damage — always fight */
    if (inv[23] > 0) return 100;

    /* Enemy is way higher than bot — extreme height disadvantage.
     * Mirrors Q3's BotAggression (ai_dmq3.c:2210). */
    if (bc->enemy_height > 200) return 0;

    /* Low health + no strong weapon — don't fight */
    if (bc->health < 60 &&
        inv[14] == 0 && inv[16] == 0 && inv[17] == 0)
        return 0;

    /* Low health + no armor — disengage.
     * Mirrors Q3: health<80 && armor<40 → return 0.
     * #8 — Q2 armor value comes from stats[STAT_ARMOR=5], not inventory.
     * We store it in bc->armor (populated from BotUpdateClient). */
    if (bc->health < 80) {
        if (bc->armor < 40) return 0;
    }

    /* Check from strongest to weakest weapon + sufficient ammo */
    if (inv[17] > 0 && inv[20] > 7)   return 100; /* BFG + cells */
    if (inv[16] > 0 && inv[22] > 5)   return 95;  /* Railgun + slugs */
    if (inv[14] > 0 && inv[21] > 5)   return 90;  /* Rocket Launcher + rockets */
    if (inv[15] > 0 && inv[20] > 40)  return 90;  /* Hyperblaster + cells */
    if (inv[11] > 0 && inv[19] > 50)  return 85;  /* Chaingun + bullets */
    if (inv[13] > 0 && inv[12] > 10)  return 80;  /* Grenade Launcher + grenades */
    if (inv[9]  > 0 && inv[18] > 10)  return 60;  /* Super Shotgun + shells */
    if (inv[10] > 0 && inv[19] > 30)  return 50;  /* Machinegun + bullets */
    if (inv[8]  > 0 && inv[18] > 10)  return 50;  /* Shotgun + shells */

    /* Blaster only, or weapons without ammo */
    return 0;
}

/* ====================================================================
 * Q2BotCheckGrenades — dodge grenades and rockets on the ground
 *
 * Scans all ET_MISSILE entities and places avoid spots so the AAS
 * pathfinder routes around them.  Mirrors Q3's BotCheckForGrenades
 * (ai_dmq3.c:4718) and BotCheckForProxMines.
 * ==================================================================== */
static void Q2BotCheckGrenades(q2_botclient_t *bc)
{
    int ent;
    aas_entityinfo_t entinfo;
    vec3_t diff;
    float speed;

    /* Clear previous avoid spots and rebuild from current missiles.
     *
     * Q3's BotCheckForGrenades checks state->weapon == WP_GRENADE_LAUNCHER
     * to only avoid grenades.  Q2 missiles don't carry a weapon type, so
     * we filter by VELOCITY from our adapter-side cache (computed from
     * origin deltas in Q2BotUpdateEntity).  Only SLOW projectiles (speed
     * < 300) are avoided — these are grenades bouncing on the ground.
     * Fast projectiles (rockets at 650, blaster at 1000+) are too fast
     * to dodge via AAS navigation and would flood the 32-slot array. */
    BotAddAvoidSpot(bc->movestate, vec3_origin, 0, AVOID_CLEAR);

    for (ent = AAS_NextEntity(0); ent; ent = AAS_NextEntity(ent)) {
        if (AAS_EntityType(ent) != 3 /* ET_MISSILE */) continue;
        AAS_EntityInfo(ent, &entinfo);
        if (!entinfo.valid) continue;
        /* Only nearby missiles */
        VectorSubtract(entinfo.origin, bc->origin, diff);
        if (VectorLength(diff) > 400.0f) continue;
        /* Read velocity from adapter-side cache (NOT from entinfo.velocity
         * which is always zero because Q3's aas_entityinfo_t has no
         * velocity field and AAS_UpdateEntity never copies one). */
        if (ent >= 0 && ent < Q2_MAX_ENTITIES) {
            speed = VectorLength(q2_entvelocity[ent].velocity);
        } else {
            speed = 9999.0f; /* unknown → skip */
        }
        if (speed > 300.0f) continue; /* fast projectile → don't avoid */
        BotAddAvoidSpot(bc->movestate, entinfo.origin, 120, AVOID_ALWAYS);
    }
}

/* ====================================================================
 * Q2BotCheckAir — track breathing, detect water/lava/slime hazards
 *
 * Updates lastair_time when not submerged.  The adapter currently uses
 * this for awareness only; future versions could force seek-air goals.
 * Mirrors Q3's BotCheckAir (ai_dmq3.c:5063).
 * ==================================================================== */
static void Q2BotCheckAir(q2_botclient_t *bc)
{
    int contents = q2import.PointContents(bc->origin);
    /* Q2: CONTENTS_WATER = 8, CONTENTS_SLIME = 16, CONTENTS_LAVA = 32 */
    if (!(contents & (8 | 16 | 32))) {
        bc->lastair_time = AAS_Time();
        return;
    }

    /* Submerged for > 6 seconds: push an air-seeking goal.
     * Mirrors Q3's BotGoForAir (ai_dmnet.c:156).
     * Trace upward to find the water surface, create a goal there,
     * and push it onto the goal stack so BotMoveToGoal navigates up. */
    if (bc->lastair_time < AAS_Time() - 6.0f) {
        vec3_t above;
        bsp_trace_t trace;
        int above_contents;
        VectorCopy(bc->origin, above);
        above[2] += 1000.0f;
        trace = q2import.Trace(bc->origin, NULL, NULL, above,
                               -1, 1 /* CONTENTS_SOLID */);
        /* Check if the trace endpoint is above water */
        above_contents = q2import.PointContents(trace.endpos);
        if (!(above_contents & (8 | 16 | 32))) {
            /* Found air! Push an air goal onto the goal stack. */
            bot_goal_t airgoal;
            Com_Memset(&airgoal, 0, sizeof(airgoal));
            VectorCopy(trace.endpos, airgoal.origin);
            airgoal.origin[2] -= 2.0f;  /* just inside the water surface */
            airgoal.areanum = AAS_PointAreaNum(airgoal.origin);
            airgoal.mins[0] = -8; airgoal.mins[1] = -8; airgoal.mins[2] = -8;
            airgoal.maxs[0] =  8; airgoal.maxs[1] =  8; airgoal.maxs[2] =  8;
            if (airgoal.areanum > 0) {
                BotPushGoal(bc->goalstate, &airgoal);
                bc->hasnbg = true;
            }
        }
    }
}

/* ====================================================================
 * Q2BotCheckBlocked — handle stuck bots and movement failures
 *
 * Examines moveresult.blocked and detects position stalls.  If stuck,
 * attempts random movement or sideward avoidance.  Mirrors Q3's
 * BotAIBlocked (ai_dmq3.c:4436).
 * ==================================================================== */
static void Q2BotCheckBlocked(int client, q2_botclient_t *bc,
                               bot_moveresult_t *moveresult)
{
    float now = AAS_Time();

    /* First-frame sentinel: initialize timer to current time so the stall
     * detection doesn't fire on the very first frame (when notblocked_time
     * was 0 and (now - 0) > 2.0 was always true — the original regression). */
    if (bc->notblocked_time < 0.0f) {
        bc->notblocked_time = now;
        VectorCopy(bc->origin, bc->lastorigin);
        return;
    }

    /* #8 — Enhanced blocked handling mirroring Q3's BotAIBlocked.
     * If blocked, try sideward movement and flip avoidance direction. */
    if (!moveresult->blocked) {
        bc->notblocked_time = now;
    } else if (moveresult->type == RESULTTYPE_INSOLIDAREA) {
        /* Stuck inside solid — random direction escape */
        vec3_t rdir;
        rdir[0] = (float)(rand() % 200 - 100);
        rdir[1] = (float)(rand() % 200 - 100);
        rdir[2] = 0;
        VectorNormalize(rdir);
        BotMoveInDirection(bc->movestate, rdir, 400, MOVE_WALK);
        /* Reset avoid reach to force re-evaluation of routes.
         * Mirrors Q3's trap_BotResetAvoidReach in BotAIBlocked. */
        BotResetAvoidReach(bc->movestate);
    } else if (moveresult->blocked) {
        /* Blocked by an entity — try sideward movement.
         * Mirrors Q3's BotAIBlocked sideward avoidance (ai_dmq3.c:4510). */
        vec3_t hordir, sideward, up;
        VectorSet(up, 0, 0, 1);
        hordir[0] = moveresult->movedir[0];
        hordir[1] = moveresult->movedir[1];
        hordir[2] = 0;
        if (VectorNormalize(hordir) < 0.1f) {
            float yaw = (float)(rand() % 360);
            hordir[0] = cos(DEG2RAD(yaw));
            hordir[1] = sin(DEG2RAD(yaw));
            hordir[2] = 0;
        }
        CrossProduct(hordir, up, sideward);
        if (bc->flags & BFL_AVOIDRIGHT)
            VectorNegate(sideward, sideward);
        if (!BotMoveInDirection(bc->movestate, sideward, 400, MOVE_WALK)) {
            bc->flags ^= BFL_AVOIDRIGHT;
            VectorNegate(sideward, sideward);
            BotMoveInDirection(bc->movestate, sideward, 400, MOVE_WALK);
        }
        /* After 0.4s blocked, reset goals to try another path */
        if (bc->notblocked_time < now - 0.4f) {
            bc->ltg_check_time = 0; /* force LTG re-evaluation */
        }
    }

    /* Stall detection: if bot hasn't moved > 5 units in 3 seconds, nudge.
     * Only triggers after sustained stillness, not transient pauses
     * (waiting for elevator, etc.). */
    {
        vec3_t diff;
        VectorSubtract(bc->origin, bc->lastorigin, diff);
        if (VectorLength(diff) >= 5.0f) {
            /* Bot is moving — reset stall timer */
            bc->notblocked_time = now;
        } else if ((now - bc->notblocked_time) > 3.0f) {
            /* Stuck for 3+ seconds — random nudge */
            vec3_t rdir;
            rdir[0] = (float)(rand() % 200 - 100);
            rdir[1] = (float)(rand() % 200 - 100);
            rdir[2] = 0;
            VectorNormalize(rdir);
            BotMoveInDirection(bc->movestate, rdir, 400, MOVE_WALK);
            bc->notblocked_time = now;
        }
    }
    VectorCopy(bc->origin, bc->lastorigin);
}

/* ====================================================================
 * Q2BotAttackMove — combat movement: strafing, jumping, distance mgmt
 *
 * Ported from Q3's BotAttackMove (ai_dmq3.c:2635-2765).  During
 * BATTLE_FIGHT, the bot circle-strafes, jumps, crouches, and manages
 * distance to the enemy instead of standing still.
 * ==================================================================== */
#define IDEAL_ATTACKDIST  200.0f

static void Q2BotAttackMove(int client, q2_botclient_t *bc, int enemy)
{
    aas_entityinfo_t entinfo;
    vec3_t forward, backward, sideward, hordir;
    vec3_t up = {0, 0, 1};
    float dist, strafechange_time;
    int movetype, i;

    AAS_EntityInfo(enemy, &entinfo);
    if (!entinfo.valid) return;

    /* Direction and distance to enemy */
    VectorSubtract(entinfo.origin, bc->origin, forward);
    dist = VectorNormalize(forward);
    VectorNegate(forward, backward);

    /* Movement type: walk, jump, or crouch.
     * Uses personality traits (jumper, croucher) from character file.
     * Mirrors Q3's BotAttackMove (ai_dmq3.c:2665-2690). */
    movetype = MOVE_WALK;
    if (bc->attackjump_time < AAS_Time()) {
        if ((float)(rand() & 0x7FFF) / 0x7FFF < bc->jumper * 0.1f) {
            movetype = MOVE_JUMP;
            bc->attackjump_time = AAS_Time() + 1.0f;
        }
    }
    if (movetype != MOVE_JUMP && bc->attackcrouch_time < AAS_Time()) {
        if ((float)(rand() & 0x7FFF) / 0x7FFF < bc->croucher * 0.1f) {
            movetype = MOVE_CROUCH;
            bc->attackcrouch_time = AAS_Time() + 1.0f;
        }
    }

    /* Randomly back away 10% of the time to be less predictable.
     * Mirrors Q3 ai_dmq3.c:2694-2700. */
    if ((float)(rand() & 0x7FFF) / 0x7FFF < 0.1f) {
        BotMoveInDirection(bc->movestate, backward, 400, movetype);
        return;
    }

    /* Low-skill bots: just walk toward/away from enemy */
    if (bc->attack_skill <= 0.4f) {
        if (dist > IDEAL_ATTACKDIST + 40.0f)
            BotMoveInDirection(bc->movestate, forward, 400, movetype);
        else if (dist < IDEAL_ATTACKDIST - 40.0f)
            BotMoveInDirection(bc->movestate, backward, 400, movetype);
        return;
    }

    /* Skilled bots: circle-strafe */
    bc->attackstrafe_time += bc->enemy_time > 0 ? 0.1f : 0.0f; /* ~thinktime */
    strafechange_time = 0.4f + (1.0f - bc->attack_skill) * 0.2f;

    if (bc->attackstrafe_time > strafechange_time) {
        if ((float)(rand() & 0x7FFF) / 0x7FFF > 0.935f) {
            bc->flags ^= BFL_STRAFERIGHT;
            bc->attackstrafe_time = 0;
        }
    }

    for (i = 0; i < 2; i++) {
        hordir[0] = forward[0];
        hordir[1] = forward[1];
        hordir[2] = 0;
        VectorNormalize(hordir);
        CrossProduct(hordir, up, sideward);
        if (bc->flags & BFL_STRAFERIGHT)
            VectorNegate(sideward, sideward);

        /* Mix in forward/backward to maintain ideal distance */
        if (dist > IDEAL_ATTACKDIST + 40.0f)
            VectorAdd(sideward, forward, sideward);
        else if (dist < IDEAL_ATTACKDIST - 40.0f)
            VectorAdd(sideward, backward, sideward);

        VectorNormalize(sideward);
        if (BotMoveInDirection(bc->movestate, sideward, 400, movetype))
            return;

        /* Movement failed — flip strafe direction and retry */
        bc->flags ^= BFL_STRAFERIGHT;
        bc->attackstrafe_time = 0;
    }
}

/* ====================================================================
 * Q2BotCheckPowerups — detect powerup pickup, boost aggression
 *
 * Compares current inventory with previous frame.  If Quad or
 * Invulnerability was just picked up, force transition to BATTLE_FIGHT.
 * Mirrors Q3's BotCheckItemPickup (ai_dmq3.c:1622).
 * ==================================================================== */
static void Q2BotCheckPowerups(q2_botclient_t *bc)
{
    /* Quad Damage just picked up → go aggressive */
    if (bc->inventory[23] > 0 && bc->prev_quad == 0) {
        if (bc->enemy >= 0)
            bc->aistate = Q2AI_BATTLE_FIGHT;
    }
    /* Invulnerability just picked up */
    if (bc->inventory[24] > 0 && bc->prev_invuln == 0) {
        if (bc->enemy >= 0)
            bc->aistate = Q2AI_BATTLE_FIGHT;
    }
    bc->prev_quad   = bc->inventory[23];
    bc->prev_invuln = bc->inventory[24];
}

/*
 * Q2BotFindEnemy — scan for nearest visible enemy player
 *
 * Returns entity number (1-indexed) or -1 if no enemy visible.
 * Always records enemy distance in bc->enemy_hdist/enemy_height
 * for range-aware weapon selection, even if the bot won't fight.
 *
 * Mirrors Q3's BotFindEnemy (ai_dmq3.c:2935) with:
 * - Alertness-scaled detection range (900 + alertness * 4000)
 * - FOV-limited detection: normally 90°, expands to 360° when
 *   recently damaged or enemy is shooting
 * - Spawn protection: skip enemies near recent teleport for 3s
 * - Dead player filtering (solid==0)
 */
static int Q2BotFindEnemy(int client, q2_botclient_t *bc, vec3_t bot_eye)
{
    int maxcl = (int)LibVarGetValue("maxclients");
    int best = -1;
    float best_dist_sq = 99999999.0f;
    aas_entityinfo_t best_info;
    int i;
    qboolean healthdecrease;
    float max_range;

    /* Detect health decrease since last frame (damage awareness).
     * Mirrors Q3's healthdecrease = bs->lasthealth > bs->inventory[HEALTH]. */
    healthdecrease = (bc->lasthealth > bc->health);
    bc->lasthealth = bc->health;

    /* Alertness-scaled detection range.
     * Q3: squaredist > Square(900.0 + alertness * 4000.0) → skip.
     * Range varies from 900 (alertness=0) to 4900 (alertness=1). */
    max_range = 900.0f + bc->alertness * 4000.0f;

    for (i = 1; i <= maxcl; i++) {
        aas_entityinfo_t entinfo;
        vec3_t enemy_center, dir, angles;
        bsp_trace_t los;
        float dist_sq, fov;

        if (i == client + 1) continue;
        AAS_EntityInfo(i, &entinfo);
        if (!entinfo.valid || entinfo.type != 1) continue;
        /* Skip dead players (solid==0 = SOLID_NOT) */
        if (entinfo.solid == 0) continue;

        /* #12 — Spawn protection: skip enemies near a recent teleport
         * destination for 3 seconds.  Mirrors Q3 ai_dmq3.c:2985.
         * We use the bot's own teleport_time as a proxy — in Q2 we
         * can't track other players' teleport events, but we can
         * avoid targeting freshly-teleported bots. */
        if (bc->teleport_time > AAS_Time() - 3.0f) {
            vec3_t tdiff;
            VectorSubtract(entinfo.origin, bc->origin, tdiff);
            if (VectorLengthSquared(tdiff) < 70.0f * 70.0f) continue;
        }

        /* Distance check (squared for efficiency) */
        VectorSubtract(entinfo.origin, bc->origin, dir);
        dist_sq = VectorLengthSquared(dir);
        if (dist_sq > max_range * max_range) continue;

        /* Skip if farther than current best (find nearest) */
        if (best >= 0 && dist_sq > best_dist_sq) continue;

        /* #5 — FOV-limited detection.
         * Q3: if health decreased or enemy is shooting → 360° FOV.
         * Otherwise, scale FOV based on distance:
         *   ~90° at close range, scaling up with distance.
         * Q3 formula: f = 90 + 90 - (90 - dist²/(810*9))
         * We simplify: if damaged, 360°; otherwise 90° base + dist scale */
        if (healthdecrease) {
            fov = 360.0f;
        } else {
            float d = dist_sq > (810.0f * 810.0f) ? (810.0f * 810.0f) : dist_sq;
            fov = 90.0f + 90.0f - (90.0f - d / (810.0f * 9.0f));
            if (fov > 360.0f) fov = 360.0f;
            if (fov < 90.0f)  fov = 90.0f;
        }

        /* Check if enemy is within our field of vision */
        {
            vec3_t toenemy;
            float enemy_yaw, view_yaw, diff_yaw;
            VectorSubtract(entinfo.origin, bc->origin, toenemy);
            enemy_yaw = atan2(toenemy[1], toenemy[0]) * 180.0f / M_PI;
            view_yaw  = bc->viewangles[1]; /* YAW */
            diff_yaw  = enemy_yaw - view_yaw;
            /* Normalize to [-180, 180] */
            while (diff_yaw > 180.0f)  diff_yaw -= 360.0f;
            while (diff_yaw < -180.0f) diff_yaw += 360.0f;
            if (fabs(diff_yaw) > fov * 0.5f) continue;
        }

        /* LOS trace */
        VectorCopy(entinfo.origin, enemy_center);
        enemy_center[2] += 22.0f;
        los = q2import.Trace(bot_eye, NULL, NULL, enemy_center,
                             client + 1, 1 /* CONTENTS_SOLID */);
        if (los.fraction < 1.0f) continue;

        /* This enemy is closer and visible — record as best */
        best_dist_sq = dist_sq;
        best = i;
        Com_Memcpy(&best_info, &entinfo, sizeof(best_info));
    }

    if (best >= 0) {
        vec3_t hdiff;
        hdiff[0] = best_info.origin[0] - bc->origin[0];
        hdiff[1] = best_info.origin[1] - bc->origin[1];
        hdiff[2] = 0.0f;
        bc->enemy_hdist  = (int)VectorLength(hdiff);
        bc->enemy_height = (int)(best_info.origin[2] - bc->origin[2]);
    } else {
        bc->enemy_hdist  = 9999;
        bc->enemy_height = 0;
    }
    return best;
}

/*
 * Q2BotAimAndFire — aim at enemy with prediction and fire with safety checks
 *
 * Combines Q3's BotAimAtEnemy (predictive aiming, skill-based accuracy)
 * and BotCheckAttack (reaction time, fire throttle, LOS check).
 *
 * Prediction tiers (mirrors Q3 ai_dmq3.c:3271):
 *   aim_skill > 0.4: linear leading (simple velocity * flight_time)
 *   aim_skill > 0.6: splash weapons aim at ground near enemy
 *   Accuracy scatter applied based on aim_accuracy characteristic.
 *
 * Fire gating (mirrors Q3 ai_dmq3.c:3565):
 *   1. Reaction time delay after first spotting enemy
 *   2. Fire throttle: random fire/pause pattern based on firethrottle trait
 *   3. LOS check: don't fire into walls
 *   4. Splash safety: don't fire RL/GL at point blank
 */
static void Q2BotAimAndFire(int client, int enemy, vec3_t bot_eye)
{
    q2_botclient_t *bc = &q2clients[client];
    aas_entityinfo_t entinfo;
    vec3_t bestorigin, dir, aim_angles;
    bsp_trace_t trace;
    float dist, accuracy;
    weaponinfo_t wi;

    AAS_EntityInfo(enemy, &entinfo);
    if (!entinfo.valid) return;

    /* --- Aim prediction (mirrors Q3's BotAimAtEnemy) --- */
    VectorCopy(entinfo.origin, bestorigin);
    bestorigin[2] += 8.0f;
    accuracy = bc->aim_accuracy;
    if (accuracy <= 0.0f) accuracy = 0.0001f;

    /* Cache enemy velocity every 0.5s for prediction.
     * Q3: enemyvelocity = (origin - lastvisorigin) / update_time */
    if (bc->enemyposition_time < AAS_Time()) {
        vec3_t evel;
        if (entinfo.update_time > 0.001f) {
            VectorSubtract(entinfo.origin, entinfo.lastvisorigin, evel);
            VectorScale(evel, 1.0f / entinfo.update_time, evel);
        } else {
            VectorClear(evel);
        }
        /* If enemy changed direction, reduce accuracy (Q3 ai_dmq3.c:3388) */
        if (bc->aim_skill < 0.9f) {
            if (DotProduct(bc->enemyvelocity, evel) < 0)
                accuracy *= 0.7f;
        }
        VectorCopy(evel, bc->enemyvelocity);
        VectorCopy(entinfo.origin, bc->enemyorigin);
        bc->enemyposition_time = AAS_Time() + 0.5f;
    }

    /* Get current weapon info for projectile speed */
    BotGetWeaponInfo(bc->weaponstate, bc->best_weapon_num, &wi);

    /* Projectile leading: only for non-instant weapons (wi.speed > 0).
     * Mirrors Q3's linear prediction for aim_skill > 0.4. */
    if (wi.speed > 0 && bc->aim_skill > 0.4f) {
        VectorSubtract(entinfo.origin, bc->origin, dir);
        dist = VectorLength(dir);
        /* Linear prediction: aim at origin + velocity * (dist / projectile_speed) */
        {
            vec3_t evel;
            float flight_time = dist / wi.speed;
            VectorSubtract(entinfo.origin, entinfo.lastvisorigin, evel);
            evel[2] = 0; /* don't predict vertical (Q3 strips Z for linear) */
            if (entinfo.update_time > 0.001f)
                VectorScale(evel, 1.0f / entinfo.update_time, evel);
            else
                VectorClear(evel);
            VectorMA(entinfo.origin, flight_time * VectorLength(evel),
                     evel, bestorigin);
            bestorigin[2] = entinfo.origin[2] + 8.0f;
        }
    }

    /* Splash weapons: aim at ground if enemy is at same height or below.
     * Mirrors Q3 ai_dmq3.c:3444 (aim_skill > 0.6 && radial damage). */
    if (bc->aim_skill > 0.6f &&
        (bc->best_weapon_num == 8 /* RL */ || bc->best_weapon_num == 7 /* GL */)) {
        if (entinfo.origin[2] < bc->origin[2] + 16.0f) {
            vec3_t ground_end;
            bsp_trace_t gtrace;
            VectorCopy(entinfo.origin, ground_end);
            ground_end[2] -= 64.0f;
            gtrace = q2import.Trace(entinfo.origin, NULL, NULL, ground_end,
                                     enemy, 1);
            if (!gtrace.startsolid) {
                vec3_t gcheck;
                VectorSubtract(gtrace.endpos, bot_eye, gcheck);
                /* Only aim at ground if far enough from bot (avoid splash) */
                if (VectorLengthSquared(gcheck) > 100.0f * 100.0f) {
                    bestorigin[2] = gtrace.endpos[2] - 8.0f;
                }
            }
        }
    }

    /* Apply accuracy scatter (Q3 ai_dmq3.c:3500) */
    bestorigin[0] += 20.0f * ((float)(rand() & 0x7FFF) / 0x7FFF * 2.0f - 1.0f) * (1.0f - accuracy);
    bestorigin[1] += 20.0f * ((float)(rand() & 0x7FFF) / 0x7FFF * 2.0f - 1.0f) * (1.0f - accuracy);
    bestorigin[2] += 10.0f * ((float)(rand() & 0x7FFF) / 0x7FFF * 2.0f - 1.0f) * (1.0f - accuracy);

    /* Compute final aim direction and view angles */
    VectorSubtract(bestorigin, bot_eye, dir);

    /* For hitscan weapons, scale accuracy with distance (Q3 ai_dmq3.c:3509) */
    if (wi.speed == 0) {
        dist = VectorLength(dir);
        if (dist > 150.0f) dist = 150.0f;
        accuracy *= (0.6f + dist / 150.0f * 0.4f);
    }

    /* Add extra scatter for low accuracy bots (Q3 ai_dmq3.c:3515) */
    if (accuracy < 0.8f) {
        VectorNormalize(dir);
        dir[0] += 0.3f * ((float)(rand() & 0x7FFF) / 0x7FFF * 2.0f - 1.0f) * (1.0f - accuracy);
        dir[1] += 0.3f * ((float)(rand() & 0x7FFF) / 0x7FFF * 2.0f - 1.0f) * (1.0f - accuracy);
        dir[2] += 0.3f * ((float)(rand() & 0x7FFF) / 0x7FFF * 2.0f - 1.0f) * (1.0f - accuracy);
    }

    Vector2Angles(dir, aim_angles);
    EA_View(client, aim_angles);

    /* --- Fire gating (mirrors Q3's BotCheckAttack) --- */

    /* #2 — Reaction time: don't fire until reactiontime seconds after
     * first spotting this enemy.  Mirrors Q3 ai_dmq3.c:3590. */
    if (bc->enemysight_time > AAS_Time() - bc->reactiontime) return;
    if (bc->teleport_time > AAS_Time() - bc->reactiontime) return;

    /* Fire throttle: creates realistic fire-pause patterns.
     * Mirrors Q3 ai_dmq3.c:3594-3604. */
    if (bc->firethrottlewait_time > AAS_Time()) return;
    if (bc->firethrottleshoot_time < AAS_Time()) {
        if ((float)(rand() & 0x7FFF) / 0x7FFF > bc->firethrottle) {
            bc->firethrottlewait_time = AAS_Time() + bc->firethrottle;
            bc->firethrottleshoot_time = 0;
        } else {
            bc->firethrottleshoot_time = AAS_Time() + 1.0f - bc->firethrottle;
            bc->firethrottlewait_time = 0;
        }
    }

    /* Pre-fire LOS check: trace from eye to aim target */
    trace = q2import.Trace(bot_eye, NULL, NULL, bestorigin,
                           client + 1, 1 /* CONTENTS_SOLID */);
    if (trace.fraction < 1.0f) return; /* wall in the way */

    /* Splash weapon safety at point blank */
    {
        vec3_t diff;
        float enemy_dist;
        VectorSubtract(entinfo.origin, bot_eye, diff);
        enemy_dist = VectorLength(diff);
        if (enemy_dist < 100.0f) {
            if (bc->best_weapon_num == 8 || bc->best_weapon_num == 7 ||
                bc->best_weapon_num == 6) {
                return;
            }
        }
    }

    EA_Attack(client);
    bc->flags ^= BFL_ATTACKED;
}

/*
 * Q2BotNavigateGoals — manage goal stack, navigate toward active goal
 *
 * Returns true if the bot is actively navigating (has a goal and
 * BotMoveToGoal was called).
 */
static qboolean Q2BotNavigateGoals(int client, q2_botclient_t *bc,
                                    vec3_t bot_eye,
                                    bot_moveresult_t *moveresult)
{
    float now = AAS_Time();

    /* Timeout: abandon goals that haven't been reached in 60s */
    if (bc->hasgoal && bc->goal_set_time > 0.0f &&
        (now - bc->goal_set_time) > 60.0f)
    {
        if (bc->hasnbg) { BotPopGoal(bc->goalstate); bc->hasnbg = false; }
        BotPopGoal(bc->goalstate);
        bc->hasgoal       = false;
        bc->ltg_check_time = 0.0f;
    }

    /* LTG: pick a goal when we don't have one (throttled to 0.5s). */
    if (!bc->hasgoal && (now - bc->ltg_check_time) >= 0.5f) {
        bc->ltg_check_time = now;
        /* Try goal selection with the bot's actual origin first.
         * If that fails (spawn point in bad AAS area), retry with
         * the origin nudged upward by 18 units (Q2 STEPSIZE).
         * Many Q2 spawn points are a few units inside the floor
         * geometry, putting the bot below the AAS ground plane.
         * A small upward offset often lands in a valid area. */
        if (!BotChooseLTGItem(bc->goalstate, bc->origin,
                               bc->inventory, TFL_DEFAULT)) {
            vec3_t nudged;
            VectorCopy(bc->origin, nudged);
            nudged[2] += 18.0f;
            BotChooseLTGItem(bc->goalstate, nudged,
                              bc->inventory, TFL_DEFAULT);
        }
        if (BotGetTopGoal(bc->goalstate, &bc->ltg)) {
            bc->hasgoal      = true;
            bc->goal_set_time = now;
            if (LibVarGetValue("bot_developer")) {
                char goalname[64];
                BotGoalName(bc->ltg.number, goalname, sizeof(goalname));
                botimport.Print(PRT_MESSAGE,
                    "bot %d: LTG '%s' area %d (weapons: sg=%d ssg=%d mg=%d "
                    "cg=%d gl=%d rl=%d hb=%d rg=%d bfg=%d)\n",
                    client, goalname, bc->ltg.areanum,
                    bc->inventory[8], bc->inventory[9], bc->inventory[10],
                    bc->inventory[11], bc->inventory[13], bc->inventory[14],
                    bc->inventory[15], bc->inventory[16], bc->inventory[17]);
            }
        } else if (LibVarGetValue("bot_developer")) {
            botimport.Print(PRT_MESSAGE,
                "bot %d: BotChooseLTGItem returned false (no goal found)\n",
                client);
        }
    }

    /* NBG: opportunistically divert to a nearby item (throttled to 0.5s). */
    if (bc->hasgoal && !bc->hasnbg && (now - bc->nbg_check_time) >= 0.5f) {
        bot_goal_t *ltg_ptr = (bc->ltg.flags & GFL_ROAM) ? NULL : &bc->ltg;
        bc->nbg_check_time = now;
        if (BotChooseNBGItem(bc->goalstate, bc->origin, bc->inventory,
                              TFL_DEFAULT, ltg_ptr, 400)) {
            bc->hasnbg = true;
        }
    }

    /* Navigate toward the current active goal (top of stack).
     * If no goal could be found (bad AAS area at spawn point, all items
     * unreachable, etc.), walk in a random direction.  This moves the bot
     * out of a "dead" AAS area and into one with valid reachability links,
     * so the next BotChooseLTGItem attempt (0.5s later) can succeed. */
    Com_Memset(moveresult, 0, sizeof(*moveresult));
    if (bc->hasgoal) {
        bot_goal_t active_goal;
        if (BotGetTopGoal(bc->goalstate, &active_goal)) {
            BotMoveToGoal(moveresult, bc->movestate, &active_goal, TFL_DEFAULT);

            /* BotMoveToGoal can fail to produce movement in several ways:
             * 1. Explicit failure (RESULTTYPE_INSOLIDAREA): bot in bad AAS area
             * 2. Silent no-op: valid area but no route found to goal
             * 3. Waiting for elevator (MOVERESULT_WAITING): this is OK
             * 4. Brief pause between reachability edges: normal, 1-2 frames
             *
             * Only trigger roaming after SUSTAINED failure (1+ second of
             * zero movedir).  Single-frame pauses (case 4) are normal
             * during route computation and shouldn't trigger roaming. */
            if (!(moveresult->flags & MOVERESULT_WAITING) &&
                moveresult->movedir[0] == 0.0f &&
                moveresult->movedir[1] == 0.0f &&
                moveresult->movedir[2] == 0.0f)
            {
                float now2 = AAS_Time();
                if (bc->move_fail_time == 0.0f)
                    bc->move_fail_time = now2;
                if ((now2 - bc->move_fail_time) > 1.0f)
                    goto roam_fallback;
            } else {
                bc->move_fail_time = 0.0f;  /* movement succeeded, reset */
            }

            /* #14 — Weapon jump gating: only allow rocket jump if
             * conditions are met.  Mirrors Q3's BotCanAndWantsToRocketJump
             * (ai_dmq3.c:2385): RL + 3 rockets, health>=60,
             * no Quad, weaponjumping>=0.5. */
            if (moveresult->flags & MOVERESULT_MOVEMENTWEAPON) {
                qboolean allow_jump = true;
                /* Check if this is a rocket jump (weapon 8 = RL in weapons.c) */
                if (moveresult->weapon == 8) {
                    if (bc->inventory[14] <= 0)      allow_jump = false; /* no RL */
                    if (bc->inventory[21] < 3)        allow_jump = false; /* low rockets */
                    if (bc->inventory[23] > 0)        allow_jump = false; /* has Quad */
                    if (bc->health < 60)              allow_jump = false;
                    if (bc->health < 90 && bc->armor < 40)
                                                      allow_jump = false;
                    if (bc->weaponjumping < 0.5f)     allow_jump = false;
                }
                if (allow_jump) {
                    weaponinfo_t jump_wi;
                    BotGetWeaponInfo(bc->weaponstate, moveresult->weapon, &jump_wi);
                    if (jump_wi.name[0]) {
                        q2import.BotClientCommand(client, "use", jump_wi.name, NULL);
                        bc->best_weapon_num = moveresult->weapon;
                    }
                }
            }

            /* Detect item gone */
            if (BotItemGoalInVisButNotVisible(client, bot_eye,
                                               bc->viewangles, &active_goal)) {
                BotSetAvoidGoalTime(bc->goalstate, active_goal.number, 30.0f);
                BotPopGoal(bc->goalstate);
                if (bc->hasnbg) {
                    bc->hasnbg = false;
                } else {
                    bc->hasgoal       = false;
                    bc->ltg_check_time = 0.0f;
                }
            }
            /* Goal reached — pop and keep going */
            else if (BotTouchingGoal(bc->origin, &active_goal)) {
                BotPopGoal(bc->goalstate);
                if (bc->hasnbg) {
                    bc->hasnbg = false;
                } else {
                    bc->hasgoal       = false;
                    bc->ltg_check_time = 0.0f;
                }
            }
            return true;
        }
    }

roam_fallback:
    /* No goal found OR BotMoveToGoal failed (RESULTTYPE_INSOLIDAREA) —
     * area (common in Q2 maps where spawn positions are slightly inside
     * geometry).  Walk in a persistent random direction to escape the
     * dead zone.  The direction is held for 2 seconds so the bot makes
     * actual progress (a new random dir every frame just jitters in
     * place with zero net displacement).  Mirrors Q3's BotRoamGoal
     * which picks a random target and sticks with it. */
    {
        float now = AAS_Time();
        if (now - bc->roam_dir_time > 2.0f) {
            float yaw = (float)(rand() % 360);
            bc->roam_dir[0] = cos(DEG2RAD(yaw));
            bc->roam_dir[1] = sin(DEG2RAD(yaw));
            bc->roam_dir[2] = 0;
            bc->roam_dir_time = now;
        }
        EA_Move(client, bc->roam_dir, 400);
        EA_Action(client, ACTION_MOVEFORWARD);
        if (LibVarGetValue("bot_developer")) {
            static float last_roam_log = 0;
            if (now - last_roam_log > 1.0f) {
                botimport.Print(PRT_MESSAGE,
                    "bot %d: ROAMING (bad AAS area) dir=(%.1f,%.1f) area=%d origin=(%.0f,%.0f,%.0f)\n",
                    client, bc->roam_dir[0], bc->roam_dir[1],
                    BotReachabilityArea(bc->origin, client),
                    bc->origin[0], bc->origin[1], bc->origin[2]);
                last_roam_log = now;
            }
        }
    }
    return false;
}

/*
 * Q2BotAI  —  per-bot, per-frame AI entry point
 *
 * State-machine AI modelled after Q3's AINode system (ai_dmnet.c) and
 * the original Gladiator bot's internal state machine:
 *
 *   SEEK_LTG / SEEK_NBG:  Navigate toward item goals.  No combat.
 *     → enemy found + aggression >= 50 → BATTLE_FIGHT
 *     → enemy found + aggression <  50 → BATTLE_RETREAT
 *
 *   BATTLE_FIGHT:  Dedicated combat.  Pause navigation, aim + fire.
 *     → enemy lost                     → SEEK_LTG
 *     → aggression drops < 50          → BATTLE_RETREAT
 *
 *   BATTLE_RETREAT:  Navigate toward goals while defending.
 *     → enemy lost                     → SEEK_LTG
 *     → aggression rises >= 50         → BATTLE_FIGHT
 */
static int Q2BotAI(int client, float thinktime)
{
    q2_botclient_t   *bc;
    bot_initmove_t    initmove;
    bot_moveresult_t  moveresult;
    bot_input_t       q3input;
    q2_bot_input_t    q2input;
    qboolean          in_combat = false;

    if (client < 0 || client >= Q2_BOTLIB_MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    bc = &q2clients[client];
    if (!bc->inuse) return Q2_BLERR_AICLIENTNOTSETUP;

    /* --- Dead bot: press attack to respawn --- */
    if (bc->pm_type == Q2PM_DEAD || bc->pm_type == Q2PM_GIB) {
        q2_bot_input_t respawn;
        Com_Memset(&respawn, 0, sizeof(respawn));
        respawn.thinktime   = thinktime;
        respawn.actionflags = Q2_ACTION_RESPAWN;
        /* Botlib state reset on death — clear stale navigation and
         * goals but preserve critical persistent fields.
         *
         * BotEmptyGoalStack: clears the goal stack (all entries).
         * BotResetAvoidGoals: clears item avoidance timers.
         * BotResetMoveState: zeroes the move state — clears stale
         *   reachability chains and cached areas that accumulate
         *   across deaths and prevent navigation.
         *
         * We do NOT use BotResetGoalState here because it zeroes
         * gs->lastreachabilityarea.  BotChooseLTGItem uses that as
         * a fallback when the bot's spawn point doesn't map to a
         * valid AAS area (common in Q2 maps where spawn points are
         * slightly inside geometry).  Without the fallback, the bot
         * can't compute travel times → no goals → stuck at spawn. */
        BotEmptyGoalStack(bc->goalstate);
        BotResetAvoidGoals(bc->goalstate);
        BotResetMoveState(bc->movestate);
        bc->hasgoal         = false;
        bc->hasnbg          = false;
        bc->ltg_check_time  = 0.0f;
        bc->nbg_check_time  = 0.0f;
        bc->goal_set_time   = 0.0f;
        bc->best_weapon_num = 0;
        bc->aistate         = Q2AI_SEEK_LTG;
        bc->enemy           = -1;
        bc->enemy_hdist     = 9999;
        bc->enemy_height    = 0;
        bc->chase_time      = 0;
        bc->lastenemyareanum= 0;
        bc->notblocked_time = -1.0f;
        q2import.BotInput(client, &respawn);
        EA_ResetInput(client);
        return Q2_BLERR_NOERROR;
    }

    /* --- Initialise move state --- */
    Com_Memset(&initmove, 0, sizeof(initmove));
    VectorCopy(bc->origin,     initmove.origin);
    VectorCopy(bc->velocity,   initmove.velocity);
    VectorCopy(bc->viewoffset, initmove.viewoffset);
    VectorCopy(bc->viewangles, initmove.viewangles);
    initmove.entitynum    = client + 1;
    initmove.client       = client;
    initmove.thinktime    = thinktime;
    initmove.presencetype = (bc->pm_flags & 1) ? PRESENCE_CROUCH : PRESENCE_NORMAL;
    if (bc->pm_flags & 4) initmove.or_moveflags |= MFL_ONGROUND;
    /* Swimming detection: Q2 has no PMF_SWIMMING flag.  Check PointContents
     * instead.  Without MFL_SWIMMING, BotTravel_Swim and water-based
     * reachabilities won't execute — the bot won't dive for the railgun
     * in q2dm1 or navigate through any water tunnel.
     * Q2: CONTENTS_WATER = 8, CONTENTS_SLIME = 16, CONTENTS_LAVA = 32 */
    {
        int contents = q2import.PointContents(bc->origin);
        if (contents & (8 | 16 | 32))
            initmove.or_moveflags |= MFL_SWIMMING;
    }
    /* Teleport detection (mirrors Q3's BotSetTeleportTime, ai_dmq3.c:2060) */
    if (bc->pm_flags & 32 /* PMF_TIME_TELEPORT */) {
        initmove.or_moveflags |= MFL_TELEPORTED;
        bc->enemy         = -1;
        bc->chase_time    = 0;
        bc->teleport_time = AAS_Time(); /* #12: spawn protection timer */
        bc->aistate       = Q2AI_SEEK_LTG;
    }
    /* Waterjump detection */
    if (bc->pm_flags & 8 /* PMF_TIME_WATERJUMP */)
        initmove.or_moveflags |= MFL_WATERJUMP;
    BotInitMoveState(bc->movestate, &initmove);

    /* --- Hazard awareness (before any navigation/combat) --- */
    Q2BotCheckGrenades(bc);
    Q2BotCheckAir(bc);
    Q2BotCheckPowerups(bc);

    /* --- Weapon selection --- */
    bc->inventory[200] = bc->enemy_hdist;
    bc->inventory[201] = bc->enemy_height;
    {
        int best = BotChooseBestFightWeapon(bc->weaponstate, bc->inventory);
        if (best > 0 && best != bc->best_weapon_num) {
            weaponinfo_t wi;
            BotGetWeaponInfo(bc->weaponstate, best, &wi);
            if (wi.name[0])
                q2import.BotClientCommand(client, "use", wi.name, NULL);
            bc->best_weapon_num = best;
        }
    }

    /* --- Enemy scan + state transitions + per-state behavior --- */
    {
        vec3_t bot_eye;
        int    vis_enemy;
        int    aggression;
        float  now = AAS_Time();

        VectorAdd(bc->origin, bc->viewoffset, bot_eye);
        vis_enemy  = Q2BotFindEnemy(client, bc, bot_eye);
        aggression = Q2BotAggression(bc);

        /* --- Dead enemy detection (mirrors Q3's EntityIsDead) ---
         * If our current enemy has died (solid==0 in AAS), immediately
         * clear it and return to seeking.  Q3 uses a 1-second grace
         * period for chat; we skip that since Q2 bots don't chat. */
        if (bc->enemy >= 0) {
            aas_entityinfo_t einfo;
            AAS_EntityInfo(bc->enemy, &einfo);
            if (!einfo.valid || einfo.solid == 0) {
                bc->enemy      = -1;
                bc->chase_time = 0;
                bc->aistate    = Q2AI_SEEK_LTG;
            }
        }

        /* --- State transitions --- */
        switch (bc->aistate) {
        case Q2AI_SEEK_LTG:
        case Q2AI_SEEK_NBG:
            if (vis_enemy >= 0) {
                int areanum;
                aas_entityinfo_t einfo;
                AAS_EntityInfo(vis_enemy, &einfo);
                areanum = AAS_PointAreaNum(einfo.origin);
                if (areanum > 0) {
                    VectorCopy(einfo.origin, bc->lastenemyorigin);
                    bc->lastenemyareanum = areanum;
                }
                bc->enemy          = vis_enemy;
                bc->enemy_time     = now;
                bc->enemysight_time = now; /* #2: reaction timer starts */
                bc->enemyposition_time = 0; /* force velocity cache update */
                if (aggression >= 50) {
                    bc->aistate = Q2AI_BATTLE_FIGHT;
                } else {
                    bc->aistate = Q2AI_BATTLE_RETREAT;
                }
            }
            break;

        case Q2AI_BATTLE_FIGHT:
            if (vis_enemy >= 0) {
                /* Enemy visible: update last-known position for chase.
                 * Mirrors Q3 ai_dmnet.c:2068 — only update if in a
                 * valid reachable AAS area. */
                int areanum;
                aas_entityinfo_t einfo;
                AAS_EntityInfo(vis_enemy, &einfo);
                areanum = AAS_PointAreaNum(einfo.origin);
                if (areanum > 0) {
                    VectorCopy(einfo.origin, bc->lastenemyorigin);
                    bc->lastenemyareanum = areanum;
                }
                bc->enemy      = vis_enemy;
                bc->enemy_time = now;
                if (aggression < 50)
                    bc->aistate = Q2AI_BATTLE_RETREAT;
            } else {
                /* LOS lost — chase or give up based on aggression.
                 * Mirrors Q3's BotWantsToChase (ai_dmq3.c:2322):
                 * only chase if aggression > 50 (well-armed bot). */
                if (aggression > 50 && bc->lastenemyareanum > 0) {
                    bc->aistate    = Q2AI_BATTLE_CHASE;
                    bc->chase_time = now;
                } else {
                    bc->aistate = Q2AI_SEEK_LTG;
                    bc->enemy   = -1;
                }
            }
            break;

        case Q2AI_BATTLE_RETREAT:
            if (vis_enemy >= 0) {
                int areanum;
                aas_entityinfo_t einfo;
                AAS_EntityInfo(vis_enemy, &einfo);
                areanum = AAS_PointAreaNum(einfo.origin);
                if (areanum > 0) {
                    VectorCopy(einfo.origin, bc->lastenemyorigin);
                    bc->lastenemyareanum = areanum;
                }
                bc->enemy      = vis_enemy;
                bc->enemy_time = now;
                /* If picked up enough items to fight, switch.
                 * Mirrors Q3's BotWantsToChase check in AINode_Battle_Retreat. */
                if (aggression >= 50)
                    bc->aistate = Q2AI_BATTLE_FIGHT;
            } else if ((now - bc->enemy_time) > 4.0f) {
                /* Q3 uses 4 seconds visibility loss in retreat (not 2).
                 * ai_dmnet.c:2410: enemyvisible_time < FloatTime() - 4 */
                bc->aistate = Q2AI_SEEK_LTG;
                bc->enemy   = -1;
            }
            break;

        case Q2AI_BATTLE_NBG:
            /* Picking up a nearby item during combat.
             * Mirrors Q3's AINode_Battle_NBG (ai_dmnet.c:2491).
             * When the NBG is reached/timed out, return to retreat or fight. */
            if (vis_enemy >= 0) {
                bc->enemy      = vis_enemy;
                bc->enemy_time = now;
            }
            if (bc->nbg_combat_time < now) {
                /* Time's up — pop NBG and return to combat */
                if (bc->hasnbg) {
                    BotPopGoal(bc->goalstate);
                    bc->hasnbg = false;
                }
                bc->aistate = (aggression >= 50)
                              ? Q2AI_BATTLE_FIGHT : Q2AI_BATTLE_RETREAT;
            }
            break;

        case Q2AI_BATTLE_CHASE:
            /* Pursuing enemy's last known position.
             * Mirrors Q3's AINode_Battle_Chase (ai_dmnet.c:2163). */
            if (vis_enemy >= 0) {
                /* Re-acquired LOS — back to fight */
                bc->enemy      = vis_enemy;
                bc->enemy_time = now;
                bc->chase_time = 0;
                bc->aistate    = (aggression >= 50)
                                 ? Q2AI_BATTLE_FIGHT : Q2AI_BATTLE_RETREAT;
            } else if ((now - bc->chase_time) > 10.0f) {
                /* Chase timeout (Q3 uses 10 seconds) */
                bc->aistate    = Q2AI_SEEK_LTG;
                bc->enemy      = -1;
                bc->chase_time = 0;
            } else if (aggression < 50) {
                /* Took damage during chase, no longer well-armed enough.
                 * Mirrors Q3's BotWantsToRetreat check at the end of
                 * AINode_Battle_Chase (ai_dmnet.c:2280). */
                bc->aistate    = Q2AI_BATTLE_RETREAT;
                bc->chase_time = 0;
            }
            break;
        }

        /* --- Per-state behavior --- */
        Com_Memset(&moveresult, 0, sizeof(moveresult));

        switch (bc->aistate) {
        case Q2AI_SEEK_LTG:
        case Q2AI_SEEK_NBG:
            /* Pure navigation — no combat.
             * #13 — Camping: well-armed bots occasionally camp near spots.
             * Mirrors Q3's BotWantsToCamp (ai_dmq3.c:2489). */
            if (bc->aistate == Q2AI_SEEK_LTG && bc->camper > 0.1f &&
                aggression >= 50 &&
                bc->camp_time < AAS_Time() - (60.0f + 300.0f * (1.0f - bc->camper)))
            {
                /* Q3 requires RL+10 or RG+10 or BFG+10 to camp */
                if ((bc->inventory[14] > 0 && bc->inventory[21] >= 10) ||
                    (bc->inventory[16] > 0 && bc->inventory[22] >= 10) ||
                    (bc->inventory[17] > 0 && bc->inventory[20] >= 10))
                {
                    /* Random check: higher camper trait = more likely */
                    if ((float)(rand() & 0x7FFF) / 0x7FFF < bc->camper) {
                        bot_goal_t campgoal;
                        int cs, besttraveltime = 99999;
                        bot_goal_t bestcampgoal;
                        qboolean found = false;
                        for (cs = BotGetNextCampSpotGoal(0, &campgoal); cs;
                             cs = BotGetNextCampSpotGoal(cs, &campgoal)) {
                            int tt = BotReachabilityArea(bc->origin, client);
                            (void)tt;
                            /* Just use the first camp spot found for simplicity */
                            Com_Memcpy(&bestcampgoal, &campgoal, sizeof(campgoal));
                            found = true;
                            break;
                        }
                        if (found) {
                            BotPushGoal(bc->goalstate, &bestcampgoal);
                            bc->hasgoal = true;
                            bc->camp_time = AAS_Time();
                        }
                    } else {
                        bc->camp_time = AAS_Time(); /* cooldown even on skip */
                    }
                }
            }
            Q2BotNavigateGoals(client, bc, bot_eye, &moveresult);
            Q2BotCheckBlocked(client, bc, &moveresult);
            break;

        case Q2AI_BATTLE_FIGHT:
            /* Dedicated combat — strafe, dodge, aim and fire.
             * #3 — Check for nearby items periodically even during fight.
             * Mirrors Q3 ai_dmnet.c: AINode_Battle_Fight checks for NBG
             * and transitions to AINode_Battle_NBG. */
            in_combat = true;
            if (bc->enemy >= 0) {
                Q2BotAttackMove(client, bc, bc->enemy);
                Q2BotAimAndFire(client, bc->enemy, bot_eye);
            }
            /* Every 1s, check for nearby health/armor/ammo pickup */
            if (bc->retreat_check_time < AAS_Time()) {
                bc->retreat_check_time = AAS_Time() + 1.0f;
                if (BotChooseNBGItem(bc->goalstate, bc->origin,
                                      bc->inventory, TFL_DEFAULT,
                                      NULL, 150)) {
                    bc->hasnbg = true;
                    bc->nbg_combat_time = AAS_Time() + 2.5f;
                    bc->aistate = Q2AI_BATTLE_NBG;
                }
            }
            break;

        case Q2AI_BATTLE_RETREAT:
            /* #7 — Navigate toward goals while defending + check nearby items.
             * Mirrors Q3's AINode_Battle_Retreat: grabs nearby health/armor
             * within 150 units during retreat. */
            Q2BotNavigateGoals(client, bc, bot_eye, &moveresult);
            Q2BotCheckBlocked(client, bc, &moveresult);
            in_combat  = true;
            if (bc->enemy >= 0)
                Q2BotAimAndFire(client, bc->enemy, bot_eye);
            /* Check for nearby goals every 1s (Q3 ai_dmnet.c:2429) */
            if (bc->retreat_check_time < AAS_Time()) {
                bc->retreat_check_time = AAS_Time() + 1.0f;
                if (BotChooseNBGItem(bc->goalstate, bc->origin,
                                      bc->inventory, TFL_DEFAULT,
                                      &bc->ltg, 150)) {
                    bc->hasnbg = true;
                    bc->nbg_combat_time = AAS_Time() + 2.5f;
                    bc->aistate = Q2AI_BATTLE_NBG;
                }
            }
            break;

        case Q2AI_BATTLE_NBG:
            /* Navigate toward the nearby item while still aiming/firing.
             * Mirrors Q3's AINode_Battle_NBG (ai_dmnet.c:2491). */
            Q2BotNavigateGoals(client, bc, bot_eye, &moveresult);
            in_combat = true;
            if (bc->enemy >= 0 && bc->attack_skill > 0.3f)
                Q2BotAimAndFire(client, bc->enemy, bot_eye);
            /* Check if NBG was reached */
            {
                bot_goal_t nbg_top;
                if (bc->hasnbg && BotGetTopGoal(bc->goalstate, &nbg_top)) {
                    if (BotTouchingGoal(bc->origin, &nbg_top)) {
                        BotPopGoal(bc->goalstate);
                        bc->hasnbg = false;
                        bc->aistate = (Q2BotAggression(bc) >= 50)
                                      ? Q2AI_BATTLE_FIGHT : Q2AI_BATTLE_RETREAT;
                    }
                }
            }
            break;

        case Q2AI_BATTLE_CHASE:
            /* Navigate toward enemy's last known position.
             * Mirrors Q3's AINode_Battle_Chase — create a temporary
             * goal from lastenemyorigin/lastenemyareanum and drive
             * BotMoveToGoal toward it. */
            in_combat = true;
            {
                bot_goal_t chase_goal;
                bot_moveresult_t chase_mr;

                Com_Memset(&chase_goal, 0, sizeof(chase_goal));
                chase_goal.entitynum = bc->enemy;
                chase_goal.areanum   = bc->lastenemyareanum;
                VectorCopy(bc->lastenemyorigin, chase_goal.origin);
                chase_goal.mins[0] = -8; chase_goal.mins[1] = -8; chase_goal.mins[2] = -8;
                chase_goal.maxs[0] =  8; chase_goal.maxs[1] =  8; chase_goal.maxs[2] =  8;

                BotMoveToGoal(&chase_mr, bc->movestate, &chase_goal,
                              TFL_DEFAULT);

                /* If we reached the last-known position, give up */
                if (BotTouchingGoal(bc->origin, &chase_goal)) {
                    bc->aistate    = Q2AI_SEEK_LTG;
                    bc->enemy      = -1;
                    bc->chase_time = 0;
                } else if (chase_mr.flags & MOVERESULT_MOVEMENTVIEW) {
                    EA_View(client, chase_mr.ideal_viewangles);
                }
            }
            break;
        }
    }

    /* Debug: log current AI state if developer mode is on */
    if (LibVarGetValue("bot_developer")) {
        static float last_state_log = 0;
        float now2 = AAS_Time();
        if (now2 - last_state_log > 2.0f) {
            static const char *statenames[] = {
                "SEEK_LTG", "SEEK_NBG", "BATTLE_FIGHT", "BATTLE_RETREAT",
                "BATTLE_CHASE", "BATTLE_NBG"
            };
            botimport.Print(PRT_MESSAGE,
                "bot %d: state=%s enemy=%d aggr=%d hasgoal=%d health=%d\n",
                client,
                statenames[bc->aistate < 6 ? bc->aistate : 0],
                bc->enemy,
                Q2BotAggression(bc),
                bc->hasgoal,
                bc->health);
            last_state_log = now2;
        }
    }

    /* --- Collect EA input and translate to Q2 --- */
    Com_Memset(&q3input, 0, sizeof(q3input));
    EA_GetInput(client, thinktime, &q3input);

    Com_Memset(&q2input, 0, sizeof(q2input));
    q2input.thinktime   = q3input.thinktime;
    VectorCopy(q3input.dir, q2input.dir);
    q2input.speed       = q3input.speed;
    q2input.actionflags = Q3ActionsToQ2(q3input.actionflags);

    /* #7 — Bot think time is managed by the game DLL which calls BotAI
     * at its own rate.  No residual scheduling needed in the adapter. */

    /* Viewangle priority:
     *   1. MOVERESULT_MOVEMENTVIEW — special moves (rocket jump, etc.)
     *   2. Combat aim (when in_combat, EA_View already set it)
     *   3. Navigation yaw (derived from movement direction for efficient path following)
     *   4. Fallback to EA viewangles */
    if (moveresult.flags & MOVERESULT_MOVEMENTVIEW) {
        VectorCopy(moveresult.ideal_viewangles, q2input.viewangles);
    } else if (in_combat) {
        VectorCopy(q3input.viewangles, q2input.viewangles);
    } else if (q3input.speed > 0.0f &&
               (q3input.dir[0] != 0.0f || q3input.dir[1] != 0.0f)) {
        vec3_t movedir_angles;
        Vector2Angles(q3input.dir, movedir_angles);
        VectorCopy(movedir_angles, q2input.viewangles);
    } else {
        VectorCopy(q3input.viewangles, q2input.viewangles);
    }

    q2import.BotInput(client, &q2input);
    EA_ResetInput(client);

    return Q2_BLERR_NOERROR;
}

static int Q2BotConsoleMessage(int client, int type, char *message)
{
    q2_botclient_t *bc;

    if (client < 0 || client >= Q2_BOTLIB_MAX_CLIENTS)
        return Q2_BLERR_INVALIDCLIENTNUMBER;
    bc = &q2clients[client];
    if (!bc->inuse) return Q2_BLERR_AICMFORINACTIVECLIENT;

    BotQueueConsoleMessage(bc->chatstate, type, message);
    return Q2_BLERR_NOERROR;
}

static int Q2BotTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3)
{
    return BotExportTest(parm0, parm1, parm2, parm3);
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
    Com_Memset(q2clients, 0, sizeof(q2clients));
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
    LibVarSet("bot_developer", "1");

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

    return &q2_export;
}
