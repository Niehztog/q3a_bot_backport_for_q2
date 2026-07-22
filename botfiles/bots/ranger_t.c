
//===========================================================================
//
// Name:			Ranger
// Function:		chat lines for Ranger
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			Seven Swords, R.A. Salvatore & Paul Jaquays
// Editor:			Paul Jaquays
// Last update:		May 19. 2000
// Tab Size:		3 (real tabs)
//===========================================================================

chat "ranger"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		HELLO7;
		"Special orders for ", 4 ,": ... 'Remove all hostiles!'";
		"Where have the portals taken me now?";
		"~Ranger reporting in. Am engaging the enemy in close combat.";
		"I'm surrounded by Quake minions. Laying down suppressing fire.";
		"Negative. Negative. I'm going in.";
		"~Ranger to Base. ", negative, ", I can handle the opposition here.";
		"~Ranger to Base. ", affirmative, ", We're good to go here.";
		HELLO5;
		// 0 = bot name
		// 4 = Level's title
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		GOODBYE5;
		"~Ranger to base ... evac of ", 4, " commencing!";
		"I'm going for reinforcements! Cover me!";
		"We're pulling out ...";
		// 0 = bot name
		// 4 = Level's title
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Go! Go! Go!";
		"You set 'em up, I'll knock 'em down!";
		"Commencing operation ", operation1, " ", operation2, ".";
		LEVEL_START0;
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Hey, ", 2, ", you train with the SEALS?";
		LEVEL_END1;
		"A win's a win, but that was a waste of freaking time.";
		"This operation's outcome is not acceptable.";
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"All in a day's work.";
		"Semper fi, do or die! Semper fi, I did, you died!";
		LEVEL_END_VICTORY1;
		"Hey, ", 3, ", you sissied out of boot camp, didn't you?";
		"Ranger to base.", 4, " is secured. Phase two can begin. Over.";
		"Operation ", operation1, " ", operation2, " is complete. Mission objectives obtained.";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"Failure is not a part of a ~Marine's vocabulary.";
		"Operation ", operation1, " ", operation2, " has failed to achieve its objective.";
		"Freaking Navy-issue equipment!";
		"Son-of-a-butt-smellin'-momma-dog!";
		"~Ranger to base. Mission Objectives not attainable. Permission to withdraw requested.";
		"The Enemy has bested us.";
		LEVEL_END_LOSE2;
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Here's a dime, ", 0, ". Call the chaplain. You're gonna need him.";
		"That confirms that I have the moral high ground here.";
		"Combatants will respect the Geneva Conventions.";
		"I wasn't mad until now, ", fighter, ".";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"You're just pissin' me off, ", 0, ".";
		"I've cut farts that hurt more than that.";
		"Nice shot, private-first-class-wiseguy.";
		0, ", I'm gonna rip your arm off and beat you to death with it.";
		"Not using regulation ordinance again, eh ", 0, ".";
		"I'm gonna convene a ~one man court martial right now.";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"I'm goin' slow, cuz I like the look on your face, ", fighter, ".";
		"Stay still, ", 0, ", so I can connect the dots.";
		"You got guts, ", 0, ". I can see most of 'em now.";
		"Gotta correct my aim for windage.";
		"Not dead? Must be defective ordinance.";
		"Let me guess, ", 0, ". That was a headshot.";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"What brainy-butt occifer plotted this course?";
		"I hate portals with a freakin' passion.";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Wasn't I issued a parachute?";
		"I failed FLYING 101 in Basic.";
		DEATH_FALLING;
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"It's just form-fitting armor. Wait till it cools.";
		"They taught me to keep my head down.";
		"Stop, drop, and roll!";
		"Nope. Still hate this stuff.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"MRE production plant. Freakin' great."
		"I hope I can get a shot to clear this up.";
		"Ranger to Base: Toxic Spill complicating operation ", operation1, " ", operation2, ".";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Dropped the chute on the LV a bit early, didn't we?";
		"Join the Navy, they said. See the world, they said ... ";
		DEATH_DROWN1;
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"In a blaze of glory! Oh, hell, at least it's a blaze.";
		"Now that I've demonstrated how NOT to do it ...";
		DEATH_SUICIDE3;
		DEATH_SUICIDE6;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"I knew you were there, ", 0, ". I was just testing you.";
		DEATH_GAUNTLET1;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Whatever happened to waiting till you see the whites of their eyes?";
		DEATH_RAIL2;
		"I hate taking the point.";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"MEDIC!";
		"Incoming! Lots of incoming!";
		DEATH_BFG0;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Best trophy you'll ever get, scumbag.";
		"So your momma's apron proved to be good armor. Big deal.";
		"~Ranger to Base, we're under heavy fire here.";
		DEATH_INSULT5;
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Okay, ", 0, ", I'd salute you ... if I could find my head!";
		"I'd give you my boots, but I ~don't think you could fill 'em!";
		"I can respect a shot like that, ", 0, ".";
		"Pin me up and call me a target. An outstanding shot, ", 0, ".";
		D_PRAISE6;
		// 0 = enemy name
	} //end type

		type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"The medic's got medications that can help you cope with that, ", 0, ".";
		"~Ranger to Base, enemy has deployed tactical nukes. Advise.";
		"~Ranger to Base. Operation ", operation1, " ", operation2, " at risk. Advise.";
		"All that training just to qualify as hazardous waste.";
		0, ", I'm gonna make you eat the next one of those, ", 0, ".";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"Quakin'!!!";
		"Blast. I think I missed one of ya.";
		"Airstrike!";
		"Not this time sucker!";
		"This is too much like fun. I gotta get out of the house more.";
	  "Hey ", 0, "! Anybody tell you that you got a purty spleen?";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"SBD, Silent But Deadly... and I ain't talking spicy food.";
		"I just don't get to use these ~railgun's often enough.";
		KILL_RAIL1;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"'Mr. Glove' just wanted to be friendly, ", 0, ".";
		"See, I used to have this axe....";
		"See how close I can get, ", 0, "? I can whisper in your ear.";
		KILL_GAUNTLET2;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Call me ~Ranger, human hand-grenade.";
		TELEFRAGGED5;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"Way to go, kamikaze. Next time rent a plane.";
		"That's the coward's way out, ", 0, ".";
		"Trying to go negative on me, eh, ", 0, "?";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"There's a letter your Mommy don't want to see, ", 0, "!";
		"I'd take your dog-tags, ", 0, ", but there's no market for baby bracelets.";
		KILL_INSULT2;
		KILL_INSULT25;
		"You quit in the third minute of basic training, right ", 0, "?";
		"~Ranger to Base, the enemy is taking heavy losses.";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"You practiced with paint-balls, didn't you, ", 0, "?";
		"Well done, ", 0, ", we'll put a flag on your box.";
		"You almost got it right, maggot.";
		PRAISE2;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"You got 'retreat' down to a science, ~don't you, ", 0, "?";
		"Hey, ", 0, ", get that butt down. Oh, that's your head.";
		"If I gave myself a sucking chest wound, you'd still lose, ", 1, ".";
		"Surrender and it will go easier for you.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"Hey, ", 0, ", is my gig-line straight?";
		"From the Halls of Montezuma ... hmm hmm hmm ... hey, anyone know the rest of it?";
		"Operation ", operation1, " ", operation2, " is runnin' smooth as silk.";
		"Surrender is not an option.";
		one_liners;
		GUYTALK;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end chat


