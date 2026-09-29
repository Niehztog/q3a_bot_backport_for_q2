//===========================================================================
//
// Name:		Cadavre_c.c
// Function:		chat lines for Cadavre
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:		The Seven Swords & Bob Salvatore
// Editor:			Paul Jaquays
// Last update:		Oct. 10, 1999
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats
chat "cadavre"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"
	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		"That churning, squishy sound means ~Cadavre has arrived.";
		"Heh! I'm just falling apart with anticipation!";
		"Win or lose, I'm loadin' up on snacks.";
		"I'm collectin' donations for the hungry dead.";
		"Drat, nothin' but left-overs again.";
		"Name's ~Cadavre. Just say it ~three times, real fast!";
		"It's 'Thriller Time'!";
		HELLO6;
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"I don't frag with wimps, sissies, or ", fighter, "s. That rules out all of you.";
		"I've got me an upset stomach. You have NO idea what that really means.";
		GOODBYE5;
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"You want 'em marked or unmarked? Your graves, I mean.";
		"We're renaming ", 4, " to 'The Morgue' after I induct you clowns into Cadavre's world.";
		LEVEL_START0;
		LEVEL_START3;
		"Can we speed this up? I got a baby-sitting job to go to.";
		"Can we speed this up? I'm moonlighting at a medical college.";
		// 0 = bot name
		// 4 = Level's title
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first or last 
	{
		"I managed to shove a few maggots in you, ", 2, ". They oughta be ripe real soon.";
		"HA!! You didn't bury me face down. I'm ready to fight again.";
		2, ", I hand you my heart. ", 3, ", you get the butt. Har Har Har!";
		// 0 = bot name
		// 2 = opponent in first place
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Feel that rot settlin' in. It kind of tickles, but you get used to it.";
		"To the victor goes the spoiled... and the ripe and the rotting. Yummm!";
		LEVEL_END_VICTORY3;
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{	
		"The maggots beat everyone eventually.";
		"No sense in beating a dead horse.";
		"Drat. I popped a maggot outta my eye. Help me look for it.";
		"Course I lost. Dang maggots ate up all my mojo.";
		"Keep smiling, ", 2, ". Life is brief and so is glory, but death is forever.";	
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Nice gun, ", 0, ". for your sake, I hope it's tasty.";
		"Now you're going to have to ask permission to beg.";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Who said you couldn't hurt a fly? You just killed ~four of mine.";
		"Hey, ", 0, "! You just whacked ~three of my favorite maggots!";
		"You got me right in the butt. Now I got me a semi-colon.";
		"I let you hit me because I like to watch the pus run.";
		"Now I'm gonna hug you, ", 0, ". Maybe we'll stick together.";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Well, there's ~one bodily fluid. I want to see ~three more.";
		"Who you gonna call ...?";
		"Let's speed this up, ", 0,". I got larvae waiting to hatch.";
		"A few more shots like that, ", 0, " and you'll begin to look like me.";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Whoooaaa! That's ~one big maggot!";
		"Har! I bet that's how a zit feels!";
		"Ashes to Ashes. Dust to dust. Drippy, smelly gunk to drippy, smelly gunk.";
		DEATH_TELEFRAGGED1;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"I've been buried in deeper holes than this!";
		"This better be deeper than 6 feet.";
		"I wish I'd been around to watch that splat! Had to be beautiful.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Arrgh! Not cremation!!!!";
		"Hey, maggots pop like corn kernels!";
		"Sear me on both sides to lock in the juices.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"Once it eats through the skin, it's gonna meet the rest of its family.";
		"If it can't beat me from the inside, it sure as heck won't take me from the outside!";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"I thought dead bodies floated.";
		"Dead in the water. Heh."; 
		"All the little ~bugs inside are drowning! Peace at last!";
		"Hey everybody, come get your Cadavre stew!";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"You see what death looks like, ", 0, "? Ain't pretty, is it?";
		"I am a master at killin' the dead, me included.";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Your mama slaps harder than that, ", 0, ".";
		"Push any harder, ", 0, ", and your hand will go right through me.";
		DEATH_GAUNTLET1;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Heh heh, I see you in your little camper hole.";
		"Thanks for the help, camper. I had a little gas I'd been wantin' to get out.";
		"Now that definitely hit the spot!";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"The worst part is that most of these pops are coming from inside me.";
		"Guess the worms'll be eating at separate tables.";
		"I'm falling to pieces ... and I mean that literally!";
		DEATH_BFG2;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"I guess my heart's not in this. Mind handing it back to me?";
		"You can't kill somethin' what's already dead! At least not easily.";
		"Been there, done that. I'll make sure you share the experience, loser.";
		DEATH_INSULT2;
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Lucky shot, ", 0, "? Or did some other ", fighter, " pull the trigger for you?";
		"You were fun. I'll stick around in your nightmares, I promise.";
		"Bring out your dead! Bring out your Dead!";
		// 0 = enemy name
		// 1 = random opponent
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"That just spread the goodness all around.";
		"I've just been promoted from dumpster bait to air pollution.";
		"Yeah? Well I love you too, ", 0, ".";
		"Now I'm mad. You know how long it took to collect all them maggots the first time?";
		"Now you've gone and scared my pretty little maggots.";
		"Can I get back to you on that? I'm spread a little thin right now.";
		"That's some temper you got there, ", fighter, ".";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with rail gun
	{
		0, " isn't just a ", fighter, ", it's a tasty dessert topping too.";
		"Heh. That was loud enough to wake the dead.";
		"Bwahahahahahaa!";
		"Yeah! Just sharing the love.";
		"Bartender! Maggots for everyone!";
		"Lunchtime!";
		// 0 = enemy name
	} //end type


	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Dead aim...dead on ...dead eye ...dead to rights ...dead ringer.";
		"I think I struck oil with that ~one.";
		"That's an exit door for guts and an entryway for all the little critters I've been saving for you.";
		KILL_RAIL1;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Since I'm this close, ", 0, ", I'll just grab some snacks for later.";
		"The touch of death.";
		"I'll just help myself to a couple o' yer organs here ... I need some spares.";
		KILL_GAUNTLET2;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"I just love jigsaw puzzles with LOTS of pieces.";
		"Can someone get a shop vacuum for ", 0, ", please?";
		"You're spreading yourself thin again, ", 0, ".";
		"Clean up in aisle ~five";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"That only leads to an unhappy afterlife, ", 0, ".";
		"And they say that maggots are eating MY brains.";
		"Har, Har, Har!";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Now I'm going to kiss you. Stick my tongue down your throat... maggot slide.";
		"I'm hungry and you're dead, ", 0, ". Look, I even brought gravy.";
		"Making an S.O.S. with your intestines isn't going to help.";
		KILL_INSULT0;
		KILL_INSULT3;
		KILL_INSULT4;
		KILL_INSULT8;
		KILL_INSULT19;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Somewhat impressive moves, ", 0, ". But you're still a heap o' frags.";
		"Okay, ", 0, " since you took your head off for me, I'll take my hat off for you!";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"I can smell you over my own rot, ", 0, ". Means you really stink!";
		"Spell yer name for me, ", 0, "? I'm doing up yer gravestone now.";
		MISC4;
		TAUNT2;
		TAUNT4;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		0, "'s final suffering will be exquisite.";
		"I hate it when flies fly OUT of my mouth.";
		"Anyone got a truss? I keep tripping over my guts.";
		0, ", pull my finger. Hey, give it back!";
		"This place is deadsville!";
		"Bwahahahahahaa!";
		"Life, un-unlife, what's the diff?";
		"Anybody got a ~lightning gun?  I need me a recharge.";
		"Gibs are my normal state of existence.";
		"I'm just a bag o' random gibs.";
		MISC12;
		MISC5;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end chat


