//===========================================================================
//
// Name:		Trillian_c.c
// Function:		chat lines for Trillian [Chat text form]
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:		R.A. Salvatore & The Seven Swords
// Editor:		Paul Jaquays
// Last update:		Sept. 26, 1999
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats
chat "tim"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"You guys are gonna be so dead.";
		"Bring it on losers.";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
			" ";
	//		"GAME_EXIT 0 = ", 0, ", 4 = ", 4;
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
			" ";
	//		"LEVEL_START 0 = ", 0;
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
			" ";
	//		"LEVEL_END 0 = ", 0, " 1 = ", 1, ", 2 = ", 2, " 3 = ", 3, ", 4 = ", 4;
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
			" ";
	//		"LEVEL_END_VICTORY 0 = ", 0, " 1 = ", 1, " 3 = ", 3, ", 4 = ", 4;
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
			" ";
	//		"LEVEL_END_LOSE 0 = ", 0, " 1 = ", 1, ", 2 = ", 2,  " 4 = ", 4;
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		" ";
	//	"HIT_TALKING 0 = ", 0, " 1 = ", 1;
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		" ";
		//	"HIT_NODEATH 0 = ", 0, " 1 = ", 1;
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
			" ";
	//	"HIT_NOKILL 0 = ", 0, " 1 = ", 1;
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
			" ";
	//	"DEATH_TELEFRAG 0 = ", 0;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
			" ";
	//	"DEATH_CRATERED 0 = ", 0;
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
			" ";
	//	"DEATH_LAVA 0 = ", 0;
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
			" ";
	//	"DEATH_SLIME 0 = ", 0;
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"hole";
	//	"DEATH_DROWN 0 = ", 0;
	//	// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"hole";
	//	"DEATH_SUICIDE 0 = ", 0;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"hole";
	//	"DEATH_GAUNTLET 0 = ", 0, " 1 = ", 1;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"hole";
	//	"DEATH_RAIL 0 = ", 0, " 1 = ", 1;
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"hole";
	//	"DEATH_BFG 0 = ", 0, " 1 = ", 1;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"hole";
	//	"DEATH_INSULT 0 = ", 0, " 1 = ", 1;
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"hole";
		//"DEATH_PRAISE 0 = ", 0, " 1 = ", 1;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
			" ";
	//	"KILL_RAIL 0 = ", 0;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
			" ";
		//"KILL_GAUNTLET 0 = ", 0;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
			" ";
		//"KILL_TELEFRAG 0 = ", 0;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self with a weapon of craters
	{
		" ";
	//	"KILL_SUICIDE 0 = ", 0;
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
			" ";
	//		"KILL_INSULT 0 = ", 0;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
			" ";
	//		"KILL_PRAISE 0 = ", 0;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
			" ";
	//		"RANDOM_INSULT 0 = ", 0, " 1 = ", 1, ", 4 = ", 4, " 5 = ", 5;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
			" ";
	//		"RANDOM_INSULT 0 = ", 0, " 1 = ", 1, ", 4 = ", 4, " 5 = ", 5;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end chat







