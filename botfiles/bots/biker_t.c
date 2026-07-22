//===========================================================================
//
// Name:			Biker_c.c
// Function:		chat lines for Biker
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			The Seven Swords & R.A. Salvatore
// Editor:			Paul Jaquays
// Last update:		Oct. 10, 1999
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats
chat "biker"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		HELLO5;
		"Make this a good day, cuz it'll be your last.";
		"Everyone out of the pool.";
		"You just keep your hands off my hawg, ", 1, ". Understand?";
		1, ", I'm gonna kick your sorry butt into the next county.";
		"I squash cockroaches bigger and scarier than you, ", 1, ".";
		"This better not make me late for lunch.";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"I have some boredom to catch up on.";
		"Screw this.";
		GOODBYE1;
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		LEVEL_START1;
		"I'm makin' ", 4, " my playground. Get off the swings, ", fighter, "s.";
		"Where'd I park my hawg?";
		// 0 = bot name
		// 1 = randomly chosen player
		// 4 = Level's title
		} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Waste of time.";
		LEVEL_END0;
		2, " cheats and ", 3, " sucks. Now that we know, let's go again.";
		// 0 = bot name
		// 2 = opponent in first place
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Yeehaa! Kick some doggie butt!";
		"Did that hurt ya, ", 3, "?  Bet it did!";
		LEVEL_END_VICTORY3;
		// 0 = bot name
		// 3 = opponent in last place
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"Aw, you can kiss my bike wheel ", 2, " .... but lemme get it spinnin' real fast first.";
		"That's it! I'm putting me a rifle rack on my hawg.";

		LEVEL_END_LOSE1;
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		//"You just became road-kill, ", fighter, ".";
		0, ", you just became a kick-stand.";
		"Nice ~one, ", 0, ". But now it's my turn.";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"C'mon, dawg, I still gots ~one good eye!";
		"~Don't dirty the leathers, ", 0,"!";
		"You cow-poker! I'll show you what's what!";
		"OK, you gave it your best and now it's my turn.";
		"I'm gonna hurt you, ", 0, ". I'm gonna hurt you bad.";
		DEATH_INSULT2;
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		TAUNT1;
		"Yer as slow as ya are ugly.";
		TAUNT8;
		"You look mighty pretty in blood red.";
		"Lousy, cheap ammo.";
		"Sheee-oooot! I gotta get me a bigger gun than this ~one.";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Them trucks never give a hawg-rider no respect.";
		"That was some serious road rash.";
		"Anybody get the license plate on that elephant?";
		DEATH_TELEFRAGGED0;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by falling damage
	{
		"Them Kneivel boys ain't got nothin' on me.";
		"May be somethin' to them helmet laws ...";
		"Dang! Colorado Rocky Mountain high!";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Dang! That's hotter than a tailpipe runnin' jet fuel.";
		"Hoooooweeee! Too much lighter fluid on the grill there.";
		DEATH_LAVA1;
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"Any biscuits with this gravy?";
		"This is like showerin' in degreaser.";
		"Hey Lucy, this stuff's better'n yer maw's cookin'!";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Leather's heavy when it's wet. Dang.";
		"Great, now I smells like a wet dog.";
		"Stupid place for a swimmmin' pool.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"You so much as giggle, ", 0, ", and I break yer face.";
		"Did that truck run over my bike too?";
		DEATH_SUICIDE0;
		DEATH_SUICIDE5;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Get yer slimy paws off o' me, ", 0, "!";
		"Last guy that touched me can only count to ~five now.";
		"Ain't nobody touches me like that.";
		DEATH_GAUNTLET2;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		DEATH_RAIL2;
		"This ain't no campground, ", 0, ".";
		"Let's settle this outside, face to face.";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"And the friggin' rockets red ... uhhh ... green glare!";
		DEATH_BFG2;
		"Ha, ya missed, ", 0, "! 'Almost' only counts in hand grenades and ... oh, puke ...";
		"Stampede!";
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"If you hit my bike with that ...!";
		"Real funny, ", 0, ". You're like a one-man BFG.";
		"Loser! You wait there so I can kick yer teeth in.";
		"Get that carburetor checked out, ", 0, ". It's back-firing again.";
		"I ain't impressed.";
		"I hate that Commie-Cozy crap.";
		DEATH_KAMIKAZE0;
		// 0 = enemy name
	} //end type 

	type "death_insult" //insult initiated when the bot died
	{
		"I take my coffee stronger'n you, ", 0, ".";
		"You just stepped on the highway to hell, ", fighter, ".";
		"I can't believe a ", fighter, " like ", 0, " fragged me!";
		DEATH_INSULT0;
		DEATH_INSULT4;
		DEATH_INSULT5;
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Yeah, big hairy deal, ", 0, ". Ya killed me. So what?";
		"Well, ain't ", 0, " a ~hero?  Come and get your special prize.";
		D_PRAISE2;
		0, " killed me to death! Dang!";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Turn into the wind, ", fighter, ", I wanna hear ya whistle.";
		KILL_RAIL1;
		KILL_RAIL0;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Up close and impersonal.";
		"Like swattin' flies offa roadkill.";
		"That's the kind of action I like most.";
		"Yeah! Gimme some more of that!";
		KILL_GAUNTLET0;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Here's the beef!";
		"Get off the Road!";
		"Popped you like a Prairie Oyster!";
		TELEFRAGGED0;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"Hey moron, AIM, then fire!";
		"Must tickle, cuz I'm laughin'";
		// 0 = enemy name
	} //end type

	type "kill_kamikaze" //insult initiated when bot kills with the kmikaze
	{
		"That one's for lookin' at my bike, ", 0, ".";
		"That wuz real pretty.";
		"Who sez I never give you nuthin'?";
		"You is just like bugs on my windscreen!";
		"Dang. I wuz savin' that for Lucy's birthday!";
		"You want fries with that?";		
		// 0 = enemy name
	} //end type

	type "kill_insult" //Insult initiated when the bot killed someone
	{
		"I'm gonna bury ", 0, " face down so I got a place to park my bike.";
		"Ahhhh, you ain't worth the trouble to run you over.";
		"Biker 1, Roadkill 0.";
		KILL_INSULT0;
		KILL_INSULT4;
		KILL_INSULT8;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		KILL_INSULT36;
		"You'll make good maggot food, ", 0, ". That's something.";
		"You was brain-dead anyway, ", 0, ".";
	
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		TAUNT7;
		MISC6;	
		"All the sex appeal of open heart surgery!";
		"Yer like sand on the road, ", 0, ".I just blow it away.";
		"Your mamma's gonna see you on a milk carton, ", 0, ".";
		"You stay away from my dawg, ", 0, ".";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		GUYTALK2;
		"I eat my road kill.";
		"*[Bu-u-u-u-u-u-urp]* ";
		"Anybody got a ~six-pack on 'em?";
		"Okay, any o' you pretty ladies want a ride on my hawg?";
		one_liners;
		"~Don't go tellin' Lucy I was here ... she'll whip my butt.";
		femalebot, " better git her ass back in the kitchen and finish making me that chicken pot pie!"; 
		MISC0;
		MISC8;
		MISC12;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end biker chat
