// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2006-2020 by The Odamex Team.
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
//	SV_COMMANDS - All commands sent from the server.
//
//-----------------------------------------------------------------------------

#include "doomtype.h"
#include "doomstat.h"
#include "gstrings.h"
#include "d_player.h"
#include "s_sound.h"
#include "gi.h"
#include "d_net.h"
#include "g_game.h"
#include "g_level.h"
#include "p_tick.h"
#include "p_local.h"
#include "p_inter.h"
#include "sv_main.h"
#include "sv_sqp.h"
#include "sv_sqpold.h"
#include "sv_master.h"
#include "i_system.h"
#include "c_console.h"
#include "c_dispatch.h"
#include "m_argv.h"
#include "m_random.h"
#include "m_vectors.h"
#include "p_ctf.h"
#include "w_wad.h"
#include "w_ident.h"
#include "md5.h"
#include "p_mobj.h"
#include "p_unlag.h"
#include "sv_vote.h"
#include "sv_maplist.h"
#include "g_warmup.h"
#include "sv_banlist.h"
#include "d_main.h"
#include "m_fileio.h"
#include "sv_main.h"

//--------------------------------------------------------------

/*
=============================
CHAT FUNCTIONS
=============================
*/

void SVCMD_PrintMessageToPlayer(client_t *cl, int level, std::string message)
{
	MSG_WriteMarker(&cl->reliablebuf, svc_print);
	MSG_WriteByte(&cl->reliablebuf, level);
	MSG_WriteString(&cl->reliablebuf, message.c_str());
}

/*
=============================
SCORE UPDATE FUNCTIONS
=============================
*/
void SVCMD_SetPlayerScores(client_t *cl, player_t &it)
{
	MSG_WriteMarker(&cl->reliablebuf, svc_updatefrags);
	MSG_WriteByte(&cl->reliablebuf, it.id);

	if (sv_gametype != GM_COOP)
		MSG_WriteShort(&cl->reliablebuf, it.fragcount);
	else
		MSG_WriteShort(&cl->reliablebuf, it.killcount);

	MSG_WriteShort(&cl->reliablebuf, it.deathcount);
	MSG_WriteShort(&cl->reliablebuf, it.points);
}

void SVCMD_UpdateScores(player_t &player)
{
	for (Players::iterator it = players.begin(); it != players.end(); ++it) {
		SVCMD_SetPlayerScores(&it->client, player);
	}
}

/*
=============================
READY STATUS FUNCTIONS
=============================
*/
void SVCMD_SetReadyStatus(client_t *cl, int playerid, bool newstatus)
{
	MSG_WriteMarker(&cl->reliablebuf, svc_readystate);
	MSG_WriteByte(&cl->reliablebuf, playerid);
	MSG_WriteBool(&cl->reliablebuf, newstatus);
}


void SVCMD_BroadcastReadyStatus(int playerid, bool newstatus)
{
	for (Players::iterator it = players.begin(); it != players.end(); ++it) {
		SVCMD_SetReadyStatus(&it->client, playerid, newstatus);
	}
}


/*
=============================
SPECTATOR STATUS FUNCTIONS
=============================
*/

void SVCMD_SetSpectatorStatus(client_t *cl, int playerid, bool newstatus)
{
	MSG_WriteMarker(&cl->reliablebuf, svc_spectate);
	MSG_WriteByte(&cl->reliablebuf, playerid);
	MSG_WriteBool(&cl->reliablebuf, newstatus);
}

void SVCMD_BroadcastSpectatorStatus(int playerid, bool newstatus)
{
	for (Players::iterator it = players.begin(); it != players.end(); ++it) {
		SVCMD_SetSpectatorStatus(&it->client, playerid, newstatus);
	}
}


#if 0		// ToDo : Ch0wW
/*
=============================
INTERMISSION STATUS FUNCTIONS
=============================
*/
void SVCMD_SetIntermissionReadyStatus(client_t *cl, int playerid, bool newstatus)
{
	MSG_WriteMarker(&cl->reliablebuf, svc_intreadystate);
	MSG_WriteByte(&cl->reliablebuf, playerid);
	MSG_WriteBool(&cl->reliablebuf, newstatus);
}

void SVCMD_BroadcastIntermissionReadyStatus(int playerid, bool newstatus)
{
	for (Players::iterator it = players.begin(); it != players.end(); ++it) {
		SVCMD_SetIntermissionReadyStatus(&it->client, playerid, newstatus);
	}
}
#endif


/*
=============================
PING FUNCTIONS
=============================
*/

void SVCMD_SendPingRequest(client_t* cl)
{
	if (!P_AtInterval((3 * TICRATE) - 5))	// 5 tic less
		return;

	MSG_WriteMarker(&cl->reliablebuf, svc_pingrequest);
	MSG_WriteLong(&cl->reliablebuf, I_MSTime());
}

void SVCMD_UpdatePlayersPing(client_t* cl)
{
	if (!P_AtInterval(3 * TICRATE))	// Update ping broadcasts every 3 seconds
		return;

	for (Players::iterator it = players.begin(); it != players.end(); ++it)
	{
		if (!(it->ingame()))
			continue;

		MSG_WriteMarker(&cl->reliablebuf, svc_updateping);
		MSG_WriteByte(&cl->reliablebuf, it->id);  // player
		MSG_WriteLong(&cl->reliablebuf, it->ping);
	}
}