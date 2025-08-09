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

#include <menusystem_plugin.hpp>
#include <menu/profile.hpp>
#include <globals.hpp>
#include <math.hpp>

#include <stdint.h>

#include <algorithm>
#include <exception>
#include <functional>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <any_config.hpp>

#include <sourcehook/sourcehook.h>

#include <filesystem.h>
#include <igameeventsystem.h>
#include <inetchannel.h>
#include <networksystem/inetworkmessages.h>
#include <networksystem/inetworkserializer.h>
#include <recipientfilter.h>
#include <serversideclient.h>
#include <shareddefs.h>
#include <tier0/dbg.h>
#include <tier0/commonmacros.h>
#include <tier0/utlstringtoken.h>
#include <tier0/memalloc.h>
#include <tier1/convar.h>
#include <mathlib/mathlib.h>

#include <netmessages.h>
#include <usermessages.h>
// #include <cstrike15/usermessages.h>
#include <connectionless_netmessages.pb.h>

// <cstrike15/cs_shareddefs.h> BEGIN
// CS Team IDs.
#define TEAM_TERRORIST			2
#define	TEAM_CT					3
#define TEAM_MAXCOUNT			4	// update this if we ever add teams (unlikely)

#define TEAM_TERRORIST_BASE0	TEAM_TERRORIST - 2
#define TEAM_CT_BASE0			TEAM_CT - 2
// <cstrike15/cs_shareddefs.h> END

#define EF_MENU EF_BONEMERGE | EF_BRIGHTLIGHT | EF_DIMLIGHT | EF_NOINTERP | EF_NOSHADOW | EF_NODRAW | EF_NORECEIVESHADOW | EF_BONEMERGE_FASTCULL | EF_ITEM_BLINK | EF_PARENT_ANIMATES

SH_DECL_HOOK3_void(ICvar, DispatchConCommand, SH_NOATTRIB, 0, ConCommandRef, const CCommandContext &, const CCommand &);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);
SH_DECL_HOOK8_void(ISource2GameEntities, CheckTransmit, SH_NOATTRIB, 0, CCheckTransmitInfo **, int, CBitVec<MAX_EDICTS> &, CBitVec<MAX_EDICTS> &, const Entity2Networkable_t **, const uint16 *, int, bool);
SH_DECL_HOOK0_void(CNetworkGameServerBase, Release, SH_NOATTRIB, 0);
SH_DECL_HOOK8(CNetworkGameServerBase, ConnectClient, SH_NOATTRIB, 0, CServerSideClientBase *, const char *, ns_address *, uint32, const C2S_CONNECT_Message &, const char *, const byte *, int, bool);
SH_DECL_HOOK1(CServerSideClientBase, ExecuteStringCommand, SH_NOATTRIB, 0, bool, const CNETMsg_StringCmd_t &);
SH_DECL_HOOK1(CServerSideClientBase, ProcessRespondCvarValue, SH_NOATTRIB, 0, bool, const CCLCMsg_RespondCvarValue_t &);
SH_DECL_HOOK1(CServerSideClientBase, ProcessMove, SH_NOATTRIB, 0, bool, const CCLCMsg_Move_t &);
SH_DECL_HOOK1_void(CServerSideClientBase, PerformDisconnection, SH_NOATTRIB, 0, ENetworkDisconnectionReason);

static MenuSystem_Plugin s_aMenuPlugin;
MenuSystem_Plugin *g_pMenuPlugin = &s_aMenuPlugin;

PLUGIN_EXPOSE(MenuSystem_Plugin, s_aMenuPlugin);

MenuSystem_Plugin::MenuSystem_Plugin()
 :  CLogger(GetName(), [](LoggingChannelID_t nTagChannelID)
    {
    	LoggingSystem_AddTagToChannel(nTagChannelID, s_aMenuPlugin.GetLogTag());
    }, 0, LV_DETAILED, MENUSYSTEM_LOGGINING_COLOR),

    m_aEnableClientCommandDetailsConVar("mm_" META_PLUGIN_PREFIX "_enable_client_command_details", FCVAR_RELEASE | FCVAR_GAMEDLL, "Enable client command detial messages", false, true, false, true, true),
    m_aEnablePlayerRunCmdDetailsConVar("mm_" META_PLUGIN_PREFIX "_enable_player_runcmd_details", FCVAR_RELEASE | FCVAR_GAMEDLL, "Enable player usercmds detial messages", false, true, false, true, true),
    m_aEnableSilentCommandDispatchConVar("mm_" META_PLUGIN_PREFIX "_enable_silent_command_dispatch", FCVAR_RELEASE | FCVAR_GAMEDLL, "Enable dispatching silent commands to other plugins", true, true, false, true, true),

    m_mapConVarCookies(DefLessFunc(const CUtlSymbolLarge)),
    m_mapLanguages(DefLessFunc(const CUtlSymbolLarge)),

    m_pEntityManager(nullptr),
    m_pEntityManagerProviderAgent(nullptr),
    m_pEntityManagerSpawnGroupProvider(nullptr),

    m_pMySpawnGroupInstance(nullptr),
    m_pSetConVarMessage(NULL),
    m_pGetCvarValueMessage(NULL),
    m_pSayText2Message(NULL),
    m_pTextMsgMessage(NULL),
    // m_pVGUIMenuMessage(NULL),

    m_aBackControlItem(CMenu::MENU_ITEM_FULL, "Back"),
    m_aNextControlItem(CMenu::MENU_ITEM_FULL, "Next"),
    m_aExitControlItem(CMenu::MENU_ITEM_FULL, "Exit"),
    m_aControls({&m_aBackControlItem, &m_aNextControlItem, &m_aExitControlItem}),

    m_mapMenuHandlers(DefLessFunc(const IMenu *))
{
	// Adds schema listeners.
	{
		Menu::Schema::CSystem *pSchemaHelper = static_cast<Menu::Schema::CSystem*>(this);
	
		CBaseEntity_Helper::AddListeners(pSchemaHelper);
		CBaseModelEntity_Helper::AddListeners(pSchemaHelper);
		CBasePlayerController_Helper::AddListeners(pSchemaHelper);
		CBasePlayerPawn_Helper::AddListeners(pSchemaHelper);
		CBasePlayerWeapon_Helper::AddListeners(pSchemaHelper);
		CBasePlayerWeaponVData_Helper::AddListeners(pSchemaHelper);
		CBaseViewModel_Helper::AddListeners(pSchemaHelper);
		CBodyComponent_Helper::AddListeners(pSchemaHelper);
		CCSPlayerPawnBase_Helper::AddListeners(pSchemaHelper);
		CCSWeaponBaseVData_Helper::AddListeners(pSchemaHelper);
		CCSObserverPawn_Helper::AddListeners(pSchemaHelper);
		CCSPlayer_ViewModelServices_Helper::AddListeners(pSchemaHelper);
		CCSPlayerBase_CameraServices_Helper::AddListeners(pSchemaHelper);
		CCSPlayerPawn_Helper::AddListeners(pSchemaHelper);
		CGameSceneNode_Helper::AddListeners(pSchemaHelper);
		CPlayer_ObserverServices_Helper::AddListeners(pSchemaHelper);
		CPlayer_WeaponServices_Helper::AddListeners(pSchemaHelper);
		CPointWorldText_Helper::AddListeners(pSchemaHelper);
	}

	// Game events.
	{
		CGameEventSystem::AddHandler("player_team", {[&](const CUtlSymbolLarge &sName, IGameEvent *pEvent) -> bool
		{
			auto aPlayerSlot = pEvent->GetPlayerSlot("userid");

			if(aPlayerSlot.IsValid())
			{
				return false;
			}

			auto &aPlayerData = GetPlayerData(aPlayerSlot);

			const auto &vecMenus = aPlayerData.GetMenus();

			if(!vecMenus.Count())
			{
				return false;
			}

			auto *pPlayerPawn = instance_upper_cast<CBasePlayerPawn *>(pEvent->GetPlayerPawn("userid"));

			if(!pPlayerPawn)
			{
				return false;
			}

			if(pEvent->GetBool("disconnect") || pEvent->GetBool("isbot"))
			{
				return false;
			}

			int iNewTeam = pEvent->GetInt("team"), 
			    iOldTeam = pEvent->GetInt("oldteam");

			if(iNewTeam <= TEAM_SPECTATOR)
			{
				auto *pTargetPlayerPawn = instance_upper_cast<CCSPlayerPawnBase *>(pPlayerPawn);

				CPlayer_ObserverServices *pObserverServices = CCSPlayerPawnBase_Helper::GetObserverServicesAccessor(pTargetPlayerPawn);

				if(!pObserverServices)
				{
					return false;
				}

				uint8 iObserverMode = CPlayer_ObserverServices_Helper::GetObserverModeAccessor(pObserverServices);

				if(iObserverMode != OBS_MODE_NONE && iObserverMode != OBS_MODE_ROAMING)
				{
					CHandle<CBaseEntity> hObserverTarget = CPlayer_ObserverServices_Helper::GetObserverTargetAccessor(pObserverServices);

					auto *pCSObserverTarget = hObserverTarget.Get();

					if(!pCSObserverTarget)
					{
						return false;
					}

					CCSPlayerPawn *pCCSPlayerObserverTargetPawn = instance_upper_cast<CCSPlayerPawn *>(pCSObserverTarget);

					FOR_EACH_VEC(vecMenus, i)
					{
						const auto &[_, pMenu] = vecMenus[i];

						CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

						if(pInternalMenu)
						{
							AttachMenuInstanceToCSPlayer(i, pInternalMenu, pCCSPlayerObserverTargetPawn);
						}
					}

					return true;
				}

				for(const auto &[_, pMenu] : vecMenus)
				{
					CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

					if(pInternalMenu)
					{
						AttachMenuInstanceToEntity(pInternalMenu, pTargetPlayerPawn);
					}
				}

				return true;
			}
			else
			{
				auto *pCSPlayerPawn = instance_upper_cast<CCSPlayerPawn *>(pPlayerPawn);

				FOR_EACH_VEC(vecMenus, i)
				{
					const auto &[_, pMenu] = vecMenus[i];

					CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

					if(pInternalMenu)
					{
						AttachMenuInstanceToCSPlayer(i, pInternalMenu, pCSPlayerPawn);
					}
				}
			}

			return true;
		}});
	}

	/*
	// Removed test chat commands.
	*/
}

bool MenuSystem_Plugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	CLogger::MessageFormat("Starting %s plugin...\n", GetName());

	if(!InitGlobals(ismm, error, maxlen))
	{
		return false;
	}

	if(CLogger::IsChannelEnabled(LS_DETAILED))
	{
		CBufferStringN<1024> sMessage;

		DumpGlobals(g_aEmbedConcat, sMessage);
		CLogger::Detailed(sMessage);
	}

	MathLib_Init();
	ConVar_Register(FCVAR_RELEASE | FCVAR_GAMEDLL);

	if(!InitSchema(error, maxlen))
	{
		return false;
	}

	if(!LoadSchema(error, maxlen))
	{
		return false;
	}

	if(!InitPathResolver(error, maxlen))
	{
		return false;
	}

	if(CLogger::IsChannelEnabled(LS_DETAILED))
	{
		CBufferStringN<1024> sMessage;

		sMessage.Insert(0, "Path resolver:\n");
		CConcatLineBuffer(&g_aEmbedConcat, &sMessage).Append("Base game directory", m_sBaseGameDirectory.c_str());
		CLogger::Detailed(sMessage);
	}

	if(!InitProvider(error, maxlen))
	{
		return false;
	}

	if(!LoadProvider(error, maxlen))
	{
		return false;
	}

	if(!LoadProfiles(error, maxlen))
	{
		return false;
	}

	if(CLogger::IsChannelEnabled(LS_DETAILED))
	{
		CBufferStringN<1024> sMessage;

		sMessage.Insert(0, "Entity manager:\n");
		DumpEntityManager(g_aEmbedConcat, sMessage);
		CLogger::Detailed(sMessage);
	}

	if(!ParseLanguages(error, maxlen))
	{
		return false;
	}

	if(!ParseTranslations(error, maxlen))
	{
		return false;
	}

	if (!LoadChat(error, maxlen))
	{
		return false;
	}

	if(!RegisterGameFactory(error, maxlen))
	{
		return false;
	}

	SH_ADD_HOOK(ICvar, DispatchConCommand, g_pCVar, SH_MEMBER(this, &MenuSystem_Plugin::OnDispatchConCommandHook), false);
	SH_ADD_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &MenuSystem_Plugin::OnStartupServerHook), true);
	SH_ADD_HOOK(ISource2GameEntities, CheckTransmit, g_pSource2GameEntities, SH_MEMBER(this, &MenuSystem_Plugin::OnCheckTransmitHook), true);

	if(late)
	{
		// Initialize a game resource & load menu spawn groups.
		{
			char sMessage[256];
		
			bool (MenuSystem_Plugin::*pfnIntializers[])(char *error, size_t maxlen) = 
			{
				&MenuSystem_Plugin::InitEntityManager,
				&MenuSystem_Plugin::RegisterGameResource,
				&MenuSystem_Plugin::LoadSpawnGroups,
			};
		
			for(const auto &aInitializer : pfnIntializers)
			{
				if(!(this->*(aInitializer))(sMessage, sizeof(sMessage)))
				{
					CLogger::WarningFormat("%s\n", sMessage);
				}
			}
		}

		auto *pNetServer = reinterpret_cast<CNetworkGameServerBase *>(g_pNetworkServerService->GetIGameServer());

		if(pNetServer)
		{
			OnStartupServer(pNetServer, pNetServer->m_GameConfig, NULL);

			for(const auto &pClient : pNetServer->m_Clients)
			{
				if(pClient->IsConnected())
				{
					OnConnectClient(pNetServer, pClient);
				}
			}
		}
	}

	ismm->AddListener(static_cast<ISmmPlugin *>(this), static_cast<IMetamodListener *>(this));

	CLogger::MessageFormat("%s started!\n", GetName());

	return true;
}

bool MenuSystem_Plugin::Unload(char *error, size_t maxlen)
{
	{
		auto *pNetServer = reinterpret_cast<CNetworkGameServerBase *>(g_pNetworkServerService->GetIGameServer());

		if(pNetServer)
		{
			SH_REMOVE_HOOK(CNetworkGameServerBase, ConnectClient, pNetServer, SH_MEMBER(this, &MenuSystem_Plugin::OnConnectClientHook), true);
		}
	}

	SH_REMOVE_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &MenuSystem_Plugin::OnStartupServerHook), true);
	SH_REMOVE_HOOK(ISource2GameEntities, CheckTransmit, g_pSource2GameEntities, SH_MEMBER(this, &MenuSystem_Plugin::OnCheckTransmitHook), true);
	SH_REMOVE_HOOK(ICvar, DispatchConCommand, g_pCVar, SH_MEMBER(this, &MenuSystem_Plugin::OnDispatchConCommandHook), false);

	if(!UnhookGameEvents(error, maxlen))
	{
		return false;
	}

	if(!ClearChat(error, maxlen))
	{
		return false;
	}

	if(!ClearProfiles(error, maxlen))
	{
		return false;
	}

	if(!ClearLanguages(error, maxlen))
	{
		return false;
	}

	if(!ClearTranslations())
	{
		return false;
	}

	if(!UnloadProvider(error, maxlen))
	{
		return false;
	}

	if(!UnregisterNetMessages(error, maxlen))
	{
		return false;
	}

	if(!UnregisterSource2Server(error, maxlen))
	{
		return false;
	}

	if(!UnregisterGameFactory(error, maxlen))
	{
		return false;
	}

	if(!UnloadSpawnGroupsNow(error, maxlen))
	{
		return false;
	}

	if(!UnregisterGameResource(error, maxlen))
	{
		return false;
	}

	if(!DestoryGlobals(error, maxlen))
	{
		return false;
	}

	if(!UnloadEntityManager(error, maxlen))
	{
		return false;
	}

	if(!ClearPathResolver(error, maxlen))
	{
		return false;
	}

	if(!ClearSchema(error, maxlen))
	{
		return false;
	}

	m_MenuAllocator.Purge();
	m_mapMenuHandlers.Purge();

	ConVar_Unregister();

	// ...

	return true;
}

bool MenuSystem_Plugin::Pause(char *error, size_t maxlen)
{
	return true;
}

bool MenuSystem_Plugin::Unpause(char *error, size_t maxlen)
{
	return true;
}

void MenuSystem_Plugin::AllPluginsLoaded()
{
	char error[256];

	if(!InitEntityManager(error, sizeof(error)))
	{
		CLogger::WarningFormat("%s\n", error);
	}
}

const char *MenuSystem_Plugin::GetAuthor()        { return META_PLUGIN_AUTHOR; }
const char *MenuSystem_Plugin::GetName()          { return META_PLUGIN_NAME; }
const char *MenuSystem_Plugin::GetDescription()   { return META_PLUGIN_DESCRIPTION; }
const char *MenuSystem_Plugin::GetURL()           { return META_PLUGIN_URL; }
const char *MenuSystem_Plugin::GetLicense()       { return META_PLUGIN_LICENSE; }
const char *MenuSystem_Plugin::GetVersion()       { return META_PLUGIN_VERSION; }
const char *MenuSystem_Plugin::GetDate()          { return META_PLUGIN_DATE; }
const char *MenuSystem_Plugin::GetLogTag()        { return META_PLUGIN_LOG_TAG; }

void MenuSystem_Plugin::OnPluginUnload(PluginId id)
{
	if(id == m_iEntityManager)
	{
		char error[256];

		if(!UnloadSpawnGroupsNow(error, sizeof(error)))
		{
			CLogger::WarningFormat("%s\n", error);
		}

		if(!UnloadEntityManager(error, sizeof(error)))
		{
			CLogger::WarningFormat("%s\n", error);
		}
	}
}

void *MenuSystem_Plugin::OnMetamodQuery(const char *iface, int *ret)
{
	if(!strcmp(iface, MENUSYSTEM_INTERFACE_NAME))
	{
		if(ret)
		{
			*ret = META_IFACE_OK;
		}

		return static_cast<IMenuSystem *>(this);
	}

	if(ret)
	{
		*ret = META_IFACE_FAILED;
	}

	return nullptr;
}

CGameEntitySystem **MenuSystem_Plugin::GetGameEntitySystemPointer() const
{
	return &g_pGameEntitySystem;
}

CBaseGameSystemFactory **MenuSystem_Plugin::GetFirstGameSystemPointer() const
{
	return GetGameDataStorage().GetGameSystem().GetFirstPointer();
}

CUtlStringMap<IGameSystem::FactoryInfo_t> *MenuSystem_Plugin::GetGameSystemFactoriesPointer() const
{
	return GetGameDataStorage().GetGameSystem().GetFactories();
}

CUtlVector<AddedGameSystem_t> *MenuSystem_Plugin::GetGameSystemsPointer() const
{
	return GetGameDataStorage().GetGameSystem().GetList();
}

CGameSystemEventDispatcher **MenuSystem_Plugin::GetGameSystemEventDispatcherPointer() const
{
	return GetGameDataStorage().GetGameSystem().GetEventDispatcher();
}

CGameSystemEventDispatcher *MenuSystem_Plugin::GetOutOfGameEventDispatcher() const
{
	return GetGameDataStorage().GetGameSystem().GetOutOfGameEventDispatcher();
}

IGameEventManager2 **MenuSystem_Plugin::GetGameEventManagerPointer() const
{
	return reinterpret_cast<IGameEventManager2 **>(GetGameDataStorage().GetSource2Server().GetGameEventManagerPointer());
}

const ISample::ILanguage *MenuSystem_Plugin::GetLanguageByName(const char *psz) const
{
	auto iFound = m_mapLanguages.Find(FindLanguageSymbol(psz));

	return m_mapLanguages.IsValidIndex(iFound) ? &m_mapLanguages.Element(iFound) : nullptr;
}

ISample::IPlayerBase *MenuSystem_Plugin::GetPlayerBase(const CPlayerSlot &aSlot)
{
	return GetPlayer(aSlot);
}

IMenuSystem::IPlayer *MenuSystem_Plugin::GetPlayer(const CPlayerSlot &aSlot)
{
	return &GetPlayerData(aSlot);
}

MenuSystem_Plugin::CPlayer &MenuSystem_Plugin::GetPlayerData(const CPlayerSlot &aSlot)
{
	int iSlot = aSlot.Get();

	Assert(0 <= iSlot && iSlot < sizeof(m_aPlayers));

	return m_aPlayers[iSlot];
}

int MenuSystem_Plugin::FindItemIndexFromClientIndex(int iClient)
{
	if(!(0 < iClient && iClient <= ABSOLUTE_PLAYER_LIMIT))
	{
		return -1;
	}

	std::vector<int> vecCTClients, // CT side is the first for CCSGO_HudTeamCounter.
	                 vecTClients;

	auto *pNetServer = reinterpret_cast<CNetworkGameServerBase *>(g_pNetworkServerService->GetIGameServer());

	auto &vecClients = pNetServer->m_Clients;

	int nClients = vecClients.Count();

	vecCTClients.reserve(nClients);
	vecTClients.reserve(nClients);

	for(const auto &pClient : pNetServer->m_Clients)
	{
		if(!pClient->IsConnected())
		{
			continue;
		}

		auto aPlayerSlot = pClient->GetPlayerSlot();

		auto *pPlayerController = instance_upper_cast<CBasePlayerController *>(g_pEntitySystem->GetEntityInstance(CEntityIndex(aPlayerSlot.GetClientIndex())));

		if(!pPlayerController)
		{
			continue;
		}

		uint8 iTeam = CBasePlayerController_Helper::GetTeamNumAccessor(pPlayerController);

		int iTeamClient = aPlayerSlot.GetClientIndex();

		if(iTeam == TEAM_CT)
		{
			vecCTClients.push_back(iTeamClient);
		}
		else if(iTeam == TEAM_TERRORIST)
		{
			vecTClients.push_back(iTeamClient);
		}
	}

	auto it = std::find(vecCTClients.cbegin(), vecCTClients.cend(), iClient);

	if(it != vecCTClients.cend())
	{
		return std::distance(vecCTClients.cbegin(), it) + 1;
	}

	it = std::find(vecTClients.cbegin(), vecTClients.cend(), iClient);

	if(it != vecTClients.cend())
	{
		return std::distance(vecTClients.cbegin(), it) + vecCTClients.size() + 1;
	}

	return -1; // Not found.
}

int MenuSystem_Plugin::FindItemIndexFromMyWeapons(int iClient, int iEntity)
{
	auto *pPlayerController = instance_upper_cast<CBasePlayerController *>(g_pEntitySystem->GetEntityInstance(CEntityIndex(iClient)));

	if(!pPlayerController)
	{
		return -1;
	}

	CHandle<CBasePlayerPawn> hPlayerPawn = CBasePlayerController_Helper::GetPawnAccessor(pPlayerController);

	if(!hPlayerPawn.IsValid())
	{
		return -1;
	}

	CBasePlayerPawn *pPlayerPawn = hPlayerPawn.Get();

	CPlayer_WeaponServices *pPlayerWeaponServices = CBasePlayerPawn_Helper::GetWeaponServicesAccessor(pPlayerPawn);

	if(!pPlayerWeaponServices)
	{
		return -1;
	}

	CNetworkUtlVectorBase<CHandle<CBasePlayerWeapon>> vecMyWeapons = CPlayer_WeaponServices_Helper::GetMyWeaponsAccessor(pPlayerWeaponServices);

	for(const auto &hPlayerWeapon : vecMyWeapons)
	{
		int iWeaponIndex = hPlayerWeapon.GetEntryIndex();

		if(iWeaponIndex == -1 || iWeaponIndex != iEntity)
		{
			continue;
		}

		CBasePlayerWeaponVData *pPlayerWeaponVData = CBasePlayerWeapon_Helper::GetEntitySubclassVDataAccessor(hPlayerWeapon.Get());

		if(!pPlayerWeaponVData)
		{
			continue;
		}

		return CCSWeaponBaseVData_Helper::GetGearSlotAccessor(component_upper_cast<CCSWeaponBaseVData *>(pPlayerWeaponVData)) + 1;
	}

	return -1;
}

IMenuProfileSystem *MenuSystem_Plugin::GetProfiles()
{
	return static_cast<IMenuProfileSystem *>(this);
}

IMenu *MenuSystem_Plugin::CreateInstance(IMenuProfile *pProfile, IMenuHandler *pHandler)
{
	return static_cast<IMenu *>(CreateInternalMenu(pProfile, pHandler));
}

bool MenuSystem_Plugin::DisplayInstanceToPlayer(IMenu *pMenu, CPlayerSlot aSlot, IMenu::ItemPosition_t iStartItem, int nManyTimes)
{
	CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

	if(!pInternalMenu)
	{
		return false;
	}

	return DisplayInternalMenuToPlayer(pInternalMenu, aSlot, iStartItem, nManyTimes);
}

bool MenuSystem_Plugin::CloseInstance(IMenu *pMenu)
{
	auto *pMemBlock = m_MenuAllocator.FindMemBlock(pMenu);

	if(!pMemBlock)
	{
		return false;
	}

	CMenu *pInternalMenu = m_MenuAllocator.GetInstanceByMemBlock(pMemBlock);

	CloseInternalMenu(pInternalMenu, IMenuHandler::MenuEnd_Close);
	m_MenuAllocator.ReleaseByMemBlock(pMemBlock);

	return false;
}

CMenu *MenuSystem_Plugin::CreateInternalMenu(IMenuProfile *pProfile, IMenuHandler *pHandler)
{
	auto *pNewMenu = m_MenuAllocator.CreateInstance(static_cast<CMenu::CPointWorldText_Helper *>(this), &GetGameDataStorage().GetBaseEntity(), pProfile, static_cast<IMenuHandler *>(this), &m_aControls);

	m_mapMenuHandlers.InsertOrReplace(pNewMenu, pHandler);

	return pNewMenu;
}

bool MenuSystem_Plugin::UpdatePlayerMenus(CPlayerSlot aSlot)
{
	auto &aPlayer = GetPlayerData(aSlot);

	auto &vecMenus = aPlayer.GetMenus();

	const int nMenuCount = vecMenus.Count();

	if(!nMenuCount)
	{
		return false;
	}

	auto *pPlayerController = instance_upper_cast<CBasePlayerController *>(g_pEntitySystem->GetEntityInstance(CEntityIndex(aSlot.GetClientIndex())));

	if(!pPlayerController)
	{
		return false;
	}

	CBasePlayerPawn *pPlayerPawn = CBasePlayerController_Helper::GetPawnAccessor(pPlayerController)->Get();

	if(!pPlayerPawn)
	{
		return false;
	}

	const IMenu::Index_t iActiveMenu = aPlayer.GetActiveMenuIndex();

	uint8 iTeam = CBaseEntity_Helper::GetTeamNumAccessor(pPlayerController);

	auto *pCSPlayerPawnBase = instance_upper_cast<CCSPlayerPawnBase *>(pPlayerPawn);

	for(int i = 0; i < nMenuCount; i++)
	{
		const auto &[_, pMenu] = vecMenus.Element(i);

		CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

		if(pInternalMenu)
		{
			int iShift = i - iActiveMenu;

			if(iTeam <= TEAM_SPECTATOR)
			{
				AttachMenuInstanceToObserver(iShift, pInternalMenu, pCSPlayerPawnBase);
			}
			else
			{
				AttachMenuInstanceToCSPlayer(iShift, pInternalMenu, instance_upper_cast<CCSPlayerPawn *>(pCSPlayerPawnBase));
			}

			pInternalMenu->InternalDisplayAt(aSlot, pInternalMenu->GetCurrentPosition(aSlot), i == iActiveMenu ? IMenu::MENU_DISPLAY_DEFAULT : IMenu::MENU_DISPLAY_RENDER_BASE_UPDATE);
		}
	}

	return false;
}

bool MenuSystem_Plugin::DisplayInternalMenuToPlayer(CMenu *pInternalMenu, CPlayerSlot aSlot, IMenu::ItemPosition_t iStartItem, int nManyTimes)
{
	auto &aPlayer = GetPlayerData(aSlot);

	int iClient = aSlot.GetClientIndex();

	if(!aPlayer.IsConnected())
	{
		CLogger::WarningFormat("Player is not connected. Client index is %d\n", iClient);

		return false;
	}

	auto *pPlayerController = instance_upper_cast<CBasePlayerController *>(g_pEntitySystem->GetEntityInstance(CEntityIndex(iClient)));

	if(!pPlayerController)
	{
		CLogger::WarningFormat("Failed to get a player entity controller. Client index is %d\n", iClient);

		return false;
	}

	CBasePlayerPawn *pPlayerPawn = CBasePlayerController_Helper::GetPawnAccessor(pPlayerController)->Get();

	if(!pPlayerPawn)
	{
		CLogger::WarningFormat("Failed to get a player pawn. Client index is %d\n", iClient);

		return false;
	}

	// Disable a radar.
	{
		CSingleRecipientFilter aFilter(aSlot);

		CUtlVector<CVar_t> vecCVars(1);

		vecCVars.AddToTail({MENUSYSTEM_SERVER_DISABLE_RADAR_CVAR_NAME, "1"});
		SendSetConVarMessage(&aFilter, vecCVars);
	}

	SpawnMenuByEntityPosition(0, pInternalMenu, aSlot, pPlayerPawn);

	auto &vecMenus = aPlayer.GetMenus();

	uint8 iTeam = CBaseEntity_Helper::GetTeamNumAccessor(pPlayerController);

	IMenu::Index_t &iActiveMenu = aPlayer.GetActiveMenuIndexRef();

	IPlayer::MenuData_t aMenuData {nManyTimes ? Plat_GetTime() + nManyTimes : 0, static_cast<IMenu *>(pInternalMenu)}; // Move semantics?

	if(iActiveMenu == -1)
	{
		vecMenus.AddToHead(aMenuData);
		iActiveMenu = 0;
	}
	else
	{
		vecMenus.InsertBefore(iActiveMenu, aMenuData);
	}

	UpdatePlayerMenus(aSlot);

	return pInternalMenu->InternalDisplayAt(aSlot, iStartItem);
}

IMenuHandler *MenuSystem_Plugin::FindMenuHandler(IMenu *pMenu)
{
	int iFound = m_mapMenuHandlers.Find(pMenu);

	return iFound == m_mapMenuHandlers.InvalidIndex() ? nullptr : m_mapMenuHandlers[iFound];
}

int MenuSystem_Plugin::DestroyInternalMenuEntities(CMenu *pInternalMenu)
{
	for(auto *pMenuEntity : *pInternalMenu)
	{
		m_pEntityManagerProviderAgent->PushDestroyQueue(pMenuEntity);
	}

	int iDestroyedCount = m_pEntityManagerProviderAgent->ExecuteDestroyQueued();

	if(iDestroyedCount)
	{
		pInternalMenu->CMenuBase::Purge();
	}

	return iDestroyedCount;
}

bool MenuSystem_Plugin::CloseMenuHandler(IMenu *pMenu)
{
	return m_mapMenuHandlers.Remove(pMenu);
}

void MenuSystem_Plugin::CloseInternalMenu(CMenu *pInternalMenu, IMenuHandler::EndReason_t eReason, bool bCleanupPlayer)
{
	pInternalMenu->Close(eReason);
	DestroyInternalMenuEntities(pInternalMenu);

	IMenu *pMenu = static_cast<IMenu *>(pInternalMenu);

	CloseMenuHandler(pMenu);

	if(!bCleanupPlayer)
	{
		return;
	}

	// Clean menu mention from players.
	for(auto &aPlayer : m_aPlayers)
	{
		if(!aPlayer.IsConnected())
		{
			continue;
		}

		IMenu::Index_t &iActiveMenu = aPlayer.GetActiveMenuIndexRef();

		auto &vecMenus = aPlayer.GetMenus();

		FOR_EACH_VEC_BACK(vecMenus, i)
		{
			const auto &[_, pPlayerMenu] = vecMenus.Element(i);

			if(pMenu == pPlayerMenu)
			{
				if(i == iActiveMenu)
				{
					iActiveMenu--;
				}

				vecMenus.Remove(i);

				break;
			}
		}

		if(vecMenus.Count())
		{
			if(iActiveMenu == MENU_INVLID_INDEX)
			{
				iActiveMenu = 0;
			}

			UpdatePlayerMenus(aPlayer.GetServerSideClient()->GetPlayerSlot());
		}
	}
}

void MenuSystem_Plugin::PurgeAllMenus()
{
	for(auto &aPlayer : m_aPlayers)
	{
		if(!aPlayer.IsConnected())
		{
			continue;
		}

		auto &vecMenus = aPlayer.GetMenus();

		for(const auto &[_, pMenu] : vecMenus)
		{
			CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

			if(!pInternalMenu)
			{
				continue;
			}

			CloseInternalMenu(pInternalMenu, IMenuHandler::MenuEnd_Disconnected, false);
		}

		vecMenus.Purge();
	}

	m_MenuAllocator.PurgeAndDeleteElements();
}

void MenuSystem_Plugin::OnMenuStart(IMenu *pMenu)
{
	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("%s(pMenu = %p)\n", __FUNCTION__, pMenu);
	}

	auto *pHandler = FindMenuHandler(pMenu);

	if(pHandler)
	{
		pHandler->OnMenuStart(pMenu);
	}
}

void MenuSystem_Plugin::OnMenuDisplay(IMenu *pMenu, CPlayerSlot aSlot)
{
	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("%s(pMenu = %p, iClient = %d)\n", __FUNCTION__, pMenu, aSlot.GetClientIndex());
	}

	auto *pHandler = FindMenuHandler(pMenu);

	if(pHandler)
	{
		pHandler->OnMenuDisplay(pMenu, aSlot);
	}
}

void MenuSystem_Plugin::OnMenuSelect(IMenu *pMenu, CPlayerSlot aSlot, IMenu::ItemPosition_t iItem)
{
	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("%s(pMenu = %p, iClient = %d, iItem = %d)\n", __FUNCTION__, pMenu, aSlot.GetClientIndex(), iItem);
	}

	auto *pHandler = FindMenuHandler(pMenu);

	if(pHandler)
	{
		pHandler->OnMenuSelect(pMenu, aSlot, iItem);
	}

	if(iItem == IMenu::MENU_ITEM_CONTROL_EXIT_INDEX)
	{
		OnMenuExitButton(pMenu, aSlot, iItem);
	}
}

void MenuSystem_Plugin::OnMenuEnd(IMenu *pMenu, EndReason_t eReason)
{
	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("%s(pMenu = %p, eReason = %d)\n", __FUNCTION__, pMenu, eReason);
	}

	auto *pHandler = FindMenuHandler(pMenu);

	if(pHandler)
	{
		pHandler->OnMenuEnd(pMenu, eReason);
	}
}

bool MenuSystem_Plugin::OnMenuExitButton(IMenu *pMenu, CPlayerSlot aSlot, IMenu::ItemPosition_t iItem)
{
	auto *pMenuMemBlock = m_MenuAllocator.FindMemBlock(pMenu);

	if(!pMenuMemBlock)
	{
		return false;
	}

	CMenu *pInternalMenu = m_MenuAllocator.GetInstanceByMemBlock(pMenuMemBlock);

	CloseInternalMenu(pInternalMenu, IMenuHandler::MenuEnd_Exit);
	m_MenuAllocator.ReleaseByMemBlock(pMenuMemBlock);

	UpdatePlayerMenus(aSlot);

	return true;
}

bool MenuSystem_Plugin::OnMenuSwitch(CPlayerSlot aSlot)
{
	auto &aPlayer = GetPlayerData(aSlot);

	if(!aPlayer.OnMenuSwitch(aSlot))
	{
		return false;
	}

	UpdatePlayerMenus(aSlot);

	return true;
}

void MenuSystem_Plugin::OnMenuDestroy(IMenu *pMenu)
{
	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("%s(pMenu = %p)\n", __FUNCTION__, pMenu);
	}

	// Enable a radar back.
	CConVarRef<int> aSVDisableRadar(MENUSYSTEM_SERVER_DISABLE_RADAR_CVAR_NAME);

	if(!aSVDisableRadar.Get())
	{
		bool bSendDisableRadar = true;

		CRecipientFilter aFilter;

		// Find menu interface in players.
		for(auto &aPlayer : m_aPlayers)
		{
			if(!aPlayer.IsConnected())
			{
				continue;
			}

			auto &vecMenus = aPlayer.GetMenus();

			if(vecMenus.Count() != 1) // Pass mutlimenu.
			{
				continue;
			}

			if(pMenu == vecMenus[0].m_pInstance)
			{
				aFilter.AddRecipient(aPlayer.GetServerSideClient()->GetPlayerSlot());
			}
		}

		if(aFilter.GetRecipientCount()) // If found added recipient players.
		{
			CUtlVector<CVar_t> vecCVars(1);

			vecCVars.AddToTail({MENUSYSTEM_SERVER_DISABLE_RADAR_CVAR_NAME, "0"});
			SendSetConVarMessage(&aFilter, vecCVars);
		}
	}

	auto *pHandler = FindMenuHandler(pMenu);

	if(pHandler)
	{
		pHandler->OnMenuDestroy(pMenu);
	}
}

void MenuSystem_Plugin::OnMenuDrawTitle(IMenu *pMenu, CPlayerSlot aSlot, IMenu::Title_t &aTitle)
{
	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("%s(pMenu = %p, iClient = %d, sTitle = \"%s\")\n", __FUNCTION__, pMenu, aSlot.GetClientIndex(), aTitle.Get());
	}

	const char *pszPhraseName = aTitle.Get();

	int iFound;

	if(Translations::FindPhrase(pszPhraseName, iFound))
	{
		const auto &aTranslationsPhrase = Translations::GetPhrase(iFound);

		const char *pszServerContryCode = m_aServerLanguage.GetCountryCode();

		const Translations::CPhrase::CContent *pContent;

		if(aTranslationsPhrase.Find(pszServerContryCode, pContent))
		{
			aTitle.m_sText = *pContent;
		}
	}

	auto *pHandler = FindMenuHandler(pMenu);

	if(pHandler)
	{
		pHandler->OnMenuDrawTitle(pMenu, aSlot, aTitle);
	}
}

void MenuSystem_Plugin::OnMenuDisplayItem(IMenu *pMenu, CPlayerSlot aSlot, IMenu::ItemPosition_t iItem, IMenu::Item_t &aData)
{
	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("%s(pMenu = %p, iClient = %d, iItem = %d, aData = \"%s\")\n", __FUNCTION__, pMenu, aSlot.GetClientIndex(), iItem, aData.Get());
	}

	bool bPlayerAreTranslated = false;

	if(aSlot.IsValid())
	{
		auto &aPlayer = GetPlayerData(aSlot);

		if(aPlayer.IsConnected())
		{
			bPlayerAreTranslated = aPlayer.OnMenuDisplayItem(pMenu, aSlot, iItem, aData);
		}
	}

	if(!bPlayerAreTranslated)
	{
		const char *pszPhraseName = aData.Get();

		int iFound;

		if(Translations::FindPhrase(pszPhraseName, iFound))
		{
			const auto &aTranslationsPhrase = Translations::GetPhrase(iFound);

			const char *pszServerContryCode = m_aServerLanguage.GetCountryCode();

			const Translations::CPhrase::CContent *pContent;

			if(aTranslationsPhrase.Find(pszServerContryCode, pContent))
			{
				aData.m_sContent = *pContent;
			}
		}
	}

	auto *pHandler = FindMenuHandler(pMenu);

	if(pHandler)
	{
		pHandler->OnMenuDisplayItem(pMenu, aSlot, iItem, aData);
	}
}

bool MenuSystem_Plugin::Init()
{
	if(CLogger::IsChannelEnabled(LS_DETAILED))
	{
		CLogger::DetailedFormat("%s\n", __FUNCTION__);
	}

	return true;
}

void MenuSystem_Plugin::PostInit()
{
	if(CLogger::IsChannelEnabled(LS_DETAILED))
	{
		CLogger::DetailedFormat("%s\n", __FUNCTION__);
	}
}

void MenuSystem_Plugin::Shutdown()
{
	if(CLogger::IsChannelEnabled(LS_DETAILED))
	{
		CLogger::DetailedFormat("%s\n", __FUNCTION__);
	}
}

GS_EVENT_MEMBER(MenuSystem_Plugin, GameActivate)
{
	char sMessage[256];

	bool (MenuSystem_Plugin::*pfnIntializers[])(char *error, size_t maxlen) = 
	{
		&MenuSystem_Plugin::RegisterGameResource,
		&MenuSystem_Plugin::LoadSpawnGroups,
	};

	for(const auto &aInitializer : pfnIntializers)
	{
		if(!(this->*(aInitializer))(sMessage, sizeof(sMessage)))
		{
			CLogger::WarningFormat("%s\n", sMessage);
		}
	}
}

GS_EVENT_MEMBER(MenuSystem_Plugin, GameDeactivate)
{
	char sMessage[256];

	if(!UnloadSpawnGroups(sMessage, sizeof(sMessage)))
	{
		CLogger::WarningFormat("%s\n", sMessage);
	}
}

GS_EVENT_MEMBER(MenuSystem_Plugin, ServerPreEntityThink)
{
	TeleportMenusBySpectatePlayers();
}

GS_EVENT_MEMBER(MenuSystem_Plugin, GameFrameBoundary)
{
	// Check the lifecycle of timed menus.
	for(auto &aPlayer : m_aPlayers)
	{
		if(!aPlayer.IsConnected())
		{
			continue;
		}

		IMenu::Index_t &iActiveMenu = aPlayer.GetActiveMenuIndexRef();

		auto &vecMenus = aPlayer.GetMenus();

		bool bNeedUpdate = false;

		FOR_EACH_VEC_BACK(vecMenus, i)
		{
			const auto &[nEndTimestamp, pMenu] = vecMenus.Element(i);

			if(!nEndTimestamp || nEndTimestamp > Plat_GetTime())
			{
				continue;
			}

			auto *pMemBlock = m_MenuAllocator.FindMemBlock(pMenu);

			if(!pMemBlock)
			{
				continue;
			}

			if(i == iActiveMenu)
			{
				iActiveMenu--;
				bNeedUpdate = true;
			}

			vecMenus.Remove(i);

			CMenu *pInternalMenu = m_MenuAllocator.GetInstanceByMemBlock(pMemBlock);

			CloseInternalMenu(pInternalMenu, IMenuHandler::MenuEnd_Timeout, false);
			m_MenuAllocator.ReleaseByMemBlock(pMemBlock);
		}

		// Update player menus if the active menu was closed due to timeout
		if(bNeedUpdate && vecMenus.Count())
		{
			if(iActiveMenu == MENU_INVLID_INDEX)
			{
				iActiveMenu = 0;
			}

			UpdatePlayerMenus(aPlayer.GetServerSideClient()->GetPlayerSlot());
		}
	}
}

void MenuSystem_Plugin::OnSpawnGroupAllocated(SpawnGroupHandle_t hSpawnGroup, ISpawnGroup *pSpawnGroup)
{
	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("%s(hSpawnGroup = %d, pSpawnGroup = %p)\n", __FUNCTION__, hSpawnGroup, pSpawnGroup);
	}

	// AsyncSpawnMenuEntities();
}

void MenuSystem_Plugin::OnSpawnGroupInit(SpawnGroupHandle_t hSpawnGroup, IEntityResourceManifest *pManifest, IEntityPrecacheConfiguration *pConfig, ISpawnGroupPrerequisiteRegistry *pRegistry)
{
	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("%s(hSpawnGroup = %d, pManifest = %p, pConfig = %p, pRegistry = %p)\n", __FUNCTION__, hSpawnGroup, pManifest, pConfig, pRegistry);
	}

	Assert(pManifest);

	CUtlVector<const char *> vecExportResources;

	Menu::CProfileSystem::LoopByProfiles([&vecExports = vecExportResources](CUtlSymbolLarge sName, Menu::CProfile *pProfile)
	{
		auto vecResources = pProfile->GetResources();

		vecExports.AddMultipleToTail(vecResources.Count(), vecResources.Base());
	});

	// Removes empty & dublicate ones.
	FOR_EACH_VEC(vecExportResources, i)
	{
		const char *pszCurrent = vecExportResources[i];

		if(pszCurrent && pszCurrent[0])
		{
			for(int j = i + 1; j < vecExportResources.Count(); j++)
			{
				if(!V_strcmp(pszCurrent, vecExportResources[j]))
				{
					vecExportResources.Remove(j);
					j--;
				}
			}
		}
		else
		{
			vecExportResources.Remove(i);
			i--;
		}
	}

	// CMenu resource.
	vecExportResources.AddToTail(MENU_EMPTY_BACKGROUND_MATERIAL_NAME);

	// Adds clean result to an entity resource manifest.
	for(const char *pszResource : vecExportResources)
	{
		m_pEntityManagerProviderAgent->AddResourceToEntityManifest(pManifest, pszResource);

		if(CLogger::IsChannelEnabled(LV_DETAILED))
		{
			CLogger::DetailedFormat("%s: Added \"%s\" export resource from the profile system to a manifest\n", __FUNCTION__, pszResource);
		}
	}
}

void MenuSystem_Plugin::OnSpawnGroupCreateLoading(SpawnGroupHandle_t hSpawnGroup, CMapSpawnGroup *pMapSpawnGroup, bool bSynchronouslySpawnEntities, bool bConfirmResourcesLoaded, CUtlVector<const CEntityKeyValues *> &vecKeyValues)
{
	// Test menu creation code removed - was causing unwanted "Active/Inactive" menu to appear in world
}

void MenuSystem_Plugin::OnSpawnGroupDestroyed(SpawnGroupHandle_t hSpawnGroup)
{
	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("%s(hSpawnGroup = %d)\n", __FUNCTION__, hSpawnGroup);
	}

	PurgeAllMenus();

	m_pMySpawnGroupInstance = nullptr;
}

bool MenuSystem_Plugin::InitSchema(char *error, size_t maxlen)
{
	CUtlVector<const char *> vecLoadLibraries;

	vecLoadLibraries.AddToTail(
#if defined(_WINDOWS)
		"server.dll"
#elif defined(_LINUX)
		"libserver.so"
#elif defined(_OSX)
		"libserver.dylib"
#endif
	);

	Menu::Schema::CSystem::CStringVector vecMessages;

	bool bResult = Menu::Schema::CSystem::Init(g_pSchemaSystem, vecLoadLibraries, &vecMessages);

	if(CLogger::IsChannelEnabled(LS_WARNING))
	{
		auto aWarnings = CLogger::CreateWarningsScope();

		FOR_EACH_VEC(vecMessages, i)
		{
			auto &aMessage = vecMessages[i];

			aWarnings.Push(aMessage.Get());
		}

		aWarnings.SendColor([&](Color rgba, const CUtlString &sContext)
		{
			CLogger::Warning(rgba, sContext);
		});
	}

	if(!bResult)
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to initialize schema helper. See warnings", maxlen);
		}
	}

	return bResult;
}

bool MenuSystem_Plugin::LoadSchema(char *error, size_t maxlen)
{
	bool bResult {};

	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		Menu::Schema::CSystem::CStringVector vecMessages;

		using Concat_t = decltype(g_arrEmbedsConcat)::value_type;
		using SchemaFullDetails_t = Menu::Schema::CSystem::FullDetails_t;
		constexpr uintp kEmbeds = SchemaFullDetails_t::sm_nEmbeds;

		std::array<Concat_t *, kEmbeds> arrSchemaEmbedConcats;

		static_assert(g_arrEmbedsConcat.size() >= arrSchemaEmbedConcats.size());
		std::transform(g_arrEmbedsConcat.begin(), g_arrEmbedsConcat.begin() + arrSchemaEmbedConcats.size(), arrSchemaEmbedConcats.begin(), [](const Concat_t &aConcat) { return &aConcat; });
		std::reverse(arrSchemaEmbedConcats.begin(), arrSchemaEmbedConcats.end());

		auto aFullDetails = SchemaFullDetails_t 
		{
			&vecMessages,

			std::move(arrSchemaEmbedConcats)
		};

		bResult = Menu::Schema::CSystem::Load(&aFullDetails);

		for(const auto &sMessage : vecMessages)
		{
			CLogger::Detailed(sMessage.String());
		}
	}
	else
	{
		bResult = Menu::Schema::CSystem::Load();
	}

	if(!bResult)
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to load a schema. See warnings", maxlen);
		}
	}

	return bResult;
}

bool MenuSystem_Plugin::ClearSchema(char *error, size_t maxlen)
{
	Menu::Schema::CSystem::Clear();

	return true;
}

bool MenuSystem_Plugin::InitPathResolver(char *error, size_t maxlen)
{
	if(!CPathResolver::Init(this))
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to initialize a path resolver", maxlen);
		}

		return false;
	}

	m_sBaseGameDirectory = CPathResolver::Extract();

	return true;
}

bool MenuSystem_Plugin::ClearPathResolver(char *error, size_t maxlen)
{
	CPathResolver::Clear();

	return true;
}

bool MenuSystem_Plugin::InitProvider(char *error, size_t maxlen)
{
	GameData::CStringVector vecMessages;

	bool bResult = CProvider::Init(vecMessages);

	if(vecMessages.Count())
	{
		if(CLogger::IsChannelEnabled(LS_WARNING))
		{
			auto aWarnings = CLogger::CreateWarningsScope();

			FOR_EACH_VEC(vecMessages, i)
			{
				auto &aMessage = vecMessages[i];

				aWarnings.Push(aMessage.Get());
			}

			aWarnings.SendColor([&](Color rgba, const CUtlString &sContext)
			{
				CLogger::Warning(rgba, sContext);
			});
		}
	}

	if(!bResult)
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to initialize provider. See warnings", maxlen);
		}
	}

	return bResult;
}

bool MenuSystem_Plugin::LoadProvider(char *error, size_t maxlen)
{
	GameData::CStringVector vecMessages;

	bool bResult = CProvider::Load(m_sBaseGameDirectory.c_str(), MENUSYSTEM_BASE_PATHID, vecMessages);

	if(vecMessages.Count() && CLogger::IsChannelEnabled(LS_WARNING))
	{
		auto aWarnings = CLogger::CreateWarningsScope();

		FOR_EACH_VEC(vecMessages, i)
		{
			auto &aMessage = vecMessages[i];

			aWarnings.Push(aMessage.Get());
		}

		aWarnings.SendColor([&](Color rgba, const CUtlString &sContext)
		{
			CLogger::Warning(rgba, sContext);
		});
	}

	if(!bResult)
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to load provider. See warnings", maxlen);
		}
	}

	return bResult;
}

bool MenuSystem_Plugin::UnloadProvider(char *error, size_t maxlen)
{
	GameData::CStringVector vecMessages;

	bool bResult = CProvider::Destroy(vecMessages);

	if(vecMessages.Count() && CLogger::IsChannelEnabled(LS_WARNING))
	{
		auto aWarnings = CLogger::CreateWarningsScope();

		FOR_EACH_VEC(vecMessages, i)
		{
			auto &aMessage = vecMessages[i];

			aWarnings.Push(aMessage.Get());
		}

		aWarnings.SendColor([&](Color rgba, const CUtlString &sContext)
		{
			CLogger::Warning(rgba, sContext);
		});
	}

	if(!bResult)
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to unload provider. See warnings", maxlen);
		}
	}

	return bResult;
}

bool MenuSystem_Plugin::LoadProfiles(char *error, size_t maxlen)
{
	CUtlVector<CUtlString> vecMessages;

	bool bResult = Menu::CProfileSystem::Load(m_sBaseGameDirectory.c_str(), MENUSYSTEM_BASE_PATHID, vecMessages);

	if(vecMessages.Count() && CLogger::IsChannelEnabled(LS_WARNING))
	{
		auto aWarnings = CLogger::CreateWarningsScope();

		FOR_EACH_VEC(vecMessages, i)
		{
			auto &aMessage = vecMessages[i];

			aWarnings.Push(aMessage.Get());
		}

		aWarnings.SendColor([&](Color rgba, const CUtlString &sContext)
		{
			CLogger::Warning(rgba, sContext);
		});
	}

	if(!bResult)
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to load a profile system. See warnings", maxlen);
		}
	}

	return bResult;
}

bool MenuSystem_Plugin::ClearProfiles(char *error, size_t maxlen)
{
	Menu::CProfileSystem::Clear();

	return true;
}

bool MenuSystem_Plugin::InitEntityManager(char *error, size_t maxlen)
{
	// Gets a main entity manager interface.
	{
		m_pEntityManager = reinterpret_cast<IEntityManager *>(g_SMAPI->MetaFactory(ENTITY_MANAGER_INTERFACE_NAME, nullptr, &m_iEntityManager));

		if(!m_pEntityManager)
		{
			strncpy(error, "Failed to get a entity manager interface", maxlen);

			return false;
		}
	}

	// Gets an entity manager provider agent interface.
	{
		m_pEntityManagerProviderAgent = m_pEntityManager->GetProviderAgent();

		if(!m_pEntityManagerProviderAgent)
		{
			strncpy(error, "Failed to get a entity manager provider agent interface", maxlen);

			return false;
		}
	}

	// Gets an entity manager spawn group provider interface.
	{
		m_pEntityManagerSpawnGroupProvider = m_pEntityManager->GetSpawnGroupProvider();

		if(!m_pEntityManagerSpawnGroupProvider)
		{
			strncpy(error, "Failed to get a entity manager spawn group provider interface", maxlen);

			return false;
		}
	}

	return true;
}

void MenuSystem_Plugin::DumpEntityManager(const CConcatLineString &aConcat, CBufferString &sOutput)
{
	CConcatLineBuffer aConcatBuffer(&aConcat, &sOutput);
	GLOBALS_APPEND_VARIABLE(aConcatBuffer, m_pEntityManager);
	GLOBALS_APPEND_VARIABLE(aConcatBuffer, m_pEntityManagerProviderAgent);
	GLOBALS_APPEND_VARIABLE(aConcatBuffer, m_pEntityManagerSpawnGroupProvider);
}

bool MenuSystem_Plugin::UnloadEntityManager(char *error, size_t maxlen)
{
	m_pEntityManager = nullptr;
	m_pEntityManagerProviderAgent = nullptr;
	m_pEntityManagerSpawnGroupProvider = nullptr;

	return true;
}

bool MenuSystem_Plugin::LoadSpawnGroups(char *error, size_t maxlen)
{
	if(CLogger::IsChannelEnabled(LS_DETAILED))
	{
		CLogger::DetailedFormat("%s\n", __FUNCTION__);
	}

	{
		CUtlString sMenu = GetName();

		SpawnGroupDesc_t aDesc;

		// aDesc.m_hOwner = 1; // Merge the spawn group into active one.
		// aDesc.m_sWorldName = sMenu;
		aDesc.m_sDescriptiveName = sMenu;
		aDesc.m_sEntityLumpName = "main lump";
		aDesc.m_sEntityFilterName = "menu_loader";
		aDesc.m_sLocalNameFixup = "menu_system";
		aDesc.m_sWorldGroupname = "menu";
		aDesc.m_manifestLoadPriority = RESOURCE_MANIFEST_LOAD_PRIORITY_HIGH;
		aDesc.m_bCreateClientEntitiesOnLaterConnectingClients = true;
		aDesc.m_bBlockUntilLoaded = true;

		auto *pSpawnGroupInstance = m_pEntityManagerProviderAgent->CreateSpawnGroup();

		pSpawnGroupInstance->AddNotificationsListener(static_cast<IEntityManager::IProviderAgent::ISpawnGroupNotifications *>(this));

		if(pSpawnGroupInstance->Load(aDesc, {0.f, 0.f, 0.f}))
		{
			m_pMySpawnGroupInstance = pSpawnGroupInstance;
		}
		else
		{
			snprintf(error, maxlen, "Failed to load \"%s\" spawn group", sMenu.Get());
		}
	}

	return true;
}

bool MenuSystem_Plugin::UnloadSpawnGroups(char *error, size_t maxlen)
{
	if(m_pMySpawnGroupInstance)
	{
		m_pMySpawnGroupInstance->Unload();
		// Reset the instance into OnSpawnGroupDestroyed().
	}

	return true;
}

bool MenuSystem_Plugin::UnloadSpawnGroupsNow(char *error, size_t maxlen)
{
	if(m_pMySpawnGroupInstance)
	{
		PurgeAllMenus();

		m_pMySpawnGroupInstance->Unload();
		m_pMySpawnGroupInstance = nullptr;
	}

	return true;
}

void MenuSystem_Plugin::SetMenuKeyValues(CEntityKeyValues *pMenuKV, const Vector &vecOrigin, const QAngle &angRotation)
{
	Assert(pMenuKV);

	pMenuKV->SetString("classname", "point_worldtext");
	pMenuKV->SetVector("origin", vecOrigin);
	pMenuKV->SetQAngle("angles", angRotation);
}

void MenuSystem_Plugin::SetViewModelKeyValues(CEntityKeyValues *pViewModelKV, const Vector &vecOrigin, const QAngle &angRotation)
{
	Assert(pViewModelKV);

	pViewModelKV->SetString("classname", "viewmodel");
	pViewModelKV->SetVector("origin", vecOrigin);
	pViewModelKV->SetQAngle("angles", angRotation);
}

Vector MenuSystem_Plugin::GetEntityPosition(CBaseEntity *pEntity, QAngle *pRotation)
{
	CBodyComponent *pEntityBodyComponent = CBaseEntity_Helper::GetBodyComponentAccessor(pEntity);

	CGameSceneNode *pEntitySceneNode = CBodyComponent_Helper::GetSceneNodeAccessor(pEntityBodyComponent);

	if(pRotation)
	{
		*pRotation = CGameSceneNode_Helper::GetAbsRotationAccessor(pEntitySceneNode);
	}

	return CGameSceneNode_Helper::GetAbsOriginAccessor(pEntitySceneNode);
}

void MenuSystem_Plugin::CalculateMenuEntitiesPosition(const Vector &vecOrigin, const QAngle &angRotation, int i, const Menu::CProfile *pProfile, Vector &vecBackgroundResult, Vector &vecResult, QAngle &angResult)
{
	auto *pMatrixOffset = pProfile->GetMatrixOffset();

	if(pMatrixOffset)
	{
		auto aMatrixOffset = *pMatrixOffset;

		vecResult = AddToFrontByRotation2(vecOrigin, angRotation, aMatrixOffset.m_flForward, aMatrixOffset.m_flLeft, aMatrixOffset.m_flRight, aMatrixOffset.m_flUp);
	}

	const auto flBackgroundAway = pProfile->GetBackgroundAwayUnits();

	vecBackgroundResult = AddToFrontByRotation2(vecResult, angRotation, flBackgroundAway, flBackgroundAway);

	if(i) // If a previous one.
	{
		auto *pPrevios_MatrixOffset = pProfile->GetPreviosMatrixOffset();

		if(pPrevios_MatrixOffset)
		{
			auto aPrevios_MatrixOffset = *pPrevios_MatrixOffset;

			vecResult = AddToFrontByRotation2(vecOrigin, angRotation, aPrevios_MatrixOffset.m_flForward * i, aPrevios_MatrixOffset.m_flLeft * i, aPrevios_MatrixOffset.m_flRight * i, aPrevios_MatrixOffset.m_flUp * i);
			vecBackgroundResult = AddToFrontByRotation2(vecResult, angRotation, flBackgroundAway, flBackgroundAway);
		}
		else
		{
			CLogger::WarningFormat("Second menu (N%d) can be displayed on top of first", i + 1);
		}
	}

	angResult = {0.f, AngleNormalize(angRotation.y - 90.f), AngleNormalize(-angRotation.x + 90.f)};
}

void MenuSystem_Plugin::CalculateMenuEntitiesPositionByEntity(CBaseEntity *pTarget, int i, const Menu::CProfile *pProfile, Vector &vecBackgroundResult, Vector &vecResult, QAngle &angResult)
{
	vecResult = GetEntityPosition(pTarget, &angResult);
	CalculateMenuEntitiesPosition(vecResult, angResult, i, pProfile, vecBackgroundResult, vecResult, angResult);
}

void MenuSystem_Plugin::CalculateMenuEntitiesPositionByViewModel(CBaseViewModel *pTarget, int i, const Menu::CProfile *pProfile, Vector &vecBackgroundResult, Vector &vecResult, QAngle &angResult)
{
	vecResult = GetEntityPosition(pTarget, &angResult);
	CalculateMenuEntitiesPosition(vecResult, angResult, i, pProfile, vecBackgroundResult, vecResult, angResult);
}

void MenuSystem_Plugin::CalculateMenuEntitiesPositionByCSPlayer(CCSPlayerPawnBase *pTarget, int i, const Menu::CProfile *pProfile, Vector &vecBackgroundResult, Vector &vecResult, QAngle &angResult)
{
	vecResult = GetEntityPosition(pTarget) + CBaseModelEntity_Helper::GetViewOffsetAccessor(pTarget);
	angResult = CCSPlayerPawnBase_Helper::GetEyeAnglesAccessor(pTarget);
	CalculateMenuEntitiesPosition(vecResult, angResult, i, pProfile, vecBackgroundResult, vecResult, angResult);
}

void MenuSystem_Plugin::SpawnEntities(const CUtlVector<CEntityKeyValues *> &vecKeyValues, CUtlVector<CEntityInstance *> *pEntities, IEntityManager::IProviderAgent::IEntityListener *pListener)
{
	if(CLogger::IsChannelEnabled(LS_DETAILED))
	{
		CLogger::DetailedFormat("%s\n", __FUNCTION__);
	}

	auto *pEntitySystemAllocator = g_pEntitySystem->GetEntityKeyValuesAllocator();

	const SpawnGroupHandle_t hSpawnGroup = m_pMySpawnGroupInstance->GetSpawnGroupHandle();

	COMPILE_TIME_ASSERT(INVALID_SPAWN_GROUP == ANY_SPAWN_GROUP);

	for(auto *pKeyValues : vecKeyValues)
	{
		g_pEntitySystem->AddRefKeyValues(pKeyValues);
		m_pEntityManagerProviderAgent->PushSpawnQueue(pKeyValues, hSpawnGroup);
	}

	{
		CUtlVector<CUtlString> vecDetails, 
		                       vecWarnings;

		m_pEntityManagerProviderAgent->ExecuteSpawnQueued(hSpawnGroup, pEntities, pListener, &vecDetails, &vecWarnings);

		if(vecDetails.Count())
		{
			if(CLogger::IsChannelEnabled(LS_DETAILED))
			{
				auto aDetails = CLogger::CreateDetailsScope();

				for(const auto &it : vecDetails)
				{
					aDetails.Push(it);
				}

				aDetails.SendColor([&](Color rgba, const CUtlString &sContext)
				{
					CLogger::Detailed(rgba, sContext);
				});
			}
		}

		if(vecWarnings.Count())
		{
			if(CLogger::IsChannelEnabled(LS_WARNING))
			{
				auto aWarnings = CLogger::CreateWarningsScope();

				for(const auto &it : vecWarnings)
				{
					aWarnings.Push(it);
				}

				aWarnings.SendColor([&](Color rgba, const CUtlString &sContext)
				{
					CLogger::Warning(rgba, sContext);
				});
			}
		}
	}
}

void MenuSystem_Plugin::SpawnMenu(CMenu *pInternalMenu, CPlayerSlot aInitiatorSlot, const Vector &vecBackgroundOrigin, const Vector &vecOrigin, const QAngle &angRotation)
{
	auto *pEntitySystemAllocator = g_pEntitySystem->GetEntityKeyValuesAllocator();

	const SpawnGroupHandle_t hSpawnGroup = m_pMySpawnGroupInstance->GetSpawnGroupHandle();

	static_assert(INVALID_SPAWN_GROUP == ANY_SPAWN_GROUP);

	CUtlVector<CEntityKeyValues *> vecMenuKVs = pInternalMenu->GenerateKeyValues(aInitiatorSlot, pEntitySystemAllocator, true);

	{
		SetMenuKeyValues(vecMenuKVs[MENU_ENTITY_BACKGROUND_INDEX], vecBackgroundOrigin, angRotation);
		SetMenuKeyValues(vecMenuKVs[MENU_ENTITY_INACTIVE_INDEX], vecOrigin, angRotation);
		SetMenuKeyValues(vecMenuKVs[MENU_ENTITY_ACTIVE_INDEX], vecOrigin, angRotation);
		SetMenuKeyValues(vecMenuKVs[MENU_ENTITY_DISABLED_ACTIVE_INDEX], vecOrigin, angRotation);
	}

	class CMenuEntityListener : public IEntityManager::IProviderAgent::IEntityListener
	{
	public:
		CMenuEntityListener(MenuSystem_Plugin *pInitPlugin)
		 :  m_pPlugin(pInitPlugin)
		{
		}

	public:
		void OnEntityCreated(CEntityInstance *pEntity, const CEntityKeyValues *pKeyValues) override
		{
			if(m_pPlugin->CLogger::IsChannelEnabled(LV_DETAILED))
			{
				m_pPlugin->CLogger::MessageFormat("Setting up \"%s\" menu entity\n", pEntity->GetClassname());
			}

			m_pPlugin->SettingMenuEntity(instance_upper_cast<CBaseEntity *>(pEntity));
		}

	private:
		MenuSystem_Plugin *m_pPlugin;
	} aMenuEntitySetup(this);

	CUtlVector<CEntityInstance *> vecEntities;

	SpawnEntities(vecMenuKVs, &vecEntities, &aMenuEntitySetup);
	Assert(vecEntities.Count() == MENU_MAX_ENTITIES);
	pInternalMenu->Emit(vecEntities);

	vecMenuKVs.PurgeAndDeleteElements();
}

void MenuSystem_Plugin::SpawnMenuByEntityPosition(int iMenu, CMenu *pInternalMenu, CPlayerSlot aInitiatorSlot, CBaseEntity *pTarget)
{
	Vector vecMenuAbsOriginBackground {},
	       vecMenuAbsOrigin {};

	QAngle angMenuRotation {};

	auto *pProfile = Menu::CProfileSystem::GetInternal();

	Assert(pProfile);
	CalculateMenuEntitiesPositionByEntity(pTarget, iMenu, pProfile, vecMenuAbsOriginBackground, vecMenuAbsOrigin, angMenuRotation);
	SpawnMenu(pInternalMenu, aInitiatorSlot, vecMenuAbsOriginBackground, vecMenuAbsOrigin, angMenuRotation);
}

// A universal way to create a second view model.
CBaseViewModel *MenuSystem_Plugin::SpawnViewModelEntity(const Vector &vecOrigin, const QAngle &angRotation, CBaseEntity *pOwner, const int nSlot)
{
	const SpawnGroupHandle_t hSpawnGroup = m_pMySpawnGroupInstance->GetSpawnGroupHandle();

	static_assert(INVALID_SPAWN_GROUP == ANY_SPAWN_GROUP);

	CEntityKeyValues *pViewModelKV = new CEntityKeyValues(g_pEntitySystem->GetEntityKeyValuesAllocator(), EKV_ALLOCATOR_EXTERNAL);

	CUtlVector<CEntityKeyValues *> vecKeyValues;

	CUtlVector<CEntityInstance *> vecEntities;

	class CViewModelEntityListener : public IEntityManager::IProviderAgent::IEntityListener
	{
	public:
		CViewModelEntityListener(MenuSystem_Plugin *pInitPlugin, CBaseEntity *pInitOwner, const int nInitSlot)
		 :  m_pPlugin(pInitPlugin),
		    m_pOwner(pInitOwner),
		    m_nSlot(nInitSlot)
		{
		}

	public:
		void OnEntityCreated(CEntityInstance *pEntity, const CEntityKeyValues *pKeyValues) override
		{
			if(m_pPlugin->CLogger::IsChannelEnabled(LV_DETAILED))
			{
				m_pPlugin->CLogger::MessageFormat("Setting up \"%s\" view model entity\n", pEntity->GetClassname());
			}

			m_pPlugin->SettingExtraPlayerViewModelEntity(instance_upper_cast<CBaseViewModel *>(pEntity), m_pOwner, m_nSlot);
		}

	private:
		MenuSystem_Plugin *m_pPlugin;
		CBaseEntity *m_pOwner;
		const int m_nSlot;
	} aViewModelEntitySetup(this, pOwner, nSlot);

	SetViewModelKeyValues(pViewModelKV, vecOrigin, angRotation);
	vecKeyValues.AddToTail(pViewModelKV);
	SpawnEntities(vecKeyValues, &vecEntities, &aViewModelEntitySetup);

	return instance_upper_cast<CBaseViewModel *>(vecEntities[0]);
}

void MenuSystem_Plugin::TeleportMenuInstanceToCSPlayer(int i, CMenu *pInternalMenu, CCSPlayerPawnBase *pTarget)
{
	auto &aBaseEntity = GetGameDataStorage().GetBaseEntity();

	Vector vecMenuAbsOriginBackground {},
	       vecMenuAbsOrigin {};

	QAngle angMenuRotation {};

	auto *pProfile = Menu::CProfileSystem::GetInternal();

	Assert(pProfile);
	CalculateMenuEntitiesPositionByCSPlayer(pTarget, i, pProfile, vecMenuAbsOriginBackground, vecMenuAbsOrigin, angMenuRotation);

	const auto &vecEntities = pInternalMenu->GetActiveEntities();

	FOR_EACH_VEC(vecEntities, i)
	{
		auto *pEntity = vecEntities[i];

		aBaseEntity.Teleport(pEntity, i ? vecMenuAbsOrigin : vecMenuAbsOriginBackground, angMenuRotation);
	}
}

void MenuSystem_Plugin::AttachMenuInstanceToEntity(CMenu *pInternalMenu, CBaseEntity *pTarget)
{
	auto &aBaseEntity = GetGameDataStorage().GetBaseEntity();

	auto aParentVariant = variant_t("!activator");

	const auto &vecEntities = pInternalMenu->GetActiveEntities();

	for(auto *pEntity : vecEntities)
	{
		aBaseEntity.AcceptInput(pEntity, "SetParent", pTarget, NULL, &aParentVariant, 0);
	}
}

bool MenuSystem_Plugin::AttachMenuInstanceToCSPlayer(int i, CMenu *pInternalMenu, CCSPlayerPawn *pTarget)
{
	auto &aBaseEntity = GetGameDataStorage().GetBaseEntity();

	auto aViewModelServicesAccessor = CCSPlayerPawn_Helper::GetViewModelServicesAccessor(pTarget);

	CCSPlayer_ViewModelServices *pCSPlayerViewModelServices = aViewModelServicesAccessor;

	if(!pCSPlayerViewModelServices)
	{
		CLogger::WarningFormat("Failed to get a player view model services\n");

		return false;
	}

	// 0 - the main one.
	// 1 - the hostages.
	// 2 - the extra one.
	static constexpr int s_nExtraViewModelSlot = 2;

	auto aParentVariant = variant_t("!activator");

	CHandle<CBaseViewModel> &hPlayerViewModel = CCSPlayer_ViewModelServices_Helper::GetViewModelAccessor(pCSPlayerViewModelServices)[s_nExtraViewModelSlot];

	CBaseViewModel *pPlayerViewModel = hPlayerViewModel.Get();

	Vector vecMenuAbsOriginBackground {},
	       vecMenuAbsOrigin {};

	QAngle angMenuRotation {};

	auto *pProfile = Menu::CProfileSystem::GetInternal();

	Assert(pProfile);
	CalculateMenuEntitiesPositionByEntity(pTarget, i, pProfile, vecMenuAbsOriginBackground, vecMenuAbsOrigin, angMenuRotation);

	if(CLogger::IsChannelEnabled(LS_DETAILED))
	{
		const auto &aConcat = g_aEmbedConcat;

		CBufferStringN<256> sBuffer;
		CConcatLineBuffer aConcatBuffer(&aConcat, &sBuffer);

		aConcatBuffer.Append("Menu entities position");
		aConcatBuffer.Append("Origin", vecMenuAbsOrigin);
		aConcatBuffer.Append("Rotation", angMenuRotation);

		CLogger::Detailed(sBuffer);
	}

	if(!pPlayerViewModel)
	{
		auto *pExtraPlayerViewModel = SpawnViewModelEntity(vecMenuAbsOrigin, angMenuRotation, pTarget, s_nExtraViewModelSlot);

		hPlayerViewModel.Set(pExtraPlayerViewModel);
		pPlayerViewModel = pExtraPlayerViewModel;
	}

	aBaseEntity.AcceptInput(pPlayerViewModel, "FollowEntity", pTarget, NULL, &aParentVariant, 0);

	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("Player view model: \"%s\" (%d)\n", pPlayerViewModel->GetClassname(), pPlayerViewModel->GetEntityIndex().Get());
	}

	const auto &vecEntities = pInternalMenu->GetActiveEntities();

	FOR_EACH_VEC(vecEntities, i)
	{
		auto *pEntity = vecEntities[i];

		aBaseEntity.Teleport(pEntity, i ? vecMenuAbsOrigin : vecMenuAbsOriginBackground, angMenuRotation);
		aBaseEntity.AcceptInput(pEntity, "SetParent", pPlayerViewModel, NULL, &aParentVariant, 0);
	}

	aViewModelServicesAccessor.MarkNetworkChanged(); // Update CCSPlayer_ViewModelServices < CPlayer_ViewModelServices state of the view model entities.

	return true;
}

bool MenuSystem_Plugin::AttachMenuInstanceToObserver(int i, CMenu *pInternalMenu, CCSPlayerPawnBase *pTarget)
{
	auto *pTargetPlayerPawn = instance_upper_cast<CCSPlayerPawnBase *>(pTarget);

	CPlayer_ObserverServices *pObserverServices = CCSPlayerPawnBase_Helper::GetObserverServicesAccessor(pTargetPlayerPawn);

	if(pObserverServices)
	{
		uint8 iObserverMode = CPlayer_ObserverServices_Helper::GetObserverModeAccessor(pObserverServices);

		if(iObserverMode != OBS_MODE_NONE && iObserverMode != OBS_MODE_ROAMING)
		{
			CHandle<CBaseEntity> hObserverTarget = CPlayer_ObserverServices_Helper::GetObserverTargetAccessor(pObserverServices);

			auto *pCSObserverTarget = hObserverTarget.Get();

			if(pCSObserverTarget)
			{
				AttachMenuInstanceToCSPlayer(i, pInternalMenu, instance_upper_cast<CCSPlayerPawn *>(pCSObserverTarget));

				return true;
			}
		}
	}

	AttachMenuInstanceToEntity(pInternalMenu, pTargetPlayerPawn);

	return false;
}

void MenuSystem_Plugin::TeleportMenusBySpectatePlayers()
{
	for(auto &aPlayer : m_aPlayers)
	{
		if(!aPlayer.IsConnected())
		{
			continue;
		}

		const auto &vecMenus = aPlayer.GetMenus();

		if(!vecMenus.Count())
		{
			continue;
		}

		auto *pServerSideClient = aPlayer.GetServerSideClient();

		auto *pPlayerController = instance_upper_cast<CBasePlayerController *>(g_pEntitySystem->GetEntityInstance(CEntityIndex(pServerSideClient->GetPlayerSlot().GetClientIndex())));

		if(!pPlayerController)
		{
			continue;
		}

		CBasePlayerPawn *pPlayerPawn = CBasePlayerController_Helper::GetPawnAccessor(pPlayerController)->Get();

		if(!pPlayerPawn)
		{
			continue;
		}

		auto *pCSPlayerPawn = instance_upper_cast<CCSPlayerPawnBase *>(pPlayerPawn);

		CPlayer_ObserverServices *pObserverServices = CCSPlayerPawnBase_Helper::GetObserverServicesAccessor(pCSPlayerPawn);

		if(!pObserverServices)
		{
			continue;
		}

		uint8 iObserverMode = CPlayer_ObserverServices_Helper::GetObserverModeAccessor(pObserverServices);

		if(iObserverMode != OBS_MODE_NONE && iObserverMode != OBS_MODE_ROAMING)
		{
			continue;
		}

		FOR_EACH_VEC(vecMenus, i)
		{
			const auto &[_, pMenu] = vecMenus[i];

			CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

			if(pInternalMenu)
			{
				TeleportMenuInstanceToCSPlayer(i, pInternalMenu, pCSPlayerPawn);
			}
		}
	}
}

bool MenuSystem_Plugin::SettingMenuEntity(CEntityInstance *pEntity)
{
	{
		auto aEFlagsAccessor = CBaseEntity_Helper::GetEFlagsAccessor(instance_upper_cast<CBaseEntity *>(pEntity));

		aEFlagsAccessor = EF_MENU;
		aEFlagsAccessor.MarkNetworkChanged();
	}

	return true;
}

bool MenuSystem_Plugin::SettingExtraPlayerViewModelEntity(CBaseViewModel *pViewModelEntity, CBaseEntity *pOwner, const int nSlot)
{
	{
		auto aViewModelIndexAccessor = CBaseViewModel_Helper::GetViewModelIndexAccessor(pViewModelEntity);

		aViewModelIndexAccessor = nSlot;
		aViewModelIndexAccessor.MarkNetworkChanged();
	}

	{
		auto aOwnerEntityAccessor = CBaseEntity_Helper::GetOwnerEntityAccessor(pViewModelEntity);

		aOwnerEntityAccessor = pOwner;
		aOwnerEntityAccessor.MarkNetworkChanged();
	}

	{
		auto aEFlagsAccessor = CBaseEntity_Helper::GetEFlagsAccessor(pViewModelEntity);

		aEFlagsAccessor = EF_NODRAW;
		aEFlagsAccessor.MarkNetworkChanged();
	}

	return true;
}

bool MenuSystem_Plugin::RegisterGameResource(char *error, size_t maxlen)
{
	if(!RegisterGameEntitySystem(m_pEntityManagerProviderAgent->GetSystem()))
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to register a (game) entity system", maxlen);
		}

		return false;
	}

	return true;
}

bool MenuSystem_Plugin::UnregisterGameResource(char *error, size_t maxlen)
{
	if(!UnregisterGameEntitySystem())
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to unregister a (game) entity system", maxlen);
		}

		return false;
	}

	return true;
}

bool MenuSystem_Plugin::RegisterGameFactory(char *error, size_t maxlen)
{
	CBaseGameSystemFactory **ppFactory = GetFirstGameSystemPointer();

	if(!ppFactory)
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to get a first game system factory", maxlen);
		}

		return false;
	}

	if(!RegisterFirstGameSystem(ppFactory))
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to register a first game factory", maxlen);
		}

		return false;
	}

	m_pFactory = new CGameSystemStaticFactory<MenuSystem_Plugin>(GetName(), this);

	return true;
}

bool MenuSystem_Plugin::UnregisterGameFactory(char *error, size_t maxlen)
{
	if(m_pFactory)
	{
		m_pFactory->Shutdown();

		auto *pGameSystem = m_pFactory->GetStaticGameSystem();

		// Clean up smart dispatcher listener callbacks.
		{
			auto **ppDispatcher = GetGameSystemEventDispatcherPointer();

			Assert(ppDispatcher);

			auto *pDispatcher = *ppDispatcher;

			Assert(pDispatcher);

			pDispatcher->UnregisterListener(pGameSystem);
		}

		{
			auto *pDispatcher2 = GetOutOfGameEventDispatcher();

			Assert(pDispatcher2);

			pDispatcher2->UnregisterListener(pGameSystem);
		}

		// Clean up the added game system.
		{
			auto *pGameSystemFactories = GetGameSystemFactoriesPointer();

			Assert(pGameSystemFactories);

			pGameSystemFactories->FindAndRemove(pGameSystem->GetName());
		}

		// Clean up the game systems.
		{
			auto *pGameSystems = GetGameSystemsPointer();

			Assert(pGameSystems);

			auto &vecGameSystems = *pGameSystems;

			FOR_EACH_VEC_BACK(vecGameSystems, i)
			{
				if(pGameSystem == vecGameSystems[i].m_pGameSystem)
				{
					vecGameSystems.FastRemove(i);

					break;
				}
			}
		}

		m_pFactory->DestroyGameSystem(this);
		m_pFactory->Destroy();
	}

	if(!UnregisterFirstGameSystem())
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to unregister a first game factory", maxlen);
		}

		return false;
	}

	return true;
}

bool MenuSystem_Plugin::RegisterSource2Server(char *error, size_t maxlen)
{
	IGameEventManager2 **ppGameEventManager = GetGameEventManagerPointer();

	if(!ppGameEventManager)
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to get a game event manager", maxlen);
		}

		return false;
	}

	if(!RegisterGameEventManager(*ppGameEventManager))
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to register a game event manager", maxlen);
		}

		return false;
	}

	return true;
}

bool MenuSystem_Plugin::UnregisterSource2Server(char *error, size_t maxlen)
{
	if(!UnregisterGameEventManager())
	{
		if(error && maxlen)
		{
			strncpy(error, "Failed to register a game event manager", maxlen);
		}

		return false;
	}

	return true;
}

bool MenuSystem_Plugin::RegisterNetMessages(char *error, size_t maxlen)
{
	const struct
	{
		const char *pszName;
		INetworkMessageInternal **ppInternal;
	} aMessageInitializers[] =
	{
		{
			"CNETMsg_SetConVar",
			&m_pSetConVarMessage,
		},
		{
			"CSVCMsg_GetCvarValue",
			&m_pGetCvarValueMessage,
		},
		{
			"CUserMessageSayText2",
			&m_pSayText2Message,
		},
		{
			"CUserMessageTextMsg",
			&m_pTextMsgMessage,
		},
		// {
		// 	"CCSUsrMsg_VGUIMenu",
		// 	&m_pVGUIMenuMessage,
		// },
	};

	for(const auto &aMessageInitializer : aMessageInitializers)
	{
		const char *pszMessageName = aMessageInitializer.pszName;

		INetworkMessageInternal *pMessage = g_pNetworkMessages->FindNetworkMessagePartial(pszMessageName);

		if(!pMessage)
		{
			if(error && maxlen)
			{
				snprintf(error, maxlen, "Failed to get \"%s\" message", pszMessageName);
			}

			return false;
		}

		*aMessageInitializer.ppInternal = pMessage;
	}

	return true;
}

bool MenuSystem_Plugin::UnregisterNetMessages(char *error, size_t maxlen)
{
	m_pSetConVarMessage = NULL;
	m_pGetCvarValueMessage = NULL;
	m_pSayText2Message = NULL;
	m_pTextMsgMessage = NULL;
	// m_pVGUIMenuMessage = NULL;

	return true;
}

bool MenuSystem_Plugin::ParseLanguages(char *error, size_t maxlen)
{
	std::string sTranslationsFilesPath = m_sBaseGameDirectory + CORRECT_PATH_SEPARATOR_S MENUSYSTEM_GAME_LANGUAGES_FILES;

	const char *pszPathID = MENUSYSTEM_BASE_PATHID, 
	           *pszLanguagesFiles = sTranslationsFilesPath.c_str();

	CUtlVector<CUtlString> vecLangugesFiles;
	CUtlVector<CUtlString> vecSubmessages;

	CUtlString sMessage;

	auto aWarnings = CLogger::CreateWarningsScope();

	AnyConfig::CLoadFromFile_General aLoadPresets({{&sMessage, NULL, pszPathID}, g_KV3Format_Generic});

	g_pFullFileSystem->FindFileAbsoluteList(vecLangugesFiles, pszLanguagesFiles, pszPathID);

	if(!vecLangugesFiles.Count())
	{
		if(error && maxlen)
		{
			snprintf(error, maxlen, "No found languages by \"%s\" path", pszLanguagesFiles);
		}

		return false;
	}

	AnyConfig::Anyone aLanguagesConfig;

	for(const auto &sFile : vecLangugesFiles)
	{
		const char *pszFilename = sFile.Get();

		aLoadPresets.m_pszFilename = pszFilename;

		if(!aLanguagesConfig.Load(aLoadPresets))
		{
			aWarnings.PushFormat("\"%s\": %s", pszFilename, sMessage.Get());

			continue;
		}

		if(!ParseLanguages(aLanguagesConfig.Get(), vecSubmessages))
		{
			aWarnings.PushFormat("\"%s\":", pszFilename);

			for(const auto &sSubmessage : vecSubmessages)
			{
				aWarnings.PushFormat("\t%s", sSubmessage.Get());
			}

			continue;
		}
	}

	if(aWarnings.Count())
	{
		aWarnings.Send([&](const CUtlString &sMessage)
		{
			CLogger::Warning(sMessage);
		});
	}

	return true;
}

bool MenuSystem_Plugin::ParseLanguages(KeyValues3 *pRoot, CUtlVector<CUtlString> &vecMessages)
{
	int nMemberCount = pRoot->GetMemberCount();

	if(!nMemberCount)
	{
		vecMessages.AddToTail("No members");

		return true;
	}

	const KeyValues3 *pDefaultData = pRoot->FindMember("default");

	const char *pszServerContryCode = pDefaultData ? pDefaultData->GetString() : "en";

	m_aServerLanguage.SetCountryCode(pszServerContryCode);

	KV3MemberId_t i = 0;

	do
	{
		const char *pszMemberName = pRoot->GetMemberName(i);

		auto sMemberSymbol = GetLanguageSymbol(pszMemberName);

		const KeyValues3 *pMember = pRoot->GetMember(i);

		const char *pszMemberValue = pMember->GetString(pszServerContryCode);

		m_mapLanguages.Insert(sMemberSymbol, {sMemberSymbol, pszMemberValue});

		i++;
	}
	while(i < nMemberCount);

	return true;
}

bool MenuSystem_Plugin::ClearLanguages(char *error, size_t maxlen)
{
	m_vecLanguages.Purge();

	return true;
}

bool MenuSystem_Plugin::ParseTranslations(char *error, size_t maxlen)
{
	std::string sTranslationsFilesPath = m_sBaseGameDirectory + CORRECT_PATH_SEPARATOR_S MENUSYSTEM_GAME_TRANSLATIONS_FILES;

	const char *pszPathID = MENUSYSTEM_BASE_PATHID, 
	           *pszTranslationsFiles = sTranslationsFilesPath.c_str();

	CUtlVector<CUtlString> vecTranslationsFiles;

	Translations::CStringVector vecSubmessages;

	CUtlString sMessage;

	auto aWarnings = CLogger::CreateWarningsScope();

	AnyConfig::CLoadFromFile_General aLoadPresets({{&sMessage, NULL, pszPathID}, g_KV3Format_Generic});

	g_pFullFileSystem->FindFileAbsoluteList(vecTranslationsFiles, pszTranslationsFiles, pszPathID);

	if(!vecTranslationsFiles.Count())
	{
		if(error && maxlen)
		{
			snprintf(error, maxlen, "No found translations by \"%s\" path", pszTranslationsFiles);
		}

		return false;
	}

	AnyConfig::Anyone aTranslationsConfig;

	for(const auto &sFile : vecTranslationsFiles)
	{
		const char *pszFilename = sFile.Get();

		aLoadPresets.m_pszFilename = pszFilename;

		if(!aTranslationsConfig.Load(aLoadPresets))
		{
			aWarnings.PushFormat("\"%s\": %s", pszFilename, sMessage.Get());

			continue;
		}

		if(!Translations::Parse(aTranslationsConfig.Get(), nullptr, vecSubmessages))
		{
			aWarnings.PushFormat("\"%s\":", pszFilename);

			for(const auto &sSubmessage : vecSubmessages)
			{
				aWarnings.PushFormat("\t%s", sSubmessage.Get());
			}

			continue;
		}
	}

	if(aWarnings.Count())
	{
		aWarnings.Send([&](const CUtlString &sMessage)
		{
			CLogger::Warning(sMessage);
		});
	}

	return ParseTranslations2(error, maxlen);
}

bool MenuSystem_Plugin::ParseTranslations2(char *error, size_t maxlen)
{
	std::string sTranslationsDirsPath = m_sBaseGameDirectory + CORRECT_PATH_SEPARATOR_S MENUSYSTEM_GAME_TRANSLATIONS_COUNTRY_CODES_DIRS;

	const char *pszPathID = MENUSYSTEM_BASE_PATHID, 
	           *pszTranslationsDirs = sTranslationsDirsPath.c_str();

	CUtlVector<CUtlString> vecTranslationsDirs, 
	                       vecTranslationFilenames;

	Translations::CStringVector vecSubmessages;

	CUtlString sMessage;

	auto aWarnings = CLogger::CreateWarningsScope();

	AnyConfig::CLoadFromFile_General aLoadPresets({{&sMessage, NULL, pszPathID}, g_KV3Format_Generic});

	g_pFullFileSystem->FindFileAbsoluteList(vecTranslationsDirs, pszTranslationsDirs, pszPathID);

	if(!vecTranslationsDirs.Count())
	{
		if(error && maxlen)
		{
			snprintf(error, maxlen, "No found translations directories by \"%s\" path", pszTranslationsDirs);
		}

		return false;
	}

	AnyConfig::Anyone aTranslationsConfig;

	for(const auto &sDir : vecTranslationsDirs)
	{
		const char *pszDirectory = sDir.Get();

		if(g_pFullFileSystem->IsDirectory(pszDirectory, pszPathID))
		{
			std::string sTranslationsFilesPath = std::string(pszDirectory) + CORRECT_PATH_SEPARATOR_S MENUSYSTEM_GAME_TRANSLATIONS_FILENAMES;

			const char *pszTranslationsFiles = sTranslationsFilesPath.c_str();

			g_pFullFileSystem->FindFileAbsoluteList(vecTranslationFilenames, pszTranslationsFiles, pszPathID);

			if(!vecTranslationFilenames.Count())
			{
				if(error && maxlen)
				{
					snprintf(error, maxlen, "No found translations by \"%s\" path", pszTranslationsFiles);
				}

				return false;
			}

			for(const auto &sFilename : vecTranslationFilenames)
			{
				const char *pszFilename = sFilename.Get();

				aLoadPresets.m_pszFilename = pszFilename;

				if(!aTranslationsConfig.Load(aLoadPresets))
				{
					aWarnings.PushFormat("\"%s\": %s", pszFilename, sMessage.Get());

					continue;
				}

				KeyValues3 *pData = aTranslationsConfig.Get();

				if(!Translations::Parse(pData, nullptr, vecSubmessages))
				{
					aWarnings.PushFormat("\"%s\":", pszFilename);

					for(const auto &sSubmessage : vecSubmessages)
					{
						aWarnings.PushFormat("\t%s", sSubmessage.Get());
					}

					continue;
				}
			}
		}
	}

	if(aWarnings.Count())
	{
		aWarnings.Send([&](const CUtlString &sMessage)
		{
			CLogger::Warning(sMessage);
		});
	}

	return true;
}

bool MenuSystem_Plugin::ClearTranslations(char *error, size_t maxlen)
{
	Translations::Purge();

	return true;
}

bool MenuSystem_Plugin::LoadChat(char *error, size_t maxlen)
{
	CUtlVector<CUtlString> vecMessages;

	if(!CChatSystem::Load(m_sBaseGameDirectory.c_str(), MENUSYSTEM_BASE_PATHID, vecMessages))
	{
		if(vecMessages.Count() && CLogger::IsChannelEnabled(LS_WARNING))
		{
			auto aWarnings = CLogger::CreateWarningsScope();

			FOR_EACH_VEC(vecMessages, i)
			{
				auto &aMessage = vecMessages[i];

				aWarnings.Push(aMessage.Get());
			}

			aWarnings.SendColor([&](Color rgba, const CUtlString &sContext)
			{
				CLogger::Warning(rgba, sContext);
			});
		}

		return false;
	}

	return true;
}

bool MenuSystem_Plugin::ClearChat(char *error, size_t maxlen)
{
	CChatSystem::Clear();

	return true;
}

bool MenuSystem_Plugin::HookGameEvents(char *error, size_t maxlen)
{
	if(!CGameEventSystem::HookAll())
	{
		strncpy(error, "Failed to hook game events", maxlen);

		return false;
	}

	return true;
}

bool MenuSystem_Plugin::UnhookGameEvents(char *error, size_t maxlen)
{
	if(!CGameEventSystem::UnhookAll())
	{
		strncpy(error, "Failed to unhook game events", maxlen);

		return false;
	}

	return true;
}

void MenuSystem_Plugin::OnReloadSchemaCommand(const CCommandContext &context, const CCommand &args)
{
	char error[256];

	if(!LoadSchema(error, sizeof(error)))
	{
		CLogger::WarningFormat("%s\n", error);
	}
}

void MenuSystem_Plugin::OnReloadGameDataCommand(const CCommandContext &context, const CCommand &args)
{
	char error[256];

	if(!LoadProvider(error, sizeof(error)))
	{
		CLogger::WarningFormat("%s\n", error);
	}
}

void MenuSystem_Plugin::OnReloadProfilesCommand(const CCommandContext &context, const CCommand &args)
{
	char error[256];

	if(!LoadProfiles(error, sizeof(error)))
	{
		CLogger::WarningFormat("%s\n", error);
	}
}

void MenuSystem_Plugin::OnReloadTranslationsCommand(const CCommandContext &context, const CCommand &args)
{
	char error[256];

	if(!ParseTranslations(error, sizeof(error)))
	{
		CLogger::WarningFormat("%s\n", error);
	}
}

void MenuSystem_Plugin::OnMenuSelectCommand(const CCommandContext &context, const CCommand &args)
{
	int iSelectItem = args.ArgC() > 1 ? V_atoi(args.Arg(1)) : -1;

	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("Menu: select item is %d!!!\n", iSelectItem);
	}

	auto aSlot = context.GetPlayerSlot();

	if(!aSlot.IsValid())
	{
		CLogger::MessageFormat("Menu select item is %d from a root console?\n", iSelectItem);

		return;
	}

	auto &aPlayer = GetPlayerData(aSlot);

	if(!aPlayer.IsConnected())
	{
		return;
	}

	auto &vecMenus = aPlayer.GetMenus();

	if(!vecMenus.Count())
	{
		return;
	}

	IMenu::Index_t iActiveMenu = aPlayer.GetActiveMenuIndex();

	if(iActiveMenu == MENU_INVLID_INDEX)
	{
		FOR_EACH_VEC(vecMenus, i)
		{
			const auto &[_, pMenu] = vecMenus.Element(i);

			CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

			if(pInternalMenu)
			{
				pInternalMenu->OnSelect(aSlot, iSelectItem, IMenu::MENU_DISPLAY_DEFAULT);

				break;
			}
		}
	}
	else
	{
		CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(vecMenus[iActiveMenu].m_pInstance);

		if(pInternalMenu)
		{
			pInternalMenu->OnSelect(aSlot, iSelectItem, IMenu::MENU_DISPLAY_DEFAULT);
		}
	}
}

void MenuSystem_Plugin::OnDispatchConCommandHook(ConCommandRef hCommand, const CCommandContext &aContext, const CCommand &aArgs)
{
	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		CLogger::DetailedFormat("%s(%d, %d, %s)\n", __FUNCTION__, hCommand.GetAccessIndex(), aContext.GetPlayerSlot().Get(), aArgs.GetCommandString());
	}

	auto aPlayerSlot = aContext.GetPlayerSlot();

	const char *pszArg0 = aArgs.Arg(0);

	static const char szSayCommand[] = "say";

	size_t nSayNullTerminated = sizeof(szSayCommand) - 1;

	if(!V_strncmp(pszArg0, (const char *)szSayCommand, nSayNullTerminated))
	{
		if(!pszArg0[nSayNullTerminated] || !V_strcmp(&pszArg0[nSayNullTerminated], "_team"))
		{
			const char *pszArg1 = aArgs.Arg(1);

			// Skip spaces.
			while(*pszArg1 == ' ')
			{
				pszArg1++;
			}

			bool bIsSilent = *pszArg1 == CChatSystem::GetSilentTrigger();

			if(bIsSilent || *pszArg1 == CChatSystem::GetPublicTrigger())
			{
				pszArg1++; // Skip a command character.

				// Check if there's any content after the command character
				if(*pszArg1 == '\0')
				{
					// Empty command (just ! or /), skip processing
					// Let other frameworks handle this
				}
				// Check for menu selection commands (1-9)
				else if(pszArg1[0] >= '1' && pszArg1[0] <= '9' && (pszArg1[1] == '\0' || pszArg1[1] == ' '))
				{
					int iMenuSelection = pszArg1[0] - '0';
					
					if(CLogger::IsChannelEnabled(LV_DETAILED))
					{
						CLogger::DetailedFormat("Menu selection via chat command: %d (silent: %s)\n", iMenuSelection, bIsSilent ? "true" : "false");
					}

					// Print a chat message before if not silent.
					if(!bIsSilent && g_pCVar)
					{
						SH_CALL(g_pCVar, &ICvar::DispatchConCommand)(hCommand, aContext, aArgs);
					}

					// Call menu selection directly
					if(aPlayerSlot.IsValid())
					{
						auto &aPlayer = GetPlayerData(aPlayerSlot);

						if(aPlayer.IsConnected())
						{
							auto &vecMenus = aPlayer.GetMenus();

							if(vecMenus.Count())
							{
								IMenu::Index_t iActiveMenu = aPlayer.GetActiveMenuIndex();

								if(iActiveMenu == MENU_INVLID_INDEX)
								{
									FOR_EACH_VEC(vecMenus, i)
									{
										const auto &[_, pMenu] = vecMenus.Element(i);

										CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

										if(pInternalMenu)
										{
											pInternalMenu->OnSelect(aPlayerSlot, iMenuSelection, IMenu::MENU_DISPLAY_DEFAULT);
											break;
										}
									}
								}
								else
								{
									CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(vecMenus[iActiveMenu].m_pInstance);

									if(pInternalMenu)
									{
										pInternalMenu->OnSelect(aPlayerSlot, iMenuSelection, IMenu::MENU_DISPLAY_DEFAULT);
									}
								}
							}
						}
					}

					// no return
					// cuz other frameworks may use this command so we should not block them
					//RETURN_META(MRES_SUPERCEDE);
				}

				{
					size_t nArg1Length = 0;

					// Get a length to a first space.
					while(pszArg1[nArg1Length] && pszArg1[nArg1Length] != ' ')
					{
						nArg1Length++;
					}

					CUtlVector<CUtlString> vecArgs;

					V_SplitString(pszArg1, " ", vecArgs);

					for(auto &sArg : vecArgs)
					{
						sArg.Trim(' ');
					}

					if(CLogger::IsChannelEnabled(LV_DETAILED))
					{
						const auto &aConcat = g_aEmbedConcat,
						           &aConcat2 = g_aEmbed2Concat,
						           &aConcat3 = g_aEmbed3Concat;
	
						CBufferStringN<1024> sBuffer;
						CConcatLineBuffer aConcatBuffer(&aConcat, &sBuffer);
						CConcatLineBuffer aConcatBuffer3(&aConcat3, &sBuffer);
	
						aConcatBuffer.Append("Handle a chat command");
						aConcatBuffer.Append("Player slot", aPlayerSlot.Get());
						aConcatBuffer.Append("Is silent", bIsSilent);
						aConcatBuffer.Append("Arguments");
	
						for(const auto &sArg : vecArgs)
						{
							aConcatBuffer3.Append<true>(sArg.Get());
	
							// ...
						}
	
						CLogger::Detailed(sBuffer);
					}

					uint16 iHandler = CChatSystem::FindHandler(vecArgs[0]);

					if(CChatSystem::IsValidHandler(iHandler)) // Chat command is found.
					{
						// Print a chat message before.
						if(!bIsSilent && g_pCVar)
						{
							SH_CALL(g_pCVar, &ICvar::DispatchConCommand)(hCommand, aContext, aArgs);
						}

						CChatSystem::Call(iHandler, vecArgs[0], aPlayerSlot, bIsSilent, vecArgs);
					}
					else if(g_pCVar)
					{
						SH_CALL(g_pCVar, &ICvar::DispatchConCommand)(hCommand, aContext, aArgs);
					}
				}

				RETURN_META(MRES_SUPERCEDE);
			}
		}
	}

	RETURN_META(MRES_IGNORED);
}

void MenuSystem_Plugin::OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *)
{
	OnStartupServer(reinterpret_cast<CNetworkGameServerBase *>(g_pNetworkServerService->GetIGameServer()), config, pWorldSession);

	RETURN_META(MRES_IGNORED);
}

CServerSideClientBase *MenuSystem_Plugin::OnConnectClientHook(const char *pszName, ns_address *pAddr, uint32 hSocket, const C2S_CONNECT_Message &msg, 
                                                              const char *pszChallenge, const byte *pAuthTicket, int nAuthTicketLength, bool bIsLowViolence)
{
	OnConnectClient(META_IFACEPTR(CNetworkGameServerBase), META_RESULT_ORIG_RET(CServerSideClientBase *));

	RETURN_META_VALUE(MRES_IGNORED, NULL);
}

void MenuSystem_Plugin::OnCheckTransmitHook(CCheckTransmitInfo **ppInfoList, int nInfoCount, CBitVec<MAX_EDICTS> &bvUnionTransmitEdicts, CBitVec<MAX_EDICTS> &bvUnknown,
                                            const Entity2Networkable_t **pNetworkables, const uint16 *pEntityIndicies, int nEntities, bool bEnablePVSBits)
{
	OnCheckTransmit(META_IFACEPTR(ISource2GameEntities), ppInfoList, nInfoCount, bvUnionTransmitEdicts, bvUnknown, pNetworkables, pEntityIndicies, nEntities, bEnablePVSBits);

	RETURN_META(MRES_IGNORED);
}

bool MenuSystem_Plugin::OnExecuteStringCommandPreHook(const CNETMsg_StringCmd_t &aMessage)
{
	META_RES eResult = OnExecuteStringCommandPre(META_IFACEPTR(CServerSideClientBase), aMessage);

	RETURN_META_VALUE(eResult, eResult >= MRES_HANDLED);
}

bool MenuSystem_Plugin::OnProcessRespondCvarValueHook(const CCLCMsg_RespondCvarValue_t &aMessage)
{
	RETURN_META_VALUE(MRES_IGNORED, OnProcessRespondCvarValue(META_IFACEPTR(CServerSideClientBase), aMessage));
}

bool MenuSystem_Plugin::OnProcessMoveHook(const CCLCMsg_Move_t &aMessage)
{
	META_RES eResult = OnProcessMovePre(META_IFACEPTR(CServerSideClientBase), aMessage);

	RETURN_META_VALUE(eResult, eResult >= MRES_HANDLED);
}

void MenuSystem_Plugin::OnDisconectClientHook(ENetworkDisconnectionReason eReason)
{
	OnDisconectClient(META_IFACEPTR(CServerSideClientBase), eReason);

	RETURN_META(MRES_IGNORED);
}

#include <tier0/memdbgon.h>

void MenuSystem_Plugin::SendSetConVarMessage(IRecipientFilter *pFilter, CUtlVector<CVar_t> &vecCvars)
{
	auto *pSetConVarMessage = m_pSetConVarMessage;

	Assert(pSetConVarMessage);

	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		const auto &aConcat = g_aEmbedConcat;

		CBufferStringN<1024> sBuffer;
		CConcatLineBuffer aConcatBuffer(&aConcat, &sBuffer);

		aConcatBuffer.Append(pSetConVarMessage->GetUnscopedName());

		for(const auto &[pszName, pszValue] : vecCvars)
		{
			aConcatBuffer.Append<true, true>(pszName, pszValue);
		}

		CLogger::Detailed(sBuffer);
	}

	auto *pMessage = pSetConVarMessage->AllocateMessage()->As<CNETMsg_SetConVar_t>();

	auto *pMessageCVars = pMessage->mutable_convars();

	for(const auto &[pszName, pszValue] : vecCvars)
	{
		auto *pConVar = pMessageCVars->add_cvars();

		pConVar->set_name(pszName);
		pConVar->set_value(pszValue);
	}

	g_pGameEventSystem->PostEventAbstract(-1, false, pFilter, pSetConVarMessage, pMessage, 0);

	delete pMessage;
}

void MenuSystem_Plugin::SendCvarValueQuery(IRecipientFilter *pFilter, const char *pszName, int iCookie)
{
	auto *pGetCvarValueMessage = m_pGetCvarValueMessage;

	auto *pMessage = pGetCvarValueMessage->AllocateMessage()->As<CSVCMsg_GetCvarValue_t>();

	pMessage->set_cvar_name(pszName);
	pMessage->set_cookie(iCookie);

	g_pGameEventSystem->PostEventAbstract(-1, false, pFilter, pGetCvarValueMessage, pMessage, 0);

	delete pMessage;
}

void MenuSystem_Plugin::SendChatMessage(IRecipientFilter *pFilter, int iEntityIndex, bool bIsChat, const char *pszChatMessageFormat, const char *pszParam1, const char *pszParam2, const char *pszParam3, const char *pszParam4)
{
	auto *pSayText2Message = m_pSayText2Message;

	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		const auto &aConcat = g_aEmbedConcat;

		CBufferStringN<256> sHead;

		sHead.Insert(0, "Send chat message (");
		sHead.Insert(sHead.Length(), pSayText2Message->GetUnscopedName());
		sHead.Insert(sHead.Length(), ")");

		CBufferStringN<1024> sBuffer;

		aConcat.AppendHeadToBuffer(sBuffer, sHead.Get());
		aConcat.AppendToBuffer(sBuffer, "Entity index", iEntityIndex);
		aConcat.AppendToBuffer(sBuffer, "Is chat", bIsChat);
		aConcat.AppendStringToBuffer(sBuffer, "Chat message", pszChatMessageFormat);

		if(pszParam1 && *pszParam1)
		{
			aConcat.AppendStringToBuffer(sBuffer, "Parameter #1", pszParam1);
		}

		if(pszParam2 && *pszParam2)
		{
			aConcat.AppendStringToBuffer(sBuffer, "Parameter #2", pszParam2);
		}

		if(pszParam3 && *pszParam3)
		{
			aConcat.AppendStringToBuffer(sBuffer, "Parameter #3", pszParam3);
		}

		if(pszParam4 && *pszParam4)
		{
			aConcat.AppendStringToBuffer(sBuffer, "Parameter #4", pszParam4);
		}

		CLogger::Detailed(sBuffer);
	}

	auto *pMessage = pSayText2Message->AllocateMessage()->As<CUserMessageSayText2_t>();

	pMessage->set_entityindex(iEntityIndex);
	pMessage->set_chat(bIsChat);
	pMessage->set_messagename(pszChatMessageFormat);
	pMessage->set_param1(pszParam1);
	pMessage->set_param2(pszParam2);
	pMessage->set_param3(pszParam3);
	pMessage->set_param4(pszParam4);

	g_pGameEventSystem->PostEventAbstract(-1, false, pFilter, pSayText2Message, pMessage, 0);

	delete pMessage;
}

void MenuSystem_Plugin::SendTextMessage(IRecipientFilter *pFilter, int iDestination, size_t nParamCount, const char *pszParam, ...)
{
	auto *pTextMsg = m_pTextMsgMessage;

	if(CLogger::IsChannelEnabled(LV_DETAILED))
	{
		const auto &aConcat = g_aEmbedConcat;

		CBufferStringN<256> sHead;

		sHead.Insert(0, "Send text message (");
		sHead.Insert(sHead.Length(), pTextMsg->GetUnscopedName());
		sHead.Insert(sHead.Length(), ")");

		CBufferStringN<1024> sBuffer;

		aConcat.AppendHeadToBuffer(sBuffer, sHead.Get());
		aConcat.AppendToBuffer(sBuffer, "Destination", iDestination);
		aConcat.AppendToBuffer(sBuffer, "Parameter", pszParam);
		CLogger::Detailed(sBuffer);
	}

	auto *pMessage = pTextMsg->AllocateMessage()->As<CUserMessageTextMsg_t>();

	pMessage->set_dest(iDestination);
	pMessage->add_param(pszParam);
	nParamCount--;

	// Parse incoming parameters.
	if(0 < nParamCount)
	{
		va_list aParams;

		va_start(aParams, pszParam);

		size_t n = 0;

		do
		{
			pMessage->add_param(va_arg(aParams, const char *));

			n++;
		}
		while(n < nParamCount);

		va_end(aParams);
	}

	g_pGameEventSystem->PostEventAbstract(-1, false, pFilter, pTextMsg, pMessage, 0);

	delete pMessage;
}

// void MenuSystem_Plugin::SendVGUIMenuMessage(IRecipientFilter *pFilter, const char *pszName, const bool *pIsShow, KeyValues3 *pKeys)
// {
// 	auto *pVGUIMenuMsg = m_pVGUIMenuMessage;

// 	if(CLogger::IsChannelEnabled(LV_DETAILED))
// 	{
// 		const auto &aConcat = g_aEmbedConcat, 
// 		           &aConcat2 = g_aEmbed2Concat;

// 		CBufferStringN<1024> sBuffer;

// 		sBuffer.Format("Send message (%s):\n", pVGUIMenuMsg->GetUnscopedName());
// 		aConcat.AppendToBuffer(sBuffer, "Name", pszName ? pszName : "<none>");

// 		if(pIsShow)
// 		{
// 			aConcat.AppendToBuffer(sBuffer, "Show", *pIsShow);
// 		}
// 		else
// 		{
// 			aConcat.AppendToBuffer(sBuffer, "Show", "<none>");
// 		}

// 		if(pKeys)
// 		{
// 			int nMemberCount = pKeys->GetMemberCount();

// 			if(nMemberCount)
// 			{
// 				aConcat.AppendToBuffer(sBuffer, "Keys");

// 				KV3MemberId_t i = 0;

// 				do
// 				{
// 					KeyValues3 *pMember = pKeys->GetMember(i);
// 					aConcat2.AppendToBuffer(sBuffer, "name", pKeys->GetMemberName(i));
// 					aConcat2.AppendToBuffer(sBuffer, "value", pMember->GetString());

// 					i++;
// 				}
// 				while(i < nMemberCount);
// 			}
// 			else
// 			{
// 				aConcat.AppendToBuffer(sBuffer, "Keys", "<no members>");
// 			}
// 		}
// 		else
// 		{
// 			aConcat.AppendToBuffer(sBuffer, "Keys", "<none>");
// 		}

// 		CLogger::Detailed(sBuffer);
// 	}

// 	auto *pMessage = pVGUIMenuMsg->AllocateMessage()->As<CCSUsrMsg_VGUIMenu_t>();

// 	if(pszName)
// 	{
// 		pMessage->set_name(pszName);
// 	}

// 	if(pIsShow)
// 	{
// 		pMessage->set_show(*pIsShow);
// 	}

// 	if(pKeys)
// 	{
// 		int nMemberCount = pKeys->GetMemberCount();

// 		if(nMemberCount)
// 		{
// 			KV3MemberId_t i = 0;

// 			do
// 			{
// 				KeyValues3 *pMember = pKeys->GetMember(i);

// 				auto *pMessageKeys = pMessage->add_keys();

// 				pMessageKeys->set_name(pKeys->GetMemberName(i));
// 				pMessageKeys->set_value(pMember->GetString());

// 				i++;
// 			}
// 			while(i < nMemberCount);
// 		}
// 	}

// 	g_pGameEventSystem->PostEventAbstract(-1, false, pFilter, pVGUIMenuMsg, pMessage, 0);

// 	delete pMessage;
// }

#include <tier0/memdbgoff.h>

void MenuSystem_Plugin::OnStartupServer(CNetworkGameServerBase *pNetServer, const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession)
{
	if(g_pNetworkGameServer && g_pNetworkGameServer != pNetServer)
	{
		SH_REMOVE_HOOK(CNetworkGameServerBase, ConnectClient, g_pNetworkGameServer, SH_MEMBER(this, &MenuSystem_Plugin::OnConnectClientHook), true);
	}

	SH_ADD_HOOK(CNetworkGameServerBase, ConnectClient, pNetServer, SH_MEMBER(this, &MenuSystem_Plugin::OnConnectClientHook), true);

	g_pNetworkGameServer = pNetServer;

	{
		bool (MenuSystem_Plugin::*pfnIntializers[])(char *error, size_t maxlen) = 
		{
			&MenuSystem_Plugin::RegisterSource2Server,
			&MenuSystem_Plugin::RegisterNetMessages,
			&MenuSystem_Plugin::HookGameEvents,
		};

		char sMessage[256];

		for(const auto &aInitializer : pfnIntializers)
		{
			if(!(this->*(aInitializer))(sMessage, sizeof(sMessage)))
			{
				CLogger::WarningFormat("%s\n", sMessage);
			}
		}
	}
}

void MenuSystem_Plugin::OnConnectClient(CNetworkGameServerBase *pNetServer, CServerSideClientBase *pClient)
{
	if(pClient)
	{
		SH_ADD_HOOK(CServerSideClientBase, PerformDisconnection, pClient, SH_MEMBER(this, &MenuSystem_Plugin::OnDisconectClientHook), false);

		if(pClient->IsFakeClient())
		{
			return;
		}

		SH_ADD_HOOK(CServerSideClientBase, ExecuteStringCommand, pClient, SH_MEMBER(this, &MenuSystem_Plugin::OnExecuteStringCommandPreHook), false);
		SH_ADD_HOOK(CServerSideClientBase, ProcessRespondCvarValue, pClient, SH_MEMBER(this, &MenuSystem_Plugin::OnProcessRespondCvarValueHook), false);
		SH_ADD_HOOK(CServerSideClientBase, ProcessMove, pClient, SH_MEMBER(this, &MenuSystem_Plugin::OnProcessMoveHook), false);
	}
	else
	{
		AssertMsg(0, "Failed to get a server side client pointer");

		return;
	}

	auto *pPlayer = reinterpret_cast<CServerSideClient *>(pClient);

	auto aSlot = pClient->GetPlayerSlot();

	auto &aPlayer = GetPlayerData(aSlot);

	// Get "cl_language" cvar value from a client.
	{

		CSingleRecipientFilter aFilter(aSlot);

		const char *pszCvarName = MENUSYSTEM_CLIENT_LANGUAGE_CVAR_NAME;

		int iCookie {};

		{
			auto sConVarSymbol = GetConVarSymbol(pszCvarName);

			auto iFound = m_mapConVarCookies.Find(sConVarSymbol);

			if(iFound == m_mapConVarCookies.InvalidIndex())
			{
				iCookie = 0;
				m_mapConVarCookies.Insert(sConVarSymbol, iCookie);
			}
			else
			{
				auto &iFoundCookie = m_mapConVarCookies.Element(iFound);

				iFoundCookie++;
				iCookie = iFoundCookie;
			}
		}

		SendCvarValueQuery(&aFilter, pszCvarName, iCookie);
	}

	aPlayer.OnConnected(pPlayer);
}

void MenuSystem_Plugin::OnCheckTransmit(ISource2GameEntities *pGameEntities, CCheckTransmitInfo **ppInfoList, int nInfoCount, CBitVec<MAX_EDICTS> &bvUnionTransmitEdicts, CBitVec<MAX_EDICTS> &bvUnknown, const Entity2Networkable_t **pNetworkables, const uint16 *pEntityIndicies, int nEntities, bool bEnablePVSBits)
{
	// if(CLogger::IsChannelEnabled(LV_DETAILED))
	// {
	// 	CLogger::DetailedFormat("%s(pGameEntities = %p, ppInfoList = %p, nInfoCount = %d, &bvUnionTransmitEdicts = %p, pNetworkables = %p, nEntities = %d)\n", __FUNCTION__, pGameEntities, ppInfoList, nInfoCount, &bvUnionTransmitEdicts, pNetworkables, nEntities);
	// }

	for(int i = 0; i < nInfoCount; i++)
	{
		auto *pInfo = ppInfoList[i];

		auto aInfoSlot = pInfo->m_nPlayerSlot;

		for(auto &aPlayer : m_aPlayers)
		{
			if(!aPlayer.IsConnected())
			{
				continue;
			}

			auto aPlayerSlot = aPlayer.GetServerSideClient()->GetPlayerSlot();

			if(aInfoSlot == aPlayerSlot)
			{
				continue;
			}

			auto &vecMenus = aPlayer.GetMenus();

			for(const auto &[_, pMenu] : vecMenus)
			{
				auto *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

				if(!pInternalMenu)
				{
					continue;
				}

				if(!pInternalMenu->GetRecipients().IsBitSet(aInfoSlot))
				{
					for(const auto *pMenuEntity : pInternalMenu->GetActiveEntities())
					{
						pInfo->m_pTransmitEntity->Clear(pMenuEntity->GetEntityIndex().Get());
					}
				}
			}
		}
	}
}

META_RES MenuSystem_Plugin::OnExecuteStringCommandPre(CServerSideClientBase *pClient, const CNETMsg_StringCmd_t &aMessage)
{
	const char *pszFullCommand = aMessage.command().c_str();

	if(m_aEnableClientCommandDetailsConVar.GetBool() && CLogger::IsChannelEnabled(LV_DETAILED))
	{
		const auto &aConcat = g_aEmbedConcat;

		CBufferStringN<1024> sBuffer;

		aConcat.AppendHeadToBuffer(sBuffer, aMessage.GetTypeName().c_str());
		aConcat.AppendToBuffer(sBuffer, "Tick", GetGameGlobals()->tickcount);
		aConcat.AppendStringToBuffer(sBuffer, "Player name", pClient->GetClientName());
		aConcat.AppendStringToBuffer(sBuffer, "Command", pszFullCommand);
		aConcat.AppendToBuffer(sBuffer, "Prediction sync", aMessage.prediction_sync());

		CLogger::Detailed(sBuffer);
	}

	CUtlVector<CUtlString> vecArgs;

	V_SplitString(pszFullCommand, " ", vecArgs);

	if(vecArgs.Count() > 2)
	{
		return MRES_IGNORED;
	}

	for(auto &sArg : vecArgs)
	{
		sArg.Trim(' ');
	}

	const char *pszCommand = vecArgs[0];

	static const char szSpecPrefix[] = "spec_";

	uintp nSpecPrefixLastIndex = sizeof(szSpecPrefix) - 1;

	if(!V_strncmp(pszFullCommand, szSpecPrefix, nSpecPrefixLastIndex))
	{
		const char *pszSpecCommand = &pszCommand[nSpecPrefixLastIndex];

		auto aPlayerSlot = pClient->GetPlayerSlot();

		auto &aPlayer = GetPlayerData(aPlayerSlot);

		if(!aPlayer.IsConnected())
		{
			return MRES_IGNORED;
		}

		const auto &vecMenus = aPlayer.GetMenus();

		if(!vecMenus.Count())
		{
			return MRES_IGNORED;
		}

		if(!V_strcmp(pszSpecCommand, "mode") || !V_strcmp(pszSpecCommand, "prev") || !V_strcmp(pszSpecCommand, "next") || !V_strcmp(pszSpecCommand, "goto"))
		{
			// Call handlers. It will become the post.
			SET_META_RESULT(MRES_HANDLED);
			SH_GLOB_SHPTR->DoRecall();
			(pClient->*(&CServerSideClient::ExecuteStringCommand))(aMessage);

			auto *pPlayerController = instance_upper_cast<CBasePlayerController *>(g_pEntitySystem->GetEntityInstance(CEntityIndex(aPlayerSlot.GetClientIndex())));

			if(!pPlayerController)
			{
				return MRES_SUPERCEDE;
			}

			CBasePlayerPawn *pPlayerPawn = CBasePlayerController_Helper::GetPawnAccessor(pPlayerController)->Get();

			if(!pPlayerPawn)
			{
				return MRES_SUPERCEDE;
			}

			auto *pCSPlayerPawn = instance_upper_cast<CCSPlayerPawnBase *>(pPlayerPawn);

			CPlayer_ObserverServices *pObserverServices = CCSPlayerPawnBase_Helper::GetObserverServicesAccessor(pCSPlayerPawn);

			if(!pObserverServices)
			{
				return MRES_SUPERCEDE;
			}

			uint8 iObserverMode = CPlayer_ObserverServices_Helper::GetObserverModeAccessor(pObserverServices);

			// CPlayer_ObserverServices_Helper::GetObserverModeAccessor(pObserverServices) = OBS_MODE_NONE;

			if(iObserverMode != OBS_MODE_NONE && iObserverMode != OBS_MODE_IN_EYE)
			{
				for(const auto &[_, pMenu] : vecMenus)
				{
					CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

					if(pInternalMenu)
					{
						AttachMenuInstanceToEntity(pInternalMenu, pCSPlayerPawn);
					}
				}

				return MRES_SUPERCEDE;
			}

			// auto *pCSObserverPawn = instance_upper_cast<CCSObserverPawn *>(pPlayerPawn);

			CHandle<CBaseEntity> hObserverTarget = CPlayer_ObserverServices_Helper::GetObserverTargetAccessor(pObserverServices);

			auto *pCSObserverTarget = hObserverTarget.Get();

			if(!pCSObserverTarget)
			{
				return MRES_SUPERCEDE;
			}

			CCSPlayerPawn *pCCSPlayerObserverTargetPawn = instance_upper_cast<CCSPlayerPawn *>(pCSObserverTarget);

			FOR_EACH_VEC(vecMenus, i)
			{
				const auto &[_, pMenu] = vecMenus[i];

				CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

				if(pInternalMenu)
				{
					AttachMenuInstanceToCSPlayer(i, pInternalMenu, pCCSPlayerObserverTargetPawn);
				}
			}

			return MRES_SUPERCEDE;
		}
		else if(!V_strcmp(pszSpecCommand, "player")) // "spec_player" - changing a observer target.
		{
			for(const auto &[_, pMenu] : vecMenus)
			{
				CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

				if(pInternalMenu)
				{
					int iClient = V_atoi(vecArgs[1]), 
					    iFoundItem = FindItemIndexFromClientIndex(iClient);

					if(iFoundItem != -1)
					{
						pInternalMenu->OnSelect(aPlayerSlot, iFoundItem % 10);
					}

					return MRES_SUPERCEDE;
				}
			}

			return MRES_HANDLED;
		}
	}

	return MRES_IGNORED;
}

META_RES MenuSystem_Plugin::OnProcessMovePre(CServerSideClientBase *pClient, const CCLCMsg_Move_t &aMessage)
{
	static bool s_bSkipFirstCall = true;

	if(s_bSkipFirstCall) // To construct root "cmds" (NEEDED).
	{
		s_bSkipFirstCall = false;

		return MRES_IGNORED;
	}

	auto &aUserCmdGameData = GetGameDataStorage().GetUserCmd();

	bf_write sDeltaPacket((void *)&aMessage.data()[0], aMessage.data().size());

	int iLastCommandNumber = aMessage.last_command_number();

	static uintp s_mMaxCmds = 8;

	CCSGOUserCmd *pRootCmd = aUserCmdGameData.Get(), 
	             *pIterCmd = pRootCmd, 
	             *pPrevCmd = NULL;

	Assert(pRootCmd);

	auto *pEndCmd = pIterCmd + s_mMaxCmds;

	void *pMarginController = NULL;

	int nCmds = 0;

	float flMargin = aMessage.GetMargin();

	do
	{
		if(!pIterCmd->DeltaDecode(sDeltaPacket, pPrevCmd, pMarginController, flMargin)) // Decode delta data to.
		{
			break;
		}

		nCmds++;

		pPrevCmd = pIterCmd;
	}
	while(pIterCmd = pIterCmd->m_pNext);

	pIterCmd = pRootCmd;

	auto *pPlayerController = instance_upper_cast<CBasePlayerController *>(g_pEntitySystem->GetEntityInstance(CEntityIndex(pClient->GetPlayerSlot().GetClientIndex())));

	int numCmds = (iLastCommandNumber + 1) - nCmds;

	if(numCmds <= 0)
	{
		CLogger::WarningFormat("[%s] sent move msg with invalid last_command_number %d commands.\n", pClient->GetClientName(), numCmds);

		return MRES_SUPERCEDE;
	}

	uint nHaveChanges = 0;

	do
	{
		pIterCmd->m_cmdNum = numCmds++;
		aUserCmdGameData.Read(pPlayerController, pIterCmd); // Not sensitive to "pPlayerController" value / Unpacks PB
		nHaveChanges += ProcessUserCmd(pClient, pIterCmd);
	}
	while(pIterCmd = pIterCmd->m_pNext);

	if(nHaveChanges)
	{
		aUserCmdGameData.ProcessWithPlayerController(pPlayerController, pRootCmd, nCmds, false, flMargin); // Handle PBs.

		return MRES_SUPERCEDE;
	}

	return MRES_IGNORED;
}

bool MenuSystem_Plugin::OnProcessRespondCvarValue(CServerSideClientBase *pClient, const CCLCMsg_RespondCvarValue_t &aMessage)
{
	auto sFoundSymbol = FindConVarSymbol(aMessage.name().c_str());

	if(!sFoundSymbol.IsValid())
	{
		return false;
	}

	auto iFound = m_mapConVarCookies.Find(sFoundSymbol);

	if(!m_mapConVarCookies.IsValidIndex(iFound))
	{
		return false;
	}

	const auto &itCookie = m_mapConVarCookies.Element(iFound);

	if(itCookie != aMessage.cookie())
	{
		return false;
	}

	auto iLanguageFound = m_mapLanguages.Find(FindLanguageSymbol(aMessage.value().c_str()));

	if(!m_mapLanguages.IsValidIndex(iLanguageFound))
	{
		return false;
	}

	auto aSlot = pClient->GetPlayerSlot();

	auto &aPlayer = GetPlayerData(aSlot);

	auto &itLanguage = m_mapLanguages.Element(iLanguageFound);

	aPlayer.OnLanguageChanged(aSlot, &itLanguage);

	{
		CUtlVector<CUtlString> vecMessages;

		auto aWarnings = CLogger::CreateWarningsScope();

		aPlayer.TranslatePhrases(this, m_aServerLanguage, vecMessages);

		for(const auto &sMessage : vecMessages)
		{
			aWarnings.Push(sMessage.Get());
		}

		aWarnings.SendColor([&](Color rgba, const CUtlString &sContext)
		{
			CLogger::Warning(rgba, sContext);
		});
	}

	return true;
}

void MenuSystem_Plugin::OnDisconectClient(CServerSideClientBase *pClient, ENetworkDisconnectionReason eReason)
{
	SH_REMOVE_HOOK(CServerSideClientBase, PerformDisconnection, pClient, SH_MEMBER(this, &MenuSystem_Plugin::OnDisconectClientHook), false);

	if(pClient->IsFakeClient())
	{
		return;
	}

	SH_REMOVE_HOOK(CServerSideClientBase, ProcessMove, pClient, SH_MEMBER(this, &MenuSystem_Plugin::OnProcessMoveHook), false);
	SH_REMOVE_HOOK(CServerSideClientBase, ProcessRespondCvarValue, pClient, SH_MEMBER(this, &MenuSystem_Plugin::OnProcessRespondCvarValueHook), false);
	SH_REMOVE_HOOK(CServerSideClientBase, ExecuteStringCommand, pClient, SH_MEMBER(this, &MenuSystem_Plugin::OnExecuteStringCommandPreHook), false);

	auto *pPlayer = reinterpret_cast<CServerSideClient *>(pClient);

	auto aSlot = pClient->GetPlayerSlot();

	auto &aPlayer = GetPlayerData(aSlot);

	// Destroy all player's menus.
	{
		auto &vecMenus = aPlayer.GetMenus();

		for(const auto &[_, pMenu] : vecMenus)
		{
			auto *pMemBlock = m_MenuAllocator.FindMemBlock(pMenu);

			if(!pMemBlock)
			{
				continue;
			}

			CMenu *pInternalMenu = m_MenuAllocator.GetInstanceByMemBlock(pMemBlock);

			if(!pInternalMenu)
			{
				continue;
			}

			CloseInternalMenu(pInternalMenu, IMenuHandler::MenuEnd_Disconnected, false);
			m_MenuAllocator.ReleaseByMemBlock(pMemBlock);
		}

		vecMenus.Purge();
	}

	aPlayer.OnDisconnected(pPlayer, eReason);
}

bool MenuSystem_Plugin::ProcessUserCmd(CServerSideClientBase *pClient, CCSGOUserCmd *pMessage)
{
	const auto *pBaseUserCmd = pMessage->has_base() ? &pMessage->base() : nullptr;

	// Dump runcmd proto.
	if(m_aEnablePlayerRunCmdDetailsConVar.Get() && CLogger::IsChannelEnabled(LV_DETAILED))
	{
		const auto &aConcat = g_aEmbedConcat, 
		           &aConcat2 = g_aEmbed2Concat, 
		           &aConcat3 = g_aEmbed3Concat;

		CBufferStringN<2048> sBuffer;

		aConcat.AppendHeadToBuffer(sBuffer, pMessage->GetTypeName().c_str());
		aConcat.AppendPointerToBuffer(sBuffer, "Base", pBaseUserCmd);

		if(pBaseUserCmd)
		{
			aConcat2.AppendToBuffer(sBuffer, "Legacy Command Number", pBaseUserCmd->legacy_command_number());
			aConcat2.AppendToBuffer(sBuffer, "Client tick", pBaseUserCmd->client_tick());

			if(pBaseUserCmd->has_buttons_pb())
			{
				aConcat2.AppendToBuffer(sBuffer, "Buttons PB");

				const auto &aButtonsPB = pBaseUserCmd->buttons_pb();

				aConcat3.AppendToBuffer(sBuffer, "State", (uint64)aButtonsPB.buttonstate1());
				aConcat3.AppendToBuffer(sBuffer, "Changed", (uint64)aButtonsPB.buttonstate2());
				aConcat3.AppendToBuffer(sBuffer, "Of scroll", (uint64)aButtonsPB.buttonstate3());
			}

			if(pBaseUserCmd->has_viewangles())
			{
				const auto &aViewAnglesPB = pBaseUserCmd->viewangles();

				aConcat2.AppendToBuffer(sBuffer, "View angles", QAngle(aViewAnglesPB.x(), aViewAnglesPB.y(), aViewAnglesPB.z()));
			}

			aConcat2.AppendToBuffer(sBuffer, "Forward move", pBaseUserCmd->forwardmove());
			aConcat2.AppendToBuffer(sBuffer, "Left move", pBaseUserCmd->leftmove());
			aConcat2.AppendToBuffer(sBuffer, "Up move", pBaseUserCmd->upmove());
			aConcat2.AppendToBuffer(sBuffer, "Impulse", pBaseUserCmd->impulse());
			aConcat2.AppendToBuffer(sBuffer, "Weapon select", pBaseUserCmd->weaponselect());
			aConcat2.AppendToBuffer(sBuffer, "Random seed", pBaseUserCmd->random_seed());
			aConcat2.AppendToBuffer(sBuffer, "Mouse X", pBaseUserCmd->mousedx());
			aConcat2.AppendToBuffer(sBuffer, "Mouse Y", pBaseUserCmd->mousedy());
			aConcat2.AppendHandleToBuffer(sBuffer, "Pawn entity handle", pBaseUserCmd->pawn_entity_handle());

			const auto &aSubtickMoves = pBaseUserCmd->subtick_moves();
			
			if(aSubtickMoves.size())
			{
				aConcat2.AppendToBuffer(sBuffer, "Subtick moves");
			}

			// Subtick data.
			for(const auto &aSubtickMove : aSubtickMoves)
			{
				aConcat3.AppendToBuffer(sBuffer, "Button", (uint64)aSubtickMove.button());
				aConcat3.AppendToBuffer(sBuffer, "Pressed", aSubtickMove.pressed());
				aConcat3.AppendToBuffer(sBuffer, "When", aSubtickMove.when());
				aConcat3.AppendToBuffer(sBuffer, "Analog forward delta", aSubtickMove.analog_forward_delta());
				aConcat3.AppendToBuffer(sBuffer, "Analog left delta", aSubtickMove.analog_left_delta());
			}

			const auto &aMoveCRC = pBaseUserCmd->move_crc();

			aConcat2.AppendBytesToBuffer(sBuffer, "Move CRC", (const byte *)aMoveCRC.data(), aMoveCRC.size());
			aConcat2.AppendToBuffer(sBuffer, "Consumed Server Angle Changes", pBaseUserCmd->consumed_server_angle_changes());
			aConcat2.AppendToBuffer(sBuffer, "Flags", pBaseUserCmd->cmd_flags());
		}

		{
			aConcat.AppendToBuffer(sBuffer, "Input history");

			const auto &aInputHistory = pMessage->input_history();

			for(const auto &aInput : aInputHistory)
			{
				if(aInput.has_view_angles())
				{
					const auto &aViewAnglesPB = aInput.view_angles();

					aConcat2.AppendToBuffer(sBuffer, "View angles", QAngle(aViewAnglesPB.x(), aViewAnglesPB.y(), aViewAnglesPB.z()));
				}

				aConcat2.AppendToBuffer(sBuffer, "Render tick count", aInput.render_tick_count());
				aConcat2.AppendToBuffer(sBuffer, "Render tick fraction", aInput.render_tick_fraction());
				aConcat2.AppendToBuffer(sBuffer, "Player tick count", aInput.player_tick_count());
				aConcat2.AppendToBuffer(sBuffer, "Player tick fraction", aInput.player_tick_fraction());

				if(aInput.has_cl_interp())
				{
					aConcat2.AppendToBuffer(sBuffer, "CL interpolation");

					const auto &aCLInterp = aInput.cl_interp();

					aConcat3.AppendToBuffer(sBuffer, "Fraction", aCLInterp.frac());
				}

				if(aInput.has_sv_interp0())
				{
					aConcat2.AppendToBuffer(sBuffer, "Server interpolation");

					const auto &aSVInterpolation0 = aInput.sv_interp0();

					aConcat3.AppendToBuffer(sBuffer, "Source tick", aSVInterpolation0.src_tick());
					aConcat3.AppendToBuffer(sBuffer, "Destination tick", aSVInterpolation0.dst_tick());
					aConcat3.AppendToBuffer(sBuffer, "Fraction", aSVInterpolation0.frac());
				}

				if(aInput.has_sv_interp1())
				{
					aConcat2.AppendToBuffer(sBuffer, "Server interpolation (2)");

					const auto &aSVInterpolation1 = aInput.sv_interp1();

					aConcat3.AppendToBuffer(sBuffer, "Source tick", aSVInterpolation1.src_tick());
					aConcat3.AppendToBuffer(sBuffer, "Destination tick", aSVInterpolation1.dst_tick());
					aConcat3.AppendToBuffer(sBuffer, "Fraction", aSVInterpolation1.frac());
				}

				if(aInput.has_player_interp())
				{
					aConcat2.AppendToBuffer(sBuffer, "Player interpolation");

					const auto &aPlayerInterpolation = aInput.player_interp();

					aConcat3.AppendToBuffer(sBuffer, "Source tick", aPlayerInterpolation.src_tick());
					aConcat3.AppendToBuffer(sBuffer, "Destination tick", aPlayerInterpolation.dst_tick());
					aConcat3.AppendToBuffer(sBuffer, "Fraction", aPlayerInterpolation.frac());
				}

				aConcat2.AppendToBuffer(sBuffer, "Frame number", aInput.frame_number());
				aConcat2.AppendToBuffer(sBuffer, "Target entity index", aInput.target_ent_index());

				if(aInput.has_shoot_position())
				{
					const auto &aVectorPB = aInput.shoot_position();

					aConcat2.AppendToBuffer(sBuffer, "Shot position", Vector(aVectorPB.x(), aVectorPB.y(), aVectorPB.z()));
				}

				if(aInput.has_target_head_pos_check())
				{
					const auto &aVectorPB = aInput.target_head_pos_check();

					aConcat2.AppendToBuffer(sBuffer, "Target head position check", Vector(aVectorPB.x(), aVectorPB.y(), aVectorPB.z()));
				}

				if(aInput.has_target_abs_pos_check())
				{
					const auto &aVectorPB = aInput.target_abs_pos_check();

					aConcat2.AppendToBuffer(sBuffer, "Target abs position check", Vector(aVectorPB.x(), aVectorPB.y(), aVectorPB.z()));
				}

				if(aInput.has_target_abs_ang_check())
				{
					const auto &aAnglesPB = aInput.target_abs_ang_check();

					aConcat2.AppendToBuffer(sBuffer, "Target abs angles check", QAngle(aAnglesPB.x(), aAnglesPB.y(), aAnglesPB.z()));
				}
			}
		}

		{
			aConcat.AppendToBuffer(sBuffer, "Start History index attack", pMessage->attack1_start_history_index());
			aConcat.AppendToBuffer(sBuffer, "Start History index attack (2)", pMessage->attack2_start_history_index());
			aConcat.AppendToBuffer(sBuffer, "Start History index attack (3)", pMessage->attack3_start_history_index());
		}

		aConcat.AppendToBuffer(sBuffer, "Left hand desired", pMessage->left_hand_desired());

		{
			aConcat.AppendToBuffer(sBuffer, "Is predicting body shot FX", pMessage->is_predicting_body_shot_fx());
			aConcat.AppendToBuffer(sBuffer, "Is predicting head shot FX", pMessage->is_predicting_head_shot_fx());
			aConcat.AppendToBuffer(sBuffer, "Is predicting kill ragdolls", pMessage->is_predicting_kill_ragdolls());
		}

		CLogger::Detailed(sBuffer);
	}

	auto aPlayerSlot = pClient->GetPlayerSlot();

	auto &aPlayer = GetPlayerData(aPlayerSlot);

	auto &vecMenus = aPlayer.GetMenus();

	if(!vecMenus.Count())
	{
		return false;
	}

	if(pBaseUserCmd)
	{
		int nClientTick = pBaseUserCmd->client_tick();

		// Menu switcher.
		{
			bool bLeftHandDesired = pMessage->left_hand_desired();

			bool &bMenuToggler = aPlayer.GetMenuTogglerStateRef();

			if(bLeftHandDesired != bMenuToggler)
			{
				int &mMenuTogglerClientTick = aPlayer.GetMenuTogglerClientTickRef();

				if(nClientTick != mMenuTogglerClientTick)
				{
					OnMenuSwitch(aPlayerSlot);
					mMenuTogglerClientTick = nClientTick;
				}

				bMenuToggler = bLeftHandDesired;
			}
		}

		// Change the weapon selection to an item of the menu.
		// DISABLED: This causes double execution when users have menuselect bound to 1-9 keys
		// The menuselect command already handles menu item selection properly
		/*
		if(pBaseUserCmd->has_weaponselect())
		{
			int iClient = aPlayerSlot.GetClientIndex();
			int iWeaponIndex = pBaseUserCmd->weaponselect();

			int iFoundItem = FindItemIndexFromMyWeapons(iClient, iWeaponIndex);

			if(iFoundItem != -1)
			{
				for(const auto &[_, pMenu] : vecMenus)
				{
					CMenu *pInternalMenu = m_MenuAllocator.FindAndUpperCast(pMenu);

					if(pInternalMenu)
					{
						bool bSelectResult = pInternalMenu->OnSelect(aPlayerSlot, iFoundItem % 10);

						if(bSelectResult)
						{
							const_cast<CBaseUserCmdPB *>(pBaseUserCmd)->set_weaponselect(0); // Change the cmd weapon to rehandled select a menu item.
						}

						return bSelectResult;
					}
				}
			}
		}
		*/
	}

	return false;
}

CUtlSymbolLarge MenuSystem_Plugin::GetConVarSymbol(const char *pszName)
{
	return m_tableConVars.AddString(pszName);
}

CUtlSymbolLarge MenuSystem_Plugin::FindConVarSymbol(const char *pszName) const
{
	return m_tableConVars.FindString(pszName);
}

CUtlSymbolLarge MenuSystem_Plugin::GetLanguageSymbol(const char *pszName)
{
	return m_tableLanguages.AddString(pszName);
}

CUtlSymbolLarge MenuSystem_Plugin::FindLanguageSymbol(const char *pszName) const
{
	return m_tableLanguages.FindString(pszName);
}
