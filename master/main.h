// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2000-2006 by Sergey Makovkin (CSDoom .62)
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
//	Master Server declarations
//
//-----------------------------------------------------------------------------

#ifndef __MAIN__
#define __MAIN__

using namespace std;

#if _MSC_VER == 1200
// MSVC6, disable broken warnings about truncated stl lines
#pragma warning(disable:4786)
#include <iostream>
#endif

#include <string>
#include <vector>
#include <list>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

#ifdef UNIX
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#endif

#ifdef _WIN32
#include <winsock.h>
#include <time.h>
#define usleep(n) Sleep(n/1000)
#endif

#include "i_net.h"

typedef struct server
{
	netadr_t addr;
	int age;

	// from server itself
	string hostname;
	int players, maxplayers;
	string map;
	vector<string> PWADs;
	int gametype, skill;
	vector<string> playernames;
	vector<int> playerfrags;
	vector<int> playerpings;
	vector<int> playerteams;

	unsigned int key_sent;
	bool pinged, verified;

	server() : age(0), players(0), maxplayers(0), gametype(0), skill(0), key_sent(0), pinged(0), verified(0) { memset(&addr, 0, sizeof(addr)); }

} SServer;

#endif