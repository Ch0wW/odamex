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
//   Last Man Standing support
//
//-----------------------------------------------------------------------------

class Survival {

public:

	typedef enum
	{
		LMS_DISABLED,				// Mode is either disabled or not present
		LMS_WARMUP,					// LMS Warming up (aka not enough players)
		LMS_WARMUPCOUNTDOWN,		// Survival Countdown | LMS Warmup Countdown 

		LMS_ROUNDCOUNTDOWN,			// Round countdown (LMS only)
		LMS_ROUNDSTARTED,			// Survival : Game has started | LMS: Round started
		LMS_ROUNDOVER,				// Round is over
	} lms_states;

	Survival() : status(LMS_DISABLED) {}

	lms_states GetStatus();						// Get the current status
	void SendCountdown();						// Send the countdown to clients
	void SendRoundTimer();						// Send the current countdown

	void CheckPlayersAlive();					// Check if players are alive
	void Reset();								// Reset on map/vote
	void SetStatus(lms_states newstatus);		// Send the current status

private:

	void BroadcastNewStatus(player_t &player, lms_states newstatus, unsigned int timer = 0);
	lms_states status;
};

void SV_SendSurvivalStatus(player_t &player, Survival::lms_states newstatus, unsigned int timer = 0);
void SV_BroadcastSurvivalStatus(Survival::lms_states newstatus, unsigned int timer = 0);

extern Survival surv;
