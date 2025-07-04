
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

#include <dynlibutils/virtual.hpp>

Menu::CProvider::CGameDataStorage::CBasePlayerPawn::CBasePlayerPawn()
 :  m_nGetEyePositionOffset(-1)
{
	{
		auto &aCallbacks = m_aOffsetCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("CBasePlayerPawn::GetEyePosition"), GAMEDATA_OFFSET_SHARED_LAMBDA_CAPTURE(m_nGetEyePositionOffset));

		m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool Menu::CProvider::CGameDataStorage::CBasePlayerPawn::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages)
{
	return m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void Menu::CProvider::CGameDataStorage::CBasePlayerPawn::Reset()
{
	m_nGetEyePositionOffset = -1;
}

Vector Menu::CProvider::CGameDataStorage::CBasePlayerPawn::GetEyePosition(CEntityInstance *pInstance) const
{
	return reinterpret_cast<DynLibUtils::VirtualTable *>(pInstance)->CallMethod<Vector>(m_nGetEyePositionOffset);
}
