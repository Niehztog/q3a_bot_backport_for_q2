//===========================================================================
//
// Name:			Xaero_c.c
// Function:		chat lines for Xaero
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Last update:		Nov,9.1999
// Author:			Christian Antkow
// Tab Size:		3 (real tabs)
// Notes:			Todd tells me that I need to keep this clean, so I can't
//					use phrases like $@#!smoker, &%@#blower and @#*!saq! =)
//===========================================================================

chat "xian"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Werd to your mother";
		"Werd to each of your respective mothers";
		"Werd";
		"Greets... and stuff...";
		"Hi.";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Ciao";
		"Later";
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"ph34r muh 5k1llZ!";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		0, ", you sub-amoebic pile, that was a pretty low thing to do.";
		"Look ", 0, ", keep doing that and I'm gonna hafta get nasty!";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Ok. That hurt.";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"That was a really stupid thing I just did there...";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"DOH!";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Ok ", 0, ", I am suitably embarassed";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Nice ", 0, "!";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"You pile!";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Haha! Shouldn't have been there!";
		"wewp!";
		"j00 mu55 b0w d0wN!";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		0, " dude... you suck pretty hard.";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Heh, bet you're feeling like a pretty big loser right now, huh ", 0, " ?";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self with a weapon of craters
	{
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		0, ". You suck!";
		0, "... my cats play better than you...";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end xian chat







