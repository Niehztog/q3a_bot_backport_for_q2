//===========================================================================
//
// Name:				bl_cmd.c
// Function:		bot commands
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1999-02-10
// Tab Size:		3
//===========================================================================

#include "g_local.h"

#ifdef BOT

#include "bl_main.h"
#include "bl_spawn.h"
#include "bl_redirgi.h"
#include "bl_botcfg.h"

void ClientUserinfoChanged (edict_t *ent, char *userinfo);

typedef struct nearbyitem_s
{
	char *classname;
	float weight;
} nearbyitem_t;

nearbyitem_t nearbyitems[] =
{
	{"item_armor_body",			70},
	{"item_armor_combat", 		70},
	{"item_armor_jacket", 		50},
	{"item_power_screen",		90},
	{"item_power_shield",		90},
	{"weapon_shotgun",			40},
	{"weapon_supershotgun",		60},
	{"weapon_machinegun",		50},
	{"weapon_chaingun",			100},
	{"weapon_grenadelauncher",	100},
	{"weapon_rocketlauncher",	100},
	{"weapon_hyperblaster",		100},
	{"weapon_railgun",			100},
	{"weapon_bfg",					100},
	{"item_quad",					100},
	{"item_invulnerability",	100},
	{"item_silencer",				60},
	{"item_breather",				60},
	{"item_enviro",				60},
	{"item_ancient_head",		60},
	{"item_bandolier",			60},
	{"item_pack",					70},
	{"key_data_cd",				40},
	{"key_power_cube",			40},
	{"key_pyramid",				40},
	{"key_data_spinner",			40},
	{"key_pass",					40},
	{"key_blue_key",				40},
	{"key_red_key",				40},
	//CTF
	{"item_flag_team1",			100},
	{"item_flag_team2",			100},
	{NULL,							0}
};

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void ShowGPSText(edict_t *ent, vec3_t goal)
{
	vec3_t hordir, dir, forward, right;
	float dot;
	char buf[128];

	VectorSubtract(ent->s.origin, goal, dir);
	VectorCopy(dir, hordir);
	hordir[2] = 0;
	AngleVectors(ent->client->resp.cmd_angles, forward, right, NULL);
	forward[2] = 0;
	right[2] = 0;
	VectorNormalize(hordir);
	VectorNormalize(forward);
	VectorNormalize(right);

	dot = DotProduct(hordir, forward);

	sprintf(buf, "^\n< >\n_\n\ndx=%1.0f dy=%1.0f dz=%1.0f\n\ndistance = %1.0f",
						dir[0], dir[1], dir[2], VectorLength(dir));
	if (dot > 0.7) buf[0] += 128;
	else if (dot < -0.7) buf[6] += 128;
	else if (DotProduct(hordir, right) > 0) buf[4] += 128;
	else buf[2] += 128;

	gi.centerprintf(ent, buf);
} //end of the function ShowGPSText
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void TeamHelp_f(edict_t *ent)
{
	int i, j;
	float radius, weight, bestweight, dist;
	char buf[144];
	vec3_t eorg;
	edict_t *item, *bestitem;

	if (!ent) return;
	if (!ent->client) return;
#ifdef OBSERVER
	if (ent->flags & FL_OBSERVER) return;
#endif //OBSERVER

	radius = 500;

	bestweight = 0;
	bestitem = NULL;
	for (item = g_edicts; item < &g_edicts[globals.num_edicts]; item++)
	{
		if (!item->inuse) continue;
		for (j = 0; j < 3; j++)
		{
			eorg[j] = ent->s.origin[j] - (item->s.origin[j] + (item->mins[j] + item->maxs[j])*0.5);
		} //end for
		dist = VectorLength(eorg);
		//the item should be in the given radius
		if (dist > radius) continue;
		//never use dropped items
		if (item->spawnflags & DROPPED_ITEM) continue;
		//the location of the item should be visible
		if (!visible(ent, item)) continue;
		//check if item and calculate weight
		for (i = 0; nearbyitems[i].classname; i++)
		{
			if (!strcmp(item->classname, nearbyitems[i].classname))
			{
				weight = nearbyitems[i].weight / dist;
				if (weight > bestweight)
				{
					bestweight = weight;
					bestitem = item;
				} //end if
				break;
			} //end if
		} //end for
	} //end for
	strcpy(buf, "");
	//if addressed
	if (gi.argc() > 1)
	{
		strcat(buf, gi.argv(1));
		strcat(buf, " ");
	} //end if
	//help or accompany
	if (!strcmp(gi.argv(0), "teamhelp")) strcat(buf, "help me");
	else strcat(buf, "accompany me");
	//if near an item
	if (bestitem)
	{
		strcat(buf, " near the ");
		strcat(buf, bestitem->item->pickup_name);
	} //end if
	BotClientCommand(ent - g_edicts - 1, "say_team", buf, NULL);
} //end of the function TeamHelp_f
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void CheckPoint_f(edict_t *ent)
{
	vec3_t mins = {-16, -16, -24}, maxs = {16, 16, 4};
	vec3_t start, end;
	char buf[144];
	int loc[3], x, y, z;
	trace_t trace;

	if (gi.argc() <= 1)
	{
		gi.cprintf(ent, PRINT_HIGH, "checkpoint <name>\n");
		return;
	} //end if

	VectorCopy(ent->s.origin, end);
	loc[0] = (int) end[0];
	loc[1] = (int) end[1];
	loc[2] = (int) end[2];
	for (x = -1; x <= 1; x++)
	{
		for (y = -1; y <= 1; y++)
		{
			for (z = -1; z <= 1; z++)
			{
				start[0] = loc[0] + x;
				start[1] = loc[1] + y;
				start[2] = loc[2] + z;
				//
				trace = gi.trace(start, mins, maxs, end, ent, MASK_PLAYERSOLID);
				//
				if (!trace.startsolid)
				{
					loc[0] = (int) start[0];
					loc[1] = (int) start[1];
					loc[2] = (int) start[2];
					break;
				} //end if
			} //end for
			if (z <= 1) break;
		} //end for
		if (y <= 1) break;
	} //end for
	if (x > 1)
	{
		gi.cprintf(ent, PRINT_HIGH, "invalid checkpoint position\n");
		return;
	} //end if
	sprintf(buf, "checkpoint %s is at gps %d %d %d", gi.argv(1), loc[0], loc[1], loc[2]);
	BotClientCommand(ent - g_edicts - 1, "say_team", buf, NULL);
} //end of the function CheckPoint_f
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
#ifdef BOT_DEBUG
void Cmd_ServerCommand(edict_t *ent)
{
	char str[1024];
	int i;

	str[0] = '\0';
	str[sizeof(str)-1] = '\0'; //Riv++
	for (i = 1; i < gi.argc(); i++)
	{
		//add space and double quote
		if (i > 1) strncat(str, " \"", sizeof(str) - strlen(str) - 1); //Riv++
		//add the argument
		strncat(str, gi.argv(i), sizeof(str) - strlen(str) - 1); //Riv++
		//add double quote
		if (i > 1) strncat(str, "\"", sizeof(str) - strlen(str) - 1); //Riv++
	} //end for
	gi.dprintf("%s adds server command: %s\n", ent->client->pers.netname, str);
	gi.AddCommandString(str);
} //end of the function Cmd_ServerCommand
#endif //BOT_DEBUG
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotDumpInventory(void)
{
	int i;

	for (i = 1; itemlist[i].pickup_name; i++)
	{
		gi.dprintf("%-16s %d\n", itemlist[i].pickup_name, i);
	} //end for
} //end of the function BotDumpInventory
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
#ifdef BOT_DEBUG
qboolean BotDebugCmd(char *cmd, edict_t *ent, int server)
{
	return true;
} //end of the function BotDebugCmd
#endif //BOT_DEBUG
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
/*
 * AAS debug console commands.
 * The visualization functions (AAS_ShowArea, AAS_ShowReachableAreas) come
 * from Q3's botlib (be_aas_debug.c) and use the debug line infrastructure
 * from Gladiator's game DLL (bl_debug.c: DebugLineCreate/Show/Delete).
 *
 * Q3 had cvars like bot_testsolid and bot_testclusters that showed area
 * info for the player's current position, but NOT direct "show area N"
 * or "teleport to area" commands.  These are new convenience commands
 * that wire Q3's existing debug drawing to user-accessible controls.
 */
/* Ensure botlib is loaded for debug commands.  If no bots have been
 * added yet, load the library on demand (same path as bot spawning). */
static bot_library_t *BotCmd_EnsureLib(edict_t *ent)
{
	bot_library_t *lib = botglobals.firstbotlib;
	if (!lib) {
#if defined(WIN32) || defined(_WIN32)
		lib = BotUseLibrary(gi.cvar("botlib", "botlib.dll", 0)->string);
#else
		lib = BotUseLibrary(gi.cvar("botlib", "botlib.so", 0)->string);
#endif
	}
	if (!lib || !lib->funcs.BotLibraryInitialized()) {
		gi.cprintf(ent, PRINT_HIGH, "botlib failed to load\n");
		return NULL;
	}
	return lib;
}

static void BotCmd_ShowArea(edict_t *ent)
{
	int areanum;
	bot_library_t *lib = BotCmd_EnsureLib(ent);
	if (!lib) return;
	if (gi.argc() < 2) {
		/* No argument: show area at player's current position */
		vec3_t pos;
		VectorCopy(ent->s.origin, pos);
		areanum = lib->funcs.AAS_PointAreaNum(pos);
		gi.cprintf(ent, PRINT_HIGH, "current area: %d\n", areanum);
	} else {
		areanum = atoi(gi.argv(1));
	}
	if (areanum <= 0) {
		gi.cprintf(ent, PRINT_HIGH, "invalid area %d\n", areanum);
		return;
	}
	lib->funcs.AAS_ShowArea(areanum);
	gi.cprintf(ent, PRINT_HIGH, "showing area %d\n", areanum);
}

static void BotCmd_ShowReach(edict_t *ent)
{
	int areanum;
	bot_library_t *lib = BotCmd_EnsureLib(ent);
	if (!lib) return;
	if (gi.argc() < 2) {
		vec3_t pos;
		VectorCopy(ent->s.origin, pos);
		areanum = lib->funcs.AAS_PointAreaNum(pos);
		gi.cprintf(ent, PRINT_HIGH, "current area: %d\n", areanum);
	} else {
		areanum = atoi(gi.argv(1));
	}
	if (areanum <= 0) {
		gi.cprintf(ent, PRINT_HIGH, "invalid area %d\n", areanum);
		return;
	}
	lib->funcs.AAS_ShowReachableAreas(areanum);
	gi.cprintf(ent, PRINT_HIGH, "showing reachabilities from area %d\n", areanum);
}

static void BotCmd_GotoArea(edict_t *ent)
{
	int areanum;
	bot_library_t *lib = BotCmd_EnsureLib(ent);
	if (!lib) return;
	if (gi.argc() < 2) {
		gi.cprintf(ent, PRINT_HIGH, "usage: bot_gotoarea <areanum>\n");
		return;
	}
	areanum = atoi(gi.argv(1));
	if (areanum <= 0) {
		gi.cprintf(ent, PRINT_HIGH, "invalid area %d\n", areanum);
		return;
	}
	{
		vec3_t center;
		if (lib->funcs.AAS_AreaCenter(areanum, center)) {
			VectorCopy(center, ent->s.origin);
			ent->s.origin[2] += 10; /* slightly above ground */
			gi.linkentity(ent);
			gi.cprintf(ent, PRINT_HIGH, "teleported to area %d (%.0f %.0f %.0f)\n",
			           areanum, center[0], center[1], center[2]);
		} else {
			gi.cprintf(ent, PRINT_HIGH, "area %d not found in AAS data\n", areanum);
		}
	}
}

static void BotCmd_ClearDebug(edict_t *ent)
{
	bot_library_t *lib = botglobals.firstbotlib;
	if (!lib || !lib->funcs.BotLibraryInitialized()) return;
	lib->funcs.AAS_ClearShownDebugLines();
	gi.cprintf(ent, PRINT_HIGH, "debug lines cleared\n");
}

qboolean BotServerCmd(char *cmd, edict_t *ent, int server)
{
	/* AAS debug commands */
	if (Q_stricmp(cmd, "bot_showarea") == 0)
	{
		if (ent) BotCmd_ShowArea(ent);
		return true;
	}
	else if (Q_stricmp(cmd, "bot_showreach") == 0)
	{
		if (ent) BotCmd_ShowReach(ent);
		return true;
	}
	else if (Q_stricmp(cmd, "bot_gotoarea") == 0)
	{
		if (ent) BotCmd_GotoArea(ent);
		return true;
	}
	else if (Q_stricmp(cmd, "bot_cleardebug") == 0)
	{
		if (ent) BotCmd_ClearDebug(ent);
		return true;
	}
	else if (Q_stricmp(cmd, "addbot") == 0)
	{
		if (ent && (gi.cvar("serveronlybotcmds", "0", 0))->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "not allowed to add bots\n");
		} //end if
		else BotAddDeathmatch(ent);
	} //end else if
	else if (Q_stricmp(cmd, "removebot") == 0)
	{
		if (ent && (gi.cvar("serveronlybotcmds", "0", 0))->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "not allowed to remove bots\n");
		} //end if
		else BotRemoveDeathmatch(ent);
	} //end else if
	else if (Q_stricmp(cmd, "addrandom") == 0)
	{
		if (ent && (gi.cvar("serveronlybotcmds", "0", 0))->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "not allowed to randomly add bots\n");
		} //end if
		else
		{
			int i, num;

			if (!Q_stricmp(gi.argv(0), "sv")) num = 2;
			else num = 1;
			if (gi.argc() <= num) AddRandomBot(ent);
			else
			{
				num = atoi(gi.argv(num));
				for (i = 0; i < num; i++) if (!AddRandomBot(ent)) break;
			} //end else
		} //end else
	} //end if
	else if (Q_stricmp(cmd, "botpause") == 0)
	{
		if (ent && (gi.cvar("serveronlybotcmds", "0", 0))->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "not allowed to pause the game\n");
		} //end if
		else
		{
			if (!(gi.cvar("allow_pause" , "1", 0))->value)
			{
				if (ent) gi.cprintf(ent, PRINT_HIGH, "not allowed to pause the game\n");
				else gi.dprintf("not allowed to pause the game\n");
			} //end if
			else
			{
				paused = !paused;
				if (!paused)
				{
					int i;
					edict_t *cl_ent;

					for (i = 0; i < game.maxclients; i++)
					{
						cl_ent = g_edicts + 1 + i;
						if (!cl_ent->inuse) continue;
						if (cl_ent->flags & FL_BOT) continue;
						cl_ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
						gi.centerprintf(ent, "");
					} //end for
					gi.bprintf(PRINT_HIGH, "%s unpaused the game\n", ent->client->pers.netname);
				} //end if
			} //end else
		} //end else
	} //end else if
	else if (ent && Q_stricmp(cmd, "menu") == 0)
	{
		ToggleBotMenu(ent);
	} //end else if
	else
	{
		return false;
	} //end else
	return true;
} //end of the function BotServerCmd
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
qboolean BotCmd(char *cmd, edict_t *ent, int server)
{
	char userinfo[MAX_INFO_STRING];

	//check for commands that might be server only
	if (BotServerCmd(cmd, ent, server))
	{
	} //end if
	else if (server && Q_stricmp(cmd, "modelindex") == 0)
	{
		BotDumpModelindex();
	} //end else if
	else if (server && Q_stricmp(cmd, "soundindex") == 0)
	{
		BotDumpSoundindex();
	} //end else if
	else if (server && Q_stricmp(cmd, "imageindex") == 0)
	{
		BotDumpImageindex();
	} //end else if
	else if (server && Q_stricmp(cmd, "inventory") == 0)
	{
		BotDumpInventory();
	} //end else if
	else if (server && Q_stricmp(cmd, "botlibdump") == 0)
	{
		BotLibraryDump();
	} //end else if
	else if (server && Q_stricmp(cmd, "clientdump") == 0)
	{
		BotClientDump();
	} //end else if
	//so the bot can easily change it's name
	else if (ent && Q_stricmp(cmd, "name") == 0)
	{
		memcpy(userinfo, ent->client->pers.userinfo, sizeof(ent->client->pers.userinfo)-1);
		Info_SetValueForKey(userinfo, "name", gi.argv(1));
		ClientUserinfoChanged(ent, userinfo);
	} //end else if
	//so the bot can easily change it's skin
	else if (ent && Q_stricmp(cmd, "skin") == 0)
	{
		memcpy(userinfo, ent->client->pers.userinfo, sizeof(ent->client->pers.userinfo)-1);
		Info_SetValueForKey(userinfo, "skin", gi.argv(1));
		ClientUserinfoChanged(ent, userinfo);
	} //end else if
	//so the bot can easily change it's gender
	else if (ent && Q_stricmp(cmd, "gender") == 0)
	{
		memcpy(userinfo, ent->client->pers.userinfo, sizeof(ent->client->pers.userinfo)-1);
		Info_SetValueForKey(userinfo, "gender", gi.argv(1));
		ClientUserinfoChanged(ent, userinfo);
	} //end else if
	else if (ent && Q_stricmp(cmd, "teamhelp") == 0)
	{
		TeamHelp_f(ent);
	} //end else if
	else if (ent && Q_stricmp(cmd, "teamaccompany") == 0)
	{
		TeamHelp_f(ent);
	} //end else if
	else if (ent && Q_stricmp(cmd, "checkpoint") == 0)
	{
		CheckPoint_f(ent);
	} //end else if
#ifdef BOT_DEBUG
	else if (BotDebugCmd(cmd, ent, server))
	{
	} //end else if
#endif //BOT_DEBUG
	else
	{
		return false;
	} //end else
	return true;
} //end of the function BotCmd

#endif //BOT
