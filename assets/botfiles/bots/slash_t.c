//===========================================================================
//
// Name:			slash_c.c
// Function:		chat lines for [Slash]
// Programmer:		Jan Paul & J. Cash
// Scripter:		Paul Jaquays
// Last update:		May 19, 2000
// Tab Size:		3 (real tabs)
//===========================================================================

chat "slash"
{
	#include "teamplay.h"
	//
	type "game_enter"
	{
		"Konnichiwa, campers.";
		"Ooooo. Gimme some room, I feel a victory comin' on.";
		"My wetware is hot tonight!";
		"I'm ready to log in and get virtual.";
		"Been here, done this and am ready to do it again.";
		"Whoaaaa. I need a global positioning implant to find my way around in here.";
		"Like, dibs on the BFG!";
		HELLO4;
		HELLO10;
	} //end type

	type "game_exit"
	{
		"Sayonara, babies.";
		"I'm going offline.";
		"Like, I feel a ~major download comin' on. Gotta scoot.";
		GOODBYE1;
		GOODBYE2;
		GOODBYE4;
	} //end type

	type "level_start"
	{
		"Been here, done this and am ready to do it again.";
		"I'm ready to jack in and get virtual.";
		LEVEL_START0;
	} //end type

	type "level_end"
	{
		"So soon?. Like, I'm not breathing hard yet.";
		"I see I underestimated you fellows.";
		LEVEL_END2;
	} //end type

	type "level_end_victory"
	{
		"Winning comes naturally to me.";
		"I think I'll have my gun bronzed.";
		"This is, like, so sweet.";
		LEVEL_END_VICTORY2;
	} //end type

	type "level_end_lose"
	{
		"I have brought, y'know, dishonor to those who trusted me.";
		"Where I come from, they don't, like, teach you how to lose.";
		"I just hate losing.";
		LEVEL_END_LOSE1;
	} //end type

	//======================================================
	//======================================================
	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"I can't believe you shot me while I was talking, ", 0, "!";
		"I'll guess you're not a very popular person, ", 0, ".";
		"That was ~one cheap shot you took there, ", 0, ".";
		"Ow! Right through the cell phone.";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"You'll have to do better than that to keep this blade-lady down.";
		"Nice shot, ", 0, ". But definitely not nice enough.";
		"Oooooo! You like me. You really like me.!";
		"Frack! Do you know how it is to replace those implants, ", 0, "?";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"~One shot, ~one .... Ok, I guess I'll have to make it ~two shots.";
		"You've been whoring the armor again, ", 0, ", haven't you?";
		"~Don't run off, ", 0, ", I've got more just like that for you.";
		//0 = opponent
	} //end type

	type "enemy_suicide"
	{
		"Laughing? Me? Of course I'm laughing at you, ", 0, ".";
		"Wow! Do you do that often, ", 0, "?";
		"Death by papercut. What a terrible way to go.";
		"Wooohooo. Can we catch that on instant replay, ", 0, "?";

		//0 = enemy
	} //end type

	//================================================================
	//================================================================

	type "death_telefrag"
	{
		"no way....";
		DEATH_TELEFRAGGED0;
		DEATH_TELEFRAGGED2;
	} //end type

	type "death_lava"
	{
		"That lava stuff really messes with my wetware.";
		"I'm gonna need another cooling fan implant.";
		DEATH_LAVA0;
	} //end type

	type "death_slime"
	{
		"That slime reminds me of a guy I once dated.";
		"It's 'Slime Time' again.";
		"That slime overloaded my implants.";
		DEATH_SLIME0;
	} //end type

	type "death_drown"
	{
		"All that water and no hunky lifeguards. No beach either.";
		"If I were meant to swim, I'd have gill implants.";
		"Now all my ware is wet,";
		"Like, terminal wet t-shirt contest!";
		DEATH_DROWN0;
	} //end type

	type "death_gauntlet"
	{
		"Did I give you permission to touch me?";
		"Y'know, that really pisses me off.";
		DEATH_GAUNTLET1;
	} //end type

	type "death_rail"
	{
		"That won't happen again.";
		"Camping Scum!";
		DEATH_RAIL2;
	} //end type

	type "death_suicide"
	{
		"So. Like, I suppose you never off yourself? Right?";
		"Like, 'Ow.'";
		"There is, like, no fun in wasting ~one's self like that.";
		DEATH_SUICIDE0;
	} //end type

	type "death_bfg"
	{
		"I, like, take special delight in fragging BFG campers.";
		"That was a mistake. You've made me, like, very angry now.";
		"Gross! Like, fragged by my own fave tool.";
		DEATH_BFG0;
	} //end type

	type "death_insult"
	{
		DEATH_INSULT4;
		DEATH_FEM_INSULT;
		"Like, am I supposed to be impressed by that?";
		"Son of a ...? Am I, like, some kind of loser magnet?";
		"Totally uncool. Totally.";
	} //end type

	type "death_praise"
	{
		"This match might be, like, interesting after all.";
		"Consider yourself lucky. I only let the Adrianator do that.";
		"Nice moves! Like, are they yours or implants?";
		D_PRAISE;
		D_PRAISE4;
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
	"^1**[giggle]**";
		"Y'know, like, that was totally gross.";
		"Next time, ", 0, " ... my foot, your butt.";
		"Like, was that the best you could do?";
		"Those, like, oughta be illegal.";
		"Wheee! My turn now!";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"That is just too kewl!";
		"Nice moves babe! The gib bounce at the end is the best.";
		"Lady Slash goes off-line with a bang!";
		"Yippeee!";
		"One Neural Overload with sashimi to go!";
	  "I'll bet that purged your hard drive!";
		// 0 = enemy name
	} //end type

	type "kill_rail" 
	{
		"I love modern technology.";
		"So I'm a camper. Sue me.";
		"Ka-POW!! **[giggle]**";
		"Not a BFG, but I, like, do what I can.";
		KILL_RAIL0;
	} //end type

	type "kill_gauntlet"
	{
		"Nothing like a friendly touch to make a girl's day.";
		"I've got more where that came from.";
		KILL_GAUNTLET0;
		KILL_GAUNTLET2;
	} //end type

	type "kill_telefrag"
	{
		"That was so wrong in all the right ways";
		TELEFRAGGED1;
		TELEFRAGGED3;
		"Too much data, not enough storage.";
		"I'm a terminal download!";
	} //end type 

	type "kill_insult"
	{
		"Wrongful!";
		KILL_INSULT2;
		KILL_INSULT9;
		KILL_INSULT22;
		KILL_INSULT30;
		"And the Lady Slash scores again.";
	} //end type

	type "kill_praise"
	{
		PRAISE2;
		"You dance really fine, babe. Just not fast enough.";
		"Nice moves, but not quick enough, ", 0, ".";
		"Hey, ", 0, " Let's see you try that move with skates on.!";
		PRAISE3;
	} //end type

	//================================================================
	//================================================================

	type "random_insult"
	{
		"You! ", 0, "! Out of the gene pool!";
		"Did you buy those implants in a pawn shop, ", 0, "?";
		"Can't you ", fighter, "s die with a little more style than that?";
		TAUNT4;
		TAUNT_FEM;
		TAUNT_FEM1;
		TAUNT_FEM8;
		TAUNT7;
	} //end type

	type "random_misc"
	{
		MISC0;
		MISC6;
		MISC10;
		MISC13;
		one_liners;
		"Like the skates?";
		"I so love a challenge. But I guess I have to take what I'm given.";
		"Zaphod?  Isn't he just 'some guy'?";
	} //end type

} //end chat slash
