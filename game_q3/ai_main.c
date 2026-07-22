/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//

/*****************************************************************************
 * name:		ai_main.c
 *
 * desc:		Quake3 bot AI
 *
 * $Archive: /MissionPack/code/game/ai_main.c $
 *
 *****************************************************************************/


/* Q2 compatibility: pull in the shared compat shim instead of Q3's own
 * g_local.h/q_shared.h/botlib.h/be_*.h engine-shaped headers (see
 * botlib/ai_q2_compat.h). */
#include "../botlib/ai_q2_compat.h"
//
#include "ai_main.h"
#include "ai_dmq3.h"
#include "ai_chat.h"
#include "ai_cmd.h"
#include "ai_dmnet.h"
#include "ai_vcmd.h"

//
#include "chars.h"
#include "inv.h"
#include "syn.h"

/* botlib/l_precomp.h (pulled in via ai_q2_compat.h) already #defines
 * MAX_PATH as MAX_QPATH (64) for its own purposes; #undef first so this
 * file's own local buffers keep their original, larger 144-char size
 * instead of silently shrinking to 64 (and so the two don't just warn
 * about a differing redefinition instead of properly overriding it). */
#undef MAX_PATH
#define MAX_PATH		144


//bot states
bot_state_t	*botstates[MAX_CLIENTS];
//number of bots
int numbots;
//floating point time
float floattime;
//time to do a regular update
float regularupdate_time;
//
int bot_interbreed;
int bot_interbreedmatchcount;
//
vmCvar_t bot_thinktime;
vmCvar_t bot_memorydump;
vmCvar_t bot_saveroutingcache;
vmCvar_t bot_pause;
vmCvar_t bot_report;
vmCvar_t bot_testsolid;
vmCvar_t bot_testclusters;
vmCvar_t bot_developer_cvar;	/* renamed from bot_developer: botlib/be_interface.h
                                 * already declares a real, unrelated `extern int
                                 * bot_developer` (the LibVar's cached value) --
                                 * harmless while ai_main.c compiled into game.so
                                 * separately from botlib.so, but a real C-symbol
                                 * collision now that both live in botlib.so. The
                                 * LibVar name itself ("bot_developer", the string
                                 * literal below) is unchanged and still the same
                                 * one botlib's own bot_developer int reads from. */
vmCvar_t bot_interbreedchar;
vmCvar_t bot_interbreedbots;
vmCvar_t bot_interbreedcycle;
vmCvar_t bot_interbreedwrite;


void ExitLevel( void );


/* BotAI_Print, BotAI_Trace, BotAI_GetClientState, BotAI_GetEntityState,
 * BotAI_GetSnapshotEntity and BotAI_BotInitialChat were removed here as
 * Q3-engine-shaped dead code superseded by the Q2 adapter's own shim, per
 * this port: their original bodies read gentity_t/g_entities[]/trap_Trace
 * (a Q3 engine trace_t, not Q2's bsp_trace_t) directly, none of which
 * botlib.so has access to across the frozen game<->botlib ABI. Q2-shaped
 * replacements (adapted from game_q2/bl_chat.c, which already solved this
 * for ai_chat.c alone) now live in botlib/ai_q2_shim.c instead. */

/*
==================
BotTestAAS
==================
*/
void BotTestAAS(vec3_t origin) {
	int areanum;
	aas_areainfo_t info;

	trap_Cvar_Update(&bot_testsolid);
	trap_Cvar_Update(&bot_testclusters);
	if (bot_testsolid.integer) {
		if (!trap_AAS_Initialized()) return;
		areanum = BotPointAreaNum(origin);
		if (areanum) BotAI_Print(PRT_MESSAGE, "\remtpy area");
		else BotAI_Print(PRT_MESSAGE, "\r^1SOLID area");
	}
	else if (bot_testclusters.integer) {
		if (!trap_AAS_Initialized()) return;
		areanum = BotPointAreaNum(origin);
		if (!areanum)
			BotAI_Print(PRT_MESSAGE, "\r^1Solid!                              ");
		else {
			trap_AAS_AreaInfo(areanum, &info);
			BotAI_Print(PRT_MESSAGE, "\rarea %d, cluster %d       ", areanum, info.cluster);
		}
	}
}

/*
==================
BotReportStatus
==================
*/
void BotReportStatus(bot_state_t *bs) {
	char goalname[MAX_MESSAGE_SIZE];
	char netname[MAX_MESSAGE_SIZE];
	char *leader, flagstatus[32];
	//
	ClientName(bs->client, netname, sizeof(netname));
	if (Q_stricmp(netname, bs->teamleader) == 0) leader = "L";
	else leader = " ";

	strcpy(flagstatus, "  ");
	if (gametype == GT_CTF) {
		if (BotCTFCarryingFlag(bs)) {
			if (BotTeam(bs) == TEAM_RED) strcpy(flagstatus, S_COLOR_RED"F ");
			else strcpy(flagstatus, S_COLOR_BLUE"F ");
		}
	}
#ifdef MISSIONPACK
	else if (gametype == GT_1FCTF) {
		if (Bot1FCTFCarryingFlag(bs)) {
			if (BotTeam(bs) == TEAM_RED) strcpy(flagstatus, S_COLOR_RED"F ");
			else strcpy(flagstatus, S_COLOR_BLUE"F ");
		}
	}
	else if (gametype == GT_HARVESTER) {
		if (BotHarvesterCarryingCubes(bs)) {
			if (BotTeam(bs) == TEAM_RED) Com_sprintf(flagstatus, sizeof(flagstatus), S_COLOR_RED"%2d", bs->inventory[INVENTORY_REDCUBE]);
			else Com_sprintf(flagstatus, sizeof(flagstatus), S_COLOR_BLUE"%2d", bs->inventory[INVENTORY_BLUECUBE]);
		}
	}
#endif

	switch(bs->ltgtype) {
		case LTG_TEAMHELP:
		{
			EasyClientName(bs->teammate, goalname, sizeof(goalname));
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: helping %s\n", netname, leader, flagstatus, goalname);
			break;
		}
		case LTG_TEAMACCOMPANY:
		{
			EasyClientName(bs->teammate, goalname, sizeof(goalname));
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: accompanying %s\n", netname, leader, flagstatus, goalname);
			break;
		}
		case LTG_DEFENDKEYAREA:
		{
			trap_BotGoalName(bs->teamgoal.number, goalname, sizeof(goalname));
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: defending %s\n", netname, leader, flagstatus, goalname);
			break;
		}
		case LTG_GETITEM:
		{
			trap_BotGoalName(bs->teamgoal.number, goalname, sizeof(goalname));
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: getting item %s\n", netname, leader, flagstatus, goalname);
			break;
		}
		case LTG_KILL:
		{
			ClientName(bs->teamgoal.entitynum, goalname, sizeof(goalname));
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: killing %s\n", netname, leader, flagstatus, goalname);
			break;
		}
		case LTG_CAMP:
		case LTG_CAMPORDER:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: camping\n", netname, leader, flagstatus);
			break;
		}
		case LTG_PATROL:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: patrolling\n", netname, leader, flagstatus);
			break;
		}
		case LTG_GETFLAG:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: capturing flag\n", netname, leader, flagstatus);
			break;
		}
		case LTG_RUSHBASE:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: rushing base\n", netname, leader, flagstatus);
			break;
		}
		case LTG_RETURNFLAG:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: returning flag\n", netname, leader, flagstatus);
			break;
		}
		case LTG_ATTACKENEMYBASE:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: attacking the enemy base\n", netname, leader, flagstatus);
			break;
		}
		case LTG_HARVEST:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: harvesting\n", netname, leader, flagstatus);
			break;
		}
		default:
		{
			BotAI_Print(PRT_MESSAGE, "%-20s%s%s: roaming\n", netname, leader, flagstatus);
			break;
		}
	}
}

/*
==================
BotTeamplayReport
==================
*/
void BotTeamplayReport(void) {
	int i;
	char buf[MAX_INFO_STRING];

	BotAI_Print(PRT_MESSAGE, S_COLOR_RED"RED\n");
	for (i = 0; i < maxclients && i < MAX_CLIENTS; i++) {
		//
		if ( !botstates[i] || !botstates[i]->inuse ) continue;
		//
		trap_GetConfigstring(CS_PLAYERS+i, buf, sizeof(buf));
		//if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) continue;
		//skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_RED) {
			BotReportStatus(botstates[i]);
		}
	}
	BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE"BLUE\n");
	for (i = 0; i < maxclients && i < MAX_CLIENTS; i++) {
		//
		if ( !botstates[i] || !botstates[i]->inuse ) continue;
		//
		trap_GetConfigstring(CS_PLAYERS+i, buf, sizeof(buf));
		//if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n"))) continue;
		//skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_BLUE) {
			BotReportStatus(botstates[i]);
		}
	}
}

/*
==================
BotSetInfoConfigString
==================
*/
void BotSetInfoConfigString(bot_state_t *bs) {
	char goalname[MAX_MESSAGE_SIZE];
	char netname[MAX_MESSAGE_SIZE];
	char action[MAX_MESSAGE_SIZE];
	char *leader, carrying[32], *cs;
	bot_goal_t goal;
	//
	ClientName(bs->client, netname, sizeof(netname));
	if (Q_stricmp(netname, bs->teamleader) == 0) leader = "L";
	else leader = " ";

	strcpy(carrying, "  ");
	if (gametype == GT_CTF) {
		if (BotCTFCarryingFlag(bs)) {
			strcpy(carrying, "F ");
		}
	}
#ifdef MISSIONPACK
	else if (gametype == GT_1FCTF) {
		if (Bot1FCTFCarryingFlag(bs)) {
			strcpy(carrying, "F ");
		}
	}
	else if (gametype == GT_HARVESTER) {
		if (BotHarvesterCarryingCubes(bs)) {
			if (BotTeam(bs) == TEAM_RED) Com_sprintf(carrying, sizeof(carrying), "%2d", bs->inventory[INVENTORY_REDCUBE]);
			else Com_sprintf(carrying, sizeof(carrying), "%2d", bs->inventory[INVENTORY_BLUECUBE]);
		}
	}
#endif

	switch(bs->ltgtype) {
		case LTG_TEAMHELP:
		{
			EasyClientName(bs->teammate, goalname, sizeof(goalname));
			Com_sprintf(action, sizeof(action), "helping %s", goalname);
			break;
		}
		case LTG_TEAMACCOMPANY:
		{
			EasyClientName(bs->teammate, goalname, sizeof(goalname));
			Com_sprintf(action, sizeof(action), "accompanying %s", goalname);
			break;
		}
		case LTG_DEFENDKEYAREA:
		{
			trap_BotGoalName(bs->teamgoal.number, goalname, sizeof(goalname));
			Com_sprintf(action, sizeof(action), "defending %s", goalname);
			break;
		}
		case LTG_GETITEM:
		{
			trap_BotGoalName(bs->teamgoal.number, goalname, sizeof(goalname));
			Com_sprintf(action, sizeof(action), "getting item %s", goalname);
			break;
		}
		case LTG_KILL:
		{
			ClientName(bs->teamgoal.entitynum, goalname, sizeof(goalname));
			Com_sprintf(action, sizeof(action), "killing %s", goalname);
			break;
		}
		case LTG_CAMP:
		case LTG_CAMPORDER:
		{
			Com_sprintf(action, sizeof(action), "camping");
			break;
		}
		case LTG_PATROL:
		{
			Com_sprintf(action, sizeof(action), "patrolling");
			break;
		}
		case LTG_GETFLAG:
		{
			Com_sprintf(action, sizeof(action), "capturing flag");
			break;
		}
		case LTG_RUSHBASE:
		{
			Com_sprintf(action, sizeof(action), "rushing base");
			break;
		}
		case LTG_RETURNFLAG:
		{
			Com_sprintf(action, sizeof(action), "returning flag");
			break;
		}
		case LTG_ATTACKENEMYBASE:
		{
			Com_sprintf(action, sizeof(action), "attacking the enemy base");
			break;
		}
		case LTG_HARVEST:
		{
			Com_sprintf(action, sizeof(action), "harvesting");
			break;
		}
		default:
		{
			trap_BotGetTopGoal(bs->gs, &goal);
			trap_BotGoalName(goal.number, goalname, sizeof(goalname));
			Com_sprintf(action, sizeof(action), "roaming %s", goalname);
			break;
		}
	}
  	cs = va("l\\%s\\c\\%s\\a\\%s",
				leader,
				carrying,
				action);
  	trap_SetConfigstring (CS_BOTINFO + bs->client, cs);
}

/*
==============
BotUpdateInfoConfigStrings
==============
*/
void BotUpdateInfoConfigStrings(void) {
	int i;
	char buf[MAX_INFO_STRING];

	for (i = 0; i < maxclients && i < MAX_CLIENTS; i++) {
		//
		if ( !botstates[i] || !botstates[i]->inuse )
			continue;
		//
		trap_GetConfigstring(CS_PLAYERS+i, buf, sizeof(buf));
		//if no config string or no name
		if (!strlen(buf) || !strlen(Info_ValueForKey(buf, "n")))
			continue;
		BotSetInfoConfigString(botstates[i]);
	}
}

/*
==============
BotInterbreedBots
==============
*/
void BotInterbreedBots(void) {
	float ranks[MAX_CLIENTS];
	int parent1, parent2, child;
	int i;

	// get rankings for all the bots
	for (i = 0; i < MAX_CLIENTS; i++) {
		if ( botstates[i] && botstates[i]->inuse ) {
			ranks[i] = botstates[i]->num_kills * 2 - botstates[i]->num_deaths;
		}
		else {
			ranks[i] = -1;
		}
	}

	if (trap_GeneticParentsAndChildSelection(MAX_CLIENTS, ranks, &parent1, &parent2, &child)) {
		trap_BotInterbreedGoalFuzzyLogic(botstates[parent1]->gs, botstates[parent2]->gs, botstates[child]->gs);
		trap_BotMutateGoalFuzzyLogic(botstates[child]->gs, 1);
	}
	// reset the kills and deaths
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (botstates[i] && botstates[i]->inuse) {
			botstates[i]->num_kills = 0;
			botstates[i]->num_deaths = 0;
		}
	}
}

/*
==============
BotWriteInterbreeded
==============
*/
void BotWriteInterbreeded(char *filename) {
	float rank, bestrank;
	int i, bestbot;

	bestrank = 0;
	bestbot = -1;
	// get the best bot
	for (i = 0; i < MAX_CLIENTS; i++) {
		if ( botstates[i] && botstates[i]->inuse ) {
			rank = botstates[i]->num_kills * 2 - botstates[i]->num_deaths;
		}
		else {
			rank = -1;
		}
		if (rank > bestrank) {
			bestrank = rank;
			bestbot = i;
		}
	}
	if (bestbot >= 0) {
		//write out the new goal fuzzy logic
		trap_BotSaveGoalFuzzyLogic(botstates[bestbot]->gs, filename);
	}
}

/*
==============
BotInterbreedEndMatch

add link back into ExitLevel?
==============
*/
void BotInterbreedEndMatch(void) {

	if (!bot_interbreed) return;
	bot_interbreedmatchcount++;
	if (bot_interbreedmatchcount >= bot_interbreedcycle.integer) {
		bot_interbreedmatchcount = 0;
		//
		trap_Cvar_Update(&bot_interbreedwrite);
		if (strlen(bot_interbreedwrite.string)) {
			BotWriteInterbreeded(bot_interbreedwrite.string);
			trap_Cvar_Set("bot_interbreedwrite", "");
		}
		BotInterbreedBots();
	}
}

/*
==============
BotInterbreeding
==============
*/
void BotInterbreeding(void) {
	int i;

	trap_Cvar_Update(&bot_interbreedchar);
	if (!strlen(bot_interbreedchar.string)) return;
	//make sure we are in tournament mode
	if (gametype != GT_TOURNAMENT) {
		trap_Cvar_Set("g_gametype", va("%d", GT_TOURNAMENT));
		ExitLevel();
		return;
	}
	//shutdown all the bots
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (botstates[i] && botstates[i]->inuse) {
			BotAIShutdownClient(botstates[i]->client, false);
		}
	}
	//make sure all item weight configs are reloaded and Not shared
	trap_BotLibVarSet("bot_reloadcharacters", "1");
	//add a number of bots using the desired bot character
	for (i = 0; i < bot_interbreedbots.integer; i++) {
		trap_SendConsoleCommand( EXEC_INSERT, va("addbot %s 4 free %i %s%d\n",
						bot_interbreedchar.string, i * 50, bot_interbreedchar.string, i) );
	}
	//
	trap_Cvar_Set("bot_interbreedchar", "");
	bot_interbreed = true;
}

/*
==============
BotEntityInfo

Q2 port fix: every real call site here passes a Q3-style client number
(bs->enemy/bs->teammate/bs->lead_teammate/bs->entitynum/a loop variable
ranging [0, maxclients)), NOT a real Q2 entity number -- translate via
the same client+1 convention Q2BotUpdateClient/Q2_ClientsOnSameTeam
already use (see ai_q2_compat.h's Q2_ClientNumToEntityNum for the full
rationale). Values that are already genuinely-native Q2 entity numbers
(non-player entities, always > maxclients per Q2's own invariant) pass
through unchanged, so this is also correct, unmodified, for the many
callers that pass an item/mover bot_goal_t's entitynum.

NOTE: moveresult->blockentity (ai_dmq3.c's BotAIBlocked) is the one
confirmed exception -- it can genuinely be a low-numbered, ALREADY-NATIVE
player edict number (from a real obstruction trace), so that one call
site intentionally bypasses this function and calls trap_AAS_EntityInfo
directly instead of going through the translation below.
==============
*/
void BotEntityInfo(int entnum, aas_entityinfo_t *info) {
	trap_AAS_EntityInfo(Q2_ClientNumToEntityNum(entnum), info);
}

/*
==============
NumBots
==============
*/
int NumBots(void) {
	return numbots;
}

/*
==============
BotTeamLeader
==============
*/
int BotTeamLeader(bot_state_t *bs) {
	int leader;

	leader = ClientFromName(bs->teamleader);
	if (leader < 0) return false;
	if (!botstates[leader] || !botstates[leader]->inuse) return false;
	return true;
}

/*
==============
AngleDifference
==============
*/
float AngleDifference(float ang1, float ang2) {
	float diff;

	diff = ang1 - ang2;
	if (ang1 > ang2) {
		if (diff > 180.0) diff -= 360.0;
	}
	else {
		if (diff < -180.0) diff += 360.0;
	}
	return diff;
}

/*
==============
BotChangeViewAngle
==============
*/
float BotChangeViewAngle(float angle, float ideal_angle, float speed) {
	float move;

	angle = AngleMod(angle);
	ideal_angle = AngleMod(ideal_angle);
	if (angle == ideal_angle) return angle;
	move = ideal_angle - angle;
	if (ideal_angle > angle) {
		if (move > 180.0) move -= 360.0;
	}
	else {
		if (move < -180.0) move += 360.0;
	}
	if (move > 0) {
		if (move > speed) move = speed;
	}
	else {
		if (move < -speed) move = -speed;
	}
	return AngleMod(angle + move);
}

/*
==============
BotChangeViewAngles
==============
*/
void BotChangeViewAngles(bot_state_t *bs, float thinktime) {
	float diff, factor, maxchange, anglespeed, disired_speed;
	int i;

	if (bs->ideal_viewangles[PITCH] > 180) bs->ideal_viewangles[PITCH] -= 360;
	//
	if (bs->enemy >= 0) {
		factor = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_VIEW_FACTOR, 0.01f, 1);
		maxchange = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_VIEW_MAXCHANGE, 1, 1800);
	}
	else {
		factor = 0.05f;
		maxchange = 360;
	}
	if (maxchange < 240) maxchange = 240;
	maxchange *= thinktime;
	for (i = 0; i < 2; i++) {
		//
		if (bot_challenge.integer) {
			//smooth slowdown view model
			diff = abs(AngleDifference(bs->viewangles[i], bs->ideal_viewangles[i]));
			anglespeed = diff * factor;
			if (anglespeed > maxchange) anglespeed = maxchange;
			bs->viewangles[i] = BotChangeViewAngle(bs->viewangles[i],
											bs->ideal_viewangles[i], anglespeed);
		}
		else {
			//over reaction view model
			bs->viewangles[i] = AngleMod(bs->viewangles[i]);
			bs->ideal_viewangles[i] = AngleMod(bs->ideal_viewangles[i]);
			diff = AngleDifference(bs->viewangles[i], bs->ideal_viewangles[i]);
			disired_speed = diff * factor;
			bs->viewanglespeed[i] += (bs->viewanglespeed[i] - disired_speed);
			if (bs->viewanglespeed[i] > 180) bs->viewanglespeed[i] = maxchange;
			if (bs->viewanglespeed[i] < -180) bs->viewanglespeed[i] = -maxchange;
			anglespeed = bs->viewanglespeed[i];
			if (anglespeed > maxchange) anglespeed = maxchange;
			if (anglespeed < -maxchange) anglespeed = -maxchange;
			bs->viewangles[i] += anglespeed;
			bs->viewangles[i] = AngleMod(bs->viewangles[i]);
			//demping
			bs->viewanglespeed[i] *= 0.45 * (1 - factor);
		}
		//BotAI_Print(PRT_MESSAGE, "ideal_angles %f %f\n", bs->ideal_viewangles[0], bs->ideal_viewangles[1], bs->ideal_viewangles[2]);`
		//bs->viewangles[i] = bs->ideal_viewangles[i];
	}
	//bs->viewangles[PITCH] = 0;
	if (bs->viewangles[PITCH] > 180) bs->viewangles[PITCH] -= 360;
	//elementary action: view
	trap_EA_View(bs->client, bs->viewangles);
}

/* BotInputToUserCommand and BotUpdateInput were removed here as
 * Q3-engine-shaped dead code superseded by the Q2 adapter's own frame
 * driver, per this port: they convert a bot_input_t into a Q3 usercmd_t
 * and hand it to trap_BotUserCommand, duplicating what
 * game_q2/g_main.c's existing frame loop and be_interface_q2.c's own
 * Q2BotStartFrame/Q2BotUpdateEntity/Q2BotUpdateClient/EA_GetInput ->
 * q2_bot_input_t translation already do; running both would double-drive
 * entity updates. Confirmed dead: BotInputToUserCommand's only caller was
 * BotUpdateInput, and BotUpdateInput's only caller was BotAIStartFrame
 * (removed below for the same reason). */

/*
==============
BotAIRegularUpdate
==============
*/
void BotAIRegularUpdate(void) {
	if (regularupdate_time < FloatTime()) {
		trap_BotUpdateEntityItems();
		regularupdate_time = FloatTime() + 0.3;
	}
}

/*
==============
RemoveColorEscapeSequences
==============
*/
void RemoveColorEscapeSequences( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if (Q_IsColorString(&text[i])) {
			i++;
			continue;
		}
		if (text[i] > 0x7E)
			continue;
		text[l++] = text[i];
	}
	text[l] = '\0';
}

/*
==============
BotAI
==============
*/
int BotAI(int client, float thinktime) {
	bot_state_t *bs;
	char buf[1024], *args;
	int j;

	trap_EA_ResetInput(client);
	//
	bs = botstates[client];
	if (!bs || !bs->inuse) {
		BotAI_Print(PRT_FATAL, "BotAI: client %d is not setup\n", client);
		return false;
	}

	//retrieve the current client state
	BotAI_GetClientState( client, &bs->cur_ps );

	//retrieve any waiting server commands
	while( trap_BotGetServerCommand(client, buf, sizeof(buf)) ) {
		//have buf point to the command and args to the command arguments
		args = strchr( buf, ' ');
		if (!args) continue;
		*args++ = '\0';

		//remove color espace sequences from the arguments
		RemoveColorEscapeSequences( args );

		if (!Q_stricmp(buf, "cp "))
			{ /*CenterPrintf*/ }
		else if (!Q_stricmp(buf, "cs"))
			{ /*ConfigStringModified*/ }
		else if (!Q_stricmp(buf, "print")) {
			//remove first and last quote from the chat message
			memmove(args, args+1, strlen(args));
			args[strlen(args)-1] = '\0';
			trap_BotQueueConsoleMessage(bs->cs, CMS_NORMAL, args);
		}
		else if (!Q_stricmp(buf, "chat")) {
			//remove first and last quote from the chat message
			memmove(args, args+1, strlen(args));
			args[strlen(args)-1] = '\0';
			trap_BotQueueConsoleMessage(bs->cs, CMS_CHAT, args);
		}
		else if (!Q_stricmp(buf, "tchat")) {
			//remove first and last quote from the chat message
			memmove(args, args+1, strlen(args));
			args[strlen(args)-1] = '\0';
			trap_BotQueueConsoleMessage(bs->cs, CMS_CHAT, args);
		}
#ifdef MISSIONPACK
		else if (!Q_stricmp(buf, "vchat")) {
			BotVoiceChatCommand(bs, SAY_ALL, args);
		}
		else if (!Q_stricmp(buf, "vtchat")) {
			BotVoiceChatCommand(bs, SAY_TEAM, args);
		}
		else if (!Q_stricmp(buf, "vtell")) {
			BotVoiceChatCommand(bs, SAY_TELL, args);
		}
#endif
		else if (!Q_stricmp(buf, "scores"))
			{ /*FIXME: parse scores?*/ }
		else if (!Q_stricmp(buf, "clientLevelShot"))
			{ /*ignore*/ }
	}
	//add the delta angles to the bot's current view angles
	for (j = 0; j < 3; j++) {
		bs->viewangles[j] = AngleMod(bs->viewangles[j] + SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
	}
	//increase the local time of the bot
	bs->ltime += thinktime;
	//
	bs->thinktime = thinktime;
	//origin of the bot
	VectorCopy(bs->cur_ps.origin, bs->origin);
	//eye coordinates of the bot
	VectorCopy(bs->cur_ps.origin, bs->eye);
	bs->eye[2] += bs->cur_ps.viewheight;
	//get the area the bot is in
	bs->areanum = BotPointAreaNum(bs->origin);
	//the real AI
	BotDeathmatchAI(bs, thinktime);
	//set the weapon selection every AI frame
	trap_EA_SelectWeapon(bs->client, bs->weaponnum);
	//subtract the delta angles
	for (j = 0; j < 3; j++) {
		bs->viewangles[j] = AngleMod(bs->viewangles[j] - SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
	}
	//everything was ok
	return true;
}

/*
==============
BotUpdateInput
==============
*/
void BotUpdateInput(bot_state_t *bs, float thinktime) {
	int j;

	//add the delta angles to the bot's current view angles
	for (j = 0; j < 3; j++) {
		bs->viewangles[j] = AngleMod(bs->viewangles[j] + SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
	}
	//change the bot view angles
	BotChangeViewAngles(bs, thinktime);
	//subtract the delta angles
	for (j = 0; j < 3; j++) {
		bs->viewangles[j] = AngleMod(bs->viewangles[j] - SHORT2ANGLE(bs->cur_ps.delta_angles[j]));
	}
}

/*
==================
BotScheduleBotThink
==================
*/
void BotScheduleBotThink(void) {
	int i, botnum;

	botnum = 0;

	for( i = 0; i < MAX_CLIENTS; i++ ) {
		if( !botstates[i] || !botstates[i]->inuse ) {
			continue;
		}
		//initialize the bot think residual time
		botstates[i]->botthink_residual = bot_thinktime.integer * botnum / numbots;
		botnum++;
	}
}

/*
==============
BotWriteSessionData
==============
*/
void BotWriteSessionData(bot_state_t *bs) {
	const char	*s;
	const char	*var;

	s = va(
			"%i %i %i %i %i %i %i %i"
			" %f %f %f"
			" %f %f %f"
			" %f %f %f",
		bs->lastgoal_decisionmaker,
		bs->lastgoal_ltgtype,
		bs->lastgoal_teammate,
		bs->lastgoal_teamgoal.areanum,
		bs->lastgoal_teamgoal.entitynum,
		bs->lastgoal_teamgoal.flags,
		bs->lastgoal_teamgoal.iteminfo,
		bs->lastgoal_teamgoal.number,
		bs->lastgoal_teamgoal.origin[0],
		bs->lastgoal_teamgoal.origin[1],
		bs->lastgoal_teamgoal.origin[2],
		bs->lastgoal_teamgoal.mins[0],
		bs->lastgoal_teamgoal.mins[1],
		bs->lastgoal_teamgoal.mins[2],
		bs->lastgoal_teamgoal.maxs[0],
		bs->lastgoal_teamgoal.maxs[1],
		bs->lastgoal_teamgoal.maxs[2]
		);

	var = va( "botsession%i", bs->client );

	trap_Cvar_Set( var, s );
}

/*
==============
BotReadSessionData
==============
*/
void BotReadSessionData(bot_state_t *bs) {
	char	s[MAX_STRING_CHARS];
	const char	*var;

	var = va( "botsession%i", bs->client );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf(s,
			"%i %i %i %i %i %i %i %i"
			" %f %f %f"
			" %f %f %f"
			" %f %f %f",
		&bs->lastgoal_decisionmaker,
		&bs->lastgoal_ltgtype,
		&bs->lastgoal_teammate,
		&bs->lastgoal_teamgoal.areanum,
		&bs->lastgoal_teamgoal.entitynum,
		&bs->lastgoal_teamgoal.flags,
		&bs->lastgoal_teamgoal.iteminfo,
		&bs->lastgoal_teamgoal.number,
		&bs->lastgoal_teamgoal.origin[0],
		&bs->lastgoal_teamgoal.origin[1],
		&bs->lastgoal_teamgoal.origin[2],
		&bs->lastgoal_teamgoal.mins[0],
		&bs->lastgoal_teamgoal.mins[1],
		&bs->lastgoal_teamgoal.mins[2],
		&bs->lastgoal_teamgoal.maxs[0],
		&bs->lastgoal_teamgoal.maxs[1],
		&bs->lastgoal_teamgoal.maxs[2]
		);
}

/*
==============
BotAISetupClient
==============
*/
int BotAISetupClient(int client, struct bot_settings_s *settings, qboolean restart) {
	char filename[MAX_PATH], name[MAX_PATH], gender[MAX_PATH];
	bot_state_t *bs;
	int errnum;

	if (!botstates[client]) botstates[client] = G_Alloc(sizeof(bot_state_t));
	bs = botstates[client];

	if (bs && bs->inuse) {
		BotAI_Print(PRT_FATAL, "BotAISetupClient: client %d already setup\n", client);
		return false;
	}

	if (!trap_AAS_Initialized()) {
		BotAI_Print(PRT_FATAL, "AAS not initialized\n");
		return false;
	}

	//load the bot character
	bs->character = trap_BotLoadCharacter(settings->characterfile, settings->skill);
	if (!bs->character) {
		BotAI_Print(PRT_FATAL, "couldn't load skill %f from %s\n", settings->skill, settings->characterfile);
		return false;
	}
	//copy the settings
	memcpy(&bs->settings, settings, sizeof(bot_settings_t));
	//allocate a goal state
	bs->gs = trap_BotAllocGoalState(client);
	//load the item weights
	trap_Characteristic_String(bs->character, CHARACTERISTIC_ITEMWEIGHTS, filename, MAX_PATH);
	errnum = trap_BotLoadItemWeights(bs->gs, filename);
	if (errnum != BLERR_NOERROR) {
		trap_BotFreeGoalState(bs->gs);
		return false;
	}
	//allocate a weapon state
	bs->ws = trap_BotAllocWeaponState();
	//load the weapon weights
	trap_Characteristic_String(bs->character, CHARACTERISTIC_WEAPONWEIGHTS, filename, MAX_PATH);
	errnum = trap_BotLoadWeaponWeights(bs->ws, filename);
	if (errnum != BLERR_NOERROR) {
		trap_BotFreeGoalState(bs->gs);
		trap_BotFreeWeaponState(bs->ws);
		return false;
	}
	//allocate a chat state
	bs->cs = trap_BotAllocChatState();
	//load the chat file
	trap_Characteristic_String(bs->character, CHARACTERISTIC_CHAT_FILE, filename, MAX_PATH);
	trap_Characteristic_String(bs->character, CHARACTERISTIC_CHAT_NAME, name, MAX_PATH);
	errnum = trap_BotLoadChatFile(bs->cs, filename, name);
	if (errnum != BLERR_NOERROR) {
		trap_BotFreeChatState(bs->cs);
		trap_BotFreeGoalState(bs->gs);
		trap_BotFreeWeaponState(bs->ws);
		return false;
	}
	//get the gender characteristic
	trap_Characteristic_String(bs->character, CHARACTERISTIC_GENDER, gender, MAX_PATH);
	//set the chat gender
	if (*gender == 'f' || *gender == 'F') trap_BotSetChatGender(bs->cs, CHAT_GENDERFEMALE);
	else if (*gender == 'm' || *gender == 'M') trap_BotSetChatGender(bs->cs, CHAT_GENDERMALE);
	else trap_BotSetChatGender(bs->cs, CHAT_GENDERLESS);

	bs->inuse = true;
	bs->client = client;
	bs->entitynum = client;
	bs->setupcount = 4;
	bs->entergame_time = FloatTime();
	bs->ms = trap_BotAllocMoveState();
	bs->walker = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_WALKER, 0, 1);
	numbots++;

	if (trap_Cvar_VariableIntegerValue("bot_testichat")) {
		trap_BotLibVarSet("bot_testichat", "1");
		BotChatTest(bs);
	}
	//NOTE: reschedule the bot thinking
	BotScheduleBotThink();
	//if interbreeding start with a mutation
	if (bot_interbreed) {
		trap_BotMutateGoalFuzzyLogic(bs->gs, 1);
	}
	// if we kept the bot client
	if (restart) {
		BotReadSessionData(bs);
	}
	//bot has been setup succesfully
	return true;
}

/*
==============
BotAIShutdownClient
==============
*/
int BotAIShutdownClient(int client, qboolean restart) {
	bot_state_t *bs;

	bs = botstates[client];
	if (!bs || !bs->inuse) {
		//BotAI_Print(PRT_ERROR, "BotAIShutdownClient: client %d already shutdown\n", client);
		return false;
	}

	if (restart) {
		BotWriteSessionData(bs);
	}

	if (BotChat_ExitGame(bs)) {
		trap_BotEnterChat(bs->cs, bs->client, CHAT_ALL);
	}

	trap_BotFreeMoveState(bs->ms);
	//free the goal state`			
	trap_BotFreeGoalState(bs->gs);
	//free the chat file
	trap_BotFreeChatState(bs->cs);
	//free the weapon weights
	trap_BotFreeWeaponState(bs->ws);
	//free the bot character
	trap_BotFreeCharacter(bs->character);
	//
	BotFreeWaypoints(bs->checkpoints);
	BotFreeWaypoints(bs->patrolpoints);
	//clear activate goal stack
	BotClearActivateGoalStack(bs);
	//clear the bot state
	memset(bs, 0, sizeof(bot_state_t));
	//set the inuse flag to false
	bs->inuse = false;
	//there's one bot less
	numbots--;
	//everything went ok
	return true;
}

/*
==============
BotResetState

called when a bot enters the intermission or observer mode and
when the level is changed
==============
*/
void BotResetState(bot_state_t *bs) {
	int client, entitynum, inuse;
	int movestate, goalstate, chatstate, weaponstate;
	bot_settings_t settings;
	int character;
	playerState_t ps;							//current player state
	float entergame_time;

	//save some things that should not be reset here
	memcpy(&settings, &bs->settings, sizeof(bot_settings_t));
	memcpy(&ps, &bs->cur_ps, sizeof(playerState_t));
	inuse = bs->inuse;
	client = bs->client;
	entitynum = bs->entitynum;
	character = bs->character;
	movestate = bs->ms;
	goalstate = bs->gs;
	chatstate = bs->cs;
	weaponstate = bs->ws;
	entergame_time = bs->entergame_time;
	//free checkpoints and patrol points
	BotFreeWaypoints(bs->checkpoints);
	BotFreeWaypoints(bs->patrolpoints);
	//reset the whole state
	memset(bs, 0, sizeof(bot_state_t));
	//copy back some state stuff that should not be reset
	bs->ms = movestate;
	bs->gs = goalstate;
	bs->cs = chatstate;
	bs->ws = weaponstate;
	memcpy(&bs->cur_ps, &ps, sizeof(playerState_t));
	memcpy(&bs->settings, &settings, sizeof(bot_settings_t));
	bs->inuse = inuse;
	bs->client = client;
	bs->entitynum = entitynum;
	bs->character = character;
	bs->entergame_time = entergame_time;
	//reset several states
	if (bs->ms) trap_BotResetMoveState(bs->ms);
	if (bs->gs) trap_BotResetGoalState(bs->gs);
	if (bs->ws) trap_BotResetWeaponState(bs->ws);
	if (bs->gs) trap_BotResetAvoidGoals(bs->gs);
	if (bs->ms) trap_BotResetAvoidReach(bs->ms);
}

/*
==============
BotAILoadMap
==============
*/
int BotAILoadMap( int restart ) {
	int			i;
	vmCvar_t	mapname;

	if (!restart) {
		trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
		trap_BotLibLoadMap( mapname.string );
	}

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (botstates[i] && botstates[i]->inuse) {
			BotResetState( botstates[i] );
			botstates[i]->setupcount = 4;
		}
	}

	BotSetupDeathmatchAI();

	return true;
}

/* BotAIStartFrame was removed here as Q3-engine-shaped dead code
 * superseded by the Q2 adapter's own frame driver, per this port: it
 * duplicates what game_q2/g_main.c's existing frame loop plus
 * be_interface_q2.c's Q2BotStartFrame/Q2BotUpdateEntity/Q2BotUpdateClient
 * already do (bot library frame pump, per-entity botlib updates,
 * per-bot AI scheduling); running both would double-drive entity updates
 * and violate the update-before-AI frame ordering. Its two callees
 * (BotUpdateInput, G_CheckBotSpawn's only call site) are also gone or
 * stubbed accordingly -- see above and botlib/ai_q2_shim.c. */

/*
==============
BotInitLibrary
==============
*/
int BotInitLibrary(void) {
	char buf[144];

	//set the maxclients and maxentities library variables before calling BotSetupLibrary
	trap_Cvar_VariableStringBuffer("sv_maxclients", buf, sizeof(buf));
	if (!strlen(buf)) strcpy(buf, "8");
	trap_BotLibVarSet("maxclients", buf);
	Com_sprintf(buf, sizeof(buf), "%d", MAX_GENTITIES);
	trap_BotLibVarSet("maxentities", buf);
	//bsp checksum
	trap_Cvar_VariableStringBuffer("sv_mapChecksum", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("sv_mapChecksum", buf);
	//maximum number of aas links
	trap_Cvar_VariableStringBuffer("max_aaslinks", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("max_aaslinks", buf);
	//maximum number of items in a level
	trap_Cvar_VariableStringBuffer("max_levelitems", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("max_levelitems", buf);
	//game type
	trap_Cvar_VariableStringBuffer("g_gametype", buf, sizeof(buf));
	if (!strlen(buf)) strcpy(buf, "0");
	trap_BotLibVarSet("g_gametype", buf);
	//bot developer mode and log file
	trap_BotLibVarSet("bot_developer", bot_developer_cvar.string);
	trap_BotLibVarSet("log", buf);
	//no chatting
	trap_Cvar_VariableStringBuffer("bot_nochat", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("nochat", "0");
	//visualize jump pads
	trap_Cvar_VariableStringBuffer("bot_visualizejumppads", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("bot_visualizejumppads", buf);
	//forced clustering calculations
	trap_Cvar_VariableStringBuffer("bot_forceclustering", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("forceclustering", buf);
	//forced reachability calculations
	trap_Cvar_VariableStringBuffer("bot_forcereachability", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("forcereachability", buf);
	//force writing of AAS to file
	trap_Cvar_VariableStringBuffer("bot_forcewrite", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("forcewrite", buf);
	//no AAS optimization
	trap_Cvar_VariableStringBuffer("bot_aasoptimize", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("aasoptimize", buf);
	//
	trap_Cvar_VariableStringBuffer("bot_saveroutingcache", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("saveroutingcache", buf);
	//reload instead of cache bot character files
	trap_Cvar_VariableStringBuffer("bot_reloadcharacters", buf, sizeof(buf));
	if (!strlen(buf)) strcpy(buf, "0");
	trap_BotLibVarSet("bot_reloadcharacters", buf);
	//base directory
	trap_Cvar_VariableStringBuffer("fs_basepath", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("basedir", buf);
	//game directory
	trap_Cvar_VariableStringBuffer("fs_game", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("gamedir", buf);
	//cd directory
	trap_Cvar_VariableStringBuffer("fs_cdpath", buf, sizeof(buf));
	if (strlen(buf)) trap_BotLibVarSet("cddir", buf);
	//
#ifdef MISSIONPACK
	trap_BotLibDefine("MISSIONPACK");
#endif
	//setup the bot library
	return trap_BotLibSetup();
}

/*
==============
BotAISetup
==============
*/
int BotAISetup( int restart ) {
	int			errnum;

	trap_Cvar_Register(&bot_thinktime, "bot_thinktime", "100", CVAR_CHEAT);
	trap_Cvar_Register(&bot_memorydump, "bot_memorydump", "0", CVAR_CHEAT);
	trap_Cvar_Register(&bot_saveroutingcache, "bot_saveroutingcache", "0", CVAR_CHEAT);
	trap_Cvar_Register(&bot_pause, "bot_pause", "0", CVAR_CHEAT);
	trap_Cvar_Register(&bot_report, "bot_report", "0", CVAR_CHEAT);
	trap_Cvar_Register(&bot_testsolid, "bot_testsolid", "0", CVAR_CHEAT);
	trap_Cvar_Register(&bot_testclusters, "bot_testclusters", "0", CVAR_CHEAT);
	trap_Cvar_Register(&bot_developer_cvar, "bot_developer", "0", CVAR_CHEAT);
	trap_Cvar_Register(&bot_interbreedchar, "bot_interbreedchar", "", 0);
	trap_Cvar_Register(&bot_interbreedbots, "bot_interbreedbots", "10", 0);
	trap_Cvar_Register(&bot_interbreedcycle, "bot_interbreedcycle", "20", 0);
	trap_Cvar_Register(&bot_interbreedwrite, "bot_interbreedwrite", "", 0);

	//if the game is restarted for a tournament
	if (restart) {
		return true;
	}

	//initialize the bot states
	memset( botstates, 0, sizeof(botstates) );

	errnum = BotInitLibrary();
	if (errnum != BLERR_NOERROR) return false;
	return true;
}

/*
==============
BotAIShutdown
==============
*/
int BotAIShutdown( int restart ) {

	int i;

	//if the game is restarted for a tournament
	if ( restart ) {
		//shutdown all the bots in the botlib
		for (i = 0; i < MAX_CLIENTS; i++) {
			if (botstates[i] && botstates[i]->inuse) {
				BotAIShutdownClient(botstates[i]->client, restart);
			}
		}
		//don't shutdown the bot library
	}
	else {
		trap_BotLibShutdown();
	}
	return true;
}

