
//===========================================================================
//
// Name:			Hossman_c.c
// Function:		chat lines for Hossman
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			The Seven Swords & R.A. Salvatore
// Editor:			Paul Jaquays
// Last update:		10-05-99
// Tab Size:		3 (real tabs)
//===========================================================================

chat "hossman"
{
	
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"The ", 4, ",? This place ain't in my contract.";
		"Not ", 1, " again! If the $$$ is right you may not lose... this time.";
		HELLO3;
		// 4 = Level's Title
		// 1 = random opponent
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{	
		"Farewell, ", fighter, "s. I got a better paying gig elsewhere.";
		"I guess you can breathe a little easier now that I'm outta here.";
		GOODBYE1;
		// 1 = random opponent
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{	
		"I have a simple game plan. I shoot you. You die. I win. You rot.";
		"To be perfectly honest I could care less who dies, long as I walk out of here.";
		LEVEL_START0;
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"If you want to do my job for me,", 2, ", that's fine by me.";
		"I'm using your ~bones to uphold my reputation.";
		LEVEL_END2;
		// 2 = opponent in first place
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"A ~cool steady hand. The calling card of a professional.";
		"Well, we've established that ", 3, " isn't worth much.";
		"This is a sucker's job. These ", fighter, "s are all easy marks.";
		LEVEL_END_VICTORY3;
		// 0 = bot name
		// 3 = oppnent in last place
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"You won't get away with this, ", 2, ". I don't leave witnesses.";
		"I think my new assignment is redecorating ", 2, "'s face.";
		"Ok, I'm ~cool. Just need to frag ", 2, " more often next time.";
		LEVEL_END_LOSE0;
		// 2 = opponent in first place
		// 3 = opponent in last place
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		0, ", do I scare you that much?";
		"You're going to eat that chat bubble, ", 0, "!";
		"I was on the phone!";
		// 0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"You want to fight me? Me? You better get some help, ", 0, ".";
		"Hey ", 0, "! Good idea! Get me mad and you die first.";
		"Well ", 0, " you're at the top of my hit list ... with a bullet.";
		"You wanna peice of me? Yer gonna need a bigger spoon!";
		PRAISE;
		MISC13;
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Get back here, ", 0, ", and I'll finish what I started.";
		"That should've killed you! Must be a wind change or somethin'.";
		TAUNT5;
		TAUNT1;
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Is this some new form of separation anxiety?";
		"Makes me remember why I gave up drinking.";
		"Ugh. Feels like I slugged down about ", number, " shots of ~m0nkey ~killer.";
		DEATH_TELEFRAGGED1;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"It's the chance you take when you perform without a net.";
		"My new technique: a fly-by shooting. Needs a little work.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Heh, I like my coffee hotter than this.";
		"Heh, I like my ", food, " hotter than this.";
		"I guess it's better to burn out than fade away.";
		"Hot? You call that hot?";
		DEATH_LAVA1;
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"Is this that new special sauce?";
		"Here I go, hittin' the sauce again.";
		"Just great ... death in a slime mold.";
		"And to think, I used to like lime gelatin.";
		"Ugh. Like swillin' radioactive ", liquid, ".";
		DEATH_SLIME0;
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Yeah sure, never a lifeguard on duty when you need ~one.";
		"Next time, I need to wait an hour after I eat that much ", food, ".";
		"No fence. I oughta sue.";
		DEATH_DROWN1;
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Now that's the way to dust someone!";
		"Clumsy of you.";
		"Mental note, can't collect price on own head...";
		"Klutz!";
		DEATH_SUICIDE1;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Nice punch, It's refreshing to have a real competitor.";
		DEATH_GAUNTLET1;
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{	
		"Sneaky! You should talk to me about a job, ", 0, "!";
		"Heh. Caught that ~one with my teeth.";
		DEATH_RAIL2;
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"My mother fragged me once. Once.";
		"Messy ... it's just not like the old days.";
		DEATH_INSULT3;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Give a kid a weapon and he thinks he's a soldier.";
		0," is still shaking. First kill for you kid?";
		"Ok, ", 0, ", you I kill for free.";
		"~Two-bit Punk!";
		"You better have eyes in the back of your head, ", 0 , "."; 
		"You're dog food now, ", 0, ".";
		"Well I imagine someone's got to win the lottery.";
		"I look better dying than you do shooting, ", 0, ".";
		DEATH_INSULT4;
		DEATH_INSULT1;
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Hate to admit it, ", 0, ", but from ~one pro to another, good shot!";
		"Looks like you got a little talent there kid. Use it.";
		"Okay, you win this ~one, ", 0, ". ~Don't get cocky.";
		"Not bad for an amateur.";
		"And a round of applause goes to ", 0, ", for a fine performance.";
		"I've seen a lot worse from a lot better.  Good shot, ", 0, ".";
		D_PRAISE5;
		D_PRAISE2;
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"They don't pay me enough to put up with this kinda junk.";
		"Punk";
		"I am at a lost for words.";
		"I guess you gotta use weapons that match your skill level, ", 0, ".";
		"No-skill, kamikaze-lovin' twerp.";
		"I bet you post anonymously on message boards too, ", 0, ".";
		DEATH_KAMIKAZE3;
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with rail gun
	{
	"Cheap or not, points are points!";
	"I'm selling death and destruction at wholesale prices!";
	"I told you to keep away from me, ", 0, ". But you wouldn't listen.";
	"I bet they could feel that one all the way to ", place, "!";
	KILL_KAMIKAZE1;
	// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		0,", don't take it personally, but yer makin' my job too easy.";
		"Perfect ... ~one slug, one kill.";
		KILL_RAIL2;
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Great, now my hand is stuck in your skull!";
		"Slap!";
		KILL_GAUNTLET2;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Geez, ", 0, ", you make ~one ugly cup of stew.";
		"Spontaneous Frag-bustion!";
		"Cleaned and gutted. I'm too good!";
		TELEFRAGGED0;
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when player kills self
	{
		"Quick, frag ", 0, " before the stupid ", fighter, " suicides again.";
		"Wow, I guess my reputation precedes me.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"You know, ", 0, ", I really just did it to stop your whining.";
		"Maybe they should call you hamburger instead of ", 0, ".";
		"You realize, ", 0, ", you just keep improving my resume.";
		"Hehehe ... sorry, I just never saw anyone make a face like that... before I blew it off.";
		"Honestly, ", 0, ", you look much better dead.";
		"I found your face, ", 0, ". You want it back?";
		KILL_INSULT0;
		KILL_INSULT3;
		KILL_INSULT34;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		0, " I sure earned my paycheck on you.";
		"Wow! Thats the most excitement ", 0, " has ever shown me.";
		"Hey, ", 0, ". Atleast pretend like you're trying to win.";
		"How many times have you died, ", 0 , "? There's parts of you everywhere.";
		PRAISE4;
		PRAISE2;
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		one_liners;
		"I'm on vacation. I just do this to relax.";
		"Wow, is this beginner night?";
		"Look! ", 0 ," is hiding in the corner again!";
		"Hmmm ... so many ways to kill you, ", 1, ", and so little time.";
		"At least ", 1, " has a reason to be so bad. What's your excuse, ", 0, "?";
		"Hey, ", 1, ". This yellow puddle must be where I nearly got you.";
		"I refuse to have a battle of wits with an unarmed opponent, ", 1, ".";
		MISC12;
		MISC10;
		TAUNT9;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"I'm passing out free trips to hell. Apply in person.";
		"Will someone wake up ", 0, ". I have to use up my ammo by the expiration date.";
		"So, ", 0, ". ~Don't you think we oughta smoke ", 1, ", and then have us a little chat?";
		"This is my kind of happy hour.";
		"I don't have Mynx's phone number. Try the restroom walls in ", level_title, ".";
		MISC1;
		MISC9;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end chat


