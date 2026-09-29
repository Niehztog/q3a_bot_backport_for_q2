//===========================================================================
//
// Name:			Phobos_c.c
// Function:		chat lines for Phobos
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author: 			The Seven Swords & R.A. Salvatore
// Editor:			Paul Jaquays
// Last update:		Oct. 8, 1999
// Tab Size:		3 (real tabs)
//===========================================================================

chat "phobos"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Make it challenging!";
		"There are no guns in Heaven, so this must Hell again.";
		HELLO6;
		"Time to face your deepest fears, ", 1, ". Have you got it in you?";
		"Let's give the alien bastards a show they won't soon forget.";
		// 1 = random opponent
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Good hunting ... I'm finding ", 4, " a little slow.";
		GOODBYE5;
		"Does it always have to be ", 4, "? I'm off to find a better battle!";
		// 4 = Level's title
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Another group of fresh recruits ... although I prefer to call them 'targets.'";
		"Let the operation commence!";
		"Battle stations!";
		"Prepare to repel the enemy!";
		"Once more into the breach!";
		LEVEL_START0;
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Finally it starts to get interesting.";
		"Betrayed into the hands of fiends again.";
		"Unacceptable! I will not let my superiors down!";
		"If ", 1, " would have stayed out of my way, the battle would have turned!";
		"It could be worse. Look at ", 3, ", for example.";
		LEVEL_END2;
		// 1 = random opponent
		// 0 = bot name
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"A true leader will always finish first!";
		"A ~Marine finds a way! ~Don't whine, ~don't complain, just find a way!";
		"Careful planning and good execution are the keys to victory.";
		"I win ... and no ~one is surprised.";
		"Pfah! I think I still have ", 3, "'s entrails on me!";
		"Bag them and tag them!";
		LEVEL_END_VICTORY4;
		// 3 = opponent in last place
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{	
		"~One promise: this is not going to happen again.";
		"I do what I do to preserve the human way of life. ~Do ~not question me.";
		"I no longer trust ", 2, " to fight fairly.";
		"This was not an acceptable outcome to this mission.";
		LEVEL_END_LOSE0;
		LEVEL_END4;
		"You're going to die horribly, ", 2, "! Your slimy tactics insult us all!";
		// 2 = opponent in first place
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"How unfortunate that toads like ", 0, " are allowed in here.";
		"I told you to put your safety on! Report for administrative punishment!";
		"Inhuman scum!";
		"Traitor!";
		"Such treachery will not go unpunished!";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"That pin-prick is not going stop me from dominating ", 4, ".";
		"Well done, ", 0, ". Your skills are indeed worthy. Shall we duel?";
		"Typical attack of the under-educated opponent!";
		PRAISE;
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Bravo, ", 0, "! You make a superior practice target.";
		"Bah, it is merely a flesh wound. Stand up and start running!";
		"Okay, ", 0, ", I will accept your unconditional surrender.";
		"The chase! Ah, the thrill of the chase!";
		TAUNT3;
		"Drop the ", 1, " and step away from the wall!";
		TAUNT5;
		//1 = weapon used by shooter
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Quite a concept ... being everywhere at once!";
		"I am indeed 'a real nowhere man.'";
		"Sharing the same space with Crash would be a treat.";
		"Sharing the same space with ", femalebot, " would be a treat.";
		DEATH_TELEFRAGGED0;
		DEATH_TELEFRAGGED1;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"I did better in jump school.";
		"That cursed ledge needs a safety ~rail.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Sprinkle my ashes among those of my fellow officers.";
		DEATH_LAVA0;
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"It's so hard to tell the difference between real slime and the aliens.";
		"Argh! Bioweapons!";
		DEATH_SLIME0;
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"I bet I can hold my breath longer than you.";
		"It's a good thing my battle armor is waterproof.";
		"This is not what you think ... it just makes it easier to do push-ups.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Pay attention soldier; a classic example of what not to do!";
		DEATH_SUICIDE1;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"My gloves will come off if you wish to make war like this!";
		DEATH_GAUNTLET2;
		"Is that the same ", 1, " that you were picking your nose with, ", 0, "?";
		// 1= weapon used by shooter
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{	
		"I must talk with HQ about these 'improved' bullet-proof jackets.";
		DEATH_RAIL2;
		1, " is the perfect weapon for cowards like ", 0, ".";
		// 1 = weapon used by shooter
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"So this is the light at the end of the tunnel everyone talks about.";	
		DEATH_BFG0;
		"I think this passes the weapon effectiveness evaluation."; 
		// 1 = Weapon used by shooter
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Blast! Allowing you to kill me ", 0, " is an insult to this uniform.";
		"Bah. I'll wager you can't repeat that accident.";
		"Coward. Look me in the eye next time if you dare.";
		DEATH_INSULT4;
		// 1 = weapon used by shooter
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Congratulations, ", 0, ", I am victim to your victor.";
		"With you in my squad, we could take on the rest of these ", fighter, "s.";
		"I salute you for a shot well placed, ", 0, ".";
		D_PRAISE2;
		// 0 = enemy name
	} //end type


	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Soldier, the penalties for war crimes like that are severely permanent.";
		"In other circumstances, that act would make you a hero, ", 0, ".";
		"Soldier, you're a disgrace to that uniform.";
		"War just hasn't been the same since.";
		"These are not acceptable casualty figures.";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with rail gun
	{
		"I'll bet that took the crease out of your trousers, ", 0, ".";
		"I must be crazier than I thought.";
		"That's a small price to pay for a clear victory.";
		"Assessing peripheral damage now.";
		"Simply awe-inspiring.";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Rule #1, 'Avoid big bullets.'";
		"Rule #2, 'It is better to give than receive.'";
		"Rule #3, 'Don't let your lame, ~pansy, camping butt get anywhere in range of my return fire.'";
		KILL_RAIL1;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"I guess ", 0, " is too fragile to handle a real punch.";
		"Where I come from, we call that a 'glass jaw,' ", 0, ".";
		KILL_GAUNTLET2;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"I love the smell of toasted carcass in the morning!";
		TELEFRAGGED0;
		"This is a new weapon, I think. I'm going to have to try it again.";
		TELEFRAGGED1;
		TELEFRAGGED7;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"You have a very unusual technique, ", 0, ". Is this to confuse your enemies?";
		"~Three words, ", 0, ". 'Be more careful.'";
		"Keep your distance, ", 0, ". I won't get caught hanging around with anyone that stupid.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"I do not hold your inferiority against you, ", 0, ".";
		"Evolution predicted your extinction long ago, ", 0, ".";
		"'Kidnapped by Demonic Aliens' is a poor excuse for lack of skill, ", 0, ".";
		KILL_INSULT0;
		KILL_INSULT15;
		KILL_INSULT9;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"My heart aches when such a worthy adversary must perish.";
		"Touche'! Be proud, ", 0, ", you died bravely.";
		"Nice gibs there, honey.";
		PRAISE4;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{	
		"Run freaks! Run while you still can!";
		"Let me throw out a new word to ", 0, ". 'Tactics.'";
		"Is ", 0, " trying to give us the impression that by firing continuously ~one may hit something?";
		"Your fear of death will be your downfall, ", 0, ".";
		"Stupidity is not to be confused with bravery.";
		"Wearing 'Eau de Cat Box' again, ", 0, "? At least I can smell you coming."; 
		MISC11;
		1, ", please tell me why you seem to enjoy dying so often?";
		"Apparently ", 0, " has camped the ", 4, " before.";
		// 0 = name of randomly chosen player
		// 1 = bot name
		// 4 = Level's title
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"This is all so terribly real.";
		"My spirit is indomitable. I will persevere and survive this.";
		"What in the universe did you do to earn this billet, ", 0, "?";
		MISC0;		
		0, ", you grab the ", 5, " and I'll grab the ", 5, ". We'll own this place!";
		// 0 = name of randomly chosen player
		// 1 = bot name
		// 5 = random weapon from weapon list
	} //end type
} //end chat


