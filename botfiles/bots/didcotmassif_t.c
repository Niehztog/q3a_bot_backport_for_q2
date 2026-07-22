//===========================================================================
//
// DIDCOTMASSIF bot by:
//	oony, Stepto, Pathos, Drunk & Dirty Irishman, and Spanky
//
// Special thanks to:
//	DIDCOTMASSIF for his vast wisdom
//	Old Man Murray (www.oldmanmurray.com) and the OMM forums
//	father Umberto for all his guidance
//	
//  
//
// Name:			didcotmassif_t.c
// Function:		chat lines for didcotmassif
// Programmer:		Jan Paul & J. Cash
// Scripter:		oony
// Code Janitor:	Xian
// Last update:		9/08/99
// Tab Size:		3 (real tabs)
//===========================================================================

chat "didcotmassif"
{
	#include "teamplay.h"
	//
	type "game_enter"
	{
		"WHAT UP BATTY?";
		"COAST TO COAST MASSIF SLICKER DAN MOST";
		"ANYBODY SEEN JC?";
		"BIG UP TO THE JUNTA COLLECTIVE";
	} //end type

	type "game_exit"
	{
		"ROCK THE MIKE (PSYCHE)";
		"KISS DE KITTEN GOOD SPORT";
	} //end type

	type "level_start"
	{
		"GO GO GO GIRLFRIEND";
		"RRRRRRRRRRRRRRRRRRRRRRRRUGGED STyLEE";
	} //end type

	type "end_level"
	{
		"WET BREAD ALL FOR ME ALLDAY";
	} //end type

	type "end_level_victory"
	{
		"MASSIF MASSIF MIGHTY";
		"ALL D BICS LUV FUZZBUZZ";
	} //end type

	type "level_end_lose"
	{
		"THESE PEOPLE DO NOT MAKE HOOLIGANISME";
	} //end type
	//======================================================
	//======================================================
	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"ITS JUST A LIKKLE RUBY TRACK";
		//0 = shooter
	} //end type
	type "damaged_nokill" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"DOSE ARE THE KINNA SOUN E MAEK";
		"ALLOVER MESEF";
		//0 = shooter
	} //end type
	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"OUT NOW PLASTIC CUTLERY";
		//0 = opponent
	} //end type
	type "enemy_suicide"
	{
		"I SOLVE IT AL WITH A BACKSLIT HAMSLIT";
		"NUVVER ONE BITE DE PILLOW";
		//0 = enemy
	} //end type
	//======================================================
	//======================================================

	//================================================================
	//================================================================

	type "death_telefrag"
	{
		"FALSE MASSIF BUSTIN TRUE";
	} //end type

	type "death_lava"
	{
		"GOT A MIGHTY MIGHTY STOKE IN DE FURNACE";
		"GOT A SAZZLE o GRiS-GRiS";
	} //end type

	type "death_slime"
	{
		"SCUM WORZEL BICS ALL ROUND SHOP LOIKE";
		"E WALKS ACROOS DE SWIMMIN POOL";
	} //end type0

	type "death_drown"
	{
		"FOOD OXYGen NEEDTOLIVE";
		"FIZZY WET REFRESHING COLONIALS MILLIONS OF EM";
	} //end type

	type "death_suicide"
	{
		"DIDCOTMASSIF NEVER TOUCH DE ZeR0";
	} //end type 

	type "death_gauntlet"
	{
		"TAKE NO YANQUIS DOLLAR!";
		"GOTTA HAVE SECOND HELPIN";
	} //end type

	type "death_rail"
	{
		"ME BE REALIN WID SHOCK";
	} //end type 

	type "death_bfg"
	{
		"DON'T TOUCH IT MIGHT SHATTER";
		"ALL SUDDEN LIKE";
	} //end type

	type "death_insult"
	{
		"spanky SMINKI PINKI";
		"BIG HOOHAH HA HA";
		"SAME ON YOU VINDALOO";
	} //end type

	type "death_praise"
	{
		"OOOOOOOOOH VINO VINO VINO VERITAS";
		"TELL YU WHAT HHHHHERES A PENGUIN";
	} //end type

	//================================================================
	//================================================================

	type "kill_rail" 
	{
		"RAILGUN: TIDY";
		"YOU HAVE SEEN THE MIGHTY TOWERS";
	} //end type

	type "kill_gauntlet"
	{
		"CHOP UP CHARLIE WID tASSCO CLUBCARD";
		"NO SHARP CRAYONS";
		"TUSKY BOYS TO MUCH FOR THE PILGRIM MUTHAS";
	} //end type

	type "kill_telefrag"
	{
		"YOU NOT TURNIN MILKY ON ME?";
	} //end type

	type "kill_insult"
	{
		"I GOT HAM N PICKLE YOU GOT OWT";
		"UNDER THE TABLE HAIRY HAND GRAAAAURGH";
		"DAnGER rUNAWAY";
		"FFFFEEEEL DA WHOLEY HOLY LOVIN FORCE RIDIN OVER SOMEONE ELSE";
	} //end type

	type "kill_praise"
	{
		"MASSIF MASSIF HOLDING HANDBAG UP TO CHIN";
		"SENTIMENT IN HEART OF DE MASSIF";
		"BEAUCOUP SHOUT TO CRUSTIES";
		"DIDCOT STATE OF MIND";
	} //end type

	//================================================================
	//================================================================

	type "random_insult"
	{
		"ME MORRIS MAN SHEKIN DE BLADDER AT THE BIG MAN";
		"DO NOT SPEAK ILL OF DEAD FALCO & DOGS";
		"HOO HELL HE HON DE HIGH HORSE?";
	} //end type

	type "random_misc"
	{
		"DIDCOTMASSIF have no parent DIDCOT heap big plenty nice shiney";
		"DIDCOT DAMMIN HIS OWN EDUcATION SENOR";
		"WALKIN ON GUiLDED SPLINTER GROUP";
		"MassiF CLIMBING MILLENIUM WHEEL FOR A BETTER VIEW OF EASTIES";
		"SILENCE WHILST ME PEEK";
	} //end type

} //end chat didcotmassif
