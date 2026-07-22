
//===========================================================================
//
// Name:			Lucy_c.c
// Function:		chat lines for Lucy
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			The Seven Swords, R.A. Salvatore, & Paul Jaquays
// Editor:			Paul Jaquays
// Last update:		May 11, 2000
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats
chat "lucy"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{	
		"I'm here to frag someone's lazy butt and I'm not leaving until I do.";
		"What's a girl need to do to get some action around here?";
		"Somebody told me ", botnames, " was hanging here.";
		"Hey, looks like someone gave ", 4, " a new coat of paint.";
		"Whose hawg was that parked outside?";
		"Mistress Lucy is in the house.";
		HELLO5;
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{	
		"*YAWN* I'm going to find some real excitement.";
		"Typical ", fighter, "s ... all talk, no action.";
		GOODBYE2;
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{	
		"Alright , ", fighter, "s, these boots were made for kicking heinie and that's what I came to do.";
		"Normally, I got no time for this... but I'll make time just for the fun of fragging ",1, ".";
		"Back to your cells or Momma Lucy spanks!";
		LEVEL_START0;
		// 0 = bot name
		// 1 = randomly chosen opponent
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{	
		"This will NOT look good on my work evaluation.";
		"Stink! I can't move in these heels.";
		"Sunuva ... I'm writing ", 2, " up for harassment.";
		LEVEL_END2;
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{	
		"Hey, ", 3, ", I was a state employee. I kill smarter not harder.";
		"Aw poor baby. Come here and let Lucy give you a hug ", 3, ".";
		"Tap the keg, ", fighter, "s. We're partying down tonight!";
		"Drinks are on me, boys!";
		LEVEL_END_VICTORY1;
		// 0 = bot name
		// 3 = opponent in last place
	} //end type


	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{	
		"Daddy must be rolling in his grave.";
		"Oh, this girdle is killing me ... literally.";
		"Hey, ", 2, ". Let's grab a couple cold ones and you show me those fancy moves of yours.";
		LEVEL_END_LOSE0;
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{	
		"You must be some kind of management trainee, ", 0, ".";
		"Big Mistake there, ", 0, ". BIG mistake.";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{	
		"Come here, ", 0, ", and let Momma show you how to use that thing.";
		"You do that again, ", 0, " and I'm going to take that away and beat you with it.";
		"I can't believe you hit a lady.";
		"Payback's a bitch, and so am I.";
		TAUNT_FEM1;
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{	
		"You think that hurt, ", 0, "? Wait until I put my boots to you.";
		"you got nothing, ", 0, ". And I'm making less of it with each shot.";
		"I never frag 'em right away ... (wink, wink).";
		"Did I shoot off anything important, ", 0, "?";
		TAUNT_FEM4;
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{	
		"Oooh, I can feel those diet pills working.";
		"She's everywhere!";
		"Now that's spreading myself a bit too thin.";
		DEATH_TELEFRAGGED2;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{	
		"Oh great, gravity. That's all I need.";
		"I hope I splattered all over you, ", 0, ".";
		// 0  = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{	
		"I can just feel the pounds melting off me.";
		"Now that's Barbeque!";
		DEATH_LAVA0;
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{	
		"Hey, nobody said they were serving key-lime pie from the chow hall.";
		"Come on in, ", 0, ".  Your whole family's here.";
		DEATH_SLIME0;
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{	
		"Someone lied to me. There ain't no lifeguards here.";
		"I should be floating. Must be the boots.";
		"I really hope that's a candy bar.";
		"Hey, ", 0, "! Got time for a skinny dip?";
		DEATH_DROWN1;
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{	
		"Dang state trooper hand-me-downs.";
		"Well, that's just about it.";
		DEATH_SUICIDE4;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{	
		"Oooh, can I have your phone number, ", 0, "?";
		"You sure know how to show a lady a good time, ", 0, ".";
		DEATH_GAUNTLET2;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{	
		"What the .... I wear a badge, not a target.";
		"Yeah, be a happy camper, ", 0, ".  Be very happy and be VERY afraid.";
		DEATH_RAIL2;
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{	
		"**[Sniff]** ... **[sniff]** ... Is someone frying bacon?";
		TAUNT_FEM3;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{	
		"You wouldn't have lasted ~two minutes in the joint, ", 0, ".";
		"Lousy ", fighter, "!";
		"Can you find anyone else to hammer on?";
		"You must have planted a locator ~bug on me, ", 0, ".";
		DEATH_FEM_INSULT;
		DEATH_INSULT6;
		// 0 = enemy name
		// 3 = opponent in last place
	} //end type

	type "death_praise" //praise initiated when the bot died
	{	
		"Alright, you've convinced me to go on a diet.";
		"So, ", 0, ", you think you've got my moves figured out?";
		"OK, ", 0, ". You're buying the first round tonight.";
		 D_PRAISE1;
		// 0 = enemy name
	} //end type

		type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"You trying to tell me something, ", 0, "? Is it my breath?.";
		"Yeah. I thought I saw you sneaking something into your cell, ", 0,".";
		0, ", you just became a grease stain on the pages of history.";
		"Blast it, ", 0, "! I told you not to put batteries in that thing!.";
		"Jailbreak!!";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with rail gun
	{
		"I needed to lose a couple pounds anyway.";
		"I hope to hell I took some of you with me.";
		"Whew! That had more kick than I thought it would!";
		"OK everyone. You can start whining now. Big bad Lucy is done letting off steam.";
		"Just call me 'Ms. Spontaneous Human Combustion!'";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{	
		"Never mess with a woman on a diet.";
		"I spent a lot of time in deer stands.";
		"I took my degree in target shooting at the academy.";
		"Always wanted to do that to the cons.";
		TAUNT_FEM7;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{	
		"I just wanted to show you ~one of my special toys.";
		TAUNT_FEM2;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Come back when you think you're ready to handle a real woman.";
		"I felt so close to you then, ", 0, ". Maybe too close, huh?";
		";-D";
		TELEFRAGGED5;
		TELEFRAGGED7;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when player kills self
	{
		"Hey, who had the suicide watch on ", 0, "?";
		"They always make me clean up after this kind of thing.";
		"There's easier ways out than the way you done it, ", 0, ".";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{	
		"Let me give you some advice about real women, ", fighter, ". Awwww ... forget it.";
		"I hope it hurt. You deserve to hurt, ", fighter, ".";
		TAUNT_FEM4;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{	
		"Well at least you lasted long enough to make me sweat, ", 0, ".";
		"You done well for a rookie, ", 0, ".";
		"May the best man win ... yeah, I love THAT saying.";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{	
		"Who gave you permission to dress that way, ", 0, "?";
		"I've been on diets that lasted longer than you, ", 1, ".";
		"If my dog had a face like ", 0, ", well, I'd just kill it.";
		TAUNT_FEM;
		"yeah, keep yapping, ", 0, ". You're just like a dog barking at both ends.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{	
		one_liners;
		"I ain't yer type, ", 0, ".";
		"Buy a girl dinner?";
		"Hey! Somebody send out for some ", food, ", OK?";
		"I am PMS personified today.";
		"What do you mean 'time of the month?' I'm always this ornery!";
		"Does this gun make me look fat?";
		"When's that roach coach gonna get here? I'm starving.";
		MISC9;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end lucy chat
