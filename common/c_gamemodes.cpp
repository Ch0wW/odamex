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

	return "UNKNOWN";
}

bool GIClass::IsTeamGame()
{
	return (sv_gametype == GM_CTF || sv_gametype == GM_TEAMDM);
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