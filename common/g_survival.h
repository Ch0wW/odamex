// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2006-2018 by The Odamex Team.
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
//   Last Man Standing / Survival support
//
//-----------------------------------------------------------------------------

#ifndef __G_SURVIVAL_H__
#define __G_SURVIVAL_H__

class Survival {

public:

	typedef enum
	{
		LMS_DISABLED,
		LMS_WARMUP,					// LMS Warming up (aka not enough players)
		LMS_WARMUPCOUNTDOWN,		// Survival Countdown | LMS Warmup Countdown 

		LMS_ROUNDCOUNTDOWN,			// Round countdown (LMS only)
		LMS_ROUNDSTARTED,			// Survival : Game has started | LMS: Round started
		LMS_ROUNDOVER,				// Round is over
	} lms_states;

	Survival() : status(LMS_WARMUP), iTimer(0) {}

	void Ticker();								// Ticker

	void SetStatus(lms_states newstatus);		// Set the current status

	lms_states GetStatus();						// Get the current status
	short GetTimer();							// Get the current timer for the state

	bool CanFireWeapon();
	bool HasInternalTimer();					// Check if our gamemode uses an ingame-timer

	//===============
	// SERVER-ONLY FUNCTIONS
#ifdef SERVER_APP
	void Reset();								// Reset default status on map/vote
	void BroadcastNewStatus(lms_states newstatus, unsigned int timer);

	// -- GAME CONDITIONS --
	void CheckGameConditions();					// Check if players are alive
#endif

	//===============
	// CLIENT-ONLY FUNCTIONS
#ifdef CLIENT_APP
	void SetSeconds(short secs);
	short GetSeconds();	
#endif

//--------------------------------------------------------------------------------------------------------

private:
	lms_states status;		// Status of the game
	int	iTimer;				// Internal timer for warmup, round begin, round (if LMS), and round end.

#ifdef CLIENT_APP
	short iSeconds;			// Seconds left for the internal timer
#endif
};


//--------------------------------------------------------------------------------------------------------

#ifdef SERVER_APP
void SV_SendSurvivalStatus(player_t &player, Survival::lms_states newstatus, unsigned int timer);
#endif

#ifdef CLIENT_APP
void SURV_ParseEvent();
#endif 

extern Survival surv;

#endif