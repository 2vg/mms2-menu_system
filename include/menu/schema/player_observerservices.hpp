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

#ifndef _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_PLAYER_OBSERVERSERVICES_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_PLAYER_OBSERVERSERVICES_HPP_

#	pragma once

#	include <menu/schema.hpp>

#	include <basetypes.h>
#	include <ehandle.h>
#	include <shareddefs.h>

#	define CPLAYER_OBSERVERSERVICES_CLASS_NAME "CPlayer_ObserverServices"

class CBaseEntity;
class CPlayer_ObserverServices;

namespace Menu
{
	namespace Schema
	{
		class CPlayer_ObserverServices_Helper
		{
		public:
			void AddListeners(CSystem *pSchemaSystemHelper);
			void Clear();

		public:
			SCHEMA_COMPONENT_ACCESSOR_METHOD(GetObserverModeAccessor, CPlayer_ObserverServices, uint8, m_aOffsets.m_nObserverMode); // OBS_MODE_* enumuration.
			SCHEMA_COMPONENT_ACCESSOR_METHOD(GetObserverTargetAccessor, CPlayer_ObserverServices, CHandle<CBaseEntity>, m_aOffsets.m_nObserverTarget);

		private:
			CSystem::CClass *m_pClass;
			CSystem::CClass::Fields::CListenerCallbacksCollector m_aClassFieldsClassbacks;

			struct
			{
				int m_nObserverMode = INVALID_SCHEMA_FIELD_OFFSET;
				int m_nObserverTarget = INVALID_SCHEMA_FIELD_OFFSET;
			} m_aOffsets;
		}; // Menu::Schema::CPlayer_ObserverServices_Helper
	}; // Menu::Schema
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_PLAYER_OBSERVERSERVICES_HPP_
