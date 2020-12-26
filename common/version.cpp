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
//
// Source code and protocol versioning
//
//-----------------------------------------------------------------------------

#include "version.h"
#include "git_describe.h"

#include <map>
#include <string>
#include <sstream>
#include <memory>

#include "cmdlib.h"
#include "c_dispatch.h"

typedef std::map<std::string, std::string> source_files_t;

source_files_t &get_source_files()
{
	static std::auto_ptr<source_files_t> source_files(new source_files_t);
	return *source_files.get();
}


file_version::file_version(const char *uid, const char *id, const char *pp, int l, const char *t, const char *d)
{
	std::stringstream rs(id), ss;
	std::string p = pp;

	size_t e = p.find_last_of("/\\");
	std::string file = p.substr(e == std::string::npos ? 0 : e + 1);

	ss << id << " " << l << " " << t << " " << d << " " << p.substr(e == std::string::npos ? 0 : e + 1);

	get_source_files()[file] = ss.str();
}

/**
 * @brief Return the current commit hash.
 */
const char* GitHash()
{
#ifdef GIT_HASH
	return GIT_HASH;
#else
	return "unknown";
#endif
}

/**
 * @brief Return the current branch name.
 */
const char* GitBranch()
{
#ifdef GIT_BRANCH
	return GIT_BRANCH;
#else
	return "unknown";
#endif
}

/**
 * @brief Return a truncated unambiguous hash.
 */
const char* GitShortHash()
{
#ifdef GIT_SHORT_HASH
	return GIT_SHORT_HASH;
#else
	return "unknown";
#endif
}

/**
 * @brief Return the Git version in a format that's good-enough to display
 *        in most end-user contexts.
 * 
 * @return A version string in the format of "version (branch, short hash)".
 *         On master the branch name is omitted.
*/
const char* GitNiceVersion()
{
	static std::string version;
	if (version.empty())
	{
		if (!strcmp(GitBranch(), "master"))
		{
			StrFormat(version, "%s (%s)", DOTVERSIONSTR, GitShortHash());
		}
		else
		{
			StrFormat(version, "%s (%s, %s)", DOTVERSIONSTR, GitBranch(), GitShortHash());
		}
	}
	return version.c_str();
}

BEGIN_COMMAND (version)
{
	if (argc == 1)
	{
		// distribution
		Printf(PRINT_HIGH, "Odamex v%s - %s\ncommit %s (%s)\n", DOTVERSIONSTR,
		       COPYRIGHTSTR, GitHash(), GitBranch());
	}
	else
	{
		// specific file version
		source_files_t::const_iterator it = get_source_files().find(argv[1]);

		if (it == get_source_files().end())
			Printf(PRINT_WARNING, "no such file: %s", argv[1]);
		else
			Printf(PRINT_HIGH, "%s", it->second.c_str());
	}
}
END_COMMAND (version)

BEGIN_COMMAND (listsourcefiles)
{
	for (source_files_t::const_iterator it = get_source_files().begin(); it != get_source_files().end(); ++it)
		Printf(PRINT_HIGH, "%s\n", it->first.c_str());
		
	Printf(PRINT_HIGH, "End of list\n");
}
END_COMMAND(listsourcefiles)

VERSION_CONTROL(version_cpp, "$Id$")



