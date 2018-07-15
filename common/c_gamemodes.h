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
	bool IsCooperation();
	bool IsDeathmatch();
	bool IsDuel();
	bool IsTeamDM();
	bool IsCTF();
	bool IsTeamGame();

	void Set_Gamemode(int game);
};

extern GIClass GAME;

#endif