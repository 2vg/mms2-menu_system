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

Menu::CProvider::CGameDataStorage::CUserCmd::CUserCmd()
 :  m_pCmds(nullptr), 
    m_pRead(nullptr), 
    m_pProcessWithPlayerController(nullptr)
{
	{
		auto &aCallbacks = m_aAddressCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("&cmds"), GAMEDATA_ADDRESS_SHARED_LAMBDA_CAPTURE(m_pCmds));
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CBasePlayerController::ReadUsercmd"), GAMEDATA_ADDRESS_SHARED_LAMBDA_CAPTURE(m_pRead));
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CBasePlayerController::ProcessUsercmds"), GAMEDATA_ADDRESS_SHARED_LAMBDA_CAPTURE(m_pProcessWithPlayerController));

		m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
}

bool Menu::CProvider::CGameDataStorage::CUserCmd::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages)
{
	return m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void Menu::CProvider::CGameDataStorage::CUserCmd::Reset()
{
	m_pCmds = nullptr;
	m_pRead = nullptr;
	m_pProcessWithPlayerController = nullptr;
}

CCSGOUserCmd *Menu::CProvider::CGameDataStorage::CUserCmd::Get() const
{
	Assert(m_pCmds);

	return m_pCmds;
}

void Menu::CProvider::CGameDataStorage::CUserCmd::Read(CBasePlayerController *pController, CCSGOUserCmd *pMessage) const
{
	Assert(m_pRead);

	m_pRead(pController, pMessage);
}

void Menu::CProvider::CGameDataStorage::CUserCmd::ProcessWithPlayerController(CBasePlayerController *pController, CCSGOUserCmd *cmds, int numcmds, bool paused, float margin) const
{
	Assert(m_pProcessWithPlayerController);

	m_pProcessWithPlayerController(pController, cmds, numcmds, paused, margin);
}

