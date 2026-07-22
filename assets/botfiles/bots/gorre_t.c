//===========================================================================
//
// Name:			Gorre
// Function:		chat lines for Gorre
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			The Seven Swords & R. A. Salvatore
// Editor:			Paul Jaquays
// Last update:		April 9, 2000
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats
chat "gorre"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		4, "? Super! Gorre owns this place!";
		"Oh looky, Gorre's here. You're all toast.";
		"I am Gorre. I will frag you all!";
		HELLO6;
		HELLO9;
		// 0 = bot name
		// 4 = Level's title
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"I'm too good to waste my time with you ", 1, ".";
		4," makes for a level playing field ... but only for me.";
		GOODBYE2;
		// 0 = bot name
		// 4 = Level's title
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Fraggie went a courtin', he did ride, uh-huh.";
		"Keep on bowing to me, ", fighter, " ... I'm aiming for your bald spot.";
		LEVEL_START0;
		LEVEL_START3;
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Awwww. I was having too much fun chopping you guys up.";
		"Hey, ", 2, ", you got in my way. You're gonna be lunchmeat.";
		"Okay, ", 3,", why ~don't you just let ", 2, " kill you again? You guys working together?";
		LEVEL_END2;
		// 0 = bot name
		// 2 = opponent in first place
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Took me longer to learn how to tie my shoes than how to frag you pus balls.";
		"Call housekeeping. Stains everywhere.";
		"~Don't cry, ", 3, ". Keep trying. I like stomping you.";
		LEVEL_END_VICTORY1;
		LEVEL_END_VICTORY4;
		// 0 = bot name
		// 3 = opponent in last place
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"How can I help but get cocky?";
		"Got a hangnail on my trigger finger ... yeah, that's it.";
		"Okay, so it didn't work with my eyes closed. They're open now.";
		"Hey, ", 2, "'s using my tactics! He's gotta be!";
		LEVEL_END_LOSE1;
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Oh yeah, ", 0, ". You just did nothing but click me into evisceration mode.";
		"I'm gonna bite you on the toes and chew my way up to your heart.";
		"There is no hole deep enough for you to hide in.";
		// 0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"You had your chance, ", 0, ". You've lost and just don't know it yet.";
		"You've marred perfection, ", 0, ". You've made perfection mad. This is gonna hurt.";
		"You can't take down this body with that little gun, pus ball.";
		"Was I supposed to yell ouch? Ain't gonna happen, ", fighter, ".";
		"~Don't you go blaming your lack of skill on cheap guns and poor ammo.";
		"I'm immortal. Hitting me won't kill me.";
		TAUNT3;
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Cheap guns and bargain basement ammo loads are throwing me off my game.";
		"Off the wall, around the corner, nothing but net.";
		"When they're shaking this hard, you gotta shoot center mass.";
		"Clone ", 0, " a ~hundred times and you got a carnival duck shoot.";
		TAUNT6;
		// 0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Too much power in ~one place! 99% of it was mine, but ...";
		DEATH_TELEFRAGGED3;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		0, ", put your squishy little head under this ...";
		"Didn't think I could jump that far, now did ya?";
		DEATH_FALLING;
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Just a shortcut, ", fighter, ". I'll be rising right behind you!";
		"Where's the 'Under Construction' sign? Who built this freakin' place?";
		DEATH_LAVA1;
		// 0 = enemy name;
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		botnames, " put that here! I know it! They're all afraid I'll climb the ranks.";
		"Gurp!!";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"I got time to do a little fishing and still frag you, ", 0, ".";
		"No way I'm going out like this! No bl.bl.bl.way ... glub.";
		"Must...FEEESH!";
		DEATH_DROWN0;
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Unfriggin'-believable.";
		"Ain't what it seems... you bored me to death.";
		"You see how it's done, ", 0, "? Okay, now you do it.";
		"~Don't laugh, ", 0, ", you're next!";
		DEATH_SUICIDE4;
		DEATH_SUICIDE1;
		// 0 = enemy name
	} //end type


	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"I thought you said we were gonna arm wrestle. Cheatin' pus ball!";
		"You'll never wash that hand again, will you, ", 0, "?";
		DEATH_GAUNTLET2;
		DEATH_GAUNTLET0;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"I'm a force of nature ... and nature always wins, ", fighter, ".";
		"Hide and shoot. Figures for a ", fighter, " like you, ",0, ".";
		"Afraid to face me square, eh, ", 0, "? You're smarter than you look.";
		DEATH_RAIL1;
// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"My DNA will be feeling that ~one for several generations";
		"Had to call in the big guns for me, didn't you, ", fighter, "?";
		DEATH_BFG1;
		DEATH_BFG2;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"I knew you were a pus ball, ", 0, ", so I let up on you.";
		"I've never lost when I was trying to win.";
		"My shot must have bounced off your head, ",0, " and fragged me.";
		DEATH_INSULT2;
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"I didn't like where they put my head anyways.";
		"Brag about this to just ~one guy and I'll use your tongue to clean your ears.";
		"If I didn't let 'em win once in a while, they'd lock me out.";
		D_PRAISE4;
		D_PRAISE5;
		// 0 = enemy name
	} //end type

		type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Big deal. I once wiped out fourty ", fighter, "s in a single blast with it.";
		"^1**NUKED**";
		"I like you, ", 0, ". Nobody else masters the cheap shot like you do.";
		"You almost missed with that.";
		"Bah. I should have expected a trick like that from you, ", 0, ".";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with rail gun
	{
		"I OWN you all!";
		"Bow before my godlike power!";
		"I RAWK BABY!!";
		"You can keep the change!";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"~Don't whine about things you can't control, ", 0, ".";
		"I didn't even shoot that... I threw it.";
		KILL_RAIL1;
		KILL_RAIL2;
		KILL_INSULT28;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Hey, ", 0, ". When will you learn to just kneel before me?";
		"It's like mole wacking at the carnival!";
		"Heh. And I consider stealth the least of my talents.";
		"Slap bag, zipperface.";
		KILL_GAUNTLET0;
		KILL_INSULT25;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Behold the power of a sneeze.";
		"Pop!";
		"When I stomp, I ^1STOMP!";
		KILL_INSULT29;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"Facing me, your actions are understandable.";
		"I'm gonna paint a big 'L' on your toe tag, LOSER.";
	
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Here lies, ", 0, ", the scab. Fought a god and bought a slab.";
		"Here lies, ", 0, ", the whining puke. He'd have had more chance if he swallowed a nuke.";
		"You never had a chance. Do you know how good I am?";
		KILL_INSULT5;
		KILL_INSULT21;
		KILL_INSULT23;
		KILL_INSULT8;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"No shame in this ,", 0, " ... you were killed by the best.";
		"~Three more seconds, ", 0, ", and you'd have set a new survival record against me.";
		"If they cloned you ~ten times and you fought me together, you might have a chance, ", 0, ".";
		PRAISE4;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"Get over here, ", 0, ". I want to press your face in dough and make monkey cookies.";
		"I could kill you with a look, ", 0, ", but then I'd have to look at you.";
		"Bring me a bag of ", substance, ", and I might let you live next time, ", 1, ".";
		"Deity in the house!";
		TAUNT6;
		TAUNT7;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{	
		"I am the god of frags, and ", 4, " is my temple!";
		"Someone tell me what it's like to get fragged. I just can't imagine it.";
		"~Don't you EVER call me ~Al!";
		"I just use the teddy bear to keep my knees in the proper position at night.";
		"I practice with a mirror. I'm the only ~one good enough for me to learn from.";
		"Mmmmmmm, ", food, ".";
		MISC15;
		MISC2;
		MISC9;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end gorre chat



