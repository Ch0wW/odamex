//=================================================================
//
//
// CLIENT ! CLIENT ! CLIENT ! CLIENT ! CLIENT ! CLIENT ! CLIENT !
//
//
//=================================================================

#include <string>
#include <sstream>

#include "c_console.h"
#include "cl_main.h"
#include "d_player.h"
#include "doomstat.h"

#include "c_gamemodes.h"
#include "c_cvars.h"

#include "g_survival.h"
#include "g_level.h"

#include "hudmessages.h"

Survival surv;

void SURV_ParseEvent()
{
	// Get the status
	surv.SetStatus(static_cast<Survival::lms_states>(MSG_ReadByte()));
	
	// Also get the seconds
	short secs = MSG_ReadShort();
	surv.SetSeconds(secs);

	// Now adapt the client to what the server sends us.
	Survival::lms_states status = surv.GetStatus();
	std::ostringstream buffer;

	if (status == Survival::LMS_WARMUPCOUNTDOWN)
	{
		buffer << "Match begins in " << surv.GetSeconds() << "...";
	}
	else if (status == Survival::LMS_ROUNDCOUNTDOWN)
	{
		buffer << "Round begins in " << surv.GetSeconds() << "...";
	}
	else if (status == Survival::LMS_ROUNDSTARTED)
	{
		if (GAME.IsLMS() || GAME.IsTeamLMS())
			buffer << "FIGHT !";
		else
			buffer << "";
	}
	C_GMidPrint(buffer.str().c_str(), CR_GREEN, 0);
}

//
// SetStatus
// Clientside representation of this function.
//
void Survival::SetStatus(lms_states newstatus)
{
	status = newstatus;
}

//
// GetStatus
//
Survival::lms_states Survival::GetStatus()
{
	return status;
}

//
// SetSeconds
// Clientside representation of this function.
//
void Survival::SetSeconds(short Timer)
{
	iSeconds = Timer;
}

//
// GetSeconds
//
short Survival::GetSeconds()
{
	return iSeconds;
}

//
// Survival::CanFireWeapon
// Check if you're allowed to fire your weapon.
//
bool Survival::CanFireWeapon() 
{
	return (status == Survival::LMS_ROUNDCOUNTDOWN || status == Survival::LMS_WARMUPCOUNTDOWN || status == Survival::LMS_ROUNDOVER);
}
