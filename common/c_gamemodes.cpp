#include "doomdef.h"
#include "doomstat.h"
#include "c_cvars.h"

EXTERN_CVAR(sv_gametype)
EXTERN_CVAR(sv_maxplayers)
EXTERN_CVAR(sv_survivalmode)
EXTERN_CVAR(sv_maxlives)

GIClass GAME;

GIClass::GIClass() {}

const char *GIClass::GetFullName() const
{
	if (GAME.IsCoop())
		return "COOPERATIVE";
	else if (GAME.IsSurvival())
		return "SURVIVAL";
	else if (GAME.IsDeathmatch())
		return "DEATHMATCH";
	else if (GAME.IsDuel())
		return "DUEL";
	else if (GAME.IsTeamDM())
		return "TEAM DEATHMATCH";
	else if (GAME.IsCTF())
		return "CAPTURE THE FLAG";
	else if (GAME.IsLMS())
		return "LAST MAN STANDING";
	else if (GAME.IsTeamLMS())
		return "TEAM LAST MAN STANDING";

	return "UNKNOWN";
}

const char *GIClass::GetShortName() const
{
	if (GAME.IsCoop())
		return "COOP";
	else if (GAME.IsSurvival())
		return "SURV";
	else if (GAME.IsDeathmatch())
		return "DM";
	else if (GAME.IsDuel())
		return "DUEL";
	else if (GAME.IsTeamDM())
		return "TDM";
	else if (GAME.IsCTF())
		return "CTF";
	else if (GAME.IsLMS())
		return "LMS";
	else if (GAME.IsTeamLMS())
		return "TLMS";

	return "UNKNOWN";
}

bool GIClass::IsSinglePlayer()
{
	return (multiplayer == false);
}

bool GIClass::HasCooperation()
{
	return (IsCoop() || IsSurvival());
}

bool GIClass::IsCoop()
{
	return (sv_gametype == GM_COOP && !sv_maxlives);
}

bool GIClass::IsDeathmatch()
{
	return (sv_gametype == GM_DM);
}

bool GIClass::IsTeamDM()
{
	return (sv_gametype == GM_TEAMDM);
}

bool GIClass::IsCTF()
{
	return (sv_gametype == GM_CTF);
}

bool GIClass::IsLMS()
{
	return (sv_gametype == GM_LMS);
}

bool GIClass::IsTeamLMS()
{
	return (sv_gametype == GM_TEAMLMS);
}

bool GIClass::IsSurvival()
{
	return (sv_gametype == GM_COOP && sv_maxlives);
}

/*==================================================
GAMEMODE CHECKS
==================================================
*/
bool GIClass::isFFA()
{
	return (this->IsDeathmatch() && sv_maxplayers != 2);
}

bool GIClass::IsDuel()
{
	return (this->IsDeathmatch() && sv_maxplayers == 2);
}

bool GIClass::IsTeamGame()
{
	return (this->IsTeamDM() || this->IsCTF() || this->IsTeamLMS());			// Do not forget to do the same with the same function below!
}

bool GIClass::IsTeamGame(byte gamebyte)
{
	return (gamebyte == GM_CTF || gamebyte == GM_TEAMDM || gamebyte == GM_TEAMLMS);				// Do not forget to do the same with the same function above!
}

bool GIClass::HasWarmup()
{
	return (this->IsDuel() || this->IsCTF() || this->IsTeamDM());
}

bool GIClass::HasLives()
{
	return (this->IsSurvival() || this->IsLMS() || this->IsTeamLMS());
}

void GIClass::Set_Gamemode(int game)
{
	sv_gametype = game;
}