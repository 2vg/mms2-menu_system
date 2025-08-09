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

#include <menu/profilesystem.hpp>
#include <menu/profile.hpp>

#include <filesystem.h>
#include <tier0/dbg.h>
#include <tier1/keyvalues3.h>
#include <tier1/utlrbtree.h>
#include <tier1/utlmap.h>

#include <any_config.hpp>

Menu::CProfileSystem::CProfileSystem()
 :  CLogger(Base::GetName(), NULL, 0, LV_DEFAULT, MENU_SYSTEMBASE_LOGGINING_COLOR), 

    m_map(DefLessFunc(const CUtlSymbolLarge)), 
    m_mapRefs(DefLessFunc(const CUtlSymbolLarge))
{
}

bool Menu::CProfileSystem::Load(const char *pszBaseGameDir, const char *pszPathID, CUtlVector<CUtlString> &vecMessages)
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
		sConfigFile.Insert(sConfigFile.Length(), MENU_SYSTEM_PROFILES_FILENAME);

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

		if(!Load(aGameConfig.Get(), vecMessages))
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

void Menu::CProfileSystem::Clear()
{
	m_tableSymbols.Purge();
	m_map.PurgeAndDeleteElements();
	m_mapRefs.Purge();

	m_aEntityKeyValuesAllocator.Purge();
}

bool Menu::CProfileSystem::Load(KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages)
{
	int nMemberCount = pData->GetMemberCount();

	if(!nMemberCount)
	{
		vecMessages.AddToTail("Profiles is empty");

		return false;
	}

	CBufferStringN<256> sMessage;

	KV3MemberId_t i = 0;

	do
	{
		const char *pszMemberName = pData->GetMemberName(i);

		KeyValues3 *pMember = pData->GetMember(i);

		KV3Type_t eMemberType = pMember->GetType();

		if(eMemberType == KV3_TYPE_STRING)
		{
			// Handle it in Load2().
		}
		else if(eMemberType == KV3_TYPE_TABLE)
		{
			m_map.Insert(GetSymbol(pszMemberName), new CProfile());
		}
		else
		{
			const char *pszMessageConcat[] = {"No handler ", "of \"", pMember->GetTypeAsString(), "\" type", "for \"", pszMemberName, "\" member"};

			sMessage.AppendConcat(ARRAYSIZE(pszMessageConcat), pszMessageConcat, NULL);
			vecMessages.AddToTail(sMessage);
			sMessage.Clear();
		}

		i++;
	}
	while(i < nMemberCount);

	return Load2(pData, vecMessages);
}

bool Menu::CProfileSystem::Load2(KeyValues3 *pData, CUtlVector<CUtlString> &vecMessages)
{
	CBufferStringN<256> sMessage;

	KV3MemberId_t i = 0;

	int nMemberCount = pData->GetMemberCount();

	do
	{
		const char *pszMemberName = pData->GetMemberName(i);

		KeyValues3 *pMember = pData->GetMember(i);

		KV3Type_t eMemberType = pMember->GetType();

		if(eMemberType == KV3_TYPE_STRING)
		{
			const char *pszMemberValue = pMember->GetString();

			auto *pFoundProfile = GetInternal(pszMemberValue);

			if(pFoundProfile)
			{
				m_mapRefs.Insert(GetSymbol(pszMemberName), pFoundProfile);
				Handle(pszMemberName, pFoundProfile);
			}
			else
			{
				const char *pszMessageConcat[] = {"No found \"", pszMemberValue, "\" profile ", "for \"", pszMemberName, "\" member"};

				sMessage.AppendConcat(ARRAYSIZE(pszMessageConcat), pszMessageConcat, NULL);
				vecMessages.AddToTail(sMessage);
				sMessage.Clear();
			}
		}
		else if(eMemberType == KV3_TYPE_TABLE)
		{
			auto *pFoundProfile = GetInternal(pszMemberName);

			Assert(pFoundProfile);
			pFoundProfile->Load(this, pMember, PROFILE_LOAD_STEP2, vecMessages);
			Handle(pszMemberName, pFoundProfile);
		}
		else
		{
			AssertMsg2(0, "Second load was found unknown \"%s\" type of \"%s\" member", pMember->GetTypeAsString(), pszMemberName);
		}

		i++;
	}
	while(i < nMemberCount);

	return true;
}

Menu::CProfile *Menu::CProfileSystem::GetInternal(const char *pszName)
{
	CUtlSymbolLarge sFound = FindSymbol(pszName);

	if(!sFound.IsValid())
	{
		return nullptr;
	}

	// m_mapRefs commonly used.
	auto iFound = m_mapRefs.Find(sFound);

	if(iFound == m_mapRefs.InvalidIndex())
	{
		iFound = m_map.Find(sFound);

		if(iFound == m_map.InvalidIndex())
		{
			return nullptr;
		}

		return m_map.Element(iFound);
	}

	return m_mapRefs.Element(iFound);
}

IMenuProfile *Menu::CProfileSystem::Get(const char *pszName)
{
	return GetInternal(pszName);
}

void Menu::CProfileSystem::AddOrReplaceRef(const char *pszName, IMenuProfile *pProfile)
{
	m_mapRefs.InsertOrReplace(GetSymbol(pszName), static_cast<CProfile *>(pProfile));
}

CKeyValues3Context *Menu::CProfileSystem::GetEntityKeyValuesAllocator()
{
	return &m_aEntityKeyValuesAllocator;
}

uint Menu::CProfileSystem::LoopByProfiles(const OnProfileCallback_t &funcCallback)
{
	FOR_EACH_MAP(m_map, i)
	{
		funcCallback(m_map.Key(i), m_map.Element(i));
	}

	return m_map.Count();
}

uint Menu::CProfileSystem::LoopByProfileRefs(const OnProfileCallback_t &funcCallback)
{
	FOR_EACH_MAP(m_mapRefs, i)
	{
		funcCallback(m_mapRefs.Key(i), m_mapRefs.Element(i));
	}

	return m_mapRefs.Count();
}

CUtlSymbolLarge Menu::CProfileSystem::GetSymbol(const char *pszName)
{
	return m_tableSymbols.AddString(pszName);
}

CUtlSymbolLarge Menu::CProfileSystem::FindSymbol(const char *pszName) const
{
	return m_tableSymbols.FindString(pszName);
}
