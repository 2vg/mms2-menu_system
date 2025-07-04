
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

Menu::CProvider::CGameDataStorage::CBaseEntity::CBaseEntity()
 :  m_pAcceptInputMethod(nullptr), 
    m_nTeleportOffset(-1)
{
	{
		auto &aCallbacks = m_aAddressCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("CEntityInstance::AcceptInput"), GAMEDATA_ADDRESS_SHARED_LAMBDA_CAPTURE(m_pAcceptInputMethod));

		m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}

	{
		auto &aCallbacks = m_aOffsetCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("CBaseEntity::Teleport"), GAMEDATA_OFFSET_SHARED_LAMBDA_CAPTURE(m_nTeleportOffset));

		m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool Menu::CProvider::CGameDataStorage::CBaseEntity::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages)
{
	return m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void Menu::CProvider::CGameDataStorage::CBaseEntity::Reset()
{
	m_pAcceptInputMethod = nullptr;
	m_nTeleportOffset = -1;
}

void Menu::CProvider::CGameDataStorage::CBaseEntity::AcceptInput(CEntityInstance *pInstance, const char *pInputName, CEntityInstance *pActivator, CEntityInstance *pCaller, variant_t *pValue, int nOutputID) const
{
	m_pAcceptInputMethod(pInstance, pInputName, pActivator, pCaller, pValue, nOutputID);
}

void Menu::CProvider::CGameDataStorage::CBaseEntity::Teleport(CEntityInstance *pInstance, const Vector &vecPosition, const QAngle &angRotation, const Vector &velocity) const
{
	reinterpret_cast<DynLibUtils::VirtualTable *>(pInstance)->CallMethod<void, const Vector &, const QAngle &, const Vector &>(m_nTeleportOffset, vecPosition, angRotation, velocity);
}
