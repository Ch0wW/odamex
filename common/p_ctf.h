// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2006-2015 by The Odamex Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	 CTF Implementation
//
//-----------------------------------------------------------------------------

#ifndef __P_CTF_H__
#define __P_CTF_H__

#include "d_netinf.h"
#include "p_local.h"

//	Map ID for flags
#define	ID_BLUE_FLAG	5130
#define	ID_RED_FLAG		5131
// Reserve for maintaining the DOOM CTF standard.
//#define ID_NEUTRAL_FLAG	5132
//#define ID_TEAM3_FLAG	5133
//#define ID_TEAM4_FLAG	5134

// events
enum flag_score_t
{
	SCORE_NONE,			// FULL REFRESH
	SCORE_REFRESH,		// Set the score
	SCORE_KILL,
	SCORE_BETRAYAL,
	SCORE_GRAB,
	SCORE_FIRSTGRAB,
	SCORE_CARRIERKILL,
	SCORE_RETURN,
	SCORE_CAPTURE,
	SCORE_DROP,
	SCORE_MANUALRETURN,

	NUM_CTF_SCORE
};

class CaptureTheFlag {

public:
	CaptureTheFlag(){};

	// Check the validity of the event.
	bool isEventValid(flag_score_t event) {
		return ((event <= SCORE_NONE || event >= NUM_CTF_SCORE) ? false : true);
	};

	void Ticker(void);
	void DropFlags(player_t &player);

// Set functions specific to clients ONLY.
#ifdef CLIENT_APP
	//void ParseEvent();
	void ParseFullUpdate(void);

	void CarryFlag(player_t &who, flag_t flag);
	void PlaySound(flag_t flag, flag_score_t event);
	void SetMessage(flag_t flag, flag_score_t event);

	void CaptureTheFlag::DrawHud(void);
#endif

// Set functions specific to server ONLY.
#ifdef SERVER_APP
	void SendEvent(flag_t f, flag_score_t event, player_t &who);			// Send the event to the player.
	void SendFullUpdate(player_t &player);									// Send the FLAG infos to the player

	// Events from the game
	void onFlagPickup(player_t &player, flag_t f, bool firstgrab);	// When the flag is picked up. 
	void onFlagReturn(player_t &player, flag_t f);					// When a flag has been returned.
	void onFlagCapture(player_t &player, flag_t f);					// When a flag has been captured.
#endif

	// Stubs clientside, not serverside
	bool onFlagTouch(player_t &player, flag_t f, bool firstgrab);
	void onSocketTouch(player_t &player, flag_t f);
	void SpawnFlag(flag_t f);
	void RememberFlagPos(mapthing2_t *mthing);

private:
	bool isOwnFlag(player_t &player, flag_t flag) {
		return player.userinfo.team == (flag_t)flag ? true : false;
	}

#ifdef CLIENT_APP
	void CaptureTheFlag::MoveFlags(void);
#endif

#ifdef SERVER_APP
	void SpawnDroppedFlag(flag_t f, int x, int y, int z);
	void onFlagDrop(player_t &player, flag_t f);					// When the flag is dropped.
#endif

};

// flags can only be in one of these states
enum flag_state_t
{
	flag_home,
	flag_dropped,
	flag_carried,

	NUMFLAGSTATES
};

// data associated with a flag
struct flagdata
{
	bool flaglocated;		// Does this flag have a spawn yet?

	AActor::AActorPtr actor;// Actor when being carried by a player, follows player
	
	byte flagger;			// Integer representation of WHO has each flag (player id)
	flag_state_t state;		// True when a flag has been dropped

	int x, y, z;			// Flag position

	int timeout;			// Flag Timout Counters
	long pickup_time;

	int sb_tick;			// Used for the blinking flag indicator on the statusbar
};

void CTF_ParseEvent();
// void CTF_SpawnPlayer(player_t &player);	// denis - todo - where's the implementation!?

//	Externals
// EXTERN_CVAR (sv_scorelimit)

// CTF Game Data
extern flagdata CTFdata[NUMFLAGS];
extern int TEAMpoints[NUMFLAGS];
extern const char *team_names[NUMTEAMS+2];

FArchive &operator<< (FArchive &arc, flagdata &flag);
FArchive &operator>> (FArchive &arc, flagdata &flag);

extern CaptureTheFlag CTF;

#endif

