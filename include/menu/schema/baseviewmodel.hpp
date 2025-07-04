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

#ifndef _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_BASEVIEWMODEL_HPP_
#	define _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_BASEVIEWMODEL_HPP_

#	pragma once

#	include <menu/schema/baseentity.hpp>
#	include <menu/schema.hpp>

#	include <entity2/entityinstance.h>

#	define CBASEVIEWMODEL_CLASS_NAME "CBaseViewModel"

class QAngle;
class Vector;
class CBodyComponent;

class CBaseViewModel : public CBaseEntity
{
public:
	// ...
};

namespace Menu
{
	namespace Schema
	{
		class CBaseViewModel_Helper : virtual public CBaseEntity_Helper
		{
		public:
			void AddListeners(CSystem *pSchemaSystemHelper);
			void Clear();

		public:
			SCHEMA_INSTANCE_ACCESSOR_METHOD(GetViewModelIndexAccessor, CBaseViewModel, uint, m_aOffsets.m_nViewModelIndex);

		private:
			CSystem::CClass *m_pClass;
			CSystem::CClass::Fields::CListenerCallbacksCollector m_aClassFieldsClassbacks;

			struct
			{
				int m_nViewModelIndex = INVALID_SCHEMA_FIELD_OFFSET;
			} m_aOffsets;
		}; // Menu::Schema::CBaseViewModel_Helper
	}; // Menu::Schema
}; // Menu

#endif // _INCLUDE_METAMOD_SOURCE_MENU_SCHEMA_BASEVIEWMODEL_HPP_
