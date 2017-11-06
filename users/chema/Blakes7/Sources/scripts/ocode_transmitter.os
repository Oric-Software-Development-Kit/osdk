/****************************/
/* Blake's 7: the Oric game */
/* Using OASIS              */
/* (c) Chema - 2016         */
/****************************/

#include "globals.h"

/* Object command script and strings */

objectcode TRANSMITTER{
	LookAt:
		scActorTalk(sfGetEgo(),TRANSMITTER,0);
}


stringpack TRANSMITTER{
#ifdef ENGLISH
	/**************************************/
	"A transmitter to operate the relay.";
#endif

#ifdef SPANISH
	/***************************************/
	"Un transmisor para operar el relé.";
#endif

}

