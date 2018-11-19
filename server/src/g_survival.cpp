#include "d_player.h"
#include "doomstat.h"

#include "c_gamemodes.h"
#include "c_cvars.h"

#include "g_survival.h"
#include "g_level.h"

CVAR_RANGE(		sv_survival_countdown, "0", "Countdown before a game on survival starts (1 to 30s)",
				CVARTYPE_BYTE, CVAR_SERVERARCHIVE | CVAR_SERVERINFO | CVAR_LATCH | CVAR_NOENABLEDISABLE,
				1.0f, 30.0f)

CVAR_RANGE(		sv_jointime, "0", "Maximum time before a client can't join back (30sec to 10 minutes)",
				CVARTYPE_BYTE, CVAR_SERVERARCHIVE | CVAR_SERVERINFO | CVAR_LATCH | CVAR_NOENABLEDISABLE,
				30.0f, 600.0f)

CVAR_RANGE(		sv_lms_pretime, "0", "Countdown before a round of LMS begins (1 to 10s)",
				CVARTYPE_BYTE, CVAR_SERVERARCHIVE | CVAR_SERVERINFO | CVAR_LATCH | CVAR_NOENABLEDISABLE,
				1.0f, 10.0f)

CVAR_RANGE(		sv_lms_roundtime, "0", "Time (in seconds) of a LMS round. (30 to 180s)",
				CVARTYPE_BYTE, CVAR_SERVERARCHIVE | CVAR_SERVERINFO | CVAR_LATCH | CVAR_NOENABLEDISABLE,
				30.0f, 180.0f)

int PostTime = 5 * TICRATE;	// PostRound time is fixed to 5 seconds

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

	if (newstatus == Survival::LMS_WARMUPCOUNTDOWN || newstatus == Survival::LMS_ROUNDCOUNTDOWN)
		MSG_WriteShort(&cl->reliablebuf, timer);
}

//
// SV_BroadcastSurvivalStatus
// Sends the new Survival Status to everyone.
//
void SV_BroadcastSurvivalStatus(Survival::lms_states newstatus, unsigned int timer = 0)
{
	for (Players::iterator it = players.begin(); it != players.end(); ++it)
	{
		if (!it->ingame())
			continue;
		SV_SendSurvivalStatus(*it, newstatus, timer);
	}
}

//
// SendCountdown
// Sends the current countdown state
//
void Survival::SetStatus(lms_states newstatus)
{
	status = newstatus;
}


void Survival::Reset()
{
	// Make sure our status is working
	if (GAME.IsSurvival() || GAME.IsLMS() || GAME.IsTeamLMS() )
		this->SetStatus(Survival::LMS_WARMUP);
	else
		this->SetStatus(Survival::LMS_DISABLED);

	// Reset all infos about the level and stuff
	level.round = 0;
	level.roundtimeleft = 0;

	/*this->overtime_count = 0;
	this->time_begin = 0;*/
}
