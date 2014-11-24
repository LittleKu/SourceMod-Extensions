/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod (C)2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This file is part of the SourceMod/SourcePawn SDK.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#if defined _filter_included
 #endinput
#endif
#define _filter_included

/**
 * Get the count of the banned ip list 
 * 
 * @return		the count of the banned ip list
 */
native FT_GetFilterCount();

/**
 * Get a ip from the filter list with the given index
 * 
 * @param nIndex		the index at the banned ip list, should be start at 0 and the last one is count - 1.
 * @param ip			store the ip with the nIndex
 * @param maxLen		the max length store the ip
 * @return				1 on success, 0 on failed.
 */
native FT_GetFilterIP(nIndex, String:ip[], maxLen);


/**
 * Whether the ip has been banned or not.
 * 
 * @param ip			the ip want to check whether is banned or not
 * @param bantime		If the IP is banned, it will save time to be banned
 * @return				1:the ip has been banned, otherwise not
 */
native FT_GetBannedTime(const String:ip[], &Float:bantime);

/**
 * Call when type "addip" command
 * 
 * @note Ignore the return value.
 * 
 * @param cmd			always is "addip"
 * @param bantime		how long does the ip was banned
 * @param ip			the ip
 * 
 */
forward OnFillterAdd_Post(const String:cmd[], Float:bantime, const String:ip[]);


public Extension:__ext_filter = 
{
	name = "Filter",
	file = "filter.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};