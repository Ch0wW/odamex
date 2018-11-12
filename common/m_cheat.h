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


#ifndef __M_CHEAT_H__
#define __M_CHEAT_H__

class player_s;

//
// CHEAT SEQUENCE PACKAGE
//

enum ECheatCommand
{
	CHT_GOD,
	CHT_NOCLIP,
	CHT_NOTARGET,
	CHT_CHAINSAW,
	CHT_IDKFA,
	CHT_IDFA,
	CHT_BEHOLDV,
	CHT_BEHOLDS,
	CHT_BEHOLDI,
	CHT_BEHOLDR,
	CHT_BEHOLDA,
	CHT_BEHOLDL,
	CHT_IDDQD,				// Same as CHT_GOD but sets health
	CHT_MASSACRE,
	CHT_CHASECAM,
	CHT_FLY,
};

struct cheatseq_t
{
	unsigned char *Sequence;
	unsigned char *Pos;
	unsigned char DontCheck;
	unsigned char CurrentArg;
	unsigned char Args[2];
	bool(*Handler)(cheatseq_t *);
};


struct CheatInfos {
public:
	bool AreCheatsEnabled();

	bool AddKey(cheatseq_t *cheat, unsigned char key, bool *eat);

	void DoCheat(player_s *player, int cheat);
	void Give(player_s *player, const char *item);
};

extern CheatInfos cht;

#endif


