//===========================================================================
//
// Name:			orbb_c.c
// Function:		chat lines for Orbb
// Programmer:		Jan Paul & J. Cash
// Scripter:		Paul Jaquays
// Last update:		09/08/99
// Tab Size:		3 (real tabs)
//===========================================================================

chat "orbb"
{
	#include "teamplay.h"
	//
	type "game_enter"
	{
		HELLO;
		HELLO6;
		"Hey Cow!";
		"Ahhhhh. I see we are in this place again.";
		"Salutations to all, and to all a good fight.";
		"Salut";
		"Guten Tag";
		"Buenos Dias";
		"Aloha";
		"Oh look! ", 1, " is here! So much fun will be had.";
	} //end type

	type "game_exit"
	{
		GOODBYE;
		GOODBYE1;
		GOODBYE4;
		"Adios";
		"Au revoir, mon amis.";
		"Auf Wiedersehen";
		"Aloha";
		"Happy Trails to You, until we meet again!";
	} //end type

	type "level_start"
	{
		LEVEL_START2;
		"This will be so fun.";
		"You will all die. It will be fun. Yes?";
	} //end type

	type "level_end"
	{
		"So soon? This unit is only warming up now.";
		"Always the fun ends too soon.";
		LEVEL_END2;
	} //end type

	type "level_end_victory"
	{
		"This unit wins match? Really?";
		"Win ... lose. All the same.";
		"This unit is so very happy.  It feels tear welling up in its eye.";
		"This unit would like to thank the academy ...";
	} //end type

	type "level_end_lose"
	{
		"Win ... lose. All the same.";
		"This unit does not understand its feelings.  Feel ... sad.";
		"This unit very happy for winner.  But next time this unit frags you better.";
	} //end type

	//================================================================
	//================================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Boooo!! You interrupt this unit talking, ", 0, ".";
		"This unit don't think it like you, ", 0, ". You bad sport.";
		"Ohhhhh! You think blue balloon mean 'shoot this unit now', ", 0, "? Yes?";
		//0 = shooter
	} //end type

	type "damaged_nokill" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Ow! You hurt orbb.  You not nice person, ", 0, ".";
		"Is Orbb correct in assuming that you not know how to use that weapon, ", 0, "?";
		"You must do better than that to beat this unit, ", 0, ".";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Phooey! ", 0, " unit survived this unit's attack.";
		"Mmmmmm. ", 0, " unit not die.  Must use bigger gun next time.";
		"Not good hit.  This unit must need eyeglass.";
		//0 = opponent
	} //end type

	type "enemy_suicide"
	{
		"You practice that move much, ", 0, "?";
		"This unit could have done that for you, ", 0, ".";
		"Can this unit have your weapon, ", 0, "?";
		//0 = enemy
	} //end type

	//================================================================
	//================================================================
	
	type "death_telefrag"
	{
		"~Two cannot live in the same space as ~one.";
		DEATH_TELEFRAGGED3;
	} //end type

	type "death_lava"
	{
		"My sensors indicate melted stone is bad for continued function.";
		DEATH_LAVA0;
	} //end type

	type "death_slime"
	{
		"Curse you Dorothy, I'm melting ... melting.";
		DEATH_SLIME0;
		DEATH_SUICIDE2;
	} //end type

	type "death_drown"
	{
		"Note to self: Get swim fins for hands.";
		"Oops.";
		"Error. Error.";
		DEATH_DROWN0;
		DEATH_SUICIDE6;
	} //end type

	type "death_suicide"
	{
		"Oops.";
		"Error. Error.";
		"This unit's mistake.";
	} //end type

	type "death_gauntlet"
	{
		"Ahhh. The Agony of Defeat. Very Interesting.";
		"Heehee. Very Fun ", 0, ".";
		"Must make note. Do not let ", 0, " come near to this unit.";
	} //end type

	type "death_rail"
	{
		"Are you cheating, ", 0, "?";
		"Note to self: Keep an eye on ", 0, ".";
		DEATH_RAIL0;
	} //end type

	type "death_bfg"
	{
		0, ", you use this weapon because you are not good warrior? Yes?";
		0,"is not very sporting, this unit thinks.";
		"You are a 'BFG-Camper', ", 0, ", yes?";
	} //end type

	type "death_insult"
	{
		"Very inefficient ", 0, ". Too much ammo wasted.";
		"Your female parental unit.";
		"That was fun, ", 0, ".  Let us do it again.";
	} //end type

	type "death_praise"
	{
		"You make this unit die good, ", 0, "!";
		"WOW! This unit not see that ~one coming, ", 0, "!";
		"Ow! ", 0, " hit this unit in blind side.";
		D_PRAISE;
		D_PRAISE3;
		D_PRAISE5;
	} //end type

		type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
		{
		"WOW! Too much fun!";
		"Danger! Danger! Danger!";
		"No disassemble this unit, ", 0, ".";
		"That was definitely ", different, ", than the last time, you ", fighter, ".";
		"Next time you warn this unit. Send postcard.";
		one_liners;
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"Jackpot!!";
		"Better luck next time, ", friendname, ".";
		affirmative, " This unit likes kamikaze.";
		"Too bad for ", fighter, "s. Move too slow.";
		"Can you dig it?";
	  "He shoots! He SCORES! The fans go wild!";
	  ramble;
		// 0 = enemy name
	} //end type

	type "kill_Rail" 
	{
		"Hmmmmmmm. ~Railgun extremely efficient.";
		"Pretty.";
		KILL_RAIL1;
	} //end type

	type "kill_gauntlet"
	{
		"Mmmmm. Do you like being made to look bad like that, ", 0, "?";
		"Yes. Yes. Very fun.";
		KILL_GAUNTLET0;
	} //end type

	type "kill_telefrag"
	{
		TELEFRAGGED2;
		TELEFRAGGED3;
		TELEFRAGGED5;
	} //end type

	type "kill_insult"
	{
		KILL_INSULT27;
		KILL_INSULT12;
		KILL_INSULT18;
		KILL_INSULT24;
		KILL_INSULT22;
	} //end type

	type "kill_praise"
	{
		PRAISE4;
		"You make a very nice target, ", 0, ".";
		"You die very much nicely, ", 0, ".";
		"So much fun, ", 0,".  Now you shoot me.";
		"You very good at dying, ", 0, ". You must practice a lot.";
	} //end type

	//================================================================
	//================================================================

	type "random_insult"
	{
		TAUNT_FEM4;
		KILL_INSULT32;
		TAUNT4;
		TAUNT8;
		"This unit's sensors detect a lame loser in Quadrant A7. Must be ", 1, ".";
		"Are you going to let a big ~eyeball beat you, ", 1, "?";
		"Look, another piece of ", 1, ". Be first to collect them all!!";
	} //end type

	type "random_misc"
	{
		0, ", could you love a being with only ~one eye?";
		"This unit blames Steed for its lousy love life.";
		"Mmmmmmm. ", 0, " is very easy to look at.";
		"The chicks really dig big eyes.";
		"Time to party, ", 1, ".";
		"Hey! You crazy kids! Get offa my lawn.";
		"Hey Cow!";
		MISC1;
		MISC4;
		MISC7;
	} //end type

} //end chat orbb

