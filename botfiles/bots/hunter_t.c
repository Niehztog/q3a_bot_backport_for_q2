
//===========================================================================
//
// Name:		Hunter_c.c
// Function:		chat lines for Hunter
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:		R.A. Salvatore & The Seven Swords
//Editor: 		Paul Jaquays
// Last update:		May 11, 2000 (PAJ)
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats
chat "hunter"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Come forth, scum Sorg! Meet your dire destiny.";
		"We are Hunter. You are prey.";
		"Forward! Forward we march to the extinction of the reptiloids.";
		"We will use this time to sharpen our tactics.";
		"We know well the hiding places in ", 4, ".";
		"In the name of our lost people, we greet you.";
		"Aha! ", 4, " is ~one of our favorite places to be.";
		"Today, we are death incarnate! Fear the name of ", 0, "!";
		"Avaunt, ", 0, ",! We have  a light bolt with thy name on it!";
		0, " is here. There will be no survivors!";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"We must away to the hunt before the trail grows cold.";
		"More pressing battles await.";
		"We are off to find the lizard.";
		"Pfffah! Bigger prey is to be found elsewhere.";
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"The smell of reptiloids infects this place! And we are in need of new boots.";
		"Scatter quickly, mine prey.";
		"Set loose the hounds! the lizards are near!";
		"'Tis time to learn who are the ruled and who is the ruler.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Only we decide where we shall be placed. This is our choice ... this time.";
		"There is no turning from the path.";
		"We do not lose. We blend in.";
		"Know, O' ", 2, ", that the true huntswoman kills only the strongest. The rest are unworthy.";
		"Patience separates the good from the great. We bide our time.";
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Nyah, there be too much warm blood here.";
		"The she-wolf reigns.";
		"We walk the straight road. You move or die.";
		"Woman is the source of life... and the source of death.";
		"Trophies, trophies everywhere.";
		"We know you are believers now... a pity you are all dead.";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"Bah! You be Sorg sucklings.";
		"So be it.";
		"Last? First? It matters not at all.";
		"Know, ", 2, ", We only find more to avenge.";
		"This loss is the fuel of desire.";
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"You trained with the Sorg, eh ", 0, "?";
		"Bah! A pox upon your house ", 0, "!";
		"This speech just became your eulogy ", 0, ".";
		"Pfahhh! We knew we smelled the taint of Sorg on you, ", 0, "!";
		"We have come to expect such vileness from the accursed Sorg.";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"You wound the tigress, ", 0, ". More the Fool, you.";
		"We feel no pain, ", 0, ".";
		"Keep shooting us, ", 0, ". Feed the hunger that burns in our soul.";
		"If you think that scratch evens the score, ", 0, ", think again.";
		"We like it rough, ", 0, ". It heightens our appreciation for the pain we inflict.";
		"Do you wish us to scream, ", 0, "? Pfahh! We spit instead.";
		"Who is winning is not important, ", 0, ". All that matters is who is left alive.";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Ahhh! The blood trail ... there is no hope for you, ", 0, ".";
		"Flee, little mouse. The chase is the sugar of our meal.";
		"We smell your fear, ", 0, ".";
		"Piece by piece if we must, but you will die, ", 0, ".";
		"You will not die unfound in the forest, ", 0, ". We will have your skin.";
		"Kill yourself, ", 0, " ... it will be easier.";
		HIT_NOKILL1;
		//0 = opponent
	} //end type

			type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"The reptiloid kills this way. Have you no courage, no honor?";
		"Ha! Our shattered corpse leaves you no trophies, ", 0, "!";
		"You shame your tribe, your clan, and your family to strike in this way.";
		"Pfahhh! We will gain no honor destroying one such as you, ", 0, ".";
		0, ", we take pleasure knowing that you have joined us in destruction.";
		"You are of Sorg lineage, no doubt.";
		"The kamikaze is the coward's weapon.";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with rail gun
	{
		"It was necessary. We take no joy in what we have done.";
		"Know, oh ", fighter, "s, that we regret our recent action.";
		"Gladly would we use this device against the vile Sorg! May they all die in agony.";
		"Though we be judged for this action, we would do it again to gain victory!";
		"Fear us, for we are Death's avatar made flesh.";
		// 0 = enemy name
	} //end type

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Choke on our mist, ", 0, ".";
		"Our essence returns to the forest.";
		// 0 = eneour name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Catch us next time! Qur life has more worth than thine!";
		"Aiyeeeeeee!";
		"The fall is freeing, the ending ... sucks.";
		// 0 = eneour name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"It does not kill us ... it but hardens our skin.";
		"Our blood boils with anger, not pain.";
		"A fiery death intensifies our passion.";
		// 0 = eneour name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"A death more befitting a Sorg! Argh!";
		"A more worthy foe, at least, than those who slew our people.";
		"This is not how we envisioned our ending.";
		// 0 = eneour name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"It is so cold, like the blood of a reptiloid.";
		"Our road has led us astray.";
		"That we should die this way, there is no justice.";
		"The blood of our body imparts the venom in our heart. The water is now poison.";
		// 0 = eneour name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"We grow too hungry. We err.";
		"We have failed our quest.";
		"We are the last ... is this the end?";
		"This is the only honorable path when facing honorless opponents.";
		// 0 = eneour name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Deceiver! Dog! Sorg!";
		"We will tear the nails from the fingers that did this, ", 0, "!";
		"Do not touch us again, ", 0, ". Do you know who we are?";
		"Enjoy that touch, ", 0, ", few have found such pleasure.";
		// 0 = eneour name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Come out and fight like a woman, ", 0, "!";
		"You have no honor, ", 0, "! You wear the skin of a reptiloid.";
		"Where we come from, ", 0, ", we torture campers.";
		0, ", those who know they cannot honorably win, hide.";
		// 0 = eneour name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"The sudden end of a long road.";
		"It is as if the hatred within us materialized against us.";
		"Why is it suddenly all so... green?";
		// 0 = eneour name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"You do not deserve to wear our feathered crown, ", 0, "!";
		"We have come to expect such vile tactics from Sorg.";
		0, ", you are a newt! It won't get better.";
		"Vile ", fighter, "! Treachery is not conquest, ", 0, ".";
		"A starving wolf would spit out your rancid flesh, ", 0, ".";
		0, ", only a diseased mosquito could bring low the graceful deer.";
		"Come closer, ", 0, ", that we might spit upon you.";
		"You killing us is like the slime killing you, ", 0, ".";
		"We die content in knowing that your luck cannot last long, ", 0, ".";
		// 0 = eneour name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Take up our quest, ", 0, ", we beg. You will succeed.";
		"Well fought, ", 0, ". our clan could have allowed you to serve them.";
		0, ", you have just climbed onto the list of those we must kill.";
		"You took advantage of our distraction, ", 0, ".";
		"Clever, ", 0, " ... you surprise us.";
		0, ", you fight like a woman ... there is no higher praise.";
		"Impressive, perhaps ... to some.";
		"Improve, ", 0, " ... there is an absence of greatness in you now.";
		// 0 = eneour name
	} //end type

	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		0, ", we killed you only to sharpen our skills.";
		"We will use any and every weapon in our arsenal to rid the world of scum like you, ", 0, "!";
		// 0 = eneour name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"There is no bullet cheap enough for you, ", 0, ", so we used our hand.";
		"Cherish that mark, ", 0, ", it was delivered personally.";
		// 0 = eneour name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Out of our way, ", 0, "! We have more important prey!";
		"Our thanks, ", 0, ". We do so love a morning shower.";
		// 0 = eneour name
	} //end type

	type "kill_suicide" //initiated when the players kills self
	{
		"Were you scared to death, ", 0, "?";
		"Would that we could teach your style to the hordes of Sorg, ", 0, "!";
		"We expected as much from you, ", 0, ".";
		"Trained by vile Sorg mercenaries, no doubt.";
		"Clumsy Peasant.";
		// 0 = eneour name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Next, ", 0, ", we kill your dog.";
		"We frag you, ", 0, ", because there is no respect.";
		"You are fragged, ", 0, ". We move on.";
		"The world is now a better place, ", 0, ".";
		"Ashes to ashes, ", 0, ", and slime to slime.";
		"A pity ... we wished to hurt you more, ", 0, ".";
		"Hmmm. The pieces look better than the whole, ", 0, ".";
		"Your destruction improves the breed, ", 0, ".";
		"Our ", weapon, " seems to like you, ", 0, ".";
		"You seem to be faltering and failing, ", fighter, ".";
		"That was over far too quickly, ", 0, ".";
		"We ask for challenge and we are given this?!";
		"We take no joy in this. We prefer a challenge.";
		"Perhaps a career as a ", profession, " would be more appropriate?";
		// 0 = eneour name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"We almost considered enlisting you in our quest, ", 0, ". Almost.";
		"To die is to seek a higher level. You have far to go, ", 0, ".";
		"You are elusive, swift, and cunning, ", 0, ", but then, we are THE ~hunter.";
		"Perhaps you will get lucky and come back as a woman.";
		"Our applause, ", 0, "... that took longer than we expected.";
		"Let us hope your remains better serve the world, ", 0, ".";
		// 0 = eneour name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"Your mother died of sadness from seeing you, ", 0, ", did she not?";
		"If we meet again, ", 0, "... bathe. We beg of you.";
		"If we want any lip from you, ", 0, ", we will scrape it off our boot.";
		"Your odor betrays your position, ", 0, ".";
		"We doubt you are worth the effort or the ammo, ", 0, ".";
		"We kill you, ", 1, ", only so that other inferiors will remember their place.";
		"In the annals of the skalds, your defeat will be a fairy tale, ", 1, ".";
		MISC12;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"If Sorg had feathers, our headdress would befit a queen.";
		0, ", are those boots reptiloid? Very nice.";
		"Come to us o' pretty ~lightning gun.";
		"Sizzling flesh is our perfume.";
		"There is a peculiar design to ", 4, ". Reminds us of home.";
		"Females will learn their place. Men will not be pleased.";
		"We demand someone show us the 'test' in 'testosterone.'";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end hunter chat

