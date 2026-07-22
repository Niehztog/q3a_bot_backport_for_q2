//===========================================================================
//
// Name:		Anarki_c.c
// Function:      chat lines for Anarki
// Programmer:	MrElusive (MrElusive@idsoftware.com)
// Last update:	April 30, 2000
// Author:	R.A. Salvatore & The Seven Swords
// Missionpack Update: Paul Jaquays
// Editor:		Paul Jaquays
// Tab Size:		3 (real tabs)
//===========================================================================


chat "anarki"
{
	#include "teamplay.h"

	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		"I feel the need for speed. Road trip on the cyber-highway!";
 		"Hehehe, I am the master of your cyber-space nightmares.";
		"Jacked in, powered up, and ready to scatter.";
		"Oh ~d00d! ", 4, " is like my totally favorite place to hang.";
		"Whoaaaa, ", 1, "! You are like, totally ancient.";
		// 0 = bot name
		//1 = random player
		//4 = level's title
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"'O-H' time, ~d00d. Outta here!";
		"Cya, ", 1, ". I'm looking for some real talent.";
		"I ride in kewler circles, lamer. CU L8r.";
		4, " rocks, but I gotta buzz outa here.";
		"Hangin' with railbait like you is gonna lower my rep.";
		// 0 = bot name
		//4 = level's title
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Hey, ", 1, "! Old, slow and ugly is no way to go through life, ~d00d.";
		"Woohoo! Can you feel that rush?";
		"Welcome to life on the edge!";
		"Anarki has entered the tube!";
		"Whoa! Headrush. I haven't even had breakfast yet.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Reboot! Reboot!";
		"Totally bummed, ~d00d. The fun's over.";
		"Charging the board for round ~two.";
		"I need a better ping.";
		"Anyone up for a ", food, " break?";
		"This is totally bogus. You were totally bogarting all the weapons, ", 2, "!";
		"Next round, I'm using my board.";
		// 0 = bot name
		// 1 = random opponent
		// 2 = opponent in first place
		// 3 = opponent in last place
		// 4 = level's title
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Whoa! I just completely messed you up, ", 1, ".";
		"Boring tour, ", fighter, " ~d00dZ. Like, how do I get into the main room?";
		"Anarki: 1, Llamas: 0.";
		"~Major bummer for you, ", 3, " dude.";
		"You lamers running 96 baud, or what?";
		"I'm the undertaker, cuz you were dead before we began.";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"Bad connection ... gotta be the connection.";
		"Static on the line. Yeah, it was static.";
		"System overload, ~d00d. I'm multitasking way too much here.";
		"Not possible! Check your data tables!";
		"How did I lose to a llama like you, ", 2, "?";
		"Fragged by vidiots.";
		"Fragged by ", fighter, "s!";
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"~D00d! like, you're taking this age-and-treachery crap too far, ", 0, "!";
		"New board, ~d00d. You're about to see it up real close.";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"~D00d, you, like, totally slagged my board!";
		"Temporary glitch, ", 0, ", I'm telling you.";
		"Enjoy the moment, ancient ~one. It ain't lasting.";
		"Watch where you're pointing that thing, ~d00d.";
		"Aim higher, ", 0, "! Watch out for the board, man!";
		"Whoa, ~d00d, that was, like, totally hostile!";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Hey, ", 0, "! I smoked your six, man.";
		"Hehe. ", 0, ", I can't believe I hit you with my eyes closed.";
		"Nice catch, ", 0, " ~d00d. Next time try it with your mouth.";
		"Your server kick you yet, ", 0, "?";
		0, ", you can't even catch my vid trail!";
		"Hehe, there's an asterisk for your URL, ", 0, "!";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"You knocked me off my board, ~d00d.";
		"Wow. I feel like I'm in the middle of a download.";
		"Whoaaaa, like, overclockin' brain drain!";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Woaaaah, Take a load off ~granny.";
		"Ctrl-Alt-Del! Ctrl-Alt-Del!";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"New road! Oh man! Tar's not dry yet!";
		"Save the board! Save the board!";
		"Hard to mosh in here.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"Oh man! Old d00d filled his diaper.";
		"Ewh. Feels like I caught a virus.";
		"Cafeteria food! Oh, puke, ~d00d.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"I'd rather drown than fight you lamers.";
		"Who filled the pool? ~d00d!";
		"Crashed and splashed!";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Permanently kicked.";
		"~There's a report card I'm not taking home, ~d00d.";
		"Is there, like, a make-up quiz?";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		0, "! You overcharged my batteries, ~d00d.";
		"Illegal search! Illegal sear...";
		0, " ~d00d, how'd you get through my surge protector?";
		"Whoaaaaa, ", 0, " ~d00d! I think you knocked a coupla my chips loose.";
		"You are like way too stressed out, ", 0, " ~d00d.";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"What? No fair, ~d00d!";
		"Pisser shot, ", 0, ".";
		"Are you, like, jacking in from ", place, ", ", 0, "?";
		"Now there's ~one happy camper.";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"D0000d, you, like, totally fragged me!";
		"Metal, dude, lots of metal.";
		"~D00D! I'm Pi22a toppin6! ", 0, ".";
		DEATH_BFG0;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"I can't believe a lamer like you got me, ", 0, ".";
		"You must have, like, gone to church this morning, right ", 0, "?";
		"How many walls did that bounce off of before it hit me, ", 0, "?";
		"Score ~one for ol' ", 0, ". 1 more 'K' like that and I'll let you try my board.";
		"Chaos theory in action, ", 0, ".";
		"Nice shot, PC Junior. What's that, ", 0, ", like a Y1K thing?";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Bow to the ", 0, " ~d00d, king of the tube!";
		"Catch some air, o' worthy old ~one, and hold your fists up high!";
		0, "! ~D00d, you have obviously been at this for a long time.";
		"Who's the ~Man? Who's the ~Man? ";
		"I didn't know you could still move like that, ", 0, "!";
		"Whoa! Stretch that old arm, ", 0, ", and pat yourself on the back.";
		// 0 = enemy name
	} //end type

		type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
		{
		"Whooaaaa, trippin'!";
		"You like totally wasted my bandwidth with that one ~d00d!";
		"Like, total roadkill, ", 0, "!";
		"That was ~major unfriendly-like, ~d00d.";
		"K1cK1n!";
		"Whoaaa, old ~d00d. ~Don't strain yer M4d 5k1LL2 on my account.";
		"That one reformatted my hard drive!";
		DEATH_KAMIKAZE0;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================
	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Commuter ~rail, exit left!";
		"Gotta move quicker than that, ", 0, ".";
		"See, ", 0, ", that's what V-Chips are made for.";
		"D00d! That puddle was your brain? Awesome!";
		"You've got ~rail!";
		"Whoaaaaaa! ~D00d!! I, like, knocked your frags all the way to ", place, "!";
		"Totally awesome.";
		"I RAWK!";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"I'm too quick and you're too lame, ", 0, ".";
		"Board movin' too fast for you to keep up, O' ", 0, " ~d00d?";
		"~D00d! Up to my elbow! Awesome!";
		"Spankity Spank!";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Go defrag your drive, ", 0, " ~d00d. You're scattered.";
		"You think you own the sidewalk, ", 0, "?";
		"Yer stew meat, ", 0, "?";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the players kills self
	{
		"Face it, ", 0, ", you suck.";
		"ROFLMAO!";
		"It wasn't even pretty. ", 0, " dude. You can't even do that right.";
		"Is your picture in the dictionary next to 'lamer', ", 0, "?";
		"Self-inflicted frag! ", 0, " enters domain of the totally stupid!";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Speedbump.";
		"~Don't bleed so much, ", 0, " ~d00d. Makes the road slippery.";
		"You've got Virus, ", 0, ".";
		0, " at dead dot com. ";
		"I'm ~lightning and you got no surge protector, ", 0, ".";
		"I am the busy signal on your dial-up modem, ", 0, ".";		
		"What a lamer!";
		0, " is now connecting to MORTICIAN.COM.";
		0, ", I formatted your sorry butt.";
		0, " = LAMER!";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Not your fault, ", 0, " ~d00d. You don't got the 'gig' to handle my code.";
		"Cool, d00d! You should see the look on your face.";
		"Nice move, ", 0, ". If you had a board under you, I never would've got you.";
		"Not bad for an opening act, ", 0, ".";
		"I'll put that notch high on the board, ", 0, ".";
		"Good fight, ", 0, " ~d00d. Next time we'll run together.";
		"I'll high-five you, ", 0, " ... if you re-attach your hand.";
		"~D00000d, I almost feel bad! ";
		// 0 = enemy name
	} //end type

	type "kill_kamikaze" //insult initiated when bot kills with the kamikaze
	{
		"That totally kicks 455!";
		"H4rdc0r3!";
		"Anarki has got game 2-NITE!.";
		"Splat, splat, splat!";
		"Call me 'Mr. System Overload!'";
		"Bet ya didn't see that one coming ol' buddy.";		
		"That wiped your drive for sure, ", 0, ".";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"Are you, like, pretending to be a statue, ", 0, "? Good disguise!";
		"Who wrote your program, ", 0, "? Like, Benjamin Franklin?";
		"Try a game-pad, fat fingers. You're, like, hitting all the wrong keys.";
		"Bend over and hold your arm out straight so I can catch a slide.";
		"How do I get into your Lamers' Clique?";
		"Charging my batteries is tougher than fragging you, ", 1, ".";
		0, ", looks like a binary meltdown to me.";
		0, " ~d00d, you're a waste of good silicon.";
		0, " ~d00d, were you like, sad, when the dinosaurs died?";
		// 0 = name of randomly chosen player
		// 1 = last victim name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"Anyone wanna sell a ticket?";
		"Aww! Freakin' gum. Who dropped it on the stairs?";
		"Dude cut the cheese.";
		"Hey, if you rearrange the letters, this URL spells ", rearrange, ".";
		one_liners;
		// 0 = name of randomly chosen player
		// 1 = last victim name
	} //end type
} //end anarki chat	
	
