#include "doomdef.h"
#include "doomstat.h"
#include "c_cvars.h"

EXTERN_CVAR(sv_gametype)
EXTERN_CVAR(sv_maxplayers)

GIClass GAME;

GIClass::GIClass() {}

const char *GIClass::GetFullName() const
{
	if (sv_gametype == 0)
		return "COOPERATIVE";
	else if (sv_gametype == 1)
		return "DEATHMATCH";
	else if (sv_gametype == 1 && sv_maxplayers == 2)
		return "DUEL";
	else if (sv_gametype == 2)
		return "TEAM DEATHMATCH";
	else if (sv_gametype == 3)
		return "CAPTURE THE FLAG";
	else if (sv_gametype == 4)
		return "LAST MAN STANDING";
	else if (sv_gametype = 5)
		return "TEAM LAST MAN STANDING";
	/*else if (sv_gametype == 6)
		return "SURVIVAL";*/

	return "UNKNOWN";
}

const char *GIClass::GetShortName() const
{
	if (sv_gametype == 0)
		return "COOP";
	else if (sv_gametype == 1)
		return "DM";
	else if (sv_gametype == 1 && sv_maxplayers == 2)
		return "DUEL";
	else if (sv_gametype == 2)
		return "TDM";
	else if (sv_gametype == 3)
		return "CTF";
	else if (sv_gametype == 4)
		return "LMS";
	else if (sv_gametype = 5)
		return "TLMS";
	/*else if (sv_gametype == 6)
		return "SURV";*/

	return "UNKNOWN";
}

// Checks if the Current game is a Team-based Game.
// Ch
bool GIClass::IsTeamGame()
{
	return (sv_gametype == GM_CTF || sv_gametype == GM_TEAMDM || sv_gametype == GM_TEAMLMS);			// Do not forget to do the same with the same function below!
}

bool GIClass::IsTeamGame(byte gamebyte)
{
	return (gamebyte == GM_CTF || gamebyte == GM_TEAMDM || gamebyte == GM_TEAMLMS);				// Do not forget to do the same with the same function above!
}

bool GIClass::HasCountdown()
{
	return (sv_gametype == GM_LMS || sv_gametype == GM_TEAMLMS /*|| sv_gametype == GM_SURVIVAL*/);
}

bool GIClass::IsSinglePlayer()
{
	return (multiplayer == false);
}

bool GIClass::IsCooperation()
{
	return (sv_gametype == GM_COOP);
}

bool GIClass::IsDeathmatch()
{
	return (sv_gametype == GM_DM);
}

bool GIClass::IsDuel()
{
	return (sv_gametype == GM_DM && sv_maxplayers == 2);
}

bool GIClass::isFFA()
{
	return (sv_gametype == GM_DM && sv_maxplayers != 2);
}

bool GIClass::IsTeamDM()
{
	return (sv_gametype == GM_TEAMDM);
}

bool GIClass::IsCTF()
{
	return (sv_gametype == GM_CTF);
}

void GIClass::Set_Gamemode(int game)
{
	sv_gametype = game;
}