//===========================================================================
//
// Name:			Bitterman_c.c
// Function:		chat lines for Bitterman
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			Seven Swords & R.A. Salvatore
// Editor:			Paul Jaquays
// Last update:		09-29-99
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats
chat "bitterman"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		HELLO1;
		HELLO6;
		"Hang the gore, ", 1, ", let's get it on!";
		4," again! I guess you campers are happy!";
		// 4 = Level's title
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"My work here is done, Adios!";
		"All y'all take care and watch your ~six.";
		"I just can't stand being near ", 1, ". I'm outta here.";
		"I'm fixin' to leave.";
		GOODBYE;
		GOODBYE4;
		// 0 = bot name
		// 1 = random opponent
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Crank up the tunes, this ol' boy's back in Dodge.";
		"Well I think its time to dance!";
		"Y'all should know that ", 4, ", is my favorite killing grounds.";
		LEVEL_START2;
		// 0 = bot name
		// 4 = Level's title
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{	
		"~Don't you be thinkin' you good ol' boys got me runnin'.";
		"Dang! Whumped by a carpet-baggin' Yankee ", fighter, ".";
		"The South will rise again!";
		"Hey there, ", 3, ". Wear all that blood like a badge of honor.";
		// 0 = bot name
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Wahoo! The music roars and I bow to the crowd.";
		"Looks like this bronco is done kickin!";
		LEVEL_END_VICTORY2;
		LEVEL_END_VICTORY3;
		"Shoot, I ~don't even need my dog to flush you city folk.";
		3, ", thanks again. Y'all made some really great targets.";
		// 0 = bot name
		// 3 = opponent in last place
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"~Don't push me, ", fighter, ". I've been to hell and back, and back I'll be!";
		"I've seen tougher, killed 'em too.";
		LEVEL_END_LOSE2;
		LEVEL_END_LOSE1;
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"We have names for y'all's type, sissy!";
		0, ", I can see your yella ~stripe a mile away!";
		"Dang! Ain't no ~one got any manners in this universe?";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Is that my blood I taste? ", 0, " may just survive after all!";	
		"If y'all fire enough rounds you're bound to hit something, I guess.";
		0," thinks a little poke like that is gonna bring me down.";
		"A ", 1, "? Who exactly are you trying to hit with that, ", 0, "?";
		//0 = shooter
		// 1 = weapon used by shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Just my way of sayin' 'howdy', Amigo.";
		"~Don't y'all be hiding, ", 0, ". I got your blood trail now.";
		"Looks like ", 0, " has sprung a few leaks.";
		TAUNT;
		PRAISE2;
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Guess ya'll'l be seeing my pieces in all the familiar places.";
		DEATH_TELEFRAGGED2;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"I really hate that first rush of air on my brain.";
		"I feel like an overloaded skeeter.";
		"Now I know what bird doo feels like.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{	
		"Looks like a BAR-B-Q and I'm the only ~one invited.";
		"This is like an Alabama August!";
		DEATH_LAVA0;
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"Dang, killed by that sissy hair goop.";
		//"Let's grease y'all with this, ", 0, ", and see who catches you the fastest.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Guess they'll tell Ma I was buried at sea.";
		"You think a little water will slow me down.";
		DEATH_DROWN1;
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"All I said was 'Hey, lookit what I can do!'";
		"Ironic ain't it! I won't give anyone else that pleasure.";
		DEATH_SUICIDE3;
		DEATH_SUICIDE4;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"What was that? Did a fly come land on me?";
		DEATH_GAUNTLET0;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Could you use anything smaller? I got enough scars as it is.";
		DEATH_RAIL1;
		"Skeeters hit harder than that, ", 0,"!";
		// 0 = enemy name
		// 1 = weapon used by shooter
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"Yeah sure, any ", fighter, "'s a ~hero with a gun like that.";
		DEATH_BFG1;
		// 0 = enemy name
	} //end type

	
	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"~Y'all need to deal with that anger problem, ", 0, ".";
		"Just my luck, I show up in time for the fireworks.";
		"If ~y'all want me to quit talking, just say so.";
		"~Y'all gonna do that here, then ~y'all gonna clean up the mess.";
		"I ain't impressed.";
		"Y'all got yourself a special talent for blowin' up like that, ", 0, ".";
		DEATH_KAMIKAZE1;
		// 0 = enemy name
	} //end type 

	type "death_insult" //insult initiated when the bot died
	{	
		"How about that. ", 0, " CAN pull a trigger!";
		"Dang it! Why do I have to be here when ", 0, " gets a lucky shot off!";
		DEATH_INSULT2;
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Good shootin' ", fighter, ". A good eye means a long life.";
		"I tip my hat to y'all, ", 0, ", It's been a pleasure.";
		D_PRAISE1;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Whoaaaa. I can see light on the other side of that hole, ", 0, "!";
		"Perforated!";
		KILL_RAIL0;
		// 0 = enemy name
		// 1 = weapon used by shooter
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"I have always been good with my hands.";
		KILL_GAUNTLET1;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Pull yourself together, ", fighter, ".";
		TELEFRAGGED2;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"Y'all'd do anything to collect insurance, wouldn't you?";
		"Y'all think I'm gonna hold my shot now, ~don't you?";
		// 0 = enemy name
	} //end type

	type "kill_kamikaze" //insult initiated when bot kills with the kmikaze
	{
		"All ~y'all understand that I had to do that, right?";
		"Now that's some firecracker! Wahoooo!";
		"Ooops. Sorry. Wrong button.";
		"Bet that blew all ~y'all to ", place, ", didn't it?";
		"Sorry 'bout the mess.";
		"~Y'all come back now, hear?";		
		"After that, I need me a cold ", liquid, ". Anyone want to join me?";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Hey, ", 0, "! A sleeping cow makes a tougher target than you!";
 		"I think ", 0, " enjoys standing in front of my gun.";
		"Are you a masochist, man?  RUN!";
		KILL_INSULT3;
		KILL_INSULT6;
		// 0 = enemy name
	} //end type


	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Sometimes it hurts to kill the good ones.";
		"Congrats, amigo! You died well.";
		"You earned your tombstone, ", fighter, ".";

		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		0, "'s guts look mighty pretty all over that wall.";
		"I'm gonna give y'all a new purpose in life ... dying gloriously.";
		"I'm determined to make something of all y'all ... fertilizer.";
		"Feel the pain. Feel the pain.";
		MISC10;
		MISC12;
		"Every time I get in ", 4, " , I just wanna break my ", 5, " over ", 0, "'s head.";
		// 0 = name of randomly chosen player
		// 1 = bot name
		// 4 = Level's Title
		// 5 = random weapon from weapons list
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"Y'all ain't from around these parts are ya, ", 0, "?";
		"Y'know, I'm craving some ", food, " and a cold ", liquid, " 'bout right now.";
		"Hey ", 1, ", you got family in ", place, "? You remind me of someone I know.";
		"Hey ", 0, ", it's good to see some new faces in this hole.";
		1, ", will I have to show you how to use that ", 5, " again?";
		MISC2;
		one_liners;
		// 0 = name of randomly chosen player
		// 1 = bot name
		// 5 = random weapon from weapon list
	} //end type
} //end bitterman chat


