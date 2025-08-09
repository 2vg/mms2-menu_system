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

#include <menu/schema/player_observerservices.hpp>

// Undefine Assert macro to avoid conflicts before including tier0 headers
#ifdef Assert
#undef Assert
#endif
#include <tier0/dbg.h>
#include <schemasystem/schemasystem.h>

void Menu::Schema::CPlayer_ObserverServices_Helper::AddListeners(CSystem *pSchemaSystemHelper)
{
	auto &aCallbacks = m_aClassFieldsClassbacks;

	m_pClass = pSchemaSystemHelper->GetClass(CPLAYER_OBSERVERSERVICES_CLASS_NAME);
	Assert(m_pClass);

	aCallbacks.Insert(m_pClass->GetFieldSymbol("m_iObserverMode"), SCHEMA_CLASS_FIELD_SHARED_LAMBDA_CAPTURE(m_aOffsets.m_nObserverMode));
	aCallbacks.Insert(m_pClass->GetFieldSymbol("m_hObserverTarget"), SCHEMA_CLASS_FIELD_SHARED_LAMBDA_CAPTURE(m_aOffsets.m_nObserverTarget));

	m_pClass->GetFields().AddListener(&aCallbacks);
}

void Menu::Schema::CPlayer_ObserverServices_Helper::Clear()
{
	m_aOffsets = {};
}
