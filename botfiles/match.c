//===========================================================================
//
// Name:			match.c
// Function:		match templates
// Programmer:		Mr Elusive
// Last update:
// Tab Size:		4 (real tabs)
// Notes:			currently maximum of 8 match variables
//                  Adapted for Q2: team command sections removed (FFA only)
//===========================================================================

#include "match.h"

// this is rare but people can always fuckup
// don't use EC"(", EC")", EC"[", EC"]" or EC":" inside player names
// don't use EC": " inside map locations

//entered the game message
MTCONTEXT_MISC
{
	//enter game message
	NETNAME, " entered the game" = (MSG_ENTERGAME, 0);
} //end MTCONTEXT_MISC

MTCONTEXT_REPLYCHAT
{
	EC"(", NETNAME, EC")", PLACE, EC": ", MESSAGE = (MSG_CHATTEAM, ST_TEAM);
	EC"[", NETNAME, EC"]", PLACE, EC": ", MESSAGE = (MSG_CHATTELL, ST_TEAM);
	NETNAME, EC": ", MESSAGE = (MSG_CHATALL, 0);
} //end MTCONTEXT_REPLYCHAT
