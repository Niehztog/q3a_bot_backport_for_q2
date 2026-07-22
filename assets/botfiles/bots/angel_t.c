//===========================================================================
//
// Name:			Angel_c.c
// Function:		chat lines for Angel
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			Seven Swords & R.A. Salvatore
// Editor:			Paul Jaquays
// Last update:		Oct. 10, 1999
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats

chat "angel"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Somebody farted. Got to be ~one of those organic ", fighter, "s.";
		"Is this the best evolution could come up with?";
		"I'm gonna own ", 4, " before we're done.";
		HELLO4;
		HELLO7;
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"You ", fighter, "s wouldn't know real beauty if it bit your sorry butt. I'm leaving.";
		"Yeah, that was fun. Not.";
		"I'm gone.";
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Okay, fleshy slugs. Let's get it on.";
		LEVEL_START0;
		"Wake up, fleshbags. Angel's crashed your little party.";
		"Angel of Mercy? Not in your lifetime, ", fighter, ".";
		"What, you expected cherubs and sugary stuff?";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"I would have won, too, if I had taken nothin' but cheap and easy shots.";		
		"Yeah, well, we're all having fun now.";
		"Stuck in the middle again.";
		"There just ain't no justice.";
		"I thought I was a bit rusty.";
		// 0 = bot name
		// 2 = opponent in first place
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Okay, kiddies. Pick up all your body parts and get out.";
		"Bet you ", fighter, "s didn't expect me to be prom queen.";
		"Hey, ", 3, ". You make an easy target with that big 'L' painted on your forehead.";
		"I told you ", fighter, "s I would own this place.";
		LEVEL_END_VICTORY1;
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"Ooh, well isn't ", 2, " a big ~hero?";
		"Always a bridesmaid, never a bride.";
		"Wait. All of you ", fighter, "s here, and I'm the loser? Who else finds this funny?";
		LEVEL_END_LOSE1;
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"What's the matter, ", 0, "? Your organic brain can't decipher good manners?";
		"Let me finish the stupid sentence, ", 0, ".";
		"Fair is fair. I'd do the same to you, ", fighter, ".";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Ooh, I like the rough stuff.";
		"Oh look, a dweeb with a pee-shooter. And I'm not talking about your gun.";
		PRAISE;
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"You flesh folk leak too much.";
		"Oh wait ... I'm sorry ... did that hurt, ", 0, "?";
		"Guess I'll have to set this gun to SPLATTER.";
		TAUNT_FEM;
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Could I get a magnet here?";
		DEATH_TELEFRAGGED1;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Hey, so what? It's the only rush I'll get in here.";
		"So I'll get a job as an ashtray.";
		"So this is why they call it TERMINAL velocity.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Now I know the melting point of... me. ~Ten degrees less than this.";
		"Now you think I'm hot, right?";
		"Terminal hotfoot!";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"It's like being hugged by a human.";
		"Gaaahhhh! It's not supposed to hurt like this!";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Wait a minute... robots can't drown! Blub, blub, blub.";
		"If the lack of air don't get you, the rust will.";
		"Anyone got some rust remover?";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Aw, the heck with it.";
		"I just wanted to see how much it hurts before I use it on ", 0, ".";
		DEATH_SUICIDE1;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Get your sweaty, fleshy hands off me, ", 0, ".";
		DEATH_GAUNTLET0;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Ever get the feeling you're being watched?";
		"Okay, ", 0, ". You like what you see?";
		DEATH_RAIL1;
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"~Don't stand in a puddle when re-charging. Gotta write that down ...";
		"Hey, ", 0, ", Quit laughing. Half those sparks are coming from me!";
		"Green. I hate green.";
		DEATH_BFG0;
		// 0 = enemy name
	} //end type

	//type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
//	{
//		"What am I here? Guest of honor at the losers' convention?";
//		"So you can light your farts, ", 0, ", big deal.";
//		"Yeah ... that was fun ... NOT!";
//		"Oh yeah, that took REAL skill.";
//		"You ought to take something for that gas problem, ", 0, ".";
//		"Score one for the No-Skill Express.";
//		DEATH_KAMIKAZE1;
		// 0 = enemy name
//	} //end type


	type "death_insult" //insult initiated when the bot died
	{
		"I'll get re-wired, ", 0, ", ... you'll just decompose.";
		"At least I don't stink when I rot.";
		"I'd tell you to kiss my butt, but I'm afraid you'd take me up on it.";
		DEATH_INSULT3;
		DEATH_FEM_INSULT2;
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"It's good that you did this before you got old and weak, ", 0, ".";
		"I'm impressed, ", 0, ". You're the best inferior I've ever met.";
		D_PRAISE2;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"This way I ~don't have to risk getting your guts all over me, ", 0, ".";
		KILL_RAIL1;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		KILL_GAUNTLET2;
		"It'd be your pleasure to wake up next to me, but tragically, you won't be waking up.";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"'Scuse me, ", fighter, ", Angel needs some room to move.";
		TELEFRAGGED1;
		// 0 = enemy name
	} //end type

		//type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
//	{
//		"I can see you weren't expecting that!";
//		"Awwww. Did the big bad boom scare the widdle ", fighter, "s?";
//		"Now that was cooool.";
//		"You're just mad 'cause I got to it first.";
		// 0 = enemy name
//	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Someone call the big orange truck to clean up this road kill.";
		"You're a pus bag, ", 0, ", and you can take that literally.";
		KILL_INSULT4;
		KILL_INSULT0;
		KILL_INSULT5;
		KILL_INSULT12;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"You've got to have some non-organic parts in there somewhere, ", 0, ".";
		"I wanted you to feel like you could do something well, so I killed you again.";
		PRAISE4;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"You cut the cheese, ", 0, "? Or is this your normal perfume?";
		"Hey, ", 0 ,", repeat after me ... I WILL brush my teeth.";
		"All the leaves have shaken off that bush. I see you now.";
		MISC15;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellaneous chats initiated randomly
	{
		"The hot wings made my fingers all sticky.";
		"Robot?!?! I'm a synthetic humanoid, thank you very much.";
		"Hi, my name is Angel, and yes, I am a fragaholic.";
		"I ~don't dance, unless it's on ", 0, "'s head.";
		"Pygmalion was a ~jerk.";
		one_liners;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end angel chat

