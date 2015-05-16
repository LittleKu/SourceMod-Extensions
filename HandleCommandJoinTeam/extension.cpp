/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
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

#include "extension.h"

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

CHandleCommandJoinTeam g_Ext;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_Ext);

IGameConfig *g_pGameConf = NULL;
CDetour *g_pDetourHandleCommandJoinTeam = NULL;
IForward *g_pfwOnJoinTeam = NULL;
extern sp_nativeinfo_t natives[];

int mlTeam = 0;

DETOUR_DECL_MEMBER3(HandleCommandJoinTeam, bool, int, team, bool, bStatus, int, a)
{
	//printf("\n\tteam \"%d\", b \"%d\" member\"%d\"\n", team, a, *(int*)((char*)this + 3032));
	//*(bool*)((char*)this + 5249) = true;/*m_iCoachingTeam*/
	bool bRet = DETOUR_MEMBER_CALL(HandleCommandJoinTeam)(team, bStatus, a);
	
	return bRet;
}

bool CHandleCommandJoinTeam::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	char szErr[255] = {0};
	if (!gameconfs->LoadGameConfigFile("HandleCommandJoinTeam.games", &g_pGameConf, szErr, sizeof(szErr)))
	{
		if (error)
			snprintf(error, maxlength, "Could not load \'HandleCommandJoinTeam.games\"");
		return false;
	}

	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);
	g_pDetourHandleCommandJoinTeam = DETOUR_CREATE_MEMBER(HandleCommandJoinTeam, "HandleCommand_JoinTeam");

	if (g_pDetourHandleCommandJoinTeam != NULL)
		g_pDetourHandleCommandJoinTeam->EnableDetour();

	sharesys->AddNatives(myself, natives);
	return true;
}

void CHandleCommandJoinTeam::SDK_OnUnload()
{
	gameconfs->CloseGameConfigFile(g_pGameConf);
	if (g_pDetourHandleCommandJoinTeam != NULL)
		g_pDetourHandleCommandJoinTeam->Destroy();
}

static cell_t JoinTeam_Return(IPluginContext *pContext, const cell_t *params)
{
	return 1;
}

sp_nativeinfo_t natives[] =
{
	{"JoinTeam_Return",JoinTeam_Return},
	{NULL,NULL}
};