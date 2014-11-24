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
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

typedef struct
{
	unsigned int	mask;
	unsigned int	compare;
	float       banEndTime; // 0 for permanent ban
	float       banTime;
} ipfilter_t;
/*
typedef struct
{
	USERID_t userid;
	float	banEndTime;
	float	banTime;
} userfilter_t;*/

#define	MAX_IPFILTERS	    32768
//#define	MAX_USERFILTERS	    32768

CFilter g_Filter;		/**< Global singleton for extension's main interface */
IGameConfig *g_pGameConf = NULL;
void *g_pListIP = NULL;
int g_iOffsetCount = 0;
int g_iOffsetIPFilter = 0;
CDetour *g_pDetourFilter_Add_f = NULL;
IForward *g_pFillter_Add_fForward = NULL;

SMEXT_LINK(&g_Filter);

DETOUR_DECL_STATIC1(Filter_Add_f, void, const CCommand&, args)
{
	DETOUR_STATIC_CALL(Filter_Add_f)(args);
	if (args.ArgC() == 3 && g_pFillter_Add_fForward != NULL)
	{
		char *dummy = NULL;
		float bantime = strtod(args.ArgV()[1], &dummy);
		g_pFillter_Add_fForward->PushString(args.ArgV()[0]);
		g_pFillter_Add_fForward->PushFloat(bantime);
		g_pFillter_Add_fForward->PushString(args.ArgV()[2]);
		g_pFillter_Add_fForward->Execute(NULL);
	}
}

ipfilter_t *GetFilterHeader()
{
	return **(ipfilter_t***)((char*)g_pListIP + g_iOffsetIPFilter);
}

int GetFilterCount()
{
	return **(int**)((char*)g_pListIP + g_iOffsetCount);
}

bool CFilter::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	char conf_error[255];
	if (!gameconfs->LoadGameConfigFile("filter.games", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if (error)
			snprintf(error, maxlength, "Could not read sample.games: %s", conf_error);
		return false;
	}

	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);
	g_pFillter_Add_fForward = forwards->CreateForward("OnFillterAdd_Post", ET_Ignore, 3, NULL, Param_String, Param_Float, Param_String);

	g_pDetourFilter_Add_f = DETOUR_CREATE_STATIC(Filter_Add_f, "Filter_Add_f");
	if (g_pDetourFilter_Add_f == NULL)
	{
		if (error)
			snprintf(error, maxlength, "Failed to create Filter_Add_f detour");
		return false;
	}
	g_pDetourFilter_Add_f->EnableDetour();
	if (!g_pGameConf->GetMemSig("listip", &g_pListIP))
	{
		if (error)
			snprintf(error, maxlength, "Could not read listip address");
		return false;
	}

	if (!g_pGameConf->GetOffset("count", &g_iOffsetCount))
	{
		if (error)
			snprintf(error, maxlength, "Could not read count offset");
		return false;
	}

	if (!g_pGameConf->GetOffset("filters", &g_iOffsetIPFilter))
	{
		if (error)
			snprintf(error, maxlength, "Could not read ipfilter offset");
		return false;
	}

	g_pShareSys->AddNatives(myself, g_Natives);
	return true;
}

void CFilter::SDK_OnUnload()
{
	gameconfs->CloseGameConfigFile(g_pGameConf);
	if (g_pDetourFilter_Add_f != NULL)
		g_pDetourFilter_Add_f->Destroy();
	forwards->ReleaseForward(g_pFillter_Add_fForward);
}

//native FT_GetFilterCount();
static cell_t SM_GetFilterCount(IPluginContext *pContext, const cell_t *params)
{
	return GetFilterCount();
}

//native FT_GetListIP(nIndex, String:ip[], maxLen)
static cell_t SM_GetFilterIP(IPluginContext *pContext, const cell_t *params)
{
	int nIndex = params[1];
	int nCount = GetFilterCount();
	if (nCount <= 0)
	{
		pContext->StringToLocal(params[2], params[3], "empty");
		return 0;
	}

	if (nIndex < 0 && nIndex >= nCount)
	{
		return pContext->ThrowNativeError("IP index out of bounds 0~%d (%d)", nCount - 1, nIndex);
	}

	char szIP[32];
	ipfilter_t *pFilterHeader = GetFilterHeader();

	for (int i = 0; i < GetFilterCount(); i++)
	{
		if (i == nIndex)
		{
			BYTE b[4];
			*(unsigned int*)b = pFilterHeader->compare;
			snprintf(szIP, sizeof(szIP), "%i.%i.%i.%i", b[0], b[1], b[2], b[3]);
			pContext->StringToLocal(params[2], params[3], szIP);
			return 1;
		}
		pFilterHeader += sizeof(ipfilter_t);
	}
	pContext->StringToLocal(params[2], params[3], "empty");
	return 0;
}

//native FT_GetBannedTime(const String:ip[], &Float:bantime)
static cell_t SM_GetBannedTime(IPluginContext *pContext, const cell_t *params)
{
	char *lpIP = NULL;
	pContext->LocalToString(params[1], &lpIP);

	cell_t *lpBanTime = NULL;
	pContext->LocalToPhysAddr(params[2], &lpBanTime);

	if (lpIP != NULL && lpBanTime != NULL)
	{
		DWORD dwIP = ::inet_addr(lpIP);
		ipfilter_t *pFilterHeader = GetFilterHeader();
		for (int i = 0; i < GetFilterCount(); i++)
		{
			if (dwIP == pFilterHeader->compare)
			{
				*lpBanTime = sp_ftoc(pFilterHeader->banTime);
				return 1;
			}
			pFilterHeader += sizeof(ipfilter_t);
		}
	}
	return 0;
}

const sp_nativeinfo_t g_Natives[] = 
{
	{"FT_GetFilterCount", SM_GetFilterCount},
	{"FT_GetFilterIP", SM_GetFilterIP},
	{"FT_GetBannedTime", SM_GetBannedTime},
	{NULL, NULL},
};