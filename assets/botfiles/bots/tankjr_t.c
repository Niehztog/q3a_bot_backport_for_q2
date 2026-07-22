//===========================================================================
//
// Name:			Tankjr
// Function:		chat lines for Tankjr
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Author:			The Seven Swords, R.A. Salvatore, & Paul Jaquays
// Editor:			Paul Jaquays
// Last update:		May 22, 2000
// Tab Size:		3 (real tabs)
//===========================================================================

chat "tankjr"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"Power up ... systems on-line. Pathetic carbon-based ", fighter, "s targeted.";
		"Selecting ", 1, " for initial termination.";
		"Sensors indicate ", 1, " as best choice for minimal risk 'frag.'";
		"Nothing but inferior lifeforms indicated in ", 4, ".";
		"Exterminate!";
		// 0 = bot name
		// 1 = random opponent
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Carbon life forms inadequate ... Executing search program.";
		"Initiating depature sequence.";
		"Exiting loop.";
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		LEVEL_START0;
		"Systems indicate multiple units for replacement parts.";
		"Systems indicate sniveling baby that's ready to die.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Damage assessment underway ... report: damage minimal ...";
		"Re-run previous combat matrix ... correct for maximum efficiency.";
		"Current data incompatible with world domination program. REBOOT.";
		"Read-outs indicate insufficient hierarchical position.";
		// 0 = bot name
		// 2 = opponent in first place
		// 3 = opponent in last place
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Multiple fatalities indicated. Activating mulcher/bagger systems.";
		"Situation read-outs indicate satisfactory program results.";
		"Ultimate victory achievable.";
		3, ", report to Arena Masters for disassembly.";
		// 0 = bot name
		// 3 = opponent in last place
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"PRINT: I will wind your entrails into pretzels, ", 2, ".";
		"'Luck' incompatible with logic. Analysis of ", 2, " inconclusive.";
		"Researching contextual defination of 'llama.";
		"Attempts to initiate nuclear self-destruct sequence failing.";
		"Biological units are to be exterminated in future encounters.";
		4," is yet to be conquered.";
		// 0 = bot name
		// 4 = Level's title
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Evisceration? Decapitation? ", 0, " may choose.";
		"PRINT: 'Ow ow! Help! Help! Help!.... ha ha ha ha.'";
		0, " will need to locate a bigger ", 1,".";
		"Data stream interrupted.";
		// 0 = shooter
		// 1 = weapon used by shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"DATA LOG: Systems functional. Return fire indicated.";
		"Damaged biological components require replacement. Collect from, ", 0, ".";
		"Inferior Unit! You will not survive to make that mistake again.";
		"Provide me with a target, ", 0, ". Tankjr challenges you.";
		"PROCESSING.";
		// 0 = shooter:
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"ANALYSIS: Inferior ordinance.";
		"Preparing target for processing.";
		"ACQUIRING TARGET";
		"DATA INQUIRY: How many hits to reach the juicy center?";
		"SENSOR READOUT: Biological infestation implanted in foe.";
		"Will your entrails spill out when I shake you up and pull your head off?";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"PLAYBACK RECORDING: '", fighter, "! Scumsucking re-wound ball of puke!'";
		"SENSOR READOUT: Uable to reconstruct data structure.";
		"REBOOT INITIATED";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"SENSOR READOUT: 'Terminal velocity reached in 1.3409 sec ... '";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"CRITICAL WARNING: 'System meltdown imminent.'";
		"Critical temperature exceeded ... systems shutting d ....";
		"DANGER! DANGER! Will Robi ....";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"SENSOR READOUT: 'Substance analysis incompatible with known lubricants.'";
		"SENSOR READOUT: 'Substance incompatible with biological components.'";
		"PLAYBACK RECORDING: 'I'm melting. I'm melting. What a world. What a world.'";
		//I didn't think I'd meet your family until after our first kiss."
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Braaaaaaaaappppppp! Zzzzpt!";
		"Undercoating failing. Terminal corrosion imminent.";
		"HUMOR: 'My male parent was a tank ... not a U-boat. *[laugh track]*'";
		"Hull compromised. Taking on water.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"WARNING: Targetting system malfunction.";
		"FATAL SYSTEM ERROR.";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"WARNING: 'Grenade in the hatch! Grenade in the hatch!'";
		"SENSOR FAILURE: 'Target range undetermined.'";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"PLAYBACK RECORDING:'You maggot-lapping piece of fly snot!'";
		DEATH_RAIL1;
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"PLAYBACK RECORDING: 'My exit wound is bigger than you, ", fighter, ".'";
		"PLAYBACK RECORDING: 'Ow. That really hurt.'";
		DEATH_BFG1;
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		//"C'mere so I can fall on you, squishy boy.";
		"Biological units cannot compete with Tankjr.";
		"You're lucky I was still hurt from yesterday's battles.";
		"I can be repaired. Can you, little squishy ", fighter, "?.";
		DEATH_INSULT4;
		DEATH_INSULT3;
		// 0 = enemy name
		// 1 = weapon used by enemy
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"SENSOR READOUT: ", 0, " is worthy to be assimilated.";
		"When I am repaired, ", 0, ", I will etch your name onto my hull.";
		D_PRAISE2;
		// 0 = enemy name
	} //end type

		type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"WARNING: 'Emergency escape systems indicating failure ... all systems down.'";
		"CALCULATING: 35% of body mass has reached escape velocity.";
		"PLAYBACK RECORDING: 'I'll be back.'";
		"PLAYBACK RECORDING: 'Yo Mama.'"
		"Enemy has resorted to weapons of desperation. Victory imminent.";
		"Daisy, daisy ....";
		"STATUS REPORT: 'Revising threat evaluation programming.'";
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with kamikaze
	{
		"SENSOR FEEDBACK: 'Opposing units neutralized.";
		"Exterminate! Exterminate!";
		"Death to biological units!";
		"Galactic Cyborg Domination now imminent.";
		"WARNING: 'Biological insurrection will not be tolerated.";
		// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"~Don't die just yet, ", 0, ", Tankjr must run over you a few times.";
		"STATUS: 'Terminal Penetration of Opponent acheived.'";
		"TARGET TERMINATED";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Taste Strogg steel, squishy, ", fighter, ".";
		"TARGET TERMINATED";
		KILL_GAUNTLET0;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"SENSOR EVALUATION: 'Target data stream fatally interrupted.'";
		"ACQUIRING TARGET: 'Scan Failure. Cannot locate.";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		"Opponent ", 0," succesful in activation of self-destruct sequence.";
		"Opponent self-terminated.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"The Strogg Empire will rise again.";
		"Tankjr has pistons bigger than ", 0, ".";
		"How do biological units live with the smell?";
		KILL_INSULT0;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"DATA SEARCH: 'Cannot find praise string. Search aborted.";
		"I will keep your organs as a memento of this match, ", 0, ".";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"I have been created to be your worst nightmare, ", fighter, ".";
		"Opposing life forms considered non-sentient.";
		"OPPONENT EVALUATION: 'Nonlethal/mostly harmless.'";
		"Tankjr will enjoy ripping you into components, bio-failure.";
		0, ", your skills suck.";
		"Surrender now. Cyborg steel will dominate.";
		"Your race has been targeted for elimination, ", 1, ".";
		"Our spawn will use your flesh as hosts.";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"I think I slipped a gear.";
		"SYSTEMS OVERHEATING: Must stop processing Angel images.";
		"All will be taken to the flesh vats.";
		"Daisy, daisy ... ";
		"Mother, is that you?";
		MISC12;
		MISC9;
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end chat


