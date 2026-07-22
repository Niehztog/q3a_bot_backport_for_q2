//===========================================================================
//
// Name:			Xaero_c.c
// Function:		chat lines for Xaero
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			R.A. Salvatore, The Seven Swords, and Paul Jaquays
// Editor:			Paul Jaquays
// Last update:		May 23, 2000
// Tab Size:		3 (real tabs)
//===========================================================================

chat "xaero"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Now that we are all here, let the lesson begin.";
		"Welcome unenlightened ones. Learn, and then die.";
		"The bee is drawn to the flower, yet the spider lurks below.";
		"Do I hear a sharp intake of breath? Did you not expect me?";
		"A mole tunnels endlessly to dig his warren. I command it; it is done.";
		"Today's lesson will be pain.";
		"Greetings, ", 1, ", You come for another lesson?";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Would a dragon swat a fly simply because he could?";
		"You could not stand in a gentle breeze, yet I am a hurricane.";
		"Greater mountains to climb, though not yet have I found a challenge.";
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"This day shall the new prophecy be written. Your blood shall be my ink.";
		"Pray for a quick death, and if you are fortunate, I shall answer.";
		"It is time for your first lesson: humility.";
		"I can feel the terror as my shadow crosses the Arena.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Only through worthy opponents can I ascend beyond this.";
		"I walk the edge of a fine sword. On ~one side, mediocrity. On the other, victory.";
		"Confusing is the heat of battle. I fear the count is not correct.";
		"At last the slate has been put to the chalk.";
		"There is success, or failure. There is no mediocrity.";
		"Only the Chosen can reveal the Chosen, ", 2, ". We are ~one in glory.";
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"The sun rises. The sun sets. I win. These are the precepts of a logical world.";
		"Wise is the warrior who appreciates his opponents. Alas, I am a fool.";
		"The rain yields to the stream. The stream cowers before the river. I am the ocean.";
		"Your blood is my wine ... my cup overfloweth.";
		"All fall before me.";
		"Your parts intermingle. I cannot sort them out.";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"At last, the irresistible force has met the immovable object.";
		"The lesson I could not teach myself: to see battle through the eyes of my foes.";
		"Now that I am finished stretching, the real fighting may commence.";
		"Humility is a lesson that you will not teach to me.";
		"You have not defeated me, you have freed me to seek the higher level.";
		"Only time conquers all.";
		"Only the Chosen can reveal the Chosen, ", 2, ". We are ~one in glory.";
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		0, ", it takes a child but once to learn that a flame will burn.";
		"If there is not honor, I cannot teach. I will kill you without instruction, ", 0, ".";
		0, ", when the master speaks, the student would do well to listen.";
		"Grab every advantage, ", 0, ". It will not be enough.";
		"This is no victory, ", 0, ", for glory pales with treachery.";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Defiler!";
		"Sacrilege!";
		"The bee stings but once ... then it dies.";
		"So, ", 0, "; has the kitten found its claws?";
		"Follow the road of chance, child, but follow it to the end.";
		"The instructor is pleased that you did not sleep through ALL of his lessons, ", 0, ".";
		"The rungs of the ladder are narrow. There is no room to pass.";
		"A true master kills with ~one shot ", 0, ".";
		"The wind must have corrected your errant shot, ", 0, ".";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"That is the first of the ~seventeen points of pain, ", 0, ". ~Sixteen to go.";
		"Well done, ", 0, ". You show the enviable ability to attract your foe's weapons.";
		"Be proud of your scars, ", 0, ". They were given by the master.";
		"Honor demands that you fall on a grenade ", 0, " for making such a whimper.";
		0, ", the smell of your charred flesh is incense that speaks to my soul.";
		"The beast is surrounded. Then it is struck. Then it tires. Then it dies.";
		"Do not fear, ", 0, ", for your wound will have no time to fester.";
		"That is but a hole through which the maggots might enter.";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Alas, the coward will seek any means to victory.";
		"The intervention of fate alone allows you this hollow victory, ", 0, ".";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"I descend from on high. Too fast, I fear.";
		"Every creature finds its niche, where its form is perfection. Would that I was a bird!";
		"Alas, gravity, the universal truths apply to all. Even to ~Xaero.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Alas, the elements beckon me.";
		"I embark upon a river of stone, and the flames free my soul.";
		"This is the essence of creation, and destruction.";
		"Every creature finds its niche, where its form is perfection. Would that I were someplace else entirely.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"The simplest creatures are oft the most deadly.";
		"The willow bends to the wind, as the slime absorbs the strike. I am caught.";
		"As it was in the beginning, so it shall be in the end.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"The irony: that that which gave me life, now gives me death.";
		"Every creature finds its niche, where its form is perfection. Would that I was a fish.";
		"I know a thousand ways to kill, and yet, I could not spare a day to learn to swim.";
		"Remember little ~one: he who is the best armored ... sinks.";
		"How I have envied its perfect motion, and now at last, I join the fluid bath.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"It would seem that I have to do for you that which you cannot do for yourselves.";
		"A true warrior controls the fight, from beginning to end.";
		"A true warriors does nothing without purpose.";
		"In facing you, I have dishonored my sacred oath, and so I have fallen on my sword.";
		"There is honor in dying ... sometimes.";
		"I err. This is a new experience.";
		"Even in death, I am greater than thee.";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"You show promise, ", 0, " ... perhaps too much so.";
		"Perhaps I have underestimated you, ", 0, ".";
		"You have touched perfection, ", 0, ". Can you grab onto it?";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"The snake can strike but once against the quick of hand.";
		"I feel the sting of an insignificant wasp, and yet, I fear I am allergic.";
		"You fear to face me openly, ", 0, "? You are wise beyond your years.";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"Your tactics offend the philosopher in me, ", 0, ".";
		"Chaos reigns when a gnat is given the power of the bear.";
		"I end in a green blaze. There could be no other way.";
		"The candle that burns twice as fast, burns twice as bright.";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Cowards cannot comprehend the joy of valor, ", 0, ".";
		"The weak can win ... once.";
		"It must be true: fate is a path of many unforeseeable twists and turns.";
		"When all logic is cast aside, and all reason is reduced to madness, there is chaos.";
		"Call the scribe. This is an occasion which you will want to remember, ", 0, ".";
		"Brief are those moments of insanity, fortunately. Enjoy yours, ", 0, ".";
		"Boast of this kill, ", 0, ". It is the ~one mask to hide the fool that is you.";
		"Thrice cursed ", 1, " ", animal, "! You will learn your manners.";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"There is no trophy greater. I pray you are worthy, ", 0, ".";
		"You are my peer, ", 0, ". We are rare.";
		"I will see you on the higher level, ", 0, ".";
		"I salute you, ", 0, ", for magnificent deeds should not go unheralded.";
		"My corpse is my shrine. Build well your own, ", 0, ".";
		"You have taken a great step along the road to perfection.";
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Fate has thrown you a powerful tool. Though such are often wasted upon fools.";
		"The mosquito has borrowed the dragon's teeth.";
		"The flame of honor burns low and flickers in you, ", 0, ".";
		"Where will you hide, ", 0, ",  when the divine wind deserts you?";
		"The tiger faces death fiercely, ", 0, ". You are no tiger.";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"Loose the dragon. Unchain the thunderclap. Let chaos rule!";
		"Waste not, the providence of Heaven. It may not come again.";
		"A warrior knows that victory may not come til his own life is spilled out.";
		"The fire of my soul reaches out to caress the the spirits of my foes.";
	    "Such power should not be wasted upon lesser beings.";
		// 0 = enemy name
	} //end type


	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"I easily slide the thread through the eye of the needle.";
		"You must learn to anticipate, little ~one.";
		"A clean kill. Rejoice, ", 0, ", for I was merciful.";
		"You die, ", 0, ". Any way I choose.";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Often the master must take matters into his own 'hands'.";
		"The true warrior is a shadow within the darkness, a whisper among shouts.";
		"I am all about you, ", 0, "; omnipotent and ever-present.";
		"You see, ", 0, "? The scorpion has a tail.";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Out of my way!";
		"It is good to know that there was nothing of significance blocking my entrance.";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self with a weapon of craters
	{
		"Once there were people that honored such an act. They have inexplicably disappeared.";
		"Why do you try to steal the serpent's strike, ", 0, "?";
		"I agree with your choice, ", 0, ", for what was the point?";
		"He who accepts the inevitable is not a fool, ", 0, ". But he is still dead.";
		"You have taught me, little ~one, what not to do.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"They say there is honor in death. ~Do ~not believe them, ", 0, ".";
		"My soul for a true challenge!";
		"They waste my time by feeding me insects like you.";
		"The elephant steps on the ant. Does he even notice?";
		"You should die childless. Your existence insults the theory of evolution.";
		"There are among us those rare creatures who move better dead than alive.";
		"You lived as a coward, ", 0, ". And you died as a coward.";
		"With you in mind, ", 0, ", I tremble at the concept of reincarnation.";
		"Have you encountered ", botnames, "? That ~one failed against me also.";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Fine is the line between risk and foolishness. You stepped too far, ", 0, ".";
		"Great is better than good. I win.";
		"Take heart, ", 0, ", for those who do not try, cannot win.";
		"For a brief moment, you were a true fighter, ", 0, ". Sleep now, for that moment is passed.";
		0, ", you have rekindled my hope that ~one day I will be truly challenged.";
		"I have killed the likes of you a thousand times, ", 0, ". Warriors who nearly made the grade.";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"You walk as a drunkard, ", 1, "! ~One step ahead, ~two to the side, and ~three backwards.";
		"I would kill you quickly, ", 0, ", if I wasn't taking such pleasure from your shortcomings.";
		"Jump into the void, ", 0, " ...your chances will improve.";
		0, ", what made you choose this road of utter annihilation?";
		"They speak of bravery so that you will blindly walk to your death.";
		"I will prove to you that you are made of the basic elements, ", 0, ".";
		"Avoiding death by my hands is like avoiding fate itself.";
		"Does your world truly see you as a champion, ", 0, "?";
		"If you tremble upon seeing me, ", 0, ", you are a coward. If you do not, you are a fool.";
		"What words of comfort would you have me tell your next of kin, ", 1, "?";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"Where am I on the scale? At the top, because I CAN kill? Or at the bottom, because I DO?";
		"Tell me of your sensei, ", 0, ".";
		"The hawk or the wolf? ~One works in concert, the other, alone.";
		"Tell me of life outside the Arena.";
		"Anarki annoys me. His very name speaks of chaos.";
		"Perhaps I will keep you alive, ", 0, ", to serve as Klesk's translator.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end xaero chat







