/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source Menu System
 * Written by Wend4r & komashchenko (Vladimir Ezhikov & Borys Komashchenko).
 * ======================================================

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <menu/chatsystem.hpp>

#include <filesystem.h>
#include <tier1/keyvalues3.h>
#include <tier1/utlrbtree.h>
#include <tier1/utlmap.h>

#include <any_config.hpp>

Menu::CChatSystem::CChatSystem()
 :  CLogger(Base::GetName(), NULL, 0, LV_DEFAULT, MENU_CHATCOMMANDSYSTEM_LOGGINING_COLOR), 

    m_mapAliases(DefLessFunc(const CUtlSymbolLarge))
{
}

bool Menu::CChatSystem::Load(const char *pszBaseGameDir, const char *pszPathID, CUtlVector<CUtlString> &vecMessages)
{
	CBufferStringN<MAX_PATH> sConfigFile;

	CUtlVector<CUtlString> vecConfigFiles;

	CUtlString sError;

	CBufferStringN<256> sMessage;

	AnyConfig::Anyone aGameConfig;

	AnyConfig::CLoadFromFile_General aLoadPresets({{&sError, NULL, pszPathID}, g_KV3Format_Generic});

	{
		sConfigFile.Insert(0, pszBaseGameDir);
		sConfigFile.Insert(sConfigFile.Length(), CORRECT_PATH_SEPARATOR_S);
		sConfigFile.Insert(sConfigFile.Length(), MENU_CHATSYSTEM_ALIASES_FILENAME);

		const char *pszConfigFile = sConfigFile.Get();

		g_pFullFileSystem->FindFileAbsoluteList(vecConfigFiles, pszConfigFile, pszPathID);

		if(vecConfigFiles.Count() < 1)
		{
			const char *pszMessageConcat[] = {"Failed to ", "find \"", pszConfigFile, "\" file"};

			sMessage.AppendConcat(ARRAYSIZE(pszMessageConcat), pszMessageConcat, NULL);
			vecMessages.AddToTail(sMessage);

			return false;
		}

		aLoadPresets.m_pszFilename = vecConfigFiles[0].Get();

		if(!aGameConfig.Load(aLoadPresets)) // Hot.
		{
			const char *pszMessageConcat[] = {"Failed to ", "load \"", pszConfigFile, "\" file", ": ", sError.Get()};

			sMessage.AppendConcat(ARRAYSIZE(pszMessageConcat), pszMessageConcat, NULL);
			vecMessages.AddToTail(sMessage);

			return false;
		}

		if(!LoadAliases(aGameConfig.Get(), vecMessages))
		{
			const char *pszMessageConcat[] = {"Failed to ", "apply \"", pszConfigFile, "\" file", ": ", sError.Get()};

			sMessage.AppendConcat(ARRAYSIZE(pszMessageConcat), pszMessageConcat, NULL);
			vecMessages.AddToTail(sMessage);

			return false;
		}

		// ...
	}

	return true;
}

void Menu::CChatSystem::Clear()
{
	m_tableAliases.Purge();
	m_mapAliases.Purge();
	m_tableAliasValues.Purge();
}

bool Menu::CChatSystem::LoadAliases(KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages)
{
	int nMemberCount = pData->GetMemberCount();

	if(!nMemberCount)
	{
		vecMessages.AddToTail("Aliases is empty");

		return false;
	}

	KV3MemberId_t i = 0;

	do
	{
		const char *pszMemberName = pData->GetMemberName(i);

		KeyValues3 *pMember = pData->GetMember(i);

		m_mapAliases.Insert(GetAliasSymbol(pszMemberName), GetAliasValueSymbol(pMember->GetString()));

		i++;
	}
	while(i < nMemberCount);

	return true;
}

int Menu::CChatSystem::ReplaceString(CBufferString &sBuffer)
{
	int iStoredLength = 0;

	FOR_EACH_MAP_FAST(m_mapAliases, i)
	{
		auto sAlias = m_mapAliases.Key(i);
		auto sReplaceTo = m_mapAliases.Element(i);

		iStoredLength += sBuffer.Replace(sAlias.String(), sReplaceTo.String());
	}

	return iStoredLength;
}

CUtlSymbolLarge Menu::CChatSystem::GetAliasSymbol(const char *pszName)
{
	return m_tableAliases.AddString(pszName);
}

CUtlSymbolLarge Menu::CChatSystem::FindAliasSymbol(const char *pszName) const
{
	return m_tableAliases.FindString(pszName);
}

CUtlSymbolLarge Menu::CChatSystem::GetAliasValueSymbol(const char *pszValue)
{
	return m_tableAliasValues.AddString(pszValue);
}

CUtlSymbolLarge Menu::CChatSystem::FindAliasValueSymbol(const char *pszValue) const
{
	return m_tableAliasValues.FindString(pszValue);
}
