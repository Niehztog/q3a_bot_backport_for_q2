//===========================================================================
//
// Name:			match.c
// Function:		match templates
// Programmer:		Mr Elusive
// Last update:
// Tab Size:		4 (real tabs)
// Notes:			currently maximum of 8 match variables
//                  Q2: id's Quake III Arena templates. The bot AI's console
//                  gets Q2's chat in Q3's EC form (Q2BotConsoleMessage,
//                  botlib/be_interface_q2.c) and Q2's own prints, whose
//                  "entered the game" and CTF flag wording is Q3's. Added: the
//                  CTF message of a flag returned by itself, which Q3 signals
//                  with a sound event instead, and the obituaries, which Q3's
//                  bots never read: Q3 has EV_OBITUARY events.
//===========================================================================

#include "match.h"
#include "mod.h"

// this is rare but people can always fuckup
// don't use EC"(", EC")", EC"[", EC"]" or EC":" inside player names
// don't use EC": " inside map locations

//entered the game message
MTCONTEXT_MISC
{
	//enter game message
	NETNAME, " entered the game" = (MSG_ENTERGAME, 0);
	NETNAME, " is the new team leader" = (MSG_NEWLEADER, 0);
} //end MTCONTEXT_ENTERGAME

//initial team command chat messages
MTCONTEXT_INITIALTEAMCHAT
{
	//help someone (and meet at the rendezvous point)
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": help "|" meet ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM = (MSG_HELP, ST_NEARITEM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": help "|" meet ", TEAMMATE = (MSG_HELP, ST_SOMEWHERE);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " help "|" meet ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM = (MSG_HELP, $evalint(ST_NEARITEM|ST_ADDRESSED));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " help "|" meet ", TEAMMATE = (MSG_HELP, $evalint(ST_SOMEWHERE|ST_ADDRESSED));

	//accompany someone (and meet at the rendezvous point) ("hunk follow me", "hunk go with babe", etc.)
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "accompany "|"go with "|"follow "|"cover "|" protect ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM, " for", TIME = (MSG_ACCOMPANY, $evalint(ST_NEARITEM|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "accompany "|"go with "|"follow "|"cover "|" protect ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM = (MSG_ACCOMPANY, ST_NEARITEM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "accompany "|"go with "|"follow "|"cover "|" protect ", TEAMMATE, " for", TIME = (MSG_ACCOMPANY, $evalint(ST_SOMEWHERE|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "accompany "|"go with "|"follow "|"cover "|" protect ", TEAMMATE = (MSG_ACCOMPANY, ST_SOMEWHERE);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " accompany "|" go with "|" follow "|" cover "|" protect ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM, " for", TIME = (MSG_ACCOMPANY, $evalint(ST_NEARITEM|ST_ADDRESSED|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " accompany "|" go with "|" follow "|" cover "|" protect ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM = (MSG_ACCOMPANY, $evalint(ST_NEARITEM|ST_ADDRESSED));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " accompany "|" go with "|" follow "|" cover "|" protect ", TEAMMATE, " for", TIME = (MSG_ACCOMPANY, $evalint(ST_SOMEWHERE|ST_ADDRESSED|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " accompany "|" go with "|" follow "|" cover "|" protect ", TEAMMATE = (MSG_ACCOMPANY, $evalint(ST_SOMEWHERE|ST_ADDRESSED));

	//teamplay task preference
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to defend" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to defend the ", "Red Flag"|"Blue Flag"|"Red Obelisk"|"Blue Obelisk" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to be ", "on "|"", "defense" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to capture the ", "Red Flag"|"Blue Flag" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to get the ", "Red Flag"|"Blue Flag" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to ", "attack"|"assault" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to ", "attack"|"assault", " the ", "Red Flag"|"Blue Flag"|"Red Obelisk"|"Blue Obelisk" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to harvest", " skulls"|" cubes"|"" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to be ", "on "|"", "offense" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will defend" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will defend", " the ", "Red Obelisk"|"Blue Obelisk" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will be ", "on "|"", "defense" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will not harvest", " skulls"|" cubes"|"" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I am ", "on "|"", "defense" = (MSG_TASKPREFERENCE, ST_DEFENDER);

	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to defend" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to defend the ", "Red Flag"|"Blue Flag"|"Red Obelisk"|"Blue Obelisk" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to be ", "on "|"", "defense" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to capture the ", "Red Flag"|"Blue Flag" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to get the ", "Red Flag"|"Blue Flag" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to ", "attack"|"assault" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to ", "attack"|"assault", " the ", "Red Flag"|"Blue Flag"|"Red Obelisk"|"Blue Obelisk" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to harvest", " skulls"|" cubes"|"" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to be ", "on "|"", "offense" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will ","attack"|"assault" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will ","attack"|"assault", " the ", "Red Obelisk"|"Blue Obelisk" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will be ", "on "|"", "offense" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will harvest", " skulls"|" cubes"|"" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I am ", "on "|"", "offense" = (MSG_TASKPREFERENCE, ST_ATTACKER);

	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to roam" = (MSG_TASKPREFERENCE, ST_ROAMER);

	//get the flag in CTF
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": get ", "the "|"", "blue "|"red "|"enemy "|"", "flag" = (MSG_GETFLAG, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": go get ", "the "|"", "blue "|"red "|"enemy "|"", "flag" = (MSG_GETFLAG, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": capture ", "the "|"", "blue "|"red "|"enemy "|"", "flag" = (MSG_GETFLAG, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": go capture ", "the "|"", "blue "|"red "|"enemy "|"", "flag" = (MSG_GETFLAG, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go get ", "the "|"", "blue "|"red "|"enemy "|"", "flag" = (MSG_GETFLAG, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " get ", "the "|"", "blue "|"red "|"enemy "|"", "flag" = (MSG_GETFLAG, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go capture ", "the "|"", "blue "|"red "|"enemy "|"", "flag" = (MSG_GETFLAG, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " capture ", "the "|"", "blue "|"red "|"enemy "|"", "flag" = (MSG_GETFLAG, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": kill the flag carrier" = (MSG_GETFLAG, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " kill the flag carrier" = (MSG_GETFLAG, ST_ADDRESSED);

	//attack the enemy base
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " attack "|" assault ", "the "|"", "enemy "|"red "|"blue "|"", "base"|"flag"|"obelisk" = (MSG_ATTACKENEMYBASE, ST_ADDRESSED);

	//go harvesting
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " harvest" = (MSG_HARVEST, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go harvesting" = (MSG_HARVEST, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " collect skulls" = (MSG_HARVEST, ST_ADDRESSED);

	//kill someone (NOTE: make sure these are after the get flag match templates because of the "kill"
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": kill ", ENEMY = (MSG_KILL, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " kill ", ENEMY = (MSG_KILL, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": hunt down ", ENEMY = (MSG_KILL, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " hunt down ", ENEMY = (MSG_KILL, ST_ADDRESSED);

	//get item
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": get ", "the "|"", ITEM = (MSG_GETITEM, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": go get ", "the "|"", ITEM = (MSG_GETITEM, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " get ", "the "|"", ITEM = (MSG_GETITEM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go get ", "the "|"", ITEM = (MSG_GETITEM, ST_ADDRESSED);

	//defend/guard a key area
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "defend "|"guard ", "the "|"checkpoint "|"waypoint "|"", KEYAREA, " for", TIME = (MSG_DEFENDKEYAREA, ST_TIME);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "defend "|"guard ", "the "|"checkpoint "|"waypoint "|"", KEYAREA = (MSG_DEFENDKEYAREA, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " defend "|" guard ", "the "|"checkpoint "|"waypoint "|"", KEYAREA, " for", TIME = (MSG_DEFENDKEYAREA, $evalint(ST_ADDRESSED|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " defend "|" guard ", "the "|"checkpoint "|"waypoint "|"", KEYAREA = (MSG_DEFENDKEYAREA, ST_ADDRESSED);

	//camp somewhere ("hunk camp here", "hunk camp there", "hunk camp near the rl", etc.)
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " camp ", "there "|"over there ", " for", TIME = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_TIME|ST_THERE));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " camp ", "there"|"over there" = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_THERE));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " camp ", "here"|"over here ", " for", TIME = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_TIME|ST_HERE));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " camp ", "here"|"over here" = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_HERE));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " camp ", "near "|"at "|"", "the "|"checkpoint "|"waypoint "|"", KEYAREA, " for", TIME = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_NEARITEM|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " camp ", "near "|"at "|"", "the "|"checkpoint "|"waypoint "|"", KEYAREA = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_NEARITEM));
	//go to (same as camp)
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go to ", "the "|"checkpoint "|"waypoint "|"", KEYAREA = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_NEARITEM));

	//rush to the base in CTF
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " rush base" = (MSG_RUSHBASE, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " rush to base" = (MSG_RUSHBASE, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " rush to the base" = (MSG_RUSHBASE, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go to base" = (MSG_RUSHBASE, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go to the base" = (MSG_RUSHBASE, ST_ADDRESSED);

	//return the flag
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " return the flag" = (MSG_RETURNFLAG, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " return our flag" = (MSG_RETURNFLAG, ST_ADDRESSED);


	//who is the team leader
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": who is ", "the leader"|"the team leader"|"team leader"|"leader","?"|"" = (MSG_WHOISTEAMLAEDER, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": is there a ", "leader"|"team leader","?"|"" = (MSG_WHOISTEAMLAEDER, 0);

	//become the team leader
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " will be ", THE_TEAM, "leader" = (MSG_STARTTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " want to be ", THE_TEAM, "leader" = (MSG_STARTTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " wants to be ", THE_TEAM, "leader" = (MSG_STARTTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " is ", THE_TEAM, "leader" = (MSG_STARTTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " you are ", THE_TEAM, "leader" = (MSG_STARTTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I am ", "the leader"|"the team leader"|"team leader"|"leader" = (MSG_STARTTEAMLEADERSHIP, ST_I);

	//stop being the team leader
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " is not ", THE_TEAM, "leader" = (MSG_STOPTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " does not want to be ", THE_TEAM, "leader" = (MSG_STOPTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " quits being ", THE_TEAM, "leader" = (MSG_STOPTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " stops being ", THE_TEAM, "leader" = (MSG_STOPTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will not be ", THE_TEAM, "leader" = (MSG_STOPTEAMLEADERSHIP, ST_I);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to be ", THE_TEAM, "leader" = (MSG_STOPTEAMLEADERSHIP, ST_I);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I quit being ", THE_TEAM, "leader" = (MSG_STOPTEAMLEADERSHIP, ST_I);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I stop being ", THE_TEAM, "leader" = (MSG_STOPTEAMLEADERSHIP, ST_I);

	//wait for someone
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " wait for me", " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM = (MSG_WAIT, $evalint(ST_NEARITEM|ST_ADDRESSED|ST_I));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " wait for me" = (MSG_WAIT, $evalint(ST_SOMEWHERE|ST_ADDRESSED|ST_I));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " wait for ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM = (MSG_WAIT, $evalint(ST_NEARITEM|ST_ADDRESSED));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " wait for ", TEAMMATE = (MSG_WAIT, $evalint(ST_SOMEWHERE|ST_ADDRESSED));

	//ask what someone/everyone is doing
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " what are you doing", "?"|"" = (MSG_WHATAREYOUDOING, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": what are you doing ", ADDRESSEE, "?"|"" = (MSG_WHATAREYOUDOING, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " report" = (MSG_WHATAREYOUDOING, ST_ADDRESSED);

	//ask the team leader what to do
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": what is my command", "?"|"" = (MSG_WHATISMYCOMMAND, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": what should I do", "?"|"" = (MSG_WHATISMYCOMMAND, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": what am I supposed to do", "?"|"" = (MSG_WHATISMYCOMMAND, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": what is my job", "?"|"" = (MSG_WHATISMYCOMMAND, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " what is my command", "?"|"" = (MSG_WHATISMYCOMMAND, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " what should I do", "?"|"" = (MSG_WHATISMYCOMMAND, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " what am I supposed to do", "?"|"" = (MSG_WHATISMYCOMMAND, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " what is my job", "?"|"" = (MSG_WHATISMYCOMMAND, ST_ADDRESSED);

	//ask where someone or everyone is
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " where are you", "?"|"" = (MSG_WHEREAREYOU, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": where are you ", ADDRESSEE, "?"|"" = (MSG_WHEREAREYOU, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": where is ", ADDRESSEE, "?"|"" = (MSG_WHEREAREYOU, ST_ADDRESSED);

	//join a sub team
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " create team ", TEAMNAME = (MSG_JOINSUBTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " create squad ", TEAMNAME = (MSG_JOINSUBTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " join team ", TEAMNAME = (MSG_JOINSUBTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " join squad ", TEAMNAME = (MSG_JOINSUBTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " you"|" we"|"", " are", " in"|"", " team ", TEAMNAME = (MSG_JOINSUBTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " you"|" we"|"", " are", " in"|"", " squad ", TEAMNAME = (MSG_JOINSUBTEAM, ST_ADDRESSED);

	//leave a sub team
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " leave your team" = (MSG_LEAVESUBTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " leave your squad" = (MSG_LEAVESUBTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " ungroup" = (MSG_LEAVESUBTEAM, ST_ADDRESSED);

	//what team are you in
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " which "|" what ", "team"|"squad", " are you ", "in"|"on", "?"|"" = (MSG_WHICHTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " in"|" on", " which "|" what ", "team"|"squad", " are you ", "?"|"" = (MSG_WHICHTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " what is your ", "team"|"squad","?"|"" = (MSG_WHICHTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " are you ", "in"|"on", " a ", "team"|"squad","?"|"" = (MSG_WHICHTEAM, ST_ADDRESSED);

	//dismiss
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " dismissed" = (MSG_DISMISS, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " dismiss" = (MSG_DISMISS, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " roam" = (MSG_DISMISS, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " stop action" = (MSG_DISMISS, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " cancel order" = (MSG_DISMISS, ST_ADDRESSED);

	//remember checkpoint
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "checkpoint "|"waypoint ", NAME, " is at gps ", POSITION = (MSG_CHECKPOINT, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "checkpoint "|"waypoint ", NAME, " is at ", POSITION = (MSG_CHECKPOINT, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " checkpoint "|" waypoint ", NAME, " is at gps ", POSITION = (MSG_CHECKPOINT, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " checkpoint "|" waypoint ", NAME, " is at ", POSITION = (MSG_CHECKPOINT, ST_ADDRESSED);

	//patrol
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": patrol ", "from "|"", KEYAREA, " for", TIME = (MSG_PATROL, ST_TIME);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": patrol ", "from "|"", KEYAREA = (MSG_PATROL, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " patrol ", "from "|"", KEYAREA, " for", TIME = (MSG_PATROL, $evalint(ST_ADDRESSED|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " patrol ", "from "|"", KEYAREA = (MSG_PATROL, ST_ADDRESSED);

	//create new formation
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " create a new ", FORMATION, " formation" = (MSG_CREATENEWFORMATION, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " we are going to create a new ", FORMATION, " formation" = (MSG_CREATENEWFORMATION, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " we are going to create a new formation called ", FORMATION = (MSG_CREATENEWFORMATION, ST_ADDRESSED);

	//formation position
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " your formation position is ", POSITION, " relative to ", TEAMMATE = (MSG_FORMATIONPOSITION, ST_ADDRESSED);

	//form a known formation
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " form the ", FORMATION, " formation" = (MSG_DOFORMATION, ST_ADDRESSED);

	//the formation intervening space
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " the formation intervening space is ", NUMBER, " meter" = (MSG_FORMATIONSPACE, $evalint(ST_ADDRESSED|ST_METER));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " the formation intervening space is ", NUMBER, " feet" = (MSG_FORMATIONSPACE, $evalint(ST_ADDRESSED|ST_FEET));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " the"|""," follow distance is ", NUMBER, " meter" = (MSG_FORMATIONSPACE, $evalint(ST_ADDRESSED|ST_METER));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " the"|""," follow distance is ", NUMBER, " feet" = (MSG_FORMATIONSPACE, $evalint(ST_ADDRESSED|ST_FEET));

	//lead the way
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": lead the way" = (MSG_LEADTHEWAY, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": lead the way ", ADDRESSEE = (MSG_LEADTHEWAY, $evalint(ST_ADDRESSED));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " lead the way" = (MSG_LEADTHEWAY, $evalint(ST_ADDRESSED));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": lead ", TEAMMATE, " the way ", ADDRESSEE = (MSG_LEADTHEWAY, $evalint(ST_ADDRESSED|ST_SOMEONE));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " lead ", TEAMMATE, " the way" = (MSG_LEADTHEWAY, $evalint(ST_ADDRESSED|ST_SOMEONE));

	// suicide
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": suicide" = (MSG_SUICIDE, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " suicide" = (MSG_SUICIDE, $evalint(ST_ADDRESSED));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": die" = (MSG_SUICIDE, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " die" = (MSG_SUICIDE, $evalint(ST_ADDRESSED));

	// make love
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " fuck "|" make love to ", TEAMMATE = (MSG_MAKELOVE, $evalint(ST_ADDRESSED));

} //end MTCONTEXT_INITIALTEAMCHAT

MTCONTEXT_CTF
{
	NETNAME, " got the ", FLAG, " flag", "!"|"" = (MSG_CTF, ST_GOTFLAG);
	NETNAME, " captured the ", FLAG, " flag", "!"|"" = (MSG_CTF, ST_CAPTUREDFLAG);
	NETNAME, " returned the ", FLAG, " flag", "!"|"" = (MSG_CTF, ST_RETURNEDFLAG);
	"The ", FLAG, " flag has returned", "!"|"" = (MSG_CTF, ST_RETURNEDFLAG);
	//for One Flag CTF
	NETNAME, " got the flag", "!"|"" = (MSG_CTF, ST_1FCTFGOTFLAG);
} //end MTCONTEXT_CTF

MTCONTEXT_TIME
{
	TIME, " minute"|" min","s"|"" = (MSG_MINUTES, 0);
	TIME, " second"|" sec","s"|"" = (MSG_SECONDS, 0);
	"ever" = (MSG_FOREVER, 0);
	" a long time" = (MSG_FORALONGTIME, 0);
	" a while" = (MSG_FORAWHILE, 0);
} //end MTCONTEXT_TIME

MTCONTEXT_PATROLKEYAREA
{
	"the "|"checkpoint "|"waypoint "|"", KEYAREA, " to "|" and ", MORE = (MSG_PATROLKEYAREA, ST_MORE);
	"the "|"checkpoint "|"waypoint "|"", KEYAREA, " and loop"|" and back", " to the start"|"" = (MSG_PATROLKEYAREA, ST_BACK);
	"the "|"checkpoint "|"waypoint "|"", KEYAREA, " and reverse" = (MSG_PATROLKEYAREA, ST_REVERSE);
	"the "|"checkpoint "|"waypoint "|"", KEYAREA = (MSG_PATROLKEYAREA, 0);
} //end MTCONTEXT_PATROL

MTCONTEXT_TEAMMATE
{
	"me"|"I" = (MSG_ME, 0);
} //end MTCONTEXT_TEAMMATE

MTCONTEXT_ADDRESSEE
{
	"everyone"|"everybody" = (MSG_EVERYONE, 0);
	TEAMMATE, " and "|", "|","|" ,", MORE = (MSG_MULTIPLENAMES, 0);
	TEAMMATE = (MSG_NAME, 0);
} //end MTCONTEXT_ADDRESSEE

MTCONTEXT_REPLYCHAT
{
	EC"(", NETNAME, EC")", PLACE, EC": ", MESSAGE = (MSG_CHATTEAM, ST_TEAM);
	EC"[", NETNAME, EC"]", PLACE, EC": ", MESSAGE = (MSG_CHATTELL, ST_TEAM);
	NETNAME, EC": ", MESSAGE = (MSG_CHATALL, 0);
} //end MTCONTEXT_REPLYCHAT

//the obituaries, as Gladiator's botlib read them: Q2 tells the bot library of
//no death, it prints them (game_q2/p_client.c ClientObituary). The adapter
//makes Q3's EV_OBITUARY of them (Q2CheckObituary, botlib/be_interface_q2.c).
//Generated from ClientObituary by tools/obituaries.py, which also checks
//that every line the game prints matches the template that means it.
MTCONTEXT_CLIENTOBITUARY
{
	//killed by the world: MOD_SUICIDE
	VICTIM, " commits suicide." = (MSG_WORLDDEATH, MOD_SUICIDE);
	VICTIM, " takes the easy way out." = (MSG_WORLDDEATH, MOD_SUICIDE);
	VICTIM, " has fragged himself." = (MSG_WORLDDEATH, MOD_SUICIDE);
	VICTIM, " took his own life." = (MSG_WORLDDEATH, MOD_SUICIDE);
	VICTIM, " can be scraped off the pavement." = (MSG_WORLDDEATH, MOD_SUICIDE);
	VICTIM, " has fragged herself." = (MSG_WORLDDEATH, MOD_SUICIDE);
	VICTIM, " took her own life." = (MSG_WORLDDEATH, MOD_SUICIDE);
	VICTIM, " has fragged itself." = (MSG_WORLDDEATH, MOD_SUICIDE);
	VICTIM, " took it's own life." = (MSG_WORLDDEATH, MOD_SUICIDE);
	//killed by the world: MOD_FALLING
	VICTIM, " cratered." = (MSG_WORLDDEATH, MOD_FALLING);
	VICTIM, " discovers the effects of gravity." = (MSG_WORLDDEATH, MOD_FALLING);
	//killed by the world: MOD_CRUSH
	VICTIM, " was squished." = (MSG_WORLDDEATH, MOD_CRUSH);
	VICTIM, " was squeezed like a ripe grape." = (MSG_WORLDDEATH, MOD_CRUSH);
	VICTIM, " turned to juice." = (MSG_WORLDDEATH, MOD_CRUSH);
	//killed by the world: MOD_WATER
	VICTIM, " sank like a rock." = (MSG_WORLDDEATH, MOD_WATER);
	VICTIM, " tried unsuccesfully to breathe water." = (MSG_WORLDDEATH, MOD_WATER);
	VICTIM, " tried to immitate a fish." = (MSG_WORLDDEATH, MOD_WATER);
	VICTIM, " must learn when to breathe." = (MSG_WORLDDEATH, MOD_WATER);
	VICTIM, " thought he didn't need a rebreather." = (MSG_WORLDDEATH, MOD_WATER);
	VICTIM, " needs to learn how to swim." = (MSG_WORLDDEATH, MOD_WATER);
	VICTIM, " took a long walk of a short pier." = (MSG_WORLDDEATH, MOD_WATER);
	VICTIM, " might want to use a rebreather next time." = (MSG_WORLDDEATH, MOD_WATER);
	VICTIM, " thought she didn't need a rebreather." = (MSG_WORLDDEATH, MOD_WATER);
	VICTIM, " thought it didn't need a rebreather." = (MSG_WORLDDEATH, MOD_WATER);
	//killed by the world: MOD_SLIME
	VICTIM, " melted." = (MSG_WORLDDEATH, MOD_SLIME);
	VICTIM, " was dissolved." = (MSG_WORLDDEATH, MOD_SLIME);
	VICTIM, " sucked slime." = (MSG_WORLDDEATH, MOD_SLIME);
	VICTIM, " found an alternative way to die." = (MSG_WORLDDEATH, MOD_SLIME);
	VICTIM, " needs more slime-resistance." = (MSG_WORLDDEATH, MOD_SLIME);
	VICTIM, " might try on an environmental suit next time." = (MSG_WORLDDEATH, MOD_SLIME);
	//killed by the world: MOD_LAVA
	VICTIM, " does a back flip into the lava." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " was fried to a crisp." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " thought that lava was water." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " turned into a real hothead." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " thought lava was 'funny water'." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " tried to hide in the lava." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " thought he was fire resistant." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " tried to emulate the god of hell-fire." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " needs to rebind his 'strafe' keys." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " thought she was fire resistant." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " tried to emulate the goddess Pele." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " needs to rebind her 'strafe' keys." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " thought it was fire resistant." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " tried to emulate the demigod." = (MSG_WORLDDEATH, MOD_LAVA);
	VICTIM, " needs to rebind it's 'strafe' keys." = (MSG_WORLDDEATH, MOD_LAVA);
	//killed by the world: MOD_BARREL
	VICTIM, " blew up." = (MSG_WORLDDEATH, MOD_BARREL);
	//killed by the world: MOD_EXIT
	VICTIM, " found a way out." = (MSG_WORLDDEATH, MOD_EXIT);
	VICTIM, " had enough for today." = (MSG_WORLDDEATH, MOD_EXIT);
	VICTIM, " exit, stage left." = (MSG_WORLDDEATH, MOD_EXIT);
	VICTIM, " has returned to real life(tm)." = (MSG_WORLDDEATH, MOD_EXIT);
	//killed by the world: MOD_TARGET_LASER
	VICTIM, " saw the light." = (MSG_WORLDDEATH, MOD_TARGET_LASER);
	//killed by the world: MOD_TARGET_BLASTER
	VICTIM, " got blasted." = (MSG_WORLDDEATH, MOD_TARGET_BLASTER);
	//killed by the world: MOD_TRIGGER_HURT
	VICTIM, " was in the wrong place." = (MSG_WORLDDEATH, MOD_TRIGGER_HURT);
	VICTIM, " shouldn't play with equipment." = (MSG_WORLDDEATH, MOD_TRIGGER_HURT);
	VICTIM, " can't move around moving objects." = (MSG_WORLDDEATH, MOD_TRIGGER_HURT);
	//killed by oneself: MOD_HELD_GRENADE
	VICTIM, " tried to put the pin back in." = (MSG_SELFDEATH, MOD_HELD_GRENADE);
	VICTIM, " held his grenade too long." = (MSG_SELFDEATH, MOD_HELD_GRENADE);
	VICTIM, " got the red and blue wires mixed up." = (MSG_SELFDEATH, MOD_HELD_GRENADE);
	VICTIM, " tried to disassemble his own grenade." = (MSG_SELFDEATH, MOD_HELD_GRENADE);
	VICTIM, " held her grenade too long." = (MSG_SELFDEATH, MOD_HELD_GRENADE);
	VICTIM, " tried to disassemble her own grenade." = (MSG_SELFDEATH, MOD_HELD_GRENADE);
	VICTIM, " held it's grenade too long." = (MSG_SELFDEATH, MOD_HELD_GRENADE);
	VICTIM, " tried to disassemble it's own grenade." = (MSG_SELFDEATH, MOD_HELD_GRENADE);
	//killed by oneself: MOD_G_SPLASH
	VICTIM, " tripped on his own grenade." = (MSG_SELFDEATH, MOD_G_SPLASH);
	VICTIM, " stepped on his own pineapple." = (MSG_SELFDEATH, MOD_G_SPLASH);
	VICTIM, " tried to grenade-jump unsuccessfully." = (MSG_SELFDEATH, MOD_G_SPLASH);
	VICTIM, " tried to play football with a grenade." = (MSG_SELFDEATH, MOD_G_SPLASH);
	VICTIM, " shouldn't mess around with explosives." = (MSG_SELFDEATH, MOD_G_SPLASH);
	VICTIM, " tripped on her own grenade." = (MSG_SELFDEATH, MOD_G_SPLASH);
	VICTIM, " stepped on her own pineapple." = (MSG_SELFDEATH, MOD_G_SPLASH);
	VICTIM, " tripped on it's own grenade." = (MSG_SELFDEATH, MOD_G_SPLASH);
	VICTIM, " stepped on it's own pineapple." = (MSG_SELFDEATH, MOD_G_SPLASH);
	//killed by oneself: MOD_R_SPLASH
	VICTIM, " blew himself up." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " thought he was Werner von Braun." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " knows didley squatt about rocket launchers." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " thought he had more health." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " thought up a novel new way to fly." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " found his own rocketlauncher's trigger." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " thought he had more armor on." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " blew himself to kingdom come." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " blew herself up." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " thought she was Werner von Braun." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " thought she had more health." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " found her own rocketlauncher's trigger." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " thought she had more armor on." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " blew herself to kingdom come." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " blew itself up." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " thought it was Werner von Braun." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " thought it had more health." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " found it's own rocketlauncher's trigger." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " thought it had more armor on." = (MSG_SELFDEATH, MOD_R_SPLASH);
	VICTIM, " blew itself to kingdom come." = (MSG_SELFDEATH, MOD_R_SPLASH);
	//killed by oneself: MOD_BFG_BLAST
	VICTIM, " should have used a smaller gun." = (MSG_SELFDEATH, MOD_BFG_BLAST);
	VICTIM, " shouldn't play with big guns." = (MSG_SELFDEATH, MOD_BFG_BLAST);
	VICTIM, " doesn't know how to work the BFG." = (MSG_SELFDEATH, MOD_BFG_BLAST);
	VICTIM, " has trouble using big guns." = (MSG_SELFDEATH, MOD_BFG_BLAST);
	VICTIM, " can't distinguish which end is which with the BFG." = (MSG_SELFDEATH, MOD_BFG_BLAST);
	VICTIM, " should try to avoid using the BFG near obstacles." = (MSG_SELFDEATH, MOD_BFG_BLAST);
	VICTIM, " tried to BFG-jump unsuccesfully." = (MSG_SELFDEATH, MOD_BFG_BLAST);
	//killed by oneself: MOD_TRAP
	VICTIM, " sucked into his own trap." = (MSG_SELFDEATH, MOD_TRAP);
	VICTIM, " sucked into her own trap." = (MSG_SELFDEATH, MOD_TRAP);
	VICTIM, " sucked into it's own trap." = (MSG_SELFDEATH, MOD_TRAP);
	//killed by oneself: MOD_DOPPLE_EXPLODE
	VICTIM, " got caught in his own trap." = (MSG_SELFDEATH, MOD_DOPPLE_EXPLODE);
	VICTIM, " got caught in her own trap." = (MSG_SELFDEATH, MOD_DOPPLE_EXPLODE);
	VICTIM, " got caught in it's own trap." = (MSG_SELFDEATH, MOD_DOPPLE_EXPLODE);
	//killed by oneself: MOD_SUICIDE
	VICTIM, " killed himself." = (MSG_SELFDEATH, MOD_SUICIDE);
	VICTIM, " commited suicide." = (MSG_SELFDEATH, MOD_SUICIDE);
	VICTIM, " went the way of the dodo." = (MSG_SELFDEATH, MOD_SUICIDE);
	VICTIM, " thought 'kill' was a funny console command." = (MSG_SELFDEATH, MOD_SUICIDE);
	VICTIM, " wanted one frag less." = (MSG_SELFDEATH, MOD_SUICIDE);
	VICTIM, " thought he had one many frags." = (MSG_SELFDEATH, MOD_SUICIDE);
	VICTIM, " killed herself." = (MSG_SELFDEATH, MOD_SUICIDE);
	VICTIM, " thought she had one many frags." = (MSG_SELFDEATH, MOD_SUICIDE);
	VICTIM, " killed itself." = (MSG_SELFDEATH, MOD_SUICIDE);
	VICTIM, " thought it had one many frags." = (MSG_SELFDEATH, MOD_SUICIDE);
	//killed by KILLER: MOD_BLASTER
	VICTIM, " (quakeweenie) was massacred by ", KILLER, " (quakegod)!!!" = (MSG_DEATH, MOD_BLASTER);
	VICTIM, " was killed with the wimpy blaster by ", KILLER = (MSG_DEATH, MOD_BLASTER);
	VICTIM, " died a wimp's death by ", KILLER = (MSG_DEATH, MOD_BLASTER);
	VICTIM, " can't even avoid a blaster from ", KILLER = (MSG_DEATH, MOD_BLASTER);
	VICTIM, " was blasted by ", KILLER = (MSG_DEATH, MOD_BLASTER);
	//killed by KILLER: MOD_SHOTGUN
	VICTIM, " found himself on the wrong end of ", KILLER, "'s gun" = (MSG_DEATH, MOD_SHOTGUN);
	VICTIM, " was gunned down by ", KILLER = (MSG_DEATH, MOD_SHOTGUN);
	VICTIM, " found herself on the wrong end of ", KILLER, "'s gun" = (MSG_DEATH, MOD_SHOTGUN);
	VICTIM, " found itself on the wrong end of ", KILLER, "'s gun" = (MSG_DEATH, MOD_SHOTGUN);
	//killed by KILLER: MOD_SSHOTGUN
	VICTIM, " had his ears cleaned out by ", KILLER, "'s super shotgun" = (MSG_DEATH, MOD_SSHOTGUN);
	VICTIM, " was put full of buckshot by ", KILLER = (MSG_DEATH, MOD_SSHOTGUN);
	VICTIM, " had her ears cleaned out by ", KILLER, "'s super shotgun" = (MSG_DEATH, MOD_SSHOTGUN);
	VICTIM, " had it ears cleaned out by ", KILLER, "'s super shotgun" = (MSG_DEATH, MOD_SSHOTGUN);
	//killed by KILLER: MOD_MACHINEGUN
	VICTIM, " was machinegunned by ", KILLER = (MSG_DEATH, MOD_MACHINEGUN);
	VICTIM, " was filled with lead by ", KILLER = (MSG_DEATH, MOD_MACHINEGUN);
	VICTIM, " was put full of lead by ", KILLER = (MSG_DEATH, MOD_MACHINEGUN);
	VICTIM, " was pumped full of lead by ", KILLER = (MSG_DEATH, MOD_MACHINEGUN);
	VICTIM, " ate lead dished out by ", KILLER = (MSG_DEATH, MOD_MACHINEGUN);
	VICTIM, " eats lead from ", KILLER = (MSG_DEATH, MOD_MACHINEGUN);
	VICTIM, " bites the bullet from ", KILLER = (MSG_DEATH, MOD_MACHINEGUN);
	//killed by KILLER: MOD_CHAINGUN
	VICTIM, " was cut in half by ", KILLER, "'s chaingun" = (MSG_DEATH, MOD_CHAINGUN);
	VICTIM, " was put so full of lead by ", KILLER, " you can call him a pencil" = (MSG_DEATH, MOD_CHAINGUN);
	VICTIM, " was turned into a strainer by ", KILLER = (MSG_DEATH, MOD_CHAINGUN);
	VICTIM, " was put full of holes by ", KILLER = (MSG_DEATH, MOD_CHAINGUN);
	VICTIM, " couldn't avoid death by painless from ", KILLER = (MSG_DEATH, MOD_CHAINGUN);
	VICTIM, " was put so full of lead by ", KILLER, " you can call her a pencil" = (MSG_DEATH, MOD_CHAINGUN);
	VICTIM, " was put so full of lead by ", KILLER, " you can call it a pencil" = (MSG_DEATH, MOD_CHAINGUN);
	//killed by KILLER: MOD_GRENADE
	VICTIM, " was popped by ", KILLER, "'s grenade" = (MSG_DEATH, MOD_GRENADE);
	VICTIM, " caught ", KILLER, "'s grenade in the head" = (MSG_DEATH, MOD_GRENADE);
	VICTIM, " tried to headbutt the grenade of ", KILLER = (MSG_DEATH, MOD_GRENADE);
	//killed by KILLER: MOD_G_SPLASH
	VICTIM, " was shredded by ", KILLER, "'s shrapnel" = (MSG_DEATH, MOD_G_SPLASH);
	//killed by KILLER: MOD_ROCKET
	VICTIM, " ate ", KILLER, "'s rocket" = (MSG_DEATH, MOD_ROCKET);
	VICTIM, " sucked on ", KILLER, "'s boomstick" = (MSG_DEATH, MOD_ROCKET);
	VICTIM, " tried to play 'dodge the missile' with ", KILLER = (MSG_DEATH, MOD_ROCKET);
	VICTIM, " tried the 'patriot move' on the rocket from ", KILLER = (MSG_DEATH, MOD_ROCKET);
	VICTIM, " had a rocket stuffed down the throat by ", KILLER = (MSG_DEATH, MOD_ROCKET);
	VICTIM, " got a rocket up the tailpipe by ", KILLER = (MSG_DEATH, MOD_ROCKET);
	VICTIM, " tried to headbutt ", KILLER, "'s rocket" = (MSG_DEATH, MOD_ROCKET);
	//killed by KILLER: MOD_R_SPLASH
	VICTIM, " almost dodged ", KILLER, "'s rocket" = (MSG_DEATH, MOD_R_SPLASH);
	VICTIM, " was spread around the place by ", KILLER = (MSG_DEATH, MOD_R_SPLASH);
	VICTIM, " was gibbed by ", KILLER = (MSG_DEATH, MOD_R_SPLASH);
	VICTIM, " has been blown to smithereens by ", KILLER = (MSG_DEATH, MOD_R_SPLASH);
	VICTIM, " was blown to itsie bitsie tiny pieces by ", KILLER = (MSG_DEATH, MOD_R_SPLASH);
	//killed by KILLER: MOD_HYPERBLASTER
	VICTIM, " was melted by ", KILLER, "'s hyperblaster" = (MSG_DEATH, MOD_HYPERBLASTER);
	VICTIM, " was used by ", KILLER, " for target practice" = (MSG_DEATH, MOD_HYPERBLASTER);
	VICTIM, " was hyperblasted by ", KILLER = (MSG_DEATH, MOD_HYPERBLASTER);
	VICTIM, " was pumped full of cells by ", KILLER = (MSG_DEATH, MOD_HYPERBLASTER);
	VICTIM, " couldn't outrun the hyperblaster from ", KILLER = (MSG_DEATH, MOD_HYPERBLASTER);
	//killed by KILLER: MOD_RAILGUN
	VICTIM, " was railed by ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " got a slug put through him by ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " played 'catch the slug' with ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " was corkscrewed through his head by ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " bites the slug from ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " caught the slug from ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " had his body pierced with a slug from ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " had his brains blown out by ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " got a slug put through her by ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " was corkscrewed through her head by ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " had her body pierced with a slug from ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " had her brains blown out by ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " got a slug put through it by ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " was corkscrewed through it's head by ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " had it's body pierced with a slug from ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	VICTIM, " had it's brains blown out by ", KILLER = (MSG_DEATH, MOD_RAILGUN);
	//killed by KILLER: MOD_BFG_LASER
	VICTIM, " saw the pretty lights from ", KILLER, "'s BFG" = (MSG_DEATH, MOD_BFG_LASER);
	VICTIM, " was diced by the BFG from ", KILLER = (MSG_DEATH, MOD_BFG_LASER);
	//killed by KILLER: MOD_BFG_BLAST
	VICTIM, " was disintegrated by ", KILLER, "'s BFG blast" = (MSG_DEATH, MOD_BFG_BLAST);
	VICTIM, " was flatched with the green light by ", KILLER = (MSG_DEATH, MOD_BFG_BLAST);
	//killed by KILLER: MOD_BFG_EFFECT
	VICTIM, " couldn't hide from ", KILLER, "'s BFG" = (MSG_DEATH, MOD_BFG_EFFECT);
	VICTIM, " tried to soak up green energy from ", KILLER, "'s BFG" = (MSG_DEATH, MOD_BFG_EFFECT);
	VICTIM, " was energized with 50 cells by ", KILLER = (MSG_DEATH, MOD_BFG_EFFECT);
	VICTIM, " doesn't know when to run from ", KILLER = (MSG_DEATH, MOD_BFG_EFFECT);
	VICTIM, " 'saw the light' from ", KILLER = (MSG_DEATH, MOD_BFG_EFFECT);
	//killed by KILLER: MOD_HANDGRENADE
	VICTIM, " caught ", KILLER, "'s handgrenade" = (MSG_DEATH, MOD_HANDGRENADE);
	VICTIM, " should watch more carefully for handgrenades from ", KILLER = (MSG_DEATH, MOD_HANDGRENADE);
	VICTIM, " caught ", KILLER, "'s handgrenade in the head" = (MSG_DEATH, MOD_HANDGRENADE);
	VICTIM, " tried to headbutt the handgrenade of ", KILLER = (MSG_DEATH, MOD_HANDGRENADE);
	//killed by KILLER: MOD_HG_SPLASH
	VICTIM, " didn't see ", KILLER, "'s handgrenade" = (MSG_DEATH, MOD_HG_SPLASH);
	//killed by KILLER: MOD_HELD_GRENADE
	VICTIM, " feels ", KILLER, "'s pain" = (MSG_DEATH, MOD_HELD_GRENADE);
	//killed by KILLER: MOD_TELEFRAG
	VICTIM, " tried to invade ", KILLER, "'s personal space" = (MSG_DEATH, MOD_TELEFRAG);
	VICTIM, " is less telefrag aware than ", KILLER = (MSG_DEATH, MOD_TELEFRAG);
	VICTIM, " should appreciate scotty more like ", KILLER = (MSG_DEATH, MOD_TELEFRAG);
	//killed by KILLER: MOD_RIPPER
	VICTIM, " ripped to shreds by ", KILLER, "'s ripper gun" = (MSG_DEATH, MOD_RIPPER);
	//killed by KILLER: MOD_PHALANX
	VICTIM, " was evaporated by ", KILLER = (MSG_DEATH, MOD_PHALANX);
	//killed by KILLER: MOD_TRAP
	VICTIM, " caught in trap by ", KILLER = (MSG_DEATH, MOD_TRAP);
	//killed by KILLER: MOD_CHAINFIST
	VICTIM, " was shredded by ", KILLER, "'s ripsaw" = (MSG_DEATH, MOD_CHAINFIST);
	//killed by KILLER: MOD_DISINTEGRATOR
	VICTIM, " lost his grip courtesy of ", KILLER, "'s disintegrator" = (MSG_DEATH, MOD_DISINTEGRATOR);
	//killed by KILLER: MOD_ETF_RIFLE
	VICTIM, " was perforated by ", KILLER = (MSG_DEATH, MOD_ETF_RIFLE);
	//killed by KILLER: MOD_HEATBEAM
	VICTIM, " was scorched by ", KILLER, "'s plasma beam" = (MSG_DEATH, MOD_HEATBEAM);
	//killed by KILLER: MOD_TESLA
	VICTIM, " was enlightened by ", KILLER, "'s tesla mine" = (MSG_DEATH, MOD_TESLA);
	//killed by KILLER: MOD_PROX
	VICTIM, " got too close to ", KILLER, "'s proximity mine" = (MSG_DEATH, MOD_PROX);
	//killed by KILLER: MOD_NUKE
	VICTIM, " was nuked by ", KILLER, "'s antimatter bomb" = (MSG_DEATH, MOD_NUKE);
	//killed by KILLER: MOD_VENGEANCE_SPHERE
	VICTIM, " was purged by ", KILLER, "'s vengeance sphere" = (MSG_DEATH, MOD_VENGEANCE_SPHERE);
	//killed by KILLER: MOD_DEFENDER_SPHERE
	VICTIM, " had a blast with ", KILLER, "'s defender sphere" = (MSG_DEATH, MOD_DEFENDER_SPHERE);
	//killed by KILLER: MOD_HUNTER_SPHERE
	VICTIM, " was killed like a dog by ", KILLER, "'s hunter sphere" = (MSG_DEATH, MOD_HUNTER_SPHERE);
	//killed by KILLER: MOD_TRACKER
	VICTIM, " was annihilated by ", KILLER, "'s disruptor" = (MSG_DEATH, MOD_TRACKER);
	//killed by KILLER: MOD_DOPPLE_EXPLODE
	VICTIM, " was blown up by ", KILLER, "'s doppleganger" = (MSG_DEATH, MOD_DOPPLE_EXPLODE);
	//killed by KILLER: MOD_DOPPLE_VENGEANCE
	VICTIM, " was purged by ", KILLER, "'s doppleganger" = (MSG_DEATH, MOD_DOPPLE_VENGEANCE);
	//killed by KILLER: MOD_DOPPLE_HUNTER
	VICTIM, " was hunted down by ", KILLER, "'s doppleganger" = (MSG_DEATH, MOD_DOPPLE_HUNTER);
	//killed by KILLER: MOD_GRAPPLE
	VICTIM, " was caught by ", KILLER, "'s grapple" = (MSG_DEATH, MOD_GRAPPLE);
	//killed by the world: MOD_UNKNOWN
	VICTIM, " died." = (MSG_WORLDDEATH, MOD_UNKNOWN);
} //end MTCONTEXT_CLIENTOBITUARY
