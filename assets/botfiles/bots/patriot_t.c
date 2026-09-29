
//===========================================================================
//
// Name:			Patriot_c.c
// Function:		chat lines for Malaki
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			The Seven Swords, R.A. Salvatore & Paul Jaquays
// Editor:			Paul Jaquays
// Last update:		1999-09-08
// Tab Size:		3 (real tabs)
//===========================================================================

chat "patriot"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{	
		"Let the feast begin.";
		"Good evening....hey, I had to say it.";
		"Once more to suffer under Tyranny.";
		HELLO6;
		HELLO8;
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{	
		"Another time, perhaps, ", fighter, ".  I've been around for years.";
		"When ~one is immortal, ~one can afford to bide one's time.";
		GOODBYE5;
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{	
		"Look how they line up like lambs to the slaughter.";
		"An abundance of life energy in here. I will enjoy ", 4, ".";
		LEVEL_START0;
		// 0 = bot name
		// 4 = Level's title
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{	
		"'Tis the story of my life; neither first nor last, alive nor dead.";
		"That was just for fun. What do you say we up the ante?";
		"Now I know to focus my attention on ", 2, " and not ", 3 ,".";
		LEVEL_END2;
		// 0 = bot name
		// 2 = opponent in first place
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{	
		"Like any great actor, I shall perform encore after encore.";
		"I should have told you this before, ", 3, "...you can't kill me.";
		LEVEL_END_VICTORY2;
		// 0 = bot name
		// 3 = opponent in last place
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{	
		"I beg thee, end my suffering.";
		"I do not know which is worse, eternal damnation or losing to ", 2, ".";
		LEVEL_END_LOSE2;
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	//==========================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{	
		"I shall feast upon your lifeforce, ", 0, ", for all eternity.";
		"Ah, my confused friend. The true pleasure comes from winning fairly.";
		"Marquess of Queensbury, I say!";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{	
		0," Tsk, tsk, tsk. You will have to do much better than that.";
		"I can smell your fear, ", 0, ".  See how it affects your aim.";
		"Ask yourself ~one question, ",1,". Can the dead be slain?";
		//0 = shooter
		//1 = random opponent
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{	
		"Now you will be easier to track, ", 0, ".";
		"The smell of your blood inspires me, ", 0, ".";
		"I do not wish it to be messy, ", 0, ". Stand still, if you please.";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"A banquet of blood, a meal made of my heart.";
		"That mist trick never works when I need it.";
		DEATH_TELEFRAGGED3;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{	
		"Once upon a time I could fly.";
		"At least I didn't land on a stake.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{	
		"So I was born in hell and to hell I return. I bid you adieu.";
		DEATH_LAVA1;
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{	
		"You can never find a good chamber maid when you need ~one.";
		"Stay out of this my good foes, it leaves nothing to feed upon.";
		DEATH_SLIME0;
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{	
		"At last, ~one of the few elements that can truly end my pain.";
		"Death take me. I embrace thee as my lover.";
		DEATH_DROWN1;
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{	
		"If you cannot end my eternal damnation then I will do it myself.";
		"No death is truly final.";
		DEATH_SUICIDE2;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{	
		"Do not mock me. I have no patience for fools.";
		DEATH_GAUNTLET1;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{	
		"I don't think that will heal right.";
		"Silver bullet, right?";
		curse, 0, ", for the foul ~rail ", animal, " that you are.";
		KILL_RAIL1;
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{	
		"At last the gates of hell are opened and I can rest.";
		"My friend, you have no idea how much this hurts.";
		"I did not think that final rest would be so ... green.";
		DEATH_BFG1;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{	
		"I cannot believe that a living mortal has vanquished me.";
		"Providence must be smiling on you, ", 0, ".";
		"The living have always been slow to learn.";
		DEATH_INSULT5;
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{	
		"I thank thee, ", 0, ", for releasing me from my torment, if only for a moment.";
		"You did well, ", 0, ". I would have offered you eternal life, alas!";
		"Luckily for us, ", 0, ", the colonies did not face an opponent of your skill.";
		D_PRAISE1;
		D_PRAISE5;
		// 0 = enemy name
	} //end type

		type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Blackguard!";
		0, "! Thou'rt a pestilence upon the land!";
		"You waste life so casually for one so young, ", 0, ".";
		"Greedy fool! All this will spoil 'ere it can be tasted!";
		"Ah me, the waste of it all.";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"The rush! Too ... too much all at once.";
		"Mmmmm. Notice how the flavors of each life blend into a savory melange.";
		"Tsk. Tsk. Add gluttony to my list of sins.";
		"Moments like this make unlife worth ... um ... unliving?";
		"Invention 'tis a marvelous thing.";
	  "Ahhhh, It does not get better than this.";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{	
		"Funny how death can come with just a whisper.";
		"I like it close enough to smell the blood ... but I take what I can get.";
		"Quite an improvement over black powder and lead shot.";
		KILL_RAIL2;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{	
		"I prefer to look into the eyes of my prey when I feed.";
		"Let me guess, you thought you had to invite me in.";
		"Direct contact is the surest way to transfer life force.";
		KILL_GAUNTLET0;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		TELEFRAGGED0;
		"Step aside, ",0,". Demigod coming through.";
		"I can be everywhere, and yet be nowhere.";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"I must be quick before the meal grows cold.";
		"You must be dead to be like me, yes, but not like that!";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{	
		"Pffaaaah. You call that life energy? Foulness runs in your veins.";
		"Maybe I will make you my lackey. Maybe not.";
		"Sic Semper Tyrannis!";
		KILL_INSULT15;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{	
		"Do not be ashamed, I have seen empires rise and fall.";
		"It will comfort you to know that I have not had this much fun in ages.";
		"I can see that you fight for what you believe in.";
		PRAISE4;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{	
		"**Sniff** ... **sniff**. Did you soil yourself, ", 1, "?";
		"Mother admonished me not to play with my food, but it's so much fun.";
		"Patriotrism is indeed the last resort of a scoundrel.";
		KILL_INSULT19;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{	
		"Eternal unlife sucks.";
		"I fought and died to make men free ... many times.";
		"Do you come here often?";
		"To fight and die for what ~one believes, that is a worthy life.";
		"Give me Liberty or give me... oh, I think ", 0, " looks tasty.";
		"General Washington gave me this scarf.";
		"You remind me of that Benedict Arnold character, ", 0, ".";
		"If need be, I can survive on the life energy of the occasional ", animal, ".";
		"Why yes, Ben Franklin did program me.";
		"Xaero reminds me of the Vampire who made me this way.";
		MISC13;
		one_liners;
		// 0 = name of randomly chosen player
		// 1 = bot name
		// 2 = opponent in first place
	} //end type
} //end chat patriot


