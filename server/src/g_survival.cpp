//=================================================================
//
//
// SERVER ! SERVER ! SERVER ! SERVER ! SERVER ! SERVER ! SERVER !
//
//
//=================================================================

#include "d_player.h"
#include "doomstat.h"

#include "c_gamemodes.h"
#include "c_cvars.h"

#include "g_survival.h"
#include "g_level.h"

#include "sv_main.h"

extern size_t P_NumPlayersInGame();
extern int mapchange;

EXTERN_CVAR(sv_countdown)
EXTERN_CVAR(sv_maxlives)

CVAR_RANGE(	sv_jointime, "0", "Maximum time before a client can't join back (30sec to 10 minutes)",
			CVARTYPE_BYTE, CVAR_SERVERARCHIVE | CVAR_SERVERINFO | CVAR_LATCH | CVAR_NOENABLEDISABLE,
			30.0f, 600.0f)

CVAR_RANGE(	sv_lms_warmuptime, "10", "Warmup countdown for LMS modes (5 to 10s)",
			CVARTYPE_BYTE, CVAR_SERVERARCHIVE | CVAR_SERVERINFO | CVAR_LATCH | CVAR_NOENABLEDISABLE,
			5.0f, 10.0f)

CVAR_RANGE(	sv_lms_pretime, "0", "Countdown before a round of LMS begins (1 to 10s)",
			CVARTYPE_BYTE, CVAR_SERVERARCHIVE | CVAR_SERVERINFO | CVAR_LATCH | CVAR_NOENABLEDISABLE,
			1.0f, 10.0f)

CVAR_RANGE(	sv_lms_roundtime, "0", "Time (in seconds) of a LMS round. (30 to 180s)",
			CVARTYPE_BYTE, CVAR_SERVERARCHIVE | CVAR_SERVERINFO | CVAR_LATCH | CVAR_NOENABLEDISABLE,
			30.0f, 180.0f)

#define LMS_POSTROUND_WAIT 5	// PostRound time is fixed to 5 seconds

Survival surv;

//
// SV_SendSurvivalStatus
// Sends the new Survival Status to a specific client
//
void SV_SendSurvivalStatus(player_t &player, Survival::lms_states newstatus, unsigned int timer = 0)
{
	client_t* cl = &player.client;
	MSG_WriteMarker(&cl->reliablebuf, svc_survivalevent);
	MSG_WriteByte(&cl->reliablebuf, static_cast<byte>(newstatus));

	// Systematically send the timer linked to the new state if valid.
	MSG_WriteShort(&cl->reliablebuf, timer);
}


/*		LMS_DISABLED,
		LMS_WARMUP,					// LMS Warming up (aka not enough players)
		LMS_WARMUPCOUNTDOWN,		// Survival Countdown | LMS Warmup Countdown 

		LMS_ROUNDCOUNTDOWN,			// Round countdown (LMS only)
		LMS_ROUNDSTARTED,			// Survival : Game has started | LMS: Round started
		LMS_ROUNDOVER,				// Round is over
		*/
char *Survival::GetStringEvent(lms_states status)
{
	char *text;
	switch (status)
	{
	case LMS_DISABLED:
		text = "LMS_DISABLED"; break;
	case LMS_WARMUP:
		text = "LMS_WARMUP"; break;
	case LMS_WARMUPCOUNTDOWN:
		text = "LMS_WARMUPCOUNTDOWN"; break;
	case LMS_ROUNDCOUNTDOWN:
		text = "LMS_ROUNDCOUNTDOWN"; break;
	case LMS_ROUNDSTARTED:
		text = "LMS_ROUNDSTARTED"; break;
	case LMS_ROUNDOVER:
		text = "LMS_ROUNDOVER"; break;
	default:
		text = "UNKNOWN"; break;
	}
	return text;
}

//
// Survival::HasInternalTimer()
// Check if the state we currently are uses a timer to send to clients
//
bool Survival::HasInternalTimer()
{
	if (status == LMS_WARMUPCOUNTDOWN
		|| status == LMS_ROUNDCOUNTDOWN
		|| (status == LMS_ROUNDSTARTED && (GAME.IsLMS() || GAME.IsTeamLMS()) )
		|| status == LMS_ROUNDOVER)
		return true;

	return false;
}

//
// Survival::GetTimer()
// Get the current timer.
//
short Survival::GetTimer()
{
	return ceil((iTimer - level.time) / (float)TICRATE);
}

//
// Survival::Ticker
//
void Survival::Ticker()
{
	// Don't bother if not on Survival game.
	if (status == LMS_DISABLED)
		return;

	// Start the game if we have enough players, according to the gamemode.
	if (status == LMS_WARMUP 
		&& (  ((P_NumPlayersInGame()) >= 2 && (GAME.IsLMS() || GAME.IsTeamLMS())) 
			|| (P_NumPlayersInGame() >= 1 && GAME.IsSurvival()) ) )
		SetStatus(LMS_WARMUPCOUNTDOWN);

	// If there aren't any more active players, go back to warm up mode [tm512 2014/04/08]
	if (status > LMS_WARMUP && P_NumPlayersInGame() == 0)
		SetStatus(LMS_WARMUP);

	if (!HasInternalTimer())
		return;

	// If we haven't reached the level tic that we begin the map on,
	// we don't care.
	if (iTimer > level.time)
	{
		// Broadcast a countdown (this should be handled clientside)
		if ((iTimer - level.time) % TICRATE == 0) {
			BroadcastNewStatus(status, GetTimer());
		}
		return;
	}

	// The timer has been depleted.
	// Now, set our next state according to what the game gives us.
	// Ch0wW: THIS PART IS ENTIRELY BAD ! FIX IT ACCORDING TO THE NEXT STATE OF THE GAMEMODE
	// v
	//--------------------------------------
	if (GAME.IsLMS() || GAME.IsTeamLMS())
	{
		if (status == LMS_WARMUPCOUNTDOWN)
			SetStatus(LMS_ROUNDCOUNTDOWN);
		else if (status == LMS_ROUNDCOUNTDOWN)
			SetStatus(LMS_ROUNDSTARTED);
		else if (status == LMS_ROUNDOVER)
		{
			G_DeferedReset();				// Reset map, not stats
			SetStatus(LMS_ROUNDCOUNTDOWN);
		}
	}
	else if (GAME.IsSurvival())
	{
		if (status == LMS_WARMUPCOUNTDOWN)
		{
			G_DeferedFullReset();
			SetStatus(LMS_ROUNDSTARTED);
		}
		else if (status == LMS_ROUNDOVER)
		{
			G_DeferedFullResetWithSpectators();			// Full reset map + stats
			SetStatus(LMS_WARMUP);
		}
	}
		
	else
		SetStatus(LMS_DISABLED);

	// [SL] always reset the time (for now at least)
	level.time = 0;
	level.timeleft = 0;
	level.inttimeleft = mapchange / TICRATE;

	//--------------------------------------
	// ^
	// Ch0wW: THIS PART IS ENTIRELY BAD ! FIX IT ACCORDING TO THE NEXT STATE OF THE GAMEMODE
}

//
// SV_BroadcastSurvivalStatus
// Sends the new Survival Status to everyone.
//
void Survival::BroadcastNewStatus(lms_states newstatus, unsigned int timer = 0)
{
	for (Players::iterator it = players.begin(); it != players.end(); ++it)
	{
		if (!it->ingame())
			continue;

		SV_SendSurvivalStatus(*it, newstatus, timer);
	}
}

//
// SetStatus
// Sets a new status for the class.
//
void Survival::SetStatus(lms_states newstatus)
{
	status = newstatus;

	SV_BroadcastPrintf(PRINT_GAMEEVENT, "[LMS] THE CURRENT STATUS IS: %s\n", GetStringEvent(status));

	// Add the correct time according to the gamemode.
	if (this->HasInternalTimer())
	{
		short timingadd;

		switch (newstatus)
		{
			case LMS_WARMUPCOUNTDOWN:
				timingadd = sv_countdown;
				break;
			case LMS_ROUNDCOUNTDOWN:
				if (GAME.IsLMS() || GAME.IsTeamLMS())
					timingadd = sv_lms_pretime;
				break;
			case LMS_ROUNDSTARTED:
				if (GAME.IsLMS() || GAME.IsTeamLMS())
					timingadd = sv_lms_roundtime;
				break;
			case LMS_ROUNDOVER:
				timingadd = LMS_POSTROUND_WAIT;
		}
		iTimer = level.time + (timingadd * TICRATE);
		BroadcastNewStatus(newstatus, timingadd);
	}
	else
		BroadcastNewStatus(newstatus);

}

//
// GetStatus
//
Survival::lms_states Survival::GetStatus()
{
	return status;
}

//
// Survival::Reset
// Initializes the default status of the game.
//
void Survival::Reset()
{
	// Make sure our status is working
	if ( GAME.HasLives() )
		SetStatus(Survival::LMS_WARMUP);
	else
		SetStatus(Survival::LMS_DISABLED);

	// Reset all infos about the level and stuff
	level.round = 0;
	level.roundtimeleft = 0;
	iTimer = 0;
}

//
// Survival::CanFireWeapon
// Check if you're allowed to fire
//
bool Survival::CanFireWeapon()
{
	return (status == Survival::LMS_ROUNDCOUNTDOWN || status == Survival::LMS_WARMUPCOUNTDOWN || status == Survival::LMS_ROUNDOVER);
}

void Survival::Check_GameConditions()
{
	// Only allow those gamemodes.
	if (!GAME.HasLives())
		return;

	// Game hasn't been started, no win conditions yet ! 
	if (status == LMS_WARMUP)
		return;

	// Round has already been over, and checks done, so skip them !
	if (status == LMS_ROUNDOVER)
		return;

	// If Survival, 
	if (GAME.IsSurvival() && Check_GameEnd_Survival())
	{
		SetStatus(LMS_ROUNDOVER);
	}

	if (GAME.IsLMS())
	{

	}

	if (GAME.IsTeamLMS())
	{

	}
	// ToDo:
	/* if (GAME.IsSurvival())
	{
	if (!P_CheckAlivePlayers())
	return true;
	}
	else if (GAME.IsLMS())
	{
	// Check current alive players
	if (!P_CheckAlivePlayers() <= 1)
	return true;
	}
	else if (GAME.IsTeamLMS())
	{
	// Check both teams

	// If there are still 1 people on their team
	}
	*/

	// Round is still OK, so continue.
}

bool Survival::Check_GameEnd_Survival()
{
	// Check if players have 
	for (Players::const_iterator it = players.begin(); it != players.end(); ++it)
		if (it->ingame() && !it->spectator && it->lives)
			return false;

	return true;	// Game has no alive players anymore. boo!
}