//===========================================================================
//
// Name:			malaki_c.c
// Function:		chat lines for malaki
// Programmer:		Jan Paul & J. Cash
// Scripter:		Paul Jaquays
// Last update:		9/08/99
// Tab Size:		3 (real tabs)
//===========================================================================

chat "malaki"
{
	#include "teamplay.h"
	//
	type "game_enter"
	{
		HELLO3;
		HELLO4;
		HELLO6;
		"Heh. Fresh Meat.";
		"Time to Rock 'n Roll.";
	} //end type

	type "game_exit"
	{
		GOODBYE;
		GOODBYE2;
		"I'm history. Later kids.";
	} //end type

	type "level_start"
	{
		LEVEL_START2;
		"This place rocks!";
		"I'm ready to win.";
	} //end type

	type "end_level"
	{
		LEVEL_END2;
		"Perhaps luck won't favor you next time, eh?";
		"I won't make the same mistakes next time.";
	} //end type

	type "end_level_victory"
	{
		"I deserve this!";
		"This is what it's all about, kid!";
		LEVEL_END_VICTORY0;
		LEVEL_END_VICTORY2;
	} //end type

	type "level_end_lose"
	{
		"Grrrr.";
		"Luck. Pure luck.";
		"Next time, your butts are mine.";
		LEVEL_END_LOSE1;
	} //end type
	//======================================================
	//======================================================
	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		//0 = shooter
	} //end type
	type "damaged_nokill" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		//0 = shooter
	} //end type
	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		//0 = opponent
	} //end type
	type "enemy_suicide"
	{
		//0 = enemy
	} //end type
	//======================================================
	//======================================================

	//================================================================
	//================================================================

	type "death_telefrag"
	{
		DEATH_TELEFRAGGED0;
		DEATH_TELEFRAGGED1;
	} //end type

	type "death_lava"
	{
		"Lousy stuff.";
		"This stuff makes me mad.";
		DEATH_SUICIDE2;
		DEATH_LAVA1;
	} //end type

	type "death_slime"
	{
		"I never get used to that stuff.";
		"Get that slime in your cybronics and they're never the same again.";
		DEATH_SUICIDE0;
	} //end type

	type "death_drown"
	{
		"I knew I should have packed a snorkel for this trip.";
		DEATH_DROWN0;
	} //end type

	type "death_suicide"
	{
		"Don't laugh kid, you've probably done this a lot.";
		"I've gone and made a mess of myself again.";
		DEATH_SUICIDE0;
		DEATH_SUICIDE2;
	} //end type 

	type "death_gauntlet"
	{
		"Will you stop touching me?";
		"Do that again and you are toast.";
		DEATH_GAUNTLET0;
		DEATH_GAUNTLET1;
	} //end type

	type "death_rail"
	{
		"Heh. Nice shot kid.";
		"I hate campers. I really do.";
		"Did Xian show you how to do that?";
		DEATH_RAIL1;
	} //end type 

	type "death_bfg"
	{
		DEATH_BFG0;
		DEATH_BFG1;
	} //end type

	type "death_insult"
	{
		DEATH_INSULT5;
		"Got any ammo left, punk?  You're gonna need it.";
		"Did you enjoy that?";
	} //end type

	type "death_praise"
	{
		D_PRAISE5;
		"Oh, yeah. You had me good there kid.";
		"Not bad kid.";
	} //end type

	//================================================================
	//================================================================

	type "kill_rail" 
	{
		"Yessssss!!";
		"Gotcha!";
		"You're it!";
		KILL_RAIL0;
	} //end type

	type "kill_gauntlet"
	{
		"I love doing that!";
		"Humiliation is the name of the game, kid.";
		KILL_GAUNTLET0;
		KILL_GAUNTLET1;
	} //end type

	type "kill_telefrag"
	{
		TELEFRAGGED0;
		TELEFRAGGED2;	
	} //end type

	type "kill_insult"
	{
		KILL_INSULT7;
		KILL_INSULT10;
		KILL_INSULT16;
		"Pick up your toys and go home kid.";
		"I was told you were competition, ", 0, ". Somebody got that wrong.";
	} //end type

	type "kill_praise"
	{
		"Not bad, kid. Most don't last that long.";
		"Almost didn't get you there, kid.";
		PRAISE4;
	} //end type

	//================================================================
	//================================================================

	type "random_insult"
	{
		"You will learn to fear me.";
		"I will be your personal nightmare, kid.";
		TAUNT7;
		TAUNT8;	
	} //end type

	type "random_misc"
	{
		"Victory will be mine!";
		"You can't win. It's not in you to be a winner.";
		MISC8;
		MISC9;
		MISC11;
	} //end type

} //end chat malaki
