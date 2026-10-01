/*
===========================================================================

be_interface_q2.h  --  the Quake II bot library interface this botlib exports

The Gladiator Bot game library loads a bot library and calls its GetBotAPI
with a bot_import_t (10 engine callbacks) and gets back a bot_export_t. This
is that interface, as the game side (Gladiator's game/botlib.h) lays it out,
under q2_ names: the Quake III headers this library is built with define
their own bot_input_t, bot_settings_t and so on, with other layouts.

Include after game_q3/q_shared.h, game_q3/botlib.h (bsp_trace_t) and
game_q3/be_ai_chat.h (bot_consolemessage_t).

The game copies the table GetBotAPI returns. The first 20 slots, through
Test, are Gladiator's and in Gladiator's order; the slots after them are
this library's own additions, which a Gladiator game never reads. The
import table is exactly Gladiator's: an 11th slot would read whatever
follows the table in a game built against that header.

Every struct below crosses the library boundary and none holds a pointer,
so their sizes are the same at 32 and 64 bits; the _Static_asserts at the
end are the sizes a game's side of the same contract asserts.

===========================================================================
*/

#ifndef BE_INTERFACE_Q2_H
#define BE_INTERFACE_Q2_H

#include <stddef.h>     /* offsetof */


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
/* That is all of Gladiator's import table (game_q2/botlib.h bot_import_t).
 * GetBotAPI copies a game's table whole: an entry beyond it would read
 * whatever follows the table in a Gladiator game. The PVS and the teams
 * come from the map (Q3inPVS_Adapter) and the skins (Q2_ClientsOnSameTeam)
 * instead, as they did for Gladiator's botlib. */

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

/* GetBotAPI is the one symbol this library exports (the Makefile builds it
 * with hidden visibility). It is a plain C function on every target,
 * 32-bit Windows included: Gladiator's own gladiator.dll returns from it
 * with a plain `ret`, i.e. __cdecl, so a game has to call it that way to
 * load Gladiator's library, and this one has to agree with that game. */
#if defined(_WIN32)
#define Q2_BOTLIB_EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
#define Q2_BOTLIB_EXPORT __attribute__((visibility("default")))
#else
#define Q2_BOTLIB_EXPORT
#endif

Q2_BOTLIB_EXPORT q2_bot_export_t *GetBotAPI(q2_bot_import_t *import);

/* The interface's sizes. A struct size cannot see two members trading
 * places, so bsp_trace_t's two most used offsets are asserted as well. */
_Static_assert(sizeof(bsp_surface_t) == 24, "bsp_surface_t is 24 bytes");
_Static_assert(sizeof(bsp_trace_t) == 84, "bsp_trace_t is 84 bytes");
_Static_assert(offsetof(bsp_trace_t, fraction) == 8, "bsp_trace_t.fraction at 8");
_Static_assert(offsetof(bsp_trace_t, endpos) == 12, "bsp_trace_t.endpos at 12");
_Static_assert(sizeof(q2_bot_settings_t) == 432, "bot_settings_t is 432 bytes");
_Static_assert(sizeof(q2_bot_clientsettings_t) == 144, "bot_clientsettings_t is 144 bytes");
_Static_assert(sizeof(q2_bot_input_t) == 36, "bot_input_t is 36 bytes");
_Static_assert(sizeof(q2_bot_updateclient_t) == 1228, "bot_updateclient_t is 1228 bytes");
_Static_assert(sizeof(q2_bot_updateentity_t) == 104, "bot_updateentity_t is 104 bytes");

#endif /* BE_INTERFACE_Q2_H */
