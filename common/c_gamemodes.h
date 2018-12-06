#ifndef __C_GAMEMODES_H__
#define __C_GAMEMODES_H__

//===================
// GameInfo Class
//===================
class GIClass
{
public:	
	GIClass();
	const char *GetFullName() const;
	const char *GetShortName() const;	

	bool IsSinglePlayer();
	bool IsCoop();
	bool IsDeathmatch();
	bool isFFA();
	bool IsDuel();
	bool IsTeamDM();
	bool IsCTF();
	bool IsSurvival();
	bool IsLMS();
	bool IsTeamLMS();
	bool IsTeamGame();
	bool IsTeamGame(byte gamebyte);

	bool HasCooperation();				// Ch0wW : Check if we're in a PvE gamemode
	bool HasWarmup();					// Ch0wW : if the gamemode can use warmups
	bool HasLives();					// Ch0wW: if the gamemode uses rounds (LMS/TLMS/Surv)


	void Set_Gamemode(int game);
};

extern GIClass GAME;

#endif