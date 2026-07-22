//===========================================================================
//
// Name:			Klesk_c.c
// Function:		chat lines for Klesk
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Script Author:	R.A. Salvatore & Seven Swords
// Editor:			Paul Jaquays
// Last update:		1999-09-24
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats
chat "klesk"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"We am to kill everyone in ", 4, ". This we like.";
		"We hunt. We kill. We leave. We happy.";
		"We see ", 1, "! Good sacrifice. Much joy.";
		"Klesk lord of ", 4, ". All die.";
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Bigger bone to break than skinny-neck you.";
		"We go chase others now. Head start you get.";
		4, " not good for Klesk. Klesk gone.";
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Breath of Klesk hot like fire wind. It burn you.";
		"~One mind many. All want dead you.";
		"No chance for single-brainers in ", 4, ".";
		"Creed says kill. You are my offerings.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		2, " must come into my mind and join.";
		2, " bone harder than ", 3, " bone. Next time both break.";
		"Minds must on more than ~one focus. Next time kill all.";
		"We need ascend. Find ~one within many.";
		"Too many inside Klesk head, choke Klesk skills.";
		// 0 = bot name
		// 2 = opponent in first place
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Chosen to lead. We am chosen.";
		"Blood of victims bring we power.";
		"Silence to psychic voices. Klesk like quiet now.";
		"Your mind screams revenge, ", 3, ", but only to die again.";
		"The road before us grow straight. We run fast to glory.";
		"Is happiness to see you gory.";
		// 3 = opponent in last place
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"No good. We must fulfill destiny.";
		"Clutter! Clutter all about. Head will explode.";
		"We go to find new way. We return.";
		"You not happy, ", 2, ". You soon dead.";
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Cheat is inferior!";
		"No place for ", 0, " in perfect Klesk world.";
		"You die hard. It hurt. We happy.";
		//0 = shooter
	} //end type

	type "damaged_nokill" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"It hurt! Hurt! Make pain! Be angry!";
		"All minds find focus hate on ,", 0, ".";
		"Now we shoot back. You still stand ", 0, "?";
		"Feed hunger, make focus. Not good day for ", 0, ".";
		"In pain, we learn.";
		0, "! You see?  You see? Still here.";
		"Hunt only ", 0, " now.";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Choose to kill you slow. Is happiness.";
		"Armor break. Bone break. Soft inside.";
		0, " fear smell good, ", 0,". You scream make music.";
		"All voices of ", 0, " speak fear and pain. ~One line they take. We hear.";
		"Too easy. " , 0, " is no challenge to grow.";
		"It is what we do. It is destiny. It is joy.";
		"Mind feed on ", 0, " silent scream.";
		"Too much that hurt, ", 0, ". We know. We like.";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Too many! Too many!";
		"Enter mind, not body!";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"They say we can. They wrong!";
		"Voices catch me ... ungh ... voices not enough.";
		"All see ground! Ground see we!";
		"Road end cliff. Klesk fall.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Abandon mind! Get out! Too hot!";
		"All voices scream to pain.";
		"Find new host for psyche. Must.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"It burn in body and creep in mind.";
		"No thought, just eat us. Consume.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"We birth new in here. New and better.";
		"Shoot We... it too slow. No happiness.";
		"Little bubbles not pretty. Run life away...";
		"Find peace. Find harmony. Find ...";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Confusion! Sort out...too late.";
		"Pain burn yellow. Burn red. Burn white. White pain!";
		"Some in We want this. We know.";
		"Not planned. No ascent."	;
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Touch of single-brainer burns many.";
		"Energy scatter. Discord! Die, die, dying.";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"No sense it. No happy.";
		0," ruins all world.";
		"No thoughts to sense. Just... ssssss... camper ...";
		"No good this destiny, ", 0, ". We change. You get eat.";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"Too many poppers!";
		"Each voice get boom-boom.";
		"Shrieking! Too many noise. Too many hurt!";
		"My pieces make altar. You need to pray.";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Now We know mystery you not know, ", 0, ".";
		"Forever all know ", 0, " kill Klesk. All hate ", 0, ".";
		"All is chaos! ", 0 ," win? No order to universe.";
		"Single-brainer ", 0, " too stupid to find joy. No deserve.";
		"We devolve to ", 0, " level. We fail.";
		"Colossal ridiculity is ", 0, ".";
		"We will find you, ", 0, " ... kill body, swallow soul, scatter thoughts. Nothing of ", 0, " remain.";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Peace now. We thank.";
		"You alpha ~one. ", 0, " make new race.";
		"Too easy for ", 0 ,"...", 0 ," is chosen.";
		"We come to your mind now, ", 0, ". We stronger.";
		"Now you learn secret. We be you.";
		"Is sad.";
		"Bigger voice, ", 0, ", than all in me together.";
		// 0 = enemy name
	} //end type

			type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"Sudden silence. All minds stop at once.";
		"All at peace now.";
		"Small mind. Big boom.";
		"Many mind voice cry vengeance. ~Klesk honor.";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with rail gun
	{
		"Powerful mind death. Silence all!";
		"The flesh of many shall feed the few.";
		"Fall trembling before insane skillz of ~Klesk.";
		"Worthy ~Klesk honors with sacrifice of many.";
		"Ahhhhhh ... quiet good.";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"We see you always, ", 0, ". Nowhere to hide.";
		"Poke little hole in ", 0, ". You spill out. Happy.";
		"Poor ", 0, ". So much left, but voice is quiet.";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Close ... close, ", 0, ". We smell you fear of die.";
		"We watch light leave eyes of ", 0, ". We like.";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Many voices ~one place. Many bodies not fit.";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		0, " accept inevitable. Good logic.";
		"We think ", 0, " good. We wrong.";
		"Still We like. Less to hear.";
		"Single-brainer ", 0, " wrong compute. Hee hee.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"My egglings in you hatch, ", 0, ".";
		"Much happiness me. I wish you back so kill you over again.";
		"To followers We feed you.";
		"~One little voice to silence. Not enough, but happy.";
		0, " mind weak, body weak. It follows. Too easy.";
		"Glide on ", 0, " spill guts. Dance of joy.";
		"Hollow skull, make ", 0 ," candle. Need focus.";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Worthy sacrifice ", 0, ". They pleased. Klesk stronger.";
		"Take pride, ", 0, ". Klesk outnumber you.";
		"Now ", 0, " find mystery and peace.";
		"More thoughts I gather to kill ", 0, ".";
		0, " find focus. Klesk need follow thought line.";
		0, " strong, but we strongest.";
		"We remember ", 0, " long time.";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"We hear you, ", 0 ,". Your mind cry out inferior.";
		"We am many, you only ~one, ", 0, ".";
		1, " voice run with spilling blood and gut. Is ecstacy.";
		"Better ", 1, " serve as nest for egglings.";
		"Only great survive. ", 0 ," easy die.";
		"You think stupid, ", 1, ". You weak! Klesk do favor killing you.";
		"There only black empty for ", 0, ". No revelation.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"Silence! All of you! Too much talk.";
		"Big boom-boom poppers make quiet faster. Need quiet!";
		"Ascend...climb...lie down, ", 0, " so ", 1 ," can stand on top you.";
		"The new psyche begins. Many voices as one. Join or be silence!";
		"We am never alone.";
		"Single-brainer know privacy. Envy...";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

} //end klesk chat
