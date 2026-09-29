//===========================================================================
//
// Name:			Wrack
// Function:		chat lines for Wrack
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Writer:			RA SAlvatore, the Seven Swords, Paul Jaquays
// Last update:		May 22, 2000
// Tab Size:		3 (real tabs)
//===========================================================================

chat "wrack"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		"How much blood must I bathe in before I find peace?";
		"This is ", 4,"? They've really cleaned this place up!";
		"Aren't there supposed to be 'slipgates' here?";
		HELLO;
		HELLO11;
		HELLO10;
		// 1 = random opponent
		// 4 = Level's title
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Come with me, ", 1, ". Find peace.";
		"Gonna find me some fiends to frag.";
		GOODBYE1;
		GOODBYE5;
		// 1 = random opponent
		// 4 = Level's title
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Deja vu all over again ... aargh!";
		"I'm having flashbacks.";
		"And the sky was full of rockets.";
		"Wish I had some real opponents here.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Why is everyone stopping? I'm still here ... I think...yeah, I am!";
		2," is bucking for sergeant.";
		3," should've stayed back in the mess hall.";
		"Over already? I remember these things lasting for hours.";
		// 1 = random opponent
		// 2 = opponent in first place
		// 3 = opponent in last place
		// 4 = Level's title
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"They ~don't train 'em like the used to.";
		"",4," always was sweet to me.";
		"~Don't take it personally,", 1, ", I was doing this when you were in diapers.";
		// 1 = random opponent
		// 3 = opponent in last place
		// 4 = Level's title
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"Yeah. Technology over tactics. Big guns over good moves.";
		"You proud, ", 2, "? You shouldn't be proud! That was lame!";
		"They've changed this place. I remember ", 4, " before they messed it up.";
		"I ~don't care what it freakin' says. ^1I DON'T LOSE.";
		// 1 = random opponent
		// 2 = opponent in first place
		// 4 = Level's title
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"~Don't they teach etiquette in your clan, ", fighter, ".";
		"I really hope that ", 1, " just misfired.";
		"I remember when ", fighter, "s like you had better manners.";
		// 1 = weapon used by shooter
		// 0 = shooter
	} //end type

	type "damaged_nokill" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Can't jump right anymore, can't run fast. When I was younger ...";
		"You're making it harder for me to go through airports... the bells, the bells!";
		"~Pinged another ~one off the plate in my head. What're you shooting... magnets?";
		DEATH_INSULT2;
		// 1 = weapon used by shooter
		// 0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"When I started fighting, you were in diapers, ", 0, ", and now, you need 'em again.";
		"Stop screaming, ", 0, "! I can't hear the other voices in my head!";
		"Wait, you're not 'Old Yeller!'";
		TAUNT1;
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Leave me alone, ", 0, "! There's already too many people in here!";
		"Stinkin' slipgates.";
		"Nothing but static at this end.";
		DEATH_TELEFRAGGED3;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Cats can do it, so can I!";
		"Up, up, and away!  I'm a bird!  I'm a plane!";
		"Used to fall a lot farther than that with no pain.";
		"Let's see you do better, ", 0, ".";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"How did Cthon manage this stuff?";
		"Hey, ", 0, ". Got any marshmallows?";
		"Battle suits are for wimps. Give me an Enviro Suit any day.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"KP again ... too much creamed corn ... way too much creamed corn.";
		"Doesn't smell like liniment.";
		"This stuff is primordial ooze.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"I can't hear! Why can't I hear?";
		"We had gills when I was a boy!";
		"Hey, ", 0, ", throw me a lifesaver. And I ~don't mean candy!";
		"Course I can swim.  Just didn't feel like it then.";
		"This doesn't look like the well of wishes...";
		DEATH_DROWN0;
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Too complicated. Used to be simple. Mano ~y mano.";
		"Not the first time I've done this ...won't be the last.";
		DEATH_SUICIDE1;
	 	// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Charlie got tougher in the jungle.";
		"You just try that again when I'm drunk.";
		// 1 = weapon used by enemy
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Sissy gun! Sissy gun! We wouldn't use crap like that in the old days!";
		DEATH_RAIL1;
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"Oh look, a viking burial! Hey wait, it's mine!";
		"Whose bright idea was this weapon?";
		"No-skill llama!";
		DEATH_BFG0;
		"What're the fireworks for, ", 0, "?  Ow...never mind.";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"~Four years ago, I could've killed you with a projectile-shaped piece of earwax.";
		"Yeah, in the old days, I had to walk a mile through ~six feet of snow to frag someone.";
		DEATH_INSULT1;
		"They make things too easy now. Anybody can fight here.";
		"You're a long way from the elite, ", fighter, ".";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"And the old stallion is driven from the herd...";
		D_PRAISE3;
		"You fight like that for 30 years ... you just might be me someday.";
		"Nice. You trained with the 'old ones,' didn't you, ", 0, "?";
		"That you, G. Narley?";
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"What happened, ", fighter, "? Your backpack explode?";
		"Never had to put up with that kind of crap that I can remember.";
		"Either quit eatin' beans, ", 0, ", or find yourself some other opponents.";
		"I'm gonna pretend I didn't see you do that, ", 0, ".";
		"You work for Shub, right, ", 0, "?";
		"You're not licensed for that weapon, are you ", 0, ".";
	    DEATH_KAMIKAZE0;
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"Yeah, there'll be guys complaining about this one til the end of time.";
		"Heh. Guess I'm not ready to retire just yet.";
		"**SIGH**  Whatever happened to weapons you had to aim?";
		"No whining! There is no whining in the arena!";
	    KILL_KAMIKAZE0;
		KILL_KAMIKAZE1;
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Woohoo! That was too easy. They're taking all the fun out of it.";
		KILL_RAIL1;
		"Choo choo! Train's coming through!";
		"I'd rather have a nail gun.";
		"I'd rather have a 'Thunderbolt.'";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"In the old days, we cooked hot dogs like this."
		"Gimme back my axe! I hate this thing.";
		"Way too easy.";
		"No challenge here.";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"That's like putting an 8-track in a cassette deck!";
		"I'm a slip-gating fool! Wahoooo!";
		TELEFRAGGED5;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when an opponent kills self
	{
		"Wait minute ... what's the point of this again?";
		"Go as negative as you want, ", 0, ".";
		"If you can't handle the heat, ", 0, ", stay outta the lava.";
		"Lamebrain, ", fighter, ".";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"I've had better fights trying to put my pants on in the middle of the night.";
		"I hate fraggin' them before they're old enough to drive.";
		"You old enough to be here, ", 0, "? Can I see some ID.";
		"Come back when you've improved enough to be competition, ", 0, ".";
		KILL_INSULT22;
		KILL_INSULT29;
		"Age and treachery always overcome youth and skill. You bet!";		
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"You done good, ",0,". Too bad you won't get to polish your technique.";
		"You remind me of me in the old days ... course, I got better.";
		"You've been watching ol' ~Wrack's moves, right, ", 0, "?";
		PRAISE2;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"Wipe your nose, ", fighter, ". Your whiney sniffling's giving you away.";
		"You want a little cheese with that whine, ", 0, "?";
		"And I thought I didn't know where I was. You lost there, ", 0, "?";
		TAUNT3;
		// 0 = name of randomly chosen player
		// 1 = name of last victim
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"I got a cramp. Hey, ", 1, ", rub my leg.";
		MISC14;
		MISC4;
		"Improve or die, ", fighter, "s. Improve or die.";
		"I want to feel some more of your pain, ", 1, ".";
		"And the sky was full of planes ...";
		"Hey, ", 0, "! I need a ", 5, ". Think I'll take yours.";
		// 0 = name of randomly chosen player
		// 1 = namne of last victim
	} //end type
} //end chat

