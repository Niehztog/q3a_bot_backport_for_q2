//===========================================================================
//
// Name:			Keel
// Function:		chat lines for Keel
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			The Seven Swords & R. A. Salvatore
// Editor:			Paul Jaquays
// Last update:		Oct. 5, 1999
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats
chat "keel"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"
	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{	
		"Where am I? This doesn't look like Subic 3.";
		"You don't seriously think you can beat me.";
		"Time to show you a new level of pain.";
		"I trained in a place just like ", 4, ".";
		"Ahhh, my old nemesis, ", 1, ". We meet again.";
		HELLO5;
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{	
		"I do not terminate civilians.";
		"Incoming transmission. Return to HQ.";
		"Do not expect me to go easy on you when I return.";
		GOODBYE5;
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{	
		"No Commonwealth colors? That makes ", 0, " the enemy.";
		"Obviously this mission calls for a professional.";
		LEVEL_START1;
		// 0 = bot name
		// 1 = random player
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{	
		"These cybronic enhancements are slowing me down.";
		"Next time, ", 2, ", I'll break your neck with my own hands.";
		LEVEL_END2;
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{	
		"Give up, ", 3, ". I was designed for combat.";
		"I hate dealing with amateurs.";
		"Mission objective accomplished. Awaiting further instructions.";
		LEVEL_END_VICTORY1;
		// 0 = bot name
		// 2 = opponent in first place
		// 3 = opponent in last place
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{	
		"I think I'm ready for the scrap heap.";
		"Somebody get me to a body shop. Too many dents this time.";
		"They should have let me die a ~hero on Subic 3.";
		"I'm ready for that oil change now.";
		LEVEL_END_LOSE0;
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{	
		"Okay, ~hero. From now on it's no holds barred.";
		"Okay, ", fighter, ". From now on it's no holds barred.";
		"To fight and die without honor is the saddest death of all.";
		"I ain't letting up on you 'til you squish, ", 0, ".";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{	
		"You little puke, you're gonna need a bigger gun.";
		"Hey ... they told me this armor was bulletproof.";
		"That which don't kill me ... ";
		"Nice shot, ", 0, ". Now watch a professional in action.";
		"~Ho ~ho ~ho, ", 0, ". You're just making the final payback sweeter.";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{	
		"Hurts, ~don't it?";
		"Obviously my human feelings are spoiling my aim.";
		"I'm going to swab the floor with your bloody carcass, ", 0, ".";
		"Frag is short for fragment ... see the little ", 0, " fragment on the deck?";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"That's murder on the cybronics.";
		"Yow! That some kind of EMP you hit me with, ", 0, "?";
		DEATH_TELEFRAGGED1;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{	
		"That's what they forgot to build in ... booster rockets.";
		"Could've made it if I landed on the cyborg part. Figures.";
		"All this armor ... and I go and land on my head.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{	
		"System meltdown imminent ... I wonder if the part that eats will survive.";
		"At last I am free of this lie.";
		"Someone oughtta bottle this hot sauce.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{	
		"That don't look like no hot wax dip.";
		DEATH_SLIME0;
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{	
		"Great, all this technology and they forget the waterproofing.";
		"Break out the rust remover. I'm gonna need it.";
		DEATH_DROWN1;
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{	
		"Aarrrgh. Friggin' computer glitches.";
		"They must have programmed me in DOS.";
		"Triple redundancy in everything except common sense.";
		"Stink!! My onboard computer just lagged me out.";
		DEATH_SUICIDE1;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{	
		"You shorted out my systems, ", 0, "!";
		"That was ~one heck of dent you left, ", 0, "!";
		"Terminal tickling ... what a way to go.";
		DEATH_GAUNTLET1;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{	
		"Come out, ", 0, ", and fight like Special Forces.";
		"Coward! Face me man to... ummm... man.";
		"At least you ain't a camping BFG scumbag.";
		DEATH_RAIL1;
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{	
		"That will do a number on your ego.";
		"'Scuse me while I go raid a used car lot for replacement parts.";
		DEATH_BFG1;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{	
		"Your mother, ", 0, ". She as ugly as you?";
		"Laugh while you can, gerbil-brain. I'll be back.";
		"I see that mail-order degree you earned is finally paying off, ", 0, ".";
		DEATH_INSULT2;
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{	
		"You are a testament. Courage and heart can overcome the machine.";
		"A toast to ", 0, ".  Well done.";
		D_PRAISE4;
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Man! Just my luck to be a ground zero at a gas-passin' contest.";
		"That fried me right down to the circuit boards.";
		"Be the BFG Danny!";
		"This Tin Man is gonna call an airstrike in on your sorry butt, ", 0, ".";
		"~Ain't they outlawed them things yet?";
		"I am truly humbled to be in the presence of such skill ... ~loser.";
		"You gotta lay off the beans ", fighter, ".";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with rail gun
	{
		"Special delivery for, ", 0, ". You gonna sign for it?";
		"Anybody else hungry?";
		"Yeah, I forgot to put that Death Wish thingie on my resume.";
		"Can you say 'Nuclear Devastation?' I knew that you could.";
		"That's the cabbage and beans talking.";
	"I thought those servos was overheatin'";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{	
		"I could do that at 1,000 meters in a high wind.";
		KILL_RAIL0;
		KILL_RAIL2;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{	
		"I just wanted to look you in the eyes, ", 0, ", while you die.";
		KILL_GAUNTLET2;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefrags someone
	{	
		0, "! Your rent is overdue.";
		"Candygram!!";
		TELEFRAGGED0;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{	
		"Feh. You ain't worth putting back together.";
		"They could rebuild you, ", 0, ", but they all outta 'stupid' parts.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{	
		"I see no honor in this.";
		"Remember, ", fighter, ", practice makes perfect.";
		KILL_INSULT3;
		KILL_INSULT13;
		KILL_INSULT39;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{	
		"For a moment ", fighter, ", I thought you might actually win ... ^1 NOT.";
		"I think I'll donate your body to science, ", 0, ".";
		PRAISE2;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{	
		"Come on, Alice! fight!";
		"Do you smell something? ", 0, ", maybe? Peeee-YEW!";
		"This tin man's got his heart. But I think that ", 0, " the scarecrow is missing some brains.";
		"I know cowards when I can't see 'em.";
		MISC15;
		TAUNT4;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"Anybody got an oil can?";
		"Sometimes, I imagine I got ", liquid, " runnin' in my hydraulics.";
		"I really could go for an order of Buffalo Wings.";
		"I really could go for an order of ", food, ".";
		MISC6;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end keel chat


