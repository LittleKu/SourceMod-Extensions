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

CDropReason g_DropReason;		/**< Global singleton for extension's main interface */
IGameConfig *g_pGameConf = NULL;

SMEXT_LINK(&g_DropReason);
CDetour *g_pDetourPerformDisconnection = NULL;
IForward *g_pfwClientDropReason = NULL;

DETOUR_DECL_MEMBER1(PerformDisconnection, void, const char *, reason)
{
	IClient *pClient = reinterpret_cast<IClient*>((char*)this + 4);
	int iSlot = pClient->GetPlayerSlot();
	const char *lpName = pClient->GetClientName();
	g_pfwClientDropReason->PushCell(iSlot);
	g_pfwClientDropReason->PushString(lpName);
	g_pfwClientDropReason->PushString(reason);
	g_pfwClientDropReason->Execute(NULL);

	DETOUR_MEMBER_CALL(PerformDisconnection)(reason);
}

bool CDropReason::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	char szErr[256] = {0};
	if (!gameconfs->LoadGameConfigFile("PerformDisconnection.games", &g_pGameConf, szErr, sizeof(szErr)))
	{
		if (error)
			snprintf(error, maxlength, "Could not load \"PerformDisconnection.games\"");
		return false;
	}

	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);
	g_pDetourPerformDisconnection = DETOUR_CREATE_MEMBER(PerformDisconnection, "PerformDisconnection");
	if (g_pDetourPerformDisconnection != NULL)
		g_pDetourPerformDisconnection->EnableDetour();

	g_pfwClientDropReason = forwards->CreateForward("OnClientDrop", ET_Ignore, 3, NULL, Param_Cell, Param_String, Param_String);

	return true;
}

void CDropReason::SDK_OnUnload()
{
	gameconfs->CloseGameConfigFile(g_pGameConf);
	if (g_pDetourPerformDisconnection != NULL)
		g_pDetourPerformDisconnection->Destroy();

	forwards->ReleaseForward(g_pfwClientDropReason);
}