//===========================================================================
//
// Name:		Crash_c.c
// Function:      chat lines for Anarki
// Programmer:	MrElusive (MrElusive@idsoftware.com)
// Last update:	November 13, 1999
//Author:	Paul Jaquays
// Tab Size:		3 (real tabs)
//===========================================================================

chat "crash"
{
	#include "teamplay.h"

	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		"Welcome to the Arenas Eternal, warrior.";
		"You have entered the Arenas Eternal, ", 1, ". Prepare to fight.";
		// 0 = bot name
		//1 = random player
		//4 = level's title
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Yeah, I think the new warrior may be on the the fast track.";
		// 0 = bot name
		//4 = level's title
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Alright, let's see what ya got.";
		"It's time for Crash to lay down the law here.";
		"If it's not too much to ask, would you make this interesting for me.";

		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Not what I expected. Definitely not.";
		"Can we try that again, ", 2, "? I think you were just lucky.";

		// 0 = bot name
		// 1 = random opponent
		// 2 = opponent in first place
		// 3 = opponent in last place
		// 4 = level's title
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"~Don't go for a perfect record, ", 3, ". Try to win some matches.";
		"If you can't beat me, it's time to take up something less challenging.";
		"Do it again. But this time, with feeling.";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"I guess I had that coming.";
		"Good luck. ~Don't be a stranger ... well at least not stranger than you already are.";
		"Have fun storming the castle.";
		"Good luck, ", 2, ". But I think Ranger is going to toast your biscuits.";
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Now that was nice manners. Bet you fart in public too.";
		"Keep that up and you'll be REAL popular here.";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"I'm tougher than I look. Really.";
		"My ~momma didn't raise no cream puff.";
		"Just a flesh wound.";

		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Not my day. Need to correct for windage next time.";
		"Nuts. Just winged you.";
		"You want a bandage or an aspirin for that?";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Bizzzzaap!! Got me like a ~bug in a zap trap.";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Like a ~bug on a windshield again.";
		"Scrape me off the floor and send me back to the front.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Always a back flip. Never a swan dive. So predictable.";
		"Lava. Hazard or nuisance. You be the judge.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"Feh.";
		"Ever wonder what happens to all that toxic waste they store?";
		"Gick. And I just polished those shoes.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Back to the kiddie pool for me, I think.";
		"Hand me a towel, will you?";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"After all this time, I ought to be more careful.";
		"Even the best of us make the occasional foot fault.";
		"~Don't laugh, but we used to call ", androbot, ", 'King of the Lava Divers.'";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"You don't EVER touch a lady like that. You hear me?";
		"Now aren't you the tough ~one.";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Fastball, low and across the plate! She's outta there!";
		"That ~one hits you before you hear it. Ouch!";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"Ow. Really.";
		DEATH_BFG2;
			// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Beginners luck... Again.";
		"Hmmmmm.I occasionally need to be reminded that I'm not immortal.";
		"Was that REALLY your best shot?";
		"I ought to charge by the hour for this.";
		"So... you're up to what ... 3 frags an hour?";

		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		
			"Wooo! Looks like some new competition has moved into town.";
			"Oh my! Xaero look OUT!";
			androbot, " better keep an eye on his ~rear view mirror now.";
			femalebot, " can't coast along on her good looks anymore.";
			"I'd say that this train is bound for glory.";
			"Sorry. I'm not much of a challenge.";
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Tactical nukes make poor neighbors.";
		"Can't we all just get along?";
		"You'll hear from the neighborhood association about this one.";
		"That's a grand slam for the visitors.";
		"Well, my headache's gone now.";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with rail gun
	{
		"Awesome. I never get to play with the big toys.";
		"Oh. You meant don't press THAT button.";
		"Sort of like a group telefrag.";
		"That was a blast!";
		"Ugh. Someone call housekeeping.";
		// 0 = enemy name
	} //end type

	
type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"I have a deep and abiding love for power tools.";
		"Explosive decompression works for me.";
		"Can I have the slug back? I'd like to have it bronzed.";
		"That ~one's out of the park.";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Oops. ~Don't know my own strength.";
		"Well, I am stronger than I look.";
		"If the glove don't fit...  Looks like it fit. Heh.";
		"Whattayaknow, the batteries in that were still good.";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"You have just been vaporized, ", 0, ".";
		"Now that was a classic telefrag.";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the players kills self
	{
		"~Don't give up so easily. I'm tough, but not THAT tough.";
		"And you had so much to live for.";
		"I'm not cleaning that mess up.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"I told you to duck.";
		"Of course you don't have an ounce of brains in your head -- they're all on the wall there.";
		"Easy frag.";
		"Wheee. I love fragging easy targets.";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"You blow up real pretty.";
		"Were you trying to give me an easy frag there, ", 0, "?";

		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
	"Remember to take the training wheels off before you leave, ", 0, ".";
	"What a coincidence. My grandmother said she whupped your behind just last week.";
	"There must be worse nicknames than 'Ol' Harmless.'";
	one_liners;
	"Any time you want to start the fight, just let me know.";
	"Big clue here. ~Don't quit your day job, ", 0, ".";

		// 0 = name of randomly chosen player
		// 1 = last victim name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"How do you say '", list_sort, "' in ", language, "?";
		"Huh. I was going to say something, but now my mind's a blank.";
		"Hope you're having fun.";
		"I told ", androbot, " that you're the ~one who dented his new ", weapon, ".";
		"When you see Doom, ask him why he never calls.";
		one_liners;
		// 0 = name of randomly chosen player
		// 1 = last victim name
	} //end type
} //end crash chat	