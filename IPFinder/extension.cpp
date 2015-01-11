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
#include <string.h>

#ifndef WIN32
#include <stdlib.h>
#include <iconv.h>
#endif

/**
* @file extension.cpp
* @brief Implement extension code here.
*/

CIpFinder g_IPFinder;		/**< Global singleton for extension's main interface */

SMEXT_LINK(&g_IPFinder);

// ============================================================================
// ==============================================================================
CIpFinder::CIpFinder()
{
}

// ============================================================================
// ==============================================================================
CIpFinder::CIpFinder(const char *pszFileName)
{
	this->Open(pszFileName);
}

// ============================================================================
//    打开数据库文件
// ==============================================================================
bool CIpFinder::Open(const char *pszFileName)
{
	m_fpIpDataFile = fopen(pszFileName, "rb");
	if (!m_fpIpDataFile)
		return false;

	// IP头由两个十六进制4字节偏移量构成，分别为索引开始，和索引结束
	m_indexStart = this->GetValue4(0);
	m_indexEnd = this->GetValue4(4);
	return true;
}

// ============================================================================
// ==============================================================================
CIpFinder::~CIpFinder()
{
	fclose(m_fpIpDataFile);
}

// ============================================================================
//    根据IP地址字符串返回其十六进制值（4字节）
// ============================================================================
unsigned long CIpFinder::IpString2IpValue(const char *pszIp) const
{
	if (!this->IsRightIpString(pszIp))
		return 0;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int nNum = 0;            // 每个段数值
	const char *pBeg = pszIp;
	const char *pPos = NULL;
	unsigned long ulIp = 0; // 整个IP数值
	char *dummy;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	pPos = strchr(pszIp, '.');
	while (pPos != NULL)
	{
		nNum = strtoul(pBeg, &dummy, 10);
		ulIp += nNum;
		ulIp *= 0x100;
		pBeg = pPos + 1;
		pPos = strchr(pBeg, '.');
	}

	nNum = strtoul(pBeg, &dummy, 10);
	ulIp += nNum;
	return ulIp;
}

// ============================================================================
//    根据ip值获取字符串（由点分割）
// ============================================================================
void CIpFinder::IpValue2IpString(unsigned long ipValue,
								 char *pszIpAddress,
								 int nMaxCount) const
{
	if (!pszIpAddress)
		return;

	_snprintf(pszIpAddress, nMaxCount, "%d.%d.%d.%d", (ipValue & 0xFF000000) >> 24,
		(ipValue & 0x00FF0000) >> 16, (ipValue & 0x0000FF00) >> 8,ipValue & 0x000000FF);
	pszIpAddress[nMaxCount - 1] = 0;
}

// ============================================================================
//    根据指定IP(十六进制值)，返回其在索引段中的位置(索引)
//    ulIndexStart和ulIndexEnd可以指定搜索范围 均为0表示搜索全部
// ============================================================================
unsigned long CIpFinder::SearchIp(unsigned long ipValue,
								  unsigned long indexStart,
								  unsigned long indexEnd) const
{
	if (!m_fpIpDataFile)
		return 0;

	if (0 == indexStart)
		indexStart = m_indexStart;

	if (0 == indexEnd)
		indexEnd = m_indexEnd;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	unsigned long indexLeft = indexStart;
	unsigned long indexRight = indexEnd;

	// 先除后乘是为了保证mid指向一个完整正确的索引
	unsigned long indexMid = (indexRight - indexLeft) / INDEX_LENGTH / 2 * INDEX_LENGTH + indexLeft;

	// 起始Ip地址(如172.23.0.0),他和Ip记录中的Ip地址(如172.23.255.255)构成一个Ip范围，在这个范围内的Ip都可以由这条索引来获取国家、地区
	unsigned long ulIpHeader = 0;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	do
	{
		ulIpHeader = this->GetValue4(indexMid);
		if (ipValue < ulIpHeader)
		{
			indexRight = indexMid;
			indexMid = (indexRight - indexLeft) /    INDEX_LENGTH / 2 * INDEX_LENGTH + indexLeft;
		} else {
			indexLeft = indexMid;
			indexMid = (indexRight - indexLeft) /    INDEX_LENGTH / 2 * INDEX_LENGTH + indexLeft;
		}
	} while (indexLeft < indexMid);            // 注意是根据mid来进行判断

	// 只要符合范围就可以，不需要完全相等
	return indexMid;
}

// ============================================================================
// ==============================================================================
unsigned long CIpFinder::SearchIp(const char *pszIp,
								  unsigned long indexStart,
								  unsigned long indexEnd) const
{
	if (!this->IsRightIpString(pszIp))
		return 0;

	return this->SearchIp(this->IpString2IpValue(pszIp), indexStart, indexEnd);
}

// ==========================================================================================================
//    从指定位置获取一个十六进制的数 (读取3个字节， 主要用于获取偏移量， 与效率紧密相关的函数，尽可能优化）
// ==========================================================================================================
unsigned long CIpFinder::GetValue3(unsigned long indexStart) const
{
	if (!m_fpIpDataFile)
		return 0;

	//~~~~~~~~~~~~~~~~~~~~
	int nVal[3];
	unsigned long ulValue = 0;
	//~~~~~~~~~~~~~~~~~~~~

	fseek(m_fpIpDataFile, indexStart, SEEK_SET);
	for (int i = 0; i < 3; i++)
	{
		// 过滤高位，一次读取一个字符
		nVal[i] = fgetc(m_fpIpDataFile) & 0x000000FF;
	}

	for (int j = 2; j >= 0; --j)
	{
		// 因为读取多个16进制字符，叠加
		ulValue = ulValue * 0x100 + nVal[j];
	}
	return ulValue;
}

// ==========================================================================================================
//    从指定位置获取一个十六进制的数 (读取4个字节， 主要用于获取IP值， 与效率紧密相关的函数，尽可能优化）
// ==========================================================================================================
unsigned long CIpFinder::GetValue4(unsigned long indexStart) const
{
	if (!m_fpIpDataFile)
		return 0;

	//~~~~~~~~~~~~~~~~~~~~
	int nVal[4];
	unsigned long ulValue = 0;
	//~~~~~~~~~~~~~~~~~~~~

	fseek(m_fpIpDataFile, indexStart, SEEK_SET);
	for (int i = 0; i < 4; i++)
	{
		// 过滤高位，一次读取一个字符
		nVal[i] = fgetc(m_fpIpDataFile) & 0x000000FF;
	}

	for (int j = 3; j >= 0; --j)
	{
		// 因为读取多个16进制字符，叠加
		ulValue = ulValue * 0x100 + nVal[j];
	}
	return ulValue;
}

// ============================================================================
//    从指定位置获取字符串
// ============================================================================
unsigned long CIpFinder::GetString(std::string &str, unsigned long indexStart) const
{
	if (!m_fpIpDataFile)
		return 0;

	str.erase(0, str.size());

	fseek(m_fpIpDataFile, indexStart, SEEK_SET);
	//~~~~~~~~~~~~~~~~~~~~~~
	int nChar = fgetc(m_fpIpDataFile);
	unsigned long ulCount = 1;
	//~~~~~~~~~~~~~~~~~~~~~~

	// 读取字符串，直到遇到0x00为止
	while (nChar != 0x00)
	{
		// 依次放入用来存储的字符串空间中
		str += static_cast<char>(nChar);
		++ulCount;
		nChar = fgetc(m_fpIpDataFile);
	}

	// 返回字符串长度
	return ulCount;
}

// ============================================================================
//    通过指定的偏移量来获取ip记录中的国家名和地区名，偏移量可由索引获取
//    ulOffset为Ip记录偏移量
// ============================================================================
void CIpFinder::GetAddressByOffset(unsigned long ulOffset, std::string &strCountry, std::string &strLocation) const
{
	if (!m_fpIpDataFile)
		return;

	// 略去4字节Ip地址
	ulOffset += IP_LENGTH;
	fseek(m_fpIpDataFile, ulOffset, SEEK_SET);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// 读取首地址的值
	int nVal = (fgetc(m_fpIpDataFile) & 0x000000FF);
	unsigned long ulCountryOffset = 0;    // 真实国家名偏移
	unsigned long ulLocationOffset = 0; // 真实地区名偏移
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// 为了节省空间，相同字符串使用重定向，而不是多份拷贝
	if (REDIRECT_MODE_1 == nVal)
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// 重定向1类型
		unsigned long ulRedirect = this->GetValue3(ulOffset + 1); // 重定向偏移
		///
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		fseek(m_fpIpDataFile, ulRedirect, SEEK_SET);

		if ((fgetc(m_fpIpDataFile) & 0x000000FF) == REDIRECT_MODE_2)
		{
			// 混合类型1，重定向1类型进入后遇到重定向2类型 
			// 0x01 1字节
			// 偏移量 3字节 -----> 0x02 1字节 
			//                     偏移量 3字节 -----> 国家名
			//                     地区名
			ulCountryOffset = this->GetValue3(ulRedirect + 1);
			this->GetString(strCountry, ulCountryOffset);
			ulLocationOffset = ulRedirect + 4;
		} else {

			// 单纯的重定向模式1
			// 0x01 1字节
			// 偏移量 3字节 ------> 国家名
			//                      地区名
			ulCountryOffset = ulRedirect;
			ulLocationOffset = ulRedirect + this->GetString(strCountry,
				ulCountryOffset);
		}
	} else if (REDIRECT_MODE_2 == nVal) {

		// 重定向2类型
		// 0x02 1字节
		// 国家偏移 3字节 -----> 国家名
		// 地区名
		ulCountryOffset = this->GetValue3(ulOffset + 1);
		this->GetString(strCountry, ulCountryOffset);

		ulLocationOffset = ulOffset + 4;
	} else {

		// 最简单的情况 没有重定向
		// 国家名
		// 地区名
		ulCountryOffset = ulOffset;
		ulLocationOffset = ulCountryOffset + this->GetString(strCountry,
			ulCountryOffset);
	}

	// 读取地区
	fseek(m_fpIpDataFile, ulLocationOffset, SEEK_SET);
	if ((fgetc(m_fpIpDataFile) & 0x000000FF) == REDIRECT_MODE_2 || (fgetc(m_fpIpDataFile) & 0x000000FF) == REDIRECT_MODE_1)
	{

			// 混合类型2(最复杂的情形，地区也重定向)
			// 0x01 1字节
			// 偏移量 3字节 ------> 0x02 1字节
			//                      偏移量 3字节 -----> 国家名
			//                      0x01 or 0x02 1字节
			//                      偏移量 3字节 ----> 地区名 偏移量为0表示未知地区
			ulLocationOffset = this->GetValue3(ulLocationOffset + 1);
	}

	this->GetString(strLocation, ulLocationOffset);
}

// ============================================================================
//    根据十六进制ip获取国家名地区名
// ============================================================================
void CIpFinder::GetAddressByIp(unsigned long ipValue,
							   std::string &strCountry,
							   std::string &strLocation) const
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	unsigned long ulIndexOffset = this->SearchIp(ipValue);
	unsigned long ulRecordOffset = this->GetValue3(ulIndexOffset + IP_LENGTH);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	this->GetAddressByOffset(ulRecordOffset, strCountry, strLocation);
}

// ============================================================================
//    根据ip字符串获取国家名地区名
// ============================================================================
void CIpFinder::GetAddressByIp(const char *pszIp,
							   std::string &strCountry,
							   std::string &strLocation) const
{
	if (!this->IsRightIpString(pszIp)) {
		return;
	}
	this->GetAddressByIp(this->IpString2IpValue(pszIp), strCountry, strLocation);
}

// ============================================================================
//    将ip数据导出，start和end界定导出范围， 可通过SearchIp来获取
// ============================================================================
unsigned long CIpFinder::OutputData(const char *pszFileName,
									unsigned long indexStart,
									unsigned long indexEnd) const
{
	if (!m_fpIpDataFile || !pszFileName) {
		return 0;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	FILE *fpOut = fopen(pszFileName, "wb");
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (!fpOut) {
		return 0;
	}

	if (0 == indexStart) {
		indexStart = m_indexStart;
	}

	if (0 == indexEnd) {
		indexEnd = m_indexEnd;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~
	char szEndIp[255];
	char szStartIp[255];
	std::string strCountry;
	std::string strLocation;
	unsigned long ulCount = 0;
	unsigned long ipValueEnd = 0;
	unsigned long ipValueStart = 0;
	//~~~~~~~~~~~~~~~~~~~~~~~~

	for (unsigned long i = indexStart; i < indexEnd; i += INDEX_LENGTH) {

		// 获取IP段的起始IP和结束IP， 起始IP为索引部分的前4位16进制
		// 结束IP在IP信息部分的前4位16进制中，靠索引部分指定的偏移量找寻
		ipValueStart = this->GetValue4(i);
		ipValueEnd = this->GetValue4(this->GetValue3(i + IP_LENGTH));

		// 导出IP信息，格式是 起始IP/t结束IP/t国家位置/t地域位置/n
		this->IpValue2IpString(ipValueStart, szStartIp, sizeof(szStartIp));
		this->IpValue2IpString(ipValueEnd, szEndIp, sizeof(szEndIp));
		this->GetAddressByOffset(this->GetValue3(i + IP_LENGTH), strCountry,
			strLocation);
		fprintf(fpOut, "%s/t%s/t%s/t%s/n", szStartIp, szEndIp,
			strCountry.c_str(), strLocation.c_str());
		ulCount++;
	}

	fclose(fpOut);

	// 返回导出总条数
	return ulCount;
}

// ============================================================================
//    通过ip值界定导出范围
// ==============================================================================
unsigned long CIpFinder::OutputDataByIp(const char *pszFileName,
										unsigned long ipValueStart,
										unsigned long ipValueEnd) const
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	unsigned long indexStart = this->SearchIp(ipValueStart);
	unsigned long indexEnd = this->SearchIp(ipValueEnd);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	return this->OutputData(pszFileName, indexStart, indexEnd);
}

// ============================================================================
//    通过ip字符串界定导出范围
// ==============================================================================
unsigned long CIpFinder::OutputDataByIp(const char *pszFileName,
										const char *pszStartIp,
										const char *pszEndIp) const
{
	if (!this->IsRightIpString(pszStartIp) || !this->IsRightIpString(pszEndIp)) {
		return 0;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	unsigned long ipValueEnd = this->IpString2IpValue(pszEndIp);
	unsigned long ipValueStart = this->IpString2IpValue(pszStartIp);
	unsigned long indexEnd = this->SearchIp(ipValueEnd);
	unsigned long indexStart = this->SearchIp(ipValueStart);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	return this->OutputData(pszFileName, indexStart, indexEnd);
}

// ============================================================================
//    判断给定IP字符串是否是合法的ip地址
// ==============================================================================
bool CIpFinder::IsRightIpString(const char* pszIp) const
{
	if (!pszIp) {
		return false;
	}

	int nLen = strlen(pszIp);
	if (nLen < 7) {

		// 至少包含7个字符"0.0.0.0"
		return false;
	}

	for (int i = 0; i < nLen; ++i) {
		if (!isdigit(pszIp[i]) && pszIp[i] != '.') {
			return false;
		}

		if (pszIp[i] == '.') {
			if (0 == i) {
				if (!isdigit(pszIp[i + 1])) {
					return false;
				}
			} else if (nLen - 1 == i) {
				if (!isdigit(pszIp[i - 1])) {
					return false;
				}
			} else {
				if (!isdigit(pszIp[i - 1]) || !isdigit(pszIp[i + 1])) {
					return false;
				}
			}
		}
	}
	return true;
}

// ============================================================================
//    转格式
// ==============================================================================
char *CIpFinder::ANSIToUTF8(const char *lpString, size_t nSize)
{
#ifdef WIN32
	int alen, ulen;

	if ((alen = strlen(lpString)) <= 0)
	{
		return NULL;
	}

	static wchar_t unicode[1024];
	static char output[1024];

	int unicodelen = 0;

	//change ansi to unicode.
	unicodelen = MultiByteToWideChar(CP_ACP, NULL, lpString, alen, NULL, NULL);
	MultiByteToWideChar(CP_ACP, NULL, lpString, alen, unicode, unicodelen);

	//change unicode to utf-8.
	ulen = WideCharToMultiByte(CP_UTF8, NULL, unicode, unicodelen, NULL, NULL, NULL, NULL);
	if (ulen < sizeof(output))
	{
		WideCharToMultiByte(CP_UTF8, NULL, unicode, unicodelen, output, ulen, NULL, NULL);
		output[ulen] = '\0';
	}
	else return NULL;

#else
	iconv_t cd;
	cd = iconv_open("utf-8","gb2312");
	static char output[1024];
	static char input[1024];
	strncpy(input, lpString, nSize);

	char *inp = input;
	char **pin = &inp;

	char *outp = output;
	char **pout = &outp;

	int inlen = strlen(input);
	int outlen = sizeof(output);
	memset(output,0,outlen);
	iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen);
	iconv_close(cd);
#endif
	return output;
}

bool CIpFinder::SDK_OnLoad(char *error, size_t length, bool late)
{
	char path[PLATFORM_MAX_PATH];
	
	g_pSM->BuildPath(Path_SM, path, sizeof(path), "configs/ipsee/qqwry.dat");

	if (!Open(path))
	{
		if (error)
			snprintf(error, length, "Could not load \"%s\"\n", path);
		return false;
	}

	g_pShareSys->AddNatives(myself, ip_native);
	g_pShareSys->RegisterLibrary(myself, "IPFinder");
	return true;
}

void CIpFinder::SDK_OnUnload()
{
}

static cell_t sm_ipsee_info(IPluginContext *pCtx, const cell_t *params)
{
	char *ip;
	pCtx->LocalToString(params[1], &ip);
	std::string country, location;
	g_IPFinder.GetAddressByIp(ip, country, location);

	if (params[6])
	{
		char *uConntry = g_IPFinder.ANSIToUTF8(country.c_str(), country.size());
		pCtx->StringToLocal(params[2], static_cast<size_t>(params[3]), uConntry);

		char *uLocation = g_IPFinder.ANSIToUTF8(location.c_str(), location.size());
		pCtx->StringToLocal(params[4], static_cast<size_t>(params[5]), uLocation);
	}
	else
	{
		pCtx->StringToLocal(params[2], static_cast<size_t>(params[3]), country.c_str());
		pCtx->StringToLocal(params[4], static_cast<size_t>(params[5]), location.c_str());
	}

	return 1;
}

const sp_nativeinfo_t ip_native[] = 
{
	{"ipsee_info",		sm_ipsee_info},
	{NULL,					NULL},
};

