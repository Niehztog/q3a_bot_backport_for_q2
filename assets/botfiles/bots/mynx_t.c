//===========================================================================
//
// Name:			Mynx_c.c
// Function:		chat lines for Mynx
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			The Seven Swords, R.A. Salvatore, & Paul Jaquays
// Editor:			Paul Jaquays
// Last update:		May 16, 2000
// Tab Size:		3 (real tabs)
//===========================================================================

chat "mynx"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Put on some dancing music. ", 0, " wants to play!";
		"Snap to it, you scruffy, ", fighter, "s, ~Mynx is back in town.";
		"Been a long time, ", 1, ". Miss me?";
		"Looks like I'm up against the 'Mystery Meat' again.";
		"Let me guess. We're not in Kansas... again.";
		"It's time to Party!!";
		HELLO6;
		HELLO10;
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Later, ", fighter, "s. I'm sure you'll all see me again in your dreams.";
		"Toodles.";
		"Ta Ta For Now.";
		"Later For you.";
		"Kisses for luck, ", 1, ".";
		"There's no place like home ... there's no place like home.";
		GOODBYE2;
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"You look at me and you think you've gone to heaven. And you are soooo wrong.";
		"This place could use some mirrors.";
		"The bigger the gun, the better the fun.";
		"I like warming up before a hot date.";
		LEVEL_START0;
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Well, this looks like a circle-whack.";
		"Look! A ", 0, " sandwich. Ooooo.";
		"Thanks for warming that podium up for me, ", 2, ".";
		"Always a bridesmaid, never a bride.";
		LEVEL_END2;
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Come on guys, I KNOW you like it when I'm on top.";
		"With a body like this, I just don't lose.";
		"I can even make killing look good.";
		"Photo op! Everyone smile!";
		LEVEL_END_VICTORY4;
		LEVEL_END_VICTORY1;
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"Bad idea, ", 2 ,". Nice guys don't finish first.";
		"Hey, ", 2, ". I guess you wanted to be on top this time, huh?";
		"Hmmm ... flat on my back and everyone above me. I've been here before.";
		LEVEL_END_LOSE1;
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Little ~one, never interrupt a lady when she's talking.";
		"So much for foreplay.";
		"Great, premature eradication.";
		DEATH_RAIL2;
		KILL_INSULT30;
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Okay, so you hit me, ", 0, ". If you think I'm going to call you 'master', forget it.";
		"Put it in your holster, minute-man.";
		"Good for ~one round, ", 0, "...up for a second?";
		"That's it, punk, I'm aiming lower.";
		PRAISE3;
		TAUNT_FEM5;
		// 0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Try not to splatter, ", 0, " ... I don't look good in red.";
		"Hold still, ", 0, ". I want to match my lipstick against that color.";
		"I'm sorry, did your voice go up an octave or ~two?";
		"I would have shot lower, but my scope is all fogged up.";
		"Bark for me, ", 0 ,"! Bark for me!";
		"Fantasize that, pimple-puss!";
		"** winks and blows kisses **";
		TAUNT_FEM4;
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Whew, ", 0, ", I'm usually the ~one who blows the other guy's socks off.";
		"I usually like a big ending ...";
		DEATH_TELEFRAGGED0;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Make a mold of the imprint, ", 0, "! It'll make a great doll!";
		"I wanted a suntan, not a windburn.";
		"What can I say, ", 0, "? Sometimes a girl's just gotta fly solo.";
		"This is ironic. I thought ", 0, " would fall for me.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Everywhere I go, I sizzle.";
		"I knew I should have worn protection.";
		DEATH_LAVA1;
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"My doc's got a shot that will clear this up.";
		"Sticky, sticky, always sticky.";
		"Um, ", 0, ", I hope this isn't yours.";
		DEATH_SLIME0;
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"I don't feel like skinny-dipping, ", 0, ".";
		"Hey, at least turn on the jacuzzi bubbles.";
		"~Don't worry, I brought my own floatation devices.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{	
		"I figured I'd take care of myself, since I know you couldn't do it.";
		"Well, ", 0, ", you said you like to watch...";
		DEATH_SUICIDE3;
		DEATH_SUICIDE2;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		0, " touched me! Can an infection be far behind?";
		0, " touched me and I haven't had my shots yet!";
		"Last guy that touched me without asking is singing ~two octaves higher now.";
		"I can think of a better place for you to put that hand, ", 0, ".";
		DEATH_GAUNTLET0;
		DEATH_GAUNTLET2;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Is that a pin-prick, ", 0, "? You bet.";
		"Couldn't even wait to get near me before you shot, eh camper?";
		DEATH_RAIL2;
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"Oh my! ", 0, "! I can tell I got you really excited.";
		"When I said fireworks, that wasn't what I had in mind.";
		"So now are you going to roll over and go to sleep, ", 0, "?";
		"Did the earth move for you too, ", 0, "?";
		DEATH_BFG1;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"~Don't pity me. It's better than having to keep looking at ", 0, ".";
		"When I said, 'Over my dead body,' this wasn't what I meant.";
		"My corpse, is the best you'll ever get, ", 0, ".";
		"~Don't flatter yourself, ", 0, ", I can fake this too.";
		DEATH_FEM_INSULT1;
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"You've been practicing alone, haven't you, ", 0, "?";
		"Finally, someone who knows how to handle their weapon!";
		"I like a man that shoots hard and shoots straight.";
		D_PRAISE1;
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Are you always this good, or is today just special?";
		"Dammit. You always do that before I'm ready.";
		"Next time, ", 0, ", wait for me.";
		"Omigod. Let me catch my breath.";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"I want you to know that I charge extra for that.";
		"It's always more fun with a group.";
		"Drat. I wasn't protected this time.";
		"Of course it's not safe, but that's part of the fun.";
		"Did the earth move for you too?";
	  "The first sample is always free.";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"What's the matter, ", 0, "? ~Don't you like my kisses?";
		"I did it because I couldn't bear to get closer to you, ", 0, ".";
		KILL_RAIL2;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Oooh, that tingled. I know a little glove that's sleeping in Mynx's bed tonight.";
		"So how does that stack up against what you fantasized it would be?";
		"This touch is for free ... next ~one's gonna cost you.";
		KILL_GAUNTLET0;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Now there's a hug ", 0, " won't soon forget.";
		"I've been known to make some people just explode!";
		TELEFRAGGED1;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		0, ", you dirtbag! Finishing before I even got a chance to play!";
		"Silly ", fighter, ", it's not how big your gun is, it's how you use it.";
		"You can't handle yourself. How are you going to handle me?";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Look on the bright side, ", 0, ". The last thing you saw before you died was... me!";
		"I've been told that I'm good enough to die for.";
		"If that's your biggest gun, ", 0, ", I did you a favor.";
		KILL_INSULT28;
		KILL_INSULT31;
		KILL_INSULT37;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Cheer up, ", 0 ,". You lasted longer than most of my playmates.";
		"You made the cutest face right before you died ... ";
		"Rigor mortis wasn't exactly what you had in mind, was it?";
		"It's not your fault, ", 0 ,". Big girls carry BIG guns.";
		PRAISE4;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"If I had toys that shook as much as ", 0 ,", I'd never leave my house.";
		"You look familiar, ", 0, ". Did I ever do a contract on you?";
		"If looks could kill, ", 0, ", your face would be a serial murderer.";
		"Too many curves, ", 0, "? Are you having trouble aiming at me?";
		"Your fly's open, ", 0, "... oh, no big deal.";
		"It's okay, ", 0, ". It happens to lots of men.";
		"Beautiful women make you nervous, ", 1, "?";
		MISC13;
		TAUNT_FEM3;
		TAUNT_FEM5;
		TAUNT_FEM8;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"Am I ... distracting you, ", 0, "?";
		"Let me show you some real '~Quad Damage.'";
		"Who was the interior decorator here? Boris Karloff?";
		"Hey everybody! ", 0, "'s still shooting blanks.";
		"Is my panty-line showing?";
		"Like the shades? Got 'em off some guy, the 'Two' or 'One' ... something like that.";
		0, " runs like a cheap pair of pantyhose.";
		"I'm a 64 bit girl in an 8 bit world.";
		"I can play rougher, if that's what you want.";
		one_liners;
		"Do you know how hard it is to find a ", 5, " that color coordinates with my outfits?";
		"Designer shotgun. Do you like it?";
		"What about MY fantasies?";
		TAUNT9;
		MISC8;
		MISC2;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end mynx chat

