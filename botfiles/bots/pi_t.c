//===========================================================================
//
// Name:			pi_c.c
// Function:		chat lines for Pi
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			Paul Jaquays
// Editor:			Paul Jaquays
// Last update:		April 05, 2000
// Tab Size:		3 (real tabs)
//===========================================================================

chat "pi"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Pi has been summoned to be your death today, ", 1, ".";
		"Beware ", 1, ". Pi's skill is endless does not repeat itself.";
		"Let it begin.";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"This is not ending, only change of direction.";
		"The circle will come about and with it, Pi will return.";
		
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"We begin ... NOW!";
		"We come about full circle again.";
		"All variables are equal once again. Victory favors Pi.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Today Pi was unequal to challenge. Tomorrow will care for itself.";
		"Your techniques warrant future study, ",2,".";
		"Pi lives for challenge. Another match?";
		"Again. My mistakes will not be repeated.";
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"The circle turns, crushing all before it.";
		"Combat is my religion. Victory, my Paradise.";
		"You accept defeat far too well, ", 3, ".";
		"Defeat clings to you like lover, ", 3, ".";
		"Defeat loves you well, ", 3, ".";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"I will think on my mistakes. This will not happen again.";
		"Defeat tastes of ashes and bile. It is not good taste.";
		"My hatred, I will store away until next we meet.";
		"Hail, ", 2, ". You have earned my respect.";
		// 0 = bot name
		// 2 = opponent in first place
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"There are no rules. Only Victory or Defeat.";
		"My mistake. To talk is to die.";
		"Well shot, ", 0, ".";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"Your mistake, ", 0, ". You let me live.";
		"For shame, ", 0, ". You led me to believe you had bigger gun.";
		"You waste shot, ", 0, ".";
		// 0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Pi has miscalculated. You live this time.";
		"That was a tracer round.";
		"Bah. Pi thinks you have no vital organs.";
		"No matter. I am just practicing that time.";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"I am breathless. Was it good for you also, ", 0, "?";
		"Once this happens and I respawn with a big eyeball for head.";
		"This is very hard on cybronic parts.";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Pi makes crater like circle, yes?";
		"Sometimes, ground is harder than Pi.";
		"Gravity can be very unkind to a girl.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Must remember to retune my pain sensors.";
		"Cooling system shutdown."
		"Bah. Flesh is weak. If I am all metal this is not happening.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"I do not think that green is my color.";
		"It feels like my batteries are leaking again.";
		"Ugh. It is vat of molten ", 0, " goo.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"I curse this weak flesh. Air should not be necessary!";
		"Cybronics do not float well.";
		"Air is for weaklings.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{	
		"A slight miscalculation.";
		"I think I experience error in my targeting computer.";
		"Pi killed self to deprive you of the pleasure, ", 0, ".";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Strange. I feel no humiliation at this. I feel I should kick ", 0, "'s butt.";
		"Luck is strong with you, ", 0, ". But skill is not.";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"So. You fear to face me in level combat, ", 0, ".";
		"Hah. ~Xaero, you are not.";
		"How do you win when there is no rail gun?";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"It is good weapon, this BFG, yes?";
		"All fair. No complaining when I do same to you.";
		"No skill there.";
		"I think you are compensating for something that is not so big.";
		// 0 = enemy name
	} //end type

	//type "death_kamikaze" //initiated when the bot died by a kamikazee blast
	//{
	//	"Your radius is quite impressive, ", 0, ".";
	//	"How clumsy of you.";
	//	"Perhaps you should cut down on caffeine, I think.";
	//	"And you are happier now, yes?";
		// 0 = enemy name
	//} //end type


	type "death_insult" //insult initiated when the bot died
	{
		"Hmmpf. I am not impressed, ", 0, ".";
		"Shot in the back again. I should not expect more of you, ", 0, ".";
		"Was cheap shot.";
		"You were best fighter at what, a school for little girls?";
		"Bah. I have seen better moves in chess game.";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"That was shot was worthy of my own skill, ", 0, ".";
		"Bravo. Pi salutes you, ", 0, "!";
		"I am impressed. You are a warrior of many surprises, ", 0, ".";
		"That was a move worthy of a champion, ", 0, ". Are you sure it was yours?";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"I am sniper, not camper.";
		"Was easy shot. My eye is zoom lens.";
		"I think am good shot with this weapon.";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"'Humiliation.' I think I am liking this word.";
		"You should not come within the radius of my touch, ", 0, ".";
		"Some things require a woman's touch. A very, very hard touch.";
		"How very touching.";
		KILL_GAUNTLET0;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Pi feels so very close to you now, ", 0, ".";
		"Mmmm. That was linked into my pleasure circuits.";
		"Pi thinks you need faster processor, ", 0, ".";
		"Hah! You are too slow again, ", fighter, ".";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"I am confused, ", 0, ". Is not purpose of match to kill other person?";
		"You are doing comedy routine, ", 0, "? That was big joke, yes?";
		"If you are trying to help pi win, ", 0, ", I am most grateful.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Pi is like kamikazee. Inside my kill radius, no one lives.";
		"We must do this again sometime ... after you have improved skill.";
		"I think you like this dying. You do so much of it.";
		"You must practice more, ", 0, ". That was a very unprofessional death.";
		"Bah! I am needing more challenge to be arena lord.";
		// 0 = enemy name
	} //end type

	type "kill_kamikaze" //insult initiated when the bot killed another player with kamikaze
	{
		"Fear my diameter!";
		"Pi does not recognize nuclear arms treaty. Treaties are for weak.";
		"If you cannot stand heat, stay out of kitchen.";
		"No skill, yes. But good for easy kill.";
		"Bah. That is not as satisfying as I had hoped.";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"We are good match, ", 0, ". I am programmed to kill ... and you die so very well.";
		"Pi wishes for more opponents like you, ", 0, ". You are easy to kill.";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"Do you know when my REAL opponent is supposed to arrive?";
		"I do not think you have the radius to be here, ", 0, ".";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"I have argument to settle with a 'hardcore' warrior. Do you know 'Anonymous'?";
		"We are all points on the circumference of universe.";
		"This match is lacking challenge.";
		
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end pi chat

