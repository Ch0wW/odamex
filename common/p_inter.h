// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
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
//	The Give commands (?)
//
//-----------------------------------------------------------------------------


#ifndef __P_INTER__
#define __P_INTER__

#define BONUSADD 6

class player_s;

BOOL P_GiveAmmo(player_s*, ammotype_t, int);
BOOL P_GiveWeapon(player_s*, weapontype_t, BOOL);
BOOL P_GiveArmor(player_s*, int);
void P_GiveCard(player_s*, card_t);
BOOL P_GivePower(player_s*, int);
void P_KillMobj (AActor *source, AActor *target, AActor *inflictor, bool joinkill);

//===============
// Sprees Info
//===============

// Frags needed in one life to be seen as on a spree.
#define SPREE_COUNTLEVEL 5	

typedef enum
{
	SPREE_NONE,

	SPREE_FRAGVALID,
	SPREE_DEATH,
	SPREE_TEAMKILL,
	SPREE_SUICIDE,

	SPREE_MAX_EVENTS
} spree_event_t;


//===============
// FragCombo Info
//===============

// Maximum Time allowed between 2 frags to start/continue combos.
// We choose 4 seconds, as Zdaemon uses 5 seconds, while Zandronum uses 3 seconds.
#define COMBO_ALLOWED_SECONDS (TICRATE * 4)

#endif


