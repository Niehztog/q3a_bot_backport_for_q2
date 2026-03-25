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
    - EntityTrace: stub (no-hit)
    - inPVS     : stub (returns true)
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
#define Q2CHAR_GENDER           1
#define Q2CHAR_WEAPONWEIGHTS    3
#define Q2CHAR_CHAT_FILE        21
#define Q2CHAR_CHAT_NAME        22
#define Q2CHAR_ITEMWEIGHTS      40
#define Q2CHAR_WALKER           48

/* ====================================================================
 * Per-client AI state
 * ==================================================================== */
#define Q2_BOTLIB_MAX_CLIENTS 256

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
} q2_botclient_t;

/* ====================================================================
 * Module globals
 * ==================================================================== */
static q2_bot_import_t q2import;         /* stored Q2 import callbacks  */
static q2_bot_export_t q2_export;        /* returned Q2 export struct   */
static q2_botclient_t  q2clients[Q2_BOTLIB_MAX_CLIENTS];

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
#define Q2BSP_LUMP_ENTITIES 0

typedef struct { int fileofs, filelen; } q2bsplump_t;
typedef struct { int ident, version; q2bsplump_t lumps[19]; } q2bspheader_t;

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

    len = hdr.lumps[Q2BSP_LUMP_ENTITIES].filelen;
    if (len <= 0) return 0;
    if (len >= Q2_BSP_ENTITYSTRING_MAX) len = Q2_BSP_ENTITYSTRING_MAX - 1;

    fseek(f, base_offset + hdr.lumps[Q2BSP_LUMP_ENTITIES].fileofs, SEEK_SET);
    if ((int)fread(q2_bsp_entitystring, 1, len, f) != len) {
        botimport.Print(PRT_WARNING,
            "Q2Adapt: short read of BSP entity lump for '%s'\n", mapname);
    }
    q2_bsp_entitystring[len] = '\0';
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

/* Q3 Trace: void out-param variant; Q2 returns by value */
static void Q3Trace_Adapter(bsp_trace_t *trace, vec3_t start, vec3_t mins,
                              vec3_t maxs, vec3_t end, int passent, int contentmask)
{
    *trace = q2import.Trace(start, mins, maxs, end, passent, contentmask);
    /* Q2 uses entity 0 for the world; Q3 botlib uses ENTITYNUM_WORLD (1023).
     * BotOnTopOfEntity checks trace.ent != ENTITYNUM_WORLD to detect standing
     * on a non-world entity — translate so the check works correctly. */
    if (trace->ent == 0)
        trace->ent = ENTITYNUM_WORLD;
}

/* Q2 has no per-entity trace; return a clean no-hit result */
static void Q3EntityTrace_Stub(bsp_trace_t *trace, vec3_t start, vec3_t mins,
                                 vec3_t maxs, vec3_t end,
                                 int entnum, int contentmask)
{
    (void)start; (void)mins; (void)maxs;
    (void)entnum; (void)contentmask;
    Com_Memset(trace, 0, sizeof(*trace));
    trace->fraction = 1.0f;
    VectorCopy(end, trace->endpos);
}

/* Q2 doesn't expose inPVS; conservatively return true */
static int Q3inPVS_Stub(vec3_t p1, vec3_t p2)
{
    (void)p1; (void)p2;
    return 1;
}

static char *Q3BSPEntityData_Callback(void)
{
    return q2_bsp_entitystring;
}

static void Q3BSPModelMinsMaxsOrigin_Stub(int modelnum, vec3_t angles,
                                           vec3_t mins, vec3_t maxs, vec3_t origin)
{
    (void)modelnum; (void)angles;
    VectorClear(mins);
    VectorClear(maxs);
    if (origin) VectorClear(origin);
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

static int Q2BotLoadMap(char *mapname, int modelindexes, char *modelindex[],
                         int soundindexes, char *soundindex[],
                         int imageindexes, char *imageindex[])
{
    (void)modelindexes; (void)modelindex;
    (void)soundindexes; (void)soundindex;
    (void)imageindexes; (void)imageindex;

    int errnum;

    /* Read BSP entity lump from disk before calling Q3's load */
    Q2_ReadBSPEntityData(mapname);

    errnum = Export_BotLibLoadMap(mapname);
    if (errnum != BLERR_NOERROR) return errnum;

    /* Mark all items loaded from BSP as "always present" so that
     * BotChooseLTGItem won't skip them (Q3 links them dynamically via
     * BotUpdateEntityItems; Q2 BSP items are always in the world). */
    BotMarkLevelItemsPresent();

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
    bc->enemy_hdist  = 9999;   /* no enemy yet — don't bias range weights */
    bc->enemy_height = 0;
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

static int Q2BotStartFrame(float time)
{
    int ret = Export_BotLibStartFrame(time);
    /* Update dynamic item entities (dropped weapons, flags) each frame */
    BotUpdateEntityItems();
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

    /* Translate to Q3 entity state for AAS entity tracking */
    Com_Memset(&state, 0, sizeof(state));
    state.type = 1;  /* ET_PLAYER */
    VectorCopy(buc->origin,     state.origin);
    VectorCopy(buc->viewangles, state.angles);
    VectorCopy(buc->origin,     state.old_origin);  /* no prev pos in Q2 buc */
    /* Standard Q2 player bounding box */
    state.mins[0] = -16.0f; state.mins[1] = -16.0f;
    state.mins[2] = (buc->pm_flags & 4) ? -16.0f : -24.0f; /* ducked? */
    state.maxs[0] =  16.0f; state.maxs[1] =  16.0f;
    state.maxs[2] = (buc->pm_flags & 4) ?   4.0f :  32.0f;
    state.solid  = (buc->pm_type == Q2PM_NORMAL) ? 1 : 0;

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
    state.event      = bue->event;
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

/*
 * Q2BotAI  —  per-bot, per-frame AI entry point
 *
 * Navigation + item collection + minimal combat:
 *   1. Initialise the Q3 move state with the bot's current Q2 position.
 *   2. Pick a long-term item goal if we don't already have one.
 *   3. Drive BotMoveToGoal to compute the next movement step.
 *   4. Detect items that have been picked up (BotItemGoalInVisButNotVisible).
 *   5. Scan for nearby enemy players and fire if found.
 *   6. Collect EA input and translate to Q2 bot_input_t.
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

    /* --- Dead bot: press attack to respawn, skip all navigation --- */
    if (bc->pm_type == Q2PM_DEAD || bc->pm_type == Q2PM_GIB) {
        q2_bot_input_t respawn;
        Com_Memset(&respawn, 0, sizeof(respawn));
        respawn.thinktime   = thinktime;
        /* Use ACTION_RESPAWN, not ACTION_ATTACK.  BotExecuteInput handles
         * ACTION_RESPAWN by doing latched_buttons |= BUTTON_ATTACK directly,
         * bypassing edge detection.  Using ACTION_ATTACK only latches on the
         * 0→1 transition; if the bot was attacking when killed, oldbuttons
         * already has BUTTON_ATTACK and no latch fires → no respawn. */
        respawn.actionflags = Q2_ACTION_RESPAWN;
        if (bc->hasnbg) { BotPopGoal(bc->goalstate); bc->hasnbg = false; }
        if (bc->hasgoal) { BotPopGoal(bc->goalstate); bc->hasgoal = false; }
        bc->ltg_check_time = 0.0f;
        bc->nbg_check_time = 0.0f;
        bc->goal_set_time  = 0.0f;
        /* After respawn the game resets inventory to blaster-only, so force
         * weapon re-selection on the first live frame. */
        bc->best_weapon_num = 0;
        bc->enemy_hdist  = 9999;
        bc->enemy_height = 0;
        q2import.BotInput(client, &respawn);
        /* Clear stale EA state (attack/view flags from before death) so the
         * first live frame after respawn doesn't fire spurious shots. */
        EA_ResetInput(client);
        return Q2_BLERR_NOERROR;
    }

    /* --- Initialise move state with current Q2 position --- */
    Com_Memset(&initmove, 0, sizeof(initmove));
    VectorCopy(bc->origin,     initmove.origin);
    VectorCopy(bc->velocity,   initmove.velocity);
    VectorCopy(bc->viewoffset, initmove.viewoffset);
    VectorCopy(bc->viewangles, initmove.viewangles);
    initmove.entitynum    = client + 1;  /* entity numbers are 1-indexed */
    initmove.client       = client;
    initmove.thinktime    = thinktime;
    initmove.presencetype = (bc->pm_flags & 1) ? PRESENCE_CROUCH : PRESENCE_NORMAL;
    /* PMF_ON_GROUND = 4 in Q2 (see BotSetPMoveState defines in bl_main.c) */
    if (bc->pm_flags & 4) initmove.or_moveflags |= MFL_ONGROUND;
    BotInitMoveState(bc->movestate, &initmove);

    /* --- Goal management ---
     *
     * The Q3 botlib goal stack:
     *   [ LTG ]          when hasgoal only
     *   [ NBG | LTG ]    when hasnbg (NBG on top)
     *
     * Policy:
     *   - Abandon stale LTGs after 60s (safety valve for unreachable goals).
     *   - Throttle BotChooseLTGItem to at most once per 0.5s.
     *   - After gaining an LTG, scan for a faster nearby item (NBG) every 0.5s.
     *   - For roam goals (no real items found) allow any reachable item as NBG.
     */
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
            if (BotChooseLTGItem(bc->goalstate, bc->origin,
                                  bc->inventory, TFL_DEFAULT)) {
                BotGetTopGoal(bc->goalstate, &bc->ltg);
                bc->hasgoal      = true;
                bc->goal_set_time = now;
            }
        }

        /* NBG: opportunistically divert to a nearby item (throttled to 0.5s).
         * Radius 400 lets the bot grab health/ammo within ~10m even when
         * navigating toward a distant weapon — important during combat.
         * Pass NULL as ltg for roam goals so any reachable item qualifies. */
        if (bc->hasgoal && !bc->hasnbg && (now - bc->nbg_check_time) >= 0.5f) {
            bot_goal_t *ltg_ptr = (bc->ltg.flags & GFL_ROAM) ? NULL : &bc->ltg;
            bc->nbg_check_time = now;
            if (BotChooseNBGItem(bc->goalstate, bc->origin, bc->inventory,
                                  TFL_DEFAULT, ltg_ptr, 400)) {
                bc->hasnbg = true;
            }
        }
    }

    /* --- Weapon selection: switch to best available weapon ---
     * Write enemy distance into the inventory before calling
     * BotChooseBestFightWeapon so that range-aware fuzzy weights in
     * default_w.c can gate weapon scores on distance.
     * Slots 200 (ENEMY_HORIZONTAL_DIST) and 201 (ENEMY_HEIGHT) mirror Q3's
     * BotUpdateBattleInventory convention.  BotUpdateClient already
     * memcpy-ed pers.inventory (which has 0 in those slots), so we
     * overwrite here before the weapon-weight evaluation. */
    bc->inventory[200] = bc->enemy_hdist;    /* ENEMY_HORIZONTAL_DIST */
    bc->inventory[201] = bc->enemy_height;   /* ENEMY_HEIGHT          */
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

    /* Bot eye position — used by both navigation and combat below. */
    {
        vec3_t bot_eye;
        VectorAdd(bc->origin, bc->viewoffset, bot_eye);

        /* --- Navigate toward the current active goal (top of stack) --- */
        Com_Memset(&moveresult, 0, sizeof(moveresult));
        if (bc->hasgoal) {
            bot_goal_t active_goal;
            if (BotGetTopGoal(bc->goalstate, &active_goal)) {
                BotMoveToGoal(&moveresult, bc->movestate, &active_goal, TFL_DEFAULT);

                /* Weapon jump override: BotTravel_RocketJump sets this flag
                 * to signal that the bot must hold the rocket launcher (or
                 * BFG) for the jump.  Issue a "use" command now so Q2 sees
                 * the weapon switch before the attack input fires.  This
                 * overrides the normal BotChooseBestFightWeapon selection
                 * made earlier in the frame. */
                if (moveresult.flags & MOVERESULT_MOVEMENTWEAPON) {
                    weaponinfo_t jump_wi;
                    BotGetWeaponInfo(bc->weaponstate, moveresult.weapon, &jump_wi);
                    if (jump_wi.name[0]) {
                        q2import.BotClientCommand(client, "use", jump_wi.name, NULL);
                        bc->best_weapon_num = moveresult.weapon;
                    }
                }

                /* Detect item gone: visible position but entity has left
                 * (someone else picked it up).  Reset avoid timer and pop. */
                if (BotItemGoalInVisButNotVisible(client, bot_eye,
                                                   bc->viewangles, &active_goal)) {
                    BotSetAvoidGoalTime(bc->goalstate, active_goal.number, 30.0f);
                    BotPopGoal(bc->goalstate);
                    if (bc->hasnbg) {
                        bc->hasnbg = false;  /* revealed LTG is still on stack */
                    } else {
                        bc->hasgoal       = false;
                        bc->ltg_check_time = 0.0f;
                    }
                }
                /* Goal reached — pop and keep going */
                else if (BotTouchingGoal(bc->origin, &active_goal)) {
                    BotPopGoal(bc->goalstate);
                    if (bc->hasnbg) {
                        bc->hasnbg = false;  /* back to LTG */
                    } else {
                        bc->hasgoal       = false;
                        bc->ltg_check_time = 0.0f;
                    }
                }
            }
        }

        /* --- Combat: find nearest visible enemy player, aim and fire ---
         * Uses entity type 1 (ET_PLAYER) set by Q2BotUpdateEntity.
         * maxclients is set via BotLibVarSet("maxclients",...) at setup.
         *
         * Aim vector is computed from bot_eye (not feet) because that is
         * where the blaster projectile actually spawns.  A LOS trace is
         * performed before committing to an enemy so the bot cannot fire
         * through solid walls.
         *
         * Also updates bc->enemy_hdist / bc->enemy_height (used by weapon
         * selection on the next frame via inventory[200/201]). */
        {
            int maxcl = (int)LibVarGetValue("maxclients");
            int best  = -1;
            float best_dist = 1200.0f;  /* max engagement range */
            aas_entityinfo_t best_info;
            int i;

            for (i = 1; i <= maxcl; i++) {
                aas_entityinfo_t entinfo;
                vec3_t enemy_center, dir;
                bsp_trace_t los;
                float dist;

                if (i == client + 1) continue;  /* skip self (entity = client+1) */
                AAS_EntityInfo(i, &entinfo);
                if (!entinfo.valid || entinfo.type != 1 /* ET_PLAYER */) continue;

                /* LOS: trace a point ray from bot eye to enemy chest.
                 * CONTENTS_SOLID = 1 in both Q2 and Q3. */
                VectorCopy(entinfo.origin, enemy_center);
                enemy_center[2] += 22.0f;
                los = q2import.Trace(bot_eye, NULL, NULL, enemy_center,
                                     client + 1, 1 /* CONTENTS_SOLID */);
                if (los.fraction < 1.0f) continue;  /* wall in the way */

                VectorSubtract(entinfo.origin, bc->origin, dir);
                dist = VectorLength(dir);
                if (dist < best_dist) {
                    best_dist = dist;
                    best      = i;
                    Com_Memcpy(&best_info, &entinfo, sizeof(best_info));
                }
            }

            if (best >= 0) {
                vec3_t aimdir, aim_angles, aim_target;
                vec3_t hdiff;
                in_combat = true;

                /* Record horizontal distance and height for range-aware weapon
                 * selection on the next frame (inventory[200/201]). */
                hdiff[0] = best_info.origin[0] - bc->origin[0];
                hdiff[1] = best_info.origin[1] - bc->origin[1];
                hdiff[2] = 0.0f;
                bc->enemy_hdist  = (int)VectorLength(hdiff);
                bc->enemy_height = (int)(best_info.origin[2] - bc->origin[2]);

                VectorCopy(best_info.origin, aim_target);
                aim_target[2] += 22.0f;          /* target chest height */
                /* Compute aim from eye position — projectile spawns here */
                VectorSubtract(aim_target, bot_eye, aimdir);
                VectorNormalize(aimdir);
                Vector2Angles(aimdir, aim_angles);
                EA_View(client, aim_angles);     /* sets botinputs[client].viewangles */
                EA_Attack(client);               /* sets ACTION_ATTACK in botinputs */
            } else {
                /* No enemy visible — reset distance so weapon weights use
                 * no-range-preference mode (select by power, not range). */
                bc->enemy_hdist  = 9999;
                bc->enemy_height = 0;
            }
        }
    } /* bot_eye scope */

    /* --- Collect EA input (navigation dir + combat overrides) --- */
    Com_Memset(&q3input, 0, sizeof(q3input));
    EA_GetInput(client, thinktime, &q3input);

    Com_Memset(&q2input, 0, sizeof(q2input));
    q2input.thinktime   = q3input.thinktime;
    VectorCopy(q3input.dir, q2input.dir);
    q2input.speed       = q3input.speed;
    q2input.actionflags = Q3ActionsToQ2(q3input.actionflags);

    /* BotExecuteInput projects dir onto the bot's forward/right vectors using
     * viewangles[YAW].  During combat, EA_View already set the correct aim
     * angles (captured in q3input.viewangles).  During navigation only,
     * derive the yaw from the movement direction so the projection gives
     * full speed.  MOVERESULT_MOVEMENTVIEW overrides both (special moves). */
    if (moveresult.flags & MOVERESULT_MOVEMENTVIEW) {
        VectorCopy(moveresult.ideal_viewangles, q2input.viewangles);
    } else if (!in_combat && q3input.speed > 0.0f &&
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
    q3imp.EntityTrace           = Q3EntityTrace_Stub;
    q3imp.PointContents         = import->PointContents;
    q3imp.inPVS                 = Q3inPVS_Stub;
    q3imp.BSPEntityData         = Q3BSPEntityData_Callback;
    q3imp.BSPModelMinsMaxsOrigin = Q3BSPModelMinsMaxsOrigin_Stub;
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
    LibVarSet("phys_maxvelocity",       "300");
    LibVarSet("phys_maxwalkvelocity",   "300");
    LibVarSet("phys_maxcrouchvelocity", "150");
    LibVarSet("phys_maxstep",           "18");
    LibVarSet("phys_maxbarrier",        "50");
    LibVarSet("phys_watergravity",      "100");

    /* CTF grappling hook: GrappleState() matches ET_MISSILE entities whose
     * state.weapon == weapindex_grapple.  In our Q2 adapter, state.weapon is
     * set to the entity's modelindex (see Q2BotUpdateEntity).  The game DLL
     * must override this libvar with the actual grapple hook model index:
     *   BotLibVarSet("weapindex_grapple", "<gi.modelindex of hook model>")
     * Until then, 0 keeps grapple detection safely disabled (no Q2 entity
     * will ever have modelindex 0). */
    LibVarSet("weapindex_grapple", "0");

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
