//===========================================================================
//
// Name:			Uriel_c.c
// Function:		chat lines for Uriel
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Last update:		May 22, 2000
// Author:			R.A. Salvatore, the Seven Swords, and Paul Jaquays
// Editor:			Paul Jaquays
// Tab Size:		3 (real tabs)
//===========================================================================

chat "uriel"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"
	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		"My wings do droop in anticipation.";
		"How many must I kill before my scale is balanced again?";
		4, ". This place looks too familiar.";
		"The call of the ages beckons me once again.";
		"Tremble, mortals, Uriel returns to ", 4, ".";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"I am doomed to return. There will be no rest.";
		"Perhaps I will find some worthy to ascend... next time.";
		"My rest will be short, of that, I am sure.";
		"Farewell. Even Death takes a holiday.";
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Again I am summoned to show insects the way to perfection.";
		"I am the darkness and the way.";
		"This place is filled with inferior beings.";
		"This will not take much time or ammunition.";
		"Tremble, mortals, and more so if you survive.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Some will ascend. Others must try again.";
		"It all blends together.";
		" **sigh** And now again... always again... ever again.";
		"Eternal is the Arena... I fear that more than I fear you, ", 2, ".";
		"I am mired in the midst of foolish ambition.";
		"Defeat is not you greatest worry, ", 3, ". That will be victory.";
		"What is it that you have truly won, ", 2, "?";
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Behold the Dark Angel of Death!";
		"I take no pleasure in killing you ", 3, ". It is expected.";
		"There can be no other outcome, lesser ones.";
		"Do you not understand who I am?";
		"Where is the thrill?";
		"Defeat is not you greatest worry, ", 3, ". That will be victory.";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"If you defeat a thousand, this will remain your greatest trophy, ", 2, ".";
		"Perhaps there is rest, at last.";
		"Too long have I toiled to sate the egos of inferiors.";
		"Move on, ", 2, ". Ascend, and then descend, and know despair.";
		"Where the heart cannot go, the body cannot follow.";
		"Weigh carefully the choice of being victorious, ", 2, ".";
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Your deceptions will not elevate you, ", 0, ".";
		"Too long have I lived to suffer such cowardly acts, ", 0, ".";
		"Your treachery will find you out, ", 0, ".";
		//0 = shooter
	} //end type

	type "damaged_nokill" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		0, "! Shoot straighter, I say! For the heart!";
		"Scars upon scars upon scars.";
		"Curse this life!";
		"You think yourself worthy, ", 0, "? Prove it.";
		"I must retaliate. It is the way.";
		"Is there no end to this madness? Nay, none, and so your death will not be swift, ", 0, ".";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Only a child should weep so.";
		0, ", you would not last a span of ~five heartbeats in Vadrigar.";
		"Insignificant worm. You spill your life upon halls stained by the blood of thousands.?";
		"Ah, the eternal sound: an inferior wailing in agony.";
		"A thousand times a thousand rockets will not bring you the pain I have known, ", 0, "."; 
		"Is this the life you willingly choose, ", 0, "?";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"They crowd the Arena, eager to ascend... but there is only death.";
		"A fit end for an insect, ", 0, ".";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Too close to the sun have I flown, and thus my wings are no more.";
		"I fly on wings of lead!";
		"I will rise again, I fear.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Sweet heat, release me these mortal bonds.";
		"Encase me, molten stone. A fitting statue for the Dark Angel.";
		"I burn. I care not at all.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"Indignity. Once I ruled, now ...";
		"In the end, the slime will rule and all the rest shall be dust.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"All my life I see before me. There is only pain.";
		"Darkness take me, before I rise up, only to fall again.";
		"There is a peacefulness here that is appealing.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"'Tis my own doing ... none shall ascend!";
		"Was this done on purpose? I must examine that which is in my heart.";
		"I have lost my edge. The mighty have fallen.";
		"Immortal weariness has stolen my edge.";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Are you worthy to ascend, ", 0, "? Or have I sunk so low?";
		"The ages have diminished the mighty to the level of ... ", 0, ".";
		"The mewling child dares swing at the father?";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"I did not see it, ", 0, "... that is preferred.";
		"How? Where? Nay, the question is 'why?'";
		"There is promise in ~one who displays such stealth, ", 0, ".";
		"Sudden and without warning, ", 0, "... you have perfected one aspect of death.";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"But how would you fight without such firepower, ", 0, "?";
		"It is a challenge reduced to throw weight. There is no honor.";
		"Scatter me across all the world, ", 0, ".";
		"I pray that this mighty weapon is enough power to end my reign.";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Now you learn the truth, ", 0, ".";
		"Rejoice not, ", 0, ". All victory is temporary.";
		"You are not worthy, ", 0, ". You are merely lucky, and luck cannot hold.";
		"~One frag does not an Arena Lord make, ", 0, "!";
		"If only you understood the truth: that I let you win.";
		"Dance not, puny ~one. The eons felled me, not ", 0, ".";
		"What is it that you have truly won, ", 0, "?";
		"Your cheers reveal your ignorance, ", 0, ".";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Attach my wings to your coil if you can ", 0, ". Wear them with pride.";
		"My cycle is near completed. Yours is just begun, ", 0, ". I pity you." ;
		"Behold ye, the new Dark Angel of Death. Enjoy this time of ignorance.";
		"I fall to you, ", 0, ", but there are levels you must yet scale.";
		"I honor you in deaths prayer, ", 0, " ... may your fall will be as swift as your rise.";
		"May death find you in battle, ", 0, ", and quickly, before you begin the long descent.";
		"Has the Arena found its next champion?";
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Though our blood and ichor co-mingle, we are not brothers, you and I, ", 0, ".'";
		"I am spread to the winds, a fine mist, floating upon fate's dark vapours.";
		"Tis no honor to slay your foes thusly.";
		"The shadow stain upon you soul flees, yet you remain unclean.";
		"Again? How tragic.";
		"Poor little ", fighter, ". What will you do when you must rely upon skill alone?";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"Here, ", 0, ", I share with you the darkness that cloaks my soul.";
		"Pity. Twas but a moment's fleeting distraction, now gone.";
		"Alas it is done. What glory was there in this ephemeral event?";
		"Hmmmmph. I gain so little pleasure from the deaths of so many.";
		"Mark my words, there will be gnashing of teeth and wailing unto heaven over this.";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"I prefer not this weapon, but neither would I waste my time in battling you, ", 0, ".";
		"Ignorance is bliss, but only if bliss is death.";
		"Sometimes it is preferable not to hear the serpent's rattle.";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Go home, puny ~one, this is not your place.";
		"I waste no ammo on the likes of you, ", 0, ".";
		"The touch of Death. Cold ...";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"The fates have deemed you unworthy ", 0, "... obviously.";
		"Just go away.";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"Why do I bother?";
		"Hmmph. Hmmmph. This is what you must call humor.";
		"Well done. You do excel at some things, ", 0, ".";
		"It could go on without me, I see.";
		"A feint within a feint? Or are you just dead ", 0, "?";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Bah! Each new round brings an opponent less worthy.";
		"The ease of your execution only heightens my boredom, ", 0, ".";
		"I have forgotten your name already.";
		"Your race weakens with the eons... your ancestors would still be alive.";
		"There are many ways to die. I suspect that you have perfected them all, ", 0, ".";
		"In a thousand years, I have never seen ~one as pitiful as thee, ", 0, ".";
		"Too easy. Where has the challenge gone?";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Trust me, I have done you a favor and an honor, ", 0, ".";
		"If you so desire the life that I have known, try again. You may get there, ", 0, ".";
		"The Lords shall be grateful that I killed you. You would have brought them pain, ", 0, ".";
		"I will write your name in my great book ... that, I have not done for centuries untold.";
		"The Dark Angel of Death has spoken. Hear his voice!";
		"Show promise and learn well, and learn now that this is not the course for you, ", 0, ".";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"Run faster! Shoot straighter! Do something right and make this interesting, ", 0, ".";
		"This is what they send me? The Masters' respect for me has waned, I fear.";
		"I will defeat you, ", 0, ". That is assured.";
		"Have you no skill, ", 1, "? Or have you just surrendered to the inevitable?";
		"Kill yourself, ", 0, ", and save me the trouble.";
		0, ", there is no belt small enough to carve a proper notch for you.";
		1, ", you are a worthless ", animal, " No! Less than worthless!";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"Yes, I know. I talk too much.";
		"Me? I talk too much? You have yet to meet Xaero!";
		"Miserable wings, fly me far from this place!";
		"My talons once raked through stone. The stone is harder now, I fear.";
		"The world is diminished! They let inferiors in here!";
		"Slime corrupts water; lava melts slime; water cools lava. There is no winner.";
		"I know every inch of ", 4, ", every trap and every stain.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end uriel chat

	

