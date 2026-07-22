//===========================================================================
//
// Name:			fritzkrieg_c.c
// Function:		chat lines for Fritzkrieg
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			Paul Jaquays
// Editor:			Paul Jaquays
// Last update:		April 5, 2000
// Tab Size:		3 (real tabs)
//===========================================================================

chat "fritzkrieg"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Ohhhhhh yeah. It's play-time for Fritzie!";
		"Nice place ya got here. Mind if I make myself more comfortable?";
		"Hey ", 0, ", you gonna let me win again? Just like last time?";
		"Heh. ", 4, " again? I had me some fine victories here.";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Ow. Paper cut! Gotta see the doc before it gets infected.";
		"Hey! I'm checking out! Come 'n get yer free frag before I leave.";
		"Gotta go. A pity that I'm wasting all this good ammo.";
		
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Well lookie here! It's the infamous ", 0, ". Time to ph34r those l33t 5k1ll5 again, eh?";
		"I am feeling a serious case of BAD coming on.";
		"We come about full circle again.";
		"All variables are equal once again. Victory favors Pi.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"'Clowns to the left of me, jokers to the right ...'";
		2, ", did you cash in all them '1 free frag' coupons you been collectin'?";
		2, ", next time, I'm gonna do somethin' real BAD to ya.";
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Who da Man?!";
		"Fritzkrieg rules ", 4, ".";
		"Bow down to the King, baby! Oh yeah!";
		"I rock!";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		2, " next time, I'm gonna do somethin' real bad to ya.";
		"This sucks.";
		"Losing sucks big time.";
		"I hate losing to a lousy camper.";
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"That ought to teach me to keep my yap shut.";
		"Cheap shot.";
		"~You'd shoot yer momma in the back too, wouldn't ya ", 0, ".";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Oooooo. That's a big 'NO-pointer' for you, ", 0, ".";
		"I've had worse paper cuts, loser.";
		"~You better have another one of those, ", 0, "'cause you're gonna need it.";
		"I ~ain't got any vital organs you can take out in one shot, ", 0, ".";
		"Wimp.";
		"Wuss. Gotta shoot straighter than that.";
		// 0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Fraggitt! I could have sworn you were keeping your brain down there, ", 0, ".";
		"I'm such a tease.";
		"Fraggitt! I rolled a spare.";
		"Blast. No vitals organs there.";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"That just cracks me up.";
		"You get a gold star for that telefrag, ", 0, ".";
		"Wanna try for two outta three there, ", 0, "?";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Just scrape me off the walls and send me home.";
		"Splat! Did I get any on ya?";
		"Awww. Fritzkrieg goes BOOM!";
		"Air Raid!";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Smells just like my momma's cookin'.";
		"When you're hot, you're hot ..."
		"Toss some long pig on the barbie, mate.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"Someone get the janitor in here, ", 0, "'s upchucked again.";
		"Medic!";
		"Too much chlorine in the pool!";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Guess I gotta give back that swimming merit badge.";
		"Pool smells like somebody died in it.";
		"Anyone for some skinny-dipping?";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{	
		"Ooops.";
		"Gotta practice that move on someone else next time.";
		"OK. I ~ain't giving out any more free lessons."
		"Y'know. I think I could'a handled that better.";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"~You got a mean right hook there, ", 0, ".";
		"~You gonna pay for that one, ", 0, ". Big Time.";
		"Fraggitt! Not again.";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Fraggitt!.";
		"~You can just zoom that rail gun right up yer ...";
		"Laugh while you can sniper-wuss.";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"I got you runnin' scared now, ", 0, ".";
		"Hey! Gimme that gun.";
		"Skillz check. Oops you failed again, ", 0, ".";
		"BFG-Wimp!";
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot died by a kamikazee blast
	{
		"Kamikaze wuss.";
		"Yup. That sure smelled like ", 0, "'s 'Divine Wind.'";
		"No skill there, ", fighter, ".";
		"Somebody call the bomb squad on this ", fighter, ".";
		"My point egg-zackly. No skillz.";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"I'm ~gonna do sumthin' to ~ya, ", 0, ". ~Sumthin' ~bad.";
		"Wuss!";
		"Heh. Couldn't find the kamikaze, huh?";
		"Y'know, ", 0, ". I'm startin' not to like you.";
		"Yup. It's always easier to shoot a guy in the back.";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"I'm s'posed to say sumpthin' nice about ya, ", 0, ". Man this is hard.";
		"Look, ", 0, "! ~Is that your boss behind you?";
		"Some day, I'll look back at this and laugh, right?";
		"Nice shell casings there, ", fighter, ".";
		// 0 = enemy name
	} //end type


	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Heh. Auto-aiming at its finest.";
		"Isn't there some rail arena match you could be playing somewhere else?";
		"I hate stinkin', snipin', camper wusses!";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"You need me to pound any more sense into to your pointy little head, ", 0, "?";
		"That was TOO easy.";
		"Oh yeah. Let's do that ol' Humiliation Dance' again, ", 0, "!";
		"BIFF!! WHAM!! POW! I love it!";
		KILL_GAUNTLET0;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Gotta admire my timing there.";
		"Face it, ", 0, ". My particles got mo' mojo than your particles.";
		"Candygram for ", 0, ". What did I miss that ", fighter, " again?";
		"**<Burp>**";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when another player kills self
	{
		"Stop doin' that man! I got money on the point spread.";
		"Fraggit! I'm late again. You started the party without me.";
		"Loozer!";
		"Oooooo. I'm just overwhelmed by yer elite haxor skillz";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Wuss!";
		"I don't think you're cut out for this kinda fun, ", 0, ".";
		"You takin' 'Loser Lessons' again, ", 0, "?";
		"Oh yeah! It's Rabid Bunny Fever Time!";
		KILL_INSULT2;
		"Never Surrender! Never Give Up! Never point that gun at me again!";
	
		// 0 = enemy name
	} //end type

	type "kill_kamikaze" //insult initiated when the bot killed another player with kamikaze
	{
		"That feels so much better!";
		"Did the earth move for you too, ", 0, ", baby?";
		"Man, I gotta cut the ", food, " outta my diet. I can't handle the gas.";
		"Lookit me ... I'm a one-man Bee-eF-Gee!";
		"I told you not to make me mad, ", 0, ".";
		"Was that me?";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Nice splatter there, ", 0, ". Good color choice.";
		"Join me, ", 0, " ... and we shall rule the Empire together ... NOT!";
		"... all this and I get your gun too!";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		male_one_liners;
		"~Don't come griping to me. I ~ain't yer ", random_counselor, ".";
		"Hey ", 0, ". Maybe if you weren't so ", adj, " and ", different, ", you'd be more of a challenge.";
		"Hey ", 0, "! You talk funny. You from ", place, " or sumthin'?";
		ramble;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"Who's yer daddy?";
		"I gots ~yer stress relief right here, ", friendname, ".";
		"Hoo-eeee. This is more fun than stickin' a ", item, " down my shorts.";
		one_liners;
		"*Sniff* You doin' ", substance, " again, ", 0, "? I told you to stay off the hard stuff.";
		"Whattaya think? Should I change this out for a ", color, " trenchcoat?";
		"Hey ", 0, ". I heard that ", namez, " was lookin' for ya.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end fritzkrieg chat

