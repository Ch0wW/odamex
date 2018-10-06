// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2000-2006 by Sergey Makovkin (CSDoom .62).
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
//		Functions and data used for 'Capture The Flag' mode.
//
// ----------------------------------------------------------------------------


#include "sv_main.h"
#include "m_random.h"
#include "p_ctf.h"
#include "i_system.h"
#include "g_warmup.h"
#include "p_unlag.h"

CaptureTheFlag CTF;

bool G_CheckSpot (player_t &player, mapthing2_t *mthing);

extern int shotclock;

EXTERN_CVAR (sv_teamsinplay)
EXTERN_CVAR (sv_scorelimit)
EXTERN_CVAR (ctf_manualreturn)
EXTERN_CVAR (ctf_flagathometoscore)
EXTERN_CVAR (ctf_flagtimeout)

flagdata CTFdata[NUMFLAGS];
int TEAMpoints[NUMFLAGS];

// denis - this is a lot clearer than doubly nested switches
static mobjtype_t flag_table[NUMFLAGS][NUMFLAGSTATES] =
{
	{MT_BFLG, MT_BDWN, MT_BCAR},
	{MT_RFLG, MT_RDWN, MT_RCAR}
};

const char *team_names[NUMTEAMS + 2] =
{
	"BLUE", "RED", "", ""
};

static int ctf_points[NUM_CTF_SCORE] =
{
	0, // NONE
	0, // REFRESH
	1, // KILL
	-1, // BETRAYAL
	1, // GRAB
	3, // FIRSTGRAB
	3, // CARRIERKILL
	2, // RETURN
	10, // CAPTURE
	0, // DROP
	1 // MANUALRETURN
};

//
//	[Toke - CTF] SV_FlagGrab
//	Event of a player picking up a flag
//
void CaptureTheFlag::onFlagPickup (player_t &player, flag_t f, bool firstgrab)
{
	player.flags[f] = true;
	CTFdata[f].flagger = player.id;
	CTFdata[f].state = flag_carried;
	CTFdata[f].pickup_time = I_MSTime();

	if (player.userinfo.team != (team_t)f) {
		if (firstgrab) {
			SV_BroadcastPrintf (PRINT_GAMEEVENT, "%s has taken the %s flag\n", player.userinfo.GetName(), team_names[f]);
			this->SendEvent(f, SCORE_FIRSTGRAB, player);
		} else {
			SV_BroadcastPrintf (PRINT_GAMEEVENT, "%s picked up the %s flag\n", player.userinfo.GetName(), team_names[f]);
			this->SendEvent(f, SCORE_GRAB, player);
		}
	} else {
		SV_BroadcastPrintf (PRINT_GAMEEVENT, "%s is recovering the %s flag\n", player.userinfo.GetName(), team_names[f]);
		this->SendEvent(f, SCORE_MANUALRETURN, player);
	}
}

//
//	[Toke - CTF] SV_FlagReturn
//	Returns the flag to its socket
//
void CaptureTheFlag::onFlagReturn(player_t &player, flag_t f)
{
	this->SendEvent(f, SCORE_RETURN, player);

	CTF.SpawnFlag (f);

	SV_BroadcastPrintf (PRINT_GAMEEVENT, "%s has returned the %s flag\n", player.userinfo.GetName(), team_names[f]);
}

//
// CTF_TimeMSG
// denis - tells players how long someone held onto a flag
//
static const char *CTF_TimeMSG(unsigned int milliseconds)
{
	static char msg[64];

	milliseconds /= 10;
	int ms = milliseconds%100;
	milliseconds -= ms;
	milliseconds /= 100;
	int se = milliseconds%60;
	milliseconds -= se;
	milliseconds /= 60;
	int mi = milliseconds;

	sprintf((char *)&msg, "%d:%.2d.%.2d", mi, se, ms);

	return msg;
}

extern void P_GiveTeamPoints(player_t* player, int num);

//
//	[Toke - CTF] SV_FlagScore
//	Event of a player capturing the flag
//
void CaptureTheFlag::onFlagCapture (player_t &player, flag_t f)
{
	P_GiveTeamPoints(&player, 1);

	this->SendEvent (f, SCORE_CAPTURE, player);

	int time_held = I_MSTime() - CTFdata[f].pickup_time;

	SV_BroadcastPrintf (PRINT_GAMEEVENT, "%s has captured the %s flag (held for %s)\n", player.userinfo.GetName(), team_names[f], CTF_TimeMSG(time_held));

	player.flags[f] = false; // take scoring player's flag
	CTFdata[f].flagger = 0;

	CTF.SpawnFlag(f);

	// checks to see if a team won a game
	if( (sv_scorelimit > 0) && TEAMpoints[player.userinfo.team] >= sv_scorelimit)
	{
		SV_BroadcastPrintf (PRINT_GAMEEVENT, "Score limit reached. %s team wins!\n", team_names[player.userinfo.team]);
		shotclock = TICRATE*2;
	}
}

//
// SV_FlagTouch
// Event of a player touching a flag, called from P_TouchSpecial
//
bool CaptureTheFlag::onFlagTouch (player_t &player, flag_t f, bool firstgrab)
{
	if (shotclock)
		return false;

	if (CTF.isOwnFlag(player, f))
	{
		if(CTFdata[f].state == flag_home) {
			return false;	// Do nothing.
		}
		else // Returning team flag.
		{
			if (ctf_manualreturn)
				CTF.onFlagPickup(player, f, firstgrab);
			else
				CTF.onFlagReturn(player, f);
		}
	}
	else 
	{
		CTF.onFlagPickup(player, f, firstgrab);	// Run the enemy flag pickup event.
	}

	// returning true should make P_TouchSpecial destroy the touched flag
	return true;
}

//
// SV_SocketTouch
// Event of a player touching a socket, called from P_TouchSpecial
//
void CaptureTheFlag::onSocketTouch(player_t &player, flag_t f)
{
	if (shotclock)
		return;

	// Returning team's flag manually.
	if (player.userinfo.team == (team_t)f && player.flags[f]) {
		player.flags[f] = false;
		CTFdata[f].flagger = 0;
		CTF.onFlagReturn(player, f);
	}

	// Scoring with enemy flag.
	for(size_t i = 0; i < NUMFLAGS; i++)
		if(player.userinfo.team == (team_t)f && player.userinfo.team != (team_t)i &&
			player.flags[i] && (!ctf_flagathometoscore || CTFdata[f].state == flag_home))
			CTF.onFlagCapture(player, (flag_t)i);
}

//
//	[Toke - CTF] SV_FlagDrop
//	Event of a player dropping a flag
//
void CaptureTheFlag::onFlagDrop (player_t &player, flag_t f)
{
	if (shotclock)
		return;

	this->SendEvent (f, SCORE_DROP, player);

	int time_held = I_MSTime() - CTFdata[f].pickup_time;

	SV_BroadcastPrintf (PRINT_GAMEEVENT, "%s has dropped the %s flag (held for %s)\n", player.userinfo.GetName(), team_names[f], CTF_TimeMSG(time_held));

	player.flags[f] = false; // take ex-carrier's flag
	CTFdata[f].flagger = 0;

	fixed_t x, y, z;
	Unlag::getInstance().getCurrentPlayerPosition(player.id, x, y, z);
	this->SpawnDroppedFlag(f, x, y, z);
}

//
//	[Toke - CTF] Ticker
//	Runs once per gametic when ctf is enabled
//
void CaptureTheFlag::Ticker (void)
{
	if (shotclock || gamestate != GS_LEVEL)
		return;

	for(size_t i = 0; i < NUMFLAGS; i++)
	{
		flagdata *data = &CTFdata[i];

		if(data->state != flag_dropped)
			continue;

		if (!ctf_flagtimeout)
			continue;

		if(data->timeout--)
			continue;

		if(data->actor)
			data->actor->Destroy();

		this->SendEvent ((flag_t)i, SCORE_RETURN, idplayer(0));

		SV_BroadcastPrintf (PRINT_GAMEEVENT, "%s flag returned.\n", team_names[i]);

		this->SpawnFlag((flag_t)i);
	}
}

//
//	[Toke - CTF] SpawnFlag
//	Spawns a flag of the designated type in the designated location
//
void CaptureTheFlag::SpawnFlag (flag_t f)
{
	flagdata *data = &CTFdata[f];

	AActor *flag = new AActor (flag_table[f][flag_home], data->x, data->y, data->z);

	SV_SpawnMobj(flag);

	data->actor = flag->ptr();
	data->state = flag_home;
	data->flagger = 0;
}

//
// CTF_SpawnDroppedFlag
// Spawns a dropped flag
//
void CaptureTheFlag::SpawnDroppedFlag (flag_t f, int x, int y, int z)
{
	flagdata *data = &CTFdata[f];

	AActor *flag = new AActor (flag_table[f][flag_dropped], x, y, z);

	SV_SpawnMobj(flag);

	data->actor = flag->ptr();
	data->state = flag_dropped;
	data->timeout = (size_t)(ctf_flagtimeout * TICRATE);
	data->flagger = 0;
}

//
//	[Toke - CTF] CTF_CheckFlags
//	Checks to see if player has any flags in his inventory, if so it spawns that flag on players location
//
void CaptureTheFlag::DropFlags(player_t &player)
{
	for(size_t i = 0; i < NUMFLAGS; i++)
		if(player.flags[i])
			CTF.onFlagDrop (player, (flag_t)i);
}

//
//	[Toke - CTF] CTF_RememberFlagPos
//	Remembers the position of flag sockets
//
void CaptureTheFlag::RememberFlagPos (mapthing2_t *mthing)
{
	flag_t f;

	switch(mthing->type)
	{
		case ID_BLUE_FLAG: f = it_blueflag; break;
		case ID_RED_FLAG: f = it_redflag; break;
		default:
			return;
	}

	flagdata *data = &CTFdata[f];

	data->x = mthing->x << FRACBITS;
	data->y = mthing->y << FRACBITS;
	data->z = mthing->z << FRACBITS;

	data->flaglocated = true;
}

FArchive &operator<< (FArchive &arc, flagdata &flag)
{
	return arc;
}

FArchive &operator>> (FArchive &arc, flagdata &flag)
{
	return arc;
}

//
//	[Toke - CTF] SV_CTFEvent
//	Sends CTF events to player
//
void CaptureTheFlag::SendEvent(flag_t f, flag_score_t event, player_t &who)
{
	if (!this->isEventValid(event)) {
		Printf("[CTF] Invalid Event detected! (%d)\n", event);
		return;
	}

	if (validplayer(who) && warmup.checkscorechange())
		who.points += ctf_points[event];

	for (Players::iterator it = players.begin(); it != players.end(); ++it)
	{
		client_t *cl = &(it->client);

		MSG_WriteMarker(&cl->reliablebuf, svc_ctfevent);
		MSG_WriteByte(&cl->reliablebuf, event);
		MSG_WriteByte(&cl->reliablebuf, f);

		if (validplayer(who))
		{
			MSG_WriteByte(&cl->reliablebuf, who.id);
			MSG_WriteLong(&cl->reliablebuf, who.points);
		}
		else
		{
			MSG_WriteByte(&cl->reliablebuf, 0);
			MSG_WriteLong(&cl->reliablebuf, 0);
		}

		for (size_t j = 0; j < NUMFLAGS; j++)
			MSG_WriteLong(&cl->reliablebuf, TEAMpoints[j]);
	}
}

//
// SendFullUpdate
// Send states of all flags to new player
//
void CaptureTheFlag::SendFullUpdate(player_t &player)
{
	client_t *cl = &player.client;

	MSG_WriteMarker(&cl->reliablebuf, svc_ctfevent);
	MSG_WriteByte(&cl->reliablebuf, SCORE_NONE);

	for (size_t i = 0; i < NUMFLAGS; i++)
	{
		MSG_WriteByte(&cl->reliablebuf, CTFdata[i].state);
		MSG_WriteByte(&cl->reliablebuf, CTFdata[i].flagger);
	}

	// Send then the team scores to the client to end our full refresh
	this->SendEvent((flag_t)0, SCORE_REFRESH, player);
}

VERSION_CONTROL (sv_ctf_cpp, "$Id$")

