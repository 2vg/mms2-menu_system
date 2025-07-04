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

#ifndef _INCLUDE_METAMOD_SOURCE_MENU_PROVIDER_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_PROVIDER_HPP_

#	pragma once

#	include <stddef.h>
#	include <stdint.h>

#	include <tier0/dbg.h>
#	include <tier0/platform.h>
#	include <tier0/utlscratchmemory.h>
#	include <tier1/bitbuf.h>
#	include <tier1/utldelegateimpl.h>
#	include <tier1/utlmap.h>
#	include <entity2/entitykeyvalues.h>
#	include <igamesystemfactory.h>
#	include <variant.h>

#	include "provider/csgousercmd.hpp"

#	include <gamedata.hpp> // GameData

#	define MENU_PROVIDER_BASE_DIR "gamedata"
#	define MENU_PROVIDER_BASEENTITY_FILENAME MENU_PROVIDER_BASE_DIR CORRECT_PATH_SEPARATOR_S "baseentity.games.*"
#	define MENU_PROVIDER_BASEPLAYERPAWN_FILENAME MENU_PROVIDER_BASE_DIR CORRECT_PATH_SEPARATOR_S "baseplayerpawn.games.*"
#	define MENU_PROVIDER_GAMESYSTEM_FILENAME MENU_PROVIDER_BASE_DIR CORRECT_PATH_SEPARATOR_S "gamesystem.games.*"
#	define MENU_PROVIDER_SOURCE2SERVER_FILENAME MENU_PROVIDER_BASE_DIR CORRECT_PATH_SEPARATOR_S "source2server.games.*"
#	define MENU_PROVIDER_USERCMD_FILENAME MENU_PROVIDER_BASE_DIR CORRECT_PATH_SEPARATOR_S "usercmd.games.*"

class CBaseGameSystemFactory;
class CGameEventManager;
class CBasePlayerController;

namespace Menu
{
	class CProvider : public IGameData
	{
	public:
		CProvider();

	public:
		bool Init(GameData::CStringVector &vecMessages);
		bool Load(const char *pszBaseDir, const char *pszPathID, GameData::CStringVector &vecMessages);
		bool Destroy(GameData::CStringVector &vecMessages);

	protected:
		CUtlSymbolLarge GetSymbol(const char *pszText);
		CUtlSymbolLarge FindSymbol(const char *pszText) const;

	public: // IGameData
		const DynLibUtils::CModule *FindLibrary(const char *pszName) const;

	protected:
		bool LoadGameData(const char *pszBaseGameDir, const char *pszPathID, GameData::CStringVector &vecMessages);

	public:
		class CGameDataStorage
		{
		public:
			bool Load(IGameData *pRoot, const char *pszBaseGameDir, const char *pszPathID, GameData::CStringVector &vecMessages);

		protected:
			bool LoadBaseEntity(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
			bool LoadBasePlayerPawn(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
			bool LoadGameSystem(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
			bool LoadSource2Server(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
			bool LoadUserCmd(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);

		public:
			class CBaseEntity
			{
			public:
				CBaseEntity();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
				void Reset();

			public:
				void AcceptInput(CEntityInstance *pInstance, const char *pInputName, CEntityInstance *pActivator, CEntityInstance *pCaller, variant_t *pValue, int nOutputID) const;
				void Teleport(CEntityInstance *pInstance, const Vector &vecPosition = {}, const QAngle &angRotation = {}, const Vector &velocity = {}) const;

			private:
				GameData::Config::Addresses_t::CListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config::Offsets_t::CListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;

			private: // Addresses.
				using AcceptInput_t = void (CEntityInstance *, const char *, CEntityInstance *, CEntityInstance *, variant_t *, int);

				AcceptInput_t *m_pAcceptInputMethod;
				ptrdiff_t m_nTeleportOffset;
			}; // Menu::CProvider::GameStorage::CBaseEntity

			class CBasePlayerPawn
			{
			public:
				CBasePlayerPawn();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
				void Reset();

			public:
				Vector GetEyePosition(CEntityInstance *pInstance) const;

			private:
				GameData::Config::Offsets_t::CListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;

			private: // Addresses.
				ptrdiff_t m_nGetEyePositionOffset;
			}; // Menu::CProvider::GameStorage::CBasePlayerPawn

			class CGameSystem
			{
			public:
				CGameSystem();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
				void Reset();

			public:
				CBaseGameSystemFactory **GetFirstPointer() const;
				CUtlStringMap<IGameSystem::FactoryInfo_t> *GetFactories() const;
				CUtlVector<AddedGameSystem_t> *GetList() const;
				CGameSystemEventDispatcher **GetEventDispatcher() const;
				CGameSystemEventDispatcher *GetOutOfGameEventDispatcher() const;

			private:
				GameData::Config::Addresses_t::CListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config m_aGameConfig;

			private: // Addresses.
				CBaseGameSystemFactory **m_ppFirst;
				CUtlStringMap<IGameSystem::FactoryInfo_t> *m_pGameSystemFactories;
				CUtlVector<AddedGameSystem_t> *m_pGameSystems;
				CGameSystemEventDispatcher **m_ppEventDispatcher;
				CGameSystemEventDispatcher *m_pOutOfGameEventDispatcher;
			}; // Menu::CProvider::GameStorage::CGameSystem

			class CSource2Server
			{
			public:
				CSource2Server();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
				void Reset();

			public:
				CGameEventManager **GetGameEventManagerPointer() const;

			private:
				GameData::Config::Addresses_t::CListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config m_aGameConfig;

			private: // Addresses.
				CGameEventManager **m_ppGameEventManager;
			}; // Menu::CProvider::GameStorage::CSource2Server

			class CUserCmd
			{
			public:
				CUserCmd();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
				void Reset();

			public:
				CCSGOUserCmd *Get() const;
				void Read(CBasePlayerController *pController, CCSGOUserCmd *pMessage) const;
				void ProcessWithPlayerController(CBasePlayerController *pController, CCSGOUserCmd *cmds, int numcmds, bool paused, float margin) const;

			private:
				GameData::Config::Addresses_t::CListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config m_aGameConfig;

			private: // Addresses.
				using ReadWithPlayerController_t = void (CBasePlayerController *, CCSGOUserCmd *);
				using ProcessWithPlayerController_t = void (CBasePlayerController *, CCSGOUserCmd *, int, bool, float);

				CCSGOUserCmd *m_pCmds;
				ReadWithPlayerController_t *m_pRead;
				ProcessWithPlayerController_t *m_pProcessWithPlayerController;
			}; // Menu::CProvider::GameStorage::CUserCmd

			const CBaseEntity &GetBaseEntity() const
			{
				return m_aBaseEntity;
			}

			const CBasePlayerPawn &GetBasePlayerPawn() const
			{
				return m_aBasePlayerPawn;
			}

			const CGameSystem &GetGameSystem() const
			{
				return m_aGameSystem;
			}

			const CSource2Server &GetSource2Server() const
			{
				return m_aSource2Server;
			}

			const CUserCmd &GetUserCmd() const
			{
				return m_aUserCmd;
			}

		private:
			CBaseEntity m_aBaseEntity;
			CBasePlayerPawn m_aBasePlayerPawn;
			CGameSystem m_aGameSystem;
			CSource2Server m_aSource2Server;
			CUserCmd m_aUserCmd;
		}; // Menu::CProvider::CGameDataStorage

		const CGameDataStorage &GetGameDataStorage() const
		{
			return m_aStorage;
		}

	private:
		CUtlSymbolTableLarge_CI m_aSymbolTable;
		CUtlMap<CUtlSymbolLarge, DynLibUtils::CModule *> m_mapLibraries;

	private:
		CGameDataStorage m_aStorage;

	private:
		DynLibUtils::CModule m_aEngine2Library, 
		                     m_aFileSystemSTDIOLibrary, 
		                     m_aServerLibrary;
	}; // Menu::CProvider
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_PROVIDER_HPP_
