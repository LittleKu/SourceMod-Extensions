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

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

/**
 * @file extension.h
 * @brief Sample extension code header.
 */

#include "smsdk_ext.h"
#include <string>

const int INDEX_LENGTH = 7;        // һ����������4�ֽڵ���ʼIP��3�ֽڵ�IP��¼ƫ�ƣ���7�ֽ�
const int IP_LENGTH = 4;
const int OFFSET_LENGTH = 3;

enum
{
	REDIRECT_MODE_1 = 0x01,    // �ض���ģʽ1 ƫ�������޵�����
	REDIRECT_MODE_2 = 0x02,    // �ض���ģʽ2 ƫ�������е�����
};

class CIpFinder : public SDKExtension
{
public:
	CIpFinder();
	CIpFinder(const char* pszFileName);
	~CIpFinder();
	
	bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	void SDK_OnUnload();

    // ��ȡip��������������
    void GetAddressByIp(unsigned long ipValue, std::string& strCountry, std::string& strLocation) const;
    void GetAddressByIp(const char* pszIp, std::string& strCountry, std::string& strLocation) const;
    void GetAddressByOffset(unsigned long ulOffset, std::string& strCountry, std::string& strLocation) const;

    unsigned long GetString(std::string& str, unsigned long indexStart) const;
    unsigned long GetValue3(unsigned long indexStart) const;
    unsigned long GetValue4(unsigned long indexStart) const;

    // ת��
    unsigned long IpString2IpValue(const char *pszIp) const;
    void IpValue2IpString(unsigned long ipValue, char* pszIpAddress, int nMaxCount) const;
    bool IsRightIpString(const char* pszIp) const;

    // �������
    unsigned long OutputData(const char* pszFileName, unsigned long ulIndexStart = 0, unsigned long ulIndexEnd = 0) const;
    unsigned long OutputDataByIp(const char* pszFileName, unsigned long ipValueStart, unsigned long ipValueEnd) const;
    unsigned long OutputDataByIp(const char* pszFileName, const char* pszStartIp, const char* pszEndIp) const;

    unsigned long SearchIp(unsigned long ipValue, unsigned long indexStart = 0, unsigned long indexEnd = 0) const;
    unsigned long SearchIp(const char* pszIp, unsigned long indexStart = 0, unsigned long indexEnd = 0) const;

    bool Open(const char* pszFileName);

	char *ANSIToUTF8(const char *lpString, size_t nSize);
private:
	FILE *m_fpIpDataFile;            // IP���ݿ��ļ�
    unsigned long m_indexStart;    // ��ʼ����ƫ��
    unsigned long m_indexEnd;        // ��������ƫ��
};

extern const sp_nativeinfo_t ip_native[];
#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
