//===========================================================================
//
// Name:	       	Razor_c.c
// Function:		chat lines for Razor
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			Seven Swords & R. A. Salvatore
// Editor:			Paul Jaquays
// Last update:		Oct. 10, 1999
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats
chat "razor"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"It's a beautiful day in the neighborhood.";
		"For the right offer, I could probably let you get out of here.";
		"The game's afoot!";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Time to move on to better hunting grounds.";
		"Hitting the road again ... ";
		"Parting 'tis sweet sorrow ... ";
		GOODBYE;
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Ah, ", 4, ". I know this place so very well.";
		"Welcome to my zone of pain ... well, it's really just your pain that interests me.";
		"What a coincidence, meeting you all here again.";
		"Welcome! Just leave your valuables with my assistant.";
		HELLO8;
		// 0 = bot name
		// 4 = Level's title
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"All this fighting doesn't leave much time for my boys to practice their dancing.";
		"~Don't smile, ", 2, ". You just got a whole convoy looking for your sorry behind.";
		"Here's a career pointer, ", 3, "... Learn to cook.";
		// 0 = bot name
		// 2 = opponent in first place
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		4, " looks so much better when I turn you all into little stains.";
		"I win! Alas, no mountains left to climb.";
		"And you're wondering, how can ~one man be so good?";
		"Hey, ", 3, ". Just to be sporting next time, I'll lash you to death with my tongue.";
		// 0 = bot name
		// 3 = opponent in last place
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"Conventional wisdom says no fooling before a match ... I should've listened, eh?";
		"Okay, winner buys the first round.";
		"I'm afraid I'm not taking this at all well.";
		DEATH_INSULT3;
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Argh! Foul! Foul, I say!";
		"The old brick-in-the-gauntlet trick, eh, ", 0, "?";
		"I try, oh how I try to observe the rules. All bets are off, now.";
		DEATH_TALKING;
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Well done, ", 0, ", Well done!";
		"My, my, ", 0, ". Getting a bit big for your britches, eh?";
		"I definitely didn't enjoy that.";
		"Well, I'm waiting for an apology, ", 0, ".";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		DEATH_INSULT5;
		HIT_NOKILL0;
		"I hear the pitter-patter of dripping blood. Must be yours, ", 0, ".";
		//"Will you stay down, ", 0, "? These ", 1, " cost money!";
		// 0 = opponent
		// 1 = ammo type
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Quit crowding the road!";
		"Have you no manners? I was here first.";
		TELEFRAGGED1;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"I can see the grou ... ";
		"I always wanted to jump a canyon on my hog ... but this is ridiculous.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		DEATH_LAVA0;
		"This isn't going well with the Mexican dinner special I just ate.";
		"And here I forgot to bring marshmallows.";
		"It's a luau, and I'm the roast long pig.";
		"I'm the toast with the most!";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"Somebody forgot to clean the spa! Oo oo oo!";
		"A bit too much caffeine for my tastes.";
		"A spoon, a spoon! My kingdom for a spoon!";
		"Roto-Ryders just hate oil... or whatever... slicks.";
		"Aren't there government agencies that deal with this stuff?";
		"I think I just found a batch of melted politicians.";
		"And here I am wearing dry clean only.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Well, I guess I can't drink my way out of this ~one.";
		"I think we can safely assume that the drought is over.";
		"Could somebody get me a towel?";
		"Call me a looney, but I think I saw the blue jet down there.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		DEATH_SUICIDE6;
		"See, it takes a god to kill a god.";
		"I'm always willing to make a noble sacrifice.";
		"There, ", 0, ". That should help you catch up.";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Okay, okay, but don't touch the bike!";
		"Why is my hair standing up straight? Oh, not the hair, man!";
		"So, ", 0, ", you can use a gauntlet without hurting yourself.";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"I didn't even hear the shot.";
		"See what happens when they let children play with guns.";
		"I thought I recognized that ~rail trail color, ", 0, ".";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"Drat! The house maids aren't due for another ~two days.";
		"Whoa, I feel like I just got locked up with a convoy of ~eighteen wheelers.";
		"I would have sworn that the BFG was banned by the Arena Masters.";
		"Now I know how a ~bug on a windshield feels.";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Lucky and good aren't the same thing.";
		"Next time, try it with your eyes open, junior.";
		");-P";
		"Ouch!";

		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"The road is yours now, ",0,". Use it well.";
		"Okay, so I can't beat you with ~one hand tied behind my back, ", 0, ".";
		"Wow, was that the new and improved ", 1, "?";
		D_PRAISE4;
		// 0 = enemy name
		// 1 = opponent's weapon
	} //end type

		type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Did anyone get the license number off that ~eighteen wheeler?";
		"My bike! Where's my .... awwww crap.";
		"Becoming one with the universe sucks.";
		"How long before we can look back on this and laugh?";
		"How nice for you, ", 0, ". You've finally found a weapon to match your skill.";
		"Roadhog!";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"Fore!";
		"Clear the road!";
		"Why yes. I DO own the road.";
		"You are all bug-splats on my windscreen.";
		"Actual mileage may vary!";
	  "Did I miss seeing the warning label on this ~kamikaze thing?";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Just a whisper on the wind, eh? Not!";
		"Adopt a highway, skidmark.";
		"So you're a member of the 'slow-moving target gang,' eh, ", 0, ".?"; 
		KILL_RAIL2;
		KILL_INSULT10;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Look, I'm a ~bug light.";
		"Think I'll use this to weld a new tailpipe on my bike next.";
		"Sorry, ", 0, ", gotta run. Bitterman needs a jump start.";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Hey, ", 0, ", it's not my fault. I skidded on the sand.";
		"Thanks for cushioning the entry, ", 0, ".";
		TELEFRAGGED5;
		"Pull yourself together friend!";
		"Sorry. I didn't know this stall was occupied.";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"I'm next, right?";
		"I like your technique, ", 0 ,". Could you show me that again?";
		"**[shakes head]**";
		"Bravo, ", 0, "! It's not everyone who can frag themselves with their own gauntlet.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"How many times do I have to kill you before you get a clue, ", 0, "?";
		KILL_INSULT0;
		KILL_INSULT5;
		"The joy I have in wiping out galactic scum like ", 0, " is incredible.";
		KILL_INSULT21;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"It was an honor to kill you, ", 0, "... I'll gladly do it again and again.";
		"You're good, ", 0, ", but only the best can survive.";
		"You are a challenging target, ", 0, ".";
		PRAISE4;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"Nothing personal ... well, seeing you, ", 0, ", it becomes personal.";
		0, ", you smell too wretched to even think of hiding.";
		"If owning the road means having ", 0, " as a tenant, I'm giving the road back!";
		"The bigger they are, the harder they fall ... and you look about redwood-sized.";
		"Prepare to be stomped!";
		"You don't look so scary to me.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"The shades are bionic, but the baby-blues behind them are all mine.";
		"No, the Roto-Ryders don't clean drains. That's someone else.";
		"Shades fogging again? I must have lost a seal on these things.";
		"Is this some kind of entertainment ritual?";
		"So this is what you rogues call entertainment.";
		"I'd keep an eye on that storm coming in, if I were you.";
		"A rather studious looking ensemble, if I do say so myself.";
		"ohmigawd.";
		"You've just violated my third law of fairness, ", 0, ".";
		"Threedy Relmz? Oh, you must mean ol' Duke!'";
		"That should put a nasty wrench in your gearbox.";
		"Must my trusty metal steed and I pummel some sense into your skull?";
		"This place is being over-run by Raster-farians.";
		MISC0;
		MISC11;
		MISC14;
		"Hey, ", 0, ", wanna buy a full dresser?";
		"We're not a gang, we're a club!";
		one_liners;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end razor chat

