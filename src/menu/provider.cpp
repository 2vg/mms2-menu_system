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

#include <menu/provider.hpp>
#include <globals.hpp>

#include <filesystem.h>
#include <tier0/commonmacros.h>
#include <tier0/keyvalues3.h>
#include <tier0/strtools.h>

#include <any_config.hpp>

Menu::CProvider::CProvider()
 :  m_mapLibraries(DefLessFunc(const CUtlSymbolLarge))
{
}

bool Menu::CProvider::Init(GameData::CStringVector &vecMessages)
{
	// Enigne 2.
	{
		const char szEngineModuleName[] = "engine2";

		if(!m_aEngine2Library.InitFromMemory(g_pEngineServer))
		{
			static const char *s_pszMessageConcat[] = {"Failed to ", "get \"", szEngineModuleName, "\" module"};

			vecMessages.AddToTail({s_pszMessageConcat});
		}

		m_mapLibraries.Insert(GetSymbol(szEngineModuleName), &m_aEngine2Library);
	}

	// File System.
	{
		const char szFileSystemSTDIOModuleName[] = "filesystem_stdio";

		if(!m_aFileSystemSTDIOLibrary.InitFromMemory(g_pFullFileSystem))
		{
			static const char *s_pszMessageConcat[] = {"Failed to ", "get \"", szFileSystemSTDIOModuleName, "\" module"};

			vecMessages.AddToTail({s_pszMessageConcat});
		}

		m_mapLibraries.Insert(GetSymbol(szFileSystemSTDIOModuleName), &m_aFileSystemSTDIOLibrary);
	}

	// Server.
	{
		const char szServerModuleName[] = "server";

		if(!m_aServerLibrary.InitFromMemory(g_pSource2Server))
		{
			static const char *s_pszMessageConcat[] = {"Failed to ", "get \"", szServerModuleName, "\" module"};

			vecMessages.AddToTail({s_pszMessageConcat});
		}

		m_mapLibraries.Insert(GetSymbol(szServerModuleName), &m_aServerLibrary);
	}

	return true;
}

bool Menu::CProvider::Load(const char *pszBaseDir, const char *pszPathID, GameData::CStringVector &vecMessages)
{
	if(!LoadGameData(pszBaseDir, pszPathID, vecMessages))
	{
		return false;
	}

	// ...

	return true;
}

bool Menu::CProvider::Destroy(GameData::CStringVector &vecMessages)
{
	m_mapLibraries.PurgeAndDeleteElements();

	return true;
}

const DynLibUtils::CModule *Menu::CProvider::FindLibrary(const char *pszName) const
{
	auto iFound = m_mapLibraries.Find(FindSymbol(pszName));

	Assert(m_mapLibraries.IsValidIndex(iFound));

	return m_mapLibraries.Element(iFound);
}

CUtlSymbolLarge Menu::CProvider::GetSymbol(const char *pszText)
{
	return m_aSymbolTable.AddString(pszText);
}

CUtlSymbolLarge Menu::CProvider::FindSymbol(const char *pszText) const
{
	return m_aSymbolTable.Find(pszText);
}

bool Menu::CProvider::LoadGameData(const char *pszBaseGameDir, const char *pszPathID, GameData::CStringVector &vecMessages)
{
	return m_aStorage.Load(this, pszBaseGameDir, pszPathID, vecMessages);
}

bool Menu::CProvider::CGameDataStorage::Load(IGameData *pRoot, const char *pszBaseGameDir, const char *pszPathID, GameData::CStringVector &vecMessages)
{
	const struct
	{
		const char *pszFilename;
		bool (Menu::CProvider::CGameDataStorage::*pfnLoad)(IGameData *, KeyValues3 *, GameData::CStringVector &);
	} aConfigs[] =
	{
		{
			MENU_PROVIDER_BASEENTITY_FILENAME,
			&CGameDataStorage::LoadBaseEntity
		},
		{
			MENU_PROVIDER_BASEPLAYERPAWN_FILENAME,
			&CGameDataStorage::LoadBasePlayerPawn
		},
		{
			MENU_PROVIDER_GAMESYSTEM_FILENAME,
			&CGameDataStorage::LoadGameSystem
		},
		{
			MENU_PROVIDER_SOURCE2SERVER_FILENAME,
			&CGameDataStorage::LoadSource2Server
		},
		{
			MENU_PROVIDER_USERCMD_FILENAME,
			&CGameDataStorage::LoadUserCmd
		}
	};

	CBufferStringN<MAX_PATH> sConfigFile;

	CUtlVector<CUtlString> vecConfigFiles;

	CUtlString sError;

	AnyConfig::Anyone aGameConfig;

	AnyConfig::CLoadFromFile_General aLoadPresets({{&sError, NULL, pszPathID}, g_KV3Format_Generic});

	for(const auto &aConfig : aConfigs)
	{
		sConfigFile.Clear();
		sConfigFile.Insert(0, pszBaseGameDir);
		sConfigFile.Insert(sConfigFile.Length(), CORRECT_PATH_SEPARATOR_S);
		sConfigFile.Insert(sConfigFile.Length(), aConfig.pszFilename);

		const char *pszConfigFile = sConfigFile.Get();

		g_pFullFileSystem->FindFileAbsoluteList(vecConfigFiles, pszConfigFile, pszPathID);

		if(vecConfigFiles.Count() < 1)
		{
			const char *pszMessageConcat[] = {"Failed to ", "find \"", pszConfigFile, "\" file"};

			vecMessages.AddToTail({pszMessageConcat});

			continue;
		}

		aLoadPresets.m_pszFilename = vecConfigFiles[0].Get();

		if(!aGameConfig.Load(aLoadPresets)) // Hot.
		{
			const char *pszMessageConcat[] = {"Failed to ", "load \"", pszConfigFile, "\" file", ": ", sError.Get()};

			vecMessages.AddToTail({pszMessageConcat});

			continue;
		}

		if(!(this->*(aConfig.pfnLoad))(pRoot, aGameConfig.Get(), vecMessages))
		{
			const char *pszMessageConcat[] = {"Failed to ", "apply \"", pszConfigFile, "\" file", ": ", sError.Get()};

			vecMessages.AddToTail({pszMessageConcat});

			continue;
		}

		// ...
	}

	return true;
}

bool Menu::CProvider::CGameDataStorage::LoadBaseEntity(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages)
{
	return m_aBaseEntity.Load(pRoot, pGameConfig, vecMessages);
}

bool Menu::CProvider::CGameDataStorage::LoadBasePlayerPawn(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages)
{
	return m_aBasePlayerPawn.Load(pRoot, pGameConfig, vecMessages);
}

bool Menu::CProvider::CGameDataStorage::LoadGameSystem(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages)
{
	return m_aGameSystem.Load(pRoot, pGameConfig, vecMessages);
}

bool Menu::CProvider::CGameDataStorage::LoadSource2Server(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages)
{
	return m_aSource2Server.Load(pRoot, pGameConfig, vecMessages);
}

bool Menu::CProvider::CGameDataStorage::LoadUserCmd(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages)
{
	return m_aUserCmd.Load(pRoot, pGameConfig, vecMessages);
}
