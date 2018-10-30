// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// sc_man.c : Heretic 2 : Raven Software, Corp.
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
//
//		General lump script parser from Hexen (MAPINFO, etc)
//
//-----------------------------------------------------------------------------


// HEADER FILES ------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>

#include "doomtype.h"
#include "i_system.h"
#include "sc_man.h"
#include "w_wad.h"
#include "z_zone.h"
#include "cmdlib.h"
#include "m_fileio.h"


FScriptParser sc;

// MACROS ------------------------------------------------------------------

#define MAX_STRING_SIZE 4096
#define ASCII_COMMENT (';')
#define CPP_COMMENT ('/')
#define C_COMMENT ('*')
#define ASCII_QUOTE (34)
#define LUMP_SCRIPT 1
#define FILE_ZONE_SCRIPT 2

// CODE --------------------------------------------------------------------


// Constructor
FScriptParser::FScriptParser()
{
	ScriptOpen = false;
}

// Destructor
FScriptParser::~FScriptParser()
{
}

//
// SC_Open
//
void FScriptParser::Open (const char *lumpname)
{
	this->OpenLumpNum (W_GetNumForName (lumpname), lumpname);
}


//
// SC_OpenFile
//
// Loads a script (from a file). Uses the zone memory allocator for
// memory allocation and de-allocation.
//
void FScriptParser::OpenFile (const char *filename)
{
	this->Close ();
	ScriptSize = M_ReadFile (filename, (byte **)&this->ScriptBuffer);
	M_ExtractFileBase (filename, this->ScriptName);
	FreeScript = true;
	this->PrepareScript ();
}


//
// SC_OpenMem
//
// Prepares a script that is already in memory for parsing. The caller is
// responsible for freeing it, if needed.
//
void FScriptParser::OpenMem (const char *name, char *buffer, int size)
{
	this->Close ();
	ScriptSize = size;
	this->ScriptBuffer = buffer;
	this->ScriptName = name;
	FreeScript = false;
	this->PrepareScript ();
}


//
// SC_OpenLumpNum
//
// Loads a script (from the WAD files).
//
void FScriptParser::OpenLumpNum (int lump, const char *name)
{
	this->Close ();
	this->ScriptBuffer = (char *)W_CacheLumpNum (lump, PU_STATIC);
	ScriptSize = W_LumpLength (lump);
	this->ScriptName = name;
	FreeScript = true;
	this->PrepareScript ();
}

//
// CheckOpen
//
void FScriptParser::CheckOpen(void)
{
	if (ScriptOpen == false) {
		I_FatalError("SC_ call before SC_Open().");
	}
}

//
// SC_PrepareScript
//
// Prepares a script for parsing.
//
void FScriptParser::PrepareScript (void)
{
	this->ScriptPtr = this->ScriptBuffer;
	ScriptEndPtr = this->ScriptPtr + ScriptSize;
	this->Line = 1;
	this->End = false;
	ScriptOpen = true;
	this->String = StringBuffer;
	AlreadyGot = false;
	SavedScriptPtr = NULL;
}


//
// SC_Close
//
void FScriptParser::Close (void)
{
	if (ScriptOpen)
	{
		if (FreeScript && this->ScriptBuffer)
			Z_Free (this->ScriptBuffer);
		this->ScriptBuffer = NULL;
		ScriptOpen = false;
	}
}


//
// SC_SavePos
//
// Saves the current script location for restoration later
//
void FScriptParser::SavePos (void)
{
	this->CheckOpen ();
	if (this->End)
	{
		SavedScriptPtr = NULL;
	}
	else
	{
		SavedScriptPtr = this->ScriptPtr;
		SavedScriptLine = this->Line;
	}
}


//
// SC_RestorePos
//
// Restores the previously saved script location
//
void FScriptParser::RestorePos (void)
{
	if (SavedScriptPtr)
	{
		this->ScriptPtr = SavedScriptPtr;
		this->Line = SavedScriptLine;
		this->End = false;
		AlreadyGot = false;
	}
}


//
// SC_GetString
//
bool FScriptParser::GetString (void)
{
	char *text;
	BOOL foundToken;

	this->CheckOpen();
	if (AlreadyGot)
	{
		AlreadyGot = false;
		return true;
	}
	foundToken = false;
	this->Crossed = false;
	if (this->ScriptPtr >= ScriptEndPtr)
	{
		this->End = true;
		return false;
	}
	while (foundToken == false)
	{
		while (*this->ScriptPtr <= 32)
		{
			if (this->ScriptPtr >= ScriptEndPtr)
			{
				this->End = true;
				return false;
			}
			if (*ScriptPtr++ == '\n')
			{
				this->Line++;
				this->Crossed = true;
			}
			if (ScriptPtr >= ScriptEndPtr)
			{
				this->End = true;
				return false;
			}
		}
		if (ScriptPtr >= ScriptEndPtr)
		{
			this->End = true;
			return false;
		}
		if (*ScriptPtr != ASCII_COMMENT &&
			!(ScriptPtr[0] == CPP_COMMENT && ScriptPtr < ScriptEndPtr - 1 &&
			  (ScriptPtr[1] == CPP_COMMENT || ScriptPtr[1] == C_COMMENT)))
		{ // Found a token
			foundToken = true;
		}
		else
		{ // Skip comment
			if (ScriptPtr[0] == CPP_COMMENT && ScriptPtr[1] == C_COMMENT)
			{	// C comment
				while (ScriptPtr[0] != C_COMMENT || ScriptPtr[1] != CPP_COMMENT)
				{
					if (ScriptPtr[0] == '\n')
					{
						this->Line++;
						this->Crossed = true;
					}
					ScriptPtr++;
					if (ScriptPtr >= ScriptEndPtr - 1)
					{
						this->End = true;
						return false;
					}
				}
				ScriptPtr += 2;
			}
			else
			{	// C++ comment
				while (*ScriptPtr++ != '\n')
				{
					if (ScriptPtr >= ScriptEndPtr)
					{
						this->End = true;
						return false;
					}
				}
				this->Line++;
				this->Crossed = true;
			}
		}
	}
	text = this->String;
	if (*ScriptPtr == ASCII_QUOTE)
	{ // Quoted string
		ScriptPtr++;
		while (*ScriptPtr != ASCII_QUOTE)
		{
			*text++ = *ScriptPtr++;
			if (ScriptPtr == ScriptEndPtr
				|| text == &this->String[MAX_STRING_SIZE-1])
			{
				break;
			}
		}
		ScriptPtr++;
	}
	else
	{ // Normal string
		if (strchr ("{}|=", *this->ScriptPtr))
		{
			*text++ = *this->ScriptPtr++;
		}
		else
		{
			while ((*this->ScriptPtr > 32) && (strchr ("{}|=", *this->ScriptPtr) == NULL)
				&& (*this->ScriptPtr != ASCII_COMMENT)
				&& !(this->ScriptPtr[0] == CPP_COMMENT && (this->ScriptPtr < ScriptEndPtr - 1) &&
					 (this->ScriptPtr[1] == CPP_COMMENT || this->ScriptPtr[1] == C_COMMENT)))
			{
				*text++ = *this->ScriptPtr++;
				if (this->ScriptPtr == ScriptEndPtr
					|| text == &this->String[MAX_STRING_SIZE-1])
				{
					break;
				}
			}
		}
	}
	*text = 0;
	return true;
}


//
// SC_MustGetString
//
void FScriptParser::MustGetString (void)
{
	if (this->GetString () == false) {
		this->ScriptError("Missing string (unexpected end of file).");
	}
}


//
// SC_MustGetStringName
//
void FScriptParser::MustGetStringName (const char *name)
{
	this->MustGetString ();
	if (this->Compare(name) == false)
	{
		const char *args[2];
		args[0] = name;
		args[1] = this->String;
		this->ScriptError("Expected '%s', got '%s'.", args);
	}
}


//
// SC_GetNumber
//
bool FScriptParser::GetNumber (void)
{
	char *stopper;

	this->CheckOpen();
	if (sc.GetString())
	{
		if (strcmp (this->String, "MAXINT") == 0)
		{
			this->Number = MAXINT;
		}
		else
		{
			this->Number = strtol (this->String, &stopper, 0);
			if (*stopper != 0)
			{
				//I_Error ("SC_GetNumber: Bad numeric constant \"%s\".\n"
				//	"Script %s, Line %d\n", sc_String, this->ScriptName.c_str(), this->Line);
				Printf (PRINT_WARNING,"SC_GetNumber: Bad numeric constant \"%s\".\n"
					"Script %s, Line %d\n", this->String, this->ScriptName.c_str(), this->Line);
			}
		}
		this->Float = (float)this->Number;
		return true;
	}
	else
	{
		return false;
	}
}


//
// SC_MustGetNumber
//
void FScriptParser::MustGetNumber (void)
{
	if (this->GetNumber() == false) {
		this->ScriptError("Missing integer (unexpected end of file).");
	}
}


//
// SC_GetFloat
//
BOOL FScriptParser::GetFloat (void)
{
	char *stopper;

	this->CheckOpen();
	if (this->GetString())
	{
		this->Float = (float)strtod (this->String, &stopper);
		if (*stopper != 0)
		{
			//I_Error ("SC_GetFloat: Bad numeric constant \"%s\".\n"
			//	"Script %s, Line %d\n", this->String, this->ScriptName.c_str(), this->Line);
			Printf (PRINT_WARNING, "SC_GetFloat: Bad numeric constant \"%s\".\n"
				"Script %s, Line %d\n", this->String, this->ScriptName.c_str(), this->Line);
		}
		this->Number = (int)this->Float;
		return true;
	}
	else
	{
		return false;
	}
}


//
// SC_MustGetFloat
//
void FScriptParser::MustGetFloat (void)
{
	if (this->GetFloat() == false) {
		this->ScriptError("Missing floating-point number (unexpected end of file).");
	}
}


//
// SC_UnGet
//
// Assumes there is a valid string in sc.String.
//
void FScriptParser::UnGet (void)
{
	AlreadyGot = true;
}


//
// SC_Check
//
// Returns true if another token is on the current line.
//
/*
BOOL SC_Check(void)
{
	char *text;

	this->CheckOpen();
	text = this->ScriptPtr;
	if(text >= ScriptEndPtr)
	{
		return false;
	}
	while(*text <= 32)
	{
		if(*text == '\n')
		{
			return false;
		}
		text++;
		if(text == ScriptEndPtr)
		{
			return false;
		}
	}
	if(*text == ASCII_COMMENT)
	{
		return false;
	}
	return true;
}
*/


//
// SC_MatchString
//
// Returns the index of the first match to sc.String from the passed
// array of strings, or -1 if not found.
//
int FScriptParser::MatchString (const char **strings)
{
	int i;

	for (i = 0; *strings != NULL; i++)
	{
		if (this->Compare(*strings++)) {
			return i;
		}
	}
	return -1;
}


//
// SC_MustMatchString
//
int FScriptParser::MustMatchString (const char **strings)
{
	int i;

	i = this->MatchString (strings);
	if (i == -1) {
		this->ScriptError(NULL);
	}

	return i;
}


//
// SC_Compare
//
bool FScriptParser::Compare (const char *text)
{
	return (stricmp (text, this->String) == 0);
}


//
// SC_ScriptError
//
void FScriptParser::ScriptError (const char *message, const char **args)
{
	//char composed[2048];
	if (message == NULL)
		message = "Bad syntax.";

/*#if !defined(__GNUC__) && !defined(_MSC_VER)
	va_list arglist;
	va_start (arglist, *args);
	vsprintf (composed, message, arglist);
	va_end (arglist);
#else
	vsprintf (composed, message, args);
#endif*/

    Printf(PRINT_ERROR,"Script error, \"%s\" line %d: %s\n", this->ScriptName.c_str(),
		this->Line, message);

	//I_Error ("Script error, \"%s\" line %d: %s\n", this->ScriptName.c_str(),
	//	this->Line, message);
}

VERSION_CONTROL (sc_man_cpp, "$Id$")
