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
//	Cheat code checking.
//
//-----------------------------------------------------------------------------


#include <stdlib.h>
#include <math.h>


#include "m_cheat.h"
#include "d_player.h"
#include "doomstat.h"
#include "gstrings.h"
#include "p_inter.h"
#include "d_items.h"
#include "p_local.h"

#ifdef CLIENT_APP
#include "cl_main.h"
#endif

CheatInfos cht;

EXTERN_CVAR(sv_allowcheats)

extern bool simulated_connection;
extern void A_PainDie(AActor *);	// Required for CHT_MASSACRE

void STACK_ARGS SV_BroadcastPrintf(int level, const char *fmt, ...);
void STACK_ARGS SV_BroadcastPrintf(const char *fmt, ...);

//
// Key handler for cheats
//
bool CheatInfos::AddKey(cheatseq_t *cheat, byte key, bool *eat)
{
	if (cheat->Pos == NULL)
	{
		cheat->Pos = cheat->Sequence;
		cheat->CurrentArg = 0;
	}
	if (*cheat->Pos == 0)
	{
		*eat = true;
		cheat->Args[cheat->CurrentArg++] = key;
		cheat->Pos++;
	}
	else if (key == *cheat->Pos)
	{
		cheat->Pos++;
	}
	else
	{
		cheat->Pos = cheat->Sequence;
		cheat->CurrentArg = 0;
	}
	if (*cheat->Pos == 0xff)
	{
		cheat->Pos = cheat->Sequence;
		cheat->CurrentArg = 0;
		return true;
	}
	return false;
}

//
// Checks whether cheats can be used or not.
// And it's done correctly this time !
//
bool CheatInfos::AreCheatsEnabled(void)
{
	// [SL] 2012-04-04 - Don't allow cheat codes to be entered while playing
	// back a netdemo
	if (simulated_connection)
		return false;

	// [Russell] - Allow vanilla style "no message" in singleplayer when cheats
	// are disabled
	if (sv_skill == sk_nightmare && !multiplayer)
		return false;

	if ((multiplayer || !GAME.IsCooperation()) && !sv_allowcheats)
	{
		Printf(PRINT_WARNING, "You must run the server with '+set sv_allowcheats 1' to enable this command.\n");
		return false;
	}

	return true;
}

//
// Sets clientside the new cheat flag
// and also requests its new status serverside
//
bool SetGenericCheat(cheatseq_t *cheat)
{
	if (!cht.AreCheatsEnabled())
		return true;

	if (cheat->Args[0] == CHT_NOCLIP)
	{
		if (cheat->Args[1] == 0 && gamemode != shareware && gamemode != registered &&
			gamemode != retail && gamemode != retail_bfg)
			return true;
		else if (cheat->Args[1] == 1 && gamemode != commercial && gamemode != commercial_bfg)
			return true;
	}

	cht.DoCheat(&consoleplayer(), cheat->Args[0]);

#ifdef CLIENT_APP
	MSG_WriteMarker(&net_buffer, clc_cheat);
	MSG_WriteByte(&net_buffer, cheat->Args[0]);
#endif

	return true;
}

//
// [RH] Actually handle the cheat. The cheat code in st_stuff.c now just
// writes some bytes to the network data stream, and the network code
// later calls us.
//
// Now, it just applies the state, as this code can be both called from the client & the server
//
void CheatInfos::DoCheat (player_t *player, int cheat)
{
	const char *msg = "";
	char msgbuild[32];

	if ( player->health <= 0 || !player || player->spectator || player->playerstate == PST_DEAD )
		return;

	switch (cheat) {

		case CHT_FLY:
			player->cheats ^= CF_FLY;
			if (player->cheats & CF_FLY)
				msg = "You feel lighter";
			else
				msg = "Gravity weighs you down";
			break;

		// IDDQD also uses CHT_GOD, so don't break.
		case CHT_IDDQD:
			if (!(player->cheats & CF_GODMODE)) {
				if (player->mo)
					player->mo->health = deh.GodHealth;

				player->health = deh.GodHealth;
			}	
		case CHT_GOD:
			player->cheats ^= CF_GODMODE;
			if (player->cheats & CF_GODMODE)
				msg = GStrings(STSTR_DQDON);
			else
				msg = GStrings(STSTR_DQDOFF);
			break;

		case CHT_NOCLIP:
			player->cheats ^= CF_NOCLIP;
			if (player->cheats & CF_NOCLIP)
				msg = GStrings(STSTR_NCON);
			else
				msg = GStrings(STSTR_NCOFF);
			break;

		case CHT_NOTARGET:
				player->cheats ^= CF_NOTARGET;
				if (player->cheats & CF_NOTARGET)
					msg = "notarget ON";
				else
					msg = "notarget OFF";
			
			break;

		case CHT_CHAINSAW:
			player->weaponowned[wp_chainsaw] = true;
			player->powers[pw_invulnerability] = true;
			msg = GStrings(STSTR_CHOPPERS);
			break;

		case CHT_IDKFA:
			Give (player, "backpack");
			Give (player, "weapons");
			Give (player, "ammo");
			Give (player, "keys");
			player->armorpoints = deh.KFAArmor;
			player->armortype = deh.KFAAC;
			msg = GStrings(STSTR_KFAADDED);
			break;

		case CHT_IDFA:
			Give (player, "backpack");
			Give (player, "weapons");
			Give (player, "ammo");
			player->armorpoints = deh.FAArmor;
			player->armortype = deh.FAAC;
			msg = GStrings(STSTR_FAADDED);
			break;

		case CHT_BEHOLDV:
		case CHT_BEHOLDS:
		case CHT_BEHOLDI:
		case CHT_BEHOLDR:
		case CHT_BEHOLDA:
		case CHT_BEHOLDL:
			{
				if (player->health <= 0 || !player || player->spectator)
					return;

				int i = cheat - CHT_BEHOLDV;

				if (!player->powers[i])
					P_GivePower (player, i);
				else if (i!=pw_strength)
					player->powers[i] = 1;
				else
					player->powers[i] = 0;
			}
			msg = GStrings(STSTR_BEHOLDX);
			break;

		case CHT_MASSACRE:
			{
				// jff 02/01/98 'em' cheat - kill all monsters
				// partially taken from Chi's .46 port
				//
				// killough 2/7/98: cleaned up code and changed to use dprintf;
				// fixed lost soul bug (LSs left behind when PEs are killed)

				if (player->health <= 0 || !player || player->spectator)
					return;

				int killcount = 0;
				AActor *actor;
				TThinkerIterator<AActor> iterator;

				while ( (actor = iterator.Next ()) )
				{
					if (actor->flags & MF_COUNTKILL || actor->type == MT_SKULL)
					{
						// killough 3/6/98: kill even if PE is dead
						if (actor->health > 0)
						{
							killcount++;
							P_DamageMobj (actor, NULL, NULL, 10000, MOD_UNKNOWN);
						}
						if (actor->type == MT_PAIN)
						{
							A_PainDie (actor);    // killough 2/8/98
							P_SetMobjState (actor, S_PAIN_DIE6);
						}
					}
				}
				// killough 3/22/98: make more intelligent about plural
				// Ty 03/27/98 - string(s) *not* externalized
				sprintf (msgbuild, "%d Monster%s Killed", killcount, killcount==1 ? "" : "s");
				msg = msgbuild;
			}
			break;
	}
	if (serverside)
		SV_BroadcastPrintf (PRINT_GAMEEVENT, "%s activated a cheat: %s\n", player->userinfo.GetName(), msg);
	if ((clientside && player == &consoleplayer()) || GAME.IsSinglePlayer())
		Printf(PRINT_GAMEEVENT, "%s\n", msg);
}

//
// Gives the requested item (or group of items) to *player
//
void CheatInfos::Give (player_t *player, const char *name)
{
	BOOL giveall;
	int i;
	gitem_t *it;

	if (player->health <= 0 || !player || player->spectator)
		return;

	if (serverside)
		SV_BroadcastPrintf(PRINT_GAMEEVENT, "%s activated a cheat: give %s\n", player->userinfo.GetName(), name);

	if (stricmp (name, "all") == 0)
		giveall = true;
	else
		giveall = false;

	if (giveall || strnicmp (name, "health", 6) == 0) {
		int h;

		if (0 < (h = atoi (name + 6))) {
			if (player->mo) {
				player->mo->health += h;
	  			player->health = player->mo->health;
			} else {
				player->health += h;
			}
		} else {
			if (player->mo)
				player->mo->health = deh.GodHealth;
	  
			player->health = deh.GodHealth;
		}

		if (!giveall)
			return;
	}

	if (giveall || stricmp (name, "backpack") == 0) {
		if (!player->backpack) {
			for (i=0 ; i<NUMAMMO ; i++)
			player->maxammo[i] *= 2;
			player->backpack = true;
		}
		for (i=0 ; i<NUMAMMO ; i++)
			P_GiveAmmo (player, (ammotype_t)i, 1);

		if (!giveall)
			return;
	}

	if (giveall || stricmp (name, "weapons") == 0) {
		weapontype_t pendweap = player->pendingweapon;
		for (i = 0; i<NUMWEAPONS; i++)
			P_GiveWeapon (player, (weapontype_t)i, false);
		player->pendingweapon = pendweap;

		if (!giveall)
			return;
	}

	if (giveall || stricmp (name, "ammo") == 0) {
		for (i=0;i<NUMAMMO;i++)
			player->ammo[i] = player->maxammo[i];

		if (!giveall)
			return;
	}

	if (giveall || stricmp (name, "armor") == 0) {
		player->armorpoints = 200;
		player->armortype = 2;

		if (!giveall)
			return;
	}

	if (giveall || stricmp (name, "keys") == 0) {
		for (i=0;i<NUMCARDS;i++)
			player->cards[i] = true;

		if (!giveall)
			return;
	}

	if (giveall)
		return;

	it = FindItem (name);
	if (!it) {
		it = FindItemByClassname (name);
		if (!it) {
			if (player == &consoleplayer())
				Printf (PRINT_HIGH, "Unknown item\n");
			return;
		}
	}

	if (it->flags & IT_AMMO) {
		int howmuch;

	/*	if (argc == 3)
			howmuch = atoi (argv[2]);
		else */
			howmuch = it->quantity;

		P_GiveAmmo (player, (ammotype_t)it->offset, howmuch);
	} else if (it->flags & IT_WEAPON) {
		P_GiveWeapon (player, (weapontype_t)it->offset, 0);
	} else if (it->flags & IT_KEY) {
		P_GiveCard (player, (card_t)it->offset);
	} else if (it->flags & IT_POWERUP) {
		P_GivePower (player, it->offset);
	} else if (it->flags & IT_ARMOR) {
		P_GiveArmor (player, it->offset);
	}
}

VERSION_CONTROL (m_cheat_cpp, "$Id$")

