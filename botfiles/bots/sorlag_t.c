
//===========================================================================
//
// Name:			Sorlag
// Function:		chat lines for Sorlag
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Authors:			The Seven Swords, R. A. Salvatore, Paul Jaquays
// Editor:			Paul Jaquays
// Last update:		May 22, 2000
// Tab Size:		3 (real tabs)
//===========================================================================

chat "sorlag"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		4," is-sss the most favored of hunting grounds.";
		"So many choice ss-specimens to con-ss-sider.";
		"A pity us-sss must waste such fine resourcessss.";
		"Sss-sorlag comes.";
		"Mmm, ", 1, "'s spleen buy me a ss-supper.";
		1, ", you too droopy to ssell. I eats you instead.";
		// 0 = bot name
		// 1 = random player
		// 4 = Level's title
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Nothing here worth the harves-ssting.";
		"Sss-sorlag goes.";
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"All ss-stand ss-still. Sss-sorlag kill quick. No much pain. Sss-sorlag promissse.";
		"Kidneys-ss? Ss-sorlag smell. Ss-sorlag harvest. Ss-sorlag sell.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Lucky sss-scum are you, that us didn't get claws-sss on you, ", 2, ".";
		"Again. Again! Match not fair. Us-ss not win.";
		"Let us-sss go!! Much to harvest. Not to waste!";
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Fat ~one! Intestines sss-sell by foot.";
		"Harvest comes early. Us-sss only takes most choice.";
		"Not kill all ... too many rot before us-ss can harvest.";
		"Us-sss mus-sss-t remember sss-second body bag next time.";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"Hiss-ss!";
		"You thinks you good? Sorlag collect you-sses liver next!";
		"Us is just making you ", fighter, "s worth-sss more!";
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Sssso, you desperate to ss-save sss-spleen from us ssss-scalpel.";
		"Alien ss-scum! Con-sss-istent you are.";
		"Ssssss!";
		"Us kill you dead, ", 0, "!!";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		//"I fill that hole with your skin!";
		"It bites us back, it does.";
		"Ssssss!!! For that, Sss-sorlag must destroy it.";
		"Vivisection soon for ", 0, ". Take parts-ss while still live.";
		//0 = shooter	} //end type
	}
	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Best to take merchandise alive.";
		"Die fas-ssst, ", fighter, ". Damage goods less-sss.";
		"Sorlag sell you piece by piece if us must, ", 0, ".";
		"Us-ss slow ", 0, " down, then Ss-sorlag kills it.";
		"Yes-ssssss!";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Like sorg death rite. No body to sss-sell.";
		"Hissss!";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Arrrgh! All broken.";
		"Curses-sss!";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Heat too much. Sss-scales not compensate.";
		"It burns us-ss! It burns us-ss!";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"To my thongue ith thticksth.";
		"Mmmmm. Like Sss-sorg mating jelly.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Sss-sorg not fissss-sh. This-sss bad!";
		"Sss-sorg Reptiloid ... not Amiphibian.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Ssssss. Us-ss clumsy like mammal.";
		"us likes to kill so much, us kills us.";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Put your hand back... Us bites off you fingers!";
		"Wretched ", 0, "! Us-ss carve pieces off you later.";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Good weapon. Like Sss-surgeon sss-scalpel in gun.";
		"AAAARGGGH!!!! So much us hateses sss-sniper!";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{	
		"All ruined. Bad gun. Wasteful.";
		"Droppings-ss!";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Us take, sss-sell people of Hunter. Do sss-same to you people, ", 0, ".";
		"Us find you, ", fighter, ". Repay insss-sult!";
		"Is you tasty, ", 0, "? Us-sss find out soon.";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Worthy sss-savage. Us think you parts sss-sell good.";
		"Well does-ss ", 0, " ssss-shoot.  Too well ... hsss.";
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Chewer of Droppings-sss!!";
		"Was-sss-teful. Can harvessst nothing!";
		"Ruiner of Joy take you.";
		0, " lacks-sss skill to sss-succeed.";
		"Ssss, For dog-meat only us sss-sell you, ", 0, ".";
		"Only ~Hunter female deserves-ss sss-such a death.";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"For dogmeat us will collect you all.";
		"Sss-sorry am us to waste ss-such fine mechandis-ssse.";
		"Better to des-sss-troy evidence.";
		"S-ssorry is us to use this-ssss.";
		"Chew droppings-sss and terminate now!";
	    "Meat animals inferior quality. This-ss bes-ssst way to thin herd.";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Sss-shoot clean ... more to dissect and sss-sell.";
		"Sss-sorlag hopes us not hit exsss-pensive parts.";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Fry no organs-ss ... Cyborg makes buy much, pay good.";
		"Good anger release. Sss-sorlag not sss-so mad now.";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"No!!! Us needs those pieces!";
		"Sss-shameful wastings.";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"Improving the breed, I ss-see, ", 0, ".";
		"Presents, presents everywhere. Is-sss my hatch-day?";
		"Us thank you, ", 0, ". You brings-ss us profit with no overhead.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Die slow, ", 0, ".  Us sss-show you pieces as us take out.";
		"Ummmmm. Some pieces tastes good.";
		"Oo-oops-sss. Too much snacking on good partssss.";
		"No good sell parts in ", 0, ".  Us make into protein chowder.";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Many credits us would gets for you, ", 0, ".";
		0, " looks-sss better harvested.";
		"~Sorlag want to clone ", 0, ". Then sss-sell many time.";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly
	{
		"When with you, us am done, Sorlag kill your dog.";
		"Whisper, whisper ... you ears us clean with us forked tongue.";
		"Good shooting eye, ", 0, ". Sorlag think us dig out and sell.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"Catch it? Hmmm? Will Cyborg makers buy it whole?";
		"I sss-sense body heat ... hot blood ... profit!!";
		"Intes-ssstine make good floss-sss.";
		
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end sorlag chat